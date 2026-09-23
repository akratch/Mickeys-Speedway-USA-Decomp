#!/usr/bin/env python3
"""Run ``promotion_proof.py`` over every promoted function and tally the result.

The set is derived the way ``tools/progress.py`` counts matched C, never from a
remembered list:

* **resident**: every resident ``STT_FUNC`` the linked ELF defines whose name no
  longer carries a ``glabel``/``alabel`` anywhere under ``asm/``
  (``progress.get_elf_functions`` minus ``progress.get_asm_labelled_names``);
* **overlay**: every overlay ``STT_FUNC`` with a nonzero size whose linked
  extent lies inside exact C ownership in ``config/overlays.us.json`` (a
  matched, non-``NON_MATCHING`` C ``text_ownership`` row, or a
  ``mixed_tu_exact_c_ranges`` island), and whose name no ``.s`` still labels.

``promotion_proof.py`` proves a resident function only through its
``symbol_addrs.us.txt`` row, which must say ``type:func``, ``size:`` and
``matched C``.  A resident function progress counts as matched but whose row
does not say so is outside that contract: no proof can run, so the census
reports it as *uncovered* (a count, and a list under ``--list-uncovered``)
instead of inventing a failure class for it.  ``--require-coverage`` turns
every uncovered function into a failure.

Each proof runs with ``--no-build``; the census refreshes the build once
first.  That is also what makes ``-j`` safe: without ``--no-build`` every
proof's ROM oracle rewrites ``build/mickey.us.z64`` while the others read it,
and the readers fail with ``wb_compare failed with exit 2``.

Passing results are cached in ``build/promotion-proof-census/cache.json``,
keyed per function on its object's hash, the linked ELF's hash, the proof
tools' source hashes and the tracked proof inputs (atlas, generated alias
surface, symbol table, Makefile fragments).  Failures are never cached, so a
warm run re-proves exactly the failing and the changed functions.

Usage:
    tools/promotion_proof_census.py [-j N] [--no-build] [--no-cache]
                                    [--only resident|overlay] [--limit N]
Exit status is 1 when any proof fails (or, with ``--require-coverage``, any
function is uncovered), 0 otherwise.
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import dataclasses
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path
from typing import Callable, Iterable


REPO = Path(__file__).resolve().parent.parent
TOOLS = REPO / "tools"
sys.path.insert(0, str(TOOLS))

ELF_PATH = REPO / "build" / "mickey.us.elf"
ROM_OUT = REPO / "build" / "mickey.us.z64"
MAP_PATH = REPO / "build" / "mickey.us.map"
ATLAS = REPO / "config" / "overlays.us.json"
SYMBOLS = REPO / "symbol_addrs.us.txt"
CACHE = REPO / "build" / "promotion-proof-census" / "cache.json"
CACHE_SCHEMA = "mickey-promotion-proof-census-cache-v1"
SYNTHETIC_VMA = 0xF0000000

# Every file whose content decides a promotion-proof verdict, beyond the
# function's own object and the linked ELF.
TOOL_SOURCES = (
    "tools/promotion_proof.py", "tools/function_preflight.py",
    "tools/reloc_surface.py", "tools/reloc_identity.py",
    "tools/proof_provenance.py", "tools/overlay_tables.py",
    "tools/postprocess_audit.py", "tools/function_history.py",
    "tools/wb_compare.sh", "tools/promotion_proof_census.py",
    "tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
)
PROOF_INPUTS = (
    "config/overlays.us.json", "overlay_undefined_syms.us.txt",
    "symbol_addrs.us.txt", "Makefile",
)


@dataclasses.dataclass(frozen=True)
class Entry:
    symbol: str
    kind: str                 # "resident" | "overlay"
    obj: str | None           # object path relative to the repository
    covered: bool = True      # False: outside promotion-proof's contract
    why_uncovered: str = ""


@dataclasses.dataclass
class Outcome:
    symbol: str
    kind: str
    ok: bool
    identity: str | None = None
    error: str | None = None
    error_class: str | None = None
    cached: bool = False


# ------------------------------------------------------------------ set

def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def map_text_objects(map_text: str) -> dict[str, list[tuple[int, int, str]]]:
    """Output section -> [(start, size, object)] for every input ``.text``."""
    out: dict[str, list[tuple[int, int, str]]] = collections.defaultdict(list)
    section = None
    head = re.compile(r"^(\.[\w.]+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)")
    head_only = re.compile(r"^(\.[\w.]+)\s*$")
    text = re.compile(
        r"^ \.text\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+\.o)\s*$")
    for line in map_text.splitlines():
        match = head.match(line) or head_only.match(line)
        if match:
            section = match.group(1)
            continue
        match = text.match(line)
        if match and section is not None:
            size = int(match.group(2), 16)
            if size:
                out[section].append((int(match.group(1), 16), size, match.group(3)))
    return out


def object_for(objects: dict[str, list[tuple[int, int, str]]], section: str,
               value: int) -> str | None:
    owners = [obj for start, size, obj in objects.get(section, ())
              if start <= value < start + size]
    return owners[0] if len(owners) == 1 else None


def resident_contract_rows(text: str) -> set[str]:
    """Names whose symbol_addrs row satisfies promotion-proof's resident rule."""
    names = set()
    for line in text.splitlines():
        match = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*0x[0-9A-Fa-f]+\s*;\s*//(.*)$", line)
        if (match and "type:func" in match.group(2)
                and len(re.findall(r"\bsize:0x[0-9A-Fa-f]+\b", match.group(2))) == 1
                and re.search(r"\bmatched\s+C\b", match.group(2))):
            names.add(match.group(1))
    return names


