#!/usr/bin/env python3
"""Tests for the per-web footprint map.

Nothing here compiles or reads a ROM. The trace fixtures are written in the
grammar the instrumented compiler emits, and the histograms are synthetic, so
the properties pinned are the ones a wrong map would get wrong:

  1. the probe colour is legal, is not the colour the web already has, and
     prefers a priced colour over an unpriced one -- a declined probe wastes a
     compile and teaches nothing;
  2. a forbidden colour is never probed even when the availability mask
     offers it, because the two masks are separate fields and only their
     difference is legal;
  3. a size-changing cell is excluded rather than mapped, because the
     positional difference this tool takes is only valid while both objects
     place the same words at the same offsets;
  4. the window index inverts the map -- that table is the whole deliverable,
     and a web missing from it is a force nobody will nominate;
  5. a held set of forces is carried into every probe cell and never probed
     itself -- the second-order landscape measures the other webs against
     the held premise, and a cell that dropped or re-coloured a held web
     would be measuring a different premise under the same name.
"""
import pathlib
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import web_footprint as wf  # noqa: E402


def trace(*lines: str) -> str:
    return "\n".join(lines) + "\n"


DETAIL = ("[CDX] webdetail phase=p1 proc=0 role=target web={w} sym={w} type=3 "
          "dtype=6 table=1 chain=0 exprtable=1 exprchain=1 bb={bb} line={bb}")
COLOR = "[CDX] p1color phase=p1 proc=0 web={w} sym={w} color={c} reg={r} forced=-2"
COST = ("[CDX] p1cost phase=p1 proc=0 web={w} color={c} reg=x kind=caller "
        "cost={cost:f} best_before=0.000000")
DEC = ("[CDX] p1dec phase=p1 proc=0 web={w} sym={w} class=1 save=1.0 nocs=1 "
       "totalsave=1.0 bestcost=0.0 bestcolor={c} bestreg=x "
       "forbidden0={fb:#010x} forbidden1=0x00000000 regsleft=1 numintf=1 "
       "available0={av:#010x} available1=0x00000000 allcallersave=0 "
       "taken1=-1 taken2=-1 decision=color forced=-2")


class TraceParsingTests(unittest.TestCase):
    def test_a_web_collects_its_detail_colour_costs_and_kinds(self):
        text = trace(DETAIL.format(w=7, bb=3),
                     COLOR.format(w=7, c=14, r="s0"),
                     COST.format(w=7, c=14, cost=1.0),
                     COST.format(w=7, c=16, cost=2.0),
                     DEC.format(w=7, c=14, fb=0, av=0x1C000))
        webs = wf.parse_trace(text, 0)
        self.assertEqual(webs[7]["color"], 14)
        self.assertEqual(webs[7]["reg"], "s0")
        self.assertEqual(webs[7]["costs"], {14: 1.0, 16: 2.0})
        self.assertEqual(webs[7]["detail"]["bb"], "3")

    def test_another_procedures_records_are_not_mixed_in(self):
        text = trace(COLOR.format(w=7, c=14, r="s0"),
                     COLOR.format(w=7, c=14, r="s0").replace("proc=0", "proc=1"))
        self.assertEqual(list(wf.parse_trace(text, 1)), [7])
        self.assertEqual(wf.parse_trace(text, 2), {})

    def test_only_webs_that_took_a_colour_are_colourable(self):
        text = trace(COLOR.format(w=7, c=14, r="s0"),
                     DEC.format(w=9, c=1, fb=0, av=0xFF))
        self.assertEqual(wf.colourable(wf.parse_trace(text, 0)), [7])


