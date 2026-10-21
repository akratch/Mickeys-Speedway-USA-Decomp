#!/usr/bin/env python3
"""Tests for tools/product_group.py on a synthetic shape_product JSON."""

from __future__ import annotations

import io
import json
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import product_group as pg  # noqa: E402


def cell(a, b, c, residual, masked, delta=0):
    return {"cell": {"a": a, "b": b, "c": c}, "residual": residual, "masked": masked, "delta": delta}


DATA = {
    "symbol": "func_x", "rank": "aligned", "axes": {"a": [0, 1], "b": [0, 1], "c": [0, 1]},
    "cells": [
        cell(0, 0, 0, 30, 40), cell(0, 0, 1, 20, 50), cell(0, 1, 0, 25, 25), cell(0, 1, 1, 26, 26),
        cell(1, 0, 0, 10, 90, delta=4), cell(1, 0, 1, 12, 12), cell(1, 1, 0, 40, 40),
        {"cell": {"a": 1, "b": 1, "c": 1}, "error": ["cfe: Error"]},
    ],
}


class GroupTests(unittest.TestCase):
    def test_best_cell_per_group_by_aligned_residual(self):
        groups = pg.group_cells(DATA, ["a"])
        self.assertEqual([g["values"] for g in groups], [{"a": 1}, {"a": 0}])
        self.assertEqual(groups[0]["best"]["cell"], {"a": 1, "b": 0, "c": 0})
        self.assertEqual(groups[0]["errors"], 1)
        self.assertEqual(groups[1]["best"]["residual"], 20)

    def test_positional_ranking_prefers_size_zero(self):
        groups = pg.group_cells(DATA, ["a"], "positional")
        self.assertEqual(groups[0]["best"]["cell"], {"a": 1, "b": 0, "c": 1})

    def test_two_axes_and_an_all_error_group(self):
        groups = pg.group_cells(DATA, ["a", "b"])
        self.assertEqual(len(groups), 4)
        lines = pg.render(groups, ["a", "b"])
        self.assertTrue(lines[1].startswith("a=1 b=0"))
        self.assertIn("c=0", lines[1])
        self.assertIn("[1 compile error(s)]", "\n".join(lines))

    def test_unknown_axis_is_refused(self):
        with self.assertRaises(SystemExit):
            pg.group_cells(DATA, ["zz"])

    def test_cli_accepts_shape_prefix_and_top(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "p.json"
            path.write_text(json.dumps(DATA))
            out = io.StringIO()
            with redirect_stdout(out):
                self.assertEqual(pg.main([str(path), "--group", "SHAPE_b", "--top", "1"]), 0)
            text = out.getvalue()
            self.assertIn("8 cells in 2 group(s) over b", text)
            self.assertEqual(len(text.strip().splitlines()), 3)


if __name__ == "__main__":
    unittest.main()
