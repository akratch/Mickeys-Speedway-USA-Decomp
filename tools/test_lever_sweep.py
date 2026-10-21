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
                         [("p1", 387, "s", None), ("p2", 131, "c21", None)])
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


class ExitStatusTests(unittest.TestCase):
    def status(self, exact, no_fail):
        import contextlib
        import io
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = ls.exit_status(exact, no_fail)
        return code, out.getvalue()

    def test_default_keeps_one_for_no_exact_cell(self):
        code, text = self.status([], False)
        self.assertEqual(code, 1)
        self.assertIn("no exact cell", text)

    def test_no_fail_makes_it_zero_and_says_so(self):
        code, text = self.status([], True)
        self.assertEqual(code, 0)
        self.assertIn("no exact cell", text)
        self.assertIn("--no-fail", text)

    def test_an_exact_cell_is_zero_and_silent(self):
        self.assertEqual(self.status([{"index": 1}], False), (0, ""))



# ---------------------------------------------------------------- source levers (2026-10-21)

SWITCH_SRC = """\
typedef struct Entry { s16 command; u16 duration; } Entry;
extern Entry *gCursor;
extern s32 gState;
extern f32 gTime;
extern void fade(f32 v, u8 which);
extern void stop(s32 v);
extern s32 trap();

void interp(void) {
    Entry *cursor;
    s32 opcode;
    s32 pathIndex;
    s32 low;
    s32 word;
    s32 carry;
    s32 spare;
    u16 half;
    f32 value;
    f32 hundred;
    s16 narrow;
    s16 narrow2;

    cursor = gCursor;
    hundred = 100.0f;
    carry = 0;
    while (cursor->command != 0x7F00 && gState == 1) {
        switch ((cursor->command >> 8) & 0xFF) {
            case 0:
                pathIndex = cursor->command & 0xFF;
                cursor = (Entry *) ((u8 *) cursor + 4);
                stop(pathIndex);
                break;
            case 0x22:
                opcode = cursor->command;
                value = (f32) *((u16 *) ((u8 *) cursor + 4));
                cursor = (Entry *) ((u8 *) cursor + 6);
                fade(value / hundred, opcode);
                break;
            case 0x30:
                low = *((u16 *) ((u8 *) cursor + 4)) & 0xFF;
                word = (*((u16 *) ((u8 *) cursor + 6)) & 0xFF) << 8;
                carry = carry + low;
                cursor = (Entry *) ((u8 *) cursor + 8);
                trap(low, word, carry);
                break;
            case 0x40:
                if (gState & 2) {
                    narrow = cursor->command & 0xFF;
                    stop(narrow);
                }
                if (gTime == 1) {
                    stop(0);
                }
                cursor = (Entry *) ((u8 *) cursor + 4);
                break;
            case 0x50:
                pathIndex = cursor->command & 0xFF;
                if (pathIndex == 0xFFF) {
                    pathIndex = 0;
                }
                gState = gState & 0xFF1FFFFF;
                gState |= 0x800000U;
                switch (low) {
                    case 1:
                        gState |= 0x400000;
                        break;
                }
                cursor = (Entry *) ((u8 *) cursor + 4);
                break;
        }
    }
    gCursor = cursor;
}
"""


def interp():
    return ls.find_function(SWITCH_SRC, "interp")


def region(fn, name):
    return next(r for r in ls.regions_cached(fn) if r.name == name)


class RegionTests(unittest.TestCase):
    def test_regions_and_exits(self):
        fn = interp()
        names = [r.name for r in ls.regions_cached(fn)]
        self.assertEqual(names, ["case 0", "case 0x22", "case 0x30", "case 0x40", "case 0x50", "case 1"])
        r = region(fn, "case 0x22")
        self.assertEqual([s.kind for s in r.stmts], ["simple"] * 4 + ["jump"])
        self.assertEqual(ls.region_exit_statuses(fn, r), {"break"})
        inner = region(fn, "case 1")
        self.assertEqual(ls.region_exit_statuses(fn, inner), {"break"})
        self.assertEqual(ls.regions_at(fn, inner.start), ("case 1", "case 0x50"))

    def test_case_filter(self):
        fn = interp()
        lines = ls.parse_case_filter(["0x22"], fn)
        self.assertEqual((min(lines), max(lines)), (33, 38))
        with self.assertRaises(SystemExit):
            ls.parse_case_filter(["0x99"], fn)


