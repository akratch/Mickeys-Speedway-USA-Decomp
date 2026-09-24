#!/usr/bin/env python3
"""Disposable Git fixtures; compiler, build, lane creation and search stubs only."""
from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

TOOLS = Path(__file__).resolve().parent
REAL_GIT = shutil.which("git")

STUB = '''#!{python}
import json, os, pathlib, subprocess, sys
kind = pathlib.Path(sys.argv[0]).name
args = sys.argv[1:]
with open(os.environ["SWEEP_TEST_EVENTS"], "a") as log:
    log.write(json.dumps([kind, args]) + "\\n")
if kind == "git":
    if os.environ.get("SWEEP_TEST_GIT_FAIL") in args:
        sys.exit(128)
    os.execv(os.environ["SWEEP_TEST_GIT"], ["git", *args])
if kind == "python":
    if args[:1] == ["-B"]:
        if "SWEEP_TEST_CPU_COUNT" in os.environ:
            count = os.environ["SWEEP_TEST_CPU_COUNT"]
            value = None if count == "None" else int(count)
            args[2] = "import os; os.cpu_count = lambda: " + repr(value) + "\\n" + args[2]
        os.execv({python!r}, [{python!r}, *args])
    print("synthetic batch/progress output")
    sys.exit(int(os.environ.get("SWEEP_TEST_BATCH_EXIT", "0")))
if kind == "new_lane.sh":
    root = pathlib.Path(os.environ["SWEEP_TEST_ROOT"])
    lane = root.parent / ("mickey-lane-" + args[0])
    subprocess.run([os.environ["SWEEP_TEST_GIT"], "-C", str(root),
                    "worktree", "add", "-q", "-b", "lane/" + args[0],
                    str(lane), args[-1]], check=True)
    (lane / ".venv").symlink_to(root / ".venv", target_is_directory=True)
    sys.exit(0)
if kind == "gmake":
    print("synthetic build proof")
    sys.exit(int(os.environ.get("SWEEP_TEST_BUILD_EXIT", "0")))
sys.exit(99)
'''


class SweepCliTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.top = Path(self.temp.name).resolve()
        self.repo = self.top / "repo"
        self.repo.mkdir()
        self.events_path = self.top / "events.jsonl"
        self.bin = self.top / "bin"
        self.bin.mkdir()
        self.env = dict(os.environ)
        for name in ("GIT_DIR", "GIT_WORK_TREE", "GIT_COMMON_DIR", "GIT_INDEX_FILE"):
            self.env.pop(name, None)
        self.env.update(PATH=str(self.bin) + os.pathsep + os.environ["PATH"],
                        SWEEP_TEST_EVENTS=str(self.events_path),
                        SWEEP_TEST_GIT=REAL_GIT, SWEEP_TEST_ROOT=str(self.repo))
        for name in ("git", "gmake"):
            self.stub(self.bin / name)
        (self.repo / "tools").mkdir()
        shutil.copy2(TOOLS / "permute_sweep.sh", self.repo / "tools/permute_sweep.sh")
        # Exercise the production argparse implementation, not a mirrored parser.
        (self.repo / "tools/permute_batch.py").symlink_to(TOOLS / "permute_batch.py")
        self.stub(self.repo / "tools/new_lane.sh")
        self.stub(self.repo / ".venv/bin/python")
        (self.repo / ".gitignore").write_text(".venv\nbuild/\n")
        (self.repo / "source.c").write_text("int fixture;\n")
        self.git("init", "-q", "-b", "campaign/unchain")
        self.git("config", "user.name", "Sweep fixture")
        self.git("config", "user.email", "fixture@example.invalid")
        self.git("config", "commit.gpgSign", "false")
        self.git("add", ".")
        self.git("commit", "-qm", "fixture base")

    def stub(self, path):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(STUB.format(python=sys.executable))
        path.chmod(0o755)

    def git(self, *args, repo=None, check=True):
        return subprocess.run([REAL_GIT, "-C", str(repo or self.repo), *args],
                              text=True, capture_output=True, check=check).stdout.strip()

    def lane(self, name="owned", branch=None, detached=False):
        lane = self.top / ("mickey-lane-" + name)
        args = ["worktree", "add", "-q"]
        args += ["--detach"] if detached else ["-b", branch or "lane/" + name]
        self.git(*args, str(lane), "HEAD")
        (lane / ".venv").symlink_to(self.repo / ".venv", target_is_directory=True)
        return lane

    def run_cli(self, *args, script=None, shell="bash", **env):
        return subprocess.run([shell, str(script or self.repo / "tools/permute_sweep.sh"), *args],
                              env={**self.env, **env}, cwd=self.top,
                              text=True, capture_output=True,
                              # A hang guard, not a latency bound: the CLI
                              # runs git and python children that a loaded
                              # machine can slow well past 15 s.
                              timeout=120)

    def events(self):
        if not self.events_path.exists():
            return []
        return [json.loads(line) for line in self.events_path.read_text().splitlines()]

    def no_mutation(self):
        for kind, args in self.events():
            self.assertNotIn(kind, ("gmake", "new_lane.sh"))
            if kind == "git":
                self.assertNotIn("merge", args)
            if kind == "python":
                self.assertEqual(args[0], "-B")

    def test_help_anywhere_and_no_arguments_have_no_subprocesses(self):
        for args in ((), ("--help",), ("-h",), ("--promote", "owned", "--", "--help")):
            with self.subTest(args=args):
                result = self.run_cli(*args)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn("Usage:", result.stdout)
                self.assertEqual(self.events(), [])
        self.assertFalse((self.top / "mickey-lane---help").exists())

    def test_legacy_implicit_mode_and_invalid_lane_names_fail_before_calls(self):
        for args in (("owned",), ("--report-only",), ("--unknown",),
                     *(("--report-only", name) for name in
                       ("--bad", "../elsewhere", "a/b", ".", "UPPER", "a b", "a" * 65)),
                     ("--report-only", "owned", "--minutes", "1")):
            with self.subTest(args=args):
                self.assertNotEqual(self.run_cli(*args).returncode, 0)
                self.assertEqual(self.events(), [])

    def test_forwarded_mode_abbreviations_and_invalid_args_fail_before_git(self):
        for flag in ("--apply", "--app", "--commit", "--comm", "--list", "--bogus"):
            with self.subTest(flag=flag):
                result = self.run_cli("--report-only", "owned", "--", flag)
                self.assertNotEqual(result.returncode, 0)
        self.assertTrue(all(kind == "python" for kind, _ in self.events()))

    def test_report_only_never_passes_promotion_flags_or_postbuilds(self):
        lane = self.lane()
        result = self.run_cli("--report-only", "owned", "--", "--function", "fixture", "--minutes", "3")
        self.assertEqual(result.returncode, 0, result.stderr)
        batch = [args for kind, args in self.events() if kind == "python" and args[:1] == ["-u"]]
        self.assertEqual(len(batch), 1)
        self.assertNotIn("--apply", batch[0])
        self.assertNotIn("--commit", batch[0])
        self.assertEqual(batch[0][-4:], ["--function", "fixture", "--minutes", "3"])
        jobs = "-j" + str(os.cpu_count() or 1)
        self.assertEqual([args for kind, args in self.events() if kind == "gmake"],
                         [[jobs, "extract"], [jobs], [jobs], [jobs, "verify"]])
        self.assertEqual(self.git("status", "--porcelain", repo=lane), "")

    def assert_build_jobs(self, count, *, promoted=False):
        builds = [args for kind, args in self.events() if kind == "gmake"]
        jobs = "-j" + str(count)
        expected = [[jobs, "extract"], [jobs], [jobs], [jobs, "verify"]]
        if promoted:
            expected += [[jobs, "extract"], [jobs], [jobs, "verify"]]
        self.assertEqual(builds, expected)
        batch = next(args for kind, args in self.events() if kind == "python" and args[:1] == ["-u"])
        self.assertEqual(batch[batch.index("--build-jobs") + 1], str(count))
        for flag, value in (("--jobs", "2"), ("--permuter-threads", "4"),
                            ("--minutes", "20"), ("--flat-minutes", "6"),
                            ("--max-total-minutes", "120"), ("--extend-minutes", "20"),
                            ("--load-threshold", "13")):
            self.assertEqual(batch[batch.index(flag) + 1], value)

    def test_explicit_build_jobs_reach_every_build_and_runner(self):
        forms = (("--build-jobs", "3"), ("--build-jobs=3",),
                 ("--build-jobs", "8", "--build-jobs=3"),
                 ("--build-jobs=8", "--build-jobs", "3"),
                 ("--build-j", "3"))
        for mode in ("--report-only", "--promote"):
            for index, args in enumerate(forms):
                with self.subTest(mode=mode, args=args):
                    self.events_path.unlink(missing_ok=True)
                    name = ("report" if mode == "--report-only" else "promote") + str(index)
                    self.lane(name)
                    result = self.run_cli(mode, name, "--", *args)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assert_build_jobs(3, promoted=mode == "--promote")

    def test_machine_core_default_and_unavailable_fallback(self):
        for value, expected in (("14", 14), ("None", 1)):
            with self.subTest(cpu_count=value):
                self.events_path.unlink(missing_ok=True)
                self.lane("cpu" + value.lower())
                result = self.run_cli("--report-only", "cpu" + value.lower(),
                                      SWEEP_TEST_CPU_COUNT=value)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assert_build_jobs(expected)

    def test_invalid_build_jobs_fail_before_git_or_build(self):
        for args in (("--build-jobs", "0"), ("--build-jobs=-1",),
                     ("--build-jobs", "invalid"), ("--build-jobs",),
                     ("--build-jobs", "3", "--build-jobs=0")):
            with self.subTest(args=args):
                self.events_path.unlink(missing_ok=True)
                result = self.run_cli("--report-only", "fresh", "--", *args)
                self.assertNotEqual(result.returncode, 0)
                self.assertTrue(all(kind == "python" for kind, _ in self.events()))
                self.assertFalse((self.top / "mickey-lane-fresh").exists())

    def test_explicit_promote_retains_proof_and_commit_workflow(self):
        self.lane()
        result = self.run_cli("--promote", "owned")
        self.assertEqual(result.returncode, 0, result.stderr)
        batch = next(args for kind, args in self.events() if kind == "python" and args[:1] == ["-u"])
        self.assertIn("--apply", batch)
        self.assertIn("--commit", batch)
        self.assertEqual(sum(kind == "gmake" for kind, _ in self.events()), 7)

    def test_wrong_branch_and_detached_head_are_preserved(self):
        for name, branch, detached in (("other", "different", False), ("detached", None, True)):
            lane = self.lane(name, branch, detached)
            before = self.git("rev-parse", "HEAD", repo=lane)
            self.assertNotEqual(self.run_cli("--report-only", name).returncode, 0)
            self.assertEqual(self.git("rev-parse", "HEAD", repo=lane), before)
        self.no_mutation()

    def test_dirty_index_and_worktree_are_preserved(self):
        lane = self.lane()
        (lane / "source.c").write_text("int independent;\n")
        self.git("add", "source.c", repo=lane)
        before = self.git("diff", "--cached", repo=lane)
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        self.assertEqual(self.git("diff", "--cached", repo=lane), before)
        self.no_mutation()

    def test_divergent_lane_is_preserved(self):
        lane = self.lane()
        self.git("commit", "--allow-empty", "-qm", "independent", repo=lane)
        before = self.git("rev-parse", "HEAD", repo=lane)
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        self.assertEqual(self.git("rev-parse", "HEAD", repo=lane), before)
        self.no_mutation()

    def test_foreign_symlink_and_plain_directory_fail_closed(self):
        foreign = self.lane("foreign")
        (self.top / "mickey-lane-owned").symlink_to(foreign, target_is_directory=True)
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        (self.top / "mickey-lane-plain").mkdir()
        self.assertNotEqual(self.run_cli("--report-only", "plain").returncode, 0)
        self.no_mutation()

    def test_existing_branch_without_path_is_not_stolen(self):
        self.git("branch", "lane/owned")
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        self.no_mutation()

    def test_new_lane_uses_resolved_base_and_is_checked(self):
        result = self.run_cli("--report-only", "fresh")
        self.assertEqual(result.returncode, 0, result.stderr)
        created = next(args for kind, args in self.events() if kind == "new_lane.sh")
        self.assertEqual(created, ["fresh", "--no-extract", self.git("rev-parse", "HEAD")])

    def test_call_from_linked_worktree_resolves_primary_repository(self):
        caller = self.lane("caller")
        self.lane()
        result = self.run_cli("--report-only", "owned", script=caller / "tools/permute_sweep.sh")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_git_override_rejected_and_help_still_safe(self):
        result = self.run_cli("--report-only", "owned", GIT_INDEX_FILE="elsewhere")
        self.assertNotEqual(result.returncode, 0)
        self.assertTrue(all(kind == "python" for kind, _ in self.events()))
        self.assertEqual(self.run_cli("--help", GIT_DIR="elsewhere").returncode, 0)

    def test_build_failure_never_launches_search(self):
        self.lane()
        result = self.run_cli("--promote", "owned", SWEEP_TEST_BUILD_EXIT="17")
        self.assertEqual(result.returncode, 17)
        self.assertFalse(any(kind == "python" and args[:1] == ["-u"] for kind, args in self.events()))

    def test_batch_failure_is_nonzero_and_preserves_log_without_postbuilds(self):
        lane = self.lane()
        result = self.run_cli("--promote", "owned", SWEEP_TEST_BATCH_EXIT="23")
        self.assertEqual(result.returncode, 23, result.stderr)
        self.assertEqual(sum(kind == "gmake" for kind, _ in self.events()), 4)
        logs = list((lane / "build/permuter").glob("sweep.log.*"))
        self.assertEqual(len(logs), 1)
        self.assertIn("synthetic", logs[0].read_text())

    def test_repeated_runs_do_not_overwrite_logs(self):
        lane = self.lane()
        for _ in range(2):
            result = self.run_cli("--report-only", "owned")
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(list((lane / "build/permuter").glob("sweep.log.*"))), 2)

    def test_system_bash_report_only_empty_mode_array(self):
        self.lane()
        result = self.run_cli("--report-only", "owned", shell="/bin/bash")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_busy_index_is_preserved(self):
        lane = self.lane()
        admin = Path(self.git("rev-parse", "--absolute-git-dir", repo=lane))
        lock = admin / "index.lock"
        lock.write_text("independent writer")
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        self.assertEqual(lock.read_text(), "independent writer")
        self.no_mutation()

    def test_other_repository_worktree_is_preserved(self):
        foreign = self.top / "foreign"
        self.git("clone", "-q", str(self.repo), str(foreign))
        lane = self.top / "mickey-lane-owned"
        self.git("worktree", "add", "-q", "-b", "lane/owned", str(lane), repo=foreign)
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        self.no_mutation()

    def test_registered_noindex_alias_is_accepted(self):
        lane = self.lane()
        physical = Path(str(lane) + ".noindex")
        self.git("worktree", "move", str(lane), str(physical))
        lane.symlink_to(physical, target_is_directory=True)
        result = self.run_cli("--report-only", "owned")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_broken_lane_symlink_is_not_replaced(self):
        lane = self.top / "mickey-lane-owned"
        lane.symlink_to(self.top / "missing", target_is_directory=True)
        self.assertNotEqual(self.run_cli("--report-only", "owned").returncode, 0)
        self.assertTrue(lane.is_symlink())
        self.no_mutation()

    def test_status_failure_does_not_mean_clean(self):
        self.lane()
        self.assertNotEqual(self.run_cli("--report-only", "owned", SWEEP_TEST_GIT_FAIL="status").returncode, 0)
        self.no_mutation()

    def test_branch_lookup_failure_does_not_mean_absent(self):
        self.assertNotEqual(self.run_cli("--report-only", "fresh", SWEEP_TEST_GIT_FAIL="show-ref").returncode, 0)
        self.no_mutation()


if __name__ == "__main__":
    unittest.main()