class ProbeColourTests(unittest.TestCase):
    """Legality is the cost table, and the probe stays inside the web's kind.

    Both rules are measured on the overlay 58 trace, not assumed. All 139
    coloured webs list their own final colour in their cost table, and the
    availability mask calls three of the five colours a lane successfully
    forced illegal -- so a mask-based filter drops good probes.
    """

    def entry(self, color, costs, kinds=None, available=0, forbidden=0):
        kinds = kinds or {c: "callee" for c in costs}
        return {"color": color, "costs": costs, "kinds": kinds, "reg": "s0",
                "detail": {}, "available": available, "forbidden": forbidden}

    def test_the_cheapest_alternative_of_the_same_kind_is_chosen(self):
        self.assertEqual(
            wf.probe_colour(self.entry(14, {14: 0.0, 15: 9.0, 16: 2.0})), 16)

    def test_the_colour_the_web_already_has_is_never_the_probe(self):
        self.assertEqual(wf.probe_colour(self.entry(14, {14: 0.0, 16: 5.0})), 16)

    def test_a_colour_outside_the_cost_table_is_not_legal(self):
        # c20 is offered by the mask and absent from the table: not a probe.
        e = self.entry(14, {14: 0.0, 16: 5.0}, available=1 << 20)
        self.assertEqual(wf.probe_colour(e), 16)

    def test_a_colour_the_mask_calls_illegal_is_still_probed_if_it_is_priced(self):
        # Measured: three of five successful forces sit outside the mask.
        e = self.entry(2, {2: 0.0, 17: 0.0}, kinds={2: "caller", 17: "caller"},
                       available=1 << 2, forbidden=1 << 17)
        self.assertEqual(wf.probe_colour(e), 17)

    def test_a_same_kind_colour_beats_a_cheaper_one_of_the_other_kind(self):
        e = self.entry(14, {14: 0.0, 3: 0.0, 16: 50.0},
                       kinds={14: "callee", 3: "caller", 16: "callee"})
        self.assertEqual(wf.probe_colour(e), 16)

    def test_the_other_kind_is_used_when_the_web_has_no_same_kind_option(self):
        e = self.entry(14, {14: 0.0, 3: 0.0}, kinds={14: "callee", 3: "caller"})
        self.assertEqual(wf.probe_colour(e), 3)

    def test_equal_cost_breaks_toward_the_nearest_colour(self):
        # Without this every web in the procedure probes to the same low
        # colour, because most costs tie at zero.
        e = self.entry(16, {16: 0.0, 9: 0.0, 15: 0.0, 2: 0.0})
        self.assertEqual(wf.probe_colour(e), 15)

    def test_a_web_with_no_second_priced_colour_cannot_be_probed(self):
        self.assertIsNone(wf.probe_colour(self.entry(14, {14: 0.0})))
        self.assertIsNone(wf.probe_colour(self.entry(14, {})))

    def test_an_uncoloured_web_cannot_be_probed(self):
        self.assertIsNone(wf.probe_colour(self.entry(None, {1: 0.0, 2: 0.0})))

    def test_the_kind_is_read_from_the_cost_rows(self):
        text = trace(COLOR.format(w=7, c=14, r="s0"),
                     COST.format(w=7, c=14, cost=0.0),
                     COST.format(w=7, c=16, cost=0.0).replace("kind=caller",
                                                              "kind=callee"))
        entry = wf.parse_trace(text, 0)[7]
        self.assertEqual(entry["kinds"], {14: "caller", 16: "callee"})


class HistogramTests(unittest.TestCase):
    def test_positions_bin_by_the_byte_offset_of_their_word(self):
        self.assertEqual(wf.histogram([0, 1, 0x20], 0x80), {0x00: 2, 0x80: 1})

    def test_the_bins_sum_to_the_positions(self):
        self.assertEqual(sum(wf.histogram([0, 5, 0x20, 0x21], 0x80).values()), 4)

    def test_a_footprint_keeps_only_windows_that_moved(self):
        self.assertEqual(wf.footprint({0: 5, 0x80: 5}, {0: 3, 0x80: 5}),
                         {0: -2})

    def test_a_footprint_reports_residual_the_probe_introduced(self):
        self.assertEqual(wf.footprint({0: 5}, {0: 5, 0x80: 2}), {0x80: +2})

    def test_a_web_that_moved_nothing_has_an_empty_footprint(self):
        self.assertEqual(wf.footprint({0: 5}, {0: 5}), {})