class DefBeforeUseTests(unittest.TestCase):
    def verdict(self, reg, old, new):
        fn = interp()
        return ls.rename_verdict(fn, region(fn, reg), old, new)

    def test_loop_carried_read_is_refused(self):
        self.assertIn("read before written", self.verdict("case 0x30", "carry", "spare"))

    def test_value_read_by_another_case_next_iteration_is_refused(self):
        self.assertIn("low live after", self.verdict("case 0x30", "low", "spare"),
                      "case 0x50 reads low without writing it first")

    def test_new_name_live_elsewhere_is_refused(self):
        self.assertIn("carry live after", self.verdict("case 0x22", "opcode", "carry"))

    def test_dead_carrier_is_accepted(self):
        self.assertIsNone(self.verdict("case 0x30", "word", "spare"))
        self.assertIsNone(self.verdict("case 0x22", "opcode", "spare"))
        self.assertIsNone(self.verdict("case 0x22", "opcode", "pathIndex"),
                          "every other case that reads pathIndex writes it first")

    def test_guarded_block_assignment_is_accepted(self):
        self.assertIsNone(self.verdict("case 0x40", "narrow", "narrow2"))

    def test_new_mentioned_in_region_is_refused(self):
        self.assertEqual(self.verdict("case 0x30", "low", "carry"), "new mentioned in the region")

    def test_dataflow_primitives(self):
        fn = interp()
        df = ls.Dataflow(fn)
        r = region(fn, "case 0x30")
        self.assertEqual(df.seq(r.stmts, "carry"), ls.READ)
        self.assertEqual(df.seq(r.stmts, "low"), set(), "written first, no unwritten path leaves")
        self.assertEqual(df.seq(r.stmts, "spare"), {"break"})
        self.assertTrue(df.after(r.stmts[-1], "low", "break"), "the back edge reaches case 0x50's read")
        self.assertFalse(df.after(r.stmts[-1], "word", "break"))

    def test_dataflow_loop_header_read_is_live(self):
        fn = interp()
        df = ls.Dataflow(fn)
        r = region(fn, "case 0")
        self.assertTrue(df.after(r.stmts[-1], "cursor", "break"), "the while header reads cursor")


class LiteralTests(unittest.TestCase):
    TABLE = [
        ("x = y & 0xFF;", "0xFF", "safe", "left of &"),
        ("if (a == 1)", "1", "safe", "left of =="),
        ("x = y != 2;", "2", "safe", "left of !="),
        ("case 3:", "3", "safe", "case label"),
        ("x = y << 8;", "8", "safe", "shift count"),
        ("g |= 0x800000U;", "0x800000U", "safe", "left of |="),
        ("x = 0x10 | y;", "0x10", "safe", "right of |"),
        ("x = y + 4;", "4", "check", "arithmetic"),
        ("x = y - 1;", "1", "check", "arithmetic"),
        ("if (a < 16)", "16", "check", "relational"),
        ("f(a, 5);", "5", "check", "call argument"),
        ("x = 7;", "7", "check", "assignment"),
        ("x = -1;", "-1", "check", "assignment"),
        ("if (a && 1)", "1", "check", "logical"),
        ("x = 1 << k;", "1", "check", "shifted operand"),
    ]

    def test_value_safety_table(self):
        for snippet, lit, want, reason in self.TABLE:
            units = [u for u in ls.literal_units(snippet) if u[2].strip() == lit]
            self.assertEqual(len(units), 1, snippet)
            verdict, why, _, _ = ls.literal_site(snippet, units[0][0], units[0][1])
            self.assertEqual((verdict, why), (want, reason), snippet)

    def test_spellings_are_bit_identical(self):
        self.assertEqual(ls.literal_spellings("0xFF1FFFFF"),
                         ["-0xE00001", "0xFF1FFFFFU", "~0xE00000", "~0xE00000U"])
        self.assertIn("0xFF1FFFFF", ls.literal_spellings("~0xE00000"))
        self.assertEqual(ls.literal_spellings("1U"), ["1"])
        self.assertEqual(ls.literal_spellings("8"), ["8U"], "no complement with 31 set bits")
        self.assertIn("0xFFFFFFFF", ls.literal_spellings("-1"))

    def test_cells_only_at_safe_sites(self):
        cells = ls.generate(interp(), ["literal_type"], None)
        edits = {(c.line, c.edit) for c in cells}
        self.assertIn((26, "1 -> 1U (left of ==)"), edits, "the loop header's gState == 1")
        self.assertIn((61, "0xFF1FFFFF -> ~0xE00000 (left of &)"), edits)
        self.assertIn((62, "0x800000U -> 0x800000 (left of |=)"), edits)
        self.assertIn((65, "0x400000 -> 0x400000U (left of |=)"), edits)
        self.assertIn((26, "0x7F00 -> 0x7F00U (left of !=)"), edits, "!= against a s16 field")
        self.assertFalse([e for e in edits if e[0] == 51], "gTime == 1 compares a float")
        self.assertFalse([e for e in edits if e[0] == 52], "stop(0) is a call argument")
        self.assertFalse([e for e in edits if " + 4" in e[1] or "4 ->" in e[1]], "cursor + 4 is arithmetic")
        self.assertTrue(all(c.semantics == "safe" for c in cells), {c.semantics for c in cells})
        self.assertTrue(all(c.edits for c in cells))


