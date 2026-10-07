#!/usr/bin/env python3
"""Rank MATCHED functions as sibling shapes for each unmatched target.

    tools/sibling_scan.py [--symbol NAME ...] [--top N] [--jobs J] [--json PATH]

The sibling-copy lever: when a module (or a neighbouring one) has a matched
function written by the same author for the same job, writing the target as a
copy of that function's shape closes residuals a spelling search never
reaches. On 2026-10-02 it matched o051 F00000D0 (1,928 B, pure GLOBAL_ASM) as
the cut-down copy of o050 F0000334, o073 F0000D70 as a copy of the overlay 71
renderer, and func_8000F198 in the shape of func_8000DFBC. Finding the sibling
by hand is the slow step; this ranks candidates.

Signal: the opcode-mnemonic SEQUENCE. Each unmatched target's mnemonics come
from its splat listing under asm/, excluding only reviewed nonexecutable ROM
ranges from the executable-accounting contract; each matched function's come
from compiled objects under build/ (objdump of what our C produced, so the shape
is the shape a C body is known to reach). Candidates are prefiltered by
Jaccard overlap of mnemonic trigrams, then scored by difflib's ratio over the
two sequences. Registers, immediates and relocations are ignored: two bodies
that differ only in allocation or in which globals they touch still score
high, which is the point.

Candidates must have a C definition in the configured canonical preprocessing
output (NON_MATCHING=0). Overlay candidates must also lie within their source's
reviewed exact-C atlas range. This excludes GLOBAL_ASM bodies even when their
compiled symbol was renamed or their candidate C lives in an included file.

A score is a lead, not a verdict. Read both bodies and the target's
relocation records (tools/overlay_tables.py) before porting: the o051 lead
named o055 at 0.51, and the records named the real sibling, o050.

Needs a built tree (`gmake`) and an extracted asm/. Output is symbol names,
object paths and scores only; no instruction text leaves this tool.
"""

from __future__ import annotations

import argparse
import difflib
import json
import multiprocessing
import os
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path
from collections.abc import Sequence

import executable_accounting
import finalize_plateau
import nm_ranking
import overlay_atlas
import permute_batch
import reloc_surface

ROOT = Path(__file__).resolve().parent.parent
OBJDUMP = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
RANKING = ROOT / "config" / "nonmatching-ranking.us.json"
LISTING_ROW = re.compile(r"\s*/\*\s*([0-9A-F]+) [0-9A-F]+ [0-9A-F]{8} \*/\s+(\S+)")
DIS_FUNC = re.compile(r"^[0-9a-f]+ <([^>]+)>:")
DIS_ROW = re.compile(r"^\s+[0-9a-f]+:\s+(\S+)")


class CandidateProofError(ValueError):
    """Canonical C ownership could not be established."""


def canonical_commands(objects: list[str]) -> dict[str, list[str]]:
    """Recover real preprocessing flags in one read-only Make dry run."""
    sources = {os.path.relpath(obj, ROOT): os.path.relpath(obj, ROOT)[6:-2]
               for obj in objects}
    command = ["gmake", "--no-print-directory", "-n", "NON_MATCHING=0"]
    for source in sources.values():
        command.extend(["-W", source])
    command.extend(sources)
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=120)
    if result.returncode:
        raise CandidateProofError(result.stderr.strip() or "cannot expand canonical recipes")
    commands = {}
    for line in result.stdout.replace("\\\n", " ").splitlines():
        if "tools/ido/cc" not in line and "python3 tools/ido-phases.py" not in line:
            continue
        words = shlex.split(line)
        outputs = [obj for obj in sources if obj in words]
        if len(outputs) != 1:
            raise CandidateProofError("compiler recipe has ambiguous object ownership")
        obj = outputs[0]
        if obj in commands:
            raise CandidateProofError(f"duplicate canonical recipe: {obj}")
        try:
            # The phase driver changes optimization stages, not CPP input.
            compiler_line = line.replace("python3 tools/ido-phases.py", "tools/ido/cc")
            flags = permute_batch.compiler_arguments(compiler_line, sources[obj], obj)
        except ValueError as exc:
            raise CandidateProofError(str(exc)) from exc
        # Phase-driver codegen switches can override IDO's -E stage selection.
        # Retain the real recipe's CPP inputs, not its optimization pipeline.
        cpp_flags = []
        index = 0
        while index < len(flags):
            flag = flags[index]
            if flag == "-DNON_MATCHING" or flag.startswith("-DNON_MATCHING="):
                raise CandidateProofError(f"canonical recipe enables candidate C: {obj}")
            if flag in {"-I", "-D", "-U"}:
                if index + 1 >= len(flags):
                    raise CandidateProofError(f"missing preprocessing operand: {obj}")
                if flag == "-D" and flags[index + 1].split("=", 1)[0] == "NON_MATCHING":
                    raise CandidateProofError(f"canonical recipe enables candidate C: {obj}")
                cpp_flags.extend(flags[index:index + 2])
                index += 2
                continue
            if (flag.startswith(("-I", "-D", "-U", "-mips"))
                    or flag in {"-nostdinc", "-32", "-Xcpluscomm"}):
                cpp_flags.append(flag)
            index += 1
        commands[str(ROOT / obj)] = [str(ROOT / "tools/ido/cc"), "-E", *cpp_flags, sources[obj]]
    if set(commands) != set(objects):
        raise CandidateProofError("missing canonical recipes: " + ", ".join(
            os.path.relpath(obj, ROOT) for obj in set(objects) - set(commands)))
    return commands


