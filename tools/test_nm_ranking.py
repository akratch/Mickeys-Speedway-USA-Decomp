#!/usr/bin/env python3
"""Focused tests for fail-closed ranking persistence and documentation."""

from __future__ import annotations

import json
import concurrent.futures
import shutil
from pathlib import Path
import sys
import tempfile
import os
import unittest
import unittest.mock
from unittest import mock

import nm_ranking as ranking


def function_row(
    file_name: str,
    symbol: str,
    pct: float | None = None,
    *,
    category: str = "other",
    differing_words: int = 2,
    relocation_masked_differing_words: int | None = None,
) -> dict[str, object]:
    row: dict[str, object] = {
        "file": file_name,
        "name": symbol,
        "overlay": None,
        "tu": "main",
        "size_bytes": 16,
        "objdiff_match_pct": pct,
        "differing_words": differing_words,
        "first_mismatch_offset": 4,
        "size_delta": 0,
        "category": category,
    }
    if relocation_masked_differing_words is not None:
        row["relocation_masked_differing_words"] = (
            relocation_masked_differing_words
        )
        row["relocation_masked_first_mismatch_offset"] = (
            4 if relocation_masked_differing_words else None
        )
    return row


def ranking_document(
    functions: list[dict[str, object]] | None = None,
    unresolved: list[list[object]] | None = None,
) -> dict[str, object]:
    function_rows = functions or []
    unresolved_rows = unresolved or []
    return {
        "queue_size": len(function_rows) + len(unresolved_rows),
        "resolved": len(function_rows),
        "unresolved": len(unresolved_rows),
        "objdiff_report_used": any(
            row["objdiff_match_pct"] is not None for row in function_rows
        ),
        "objdiff_match_pct_coverage": sum(
            row["objdiff_match_pct"] is not None for row in function_rows
        ),
        "functions": function_rows,
        "unresolved_functions": unresolved_rows,
    }


def queue_item(file_name: str, symbol: str) -> object:
    return ranking.pb.QueueItem(
        func=symbol,
        c_file=ranking.ROOT / file_name,
    )


