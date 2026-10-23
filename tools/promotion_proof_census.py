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
  A ``static`` function counts too: tools/static_symbols.py gives it a sized
  local ``STT_FUNC`` from its object's ``.mdebug``, and the census tallies
  those separately. Exact C bytes that no sized function symbol covers are
  reported as an uncovered entry, as is a function name the linked ELF
  defines more than once, so a function with no boundary is never skipped.

``promotion_proof.py`` decides that a resident function is promoted by the
same rule (``function_preflight._progress_matched_resident``), so every
resident function progress counts is in contract. Its geometry is the
``symbol_addrs.us.txt`` ``type:func size:`` row when one exists, and
otherwise the linked extent authenticated against its neighbour; the census
tallies residents by that evidence. A zero-size resident ``STT_FUNC`` (a weak
alias such as ``fsin`` of ``__sinf``) is not proved on its own: it passes when
the sized definition at its address is itself promoted and proved.

A function outside the proof contract would be reported as *uncovered* (a
count, and a list under ``--list-uncovered``) rather than as a failure;
``--require-coverage`` turns every uncovered function into a failure.

Each proof runs with ``--no-build``; the census refreshes the build once
first.  That is also what makes ``-j`` safe: without ``--no-build`` every
proof's ROM oracle rewrites ``build/mickey.us.z64`` while the others read it,
and the readers fail with ``wb_compare failed with exit 2``.

Passing results are cached in ``build/promotion-proof-census/cache.json``,
keyed per function on its object's hash, the linked ELF's hash, the proof
tools' source hashes, compiler/binutils content and the tracked proof inputs (atlas, generated alias
surface, symbol table, Makefile fragments and both ROM images). Failures are never cached, so a
warm run re-proves exactly the failing and the changed functions.
The CLI checks build freshness even on a fully cached run. It also rejects
the entire run, without updating the cache, if proof inputs change while
functions are being enumerated or proved. Do not build in the census lane.

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
CACHE_SCHEMA = "mickey-promotion-proof-census-cache-v3"
SYNTHETIC_VMA = 0xF0000000

# Every file whose content decides a promotion-proof verdict, beyond the
# function's own object and the linked ELF.
TOOL_SOURCES = (
    "tools/promotion_proof.py", "tools/function_preflight.py",
    "tools/reloc_surface.py", "tools/reloc_identity.py", "tools/overlay_call_context.py",
    "tools/proof_provenance.py", "tools/overlay_tables.py",
    "tools/postprocess_audit.py", "tools/function_history.py",
    "tools/wb_compare.sh", "tools/promotion_proof_census.py",
    "tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
    "tools/static_symbols.py", "tools/link_overlay_module.py",
    "tools/source_module_link_proof.py", "tools/runlink_pack.py", "tools/runlink_metadata_link.py", "tools/rebind_elf_relocations.py",
    "tools/externalize_elf_section.py", "tools/postprocess_guard.py",
    # Fresh raw reproduction also pins these compiler/context implementations.
    "tools/permute_batch.py", "tools/sweep_receipts.py", "tools/promotion_transaction.py",
    "tools/canonical_data_owner.py", "tools/canonical_bss_view.py",
    "tools/storage_identity.py", "tools/storage_view.py", "tools/scratch_fidelity.py",
    "tools/search_local_bindings.py", "tools/search_literal_bindings.py",
    "tools/source_symbol_fidelity.py", "tools/prepared_macro_context.py",
    "tools/asm-processor/build.py", "tools/asm-processor/asm_processor.py",
)
PROOF_INPUTS = (
    "config/overlays.us.json", "overlay_undefined_syms.us.txt",
    "symbol_addrs.us.txt", "config/runlink-ort.us.txt", "Makefile",
    "build/mickey.us.z64", "baseroms/mickey.us.z64",
)


@dataclasses.dataclass(frozen=True)
class Entry:
    symbol: str
    kind: str                 # "resident" | "overlay"
    obj: str | None           # object path relative to the repository
    covered: bool = True      # False: outside promotion-proof's contract
    why_uncovered: str = ""
    alias_of: str | None = None  # zero-size alias: proved through this definition
    static: bool = False      # a local (``static``) overlay function


@dataclasses.dataclass
class Outcome:
    symbol: str
    kind: str
    ok: bool
    identity: str | None = None
    error: str | None = None
    error_class: str | None = None
    cached: bool = False
    evidence: str | None = None
    static: bool = False


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


def _exact_c_ranges(module: dict) -> list[tuple[int, int]]:
    ranges = [(int(row["offset"], 16), int(row["end_offset"], 16))
              for row in module.get("text_ownership", [])
              if row.get("type") == "c" and row.get("matched") is True
              and row.get("nonmatching") is False]
    ranges += [(int(row["offset"], 16), int(row["end_offset"], 16))
               for row in module.get("mixed_tu_exact_c_ranges") or []]
    return ranges


