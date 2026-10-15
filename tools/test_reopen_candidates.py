#!/usr/bin/env python3
"""The re-open lister must not overstate what a candidate row means."""
import json
import unittest
from unittest import mock

import reopen_candidates as rc


class ConstructMatchTests(unittest.TestCase):
    def law(self, name, text):
        return bool(rc.LAW_CONSTRUCTS[name].search(text))

    def test_each_law_recognises_its_own_construct(self):
        cases = {
            "L90 induction exit test": "the loop counter is compared against",
            "L92 operand weight": "commuting the operands was byte-identical",
            "L93 loop shape": "rewritten as a do/while and measured",
            "L94 co-region reads": "the extern is materialised twice",
        }
        for name, text in cases.items():
            with self.subTest(law=name):
                self.assertTrue(self.law(name, text))

    def test_unrelated_prose_matches_nothing(self):
        text = "The frame is exact and every relocation identity agrees."
        for name in rc.LAW_CONSTRUCTS:
            with self.subTest(law=name):
                self.assertFalse(self.law(name, text))


class LoopApplicabilityTests(unittest.TestCase):
    """A loop law's tag must not survive on a function with no loop.

    A lane spent probes proving L90 could not apply to a function that has
    neither a loop nor an induction variable. Its criticism was exact: the tag
    was a date comparison, not an applicability test.
    """

    def body(self, text):
        import tempfile, pathlib as pl
        d = tempfile.mkdtemp()
        f = pl.Path(d) / "t.c"
        f.write_text(text)
        with mock.patch.object(rc, "ROOT", pl.Path(d)):
            return rc.body_has_loop("t.c", "target")

    def test_a_body_with_no_loop_reads_false(self):
        self.assertIs(self.body("void target(int a) {\n    sink(a);\n}\n"), False)

    def test_a_for_loop_reads_true(self):
        self.assertIs(self.body("void target(void) {\n    for (;;) sink(1);\n}\n"), True)

    def test_a_while_loop_reads_true(self):
        self.assertIs(self.body("void target(void) {\n    while (x) sink(1);\n}\n"), True)

    def test_a_loop_in_a_NEIGHBOURING_function_does_not_count(self):
        """Brace matching must stop at the target's own closing brace."""
        text = ("void target(int a) {\n    sink(a);\n}\n"
                "void other(void) {\n    for (;;) sink(2);\n}\n")
        self.assertIs(self.body(text), False)

    def test_an_unreadable_body_is_None_not_False(self):
        """Unreadable is not evidence a law fails to apply; do not drop it."""
        self.assertIsNone(self.body("void something_else(void) {}\n"))


class SelectionTests(unittest.TestCase):
    """Only still-queued functions with pre-law closures may appear."""

    def run_with(self, queued, handoffs, dates, since="2026-09-10"):
        class FakePath:
            def __init__(self, stem, text): self.stem, self._t = stem, text
            def read_text(self, **kw): return self._t
            # candidates() sorts its paths, as real Paths sort by name.
            def __lt__(self, other): return self.stem < other.stem
        paths = [FakePath(s, t) for s, t in handoffs.items()]

        class FakeDir:
            def glob(self, pattern):
                import fnmatch
                return [p for p in paths if fnmatch.fnmatch(p.stem + ".md", pattern)]

        with mock.patch.object(rc, "queued", return_value=queued), \
             mock.patch.object(rc, "closure_date", side_effect=lambda p: dates[p.stem]), \
             mock.patch.object(rc, "HANDOFFS", FakeDir()):
            return rc.candidates(since)

    ROW = {"size_bytes": 400, "relocation_masked_differing_words": 4, "size_delta": 0}

    def test_a_matched_function_never_appears(self):
        """It has left the queue; there is nothing to re-open."""
        rows = self.run_with({}, {"func_gone": "the loop counter"},
                             {"func_gone": "2026-09-01"})
        self.assertEqual(rows, [])

    def test_a_closure_written_after_the_laws_is_excluded(self):
        rows = self.run_with({"func_x": dict(self.ROW, name="func_x")},
                             {"func_x": "the loop counter"},
                             {"func_x": "2026-09-11"})
        self.assertEqual(rows, [], "a law-aware closure is not a candidate")

    def test_a_closure_touching_no_named_construct_is_excluded(self):
        rows = self.run_with({"func_x": dict(self.ROW, name="func_x")},
                             {"func_x": "the frame and relocations agree"},
                             {"func_x": "2026-09-01"})
        self.assertEqual(rows, [])

    def test_a_pre_law_closure_touching_a_construct_is_listed(self):
        rows = self.run_with({"func_x": dict(self.ROW, name="func_x")},
                             {"func_x": "the loop counter is compared"},
                             {"func_x": "2026-09-01"})
        self.assertEqual([r["symbol"] for r in rows], ["func_x"])
        self.assertIn("L90 induction exit test", rows[0]["laws"])

    def test_rows_are_ranked_by_bytes_per_remaining_word(self):
        queued = {
            "cheap": {"name": "cheap", "size_bytes": 1000,
                      "relocation_masked_differing_words": 2, "size_delta": 0},
            "dear":  {"name": "dear", "size_bytes": 100,
                      "relocation_masked_differing_words": 50, "size_delta": 0},
        }
        text = "the loop counter"
        rows = self.run_with(queued, {"dear": text, "cheap": text},
                             {"dear": "2026-09-01", "cheap": "2026-09-01"})
        self.assertEqual([r["symbol"] for r in rows], ["cheap", "dear"])

    def test_an_unmeasured_row_does_not_crash_the_ranking(self):
        queued = {"func_x": {"name": "func_x", "size_bytes": 400,
                             "relocation_masked_differing_words": None,
                             "size_delta": 0}}
        rows = self.run_with(queued, {"func_x": "the loop counter"},
                             {"func_x": "2026-09-01"})
        self.assertEqual(rows[0]["bytes_per_word"], None)


