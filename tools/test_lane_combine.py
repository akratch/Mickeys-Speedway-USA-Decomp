#!/usr/bin/env python3
"""Tests for tools/lane_combine.py on synthetic TUs (no compile)."""

from __future__ import annotations

import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lane_combine as lc  # noqa: E402

BASE = """#include "common.h"

extern int D_1;

void helper(a)
unsigned char a;
{
    D_1 = a;
}

void target(int x) {
    int y;
    y = x + 1;
    D_1 = y;
    if (y) {
        D_1 = 2;
    }
    helper(y);
}

void other(void) {
    D_1 = 9;
}
"""


def resolve(text: str, values: dict[str, int]) -> str:
    """Evaluate every #if/#elif/#else/#endif whose condition names only SHAPE_ axes."""
    out, stack = [], []  # stack: [taken, active, parent_active]
    for line in text.split("\n"):
        m = re.match(r"^\s*#\s*(if|elif|else|endif)\b(.*)$", line)
        active = all(s[1] for s in stack)
        if not m:
            if active:
                out.append(line)
            continue
        kind, cond = m.group(1), m.group(2)
        expr = re.sub(r"\bSHAPE_(\w+)", lambda n: str(values.get(n.group(1), 0)), cond)
        expr = expr.replace("||", " or ").replace("&&", " and ")
        if kind == "if":
            hit = bool(eval(expr))  # noqa: S307 - synthetic test conditions only
            stack.append([hit, hit])
        elif kind == "elif":
            hit = not stack[-1][0] and bool(eval(expr))  # noqa: S307
            stack[-1] = [stack[-1][0] or hit, hit]
        elif kind == "else":
            stack[-1] = [True, not stack[-1][0]]
        else:
            stack.pop()
    return "\n".join(out)


class FunctionSpanTests(unittest.TestCase):
    def test_spans_include_knr_and_skip_alternative_arms(self):
        text = BASE + "#ifdef NON_MATCHING\nvoid late(void) {\n#if SHAPE_q == 0\n    if (D_1) {\n#else\n    if (!D_1) {\n#endif\n    }\n}\n#endif\n"
        names = {name: (a, b) for name, a, b in lc.function_spans(text)}
        self.assertEqual(set(names), {"helper", "target", "other", "late"})
        self.assertEqual(names["helper"], (4, 8))
        self.assertEqual(lc.enclosing(lc.function_spans(text), 12, 13), "target")
        self.assertEqual(lc.enclosing(lc.function_spans(text), 2, 3), "file scope")


class CombineTests(unittest.TestCase):
    A = BASE.replace("    y = x + 1;\n", "    y = x + 2;\n")
    B = BASE.replace("    helper(y);\n", "    helper(y & 0xFF);\n")

    def test_disjoint_lanes_become_independent_axes(self):
        result = lc.combine(BASE, [("a", self.A), ("b", self.B)])
        self.assertEqual(result["axes"], ["SHAPE_a", "SHAPE_b"])
        self.assertFalse(result["overlaps"])
        product = result["product"]
        self.assertEqual(resolve(product, {}), BASE)
        self.assertEqual(resolve(product, {"a": 1}), self.A)
        self.assertEqual(resolve(product, {"b": 1}), self.B)
        both = resolve(product, {"a": 1, "b": 1})
        self.assertIn("x + 2", both)
        self.assertIn("y & 0xFF", both)
        self.assertEqual([h.function for h in result["hunks"]], ["target", "target"])

    def test_overlap_is_reported_and_the_first_lane_wins(self):
        c = BASE.replace("    y = x + 1;\n", "    y = x - 1;\n")
        result = lc.combine(BASE, [("a", self.A), ("c", c)])
        self.assertEqual(len(result["overlaps"]), 1)
        lines = lc.report_lines(result, False)
        self.assertTrue(any(l.startswith("OVERLAP base lines 13-13 in target") for l in lines), lines)
        self.assertEqual(resolve(result["product"], {"a": 1, "c": 1}), self.A)
        self.assertEqual(resolve(result["product"], {"c": 1}), c)

    def test_per_hunk_axes(self):
        two = self.A.replace("        D_1 = 2;\n", "        D_1 = 3;\n")
        result = lc.combine(BASE, [("a", two)], per_hunk=True)
        self.assertEqual(result["axes"], ["SHAPE_a_0", "SHAPE_a_1"])
        only_second = resolve(result["product"], {"a_1": 1})
        self.assertIn("D_1 = 3;", only_second)
        self.assertIn("x + 1", only_second)

    def test_function_filter_reports_the_rest(self):
        lane = self.A.replace("    D_1 = 9;\n", "    D_1 = 10;\n")
        result = lc.combine(BASE, [("a", lane)], function="target")
        self.assertEqual(len(result["hunks"]), 1)
        self.assertEqual([h.function for h in result["ignored"]], ["other"])
        self.assertIn("D_1 = 9;", resolve(result["product"], {"a": 1}))

    def test_preprocessor_chains_are_never_cut(self):
        base = BASE.replace("    y = x + 1;\n", "#if SHAPE_k == 0\n    y = x + 1;\n#else\n    y = x * 2;\n#endif\n")
        lane = BASE.replace("    y = x + 1;\n", "    y = x * 2;\n")  # the lane froze axis k at 1
        result = lc.combine(base, [("a", lane)])
        self.assertEqual(len(result["hunks"]), 1)
        self.assertTrue(lc.directive_balanced(result["product"].split("\n")))
        self.assertEqual(resolve(result["product"], {"a": 1}), lane)
        self.assertEqual(resolve(result["product"], {"k": 1}), resolve(base, {"k": 1}))