def _exact_c(module: dict, start: int, end: int) -> bool:
    return any(low <= start and end <= high for low, high in _exact_c_ranges(module))


def unbounded_spans(ranges: Iterable[tuple[int, int]],
                    functions: Iterable[tuple[int, int]]) -> list[tuple[int, int]]:
    """Exact-C ``[start, end)`` spans that no function extent covers."""
    extents = sorted(functions)
    spans = []
    for low, high in sorted(set(ranges)):
        cursor = low
        for start, end in extents:
            if end <= cursor or start >= high:
                continue
            if start > cursor:
                spans.append((cursor, start))
            cursor = max(cursor, end)
        if cursor < high:
            spans.append((cursor, high))
    return spans


def promoted_functions(root: Path = REPO) -> list[Entry]:
    import progress
    import reloc_surface as rs

    asm = progress.get_asm_labelled_names(str(root / "asm"))
    adopted = progress.get_adopted_symbol_addresses(str(SYMBOLS))
    resident, addresses, _placeholders, _aliases = progress.get_elf_functions(
        str(ELF_PATH), progress.find_objdump(str(root / "tools")), adopted)
    objects = map_text_objects(MAP_PATH.read_text(errors="replace"))
    elf = rs.Elf(ELF_PATH)
    sections = {}
    for name, value, _size, info, shndx in elf.symbols():
        if info & 0xF == rs.STT_FUNC and 0 < shndx < len(elf.names):
            sections.setdefault(name, elf.names[shndx])

    matched = set(resident) - asm
    sized_at = collections.defaultdict(list)
    for name in matched:
        if resident[name] > 0 and name in addresses:
            sized_at[addresses[name]].append(name)
    entries = []
    for name in sorted(matched):
        obj = object_for(objects, sections.get(name, ""), addresses.get(name, -1))
        if resident[name] == 0:
            owners = sized_at.get(addresses.get(name, -1), [])
            entries.append(Entry(name, "resident", obj,
                                 alias_of=owners[0] if len(owners) == 1 else ""))
        else:
            entries.append(Entry(name, "resident", obj))

    atlas = json.loads(ATLAS.read_text())
    modules = {module["overlay"]: module for module in atlas["modules"]}
    seen = set()
    extents = collections.defaultdict(list)
    definitions = collections.Counter(
        name for name, _value, size, info, shndx in elf.symbols()
        if info & 0xF == rs.STT_FUNC and size > 0 and 0 < shndx < len(elf.names))
    for name, value, size, info, shndx in elf.symbols():
        if info & 0xF != rs.STT_FUNC or size <= 0 or not 0 < shndx < len(elf.names):
            continue
        section = elf.names[shndx]
        match = re.fullmatch(r"\.overlay_(\d{3})", section)
        if not match:
            continue
        overlay, offset = int(match.group(1)), value - SYNTHETIC_VMA
        extents[overlay].append((offset, offset + size))
        module = modules.get(overlay)
        if name in asm or module is None or (overlay, offset) in seen:
            continue
        if _exact_c(module, offset, offset + size):
            seen.add((overlay, offset))
            static = info >> 4 == 0
            # promotion_proof.py names a function by its symbol; a name that
            # two definitions share would prove one of them twice.
            covered = definitions[name] == 1
            entries.append(Entry(name, "overlay", object_for(objects, section, value),
                                 covered=covered, static=static,
                                 why_uncovered="" if covered else
                                 "function name defined more than once in the linked ELF"))
    for overlay, module in sorted(modules.items()):
        rom = int(module["rom"]["start"], 16)
        section = ".overlay_%03d" % overlay
        for start, end in unbounded_spans(_exact_c_ranges(module), extents[overlay]):
            entries.append(Entry(
                "func_overlay_%03d_%08X_%X" % (overlay, SYNTHETIC_VMA + start, rom + start),
                "overlay", object_for(objects, section, SYNTHETIC_VMA + start), covered=False,
                why_uncovered="exact C bytes no sized function symbol bounds"))
    return entries


# ----------------------------------------------------------------- proof

# The toolchain trees match the binary dependencies pinned by fresh proofs'
# permute_batch.sweep_tool_identity, including compiler backends and module ld.
OWNER_TREES = ("mk", "config", "src", "include", "asm", "tools/ido", "tools/binutils")


