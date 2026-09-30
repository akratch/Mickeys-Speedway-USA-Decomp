#!/usr/bin/env python3
"""Synthetic regressions for the independently preprocessed context route."""
import json
from pathlib import Path
import tempfile
import unittest
from types import SimpleNamespace

import candidate_context as cc
import prepared_macro_context as pm

BASE = b"#define STEP(x) ((x)+1)\nextern int helper(int);\nint target(int x) { return STEP(x); }\n"
EXPANDED = b"extern int helper(int);\nint target(int x) { return ((x)+1); }\n"


class MacroContextTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name) / "attempt"

    def compare(self, winner=BASE, outputs=(EXPANDED, EXPANDED), callback=None):
        iterator = iter(outputs)
        def preprocess(source, output):
            output.write_bytes(next(iterator))
            return {"argv": ["stock-compiler", "-E", str(source)], "returncode": 0}
        return pm.compare(BASE, winner, "target", self.directory, callback or preprocess)

    def test_original_comparator_still_refuses_active_macros(self):
        self.assertEqual(cc.compare_context(BASE, BASE, "target")["status"], "unverifiable")

    def test_success_binds_original_and_expanded_inputs(self):
        result = self.compare(BASE.replace(b"STEP(x)", b"STEP(x)", 1))
        self.assertEqual(result["status"], "unchanged")
        self.assertNotEqual(result["baseline_sha256"], result["preprocessing"]["expanded_comparison"]["baseline_sha256"])
        self.assertEqual((self.directory / "baseline.c").read_bytes(), BASE)
        self.assertEqual((self.directory / "baseline.i").read_bytes(), EXPANDED)
        self.assertEqual(len(result["preprocessing"]["commands"]), 2)

    def test_changed_expanded_prototype_refuses(self):
        result = self.compare(outputs=(EXPANDED, EXPANDED.replace(b"helper(int)", b"helper(float)")))
        self.assertEqual(result["status"], "changed")

    def test_changed_macro_definition_refuses_even_equal_output(self):
        with self.assertRaisesRegex(ValueError, "definition context changed"):
            self.compare(BASE.replace(b"((x)+1)", b"((x)+2)"))

    def test_body_only_change_is_allowed_after_expansion(self):
        result = self.compare(BASE.replace(b"return STEP(x)", b"return STEP(x) + 1"),
                              (EXPANDED, EXPANDED.replace(b"((x)+1)", b"((x)+1)+1")))
        self.assertEqual(result["status"], "unchanged")

    def test_helper_body_change_is_context_change(self):
        source = EXPANDED + b"int other(void) { return 1; }"
        result = self.compare(outputs=(source, source.replace(b"return 1", b"return 2")))
        self.assertEqual(result["status"], "changed")

    def test_nonzero_preprocessor_exit_preserves_command(self):
        def fail(source, output):
            output.write_bytes(EXPANDED)
            return {"argv": ["stock", "-E"], "returncode": 1}
        with self.assertRaisesRegex(ValueError, "preprocessing failed"):
            self.compare(callback=fail)
        self.assertEqual(json.loads((self.directory / "baseline-command.json").read_text())["returncode"], 1)

    def test_boolean_exit_is_not_success(self):
        def fail(source, output):
            output.write_bytes(EXPANDED)
            return {"returncode": False}
        with self.assertRaises(ValueError):
            self.compare(callback=fail)

    def test_input_mutation_refuses(self):
        def mutate(source, output):
            source.write_bytes(b"changed")
            output.write_bytes(EXPANDED)
            return {"returncode": 0}
        with self.assertRaisesRegex(ValueError, "input changed"):
            self.compare(callback=mutate)

    def test_missing_output_refuses(self):
        with self.assertRaisesRegex(ValueError, "owned output"):
            self.compare(callback=lambda source, output: {"returncode": 0})

    def test_empty_output_refuses(self):
        with self.assertRaisesRegex(ValueError, "empty output"):
            self.compare(outputs=(b"", b""))

    def test_symlink_output_refuses(self):
        def link(source, output):
            output.symlink_to(source)
            return {"returncode": 0}
        with self.assertRaisesRegex(ValueError, "owned output"):
            self.compare(callback=link)

    def test_line_macro_and_nonleading_directives_refuse(self):
        for source in (BASE + b"int line = __LINE__;", BASE + b"#define LATE 2\n", b"#include <unknown.h>\n" + BASE):
            with self.subTest(source=source), self.assertRaises(cc.ContextError):
                cc.preprocessing_macro_context(source)

    def test_missing_function_in_expansion_is_unverifiable(self):
        result = self.compare(outputs=(b"int different(void) { return 1; }", EXPANDED))
        self.assertEqual(result["status"], "unverifiable")

    def test_stock_callback_uses_positional_arguments_and_keeps_diagnostics_separate(self):
        compiler = Path(self.temp.name) / "stock compiler"
        compiler.write_bytes(b"synthetic compiler identity")
        calls = []
        def capture(argv):
            calls.append(argv)
            Path(argv[4]).write_bytes(EXPANDED)
            return SimpleNamespace(returncode=0, stdout="diagnostic only\n")
        result = pm.compare_stock(BASE, BASE, "target", self.directory, compiler,
                                  ["-c", "-O2", "-I", "include with spaces"], capture)
        self.assertEqual(result["status"], "unchanged")
        self.assertEqual(len(calls), 2)
        self.assertEqual(calls[0][5], str(compiler))
        self.assertIn("include with spaces", calls[0])
        self.assertNotIn("-c", calls[0][5:])
        self.assertIn("-E", calls[0])
        self.assertIn("-DNON_MATCHING", calls[0])
        self.assertEqual((self.directory / "baseline.log").read_text(), "diagnostic only\n")

    def test_stock_recipe_output_overrides_refuse(self):
        compiler = Path(self.temp.name) / "cc"
        compiler.write_bytes(b"identity")
        for option in ("-o", "-oalternate", "-S", "-E"):
            with self.subTest(option=option), self.assertRaisesRegex(ValueError, "output mode"):
                pm.compare_stock(BASE, BASE, "target", self.directory, compiler, [option], None)

    def test_stock_compiler_mutation_refuses(self):
        compiler = Path(self.temp.name) / "cc"
        compiler.write_bytes(b"identity")
        def capture(argv):
            Path(argv[4]).write_bytes(EXPANDED)
            compiler.write_bytes(b"different")
            return SimpleNamespace(returncode=0, stdout="")
        with self.assertRaisesRegex(ValueError, "compiler changed"):
            pm.compare_stock(BASE, BASE, "target", self.directory, compiler, ["-c"], capture)

    def test_failed_replay_artifacts_are_retained(self):
        build = Path(self.temp.name) / "build"
        source = build / "macro-context" / "attempt"
        source.mkdir(parents=True)
        (source / "failure.log").write_text("failed command")
        destination = build / "run-context"
        pm.retain({"preprocessing_directory": str(source)}, destination, build)
        self.assertEqual((destination / "preprocessing/failure.log").read_text(), "failed command")

    def test_unowned_replay_path_refuses(self):
        source = Path(self.temp.name) / "foreign"
        source.mkdir()
        with self.assertRaisesRegex(ValueError, "outside its owned directory"):
            pm.retain({"preprocessing_directory": str(source)}, Path(self.temp.name) / "target", Path(self.temp.name) / "build")


if __name__ == "__main__":
    unittest.main()
