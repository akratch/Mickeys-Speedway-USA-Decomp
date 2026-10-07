#!/usr/bin/env python3
"""Synthetic unit coverage for tools/tu_flag_impact.py; no compiler or ROM."""

from __future__ import annotations

import tempfile
import time
import unittest
from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
from pathlib import Path
from unittest.mock import patch

import sys

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import flag_sweep as fs  # noqa: E402
import function_preflight as fp  # noqa: E402
import tu_flag_impact as impact  # noqa: E402


def resolution(source: Path, symbol: str, target: Path) -> fp.Resolution:
    return fp.Resolution(
        requested_symbol=symbol,
        target_symbol=symbol,
        candidate_symbol=symbol,
        source=source,
        translation_unit="synthetic/example",
        candidate_build_dir="build_non_matching",
        candidate_object=source.parent / "example.c.o",
        target_asm=target,
        selection="synthetic guarded C",
    )


class FlagDeltaTests(unittest.TestCase):
    def test_adds_r4300_hazard_flag_to_configured_recipe(self):
        trial = impact.apply_flag_delta(
            ("-32", "-O2", "-mips2"), ("-Wab,-r4300_mul",), ()
        )
        self.assertEqual(
            trial, ("-32", "-O2", "-mips2", "-Wab,-r4300_mul")
        )

    def test_conflicting_optimization_flags_fail_closed(self):
        with self.assertRaisesRegex(impact.ImpactError, "ambiguous trial flag"):
            impact.apply_flag_delta(("-O2", "-mips2", "-32"), ("-O3",), ())

    def test_absent_removal_fails_closed(self):
        with self.assertRaisesRegex(impact.ImpactError, "absent configured"):
            impact.apply_flag_delta(("-O2", "-mips2", "-32"), (), ("-O1",))

    def test_parser_rejects_non_codegen_control_flags(self):
        with self.assertRaisesRegex(impact.ImpactError, "only code-generation"):
            impact._parse_flag_values(("-o somewhere.o",), "trial")


class RecipeIntegrationTests(unittest.TestCase):
    def test_cache_and_compiler_receive_the_same_configured_recipe(self):
        recipe = impact.pb.BuildRecipe(
            ("-O2", "-mips2", "-32"), (), (), True,
            ("-c", "-DLOCAL_FEATURE=1", "-I", "local/include", "-O2", "-mips2", "-32"),
        )
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "source.c"
            source.write_text("void candidate(void) {}\n")

            def compile_source(tu, combo, outdir, defines, compiler_args):
                self.assertEqual(tu, source)
                self.assertEqual(defines, ("NON_MATCHING",))
                self.assertEqual(
                    compiler_args, ["-c", "-DLOCAL_FEATURE=1", "-I", "local/include"]
                )
                return fs.CompileResult(combo, True, outdir / "out.o", "", 0)

            with (
                patch.object(impact, "REPO", root),
                patch.object(fs, "compilation_cache_identity", autospec=True,
                             return_value=("synthetic-key", {"recipe": "synthetic"})) as identity,
                patch.object(fs, "load_cached_result", return_value=None),
                patch.object(fs, "compile_combo", autospec=True,
                             side_effect=compile_source) as compiler,
                patch.object(fs, "write_cached_result"),
            ):
                baseline, trial, _cache, compiled = impact.compile_variants(
                    source, recipe, (*recipe.flags, "-Wab,-r4300_mul"), rescore=False
                )
            self.assertEqual(identity.call_args.args[3], recipe)
            self.assertEqual(compiler.call_count, 2)
            self.assertEqual(compiled, 2)
            self.assertEqual(baseline.combo.extra, recipe.flags)
            self.assertEqual(trial.combo.extra, (*recipe.flags, "-Wab,-r4300_mul"))


