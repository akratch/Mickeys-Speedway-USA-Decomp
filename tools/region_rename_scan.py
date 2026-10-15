#!/usr/bin/env python3
"""Rename a local inside one region of a function, scan every such rename, adopt the best.

    tools/region_rename_scan.py scan   <symbol> <candidate.c> [--regions R.json] [--driver D.c] [-j 8] [--out rows.tsv]
    tools/region_rename_scan.py greedy <symbol> <candidate.c> [--regions R.json] [--driver D.c] [-j 8] --out best.c
    tools/region_rename_scan.py regions <symbol> <candidate.c>          # print the automatic regions

uopt colours a local as one web over every region that uses it, so which
declared local carries a loop's value decides that value's register and,
through interference, everyone else's. A move is (first, last, old, new):
every occurrence of local `old` on lines first..last is renamed to declared
local `new` of the same declared type, provided `new` does not occur on those
lines. This is the switch-case scan of tools/case_rename_scan.py generalised
to arbitrary line regions; on overlay64GenerateTexture it took 252 masked
words to 203 in five adoptions (2026-10-15).

Regions come from --regions, a JSON object {name: [first_line, last_line]}
(1-based, inclusive, in the candidate's own numbering; a rename keeps line
numbers, so they stay valid across adoptions), or automatically: every
brace block opened by `while`, `for`, `do`, `if` or `else` inside the
function, plus each top-level statement run between blank lines.

A rename can change meaning (the region reads a value of `old` set outside
it, or writes `new` while `new` is live across it). --static-guard refuses
the first kind where it is visible in straight-line text. With --driver, every
cell is run through tools/behaviour_check.py against the starting candidate
and a cell whose behaviour differs is dropped before it is scored. Without a
driver nothing is checked and every adopted row must be argued by hand.
A driver only checks the inputs it builds: on 2026-10-15 a move that turned
a `< 0` clamp into a test of the loop counter passed a driver whose buffers
never made the sum zero. Make the driver reach every branch.

`scan` scores every move once (tools/fast_score.py --aligned) and prints
`masked delta residual moves` rows, best first. `greedy` adopts the best
size-exact improvement, rescans, and repeats until nothing improves, then
writes the result. The score is the measurement; the behaviour gate is a
filter, not a proof (see behaviour_check.py for its limits).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

DECL_RE = re.compile(r"^((?:unsigned |signed |struct |volatile )*[A-Za-z_]\w*)\s*(\**)\s*([A-Za-z_]\w*);$")
BLOCK_RE = re.compile(r"^\s*(?:\}\s*)?(?:while|for|do|if|else)\b.*\{\s*$")
SCORE_RE = re.compile(r":\s+\d+\s+\d+\s+(\d+)\s+\d+\s+(-?\d+)\s")
RESID_RE = re.compile(r"residual (\d+)")


def function_lines(lines: list[str], symbol: str) -> tuple[int, int, int]:
    """(definition line, first body line after the declarations, closing line), 0-based."""
    for n, line in enumerate(lines):
        if re.match(rf"^[\w\s\*]*\b{re.escape(symbol)}\s*\([^;]*\)\s*$", line) or \
           re.match(rf"^[\w\s\*]*\b{re.escape(symbol)}\s*\([^;]*\)\s*\{{\s*$", line):
            start = n
            break
    else:
        raise SystemExit(f"no definition of {symbol}")
    depth, end, opened = 0, None, False
    for n in range(start, len(lines)):
        depth += lines[n].count("{") - lines[n].count("}")
        opened = opened or "{" in lines[n]
        if opened and depth == 0:
            end = n
            break
    if end is None:
        raise SystemExit(f"unbalanced braces in {symbol}")
    body = start + 1
    while body < end and not lines[body].strip().startswith("{"):
        body += 1
    body += 1
    while body < end and (DECL_RE.match(lines[body].strip()) or not lines[body].strip()):
        body += 1
    return start, body, end


def declared_locals(lines: list[str], symbol: str) -> dict[str, str]:
    """name -> declared type for the function-scope declarations."""
    start, body, _ = function_lines(lines, symbol)
    out = {}
    for n in range(start, body):
        m = DECL_RE.match(lines[n].strip())
        if m:
            out[m.group(3)] = (m.group(1) + m.group(2)).strip()
    return out


def auto_regions(lines: list[str], symbol: str) -> dict[str, list[int]]:
    """Brace blocks opened by a control keyword, plus blank-line-separated statement runs."""
    _, body, end = function_lines(lines, symbol)
    regions: dict[str, list[int]] = {}
    for n in range(body, end):
        if not BLOCK_RE.match(lines[n]):
            continue
        depth = 0
        for m in range(n, end):
            depth += lines[m].count("{") - lines[m].count("}")
            if depth <= 0 and m > n:
                regions[f"block{n + 1}"] = [n + 1, m + 1]
                break
    run = None
    for n in range(body, end + 1):
        blank = n == end or not lines[n].strip()
        depth_ok = not lines[n].startswith(" " * 8) if n < end else True
        if not blank and run is None and depth_ok:
            run = n
        if blank and run is not None:
            regions.setdefault(f"run{run + 1}", [run + 1, n])
            run = None
    return regions


def apply_moves(lines: list[str], moves) -> list[str]:
    out = list(lines)
    for first, last, old, new in moves:
        for n in range(first - 1, last):
            out[n] = re.sub(rf"\b{re.escape(old)}\b", new, out[n])
    return out


def first_mention_is_assignment(text: str, name: str) -> bool | None:
    """True if `name`'s first mention in TEXT is a plain `name = ...` store, None if absent."""
    m = re.search(rf"\b{re.escape(name)}\b", text)
    if not m:
        return None
    return re.match(rf"{re.escape(name)}\s*=(?!=)", text[m.start():]) is not None


