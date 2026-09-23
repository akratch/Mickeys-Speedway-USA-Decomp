#!/usr/bin/env python3
"""Every case here replays waste that actually happened while planning."""
import unittest
from unittest import mock

import dispatch_check as dc

ROW = {"size_bytes": 400, "differing_words": 12,
       "relocation_masked_differing_words": 10, "size_delta": 0, "category": "other"}


def rows(*names):
    return {n: dict(ROW, name=n) for n in names}


class PlanParsing(unittest.TestCase):
    def test_lanes_and_symbol_lists_parse(self):
        plan = dc.parse_plan(["a=x,y", "b=z"])
        self.assertEqual(plan, {"a": ["x", "y"], "b": ["z"]})

    def test_a_repeated_lane_accumulates_rather_than_overwrites(self):
        self.assertEqual(dc.parse_plan(["a=x", "a=y"]), {"a": ["x", "y"]})

    def test_a_malformed_assignment_is_rejected(self):
        with self.assertRaises(ValueError):
            dc.parse_plan(["no-equals-sign"])


class Refusals(unittest.TestCase):
    def run_check(self, plan, queued):
        # assignability is stubbed out: these cases test overlap and queue
        # membership, and letting the real classifier run would consult
        # lane_status about fixture symbols that do not exist in the tree.
        with mock.patch.object(dc, "queued_rows", return_value=queued), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability", return_value={}):
            return dc.check(plan)

    def test_a_symbol_in_two_lanes_is_refused(self):
        """Caught by hand twice while planning campaign 2."""
        problems, _ = self.run_check({"a": ["dup"], "b": ["dup"]}, rows("dup"))
        self.assertTrue(any("two lanes" in p or "2 lanes" in p for p in problems))

    def test_the_refusal_names_both_lanes(self):
        problems, _ = self.run_check({"alpha": ["dup"], "beta": ["dup"]}, rows("dup"))
        joined = " ".join(problems)
        self.assertIn("alpha", joined)
        self.assertIn("beta", joined)

    def test_an_unqueued_symbol_is_refused(self):
        """A matched function has nothing left to work."""
        problems, _ = self.run_check({"a": ["gone"]}, rows("other"))
        self.assertTrue(any("not in the NON_MATCHING queue" in p for p in problems))

    def test_a_sound_plan_raises_nothing(self):
        problems, _ = self.run_check({"a": ["x"], "b": ["y"]}, rows("x", "y"))
        self.assertEqual(problems, [])

    def test_one_symbol_in_one_lane_twice_is_not_an_overlap(self):
        """Sloppy input, not a planning fault; do not refuse on it."""
        problems, _ = self.run_check({"a": ["x", "x"]}, rows("x"))
        self.assertEqual(problems, [])


class TranslationUnitOwnershipTests(unittest.TestCase):
    """Two lanes editing one TU is the conflict that costs an integration.

    The symbol check does not see it: three overlay 8 functions with distinct
    names live in one `overlay_008.c`, so splitting them across lanes passes
    every other check and then conflicts on every merge.
    """

    def run_check(self, plan, queued):
        with mock.patch.object(dc, "queued_rows", return_value=queued), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability", return_value={}):
            return dc.check(plan)

    def queued(self, **by_symbol):
        return {n: dict(ROW, name=n, file=f) for n, f in by_symbol.items()}

    def test_two_lanes_on_one_file_are_refused(self):
        problems, _ = self.run_check(
            {"a": ["f1"], "b": ["f2"]},
            self.queued(f1="src/overlays/o008/overlay_008.c",
                        f2="src/overlays/o008/overlay_008.c"))
        self.assertTrue(any("overlay_008.c is edited by 2 lanes" in p
                            for p in problems), problems)

    def test_one_lane_owning_the_whole_file_is_fine(self):
        problems, _ = self.run_check(
            {"a": ["f1", "f2"]},
            self.queued(f1="src/overlays/o008/overlay_008.c",
                        f2="src/overlays/o008/overlay_008.c"))
        self.assertEqual([p for p in problems if "edited by" in p], [])

    def test_different_files_across_lanes_are_fine(self):
        problems, _ = self.run_check(
            {"a": ["f1"], "b": ["f2"]},
            self.queued(f1="src/main/a.c", f2="src/main/b.c"))
        self.assertEqual([p for p in problems if "edited by" in p], [])

    def test_the_message_names_which_lane_holds_which_symbol(self):
        problems, _ = self.run_check(
            {"a": ["f1"], "b": ["f2"]},
            self.queued(f1="src/main/shared.c", f2="src/main/shared.c"))
        message = next(p for p in problems if "edited by" in p)
        self.assertIn("a has f1", message)
        self.assertIn("b has f2", message)

    def test_a_row_without_a_file_is_skipped_rather_than_grouped(self):
        # An unfiled row must not collide with every other unfiled row.
        queued = {n: dict(ROW, name=n) for n in ("f1", "f2")}
        problems, _ = self.run_check({"a": ["f1"], "b": ["f2"]}, queued)
        self.assertEqual([p for p in problems if "edited by" in p], [])

    def test_the_legacy_rel_c_file_key_is_still_read(self):
        queued = {"f1": dict(ROW, name="f1", rel_c_file="src/main/x.c"),
                  "f2": dict(ROW, name="f2", rel_c_file="src/main/x.c")}
        problems, _ = self.run_check({"a": ["f1"], "b": ["f2"]}, queued)
        self.assertTrue(any("edited by 2 lanes" in p for p in problems))