def proof_input_paths(root: Path) -> set[Path]:
    """Use one dependency set for cache keys and concurrent-write detection."""
    paths = {root / name for name in TOOL_SOURCES + PROOF_INPUTS}
    import runlink_metadata_link
    paths.update(runlink_metadata_link.provenance_paths(root))
    paths.add(root / "mickey.us.yaml")
    for directory in OWNER_TREES:
        paths.update(path for path in (root / directory).rglob("*") if path.is_file())
    # Metadata recipes may put @SPEC_FILE inputs outside config/. Include
    # repository-relative references even when missing, so creation is seen.
    for recipe in [root / "Makefile", *sorted((root / "mk").rglob("*.mk")),
                   *sorted((root / "config" / "normalizations").rglob("*.mk"))]:
        if recipe.is_file():
            for name in re.findall(r"(?<!\S)@([\w./-]+)", recipe.read_text()):
                relative = Path(name)
                if "/" in name and not relative.is_absolute() and ".." not in relative.parts:
                    paths.add(root / relative)
    return paths


def proof_environment_digest(root: Path = REPO) -> str:
    digest = hashlib.sha256()
    for path in sorted(proof_input_paths(root)):
        digest.update(path.relative_to(root).as_posix().encode() + b"\0")
        digest.update((_sha256(path) if path.is_file() else "missing").encode() + b"\n")
    return digest.hexdigest()


class InputsChangedError(RuntimeError):
    """A census cannot certify one coherent set of compiler artifacts."""


def input_snapshot(entries: Iterable[Entry] = (), *, root: Path = REPO) -> dict:
    """Observe content-changing metadata, including replacement and edit/restore.

    Unlike content hashes alone, ctime also detects a file rewritten with the
    same bytes while a proof might have read its partially written contents.
    Enumerate source, assembly and toolchain paths again to detect additions.
    """
    paths = proof_input_paths(root)
    paths.update(root / name for name in (
        "build/mickey.us.elf", "build/mickey.us.map", "mickey.us.yaml"))
    # A transient owner created and removed during enumeration can change the
    # discovered function set without leaving a file behind. Directory ctime
    # catches that event; never watch build/ directories that proofs write.
    for directory in OWNER_TREES:
        parent = root / directory
        paths.add(parent)
        paths.update(path for path in parent.rglob("*") if path.is_dir())
    paths.update(root / entry.obj for entry in entries if entry.obj)
    result = {}
    for path in sorted(paths):
        try:
            stat = path.stat()
            result[path.relative_to(root).as_posix()] = (
                stat.st_dev, stat.st_ino, stat.st_size, stat.st_mtime_ns, stat.st_ctime_ns)
        except FileNotFoundError:
            result[path.relative_to(root).as_posix()] = None
    return result


def require_unchanged_inputs(before: dict, after: dict) -> None:
    changed = sorted(name for name in before.keys() | after.keys()
                     if name not in before or name not in after or before[name] != after[name])
    if changed:
        detail = ", ".join(changed[:5])
        if len(changed) > 5:
            detail += f" (+{len(changed) - 5} more)"
        raise InputsChangedError(
            f"proof inputs changed during census: {detail}; results discarded, "
            "cache not updated. Refresh the build and rerun without concurrent writes.")


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
    outcome = _prove(entry, runner)
    outcome.static = entry.static
    return outcome


def _prove(entry: Entry, runner: Runner) -> Outcome:
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
                           identity=str(receipt.get("identity_proof_mode")),
                           evidence=receipt.get("geometry_evidence"))
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
           progress: Callable[[int, int], None] | None = None,
           validate_inputs: Callable[[], None] | None = None) -> list[Outcome]:
    entries = [entry for entry in entries if entry.covered]
    aliases = [entry for entry in entries if entry.alias_of is not None]
    entries = [entry for entry in entries if entry.alias_of is None]
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
                                             identity=row.get("identity"), cached=True,
                                             evidence=row.get("evidence"),
                                             static=entry.static)
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
    # Validate before either saving fresh proofs or returning cached results.
    # A concurrent rebuild invalidates the run, including successful rows.
    if validate_inputs is not None:
        validate_inputs()
    if cache_path is not None:
        fresh = {symbol: row for symbol, row in cache.items()
                 if symbol in keys and row.get("key") == keys[symbol]}
        for outcome in outcomes.values():
            if outcome.ok:
                fresh[outcome.symbol] = {"key": keys[outcome.symbol],
                                         "identity": outcome.identity,
                                         "evidence": outcome.evidence}
        save_cache(cache_path, fresh)
    for alias in aliases:
        owner = outcomes.get(alias.alias_of or "")
        if owner is not None and owner.ok:
            outcomes[alias.symbol] = Outcome(alias.symbol, alias.kind, True,
                                             identity="zero-size alias", cached=owner.cached,
                                             evidence="zero-size alias of a proved definition")
        else:
            message = ("zero-size alias of %s, which is not proved" % alias.alias_of
                       if alias.alias_of else
                       "zero-size alias with no single sized promoted definition at its address")
            outcomes[alias.symbol] = Outcome(alias.symbol, alias.kind, False, error=message,
                                             error_class=error_class(message, alias.symbol))
    return [outcomes[entry.symbol] for entry in entries + aliases]


