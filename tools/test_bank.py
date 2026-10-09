#!/usr/bin/env python3
"""Tests for bank.py against a synthetic repository (no ROM content, no build)."""

from __future__ import annotations

import io
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import bank  # noqa: E402
import finalize_plateau as fp  # noqa: E402

GUARDED = """#ifdef NON_MATCHING
void demo_symbol(int value) {
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
#endif
"""
MATCHED = "void demo_symbol(int value) {\n}\n"
SOURCE_REL = "src/main/demo.c"
NOTE = "#### 2026-10-07, lane t: tried a thing\n\nIt moved the count 9 -> 3.\n"


def row(masked=3, first=0x34, delta=0):
    return {
        "relocation_masked_differing_words": masked, "differing_words": masked + 1,
        "relocation_masked_first_mismatch_offset": first, "size_delta": delta,
        "candidate_words": 10, "target_words": 10, "frame": "0x18",
        "relocations": 2, "aligned_exact": 7, "aligned_register_naming": 2,
        "aligned_immediate_only": 0, "aligned_really_different": 1,
    }


class Fixture(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.tmp.name)
        self.addCleanup(self.tmp.cleanup)
        for args in (("init", "-q"), ("config", "user.email", "t@example.com"),
                     ("config", "user.name", "t")):
            subprocess.run(["git", *args], cwd=self.root, check=True)
        (self.root / "src/main").mkdir(parents=True)
        (self.root / "config").mkdir()
        (self.root / "docs/matching-triage-handoffs").mkdir(parents=True)
        (self.root / SOURCE_REL).write_text(GUARDED)
        (self.root / bank.RANKING_REL).write_text(
            json.dumps({"functions": [{"name": "demo_symbol"}]}))
        (self.root / bank.RANKING_DOC_REL).write_text("doc\n")
        subprocess.run(["git", "add", "-A"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "init"], cwd=self.root, check=True)
        self.queued = SOURCE_REL
        self.measurement = row()
        self.calls = []
        self.gate_status = 0
        patches = [
            mock.patch.object(bank, "queue_source", lambda s: self.queued),
            mock.patch.object(bank, "measure", lambda s: dict(self.measurement)),
            mock.patch.object(bank, "regenerate_ranking",
                              lambda r: self.calls.append("rank")),
            mock.patch.object(bank, "run_checks",
                              lambda r: self.calls.append("checks")),
            mock.patch.object(bank, "run_gates",
                              lambda r: self.calls.append("gates") or self.gate_status),
        ]
        for p in patches:
            p.start()
            self.addCleanup(p.stop)
        self.note = self.root / "note.txt"
        self.note.write_text(NOTE)

    def run_bank(self, *argv):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            code = bank.main(["demo_symbol", *argv], root=self.root)
        return code, out.getvalue(), err.getvalue()

    def shard(self):
        return (self.root / fp.handoff_shard_path("demo_symbol")).read_text()

    def source(self):
        return (self.root / SOURCE_REL).read_text()


