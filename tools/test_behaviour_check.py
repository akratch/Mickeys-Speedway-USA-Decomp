"""Tests for tools/behaviour_check.py: source preparation, the IR passes, and
end-to-end runs on small synthetic translation units (host cc, no build)."""

import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import behaviour_check as bc  # noqa: E402

HAVE_CC = shutil.which("cc") is not None


class PrepareSourceTests(unittest.TestCase):
    def test_pragma_weak_lines_are_blanked(self):
        src = "int a;\n#pragma weak aliasA = target\nint b;\n"
        out = bc.prepare_source(src)
        self.assertEqual(out.split("\n"), ["int a;", "", "int b;", ""])

    def test_knr_definition_takes_the_promoted_type(self):
        src = "void f(k)\nu8 k;\n{\n    use(k);\n}\n"
        out = bc.prepare_source(src).split("\n")
        self.assertEqual(len(out), len(src.split("\n")), "line numbers are preserved")
        self.assertEqual(out[0], "void f(int __bh_knr_k)")
        self.assertEqual(out[1], "")
        self.assertEqual(out[2], "{ u8 k = (u8) __bh_knr_k;")

    def test_knr_wide_parameter_is_kept(self):
        out = bc.prepare_source("s32 g(p, n)\nchar *p;\ns32 n;\n{\n}\n").split("\n")
        self.assertEqual(out[0], "s32 g(char * p, s32 n)".replace("* p", "* p"))
        self.assertEqual(out[3], "{")

    def test_narrow_prototype_of_knr_function_loses_its_list_but_calls_do_not(self):
        src = ("void f(k)\nu8 k;\n{\n}\n"
               "void caller(void) {\n    void f(u8 k);\n    f(stream[2]);\n    x = f2(1);\n}\n")
        out = bc.prepare_source(src)
        self.assertIn("    void f();", out)
        self.assertIn("    f(stream[2]);", out)

    def test_not_a_definition(self):
        src = "if (x)\n{\n}\n"
        self.assertEqual(bc.prepare_source(src), src)


class IRTests(unittest.TestCase):
    def test_split_top(self):
        self.assertEqual(bc.split_top("i32 1, { i8, i16 } { i8 1, i16 2 }, ptr @x"),
                         ["i32 1", "{ i8, i16 } { i8 1, i16 2 }", "ptr @x"])
        self.assertEqual(bc.split_top('[2 x i8] c"a,b", i32 3'), ['[2 x i8] c"a,b"', "i32 3"])

    def test_type_round_trip(self):
        for t in ("i32", "ptr", "[4 x %struct.S]", "{ i8, [3 x i8], float, ptr }", "<{ i8, i32 }>",
                  "[0 x [2 x i16]]"):
            node, end = bc.parse_type(t)
            self.assertEqual(end, len(t))
            self.assertEqual(bc.type_text(node), t)

    def test_layout_and_plan(self):
        lay = bc.Layout({"%struct.S": "type { i8, i32, ptr }".split(" ", 1)[1]})
        size, align, plan = lay.of(bc.parse_type("%struct.S")[0])
        self.assertEqual((size, align), (16, 8))
        self.assertEqual(bc.plan_text(plan), "r8z8")
        size, _, plan = lay.of(bc.parse_type("[3 x { float, ptr }]")[0])
        self.assertEqual(size, 48)
        self.assertEqual(bc.plan_text(plan), "*3(fr4z8)")
        self.assertEqual(bc.plan_text(lay.of(bc.parse_type("[5 x ptr]")[0])[2]), "z40")
        self.assertEqual(lay.of(bc.parse_type("<{ i8, i32 }>")[0])[:2], (5, 1))

    def test_swap_constant(self):
        lay = bc.Layout({})
        node, text = bc.swap_constant(bc.parse_type("[2 x i16]")[0], "[i16 1, i16 256]", lay)
        self.assertEqual((bc.type_text(node), text), ("[2 x i16]", "[i16 256, i16 1]"))
        node, text = bc.swap_constant(("float",), "1.000000e+00", lay)
        self.assertEqual((bc.type_text(node), text), ("i32", str(0x0000803F)))
        node, text = bc.swap_constant(bc.parse_type("{ i32, float, ptr }")[0],
                                      "{ i32 1, float 0.000000e+00, ptr @x }", lay)
        self.assertEqual(bc.type_text(node), "{ i32, i32, ptr }")
        self.assertEqual(text, "{ i32 16777216, i32 0, ptr @x }")

    def test_big_endian_transform_swaps_wide_accesses_only(self):
        unit = bc.Unit("\n".join([
            "define void @f(ptr %p) {",
            "  %1 = load i16, ptr %p, align 2",
            "  %2 = load i8, ptr %p, align 1",
            "  %3 = load float, ptr %p, align 4",
            "  store i32 7, ptr %p, align 4",
            "  store ptr null, ptr %p, align 8",
            "  ret void",
            "}",
            "@t = internal constant [2 x i16] [i16 1, i16 2], align 2",
        ]))
        counts = bc.big_endian_transform(unit)
        text = unit.text()
        self.assertEqual(counts, {"loads": 2, "stores": 1, "initializers": 1})
        self.assertIn("%1 = call i16 @llvm.bswap.i16(i16 %bhl.0)", text)
        self.assertIn("%2 = load i8, ptr %p, align 1", text)
        self.assertIn("%3 = bitcast i32 %bhl.1.s to float", text)
        self.assertIn("store ptr null, ptr %p, align 8", text)
        self.assertIn("[2 x i16] [i16 256, i16 512]", text)

    def test_big_endian_transform_refuses_aggregate_loads(self):
        unit = bc.Unit("  %1 = load %struct.S, ptr %p, align 4")
        with self.assertRaises(bc.BuildError):
            bc.big_endian_transform(unit)


