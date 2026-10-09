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

    def test_split_recipe_skips_build_py_options(self):
        # Overlay TUs run asm-processor with --force ahead of the compiler path.
        args = fast_score.split_recipe(RECIPE.replace("build.py tools/ido/cc", "build.py --force tools/ido/cc"))
        self.assertEqual(args[0], "tools/ido/cc")
        self.assertNotIn("--force", args)

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

    def test_source_grep_is_a_fixed_word_match(self):
        # `\\b` in a `git grep -E` pattern matched nothing on macOS; the word
        # boundary has to come from git's own -w, with the symbol fixed text.
        cmd = fast_score.source_grep_command("piRomLoadCompressed")
        self.assertIn("-w", cmd)
        self.assertIn("-F", cmd)
        self.assertNotIn("-E", cmd)
        self.assertFalse(any("\\b" in arg for arg in cmd))
        self.assertEqual(cmd[cmd.index("-F") + 1], "piRomLoadCompressed")

    def test_tracked_source_found_for_a_matched_function(self):
        # A matched function has no ranking row, so this exercises the grep.
        self.assertEqual(fast_score.tracked_source_for("piRomLoadCompressed"), "src/main/pi.c")

    def test_split_recipe_rejects_malformed_line(self):
        with self.assertRaises(SystemExit):
            fast_score.split_recipe("tools/asm-processor/build.py tools/ido/cc -c a.c")


LISTING = """\
.section .late_rodata
dlabel D_80081790
/* 82390 80081790 3C23D70A */ .float 0.0099
/* 82394 80081794 00000000 */ .float 0
enddlabel D_80081790

.section .text
glabel fn
/* 12580 80011980 27BDFF38 */ addiu      $sp, $sp, -0xC8
/* 12584 80011984 3C0A8009 */  lui        $t2, %hi(D_800C9D3C)
endlabel fn
"""


class ListingTests(unittest.TestCase):
    def test_text_rows_skip_a_literal_pool_before_the_function(self):
        rows = fast_score.text_rows(LISTING.splitlines())
        self.assertEqual([w for w, _ in rows], [0x27BDFF38, 0x3C0A8009])
        self.assertEqual(rows[0][1], "addiu $sp, $sp, -0xC8")

    def test_text_rows_without_a_pool_are_unchanged(self):
        src = "\n".join(l for l in LISTING.splitlines() if "late_rodata" not in l and "D_80081790" not in l
                        and ".float" not in l)
        self.assertEqual(len(fast_score.text_rows(src.splitlines())), 2)


class InstrumentedTests(unittest.TestCase):
    def test_wanted_by_each_family_or_flag(self):
        w = fast_score.instrumented_wanted
        self.assertFalse(w({"PATH": "x", "DKWB_SUBST_KEEP": ""}))
        self.assertFalse(w({"DKWB_UGEN_TRACE": "1"}))
        self.assertTrue(w({"DKWB_SUBST_KEEP": "2658"}))
        self.assertTrue(w({"DKWB_SUBST_TRACE": "/x"}))
        self.assertTrue(w({"DKWB_CUT_LINES": "5"}))
        self.assertTrue(w({"CDX_FORCE": "p1:w1=s"}))
        self.assertTrue(w({}, flag=True))

    def test_use_compiler_swaps_only_the_driver_and_keeps_the_symlink(self):
        cc = Path("/home/u/ido-instrumented/cc")
        out = fast_score.use_compiler(["tools/ido/cc", "-O2", "x.c"], cc)
        self.assertEqual(out, [str(cc), "-O2", "x.c"])
        self.assertEqual(fast_score.use_compiler(["tools/ido/cc", "-O2"], None), ["tools/ido/cc", "-O2"])

    def test_instrumented_cc_path_is_not_resolved(self):
        self.assertEqual(fast_score.INSTRUMENTED_CC.name, "cc")
        self.assertEqual(fast_score.INSTRUMENTED_CC.parent.name, "ido-instrumented")


class RelativeCandidateTests(unittest.TestCase):
    """A relative candidate path is the caller's cwd's, not the repository root's."""

    def test_missing_relative_candidate_is_reported_under_the_callers_cwd(self):
        import os
        import tempfile
        with tempfile.TemporaryDirectory() as directory:
            old = os.getcwd()
            os.chdir(directory)
            try:
                with self.assertRaises(SystemExit) as caught:
                    fast_score.main(["func_x", "nope.c"])
            finally:
                os.chdir(old)
        self.assertIn(str(Path(directory).resolve() / "nope.c"), str(caught.exception))


if __name__ == "__main__":
    unittest.main()
