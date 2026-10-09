"""Fixture tests for tools/subst_report.py (synthetic traces, no compiler needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import subst_report as sr  # noqa: E402

TRACE = """\
LOCAL proc=f use_line=5 def_line=3 var=M-76:4 decision=substituted reason=single_use_no_ilod rhs=op rhs_uses=1 vreg=1
LOCAL proc=f use_line=5 def_line=3 var=M-76:4 decision=substituted reason=shared_expr rhs=op rhs_uses=2 vreg=1
GLOBAL proc=f use_line=7 def_line=3 var=M-76:4 decision=substituted reason=copied rhs=op vreg=1 cands=[L3:ok]
STORE proc=f def_line=3 var=M-76:4 decision=deleted reason=dead lval_av=1 store_av=1 dead_after=1 rhs=op
LOCAL proc=f use_line=9 def_line=8 var=M-80:4 decision=kept reason=treekilled rhs=op rhs_uses=1 vreg=1
GLOBAL proc=f use_line=10 def_line=-1 var=M-80:4 decision=kept reason=no_usable_def rhs=none vreg=1 cands=[L8:def_not_available,L9:forced_keep]
LOCAL proc=g use_line=1 def_line=1 var=M-4:4 decision=kept reason=bigtree rhs=op rhs_uses=1 vreg=1
noise line that is not a record
"""

SOURCE = ["", "", "    twoZ1 = z1 + z1;", "", "    q = twoZ1 * 2 + twoZ1;", "", "    r = twoZ1;",
          "    size = i + 9;", "    *p = size;", "    t = size;"]


class ParseTests(unittest.TestCase):
    def test_procedure_filter_and_kinds(self):
        rows = sr.parse_trace(TRACE, "f")
        self.assertEqual([r["kind"] for r in rows], ["LOCAL", "LOCAL", "GLOBAL", "STORE", "LOCAL", "GLOBAL"])
        self.assertEqual(len(sr.parse_trace(TRACE)), 7)
        self.assertEqual(sr.parse_trace(TRACE, "missing"), [])

    def test_numeric_fields_and_cands(self):
        row = sr.parse_trace(TRACE, "f")[5]
        self.assertEqual((row["use_line"], row["def_line"]), (10, -1))
        self.assertEqual(sr.parse_cands(row["cands"]), [(8, "def_not_available"), (9, "forced_keep")])

    def test_name_hint_from_definition_lines(self):
        rows = sr.parse_trace(TRACE, "f")
        groups = sr.group_by_var(rows)
        self.assertEqual(list(groups), ["M-76:4", "M-80:4"])
        self.assertEqual(sr.name_hint(groups["M-76:4"], SOURCE), "twoZ1")
        self.assertEqual(sr.name_hint(groups["M-80:4"], SOURCE), "size")
        self.assertEqual(sr.name_hint([{"def_line": 99}], SOURCE), "")


class RenderTests(unittest.TestCase):
    def test_render_groups_and_tallies(self):
        text = sr.render(sr.parse_trace(TRACE, "f"), SOURCE)
        self.assertIn("M-76:4  (twoZ1)", text)
        self.assertIn("kept        treekilled", text)
        self.assertIn("cands: L8:def_not_available, L9:forced_keep", text)
        self.assertIn("STORE  def L3     deleted  dead", text)
        self.assertIn("summary: GLOBAL kept 1, GLOBAL substituted 1, LOCAL kept 1, LOCAL substituted 2, STORE deleted 1", text)

    def test_no_empty_cands_line(self):
        rows = [{"kind": "GLOBAL", "use_line": 5, "def_line": -1, "var": "M-8:4", "decision": "kept",
                 "reason": "not_entry_value", "rhs": "none", "cands": "[]"}]
        self.assertNotIn("cands:", sr.render(rows, SOURCE))

    def test_empty_log_is_an_exit_not_a_traceback(self):
        import tempfile
        with tempfile.NamedTemporaryFile("w", suffix=".log") as f:
            f.write("nothing\n")
            f.flush()
            self.assertEqual(sr.main(["f", "--trace", f.name]), 1)


if __name__ == "__main__":
    unittest.main()
