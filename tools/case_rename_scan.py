#!/usr/bin/env python3
"""Rename a local inside one `case` of a switch, scan every such rename, combine the best.

    tools/case_rename_scan.py scan    <symbol> <candidate.c> [--cases 0x4C 7] [-j 6] [--out moves.jsonl]
    tools/case_rename_scan.py combine <symbol> <candidate.c> --moves best.json [--max 4] [-j 6]
    tools/case_rename_scan.py apply   <symbol> <candidate.c> --moves best.json --out new.c

In a large switch, IDO's uopt colours a local as one web across every case
that uses it, so which local carries a case's value decides that value's
register and, through interference, everyone else's. A move here is
(case, old, new): every occurrence of local `old` inside that one case block
is renamed to declared local `new`, of the same class (int, float, or the
same pointer type). A move whose `new` already appears in the case is refused,
because it would merge two values and change the function's meaning.

`scan` scores every move once against the candidate (tools/fast_score.py
--aligned, in parallel) and prints `aligned size masked case old new` rows,
best first. `combine` scores every subset (up to --max moves, one move per
case) of a JSON list of moves, `[["0x4C", "pathIndex", "packedField"], ...]`.
`apply` writes the candidate with the moves applied.

On func_800517E0 four scan/combine rounds took the aligned residual from 406
to 282: each adoption moves the landscape, so rescan after every adoption.
Case labels are matched as written in the source (`0x4C`, `7`).
"""

from __future__ import annotations

import argparse
import itertools
import json
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CASE_RE = re.compile(r"^(\s*)case (0x[0-9A-Fa-f]+|\d+):", re.M)
FIXED = {"cursor"}


def function_span(src: str, symbol: str) -> tuple[int, int]:
    """[start, end) of the symbol's definition body (brace matched)."""
    m = re.search(rf"^[\w\s\*]*\b{re.escape(symbol)}\s*\([^;]*?\)\s*\{{", src, re.M)
    if not m:
        raise SystemExit(f"no definition of {symbol}")
    depth, i = 0, m.end() - 1
    while i < len(src):
        depth += {"{": 1, "}": -1}.get(src[i], 0)
        if depth == 0:
            return m.start(), i + 1
        i += 1
    raise SystemExit(f"unbalanced braces in {symbol}")


def declared_locals(src: str, symbol: str) -> dict[str, str]:
    """name -> type for the function-scope declarations at the top of the body."""
    a, b = function_span(src, symbol)
    body = src[src.index("{", a) + 1:b]
    out = {}
    for line in body.splitlines():
        s = line.strip()
        if not s or s.startswith(("/*", "*")):
            continue
        m = re.match(r"^((?:unsigned |signed |struct )?[A-Za-z_]\w*)\s*(\**)\s*([A-Za-z_]\w*);$", s)
        if not m:
            break
        out[m.group(3)] = (m.group(1) + m.group(2)).strip()
    return out


def kind(t: str) -> str:
    return "f" if t in ("f32", "float") else ("p:" + t if "*" in t else "i")


def case_blocks(src: str, symbol: str) -> dict[str, tuple[int, int]]:
    """label -> [start, end) of each top-level case of the function's outermost switch.

    Top level = the shallowest indentation any case label has in the function;
    nested switches (deeper labels) stay inside their parent's block.
    """
    a, b = function_span(src, symbol)
    labels = [(m.start(), len(m.group(1)), m.group(2)) for m in CASE_RE.finditer(src, a, b)]
    if not labels:
        return {}
    top = min(ind for _, ind, _ in labels)
    tops = [(pos, lab) for pos, ind, lab in labels if ind == top]
    out = {}
    for k, (pos, lab) in enumerate(tops):
        if k + 1 < len(tops):
            end = tops[k + 1][0]
        else:  # the last case runs to the switch's closing brace
            depth, i = 0, pos
            while i < b:
                depth += {"{": 1, "}": -1}.get(src[i], 0)
                if depth < 0:
                    break
                i += 1
            end = i
        out[lab] = (pos, end)
    return out


