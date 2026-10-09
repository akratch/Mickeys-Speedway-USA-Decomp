#!/usr/bin/env python3
"""Exercise the landing sequence without running builds or contacting remotes."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LandingTests(unittest.TestCase):
    def run_landing(self, fail_verify=False, *, release=False, unintegrated=False,
                    outdated=False, fail_gate=False, changed_tree=False,
                    fail_history=0):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            bin_dir = root / "bin"
            bin_dir.mkdir()
            log = root / "commands.jsonl"
            shim = "#!" + sys.executable + "\n" + '''import json, os, pathlib, sys
name = pathlib.Path(sys.argv[0]).name
args = sys.argv[1:]
with open(os.environ["LAND_TEST_LOG"], "a") as stream:
    stream.write(json.dumps([name, *args]) + "\\n")
if name == "git":
    if args == ["rev-parse", "--show-toplevel"]:
        print(os.environ["LAND_TEST_ROOT"])
    elif args == ["rev-parse", "--abbrev-ref", "HEAD"]:
        print("campaign/unchain")
    elif args == ["rev-parse", "--verify", "--end-of-options", "reviewed-release^{commit}"]:
        print("a" * 40)
    elif args == ["rev-parse", "--verify", "refs/heads/campaign/unchain"]:
        print("b" * 40)
    elif args == ["rev-parse", "--verify", "HEAD"]:
        print("c" * 40)
    elif args[:2] == ["merge-base", "--is-ancestor"]:
        if args[2] == "a" * 40 and os.environ["LAND_TEST_UNINTEGRATED"] == "1":
            sys.exit(1)
        if args[2] == "refs/remotes/origin/master" and os.environ["LAND_TEST_OUTDATED"] == "1":
            sys.exit(1)
    elif args == ["diff", "--quiet", "a" * 40, "c" * 40] and os.environ["LAND_TEST_TREE"] == "1":
        sys.exit(1)
    elif args == ["log", "--oneline", "-1"]:
        print("fixture landing")
if name == "gmake" and args == ["verify"] and os.environ["LAND_TEST_FAIL"] == "1":
    sys.exit(7)
if name == "gmake" and args == ["cleanroom", "check-docs", "check-scoreboard"] and os.environ["LAND_TEST_GATE"] == "1":
    sys.exit(8)
if name == "authorizer":
    sys.exit(99)
if name == "python3":
    assert args == ["tools/lane_status.py", "--check-reopen-history", "--base", "HEAD"]
    calls = sum(json.loads(line)[0] == "python3"
                for line in pathlib.Path(os.environ["LAND_TEST_LOG"]).read_text().splitlines())
    if calls == int(os.environ["LAND_TEST_HISTORY"]):
        sys.exit(9)
'''
            for name in ("git", "gmake", "authorizer", "python3"):
                path = bin_dir / name
                path.write_text(shim)
                path.chmod(0o755)
            env = dict(os.environ, PATH=str(bin_dir) + os.pathsep + os.environ["PATH"],
                       PYTHON=str(bin_dir / "authorizer"), LAND_TEST_LOG=str(log),
                       LAND_TEST_ROOT=str(root), LAND_TEST_FAIL=str(int(fail_verify)),
                       LAND_TEST_UNINTEGRATED=str(int(unintegrated)),
                       LAND_TEST_OUTDATED=str(int(outdated)), LAND_TEST_GATE=str(int(fail_gate)),
                       LAND_TEST_TREE=str(int(changed_tree)),
                       LAND_TEST_HISTORY=str(fail_history))
            result = subprocess.run(["bash", str(ROOT / "tools/land.sh")] +
                                    (["--release-ref", "reviewed-release"] if release else []),
                                    cwd=root, env=env, text=True, capture_output=True)
            commands = [json.loads(line) for line in log.read_text().splitlines()]
            return result, commands

    def assert_no_reauthorization(self, commands):
        self.assertFalse(any(c[0] == "authorizer" for c in commands))
        self.assertFalse(any(c[:2] == ["git", "commit"] for c in commands))
        self.assertFalse(any("authorize_reopen.py" in arg for c in commands for arg in c))
        self.assertEqual(commands[-1], ["git", "checkout", "-q", "campaign/unchain"])

    def test_success_pushes_verified_merge_and_restores_branch(self):
        result, commands = self.run_landing()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual([c for c in commands if c[:2] == ["git", "push"]],
                         [["git", "push", "origin", "campaign/unchain"],
                          ["git", "push", "origin", "master"]])
        merge = commands.index(["git", "merge", "--no-edit", "campaign/unchain"])
        aliases = commands.index(["gmake", "overlay-syms"])
        verify = commands.index(["gmake", "verify"])
        push = commands.index(["git", "push", "origin", "master"])
        self.assertLess(merge, aliases)
        self.assertLess(aliases, verify)
        self.assertLess(verify, push)
        self.assert_no_reauthorization(commands)

    def test_failed_verification_does_not_push_master_and_restores_branch(self):
        result, commands = self.run_landing(fail_verify=True)
        self.assertEqual(result.returncode, 7)
        self.assertEqual([c for c in commands if c[:2] == ["git", "push"]],
                         [["git", "push", "origin", "campaign/unchain"]])
        self.assertIn(["gmake", "verify"], commands)
        self.assert_no_reauthorization(commands)

    def test_invalid_authorization_history_stops_publication(self):
        for release in (False, True):
            for failure in (1, 2):
                with self.subTest(release=release, failure=failure):
                    result, commands = self.run_landing(
                        release=release, fail_history=failure,
                    )
                    self.assertEqual(result.returncode, 9, result.stderr)
                    pushes = [c for c in commands if c[:2] == ["git", "push"]]
                    expected = ([["git", "push", "origin", "campaign/unchain"]]
                                if failure == 2 and not release else [])
                    self.assertEqual(pushes, expected)
                    self.assertFalse(any(c[0] == "gmake" for c in commands))
                    self.assert_no_reauthorization(commands)

    def test_curated_release_publishes_only_pinned_master_after_all_gates(self):
        result, commands = self.run_landing(release=True)
        self.assertEqual(0, result.returncode, result.stderr)
        ancestry = ["git", "merge-base", "--is-ancestor", "a" * 40, "b" * 40]
        merge = ["git", "merge", "--no-edit", "a" * 40]
        gates = ["gmake", "cleanroom", "check-docs", "check-scoreboard"]
        push_master = ["git", "push", "origin", "c" * 40 + ":refs/heads/master"]
        self.assertLess(commands.index(ancestry), commands.index(merge))
        for ref in ("refs/heads/master", "refs/remotes/origin/master"):
            self.assertLess(commands.index(["git", "merge-base", "--is-ancestor", ref, "a" * 40]),
                            commands.index(merge))
        self.assertLess(commands.index(merge), commands.index(["gmake", "overlay-syms"]))
        self.assertLess(commands.index(["gmake", "overlay-syms"]), commands.index(["gmake", "verify"]))
        self.assertLess(commands.index(["gmake", "verify"]), commands.index(gates))
        self.assertLess(commands.index(gates), commands.index(push_master))
        self.assertEqual([push_master], [c for c in commands if c[:2] == ["git", "push"]])
        self.assert_no_reauthorization(commands)

    def test_curated_release_rejects_unintegrated_or_outdated_ref_before_publication(self):
        for defect in ("unintegrated", "outdated"):
            with self.subTest(defect=defect):
                result, commands = self.run_landing(release=True, **{defect: True})
                self.assertNotEqual(0, result.returncode)
                self.assertFalse(any(c[:2] == ["git", "push"] for c in commands))
                self.assertFalse(any(c[0] == "gmake" for c in commands))

    def test_curated_release_gate_failure_does_not_publish_master(self):
        for defect in ("fail_verify", "fail_gate", "changed_tree"):
            with self.subTest(defect=defect):
                result, commands = self.run_landing(release=True, **{defect: True})
                self.assertNotEqual(0, result.returncode)
                self.assertEqual([],
                                 [c for c in commands if c[:2] == ["git", "push"]])
                self.assert_no_reauthorization(commands)


if __name__ == "__main__":
    unittest.main()
