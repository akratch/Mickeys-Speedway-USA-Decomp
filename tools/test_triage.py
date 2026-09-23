#!/usr/bin/env python3
"""Unit tests for the wave-scoping arithmetic."""
import pathlib
import sys
import unittest
import unittest.mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import triage  # noqa: E402


def fn(name, size, words, overlay=None):
    return {"name": name, "size_bytes": size, "overlay": overlay,
            "relocation_masked_differing_words": words}


class ClusterTests(unittest.TestCase):
    def test_identical_size_in_one_overlay_is_a_cluster(self) -> None:
        rows = [fn("a", 2100, 149, 101), fn("b", 2100, 149, 101),
                fn("c", 900, 12, 101)]
        out = triage.clusters(rows)
        self.assertEqual(len(out), 1)
        self.assertEqual(out[0]["count"], 2)
        self.assertEqual(out[0]["total_bytes"], 4200)

    def test_same_size_in_different_overlays_is_not_a_cluster(self) -> None:
        """Size alone is a coincidence; it is size within one overlay that has
        meant 'the same routine specialised' in this codebase."""
        rows = [fn("a", 2100, 149, 101), fn("b", 2100, 149, 58)]
        self.assertEqual(triage.clusters(rows), [])

    def test_lead_is_the_cheapest_member_and_spread_is_reported(self) -> None:
        rows = [fn("a", 800, 145, 101), fn("b", 800, 145, 101),
                fn("c", 800, 163, 101)]
        out = triage.clusters(rows)[0]
        self.assertEqual(out["lead_words"], 145)
        self.assertEqual(out["spread"], 18)
        self.assertEqual(out["words"], [145, 145, 163])

    def test_clusters_sort_by_total_bytes(self) -> None:
        rows = [fn("a", 400, 5, 1), fn("b", 400, 5, 1),
                fn("c", 2000, 5, 2), fn("d", 2000, 5, 2)]
        self.assertEqual([c["size_bytes"] for c in triage.clusters(rows)], [2000, 400])


class RouteTests(unittest.TestCase):
    def test_the_route_prefers_bytes_per_word_not_raw_word_count(self) -> None:
        """A big function with a moderate residual is a better buy than a small
        one with a small residual -- that is the whole point of the ordering."""
        big = fn("big", 2100, 149)      # 0.07 words per byte
        small = fn("small", 200, 40)    # 0.20 words per byte
        out = triage.cheapest_route([small, big], 2000)
        self.assertEqual(out["names"][0], "big")

    def test_the_route_stops_once_the_gap_is_covered(self) -> None:
        rows = [fn("a", 1000, 10), fn("b", 1000, 10), fn("c", 1000, 10)]
        out = triage.cheapest_route(rows, 1500)
        self.assertEqual(out["functions"], 2)
        self.assertEqual(out["bytes"], 2000)

    def test_a_zero_gap_still_returns_one_function_not_a_crash(self) -> None:
        out = triage.cheapest_route([fn("a", 100, 1)], 0)
        self.assertEqual(out["functions"], 1)

    def test_words_are_summed_over_the_picked_set(self) -> None:
        rows = [fn("a", 1000, 10), fn("b", 1000, 25)]
        out = triage.cheapest_route(rows, 2000)
        self.assertEqual(out["words"], 35)