class ConfiguredContextTests(unittest.TestCase):
    def test_assembler_include_paths_do_not_override_compiler_search_order(self):
        command = ["python", "tools/asm-processor/build.py", "cc", "--",
                   "as", "-I", "assembler-only", "--", "-I", ".", "-Iinclude"]
        self.assertEqual(ranking.include_directories(command),
                         [ranking.ROOT, ranking.ROOT / "include"])

    def test_literal_command_input_changes_invalidate_its_receipt(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            prelude = root / "prelude.inc"
            prelude.write_text("initial assembler prelude\n")
            command = ["cc", "prelude.inc", "-o", "unused.o", "source.c"]
            with mock.patch.object(ranking, "ROOT", root):
                initial = ranking.command_file_inputs(command, "source.c", {})
                prelude.write_text("changed assembler prelude\n")
                self.assertNotEqual(initial, ranking.command_file_inputs(command, "source.c", {}))

    def test_concurrent_symbol_extraction_uses_independent_temporaries(self):
        with tempfile.TemporaryDirectory() as directory:
            obj = Path(directory) / "shared.o"
            destinations = []
            def extract(command, **kwargs):
                destination = Path(command[-1])
                destinations.append(destination)
                destination.write_bytes(b"synthetic text")
            with mock.patch.object(ranking.subprocess, "run", side_effect=extract), \
                 concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
                results = list(pool.map(lambda _: ranking.text_bytes(obj, 0, 9), range(2)))
            self.assertEqual(results, [b"synthetic", b"synthetic"])
            self.assertEqual(len(set(destinations)), 2)
            self.assertTrue(all(not path.exists() for path in destinations))

    def test_configured_compile_preserves_defines_include_order_and_source_location(self):
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host preprocessor unavailable")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, value in (("first", 7), ("alternate", 11)):
                (root / name).mkdir()
                (root / name / "choice.h").write_text(f"#define CHOICE {value}\n")
            source = root / "candidate.c"
            source.write_text('#include <choice.h>\n#ifdef NON_MATCHING\n'
                              'int target(void) { return PROBE + CHOICE + __LINE__; }\n'
                              '#else\n#pragma GLOBAL_ASM("asm/target.s")\n#endif\n')
            before = source.stat().st_mtime_ns
            command = [compiler, "-E", "-P", "-x", "c", "-DNON_MATCHING",
                       "-DPROBE=2", "-I", str(root / "first"),
                       "-I", str(root / "alternate"), str(source), "-o", "unused.o"]
            with mock.patch.object(ranking, "ROOT", root), \
                 mock.patch.object(ranking, "WORK_DIR", root / "build"):
                output, error = ranking.compile_configured_tu("candidate.c", command)
                self.assertIsNone(error)
                self.assertIn("return 2 + 7 + 3", output.read_text())
                command[6] = "-DPROBE=5"
                command[8], command[10] = command[10], command[8]
                output, error = ranking.compile_configured_tu("candidate.c", command)
                self.assertIsNone(error)
                self.assertIn("return 5 + 11 + 3", output.read_text())
            self.assertEqual(source.stat().st_mtime_ns, before)
            self.assertFalse((root / "unused.o").exists())

    def test_line_changes_invalidate_source_and_header_evidence(self):
        source = ('#ifdef NON_MATCHING\nint target(void) { return __LINE__; }\n'
                  '#else\n#pragma GLOBAL_ASM("asm/target.s")\n#endif\n')
        self.assertNotEqual(ranking.source_context_digest(source, "target"),
                            ranking.source_context_digest("\n" + source, "target"))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "candidate.c").write_text('#include "value.h"\n')
            header = root / "value.h"
            header.write_text("enum { value = __LINE__ };\n")
            with mock.patch.object(ranking, "ROOT", root):
                initial = ranking.header_dependencies(root / "candidate.c", [root])
                header.write_text("\nenum { value = __LINE__ };\n")
                self.assertNotEqual(initial, ranking.header_dependencies(root / "candidate.c", [root]))

    def test_failed_compile_does_not_reuse_a_previous_object(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "configured/candidate.c.o"
            output.parent.mkdir()
            output.write_bytes(b"prior object")
            with mock.patch.object(ranking, "ROOT", root), \
                 mock.patch.object(ranking, "WORK_DIR", root), \
                 mock.patch.object(ranking.subprocess, "run", return_value=mock.Mock(
                     returncode=1, stdout="", stderr="compiler failure")):
                measured, error = ranking.compile_configured_tu("candidate.c", ["cc", "-o", "unused.o"])
            self.assertIsNone(measured)
            self.assertIn("configured TU compile failed", error)
            self.assertFalse(output.exists())

    def test_shared_tu_compiles_once_without_import_or_historical_context(self):
        items = [queue_item("src/main/shared.c", symbol) for symbol in ("first", "second")]
        command = ["cc", "-DVALUE=7", "-Ialt", "-o", "unused.o", items[0].rel_c_file]
        candidate = Path("configured.o")
        with mock.patch.object(ranking, "configured_compile_commands", return_value={items[0].rel_c_file: command}), \
             mock.patch.object(ranking, "compile_configured_tu", return_value=(candidate, None)) as compile_tu, \
             mock.patch.object(ranking, "process_item", return_value=(None, None)) as measure, \
             mock.patch.object(ranking.pb, "run_import", side_effect=AssertionError("unexpected import")):
            ranking.process_items(items, 2)
            compile_tu.assert_called_once_with(items[0].rel_c_file, command)
            self.assertEqual(measure.call_count, 2)
            self.assertTrue(all(call.args[1] == candidate for call in measure.call_args_list))

    def test_batched_recipe_expansion_preserves_per_file_flags(self):
        items = [queue_item(f"src/main/{name}.c", name) for name in ("a", "b")]
        lines = []
        for item, flag in zip(items, ("-O2", "-O1")):
            lines.append(f"python3 tools/asm-processor/build.py tools/ido/cc -- as -- {flag} "
                         f"-o build_non_matching/{item.rel_c_file}.o {item.rel_c_file}")
        response = mock.Mock(returncode=0, stdout="\n".join(lines), stderr="")
        with mock.patch.object(ranking.subprocess, "run", return_value=response) as run:
            commands = ranking.configured_compile_commands(items + items)
            self.assertEqual(run.call_count, 1)
            self.assertEqual(run.call_args.args[0].count("-W"), 2)
            self.assertIn("-O2", commands[items[0].rel_c_file])
            self.assertIn("-O1", commands[items[1].rel_c_file])
            self.assertIn("build/nm_ranking/context.o", commands[items[0].rel_c_file])
            response.stdout = lines[0]
            with self.assertRaisesRegex(ranking.RankingDocumentError, "missing configured recipes"):
                ranking.configured_compile_commands(items)

    def test_transitive_headers_resolution_cycles_and_unrelated_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nested").mkdir()
            source = root / "candidate.c"
            source.write_text('#include "direct.h"\n')
            (root / "direct.h").write_text('#include "nested/indirect.h"\n')
            indirect = root / "nested/indirect.h"
            indirect.write_text('#include "../direct.h"\n#define WIDTH 4\n')
            with mock.patch.object(ranking, "ROOT", root):
                before = ranking.header_dependencies(source, [root])
                self.assertEqual(set(before), {"direct.h", "nested/indirect.h"})
                (root / "unrelated.h").write_text("#define OTHER 7\n")
                self.assertEqual(before, ranking.header_dependencies(source, [root]))
                indirect.write_text('#include "../direct.h"\n#define WIDTH 8\n')
                self.assertNotEqual(before, ranking.header_dependencies(source, [root]))

    def test_new_include_and_macro_include_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "candidate.c"
            source.write_text('#if 0\n#include "optional.h"\n#endif\n')
            with mock.patch.object(ranking, "ROOT", root):
                before = ranking.header_dependencies(source, [root])
                (root / "optional.h").write_text("#define OPTIONAL 1\n")
                self.assertNotEqual(before, ranking.header_dependencies(source, [root]))
                source.write_text('#include HEADER_NAME\n')
                with self.assertRaisesRegex(ranking.RankingDocumentError, "macro include"):
                    ranking.header_dependencies(source, [root])

    def test_recipe_and_tool_changes_invalidate_only_current_receipts(self):
        item = queue_item("src/main/example.c", "example")
        recipe = ["tools/ido/cc", "-O2", "-I", "include", "-o", "unused.o"]
        with mock.patch.object(ranking, "configured_tool_digest", return_value="tool-a") as tool, \
             mock.patch.object(ranking, "configured_compile_commands", return_value={item.rel_c_file: recipe}) as expand, \
             mock.patch.object(ranking, "assembly_dependencies", return_value={}), \
             mock.patch.object(ranking, "header_dependencies", return_value={"header.h": "h"}):
            original = ranking.configured_build_contexts([item, item])
            self.assertEqual(expand.call_count, 1)
            self.assertEqual(original, ranking.configured_build_contexts([item]))
            recipe[1] = "-O1"
            self.assertNotEqual(original, ranking.configured_build_contexts([item]))
            recipe[1] = "-O2"
            tool.return_value = "tool-b"
            self.assertNotEqual(original, ranking.configured_build_contexts([item]))

    def test_sibling_assembly_and_nested_includes_are_fingerprinted(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "candidate.c"
            source.write_text('#pragma GLOBAL_ASM("sibling.s")\n')
            (root / "sibling.s").write_text('.include "definitions.inc"\n')
            definitions = root / "definitions.inc"
            definitions.write_text("# initial definitions\n")
            with mock.patch.object(ranking, "ROOT", root):
                initial = ranking.assembly_dependencies(source, [])
                self.assertEqual(set(initial), {"sibling.s", "definitions.inc"})
                definitions.write_text("# changed definitions\n")
                self.assertNotEqual(initial, ranking.assembly_dependencies(source, []))


class StandaloneListingTests(unittest.TestCase):
    def test_asm_processor_alignment_directive_is_removed_for_gas(self):
        listing = (".section .late_rodata\n.late_rodata_alignment 4\n"
                   "dlabel jtbl_EXAMPLE\n    .word .LEXAMPLE\n"
                   ".section .text\nglabel exampleSwitch\n    nop\n")
        out = ranking.standalone_listing(listing)
        self.assertNotIn(".late_rodata_alignment", out)
        self.assertIn(".section .late_rodata\n", out)
        self.assertIn("glabel exampleSwitch\n    nop\n", out)
        self.assertEqual(out.count("\n"), listing.count("\n"),
                         "line numbers are preserved")

    def test_a_listing_without_migrated_rodata_is_unchanged(self):
        listing = "glabel examplePlain\n    nop\nendlabel examplePlain\n"
        self.assertEqual(ranking.standalone_listing(listing), listing)


class CoverageTests(unittest.TestCase):
    def test_full_identity_audit_includes_missing_retired_and_unresolved(self):
        good = ("src/main/good.c", "good")
        stale = ("src/main/stale.c", "stale")
        retired = ("src/main/retired.c", "retired")
        new = ("src/main/new.c", "new")
        pending = ("src/main/pending.c", "pending")
        document = ranking_document(
            [function_row(*key) for key in (good, stale, retired)],
            [[list(pending), "compile failed"]],
        )
        report = ranking.ranking_coverage(document, {good, stale, new, pending}, {good})
        self.assertFalse(report["complete"])
        self.assertEqual(report["missing"], [list(new)])
        self.assertEqual(report["retired"], [list(retired)])
        self.assertEqual(report["stale"], [list(stale)])
        self.assertEqual(report["unresolved"], [list(pending)])
        self.assertEqual(report["fresh"], 1)

    def test_matching_identity_cannot_hide_changed_body_or_declaration(self):
        source = ('extern int value;\n#ifdef NON_MATCHING\n'
                  'int target(void) { return value; }\n#else\n'
                  '#pragma GLOBAL_ASM("asm/nonmatchings/target.s")\n#endif\n')
        key = ("src/main/target.c", "target")
        measured = ranking.source_context_digest(source, "target")
        self.assertIsNotNone(measured)
        row = function_row(*key)
        row[ranking.SOURCE_CONTEXT_FIELD] = measured
        document = ranking_document([row])
        for changed in (source.replace("return value", "return value + 1"),
                        source.replace("extern int", "extern short")):
            with self.subTest(source=changed):
                current = ranking.source_context_digest(changed, "target")
                self.assertFalse(ranking.source_coverage(document, {key: current})["complete"])
        commented = ranking.source_context_digest(source + "/* review note */\n", "target")
        self.assertFalse(ranking.source_coverage(document, {key: commented})["complete"])

    def test_unproven_legacy_row_requires_evidence(self):
        key = ("src/main/legacy.c", "legacy")
        digest = ranking.normalize_source_context_digest("a" * 64)
        document = ranking_document([function_row(*key)])
        self.assertFalse(ranking.source_coverage(document, {key: digest})["complete"])
        self.assertTrue(ranking.source_coverage(document, {key: digest}, {key: digest})["complete"])


class RetiredGateTests(unittest.TestCase):
    """--check-retired gates check-docs, so its blast radius has to be exact.

    A retired row names a function that has been matched and left the queue:
    the row is not out of date, it is wrong, and planning that reads it will
    re-derive a match that already landed. A stale row is the ordinary
    consequence of editing a candidate-bearing TU. Gating on the first is
    safe; gating on the second would fail nearly every commit.
    """

    def test_a_matched_function_still_listed_is_reported_retired(self):
        live = ("src/main/live.c", "live")
        gone = ("src/main/gone.c", "gone")
        document = ranking_document([function_row(*key) for key in (live, gone)])
        report = ranking.ranking_coverage(document, {live}, {live})
        self.assertEqual(report["retired"], [list(gone)])

    def test_an_edited_candidate_is_stale_but_not_retired(self):
        """The property that makes this gate safe to wire into check-docs."""
        key = ("src/main/edited.c", "edited")
        document = ranking_document([function_row(*key)])
        report = ranking.ranking_coverage(document, {key}, set())
        self.assertEqual(report["stale"], [list(key)])
        self.assertEqual(report["retired"], [], "an edit must not trip the retired gate")
        self.assertFalse(report["complete"], "but it is still not fresh")

    def test_a_newly_queued_function_is_missing_not_retired(self):
        known = ("src/main/known.c", "known")
        fresh = ("src/main/fresh.c", "fresh")
        document = ranking_document([function_row(*known)])
        report = ranking.ranking_coverage(document, {known, fresh}, {known})
        self.assertEqual(report["missing"], [list(fresh)])
        self.assertEqual(report["retired"], [])

    def test_a_clean_ranking_reports_nothing_retired(self):
        key = ("src/main/ok.c", "ok")
        document = ranking_document([function_row(*key)])
        self.assertEqual(ranking.ranking_coverage(document, {key}, {key})["retired"], [])


class PruneStaleTests(unittest.TestCase):
    def test_prunes_only_nonlive_exact_identities_and_normalizes_counts(self) -> None:
        keep = ("src/main/keep.c", "keep")
        pending = ("src/main/pending.c", "pending")
        document = ranking_document(
            [
                function_row(*keep, pct=91.5),
                function_row("src/main/matched.c", "matched", pct=100.0),
            ],
            [
                [[*pending], "compile failed"],
                [["src/main/resolved.c", "resolved"], "old failure"],
            ],
        )
        # Pruning repairs derived counts but still validates every row field.
        document["queue_size"] = 99
        document["resolved"] = 98
        document["unresolved"] = 1
        document["objdiff_match_pct_coverage"] = 98

        pruned, removed, unranked = ranking.prune_stale_document(
            document,
            {keep, pending, ("src/main/new.c", "new")},
        )

        self.assertEqual([row["name"] for row in pruned["functions"]], ["keep"])
        self.assertEqual(
            pruned["unresolved_functions"], [[[*pending], "compile failed"]]
        )
        self.assertEqual(pruned["queue_size"], 2)
        self.assertEqual(pruned["resolved"], 1)
        self.assertEqual(pruned["unresolved"], 1)
        self.assertEqual(pruned["objdiff_match_pct_coverage"], 1)
        self.assertEqual(
            removed,
            [
                ("src/main/matched.c", "matched"),
                ("src/main/resolved.c", "resolved"),
            ],
        )
        self.assertEqual(unranked, [("src/main/new.c", "new")])
        self.assertEqual(document["queue_size"], 99)

    def test_unidentified_unresolved_row_fails_closed(self) -> None:
        document = ranking_document()
        document["unresolved_functions"] = ["func (src/main/file.c): failed"]
        with self.assertRaisesRegex(
            ranking.RankingDocumentError,
            r"needs \[\[file, name\], diagnostic\] identity",
        ):
            ranking.prune_stale_document(document, set())

    def test_duplicate_identity_fails_closed(self) -> None:
        key = ("src/main/dup.c", "dup")
        document = ranking_document(
            [function_row(*key)], [[[*key], "also unresolved"]]
        )
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "duplicate function identities"
        ):
            ranking.prune_stale_document(document, {key})

    def test_atomic_writer_replaces_complete_json(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.json"
            path.write_text('{"old": true}\n', encoding="utf-8")
            path.chmod(0o640)
            document = ranking_document()
            ranking.write_json_atomic(path, document)
            self.assertEqual(json.loads(path.read_text()), document)
            self.assertEqual(path.stat().st_mode & 0o777, 0o640)
            self.assertEqual(list(path.parent.glob(".ranking.json.*.tmp")), [])

    def test_transactional_writer_rolls_back_first_replacement(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            first = Path(directory) / "first.txt"
            second = Path(directory) / "second.txt"
            first.write_text("old first\n", encoding="utf-8")
            second.write_text("old second\n", encoding="utf-8")
            original_replace = Path.replace
            calls = 0

            def fail_second_replace(path: Path, target: Path) -> Path:
                nonlocal calls
                calls += 1
                if calls == 2:
                    raise OSError("injected replacement failure")
                return original_replace(path, target)

            with mock.patch.object(Path, "replace", fail_second_replace):
                with self.assertRaisesRegex(OSError, "injected"):
                    ranking.write_texts_transactionally([
                        (first, "new first\n"),
                        (second, "new second\n"),
                    ])
            self.assertEqual(first.read_text(encoding="utf-8"), "old first\n")
            self.assertEqual(second.read_text(encoding="utf-8"), "old second\n")
            self.assertEqual(list(Path(directory).glob(".*.tmp")), [])


class ValidationTests(unittest.TestCase):
    def test_same_symbol_in_different_files_is_a_distinct_identity(self) -> None:
        document = ranking_document([
            function_row("src/main/one.c", "shared"),
            function_row("src/main/two.c", "shared"),
        ])
        ranking.validate_ranking_document(document)

    def test_duplicate_exact_identity_fails_closed(self) -> None:
        row = function_row("src/main/dup.c", "dup")
        document = ranking_document([row, dict(row)])
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "duplicate function identities"
        ):
            ranking.validate_ranking_document(document)

    def test_malformed_measurement_fails_closed(self) -> None:
        row = function_row("src/main/file.c", "symbol")
        row["differing_words"] = "two"
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "differing_words must be an integer"
        ):
            ranking.validate_ranking_document(ranking_document([row]))

    def test_location_inconsistent_with_exact_file_identity_fails_closed(self) -> None:
        row = function_row("src/overlays/o007/file.c", "symbol")
        row["overlay"] = 8
        row["tu"] = "overlays/o008"
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "overlay does not match its source path"
        ):
            ranking.validate_ranking_document(ranking_document([row]))

    def test_inconsistent_derived_count_fails_closed(self) -> None:
        document = ranking_document([
            function_row("src/main/file.c", "symbol")
        ])
        document["resolved"] = 0
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "resolved is 0, expected 1"
        ):
            ranking.validate_ranking_document(document)

    def test_schema_three_coverages_are_derived_and_validated(self) -> None:
        row = function_row("src/main/file.c", "symbol")
        row[ranking.SOURCE_CONTEXT_FIELD] = ranking.group_source_context("A" * 43)
        document = ranking.make_ranking_document(
            [row], [], objdiff_report_used=False
        )
        self.assertEqual(document["schema_version"], 3)
        self.assertEqual(document["source_context_coverage"], 1)
        self.assertEqual(document["relocation_masked_coverage"], 0)
        document["source_context_coverage"] = 0
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "source_context_coverage"
        ):
            ranking.validate_ranking_document(document)

    def test_schema_two_rows_migrate_without_inventing_masked_evidence(self) -> None:
        legacy = ranking_document([
            function_row("src/main/file.c", "symbol")
        ])
        legacy["schema_version"] = 2
        legacy["source_context_version"] = ranking.SOURCE_CONTEXT_VERSION
        legacy["source_context_coverage"] = 0
        ranking.validate_ranking_document(legacy)

        migrated = ranking.make_ranking_document(
            list(legacy["functions"]), [], objdiff_report_used=False
        )

        self.assertEqual(migrated["schema_version"], 3)
        self.assertEqual(migrated["relocation_masked_coverage"], 0)
        row = migrated["functions"][0]
        self.assertIsNone(row["relocation_masked_differing_words"])
        self.assertIsNone(row["relocation_masked_first_mismatch_offset"])

    def test_masked_coverage_is_derived_and_validated(self) -> None:
        row = function_row(
            "src/main/file.c",
            "symbol",
            relocation_masked_differing_words=1,
        )
        document = ranking.make_ranking_document(
            [row], [], objdiff_report_used=False
        )
        self.assertEqual(document["relocation_masked_coverage"], 1)
        document["relocation_masked_coverage"] = 0

        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "relocation_masked_coverage"
        ):
            ranking.validate_ranking_document(document)

    def test_masked_count_cannot_exceed_raw_count(self) -> None:
        row = function_row(
            "src/main/file.c",
            "symbol",
            differing_words=1,
            relocation_masked_differing_words=2,
        )
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "exceeds differing_words"
        ):
            ranking.make_ranking_document([row], [], objdiff_report_used=False)

    def test_masked_first_offset_requires_matching_count(self) -> None:
        row = function_row(
            "src/main/file.c",
            "symbol",
            relocation_masked_differing_words=0,
        )
        row["relocation_masked_first_mismatch_offset"] = 4
        with self.assertRaisesRegex(
            ranking.RankingDocumentError,
            "disagrees with relocation_masked_differing_words",
        ):
            ranking.make_ranking_document([row], [], objdiff_report_used=False)

    def test_schema_three_sorts_by_masked_then_raw_mismatches(self) -> None:
        masked_closer = function_row(
            "src/main/masked.c",
            "masked_closer",
            differing_words=8,
            relocation_masked_differing_words=1,
        )
        raw_closer = function_row(
            "src/main/raw.c",
            "raw_closer",
            differing_words=2,
            relocation_masked_differing_words=2,
        )

        document = ranking.make_ranking_document(
            [raw_closer, masked_closer], [], objdiff_report_used=False
        )

        self.assertEqual(
            [row["name"] for row in document["functions"]],
            ["masked_closer", "raw_closer"],
        )

    def test_malformed_context_digest_fails_closed(self) -> None:
        row = function_row("src/main/file.c", "symbol")
        row[ranking.SOURCE_CONTEXT_FIELD] = "not-a-digest"
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "SHA-256 encoding"
        ):
            ranking.validate_ranking_document(ranking_document([row]))


