#!/usr/bin/env python3
"""Tests for tools/web_report.py, the per-web allocator report.

Nothing here compiles. The two fixtures under tools/fixtures/web_report/ are
the instrumented uopt's own numeric records (`[CDX]` rows: block numbers,
weights, counts and colour masks), captured with CDX_WEBREPORT=1 from two
queued functions and trimmed to the webs the tests read. Fields the tool never
reads are dropped and each record pointer (`lr`) is replaced by a symbolic id
(L1, L2, ...), which the tool treats as the opaque join key it is:

  o020_proc0.cdx        func_overlay_020_F000038C_1876964, proc 0
  o008_proc11_split.cdx the overlay 8 TU, proc 11 (splits with refused growth)

What is pinned is the reading a lane acts on:

  1. the per-reference breakdown sums back to the decision's totalsave, so
     the breakdown is the decision's own arithmetic and not a reconstruction;
  2. the col & 7 web and the index copy of the o020 body read exactly as
     measured (600 = 500 in bb23 + 100 in bb24; 300 = 100 + 200);
  3. every growth verdict agrees with the L161 rule, and the first refused
     block is named;
  4. a forbidden-seed bit is attributed to the web pinned in that block, and a
     web's own colour in a block is not counted against it;
  5. a log from a compiler without the CDX_WEBREPORT records is refused by
     name rather than printed as an empty report.
"""
import contextlib
import io
import pathlib
import sys
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import web_report as wr  # noqa: E402

FIXTURES = HERE / "fixtures" / "web_report"
OPS = tuple({1: "add", 4: "and", 24: "cvt", 54: "ilod", 65: "ixa", 91: "mpy"}
            .get(i, f"op{i}") for i in range(128))


def model(name: str, proc: int) -> dict:
    return wr.build(wr.parse_records((FIXTURES / name).read_text()), proc)


def decision(m: dict, web: int, phase: str = "p1") -> dict:
    found = [d for d in m["decisions"] if d["web"] == web and d["phase"] == phase]
    assert len(found) >= 1, web
    return found[0]


class DecodeTests(unittest.TestCase):
    def test_colour_bits_follow_the_forbidden0_layout(self):
        self.assertEqual(wr.bits(0x40000000), [1])          # v0
        self.assertEqual(wr.bits(0x18000000), [3, 4])       # a0 a1
        self.assertEqual(wr.colour_name(16), "s2")

    def test_growth_rule(self):
        row = {"new": "0", "left_before": "4", "left_after": "3", "numintf": "8", "strict": "1"}
        self.assertFalse(wr.grow_rule(row))                 # 6 < 8
        row.update(numintf="6")
        self.assertTrue(wr.grow_rule(row))                  # 6 >= 6
        row.update(new="4", numintf="2")
        self.assertFalse(wr.grow_rule(row))                 # new not below left_before

    def test_expression_rendering_with_and_without_names(self):
        bare, named = wr.Namer(), wr.Namer({-28: ["col"]}, {0: "grid"})
        bare.ops = named.ops = OPS
        expr = "op1(op4(var:-28:1:4,const:7),const:9)"
        self.assertEqual(bare.render(expr), "((auto[-28] & 7) + 9)")
        self.assertEqual(named.render(expr), "((col & 7) + 9)")
        self.assertEqual(named.render("op54(var:0:2:4,@12)"), "*(grid + 12)")
        self.assertEqual(named.render("var:2:3:4"), "$v0")
        self.assertEqual(named.render("var:32:3:4"), "$f0")
        self.assertEqual(named.variables(expr), ["col"])


class O020Tests(unittest.TestCase):
    """The o020 body: the shard asks for exactly these two breakdowns."""

    @classmethod
    def setUpClass(cls):
        cls.m = model("o020_proc0.cdx", 0)

    def occurrences(self, web):
        detail, occs = decision(self.m, web)["save"]
        return [(int(o["bb"]), int(o["uses"]), int(o["defs"]), float(o["weight"]),
                 float(o["term"])) for o in occs]

    def test_breakdown_sums_to_totalsave_for_every_decision(self):
        checked = 0
        for d in self.m["decisions"]:
            if d["save"] is None:
                continue
            detail, occs = d["save"]
            self.assertAlmostEqual(sum(float(o["term"]) for o in occs), float(detail["gross"]))
            net = float(detail["gross"]) - float(detail["chargeA"]) - float(detail["chargeB"])
            self.assertAlmostEqual(net, float(d["dec"]["totalsave"]), places=3)
            checked += 1
        self.assertEqual(checked, 5)

    def test_col_and_7_is_four_uses_and_a_def_in_bb23_and_one_use_in_bb24(self):
        self.assertEqual(self.occurrences(132), [(23, 4, 1, 100.0, 500.0),
                                                 (24, 1, 0, 100.0, 100.0)])
        d = decision(self.m, 132)
        self.assertEqual((d["order"], d["color"]["reg"], d["dec"]["forced"]), (11, "t1", "-2"))

    def test_the_index_copy_is_a_def_in_bb23_and_a_use_and_def_in_bb24(self):
        self.assertEqual(self.occurrences(130), [(23, 0, 1, 100.0, 100.0),
                                                 (24, 1, 1, 100.0, 200.0)])
        self.assertEqual(decision(self.m, 130)["color"]["reg"], "s2")

    def test_blocks_carry_their_loop_weight_and_source_lines(self):
        self.assertEqual(self.m["blocks"][23]["weight"], 100)
        self.assertEqual(self.m["blocks"][9]["weight"], 1000)
        self.assertEqual(min(self.m["blocks"][23]["lines"]), 133)

    def test_report_text_names_the_expression_and_the_blocks(self):
        namer = wr.Namer({-28: ["col"], -32: ["index"]})
        namer.ops = OPS
        text = "\n".join(wr.report_web(self.m, decision(self.m, 132), namer, [], None))
        self.assertIn("(col & 7)", text)
        self.assertIn("bb23", text)
        self.assertIn("-> 500", text)


