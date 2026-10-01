#!/usr/bin/env python3
"""Measure every cell of a product of source shapes for one symbol.

    tools/shape_product.py <symbol> <candidate.c> [--jobs N] [--top K]
                           [--fix NAME=VALUE ...] [--json PATH]

The lever that matched sixty functions on 2026-10-01 was never a single edit:
it was a PRODUCT of natural spellings, measured whole, in which cells that
regress alone are exact together (overlay 27: four field edits scoring 4, 12,
4 and 14 alone, 178 for one pair, 0 together). Every lane wrote the loop that
measures such a product by hand. This is that loop, tracked.

Mark the alternatives in ONE candidate TU with preprocessor axes:

    #if SHAPE_count == 0
        value = (gTimer -= amount);
        if (value <= 0) {
    #else
        gTimer -= amount;
        if (gTimer <= 0) {
    #endif

Any identifier spelled `SHAPE_<name>` is an axis; the integers it is compared
with in `#if` / `#elif` lines are its values (an axis compared only with 0
gets the values {0, 1}, so a bare `#if SHAPE_x == 0 ... #else` is a two-cell
axis). The runner takes the Cartesian product, compiles each cell with the
TU's configured command (per-file flags read from the recipe, never typed)
plus one `-DSHAPE_<name>=<value>` per axis, scores it the way the ranking
does, and prints the cells sorted by masked words then size delta. Marked
alternatives cost nothing in the tracked tree: a promoted file carries none of
them.

`--fix NAME=VALUE` pins an axis (to re-run a sub-product), `--top K` limits
the table, `--json` writes every cell's numbers for a shard. Exit status is 0
when at least one cell scores 0 masked words at size delta 0, else 1, so a
harness can branch on it.

Reading the result: an exact cell is the candidate to promote (write it out
without the axes, re-score, then follow the promotion sequence). When no cell
is exact, the best cells' shared settings are the facts and the axes they
disagree on are free; price the next axis from the brief's shape checklist
and add it. Record the product's size and floor in the shard: a lane that
reads "N cells, floor F" knows which axes are closed.
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

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fast_score  # noqa: E402

AXIS_RE = re.compile(r"\bSHAPE_([A-Za-z0-9_]+)\b")
VALUE_RE = re.compile(r"\bSHAPE_([A-Za-z0-9_]+)\s*(?:==|!=)\s*(-?\d+)")


def axes_of(text: str) -> dict[str, list[int]]:
    """Return {axis: sorted values} for every SHAPE_ identifier in `text`."""
    values: dict[str, set[int]] = {}
    for name in AXIS_RE.findall(text):
        values.setdefault(name, set())
    for name, value in VALUE_RE.findall(text):
        values[name].add(int(value))
    out = {}
    for name, vals in values.items():
        if not vals:
            vals = {0}
        if vals == {0}:
            vals = {0, 1}
        out[name] = sorted(vals)
    return dict(sorted(out.items()))


def cell_defines(cell: dict[str, int]) -> list[str]:
    return [f"-DSHAPE_{k}={v}" for k, v in cell.items()]


def score_object(symbol: str, obj: Path) -> dict | None:
    proc = subprocess.run(
        [sys.executable, str(fast_score.ROOT / "tools/score_symbol.py"), "--json", "--object", str(obj), symbol],
        cwd=fast_score.ROOT, capture_output=True, text=True,
    )
    try:
        rows = json.loads(proc.stdout)["functions"]
    except (ValueError, KeyError):
        return None
    for row in rows:
        if row.get("symbol") == symbol:
            return row
    return None


def run_cell(symbol: str, base_args: list[str], candidate: Path, source: str,
             workdir: Path, index: int, cell: dict[str, int]) -> dict:
    obj = workdir / f"cell{index}.o"
    args = fast_score.rewrite_io(base_args, candidate, source, obj)
    # Defines go before the source so the recipe's own -D flags keep their order.
    args = args[:-1] + cell_defines(cell) + args[-1:]
    cc = subprocess.run(args, cwd=fast_score.ROOT, capture_output=True, text=True)
    result = {"cell": cell, "index": index}
    if cc.returncode:
        result.update(error=cc.stderr.strip().splitlines()[-1:] or ["compile failed"])
        return result
    row = score_object(symbol, obj)
    if row is None:
        result.update(error=["symbol not found in object"])
        return result
    result.update(
        masked=row["relocation_masked_differing_words"],
        raw=row["differing_words"],
        delta=row["size_delta"],
        first=row["relocation_masked_first_mismatch_offset"],
    )
    return result


def sort_key(r: dict):
    if "error" in r:
        return (1, 0, 0)
    return (0, abs(r["delta"]), r["masked"])


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("candidate")
    ap.add_argument("--jobs", type=int, default=max(2, min(6, (os.cpu_count() or 4) // 2)))
    ap.add_argument("--top", type=int, default=20)
    ap.add_argument("--fix", action="append", default=[], metavar="NAME=VALUE")
    ap.add_argument("--json", metavar="PATH")
    ns = ap.parse_args(argv)

    candidate = Path(ns.candidate).resolve()
    text = candidate.read_text()
    axes = axes_of(text)
    if not axes:
        raise SystemExit("no SHAPE_<name> axes found in the candidate")
    for fix in ns.fix:
        name, _, value = fix.partition("=")
        if name not in axes:
            raise SystemExit(f"--fix names unknown axis {name!r}; axes: {', '.join(axes)}")
        axes[name] = [int(value)]
    names = list(axes)
    cells = [dict(zip(names, combo)) for combo in itertools.product(*(axes[n] for n in names))]
    source = fast_score.tracked_source_for(ns.symbol)
    base_args = fast_score.configured_cc_args(source)
    print(f"{ns.symbol}: {len(cells)} cells over {len(names)} axes "
          + ", ".join(f"{n}={axes[n]}" for n in names), flush=True)

    with tempfile.TemporaryDirectory(prefix="shape-product-") as tmp:
        workdir = Path(tmp)
        with ThreadPoolExecutor(max_workers=ns.jobs) as pool:
            results = list(pool.map(
                lambda ic: run_cell(ns.symbol, base_args, candidate, source, workdir, *ic),
                enumerate(cells),
            ))

    results.sort(key=sort_key)
    exact = [r for r in results if "error" not in r and r["masked"] == 0 and r["delta"] == 0]
    errors = [r for r in results if "error" in r]
    best = results[0] if results and "error" not in results[0] else None
    print(f"exact cells: {len(exact)}; compile errors: {len(errors)}; "
          + (f"floor: {best['masked']} masked at delta {best['delta']:+d}" if best else "no scored cell"))
    print(f"{'masked':>6} {'delta':>6} {'first':>8}  cell")
    for r in results[: ns.top]:
        if "error" in r:
            print(f"{'ERR':>6} {'':>6} {'':>8}  {r['cell']}  {r['error'][0][:80]}")
        else:
            print(f"{r['masked']:>6} {r['delta']:>+6} {r['first']:>#8x}  {r['cell']}")
    if ns.json:
        Path(ns.json).write_text(json.dumps({"symbol": ns.symbol, "axes": axes, "cells": results}, indent=1))
    return 0 if exact else 1


if __name__ == "__main__":
    sys.exit(main())
