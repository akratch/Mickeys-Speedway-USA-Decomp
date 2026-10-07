#!/usr/bin/env python3
"""Exercise tools/integrate.sh against a throwaway git repo with stub gates."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "tools/integrate.sh"

STUB_MERGE = """#!/bin/bash
set -e
for n in "$@"; do git merge -q --no-edit "lane/$n" >/dev/null; done
if [ "${FAKE_DOCS_FAIL:-0}" = 1 ] && [ ! -f .repaired ]; then
  echo "gate FAILED: gmake check-docs"; exit 1
fi
[ "${FAKE_MERGE_FAIL:-0}" = 1 ] && { echo boom; exit 3; }
exit 0
"""
STUB_LAND = """#!/bin/bash
set -e
[ -z "$(git status --porcelain --untracked-files=no)" ] || { echo dirty >&2; exit 1; }
git checkout -q master && git merge -q --no-edit "$2" && git checkout -q campaign/unchain
"""
STUB_PY = """#!/bin/bash
case "$1" in
  *check_derived_numbers.py) [ "${FAKE_DERIVED_FAIL:-0}" = 1 ] && { echo "docs/x.md:7: jump tables 12 != 13" >&2; exit 1; }; exit 0 ;;
  *check_shard_metrics.py)
    if [ "$2" = "--write" ]; then touch .repaired; echo repaired >> README.md; exit 0; fi
    [ -f .repaired ] && exit 0 || exit 1 ;;
