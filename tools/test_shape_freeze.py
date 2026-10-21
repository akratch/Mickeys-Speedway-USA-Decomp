"""Fixture tests for tools/shape_freeze.py (synthetic C; no build needed)."""

import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import shape_freeze as sf  # noqa: E402
import shape_product as sp  # noqa: E402

SRC = """\
#define SHAPE_a 1
#define T45 timer
#define amTunePlay amTunePlay
#ifdef NON_MATCHING
int f(int x) {
    int timer;
#if SHAPE_a == 0
    timer = x;
#elif SHAPE_a == 1
    T45 = x + 1;
#else
    timer = x + 2;
#endif
#if SHAPE_b == 0
    return T45;
#else
#ifdef EXTRA
    return T45 * 3;
#endif
    return T45 * 2;
#endif
}
#else
#pragma GLOBAL_ASM("asm/f.s")
#endif
"""

FROZEN_B1 = """\
#define amTunePlay amTunePlay
#ifdef NON_MATCHING
int f(int x) {
    int timer;
    timer = x + 1;
#ifdef EXTRA
    return timer * 3;
#endif
    return timer * 2;
}
#else
#pragma GLOBAL_ASM("asm/f.s")
#endif
"""


class EvaluateTests(unittest.TestCase):
    def test_operators(self):
        v = {"a": 1, "b": 0}
        self.assertTrue(sf.evaluate("SHAPE_a == 1", v))
        self.assertFalse(sf.evaluate("SHAPE_a == 0", v))
        self.assertTrue(sf.evaluate("SHAPE_a == 1 && SHAPE_b == 0", v))
        self.assertTrue(sf.evaluate("SHAPE_a == 2 || !SHAPE_b", v))
        self.assertTrue(sf.evaluate("(SHAPE_a + SHAPE_b) >= 1", v))
        self.assertFalse(sf.evaluate("SHAPE_a != 1", v))

    def test_refuses_what_it_cannot_evaluate(self):
        with self.assertRaises(sf.FreezeError):
            sf.evaluate("defined(SHAPE_a)", {"a": 1})
        with self.assertRaises(sf.FreezeError):
            sf.evaluate("SHAPE_a == FOO", {"a": 1})
        with self.assertRaises(sf.FreezeError):
            sf.evaluate("SHAPE_zz == 1", {})
        self.assertFalse(sf.evaluate("SHAPE_zz == 1", {}, default_zero=True))


class FreezeTests(unittest.TestCase):
    def test_pinned_define_cell_override_and_passthrough(self):
        self.assertEqual(sf.freeze_text(SRC, {"b": 1}), FROZEN_B1)
        out = sf.freeze_text(SRC, {"a": 2, "b": 0})
        self.assertIn("    timer = x + 2;\n    return timer;\n", out)
        self.assertNotIn("SHAPE_", out)

    def test_unpinned_axis_is_an_error(self):
        with self.assertRaises(sf.FreezeError):
            sf.freeze_text(SRC, {})
        out = sf.freeze_text(SRC, {}, default_zero=True)
        self.assertIn("    return timer;\n", out)

    def test_macro_handling(self):
        kept = sf.freeze_text(SRC, {"b": 1}, expand=False)
        self.assertIn("#define T45 timer\n", kept)
        self.assertIn("T45 = x + 1;", kept)
        self.assertIn("#define amTunePlay amTunePlay\n", sf.freeze_text(SRC, {"b": 1}),
                      "a self-define guard is not a carrier macro")
        protected = sf.freeze_text(SRC, {"b": 1}, keep={"T45"})
        self.assertIn("#define T45 timer\n", protected)
        self.assertEqual(sf.expand_carriers("#define T1 a\nx = T1 + s.T1 + p->T1 + T10;\n"),
                         "x = a + s.T1 + p->T1 + T10;\n")

    def test_axis_outside_a_conditional_is_an_error(self):
        with self.assertRaises(sf.FreezeError):
            sf.freeze_text("int x = SHAPE_a;\n", {"a": 1})

    def test_cli(self):
        with tempfile.TemporaryDirectory() as d:
            src = Path(d) / "cand.c"
            src.write_text(SRC)
            out = Path(d) / "out.c"
            self.assertEqual(sf.main([str(src), "--cell", "b=1", "-o", str(out)]), 0)
            self.assertEqual(out.read_text(), FROZEN_B1)
            self.assertEqual(sf.parse_cell("SHAPE_a=2, b=0"), {"a": 2, "b": 0})

    @unittest.skipUnless(shutil.which("unifdef"), "unifdef not installed")
    def test_agrees_with_unifdef(self):
        text = "".join(l for l in SRC.splitlines(keepends=True) if not l.startswith("#define SHAPE_"))
        r = subprocess.run(["unifdef", "-x", "2", "-DSHAPE_a=1", "-DSHAPE_b=1"], input=text,
                           capture_output=True, text=True)
        theirs = sf.expand_carriers(r.stdout)
        self.assertEqual("".join(theirs.split()), "".join(FROZEN_B1.split()))


class ShapeProductEmitTests(unittest.TestCase):
    def test_emit_best_text(self):
        text = sf.freeze_text(SRC, {"b": 1})
        self.assertEqual(sp.frozen_candidate(SRC, {"b": 1}), text)


if __name__ == "__main__":
    unittest.main()