def _exact_c(module: dict, start: int, end: int) -> bool:
    for row in module.get("text_ownership", []):
        if (row.get("type") == "c" and row.get("matched") is True
                and row.get("nonmatching") is False
                and int(row["offset"], 16) <= start and end <= int(row["end_offset"], 16)):
            return True
    for row in module.get("mixed_tu_exact_c_ranges") or []:
        if int(row["offset"], 16) <= start and end <= int(row["end_offset"], 16):
            return True
    return False


def promoted_functions(root: Path = REPO) -> list[Entry]:
    import progress
    import reloc_surface as rs

    asm = progress.get_asm_labelled_names(str(root / "asm"))
    adopted = progress.get_adopted_symbol_addresses(str(SYMBOLS))
    resident, addresses, _placeholders, _aliases = progress.get_elf_functions(
        str(ELF_PATH), progress.find_objdump(str(root / "tools")), adopted)
    contract = resident_contract_rows(SYMBOLS.read_text(errors="replace"))
    objects = map_text_objects(MAP_PATH.read_text(errors="replace"))
    elf = rs.Elf(ELF_PATH)
    sections = {}
    for name, value, _size, info, shndx in elf.symbols():
        if info & 0xF == rs.STT_FUNC and 0 < shndx < len(elf.names):
            sections.setdefault(name, elf.names[shndx])

    entries = []
    for name in sorted(set(resident) - asm):
        obj = object_for(objects, sections.get(name, ""), addresses.get(name, -1))
        if name in contract:
            entries.append(Entry(name, "resident", obj))
        else:
            entries.append(Entry(name, "resident", obj, covered=False,
                                 why_uncovered="symbol_addrs row does not say matched C"))

    atlas = json.loads(ATLAS.read_text())
    modules = {module["overlay"]: module for module in atlas["modules"]}
    seen = set()
    for name, value, size, info, shndx in elf.symbols():
        if info & 0xF != rs.STT_FUNC or size <= 0 or not 0 < shndx < len(elf.names):
            continue
        section = elf.names[shndx]
        match = re.fullmatch(r"\.overlay_(\d{3})", section)
        if not match or name in asm:
            continue
        overlay, offset = int(match.group(1)), value - SYNTHETIC_VMA
        module = modules.get(overlay)
        if module is None or (overlay, offset) in seen:
            continue
        if _exact_c(module, offset, offset + size):
            seen.add((overlay, offset))
            entries.append(Entry(name, "overlay", object_for(objects, section, value)))
    return entries


# ----------------------------------------------------------------- proof

def proof_environment_digest(root: Path = REPO) -> str:
    digest = hashlib.sha256()
    paths = [root / name for name in TOOL_SOURCES + PROOF_INPUTS]
    paths += sorted((root / "mk").glob("**/*.mk"))
    for path in paths:
        digest.update(path.relative_to(root).as_posix().encode() + b"\0")
        digest.update((_sha256(path) if path.is_file() else "missing").encode() + b"\n")
    return digest.hexdigest()


def entry_key(entry: Entry, elf_digest: str, environment: str,
              object_digest: Callable[[str], str]) -> str:
    obj = object_digest(entry.obj) if entry.obj else "no-object"
    return hashlib.sha256(
        "\0".join((CACHE_SCHEMA, entry.symbol, entry.kind, entry.obj or "",
                   obj, elf_digest, environment)).encode()).hexdigest()


_NOISE = (
    (re.compile(r"func_overlay_\d{3}_F[0-9A-Fa-f]{7}_[0-9A-Fa-f]+"), "<generated>"),
    (re.compile(r"\bfunc_[0-9A-Fa-f]{8}\w*"), "<func>"),
    (re.compile(r"[+-]?0x[0-9A-Fa-f]+"), "<n>"),
    (re.compile(r"\b\d+\b"), "<n>"),
)