def apply_moves(src: str, symbol: str, moves) -> str:
    for case, old, new in moves:
        st, en = case_blocks(src, symbol)[case]
        blk = src[st:en]
        if re.search(rf"\b{re.escape(new)}\b", blk):
            raise ValueError(f"{new} already used in case {case}")
        if not re.search(rf"\b{re.escape(old)}\b", blk):
            raise ValueError(f"{old} not used in case {case}")
        src = src[:st] + re.sub(rf"\b{re.escape(old)}\b", new, blk) + src[en:]
    return src


def all_moves(src: str, symbol: str, cases=None):
    locs = declared_locals(src, symbol)
    for lab, (st, en) in case_blocks(src, symbol).items():
        if cases and lab.lower() not in cases:
            continue
        blk = src[st:en]
        used = {w for w in re.findall(r"\b\w+\b", blk) if w in locs}
        for old in sorted(used - FIXED):
            for new in locs:
                if new in used or new in FIXED or kind(locs[new]) != kind(locs[old]):
                    continue
                yield (lab, old, new)


def score(symbol: str, text: str) -> tuple[int, int, int]:
    """(aligned residual, size delta, masked) via fast_score; (10**6, 0, 0) on failure."""
    with tempfile.TemporaryDirectory(prefix="case-rename-") as tmp:
        p = Path(tmp) / "cand.c"
        p.write_text(text)
        r = subprocess.run([sys.executable, str(ROOT / "tools/fast_score.py"), symbol, str(p), "--aligned"],
                           cwd=ROOT, capture_output=True, text=True)
    m = re.search(r": \d+ \d+ (\d+) \d+ (-?\d+)", r.stdout)
    a = re.search(r"residual (\d+)", r.stdout)
    if not (m and a):
        return (10 ** 6, 0, 0)
    return (int(a.group(1)), int(m.group(2)), int(m.group(1)))


def run_cells(symbol, src, cells, jobs):
    def one(cell):
        try:
            text = apply_moves(src, symbol, cell)
        except ValueError:
            return cell, None
        return cell, score(symbol, text)
    with ThreadPoolExecutor(jobs) as ex:
        yield from ex.map(one, cells)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("mode", choices=("scan", "combine", "apply"))
    ap.add_argument("symbol")
    ap.add_argument("candidate")
    ap.add_argument("--cases", nargs="*", help="scan only these case labels")
    ap.add_argument("--moves", help="JSON list of [case, old, new]")
    ap.add_argument("--max", type=int, default=3, help="combine: largest subset size")
    ap.add_argument("-j", type=int, default=max(1, (os.cpu_count() or 2) // 2))
    ap.add_argument("--out")
    ns = ap.parse_args(argv)
    src = Path(ns.candidate).read_text()
    if ns.mode == "apply":
        Path(ns.out).write_text(apply_moves(src, ns.symbol, json.loads(Path(ns.moves).read_text())))
        return 0
    base = score(ns.symbol, src)
    print(f"base aligned {base[0]} size {base[1]:+d} masked {base[2]}", flush=True)
    if ns.mode == "scan":
        cells = [[m] for m in all_moves(src, ns.symbol, {c.lower() for c in ns.cases} if ns.cases else None)]
    else:
        moves = [tuple(m) for m in json.loads(Path(ns.moves).read_text())]
        cells = [list(c) for k in range(1, ns.max + 1) for c in itertools.combinations(moves, k)
                 if len({m[0] for m in c}) == len(c)]
    print(f"{len(cells)} cells", file=sys.stderr, flush=True)
    results = []
    out = open(ns.out, "w") if ns.out else None
    for cell, r in run_cells(ns.symbol, src, cells, ns.j):
        if r is None:
            continue
        results.append((r, cell))
        if out:
            out.write(json.dumps([list(r), cell]) + "\n")
            out.flush()
    results.sort(key=lambda x: (x[0][1] != 0, x[0][0], abs(x[0][1])))
    for r, cell in results[:40]:
        print(f"{r[0]:5d} {r[1]:+5d} {r[2]:5d}  " + "  ".join(f"{c}:{o}->{n}" for c, o, n in cell))
    return 0


if __name__ == "__main__":
    sys.exit(main())