class EveryColourTests(unittest.TestCase):
    def entry(self, color, costs, kinds=None):
        return {"color": color, "costs": costs,
                "kinds": kinds or {c: "callee" for c in costs},
                "reg": "s0", "detail": {}, "available": 0, "forbidden": 0}

    def test_every_legal_colour_of_the_webs_own_kind_is_planned(self):
        e = self.entry(14, {14: 0.0, 15: 0.0, 16: 0.0, 17: 0.0})
        # nearest-first from 14: 15 and 13 tie at distance 1, then 16, 17.
        self.assertEqual(wf.every_colour(e), [15, 16, 17])

    def test_the_colour_the_web_holds_is_not_planned(self):
        e = self.entry(14, {14: 0.0, 15: 0.0})
        self.assertNotIn(14, wf.every_colour(e))

    def test_the_other_kind_is_excluded_by_default_and_included_on_request(self):
        e = self.entry(14, {14: 0.0, 15: 0.0, 3: 0.0},
                       kinds={14: "callee", 15: "callee", 3: "caller"})
        self.assertEqual(wf.every_colour(e), [15])
        self.assertEqual(sorted(wf.every_colour(e, same_kind=False)), [3, 15])

    def test_colours_are_ordered_nearest_first_so_a_truncated_run_still_spreads(self):
        e = self.entry(14, {14: 0.0, 20: 0.0, 15: 0.0, 12: 0.0})
        self.assertEqual(wf.every_colour(e), [15, 12, 20])

    def test_an_uncoloured_web_plans_nothing(self):
        self.assertEqual(wf.every_colour(self.entry(None, {1: 0.0})), [])


class WinnerTests(unittest.TestCase):
    def row(self, web, score, status="ok", probe=16):
        return {"web": web, "reg": "s0", "probe": probe, "score": score,
                "footprint": {}, "status": status}

    def test_only_probes_below_the_base_are_winners(self):
        rows = [self.row(7, 200), self.row(9, 227), self.row(11, 300)]
        self.assertEqual([r["web"] for r in wf.winners(rows, 227)], [7])

    def test_winners_are_ordered_by_score(self):
        rows = [self.row(7, 220), self.row(9, 205), self.row(11, 212)]
        self.assertEqual([r["web"] for r in wf.winners(rows, 227)], [9, 11, 7])

    def test_a_size_changing_cell_is_never_a_winner(self):
        # Its score is not comparable: the insertion shadow moved with it.
        rows = [self.row(7, 100, status="size delta -16; the insertion "
                                        "shadow shifts and cannot be "
                                        "differenced")]
        self.assertEqual(wf.winners(rows, 227), [])

    def test_a_declined_probe_is_never_a_winner(self):
        rows = [self.row(7, None, status="declined: forbidden mask")]
        self.assertEqual(wf.winners(rows, 227), [])

    def test_the_render_names_each_winner_as_a_force_string(self):
        text = wf.render([self.row(379, 217, probe=20)], 0x80, 227)
        self.assertIn("p1:w379=c20", text)
        self.assertIn("(+10)", text)


class FreshnessTests(unittest.TestCase):
    """A landscape describes one function body, and nothing enforced that.

    Measured cost: five consecutive dispatches on the tree's biggest function
    were told not to re-run a landscape that had been measured against a body
    two revisions old, on the grounds that the axis was "closed and proved".
    It was -- for a different function.
    """

    def test_a_matching_fingerprint_is_not_flagged(self):
        with mock.patch.object(wf, "source_fingerprint", return_value="abc"):
            self.assertIsNone(wf.freshness({"symbol": "f",
                                            "source_context_sha256": "abc"}))

    def test_a_changed_source_is_called_stale(self):
        with mock.patch.object(wf, "source_fingerprint", return_value="xyz"):
            warning = wf.freshness({"symbol": "f",
                                    "source_context_sha256": "abc"})
        self.assertIsNotNone(warning)
        self.assertTrue(warning.startswith("STALE"))
        self.assertIn("abc", warning)
        self.assertIn("xyz", warning)

    def test_a_report_with_no_fingerprint_is_flagged_too(self):
        # Reports written before the stamp existed cannot be vouched for.
        warning = wf.freshness({"symbol": "f"})
        self.assertIsNotNone(warning)
        self.assertIn("no source fingerprint", warning)

    def test_an_unknown_symbol_is_not_called_stale(self):
        # A function that has left the queue has no row to compare against;
        # refusing to guess beats inventing a verdict.
        with mock.patch.object(wf, "source_fingerprint", return_value=None):
            self.assertIsNone(wf.freshness({"symbol": "gone",
                                            "source_context_sha256": "abc"}))


