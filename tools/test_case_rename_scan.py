"""Fixture tests for tools/case_rename_scan.py (no build needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import case_rename_scan as crs  # noqa: E402

SRC = """
void f(void) {
    s32 a;
    s32 b;
    s32 k;
    f32 x;
    Thing *p;
    k = 3; /* b is named here only in a comment */
    switch (g) {
        case 0:
            a = 1;
            use(a);
            break;
        case 0x1A:
            b = 2;
            switch (a) {
                case 1:
                    b++;
                    break;
            }
            use(b);
            break;
    }
}
"""


class CaseRenameTests(unittest.TestCase):
    def test_locals_and_kinds(self):
        locs = crs.declared_locals(SRC, "f")
        self.assertEqual(locs, {"a": "s32", "b": "s32", "k": "s32", "x": "f32", "p": "Thing*"})
        self.assertEqual(crs.kind(locs["x"]), "f")
        self.assertNotEqual(crs.kind(locs["p"]), crs.kind(locs["a"]))

    def test_top_level_cases_only(self):
        blocks = crs.case_blocks(SRC, "f")
        self.assertEqual(sorted(blocks), ["0", "0x1A"])
        st, en = blocks["0x1A"]
        self.assertIn("use(b)", SRC[st:en])
        self.assertIn("b++", SRC[st:en])

    def test_apply_renames_one_case(self):
        out = crs.apply_moves(SRC, "f", [("0", "a", "b")])
        self.assertIn("b = 1;\n            use(b);", out)
        self.assertIn("switch (a)", out)

    def test_refuses_merge(self):
        with self.assertRaises(ValueError):
            crs.apply_moves(SRC, "f", [("0x1A", "b", "a")])

    def test_loop_wide_locals_are_never_moved(self):
        self.assertEqual(crs.loop_wide(SRC, "f", crs.declared_locals(SRC, "f")), {"k"})
        self.assertFalse(any("k" in m[1:] for m in crs.all_moves(SRC, "f")))

    def test_moves_respect_class(self):
        moves = set(crs.all_moves(SRC, "f"))
        self.assertIn(("0", "a", "b"), moves)
        self.assertNotIn(("0", "a", "x"), moves)
        self.assertNotIn(("0x1A", "b", "a"), moves)


if __name__ == "__main__":
    unittest.main()
