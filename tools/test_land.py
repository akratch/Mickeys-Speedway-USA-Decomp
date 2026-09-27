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
    def run_landing(self, fail_verify=False):
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
    elif args == ["log", "--oneline", "-1"]:
        print("fixture landing")
if name == "gmake" and args == ["verify"] and os.environ["LAND_TEST_FAIL"] == "1":
    sys.exit(7)
if name == "authorizer":
    sys.exit(99)
'''
            for name in ("git", "gmake", "authorizer"):
                path = bin_dir / name
                path.write_text(shim)
                path.chmod(0o755)
            env = dict(os.environ, PATH=str(bin_dir) + os.pathsep + os.environ["PATH"],
                       PYTHON=str(bin_dir / "authorizer"), LAND_TEST_LOG=str(log),
                       LAND_TEST_ROOT=str(root), LAND_TEST_FAIL=str(int(fail_verify)))
            result = subprocess.run(["bash", str(ROOT / "tools/land.sh")],
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


if __name__ == "__main__":
    unittest.main()
