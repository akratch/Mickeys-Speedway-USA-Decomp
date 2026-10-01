"""Fixture tests for tools/shape_lint.py (no build needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import shape_lint  # noqa: E402

SRC = """
extern f32 gScaleA;
extern f32 gScaleB;
extern void *gEntries[];
extern void *gShiftEntries[];
#ifdef NON_MATCHING
void target(s32 owner) {
    union { s32 a; void *b; } tail;
    volatile s32 *p;
    owner = gCount;
    value = (gTimer -= amount);
    if ((flags << 4) >= 0) { }
    x = 0;
    if (c) {
        x = f();
    }
scan:
    goto scan;
    cmd->w0 = 0xFA000000;
    y = (u32)y | 0;
    if (1) { }
    z = gScaleA * gScaleB;
}
#else
#pragma GLOBAL_ASM("x.s")
#endif
"""


class LintTests(unittest.TestCase):
    def findings(self, relocs):
        return shape_lint.lint("target", "src/x.c", SRC, relocs)

    def test_detects_each_checklist_item(self):
        items = {f["item"] for f in self.findings(None)}
        for item in (0, 1, 3, 4, 5, 6, 7, 8):
            self.assertIn(item, items, f"checklist item {item} not detected")

    def test_float_externs_without_records_are_pool_literals(self):
        fs = self.findings([("R_MIPS_HI16", "gScaleA")])
        pool = [f for f in fs if f["item"] == 2]
        self.assertEqual(len(pool), 1)
        self.assertIn("gScaleB", pool[0]["finding"])
        self.assertNotIn("gScaleA", pool[0]["finding"])

    def test_body_is_scoped_to_the_symbol(self):
        body = shape_lint.candidate_body(SRC, "target")
        self.assertTrue(body.startswith("#ifdef NON_MATCHING"))
        self.assertNotIn("GLOBAL_ASM", body)


if __name__ == "__main__":
    unittest.main()