class UnassignableTests(unittest.TestCase):
    """A symbol with a proof that it cannot match must never reach a lane.

    The bar is machine-enforced rather than remembered, because the symbols
    that earn it are large and nearly closed -- exactly what a ratio-sorted
    list puts on top -- so every wave surfaces them again.
    """

    def _fixture(self, ranking_rows, barred):
        import json
        import tempfile
        tmp = pathlib.Path(tempfile.mkdtemp())
        rank = tmp / "ranking.json"
        rank.write_text(json.dumps({"functions": ranking_rows}), encoding="utf-8")
        bar = tmp / "barred.json"
        bar.write_text(json.dumps({"schema_version": 1, "symbols": barred}),
                       encoding="utf-8")
        return rank, bar

    def test_a_barred_symbol_is_dropped_from_the_queue(self) -> None:
        rank, bar = self._fixture(
            [fn("keep", 100, 4, 1), fn("barred", 1416, 5, 57)],
            {"barred": {"reason": "proven unmatchable at a floor of 2"}})
        with unittest.mock.patch.object(triage, "RANKING", rank), \
                unittest.mock.patch.object(triage, "UNASSIGNABLE", bar):
            self.assertEqual([r["name"] for r in triage.load()], ["keep"])

    def test_a_barred_symbol_never_appears_in_the_cheapest_route(self) -> None:
        """It would sort first: 1416 bytes at 5 words is the best ratio here."""
        rank, bar = self._fixture(
            [fn("keep", 100, 4, 1), fn("barred", 1416, 5, 57)],
            {"barred": {"reason": "proven unmatchable at a floor of 2"}})
        with unittest.mock.patch.object(triage, "RANKING", rank), \
                unittest.mock.patch.object(triage, "UNASSIGNABLE", bar):
            route = triage.cheapest_route(triage.load(), 10_000)
            self.assertNotIn("barred", route["names"])

    def test_the_exclusion_is_reported_rather_than_silent(self) -> None:
        """A silently shorter list reads as the queue having shrunk. The
        report has to say a symbol was removed and why, or the next reader
        re-adds it."""
        rank, bar = self._fixture(
            [fn("keep", 100, 4, 1), fn("barred", 1416, 5, 57)],
            {"barred": {"reason": "proven unmatchable at a floor of 2"}})
        with unittest.mock.patch.object(triage, "RANKING", rank), \
                unittest.mock.patch.object(triage, "UNASSIGNABLE", bar):
            rendered = triage.render(triage.report(60.0, 5))
        self.assertIn("barred", rendered)
        self.assertIn("proven unmatchable at a floor of 2", rendered)
        self.assertIn("Never assign it", rendered)

    def test_a_barred_symbol_absent_from_the_ranking_is_not_reported(self) -> None:
        """Once a symbol leaves the queue the bar is still correct, but
        printing it every wave trains the reader to skip the line."""
        rank, bar = self._fixture(
            [fn("keep", 100, 4, 1)],
            {"gone": {"reason": "proven unmatchable"}})
        with unittest.mock.patch.object(triage, "RANKING", rank), \
                unittest.mock.patch.object(triage, "UNASSIGNABLE", bar):
            self.assertEqual(triage.report(60.0, 5)["excluded"], [])

    def test_a_missing_registry_bars_nothing(self) -> None:
        rank, bar = self._fixture([fn("keep", 100, 4, 1)], {})
        with unittest.mock.patch.object(triage, "RANKING", rank), \
                unittest.mock.patch.object(triage, "UNASSIGNABLE", bar / "absent"):
            self.assertEqual([r["name"] for r in triage.load()], ["keep"])


class ShippedRegistryTests(unittest.TestCase):
    def test_the_proven_unmatchable_overlay_57_symbol_is_barred(self) -> None:
        """A standing project constraint. It has been re-surfaced by a
        ratio-sorted list in more than one wave."""
        self.assertIn("overlay57UpdateModeState", triage.unassignable())

    def test_every_barred_symbol_carries_its_proof(self) -> None:
        """'Do not assign' without the evidence is indistinguishable from a
        plateau someone gave up on, and gets overturned."""
        for name, entry in triage.unassignable().items():
            with self.subTest(name):
                self.assertTrue(entry.get("reason", "").strip(), name)
                self.assertGreater(len(entry.get("evidence", "")), 80, name)


class AssignabilityFilterTests(unittest.TestCase):
    """Only `base-only` targets may be dispatched, and the ranking does not
    say which those are.

    Measured once: 338 queued functions, 68 assignable. A wave went out at
    nine targets of which three were assignable; the lane correctly refused
    the rest and the slot was wasted. The gap to the goal was larger than the
    entire assignable pool, so the plan was impossible for a reason no number
    in the ranking showed.
    """

    def _rows(self):
        return [fn("open", 1000, 10, 1), fn("stale", 5000, 5, 2)]

    def _patched(self, states):
        return (
            unittest.mock.patch.object(triage, "load", self._rows),
            unittest.mock.patch.object(triage, "assignability",
                                       return_value=states),
            unittest.mock.patch.object(triage, "resolved_bytes", lambda: 0),
            unittest.mock.patch.object(triage, "unassignable", dict),
        )

    def _report(self, states):
        import contextlib
        with contextlib.ExitStack() as stack:
            for p in self._patched(states):
                stack.enter_context(p)
            r = triage.report(60.0, 5)
            return r, triage.render(r)

    def test_a_non_assignable_target_leaves_the_route(self) -> None:
        """It would sort first: 5000 bytes at 5 words is the best ratio here."""
        r, _ = self._report({"open": "base-only",
                             "stale": "already-integrated/exhausted"})
        self.assertNotIn("stale", r["route"]["names"])
        self.assertIn("open", r["route"]["names"])

    def test_the_blocked_bytes_are_reported_by_state(self) -> None:
        """A silently shorter queue reads as progress. The tool must say how
        much was withheld and under which state, because the remedies differ:
        a stale pin gets repinned, an active lane gets waited on."""
        _, rendered = self._report({"open": "base-only",
                                    "stale": "already-integrated/exhausted"})
        self.assertIn("NOT ASSIGNABLE", rendered)
        self.assertIn("5,000", rendered)
        self.assertIn("already-integrated/exhausted", rendered)
        self.assertIn("authorize_reopen", rendered)

    def test_an_unavailable_classifier_says_so_rather_than_lying(self) -> None:
        """Degraded triage is fine; reporting unfiltered figures as though
        they were assignable is not."""
        r, rendered = self._report({})
        self.assertIsNone(r["blocked"])
        self.assertIn("lane_status unavailable", rendered)
        self.assertIn("stale", r["route"]["names"])

    def test_an_unknown_symbol_is_treated_as_assignable(self) -> None:
        """A symbol missing from the classifier's answer must not be dropped
        silently; the fail-closed direction belongs in lane_status, not here."""
        r, _ = self._report({"open": "base-only"})
        self.assertIn("stale", r["route"]["names"])

    def test_assignability_returns_empty_when_lane_status_raises(self) -> None:
        import lane_status
        with unittest.mock.patch.object(
                lane_status.AssignmentContext, "build",
                side_effect=RuntimeError("no base")):
            self.assertEqual(triage.assignability(["a"]), {})