class ConsumerEnumerationTests(unittest.TestCase):
    def test_every_guarded_function_in_the_tu_is_resolved(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "src" / "synthetic" / "example.c"
            fallback_dir = root / "asm" / "nonmatchings" / "synthetic" / "example"
            source.parent.mkdir(parents=True)
            fallback_dir.mkdir(parents=True)
            source.write_text(
                """
#ifdef NON_MATCHING
void first(void) { }
#else
#pragma GLOBAL_ASM("asm/nonmatchings/synthetic/example/first.s")
#endif
#ifdef NON_MATCHING
void second(void) { }
#else
#pragma GLOBAL_ASM("asm/nonmatchings/synthetic/example/second.s")
#endif
"""
            )
            targets = {}
            for symbol in ("first", "second"):
                target = fallback_dir / f"{symbol}.s"
                target.write_text(f"glabel {symbol}\nendlabel {symbol}\n")
                targets[symbol] = target

            def resolve_symbol(symbol: str) -> fp.Resolution:
                return resolution(source, symbol, targets[symbol])

            consumers = impact.collect_consumers(
                resolve_symbol("first"), root=root, resolver=resolve_symbol
            )

        self.assertEqual(
            [consumer.candidate_symbol for consumer in consumers], ["first", "second"]
        )

    def test_missing_fallback_is_not_reported_as_partial_success(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "src" / "example.c"
            target = root / "asm" / "nonmatchings" / "example" / "first.s"
            source.parent.mkdir(parents=True)
            target.parent.mkdir(parents=True)
            source.write_text(
                """
#ifdef NON_MATCHING
void first(void) { }
#else
#pragma GLOBAL_ASM("asm/nonmatchings/example/first.s")
#endif
"""
            )
            seed = resolution(source, "first", target)
            with self.assertRaisesRegex(impact.ImpactError, "missing fallback"):
                impact.collect_consumers(seed, root=root, resolver=lambda _symbol: seed)


class CompleteReportTests(unittest.TestCase):
    def test_outer_deadline_emits_no_partial_report(self):
        stdout = StringIO()
        stderr = StringIO()
        with (
            patch.object(impact, "run_analysis", side_effect=lambda _args: time.sleep(2)),
            redirect_stdout(stdout),
            redirect_stderr(stderr),
        ):
            status = impact.main(
                [
                    "seed",
                    "--add-flags=-Wab,-r4300_mul",
                    "--timeout-seconds",
                    "1",
                ]
            )

        self.assertEqual(status, 124)
        self.assertEqual(stdout.getvalue(), "")
        self.assertIn("no report emitted", stderr.getvalue())

    def test_consumer_scoring_reuses_retained_analysis_directories(self):
        consumer = impact.Consumer("candidate", "target", Path("target.s"))
        target_score_words = [1, 2]
        candidate_score_words = [1, 3]
        owner = fs.CanonicalOwnership("resident", "synthetic", 0x80000000, 8)
        with tempfile.TemporaryDirectory() as temporary:
            workdir = Path(temporary) / "analysis" / "candidate"
            with (
                patch.object(
                    fs,
                    "resolve_target",
                    return_value=(target_score_words, {}, "synthetic", owner),
                ),
                patch.object(
                    fs,
                    "get_candidate",
                    return_value=(candidate_score_words, {}, ".text"),
                ),
            ):
                first = impact.score_consumer(
                    consumer,
                    Path("source.c"),
                    Path("configured.o"),
                    Path("trial.o"),
                    workdir,
                    Path("linked.elf"),
                )
                second = impact.score_consumer(
                    consumer,
                    Path("source.c"),
                    Path("configured.o"),
                    Path("trial.o"),
                    workdir,
                    Path("linked.elf"),
                )

        self.assertEqual(first, second)

    def test_one_scoring_failure_aborts_the_complete_report(self):
        consumers = (
            impact.Consumer("first", "first", Path("first.s")),
            impact.Consumer("second", "second", Path("second.s")),
            impact.Consumer("third", "third", Path("third.s")),
        )
        score = fs.Score(False, 0, 3, 4)
        calls = []

        def scorer(consumer: impact.Consumer, index: int) -> impact.ImpactRow:
            calls.append(consumer.candidate_symbol)
            if index == 1:
                raise impact.ImpactError("synthetic extraction failure")
            return impact.ImpactRow(
                consumer.candidate_symbol,
                consumer.target_symbol,
                score,
                score,
                0,
                0,
                0,
                0,
                "unchanged",
            )

        with self.assertRaisesRegex(impact.ImpactError, "synthetic extraction"):
            impact.score_all(consumers, scorer)
        self.assertEqual(calls, ["first", "second"])

    def test_ranked_score_change_is_classified(self):
        baseline = fs.Score(False, 0, 12, 8)
        improved = fs.Score(False, 0, 4, 16)
        regressed = fs.Score(False, 4, 20, 0)
        self.assertEqual(impact._verdict(baseline, improved, 8), "improved")
        self.assertEqual(impact._verdict(baseline, regressed, 8), "regressed")
        self.assertEqual(impact._verdict(baseline, baseline, 2), "score-tie/changed")

    def test_rescore_requires_both_cached_objects(self):
        configured = fs.CompileResult(
            fs.Combo("configured", (), (), ("-O2",)), True, Path("configured.o"), "", 0
        )
        with (
            patch.object(impact, "_prepare_cache", return_value=(Path("cache"), "key")),
            patch.object(fs, "load_cached_result", side_effect=[configured, None]),
            patch.object(fs, "compile_combo", side_effect=AssertionError("compiled")),
        ):
            with self.assertRaisesRegex(impact.ImpactError, "cache is incomplete for trial"):
                impact.compile_variants(
                    Path("source.c"),
                    impact.pb.BuildRecipe(("-O2",), (), (), True, ("-c", "-O2")),
                    ("-O2", "-Wab,-r4300_mul"),
                    rescore=True,
                )


if __name__ == "__main__":
    unittest.main()