TU = r"""
#include "PR/ultratypes.h"

typedef struct Rec {
    u16 a;
    u8 b;
    s8 c;
    f32 f;
    struct Rec *next;
} Rec;

extern Rec gRecs[4];
extern s32 gTable[];
extern f32 gScale;
extern s32 gFlag;
s32 Trap();
#pragma weak aliasOne = Trap
extern void aliasOne(Rec *r, f32 x, u8 k);
void sink(u16 v, s8 w, f32 x, void **slot);
u8 mode(void);

void knr(k)
u8 k;
{
    gTable[k & 3] += k;
}

s32 target(u16 *stream, Rec *r)
{
    s32 v;
    s16 hi;
    hi = (stream[1] >> 8) & 0xFF;
    v = Trap(hi, stream[0], 1.5f);
    Trap();
    aliasOne(r, gScale * 2.0f, (u8) stream[2]);
    sink(stream[0], (s8) stream[1], 0.5f, (void **) &gRecs[2].next);
    knr(stream[2]);
    r->a = stream[0] + gTable[1];
    r->f = gScale + (f32) mode();
    gRecs[1].next = r;
    return v + LOWBYTE(stream);
}

void spin(void)
{
    while (gFlag == 1) {
    }
}
"""

DRIVER = r"""
#define BEHAVIOUR_AUTO_STUBS 1
%(be)s
#include "behaviour_rt.h"
#include BEHAVIOUR_TU

int main(void)
{
    int s;
    for (s = 0; s < 3; s++) {
        u16 *st;
        Rec *r;
        bh_reset();
        bh_scenario("s%%d", s);
        st = bh_alloc(16);
        r = bh_alloc(sizeof(Rec));
        st[0] = 0x1234 + s;
        st[1] = 0x80FF - s;
        st[2] = 0x7F00 | s;
        bh_log("ret %%d gTable1 %%d", (int) target(st, r), (int) gTable[1]);
        bh_dump_mem("rec", r, sizeof(Rec));
        bh_dump_globals();
    }
    bh_reset();
    bh_scenario("spin");
    gFlag = 1;
    bh_watchdog(2000, (volatile int32_t *) &gFlag, 1);
    spin();
    bh_log("watchdog %%d flag %%d", bh_watchdog_disarm(), (int) gFlag);
    return 0;
}
"""


