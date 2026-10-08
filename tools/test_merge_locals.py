"""Fixture tests for tools/merge_locals.py (synthetic C and objdump text; no build, no ROM content)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lever_sweep as ls  # noqa: E402
import merge_locals as ml  # noqa: E402

SRC = """\
typedef struct Box { int a; int b; int c; } Box;
extern void use(int v);
extern int gCount;

#ifdef NON_MATCHING
int fixture(Box *box) {
    int x, y;
    int z = 3;
    Box *p;
    float f;

    x = box->a;
    y = box->b;
    use(x + y);
    p = box;
    gCount = p->c + z;
    return y;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/fixture.s")
#endif
"""


class Text(unittest.TestCase):
    def test_rename_skips_members(self):
        self.assertEqual(ml.rename_str("x = box->x + p.x + x;", "x", "q"), "q = box->x + p.x + q;")

    def test_drop_declarator(self):
        self.assertEqual(ml.drop_declarator("    int x, y;", "y"), "    int x;")
        self.assertEqual(ml.drop_declarator("    int x, y;", "x"), "    int y;")
        self.assertEqual(ml.drop_declarator("    int x;", "x"), "")
        self.assertEqual(ml.drop_declarator("    Box *p, *q;", "p"), "    Box *q;")
        self.assertIsNone(ml.drop_declarator("    int z = 3;", "z"))     # initializer
        self.assertIsNone(ml.drop_declarator("    int x;", "y"))         # absent

    def test_merge_text_over_function(self):
        fn = ls.find_function(SRC, "fixture")
        decl = fn.locals["y"]["decl"]
        out = ml.merge_text(SRC, (fn.body.start + 1, fn.body.end), (decl.start, decl.end), "x", "y")
        self.assertIn("    int x;\n", out)
        self.assertNotIn("y", out.split("int fixture", 1)[1].replace("typedef", "").replace("gCount", ""))
        self.assertIn("x = box->b;", out)
        self.assertIn("return x;", out)

    def test_keep_decl_leaves_declaration(self):
        fn = ls.find_function(SRC, "fixture")
        decl = fn.locals["y"]["decl"]
        out = ml.merge_text(SRC, (fn.body.start + 1, fn.body.end), (decl.start, decl.end), "x", "y", drop_decl=False)
        self.assertIn("int x, y;", out)
        self.assertIn("return x;", out)

    def test_initializer_cell_is_refused(self):
        fn = ls.find_function(SRC, "fixture")
        decl = fn.locals["z"]["decl"]
        self.assertIsNone(ml.merge_text(SRC, (fn.body.start + 1, fn.body.end), (decl.start, decl.end), "x", "z"))

    def test_pairs_are_same_typed_only(self):
        fn = ls.find_function(SRC, "fixture")
        pairs = ml.candidate_pairs(fn, ls)
        self.assertIn(("x", "y"), pairs)
        self.assertIn(("y", "x"), pairs)
        self.assertNotIn(("x", "p"), pairs)
        self.assertNotIn(("x", "f"), pairs)
        self.assertEqual(len(pairs), 6)   # x, y, z ordered pairs


OBJ = """\
00000000 <fixture>:
   0:\t27bdffe0 \taddiu\tsp,sp,-32
   4:\tafbf001c \tsw\tra,28(sp)
   8:\t27a40018 \taddiu\ta0,sp,24
   c:\t0c000000 \tjal\t0 <fixture>
\t\t\tc: R_MIPS_26\tuse
  10:\t8fbf001c \tlw\tra,28(sp)
  14:\t27bd0020 \taddiu\tsp,sp,32
  18:\t03e00008 \tjr\tra
00000020 <other>:
  20:\t00000000 \tnop
"""


class Objects(unittest.TestCase):
    def test_rows_keep_relocations_and_stop_at_next_symbol(self):
        rows = ml.function_rows(OBJ, "fixture")
        self.assertEqual(len(rows), 7)
        self.assertTrue(rows[3].endswith("@use"))
        self.assertNotIn("nop", " ".join(rows))

    def test_normalize_masks_stack_offsets_and_frame(self):
        rows = ml.function_rows(OBJ, "fixture")
        norm = ml.normalize_listing(rows)
        self.assertEqual(norm[0], "addiu sp,sp,N")
        self.assertEqual(norm[1], "sw ra,N(sp)")
        self.assertEqual(norm[2], "addiu a0,sp,N")
        self.assertEqual(ml.frame_size(rows), 32)

    def test_classify(self):
        base = ml.function_rows(OBJ, "fixture")
        self.assertEqual(ml.classify(base, list(base)), "inert")
        moved = [r.replace("28(sp)", "44(sp)").replace("-32", "-48").replace(",32", ",48") for r in base]
        self.assertEqual(ml.classify(base, moved), "frame-only")
        changed = list(base)
        changed[6] = "jr t0"
        self.assertEqual(ml.classify(base, changed), "code-changing")


if __name__ == "__main__":
    unittest.main()