class EmittedFacts(unittest.TestCase):
    def run_check(self, plan, queued, closures=None):
        with mock.patch.object(dc, "queued_rows", return_value=queued), \
             mock.patch.object(dc, "closure_facts", return_value=closures or {}), \
             mock.patch.object(dc, "assignability", return_value={}):
            return dc.check(plan)

    def test_the_artifact_count_is_stated_so_no_lane_re_derives_it(self):
        """Two lanes were told to spend a phase computing this number."""
        _, notes = self.run_check({"a": ["x"]}, rows("x"))
        self.assertIn("artifact=2", " ".join(notes))

    def test_masked_and_delta_and_class_are_all_carried(self):
        _, notes = self.run_check({"a": ["x"]}, rows("x"))
        text = " ".join(notes)
        for field in ("masked=10", "delta=0", "class=other"):
            self.assertIn(field, text)

    def test_a_pre_law_closure_is_surfaced_in_the_brief(self):
        closures = {"x": {"symbol": "x", "closed": "2026-09-01",
                          "laws": ["L93 loop shape"]}}
        _, notes = self.run_check({"a": ["x"]}, rows("x"), closures)
        text = " ".join(notes)
        self.assertIn("closure predates", text)
        self.assertIn("L93", text)


class AssignabilityGateTests(unittest.TestCase):
    """Only `base-only` may be dispatched, and the state is invisible here.

    The state also MOVES: merging a lane that edited a handoff shifts that
    symbol's handoff commit and invalidates its reopen pin, so a target that
    was assignable when the plan was made is often shut by the time the plan
    runs. Three dispatched lanes were lost to that window on one function --
    each spun up, reported "blocked by the assignment gate" and did no work.
    """

    def _check(self, plan, states):
        with mock.patch.object(dc, "queued_rows",
                               return_value=rows(*[s for v in plan.values() for s in v])), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability", return_value=states):
            return dc.check(plan)

    def test_a_base_only_symbol_is_sound(self):
        problems, _ = self._check({"a": ["func_A"]}, {"func_A": "base-only"})
        self.assertEqual(problems, [])

    def test_a_stale_pin_is_refused_with_the_remedy_named(self):
        """The commonest case, and the one with a one-command fix."""
        problems, _ = self._check(
            {"a": ["func_A"]}, {"func_A": "reopen-authorization-stale"})
        self.assertEqual(len(problems), 1)
        self.assertIn("not base-only", problems[0])
        self.assertIn("--refresh-stale", problems[0])

    def test_an_active_symbol_is_refused(self):
        """Claimed by an unintegrated lane: dispatching a second one duplicates
        the work and the second merge conflicts."""
        problems, _ = self._check({"a": ["func_A"]}, {"func_A": "active"})
        self.assertEqual(len(problems), 1)
        self.assertIn("active", problems[0])

    def test_an_exhausted_symbol_is_refused(self):
        problems, _ = self._check(
            {"a": ["func_A"]}, {"func_A": "already-integrated/exhausted"})
        self.assertEqual(len(problems), 1)

    def test_the_refusal_says_the_lane_would_do_no_work(self):
        """A refusal that only names a state invites overriding it. Saying what
        it costs is what makes it stick."""
        problems, _ = self._check({"a": ["func_A"]}, {"func_A": "stale-ledger"})
        self.assertIn("do no work", problems[0])

    def test_one_shut_gate_among_open_ones_is_still_refused(self):
        problems, _ = self._check(
            {"a": ["func_A", "func_B"], "b": ["func_C"]},
            {"func_A": "base-only", "func_B": "active", "func_C": "base-only"})
        self.assertEqual(len(problems), 1)
        self.assertIn("func_B", problems[0])

    def test_an_unavailable_classifier_warns_rather_than_refusing(self):
        """Degraded checking still catches duplicates and unqueued symbols. A
        hard refusal here would make the tool unusable whenever lane_status is
        broken, which is when its other checks matter most."""
        problems, notes = self._check({"a": ["func_A"]}, {})
        self.assertEqual(problems, [])
        self.assertIn("assignability unchecked", "\n".join(notes))

    def test_a_symbol_missing_from_the_classifier_is_not_refused(self):
        """Absence is not a verdict; the fail-closed direction belongs in
        lane_status, not in a planning aid."""
        problems, _ = self._check({"a": ["func_A"]}, {"func_B": "base-only"})
        self.assertEqual(problems, [])


