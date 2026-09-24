#!/usr/bin/env python3
"""Source-level tests for safe plateau finalization."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import finalize_plateau as plateau  # noqa: E402
import plateau_handoff_audit as handoff_audit  # noqa: E402


VALID_SOURCE = """#include \"common.h\"

#ifdef NON_MATCHING
void demo_symbol(int value) {
#ifdef EXTRA_PATH
    value++;
#endif
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
#endif
"""

SOURCE_WITH_UNRELATED_DECLARATION_GUARD = """#include \"common.h\"

#ifdef NON_MATCHING
extern int helper_only_needed_by_candidates(void);
#ifdef DECLARATION_DETAIL
extern int another_candidate_helper(void);
#endif
#endif

""" + VALID_SOURCE


class GuardValidationTests(unittest.TestCase):
    def test_requires_exact_guarded_candidate_and_fallback(self) -> None:
        candidate = plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        self.assertEqual(candidate.fallback, "asm/nonmatchings/main/demo/demo_symbol.s")

    def test_ignores_balanced_unrelated_guard_without_else(self) -> None:
        candidate = plateau.require_guarded_candidate(
            SOURCE_WITH_UNRELATED_DECLARATION_GUARD, "demo_symbol"
        )
        self.assertEqual(candidate.fallback, "asm/nonmatchings/main/demo/demo_symbol.s")

    def test_ignores_multiline_call_followed_by_block_in_other_guard(self) -> None:
        source = """#ifdef NON_MATCHING
int other_symbol(int value) {
    if ((demo_symbol(
             value)) != 0) {
        return 1;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/other_symbol.s")
#endif

""" + VALID_SOURCE
        candidate = plateau.require_guarded_candidate(source, "demo_symbol")
        self.assertEqual(candidate.fallback, "asm/nonmatchings/main/demo/demo_symbol.s")

    def test_rejects_target_guard_without_else(self) -> None:
        source = """#ifdef NON_MATCHING
void demo_symbol(void) {}
#endif
"""
        with self.assertRaisesRegex(plateau.PlateauError, "exactly one top-level #else"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_rejects_unterminated_target_guard(self) -> None:
        source = """#ifdef NON_MATCHING
void demo_symbol(void) {}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
"""
        with self.assertRaisesRegex(plateau.PlateauError, "unterminated target"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_rejects_nested_target_guard(self) -> None:
        source = """#ifdef OUTER_FEATURE
#ifdef NON_MATCHING
void demo_symbol(void) {}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
#endif
#endif
"""
        with self.assertRaisesRegex(plateau.PlateauError, "nested target"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_rejects_ambiguous_target_branches(self) -> None:
        source = """#ifdef NON_MATCHING
void demo_symbol(void) {}
#elif defined(ANOTHER_CANDIDATE)
void demo_symbol(void) {}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/demo_symbol.s")
#endif
"""
        with self.assertRaisesRegex(plateau.PlateauError, "ambiguous target"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_rejects_unguarded_candidate(self) -> None:
        source = "void demo_symbol(void) {}\n"
        with self.assertRaisesRegex(plateau.PlateauError, "not an unambiguous"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_rejects_fallback_for_a_different_symbol(self) -> None:
        source = VALID_SOURCE.replace("demo_symbol.s", "other_symbol.s")
        with self.assertRaisesRegex(plateau.PlateauError, "exactly one"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_accepts_generated_overlay_fallback_for_friendly_symbol(self) -> None:
        generated = VALID_SOURCE.replace(
            "demo_symbol.s", "func_overlay_040_F0000690_1886F40.s"
        )
        candidate = plateau.require_guarded_candidate(generated, "demo_symbol")
        self.assertTrue(candidate.fallback.endswith("func_overlay_040_F0000690_1886F40.s"))

    def test_rejects_non_assembly_fallback_with_matching_stem(self) -> None:
        source = VALID_SOURCE.replace("demo_symbol.s", "demo_symbol.c")
        with self.assertRaisesRegex(plateau.PlateauError, "exactly one"):
            plateau.require_guarded_candidate(source, "demo_symbol")

    def test_handoff_is_fixed_field_and_idempotent(self) -> None:
        metrics = plateau.Metrics("98/101 words", "0x8", 10, "+0xC", "allocator mismatch")
        plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        once = plateau.update_source(
            VALID_SOURCE, "demo_symbol", plateau.source_handoff("demo_symbol", metrics)
        )
        plateau.require_guarded_candidate(once, "demo_symbol")
        twice = plateau.update_source(
            once, "demo_symbol", plateau.source_handoff("demo_symbol", metrics)
        )
        self.assertEqual(once, twice)
        self.assertEqual(once.count("PLATEAU-HANDOFF"), 2)
        self.assertIn(" * relocations: 10\n", once)
        self.assertNotIn("|", once)

    def test_handoff_appends_without_changing_existing_bytes_or_lines(self) -> None:
        metrics = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        updated = plateau.update_source(
            VALID_SOURCE, "demo_symbol", plateau.source_handoff("demo_symbol", metrics)
        )
        self.assertTrue(updated.startswith(VALID_SOURCE))
        self.assertEqual(
            updated.splitlines()[:len(VALID_SOURCE.splitlines())],
            VALID_SOURCE.splitlines(),
        )
        self.assertGreater(updated.index("PLATEAU-HANDOFF"), updated.index("#endif"))

    def test_updates_one_symbol_in_a_multi_symbol_eof_suffix(self) -> None:
        first = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        second = plateau.Metrics("7/8 words", "frameless", 0, "+0x4")
        plateau.require_guarded_candidate(VALID_SOURCE, "demo_symbol")
        with_first = plateau.update_source(
            VALID_SOURCE, "demo_symbol", plateau.source_handoff("demo_symbol", first)
        )
        with_both = plateau.update_source(
            with_first, "other_symbol", plateau.source_handoff("other_symbol", second)
        )
        revised = plateau.update_source(
            with_both,
            "demo_symbol",
            plateau.source_handoff(
                "demo_symbol", plateau.Metrics("99/101 words", "0x8", 10, "+0x10")
            ),
        )
        self.assertTrue(revised.startswith(VALID_SOURCE))
        self.assertIn("99/101 words", revised)
        self.assertNotIn("98/101 words", revised)
        self.assertIn("7/8 words", revised)
        self.assertEqual(revised.count("PLATEAU-HANDOFF:demo_symbol:start"), 1)
        self.assertEqual(revised.count("PLATEAU-HANDOFF:other_symbol:start"), 1)

    def test_refuses_legacy_inline_handoff_instead_of_moving_source(self) -> None:
        legacy = VALID_SOURCE.replace(
            "#ifdef NON_MATCHING\n",
            "#ifdef NON_MATCHING\n/* PLATEAU-HANDOFF\n * symbol: demo_symbol\n */\n",
        )
        plateau.require_guarded_candidate(legacy, "demo_symbol")
        with self.assertRaisesRegex(plateau.PlateauError, "move measured source lines"):
            plateau.update_source(
                legacy,
                "demo_symbol",
                plateau.source_handoff(
                    "demo_symbol", plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
                ),
            )

    def test_markdown_handoff_replaces_its_own_block(self) -> None:
        first = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        second = plateau.Metrics("99/101 words", "0x8", 10, "+0x10")
        text = "# Ledger\n"
        text = plateau.update_markdown(
            text, "demo_symbol", plateau.markdown_handoff("demo_symbol", "src/demo.c", first)
        )
        text = plateau.update_markdown(
            text, "demo_symbol", plateau.markdown_handoff("demo_symbol", "src/demo.c", second)
        )
        self.assertEqual(text.count("plateau-handoff:demo_symbol:start"), 1)
        self.assertIn("99/101 words", text)
        self.assertNotIn("98/101 words", text)

    def test_symbol_shard_is_strict_and_source_identified(self) -> None:
        metrics = plateau.Metrics("98/101 words", "0x8", 10, "+0xC")
        block = plateau.markdown_handoff(
            "demo_symbol", "src/demo.c", metrics,
        )
        self.assertEqual(
            plateau.handoff_shard_source(block, "demo_symbol"),
            "src/demo.c",
        )
        self.assertEqual(
            plateau.update_handoff_shard(block, "demo_symbol", block), block,
        )
        # A shard belonging to another symbol carries none of this symbol's
        # markers, and the diagnostic says exactly that rather than reporting
        # a bare "malformed or foreign".
        with self.assertRaisesRegex(
            plateau.PlateauError, "missing its start marker"
        ):
            plateau.update_handoff_shard(
                block.replace("demo_symbol", "other_symbol"),
                "demo_symbol",
                block,
            )
        with self.assertRaisesRegex(plateau.PlateauError, "non-canonical"):
            plateau.handoff_shard_source(
                block.replace("src/demo.c", "src/other/../demo.c"),
                "demo_symbol",
            )

    def test_symbol_shard_accepts_details_after_canonical_header(self) -> None:
        block = plateau.markdown_handoff(
            "demo_symbol",
            "src/demo.c",
            plateau.Metrics("98/101 words", "0x8", 10, "+0xC"),
        )
        enriched = block.replace(
            "<!-- plateau-handoff:demo_symbol:end -->",
            "- attempts: ten bounded source forms\n"
            "- next action: reopen only with new allocator evidence\n"
            "<!-- plateau-handoff:demo_symbol:end -->",
        )
        self.assertEqual(
            plateau.handoff_shard_source(enriched, "demo_symbol"),
            "src/demo.c",
        )

    def test_symbol_shard_rejects_nested_detail_marker(self) -> None:
        block = plateau.markdown_handoff(
            "demo_symbol",
            "src/demo.c",
            plateau.Metrics("98/101 words", "0x8", 10, "+0xC"),
        )
        malformed = block.replace(
            "<!-- plateau-handoff:demo_symbol:end -->",
            "<!-- plateau-handoff:other_symbol:start -->\n"
            "<!-- plateau-handoff:demo_symbol:end -->",
        )
        # The refusal must name the cause: a second symbol's marker nested in
        # this shard's evidence, not a generic "malformed" verdict.
        with self.assertRaisesRegex(
            plateau.PlateauError, "second plateau-handoff marker"
        ):
            plateau.handoff_shard_source(malformed, "demo_symbol")

    def test_shard_paths_are_fixed_per_symbol(self) -> None:
        self.assertEqual(
            plateau.handoff_shard_path("demo_symbol"),
            "docs/matching-triage-handoffs/demo_symbol.md",
        )
        self.assertNotEqual(
            plateau.handoff_shard_path("demo_symbol"),
            plateau.handoff_shard_path("other_symbol"),
        )
        with self.assertRaisesRegex(plateau.PlateauError, "invalid exact symbol"):
            plateau.handoff_shard_path("../escape")


class MatchedSymbolDiagnosticTests(unittest.TestCase):
    """Tell a matched function apart from a malformed guard.

    Both reach require_guarded_candidate with no candidate, but they need
    opposite fixes: a matched function's plateau block is dead and should be
    deleted, while a malformed guard should be repaired. This collision has
    landed twice, both times through a merge that reported no conflict --
    the plateau record and the match live in different regions of the file,
    so only this audit sees the result is inconsistent.
    """

    MATCHED = "void func_X(s32 arg0) {\n    return;\n}\n"
    GUARDED = (
        "#ifdef NON_MATCHING\n"
        "void func_X(s32 arg0) {\n    return;\n}\n"
        "#else\n"
        '#pragma GLOBAL_ASM("asm/nonmatchings/main/x/func_X.s")\n'
        "#endif\n"
    )

    def test_a_definition_without_a_fallback_is_recognized(self):
        self.assertTrue(plateau.defined_without_fallback(self.MATCHED, "func_X"))

    def test_a_guarded_candidate_has_its_own_fallback(self):
        self.assertFalse(plateau.defined_without_fallback(self.GUARDED, "func_X"))

    def test_an_absent_symbol_is_not_flagged(self):
        self.assertFalse(plateau.defined_without_fallback("int other(void) { return 0; }\n", "func_X"))

    def test_another_symbols_fallback_does_not_count_as_ours(self):
        text = self.MATCHED + '#pragma GLOBAL_ASM("asm/nonmatchings/main/x/func_Y.s")\n'
        self.assertTrue(plateau.defined_without_fallback(text, "func_X"))

    def test_the_error_keeps_its_original_sentence_and_adds_the_cause(self):
        with self.assertRaises(plateau.PlateauError) as caught:
            plateau.require_guarded_candidate(self.MATCHED, "func_X")
        message = str(caught.exception)
        self.assertIn("not an unambiguous", message)
        self.assertIn("already matched", message)
        self.assertIn("removed", message)

    def test_the_error_offers_both_readings_rather_than_asserting_one(self):
        """The file cannot distinguish a match from a never-wrapped candidate."""
        with self.assertRaises(plateau.PlateauError) as caught:
            plateau.require_guarded_candidate(self.MATCHED, "func_X")
        message = str(caught.exception)
        self.assertIn("never", message)
        self.assertIn("cannot tell those apart", message)

    def test_a_genuinely_malformed_guard_keeps_the_original_message(self):
        """Do not mislabel a broken guard as a landed match."""
        text = '#pragma GLOBAL_ASM("asm/nonmatchings/main/x/func_X.s")\n'
        with self.assertRaises(plateau.PlateauError) as caught:
            plateau.require_guarded_candidate(text, "func_X")
        self.assertIn("not an unambiguous", str(caught.exception))


class FinalizeCommandTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.base = Path(self.temporary.name)
        self.repo = self.base / "repo"
        self.repo.mkdir()
        self.run_command("git", "init", "-q")
        self.run_command("git", "config", "user.email", "plateau@example.invalid")
        self.run_command("git", "config", "user.name", "Plateau Test")
        (self.repo / "src").mkdir()
        (self.repo / "docs").mkdir()
        (self.repo / "docs" / "matching-triage-handoffs").mkdir()
        (self.repo / "src" / "demo.c").write_text(VALID_SOURCE, encoding="utf-8")
        (self.repo / "docs" / "handoff.md").write_text("# Handoffs\n", encoding="utf-8")
        (self.repo / "docs" / "matching-triage.md").write_text(
            "# Matching triage\n", encoding="utf-8"
        )
        (self.repo / "other.txt").write_text("clean\n", encoding="utf-8")
        self.run_command("git", "add", ".")
        self.run_command("git", "commit", "-q", "-m", "initial")

        self.bin = self.base / "bin"
        self.bin.mkdir()
        self.gate_log = self.base / "gates.log"
        fake_gmake = self.bin / "gmake"
        fake_gmake.write_text(
            "#!/bin/sh\nprintf '%s\\n' \"$*\" >> \"$GATE_LOG\"\n",
            encoding="utf-8",
        )
        fake_gmake.chmod(0o755)
        self.env = os.environ.copy()
        self.env["PATH"] = f"{self.bin}{os.pathsep}{self.env['PATH']}"
        self.env["GATE_LOG"] = str(self.gate_log)
        self.env.pop("MICKEY_COMMIT_TRAILER", None)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def run_command(self, *command: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            command, cwd=self.repo, env=getattr(self, "env", None), text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=check,
        )

    def finalize(self, *extra: str) -> subprocess.CompletedProcess[str]:
        command = (
            sys.executable,
            str(TOOLS / "finalize_plateau.py"),
            "demo_symbol",
            "src/demo.c",
            "--score", "98/101 words",
            "--frame", "0x8",
            "--relocations", "10",
            "--first-mismatch", "+0xC",
            *extra,
        )
        return self.run_command(*command, check=False)

    def test_without_commit_records_handoff_and_runs_only_source_gates(self) -> None:
        before = self.run_command("git", "rev-list", "--count", "HEAD").stdout.strip()
        result = self.finalize()
        after = self.run_command("git", "rev-list", "--count", "HEAD").stdout.strip()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(before, after)
        self.assertIn("commit: not requested", result.stdout)
        self.assertIn("PLATEAU-HANDOFF", (self.repo / "src" / "demo.c").read_text())
        self.assertIn(
            "plateau-handoff:demo_symbol:start",
            (
                self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
            ).read_text(),
        )
        self.assertEqual(
            (self.repo / "docs" / "matching-triage.md").read_text(),
            "# Matching triage\n",
        )
        self.assertIn(
            "handoff-doc: docs/matching-triage-handoffs/demo_symbol.md",
            result.stdout,
        )
        self.assertTrue((self.repo / "src" / "demo.c").read_text().startswith(VALID_SOURCE))
        self.assertEqual(self.gate_log.read_text().splitlines(), ["cleanroom", "check-docs"])

    def test_one_invocation_writes_the_shard_and_source_block_together(self) -> None:
        result = self.finalize()
        self.assertEqual(result.returncode, 0, result.stderr)
        source = self.repo / "src" / "demo.c"
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        source_text = source.read_text(encoding="utf-8")
        shard_text = shard.read_text(encoding="utf-8")
        self.assertIn("score: 98/101 words", source_text)
        self.assertIn("- score: 98/101 words\n", shard_text)
        self.assertEqual(source.stat().st_mtime_ns, shard.stat().st_mtime_ns)

    def test_command_accepts_unrelated_declaration_only_guard(self) -> None:
        (self.repo / "src" / "demo.c").write_text(
            SOURCE_WITH_UNRELATED_DECLARATION_GUARD, encoding="utf-8"
        )
        result = self.finalize()
        self.assertEqual(result.returncode, 0, result.stderr)
        updated = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        self.assertTrue(updated.startswith(SOURCE_WITH_UNRELATED_DECLARATION_GUARD))
        self.assertIn("PLATEAU-HANDOFF:demo_symbol:start", updated)

    def test_explicit_commit_contains_only_named_source_and_doc(self) -> None:
        result = self.finalize(
            "--handoff-doc", "docs/handoff.md", "--summary", "one allocator web remains",
            "--commit",
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        changed = self.run_command(
            "git", "show", "--pretty=format:", "--name-only", "HEAD"
        ).stdout.splitlines()
        self.assertEqual(set(changed), {"src/demo.c", "docs/handoff.md"})
        self.assertIn("plateau-handoff:demo_symbol:start", (self.repo / "docs/handoff.md").read_text())

    def test_default_commit_contains_only_source_and_symbol_shard(self) -> None:
        result = self.finalize("--commit")
        self.assertEqual(result.returncode, 0, result.stderr)
        changed = self.run_command(
            "git", "show", "--pretty=format:", "--name-only", "HEAD"
        ).stdout.splitlines()
        self.assertEqual(set(changed), {
            "src/demo.c",
            "docs/matching-triage-handoffs/demo_symbol.md",
        })

    def commit_body(self) -> str:
        return self.run_command("git", "log", "-1", "--format=%B").stdout

    def test_commit_without_trailer_or_environment_has_none(self) -> None:
        result = self.finalize("--commit")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.commit_body().strip(), "Plateau demo_symbol")

    def test_environment_trailer_is_the_default(self) -> None:
        self.env["MICKEY_COMMIT_TRAILER"] = "Co-Authored-By: Lane <lane@example.invalid>"
        result = self.finalize("--commit")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            self.commit_body().strip(),
            "Plateau demo_symbol\n\nCo-Authored-By: Lane <lane@example.invalid>",
        )
        trailers = self.run_command(
            "git", "log", "-1", "--format=%(trailers:key=Co-Authored-By,valueonly)",
        ).stdout.strip()
        self.assertEqual(trailers, "Lane <lane@example.invalid>")

    def test_explicit_trailers_replace_the_environment_and_repeat(self) -> None:
        self.env["MICKEY_COMMIT_TRAILER"] = "Co-Authored-By: Env <env@example.invalid>"
        result = self.finalize(
            "--commit", "--trailer", "Co-Authored-By: A <a@example.invalid>",
            "--trailer", "Claude-Session: https://example.invalid/s",
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        body = self.commit_body()
        self.assertNotIn("Env", body)
        self.assertTrue(body.strip().endswith(
            "Co-Authored-By: A <a@example.invalid>\n"
            "Claude-Session: https://example.invalid/s"
        ))

    def test_trailer_requires_commit_and_one_token_line(self) -> None:
        result = self.finalize("--trailer", "Co-Authored-By: A <a@example.invalid>")
        self.assertEqual(result.returncode, 2)
        self.assertIn("--trailer requires --commit", result.stderr)
        result = self.finalize("--commit", "--trailer", "not a trailer")
        self.assertEqual(result.returncode, 2)
        self.assertIn("invalid commit trailer", result.stderr)

    def test_two_symbols_never_edit_a_shared_ledger(self) -> None:
        other_source = VALID_SOURCE.replace("demo_symbol", "other_symbol")
        (self.repo / "src" / "other.c").write_text(other_source, encoding="utf-8")
        self.run_command("git", "add", "src/other.c")
        self.run_command("git", "commit", "-q", "-m", "add second candidate")

        first = self.finalize("--commit")
        self.assertEqual(first.returncode, 0, first.stderr)
        first_paths = set(self.run_command(
            "git", "show", "--pretty=format:", "--name-only", "HEAD"
        ).stdout.splitlines())

        second = self.run_command(
            sys.executable,
            str(TOOLS / "finalize_plateau.py"),
            "other_symbol",
            "src/other.c",
            "--score", "7/8 words",
            "--frame", "frameless",
            "--relocations", "0",
            "--first-mismatch", "+0x4",
            "--commit",
            check=False,
        )
        self.assertEqual(second.returncode, 0, second.stderr)
        second_paths = set(self.run_command(
            "git", "show", "--pretty=format:", "--name-only", "HEAD"
        ).stdout.splitlines())
        self.assertEqual(first_paths, {
            "src/demo.c", "docs/matching-triage-handoffs/demo_symbol.md",
        })
        self.assertEqual(second_paths, {
            "src/other.c", "docs/matching-triage-handoffs/other_symbol.md",
        })
        self.assertTrue(first_paths.isdisjoint(second_paths))

    def test_malformed_existing_default_shard_refuses_before_source_write(self) -> None:
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        shard.write_text("# foreign content\n", encoding="utf-8")
        self.run_command("git", "add", "docs/matching-triage-handoffs/demo_symbol.md")
        self.run_command("git", "commit", "-q", "-m", "add malformed shard")
        before = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        # Foreign content has no markers at all; the diagnostic says which
        # one is missing rather than reporting a bare "malformed".
        self.assertIn("missing its start marker", result.stderr)
        self.assertEqual((self.repo / "src" / "demo.c").read_text(), before)
        self.assertFalse(self.gate_log.exists())

    def test_explicit_option_cannot_bypass_reserved_shard_schema(self) -> None:
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        shard.write_text(
            plateau.markdown_handoff(
                "demo_symbol",
                "src/demo.c",
                plateau.Metrics("98/101 words", "0x8", 10, "+0xC"),
            ),
            encoding="utf-8",
        )
        self.run_command("git", "add", "docs/matching-triage-handoffs/demo_symbol.md")
        self.run_command("git", "commit", "-q", "-m", "add canonical shard")
        before = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        result = self.finalize(
            "--handoff-doc", "docs/matching-triage-handoffs/demo_symbol.md",
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("directory is reserved", result.stderr)
        self.assertEqual((self.repo / "src" / "demo.c").read_text(), before)
        self.assertFalse(self.gate_log.exists())

    def test_refuses_unrelated_dirt_before_writing(self) -> None:
        (self.repo / "other.txt").write_text("dirty\n", encoding="utf-8")
        before = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        self.assertIn("unrelated worktree/index dirt", result.stderr)
        self.assertEqual((self.repo / "src" / "demo.c").read_text(encoding="utf-8"), before)
        self.assertFalse(self.gate_log.exists())

    def test_rejects_unguarded_source_before_gates(self) -> None:
        (self.repo / "src" / "demo.c").write_text(
            "void demo_symbol(void) {}\n", encoding="utf-8"
        )
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        self.assertIn("not an unambiguous", result.stderr)
        self.assertFalse(self.gate_log.exists())

    def test_refuses_legacy_inline_handoff_before_gates(self) -> None:
        legacy = VALID_SOURCE.replace(
            "#ifdef NON_MATCHING\n",
            "#ifdef NON_MATCHING\n/* PLATEAU-HANDOFF\n * symbol: demo_symbol\n */\n",
        )
        (self.repo / "src" / "demo.c").write_text(legacy, encoding="utf-8")
        result = self.finalize()
        self.assertEqual(result.returncode, 2)
        self.assertIn("move measured source lines", result.stderr)
        self.assertEqual((self.repo / "src" / "demo.c").read_text(), legacy)
        self.assertFalse(self.gate_log.exists())


class PlateauHandoffAuditTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name) / "repo"
        self.repo.mkdir()
        self.run_command("git", "init", "-q")
        self.run_command("git", "config", "user.email", "audit@example.invalid")
        self.run_command("git", "config", "user.name", "Audit Test")
        (self.repo / "src").mkdir()
        (self.repo / "docs" / "matching-triage-handoffs").mkdir(parents=True)
        (self.repo / "docs" / "matching-triage-handoffs" / "README.md").write_text(
            "# Handoffs\n", encoding="utf-8"
        )
        self.write_source("98/101 words")
        self.run_command("git", "add", ".")
        self.run_command("git", "commit", "-q", "-m", "initial")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def run_command(self, *command: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            command, cwd=self.repo, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=check,
        )

    def audit(self, *args: str) -> subprocess.CompletedProcess[str]:
        return self.run_command(
            sys.executable, str(TOOLS / "plateau_handoff_audit.py"),
            *args, check=False,
        )

    def write_source(self, score: str) -> None:
        marker = plateau.source_handoff(
            "demo_symbol", plateau.Metrics(score, "0x8", 10, "+0xC", "allocator mismatch")
        )
        (self.repo / "src" / "demo.c").write_text(
            VALID_SOURCE + "\n" + marker, encoding="utf-8"
        )

    def test_check_reports_missing_then_write_projects_only_the_shard(self) -> None:
        source_before = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        checked = self.audit("--check", "--json")
        report = json.loads(checked.stdout)
        self.assertEqual(checked.returncode, 1)
        self.assertEqual(report["shards"]["missing"], 1)
        self.assertEqual(report["shards"]["reconcilable"], 1)

        written = self.audit("--write", "--json")
        write_report = json.loads(written.stdout)
        self.assertEqual(written.returncode, 0, written.stderr)
        self.assertEqual(write_report["written_paths"], [
            "docs/matching-triage-handoffs/demo_symbol.md"
        ])
        self.assertEqual((self.repo / "src" / "demo.c").read_text(), source_before)
        self.assertFalse(list(
            (self.repo / "docs" / "matching-triage-handoffs").glob("*.tmp")
        ))
        self.assertEqual(self.audit("--check").returncode, 0)

    def test_stale_write_preserves_valid_symbol_owned_details(self) -> None:
        self.assertEqual(self.audit("--write").returncode, 0)
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        shard.write_text(
            shard.read_text(encoding="utf-8").replace(
                "<!-- plateau-handoff:demo_symbol:end -->",
                "- next action: retry with allocator evidence\n"
                "<!-- plateau-handoff:demo_symbol:end -->",
            ),
            encoding="utf-8",
        )
        self.run_command("git", "add", ".")
        self.run_command("git", "commit", "-q", "-m", "add shard details")
        self.write_source("99/101 words")

        checked = json.loads(self.audit("--check", "--json").stdout)
        self.assertEqual(checked["shards"]["stale"], 1)
        written = self.audit("--write")
        self.assertEqual(written.returncode, 0, written.stderr)
        revised = shard.read_text(encoding="utf-8")
        self.assertIn("- score: 99/101 words\n", revised)
        self.assertIn("- next action: retry with allocator evidence\n", revised)

    def test_a_disagreement_names_the_newer_side(self) -> None:
        self.assertEqual(self.audit("--write").returncode, 0)
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        shard.write_text(
            shard.read_text(encoding="utf-8").replace("98/101 words", "1/101 words"),
            encoding="utf-8",
        )
        shard_newer = self.audit("--check")
        self.assertIn("demo_symbol (shard newer)", shard_newer.stdout)

        self.write_source("99/101 words")
        os.utime(shard, (1, 1))
        source_newer = self.audit("--check")
        self.assertIn("demo_symbol (source newer)", source_newer.stdout)

    def test_regenerate_stale_shards_uses_the_merged_source_not_the_lane_shard(self) -> None:
        self.assertEqual(self.audit("--write").returncode, 0)
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        shard.write_text(
            shard.read_text(encoding="utf-8").replace("98/101 words", "1/101 words"),
            encoding="utf-8",
        )
        self.write_source("99/101 words")
        written = handoff_audit.regenerate_stale_shards(self.repo)
        self.assertEqual(written, ["docs/matching-triage-handoffs/demo_symbol.md"])
        revised = shard.read_text(encoding="utf-8")
        self.assertIn("- score: 99/101 words\n", revised)
        self.assertNotIn("1/101 words", revised)

    def test_duplicate_shard_blocks_are_malformed_and_not_overwritten(self) -> None:
        self.assertEqual(self.audit("--write").returncode, 0)
        shard = self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md"
        duplicated = shard.read_text(encoding="utf-8") * 2
        shard.write_text(duplicated, encoding="utf-8")
        self.run_command("git", "add", str(shard.relative_to(self.repo)))
        self.run_command("git", "commit", "-q", "-m", "duplicate shard block")

        result = self.audit("--write", "--json")
        report = json.loads(result.stdout)
        self.assertEqual(result.returncode, 2)
        self.assertIn("validation errors", report["error"])
        self.assertEqual(shard.read_text(encoding="utf-8"), duplicated)

    def test_malformed_marker_blocks_all_writes(self) -> None:
        malformed = VALID_SOURCE.replace("demo_symbol", "other_symbol") + """

/* PLATEAU-HANDOFF:other_symbol:start
 * symbol: other_symbol
 * score: 7/8 words
 * frame: frameless
 * relocations: 0
 * PLATEAU-HANDOFF:other_symbol:end
 */
"""
        (self.repo / "src" / "other.c").write_text(malformed, encoding="utf-8")
        self.run_command("git", "add", "src/other.c")
        self.run_command("git", "commit", "-q", "-m", "add malformed marker")

        result = self.audit("--write", "--json")
        report = json.loads(result.stdout)
        self.assertEqual(result.returncode, 2)
        self.assertIn("validation errors", report["error"])
        self.assertFalse(
            (self.repo / "docs" / "matching-triage-handoffs" / "demo_symbol.md").exists()
        )

    def test_duplicate_structured_markers_are_rejected(self) -> None:
        source = (self.repo / "src" / "demo.c").read_text(encoding="utf-8")
        (self.repo / "src" / "demo.c").write_text(source + source[source.index("/* PLATEAU"):])
        report = handoff_audit.audit_tree(self.repo)
        self.assertTrue(any(row.code == "duplicate-marker" for row in report.issues))

    def test_definition_ownership_ignores_externs_and_calls(self) -> None:
        other = """extern void demo_symbol(int value);
void caller(void) {
    if (demo_symbol(1)) {
    }
}
"""
        (self.repo / "src" / "caller.c").write_text(other, encoding="utf-8")
        self.run_command("git", "add", "src/caller.c")
        paths = handoff_audit.definition_paths(
            handoff_audit.tracked_sources(self.repo, handoff_audit.tracked_paths(self.repo)),
            "demo_symbol",
        )
        self.assertEqual(paths, ["src/demo.c"])

    def test_marker_source_must_be_the_only_exact_definition(self) -> None:
        duplicate = VALID_SOURCE.replace(
            "demo_symbol", "other_symbol"
        ).replace("other_symbol", "demo_symbol")
        (self.repo / "src" / "duplicate.c").write_text(duplicate, encoding="utf-8")
        self.run_command("git", "add", "src/duplicate.c")
        report = handoff_audit.audit_tree(self.repo)
        self.assertTrue(any(row.code == "source-ownership" for row in report.issues))

    def test_prose_marker_is_reported_but_never_used_as_metrics(self) -> None:
        prose = VALID_SOURCE.replace("demo_symbol", "other_symbol") + """
/* PLATEAU-HANDOFF
 * Exact-sized candidate remains close; retry with allocator evidence.
 */
"""
        (self.repo / "src" / "other.c").write_text(prose, encoding="utf-8")
        self.run_command("git", "add", "src/other.c")
        report = handoff_audit.audit_tree(self.repo)
        self.assertEqual(len(report.markers), 1)
        self.assertEqual(len(report.unstructured), 1)
        self.assertEqual(report.unstructured[0].code, "unstructured-marker")



class ShardEvidenceRetentionTests(unittest.TestCase):
    """Refreshing a shard's measurement must not discard its evidence."""

    SYMBOL = "func_80001234"

    def shard(self, score: str, details: str = "") -> str:
        marker = f"plateau-handoff:{self.SYMBOL}"
        return (
            f"<!-- {marker}:start -->\n"
            f"### `{self.SYMBOL}` plateau handoff\n\n"
            "- source: `src/main/example.c`\n"
            f"- score: {score}\n"
            "- frame: 0x10\n"
            "- relocations: 2\n"
            "- first mismatch: +0x4\n"
            f"{details}"
            f"<!-- {marker}:end -->\n"
        )

    EVIDENCE = (
        "\nEliminated: three loop forms, two frame orders, the early return.\n"
        "Next lever: the pool rotation at the second speculative load.\n"
    )

    def test_appended_evidence_survives_a_measurement_refresh(self):
        # The regression: update_handoff_shard returned the freshly generated
        # header and dropped 84 lines of committed evidence from a real shard.
        merged = plateau.update_handoff_shard(
            self.shard("30/40", self.EVIDENCE), self.SYMBOL, self.shard("12/40")
        )
        self.assertIn("Eliminated: three loop forms", merged)
        self.assertIn("Next lever: the pool rotation", merged)

    def test_the_header_is_still_refreshed_to_the_new_measurement(self):
        merged = plateau.update_handoff_shard(
            self.shard("30/40", self.EVIDENCE), self.SYMBOL, self.shard("12/40")
        )
        self.assertIn("- score: 12/40", merged)
        self.assertNotIn("- score: 30/40", merged)

    def test_the_merged_shard_still_satisfies_the_reader_grammar(self):
        merged = plateau.update_handoff_shard(
            self.shard("30/40", self.EVIDENCE), self.SYMBOL, self.shard("12/40")
        )
        self.assertEqual(
            plateau.handoff_shard_source(merged, self.SYMBOL),
            "src/main/example.c",
        )

    def test_a_replaced_summary_is_carried_into_the_evidence(self):
        # A long committed summary cannot be re-entered through --summary
        # (160 characters); a remeasure must not silently drop it.
        long = "Eliminated the frame axis; " + "x" * 300
        old = self.shard("30/40", f"- summary: {long}\n" + self.EVIDENCE)
        new = self.shard("12/40", "- summary: Remeasured; unchanged blocker.\n")
        merged = plateau.update_handoff_shard(old, self.SYMBOL, new)
        self.assertIn(f"Summary before this remeasure: {long}", merged)
        self.assertIn("- summary: Remeasured; unchanged blocker.", merged)
        self.assertIn("Eliminated: three loop forms", merged)
        self.assertEqual(
            plateau.update_handoff_shard(merged, self.SYMBOL, new).count(
                "Summary before this remeasure"), 1,
        )

    def test_an_unchanged_summary_is_not_duplicated(self):
        same = self.shard("30/40", "- summary: same blocker\n")
        merged = plateau.update_handoff_shard(
            same, self.SYMBOL, self.shard("12/40", "- summary: same blocker\n"),
        )
        self.assertNotIn("Summary before this remeasure", merged)

    def test_a_shard_with_no_evidence_is_replaced_cleanly(self):
        merged = plateau.update_handoff_shard(
            self.shard("30/40"), self.SYMBOL, self.shard("12/40")
        )
        self.assertEqual(merged, self.shard("12/40"))

    def test_a_new_shard_is_written_as_generated(self):
        self.assertEqual(
            plateau.update_handoff_shard("", self.SYMBOL, self.shard("12/40")),
            self.shard("12/40"),
        )

    def test_a_foreign_shard_is_still_refused_rather_than_merged(self):
        with self.assertRaises(plateau.PlateauError):
            plateau.update_handoff_shard(
                "not a shard at all\n", self.SYMBOL, self.shard("12/40")
            )


class OneLineDiagnosticTests(unittest.TestCase):
    """Each rule reports itself, rather than one message for four causes."""

    def message(self, value: str, limit: int = 160) -> str:
        with self.assertRaises(plateau.PlateauError) as caught:
            plateau.validate_one_line(value, "summary", limit)
        return str(caught.exception)

    def test_an_empty_value_says_so(self):
        self.assertIn("must not be empty", self.message(""))

    def test_an_over_long_value_names_both_lengths(self):
        message = self.message("x" * 200)
        self.assertIn("200 characters", message)
        self.assertIn("limit is 160", message)

    def test_a_pipe_is_named_as_the_column_separator(self):
        self.assertIn("column separator", self.message("a | b"))

    def test_a_newline_is_named(self):
        self.assertIn("newline", self.message("a\nb"))



class ShardRejectionDiagnosticTests(unittest.TestCase):
    """A rejected shard must say which rule it broke."""

    SYMBOL = "func_80001234"

    def block(self, body: str) -> str:
        marker = f"plateau-handoff:{self.SYMBOL}"
        return f"<!-- {marker}:start -->\n{body}<!-- {marker}:end -->\n"

    def test_a_markdown_table_is_named_as_the_cause(self):
        # The reported case: a worker writes a measurement table, which is
        # made of pipes, and the shard is refused with a message that reads
        # like the symbol is wrong.
        message = plateau.shard_rejection_reason(
            self.block("before/after\n| a | b |\n"), self.SYMBOL
        )
        self.assertIn("'|'", message)
        self.assertIn("markdown table", message)

    def test_the_offending_line_is_located(self):
        message = plateau.shard_rejection_reason(
            self.block("one\ntwo\n| x |\n"), self.SYMBOL
        )
        self.assertIn("3", message)

    def test_a_missing_start_marker_says_so(self):
        self.assertIn(
            "missing its start marker",
            plateau.shard_rejection_reason("no markers at all\n", self.SYMBOL),
        )

    def test_a_repeated_marker_says_so(self):
        doubled = self.block("x\n") + self.block("y\n")
        self.assertIn(
            "repeats its start or end marker",
            plateau.shard_rejection_reason(doubled, self.SYMBOL),
        )

    def test_an_otherwise_bad_header_falls_back_to_naming_the_fields(self):
        message = plateau.shard_rejection_reason(self.block("plain\n"), self.SYMBOL)
        self.assertIn("relocations", message)
        self.assertIn("first mismatch", message)


if __name__ == "__main__":
    unittest.main()