# ------------------------------------------------------------------ main

def refresh_build() -> None:
    """Bring the ELF and ROM current once, so each proof can run --no-build."""
    import function_preflight as fp
    # The preflight's own refresh: a plain `gmake` misses an edited Makefile
    # or mk/ recipe, which the --no-build freshness check would then refuse
    # in every single proof.
    #
    # A forced rebuild recompiles every overlay object; the resident-call
    # renames are declared compile rules (mk/overlay_resident_rebinds.
    # generated.mk), so it keeps them.  If the link still fails, the surface
    # itself is stale: regenerate and link once more before giving up.
    jobs = "-j%d" % (os.cpu_count() or 4)
    # The ELF can be newer than an edited recipe while the objects it linked
    # are not (an earlier link after the edit rebuilt only what Make saw as
    # stale). Each proof checks its own object against the recipe, so force
    # the whole graph when any linked object predates a recipe input.
    recipe_time = max(path.stat().st_mtime_ns for path in fp._build_logic_inputs())
    linked = {obj for rows in map_text_objects(MAP_PATH.read_text(errors="replace")).values()
              for _start, _size, obj in rows} if MAP_PATH.is_file() else set()
    stale = any(not (REPO / obj).is_file() or (REPO / obj).stat().st_mtime_ns < recipe_time
                for obj in linked)
    try:
        if stale:
            raise fp.PreflightError("linked objects predate a build recipe input")
        fp._build_linked_boundary()
    except fp.PreflightError:
        subprocess.run(["gmake", "--no-print-directory", jobs, "--always-make",
                        "--assume-old=%s" % fp.VENV_PYTHON_TARGET,
                        ELF_PATH.relative_to(REPO).as_posix()],
                       cwd=REPO, check=False, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
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


def require_fresh_build() -> None:
    """Cached proofs must not bypass the ordinary --no-build freshness gate."""
    import function_preflight as fp
    fp._require_fresh_linked_boundary()
    if not (ROM_OUT.is_file() and ROM_OUT.stat().st_mtime_ns > ELF_PATH.stat().st_mtime_ns):
        raise fp.PreflightError(
            "compiled ROM is not strictly newer than the linked ELF; "
            "omit --no-build to refresh the proof artifacts")


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
    statics = [row for row in outcomes if row.static]
    print("  of which %d proved / %d in contract are static overlay functions"
          % (sum(row.ok for row in statics), len(statics)))
    if failures:
        print("failures by class:")
        for name, count in collections.Counter(
                row.error_class for row in failures).most_common():
            print("  %4d  %s" % (count, name))
    resident_evidence = collections.Counter(
        row.evidence or "unrecorded" for row in outcomes if row.ok and row.kind == "resident")
    if resident_evidence:
        print("resident proofs by geometry evidence:")
        for name, count in sorted(resident_evidence.items()):
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
    import function_preflight as fp
    try:
        require_fresh_build()
    except fp.PreflightError as error:
        parser.error(str(error))
    before_discovery = input_snapshot()
    entries = promoted_functions()
    try:
        require_unchanged_inputs(before_discovery, input_snapshot())
    except InputsChangedError as error:
        parser.error(str(error))
    if args.only:
        entries = [entry for entry in entries if entry.kind == args.only]
    uncovered = [entry for entry in entries if not entry.covered]
    covered = [entry for entry in entries if entry.covered]
    if args.limit is not None:
        covered = covered[:args.limit]

    def progress(done: int, total: int) -> None:
        if not args.quiet and sys.stderr.isatty():
            print("\r  proving %d/%d" % (done, total), end="", file=sys.stderr, flush=True)

    before_proof = input_snapshot(covered)

    def validate_inputs() -> None:
        require_unchanged_inputs(before_proof, input_snapshot(covered))
        require_fresh_build()

    try:
        require_unchanged_inputs(before_discovery, input_snapshot())
        outcomes = census(covered, jobs=args.jobs,
                          cache_path=None if args.no_cache else CACHE,
                          elf_digest=_sha256(ELF_PATH),
                          environment=proof_environment_digest(),
                          progress=progress, validate_inputs=validate_inputs)
    except (InputsChangedError, fp.PreflightError) as error:
        parser.error(str(error))
    if not args.quiet and sys.stderr.isatty():
        print(file=sys.stderr)
    return render(outcomes, uncovered, list_uncovered=args.list_uncovered,
                  require_coverage=args.require_coverage,
                  elapsed=time.monotonic() - started)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
