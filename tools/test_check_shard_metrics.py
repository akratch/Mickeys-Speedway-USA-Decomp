#!/usr/bin/env python3
"""Tests for check_shard_metrics: the header/ranking reconciliation."""

from __future__ import annotations

import unittest

import check_shard_metrics as mod


class ResidualClaimTests(unittest.TestCase):
    def test_equal_halves_are_a_match(self):
        """N/N words is what finalize_plateau writes for a matched function."""
        self.assertFalse(mod.claims_a_residual("3614/3614 words", "+0x0"))

    def test_unequal_halves_claim_work(self):
        self.assertTrue(mod.claims_a_residual("14/249 words", "+0x6C"))

    def test_first_mismatch_none_settles_it(self):
        """`none` outranks the score spelling: there is nowhere left to differ."""
        self.assertFalse(mod.claims_a_residual("249 differing words", "none"))

    def test_bare_zero_is_a_match(self):
        self.assertFalse(mod.claims_a_residual("0 differing words", "+0x0"))

    def test_bare_nonzero_claims_work(self):
        self.assertTrue(mod.claims_a_residual("111 differing words", "+0xD0"))

    def test_unrecognised_spelling_is_not_evidence(self):
        """A shard we cannot parse must not be reported as a defect."""
        self.assertFalse(mod.claims_a_residual("mostly done", "+0x10"))


class OffsetTests(unittest.TestCase):
    def test_plus_hex_form(self):
        self.assertEqual(mod.parse_offset("+0x1C"), 0x1C)

    def test_bare_hex_form(self):
        self.assertEqual(mod.parse_offset("0x50"), 0x50)

    def test_none_and_unknown_are_not_offsets(self):
        self.assertIsNone(mod.parse_offset("none"))
        self.assertIsNone(mod.parse_offset("unknown"))

    def test_garbage_is_not_an_offset(self):
        """An unparseable offset is skipped, never reported as a mismatch."""
        self.assertIsNone(mod.parse_offset("early"))


class HeaderScanTests(unittest.TestCase):
    def test_only_the_header_is_scanned(self):
        """A later section quoting an old score is history, not a claim."""
        self.assertLessEqual(mod.HEADER_LINES, 20)

    def test_score_pattern_needs_line_start(self):
        self.assertIsNone(mod.SCORE_RE.search("  quoted - score: 9/9 words"))
        self.assertIsNotNone(mod.SCORE_RE.search("- score: 9/9 words"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
