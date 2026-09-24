#!/usr/bin/env python3
"""Tests for the promotion-proof census: set derivation, cache, classes, exit."""

from __future__ import annotations

import contextlib
import io
import json
import subprocess
import sys
import tempfile
import threading
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
import promotion_proof_census as census  # noqa: E402


def completed(returncode: int, stdout: str = "", stderr: str = ""):
    return subprocess.CompletedProcess([], returncode, stdout=stdout, stderr=stderr)


class FakeRunner:
    """Answers promotion_proof.py invocations from a table, counting calls."""

    def __init__(self, table):
        self.table = table
        self.calls = []
        self.lock = threading.Lock()

    def __call__(self, command, **_kwargs):
        symbol = command[2]
        with self.lock:
            self.calls.append(command)
        answer = self.table[symbol]
        if isinstance(answer, str):
            return completed(0, json.dumps({"verdict": "exact",
                                            "identity_proof_mode": answer}))
        return completed(2, stderr=answer[0])


ENTRIES = [
    census.Entry("overlay1Alpha", "overlay", "build/a.o"),
    census.Entry("overlay1Beta", "overlay", "build/b.o"),
    census.Entry("func_80001000", "resident", "build/c.o"),
    census.Entry("func_80002000", "resident", "build/d.o", covered=False,
                 why_uncovered="symbol_addrs row does not say matched C"),
]
TABLE = {
    "overlay1Alpha": "static",
    "overlay1Beta": ("usage: x\npromotion_proof.py: error: function preflight failed "
                     "with exit 2: func_overlay_045_F000000C_188B438 encoded ROM "
                     "address conflicts with atlas ownership",),
    "func_80001000": "static-plus-runtime-table-and-linked-rom",
}


