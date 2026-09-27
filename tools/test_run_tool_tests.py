#!/usr/bin/env python3
"""Keep test discovery and the legacy recipe's invocation contracts intact."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock

import run_tool_tests as runner


class DiscoveryTests(unittest.TestCase):
    def test_new_tool_and_repository_tests_are_discovered_once(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for directory in ("tools", "tests"):
                (root / directory).mkdir()
                (root / directory / "test_new.py").touch()
                (root / directory / "helper.py").touch()
            self.assertEqual(
                [p.relative_to(root).as_posix() for p in runner.discover(root)],
                ["tests/test_new.py", "tools/test_new.py"],
            )

    def test_all_shipped_test_files_and_overrides_are_discovered(self):
        found = runner.discover(runner.REPO_ROOT)
        expected = set(runner.REPO_ROOT.glob("tools/test_*.py"))
        expected.update(runner.REPO_ROOT.glob("tests/test_*.py"))
        self.assertEqual(set(found), expected)
        self.assertEqual(len(found), len(set(found)))
        relative = {p.relative_to(runner.REPO_ROOT).as_posix() for p in found}
        self.assertTrue(set(runner.CLASS_RESTRICTIONS) <= relative)
        self.assertTrue(set(runner.INTERPRETER_OVERRIDES) <= relative)


class InvocationTests(unittest.TestCase):
    def invocation(self, path):
        result = subprocess.CompletedProcess([], 0, "Ran 1 test\n\nOK\n")
        with mock.patch.object(runner.subprocess, "run", return_value=result) as run:
            runner.run_one(runner.REPO_ROOT / path, "host-python")
        self.assertEqual(run.call_args.kwargs["cwd"], runner.REPO_ROOT)
        return run.call_args.args[0]

    def test_ordinary_test_uses_the_callers_interpreter(self):
        self.assertEqual(self.invocation("tests/test_make_layout.py"),
                         ["host-python", str(runner.REPO_ROOT / "tests/test_make_layout.py")])

    def test_score_symbol_keeps_the_build_free_class_restriction(self):
        self.assertEqual(self.invocation("tools/test_score_symbol.py"),
                         ["host-python", str(runner.REPO_ROOT / "tools/test_score_symbol.py"),
                          "ForcedObjectTests"])

    def test_context_and_fidelity_suites_keep_the_venv_interpreter(self):
        for name in ("test_raw_asm_census.py", "test_candidate_context.py",
                     "test_source_fidelity.py"):
            with self.subTest(name=name):
                self.assertEqual(self.invocation("tools/" + name),
                                 [str(runner.VENV_PYTHON), str(runner.REPO_ROOT / "tools" / name)])


if __name__ == "__main__":
    unittest.main()