class CarrierRenameTests(unittest.TestCase):
    def test_cells_and_semantics(self):
        cells = {c.edit: c for c in ls.generate(interp(), ["carrier_rename"], None)}
        self.assertIn("opcode -> spare in case 0x22", cells)
        self.assertEqual(cells["opcode -> spare in case 0x22"].semantics, "safe")
        self.assertIn("fade(value / hundred, spare);", cells["opcode -> spare in case 0x22"].text)
        self.assertNotIn("opcode -> carry in case 0x22", cells)
        self.assertNotIn("carry -> spare in case 0x30", cells)
        self.assertNotIn("low -> spare in case 0x30", cells)
        self.assertIn("word -> spare in case 0x30", cells)
        self.assertEqual(cells["opcode -> half in case 0x22"].semantics, "check:carrier width s32 -> u16")
        self.assertIn("narrow -> narrow2 in case 0x40", cells)
        self.assertFalse([e for e in cells if "-> cursor" in e or "-> value" in e.split(" in ")[0] and "opcode" in e],
                         "no pointer or float target for an int")

    def test_region_filter(self):
        fn = interp()
        lines = ls.parse_case_filter(["0x30"], fn)
        cells = ls.generate(fn, ["carrier_rename"], lines)
        self.assertTrue(cells)
        self.assertTrue(all(c.region == "case 0x30" for c in cells))


class MaskShiftTests(unittest.TestCase):
    def test_introduce_move_and_drop(self):
        cells = {c.edit: c for c in ls.generate(interp(), ["mask_shift"], None)}
        a = cells["opcode & 0xFF introduced in the assignment (case 0x22)"]
        self.assertEqual(a.semantics, "safe", "fade takes a u8")
        self.assertIn("opcode = cursor->command & 0xFF;", a.text)
        b = cells["opcode & 0xFF introduced at the call (case 0x22)"]
        self.assertIn("fade(value / hundred, opcode & 0xFF);", b.text)
        self.assertNotIn("opcode & 0xFFFF introduced in the assignment (case 0x22)", cells)
        c = cells["word << 8: assignment -> call (case 0x30)"]
        self.assertEqual(c.semantics, "safe")
        self.assertIn("word = *((u16 *) ((u8 *) cursor + 6)) & 0xFF;", c.text)
        self.assertIn("trap(low, word << 8, carry);", c.text)
        d = cells["pathIndex & 0xFF: assignment -> call (case 0)"]
        self.assertIn("stop(pathIndex & 0xFF);", d.text)
        self.assertNotIn("pathIndex & 0xFF dropped: stop takes s32 (case 0)", cells)
        self.assertFalse([e for e in cells if e.startswith("low &")], "low has two uses")

    def test_prototypes(self):
        fn = interp()
        _, protos = ls.declarations_for(fn)
        self.assertEqual(protos["fade"], ["f32 v", "u8 which"])
        self.assertIsNone(protos["trap"], "K&R")
        self.assertEqual(ls.param_type(protos["fade"], 1), "u8")
        self.assertIsNone(ls.param_type(protos["trap"], 0))