class PackingTests(unittest.TestCase):
    """The best COMBINATION is a packing over radii, not the top of the list.

    Measured on overlay 58: `w225=c20` beats `w225=c14` as a single force (217
    against 220), yet the five-force set holding c14 measures 185 and the one
    holding c20 measures 192, because c20 shares its radius exactly with
    `w379=c20` and taking it abandons the region c14 reaches. The packing over
    the complete 1,875-probe landscape returns that measured optimum.
    """

    def row(self, web, probe, score, windows, status="ok"):
        return {"web": web, "reg": "s0", "probe": probe, "score": score,
                "footprint": {w: -1 for w in windows}, "status": status}

    def test_disjoint_forces_are_all_taken(self):
        rows = [self.row(1, 10, 220, [0x00]), self.row(2, 11, 218, [0x80])]
        chosen, predicted = wf.pack(rows, 227)
        self.assertEqual(sorted(r["web"] for r in chosen), [1, 2])
        self.assertEqual(predicted, 227 - 7 - 9)

    def test_overlapping_forces_take_only_the_stronger(self):
        rows = [self.row(1, 10, 220, [0x00, 0x80]), self.row(2, 11, 210, [0x80])]
        chosen, predicted = wf.pack(rows, 227)
        self.assertEqual([r["web"] for r in chosen], [2])
        self.assertEqual(predicted, 210)

    def test_one_web_cannot_be_packed_at_two_colours(self):
        # A web has one colour; without this the packing predicts a score no
        # compile can produce.
        rows = [self.row(5, 14, 220, [0x00]), self.row(5, 20, 217, [0x80])]
        chosen, predicted = wf.pack(rows, 227)
        self.assertEqual(len(chosen), 1)
        self.assertEqual(predicted, 217)

    def test_a_weaker_force_wins_when_it_frees_a_region(self):
        # The overlay 58 shape exactly: w225 at c20 scores better alone but
        # duplicates w379's radius; at c14 it reaches a region nothing else does.
        rows = [self.row(225, 20, 217, [0x1700]),
                self.row(225, 14, 220, [0x0780]),
                self.row(379, 20, 217, [0x1700])]
        chosen, predicted = wf.pack(rows, 227)
        self.assertEqual(sorted((r["web"], r["probe"]) for r in chosen),
                         [(225, 14), (379, 20)])
        self.assertEqual(predicted, 227 - 7 - 10)

    def test_a_force_that_does_not_beat_the_base_is_never_packed(self):
        rows = [self.row(1, 10, 300, [0x00]), self.row(2, 11, 220, [0x80])]
        chosen, _ = wf.pack(rows, 227)
        self.assertEqual([r["web"] for r in chosen], [2])

    def test_nothing_to_pack_predicts_the_base(self):
        self.assertEqual(wf.pack([self.row(1, 10, 300, [0])], 227), ([], 227))


class RivalTests(unittest.TestCase):
    def row(self, web, probe, score, windows):
        return {"web": web, "reg": "s0", "probe": probe, "score": score,
                "footprint": {w: -1 for w in windows}, "status": "ok"}

    def test_forces_with_an_identical_radius_are_one_question(self):
        rows = [self.row(225, 20, 217, [0x1700]), self.row(379, 20, 217, [0x1700]),
                self.row(75, 16, 212, [0x0380])]
        groups = wf.rivals(rows, 227)
        self.assertEqual(len(groups), 1)
        self.assertEqual(sorted(r["web"] for r in groups[0]), [225, 379])

    def test_an_overlapping_but_unequal_radius_is_not_a_rival_group(self):
        rows = [self.row(1, 10, 220, [0x00, 0x80]), self.row(2, 11, 218, [0x80])]
        self.assertEqual(wf.rivals(rows, 227), [])

    def test_a_rival_group_is_ordered_best_first(self):
        rows = [self.row(1, 10, 224, [0x00]), self.row(2, 11, 212, [0x00])]
        self.assertEqual([r["web"] for r in wf.rivals(rows, 227)[0]], [2, 1])

    def test_the_render_names_the_packing_as_force_flags(self):
        rows = [self.row(75, 16, 212, [0x0380]), self.row(27, 17, 224, [0x0080])]
        text = wf.render(rows, 0x80, 227)
        self.assertIn("--force p1:w75=c16", text)
        self.assertIn("--force p1:w27=c17", text)
        self.assertIn("predicted 209", text)


