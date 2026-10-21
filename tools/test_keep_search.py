"""Fixture tests for tools/keep_search.py (synthetic C and traces; no compiler needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import keep_search as ks  # noqa: E402
import lever_sweep as ls  # noqa: E402
import subst_report as sr  # noqa: E402

SRC = """\
typedef struct Pt { float x; float y; } Pt;
extern float sqrtf(float);

#ifdef NON_MATCHING
float fixture(Pt *pt, float a, float b) {
    float q;
    float r;
    float spare;
    float *addr;
    int hit;
    hit = 0;
    q = pt->x;
    if (a != 0.0f) {
        r = q * q;
        if (r < b) {
            b = sqrtf(b - r);
            if (b < r) {
                hit = 1;
            }
        }
    }
    return b;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/fixture.s")
#endif
"""

TRACE = """\
LOCAL proc=fixture use_line=15 def_line=14 var=M-8:4 decision=substituted reason=single_use_no_ilod rhs=op rhs_uses=1 vreg=1
GLOBAL proc=fixture use_line=16 def_line=14 var=M-8:4 decision=substituted reason=copied rhs=op vreg=1 cands=[L14:ok]
GLOBAL proc=fixture use_line=17 def_line=14 var=M-8:4 decision=substituted reason=copied rhs=op vreg=1 cands=[L14:ok]
STORE proc=fixture def_line=14 var=M-8:4 decision=deleted reason=dead lval_av=1 store_av=1 dead_after=1 rhs=op
LOCAL proc=fixture use_line=14 def_line=12 var=M-4:4 decision=kept reason=single_use_has_ilod rhs=ilod rhs_uses=1 vreg=1
STORE proc=fixture def_line=12 var=M-4:4 decision=kept reason=live_after_block lval_av=1 store_av=1 dead_after=0 rhs=ilod
"""

KEPT_TRACE = TRACE.replace("decision=deleted reason=dead", "decision=kept reason=live_after_block")


def rows():
    return sr.parse_trace(TRACE, "fixture")


def fn():
    return ls.find_function(SRC, "fixture")


class ShapeTests(unittest.TestCase):
    def test_rhs_shape_reads_operands_and_loads(self):
        self.assertEqual(ks.rhs_shape("4.0f * quadA")["vars"], ["quadA"])
        self.assertFalse(ks.rhs_shape("4.0f * quadA")["ilod"])
        self.assertFalse(ks.rhs_shape("quadB * quadB")["ilod"])
        self.assertTrue(ks.rhs_shape("pt->x")["ilod"])
        self.assertTrue(ks.rhs_shape("*addr")["ilod"])
        self.assertTrue(ks.rhs_shape("v[2] + 1")["ilod"])
        self.assertEqual(ks.rhs_shape("-quadB")["kind"], "op")
        self.assertEqual(ks.rhs_shape("quadB")["kind"], "var")
        self.assertEqual(ks.rhs_shape("sqrtf(x)")["kind"], "call")

    def test_parent_op(self):
        self.assertEqual(ks.parent_op("discriminant = z1 * (quadC - radiusSq);", "z1"), "mul")
        self.assertEqual(ks.parent_op("if (discriminant < z2)", "z2"), "cmp")
        self.assertEqual(ks.parent_op("fraction = (twoZ1 - discriminant) / x1;", "x1"), "div")
        self.assertEqual(ks.parent_op("fraction = (twoZ1 - discriminant) / x1;", "twoZ1"), "sub")
        self.assertEqual(ks.parent_op("y = x;", "x"), "copy")
        self.assertEqual(ks.parent_op("use(x, 1);", "x"), "arg")
        self.assertEqual(ks.parent_op("z1 = 4.0f * quadA;", "z1"), "def")
        self.assertEqual(ks.parent_op("p->z1 = 2;", "z1"), "n/a")

    def test_calls_between(self):
        src = SRC.splitlines()
        self.assertEqual(ks.calls_between(src, 14, 15, "r"), [])
        self.assertEqual(ks.calls_between(src, 14, 17, "r"), ["sqrtf"])
        # a call that encloses the read on its own line does not come before it
        self.assertEqual(ks.calls_between(src, 14, 16, "r"), [])


class PictureTests(unittest.TestCase):
    def test_picture_of_a_forwarded_definition(self):
        pic = ks.picture(rows(), SRC.splitlines(), 14)
        self.assertEqual((pic["var"], pic["name"], pic["rhs"]), ("M-8:4", "r", "q * q"))
        self.assertFalse(pic["kept"])
        self.assertEqual([r["use_line"] for r in pic["reads"]], [15, 16, 17])
        self.assertEqual(pic["reads"][0]["scope"], "same-block")
        self.assertEqual(pic["reads"][1]["scope"], "other-block")
        self.assertEqual(pic["reads"][0]["this_def"], "single_use_no_ilod")
        self.assertEqual(pic["reads"][1]["this_def"], "ok")
        self.assertEqual(pic["reads"][2]["calls_between"], ["sqrtf"])
        self.assertEqual(pic["reads"][0]["parent_op"], "cmp")
        self.assertIn("forwarded", ks.render_picture(pic))

    def test_picture_of_a_kept_definition_and_unknown_line(self):
        pic = ks.picture(rows(), SRC.splitlines(), 12)
        self.assertTrue(pic["kept"])
        self.assertTrue(pic["shape"]["ilod"])
        self.assertIn("KEPT", ks.render_picture(pic))
        self.assertIsNone(ks.picture(rows(), SRC.splitlines(), 11)["var"])

    def test_var_found_through_candidate_mention_only(self):
        only_cands = [r for r in rows() if r["kind"] == "GLOBAL"]
        self.assertEqual(ks.var_of_def(only_cands, 14), "M-8:4")


class GenerateTests(unittest.TestCase):
    def cells(self, line=14, classes=ks.CLASSES, known=()):
        pic = ks.picture(rows(), SRC.splitlines(), line)
        return ks.generate(fn(), pic, list(classes), list(known))

    def test_every_class_keeps_line_numbers(self):
        n = SRC.count("\n")
        cells = self.cells()
        self.assertTrue(cells)
        for c in cells:
            text = ks.apply_spans(SRC, c.spans)
            self.assertEqual(text.count("\n"), n, c.edit)
            for l in c.def_lines:
                self.assertIn("r =", text.splitlines()[l - 1], (c.cls, c.edit))

    def test_classes_present(self):
        classes = {c.cls for c in self.cells()}
        for want in ("addr_taken", "operand_redef", "operand_cast", "dead_carrier", "zero_after",
                     "rhs_ilod", "cast_def", "hoist", "known_region", "multi_def"):
            self.assertIn(want, classes)
        self.assertNotIn("sink", classes)  # the first later read is in the same block

    def test_cell_texts(self):
        by = {c.cls: c for c in self.cells()}
        t = lambda c: ks.apply_spans(SRC, c.spans).splitlines()
        self.assertIn("r = q * q; addr = &r;", t(by["addr_taken"])[13])
        self.assertEqual(by["addr_taken"].semantics, "diag")
        self.assertIn("r = q * q; q = q;", t(by["operand_redef"])[13])
        self.assertIn("q = (float) q;", t(by["operand_cast"])[13])
        carriers = [t(c)[13] for c in self.cells() if c.cls == "dead_carrier"]
        self.assertTrue(any("spare = q; r = spare * spare; spare = 0.0f;" in l for l in carriers))
        self.assertIn("r = (float) (q * q);", t(by["cast_def"])[13])
        self.assertIn("r = pt->x * pt->x;", t(by["rhs_ilod"])[13].replace("(pt->x)", "pt->x"))
        self.assertIn("if (hit == 0) { r = q * q; }", t(by["known_region"])[13])
        self.assertIn("} else { r = q * q; }", t(by["multi_def"])[13])
        hoisted = t(by["hoist"])
        self.assertIn("r = q * q;", hoisted[11])  # appended to `q = pt->x;` before the if
        self.assertNotIn("r = q * q", hoisted[13])
        self.assertEqual(by["hoist"].def_lines, {12})
        self.assertEqual(by["hoist"].semantics, "safe")

    def test_dead_carrier_first_and_all_forms_and_cap(self):
        cells = [c for c in self.cells(classes=["dead_carrier"]) if c.cls == "dead_carrier"]
        edits = {c.edit for c in cells}
        self.assertTrue(any("r = spare * spare" in e for e in edits))
        self.assertTrue(any("r = spare * q" in e for e in edits))
        # `hit` is an int, `q` the operand, `addr` a pointer: only `spare` can carry a float
        self.assertTrue(all("spare = q" in e for e in edits))

    def test_zero_after_sits_after_the_last_read_and_needs_a_later_read(self):
        cells = [c for c in self.cells(classes=["zero_after"])]
        self.assertEqual(len(cells), 1)
        text = ks.apply_spans(SRC, cells[0].spans).splitlines()
        self.assertIn("r = q * q; q = 0.0f;", text[13])

    def test_known_true_detection_and_explicit_known(self):
        self.assertEqual(ks.known_true(fn(), 14), ["hit == 0"])
        self.assertEqual(ks.known_true(fn(), 19), [])  # `hit = 1` lies before line 19? no: it is on 18
        cells = self.cells(classes=["known_region"], known=["a != 0.0f"])
        self.assertEqual([c.edit for c in cells][0], "if (a != 0.0f) { r = q * q; }")

    def test_cells_are_distinct(self):
        cells = self.cells()
        self.assertEqual(len({c.key() for c in cells}), len(cells))


class ComposeTests(unittest.TestCase):
    def test_disjoint_and_compose(self):
        a = ks.Cell("operand_redef", 14, "q = q;", [(100, 100, " q = q;")], {14})
        b = ks.Cell("cast_def", 16, "x", [(200, 210, "y")], {16})
        c = ks.Cell("cast_def", 14, "x", [(100, 100, " z = z;")], {14})
        self.assertTrue(ks.disjoint(a, b))
        self.assertFalse(ks.disjoint(a, c))
        ab = ks.compose([a, b])
        self.assertEqual(ab.cls, "operand_redef+cast_def")
        self.assertEqual(ab.def_lines, {14, 16})
        self.assertEqual(len(ab.spans), 2)
        self.assertEqual(ab.semantics, "safe")
        self.assertEqual(ks.compose([a, ks.Cell("x", 1, "", [], {1}, "check")]).semantics, "check")

    def test_apply_spans_from_the_end(self):
        self.assertEqual(ks.apply_spans("abcdef", [(1, 2, "BB"), (4, 4, "_")]), "aBBcd_ef")


class StateAndRankTests(unittest.TestCase):
    def test_kept_state(self):
        self.assertEqual(ks.kept_state(rows(), "M-8:4", {14})[0], False)
        kept = sr.parse_trace(KEPT_TRACE, "fixture")
        self.assertEqual(ks.kept_state(kept, "M-8:4", {14})[0], True)
        self.assertEqual(ks.kept_state(rows(), "M-8:4", {99})[0], None)
        self.assertTrue(ks.goal_met(True, "kept"))
        self.assertTrue(ks.goal_met(False, "forwarded"))
        self.assertFalse(ks.goal_met(None, "kept"))
        self.assertEqual(ks.read_tally(rows(), "M-8:4", {14}), "substituted 3")

    def test_rank_goal_then_oracle_then_bytes(self):
        goal_big = {"goal": True, "oracle": "-", "masked": 40, "delta": 0, "residual": 40}
        goal_small = {"goal": True, "oracle": "-", "masked": 10, "delta": 0, "residual": 10}
        goal_no_oracle = {"goal": True, "oracle": "no", "masked": 1, "delta": 0, "residual": 1}
        nogoal = {"goal": False, "oracle": "yes", "masked": 0, "delta": 0, "residual": 0}
        err = {"error": "x"}
        order = sorted([err, nogoal, goal_big, goal_no_oracle, goal_small], key=ks.rank_key)
        self.assertEqual(order, [goal_small, goal_big, goal_no_oracle, nogoal, err])

    def test_winners_and_combinations(self):
        ca = ks.Cell("operand_redef", 14, "a", [(100, 100, " a")], {14}); ca.index = 0
        cb = ks.Cell("cast_def", 14, "b", [(100, 100, " b")], {14}); cb.index = 1
        cc = ks.Cell("zero_after", 16, "c", [(300, 300, " c")], {16}); cc.index = 2
        cd = ks.Cell("hoist", 16, "d", [(100, 100, " d")], {12}); cd.index = 3
        cells = {c.index: c for c in (ca, cb, cc, cd)}
        results = [
            {"index": 0, "def_line": 14, "oracle": "-", "masked": 5, "delta": 0, "residual": 5,
             "states": {14: {"kept": True}, 16: {"kept": False}}, "goal": False},
            {"index": 1, "def_line": 14, "oracle": "-", "masked": 9, "delta": 0, "residual": 9,
             "states": {14: {"kept": False}, 16: {"kept": False}}, "goal": False},
            {"index": 2, "def_line": 16, "oracle": "-", "masked": 7, "delta": 4, "residual": 7,
             "states": {14: {"kept": False}, 16: {"kept": True}}, "goal": False},
            {"index": 3, "def_line": 16, "oracle": "-", "masked": 2, "delta": 0, "residual": 2,
             "states": {14: {"kept": False}, 16: {"kept": True}}, "goal": False},
        ]
        self.assertEqual([c.index for c in ks.winners(results, cells, 14, "kept", 3)], [0])
        dup = dict(results[0], index=1, masked=6, residual=6, sha="same")
        results[0]["sha"] = "same"
        self.assertEqual([c.index for c in ks.winners([results[0], dup], cells, 14, "kept", 3)], [0])
        self.assertEqual([c.index for c in ks.winners(results, cells, 16, "kept", 3)], [3, 2])
        self.assertEqual([c.index for c in ks.winners(results, cells, 14, "forwarded", 3)], [1])
        combos = ks.combinations({14: [ca], 16: [cd, cc]})
        # (ca, cd) collide on the same span; (ca, cc) compose
        self.assertEqual([c.cls for c in combos], ["operand_redef+zero_after"])
        self.assertEqual(combos[0].def_lines, {14, 16})


if __name__ == "__main__":
    unittest.main()
