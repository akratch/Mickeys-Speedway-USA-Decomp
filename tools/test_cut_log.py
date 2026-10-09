"""Fixture tests for tools/cut_log.py (synthetic logs, no compiler needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import cut_log  # noqa: E402


def op(c, lim, opcode, w1=0):
    return f"OP c={c} lim={lim} {opcode:02x}000000 {w1:08x} 00000004 00000000"


LOG = "\n".join([
    op(0, 0, 0x7a),                       # data before any procedure
    op(0, 20, 0x21),                      # Uent: procedure 0
    op(0, 20, 0x51, 10),
    op(1, 20, 0x52),
    op(2, 20, 0x51, 11),
    op(25, 20, 0x52),
    "BB_TERM op=51",
    "AG site=125154 op=51 cur=1014d3e0",
    "RNI op=51",
    op(0, 20, 0x51, 12),                  # natural cut: pre 25 >= lim 20
    op(1, 20, 0x51, 12),                  # same line again
    op(0, 20, 0x21),                      # procedure 1
    op(0, 20, 0x51, 30),
    op(3, 20, 0x52),
    "AG site=90426 op=51 cur=1",          # a cut with pre < lim: another source
    op(0, 20, 0x51, 31),
    "AG site=7 op=52 cur=1",              # not a Uloc cut
    op(1, 20, 0x52),
    op(1, 20, 0x51, 32),
]) + "\n"

SRC = """int a;
int first(int x)
{
    return x;   /* { not a brace } */
}

static int second(void) {
    return 0;
}
"""


class ParseTests(unittest.TestCase):
    def setUp(self):
        self.rows = {(r["proc"], r["line"]): r for r in cut_log.parse(LOG)}

    def test_one_row_per_line_and_procedure(self):
        self.assertEqual(sorted(self.rows), [(0, 10), (0, 11), (0, 12), (1, 30), (1, 31), (1, 32)])
        self.assertEqual(self.rows[(0, 12)]["n"], 2)

    def test_counter_and_pre(self):
        r = self.rows[(0, 12)]
        self.assertEqual((r["counter"], r["pre"], r["lim"]), (0, 25, 20))
        self.assertTrue(r["natural"])
        self.assertEqual((self.rows[(0, 11)]["counter"], self.rows[(0, 11)]["pre"]), (2, 1))

    def test_cut_comes_from_an_ag_between_records(self):
        self.assertTrue(self.rows[(0, 12)]["cut"])
        self.assertEqual(self.rows[(0, 12)]["site"], 125154)
        self.assertFalse(self.rows[(0, 11)]["cut"])
        self.assertTrue(self.rows[(1, 30 + 0)]["cut"] is False)
        r = self.rows[(1, 31)]
        self.assertTrue(r["cut"] and not r["natural"] and r["site"] == 90426)

    def test_non_uloc_ag_is_not_a_cut(self):
        self.assertFalse(self.rows[(1, 32)]["cut"])

    def test_log_without_lines_is_reported(self):
        with self.assertRaises(SystemExit):
            cut_log.main(["/nonexistent-log"])


class AnnotateTests(unittest.TestCase):
    def test_function_spans_ignore_comment_braces(self):
        self.assertEqual(cut_log.function_spans(SRC), [("first", 3, 5), ("second", 7, 9)])

    def test_names_and_forced(self):
        spans = [("first", 1, 20), ("second", 21, 40)]
        rows = cut_log.annotate(cut_log.parse(LOG), spans, (11, 31), {31})
        by = {(r["procedure"], r["line"]): r for r in rows}
        self.assertEqual(by[("first", 10)]["forced"], "-")
        self.assertEqual(by[("first", 12)]["forced"], "inert")       # natural cut, in range
        self.assertEqual(by[("second", 31)]["forced"], "yes")        # cut despite pre < lim
        self.assertEqual(by[("second", 31)]["forced_to"], "cut")
        self.assertEqual(by[("first", 11)]["forced_to"], "keep")

    def test_unnamed_procedure_is_an_ordinal(self):
        rows = cut_log.annotate(cut_log.parse(LOG), None, None, set())
        self.assertEqual({r["procedure"] for r in rows}, {"p0", "p1"})

    def test_range_parse(self):
        self.assertEqual(cut_log.parse_range("5-9"), (5, 9))
        self.assertIsNone(cut_log.parse_range(None))
        with self.assertRaises(SystemExit):
            cut_log.parse_range("x")
        self.assertEqual(cut_log.parse_lines("3, 4,5"), {3, 4, 5})


if __name__ == "__main__":
    unittest.main()
