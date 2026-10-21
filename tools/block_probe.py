#!/usr/bin/env python3
"""Probe the callee-saved threshold (N/4, L171) by adding empty basic blocks to a candidate.

    tools/block_probe.py <symbol> --candidate F --proc N --owner NAME[,NAME]
                         [--blocks 0,4,8,12,20] [--maker 'if (0) { }'] [--from LINE] [--json PATH]

WHY. A symbol web is coloured when its `totalsave` exceeds the cost of a
callee-saved register, N/4 for N basic blocks (law L171). Whether a web
would SPLIT if the procedure had more blocks, and what its pieces then do,
is a question about the records, not the bytes: the empty blocks a `--maker`
adds (`if (0) { }` by default; `do { } while (0);` and `while (0) { }` also
make blocks; an unused label makes none) are not inert in the allocation, so
the bytes of a probe cell are a reading aid, never a candidate.

WHAT IT DOES. For each k in --blocks the maker is appended to the first k
statement lines of the function at or after --from (same physical line, so
every line keeps its number), the cell is compiled stock (masked, aligned
residual, size delta) and instrumented with CDX_WEBREPORT, and each owner's
phase-1 decisions are read by EXPRESSION (web numbers renumber across cells):
the block count N, N/4, and per owner every row (web, decision, colour, the
totalsave the record carries). Owners are named by their C name; the name is
resolved through one DKWB_SUBST_TRACE compile of the candidate to the frame
offset uopt's records carry.

On func_80054B3C's safe R4 cell (D1 solved by source) this showed x1
(totalsave 9) splitting at N=36 and z2 (13) at N=52, exactly N/4, with the
split remainders then coloured and the bytes moving away from the target.
"""
from __future__ import annotations

import argparse
import json
import os
import tempfile
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import fast_score  # noqa: E402
import keep_search as ks  # noqa: E402
import lever_sweep as ls  # noqa: E402
import subst_report as sr  # noqa: E402

ROOT = fast_score.ROOT
MAKERS = {"if0": "if (0) { }", "dowhile": "do { } while (0);", "while0": "while (0) { }"}


def owner_expr(var: str) -> str:
    """`M-68:4` (subst trace id) -> `var:-68:1:4` (web_report expression)."""
    m = re.fullmatch(r"M(-?\d+):(\d+)", var)
    if not m:
        raise SystemExit(f"owner id {var!r} is not a local frame variable (M<offset>:<size>)")
    return f"var:{m.group(1)}:1:{m.group(2)}"


def resolve_owners(rows: list[dict], source: list[str], names: list[str]) -> dict[str, str]:
    """C name -> web_report expression, through the trace's definition lines."""
    out = {}
    groups = sr.group_by_var(rows)
    for name in names:
        hits = [var for var, items in groups.items() if sr.name_hint(items, source) == name]
        if len(hits) != 1:
            raise SystemExit(f"owner {name!r}: {len(hits)} trace variables carry that name "
                             f"({', '.join(hits) or 'none'}); the trace names a local by its definition lines")
        out[name] = owner_expr(hits[0])
    return out


def statement_lines(fn: ls.Function, start: int) -> list[int]:
    """Distinct lines of simple statements at or after `start`, in order."""
    out = []
    for s in ls.simple_statements(fn):
        line = fn.line_of(s.end - 1)
        if line >= start and fn.line_of(s.start) == line and line not in out:
            out.append(line)
    return out


def make_cell(text: str, lines: list[int], maker: str) -> str:
    rows = text.split("\n")
    for ln in lines:
        rows[ln - 1] = rows[ln - 1] + " " + maker
    return "\n".join(rows)


def owner_rows(decs: list[dict], owners: dict[str, str]) -> dict[str, list[dict]]:
    by = {name: [] for name in owners}
    for d in decs:
        for name, expr in owners.items():
            if d["phase"] == "p1" and d["expr"] == expr:
                by[name].append({"web": d["web"], "decision": d["decision"], "colour": d["colour"]})
    return by


