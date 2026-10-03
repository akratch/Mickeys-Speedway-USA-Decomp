#!/usr/bin/env python3
"""Real Git destination fixtures for the pre-push clean-room range gate."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class PushDestinationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="mickey-pre-push-")
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)
        self.repo = self.work / "local"
        self.repo.mkdir()
        self.env = os.environ.copy()
        # Fixtures have no hooks installed. Keep Git identity and defaults local.
        self.env.update(GIT_AUTHOR_NAME="Hook fixture",
                        GIT_AUTHOR_EMAIL="fixture@example.invalid",
                        GIT_COMMITTER_NAME="Hook fixture",
                        GIT_COMMITTER_EMAIL="fixture@example.invalid")
        self.git("init", "-q")
        for name in (".githooks/pre-push", "tools/cleanroom_check.sh",
                     "tools/cleanroom_detectors.py"):
            path = self.repo / name
            path.parent.mkdir(exist_ok=True)
            shutil.copy2(ROOT / name, path)
        (self.repo / "safe.txt").write_text("Synthetic clean baseline.\n")
        self.base = self.commit("Clean synthetic baseline")
        (self.repo / "asm").mkdir()
        (self.repo / "asm/fixture.s").write_text("Synthetic non-ROM placeholder.\n")
        self.forbidden = self.commit("Synthetic forbidden-path ancestor")
        self.git("rm", "-q", "asm/fixture.s")
        self.tip = self.commit("Clean tip removes forbidden path")
        self.zero = "0" * len(self.tip)
        self.fetch = self.bare("fetch.git")
        self.destination = self.bare("destination.git")
        self.advertise(self.fetch, self.forbidden)
        self.git("remote", "add", "origin", str(self.fetch))
        self.git("config", "remote.origin.pushurl", str(self.destination))
        self.git("update-ref", "refs/remotes/origin/stale", self.forbidden)

    def run_command(self, args, cwd=None, check=True, input=None):
        return subprocess.run(args, cwd=cwd or self.repo, env=self.env,
                              text=True, input=input, capture_output=True,
                              check=check, timeout=30)

    def git(self, *args):
        return self.run_command(["git", *args]).stdout.strip()

    def commit(self, message):
        self.git("add", ".")
        self.git("commit", "-qm", message)
        return self.git("rev-parse", "HEAD")

    def bare(self, name):
        path = self.work / name
        self.run_command(["git", "init", "--bare", "-q", str(path)])
        return path

    def advertise(self, destination, tip):
        # A local fetch seeds a fixture advertisement without any push.
        self.run_command(["git", "--git-dir", str(destination), "fetch", "-q",
                          str(self.repo), tip + ":refs/heads/held"])

    def invoke(self, destination=None, include_url=True, lines=None):
        args = ["bash", ".githooks/pre-push", "origin"]
        if include_url:
            args.append(str(destination or self.destination))
        if lines is None:
            lines = [("refs/heads/new", self.tip, "refs/heads/new", self.zero)]
        data = "".join(" ".join(row) + "\n" for row in lines)
        return self.run_command(args, check=False, input=data)

    def assert_blocked(self, result):
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("fixture.s", result.stdout + result.stderr)
        self.assertIn("BLOCKED", result.stderr)

    def test_fetch_tip_absent_from_push_destination_is_scanned(self):
        self.assert_blocked(self.invoke())

    def test_actual_push_advertisement_excludes_already_published_ancestor(self):
        self.advertise(self.destination, self.forbidden)
        # The fetch destination now has no branch: only $2 proves this exclusion.
        self.run_command(["git", "--git-dir", str(self.fetch),
                          "update-ref", "-d", "refs/heads/held"])
        result = self.invoke()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_deleted_destination_ref_does_not_trust_stale_local_tracking(self):
        self.advertise(self.destination, self.forbidden)
        self.run_command(["git", "--git-dir", str(self.destination),
                          "update-ref", "-d", "refs/heads/held"])
        self.assert_blocked(self.invoke())

    def test_missing_destination_argument_scans_whole_branch(self):
        self.assert_blocked(self.invoke(include_url=False))

    def test_unreadable_destination_scans_whole_branch(self):
        self.assert_blocked(self.invoke(self.work / "absent.git"))

    def test_advertised_tip_missing_locally_is_not_excluded(self):
        foreign = self.work / "foreign"
        foreign.mkdir()
        self.run_command(["git", "init", "-q"], cwd=foreign)
        (foreign / "foreign.txt").write_text("Independent synthetic history.\n")
        self.run_command(["git", "add", "."], cwd=foreign)
        self.run_command(["git", "commit", "-qm", "Independent graph"], cwd=foreign)
        self.run_command(["git", "--git-dir", str(self.destination), "fetch", "-q",
                          str(foreign), "HEAD:refs/heads/foreign"])
        self.assert_blocked(self.invoke())

    def test_multiple_refs_retain_failure_after_deletion(self):
        self.assert_blocked(self.invoke(lines=[
            ("delete", self.zero, "refs/heads/held", self.forbidden),
            ("refs/heads/new", self.tip, "refs/heads/new", self.zero),
        ]))


if __name__ == "__main__":
    unittest.main()