class O008SplitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.m = model("o008_proc11_split.cdx", 11)

    def test_every_verdict_agrees_with_the_rule(self):
        pairs = 0
        for d in self.m["decisions"]:
            pending = None
            for event, row in d["growth"]:
                if event == "grow":
                    pending = row
                elif event == "growv":
                    self.assertEqual(wr.grow_rule(pending), row["accepted"] == "1")
                    pairs += 1
        self.assertGreater(pairs, 20)

    def test_first_refused_block_is_named(self):
        namer = wr.Namer()
        namer.ops = OPS
        text = "\n".join(wr.report_web(self.m, decision(self.m, 126), namer, [], None))
        self.assertIn("first refused block: bb13", text)
        self.assertIn("REFUSED", text)

    def test_seed_bit_is_attributed_to_the_pinned_parameter(self):
        d = decision(self.m, 72)
        own_excluded = 0
        for row in d["forbid"]:
            own = int(row["own"])
            mask = int(row["mask0"], 16) & ~((1 << (31 - own)) if 0 < own < 32 else 0)
            own_excluded |= mask
        self.assertEqual(wr.bits(own_excluded), [4])        # a1
        self.assertIn("web 22", wr.seed_sources(self.m, 0, 4, wr.Namer()))

    def test_block_filter_keeps_only_that_blocks_rows(self):
        namer = wr.Namer()
        namer.ops = OPS
        text = "\n".join(wr.report_web(self.m, decision(self.m, 126), namer, [], 13))
        self.assertNotIn("bb10 ", text)
        self.assertIn("bb13", text)


class CommandTests(unittest.TestCase):
    def run_main(self, *argv):
        original = wr.source_for
        wr.source_for = lambda symbol, candidate=None: ([], None)
        out, err = io.StringIO(), io.StringIO()
        try:
            with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
                status = wr.main(list(argv))
        finally:
            wr.source_for = original
        return status, out.getvalue(), err.getvalue()

    def test_trace_mode_finds_the_only_procedure(self):
        status, out, _ = self.run_main("sym", "--trace", str(FIXTURES / "o020_proc0.cdx"),
                                       "--web", "141")
        self.assertEqual(status, 0)
        self.assertIn("proc 0", out)
        self.assertIn("web 141", out)

    def test_a_log_without_the_records_is_refused(self):
        log = FIXTURES / "o020_proc0.cdx"
        plain = "\n".join(l for l in log.read_text().splitlines()
                          if l.split()[1] not in wr.WEBREPORT_RECORDS)
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "plain.cdx"
            path.write_text(plain)
            status, _, err = self.run_main("sym", "--trace", str(path))
        self.assertEqual(status, 2)
        self.assertIn("CDX_WEBREPORT", err)


NEIGHBOUR_LOG = """\
[CDX] p1dec phase=p1 proc=3 web=7 sym=7 class=1 save=1 nocs=1 totalsave=1 bestcost=0 bestcolor=1 bestreg=v0 forbidden0=0x0 regsleft=20 numintf=3 decision=color forced=-2
[CDX] webblocks phase=p1 proc=3 role=target web=7 sym=7 lr=0x1 bbs=4,5,6,9 aux=-
[CDX] intf phase=p1 proc=3 web=7 other=11 sym=11 assigned=1 shared=0 marked=0
[CDX] webblocks phase=p1 proc=3 role=neighbor web=11 sym=11 lr=0x2 bbs=4,5,6 aux=-
[CDX] intf phase=p1 proc=3 web=7 other=12 sym=12 assigned=0 shared=0 marked=0
[CDX] webblocks phase=p1 proc=3 role=neighbor web=12 sym=12 lr=0x3 bbs=5,6,9 aux=-
[CDX] p1dec phase=p1 proc=3 web=8 sym=8 class=1 save=1 nocs=1 totalsave=1 bestcost=0 bestcolor=1 bestreg=v0 forbidden0=0x0 regsleft=20 numintf=1 decision=color forced=-2
[CDX] intf phase=p1 proc=3 web=8 other=99 sym=99 assigned=0 shared=0 marked=0
"""