class RebaseTests(unittest.TestCase):
    A = CombineTests.A

    def test_hunks_carry_onto_a_newer_base(self):
        newer = BASE.replace("    D_1 = 9;\n", "    D_1 = 11;\n").replace("extern int D_1;\n", "extern int D_1;\nextern int D_2;\n")
        result = lc.combine(BASE, [("a", self.A)], rebase_text=newer)
        self.assertFalse(result["conflicts"])
        on = resolve(result["product"], {"a": 1})
        self.assertIn("x + 2", on)
        self.assertIn("D_1 = 11;", on)
        self.assertIn("extern int D_2;", on)
        self.assertEqual(resolve(result["product"], {}), newer)

    def test_lines_the_new_base_changed_are_a_conflict(self):
        newer = BASE.replace("    y = x + 1;\n", "    y = x + 7;\n")
        result = lc.combine(BASE, [("a", self.A)], rebase_text=newer)
        self.assertEqual(len(result["conflicts"]), 1)
        self.assertIn("LEFT OUT", "\n".join(lc.report_lines(result, False)))
        self.assertEqual(resolve(result["product"], {"a": 1}), newer)

    def test_a_hunk_already_in_the_new_base_is_reported_not_conflicted(self):
        result = lc.combine(BASE, [("a", self.A)], rebase_text=self.A)
        self.assertFalse(result["conflicts"])
        self.assertEqual(len(result["applied"]), 1)
        self.assertIn("already in the new base", "\n".join(lc.report_lines(result, False)))


class CliTests(unittest.TestCase):
    def run_cli(self, *extra: str, lanes: dict[str, str]):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        root = Path(tmp.name)
        (root / "base.c").write_text(BASE)
        args = [str(root / "base.c"), "-o", str(root / "out.c"), *extra]
        for name, text in lanes.items():
            (root / f"{name}.c").write_text(text)
            args += ["--lane", f"{name}={root / (name + '.c')}"]
        return lc.main(args), root

    def test_writes_the_product(self):
        status, root = self.run_cli(lanes={"a": CombineTests.A})
        self.assertEqual(status, 0)
        self.assertIn("#if SHAPE_a == 1", (root / "out.c").read_text())

    def test_strict_refuses_an_overlap(self):
        c = BASE.replace("    y = x + 1;\n", "    y = x - 1;\n")
        status, root = self.run_cli("--strict", lanes={"a": CombineTests.A, "c": c})
        self.assertEqual(status, 1)
        self.assertFalse((root / "out.c").exists())

    def test_bad_lane_name(self):
        status, _root = self.run_cli(lanes={"bad-name": CombineTests.A})
        self.assertEqual(status, 2)


if __name__ == "__main__":
    unittest.main()
