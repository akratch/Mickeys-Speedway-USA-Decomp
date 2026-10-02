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
from its splat listing under asm/; each matched function's come from the
compiled objects under build/ (objdump of what our C produced, so the shape
is the shape a C body is known to reach). Candidates are prefiltered by
Jaccard overlap of mnemonic trigrams, then scored by difflib's ratio over the
two sequences. Registers, immediates and relocations are ignored: two bodies
that differ only in allocation or in which globals they touch still score
high, which is the point.

A lead at 0.9+ with the target's exact length can be the target's own
GLOBAL_ASM body compiled under an alias name (o069/o088 share one body under
`#define` renames): check that the candidate really is matched C.

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
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OBJDUMP = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
RANKING = ROOT / "config" / "nonmatching-ranking.us.json"
LISTING_ROW = re.compile(r"\s*/\*\s*[0-9A-F]+ [0-9A-F]+ [0-9A-F]{8} \*/\s+(\S+)")
DIS_FUNC = re.compile(r"^[0-9a-f]+ <([^>]+)>:")
DIS_ROW = re.compile(r"^\s+[0-9a-f]+:\s+(\S+)")


def listing_mnemonics(path: Path) -> list[str]:
    """Mnemonics of one splat listing, in order."""
    out = []
    with open(path) as handle:
        for line in handle:
            match = LISTING_ROW.match(line)
            if match:
                out.append(match.group(1))
    return out


def object_functions(obj: str) -> tuple[str, dict[str, list[str]]]:
    """Mnemonic sequences of every function symbol in one object."""
    text = subprocess.run(
        [str(OBJDUMP), "-d", "-z", "--no-show-raw-insn", obj],
        capture_output=True, text=True, check=False,
    ).stdout
    functions: dict[str, list[str]] = {}
    current = None
    for line in text.splitlines():
        match = DIS_FUNC.match(line)
        if match:
            current = match.group(1)
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
    unmatched = unmatched_names()
    listings = {}
    for path in (ROOT / "asm" / "nonmatchings").rglob("*.s"):
        if args.symbol and path.stem not in args.symbol:
            continue
        if path.stem in unmatched and path.stem not in listings:
            seq = listing_mnemonics(path)
            if seq:
                listings[path.stem] = seq
    if not listings:
        print("no unmatched listings found (is asm/ extracted?)", file=sys.stderr)
        return 2

    objects = sorted(str(p) for p in (ROOT / "build" / "src").rglob("*.c.o"))
    if not objects:
        print("no compiled objects under build/src (run gmake first)", file=sys.stderr)
        return 2
    with multiprocessing.get_context("fork").Pool(args.jobs) as pool:
        dumped = pool.map(object_functions, objects)
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
