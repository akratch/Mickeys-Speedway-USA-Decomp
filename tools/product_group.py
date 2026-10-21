#!/usr/bin/env python3
"""Best cell per group of a shape_product run.

    tools/product_group.py <product.json> --group AXIS[,AXIS...] [--rank aligned|positional] [--top K]

`tools/shape_product.py --json` writes every cell of a product. With six axes
that is hundreds of rows, and the question a lane asks of them is usually
narrower: *given* a setting of these one or two axes, how good can the rest of
the product make it? This reads the JSON (no compile), groups the cells by the
named axes' values, and prints the best cell of each group, ranked the same way
shape_product ranks (`aligned`: residual, then |delta|, then masked; `positional`:
|delta|, then masked; default: the run's own ranking), with the settings of the
other axes that achieved it:

    group                 cells  resid masked delta  best cell (other axes)
    o40=0 t45=1               8    102    119    +0  b4d=0 h4d=0
    o40=1 t45=1               8    104    121    +0  b4d=2 h4d=0

A group whose best is worse than another group's best is a closed setting for
those axes, whatever the others do. Cells that failed to compile are counted
per group and never ranked.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def rank_key(rank: str):
    if rank == "positional":
        return lambda r: (abs(r["delta"]), r["masked"])
    return lambda r: (r.get("residual", r["masked"]), abs(r["delta"]), r["masked"])


def group_cells(data: dict, axes: list[str], rank: str | None = None) -> list[dict]:
    known = list(data.get("axes", {})) or sorted({k for c in data["cells"] for k in c["cell"]})
    missing = [a for a in axes if a not in known]
    if missing:
        raise SystemExit(f"product_group: unknown axis {', '.join(missing)}; axes: {', '.join(known)}")
    rank = rank or data.get("rank", "aligned")
    key = rank_key(rank)
    groups: dict[tuple, dict] = {}
    for row in data["cells"]:
        values = tuple(row["cell"].get(a) for a in axes)
        g = groups.setdefault(values, {"values": dict(zip(axes, values)), "cells": 0, "errors": 0, "best": None})
        g["cells"] += 1
        if "error" in row:
            g["errors"] += 1
            continue
        if g["best"] is None or key(row) < key(g["best"]):
            g["best"] = row
    out = list(groups.values())
    out.sort(key=lambda g: (g["best"] is None, key(g["best"]) if g["best"] else (), tuple(map(str, g["values"].values()))))
    return out


def render(groups: list[dict], axes: list[str]) -> list[str]:
    label = lambda g: " ".join(f"{a}={g['values'][a]}" for a in axes)  # noqa: E731
    width = max([len("group")] + [len(label(g)) for g in groups])
    lines = [f"{'group':<{width}}  cells  resid masked  delta  best cell (other axes)"]
    for g in groups:
        b = g["best"]
        if b is None:
            lines.append(f"{label(g):<{width}}  {g['cells']:>5}  (all {g['errors']} cell(s) failed to compile)")
            continue
        others = " ".join(f"{k}={v}" for k, v in b["cell"].items() if k not in axes) or "-"
        errs = f"  [{g['errors']} compile error(s)]" if g["errors"] else ""
        lines.append(f"{label(g):<{width}}  {g['cells']:>5}  {b.get('residual', '-'):>5} {b['masked']:>6} "
                     f"{b['delta']:>+6}  {others}{errs}")
    return lines


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("product_json", type=Path, help="shape_product.py --json output")
    ap.add_argument("--group", required=True, help="comma-separated axis names (without SHAPE_)")
    ap.add_argument("--rank", choices=("aligned", "positional"), help="default: the run's own ranking")
    ap.add_argument("--top", type=int, default=0, help="print only the best K groups")
    ns = ap.parse_args(argv)
    data = json.loads(ns.product_json.read_text())
    axes = [a.strip().removeprefix("SHAPE_") for a in ns.group.split(",") if a.strip()]
    if not axes:
        ap.error("--group needs at least one axis")
    groups = group_cells(data, axes, ns.rank)
    shown = groups[: ns.top] if ns.top else groups
    print(f"{data.get('symbol', '?')}: {len(data['cells'])} cells in {len(groups)} group(s) over "
          f"{', '.join(axes)} (ranking {ns.rank or data.get('rank', 'aligned')})")
    for line in render(shown, axes):
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
