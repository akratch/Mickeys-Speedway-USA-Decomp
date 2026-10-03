"""Fixture tests for tools/sibling_scan.py's parsing and ranking (no build needed)."""

import sys
import subprocess
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import sibling_scan  # noqa: E402

# Synthetic listing rows in splat's comment layout; the words are made up.
LISTING = """glabel func_demo
    /* 000000 00000000 11111111 */  addiu      $sp, $sp, -0x18
    /* 000004 00000004 22222222 */  sw         $ra, 0x14($sp)
    /* 000008 00000008 33333333 */  jal        helper
    /* 00000C 0000000C 44444444 */   nop
.L1:
    /* 000010 00000010 55555555 */  lw         $ra, 0x14($sp)
"""


class SiblingScanTests(unittest.TestCase):
    @staticmethod
    def symbol(name, value=0, size=160):
        return (name, value, size, 0x12, 1)

    def test_assembly_alias_is_rejected_without_a_canonical_definition(self):
        # o088's fallback listing has a generated name, subsequently renamed
        # by objcopy. Absence from the ranking does not authenticate the alias.
        text = '#pragma GLOBAL_ASM("asm/nonmatchings/func_generated.s")\n'
        self.assertEqual(sibling_scan.authenticated_names(
            text, [self.symbol("overlay88DrawSortedGeometry")],
            "src/overlays/o088/overlay88DrawSortedGeometry.c", {}), set())

    def test_preprocessed_include_alias_needs_exact_atlas_ownership(self):
        # Preprocessing has resolved #include and #define into this definition.
        text = 'void overlay88DrawSortedGeometry(void) { helper(); }\n'
        source = "src/overlays/o088/overlay88DrawSortedGeometry.c"
        symbols = [self.symbol("overlay88DrawSortedGeometry")]
        self.assertEqual(sibling_scan.authenticated_names(text, symbols, source, {}), set())
        self.assertEqual(sibling_scan.authenticated_names(
            text, symbols, source, {source: (0x1A4, [])}), set())
        self.assertEqual(sibling_scan.authenticated_names(
            text, symbols, source, {source: (0x1A4, [(0x1A4, 0x244)])}),
            {"overlay88DrawSortedGeometry"})

    def test_mixed_tu_keeps_only_functions_inside_exact_islands(self):
        source = "src/overlays/o015/overlay_015.c"
        text = 'void exact(void) {}\nvoid candidate(void) {}\n'
        symbols = [self.symbol("exact"), self.symbol("candidate", 160)]
        owners = {source: (0x100, [(0x100, 0x1A0)])}
        self.assertEqual(sibling_scan.authenticated_names(text, symbols, source, owners), {"exact"})
        # A definition straddling the reviewed boundary is not authenticated.
        self.assertEqual(sibling_scan.authenticated_names(
            text, [self.symbol("exact", size=164)], source, owners), set())

    def test_declarations_undefined_and_duplicate_definitions_fail_closed(self):
        source = "src/main/demo.c"
        symbol = self.symbol("demo")
        for text in ['void demo(void);\n', 'void demo(void) {}\nvoid demo(void) {}\n']:
            self.assertEqual(sibling_scan.authenticated_names(text, [symbol], source, {}), set())
        undefined = (*symbol[:4], 0)
        self.assertEqual(sibling_scan.authenticated_names(
            'void demo(void) {}\n', [undefined], source, {}), set())

    def test_canonical_recipes_force_matching_mode_and_preserve_cpp_inputs(self):
        root = sibling_scan.ROOT
        obj = str(root / "build/src/main/demo.c.o")
        recipe = ('.venv/bin/python tools/asm-processor/build.py python3 tools/ido-phases.py '
                  '-- tools/binutils/mips64-elf-as -- -c -nostdinc -D_LANGUAGE_C '
                  '-DRAREDIFFS -I . -I include -O2 -Xphase,uopt,-O1 -mips2 -32 '
                  '-o build/src/main/demo.c.o src/main/demo.c\n')
        with patch.object(sibling_scan.subprocess, "run", return_value=
                          subprocess.CompletedProcess([], 0, recipe, "")) as run:
            commands = sibling_scan.canonical_commands([obj])
        self.assertIn("NON_MATCHING=0", run.call_args.args[0])
        self.assertIn("-DRAREDIFFS", commands[obj])
        self.assertIn("-I", commands[obj])
        self.assertNotIn("-Xphase,uopt,-O1", commands[obj])
        self.assertNotIn("-DNON_MATCHING", commands[obj])

    def test_missing_recipe_is_an_error(self):
        obj = str(sibling_scan.ROOT / "build/src/main/demo.c.o")
        with patch.object(sibling_scan.subprocess, "run", return_value=
                          subprocess.CompletedProcess([], 0, "", "")):
            with self.assertRaises(sibling_scan.CandidateProofError):
                sibling_scan.canonical_commands([obj])

    def test_candidate_define_in_overridden_recipe_fails_closed(self):
        obj = str(sibling_scan.ROOT / "build/src/main/demo.c.o")
        for spelling in ["-DNON_MATCHING", "-D NON_MATCHING=0"]:
            recipe = ('tools/ido/cc -c ' + spelling
                      + ' -o build/src/main/demo.c.o src/main/demo.c\n')
            with patch.object(sibling_scan.subprocess, "run", return_value=
                              subprocess.CompletedProcess([], 0, recipe, "")):
                with self.assertRaises(sibling_scan.CandidateProofError):
                    sibling_scan.canonical_commands([obj])

    def test_atlas_owner_uses_mixed_exact_islands_not_broad_matched_flag(self):
        row = {"offset": "0x100", "end_offset": "0x240", "size": "0x140",
               "source": "overlays/o015/demo", "type": "c", "matched": True,
               "nonmatching": True}
        island = {"offset": "0x100", "end_offset": "0x1A0", "size": "0xA0",
                  "source": row["source"], "label": "exact"}
        atlas = {"modules": [{"overlay": 15, "text_ownership": [row],
                              "mixed_tu_exact_c_ranges": [island]}],
                 "totals": {"matched_overlay_c_bytes": 160}}
        self.assertEqual(sibling_scan.overlay_owners(atlas),
                         {"src/overlays/o015/demo.c": (0x100, [(0x100, 0x1A0)])})
        atlas["totals"]["matched_overlay_c_bytes"] = 320
        with self.assertRaises(sibling_scan.overlay_atlas.AtlasDeltaError):
            sibling_scan.overlay_owners(atlas)

    @unittest.skipUnless((sibling_scan.ROOT / "tools/ido/cc").exists(), "IDO unavailable")
    def test_ido_resolves_guarded_include_and_alias_before_authentication(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "body.c").write_text('void original(void) {}\n')
            source = root / "wrapper.c"
            source.write_text('#define original alias\n'
                              '#ifdef NON_MATCHING\n#include "body.c"\n#else\n'
                              '#pragma GLOBAL_ASM("asm/nonmatchings/generated.s")\n#endif\n'
                              '// Don\'t mistake a comment for a character literal.\n')
            compiler = str(sibling_scan.ROOT / "tools/ido/cc")
            canonical = sibling_scan.preprocess_source([compiler, "-E", str(source)])
            self.assertEqual(sibling_scan.authenticated_names(
                canonical, [self.symbol("alias")], "src/main/wrapper.c", {}), set())
            candidate = sibling_scan.preprocess_source(
                [compiler, "-E", "-DNON_MATCHING", str(source)])
            self.assertEqual(sibling_scan.authenticated_names(
                candidate, [self.symbol("alias")], "src/main/wrapper.c", {}), {"alias"})

    def test_listing_mnemonics_reads_rows_and_skips_labels(self):
        with tempfile.NamedTemporaryFile("w", suffix=".s", delete=False) as handle:
            handle.write(LISTING)
        try:
            self.assertEqual(
                sibling_scan.listing_mnemonics(Path(handle.name)),
                ["addiu", "sw", "jal", "nop", "lw"],
            )
        finally:
            Path(handle.name).unlink()

    def test_rank_prefers_the_same_shape_over_the_same_length(self):
        # Non-periodic on purpose: difflib aligns a periodic sequence against a
        # shifted copy of itself, which real function bodies never are.
        ops = ["addiu", "sw", "lw", "addu", "jal", "nop", "beq", "sll", "or", "jr"]
        target = [ops[(i * 7 + i // 3) % len(ops)] for i in range(60)]
        same_shape = list(target)
        same_shape[10] = "xor"  # one substitution, as a different allocation would give
        other = ["lui", "lwc1", "mul.s", "add.s", "swc1"] * 12
        candidates = {("sibling", "a.c.o"): same_shape, ("stranger", "b.c.o"): other}
        grams = {k: sibling_scan.trigrams(v) for k, v in candidates.items()}
        ranked = sibling_scan.rank(target, candidates, grams, top=2)
        self.assertEqual(ranked[0][1], "sibling")
        self.assertGreater(ranked[0][0], 0.95)
        self.assertLess(ranked[-1][0], 0.2)

    def test_rank_skips_candidates_far_off_in_length(self):
        target = ["addiu", "sw", "lw", "jr"] * 25
        candidates = {("tiny", "c.c.o"): ["addiu", "sw", "lw", "jr"] * 5}
        grams = {k: sibling_scan.trigrams(v) for k, v in candidates.items()}
        self.assertEqual(sibling_scan.rank(target, candidates, grams, top=3), [])


if __name__ == "__main__":
    unittest.main()
