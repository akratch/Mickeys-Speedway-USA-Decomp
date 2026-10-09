#!/usr/bin/env python3
"""Tests for the per-source-line ugen draw census.

Nothing here compiles. The fixtures are written in the grammar the instrumented
ugen emits, so what is pinned is the reading, which is where a wrong census
would mislead a lane:

  1. a DRAW is an ALLOC_*_RESULT row and nothing else -- the ALLOC_GP request
     that precedes each result would double every count;
  2. another procedure's rows never leak in, because a TU compiles several and
     the ordinal is Ucode order rather than source order;
  3. an edit that moves no line is reported as moving no line, because that is
     the finding -- a score can move for reasons the edit did not cause;
  4. draw ORDER is compared as a sequence, since the free list is a FIFO and
     the same multiset drawn in a different order is a different schedule.
"""
import json
import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import draw_census as dc  # noqa: E402


def fl_row(op, reg, line, proc=0, emitted=0):
    return (f"DKWB-FREELIST {op} proc={proc} reg={reg} emitted={emitted} "
            f"line={line}")


def emit_row(line, emit, proc=0, block=0, op=28):
    return (f"DKWB-EMIT-V1 proc={proc} block={block} emit={emit} op={op} "
            f"line={line} buffer=fwd fn=f_emit_dir2")


class TraceParsingTests(unittest.TestCase):
    def test_only_the_result_row_counts_as_a_draw(self):
        text = "\n".join([fl_row("ALLOC_GP", 14, 133),
                          fl_row("ALLOC_GP_RESULT", 14, 133)])
        parsed = dc.parse_trace(text, 0)
        self.assertEqual(len(parsed["draws"][133]), 1)

    def test_floating_point_results_are_draws_too(self):
        parsed = dc.parse_trace(fl_row("ALLOC_FP_RESULT", 4, 20), 0)
        self.assertEqual(len(parsed["draws"][20]), 1)

    def test_frees_are_counted_separately_from_draws(self):
        text = "\n".join([fl_row("FREE", 14, 133), fl_row("FREE", 15, 133),
                          fl_row("ALLOC_GP_RESULT", 14, 133)])
        parsed = dc.parse_trace(text, 0)
        self.assertEqual(parsed["frees"][133], 2)
        self.assertEqual(len(parsed["draws"][133]), 1)

    def test_another_procedures_rows_are_not_mixed_in(self):
        text = "\n".join([fl_row("ALLOC_GP_RESULT", 14, 133, proc=0),
                          fl_row("ALLOC_GP_RESULT", 15, 140, proc=1)])
        self.assertEqual(len(dc.parse_trace(text, 0)["order"]), 1)
        self.assertEqual(len(dc.parse_trace(text, 1)["order"]), 1)
        self.assertEqual(dc.parse_trace(text, 2)["order"], [])

    def test_emissions_are_collected_per_line(self):
        text = "\n".join([emit_row(125, 1), emit_row(125, 2), emit_row(126, 3)])
        parsed = dc.parse_trace(text, 0)
        self.assertEqual(len(parsed["emits"][125]), 2)
        self.assertEqual(len(parsed["emits"][126]), 1)

    def test_the_draw_order_is_kept_as_a_sequence(self):
        text = "\n".join([fl_row("ALLOC_GP_RESULT", r, 10) for r in (14, 15, 8)])
        self.assertEqual([r["reg"] for r in dc.parse_trace(text, 0)["order"]],
                         ["14", "15", "8"])

    def test_unrelated_trace_rows_are_ignored(self):
        text = "DKWB-CALL 2 > f_get_arg\nDKWB-PROC BEGIN proc=0"
        parsed = dc.parse_trace(text, 0)
        self.assertEqual(parsed["order"], [])
        self.assertEqual(dict(parsed["frees"]), {})


class ProcResolutionTests(unittest.TestCase):
    FUNCS = [("a", 0), ("b", 0x40), ("c", 0x90)]

    def test_symbol_maps_to_its_address_ordinal(self):
        self.assertEqual(dc.ordinal_from_symbols("c", self.FUNCS, 3), 2)
        self.assertEqual(dc.ordinal_from_symbols("a", self.FUNCS, 3), 0)

    def test_aliases_share_an_ordinal(self):
        funcs = self.FUNCS + [("b_alias", 0x40)]
        self.assertEqual(dc.ordinal_from_symbols("b_alias", funcs, 3), 1)

    def test_count_mismatch_is_refused(self):
        with self.assertRaises(SystemExit) as cm:
            dc.ordinal_from_symbols("b", self.FUNCS, 2)
        self.assertIn("--proc", str(cm.exception))

    def test_unknown_symbol_is_refused(self):
        with self.assertRaises(SystemExit):
            dc.ordinal_from_symbols("zzz", self.FUNCS, 3)