class SourceContextTests(unittest.TestCase):
    def test_context_digest_is_cleanroom_safe_base64url(self) -> None:
        text = """#ifdef NON_MATCHING
void a(void) {}
#else
#pragma GLOBAL_ASM(\"asm/a.s\")
#endif
"""
        digest = ranking.source_context_digest(text, "a")
        self.assertIsNotNone(digest)
        self.assertRegex(
            digest or "", r"^(?:[A-Za-z0-9_-]{4}\.){10}[A-Za-z0-9_-]{3}$"
        )

    def test_legacy_hex_context_normalizes_without_changing_evidence(self) -> None:
        legacy = "00" * 32
        self.assertEqual(
            ranking.normalize_source_context_digest(legacy),
            ranking.group_source_context("A" * 43),
        )

    def test_preserves_comments_and_other_candidates_and_shared_context(self) -> None:
        original = """extern int shared;
#ifdef NON_MATCHING
void a(void) { shared++; }
#else
#pragma GLOBAL_ASM("asm/a.s")
#endif
#ifdef NON_MATCHING
void b(void) { shared++; }
#else
#pragma GLOBAL_ASM("asm/b.s")
#endif
"""
        changed_other = original.replace(
            "void b(void) { shared++; }", "void b(void) { shared += 2; }"
        ).replace("extern int", "/* note */\nextern int")
        changed_shared = original.replace("extern int shared", "extern short shared")
        baseline = ranking.source_context_digest(original, "a")
        self.assertNotEqual(
            baseline, ranking.source_context_digest(changed_other, "a")
        )
        self.assertNotEqual(
            baseline, ranking.source_context_digest(changed_shared, "a")
        )

    def test_context_blame_supersedes_legacy_measurement_blame(self) -> None:
        lines = [
            ("1" * 40, '      "name": "func",'),
            ("1" * 40, '      "file": "src/main/file.c",'),
            ("2" * 40, '      "differing_words": 3,'),
            (
                "3" * 40,
                f'      "{ranking.SOURCE_CONTEXT_FIELD}": "'
                + ranking.group_source_context("A" * 43)
                + '",',
            ),
        ]
        with mock.patch.object(
            ranking, "blamed_source_lines", return_value=lines
        ):
            commits = ranking.ranking_evidence_commits("HEAD", "ranking.json")
        self.assertEqual(
            commits[("src/main/file.c", "func")], "3" * 40
        )


