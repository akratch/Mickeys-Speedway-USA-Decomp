#!/usr/bin/env python3
"""Rank donor-decomp functions as counterparts of one Mickey target.

    tools/donor_match.py <symbol> [--refs DIR] [--top N]

What is left in the queue is mostly reconstruction, not polish: candidates at
80-100% residual whose author's shape is unknown. Mickey's engine is Jet Force
Gemini's and much of the game code descends from Diddy Kong Racing, both
published and permitted sources (docs/CLEANROOM.md, PROVENANCE at the point of
use). The lane that matched three overlay 47/53/55 functions and the two 6 KB
initialisers did so by porting a counterpart's natural shape first. Finding the
counterpart by hand is the slow step; this ranks them.

Signal: the names a target CALLS. The target listing's `jal` operands are
mapped to donor names three ways -- shared libultra/math names verbatim
(`sqrtf`, `Arctanf`, `dAngle`), adopted names in `symbol_addrs.us.txt`
(`rumbleStart`, `camGetPtr`), and the donor name a symbol_addrs comment
records for a `func_8xxxxxxx` ("JFG amSndPlayXYZ body/order"). Every donor
function is scanned for the identifiers it calls, and candidates are scored by
Jaccard overlap of mapped callee names, tie-broken by call-count closeness and
by body length against the target's instruction count. Unmapped `func_`
callees contribute nothing, so a target that calls only unnamed resident
helpers scores every donor near zero: then the answer is "name the callees
first", which the tool says.

A hit is a lead, not a verdict: read both bodies before porting, and record
`PROVENANCE` at the point of use when you do.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import fast_score  # noqa: E402

DEFAULT_REFS = Path.home() / "Desktop/dev/decomp-refs"
DONOR_DIRS = ("jfg/src", "diddy-kong-racing/src", "perfect_dark/src", "banjo-kazooie/src", "conker/src")
KEYWORDS = {"if", "for", "while", "switch", "return", "sizeof", "do", "else", "case"}
FUNC_HEAD = re.compile(r"^[A-Za-z_][\w\s\*]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{", re.M)
CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
JAL = re.compile(r"^\s*/\*[^*]*\*/\s*jal\s+([A-Za-z_]\w*)", re.M)
HINT = re.compile(r"\b(?:JFG|DKR|PD|BK|CBFD)\s+(?!src\b)([A-Za-z_]\w{3,})")


def target_callees(symbol: str) -> tuple[list[str], int]:
    """Return (callee names in call order, instruction count) from the target listing."""
    source = fast_score.tracked_source_for(symbol)
    rows = fast_score.target_words(symbol, source)
    listing = fast_score.target_listing_for(symbol, (ROOT / source).read_text())
    text = (ROOT / listing).read_text() if listing else ""
    if not text:
        import glob
        stem = Path(source).with_suffix("").as_posix().removeprefix("src/")
        for f in glob.glob(str(ROOT / f"asm/nonmatchings/{stem}/*.s")):
            if Path(f).stem == symbol:
                text = Path(f).read_text()
    return JAL.findall(text), len(rows)


def name_map(symbol_addrs: str) -> dict[str, set[str]]:
    """Map each Mickey symbol to the donor names it is known by."""
    out: dict[str, set[str]] = {}
    for line in symbol_addrs.splitlines():
        m = re.match(r"^(\w+)\s*=\s*0x[0-9A-Fa-f]+;(.*)$", line)
        if not m:
            continue
        name, comment = m.group(1), m.group(2)
        names = {name} if not name.startswith("func_") else set()
        names.update(HINT.findall(comment))
        if names:
            out[name] = names
    return out


def donor_functions(refs: Path) -> list[tuple[str, str, int, list[str]]]:
    """Return (repo, name, line, callees) for every function in the donor sources."""
    found = []
    for rel in DONOR_DIRS:
        base = refs / rel
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*.c")):
            try:
                text = path.read_text(errors="replace")
            except OSError:
                continue
            heads = list(FUNC_HEAD.finditer(text))
            for i, h in enumerate(heads):
                end = heads[i + 1].start() if i + 1 < len(heads) else len(text)
                body = text[h.end():end]
                callees = [c for c in CALL.findall(body) if c not in KEYWORDS]
                found.append((f"{rel.split('/')[0]}:{path.relative_to(base)}", h.group(1), text[:h.start()].count("\n") + 1, callees))
    return found


def score(target_names: set[str], target_calls: int, target_words: int, callees: list[str]) -> tuple[float, float]:
    donor_names = set(callees)
    if not target_names or not donor_names:
        return 0.0, 0.0
    inter = len(target_names & donor_names)
    jacc = inter / len(target_names | donor_names)
    count_fit = 1.0 - min(1.0, abs(len(callees) - target_calls) / max(target_calls, 1))
    return jacc, count_fit


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--refs", default=str(DEFAULT_REFS))
    ap.add_argument("--top", type=int, default=12)
    ns = ap.parse_args(argv)
    callees, words = target_callees(ns.symbol)
    nmap = name_map((ROOT / "symbol_addrs.us.txt").read_text())
    mapped: set[str] = set()
    unmapped: list[str] = []
    for c in callees:
        if c == "TrapDanglingJump":
            continue
        names = nmap.get(c) or ({c} if not c.startswith("func_") else set())
        if names:
            mapped |= names
        else:
            unmapped.append(c)
    print(f"{ns.symbol}: {words} words, {len(callees)} calls; mapped callee names: {', '.join(sorted(mapped)) or 'none'}")
    if unmapped:
        print(f"  unmapped (name these to sharpen the match): {', '.join(sorted(set(unmapped)))}")
    if not mapped:
        print("  no mapped callee names; nothing to rank against")
        return 1
    ranked = []
    for repo, name, line, dc in donor_functions(Path(ns.refs)):
        j, f = score(mapped, len(callees), words, dc)
        if j > 0:
            ranked.append((j, f, repo, name, line, len(dc)))
    ranked.sort(key=lambda r: (-r[0], -r[1]))
    print(f"{'jaccard':>7} {'calls':>5}  donor")
    for j, f, repo, name, line, n in ranked[: ns.top]:
        print(f"{j:7.2f} {n:5d}  {repo}:{line} {name}")
    return 0 if ranked else 1


if __name__ == "__main__":
    sys.exit(main())
