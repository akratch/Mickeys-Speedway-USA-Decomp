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
from unittest import mock


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

    def test_changed_inputs_do_not_publish_or_replace_cache(self):
        with tempfile.TemporaryDirectory() as scratch:
            cache = Path(scratch) / "cache.json"
            self.run_census(FakeRunner(TABLE), cache)
            original = cache.read_bytes()

            def changed():
                census.require_unchanged_inputs({"build/a.o": "old"},
                                                {"build/a.o": "new"})

            for destination in (cache, Path(scratch) / "new.json", None):
                with self.subTest(destination=destination), self.assertRaisesRegex(
                        census.InputsChangedError, "cache not updated"):
                    census.census(ENTRIES, cache_path=destination, elf_digest="edited",
                                  environment="env", runner=FakeRunner(TABLE),
                                  object_digest=lambda obj: obj, validate_inputs=changed)
            self.assertEqual(original, cache.read_bytes())
            self.assertFalse((Path(scratch) / "new.json").exists())

    def test_fully_cached_results_still_validate_inputs(self):
        entries = ENTRIES[:1]
        with tempfile.TemporaryDirectory() as scratch:
            cache = Path(scratch) / "cache.json"
            kwargs = dict(cache_path=cache, elf_digest="e", environment="v",
                          object_digest=lambda obj: obj)
            census.census(entries, runner=FakeRunner(TABLE), **kwargs)
            runner = FakeRunner({})
            check = mock.Mock(side_effect=census.InputsChangedError("changed"))
            with self.assertRaises(census.InputsChangedError):
                census.census(entries, runner=runner, validate_inputs=check, **kwargs)
            check.assert_called_once_with()
            self.assertEqual([], runner.calls)

    def test_snapshot_detects_same_content_replacement_and_owner_addition(self):
        with tempfile.TemporaryDirectory() as scratch:
            root = Path(scratch)
            obj = root / "build/a.o"
            obj.parent.mkdir()
            obj.write_bytes(b"synthetic object")
            before = census.input_snapshot(ENTRIES, root=root)
            replacement = obj.with_suffix(".new")
            replacement.write_bytes(obj.read_bytes())
            replacement.replace(obj)
            with self.assertRaisesRegex(census.InputsChangedError, "build/a.o"):
                census.require_unchanged_inputs(before, census.input_snapshot(ENTRIES, root=root))
            before = census.input_snapshot(ENTRIES, root=root)
            owner = root / "asm/new.s"
            owner.parent.mkdir()
            owner.write_text("# synthetic ownership input\n")
            with self.assertRaisesRegex(census.InputsChangedError, "asm/new.s"):
                census.require_unchanged_inputs(before, census.input_snapshot(ENTRIES, root=root))

    def test_rom_content_invalidates_environment_cache_key(self):
        with tempfile.TemporaryDirectory() as scratch:
            root = Path(scratch)
            rom = root / "build/mickey.us.z64"
            rom.parent.mkdir()
            rom.write_bytes(b"synthetic original")
            before = census.proof_environment_digest(root)
            rom.write_bytes(b"synthetic changed")
            self.assertNotEqual(before, census.proof_environment_digest(root))

    def test_source_recipe_and_toolchain_changes_invalidate_warm_cache(self):
        with tempfile.TemporaryDirectory() as scratch:
            root = Path(scratch)
            cache = root / "cache.json"
            entries = ENTRIES[:1]
            for name in ("src/owner.inc", "include/owner.h", "config/normalizations/owner.mk",
                         "config/normalizations/owner.filter.spec", "mickey.us.yaml",
                         "tools/ido/cc", "tools/ido/cfe",
                         "tools/binutils/mips64-elf-ld", "tools/binutils/mips64-elf-objcopy"):
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("synthetic original\n")
                environment = census.proof_environment_digest(root)
                kwargs = dict(cache_path=cache, elf_digest="unchanged ELF",
                              object_digest=lambda obj: "unchanged object")
                census.census(entries, runner=FakeRunner(TABLE), environment=environment, **kwargs)
                before = census.input_snapshot(entries, root=root)
                path.write_text("synthetic changed provenance or recipe\n")
                with self.assertRaises(census.InputsChangedError):
                    census.require_unchanged_inputs(before, census.input_snapshot(entries, root=root))
                runner = FakeRunner(TABLE)
                outcomes = census.census(entries, runner=runner,
                                         environment=census.proof_environment_digest(root), **kwargs)
                self.assertEqual(1, len(runner.calls), name)
                self.assertFalse(outcomes[0].cached, name)

    def test_transient_owner_addition_is_detected(self):
        with tempfile.TemporaryDirectory() as scratch:
            root = Path(scratch)
            owner = root / "asm/transient.s"
            owner.parent.mkdir()
            before = census.input_snapshot(root=root)
            owner.write_text("# synthetic transient owner\n")
            owner.unlink()
            with self.assertRaisesRegex(census.InputsChangedError, "asm"):
                census.require_unchanged_inputs(before, census.input_snapshot(root=root))

    def test_external_metadata_spec_is_an_input(self):
        with tempfile.TemporaryDirectory() as scratch:
            root = Path(scratch)
            recipe = root / "mk/owner.mk"
            recipe.parent.mkdir()
            recipe.write_text("step: @evidence/owner.spec\n")
            spec = root / "evidence/owner.spec"
            before = census.input_snapshot(root=root)
            environment = census.proof_environment_digest(root)
            spec.parent.mkdir()
            spec.write_text("synthetic metadata\n")
            with self.assertRaisesRegex(census.InputsChangedError, "evidence/owner.spec"):
                census.require_unchanged_inputs(before, census.input_snapshot(root=root))
            self.assertNotEqual(environment, census.proof_environment_digest(root))

    def test_cli_rejects_stale_build_before_using_cache(self):
        import function_preflight as fp
        with mock.patch.object(census, "require_fresh_build", side_effect=fp.StaleEvidenceError(
                "stale canonical linked ELF")), mock.patch.object(census, "census") as prove:
            with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as failure:
                census.main(["--no-build"])
            self.assertEqual(2, failure.exception.code)
            prove.assert_not_called()

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

    def test_exact_c_bytes_without_a_function_symbol_are_spans(self):
        # An export at +0 then a static at +0x20: tiled, nothing unbounded.
        self.assertEqual([], census.unbounded_spans([(0, 0x60)], [(0, 0x20), (0x20, 0x60)]))
        # The same module before the static had a symbol.
        self.assertEqual([(0x20, 0x60)], census.unbounded_spans([(0, 0x60)], [(0, 0x20)]))
        self.assertEqual([(0, 0x10), (0x30, 0x40)], census.unbounded_spans(
            [(0, 0x40), (0, 0x40)], [(0x10, 0x30), (0x80, 0x90)]))

    def test_static_functions_are_proved_and_tallied(self):
        entries = [census.Entry("overlay4Export", "overlay", "build/o.o"),
                   census.Entry("overlay4Helper", "overlay", "build/o.o", static=True)]
        runner = FakeRunner({"overlay4Export": "static", "overlay4Helper": "static"})
        outcomes = census.census(entries, cache_path=None, elf_digest="e", environment="v",
                                 runner=runner, object_digest=lambda obj: obj)
        self.assertEqual({"overlay4Export", "overlay4Helper"}, {call[2] for call in runner.calls})
        self.assertEqual([False, True], [row.static for row in outcomes])
        with contextlib.redirect_stdout(io.StringIO()) as out:
            census.render(outcomes, [], list_uncovered=False, require_coverage=True,
                          elapsed=0.0)
        self.assertIn("of which 1 proved / 1 in contract are static", out.getvalue())


if __name__ == "__main__":
    unittest.main()
