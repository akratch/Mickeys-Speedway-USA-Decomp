"""Fixture tests for tools/sibling_scan.py's parsing and ranking (no build needed)."""

import sys
import tempfile
import unittest
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
