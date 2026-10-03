"""Execute the batch driver's verification block with controlled verifier exits."""

from pathlib import Path
import os
import subprocess
import tempfile
import unittest


DRIVER = Path(__file__).with_name("merge_lanes.sh")


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