def overlay_owners(atlas: dict) -> dict[str, tuple[int, list[tuple[int, int]]]]:
    """Source-to-range proof, retaining mixed-TU exact islands separately."""
    exact = overlay_atlas.exact_c_index(atlas)
    owners = {}
    for module in atlas["modules"]:
        for row in module.get("text_ownership", []):
            if row.get("type") != "c":
                continue
            source = "src/" + row["source"] + ".c"
            if source in owners:
                raise CandidateProofError(f"ambiguous atlas source owner: {source}")
            ranges = [(item["offset"], item["end_offset"]) for item in exact.values()
                      if item["overlay"] == module["overlay"] and item["source"] == row["source"]]
            owners[source] = (int(row["offset"], 0), ranges)
    return owners


def authenticated_names(text: str, symbols: list[tuple], source: str,
                        owners: dict) -> set[str]:
    """Accept only emitted functions with a canonical C definition and owner."""
    # IDO preprocessing resolves guards, includes and macro-renamed definitions.
    # Pragma operands remain strings, so they cannot satisfy the definition regex.
    text = re.sub(r'^\s*#.*$', '', text, flags=re.MULTILINE)
    accepted = set()
    for name, value, size, info, section in symbols:
        if info & 0xF != 2 or section == 0 or size <= 0:
            continue
        definition = re.compile(finalize_plateau.DEFINITION_TEMPLATE.format(
            symbol=re.escape(name)), re.MULTILINE | re.DOTALL)
        if len(definition.findall(text)) != 1:
            continue
        if source.startswith("src/overlays/"):
            owner = owners.get(source)
            if owner is None:
                continue
            base, ranges = owner
            if not any(start <= base + value and base + value + size <= end
                       for start, end in ranges):
                continue
        accepted.add(name)
    return accepted


def listing_mnemonics(path: Path, excluded_ranges: Sequence[dict] = ()) -> list[str]:
    """Target mnemonics, excluding only validated reviewed ROM intervals.

    ROM offsets distinguish overlays sharing a synthetic VMA. A zero word or
    trailing nop alone never establishes a nonexecutable range.
    """
    out = []
    with open(path) as handle:
        for line in handle:
            match = LISTING_ROW.match(line)
            if match:
                rom_offset = int(match.group(1), 16)
                if not any(row["rom_start"] <= rom_offset < row["rom_end"]
                           for row in excluded_ranges):
                    out.append(match.group(2))
    return out


def preprocess_source(command: list[str]) -> str:
    """Resolve canonical guards/includes/aliases without compiling a candidate."""
    # IDO's standalone -E path does not honor // comments like its compiling
    # path does (-Xcpluscomm). Strip comments, retaining string literals and
    # directives, and preserve the original local include search directory.
    source = ROOT / command[-1]
    with tempfile.TemporaryDirectory(prefix="sibling-cpp-") as directory:
        prepared = Path(directory) / source.name
        prepared.write_text(nm_ranking.strip_c_comments(source.read_text()))
        preprocessed = subprocess.run([*command[:-1], "-I", str(source.parent), str(prepared)],
                                      cwd=ROOT, capture_output=True, text=True, timeout=30)
    if preprocessed.returncode:
        raise CandidateProofError(f"canonical preprocessing failed: {command[-1]}: "
                                  + preprocessed.stderr.strip())
    return preprocessed.stdout


def object_functions(item: tuple[str, list[str], dict]) -> tuple[str, dict[str, list[str]]]:
    """Mnemonic sequences of authenticated canonical C functions in one object."""
    obj, command, owners = item
    text = preprocess_source(command)
    elf = reloc_surface.Elf(Path(obj))
    symbols = [symbol for symbol in elf.symbols()
               if symbol[4] < len(elf.names) and elf.names[symbol[4]] == ".text"]
    names = authenticated_names(text, symbols,
                                os.path.relpath(obj, ROOT)[6:-2], owners)
    dumped = subprocess.run(
        [str(OBJDUMP), "-d", "-z", "--no-show-raw-insn", obj],
        capture_output=True, text=True, check=False, timeout=30,
    )
    if dumped.returncode:
        raise CandidateProofError(f"object disassembly failed: {os.path.relpath(obj, ROOT)}")
    functions: dict[str, list[str]] = {}
    current = None
    for line in dumped.stdout.splitlines():
        match = DIS_FUNC.match(line)
        if match:
            current = match.group(1) if match.group(1) in names else None
            if current is not None:
                functions[current] = []
            continue
        match = DIS_ROW.match(line)
        if match and current is not None:
            functions[current].append(match.group(1))
    return obj, functions


