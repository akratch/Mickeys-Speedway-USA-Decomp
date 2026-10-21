"""Fixture tests for tools/block_probe.py (synthetic C and traces; no compiler needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import block_probe as bp  # noqa: E402
import lever_sweep as ls  # noqa: E402
import subst_report as sr  # noqa: E402

SRC = """\
extern float sqrtf(float);

#ifdef NON_MATCHING
float fixture(float a, float b) {
    float q;
    float r;
    q = a * b;
    if (a != 0.0f) {
        r = q * q;
        if (r < b) {
            b = sqrtf(b - r);
        }
    }
    else
        b = q;
    return b;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/fixture.s")
#endif
"""

TRACE = """\
LOCAL proc=fixture use_line=9 def_line=7 var=M-4:4 decision=substituted reason=single_use_no_ilod rhs=op rhs_uses=1 vreg=1
STORE proc=fixture def_line=7 var=M-4:4 decision=kept reason=live_after_block lval_av=1 store_av=1 dead_after=0 rhs=op
STORE proc=fixture def_line=9 var=M-8:4 decision=deleted reason=dead lval_av=1 store_av=1 dead_after=1 rhs=op
"""


class ProbeTests(unittest.TestCase):
    def test_owner_expr(self):
        self.assertEqual(bp.owner_expr("M-68:4"), "var:-68:1:4")
        with self.assertRaises(SystemExit):
            bp.owner_expr("P16:4")

    def test_resolve_owners_by_name(self):
        rows = sr.parse_trace(TRACE, "fixture")
        self.assertEqual(bp.resolve_owners(rows, SRC.splitlines(), ["q", "r"]),
                         {"q": "var:-4:1:4", "r": "var:-8:1:4"})
        with self.assertRaises(SystemExit):
            bp.resolve_owners(rows, SRC.splitlines(), ["nothing"])

    def test_statement_lines_and_cell(self):
        fn = ls.find_function(SRC, "fixture")
        lines = bp.statement_lines(fn, 0)
        self.assertEqual(lines, [7, 9, 11, 15, 16])  # statements only; headers and `else` never
        self.assertEqual(bp.statement_lines(fn, 10), [11, 15, 16])
        cell = bp.make_cell(SRC, lines[:2], "if (0) { }")
        self.assertEqual(cell.count("\n"), SRC.count("\n"))
        self.assertTrue(cell.splitlines()[6].endswith("q = a * b; if (0) { }"))
        self.assertTrue(cell.splitlines()[8].endswith("r = q * q; if (0) { }"))
        self.assertNotIn("if (0)", cell.splitlines()[10])

    def test_owner_rows_by_expression_and_phase(self):
        decs = [{"phase": "p1", "expr": "var:-4:1:4", "web": 7, "decision": "color", "colour": 27},
                {"phase": "p1", "expr": "var:-4:1:4", "web": 7, "decision": "split", "colour": None},
                {"phase": "p2", "expr": "var:-4:1:4", "web": 7, "decision": "color", "colour": 3},
                {"phase": "p1", "expr": "var:-8:1:4", "web": 9, "decision": "color", "colour": 30}]
        rows = bp.owner_rows(decs, {"q": "var:-4:1:4", "r": "var:-8:1:4", "none": "var:-99:1:4"})
        self.assertEqual([r["web"] for r in rows["q"]], [7, 7])
        self.assertEqual(rows["r"][0]["colour"], 30)
        self.assertEqual(rows["none"], [])
        text = bp.render(4, 36, {"masked": 10, "delta": -4, "residual": 9}, rows)
        self.assertIn("N 36  N/4 9.00", text)
        self.assertIn("q: w7 color c27 ; w7 split mem", text)
        self.assertIn("none: no row", text)


if __name__ == "__main__":
    unittest.main()