def error_class(message: str, symbol: str) -> str:
    """Reduce one proof error to a stable class: names and numbers removed."""
    line = message.strip().splitlines()[-1] if message.strip() else "no output"
    line = re.sub(r"^.*?error:\s*", "", line)
    line = re.sub(r"^function preflight failed with exit \d+:\s*", "", line)
    line = line.replace(symbol, "<symbol>")
    # A leading identifier names the object of the complaint, not its kind.
    line = re.sub(r"^[A-Za-z_]\w*(?= (?:linked|encoded|canonical|has|is|conflicts))",
                  "<name>", line)
    for pattern, replacement in _NOISE:
        line = pattern.sub(replacement, line)
    return line[:160]


Runner = Callable[..., subprocess.CompletedProcess]


def prove(entry: Entry, runner: Runner = subprocess.run) -> Outcome:
    result = runner(
        [sys.executable, str(TOOLS / "promotion_proof.py"), entry.symbol,
         "--no-build", "--json"],
        cwd=REPO, check=False, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        try:
            receipt = json.loads(result.stdout)
        except json.JSONDecodeError:
            receipt = None
        if isinstance(receipt, dict) and receipt.get("verdict") == "exact":
            return Outcome(entry.symbol, entry.kind, True,
                           identity=str(receipt.get("identity_proof_mode")))
        message = "promotion proof exited 0 without an exact receipt"
    else:
        message = (result.stderr or result.stdout or "").strip() or (
            "promotion proof exited %d" % result.returncode)
    last = message.splitlines()[-1] if message else message
    return Outcome(entry.symbol, entry.kind, False, error=last,
                   error_class=error_class(message, entry.symbol))


def load_cache(path: Path) -> dict[str, dict]:
    try:
        data = json.loads(path.read_text())
    except (OSError, ValueError):
        return {}
    if not isinstance(data, dict) or data.get("schema") != CACHE_SCHEMA:
        return {}
    rows = data.get("entries")
    return rows if isinstance(rows, dict) else {}


def save_cache(path: Path, rows: dict[str, dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(".tmp.%d" % os.getpid())
    temporary.write_text(json.dumps({"schema": CACHE_SCHEMA, "entries": rows},
                                    indent=1, sort_keys=True))
    temporary.replace(path)


def census(entries: Iterable[Entry], *, jobs: int = 1, cache_path: Path | None = CACHE,
           elf_digest: str = "", environment: str = "",
           object_digest: Callable[[str], str] | None = None,
           runner: Runner = subprocess.run,
           progress: Callable[[int, int], None] | None = None) -> list[Outcome]:
    entries = [entry for entry in entries if entry.covered]
    digests: dict[str, str] = {}

    def digest(obj: str) -> str:
        if obj not in digests:
            digests[obj] = (object_digest(obj) if object_digest is not None
                            else _sha256(REPO / obj) if (REPO / obj).is_file()
                            else "missing")
        return digests[obj]

    cache = load_cache(cache_path) if cache_path is not None else {}
    keys = {entry.symbol: entry_key(entry, elf_digest, environment, digest)
            for entry in entries}
    outcomes: dict[str, Outcome] = {}
    pending = []
    for entry in entries:
        row = cache.get(entry.symbol)
        if row and row.get("key") == keys[entry.symbol]:
            outcomes[entry.symbol] = Outcome(entry.symbol, entry.kind, True,
                                             identity=row.get("identity"), cached=True)
        else:
            pending.append(entry)

    done = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        futures = {pool.submit(prove, entry, runner): entry for entry in pending}
        for future in concurrent.futures.as_completed(futures):
            outcome = future.result()
            outcomes[outcome.symbol] = outcome
            done += 1
            if progress is not None:
                progress(done, len(pending))
    if cache_path is not None:
        fresh = {symbol: row for symbol, row in cache.items()
                 if symbol in keys and row.get("key") == keys[symbol]}
        for outcome in outcomes.values():
            if outcome.ok:
                fresh[outcome.symbol] = {"key": keys[outcome.symbol],
                                         "identity": outcome.identity}
        save_cache(cache_path, fresh)
    return [outcomes[entry.symbol] for entry in entries]


# ------------------------------------------------------------------ main

def refresh_build() -> None:
    """Bring the ELF and ROM current once, so each proof can run --no-build."""
    import function_preflight as fp
    # The preflight's own refresh: a plain `gmake` misses an edited Makefile
    # or mk/ recipe, which the --no-build freshness check would then refuse
    # in every single proof.
    #
    # A forced rebuild recompiles every overlay object, which discards the
    # resident-call renames only `gmake overlay-syms` applies, so the first
    # link can fail with R_MIPS_26 truncations (CLAUDE.md, "Promoting an
    # overlay function").  Regenerate and link once more before giving up.
    jobs = "-j%d" % (os.cpu_count() or 4)
    try:
        fp._build_linked_boundary()
    except fp.PreflightError:
        for command in (["gmake", "--no-print-directory", "overlay-syms"],
                        ["gmake", "--no-print-directory", jobs]):
            subprocess.run(command, cwd=REPO, check=False, stdout=subprocess.DEVNULL)
    try:
        fp._require_fresh_linked_boundary()
    except fp.PreflightError as error:
        raise SystemExit("promotion-proof census: build refresh failed: %s" % error)
    # The ROM must be strictly newer than the ELF for a --no-build ROM oracle.
    # -W regenerates only ELF -> BIN -> ROM; nothing recompiles or relinks.
    if not (ROM_OUT.is_file() and ROM_OUT.stat().st_mtime_ns > ELF_PATH.stat().st_mtime_ns):
        subprocess.run(["gmake", "--no-print-directory", "-W",
                        ELF_PATH.relative_to(REPO).as_posix(),
                        ROM_OUT.relative_to(REPO).as_posix()],
                       cwd=REPO, check=True, stdout=subprocess.DEVNULL)


def render(outcomes: list[Outcome], uncovered: list[Entry], *, list_uncovered: bool,
           require_coverage: bool, elapsed: float) -> int:
    failures = [row for row in outcomes if not row.ok]
    for row in sorted(failures, key=lambda row: (row.error_class or "", row.symbol)):
        print("FAIL %-44s [%s] %s" % (row.symbol, row.kind, row.error))
    by_kind = collections.Counter(row.kind for row in outcomes)
    passed = collections.Counter(row.kind for row in outcomes if row.ok)
    cached = sum(row.cached for row in outcomes)
    print()
    print("promotion-proof census: %d proved, %d failed, %d uncovered "
          "(%d from cache, %.1fs)" % (len(outcomes) - len(failures), len(failures),
                                     len(uncovered), cached, elapsed))
    for kind in ("resident", "overlay"):
        print("  %-8s %4d proved / %4d in contract, %4d uncovered"
              % (kind, passed[kind], by_kind[kind],
                 sum(entry.kind == kind for entry in uncovered)))
    if failures:
        print("failures by class:")
        for name, count in collections.Counter(
                row.error_class for row in failures).most_common():
            print("  %4d  %s" % (count, name))
    print("proofs by identity label:")
    for name, count in sorted(collections.Counter(
            row.identity for row in outcomes if row.ok).items()):
        print("  %4d  %s" % (count, name))
    if uncovered:
        print("uncovered by reason:")
        for name, count in collections.Counter(
                entry.why_uncovered for entry in uncovered).most_common():
            print("  %4d  %s" % (count, name))
        if list_uncovered:
            for entry in uncovered:
                print("  UNCOVERED %s [%s]" % (entry.symbol, entry.kind))
    if failures or (require_coverage and uncovered):
        return 1
    return 0


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("-j", "--jobs", type=int, default=1)
    parser.add_argument("--no-build", action="store_true",
                        help="do not refresh the build first (it must be current)")
    parser.add_argument("--no-cache", action="store_true")
    parser.add_argument("--only", choices=("resident", "overlay"))
    parser.add_argument("--limit", type=int, help="prove only the first N (debugging)")
    parser.add_argument("--list-uncovered", action="store_true")
    parser.add_argument("--require-coverage", action="store_true",
                        help="fail on resident functions outside the proof contract")
    parser.add_argument("--quiet", action="store_true", help="no progress line")
    args = parser.parse_args(argv)
    if args.jobs < 1:
        parser.error("-j must be at least 1")

    started = time.monotonic()
    if not args.no_build:
        refresh_build()
    entries = promoted_functions()
    if args.only:
        entries = [entry for entry in entries if entry.kind == args.only]
    uncovered = [entry for entry in entries if not entry.covered]
    covered = [entry for entry in entries if entry.covered]
    if args.limit is not None:
        covered = covered[:args.limit]

    def progress(done: int, total: int) -> None:
        if not args.quiet and sys.stderr.isatty():
            print("\r  proving %d/%d" % (done, total), end="", file=sys.stderr, flush=True)

    outcomes = census(covered, jobs=args.jobs,
                      cache_path=None if args.no_cache else CACHE,
                      elf_digest=_sha256(ELF_PATH),
                      environment=proof_environment_digest(),
                      progress=progress)
    if not args.quiet and sys.stderr.isatty():
        print(file=sys.stderr)
    return render(outcomes, uncovered, list_uncovered=args.list_uncovered,
                  require_coverage=args.require_coverage,
                  elapsed=time.monotonic() - started)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