def trigrams(seq: list[str]) -> set[tuple[str, str, str]]:
    return set(zip(seq, seq[1:], seq[2:]))


def jaccard(a: set, b: set) -> float:
    return len(a & b) / max(1, len(a | b))


def rank(target: list[str], candidates: dict, grams: dict, top: int,
         prefilter: int = 8) -> list[tuple[float, str, str, int]]:
    """Best `top` candidates for one target: (ratio, name, object, length)."""
    tg = trigrams(target)
    length = len(target)
    pool = []
    for key, seq in candidates.items():
        if not 0.4 * length <= len(seq) <= 2.5 * length:
            continue
        pool.append((jaccard(tg, grams[key]), key))
    pool.sort(reverse=True)
    scored = []
    for _, key in pool[:prefilter]:
        seq = candidates[key]
        ratio = difflib.SequenceMatcher(None, target, seq, autojunk=False).ratio()
        scored.append((ratio, key[0], key[1], len(seq)))
    scored.sort(reverse=True)
    return scored[:top]


_CANDIDATES: dict = {}
_GRAMS: dict = {}


def _rank_one(item):
    name, seq, top = item
    return name, len(seq), rank(seq, _CANDIDATES, _GRAMS, top)


def unmatched_names() -> set[str]:
    """Ranked NON_MATCHING symbols plus every name still under asm/nonmatchings."""
    names = set()
    if RANKING.exists():
        names |= {row["name"] for row in json.loads(RANKING.read_text())["functions"]}
    for path in (ROOT / "asm" / "nonmatchings").rglob("*.s"):
        names.add(path.stem)
    return names


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--symbol", action="append", help="limit to these targets")
    parser.add_argument("--top", type=int, default=3)
    parser.add_argument("--jobs", type=int, default=3)
    parser.add_argument("--min", type=float, default=0.0, help="hide leads below this ratio")
    parser.add_argument("--json", help="write every result here")
    args = parser.parse_args(argv)

    if not OBJDUMP.exists():
        print(f"missing {OBJDUMP.relative_to(ROOT)}; run gmake setup", file=sys.stderr)
        return 2
    try:
        excluded_ranges = executable_accounting.reviewed_ranges(
            ROOT, rom=(ROOT / "baseroms/mickey.us.z64").read_bytes())
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        print(f"cannot authenticate target executable ranges: {exc}", file=sys.stderr)
        return 2
    unmatched = unmatched_names()
    listings = {}
    for path in (ROOT / "asm" / "nonmatchings").rglob("*.s"):
        if args.symbol and path.stem not in args.symbol:
            continue
        if path.stem in unmatched and path.stem not in listings:
            seq = listing_mnemonics(path, excluded_ranges)
            if seq:
                listings[path.stem] = seq
    if not listings:
        print("no unmatched listings found (is asm/ extracted?)", file=sys.stderr)
        return 2

    objects = sorted(str(p) for p in (ROOT / "build" / "src").rglob("*.c.o"))
    if not objects:
        print("no compiled objects under build/src (run gmake first)", file=sys.stderr)
        return 2
    try:
        commands = canonical_commands(objects)
        owners = overlay_owners(json.loads((ROOT / "config/overlays.us.json").read_text()))
        with multiprocessing.get_context("fork").Pool(args.jobs) as pool:
            dumped = pool.map(object_functions, [(obj, commands[obj], owners) for obj in objects])
    except (CandidateProofError, overlay_atlas.AtlasDeltaError, OSError,
            subprocess.TimeoutExpired, ValueError) as exc:
        print(f"cannot authenticate matched-C siblings: {exc}", file=sys.stderr)
        return 2
    _CANDIDATES.clear()
    _GRAMS.clear()
    for obj, functions in dumped:
        rel = os.path.relpath(obj, ROOT)
        for name, seq in functions.items():
            # A GLOBAL_ASM body compiled into an object is the target itself.
            if name in unmatched or len(seq) < 40 or name.startswith("."):
                continue
            _CANDIDATES[(name, rel)] = seq
            _GRAMS[(name, rel)] = trigrams(seq)

    work = [(name, seq, args.top) for name, seq in sorted(listings.items())]
    with multiprocessing.get_context("fork").Pool(args.jobs) as pool:
        results = pool.map(_rank_one, work)
    results.sort(key=lambda r: -(r[2][0][0] if r[2] else 0.0))

    for name, length, leads in results:
        if not leads or leads[0][0] < args.min:
            continue
        best = " | ".join(f"{r:.2f} {n} ({ln}) {o}" for r, n, o, ln in leads)
        print(f"{name} ({length}): {best}")
    if args.json:
        Path(args.json).write_text(json.dumps(
            [{"symbol": n, "instructions": L,
              "leads": [{"ratio": round(r, 4), "symbol": s, "object": o, "instructions": ln}
                        for r, s, o, ln in leads]}
             for n, L, leads in results], indent=1) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