class RetainedDisplayTests(unittest.TestCase):
    def test_recovers_validated_rows_without_source_or_compile_state(self) -> None:
        document = ranking_document([
            function_row(
                "src/main/file.c",
                "func",
                category="register-only",
                differing_words=2,
                pct=98.5,
            )
        ])
        results = ranking.retained_results(document)
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].name, "func")
        self.assertEqual(results[0].differing_words, 2)
        self.assertIsNone(results[0].relocation_masked_differing_words)
        self.assertEqual(results[0].objdiff_match_pct, 98.5)

    def test_rejects_an_invalid_snapshot(self) -> None:
        document = ranking_document()
        document["queue_size"] = 99
        with self.assertRaises(ranking.RankingDocumentError):
            ranking.retained_results(document)

    def test_presentation_flag_alone_cannot_start_a_full_compile(self) -> None:
        with mock.patch.object(sys, "argv", ["nm_ranking.py", "--top", "1"]):
            with mock.patch.object(
                ranking,
                "missing_permuter_inputs",
                side_effect=AssertionError("compile path reached"),
            ):
                self.assertEqual(ranking.main(), 2)


class IncrementalRefreshTests(unittest.TestCase):
    def setUp(self) -> None:
        self.keep = ("src/main/keep.c", "keep")
        self.legacy = ("src/main/legacy.c", "legacy")
        self.stale = ("src/main/stale.c", "stale")
        self.pending = ("src/main/pending.c", "pending")
        self.new = ("src/main/new.c", "new")
        self.removed = ("src/main/removed.c", "removed")
        keep_row = function_row(
            *self.keep,
            differing_words=1,
            relocation_masked_differing_words=1,
        )
        keep_row[ranking.SOURCE_CONTEXT_FIELD] = ranking.group_source_context("A" * 43)
        legacy_row = function_row(
            *self.legacy,
            differing_words=2,
            relocation_masked_differing_words=1,
        )
        self.document = ranking.make_ranking_document(
            [
                keep_row,
                legacy_row,
                function_row(*self.stale, differing_words=3),
                function_row(*self.removed, differing_words=4),
            ],
            [[[*self.pending], "old compile failure"]],
            objdiff_report_used=False,
        )
        self.items = [
            queue_item(*key)
            for key in (self.keep, self.legacy, self.stale, self.pending, self.new)
        ]
        self.contexts = {
            self.keep: ranking.group_source_context("A" * 43),
            self.legacy: ranking.group_source_context("B" * 43),
            self.stale: ranking.group_source_context("C" * 43),
            self.pending: ranking.group_source_context("D" * 43),
            self.new: ranking.group_source_context("E" * 43),
        }

    def test_plan_migrates_legacy_proof_and_bounds_only_compile_work(self) -> None:
        plan = ranking.plan_incremental_refresh(
            self.document,
            self.items,
            self.contexts,
            {
                self.legacy: ranking.group_source_context("B" * 43),
                self.stale: ranking.group_source_context("F" * 43),
            },
            limit=2,
        )
        self.assertEqual(set(plan.fresh_rows), {self.keep, self.legacy})
        self.assertEqual(
            [(item.rel_c_file, item.func) for item in plan.selected],
            [self.stale, self.pending],
        )
        self.assertEqual(plan.removed, [self.removed])
        self.assertEqual(set(plan.deferred_unresolved), {self.new})
        self.assertEqual(plan.stale_count, 2)
        self.assertEqual(plan.new_count, 1)

    def test_merge_preserves_fresh_and_deferred_rows_and_exact_live_queue(self) -> None:
        plan = ranking.plan_incremental_refresh(
            self.document,
            self.items,
            self.contexts,
            {self.legacy: ranking.group_source_context("B" * 43)},
            limit=1,
        )
        selected = plan.selected[0]
        result = ranking.FuncResult(
            name=selected.func,
            file=selected.rel_c_file,
            overlay=None,
            tu="main",
            size_bytes=16,
            differing_words=1,
            first_mismatch_offset=4,
            relocation_masked_differing_words=1,
            relocation_masked_first_mismatch_offset=4,
            size_delta=0,
            category="register-only",
        )
        merged = ranking.merge_incremental_results(
            self.document,
            plan,
            [result],
            objdiff_report_used=False,
        )
        keys = {
            (row["file"], row["name"]) for row in merged["functions"]
        } | {
            tuple(row[0]) for row in merged["unresolved_functions"]
        }
        self.assertEqual(keys, set(self.contexts))
        self.assertEqual(merged["source_context_coverage"], 3)
        stale_row = next(
            row for row in merged["functions"] if row["name"] == "stale"
        )
        self.assertEqual(
            stale_row[ranking.SOURCE_CONTEXT_FIELD],
            ranking.group_source_context("C" * 43),
        )

    def test_missing_masked_evidence_is_backfilled_without_losing_raw_proof(self) -> None:
        row = function_row(*self.keep, differing_words=1)
        row[ranking.SOURCE_CONTEXT_FIELD] = self.contexts[self.keep]
        document = ranking_document([row])

        plan = ranking.plan_incremental_refresh(
            document,
            [queue_item(*self.keep)],
            {self.keep: self.contexts[self.keep]},
            {},
            limit=0,
        )

        self.assertEqual(plan.fresh_rows, {})
        self.assertEqual(plan.selected, [])
        self.assertEqual(plan.stale_count, 1)
        self.assertEqual(
            plan.deferred_rows[self.keep][ranking.SOURCE_CONTEXT_FIELD],
            self.contexts[self.keep],
        )

    def test_cli_item_error_leaves_input_byte_identical(self) -> None:
        document = ranking_document(
            unresolved=[[[*self.pending], "old compile failure"]]
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.json"
            original = json.dumps(document, indent=2) + "\n"
            path.write_text(original, encoding="utf-8")
            item = queue_item(*self.pending)
            argv = [
                "nm_ranking.py", "--refresh-stale", "--out", str(path),
                "--no-table", "--jobs", "1",
            ]
            with mock.patch.object(sys, "argv", argv), mock.patch.object(
                ranking.pb, "discover_queue", return_value=[item]
            ), mock.patch.object(
                ranking, "current_source_contexts",
                return_value={
                    self.pending: ranking.group_source_context("D" * 43)
                },
            ), mock.patch.object(
                ranking, "legacy_source_contexts", return_value={}
            ), mock.patch.object(
                ranking, "missing_permuter_inputs", return_value=[]
            ), mock.patch.object(
                ranking, "process_items",
                return_value=([], [(self.pending, "compile failed")]),
            ):
                self.assertEqual(ranking.main(), 2)
            self.assertEqual(path.read_text(encoding="utf-8"), original)


class ForcedObjectGuardTests(unittest.TestCase):
    """Every scorer here recompiles the TU before measuring, which overwrites
    an object an instrumented or forced run produced. A lane read the same
    score for twelve different forces before noticing, so the condition warns."""

    def setUp(self) -> None:
        ranking._forced_warning_emitted = False
        self.addCleanup(setattr, ranking, "_forced_warning_emitted", False)

    def _warn(self, env: dict) -> str:
        import contextlib
        import io
        buf = io.StringIO()
        with unittest.mock.patch.dict(os.environ, env, clear=True):
            with contextlib.redirect_stderr(buf):
                ranking._warn_if_a_forced_object_would_be_discarded()
        return buf.getvalue()

    def test_a_force_variable_warns(self) -> None:
        self.assertIn("CDX_FORCE", self._warn({"CDX_FORCE": "p1:w1=c2"}))

    def test_an_instrumented_compiler_warns(self) -> None:
        self.assertIn("IDO_DIR", self._warn({"IDO_DIR": "/tmp/instrumented"}))

    def test_a_clean_environment_is_silent(self) -> None:
        self.assertEqual(self._warn({}), "")

    def test_an_empty_value_is_not_treated_as_set(self) -> None:
        self.assertEqual(self._warn({"CDX_FORCE": ""}), "")

    def test_the_warning_is_emitted_once_not_per_translation_unit(self) -> None:
        env = {"CDX_FORCE": "p1:w1=c2"}
        self.assertNotEqual(self._warn(env), "")
        self.assertEqual(self._warn(env), "", "a sweep would drown in repeats")


class RegisterMaskTests(unittest.TestCase):
    """instr_reg_mask decides the `register-only` label, so a field it treats
    as a register when it is not (or the reverse) mislabels a whole class of
    residual. Both cases below were wrong until 2026-09-10."""

    def test_shift_amount_is_an_immediate_not_a_register(self) -> None:
        # sll $t0, $t1, 3   vs   sll $t0, $t1, 5
        by_three = (0x00 << 26) | (9 << 16) | (8 << 11) | (3 << 6) | 0x00
        by_five = (0x00 << 26) | (9 << 16) | (8 << 11) | (5 << 6) | 0x00
        self.assertNotEqual(
            ranking.instr_reg_mask(by_three), ranking.instr_reg_mask(by_five),
            "a differing shift amount is a real difference, not a register swap")

    def test_special_still_erases_its_register_fields(self) -> None:
        # addu $t0,$t1,$t2  vs  addu $s0,$s1,$s2, same shamt and function
        a = (0x00 << 26) | (9 << 21) | (10 << 16) | (8 << 11) | 0x21
        b = (0x00 << 26) | (17 << 21) | (18 << 16) | (16 << 11) | 0x21
        self.assertEqual(ranking.instr_reg_mask(a), ranking.instr_reg_mask(b))

    def test_float_register_rotation_is_a_register_difference(self) -> None:
        # add.s $f4,$f6,$f8   vs   add.s $f16,$f18,$f20
        low = (0x11 << 26) | (0x10 << 21) | (8 << 16) | (6 << 11) | (4 << 6)
        high = (0x11 << 26) | (0x10 << 21) | (20 << 16) | (18 << 11) | (16 << 6)
        self.assertEqual(
            ranking.instr_reg_mask(low), ranking.instr_reg_mask(high),
            "a rotated float ring is a naming difference, not a structural one")

    def test_float_format_is_semantic_and_survives_the_mask(self) -> None:
        # add.s vs add.d on the same registers
        single = (0x11 << 26) | (0x10 << 21) | (8 << 16) | (6 << 11) | (4 << 6)
        double = (0x11 << 26) | (0x11 << 21) | (8 << 16) | (6 << 11) | (4 << 6)
        self.assertNotEqual(
            ranking.instr_reg_mask(single), ranking.instr_reg_mask(double),
            ".s versus .d is a real difference, not a register swap")

    def test_bc1_names_no_register_and_is_left_alone(self) -> None:
        # bc1t with a branch offset: bits 20-16 are cc/nd/tf, not a register
        word = (0x11 << 26) | (0x08 << 21) | (1 << 16) | 0x0042
        self.assertEqual(ranking.instr_reg_mask(word), word)

    def test_float_load_erases_base_and_ft_but_keeps_the_offset(self) -> None:
        near = (0x31 << 26) | (29 << 21) | (4 << 16) | 0x0010   # lwc1 $f4,16($sp)
        far = (0x31 << 26) | (16 << 21) | (20 << 16) | 0x0010   # lwc1 $f20,16($s0)
        other = (0x31 << 26) | (29 << 21) | (4 << 16) | 0x0020  # different offset
        self.assertEqual(ranking.instr_reg_mask(near), ranking.instr_reg_mask(far))
        self.assertNotEqual(ranking.instr_reg_mask(near), ranking.instr_reg_mask(other))


class MismatchEvidenceTests(unittest.TestCase):
    def test_reports_raw_and_relocation_masked_evidence(self) -> None:
        base = [0x0C000001, 0x3C021234, 0x24420001, 0x24030001]
        target = [0x0C333333, 0x3C02ABCD, 0x24420001, 0x24030002]
        base_relocs = {0: ("R_MIPS_26", "callee")}
        target_relocs = {4: ("R_MIPS_HI16", "global")}

        evidence = ranking.classify(
            16, 16, base, target, base_relocs, target_relocs
        )

        self.assertEqual(evidence, ("other", 3, 0, 1, 12))

    def test_pure_linker_payload_difference_is_reloc_mismatch(self) -> None:
        base = [0x0C000001, 0x3C021234]
        target = [0x0C333333, 0x3C02ABCD]
        relocs = {
            0: ("R_MIPS_26", "candidate_callee"),
            4: ("R_MIPS_HI16", "candidate_global"),
        }

        evidence = ranking.classify(
            8, 8, base, target, relocs, relocs
        )

        self.assertEqual(evidence, ("reloc-mismatch", 2, 0, 0, None))

    def test_relocation_mask_preserves_opcode_differences(self) -> None:
        base = [0x0C000001]  # jal
        target = [0x08000001]  # j
        relocs = {0: ("R_MIPS_26", "destination")}

        evidence = ranking.classify(
            4, 4, base, target, relocs, relocs
        )

        self.assertEqual(evidence, ("other", 1, 0, 1, 0))

    def test_unknown_relocation_kind_does_not_hide_a_difference(self) -> None:
        relocs = {0: ("R_MIPS_UNKNOWN", "destination")}

        evidence = ranking.classify(
            4, 4, [0x0C000001], [0x0C333333], relocs, relocs
        )

        self.assertEqual(evidence, ("other", 1, 0, 1, 0))

    def test_missing_words_remain_mismatches_in_both_views(self) -> None:
        evidence = ranking.classify(
            8,
            4,
            [0x0C000001, 0x00000000],
            [0x0C333333],
            {0: ("R_MIPS_26", "callee")},
            {0: ("R_MIPS_26", "callee")},
        )

        self.assertEqual(evidence, ("size-mismatch", 2, 0, 1, 4))


class DocumentationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = ranking_document(
            [
                function_row(
                    "src/main/a.c",
                    "same",
                    pct=87.25,
                    category="register-only",
                    differing_words=1,
                ),
                function_row(
                    "src/main/b.c",
                    "same",
                    category="size-mismatch",
                    differing_words=5,
                ),
            ],
            [[["src/main/pending.c", "pending"], "compile failed"]],
        )

    def test_render_is_stable_and_contains_complete_exact_identities(self) -> None:
        first = ranking.render_ranking_markdown(self.document)
        second = ranking.render_ranking_markdown(
            json.loads(json.dumps(self.document, sort_keys=True))
        )
        self.assertEqual(first, second)
        self.assertIn("**3 queued identities**", first)
        self.assertIn("`src/main/a.c` | `same`", first)
        self.assertIn("`src/main/b.c` | `same`", first)
        self.assertIn("`src/main/pending.c` | `pending` | compile failed", first)

    def test_schema_three_render_exposes_both_mismatch_views(self) -> None:
        row = function_row(
            "src/main/a.c",
            "symbol",
            differing_words=3,
            relocation_masked_differing_words=1,
        )
        document = ranking.make_ranking_document(
            [row], [], objdiff_report_used=False
        )

        rendered = ranking.render_ranking_markdown(document)

        self.assertIn("**1 / 1** resolved rows", rendered)
        self.assertIn("| Raw diff | Masked diff | Raw first | Masked first |", rendered)
        self.assertIn("| 3 | 1 | 4 | 4 |", rendered)

    def test_generated_markers_preserve_authored_prose_byte_for_byte(self) -> None:
        original = (
            "authored before\n"
            f"{ranking.DOC_BEGIN}\nold generated\n{ranking.DOC_END}\n"
            "authored after\n"
        )
        replaced = ranking.replace_generated_markdown(original, "new generated\n")
        self.assertTrue(replaced.startswith("authored before\n"))
        self.assertTrue(replaced.endswith("\nauthored after\n"))
        self.assertIn("new generated\n", replaced)
        self.assertNotIn("old generated", replaced)

    def test_missing_or_duplicate_markers_fail_closed(self) -> None:
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "exactly one generated"
        ):
            ranking.replace_generated_markdown("no markers\n", "generated\n")
        duplicated = (
            f"{ranking.DOC_BEGIN}\n{ranking.DOC_BEGIN}\n{ranking.DOC_END}\n"
        )
        with self.assertRaisesRegex(
            ranking.RankingDocumentError, "exactly one generated"
        ):
            ranking.replace_generated_markdown(duplicated, "generated\n")

    def test_expected_document_detects_stale_generated_content(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "ranking.md"
            path.write_text(
                "authored\n"
                f"{ranking.DOC_BEGIN}\nstale\n{ranking.DOC_END}\n",
                encoding="utf-8",
            )
            current, expected = ranking.expected_document_text(
                self.document, path
            )
            self.assertNotEqual(current, expected)
            path.write_text(expected, encoding="utf-8")
            current, expected = ranking.expected_document_text(
                self.document, path
            )
            self.assertEqual(current, expected)


if __name__ == "__main__":
    unittest.main()