def render(k: int, n: int, score: dict | None, rows: dict[str, list[dict]]) -> str:
    head = f"+{k:<3d} N {n:<3d} N/4 {n / 4:<5.2f}"
    if score:
        head += f" masked {score['masked']:>4d} delta {score['delta']:>+4d} resid {score['residual']:>4d}"
    parts = []
    for name, items in rows.items():
        parts.append(f"{name}: " + (" ; ".join(
            f"w{r['web']} {r['decision']}" + (f" c{r['colour']}" if r["colour"] is not None else " mem")
            for r in items) or "no row"))
    return head + "  |  " + "  |  ".join(parts)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--candidate", required=True)
    ap.add_argument("--proc", type=int, required=True, help="procedure ordinal (as web_report prints)")
    ap.add_argument("--owner", required=True, help="C names of the locals whose webs to watch")
    ap.add_argument("--blocks", default="0,4,8,12,16,20,24", help="how many empty blocks to add per cell")
    ap.add_argument("--maker", default="if0", help="if0 | dowhile | while0 | a literal statement")
    ap.add_argument("--from", dest="start", type=int, default=0, help="first source line eligible for a block")
    ap.add_argument("--scratch", help="work directory (default block-probe-<pid> under the system temp directory)")
    ap.add_argument("--json", metavar="PATH")
    ns = ap.parse_args(argv)

    ls.use_root(ROOT)
    candidate = pathlib.Path(ns.candidate).resolve()
    if not candidate.is_file():
        raise SystemExit(f"no such candidate: {candidate}")
    source = fast_score.tracked_source_for(ns.symbol)
    if candidate == (ROOT / source).resolve():
        raise SystemExit("--candidate must be a scratch copy, not the tracked source")
    text = candidate.read_text()
    fn = ls.find_function(text, ns.symbol)
    maker = MAKERS.get(ns.maker, ns.maker)
    scratch = pathlib.Path(ns.scratch or os.path.join(tempfile.gettempdir(), f"block-probe-{os.getpid()}")).resolve()
    if ROOT.resolve() in scratch.parents or scratch == ROOT.resolve():
        raise SystemExit("--scratch must be outside the repository")
    scratch.mkdir(parents=True, exist_ok=True)
    run = ks.Runner(ns.symbol, source, scratch, ns.proc)

    base = scratch / "base.c"
    base.write_text(text)
    trace = scratch / "base.trace"
    err = run.compile(base, scratch / "base.o", trace=trace)
    if err:
        raise SystemExit(f"base compile failed: {err}")
    rows = sr.parse_trace(trace.read_text() if trace.is_file() else "", ns.symbol)
    owners = resolve_owners(rows, text.splitlines(), [o.strip() for o in ns.owner.split(",") if o.strip()])
    print(f"{ns.symbol} proc {ns.proc}: owners " + ", ".join(f"{n} = {e}" for n, e in owners.items())
          + f"; maker `{maker}`")
    lines = statement_lines(fn, ns.start)
    results = []
    for k in [int(x) for x in ns.blocks.split(",") if x.strip()]:
        if k > len(lines):
            print(f"+{k}: only {len(lines)} statement lines available; stopping")
            break
        cell = scratch / f"blocks{k:02d}.c"
        cell.write_text(make_cell(text, lines[:k], maker))
        obj, iobj, log = scratch / f"blocks{k:02d}.o", scratch / f"blocks{k:02d}.i.o", scratch / f"blocks{k:02d}.log"
        err = run.compile(cell, obj)
        if err:
            print(f"+{k}: compile failed: {err}")
            continue
        score = run.scorer.score(obj)
        err = run.compile(cell, iobj, log=log)
        if err:
            print(f"+{k}: instrumented compile failed: {err}")
            continue
        gate = run.func_bytes(iobj) != run.func_bytes(obj)
        decs, blocks = ls.decisions(log.read_text(errors="replace"), ns.proc)
        orows = owner_rows(decs, owners)
        row = {"blocks_added": k, "lines": lines[:k], "N": len(blocks),
               "masked": score["masked"], "delta": score["delta"], "residual": score["residual"],
               "owners": orows, "gate": gate}
        results.append(row)
        print(render(k, len(blocks), row, orows) + ("  [identity gate FAILED]" if gate else ""), flush=True)
    if ns.json:
        pathlib.Path(ns.json).write_text(json.dumps({"symbol": ns.symbol, "candidate": str(candidate),
                                                     "maker": maker, "owners": owners, "cells": results}, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
