#!/usr/bin/env python3
"""Discover and run every tool test file, so a new test_*.py is never
silently excluded from `gmake check-tooling`.

Before this script, check-tooling was a hand-maintained list of `gmake`
recipe lines in the Makefile, one per file. A new tools/test_*.py or
tests/test_*.py needed a matching line added by hand or it just never ran;
nothing detected the gap. This script instead discovers every
`tools/test_*.py` and `tests/test_*.py` file in the tree and runs each one,
so the set of tests that runs is always the set of tests that exists.

Each discovered file is still run as its own subprocess -- exactly as the
old hand-written recipe lines did (`$(HOST_PYTHON) tools/test_foo.py`) --
because a handful of files need something other than "run this file with
the default interpreter and no arguments":

  * `tools/test_score_symbol.py` exercises real compiles in most of its
    classes, but `ForcedObjectTests` mocks out `compile_configured_tu` and
    `process_item` entirely, so it alone needs no build. check-tooling is
    documented (tools/gates.sh) as needing no build, so only that class
    runs here -- the same restriction the Makefile recipe used to encode as
    `test_score_symbol.py ForcedObjectTests`.

  * `tools/test_raw_asm_census.py`, `tools/test_candidate_context.py` and
    `tools/test_source_fidelity.py` run under the venv interpreter
    (`.venv/bin/python`, `$(PYTHON)` in the Makefile) rather than the host
    interpreter (`$(HOST_PYTHON)`, plain `python3`) that every other file
    uses. For the latter two this is load-bearing: they import
    `candidate_context`/`permute_batch`, which reach into
    `tools/permuter`'s vendored `perm_pycparser`, installed only in the
    venv. For `test_raw_asm_census.py` it is parity rather than necessity:
    the module it tests, `raw_asm_census.py`, is itself only ever invoked
    as `$(PYTHON) tools/raw_asm_census.py --check-overlays` (the
    `check-raw-asm` target), so the test runs under the same interpreter
    as production to keep that behaviour honest even though the module's
    own imports (stdlib plus PyYAML) happen to be satisfied by either
    interpreter today.

Usage:
    tools/run_tool_tests.py                 # run every discovered test file
    tools/run_tool_tests.py --list          # print what would run, and how
    tools/run_tool_tests.py --only PATTERN  # fnmatch against the relative path
    tools/run_tool_tests.py --timing        # also print each file's duration,
                                             # slowest first, after the run
"""

from __future__ import annotations

import argparse
import fnmatch
import re
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
VENV_PYTHON = REPO_ROOT / ".venv" / "bin" / "python"

# relative-to-repo-root path -> extra argv (e.g. a specific TestCase class)
CLASS_RESTRICTIONS = {
    "tools/test_score_symbol.py": ["ForcedObjectTests"],
}

# relative-to-repo-root path -> interpreter to run it with, overriding the
# default of "whatever interpreter is running this script". See the module
# docstring for why each of these needs the venv interpreter.
INTERPRETER_OVERRIDES = {
    "tools/test_raw_asm_census.py": VENV_PYTHON,
    "tools/test_candidate_context.py": VENV_PYTHON,
    "tools/test_source_fidelity.py": VENV_PYTHON,
}

SUMMARY_RE = re.compile(r"^(OK|FAILED)(?:\s*\((?P<detail>[^)]*)\))?\s*$", re.MULTILINE)
RAN_RE = re.compile(r"^Ran (\d+) tests?", re.MULTILINE)


@dataclass
class Result:
    path: str
    returncode: int
    duration: float
    total: int | None = None
    failures: int = 0
    errors: int = 0
    skipped: int = 0
    reason: str | None = None  # set when counts could not be parsed
    output: str = field(default="", repr=False)

    @property
    def ok(self) -> bool:
        return self.returncode == 0

    @property
    def passed(self) -> int | None:
        if self.total is None:
            return None
        return self.total - self.failures - self.errors - self.skipped


def discover(repo_root: Path) -> list[Path]:
    found = set()
    found.update(repo_root.glob("tools/test_*.py"))
    found.update(repo_root.glob("tests/test_*.py"))
    return sorted(found, key=lambda p: p.relative_to(repo_root).as_posix())