class ReadOrderTests(unittest.TestCase):
    def test_runs_and_permutations(self):
        cells = ls.generate(interp(), ["read_order"], None)
        by = {}
        for c in cells:
            by.setdefault(c.region, []).append(c)
        self.assertEqual(sorted(by), ["case 0x22", "case 0x30"])
        self.assertEqual(len(by["case 0x22"]), 1)
        t = by["case 0x22"][0].text
        self.assertLess(t.index("value = (f32)"), t.index("opcode = cursor->command"))
        self.assertEqual(len(by["case 0x30"]), 1, "carry = carry + low is not a stream read")
        self.assertEqual(len(t.splitlines()), len(SWITCH_SRC.splitlines()))

    def test_long_run_is_sampled(self):
        src = SWITCH_SRC.replace("                opcode = cursor->command;\n",
                                 "".join(f"                {n} = cursor->command;\n"
                                         for n in ("opcode", "low", "word", "carry", "spare")))
        fn = ls.find_function(src, "interp")
        cells = [c for c in ls.generate(fn, ["read_order"], None) if c.region == "case 0x22"]
        n = 6  # five commands plus the value read: every swap, each read first and last
        self.assertEqual(len(cells), n * (n - 1) // 2 + (n - 1) + (n - 1) - 2,
                         "moving the second read first and the penultimate last repeat two swaps")

    def test_reorder_reasons(self):
        cells = {(c.line, c.semantics) for c in ls.generate(interp(), ["reorder"], None)}
        self.assertIn((34, "safe"), cells, "two loads into different locals")
        self.assertFalse([c for c in cells if c[0] == 35], "value reads cursor, cursor is rewritten")


class PairTests(unittest.TestCase):
    def test_pairs_within_the_outer_region(self):
        fn = interp()
        cells = ls.generate(fn, ["literal_type"], ls.parse_case_filter(["0x50"], fn))
        pairs = ls.compose_pairs(cells, fn, 100)
        edits = {c.edit for c in pairs}
        self.assertTrue(any("~0xE00000" in e and "0x800000U -> 0x800000" in e for e in edits),
                        "a cell in the nested switch pairs with one in its outer case")
        self.assertTrue(all(c.lever == "pair" and c.region == "case 0x50" for c in pairs))
        self.assertTrue(all(not ls.edits_overlap(c.edits[:1], c.edits[1:]) for c in pairs))