esac
"""


def git(cwd, *args):
    return subprocess.run(["git", *args], cwd=cwd, check=True, text=True,
                          capture_output=True).stdout.strip()


class IntegrateTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.repo = Path(self._tmp.name) / "repo"
        self.repo.mkdir()
        bindir = Path(self._tmp.name) / "bin"
        bindir.mkdir()
        for name, text in (("merge", STUB_MERGE), ("land", STUB_LAND), ("py", STUB_PY),
                           ("gmake", "#!/bin/bash\nexit 0\n")):
            (bindir / name).write_text(text)
            (bindir / name).chmod(0o755)
        self.bin = bindir
        r = self.repo
        git(r, "init", "-q", "-b", "master")
        git(r, "config", "user.email", "t@example.invalid")
        git(r, "config", "user.name", "t")
        git(r, "config", "commit.gpgsign", "false")
        for f in ("a.txt", "b.txt", "README.md"):
            (r / f).write_text("base\n")
        git(r, "add", "-A")
        git(r, "commit", "-qm", "base")
        git(r, "branch", "campaign/unchain")
        git(r, "checkout", "-q", "-b", "lane/x")
        (r / "a.txt").write_text("lane\n")
        git(r, "commit", "-qam", "lane x")
        git(r, "checkout", "-q", "campaign/unchain")
        self.env = dict(os.environ, PATH=f"{bindir}{os.pathsep}{os.environ['PATH']}",
                        MICKEY_INTEGRATE_PYTHON=str(bindir / "py"),
                        MICKEY_INTEGRATE_MERGE_LANES=str(bindir / "merge"),
                        MICKEY_INTEGRATE_LAND=str(bindir / "land"),
                        MICKEY_INTEGRATE_POLL="0.1")

    def run_it(self, *args, **extra):
        env = dict(self.env, **{k: str(v) for k, v in extra.items()})
        return subprocess.run(["bash", str(SCRIPT), *args], cwd=self.repo, env=env,
                              text=True, capture_output=True)

    def test_merge_without_land(self):
        res = self.run_it("x")
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        self.assertIn("== integrated x (not landed)", res.stdout)
        self.assertEqual((self.repo / "a.txt").read_text(), "lane\n")

    def test_land_prints_master_sha_and_restores_branch(self):
        res = self.run_it("--land", "x")
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        sha = git(self.repo, "rev-parse", "master")
        self.assertEqual(res.stdout.strip().splitlines()[-1], f"== integrated x landed {sha}")
        self.assertEqual(git(self.repo, "rev-parse", "--abbrev-ref", "HEAD"), "campaign/unchain")

    def test_unrelated_dirty_file_is_stashed_and_restored(self):
        (self.repo / "b.txt").write_text("owner edit\n")
        res = self.run_it("--land", "x")
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        self.assertEqual((self.repo / "b.txt").read_text(), "owner edit\n")
        self.assertEqual(git(self.repo, "stash", "list"), "")

    def test_dirty_file_the_batch_touches_is_refused(self):
        (self.repo / "a.txt").write_text("owner edit\n")
        base = git(self.repo, "rev-parse", "HEAD")
        res = self.run_it("x")
        self.assertEqual(res.returncode, 1)
        self.assertIn("a.txt", res.stdout)
        self.assertIn(f"== FAILED at preflight", res.stdout)
        self.assertIn(f"git reset --hard {base}", res.stdout)
        self.assertEqual(git(self.repo, "rev-parse", "HEAD"), base)
        self.assertEqual((self.repo / "a.txt").read_text(), "owner edit\n")

    def test_dirty_generated_file_is_refused(self):
        (self.repo / "README.md").write_text("owner edit\n")
        res = self.run_it("x")
        self.assertEqual(res.returncode, 1)
        self.assertIn("README.md", res.stdout)

    def test_merge_failure_reports_step_and_base(self):
        base = git(self.repo, "rev-parse", "HEAD")
        res = self.run_it("--land", "x", FAKE_MERGE_FAIL=1)
        self.assertEqual(res.returncode, 1)
        self.assertIn("== FAILED at merge_lanes", res.stdout)
        self.assertIn(f"git reset --hard {base}", res.stdout)
        self.assertEqual(git(self.repo, "rev-parse", "master"), base)

    def test_shard_drift_is_repaired_and_committed(self):
        res = self.run_it("--land", "x", FAKE_DOCS_FAIL=1)
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        self.assertIn("check_shard_metrics.py --write", res.stdout)
        self.assertEqual(git(self.repo, "status", "--porcelain", "--untracked-files=no"), "")
        self.assertIn("Regenerate derived artifacts", git(self.repo, "log", "-1", "--format=%s"))

    def test_derived_number_mismatch_stops_and_prints_the_line(self):
        res = self.run_it("--land", "x", FAKE_DOCS_FAIL=1, FAKE_DERIVED_FAIL=1)
        self.assertEqual(res.returncode, 1)
        self.assertIn("docs/x.md:7: jump tables 12 != 13", res.stderr)
        self.assertIn("== FAILED at check-docs", res.stdout)
        self.assertFalse((self.repo / ".repaired").exists())

    def test_stale_lock_is_reclaimed(self):
        dead = subprocess.Popen(["true"])
        dead.wait()
        lock = self.repo / ".git/integrate.lock"
        lock.mkdir()
        (lock / "pid").write_text(f"{dead.pid}\n")
        res = self.run_it("x")
        self.assertEqual(res.returncode, 0, res.stdout + res.stderr)
        self.assertIn("reclaiming stale", res.stdout)
        self.assertFalse(lock.exists())

    def test_second_run_queues_until_the_lock_is_released(self):
        holder = subprocess.Popen(["sleep", "60"])
        self.addCleanup(holder.kill)
        lock = self.repo / ".git/integrate.lock"
        lock.mkdir()
        (lock / "pid").write_text(f"{holder.pid}\n")
        proc = subprocess.Popen(["bash", str(SCRIPT), "x"], cwd=self.repo, env=self.env,
                                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        time.sleep(1.0)
        self.assertIsNone(proc.poll(), "second run must wait, not overlap")
        self.assertEqual((self.repo / "a.txt").read_text(), "base\n")
        holder.kill()
        holder.wait()
        out, _ = proc.communicate(timeout=30)
        self.assertEqual(proc.returncode, 0, out)
        self.assertIn("queued behind", out)
        self.assertEqual((self.repo / "a.txt").read_text(), "lane\n")


if __name__ == "__main__":
    unittest.main()