class CensusTests(unittest.TestCase):
    def run_census(self, runner, cache, *, elf="elf-1", objects=None, jobs=1):
        objects = objects or {}
        return census.census(ENTRIES, jobs=jobs, cache_path=cache, elf_digest=elf,
                             environment="env", runner=runner,
                             object_digest=lambda obj: objects.get(obj, "o-" + obj))

    def test_uncovered_entries_are_never_proved(self):
        runner = FakeRunner(TABLE)
        outcomes = self.run_census(runner, None)
        self.assertEqual(3, len(outcomes))
        self.assertNotIn("func_80002000", [call[2] for call in runner.calls])

    def test_every_proof_runs_no_build(self):
        runner = FakeRunner(TABLE)
        self.run_census(runner, None, jobs=3)
        self.assertTrue(all("--no-build" in call for call in runner.calls))

    def test_warm_run_reproves_only_failures_and_changed_objects(self):
        with tempfile.TemporaryDirectory() as scratch:
            cache = Path(scratch) / "cache.json"
            first = FakeRunner(TABLE)
            self.run_census(first, cache)
            self.assertEqual(3, len(first.calls))

            warm = FakeRunner(TABLE)
            outcomes = self.run_census(warm, cache)
            self.assertEqual(["overlay1Beta"], [call[2] for call in warm.calls])
            self.assertEqual({"overlay1Alpha": True, "func_80001000": True},
                             {row.symbol: row.cached for row in outcomes if row.ok})

            changed = FakeRunner(TABLE)
            self.run_census(changed, cache, objects={"build/a.o": "edited"})
            self.assertEqual({"overlay1Alpha", "overlay1Beta"},
                             {call[2] for call in changed.calls})

            relinked = FakeRunner(TABLE)
            self.run_census(relinked, cache, elf="elf-2")
            self.assertEqual(3, len(relinked.calls))

    def test_error_class_drops_names_and_numbers(self):
        message = TABLE["overlay1Beta"][0]
        self.assertEqual("<name> encoded ROM address conflicts with atlas ownership",
                         census.error_class(message, "overlay1Beta"))
        self.assertEqual(
            "<name> linked symbol conflicts with canonical overlay ownership",
            census.error_class("error: overlay65Initialize linked symbol conflicts "
                               "with canonical overlay ownership", "overlay65Release"))

    def test_render_exits_nonzero_on_failure_and_tallies_labels(self):
        outcomes = self.run_census(FakeRunner(TABLE), None)
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            status = census.render(outcomes, [ENTRIES[3]], list_uncovered=False,
                                   require_coverage=False, elapsed=0.0)
        text = buffer.getvalue()
        self.assertEqual(1, status)
        self.assertIn("FAIL overlay1Beta", text)
        self.assertIn("1  static\n", text)
        self.assertIn("1  static-plus-runtime-table-and-linked-rom", text)
        self.assertIn("1 uncovered", text)

    def test_render_passes_clean_run_unless_coverage_is_required(self):
        table = dict(TABLE, overlay1Beta="static")
        outcomes = self.run_census(FakeRunner(table), None)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(0, census.render(outcomes, [ENTRIES[3]], list_uncovered=False,
                                              require_coverage=False, elapsed=0.0))
            self.assertEqual(1, census.render(outcomes, [ENTRIES[3]], list_uncovered=False,
                                              require_coverage=True, elapsed=0.0))

    def test_map_parser_finds_the_owning_object(self):
        text = (".overlay_065    0xf0000000     0x2200 load address 0x018c4268\n"
                " build/src/o/a.c.o(.text)\n"
                " .text          0xf0000000       0x80 build/src/o/a.c.o\n"
                "                0xf0000000                a\n"
                " .text          0xf0000080      0xb40 build/src/o/b.c.o\n"
                ".overlay_066    0xf0000000      0xad8 load address 0x018c6468\n"
                " .text          0xf0000000       0x34 build/src/p/c.c.o\n")
        objects = census.map_text_objects(text)
        self.assertEqual("build/src/o/b.c.o",
                         census.object_for(objects, ".overlay_065", 0xF0000100))
        self.assertEqual("build/src/p/c.c.o",
                         census.object_for(objects, ".overlay_066", 0xF0000000))
        self.assertIsNone(census.object_for(objects, ".overlay_066", 0xF0000040))

    def test_zero_size_aliases_pass_only_through_a_proved_definition(self):
        entries = [census.Entry("__sinf", "resident", "build/s.o"),
                   census.Entry("fsin", "resident", "build/s.o", alias_of="__sinf"),
                   census.Entry("func_80001000", "resident", "build/c.o"),
                   census.Entry("func_80001004", "resident", "build/c.o",
                                alias_of="func_80001000"),
                   census.Entry("orphan", "resident", "build/c.o", alias_of="")]
        runner = FakeRunner({"__sinf": "static",
                             "func_80001000": ("promotion_proof.py: error: not exact",)})
        outcomes = {row.symbol: row for row in census.census(
            entries, cache_path=None, elf_digest="e", environment="v", runner=runner,
            object_digest=lambda obj: obj)}
        self.assertEqual({"__sinf", "func_80001000"}, {call[2] for call in runner.calls})
        self.assertTrue(outcomes["fsin"].ok)
        self.assertEqual("zero-size alias", outcomes["fsin"].identity)
        self.assertFalse(outcomes["func_80001004"].ok)
        self.assertFalse(outcomes["orphan"].ok)

    def test_resident_geometry_evidence_is_tallied(self):
        outcomes = [census.Outcome("a", "resident", True, identity="static",
                                   evidence="symbol_addrs function row+progress matched-C rule"),
                    census.Outcome("b", "resident", True, identity="static",
                                   evidence="symbol_addrs function row+progress matched-C rule"),
                    census.Outcome("c", "overlay", True, identity="static", evidence="atlas")]
        with contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(0, census.render(outcomes, [], list_uncovered=False,
                                              require_coverage=True, elapsed=0.0))
        self.assertIn("   2  symbol_addrs function row+progress matched-C rule", out.getvalue())
        self.assertNotIn("atlas", out.getvalue())


if __name__ == "__main__":
    unittest.main()