class DiagTests(unittest.TestCase):
    def test_decl_diag(self):
        out = ls.apply_diag(SWITCH_SRC, "interp", ["decl:narrow=s8"])
        self.assertIn("    s8 narrow;\n", out)
        self.assertEqual(len(out.splitlines()), len(SWITCH_SRC.splitlines()))
        with self.assertRaises(ls.DiagError):
            ls.apply_diag(SWITCH_SRC, "interp", ["decl:nothing=s8"])
        with self.assertRaises(ls.DiagError):
            ls.apply_diag(SWITCH_SRC, "interp", ["frob:x"])
        self.assertEqual(ls.parse_diag("decl:a=s8, decl:b=u8"), ["decl:a=s8", "decl:b=u8"])

    def test_spec_file(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "diag.txt"
            p.write_text("# pinned base\ndecl:narrow=s8\n")
            self.assertEqual(ls.parse_diag("@" + str(p)), ["decl:narrow=s8"])
            patch = Path(d) / "diag.patch"
            patch.write_text("--- a\n+++ b\n@@ -20,1 +20,1 @@\n-    s16 narrow;\n+    s8 narrow;\n")
            specs = ls.parse_diag("@" + str(patch))
            self.assertEqual(specs, ["patch:" + str(patch)])
            out = ls.apply_diag(SWITCH_SRC, "interp", specs)
            self.assertIn("    s8 narrow;\n", out)


def stub_scorer(text: str) -> dict:
    """A measurement stub: rewards the 0x22 mask and read order; off size unless narrow is s8."""
    residual = 10
    if "opcode = cursor->command & 0xFF;" in text or "fade(value / hundred, opcode & 0xFF);" in text:
        residual -= 3
    value_read, opcode_read = text.find("value = (f32)"), text.find("opcode = cursor->command")
    if 0 <= value_read < opcode_read:
        residual -= 2
    if "half = cursor->command" in text:
        residual -= 5  # a `check` carrier that scores best of all
    delta = 0 if "s8 narrow;" in text else 4
    return {"masked": residual + (0 if delta == 0 else 100), "delta": delta, "residual": residual,
            "buckets": [0, residual, 0, 0]}


class SourceSweepTests(unittest.TestCase):
    def sweep(self, **kw):
        logs = []
        res = ls.source_sweep(SWITCH_SRC, "interp", list(ls.SOURCE_LEVERS), stub_scorer, cases=["0x22"],
                              log=logs.append, jobs=2, **kw)
        return res, logs

    def test_double_measure_and_column(self):
        res, _ = self.sweep(diags=["decl:narrow=s8"])
        self.assertEqual(res["column"], "d", "the adoptable base is off size, so the diag column ranks")
        rows = res["rounds"][0]["cells"]
        self.assertTrue(all("a" in r and "d" in r for r in rows))
        self.assertTrue(all(r["a"]["delta"] == 4 and r["d"]["delta"] == 0 for r in rows))
        self.assertEqual(res["rounds"][0]["base"]["d"]["residual"], 10)
        best = rows[0]
        self.assertEqual(best["semantics"], "check:carrier width s32 -> u16", "ranked, never adopted")
        self.assertTrue(ls.improves(best, res["rounds"][0]["base"], "d"))
        res2, _ = self.sweep()
        self.assertEqual(res2["column"], "a")
        self.assertTrue(all("d" not in r for r in res2["rounds"][0]["cells"]))

    def test_greedy_adopts_safe_improving_cells_only(self):
        res, logs = self.sweep(diags=["decl:narrow=s8"], greedy=4)
        adopted = [r["adopted"]["edit"] for r in res["rounds"] if "adopted" in r]
        self.assertEqual(len(adopted), 2, adopted)
        self.assertTrue(adopted[0].startswith("opcode & 0xFF introduced in the assignment"))
        self.assertIn(adopted[1], ("swap with next", "reads in order 1,0"))
        self.assertIn("opcode = cursor->command & 0xFF;", res["final"])
        self.assertNotIn("half = cursor->command", res["final"])
        self.assertLess(res["final"].index("value = (f32)"), res["final"].index("opcode = cursor->command"))
        self.assertEqual(res["rounds"][-1]["base"]["d"]["residual"], 5)
        self.assertTrue(any("no safe improving cell" in l for l in logs))
        self.assertEqual(len(res["rounds"]), 3)

    def test_pairs_reach_the_combination_in_one_round(self):
        res, _ = self.sweep(diags=["decl:narrow=s8"], pairs=50)
        rows = [r for r in res["rounds"][0]["cells"] if r["lever"] == "pair" and r["semantics"] == "safe"]
        self.assertTrue(any(r["d"]["residual"] == 5 for r in rows))

    def test_function_diff_and_emit(self):
        res, _ = self.sweep(greedy=2)
        diff = ls.function_diff(SWITCH_SRC, res["final"], "interp")
        self.assertIn("-                opcode = cursor->command;", diff)
        self.assertIn("+                opcode = cursor->command & 0xFF;", diff)
        self.assertRegex(diff, r"@@ -3\d,\d+ \+3\d,\d+ @@", "hunk numbers are whole-TU line numbers")
        with tempfile.TemporaryDirectory() as d:
            out = Path(d) / "out.c"
            ls.emit_frozen("#define SHAPE_k 1\n" + SWITCH_SRC.replace(
                "    carry = 0;\n", "#if SHAPE_k == 0\n    carry = 1;\n#else\n    carry = 0;\n#endif\n"),
                out, SWITCH_SRC, "interp")
            self.assertEqual(out.read_text(), SWITCH_SRC, "axes resolved, nothing else changed")
            self.assertEqual(Path(str(out) + ".diff").read_text(), "")

    def test_edit_cells_apply_and_overlap(self):
        self.assertEqual(ls.apply_edits("abcdef", [(1, 2, "X"), (4, 4, "Y")]), "aXcdYef")
        self.assertTrue(ls.edits_overlap([(1, 3, "")], [(2, 5, "")]))
        self.assertFalse(ls.edits_overlap([(1, 3, "")], [(3, 5, "")]))
        self.assertTrue(ls.edits_overlap([(3, 3, "")], [(3, 3, "")]), "two insertions at one point")


class ComposeTests(unittest.TestCase):
    def test_edit_inside_a_swapped_statement_is_folded(self):
        fn = interp()
        cells = ls.generate(fn, ["reorder", "mask_shift"], ls.parse_case_filter(["0x22"], fn))
        swap = next(c for c in cells if c.lever == "reorder")
        mask = next(c for c in cells if c.edit.startswith("opcode & 0xFF introduced in the assignment"))
        edits = ls.compose_edits(fn, swap, mask)
        self.assertIsNotNone(edits, "the mask lives inside the swapped statement: folded, not refused")
        text = ls.apply_edits(fn.text, edits)
        self.assertIn("                value = (f32) *((u16 *) ((u8 *) cursor + 4));\n"
                      "                opcode = cursor->command & 0xFF;\n", text)
        self.assertEqual(ls.compose_edits(fn, mask, swap), edits)
        drop = next((c for c in cells if "dropped" in c.edit), None)
        self.assertIsNone(drop, "stop takes s32: no mask to drop")
        pairs = {c.edit for c in ls.compose_pairs(cells, fn, 100)}
        self.assertTrue(any("swap with next" in e and "introduced in the assignment" in e for e in pairs))

    def test_improves_accepts_adoptable_size_zero(self):
        base = {"a": {"residual": 107, "delta": 4, "masked": 859}, "d": {"residual": 103, "delta": 0, "masked": 134}}
        swap = {"a": {"residual": 103, "delta": 0, "masked": 103}, "d": {"residual": 107, "delta": -4, "masked": 1491}}
        self.assertTrue(ls.improves(swap, base, "d"), "closer to size on the adoptable column")
        worse = {"a": {"residual": 107, "delta": 4, "masked": 859}, "d": {"residual": 104, "delta": 0, "masked": 140}}
        self.assertFalse(ls.improves(worse, base, "d"))
        better_d = {"a": {"residual": 107, "delta": 4, "masked": 859}, "d": {"residual": 99, "delta": 0, "masked": 120}}
        self.assertTrue(ls.improves(better_d, base, "d"))
        self.assertFalse(ls.improves({"a": {"error": "x"}}, base, "a"))
        base["a"]["onesided"] = 5
        cancelled = {"a": {"residual": 101, "delta": 0, "masked": 739, "onesided": 6},
                     "d": {"residual": 105, "delta": -4, "masked": 1482}}
        self.assertFalse(ls.improves(cancelled, base, "d"),
                         "size 0 with more one-sided words is a long handler cancelling a short one")


V3_RECORDS = """\
[CDX] force_verdict phase=p1 site=save proc=3 web=5 lr=0xa force=y save=0.000000->0.000100 verdict=2->1
[CDX] force_verdict phase=p1 site=dec proc=3 web=5 lr=0xa force=y decision=split->color bestcolor=21 regsleft=9
[CDX] force_verdict phase=p1 site=dec proc=3 web=6 lr=0xb force=y declined=no-regs
[CDX] force_weight phase=p1 proc=3 web=9 lr=0xc pricing=1 bb=2 usesdefs=1.0->2.0
[CDX] force_grow phase=p1 proc=3 web=5 lr=0xa bb=2 refused
[CDX] force_verdict phase=p1 site=save proc=4 web=7 lr=0xd force=n save=3.0->0 verdict=1->2
"""


class V3ForceTests(unittest.TestCase):
    def test_parse_grammar(self):
        got = ls.parse_oracle("p1:w851.2=y, p2:w3=n, p1:w9=y4.5, p1:w387=s")
        self.assertEqual(got, [("p1", 851, "y", 2), ("p2", 3, "n", None), ("p1", 9, "y4.5", None),
                               ("p1", 387, "s", None)])
        self.assertEqual([f.env for f in got], ["p1:w851.2=y", "p2:w3=n", "p1:w9=y4.5", "p1:w387=s"])
        self.assertEqual([f.classic for f in got], [False, False, False, True])
        self.assertFalse(ls.parse_oracle("p1:w9.2=c21")[0].classic, "a pricing qualifier is v3 grammar")
        for bad in ("p1:w9=x", "p1:w9.=y", "w9=y", "p3:w9=y"):
            with self.assertRaises(SystemExit):
                ls.parse_oracle(bad)
        self.assertEqual(ls.parse_weight("p1:w851:bb258=+1, p1:w851.2:bb259=-2"),
                         [("p1", 851, 258, 1, None), ("p1", 851, 259, -2, 2)])
        self.assertEqual(ls.parse_weight("p1:w851.2:bb259=-2")[0].env, "p1:w851.2:bb259=-2")
        with self.assertRaises(SystemExit):
            ls.parse_weight("p1:w851:bb258=1")
        grows = ls.parse_grow("p1:w851=seed, p1:w852=-bb259-bb260")
        self.assertEqual(grows, [("p1", 851, None), ("p1", 852, (259, 260))])
        self.assertEqual([g.env for g in grows], ["p1:w851=seed", "p1:w852=-bb259-bb260"])
        with self.assertRaises(SystemExit):
            ls.parse_grow("p1:w851=bb259")

    def test_records_and_acceptance(self):
        recs = ls.force_records(V3_RECORDS)
        self.assertEqual([k for k, _ in recs], ["force_verdict"] * 3 + ["force_weight", "force_grow",
                                                                         "force_verdict"])
        self.assertTrue(recs[4][1]["refused"])
        self.assertEqual(recs[2][1]["declined"], "no-regs")
        ok = ls.parse_oracle("p1:w5=y")
        self.assertIsNone(ls.v3_force_acceptance(V3_RECORDS, 3, ok, [], []))
        self.assertIn("declined", ls.v3_force_acceptance(V3_RECORDS, 3, ls.parse_oracle("p1:w6=y"), [], []))
        self.assertIn("no force_verdict", ls.v3_force_acceptance(V3_RECORDS, 3, ls.parse_oracle("p1:w7=n"), [], []),
                      "web 7 was forced in another procedure")
        self.assertIsNone(ls.v3_force_acceptance(V3_RECORDS, 4, ls.parse_oracle("p1:w7=n"), [], []))
        self.assertIn("disagree", ls.v3_force_acceptance(V3_RECORDS, 3, ls.parse_oracle("p1:w5=n"), [], []))
        self.assertIsNone(ls.v3_force_acceptance(V3_RECORDS, 3, ls.parse_oracle("p1:w387=s"), [], []),
                          "classic forces are force_lattice's to accept")
        self.assertIsNone(ls.v3_force_acceptance(V3_RECORDS, 3, [], ls.parse_weight("p1:w9.1:bb2=+1"), []))
        self.assertIn("no force_weight", ls.v3_force_acceptance(V3_RECORDS, 3, [], ls.parse_weight("p1:w9:bb3=+1"), []))
        self.assertIsNone(ls.v3_force_acceptance(V3_RECORDS, 3, [], [], ls.parse_grow("p1:w5=seed")))
        self.assertIsNone(ls.v3_force_acceptance(V3_RECORDS, 3, [], [], ls.parse_grow("p1:w5=-bb2")))
        self.assertIn("none of the named", ls.v3_force_acceptance(V3_RECORDS, 3, [], [], ls.parse_grow("p1:w5=-bb9")))
        self.assertIn("no force_grow", ls.v3_force_acceptance(V3_RECORDS, 3, [], [], ls.parse_grow("p1:w8=seed")))

    def test_targets_from_verdict_pricing_and_state(self):
        decs, _ = ls.decisions(RECORDS, 3)  # web 5: split row then colour row c21; web 9: c19
        t = ls.oracle_targets(decs, ls.parse_oracle("p1:w5=y"))
        self.assertEqual((t[0]["want"], t[0]["spec"], len(t[0]["rows"])), ("c21", "p1:w5=y", 1),
                         "a verdict force reproduces the colour the forced base took")
        t = ls.oracle_targets(decs, ls.parse_oracle("p1:w5.1=n"))
        self.assertEqual((t[0]["want"], t[0]["rows"][0]["decision"]), ("s", "split"))
        t = ls.oracle_targets(decs, ls.parse_oracle("p1:w5.2=c21"))
        self.assertEqual(t[0]["rows"][0]["lines"], {12, 13}, "pricing 2 is the second decision row")
        with self.assertRaises(SystemExit):
            ls.oracle_targets(decs, ls.parse_oracle("p1:w5.3=y"))
        st = ls.state_targets(decs, ls.parse_weight("p2:w9:bb2=+1"), ls.parse_grow("p1:w5=seed"))
        self.assertEqual([(s["want"], s["spec"]) for s in st], [("c19", "p2:w9:bb2=+1 -> c19"), ("c21", "p1:w5=seed -> c21")])
        hits, _ = ls.oracle_status(decs, st)
        self.assertEqual(hits, 2, "the forced base reproduces its own state")


if __name__ == "__main__":
    unittest.main()