@unittest.skipUnless(HAVE_CC, "no host C compiler")
class EndToEndTests(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="test_behaviour_"))
        self.saved_cache = bc.CACHE_DIR
        bc.CACHE_DIR = self.tmp / "cache"

    def tearDown(self):
        bc.CACHE_DIR = self.saved_cache
        shutil.rmtree(self.tmp, ignore_errors=True)

    def write(self, name, text):
        p = self.tmp / name
        p.write_text(text)
        return p

    def tu(self, name, lowbyte="(*((u8 *) stream + 1))", extra=""):
        return self.write(name, f"#define LOWBYTE(s) {lowbyte}\n{extra}" + TU)

    def driver(self, be=False):
        return self.write("drv_be.c" if be else "drv.c", DRIVER % {"be": "#define BEHAVIOUR_BIG_ENDIAN 1" if be else ""})

    def test_auto_stubs_record_every_call_with_its_own_arity(self):
        ok, out = bc.run_tu(self.tu("a.c"), self.driver())
        self.assertTrue(ok, out)
        lines = out.split("\n")
        self.assertEqual(lines[0], "== s0")
        # Unprototyped: promoted arguments, the call site's own count.
        self.assertEqual(lines[1].split(" -> ")[0], "call Trap(i32 0x80, i32 0x1234, f64 1.5[3ff8000000000000])")
        self.assertEqual(lines[2].split(" -> ")[0], "call Trap()")
        # A #pragma weak alias is stubbed under its own name, after its prototype's conversions.
        self.assertTrue(lines[3].startswith("call aliasOne(p arena+0x10, f32 "), lines[3])
        self.assertTrue(lines[3].endswith("u8 0x0)"), lines[3])
        # A pointer into a generated global is printed by name.
        self.assertEqual(lines[4], "call sink(u16 0x1234, s8 0xff, f32 0.5[3f000000], p gRecs+0x28)")
        self.assertTrue(lines[5].startswith("call mode() -> 0x"))
        self.assertIn("== spin", lines)
        self.assertIn("watchdog 1 flag 0", lines)

    def test_reset_restores_globals_between_scenarios(self):
        ok, out = bc.run_tu(self.tu("a.c"), self.driver())
        self.assertTrue(ok, out)
        tables = [ln.split(" gTable1 ")[1] for ln in out.split("\n") if " gTable1 " in ln]
        # knr adds k & 3 into gTable[k & 3]: only scenario 1 (k = 1) touches gTable[1].
        self.assertEqual(int(tables[1]), int(tables[0]) + 1)
        self.assertEqual(tables[2], tables[0])

    def test_deterministic_across_builds(self):
        a = bc.run_tu(self.tu("a.c"), self.driver(), cache=False)
        b = bc.run_tu(self.tu("b.c", extra="static int unrelated[3] = {1, 2, 3};\n"), self.driver(), cache=False)
        self.assertTrue(a[0] and b[0])
        self.assertEqual(a[1], b[1])

    def test_same_different_failure(self):
        drv = self.driver()
        ref = self.tu("ref.c")
        same = self.write("same.c", ref.read_text().replace("hi = (stream[1] >> 8) & 0xFF;",
                                                            "hi = stream[1] >> 8;"))
        diff = self.write("diff.c", ref.read_text().replace("hi = (stream[1] >> 8) & 0xFF;",
                                                            "hi = (s8) (stream[1] >> 8);"))
        bad = self.write("bad.c", "this is not C")
        self.assertEqual(bc.same_behaviour(same, ref, drv)[0], 0)
        status, msg = bc.same_behaviour(diff, ref, drv, detail=True)
        self.assertEqual(status, 1)
        self.assertIn("call Trap(i32 0xffffff80", msg)
        self.assertEqual(bc.same_behaviour(bad, ref, drv)[0], 2)

    def test_big_endian_reads_the_byte_the_target_reads(self):
        byte_load = self.tu("byte.c", "(*((u8 *) stream + 1))")
        mask = self.tu("mask.c", "(*(u16 *) stream & 0xFF)")
        high = self.tu("high.c", "(*(u16 *) stream >> 8)")
        # Big-endian: the byte at +1 is the low byte of the halfword at +0.
        self.assertEqual(bc.same_behaviour(byte_load, mask, self.driver(be=True))[0], 0)
        self.assertEqual(bc.same_behaviour(byte_load, high, self.driver(be=True))[0], 1)
        # A little-endian host would have judged both the other way round.
        self.assertEqual(bc.same_behaviour(byte_load, mask, self.driver())[0], 1)
        self.assertEqual(bc.same_behaviour(byte_load, high, self.driver())[0], 0)

    def test_separate_driver_without_markers_still_works(self):
        src = self.write("s.c", "typedef int s32;\ns32 twice(s32 x) { return x * 2; }\n")
        drv = self.write("sd.c", '#include <stdio.h>\nint twice(int);\n'
                                 'int main(void) { printf("%d\\n", twice(21)); return 0; }\n')
        self.assertEqual(bc.run_tu(src, drv), (True, "42\n"))

    def test_big_endian_needs_an_including_driver(self):
        src = self.write("s.c", "int f(void) { return 1; }\n")
        drv = self.write("sd.c", "#define BEHAVIOUR_BIG_ENDIAN 1\nint main(void) { return 0; }\n")
        ok, msg = bc.run_tu(src, drv)
        self.assertFalse(ok)
        self.assertIn("BEHAVIOUR_TU", msg)


if __name__ == "__main__":
    unittest.main()
