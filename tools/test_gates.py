#!/usr/bin/env python3
"""Unit tests for tools/gates.sh.

The property under test is narrow and it is the one that has actually failed
in practice: a gate that exits nonzero must make the script exit nonzero and
say so. Piping a gate's output through `tail` reports tail's status instead,
which reads a red gate as green; that mistake put commits on top of failing
gates twice in this campaign.

A stub `gmake` on PATH stands in for the real one so these run in
milliseconds and need no build.
"""
import os
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
GATES = ROOT / "tools" / "gates.sh"


def run(stub_body: str, args: list[str]) -> subprocess.CompletedProcess:
    """Run gates.sh with a stub gmake that behaves as `stub_body` says."""
    tmp = pathlib.Path(tempfile.mkdtemp())
    stub = tmp / "gmake"
    stub.write_text("#!/usr/bin/env bash\n" + stub_body + "\n", encoding="utf-8")
    stub.chmod(0o755)
    env = dict(os.environ, PATH=f"{tmp}:{os.environ['PATH']}")
    return subprocess.run([str(GATES), *args], env=env, capture_output=True,
                          text=True, cwd=ROOT)


class GateStatusTests(unittest.TestCase):
    def test_a_failing_gate_fails_the_script(self) -> None:
        out = run('echo "boom"; exit 1', ["check-docs"])
        self.assertNotEqual(out.returncode, 0)
        self.assertIn("FAIL", out.stdout)
        self.assertIn("check-docs", out.stdout)

    def test_a_gate_that_fails_quietly_still_fails(self) -> None:
        """The failure that started this: output looked fine, status did not.
        A gate printing nothing but exiting 1 must not read as a pass."""
        out = run("exit 1", ["check-docs"])
        self.assertNotEqual(out.returncode, 0)
        self.assertIn("FAIL", out.stdout)

    def test_a_gate_printing_success_text_but_exiting_nonzero_fails(self) -> None:
        """Matching on output text rather than status is the same bug wearing
        a different hat."""
        out = run('echo "cleanroom check OK -- 1719 files clean"; exit 1',
                  ["cleanroom"])
        self.assertNotEqual(out.returncode, 0)
        self.assertIn("FAIL", out.stdout)

    def test_passing_gates_exit_zero(self) -> None:
        out = run('echo "fine"; exit 0', ["check-docs"])
        self.assertEqual(out.returncode, 0, out.stdout + out.stderr)
        self.assertIn("PASS", out.stdout)

    def test_one_failure_among_passes_still_fails(self) -> None:
        """The real hazard is a batch where most gates are green."""
        out = run('[ "$1" = check-docs ] && exit 1; exit 0',
                  ["verify", "cleanroom", "check-docs"])
        self.assertNotEqual(out.returncode, 0)
        self.assertIn("GATES FAILED", out.stdout)
        self.assertIn("check-docs", out.stdout.split("GATES FAILED")[1])

    def test_the_failing_gate_is_named_in_the_verdict(self) -> None:
        out = run('[ "$1" = verify ] && exit 3; exit 0',
                  ["verify", "check-docs"])
        verdict = out.stdout.split("GATES FAILED")[1]
        self.assertIn("verify", verdict)
        self.assertNotIn("check-docs", verdict)

    def test_the_default_set_covers_every_no_build_check(self) -> None:
        """check-tooling belongs here even though CLAUDE.md's 'before every
        commit' list omits it. It was in neither set, so a per-TU CFLAGS line
        in the root Makefile turned it red for several commits while every gate
        that WAS run stayed green -- the precise failure this script exists to
        prevent, reproduced by the script's own set being too narrow."""
        out = run('echo "$1" >> /dev/stderr; exit 0', [])
        self.assertEqual(out.returncode, 0)
        for gate in ("verify", "cleanroom", "check-docs", "check-tooling"):
            self.assertIn(gate, out.stdout)

    def test_promotion_adds_the_promotion_gates(self) -> None:
        out = run("exit 0", ["--promotion"])
        for gate in ("check-scoreboard", "check-overlay-syms",
                     "check-nonmatching-builds", "check-promotion-proofs"):
            self.assertIn(gate, out.stdout)

    def test_staged_passes_the_index_flag_to_cleanroom_only(self) -> None:
        out = run('echo "ARGS:$*"; exit 0', ["--staged", "cleanroom", "verify"])
        self.assertEqual(out.returncode, 0)
        log = (ROOT / "build" / "gates" / "cleanroom.log").read_text()
        self.assertIn("CLEANROOM_ARGS=--staged", log)
        verify_log = (ROOT / "build" / "gates" / "verify.log").read_text()
        self.assertNotIn("CLEANROOM_ARGS", verify_log)

    def test_an_unknown_option_is_refused_rather_than_treated_as_a_gate(self) -> None:
        out = run("exit 0", ["--no-verify"])
        self.assertEqual(out.returncode, 2)


if __name__ == "__main__":
    unittest.main()
