#!/usr/bin/env python3
"""Guards on what a conflict resolution is allowed to produce.

Every case here is a resolution that left no conflict marker, so the tree
looked clean and the failure landed several gates downstream.
"""
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import resolve_lane_conflicts as rlc


class ValidateResolved(unittest.TestCase):
    def write(self, name, text):
        path = Path(self.tmp.name) / name
        path.write_text(text)
        return str(path)

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)

    def test_two_json_records_spliced_without_a_delimiter_are_caught(self):
        """The measured incident: keep-both concatenated two records."""
        path = self.write("pins.json", '{"a": 1}{"b": 2}')
        message = rlc.validate_resolved(path)
        self.assertIsNotNone(message)
        self.assertIn("INVALID JSON", message)
        self.assertIn("pins.json", message)

    def test_a_function_tail_without_its_head_is_caught(self):
        """The measured incident: a stray brace closing nothing."""
        path = self.write("tool.py", 'def f():\n    return 1\n\n'
                                     '        "python": digest(),\n    }\n')
        message = rlc.validate_resolved(path)
        self.assertIsNotNone(message)
        self.assertIn("INVALID PYTHON", message)
        self.assertIn("line", message)

    def test_the_message_names_the_line_so_the_cause_is_findable(self):
        path = self.write("tool.py", "ok = 1\n" * 40 + "def broken(:\n")
        message = rlc.validate_resolved(path)
        self.assertIn("line 41", message)

    def test_valid_files_of_both_kinds_pass(self):
        self.assertIsNone(rlc.validate_resolved(self.write("a.json", '{"a": [1, 2]}')))
        self.assertIsNone(rlc.validate_resolved(
            self.write("b.py", "import os\n\n\ndef f(x):\n    return {'k': x}\n")))

    def test_unknown_suffixes_are_not_claimed_to_be_checked(self):
        """A .c file is not parse-checked here; do not imply otherwise."""
        self.assertIsNone(rlc.validate_resolved(self.write("x.c", "void f( {")))

    def test_an_undecodable_file_does_not_crash_the_resolver(self):
        path = Path(self.tmp.name) / "bin.py"
        path.write_bytes(b"\xff\xfe def f(:\n")
        self.assertIsNotNone(rlc.validate_resolved(str(path)))


class GeneratedRankingMergeTests(unittest.TestCase):
    def test_choose_regenerated_discards_an_older_lane_copy(self) -> None:
        lane = '{"functions": [{"name": "old", "size_delta": 4}]}\n'
        regenerated = '{"functions": [{"name": "current", "size_delta": 0}]}\n'
        chosen = rlc.choose_regenerated(lane, regenerated)
        self.assertEqual(chosen, regenerated)
        self.assertNotEqual(chosen, lane)

    def test_resolver_keeps_regeneration_ahead_of_the_lane_ranking_and_shard(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            previous = Path.cwd()
            os.chdir(root)
            try:
                self._resolve(root)
            finally:
                os.chdir(previous)

    def _git(self, *args: str) -> None:
        subprocess.run(["git", *args], check=True, capture_output=True, text=True)

    def _resolve(self, root: Path) -> None:
        self._git("init", "-q")
        self._git("checkout", "-q", "-b", "integration")
        self._git("config", "user.email", "merge@example.invalid")
        self._git("config", "user.name", "Merge Test")
        ranking = root / "config" / "nonmatching-ranking.us.json"
        shard_dir = root / "docs" / "matching-triage-handoffs"
        ranking.parent.mkdir(parents=True)
        shard_dir.mkdir(parents=True)
        ranking.write_text('{"resolved": "base"}\n', encoding="utf-8")
        (shard_dir / "demo_symbol.md").write_text("score: base\n", encoding="utf-8")
        self._git("add", ".")
        self._git("commit", "-q", "-m", "base")
        self._git("checkout", "-q", "-b", "lane")
        ranking.write_text('{"resolved": "lane-old"}\n', encoding="utf-8")
        (shard_dir / "demo_symbol.md").write_text("score: lane-old\n", encoding="utf-8")
        self._git("commit", "-q", "-am", "lane")
        self._git("checkout", "-q", "integration")
        ranking.write_text('{"resolved": "integrated"}\n', encoding="utf-8")
        (shard_dir / "demo_symbol.md").write_text("score: integrated\n", encoding="utf-8")
        self._git("commit", "-q", "-am", "integrated")
        subprocess.run(
            ["git", "merge", "--no-commit", "--no-ff", "lane"],
            check=False, capture_output=True, text=True,
        )
        argv = sys.argv
        sys.argv = ["resolve_lane_conflicts.py", "lane"]
        try:
            status = rlc.main()
        finally:
            sys.argv = argv
        self.assertEqual(status, 0)
        self.assertEqual(ranking.read_text(encoding="utf-8"), '{"resolved": "integrated"}\n')
        self.assertEqual(
            (shard_dir / "demo_symbol.md").read_text(encoding="utf-8"),
            "score: integrated\n",
        )
        regenerated_ranking = '{"resolved": "regenerated"}\n'
        regenerated_shard = "score: regenerated\n"
        ranking.write_text(
            rlc.choose_regenerated(
                '{"resolved": "lane-old"}\n', regenerated_ranking,
            ),
            encoding="utf-8",
        )
        (shard_dir / "demo_symbol.md").write_text(
            rlc.choose_regenerated("score: lane-old\n", regenerated_shard),
            encoding="utf-8",
        )
        self.assertEqual(ranking.read_text(encoding="utf-8"), regenerated_ranking)
        self.assertNotIn("lane-old", ranking.read_text(encoding="utf-8"))
        self.assertEqual(
            (shard_dir / "demo_symbol.md").read_text(encoding="utf-8"),
            regenerated_shard,
        )
        self.assertNotIn("lane-old", (shard_dir / "demo_symbol.md").read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
