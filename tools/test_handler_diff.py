"""Fixture tests for tools/handler_diff.py (no build needed; synthetic rows)."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import handler_diff as hd  # noqa: E402


class HandlerDiffTests(unittest.TestCase):
    def test_aliases_and_immediates_normalise_together(self):
        self.assertEqual(hd.normalise("beqz $t0,.L80001234"), hd.normalise("beq $t0,$zero,1c <f+0x1c>"))
        self.assertEqual(hd.normalise("lui $at,%hi(D_80001000)"), hd.normalise("lui $at,0x0"))
        self.assertEqual(hd.normalise("nop"), hd.normalise("sll $zero,$zero,0x0"))

    def test_regions_share_fallthrough_targets(self):
        self.assertEqual(hd.regions([0, 8, 8, 4]), [(0, 4), (8, None), (8, None), (4, 8)])

    def test_compare_splits_shape_and_naming(self):
        t = [(0, "addu $v0,$a0,$a1"), (4, "jr $ra")]
        c = [(0, "addu $v1,$a0,$a1"), (4, "jr $ra")]
        self.assertEqual(hd.compare(t, c)[3:], (0, 1))
        self.assertEqual(hd.compare(t, c[1:])[3:], (1, 0))

    def test_saved_defs_skip_stores(self):
        rows = [(0, "sw $s3,0x10($sp)"), (4, "andi $s5,$v0,0xFF"), (8, "sra $s6,$v0,8")]
        self.assertEqual(hd.saved_defs(rows), ["s5=andi $v0,255", "s6=sra $v0,8"])


if __name__ == "__main__":
    unittest.main()