class NewLawConstructTests(unittest.TestCase):
    """The 2026-09-11/12 laws must recognise the prose a closure actually uses."""

    def law(self, name, text):
        return bool(rc.LAW_CONSTRUCTS[name].search(text))

    def test_each_new_law_recognises_its_own_construct(self):
        cases = {
            "L140 unverified force sweep":
                "a 522-compile force sweep never beats 25",
            "L142 call arity offer set":
                "the p1cost list starts at colour 5 so a1 is not offered",
            "L144 reload without volatile":
                "eight volatile qualifications were measured flat",
            "L139 totalsave against bestcost":
                "the web carries totalsave 71 against bestcost 20",
            "L131 range per IR name":
                "the live range spans both calls",
        }
        for name, text in cases.items():
            with self.subTest(law=name):
                self.assertTrue(self.law(name, text))

    def test_a_force_sweep_closure_is_tagged_however_it_is_phrased(self):
        """L140 is the widest tag on the list, because Trap 20 retracts a
        CLASS of evidence rather than adding a lever: CDX_FORCE is ignored
        without CDX_PROC, so any sweep that did not record acceptance is
        unproven rather than negative."""
        for text in ("CDX_FORCE=p1:w35=c28 was applied",
                     "forcing web 104 to every colour",
                     "150 force cells, none better",
                     "we forced each web in turn"):
            with self.subTest(text=text):
                self.assertTrue(self.law("L140 unverified force sweep", text))

    def test_the_new_laws_do_not_fire_on_unrelated_prose(self):
        text = "The frame is exact and every relocation identity agrees."
        for name in ("L140 unverified force sweep", "L142 call arity offer set",
                     "L144 reload without volatile",
                     "L139 totalsave against bestcost",
                     "L131 range per IR name"):
            with self.subTest(law=name):
                self.assertFalse(self.law(name, text))


class BarredSymbolTests(unittest.TestCase):
    """A proven-unmatchable symbol must never appear as a re-open candidate.

    Candidates are ranked by how closed-yet-cheap they look, which is exactly
    the shape a proven-unmatchable function has: large, nearly exact, and so
    top of any list. One sat first in this tool's output while carrying a
    proof that no legal source reaches it.
    """

    def test_a_barred_symbol_is_dropped_from_the_queue(self):
        rows = {"functions": [{"name": "keep", "size_bytes": 100},
                              {"name": "barred", "size_bytes": 1416}]}
        with mock.patch.object(rc, "barred",
                               return_value={"barred": {"reason": "proved"}}), \
             mock.patch.object(type(rc.RANKING), "read_text",
                               return_value=json.dumps(rows)):
            self.assertEqual(sorted(rc.queued()), ["keep"])

    def test_no_bar_file_bars_nothing(self):
        missing = rc.UNASSIGNABLE.parent / "does-not-exist.json"
        with mock.patch.object(rc, "UNASSIGNABLE", missing):
            self.assertEqual(rc.barred(), {})

    def test_the_shipped_bar_covers_the_proven_unmatchable_symbol(self):
        """A standing project constraint, and this tool surfaced it once."""
        self.assertIn("overlay57UpdateModeState", rc.barred())

    def test_the_shipped_queue_excludes_it(self):
        self.assertNotIn("overlay57UpdateModeState", rc.queued())

if __name__ == "__main__":
    unittest.main()