class NeighbourTests(unittest.TestCase):
    def test_interferers_are_listed_per_block_and_runs_fold(self):
        rows = wr.parse_records(NEIGHBOUR_LOG)
        decisions = wr.neighbour_model(rows, 3, 7)
        self.assertEqual(len(decisions), 1)
        self.assertEqual(sorted(decisions[0]["neighbours"]), [11, 12])
        text = wr.render_neighbours(decisions, 7, None, "hdr")
        self.assertIn("bb4: 1  w11(v0)", text)
        self.assertIn("bb5-6: 2  w11(v0) w12", text)
        self.assertIn("bb9: 1  w12", text)
        self.assertNotIn("w99", text)
        only = wr.render_neighbours(decisions, 7, 5, "hdr")
        self.assertNotIn("bb4", only)
        self.assertIn("bb5: 2", only)

    def test_neighbours_via_trace_and_missing_web(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "n.cdx"
            path.write_text(NEIGHBOUR_LOG + "[CDX] bbline proc=3 bb=1 weight=1 entry=1 "
                            "lines=- mask1=0 mask2=0\n")
            status, out, _ = CommandTests.run_main(None, "sym", "--trace", str(path),
                                                   "--proc", "3", "--neighbours", "7")
            self.assertEqual(status, 0)
            self.assertIn("bb5-6", out)
            status, _, err = CommandTests.run_main(None, "sym", "--trace", str(path),
                                                   "--proc", "3", "--neighbours", "40")
            self.assertEqual(status, 2)
            self.assertIn("CDX_DETAIL_WEB=40", err)


class CompileEnvironment(unittest.TestCase):
    def test_cut_variables_survive_and_the_rest_do_not(self):
        env = {"PATH": "p", "CDX_FORCE": "x", "DKWB_UGEN_TRACE": "1",
               "DKWB_CUT_A": "3", "DKWB_CUT_B": ""}
        self.assertEqual(wr.cut_env(env), {"DKWB_CUT_A": "3"})
        self.assertEqual(wr.clean_env(env), {"PATH": "p"})

    def test_force_bias_and_keep_survive_alongside_the_cuts(self):
        env = {"PATH": "p", "CDX_FORCE": "p1:w1=s", "CDX_BIAS": "w2=5",
               "DKWB_SUBST_KEEP": "12", "CDX_LOG": "1", "DKWB_CUT_A": "3", "CDX_PROC": "4"}
        self.assertEqual(wr.setting_env(env), {"CDX_FORCE": "p1:w1=s", "CDX_BIAS": "w2=5",
                                               "DKWB_SUBST_KEEP": "12"})
        self.assertEqual(set(wr.pinned_env(env)),
                         {"CDX_FORCE", "CDX_BIAS", "DKWB_SUBST_KEEP", "DKWB_CUT_A"})
        self.assertIn("CDX_FORCE=p1:w1=s", wr.describe_pinned(env))
        self.assertEqual(wr.pinned_env({"PATH": "p"}), {})

    def test_with_source_replaces_only_the_last_word(self):
        cmd = ["cc", "-O2", "-o", "x.o", "src/a.c"]
        self.assertEqual(wr.with_source(cmd, pathlib.Path("/c/cand.c")),
                         ["cc", "-O2", "-o", "x.o", "/c/cand.c"])
        self.assertEqual(wr.with_source(cmd, None), cmd)

    def test_relative_source_is_the_callers_not_the_repo_roots(self):
        import os
        with tempfile.TemporaryDirectory() as directory:
            old = os.getcwd()
            os.chdir(directory)
            try:
                with self.assertRaises(SystemExit) as caught:
                    wr.main(["sym", "--source", "cand.c"])
            finally:
                os.chdir(old)
        self.assertIn(str(pathlib.Path(directory).resolve() / "cand.c"), str(caught.exception))

    def test_missing_source_is_refused(self):
        with self.assertRaises(SystemExit):
            wr.main(["sym", "--source", "/nonexistent/cand.c"])

class DefinitionSpanTest(unittest.TestCase):
    def test_a_multi_line_prototype_is_not_the_definition(self):
        lines = [
            "s32 target(s32 a,",
            "           s32 b);",
            "void other(void) {",
            "    target(1, 2);",
            "}",
            "s32 target(s32 a,",
            "           s32 b) {",
            "    return a + b;",
            "}",
        ]
        self.assertEqual(wr.definition_span(lines, "target"), (6, 9))

    def test_a_one_line_prototype_is_skipped_too(self):
        lines = ["s32 target(s32 a);", "s32 target(s32 a) {", "    return a;", "}"]
        self.assertEqual(wr.definition_span(lines, "target"), (2, 4))


if __name__ == "__main__":
    unittest.main()
