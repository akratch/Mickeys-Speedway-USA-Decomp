#!/usr/bin/env python3
"""Focused source-level tests for wb proof provenance."""

from __future__ import annotations

import pathlib
import tempfile
import unittest

import proof_provenance as provenance


class SourceClassificationTests(unittest.TestCase):
    def classify(self, text: str, *, defines: tuple[str, ...] = ()) -> tuple[str, str]:
        return provenance.classify_source_selection(
            text,
            candidate_symbol="friendly",
            target_symbol="func_1234",
            defines=defines,
        )

    def test_ordinary_c_definition_is_admissible_class(self) -> None:
        text = """
        extern void friendly(int value);
        /* friendly( does not count. */
        void friendly(int value) { if (value) { friendly(value - 1); } }
        """
        self.assertEqual(self.classify(text)[0], provenance.ORDINARY_C)

    def test_nonmatching_define_selects_guarded_c(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #endif
        """
        self.assertEqual(
            self.classify(text, defines=("NON_MATCHING",))[0],
            provenance.NON_MATCHING_C,
        )

    def test_default_build_selects_nonmatching_fallback(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_friendly_candidate_pairs_with_auto_named_fallback(self) -> None:
        text = """
        #ifdef NON_MATCHING
        void friendly(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/different_auto_name.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_promoted_symbol_ignores_another_functions_mixed_tu_fallback(self) -> None:
        text = """
        void promoted(void) { }
        #ifdef NON_MATCHING
        void another(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/another_auto_name.s")
        #endif
        """
        kind, _reason = provenance.classify_source_selection(
            text,
            candidate_symbol="promoted",
            target_symbol="promoted",
            defines=(),
        )
        self.assertEqual(kind, provenance.ORDINARY_C)

    def test_unconditional_friendly_definition_ignores_another_fallback(self) -> None:
        # A promoted friendly-named body in a multi-function TU, beside another
        # function's guarded fallback: the fallback is not its alternative.
        text = """
        void friendly(void) { }
        #ifdef NON_MATCHING
        void another(void) { }
        #else
        #pragma GLOBAL_ASM("asm/nonmatchings/x/another_auto_name.s")
        #endif
        """
        self.assertEqual(self.classify(text)[0], provenance.ORDINARY_C)

    def test_bare_global_asm_is_fallback(self) -> None:
        text = '#pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")\n'
        self.assertEqual(self.classify(text)[0], provenance.GLOBAL_ASM)

    def test_missing_symbol_fails_closed(self) -> None:
        kind, reason = self.classify("void another(void) {}\n")
        self.assertEqual(kind, provenance.UNKNOWN)
        self.assertIn("no active C definition", reason)

    def test_if_not_defined_selects_fallback_when_enabled(self) -> None:
        text = """
        #if !defined(NON_MATCHING)
        #pragma GLOBAL_ASM("asm/nonmatchings/x/func_1234.s")
        #else
        void friendly(void) { }
        #endif
        """
        self.assertEqual(
            self.classify(text, defines=("NON_MATCHING",))[0],
            provenance.NON_MATCHING_C,
        )


class ManifestPrimitiveTests(unittest.TestCase):
    def test_hash_records_exact_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "candidate.o"
            path.write_bytes(b"compiled-c")
            self.assertEqual(
                provenance.sha256_file(path),
                "c5983ffc917d4649c1e0728fd58ff6be043dd548241563c16a8c0e7b43a082d7",
            )

    def test_source_facts_ignore_calls_and_comments(self) -> None:
        facts = provenance.source_facts(
            """
            // void friendly(void) {}
            void caller(void) { friendly(); }
            void friendly(void) { }
            """,
            "friendly",
        )
        self.assertEqual(len(facts.definitions), 1)

    def build_fixture_manifest(self, source_text: str, *, emit_symbol: bool) -> dict[str, object]:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "src" / "fixture.c"
            source.parent.mkdir(parents=True)
            source.write_text(source_text)
            candidate = root / "build" / "src" / "fixture.c.o"
            candidate.parent.mkdir(parents=True)
            candidate.write_bytes(b"candidate-object")
            target = root / "target.o"
            target.write_bytes(b"target-object")
            fake_objdump = root / "objdump"
            symbol_line = "00000000 g F .text 00000004 friendly\\n" if emit_symbol else ""
            fake_objdump.write_text(
                "#!/bin/sh\n"
                "test \"$1\" = -t || exit 2\n"
                f"printf '{symbol_line}'\n"
            )
            fake_objdump.chmod(0o755)
            return provenance.build_manifest(
                root=root,
                mode="asm",
                source=source,
                symbol="friendly",
                candidate_symbol="friendly",
                candidate_build_dir=root / "build",
                candidate_object=candidate,
                target_object=target,
                candidate_artifact=candidate,
                target_artifact=target,
                objdump=fake_objdump,
            )

    def test_manifest_allows_tied_ordinary_c(self) -> None:
        manifest = self.build_fixture_manifest(
            "void friendly(void) {}\n", emit_symbol=True
        )
        self.assertTrue(manifest["exact_claim_allowed"])
        self.assertEqual(manifest["verdict"], "c_evidence")
        self.assertIsNotNone(manifest["candidate_object"]["sha256"])
        self.assertIsNotNone(manifest["target_object"]["sha256"])

    def test_manifest_fails_closed_when_object_symbol_is_missing(self) -> None:
        manifest = self.build_fixture_manifest(
            "void friendly(void) {}\n", emit_symbol=False
        )
        self.assertFalse(manifest["exact_claim_allowed"])
        self.assertEqual(manifest["exact_guard"], "assert_exact_false")
        self.assertIn("does not define", " ".join(manifest["reasons"]))

    def test_manifest_fails_closed_for_global_asm(self) -> None:
        manifest = self.build_fixture_manifest(
            '#pragma GLOBAL_ASM("asm/nonmatchings/x/friendly.s")\n',
            emit_symbol=True,
        )
        self.assertFalse(manifest["exact_claim_allowed"])
        self.assertEqual(
            manifest["selection"]["classification"], provenance.GLOBAL_ASM
        )


if __name__ == "__main__":
    unittest.main()