class DeltaGroupTests(unittest.TestCase):
    """Size-mismatch work is Track B, and the report has to say how much.

    Delta-0 methods (colour, L160, web laws) cannot emit or delete an
    instruction. A route that silently mixes 4-byte-off functions with
    delta-0 ones reads as colour work and gets dispatched as colour work.
    """

    def rows(self):
        return [dict(fn("zero", 1000, 10), size_delta=0),
                dict(fn("small", 2000, 20), size_delta=-8),
                dict(fn("edge", 500, 5), size_delta=12),
                dict(fn("big", 3000, 300), size_delta=232)]

    def test_groups_by_absolute_delta(self):
        self.assertEqual(
            [triage.delta_group(r["size_delta"]) for r in self.rows()],
            ["delta-0", "small-delta", "small-delta", "big-delta"])
        self.assertEqual(triage.delta_group(None), "delta-0")

    def test_group_totals_carry_counts_bytes_and_words(self):
        out = triage.group_totals(self.rows())
        self.assertEqual(out["delta-0"], {"functions": 1, "bytes": 1000, "words": 10})
        self.assertEqual(out["small-delta"], {"functions": 2, "bytes": 2500, "words": 25})
        self.assertEqual(out["big-delta"], {"functions": 1, "bytes": 3000, "words": 300})

    def test_every_group_is_present_even_when_empty(self):
        self.assertEqual(set(triage.group_totals([])), set(triage.DELTA_GROUPS))

    def _report(self, target=65.0):
        import contextlib
        with contextlib.ExitStack() as stack:
            stack.enter_context(unittest.mock.patch.object(triage, "load", self.rows))
            stack.enter_context(unittest.mock.patch.object(
                triage, "assignability", return_value={}))
            stack.enter_context(unittest.mock.patch.object(
                triage, "resolved_bytes", lambda: 0))
            stack.enter_context(unittest.mock.patch.object(triage, "unassignable", dict))
            r = triage.report(target, 5)
            return r, triage.render(r)

    def test_route_clusters_and_bands_are_split(self):
        r, rendered = self._report()
        self.assertEqual(sum(g["functions"] for g in r["route"]["by_group"].values()),
                         r["route"]["functions"])
        self.assertEqual(set(r["route_by_group"]), set(triage.DELTA_GROUPS))
        self.assertEqual(set(r["clusters"]["by_group"]), set(triage.DELTA_GROUPS))
        for band in r["bands"]:
            self.assertEqual(
                sum(g["bytes"] for g in band["by_group"].values()), band["bytes"])
        for label in ("small-delta", "big-delta", "each group alone",
                      "queue by delta group"):
            self.assertIn(label, rendered)

    def test_a_group_that_cannot_cover_the_gap_says_so(self):
        r, rendered = self._report()
        self.assertFalse(r["route_by_group"]["delta-0"]["covers_gap"])
        self.assertIn("cannot cover the gap", rendered)


class MilestoneTests(unittest.TestCase):
    def test_default_target_is_65(self):
        with unittest.mock.patch.object(triage, "report") as report, \
                unittest.mock.patch.object(triage, "render", return_value=""):
            triage.main([])
        self.assertEqual(report.call_args.args[0], 65.0)

    def test_next_milestone_is_the_next_multiple_of_five_above(self):
        whole = triage.WHOLE_PROGRAM
        at_61 = int(whole * 0.6139)
        nm = triage.next_milestone(at_61)
        self.assertEqual(nm["pct"], 65.0)
        self.assertEqual(nm["gap_bytes"], int(whole * 0.65) - at_61)

    def test_exactly_on_a_milestone_points_at_the_next(self):
        nm = triage.next_milestone(int(triage.WHOLE_PROGRAM * 0.70) + 1)
        self.assertEqual(nm["pct"], 75.0)

    def test_the_report_prints_it(self):
        with unittest.mock.patch.object(triage, "load", lambda: [fn("a", 100, 1)]), \
                unittest.mock.patch.object(triage, "assignability", return_value={}), \
                unittest.mock.patch.object(triage, "resolved_bytes",
                                           lambda: int(triage.WHOLE_PROGRAM * 0.62)), \
                unittest.mock.patch.object(triage, "unassignable", dict):
            rendered = triage.render(triage.report(60.0, 5))
        self.assertIn("NEXT 5%  65%", rendered)

if __name__ == "__main__":
    unittest.main()
