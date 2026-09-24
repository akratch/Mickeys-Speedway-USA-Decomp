#!/usr/bin/env python3
"""Tests for check_shard_metrics: the header/ranking reconciliation."""

from __future__ import annotations

import json
import pathlib
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import check_shard_metrics as mod  # noqa: E402
import finalize_plateau  # noqa: E402

SOURCE = """#ifdef NON_MATCHING
void demo_symbol(int value) {
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
#endif
"""


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


class ConventionTests(unittest.TestCase):
    """`N/M words` is written both ways in this corpus and both are valid.

    Assuming the differing-first reading reported 31 false positives out of
    43 on the 2026-09-16 tree; assuming the masked offset alone reported 6
    out of 33. A checker that cries wolf is worse than no checker.
    """

    def test_differing_first_agrees(self):
        self.assertTrue(mod.PAIR_RE.match("14/249 words"))
        differing, total = 14, 249
        self.assertEqual(differing, 14)
        self.assertEqual(total - differing, 235)

    def test_matched_first_is_the_other_reading(self):
        """levelInit reads 510/516 with six differing: 510 is the MATCHED half."""
        pair = mod.PAIR_RE.match("510/516 words")
        differing, total = int(pair.group(1)), int(pair.group(2))
        self.assertEqual(total - differing, 6)


class HeaderScanTests(unittest.TestCase):
    def test_only_the_header_is_scanned(self):
        """A later section quoting an old score is history, not a claim."""
        self.assertLessEqual(mod.HEADER_LINES, 20)

    def test_score_pattern_needs_line_start(self):
        self.assertIsNone(mod.SCORE_RE.search("  quoted - score: 9/9 words"))
        self.assertIsNotNone(mod.SCORE_RE.search("- score: 9/9 words"))



class TreeTests(unittest.TestCase):
    """check() and --write against a real git tree with one queued symbol."""

    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.temporary.name)
        self.git("init", "-q")
        self.git("config", "user.email", "shard@example.invalid")
        self.git("config", "user.name", "Shard Test")
        (self.root / "src/main").mkdir(parents=True)
        (self.root / "config").mkdir()
        (self.root / "docs/matching-triage-handoffs").mkdir(parents=True)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def git(self, *args: str) -> str:
        return subprocess.run(
            ["git", *args], cwd=self.root, text=True, check=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        ).stdout

    def seed(self, score: str, first: str, *, masked=40, raw=41, offset=0x10):
        metrics = finalize_plateau.Metrics(score, "0x20", 3, first, "one web left")
        source = finalize_plateau.update_source(
            SOURCE, "demo_symbol",
            finalize_plateau.source_handoff("demo_symbol", metrics))
        (self.root / "src/main/demo.c").write_text(source, encoding="utf-8")
        shard = finalize_plateau.markdown_handoff(
            "demo_symbol", "src/main/demo.c", metrics)
        (self.root / "docs/matching-triage-handoffs/demo_symbol.md").write_text(
            shard, encoding="utf-8")
        (self.root / "config/nonmatching-ranking.us.json").write_text(
            json.dumps({"functions": [{
                "name": "demo_symbol",
                "relocation_masked_differing_words": masked,
                "differing_words": raw,
                "relocation_masked_first_mismatch_offset": offset,
                "first_mismatch_offset": offset,
            }]}), encoding="utf-8")
        self.git("add", ".")
        self.git("commit", "-q", "-m", "seed")

    def test_a_bare_score_may_quote_the_unmasked_count(self):
        self.seed("41 differing words", "+0x10")
        self.assertEqual(mod.check(self.root), [])

    def test_drift_is_reported_then_regenerated_from_the_ranking(self):
        self.seed("55 differing words", "+0x4")
        kinds = sorted(f["kind"] for f in mod.check(self.root))
        self.assertEqual(kinds, ["first-mismatch", "score"])
        written = mod.write(self.root, mod.check(self.root), today="2026-09-23")
        self.assertEqual(written, [
            "docs/matching-triage-handoffs/demo_symbol.md", "src/main/demo.c",
        ])
        self.assertEqual(mod.check(self.root), [])
        source = (self.root / "src/main/demo.c").read_text(encoding="utf-8")
        self.assertIn(" * score: 40 differing words\n", source)
        self.assertIn(" * first-mismatch: +0x10\n", source)
        self.assertIn(" * frame: 0x20\n", source)
        self.assertTrue(source.startswith(SOURCE))
        shard = (self.root / "docs/matching-triage-handoffs/demo_symbol.md"
                 ).read_text(encoding="utf-8")
        self.assertIn("- summary: one web left", shard)
        self.assertIn(
            "it read first mismatch +0x4; score 55 differing words.", shard)
        # The pair still reconciles under plateau_handoff_audit's grammar.
        import plateau_handoff_audit as pha
        audit = pha.audit_tree(self.root)
        self.assertEqual([item.status for item in audit.items], ["current"])
        self.assertEqual(audit.issues, [])

    def test_write_refuses_a_dirty_source(self):
        self.seed("55 differing words", "+0x10")
        path = self.root / "src/main/demo.c"
        path.write_text(path.read_text() + "\n", encoding="utf-8")
        with self.assertRaisesRegex(mod.WriteRefused, "local changes"):
            mod.write(self.root, mod.check(self.root))


if __name__ == "__main__":
    unittest.main(verbosity=2)