def parse_unittest_output(output: str) -> tuple[int | None, int, int, int]:
    """Return (total, failures, errors, skipped) from a unittest TextTestRunner
    trailer, or (None, 0, 0, 0) if the output doesn't look like one."""
    ran = RAN_RE.search(output)
    summary = SUMMARY_RE.search(output)
    if not ran or not summary:
        return None, 0, 0, 0
    total = int(ran.group(1))
    detail = summary.group("detail") or ""
    counts = dict(re.findall(r"(\w+)=(\d+)", detail))
    failures = int(counts.get("failures", 0))
    errors = int(counts.get("errors", 0))
    skipped = int(counts.get("skipped", 0))
    return total, failures, errors, skipped


def run_one(path: Path, default_interpreter: str) -> Result:
    rel = path.relative_to(REPO_ROOT).as_posix()
    interpreter = str(INTERPRETER_OVERRIDES.get(rel, default_interpreter))
    argv = [interpreter, str(path)] + CLASS_RESTRICTIONS.get(rel, [])
    start = time.monotonic()
    proc = subprocess.run(
        argv, cwd=REPO_ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True,
    )
    duration = time.monotonic() - start
    total, failures, errors, skipped = parse_unittest_output(proc.stdout)
    reason = None if total is not None else "could not parse a unittest summary"
    return Result(
        path=rel, returncode=proc.returncode, duration=duration,
        total=total, failures=failures, errors=errors, skipped=skipped,
        reason=reason, output=proc.stdout,
    )


def describe(path: Path) -> str:
    rel = path.relative_to(REPO_ROOT).as_posix()
    bits = []
    if rel in INTERPRETER_OVERRIDES:
        bits.append(f"interpreter={INTERPRETER_OVERRIDES[rel]}")
    if rel in CLASS_RESTRICTIONS:
        bits.append(f"only={','.join(CLASS_RESTRICTIONS[rel])}")
    return f"{rel}" + (f"  ({'; '.join(bits)})" if bits else "")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--only", metavar="PATTERN",
                         help="fnmatch pattern against the repo-relative path, "
                              "e.g. --only 'tools/test_lane_*.py'")
    parser.add_argument("--list", action="store_true",
                         help="print the discovered files and exit, without running")
    parser.add_argument("--timing", action="store_true",
                         help="print each file's duration, slowest first, after the run")
    args = parser.parse_args(argv)

    files = discover(REPO_ROOT)
    if args.only:
        files = [f for f in files
                 if fnmatch.fnmatch(f.relative_to(REPO_ROOT).as_posix(), args.only)]

    if args.list:
        for f in files:
            print(describe(f))
        print(f"\n{len(files)} test file(s) discovered")
        return 0

    if not files:
        print("run_tool_tests: no test files matched", file=sys.stderr)
        return 1

    default_interpreter = sys.executable
    results: list[Result] = []
    any_failed = False
    for f in files:
        result = run_one(f, default_interpreter)
        results.append(result)
        if result.ok and result.total is not None:
            status = "PASS"
            counts = f"{result.passed} passed, {result.failures} failed, " \
                     f"{result.errors} errors, {result.skipped} skipped"
        elif result.ok:
            status = "PASS"
            counts = f"(exit 0, {result.reason})"
        else:
            status = "FAIL"
            any_failed = True
            counts = (f"{result.passed} passed, {result.failures} failed, "
                      f"{result.errors} errors, {result.skipped} skipped"
                      if result.total is not None
                      else f"(exit {result.returncode}, {result.reason})")
        print(f"{status}  {result.path:<48} {counts}  ({result.duration:.2f}s)")
        if status == "FAIL":
            tail = "\n".join(result.output.splitlines()[-20:])
            print("      | " + tail.replace("\n", "\n      | "))

    total_files = len(results)
    failed_files = [r for r in results if not r.ok]
    print()
    if failed_files:
        print(f"FAILED: {len(failed_files)}/{total_files} test file(s) failed:")
        for r in failed_files:
            print(f"  - {r.path}")
    else:
        print(f"all {total_files} test file(s) passed")

    if args.timing:
        print("\nslowest files:")
        for r in sorted(results, key=lambda r: r.duration, reverse=True)[:10]:
            print(f"  {r.duration:7.2f}s  {r.path}")

    return 1 if any_failed else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
