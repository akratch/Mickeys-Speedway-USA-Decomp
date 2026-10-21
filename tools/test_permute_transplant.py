#!/usr/bin/env python3
"""Tests for tools/permute_transplant.py on synthetic TUs (no compile)."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import permute_transplant as pt  # noqa: E402

TU = """#include "common.h"

void func_target(void);   /* prototype: not a definition */

void caller(void) {
    func_target();
}

#ifdef NON_MATCHING
/* candidate */
void func_target(void) {
    const char *s = "}{";
    if (D_1) { D_2 = '}'; }
    // a stray } in a comment
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/x/func_target.s")
#endif

void after(void) {
}
"""

WINNER = """typedef int s32;
extern s32 D_1;
extern s32 D_2;
void caller(void);
void func_target(void)
{
  if (D_1)
  {
    D_2 = 7;
  }
}

"""

BASE = WINNER.replace("D_2 = 7;", "D_2 = '}';").replace("extern s32 D_2;", "#pragma _permuter latedefine start\nextern s32 D_2;")


class TransplantTests(unittest.TestCase):
    def test_definitions_skip_prototypes_calls_and_literal_braces(self):
        defs = pt.definitions(TU, "func_target")
        self.assertEqual(len(defs), 1)
        start, end = defs[0]
        self.assertTrue(TU[start:].startswith("void func_target(void) {"))
        self.assertTrue(TU[:end].endswith("comment\n}"))

    def test_transplant_replaces_only_the_candidate_body(self):
        out = pt.transplant("func_target", WINNER, TU)
        self.assertIn("D_2 = 7;", out)
        self.assertNotIn("'}'", out)
        self.assertIn("/* candidate */\nvoid func_target(void)\n{", out)
        self.assertIn('#pragma GLOBAL_ASM("asm/nonmatchings/main/x/func_target.s")', out)
        self.assertTrue(out.startswith('#include "common.h"'))
        self.assertIn("void after(void) {\n}", out)

    def test_nonmatching_definition_wins_when_there_are_two(self):
        tu = TU.replace("void after(void) {\n}", "#ifndef NON_MATCHING\nvoid func_target(void) {\n}\n#endif")
        start, _end = pt.choose_definition(tu, "func_target")
        self.assertLess(start, tu.index("#else"))

    def test_knr_definition_is_found(self):
        text = "void f(a)\nunsigned char a;\n{\n    g(a);\n}\n"
        self.assertEqual(pt.definitions(text, "f"), [(0, len(text) - 1)])

    def test_missing_definition_is_an_error(self):
        with self.assertRaises(pt.TransplantError):
            pt.transplant("absent", WINNER, TU)

    def test_outside_changes_ignore_permuter_pragmas(self):
        changed = BASE.replace("extern s32 D_1;", "extern unsigned int D_1;")
        self.assertEqual(pt.outside_changes("func_target", BASE, WINNER), [])
        self.assertEqual(pt.outside_changes("func_target", changed, WINNER),
                         ["-extern unsigned int D_1;", "+extern s32 D_1;"])

    def test_cli_writes_output_and_reports_the_permuter_score(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            out_dir = root / "scratch" / "output-12-1"
            out_dir.mkdir(parents=True)
            (out_dir / "source.c").write_text(WINNER)
            (out_dir / "score.txt").write_text("12\n")
            (root / "scratch" / "base.c").write_text(BASE)
            tu = root / "tu.c"
            tu.write_text(TU)
            out = root / "out.c"
            self.assertEqual(pt.main(["func_target", str(out_dir), str(tu), "-o", str(out), "--no-score"]), 0)
            self.assertIn("D_2 = 7;", out.read_text())
            self.assertEqual(tu.read_text(), TU)
            self.assertEqual(pt.main(["func_target", str(out_dir), str(tu), "-o", str(tu), "--no-score"]), 2)


if __name__ == "__main__":
    unittest.main()
