"""Execute the batch driver's verification block with controlled verifier exits."""

from pathlib import Path
import os
import json
import subprocess
import tempfile
import sys
import unittest

import runlink_pack as pack


DRIVER = Path(__file__).with_name("merge_lanes.sh")


class BatchRetirementTests(unittest.TestCase):
    """Execute the real merge loop in a small Git history, with no ROM/build."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.git("init", "-q", "-b", "integration")
        self.git("config", "user.name", "Retirement Test")
        self.git("config", "user.email", "retirement@example.invalid")
        self.git("config", "commit.gpgsign", "false")
        self.manifest = self.root / "config/runlink-residuals.us.json"
        self.manifest.parent.mkdir()
        self.rows = [[1, "root_first", 8], [2, "root_second", 6], [3, "incoming", 2], [4, "keep", 4]]
        self.write(self.rows)
        (self.root / "source.txt").write_text("base\n")
        self.git("add", "config", "source.txt")
        self.git("commit", "-qm", "base")
        self.base = self.git("rev-parse", "HEAD").stdout.strip()
        (self.root / ".venv/bin").mkdir(parents=True)
        (self.root / ".venv/bin/python").symlink_to(sys.executable)
        (self.root / "tools").mkdir()
        for name in ("resolve_lane_conflicts.py",):
            (self.root / "tools" / name).symlink_to((DRIVER.parent / name).resolve())
        gate = self.root / "tools/cleanroom_check.sh"
        gate.write_text("#!/bin/sh\nexit 0\n")
        gate.chmod(0o755)
        self.environment = dict(os.environ, PYTHONPATH=str(DRIVER.parent.resolve()))

    def git(self, *args, check=True):
        return subprocess.run(["git", *args], cwd=self.root, check=check,
                              capture_output=True, text=True)

    def write(self, rows):
        self.manifest.write_text(pack.render_manifest(pack.manifest_from([], {"placeholder": rows})))

    def branch(self, name, removed):
        self.git("checkout", "-qb", "lane/" + name, self.base)
        self.write([row for row in self.rows if row[0] not in removed])
        self.git("commit", "-qam", name)
        self.git("checkout", "-q", "integration")

    def run_merge_loop(self, *names):
        source = DRIVER.read_text()
        loop = source[source.index("merged=()") : source.index('[ "${#merged[@]}" -gt 0 ]')]
        return subprocess.run(["bash", "-c", "set -euo pipefail\n" + loop
                               + '\nprintf "reached broad gates\\n"\n', "merge-test", *names],
                              cwd=self.root, env=self.environment, capture_output=True, text=True, timeout=30)

    def test_three_stale_branches_never_resurrect_the_two_root_retirements(self):
        self.branch("first", {1})
        self.branch("second", {2})
        self.branch("stale-last", {3})
        result = self.run_merge_loop("first", "second", "stale-last")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("reached broad gates", result.stdout)
        expected = pack.manifest_from([], {"placeholder": [self.rows[-1]]})
        self.assertEqual(json.loads(self.manifest.read_text()), expected)
        committed = self.git("show", "HEAD:config/runlink-residuals.us.json").stdout
        self.assertEqual(committed, self.manifest.read_text())
        self.assertEqual(self.git("diff", "--name-only", "--diff-filter=U").stdout, "")

    def test_clean_merge_is_reconciled_before_commit(self):
        self.branch("clean", {1})
        # Only the lane edits the ledger: ordinary Git reports no conflict.
        (self.root / "source.txt").write_text("independent root work\n")
        self.git("commit", "-qam", "root source")
        result = self.run_merge_loop("clean")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(json.loads(self.manifest.read_text())["totals"]["borrowed_records"], 12)
        self.assertNotIn("resolved config/runlink-residuals", result.stdout)

    def test_clean_addition_refuses_without_commit_or_reaching_gates(self):
        self.git("checkout", "-qb", "lane/addition", self.base)
        self.write(self.rows + [[5, "unreviewed", 1]])
        self.git("commit", "-qam", "addition")
        self.git("checkout", "-q", "integration")
        before = self.git("rev-parse", "HEAD").stdout
        result = self.run_merge_loop("addition")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("added or changed", result.stderr)
        self.assertNotIn("reached broad gates", result.stdout)
        self.assertEqual(self.git("rev-parse", "HEAD").stdout, before)
        self.assertEqual(self.git("rev-parse", "--verify", "MERGE_HEAD").returncode, 0)
        self.assertEqual(json.loads(self.manifest.read_text())["totals"]["borrowed_records"], 21)


class BatchVerifyExitTests(unittest.TestCase):
    def run_verify(self, status, output):
        source = DRIVER.read_text()
        start = source.index("verify_log=$(mktemp -t mickey-batch-verify)")
        end = source.index("# Banked improvements", start)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "tools").mkdir()
            verifier = root / "tools" / "with_verify_lock.sh"
            verifier.write_text(
                '#!/bin/sh\nprintf "%s\\n" "$TEST_VERIFY_OUTPUT"\n'
                'exit "$TEST_VERIFY_STATUS"\n'
            )
            verifier.chmod(0o755)
            environment = dict(os.environ)
            environment.update(
                TEST_VERIFY_STATUS=str(status), TEST_VERIFY_OUTPUT=output,
                TMPDIR=str(root),
            )
            return subprocess.run(
                ["bash", "-c", "set -euo pipefail\nbuild_jobs=1\nbase=test\n"
                 + source[start:end] + '\nprintf "accepted\\n"\n'],
                cwd=root, env=environment, capture_output=True, text=True,
                timeout=10,
            )

    def test_successful_verify_is_accepted(self):
        result = self.run_verify(0, "OK: verified")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("accepted", result.stdout)

    def test_failure_after_ok_line_is_rejected(self):
        result = self.run_verify(7, "OK: earlier verification stage")
        self.assertEqual(result.returncode, 7, result.stderr)
        self.assertNotIn("accepted", result.stdout)
        self.assertIn("exit=7", result.stderr)

    def test_failure_without_ok_line_is_rejected(self):
        result = self.run_verify(3, "error: link failed")
        self.assertEqual(result.returncode, 3, result.stderr)
        self.assertNotIn("accepted", result.stdout)

    def test_zero_exit_without_expected_receipt_is_rejected(self):
        result = self.run_verify(0, "missing verification receipt")
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("accepted", result.stdout)


if __name__ == "__main__":
    unittest.main()
