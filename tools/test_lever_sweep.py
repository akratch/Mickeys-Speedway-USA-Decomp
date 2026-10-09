"""Fixture tests for tools/lever_sweep.py (synthetic C and records; no build needed)."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lever_sweep as ls  # noqa: E402

SRC = """\
typedef struct Thing { int a; int b; struct Thing *next; } Thing;
extern int gCount;
extern void use(int v);

#ifdef NON_MATCHING
int fixture(Thing *thing, int n) {
    int i;
    int k;
    int total;
    int spare;
    Thing *p;

    total = 0;
    for (i = 0; i < n; i++) {
        total += thing[i].a;
    }
    p = thing->next;
    spare = p->b;
    gCount = total;
    use(spare);
    for (k = 0; k < 4; k++) {
        use(k);
    }
    return total;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/fixture.s")
#endif
"""


def function():
    return ls.find_function(SRC, "fixture")


def position_after(fn, needle):
    """(compound, index) right after the top-level statement containing `needle`."""
    for i, s in enumerate(fn.body.children):
        if needle in s.text(fn.text):
            return fn.body, i + 1
    raise AssertionError(needle)


class ParseTests(unittest.TestCase):
    def test_locals_and_params(self):
        fn = function()
        self.assertEqual(sorted(fn.locals), ["i", "k", "p", "spare", "total"])
        self.assertEqual(fn.locals["p"]["ptr"], 1)
        self.assertEqual(ls.category(fn.locals["total"]), "int")
        self.assertEqual(ls.category(fn.locals["p"]), "pointer")
        self.assertEqual(sorted(fn.params), ["n", "thing"])
        self.assertEqual(fn.body.ndecls, 5)

    def test_statement_tree(self):
        fn = function()
        kinds = [c.kind for c in fn.body.children]
        self.assertEqual(kinds, ["decl"] * 5 + ["simple", "loop", "simple", "simple", "simple",
                                                "simple", "loop", "jump"])
        loop = fn.body.children[6]
        self.assertEqual(loop.loop, "for")
        self.assertEqual(loop.children[0].kind, "compound")

    def test_idents_skip_members(self):
        self.assertEqual(ls.idents("total += thing[i].a;"), ["total", "thing", "i"])
        self.assertEqual(ls.idents("p = thing->next; /* spare */"), ["p", "thing"])


class DeadnessTests(unittest.TestCase):
    def test_dead_until_killed(self):
        fn = function()
        comp, idx = position_after(fn, "total = 0;")
        self.assertTrue(ls.dead_at(fn, comp, idx, "p"), "p is killed by p = thing->next")
        self.assertFalse(ls.dead_at(fn, comp, idx, "total"), "total is read by the loop")

    def test_loop_header_and_back_edge(self):
        fn = function()
        loop = fn.body.children[6]
        body = loop.children[0]
        self.assertFalse(ls.dead_at(fn, body, 0, "i"), "i is in the loop header")
        self.assertTrue(ls.dead_at(fn, body, 0, "k"), "k is next killed by the second loop's init")

    def test_end_of_function_is_dead(self):
        fn = function()
        comp, idx = position_after(fn, "use(spare);")
        self.assertTrue(ls.dead_at(fn, comp, idx, "spare"))
        self.assertTrue(ls.dead_at(fn, comp, idx, "p"))
        self.assertFalse(ls.dead_at(fn, comp, idx, "total"), "returned")


class LeverTests(unittest.TestCase):
    def cells(self, lever, lines=None):
        return ls.generate(function(), [lever], lines)

    def test_zero_def_after_loops_only_for_dead_locals(self):
        edits = {(c.line, c.edit) for c in self.cells("zero_def")}
        self.assertIn((16, "k = 0;"), edits)
        self.assertIn((16, "spare = 0;"), edits)
        self.assertNotIn((16, "total = 0;"), edits)
        for c in self.cells("zero_def"):
            self.assertEqual(len(c.text.splitlines()), len(SRC.splitlines()),
                             "same-line insertion keeps every line number")

    def test_merge_pairs_renames_whole_body_and_drops_declaration(self):
        cells = {c.edit: c for c in self.cells("merge_pairs")}
        cell = cells["k -> i (whole body)"]
        body = cell.text.split("int fixture", 1)[1]
        self.assertNotIn("int k;", body)
        self.assertIn("for (i = 0; i < 4; i++)", body)
        self.assertIn("use(i);", body)
        self.assertEqual(cell.semantics, "check")
        self.assertFalse([e for e in cells if "p " in e.split("->")[0] or e.endswith("-> p (whole body)")],
                         "a pointer never pairs with an int")
        self.assertEqual(len(cells), 12, "four ints, ordered pairs")

    def test_dead_read_uses_nearby_reads(self):
        cells = self.cells("dead_read", lines={13})
        edits = {c.edit for c in cells}
        self.assertIn("spare = thing[i].a;", edits)
        self.assertNotIn("p = thing[i].a;", edits, "an int read into a Thing * does not compile")
        self.assertFalse(any(e.startswith("total =") for e in edits), "total is live there")

    def test_keep_alive_and_noop_redef(self):
        ka = {c.edit for c in self.cells("keep_alive")}
        self.assertEqual(ka, {"spare |= 0;", "spare ^= 0;"}, "only expression assignments")
        nr = {c.edit for c in self.cells("noop_redef")}
        self.assertIn("p = (Thing *) p;", nr)

    def test_reorder_respects_dependences(self):
        swaps = {c.line for c in self.cells("reorder")}
        self.assertNotIn(17, swaps, "spare reads p")
        self.assertIn(18, swaps, "spare = p->b and gCount = total are independent")
        self.assertNotIn(19, swaps, "the next statement is a call")

    def test_const_iv_and_global_reread(self):
        civ = {c.edit for c in self.cells("const_iv")}
        self.assertIn("spare = 0; init through spare", civ)
        self.assertNotIn("total = 0; init through total", civ)
        gr = self.cells("global_reread")
        self.assertEqual([c.edit for c in gr], ["total -> gCount"])
        self.assertIn("return gCount;", gr[0].text)

    def test_every_cell_is_distinct(self):
        texts = [c.text for c in ls.generate(function(), list(ls.LEVERS), None)]
        self.assertEqual(len(texts), len(set(texts)))
        self.assertNotIn(SRC, texts)

    def test_interleave_keeps_every_lever(self):
        cells = [ls.Cell("a", 1, "", str(i)) for i in range(10)] + [ls.Cell("b", 1, "", "x")]
        kept = ls.interleave(cells, 3)
        self.assertEqual([c.lever for c in kept], ["a", "b", "a"])


RECORDS = """\
[CDX] bbline proc=3 bb=0 weight=1 entry=1 lines=10,11 mask1=0x0 mask2=0x0
[CDX] bbline proc=3 bb=1 weight=10 entry=0 lines=12,13 mask1=0x0 mask2=0x0
[CDX] bbline proc=3 bb=2 weight=1 entry=0 lines=14 mask1=0x0 mask2=0x0
[CDX] p1dec phase=p1 proc=3 web=5 sym=5 save=3.0 nocs=1 totalsave=3.0 bestcost=0.0 numintf=2 regsleft=9 forbidden0=0x0 decision=split forced=-2
[CDX] webblocks phase=p1 proc=3 role=target web=5 sym=5 lr=0xa bbs=0,1,2 aux=-
[CDX] webexpr phase=p1 proc=3 web=5 lr=0xa kind=3 dtype=6 expr=var:8:2:4
[CDX] p1dec phase=p1 proc=3 web=5 sym=5 save=2.0 nocs=1 totalsave=2.0 bestcost=0.0 numintf=2 regsleft=9 forbidden0=0x0 decision=color forced=-2
[CDX] webblocks phase=p1 proc=3 role=target web=5 sym=5 lr=0xb bbs=1 aux=-
[CDX] webexpr phase=p1 proc=3 web=5 lr=0xb kind=3 dtype=6 expr=var:8:2:4
[CDX] p1color phase=p1 proc=3 web=5 sym=5 color=21 reg=s7 forced=-2
[CDX] p2dec phase=p2 proc=3 web=9 sym=9 save=1.0 nocs=1 totalsave=1.0 bestcost=0.0 numintf=1 regsleft=9 forbidden0=0x0 decision=color forced=-2
[CDX] webblocks phase=p2 proc=3 role=target web=9 sym=9 lr=0xc bbs=2 aux=-
[CDX] webexpr phase=p2 proc=3 web=9 lr=0xc kind=4 dtype=6 expr=op91(var:-8:1:4,const:2)
[CDX] p2color phase=p2 proc=3 web=9 sym=9 color=19 reg=s5 forced=-2
"""


class OracleTests(unittest.TestCase):
    def test_parse_oracle(self):
        self.assertEqual(ls.parse_oracle("p1:w387=s, p2:w131=c21"),
                         [("p1", 387, "s"), ("p2", 131, "c21")])
        with self.assertRaises(SystemExit):
            ls.parse_oracle("p1:387=s")

    def test_decisions_join_lines_and_colours(self):
        decs, blines = ls.decisions(RECORDS, 3)
        self.assertEqual([(d["web"], d["decision"], d["colour"]) for d in decs],
                         [(5, "split", None), (5, "color", 21), (9, "color", 19)])
        self.assertEqual(decs[1]["lines"], [12, 13])
        self.assertEqual(blines[2], [14])

    def test_split_oracle_reads_memory_and_absence(self):
        decs, _ = ls.decisions(RECORDS, 3)
        forced = [dict(d, forced="-1", colour=None) if d["web"] == 5 else d for d in decs]
        targets = ls.oracle_targets(forced, [("p1", 5, "s")])
        hits, _ = ls.oracle_status(decs, targets)
        self.assertEqual(hits, 0, "the piece over lines 12-13 still holds s7")
        uncoloured = [dict(d, colour=None) if d["web"] == 5 else d for d in decs]
        self.assertEqual(ls.oracle_status(uncoloured, targets)[0], 1)
        absent = [d for d in decs if not (d["web"] == 5 and d["decision"] == "color")]
        self.assertEqual(ls.oracle_status(absent, targets)[0], 1, "a piece never formed")

    def test_colour_oracle_matches_by_expression_across_numbering(self):
        decs, _ = ls.decisions(RECORDS, 3)
        targets = ls.oracle_targets([dict(d, colour=22) if d["web"] == 9 else d for d in decs],
                                    [("p2", 9, "c22")])
        renumbered = [dict(d, web=40, phase="p1", colour=22) if d["web"] == 9 else d for d in decs]
        hits, detail = ls.oracle_status(renumbered, targets)
        self.assertEqual(hits, 1)
        self.assertEqual(detail[0]["rows"][0]["match"], "p1:w40")
        self.assertEqual(ls.oracle_status(decs, targets)[0], 0)
        self.assertEqual(ls.oracle_label(1, 4), "1/4")


class TypeTests(unittest.TestCase):
    def test_expr_types(self):
        fn = function()
        st = ls.structs_for(fn)
        self.assertEqual(ls.expr_type(fn, "thing[i].a", st)["type"], "int")
        nxt = ls.expr_type(fn, "thing->next", st)
        self.assertEqual((ls.bare_type(nxt["type"]), nxt["ptr"]), ("Thing", 1))
        self.assertEqual(ls.expr_type(fn, "*thing", st)["ptr"], 0)
        self.assertIsNone(ls.expr_type(fn, "gCount", st), "globals are unknown")
        self.assertIsNone(ls.expr_type(fn, "p->nothing", st))

    def test_compatibility(self):
        fn = function()
        st = ls.structs_for(fn)
        ptr = ls.expr_type(fn, "thing->next", st)
        self.assertTrue(ls.compatible(fn.locals["p"], ptr))
        self.assertFalse(ls.compatible(fn.locals["spare"], ptr))
        self.assertFalse(ls.compatible(fn.locals["spare"], None))

    def test_no_dead_read_pairs_incompatible_types(self):
        for c in ls.generate(function(), ["dead_read", "dead_masked"], None):
            name, _, rhs = c.edit.partition(" = ")
            if name == "p":
                self.assertIn("next", rhs, c.edit)
            else:
                self.assertNotIn("->next", rhs.replace("&", ""), c.edit)


class RootTests(unittest.TestCase):
    def test_root_is_the_cwd_worktree_not_the_script(self):
        with tempfile.TemporaryDirectory() as d:
            subprocess.run(["git", "init", "-q", d], check=True)
            self.assertEqual(ls.resolve_root(cwd=d), Path(d).resolve())
            (Path(d) / "Makefile").write_text("")
            self.assertEqual(ls.resolve_root(d), Path(d).resolve())
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(SystemExit):
                ls.resolve_root(d)

    def test_use_root_rebinds(self):
        old = ls.ROOT
        try:
            with tempfile.TemporaryDirectory() as d:
                ls.use_root(Path(d))
                self.assertEqual(ls.ROOT, Path(d))
        finally:
            ls.use_root(old)


def row(web, expr, lines, colour, phase="p1"):
    return {"web": web, "phase": phase, "expr": expr, "kind": 3, "bbs": [], "lines": lines,
            "decision": "color", "forced": "-2", "colour": colour}


class AmbiguityTests(unittest.TestCase):
    def targets(self):
        return ls.oracle_targets([row(7, "E", [10, 11], 21)], [("p1", 7, "c21")])

    def test_renumbered_and_edited_still_found(self):
        hits, d = ls.oracle_status([row(90, "E", [10, 11], 21)], self.targets())
        self.assertEqual((hits, d[0]["rows"][0]["match"]), (1, "p1:w90"))

    def test_changed_expression_falls_back_to_lines(self):
        hits, _ = ls.oracle_status([row(90, "E2", [10, 11], 21)], self.targets())
        self.assertEqual(hits, 1)

    def test_tie_with_disagreement_is_ambiguous(self):
        cands = [row(90, "E", [10, 11], 21), row(91, "E", [10, 11], 19)]
        hits, d = ls.oracle_status(cands, self.targets())
        self.assertEqual(hits, 0)
        self.assertTrue(d[0]["ambiguous"])
        self.assertEqual(d[0]["candidates"], ["p1:w90(c21)", "p1:w91(c19)"])
        self.assertEqual(ls.oracle_label(hits, 1, 1), "ambiguous")

    def test_tie_with_agreement_is_not_ambiguous(self):
        cands = [row(90, "E", [10, 11], 21), row(91, "E", [10, 11], 21)]
        hits, d = ls.oracle_status(cands, self.targets())
        self.assertEqual((hits, d[0]["ambiguous"]), (1, False))


class BiasTests(unittest.TestCase):
    def logs(self):
        a, b, c = (row(1, "A", [1], 16), row(2, "B", [5], 17), row(3, "C", [9], 18))
        return [a, b, c], [b, a, c]

    def test_parse(self):
        self.assertEqual(ls.parse_bias("w2=-4.5, 3=1"), [(2, -4.5), (3, 1.0)])
        self.assertEqual(ls.bias_env([(2, -4.5), (3, 1.0)]), "2=-4.5,3=1")
        with self.assertRaises(SystemExit):
            ls.parse_bias("2")

    def test_targets_are_reversed_pairs(self):
        un, bi = self.logs()
        t = ls.bias_targets(un, bi, [(2, 9.0)])
        self.assertEqual([x["spec"] for x in t], ["w2 before w1"])
        with self.assertRaises(SystemExit):
            ls.bias_targets(un, un, [(2, 0.0)])

    def test_cell_satisfies_when_order_holds_without_bias(self):
        un, bi = self.logs()
        t = ls.bias_targets(un, bi, [(2, 9.0)])
        hits, _ = ls.oracle_status(un, t)
        self.assertEqual(hits, 0, "the unbiased base decides w1 first")
        renumbered = [dict(d, web=d["web"] + 50) for d in bi]
        hits, d = ls.oracle_status(renumbered, t)
        self.assertEqual(hits, 1)
        self.assertEqual(d[0]["rows"][0]["match"], ["p1:w52"])


class NoOracleTests(unittest.TestCase):
    def test_label_without_targets_is_not_yes(self):
        self.assertEqual(ls.oracle_label(0, 0), "-")
        self.assertEqual(ls.oracle_label(2, 2), "yes")
        self.assertEqual(ls.oracle_label(0, 2), "no")


if __name__ == "__main__":
    unittest.main()
