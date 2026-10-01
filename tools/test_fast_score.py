"""Fixture tests for tools/fast_score.py's recipe handling (no build needed)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import fast_score  # noqa: E402

RECIPE = (
    ".venv/bin/python tools/asm-processor/build.py tools/ido/cc -- "
    "tools/binutils/mips64-elf-as -march=vr4300 -32 -mabi=32 -G0 -I include "
    "include/asm_processor_prelude.inc -- -c -non_shared -G 0 -Xcpluscomm "
    "-fullwarn -woff 649,838 -nostdinc -D_LANGUAGE_C -DVERSION_us -DNON_MATCHING "
    "-I . -I include -Wab,-r4300_mul -O2 -mips2 -32 "
    "-o build_non_matching/src/main/camera.c.o src/main/camera.c"
)


class RecipeTests(unittest.TestCase):
    def test_split_recipe_keeps_per_file_flags(self):
        args = fast_score.split_recipe(RECIPE)
        self.assertEqual(args[0], "tools/ido/cc")
        self.assertIn("-Wab,-r4300_mul", args)
        self.assertNotIn("-march=vr4300", args, "assembler flags must not leak into cc")
        self.assertEqual(args[-1], "src/main/camera.c")

    def test_rewrite_io_replaces_only_source_and_output(self):
        args = fast_score.split_recipe(RECIPE)
        out = fast_score.rewrite_io(args, Path("/tmp/x/cand.c"), "src/main/camera.c", Path("/tmp/x/cand.o"))
        self.assertEqual(out[out.index("-o") + 1], "/tmp/x/cand.o")
        self.assertEqual(out[-1], "/tmp/x/cand.c")
        self.assertEqual(out.count("src/main/camera.c"), 0)
        self.assertIn("-Wab,-r4300_mul", out)

    def test_target_listing_follows_the_candidate_guard(self):
        src = ("#ifdef NON_MATCHING\nvoid overlay101TailA6BC(s32 a) {\n    return;\n}\n#else\n"
               '#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/x/func_overlay_101_F000A6BC.s")\n#endif\n')
        self.assertEqual(fast_score.target_listing_for("overlay101TailA6BC", src),
                         "asm/nonmatchings/overlays/o101/x/func_overlay_101_F000A6BC.s")
        self.assertIsNone(fast_score.target_listing_for("missing", src))

    def test_split_recipe_rejects_malformed_line(self):
        with self.assertRaises(SystemExit):
            fast_score.split_recipe("tools/asm-processor/build.py tools/ido/cc -c a.c")


if __name__ == "__main__":
    unittest.main()
