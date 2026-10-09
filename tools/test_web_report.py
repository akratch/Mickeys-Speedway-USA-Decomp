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


class CompileEnvironment(unittest.TestCase):
    def test_cut_variables_survive_and_the_rest_do_not(self):
        env = {"PATH": "p", "CDX_FORCE": "x", "DKWB_UGEN_TRACE": "1",
               "DKWB_CUT_A": "3", "DKWB_CUT_B": ""}
        self.assertEqual(wr.cut_env(env), {"DKWB_CUT_A": "3"})
        self.assertEqual(wr.clean_env(env), {"PATH": "p"})

    def test_with_source_replaces_only_the_last_word(self):
        cmd = ["cc", "-O2", "-o", "x.o", "src/a.c"]
        self.assertEqual(wr.with_source(cmd, pathlib.Path("/c/cand.c")),
                         ["cc", "-O2", "-o", "x.o", "/c/cand.c"])
        self.assertEqual(wr.with_source(cmd, None), cmd)

    def test_missing_source_is_refused(self):
        with self.assertRaises(SystemExit):
            wr.main(["sym", "--source", "/nonexistent/cand.c"])


if __name__ == "__main__":
    unittest.main()