class ComparisonTests(unittest.TestCase):
    def profile(self, symbol="f", draws=None, emits=None, order=None):
        draws = draws or {}
        emits = emits or {}
        return {"symbol": symbol, "proc": 0,
                "draws_by_line": {str(k): v for k, v in draws.items()},
                "frees_by_line": {},
                "emits_by_line": {str(k): v for k, v in emits.items()},
                "draw_order": order if order is not None else [],
                "total_draws": sum(draws.values()),
                "total_emits": sum(emits.values())}

    def test_an_edit_that_moves_nothing_says_so(self):
        p = self.profile(draws={133: 1}, emits={133: 9}, order=["14"])
        text = dc.compare(p, p)
        self.assertIn("no line moved", text)
        self.assertIn("draw order unchanged", text)

    def test_a_line_that_gained_a_draw_is_named(self):
        before = self.profile(draws={133: 1}, order=["14"])
        after = self.profile(draws={133: 2}, order=["14", "15"])
        text = dc.compare(before, after)
        self.assertIn("133", text)
        self.assertIn("+1", text)
        self.assertNotIn("no line moved", text)

    def test_the_same_registers_in_a_different_order_is_a_change(self):
        # The free list is a FIFO; the same multiset drawn differently is a
        # different schedule.
        before = self.profile(draws={10: 2}, order=["14", "15"])
        after = self.profile(draws={10: 2}, order=["15", "14"])
        self.assertIn("draw order CHANGED", dc.compare(before, after))

    def test_emission_movement_is_reported_alongside_draws(self):
        before = self.profile(draws={12: 1}, emits={12: 3}, order=["14"])
        after = self.profile(draws={12: 1}, emits={12: 5}, order=["14"])
        text = dc.compare(before, after)
        self.assertIn("+2", text)
        self.assertNotIn("no line moved", text)

    def test_comparing_two_different_functions_is_refused(self):
        a, b = self.profile("f"), self.profile("g")
        # exercised through main() so the refusal reaches the operator
        import tempfile
        d = pathlib.Path(tempfile.mkdtemp())
        (d / "a.json").write_text(json.dumps(a))
        (d / "b.json").write_text(json.dumps(b))
        with self.assertRaises(SystemExit):
            dc.main(["--compare", str(d / "a.json"), str(d / "b.json")])


class RenderTests(unittest.TestCase):
    def test_every_line_with_any_activity_appears(self):
        data = {"symbol": "f", "proc": 0, "draws_by_line": {"133": 1},
                "frees_by_line": {"136": 4}, "emits_by_line": {"140": 2},
                "draw_order": ["14"], "total_draws": 1, "total_emits": 2}
        text = dc.render(data)
        for line in ("133", "136", "140"):
            self.assertIn(line, text)

    def test_the_draw_order_is_printed(self):
        data = {"symbol": "f", "proc": 0, "draws_by_line": {}, "frees_by_line": {},
                "emits_by_line": {}, "draw_order": ["14", "15"],
                "total_draws": 2, "total_emits": 0}
        self.assertIn("14 15", dc.render(data))



class SourceTests(unittest.TestCase):
    def test_with_source_replaces_only_the_last_word(self):
        cmd = ["cc", "-O2", "-o", "x.o", "src/a.c"]
        self.assertEqual(dc.with_source(cmd, pathlib.Path("/c/cand.c")),
                         ["cc", "-O2", "-o", "x.o", "/c/cand.c"])
        self.assertEqual(dc.with_source(cmd, None), cmd)
        self.assertEqual(cmd[-1], "src/a.c")

    def test_missing_source_is_refused_by_absolute_path(self):
        with self.assertRaises(SystemExit) as caught:
            dc.main(["sym", "--source", "nope/cand.c"])
        self.assertIn(str(pathlib.Path("nope/cand.c").resolve()), str(caught.exception))


if __name__ == "__main__":
    unittest.main()