class BankTests(Fixture):
    def test_header_comes_from_measurement(self):
        code, _, err = self.run_bank("--note", str(self.note), "--summary", "next lever")
        self.assertEqual(code, 0, err)
        shard = self.shard()
        self.assertIn("- score: 3/10 words\n", shard)
        self.assertIn("- frame: 0x18\n", shard)
        self.assertIn("- relocations: 2\n", shard)
        self.assertIn("- first mismatch: +0x34\n", shard)
        self.assertIn("- summary: next lever\n", shard)
        self.assertIn("#### 2026-10-07, lane t: tried a thing", shard)
        self.assertIn("byte-exact 7, register naming 2", shard)
        self.assertIn(" * score: 3/10 words\n", self.source())
        fp.parse_shard(shard, "demo_symbol")
        self.assertEqual(self.calls, ["rank", "checks"])

    def test_idempotent(self):
        self.run_bank("--note", str(self.note), "--summary", "s")
        first = (self.shard(), self.source())
        code, _, err = self.run_bank("--note", str(self.note), "--summary", "s")
        self.assertEqual(code, 0, err)
        self.assertEqual((self.shard(), self.source()), first)
        self.assertEqual(self.shard().count("#### 2026-10-07"), 1)

    def test_remeasure_updates_header_and_keeps_evidence(self):
        self.run_bank("--note", str(self.note), "--summary", "s")
        self.measurement = row(masked=1, first=0x40)
        self.run_bank()
        shard = self.shard()
        self.assertIn("- score: 1/10 words\n", shard)
        self.assertIn("It moved the count", shard)

    def test_zero_masked_says_none(self):
        self.measurement = row(masked=0, first=None)
        self.run_bank()
        self.assertIn("- first mismatch: none\n", self.shard())

    def test_note_with_pipe_refused(self):
        self.note.write_text("#### 2026-10-07, x\n\n| a | b |\n")
        code, _, err = self.run_bank("--note", str(self.note))
        self.assertEqual(code, 2)
        self.assertIn("'|'", err)

    def test_long_summary_refused_before_anything_is_written(self):
        code, _, err = self.run_bank("--summary", "x" * 161)
        self.assertEqual(code, 2)
        self.assertIn("161 characters", err)
        self.assertIn("160", err)
        self.assertFalse((self.root / fp.handoff_shard_path("demo_symbol")).exists())

    def test_hex_looking_block_name_refused_naming_the_rule(self):
        self.note.write_text("#### 2026-10-07, x\n\nThe split starts at bb51 and ends.\n")
        code, _, err = self.run_bank("--note", str(self.note))
        self.assertEqual(code, 2)
        self.assertIn("bb51", err)
        self.assertIn("BARE_HEX_WORD", err)
        code, _, err = self.run_bank("--summary", "stuck in bb51")
        self.assertEqual(code, 2)
        self.assertIn("--summary", err)

    def test_pipe_in_summary_and_plain_words_pass_the_preflight(self):
        with self.assertRaises(bank.BankError) as caught:
            bank.preflight_text("a | b", None)
        self.assertIn("'|'", str(caught.exception))
        # ordinary words, decimal block numbers and 0x-prefixed offsets are fine
        bank.preflight_text("dead read in block 51 at 0x7f3a", "#### 2026-10-07, x\n\nfaded 2026\n")

    def test_note_needs_dated_heading(self):
        self.note.write_text("just prose\n")
        code, _, err = self.run_bank("--note", str(self.note))
        self.assertEqual(code, 2)
        self.assertIn("heading", err)

    def test_not_queued_refused(self):
        self.queued = None
        code, _, err = self.run_bank()
        self.assertEqual(code, 2)
        self.assertIn("--match", err)

    def test_mid_file_marker_replaced_in_place(self):
        metrics = fp.Metrics("9/10 words", "0x8", 1, "+0x4", "old")
        marker = fp.source_handoff("demo_symbol", metrics)
        (self.root / SOURCE_REL).write_text(
            GUARDED + "\n" + marker + "\nint later;\n")
        subprocess.run(["git", "add", "-A"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "m"], cwd=self.root, check=True)
        code, _, err = self.run_bank()
        self.assertEqual(code, 0, err)
        text = self.source()
        self.assertEqual(text.count("PLATEAU-HANDOFF:demo_symbol:start"), 1)
        self.assertIn(" * score: 3/10 words\n", text)
        self.assertTrue(text.rstrip().endswith("int later;"))
        self.assertIn(" * summary: old\n", text)

    def test_commit_runs_gates_then_commits_with_trailer(self):
        code, out, err = self.run_bank(
            "--note", str(self.note), "--commit",
            "--trailer", "Co-Authored-By: T <t@example.com>")
        self.assertEqual(code, 0, err)
        self.assertEqual(self.calls, ["rank", "checks", "gates"])
        log = subprocess.run(["git", "log", "-1", "--format=%B"], cwd=self.root,
                             text=True, stdout=subprocess.PIPE).stdout
        self.assertIn("Bank demo_symbol", log)
        self.assertIn("Co-Authored-By: T <t@example.com>", log)

    def test_gate_failure_blocks_commit(self):
        self.gate_status = 1
        before = subprocess.run(["git", "rev-parse", "HEAD"], cwd=self.root,
                                text=True, stdout=subprocess.PIPE).stdout
        code, _, err = self.run_bank("--note", str(self.note), "--commit")
        self.assertEqual(code, 2)
        self.assertIn("nothing committed", err)
        after = subprocess.run(["git", "rev-parse", "HEAD"], cwd=self.root,
                               text=True, stdout=subprocess.PIPE).stdout
        self.assertEqual(before, after)

    def test_unrelated_dirt_refused_on_commit(self):
        (self.root / "stray.txt").write_text("x")
        code, _, err = self.run_bank("--commit")
        self.assertEqual(code, 2)
        self.assertIn("stray.txt", err)

    def test_trailer_requires_commit(self):
        code, _, err = self.run_bank("--trailer", "A: b")
        self.assertEqual(code, 2)


class MatchTests(Fixture):
    def banked(self):
        self.run_bank("--summary", "open")
        subprocess.run(["git", "add", "-A"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "b"], cwd=self.root, check=True)

    def test_match_refused_while_queued(self):
        code, _, err = self.run_bank("--match")
        self.assertEqual(code, 2)
        self.assertIn("still in the NON_MATCHING queue", err)

    def test_match_writes_matched_header_and_drops_marker(self):
        self.banked()
        (self.root / SOURCE_REL).write_text(
            (self.root / SOURCE_REL).read_text().replace(GUARDED, MATCHED))
        self.queued = None
        (self.root / bank.RANKING_REL).write_text(json.dumps({"functions": []}))
        code, _, err = self.run_bank("--match", "--summary", "natural rewrite")
        self.assertEqual(code, 0, err)
        shard = self.shard()
        self.assertIn("- score: 0 differing words\n", shard)
        self.assertIn("- first mismatch: none\n", shard)
        self.assertIn("- summary: Matched. natural rewrite\n", shard)
        self.assertIn("- frame: 0x18\n", shard)
        self.assertNotIn("PLATEAU-HANDOFF", self.source())
        fp.parse_shard(shard, "demo_symbol")

    def test_match_refused_when_fallback_remains(self):
        self.banked()
        self.queued = None
        code, _, err = self.run_bank("--match")
        self.assertEqual(code, 2)

    def test_match_refused_when_ranking_still_lists_it(self):
        self.banked()
        (self.root / SOURCE_REL).write_text(MATCHED)
        self.queued = None
        code, _, err = self.run_bank("--match")
        self.assertEqual(code, 2)
        self.assertIn("still listed", err)


class UnitTests(unittest.TestCase):
    def test_frame_of(self):
        self.assertEqual(bank.frame_of([0x27BDFFE8, 0]), "0x18")
        self.assertEqual(bank.frame_of([0x03E00008, 0]), "frameless")

    def test_apply_section_replaces_same_heading(self):
        shard = ("<!-- plateau-handoff:s:start -->\nhead\n\n#### 2026-01-01, a\n\nold\n"
                 "<!-- plateau-handoff:s:end -->\n")
        out = bank.apply_section(shard, "s", "#### 2026-01-01, a\n\nnew")
        self.assertIn("new", out)
        self.assertNotIn("old", out)


if __name__ == "__main__":
    unittest.main()