class HoldTests(unittest.TestCase):
    """The second-order landscape: every other web, with a packing held.

    Every landscape on overlay 58 was first-order -- each web against an
    unforced baseline -- and the five forces it nominated all had a named
    variable behind them. What the other 138 webs do once those five are
    right had never been asked, because nothing could hold them.
    """

    HOLD = ["p1:w75=c16", "p1:w379=c20", "p1:w225=c14"]

    def webs(self):
        return {w: {"color": 14, "costs": {14: 0.0, 16: 1.0}, "reg": "s0",
                    "kinds": {14: "callee", 16: "callee"}, "detail": {},
                    "available": 0, "forbidden": 0}
                for w in (27, 75, 225, 379, 435)}

    def test_the_held_webs_are_the_webs_the_forces_name(self):
        self.assertEqual(wf.held_webs(self.HOLD), {75, 225, 379})

    def test_a_held_web_is_never_planned(self):
        plan = wf.plan_probes(self.webs(), [27, 75, 225, 379, 435],
                              hold=self.HOLD)
        self.assertEqual([w for w, _ in plan], [27, 435])

    def test_with_nothing_held_every_wanted_web_is_planned(self):
        plan = wf.plan_probes(self.webs(), [27, 75, 435])
        self.assertEqual([w for w, _ in plan], [27, 75, 435])

    def test_a_held_web_is_skipped_under_every_colour_too(self):
        plan = wf.plan_probes(self.webs(), [75, 435], every=True,
                              hold=self.HOLD)
        self.assertEqual({w for w, _ in plan}, {435})

    def test_a_wanted_web_with_no_records_is_still_reported(self):
        plan = wf.plan_probes(self.webs(), [999], hold=self.HOLD)
        self.assertEqual(plan, [(999, None)])

    def test_every_cell_carries_the_held_set_before_its_own_probe(self):
        # Order matters for the receipt: force_acceptance checks each force
        # in turn, and a reader of command.json should see the premise first.
        self.assertEqual(wf.cell_forces(self.HOLD, 435, 15),
                         ("p1:w75=c16", "p1:w379=c20", "p1:w225=c14",
                          "p1:w435=c15"))

    def test_with_nothing_held_a_cell_is_just_its_probe(self):
        self.assertEqual(wf.cell_forces([], 435, 15), ("p1:w435=c15",))

    def test_the_render_names_the_held_set_and_calls_the_baseline_held(self):
        rows = [{"web": 435, "reg": "s0", "probe": 15, "score": 110,
                 "footprint": {0x1700: -6}, "status": "ok"}]
        text = wf.render(rows, 0x80, 116, self.HOLD)
        self.assertIn("HELD in every cell", text)
        for force in self.HOLD:
            self.assertIn(f"--force {force}", text)
        self.assertIn("beating the held 116", text)
        self.assertIn("p1:w435=c15", text)

    def test_the_render_without_a_hold_still_says_unforced(self):
        text = wf.render([], 0x80, 157)
        self.assertNotIn("HELD", text)
        self.assertIn("beating the unforced 157", text)

    def test_a_trace_cannot_be_supplied_alongside_a_hold(self):
        # The held colours change what the other webs are offered, so the
        # plan must be read from the held baseline's own trace.
        with self.assertRaises(SystemExit) as caught:
            wf.main(["sym", "--out", "x", "--hold", "p1:w75=c16",
                     "--trace", "somewhere.log"])
        self.assertIn("--trace", str(caught.exception))

    def test_a_malformed_hold_is_refused_before_any_compile(self):
        with self.assertRaises(SystemExit):
            wf.main(["sym", "--out", "x", "--hold", "w75=16"])