class TrackBRouting(unittest.TestCase):
    """A size-mismatch function is not colour work.

    Colour landscapes, L160 and the web laws operate at delta 0; they cannot
    emit or delete an instruction. The previous sprint sent size-mismatch
    targets to colour lanes and they came back flat, so a plan now has to say
    which lanes carry the insertion-pair method (track=B) before one of those
    targets may go to it.
    """

    def queued(self, **deltas):
        return {n: dict(ROW, name=n, size_delta=d) for n, d in deltas.items()}

    def run_check(self, plan, queued, tracks=None):
        with mock.patch.object(dc, "queued_rows", return_value=queued), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability", return_value={}):
            return dc.check(plan, tracks)

    def test_a_size_mismatch_symbol_on_an_unmarked_lane_is_refused(self):
        problems, _ = self.run_check({"a": ["small"]}, self.queued(small=4))
        self.assertEqual(len(problems), 1)
        self.assertIn("size-mismatch-needs-track-b", problems[0])
        self.assertIn("small", problems[0])
        self.assertIn("lane a", problems[0])
        self.assertIn("small-delta", problems[0])

    def test_a_track_b_lane_may_take_it(self):
        problems, _ = self.run_check({"a": ["small", "big"]},
                                     self.queued(small=-8, big=232), {"a": "B"})
        self.assertEqual(problems, [])

    def test_delta_zero_needs_no_track(self):
        problems, _ = self.run_check({"a": ["exact"]}, self.queued(exact=0))
        self.assertEqual(problems, [])

    def test_the_old_two_argument_call_still_works(self):
        with mock.patch.object(dc, "queued_rows", return_value=self.queued(x=0)), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability", return_value={}):
            problems, _ = dc.check({"a": ["x"]})
        self.assertEqual(problems, [])

    def test_the_note_carries_the_delta_group_and_the_track(self):
        _, notes = self.run_check({"a": ["big"]}, self.queued(big=300), {"a": "B"})
        text = "\n".join(notes)
        self.assertIn("=== a (track B) ===", text)
        self.assertIn("delta=300 (big-delta)", text)

    def test_plan_file_forms_mix(self):
        plan, tracks = dc.load_plan({"a": ["x"], "b": {"track": "b", "symbols": ["y"]}})
        self.assertEqual(plan, {"a": ["x"], "b": ["y"]})
        self.assertEqual(tracks, {"b": "B"})
        with self.assertRaises(ValueError):
            dc.load_plan({"a": {"track": "C", "symbols": ["x"]}})

    def test_track_flag_parses(self):
        self.assertEqual(dc.parse_tracks(["lane-1=B", "lane-2=a"]),
                         {"lane-1": "B", "lane-2": "A"})
        with self.assertRaises(ValueError):
            dc.parse_tracks(["lane-1"])

    def test_delta_groups(self):
        self.assertEqual([dc.delta_group(d) for d in (0, 4, -12, 13, -700)],
                         ["delta-0", "small-delta", "small-delta",
                          "big-delta", "big-delta"])



class BaseSelection(unittest.TestCase):
    """A maintenance lane's own commits must be checkable before integration."""

    def test_the_named_base_reaches_the_classifier(self):
        with mock.patch.object(dc, "queued_rows", return_value=rows("x")), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability",
                               return_value={"x": "base-only"}) as seen:
            problems, _ = dc.check({"a": ["x"]}, base="lane/maintenance")
        self.assertEqual(problems, [])
        seen.assert_called_once_with(["x"], "lane/maintenance")

    def test_the_default_base_is_the_integration_branch(self):
        with mock.patch.object(dc, "queued_rows", return_value=rows("x")), \
             mock.patch.object(dc, "closure_facts", return_value={}), \
             mock.patch.object(dc, "assignability", return_value={}) as seen:
            dc.check({"a": ["x"]})
        seen.assert_called_once_with(["x"], "campaign/unchain")


if __name__ == "__main__":
    unittest.main()