def locally_dead(lines: list[str], symbol: str, first: int, last: int, old: str) -> bool:
    """A cheap static screen: `old` is written before it is read inside the region, and
    is written before it is read after it (or not mentioned again). Loops that carry
    the value around the region's back edge are not seen; the driver is the check."""
    _, body, end = function_lines(lines, symbol)
    inside = "\n".join(lines[first - 1:last])
    after = "\n".join(lines[last:end])
    return first_mention_is_assignment(inside, old) is True and \
        first_mention_is_assignment(after, old) is not False


def candidate_moves(lines: list[str], symbol: str, regions: dict[str, list[int]], static_guard: bool = False):
    decl = declared_locals(lines, symbol)
    moves = []
    for first, last in regions.values():
        seg = "\n".join(lines[first - 1:last])
        for old, t in decl.items():
            if not re.search(rf"\b{re.escape(old)}\b", seg):
                continue
            if static_guard and not locally_dead(lines, symbol, first, last, old):
                continue
            for new, t2 in decl.items():
                if new == old or t2 != t or re.search(rf"\b{re.escape(new)}\b", seg):
                    continue
                moves.append((first, last, old, new))
    return sorted(set(moves))


def score(symbol: str, text: str, workdir: Path, reference: Path | None, driver: Path | None):
    fd, path = tempfile.mkstemp(suffix=".c", dir=workdir)
    os.write(fd, text.encode())
    os.close(fd)
    try:
        if driver is not None:
            import behaviour_check
            status, _ = behaviour_check.same_behaviour(Path(path), reference, driver)
            if status:
                return None
        r = subprocess.run([sys.executable, str(ROOT / "tools/fast_score.py"), symbol, path, "--aligned"],
                           cwd=ROOT, capture_output=True, text=True)
        m, a = SCORE_RE.search(r.stdout), RESID_RE.search(r.stdout)
        if not m:
            return None
        return int(m.group(1)), int(m.group(2)), int(a.group(1)) if a else -1
    finally:
        os.unlink(path)


def scan(symbol, lines, regions, driver, jobs, workdir, static_guard=False):
    ref = None
    if driver is not None:
        ref = workdir / "reference.c"
        ref.write_text("\n".join(lines))
    moves = candidate_moves(lines, symbol, regions, static_guard)

    def one(mv):
        return mv, score(symbol, "\n".join(apply_moves(lines, [mv])), workdir, ref, driver)

    with ThreadPoolExecutor(jobs) as ex:
        rows = [(mv, s) for mv, s in ex.map(one, moves) if s is not None]
    rows.sort(key=lambda r: (r[1][1] != 0, r[1][0], r[1][2]))
    return rows, len(moves)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("mode", choices=["scan", "greedy", "regions"])
    ap.add_argument("symbol")
    ap.add_argument("candidate", type=Path)
    ap.add_argument("--regions", type=Path)
    ap.add_argument("--driver", type=Path)
    ap.add_argument("-j", "--jobs", type=int, default=8)
    ap.add_argument("--out", type=Path)
    ap.add_argument("--static-guard", action="store_true",
                    help="also refuse moves whose `old` is read before written in the region or after it")
    a = ap.parse_args(argv)
    lines = a.candidate.read_text().split("\n")
    regions = json.loads(a.regions.read_text()) if a.regions else auto_regions(lines, a.symbol)
    if a.mode == "regions":
        for name, (f, l) in regions.items():
            print(f"{name}\t{f}\t{l}")
        return 0
    with tempfile.TemporaryDirectory(prefix="rrscan_") as tmp:
        workdir = Path(tmp)
        if a.mode == "scan":
            rows, total = scan(a.symbol, lines, regions, a.driver, a.jobs, workdir, a.static_guard)
            text = "".join(f"{s[0]}\t{s[1]}\t{s[2]}\t{json.dumps(list(mv))}\n" for mv, s in rows)
            (a.out.write_text(text) if a.out else sys.stdout.write(text))
            print(f"{total} cells, {len(rows)} scored", file=sys.stderr)
            return 0
        if a.out is None:
            ap.error("greedy needs --out")
        base = score(a.symbol, "\n".join(lines), workdir, None, None)
        best = base[0] if base else 10**9
        print(f"start {best}", file=sys.stderr)
        while True:
            rows, _ = scan(a.symbol, lines, regions, a.driver, a.jobs, workdir, a.static_guard)
            if not rows or rows[0][1][1] != 0 or rows[0][1][0] >= best:
                break
            mv, s = rows[0]
            lines = apply_moves(lines, [mv])
            best = s[0]
            print(f"adopt {list(mv)} -> {s[0]} (residual {s[2]})", file=sys.stderr)
        a.out.write_text("\n".join(lines))
        print(f"final {best}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