class RenderTests(unittest.TestCase):
    def row(self, web, footprint, status="ok", **kw):
        base = {"web": web, "reg": "s0", "probe": 16, "score": 100,
                "footprint": footprint, "status": status}
        base.update(kw)
        return base

    def index(self, text: str) -> dict[str, list[str]]:
        """Just the nomination table; the per-web listing also names offsets."""
        body = text.split("nomination table", 1)[1]
        return {l.split()[0]: l.split()[1:] for l in body.splitlines()
                if l.strip().startswith("0x")}

    def test_the_window_index_lists_every_web_that_controls_a_window(self):
        text = wf.render([self.row(7, {0x00: -3, 0x80: -1}),
                          self.row(9, {0x80: -2})], 0x80)
        self.assertIn("nomination table", text)
        index = self.index(text)
        self.assertEqual(index["0x00000"], ["7(3)"])
        self.assertEqual(sorted(index["0x00080"]), ["7(1)", "9(2)"])

    def test_a_window_ranks_the_web_that_moves_it_most_first(self):
        # A probe cascades into its neighbours, so a footprint is a superset
        # of the web's own range; the strongest mover is the best nomination.
        text = wf.render([self.row(7, {0x80: -1}), self.row(9, {0x80: -9}),
                          self.row(11, {0x80: +4})], 0x80)
        row = self.index(text)["0x00080"]
        self.assertEqual(row, ["9(9)", "11(4)", "7(1)"])

    def test_residual_a_probe_introduced_ranks_by_magnitude_not_sign(self):
        text = wf.render([self.row(7, {0x80: +6}), self.row(9, {0x80: -2})],
                         0x80)
        self.assertEqual(self.index(text)["0x00080"], ["7(6)", "9(2)"])

    def test_a_web_whose_colour_moved_nothing_is_named_not_dropped(self):
        text = wf.render([self.row(7, {})], 0x80)
        self.assertIn("moved no word", text)
        self.assertIn("7", text)

    def test_a_size_changing_cell_is_reported_and_not_mapped(self):
        text = wf.render([self.row(7, {}, status="size delta +4; the "
                                       "insertion shadow shifts and cannot "
                                       "be differenced")], 0x80)
        self.assertIn("not probed", text)
        self.assertIn("size delta +4", text)
        self.assertNotIn("nomination table\n\n   0x", text)

    def test_a_declined_probe_is_reported_with_its_reason(self):
        text = wf.render([self.row(7, {}, status="declined: forbidden mask")],
                         0x80)
        self.assertIn("declined: forbidden mask", text)

    def test_the_counts_separate_located_silent_and_unprobed_webs(self):
        text = wf.render([self.row(7, {0: -1}), self.row(9, {}),
                          self.row(11, {}, status="no second legal colour")],
                         0x80)
        self.assertIn("3 webs probed: 1 located, 1 moved nothing, 1 not probed",
                      text)


class LandscapeRefuseTests(unittest.TestCase):
    """L155: --every-colour on a size mismatch names the insertion, not the web."""

    def test_helper_refuses_only_the_landscape_on_a_nonzero_delta(self):
        self.assertIsNone(wf.refuse_every_colour(0, True))
        self.assertIsNone(wf.refuse_every_colour(None, True))
        self.assertIsNone(wf.refuse_every_colour(8, False))
        self.assertIn("size_delta +8", wf.refuse_every_colour(8, True))
        self.assertIn("size_delta -12", wf.refuse_every_colour(-12, True))

    def test_every_colour_cli_refuses_nonzero_size_without_a_probe_compile(self):
        """Drive the shipped entry: web_footprint.main --every-colour."""
        import tempfile
        cell = wf.fl.Cell((), 10, True, size_delta=8)
        with tempfile.TemporaryDirectory() as tmp:
            out = pathlib.Path(tmp) / "out"
            with mock.patch.object(wf.fl, "compile_command",
                                   return_value=["cc", "-o", "x.o"]) as compile, \
                    mock.patch.object(wf.fl, "run_cell", return_value=cell) as run:
                with self.assertRaises(SystemExit) as cm:
                    wf.main(["fixture_symbol", "--every-colour", "--out", str(out)])
        self.assertIn("refusing --every-colour", str(cm.exception))
        self.assertIn("size_delta +8", str(cm.exception))
        compile.assert_called_once()
        run.assert_called_once()


if __name__ == "__main__":
    unittest.main()
