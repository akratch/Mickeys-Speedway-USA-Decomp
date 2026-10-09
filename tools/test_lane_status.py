#!/usr/bin/env python3
"""Regression tests for fail-closed lane assignment classification."""

from __future__ import annotations

from contextlib import ExitStack, redirect_stderr, redirect_stdout
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lane_status as ls  # noqa: E402


TOOL = Path(__file__).with_name("lane_status.py").resolve()
SYMBOL = "overlay43FilterImage"
SOURCE_PATH = Path("src/overlays/o043/overlay43FilterImage.c")
SHARD_PATH = Path("docs/matching-triage-handoffs") / f"{SYMBOL}.md"


def candidate(*, plateau: bool = False) -> str:
    text = f"""#ifdef NON_MATCHING
void {SYMBOL}(void) {{
}}
#else
#pragma GLOBAL_ASM(\"asm/nonmatchings/overlays/o043/{SYMBOL}.s\")
#endif
"""
    if plateau:
        text += f"""
/* PLATEAU-HANDOFF:{SYMBOL}:start
 * symbol: {SYMBOL}
 * score: 8/43 words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x4
 * PLATEAU-HANDOFF:{SYMBOL}:end
 */
"""
    return text


def shard(*, source: str = SOURCE_PATH.as_posix(), symbol: str = SYMBOL) -> str:
    return ls.finalize_plateau.markdown_handoff(
        symbol,
        source,
        ls.finalize_plateau.Metrics(
            "35/43 words", "frameless", 0, "+0x4", "allocator web remains",
        ),
    )


class ReopenSchemaTests(unittest.TestCase):
    def document(self, **row_changes):
        return {"schema_version": 1, "authorizations": {SYMBOL: {
            "source_commit": "a" * 40, "ledger_commit": "b" * 40,
            "reason": "new mechanism", **row_changes,
        }}}

    def parse(self, document):
        return ls.parse_reopen_authorizations(json.dumps(document), label="worktree.json")

    def test_reason_boundary_and_explicit_limit_diagnostic(self):
        for size in (1, 239, 240):
            with self.subTest(size=size):
                self.assertEqual(self.parse(self.document(reason="x" * size))[SYMBOL]["reason"], "x" * size)
        with self.assertRaisesRegex(RuntimeError, f"worktree.json: {SYMBOL}.*240"):
            self.parse(self.document(reason="x" * 241))

    def test_reason_rejects_wrong_types_empty_and_delimiters(self):
        for reason in (None, True, 12, [], {}, "", " \t", "a\nb", "a|b"):
            with self.subTest(reason=reason), self.assertRaisesRegex(RuntimeError, "240"):
                self.parse(self.document(reason=reason))

    def test_pins_require_full_lowercase_hashes_but_null_ledger_is_structural(self):
        for field in ("source_commit", "ledger_commit"):
            for value in (False, 1, [], {}, "", "a" * 39, "A" * 40, "g" * 40):
                with self.subTest(field=field, value=value), self.assertRaisesRegex(RuntimeError, field):
                    self.parse(self.document(**{field: value}))
        with self.assertRaisesRegex(RuntimeError, "source_commit"):
            self.parse(self.document(source_commit=None))
        self.assertIsNone(self.parse(self.document(ledger_commit=None))[SYMBOL]["ledger_commit"])

    def test_top_level_and_row_shapes_fail_closed(self):
        documents = [None, [], {}, self.document() | {"extra": 1}]
        for version in (None, True, 1.0, "1", 2):
            documents.append(self.document() | {"schema_version": version})
        for rows in (None, [], True, {"bad-symbol": {}}, {SYMBOL: None}, {SYMBOL: []}):
            documents.append(self.document() | {"authorizations": rows})
        for field in ("source_commit", "ledger_commit", "reason"):
            document = self.document()
            del document["authorizations"][SYMBOL][field]
            documents.append(document)
        documents.append(self.document(extra="unexpected"))
        for document in documents:
            with self.subTest(document=document), self.assertRaises(RuntimeError):
                self.parse(document)
        with self.assertRaisesRegex(RuntimeError, "worktree.json"):
            ls.parse_reopen_authorizations("{", label="worktree.json")

    def test_all_structure_checked_before_committed_history(self):
        document = self.document()
        document["authorizations"]["secondSymbol"] = dict(
            document["authorizations"][SYMBOL], reason="x" * 241,
        )
        ls.reopen_authorizations.cache_clear()
        try:
            with mock.patch.object(ls, "git", return_value=json.dumps(document)), \
                    mock.patch.object(ls, "is_ancestor", side_effect=AssertionError("history consulted")):
                with self.assertRaisesRegex(RuntimeError, "secondSymbol.*240"):
                    ls.reopen_authorizations("synthetic-base")
        finally:
            ls.reopen_authorizations.cache_clear()

    def schema_cli(self, *, raw=None, error=None, extra=()):
        stdout, stderr = io.StringIO(), io.StringIO()
        with ExitStack() as stack:
            stack.enter_context(mock.patch.object(sys, "argv", [str(TOOL), "--check-reopen-schema", *extra]))
            reader = stack.enter_context(mock.patch.object(Path, "read_text", return_value=raw, side_effect=error))
            for name in ("git", "is_ancestor", "source_identity", "collect", "show_file"):
                stack.enter_context(mock.patch.object(ls, name, side_effect=AssertionError(f"{name} consulted")))
            stack.enter_context(mock.patch.object(ls.integration_base, "resolve", side_effect=AssertionError("base resolved")))
            stack.enter_context(mock.patch.object(subprocess, "run", side_effect=AssertionError("subprocess launched")))
            stack.enter_context(redirect_stdout(stdout))
            stack.enter_context(redirect_stderr(stderr))
            result = ls.main()
        reader.assert_called_once_with(encoding="utf-8")
        return result, stdout.getvalue(), stderr.getvalue()

    def test_schema_cli_has_no_git_or_source_history_dependency(self):
        code, stdout, stderr = self.schema_cli(raw=json.dumps(self.document(ledger_commit=None)))
        self.assertEqual(code, 0, stderr)
        self.assertIn("not assignment authorization", stdout)

    def test_schema_cli_missing_invalid_and_unreadable_worktree_fail_closed(self):
        for raw, error in (("{", None), (None, FileNotFoundError("missing worktree JSON")),
                           (None, PermissionError("unreadable")),
                           (None, UnicodeError("invalid UTF-8"))):
            with self.subTest(raw=raw, error=error):
                code, stdout, stderr = self.schema_cli(raw=raw, error=error)
                self.assertEqual(code, 2)
                self.assertEqual(stdout, "")
                self.assertIn("lane_status:", stderr)

    def test_schema_mode_rejects_assignment_options(self):
        for extra in (("--base", "HEAD"), ("--symbol", SYMBOL), ("--json",), ("--pending-only",)):
            with self.subTest(extra=extra), self.assertRaises(SystemExit) as raised:
                self.schema_cli(extra=extra)
            self.assertEqual(raised.exception.code, 2)


VARIANT_BODY = """#ifdef NON_MATCHING
s32 {symbol}(s32 context) {{
#if CASE_PREINC
    context++;
#else
    context += 1;
#endif
    return context;
}}
#else
#pragma GLOBAL_ASM("{fallback}")
#endif
"""


class GuardedFallbackTests(unittest.TestCase):
    """A NON_MATCHING symbol must never read as matched from its spelling.

    The first two cases were classified already-integrated/exhausted on
    campaign/unchain at 8a9764fb: the old regex took the first ``#else``
    inside the candidate body as the guard's own, found no fallback there,
    and reported a live candidate as done.
    """

    def test_variant_switch_inside_body_per_function_directory(self) -> None:
        symbol = "func_overlay_014_F0001830_1871108"
        text = VARIANT_BODY.format(
            symbol=symbol,
            fallback=f"asm/nonmatchings/overlays/o014/{symbol}/{symbol}.s",
        )
        self.assertTrue(ls.guarded_fallback(text, symbol))

    def test_variant_switch_with_renamed_overlay_fallback(self) -> None:
        generated = "func_overlay_020_F0000A68_1877040"
        text = VARIANT_BODY.format(
            symbol="overlay20UpdateGrid",
            fallback=f"asm/nonmatchings/overlays/o020/overlay20UpdateGrid/{generated}.s",
        ).replace("#if CASE_PREINC", "#ifndef SCOPED_LOCALS")
        self.assertTrue(ls.guarded_fallback(
            text, "overlay20UpdateGrid", frozenset({generated}),
        ))

    def test_alias_from_the_build_redefine_rule(self) -> None:
        rules = (
            "$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o: POSTPROCESS = \\\n"
            "\t$(OBJCOPY) --redefine-sym \\\n"
            "\t\tsplat_name_A68=overlay20UpdateGrid $@ && \\\n"
            "\t$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x35C\n"
        )
        aliases = ls.finalize_plateau.fallback_aliases(rules)
        key = ("src/overlays/o020/overlay20UpdateGrid.c", "overlay20UpdateGrid")
        self.assertEqual(aliases[key], frozenset({"splat_name_A68"}))
        text = VARIANT_BODY.format(
            symbol="overlay20UpdateGrid",
            fallback="asm/nonmatchings/overlays/o020/overlay20UpdateGrid/splat_name_A68.s",
        )
        # A non-generated stem is only this symbol's through the rule.
        self.assertFalse(ls.guarded_fallback(text, "overlay20UpdateGrid"))
        self.assertTrue(ls.guarded_fallback(text, "overlay20UpdateGrid", aliases[key]))

    def test_descriptive_resident_name_over_generated_fallback(self) -> None:
        text = VARIANT_BODY.format(
            symbol="MatrixMultiplyVec4",
            fallback="asm/nonmatchings/main/matrix/func_8002AF6C.s",
        )
        self.assertTrue(ls.guarded_fallback(text, "MatrixMultiplyVec4"))

    def test_foreign_named_fallback_is_still_refused(self) -> None:
        text = VARIANT_BODY.format(
            symbol="demo_symbol",
            fallback="asm/nonmatchings/main/demo/other_symbol.s",
        )
        self.assertFalse(ls.guarded_fallback(text, "demo_symbol"))

    def test_matched_definition_has_no_fallback(self) -> None:
        text = "s32 demo_symbol(s32 context) {\n    return context;\n}\n"
        self.assertFalse(ls.guarded_fallback(text, "demo_symbol"))

    def test_two_fallbacks_in_one_guard_stay_ambiguous(self) -> None:
        text = VARIANT_BODY.format(
            symbol="demo_symbol",
            fallback="asm/nonmatchings/main/demo/demo_symbol.s",
        ).replace(
            "#endif\n", '#pragma GLOBAL_ASM("asm/nonmatchings/main/demo/next.s")\n#endif\n', 2,
        )
        self.assertFalse(ls.guarded_fallback(text, "demo_symbol"))


class LaneStatusAssignmentTests(unittest.TestCase):
    def setUp(self) -> None:
        ls.show_file.cache_clear()
        ls.blob_id.cache_clear()
        ls.build_fallback_aliases.cache_clear()
        ls.guarded_candidate_region.cache_clear()
        ls.target_guard_changed.cache_clear()
        ls.latest_target_body_record.cache_clear()
        ls.reopen_authorizations.cache_clear()
        ls.claim_dispositions.cache_clear()
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)
        self.command("git", "init", "-q", "-b", "campaign/unchain")
        self.command("git", "config", "user.email", "lane-status@example.invalid")
        self.command("git", "config", "user.name", "Lane Status Test")
        (self.repo / ".git/info/exclude").write_text("build/\n", encoding="utf-8")
        (self.repo / SOURCE_PATH.parent).mkdir(parents=True)
        (self.repo / "docs").mkdir()
        (self.repo / SOURCE_PATH).write_text(candidate(), encoding="utf-8")
        (self.repo / "docs/matching-triage.md").write_text(
            f"| `{SYMBOL}` | clean source is pending |\n", encoding="utf-8",
        )
        self.commit("Seed overlay43FilterImage evidence")

    def tearDown(self) -> None:
        ls.show_file.cache_clear()
        ls.blob_id.cache_clear()
        ls.build_fallback_aliases.cache_clear()
        ls.guarded_candidate_region.cache_clear()
        ls.target_guard_changed.cache_clear()
        ls.latest_target_body_record.cache_clear()
        ls.merge_base.cache_clear()
        ls.reopen_authorizations.cache_clear()
        ls.claim_dispositions.cache_clear()
        self.temporary.cleanup()

    def command(self, *command: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            command, cwd=self.repo, text=True, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, check=check,
        )

    def commit(self, subject: str) -> str:
        self.command("git", "add", ".")
        self.command("git", "commit", "-q", "-m", subject)
        return self.command("git", "rev-parse", "HEAD").stdout.strip()

    def status(self) -> tuple[subprocess.CompletedProcess[str], dict[str, object]]:
        result = self.command(
            sys.executable, str(TOOL), "--base", "campaign/unchain",
            "--symbol", SYMBOL, "--json", check=False,
        )
        return result, json.loads(result.stdout)

    def current_plateau(self) -> str:
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        return self.commit(f"Plateau {SYMBOL} allocator")

    def authorize_reopen(
        self, source_commit: str, ledger_commit: str | None, *,
        reason: str = "new mechanism",
    ) -> str:
        path = self.repo / ls.REOPEN_AUTHORIZATIONS_PATH
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps({
            "schema_version": 1,
            "authorizations": {
                SYMBOL: {
                    "source_commit": source_commit,
                    "ledger_commit": ledger_commit,
                    "reason": reason,
                },
            },
        }), encoding="utf-8")
        return self.commit(f"Authorize one-shot {SYMBOL} reproof")

    def shared_guard(self) -> str:
        text = candidate().replace(
            "#else", "void sibling(void) {}\n#else", 1).replace(
            "#endif", '#pragma GLOBAL_ASM("asm/sibling.s")\n#endif', 1)
        (self.repo / SOURCE_PATH).write_text(text)
        self.commit("Seed two owned functions in one guard")
        return text

    def test_structured_bank_checkpoint_needs_fresh_authorization(self):
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True))
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        checkpoint = self.commit(f"Bank {SYMBOL}")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["state"], "already-integrated/exhausted")
        self.assertEqual(report["assignment"]["source_commit"], checkpoint)
        self.authorize_reopen(checkpoint, checkpoint)
        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True).replace(
            "void " + SYMBOL + "(void) {", "void " + SYMBOL + "(void) { int changed;"))
        changed = self.commit("Refine guarded body")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["source_commit"], changed)

    def test_structured_bank_requires_target_owned_source_change(self):
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True))
        self.commit("Record source evidence alone")
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True) +
                                           "\nvoid sibling(void) {}\n")
        self.commit(f"Bank {SYMBOL} documentation with sibling edit")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["reason_code"], "prose-needs-remeasurement")

    def test_structured_bank_does_not_own_sibling_inside_same_guard(self):
        text = self.shared_guard() + candidate(plateau=True).split(
            "/* PLATEAU-HANDOFF", 1)[1].join(["\n/* PLATEAU-HANDOFF", ""])
        (self.repo / SOURCE_PATH).write_text(text)
        self.commit("Record source evidence alone")
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        (self.repo / SOURCE_PATH).write_text(text.replace(
            "void sibling(void) {}", "void sibling(void) { int changed; }"))
        self.commit("Bank sibling improvement with target shard")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["reason_code"], "prose-needs-remeasurement")

    def test_structured_bank_handoff_only_checkpoint(self):
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True))
        self.commit("Record source evidence alone")
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True).replace(
            "8/43 words", "7/43 words"))
        checkpoint = self.commit("Bank measured handoff")
        self.authorize_reopen(checkpoint, checkpoint)
        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_bank_rejects_foreign_or_malformed_inline_identity(self):
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        variants = [
            candidate(plateau=True).replace(f"PLATEAU-HANDOFF:{SYMBOL}:end",
                                             "PLATEAU-HANDOFF:sibling:end"),
            candidate(plateau=True).replace(f" * symbol: {SYMBOL}", " * symbol: sibling"),
            candidate(plateau=True).replace(f" * symbol: {SYMBOL}",
                                             f" * symbol: {SYMBOL}\n * symbol: {SYMBOL}"),
        ]
        for index, text in enumerate(variants):
            with self.subTest(index=index):
                (self.repo / SOURCE_PATH).write_text(text)
                (self.repo / SHARD_PATH).write_text(shard().replace("allocator web remains", f"allocator web remains; review {index}"))
                self.commit(f"Bank {SYMBOL} checkpoint {index}")
                result, report = self.status()
                self.assertEqual(result.returncode, 1)
                self.assertEqual(report["assignment"]["reason_code"], "prose-needs-remeasurement")

    def test_bank_subject_without_inline_handoff_is_not_checkpoint(self):
        (self.repo / SOURCE_PATH).write_text(candidate().replace(
            "void " + SYMBOL + "(void) {", "void " + SYMBOL + "(void) { int changed;"))
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        self.commit(f"Bank {SYMBOL}")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["reason_code"], "prose-needs-remeasurement")

    def test_history_check_rejects_renamed_historical_key_without_rearming(self):
        plateau = self.current_plateau()
        self.authorize_reopen(plateau, plateau)
        renamed = "renamedFilterImage"
        source = self.repo / SOURCE_PATH
        source.write_text(f"void {renamed}(void) {{}}\n", encoding="utf-8")
        self.commit("Rename already matched function")
        path = self.repo / ls.REOPEN_AUTHORIZATIONS_PATH
        document = json.loads(path.read_text())
        document["authorizations"][renamed] = document["authorizations"].pop(SYMBOL)
        path.write_text(json.dumps(document), encoding="utf-8")

        # Shape alone passes, but the new name never existed at the old pin.
        schema = self.command(sys.executable, str(TOOL), "--check-reopen-schema")
        self.assertEqual(schema.returncode, 0)
        history = self.command(
            sys.executable, str(TOOL), "--check-reopen-history", "--base", "HEAD",
            check=False,
        )
        self.assertEqual(history.returncode, 2)
        self.assertIn(renamed, history.stderr)
        self.assertIn("does not identify exactly one source definition", history.stderr)

        # Preserve a valid historical identity or retire its consumed entry;
        # neither operation copies an old reason onto a fresh source pin.
        document["authorizations"][SYMBOL] = document["authorizations"].pop(renamed)
        path.write_text(json.dumps(document), encoding="utf-8")
        valid = self.command(
            sys.executable, str(TOOL), "--check-reopen-history", "--base", "HEAD",
        )
        self.assertIn("not assignment authorization", valid.stdout)
        document["authorizations"].clear()
        path.write_text(json.dumps(document), encoding="utf-8")
        retired = self.command(
            sys.executable, str(TOOL), "--check-reopen-history", "--base", "HEAD",
        )
        self.assertIn("0 entries", retired.stdout)

    def test_history_check_does_not_authorize_uncommitted_reopen(self):
        plateau = self.current_plateau()
        path = self.repo / ls.REOPEN_AUTHORIZATIONS_PATH
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps({
            "schema_version": 1,
            "authorizations": {SYMBOL: {
                "source_commit": plateau, "ledger_commit": plateau,
                "reason": "new mechanism",
            }},
        }), encoding="utf-8")
        history = self.command(
            sys.executable, str(TOOL), "--check-reopen-history", "--base", "HEAD",
        )
        self.assertEqual(history.returncode, 0)
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["state"], "already-integrated/exhausted")

    def test_history_check_rejects_unknown_ancestry(self):
        plateau = self.current_plateau()
        self.authorize_reopen(plateau, plateau)
        result = self.command(
            sys.executable, str(TOOL), "--check-reopen-history", "--base", "missing-ref",
            check=False,
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("missing-ref", result.stderr)

    def test_shared_guard_retains_same_function_lane_ownership(self) -> None:
        text = self.shared_guard()
        self.command("git", "switch", "-q", "-c", "lane/shared-target")
        (self.repo / SOURCE_PATH).write_text(text.replace(
            f"void {SYMBOL}(void) {{", f"void {SYMBOL}(void) {{\n    int changed = 1;"))
        self.commit("Change target candidate")
        self.command("git", "switch", "-q", "campaign/unchain")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["state"], "active")
        self.assertIn("lane/shared-target", report["assignment"]["active_lanes"])

    def test_shared_guard_is_conservatively_one_writer_region(self) -> None:
        text = self.shared_guard()
        self.command("git", "switch", "-q", "-c", "lane/shared-sibling")
        (self.repo / SOURCE_PATH).write_text(text.replace(
            "void sibling(void) {}", "void sibling(void) { int changed = 1; }"))
        self.commit("Change sibling in indivisible guard")
        self.command("git", "switch", "-q", "campaign/unchain")
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["state"], "active")

    def test_other_guard_does_not_claim_shared_guard(self) -> None:
        text = self.shared_guard() + ('\n#ifdef NON_MATCHING\nvoid other(void) {}\n'
                                     '#else\n#pragma GLOBAL_ASM("asm/other.s")\n#endif\n')
        (self.repo / SOURCE_PATH).write_text(text)
        self.commit("Add separate candidate region")
        self.command("git", "switch", "-q", "-c", "lane/separate-guard")
        (self.repo / SOURCE_PATH).write_text(text.replace(
            "void other(void) {}", "void other(void) { int changed = 1; }"))
        self.commit("Change separate guard")
        self.command("git", "switch", "-q", "campaign/unchain")
        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")
        self.assertEqual(report["assignment"]["active_lanes"], [])

    def test_base_only_is_the_only_assignable_state(self) -> None:
        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")

    def test_variant_switch_and_renamed_fallback_stay_assignable(self) -> None:
        generated = "func_overlay_043_F0000100_1880000"
        (self.repo / SOURCE_PATH).write_text(VARIANT_BODY.format(
            symbol=SYMBOL,
            fallback=f"asm/nonmatchings/overlays/o043/{SYMBOL}/{generated}.s",
        ), encoding="utf-8")
        (self.repo / "mk").mkdir()
        (self.repo / "mk/overlays.mk").write_text(
            f"$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/{SYMBOL}.c.o: POSTPROCESS = \\\n"
            f"\t$(OBJCOPY) --redefine-sym {generated}={SYMBOL} $@\n",
            encoding="utf-8",
        )
        self.commit("Seed a variant-switch candidate")
        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")

    def test_schema_cli_reads_worktree_not_committed_authorization(self) -> None:
        plateau = self.current_plateau()
        self.authorize_reopen(plateau, plateau, reason="x" * 240)
        path = self.repo / ls.REOPEN_AUTHORIZATIONS_PATH
        document = json.loads(path.read_text(encoding="utf-8"))
        document["authorizations"][SYMBOL]["reason"] += "x"
        path.write_text(json.dumps(document), encoding="utf-8")
        result = self.command(sys.executable, str(TOOL), "--check-reopen-schema", check=False)
        self.assertEqual(result.returncode, 2)
        self.assertIn("240", result.stderr)
        committed, report = self.status()
        self.assertEqual(committed.returncode, 0, committed.stderr)
        self.assertEqual(report["assignment"]["reason_code"], "authorized-reopen")

        self.commit("Synthetic malformed committed reason")
        document["authorizations"][SYMBOL]["reason"] = "corrected worktree only"
        path.write_text(json.dumps(document), encoding="utf-8")
        result = self.command(sys.executable, str(TOOL), "--check-reopen-schema", check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        committed, report = self.status()
        self.assertNotEqual(committed.returncode, 0)
        self.assertNotEqual(report["assignment"]["state"], "base-only")

    def test_exact_current_pair_authorizes_one_shot_reopen(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertEqual(assignment["source_commit"], plateau_commit)
        self.assertEqual(assignment["ledger_commit"], plateau_commit)

    def test_exact_stale_pair_authorizes_one_shot_remeasurement(self) -> None:
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        shard_commit = self.commit("Add overlay43FilterImage plateau shard")
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        source_commit = self.commit("Plateau overlay43FilterImage reproof")
        self.authorize_reopen(
            source_commit, shard_commit,
            reason="fresh configured maintenance remeasurement",
        )

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertEqual(assignment["ledger_commit"], shard_commit)

    def test_reproof_commit_automatically_exhausts_authorization(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "score: 8/43 words", "score: 9/43 words",
            ),
            encoding="utf-8",
        )
        (self.repo / SHARD_PATH).write_text(
            shard().replace("35/43 words", "36/43 words"), encoding="utf-8",
        )
        refreshed = self.commit(f"Plateau {SYMBOL} authenticated reproof")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["reason_code"], "reopen-authorization-stale")
        self.assertEqual(assignment["source_commit"], refreshed)
        self.assertEqual(assignment["ledger_commit"], refreshed)

    def test_missing_ledger_authorization_is_one_shot_maintenance(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        (self.repo / "docs/matching-triage.md").write_text(
            "| `unrelatedFunction` | current plateau |\n", encoding="utf-8",
        )
        source_commit = self.commit(f"Plateau {SYMBOL} prose maintenance")
        self.authorize_reopen(source_commit, None)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertIsNone(assignment["ledger_commit"])

        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "score: 8/43 words", "score: 9/43 words",
            ),
            encoding="utf-8",
        )
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        self.commit(f"Plateau {SYMBOL} structured reproof")
        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(
            report["assignment"]["state"], "already-integrated/exhausted",
        )

    def test_missing_ledger_authorization_rejects_later_guard_change(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        (self.repo / "docs/matching-triage.md").write_text(
            "| `unrelatedFunction` | current plateau |\n", encoding="utf-8",
        )
        source_commit = self.commit(f"Plateau {SYMBOL} prose maintenance")
        self.authorize_reopen(source_commit, None)
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "void overlay43FilterImage(void) {\n}",
                "void overlay43FilterImage(void) {\n    int refined;\n}",
            ),
            encoding="utf-8",
        )
        later_source_commit = self.commit("Refine guarded candidate evidence")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(
            assignment["reason_code"], "reopen-authorization-stale",
        )
        self.assertEqual(assignment["source_commit"], later_source_commit)

    def test_missing_ledger_authorization_allows_unrelated_tu_edit(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        (self.repo / "docs/matching-triage.md").write_text(
            "| `unrelatedFunction` | current plateau |\n", encoding="utf-8",
        )
        source_commit = self.commit(f"Plateau {SYMBOL} prose maintenance")
        self.authorize_reopen(source_commit, None)
        unrelated = self.repo / "src/main/unrelated.c"
        unrelated.parent.mkdir(parents=True, exist_ok=True)
        unrelated.write_text("void unrelated(void) { }\n", encoding="utf-8")
        self.commit("Update unrelated translation unit")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertEqual(assignment["source_commit"], source_commit)

    def test_missing_ledger_authorization_pins_later_target_evidence(self) -> None:
        (self.repo / "docs/matching-triage.md").write_text(
            "| `unrelatedFunction` | current plateau |\n", encoding="utf-8",
        )
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        plateau_commit = self.commit(f"Plateau {SYMBOL} prose maintenance")
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "void overlay43FilterImage(void) {\n}",
                "void overlay43FilterImage(void) {\n    int refined;\n}",
            ),
            encoding="utf-8",
        )
        evidence_commit = self.commit("Refine guarded candidate evidence")
        self.authorize_reopen(evidence_commit, None)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertEqual(assignment["source_commit"], evidence_commit)
        self.assertIsNone(assignment["ledger_commit"])

    def test_missing_ledger_single_guard_can_pin_latest_file_commit(self) -> None:
        (self.repo / "docs/matching-triage.md").write_text(
            "| `unrelatedFunction` | current plateau |\n", encoding="utf-8",
        )
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True), encoding="utf-8",
        )
        plateau_commit = self.commit(f"Plateau {SYMBOL} prose maintenance")
        (self.repo / SOURCE_PATH).write_text(
            "extern int changed_prototype(void);\n" + candidate(plateau=True),
            encoding="utf-8",
        )
        latest_commit = self.commit("Record generic batch source update")
        self.authorize_reopen(latest_commit, None)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertEqual(assignment["source_commit"], plateau_commit)
        self.assertIsNone(assignment["ledger_commit"])

    def test_current_evidence_repair_can_pin_missing_ledger_reopen(self) -> None:
        revised = candidate(plateau=True).replace(
            "void overlay43FilterImage(void) {\n}",
            "void overlay43FilterImage(void) {\n    int revised;\n}",
        )
        (self.repo / SOURCE_PATH).write_text(revised, encoding="utf-8")
        (self.repo / "docs/matching-triage.md").write_text(
            "| `unrelatedFunction` | current plateau |\n", encoding="utf-8",
        )
        self.commit("Refine guarded candidate evidence")
        evidence_commit = self.commit_current_evidence(revised)
        self.authorize_reopen(evidence_commit, None)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(assignment["state"], "base-only")
        self.assertEqual(assignment["reason_code"], "authorized-reopen")
        self.assertEqual(assignment["source_commit"], evidence_commit)

    def test_null_ledger_pin_cannot_ignore_preexisting_evidence(self) -> None:
        # Unlike the missing-ledger fixtures, retain setUp's exact-symbol
        # legacy row. A null pin must not erase that older evidence.
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        source_commit = self.commit(f"Plateau {SYMBOL} prose maintenance")
        self.authorize_reopen(source_commit, None)
        result, report = self.status()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(report["assignment"]["reason_code"], "stale-structured-evidence")
        self.assertIsNotNone(report["assignment"]["ledger_commit"])

    def commit_current_evidence(self, revised: str) -> str:
        (self.repo / SOURCE_PATH).write_text(
            revised.replace("score: 8/43 words", "score: 9/43 words"),
            encoding="utf-8",
        )
        return self.commit(f"Plateau {SYMBOL} current evidence")

    def test_active_lane_precedes_valid_reopen_authorization(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)
        self.command("git", "switch", "-q", "-c", "lane/o43-reproof")
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "void overlay43FilterImage", "static void overlay43FilterImage",
            ),
            encoding="utf-8",
        )
        self.commit(f"Reproof {SYMBOL} new mechanism")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "active")
        self.assertEqual(assignment["reason_code"], "lane-owned")

    def test_malformed_reopen_authorization_fails_closed(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen("short", plateau_commit)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(
            assignment["reason_code"], "reopen-authorization-invalid",
        )
        self.assertIn("full source_commit", assignment["reason"])

    def test_nonancestor_reopen_authorization_fails_closed(self) -> None:
        plateau_commit = self.current_plateau()
        self.command("git", "switch", "-q", "-c", "authorization-side")
        (self.repo / "side-note.txt").write_text("side\n", encoding="utf-8")
        side_commit = self.commit("Create nonancestor authorization commit")
        self.command("git", "switch", "-q", "campaign/unchain")
        self.authorize_reopen(side_commit, side_commit)

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(
            assignment["reason_code"], "reopen-authorization-invalid",
        )
        self.assertIn("not an ancestor", assignment["reason"])

    def test_call_followed_by_block_is_not_a_second_definition(self) -> None:
        caller = self.repo / "src/main/caller.c"
        caller.parent.mkdir(parents=True)
        caller.write_text(
            f"""extern void {SYMBOL}(void);
extern int accepts_value(int value);

void caller(void) {{
    if (accepts_value(({SYMBOL}(), 1))) {{
    }}
}}
""",
            encoding="utf-8",
        )
        self.commit("Add caller with a following control block")

        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")
        self.assertEqual(report["assignment"]["source_path"], SOURCE_PATH.as_posix())

    def test_unintegrated_source_blob_is_active(self) -> None:
        self.command("git", "switch", "-q", "-c", "lane/o43-active")
        (self.repo / SOURCE_PATH).write_text(
            candidate().replace("void overlay43FilterImage", "static void overlay43FilterImage"),
            encoding="utf-8",
        )
        self.commit("Work overlay43FilterImage allocator")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(report["assignment"]["state"], "active")
        self.assertEqual(report["assignment"]["active_lanes"], ["lane/o43-active"])

    def test_unrelated_candidate_in_same_tu_does_not_reserve_target(self) -> None:
        second = """
#ifdef NON_MATCHING
void unrelatedFunction(void) {
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o043/unrelatedFunction.s")
#endif
"""
        (self.repo / SOURCE_PATH).write_text(candidate() + second, encoding="utf-8")
        self.commit("Add mixed overlay 43 candidates")
        self.command("git", "switch", "-q", "-c", "lane/o43-unrelated")
        (self.repo / SOURCE_PATH).write_text(
            (candidate() + second).replace(
                "void unrelatedFunction(void) {\n}",
                "static void unrelatedFunction(void) {\n    int value = 1;\n}",
            ),
            encoding="utf-8",
        )
        self.commit("Work unrelatedFunction allocator")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")

    def test_unintegrated_target_handoff_is_active(self) -> None:
        self.command("git", "switch", "-q", "-c", "lane/o43-handoff")
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        self.commit("Plateau overlay43FilterImage allocator")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(report["assignment"]["state"], "active")
        self.assertEqual(report["assignment"]["active_lanes"], ["lane/o43-handoff"])

    def test_unintegrated_target_shard_without_source_edit_is_active(self) -> None:
        self.command("git", "switch", "-q", "-c", "lane/o43-shard")
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        self.commit("Plateau overlay43FilterImage shard")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(report["assignment"]["state"], "active")
        self.assertEqual(report["assignment"]["active_lanes"], ["lane/o43-shard"])

    def test_unintegrated_legacy_block_metric_edit_is_active(self) -> None:
        ledger = self.repo / "docs/matching-triage.md"
        ledger.write_text(shard(), encoding="utf-8")
        self.commit("Record overlay43FilterImage ledger block")
        self.command("git", "switch", "-q", "-c", "lane/o43-ledger-metric")
        ledger.write_text(
            shard().replace("35/43 words", "36/43 words"), encoding="utf-8",
        )
        self.commit("Update overlay43FilterImage plateau metric")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(report["assignment"]["state"], "active")
        self.assertEqual(
            report["assignment"]["active_lanes"], ["lane/o43-ledger-metric"],
        )

    def test_unrelated_symbol_shard_does_not_reserve_target(self) -> None:
        self.command("git", "switch", "-q", "-c", "lane/other-shard")
        directory = self.repo / SHARD_PATH.parent
        directory.mkdir()
        (directory / "unrelatedFunction.md").write_text(
            shard(source="src/main/other.c", symbol="unrelatedFunction"),
            encoding="utf-8",
        )
        self.commit("Plateau unrelatedFunction")
        self.command("git", "switch", "-q", "campaign/unchain")

        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")

    def test_stale_pre_cleanup_triage_row_fails_closed(self) -> None:
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        source_commit = self.commit("Plateau overlay43FilterImage temp FIFO reproof")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertIn("predates", assignment["reason"])

    def test_reconciled_plateau_is_already_exhausted(self) -> None:
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        source_commit = self.commit("Plateau overlay43FilterImage temp FIFO reproof")
        (self.repo / "docs/matching-triage.md").write_text(
            f"| `{SYMBOL}` | bounded plateau; route exhausted |\n", encoding="utf-8",
        )
        ledger_commit = self.commit("Reconcile overlay43FilterImage plateau evidence")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertEqual(assignment["ledger_commit"], ledger_commit)

    def test_legacy_generated_block_metric_refresh_is_current(self) -> None:
        source = candidate(plateau=True)
        ledger = self.repo / "docs/matching-triage.md"
        (self.repo / SOURCE_PATH).write_text(source, encoding="utf-8")
        ledger.write_text(shard(), encoding="utf-8")
        self.commit("Plateau overlay43FilterImage initial")
        (self.repo / SOURCE_PATH).write_text(
            source.replace("8/43 words", "9/43 words"), encoding="utf-8",
        )
        ledger.write_text(
            shard().replace("35/43 words", "36/43 words"), encoding="utf-8",
        )
        refreshed = self.commit("Plateau overlay43FilterImage refresh")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], refreshed)
        self.assertEqual(assignment["ledger_commit"], refreshed)

    def test_symbol_shard_reconciles_plateau_without_shared_ledger_edit(self) -> None:
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        plateau_commit = self.commit("Plateau overlay43FilterImage allocator")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], plateau_commit)
        self.assertEqual(assignment["ledger_commit"], plateau_commit)

    def test_base_only_shard_repair_does_not_activate_historical_lane(self) -> None:
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        source_commit = self.commit("Plateau overlay43FilterImage allocator")
        self.command("git", "switch", "-q", "-c", "lane/historical")
        (self.repo / "lane-note.txt").write_text("unrelated\n", encoding="utf-8")
        self.commit("Record unrelated lane note")
        self.command("git", "switch", "-q", "campaign/unchain")
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        ledger_commit = self.commit("Reconcile overlay43FilterImage evidence")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertEqual(assignment["ledger_commit"], ledger_commit)
        self.assertEqual(assignment["active_lanes"], [])

    def test_unrelated_mixed_tu_edit_does_not_own_repaired_target(self) -> None:
        source = self.repo / SOURCE_PATH
        source.write_text(candidate() + "\nvoid neighbor(void) { }\n", encoding="utf-8")
        self.commit("Add neighboring function")
        self.command("git", "switch", "-q", "-c", "lane/neighbor")
        source.write_text(
            candidate() + "\nvoid neighbor(void) { int value = 1; (void)value; }\n",
            encoding="utf-8",
        )
        self.commit("Plateau neighboring function")
        self.command("git", "switch", "-q", "campaign/unchain")
        source.write_text(
            candidate().replace("{\n}", "{\n    /* repaired */\n}")
            + "\nvoid neighbor(void) { }\n",
            encoding="utf-8",
        )
        self.commit("Repair overlay43FilterImage source")

        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")
        self.assertEqual(report["assignment"]["active_lanes"], [])

    def test_reviewed_superseded_guard_does_not_reserve_target(self) -> None:
        source = self.repo / SOURCE_PATH
        source.write_text(
            candidate().replace("{\n}", "{\n    /* repaired */\n}"),
            encoding="utf-8",
        )
        decision = self.commit("Reopen clean source")
        self.command("git", "switch", "-q", "-c", "lane/old-plateau")
        source.write_text(
            candidate().replace("{\n}", "{\n    /* old plateau */\n}"),
            encoding="utf-8",
        )
        lane_head = self.commit("Plateau overlay43FilterImage old form")
        self.command("git", "switch", "-q", "campaign/unchain")
        dispositions = self.repo / ls.DISPOSITIONS_PATH
        dispositions.parent.mkdir(parents=True, exist_ok=True)
        dispositions.write_text(json.dumps({
            "schema_version": 1,
            "claims": {
                lane_head: {
                    "symbol": SYMBOL,
                    "state": "superseded",
                    "decision_commit": decision,
                    "reason": "Canonical replaced the historical guard.",
                },
            },
        }), encoding="utf-8")
        self.commit("Record reviewed guard disposition")

        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")
        self.assertEqual(report["assignment"]["active_lanes"], [])

    def test_malformed_symbol_shard_fails_closed(self) -> None:
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text("# malformed\n", encoding="utf-8")
        self.commit("Add malformed overlay43FilterImage shard")

        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(report["assignment"]["state"], "stale-ledger")
        # Assert the PROPERTY, not the wording. This test previously pinned the
        # exact phrase "malformed or foreign"; the message was later improved to
        # name the specific marker that is missing, and the assertion went red
        # while the fail-closed behaviour it exists to protect was intact. What
        # must hold is that the refusal names the symbol and says what is wrong
        # with the shard, so a reader can act on it.
        reason = report["assignment"]["reason"]
        self.assertIn("overlay43FilterImage", reason)
        self.assertRegex(reason, r"malformed|foreign|missing|marker|unparsed")

    def test_malformed_target_block_in_legacy_ledger_fails_closed(self) -> None:
        (self.repo / "docs/matching-triage.md").write_text(
            f"<!-- plateau-handoff:{SYMBOL}:start -->\n"
            f"### `{SYMBOL}` plateau handoff\n",
            encoding="utf-8",
        )
        ledger_commit = self.commit("Damage overlay43FilterImage handoff block")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(assignment["ledger_commit"], ledger_commit)
        self.assertIn("malformed target-specific", assignment["reason"])

    def test_symbol_shard_source_mismatch_fails_closed(self) -> None:
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(
            shard(source="src/main/wrong.c"), encoding="utf-8",
        )
        shard_commit = self.commit("Add wrong-source overlay43FilterImage shard")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(assignment["ledger_commit"], shard_commit)
        self.assertIn("expected", assignment["reason"])

    def test_symbol_shard_older_than_source_plateau_is_stale(self) -> None:
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        shard_commit = self.commit("Add overlay43FilterImage plateau shard")
        (self.repo / SOURCE_PATH).write_text(candidate(plateau=True), encoding="utf-8")
        source_commit = self.commit("Plateau overlay43FilterImage reproof")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertEqual(assignment["ledger_commit"], shard_commit)
        self.assertIn("predates", assignment["reason"])

    def test_target_named_plateau_commit_is_evidence_without_marker(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            candidate().replace("void overlay43FilterImage", "static void overlay43FilterImage"),
            encoding="utf-8",
        )
        (self.repo / "docs/matching-triage.md").write_text(
            f"| `{SYMBOL}` | bounded plateau; route exhausted |\n", encoding="utf-8",
        )
        plateau_commit = self.commit("Plateau overlay43FilterImage allocator")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], plateau_commit)
        self.assertEqual(assignment["ledger_commit"], plateau_commit)

    def test_single_guard_path_plateau_without_symbol_fails_closed(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            "/* Plateau: bounded allocator route. */\n" + candidate(),
            encoding="utf-8",
        )
        source_commit = self.commit("Record overlay 043 Phase A plateaus")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertIn("predates", assignment["reason"])

    def test_single_guard_path_plateau_reconciles_by_ledger_row_edit(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            "/* Plateau: bounded allocator route. */\n" + candidate(),
            encoding="utf-8",
        )
        source_commit = self.commit("Record overlay 043 Phase A plateaus")
        (self.repo / "docs/matching-triage.md").write_text(
            f"| `{SYMBOL}` | bounded plateau; route exhausted |\n",
            encoding="utf-8",
        )
        ledger_commit = self.commit("Reconcile legacy overlay 043 evidence")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertEqual(assignment["ledger_commit"], ledger_commit)

    def test_mixed_tu_plateau_uses_exact_row_and_source_commit(self) -> None:
        second = """\n#ifdef NON_MATCHING
void unrelatedFunction(void) {
}
#else
#pragma GLOBAL_ASM(\"asm/nonmatchings/overlays/o043/unrelatedFunction.s\")
#endif
"""
        (self.repo / SOURCE_PATH).write_text(
            candidate() + second, encoding="utf-8",
        )
        (self.repo / "docs/matching-triage.md").write_text(
            f"| `{SYMBOL}` | bounded plateau; route exhausted |\n",
            encoding="utf-8",
        )
        plateau_commit = self.commit("Record overlay 43 allocation plateau")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(
            assignment["state"], "already-integrated/exhausted",
        )
        self.assertEqual(assignment["source_commit"], plateau_commit)
        self.assertEqual(assignment["ledger_commit"], plateau_commit)

    def test_mixed_tu_plateau_uses_exact_symbol_shard(self) -> None:
        second = """\n#ifdef NON_MATCHING
void unrelatedFunction(void) {
}
#else
#pragma GLOBAL_ASM(\"asm/nonmatchings/overlays/o043/unrelatedFunction.s\")
#endif
"""
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True) + second, encoding="utf-8",
        )
        (self.repo / SHARD_PATH.parent).mkdir()
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        plateau_commit = self.commit("Record overlay 43 allocation plateau")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], plateau_commit)
        self.assertEqual(assignment["ledger_commit"], plateau_commit)

    def test_non_plateau_target_body_change_advances_source_pin(self) -> None:
        plateau_commit = self.current_plateau()
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "void overlay43FilterImage(void) {\n}",
                "void overlay43FilterImage(void) {\n    /* updated body */\n}",
            ),
            encoding="utf-8",
        )
        source_commit = self.commit("Adjust local candidate expression")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(assignment["source_commit"], source_commit)
        self.assertNotEqual(assignment["source_commit"], plateau_commit)

    def test_shared_tu_sibling_change_does_not_advance_target_source_pin(self) -> None:
        sibling = '''
#ifdef NON_MATCHING
void unrelatedFunction(void) {
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o043/unrelatedFunction.s")
#endif
'''
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True) + sibling, encoding="utf-8",
        )
        (self.repo / SHARD_PATH.parent).mkdir(parents=True, exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard(), encoding="utf-8")
        target_commit = self.commit(f"Plateau {SYMBOL} allocator")
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True) + sibling.replace(
                "void unrelatedFunction(void) {\n}",
                "void unrelatedFunction(void) {\n    /* sibling edit */\n}",
            ),
            encoding="utf-8",
        )
        self.commit("Refactor neighboring function")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "already-integrated/exhausted")
        self.assertEqual(assignment["source_commit"], target_commit)

    def test_mixed_tu_plateau_row_without_source_change_is_stale(self) -> None:
        second = """\n#ifdef NON_MATCHING
void unrelatedFunction(void) {
}
#else
#pragma GLOBAL_ASM(\"asm/nonmatchings/overlays/o043/unrelatedFunction.s\")
#endif
"""
        (self.repo / SOURCE_PATH).write_text(
            candidate() + second, encoding="utf-8",
        )
        self.commit("Create mixed overlay 43 translation unit")
        (self.repo / "docs/matching-triage.md").write_text(
            f"| `{SYMBOL}` | bounded plateau; route exhausted |\n",
            encoding="utf-8",
        )
        self.commit("Record overlay 43 allocation plateau")

        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")

    def test_definition_without_fallback_is_already_integrated(self) -> None:
        (self.repo / SOURCE_PATH).write_text(
            f"void {SYMBOL}(void) {{\n}}\n", encoding="utf-8",
        )
        self.commit("Match overlay43FilterImage")

        result, report = self.status()
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(
            report["assignment"]["state"], "already-integrated/exhausted",
        )


class AssignmentCacheTests(unittest.TestCase):
    """The persistent cache must be invisible except in wall-clock time.

    Every verdict it serves has to equal the uncached classifier's verdict on
    the same tree, lane ownership must never come out of it, and a moved base
    must miss rather than serve an older base's pins.
    """

    # Borrow the git fixture without re-running the fixture class's tests.
    command = LaneStatusAssignmentTests.command
    commit = LaneStatusAssignmentTests.commit
    current_plateau = LaneStatusAssignmentTests.current_plateau
    authorize_reopen = LaneStatusAssignmentTests.authorize_reopen

    def setUp(self) -> None:
        LaneStatusAssignmentTests.setUp(self)

    def _clear(self) -> None:
        # Every in-process memo, so each classification starts as a fresh run.
        for value in vars(ls).values():
            if callable(getattr(value, "cache_clear", None)):
                value.cache_clear()

    def classify(self, cache_dir: Path | None) -> tuple[ls.Assignment, "ls.AssignmentCache | None"]:
        self._clear()
        previous = Path.cwd()
        os.chdir(self.repo)
        try:
            cache = (ls.AssignmentCache("campaign/unchain", cache_dir)
                     if cache_dir is not None else None)
            context = ls.AssignmentContext.build(
                "campaign/unchain", [SYMBOL], cache=cache,
            )
            verdict = context.classify("campaign/unchain", SYMBOL)
            context.save()
            return verdict, cache
        finally:
            os.chdir(previous)

    def cache_dir(self) -> Path:
        return Path(self.temporary.name + "-cache")

    def tearDown(self) -> None:
        import shutil
        shutil.rmtree(self.cache_dir(), ignore_errors=True)
        LaneStatusAssignmentTests.tearDown(self)

    def singular_report(self, *options: str):
        result = self.command(
            sys.executable, str(TOOL), "--base", "campaign/unchain",
            "--symbol", SYMBOL, "--json", *options, check=False,
        )
        return result, json.loads(result.stdout)

    def test_singular_cached_and_uncached_full_reports_match(self) -> None:
        plateau = self.current_plateau()
        self.authorize_reopen(plateau, plateau)
        cache_dir = self.repo / ls.ASSIGNMENT_CACHE_DIR
        uncached, expected = self.singular_report("--no-cache")
        self.assertEqual(uncached.returncode, 0, uncached.stderr)
        self.assertFalse(cache_dir.exists(), "--no-cache must not write a cache")
        cold, report = self.singular_report()
        self.assertEqual(report, expected)
        self.assertIn("0 hit(s), 1 miss(es)", cold.stderr)
        warm, report = self.singular_report()
        self.assertEqual(report, expected)
        self.assertIn("1 hit(s), 0 miss(es)", warm.stderr)
        self.assertEqual(set(report), {"base", "assignment", "lanes"})
        cache_files = {p: p.read_bytes() for p in cache_dir.glob("*.json")}
        bypassed, report = self.singular_report("--no-cache")
        self.assertEqual(report, expected)
        self.assertNotIn("assignment cache", bypassed.stderr)
        self.assertEqual(cache_files,
                         {p: p.read_bytes() for p in cache_dir.glob("*.json")})

    def test_singular_warm_cache_observes_new_owner_and_claim(self) -> None:
        first, report = self.singular_report()
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertEqual(report["assignment"]["state"], "base-only")
        self.command("git", "switch", "-q", "-c", "lane/o43-cli-cache")
        (self.repo / SOURCE_PATH).write_text(
            candidate().replace("void overlay43FilterImage",
                                "static void overlay43FilterImage"),
            encoding="utf-8",
        )
        self.commit(f"Match {SYMBOL}")
        self.command("git", "switch", "-q", "campaign/unchain")
        warm, report = self.singular_report()
        uncached, expected = self.singular_report("--no-cache")
        self.assertEqual(warm.returncode, 1)
        self.assertEqual(uncached.returncode, 1)
        self.assertIn("1 hit(s), 0 miss(es)", warm.stderr)
        self.assertEqual(report, expected)
        self.assertEqual(report["assignment"]["state"], "active")
        self.assertTrue(report["lanes"], "singular output must retain claims")
        self.assertEqual(report["lanes"][0]["claims"][0]["symbol"], SYMBOL)

    def test_warm_verdict_equals_cold_and_skips_history(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)
        uncached, _ = self.classify(None)
        cold, cache = self.classify(self.cache_dir())
        self.assertEqual(cold, uncached)
        self.assertEqual((cache.hits, cache.misses), (0, 1))
        self.assertTrue(cache.path.is_file())

        boom = mock.Mock(side_effect=AssertionError("history walked on a warm hit"))
        with mock.patch.object(ls, "target_history_commit", boom), \
                mock.patch.object(ls, "reopen_authorizations", boom), \
                mock.patch.object(ls, "_settled_status", boom):
            warm, cache = self.classify(self.cache_dir())
        self.assertEqual(warm, uncached)
        self.assertEqual(warm.reason_code, "authorized-reopen")
        self.assertEqual((cache.hits, cache.misses), (1, 0))

    def test_cache_never_serves_lane_ownership(self) -> None:
        self.current_plateau()
        warm, _ = self.classify(self.cache_dir())
        self.assertEqual(warm.state, "already-integrated/exhausted")
        self.command("git", "switch", "-q", "-c", "lane/o43-cache")
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace(
                "void overlay43FilterImage", "static void overlay43FilterImage",
            ),
            encoding="utf-8",
        )
        self.commit(f"Reproof {SYMBOL} new mechanism")
        self.command("git", "switch", "-q", "campaign/unchain")

        uncached, _ = self.classify(None)
        cached, cache = self.classify(self.cache_dir())
        self.assertEqual(cache.hits, 1, "unchanged evidence must hit")
        self.assertEqual(cached.state, "active")
        self.assertEqual(cached, uncached)

    def test_a_moved_base_misses_and_matches_the_uncached_verdict(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)
        first, _ = self.classify(self.cache_dir())
        self.assertEqual(first.state, "base-only")
        # A re-proof moves the shard and the source: the pin goes stale.
        (self.repo / SOURCE_PATH).write_text(
            candidate(plateau=True).replace("score: 8/43", "score: 9/43"),
            encoding="utf-8",
        )
        (self.repo / SHARD_PATH).write_text(
            shard().replace("35/43", "34/43"), encoding="utf-8",
        )
        self.commit(f"Plateau {SYMBOL} reproof")
        uncached, _ = self.classify(None)
        cached, cache = self.classify(self.cache_dir())
        self.assertEqual((cache.hits, cache.misses), (0, 1))
        self.assertEqual(cached, uncached)
        self.assertNotEqual(cached.state, "base-only")
        self.assertEqual(
            sorted(p.name for p in self.cache_dir().glob("*.json")),
            [ls.ASSIGNMENT_CACHE_FILE],
        )

    def test_a_merge_that_leaves_the_evidence_alone_hits(self) -> None:
        # The case the base-commit key got wrong: an integration merge moves
        # the base without touching this symbol's source, shard, ledger or
        # authorization, and must be served warm with the same verdict.
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)
        first, cache = self.classify(self.cache_dir())
        self.assertEqual((cache.hits, cache.misses), (0, 1))
        before = cache.base_commit
        self.command("git", "switch", "-q", "-c", "lane/unrelated")
        (self.repo / "src" / "unrelated.c").write_text(
            "void unrelated(void) {\n}\n", encoding="utf-8",
        )
        self.commit("Match unrelated")
        self.command("git", "switch", "-q", "campaign/unchain")
        self.command(
            "git", "merge", "-q", "--no-ff", "-m",
            "Merge lane/unrelated into campaign/unchain", "lane/unrelated",
        )
        self.command("git", "branch", "-q", "-D", "lane/unrelated")
        uncached, _ = self.classify(None)
        boom = mock.Mock(side_effect=AssertionError("history walked on a warm hit"))
        with mock.patch.object(ls, "target_history_commit", boom), \
                mock.patch.object(ls, "reopen_authorizations", boom), \
                mock.patch.object(ls, "_settled_status", boom):
            warm, cache = self.classify(self.cache_dir())
        self.assertNotEqual(cache.base_commit, before)
        self.assertEqual((cache.hits, cache.misses), (1, 0))
        self.assertEqual(warm, uncached)
        self.assertEqual(warm, first)
        self.assertEqual(warm.reason_code, "authorized-reopen")

    def test_a_pin_outside_the_authorization_anchor_keys_on_the_base(self) -> None:
        plateau_commit = self.current_plateau()
        self.authorize_reopen(plateau_commit, plateau_commit)
        _, cache = self.classify(self.cache_dir())
        self.assertIsNone(cache.base_term)
        probe = ls.AssignmentCache.__new__(ls.AssignmentCache)
        probe.__dict__.update(cache.__dict__)
        with mock.patch.object(
            ls, "parse_reopen_authorizations",
            return_value={SYMBOL: {
                "source_commit": "a" * 40, "ledger_commit": None,
                "reason": "x",
            }},
        ):
            previous = Path.cwd()
            os.chdir(self.repo)
            try:
                self.assertEqual(
                    probe._authorization_base_term(), cache.base_commit,
                )
            finally:
                os.chdir(previous)

    def test_key_covers_code_files_anchors_and_aliases(self) -> None:
        self.current_plateau()
        previous = Path.cwd()
        os.chdir(self.repo)
        self.addCleanup(os.chdir, previous)
        cache = ls.AssignmentCache("campaign/unchain", self.cache_dir())
        path = SOURCE_PATH.as_posix()
        base_key = cache.key(SYMBOL, path)
        self.assertEqual(base_key, cache.key(SYMBOL, path))
        probe = ls.AssignmentCache.__new__(ls.AssignmentCache)
        probe.__dict__.update(cache.__dict__, code="0" * 64)
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        probe.__dict__.update(cache.__dict__, base_term="e" * 40)
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        probe.__dict__.update(
            cache.__dict__, aliases={(path, SYMBOL): frozenset({"func_x"})})
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        probe.__dict__.update(
            cache.__dict__, authorization_rows={SYMBOL: {"reason": "x"}})
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        probe.__dict__.update(cache.__dict__, _validity="invalid: x")
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        # Another symbol's authorization row is not this symbol's evidence.
        probe.__dict__.update(
            cache.__dict__, authorization_rows={"otherSymbol": {"reason": "x"}},
            blobs=dict(cache.blobs, **{ls.REOPEN_AUTHORIZATIONS_PATH: "f" * 40}))
        self.assertEqual(base_key, probe.key(SYMBOL, path))
        names = (SHARD_PATH.as_posix(), ls.LEGACY_TRIAGE_PATH, path)
        for name in names:
            with self.subTest(blob=name):
                probe.__dict__.update(
                    cache.__dict__, blobs=dict(cache.blobs, **{name: "f" * 40}))
                self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        for name in (SHARD_PATH.as_posix(), path):
            with self.subTest(anchor=name):
                probe.__dict__.update(
                    cache.__dict__,
                    anchors=dict(cache.anchors, **{name: "d" * 40}))
                self.assertNotEqual(base_key, probe.key(SYMBOL, path))

    def test_a_corrupt_cache_file_is_ignored(self) -> None:
        self.current_plateau()
        _, cache = self.classify(self.cache_dir())
        cache.path.write_text("{not json", encoding="utf-8")
        uncached, _ = self.classify(None)
        again, cache = self.classify(self.cache_dir())
        self.assertEqual((cache.hits, cache.misses), (0, 1))
        self.assertEqual(again, uncached)



class IndirectSourceTests(unittest.TestCase):
    command = LaneStatusAssignmentTests.command
    commit = LaneStatusAssignmentTests.commit
    status = LaneStatusAssignmentTests.status
    authorize_reopen = LaneStatusAssignmentTests.authorize_reopen
    _clear = AssignmentCacheTests._clear
    classify = AssignmentCacheTests.classify
    cache_dir = AssignmentCacheTests.cache_dir
    tearDown = AssignmentCacheTests.tearDown

    def setUp(self):
        LaneStatusAssignmentTests.setUp(self)
        self.inc = SOURCE_PATH.with_suffix('.inc')
        self.wrapper = ('#define LOOP_INIT count = 1\n#ifdef NON_MATCHING\n'
                        f'#include "{self.inc.name}"\n#else\n'
                        f'#pragma GLOBAL_ASM("asm/{SYMBOL}.s")\n#endif\n')
        self.body = ('#ifndef LOAD_HEADER\n'
                     f'#define LOAD_HEADER void {SYMBOL}(void)\n'
                     '#endif\nLOAD_HEADER {\n    int count;\n    LOOP_INIT;\n}\n')
        (self.repo / SOURCE_PATH).write_text(self.wrapper)
        (self.repo / self.inc).write_text(self.body)
        self.source_commit = self.commit('Introduce included candidate')

    def test_indirect_owner_is_wrapper_cached_and_uncached(self):
        uncached, _ = self.classify(None)
        cold, _ = self.classify(self.cache_dir())
        warm, cache = self.classify(self.cache_dir())
        self.assertEqual(uncached.state, 'base-only')
        self.assertEqual(uncached.source_path, SOURCE_PATH.as_posix())
        self.assertEqual(cold, uncached)
        self.assertEqual(warm, uncached)
        self.assertEqual(cache.hits, 1)
        result, report = self.status()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(report['assignment']['state'], 'base-only')

    def test_warm_cache_does_not_hide_new_include_only_lane_owner(self):
        self.classify(self.cache_dir())
        self.command('git', 'checkout', '-qb', 'lane/include-owner')
        (self.repo / self.inc).write_text(self.body.replace('int count;', 'long count;'))
        self.commit('Change included body only')
        self.command('git', 'checkout', '-q', 'campaign/unchain')
        verdict, cache = self.classify(self.cache_dir())
        self.assertEqual(cache.hits, 1)
        self.assertEqual(verdict.state, 'active')
        self.assertIn('lane/include-owner', verdict.active_lanes)

    def test_include_change_on_base_invalidates_cache_and_source_pin(self):
        (self.repo / self.inc).write_text(self.body + '\n/* source plateau */\n')
        plateau = self.commit('Plateau included body')
        before, _ = self.classify(self.cache_dir())
        self.assertEqual(before.source_commit, plateau)
        (self.repo / self.inc).write_text(self.body + '\n/* revised source plateau */\n')
        latest = self.commit('Revise included body')
        after, cache = self.classify(self.cache_dir())
        self.assertEqual(cache.misses, 1)
        self.assertEqual(after.source_commit, latest)
        self.assertNotEqual(after.state, 'base-only')

    def test_include_commit_consumes_reopen_authorization(self):
        (self.repo / self.inc).write_text(self.body + '\n/* recorded plateau */\n')
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        plateau = self.commit('Plateau included candidate')
        self.authorize_reopen(plateau, plateau)
        before, _ = self.classify(self.cache_dir())
        self.assertEqual(before.state, 'base-only')
        (self.repo / self.inc).write_text(self.body + '\n/* next source attempt */\n')
        self.commit('Record included candidate attempt')
        after, _ = self.classify(self.cache_dir())
        self.assertNotEqual(after.state, 'base-only')

    def test_base_only_include_edit_does_not_claim_unchanged_old_lane(self):
        self.command('git', 'branch', 'lane/unchanged-include')
        (self.repo / self.inc).write_text(self.body + '\n/* reviewed source */\n')
        self.commit('Update included candidate')
        verdict, _ = self.classify(None)
        self.assertEqual(verdict.state, 'base-only')

    def test_invalid_indirection_fails_closed(self):
        cases = [
            (self.wrapper, None),
            ('#define LOAD_HEADER other\n' + self.wrapper, self.body),
            (self.wrapper, self.body + '#undef LOAD_HEADER\n'),
            ('#if 0\n' + self.wrapper + '#endif\n', self.body),
            (self.wrapper, '#if 0\n' + self.body + '#endif\n'),
            (self.wrapper, self.body + '#include "nested.inc"\n'),
            (self.wrapper, '#include CONFIG_HEADER\n' + self.body),
            (f'#define {SYMBOL} alias\n' + self.wrapper, self.body),
            (self.wrapper, '#define void int\n' + self.body),
            ('/*\n' + self.wrapper + '*/\n', self.body),
            (self.wrapper, '/*\n' + self.body + '*/\n'),
            (self.wrapper.replace(f'{SYMBOL}.s', 'another.s'), self.body),
        ]
        for wrapper, body in cases:
            with self.subTest(wrapper=wrapper, body=body):
                (self.repo / SOURCE_PATH).write_text(wrapper)
                if body is None:
                    (self.repo / self.inc).unlink(missing_ok=True)
                else:
                    (self.repo / self.inc).write_text(body)
                self.commit('Invalid included candidate')
                verdict, _ = self.classify(None)
                self.assertNotEqual(verdict.state, 'base-only')
                self.assertEqual(verdict.reason_code, 'source-identity')

    def test_duplicate_owner_or_direct_definition_is_ambiguous(self):
        other = self.repo / SOURCE_PATH.parent / 'other.c'
        for text in (self.wrapper, candidate()):
            with self.subTest(text=text):
                other.write_text(text)
                self.commit('Add duplicate owner')
                verdict, _ = self.classify(None)
                self.assertEqual(verdict.state, 'stale-ledger')
                self.assertIn('ambiguous', verdict.reason)

    def test_build_macro_override_refuses_cached_identity(self):
        self.classify(self.cache_dir())
        (self.repo / 'Makefile').write_text('CFLAGS += -DLOAD_HEADER=alternate\n')
        self.commit('Override included signature')
        verdict, _ = self.classify(self.cache_dir())
        self.assertEqual(verdict.reason_code, 'source-identity')


class RenamedIncludedSourceTests(unittest.TestCase):
    command = LaneStatusAssignmentTests.command
    commit = LaneStatusAssignmentTests.commit
    status = LaneStatusAssignmentTests.status
    authorize_reopen = LaneStatusAssignmentTests.authorize_reopen
    _clear = AssignmentCacheTests._clear
    classify = AssignmentCacheTests.classify
    cache_dir = AssignmentCacheTests.cache_dir
    tearDown = AssignmentCacheTests.tearDown

    def setUp(self):
        LaneStatusAssignmentTests.setUp(self)
        self.dep = Path('src/overlays/o069/shared.c')
        (self.repo / self.dep.parent).mkdir(parents=True)
        self.original = 'overlay69DrawSortedGeometry'
        self.generated = 'func_overlay_043_F00001A4_18801A4'
        self.dep_generated = 'func_overlay_069_F0000170_18C0170'
        self.wrapper = (f'#define {self.original} {SYMBOL}\n'
                        '#define overlay69SubmitDynamicReloc overlay43SubmitDynamicReloc\n'
                        '#ifdef NON_MATCHING\n'
                        f'#include "{self.dep}"\n#else\n'
                        f'#pragma GLOBAL_ASM("asm/{self.generated}.s")\n#endif\n')
        self.body = ('#include "PR/ultratypes.h"\n'
                     'extern void overlay69SubmitDynamicReloc(void);\n'
                     '#ifdef NON_MATCHING\n'
                     f'void {self.original}(void) {{\n'
                     '    overlay69SubmitDynamicReloc();\n}\n#else\n'
                     f'#pragma GLOBAL_ASM("asm/{self.dep_generated}.s")\n#endif\n')
        (self.repo / SOURCE_PATH).write_text(self.wrapper)
        (self.repo / self.dep).write_text(self.body)
        (self.repo / 'include/PR').mkdir(parents=True)
        (self.repo / 'include/PR/ultratypes.h').write_text('typedef int s32;\n')
        (self.repo / 'docs/matching-triage.md').write_text('')
        (self.repo / 'mk').mkdir()
        rules = ''
        for path, generated, symbol in [(SOURCE_PATH,self.generated,SYMBOL),
                                      (self.dep,self.dep_generated,self.original)]:
            rules += (f'$(BUILD_DIR)/$(SRC_DIR)/{str(path)[4:]}.o: POSTPROCESS = '
                      f'$(OBJCOPY) --redefine-sym {generated}={symbol} $@\n')
        (self.repo / 'mk/overlays.mk').write_text(rules)
        self.atlas = {'modules': []}
        for overlay, path, offset, rom in [(43,SOURCE_PATH,0x1A4,0x1880000),
                                          (69,self.dep,0x170,0x18C0000)]:
            self.atlas['modules'].append({'overlay': overlay,
                'sections': {'text': {'start':hex(rom),'size':'0x800'}},
                'text_ownership':[{'offset':hex(offset),'end_offset':hex(offset+0x59C),
                                  'size':'0x59C','type':'c','matched':True,
                                  'nonmatching':True,'source':str(path)[4:-2]}]})
        (self.repo / 'config').mkdir(exist_ok=True)
        self.atlas_path = self.repo / 'config/overlays.us.json'
        self.atlas_path.write_text(json.dumps(self.atlas))
        self.source_commit = self.commit('Introduce guarded renamed C include')

    def test_scalar_batch_and_cold_warm_cache_identify_wrapper(self):
        uncached,_ = self.classify(None)
        cold,_ = self.classify(self.cache_dir())
        warm,cache = self.classify(self.cache_dir())
        self.assertEqual(uncached.state,'base-only')
        self.assertEqual(uncached.source_path,SOURCE_PATH.as_posix())
        self.assertEqual(uncached,cold)
        self.assertEqual(cold,warm)
        self.assertEqual(cache.hits,1)
        result,report = self.status()
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(report['assignment']['source_path'],SOURCE_PATH.as_posix())

    def test_dependency_only_lane_work_is_active_with_warm_cache(self):
        self.classify(self.cache_dir())
        self.command('git','checkout','-qb','lane/dependency-owner')
        (self.repo / self.dep).write_text(self.body.replace('    overlay69Submit', '    /* owned change */ overlay69Submit'))
        self.commit('Match overlay69DrawSortedGeometry')
        self.command('git','checkout','-q','campaign/unchain')
        verdict,cache=self.classify(self.cache_dir())
        self.assertEqual(cache.hits,1)
        self.assertEqual(verdict.state,'active')
        self.assertIn('lane/dependency-owner',verdict.active_lanes)

    def test_missing_dependency_and_changed_wrapper_or_removed_guard_are_closed(self):
        self.classify(self.cache_dir())
        self.command('git','checkout','-qb','lane/wrapper-owner')
        (self.repo / SOURCE_PATH).write_text(self.wrapper.replace('overlay43SubmitDynamicReloc','otherSubmitReloc'))
        self.commit('Change owned alias only')
        self.command('git','checkout','-q','campaign/unchain')
        verdict,_=self.classify(self.cache_dir())
        self.assertEqual(verdict.state,'active')
        self.command('git','branch','-D','lane/wrapper-owner')
        self.command('git','checkout','-qb','lane/guard-owner')
        (self.repo / self.dep).write_text(self.body.replace('#ifdef NON_MATCHING','#if 0'))
        self.commit('Remove included candidate selection')
        self.command('git','checkout','-q','campaign/unchain')
        verdict,_=self.classify(self.cache_dir())
        self.assertEqual(verdict.state,'active')
        self.command('git','branch','-D','lane/guard-owner')
        (self.repo / self.dep).unlink()
        self.commit('Remove source dependency')
        verdict,_=self.classify(self.cache_dir())
        self.assertEqual(verdict.reason_code,'source-identity')

    def test_base_only_dependency_change_does_not_claim_old_unchanged_lane(self):
        self.command('git','branch','lane/old-unchanged')
        (self.repo / self.dep).write_text(self.body+'\n/* base-only dependency update */\n')
        self.commit('Update source dependency')
        verdict,_=self.classify(None)
        self.assertEqual(verdict.state,'base-only')
        self.assertEqual(verdict.active_lanes,[])

    def test_null_pin_tracks_composite_dependency_history_both_directions(self):
        (self.repo / self.dep).write_text(self.body+'\n/* recorded work */\n')
        plateau=self.commit('Plateau included renderer')
        self.authorize_reopen(plateau,None)
        before,_=self.classify(self.cache_dir())
        self.assertEqual(before.state,'base-only')
        (self.repo / self.dep).write_text(self.body+'\n/* new recorded attempt */\n')
        latest=self.commit('Advance included body only')
        after,_=self.classify(self.cache_dir())
        self.assertNotEqual(after.state,'base-only')
        self.assertEqual(after.source_commit,latest)
        self.authorize_reopen(latest,None)
        fresh,_=self.classify(self.cache_dir())
        self.assertEqual(fresh.state,'base-only')
        self.assertEqual(fresh.source_commit,latest)

    def test_nonnull_pin_consumption_and_fresh_composite_pin(self):
        (self.repo / SHARD_PATH.parent).mkdir(exist_ok=True)
        (self.repo / SHARD_PATH).write_text(shard())
        (self.repo / self.dep).write_text(self.body+'\n/* plateau */\n')
        plateau=self.commit('Plateau included renderer with own shard')
        self.authorize_reopen(plateau,plateau)
        before,_=self.classify(self.cache_dir())
        self.assertEqual(before.state,'base-only')
        (self.repo / self.dep).write_text(self.body+'\n/* later attempt */\n')
        latest=self.commit('Advance included body only')
        after,_=self.classify(self.cache_dir())
        self.assertNotEqual(after.state,'base-only')
        self.authorize_reopen(latest,plateau)
        fresh,_=self.classify(self.cache_dir())
        self.assertEqual(fresh.state,'base-only')

    def test_foreign_dependency_shard_is_not_target_ledger(self):
        directory=self.repo / SHARD_PATH.parent
        directory.mkdir(exist_ok=True)
        (directory / (self.original+'.md')).write_text(shard(source=str(self.dep),symbol=self.original))
        (self.repo / self.dep).write_text(self.body+'\n/* plateau */\n')
        self.commit('Plateau shared source')
        verdict,_=self.classify(None)
        self.assertEqual(verdict.state,'stale-ledger')
        self.assertIsNone(verdict.ledger_commit)

    def test_malformed_alias_and_include_forms_fail_closed(self):
        cases=[
            (self.wrapper.replace(f'#define {self.original} ',f'#define {self.original}() '),self.body),
            (self.wrapper.replace(str(self.dep),'../shared.c'),self.body),
            (self.wrapper.replace('#ifdef NON_MATCHING','#if OTHER'),self.body),
            (self.wrapper+'void '+SYMBOL+'(void) {}\n',self.body),
            (self.wrapper.replace('#else','#include "extra.c"\n#else'),self.body),
            (self.wrapper.replace('#ifdef NON_MATCHING', '#ifdef NON_MATCHING\n/*\nvoid '+SYMBOL+'(void) {}\n*/'),self.body.replace('void '+self.original+'(void) {','void other(void) {')),
            (self.wrapper,self.body.replace('#ifdef NON_MATCHING','#if 0')),
            (self.wrapper,self.body.replace('void '+self.original+'(void) {','void other(void) {')),
            (self.wrapper,self.body+'#include "nested.c"\n'),
            (self.wrapper,self.body+'void trailing(void) {}\n'),
            (self.wrapper,'#pragma GLOBAL_ASM("asm/sidecar.s")\n'+self.body),
            (self.wrapper,self.body.replace('PR/ultratypes.h','CONFIG_HEADER')),
            (self.wrapper,self.body+'#define '+self.original+' other\n'),
            (self.wrapper,self.body.replace('void '+self.original+'(void) {\n    overlay69SubmitDynamicReloc();\n}', '/* void '+self.original+'(void) {} */')),
            (self.wrapper,self.body.replace('void '+self.original+'(void) {\n    overlay69SubmitDynamicReloc();\n}', 'const char *decoy = \"void '+self.original+'(void) {}\";')),
        ]
        for wrapper,body in cases:
            with self.subTest(wrapper=wrapper,body=body):
                (self.repo / SOURCE_PATH).write_text(wrapper)
                (self.repo / self.dep).write_text(body)
                self.commit('Reject malformed include')
                verdict,_=self.classify(None)
                self.assertNotEqual(verdict.state,'base-only')

    def test_identifier_expansion_preserves_strings_and_comments(self):
        body=self.body.replace('    overlay69SubmitDynamicReloc();',
            '    const char *label = "overlay69SubmitDynamicReloc";\n'
            '    /* overlay69SubmitDynamicReloc */\n'
            '    overlay69SubmitDynamicReloc();')
        (self.repo / self.dep).write_text(body)
        self.commit('Preserve literal alias spelling')
        self._clear()
        previous=Path.cwd()
        os.chdir(self.repo)
        try:
            expanded,_=ls.indirect_source('campaign/unchain',str(SOURCE_PATH),SYMBOL)
        finally:
            os.chdir(previous)
        self.assertIn('"overlay69SubmitDynamicReloc"',expanded)
        self.assertIn('/* overlay69SubmitDynamicReloc */',expanded)
        self.assertIn('    overlay43SubmitDynamicReloc();',expanded)

    def test_header_cannot_disable_guard_or_hide_an_alias_override(self):
        header=self.repo / 'include/PR/ultratypes.h'
        for text in ['#undef NON_MATCHING\n', '#include "nested.h"\n',
                     '#pragma GLOBAL_ASM("asm/header.s")\n',
                     '#define '+self.original+' foreign\n']:
            with self.subTest(header=text):
                header.write_text(text)
                self.commit('Reject hidden preprocessing override')
                verdict,_=self.classify(None)
                self.assertEqual(verdict.reason_code,'source-identity')

    def test_wrong_tuple_and_duplicate_owner_fail_closed(self):
        import copy
        original=copy.deepcopy(self.atlas)
        variants=[]
        for key,value in [('nonmatching',False),('offset','0x100'),('source','overlays/o043/foreign')]:
            variant=copy.deepcopy(original);variant['modules'][0]['text_ownership'][0][key]=value;variants.append(variant)
        variant=copy.deepcopy(original);variant['modules'][0]['overlay']=88;variants.append(variant)
        variant=copy.deepcopy(original);variant['modules'][0]['sections']['text']['start']='0x1890000';variants.append(variant)
        variant=copy.deepcopy(original);variant['modules'][0]['text_ownership']*=2;variants.append(variant)
        variant=copy.deepcopy(original);variant['modules'][0]['text_ownership'].append({'offset':'0x200','end_offset':'0x210','source':'overlays/o043/other'});variants.append(variant)
        for variant in variants:
            with self.subTest(atlas=variant):
                self.atlas_path.write_text(json.dumps(variant));self.commit('Reject ambiguous or foreign tuple')
                verdict,_=self.classify(None)
                self.assertEqual(verdict.reason_code,'source-identity')

    def test_duplicate_wrapper_and_build_macro_override_fail_closed(self):
        other=self.repo / SOURCE_PATH.parent / 'duplicate.c'
        other.write_text(candidate())
        self.commit('Duplicate exact source definition')
        verdict,_=self.classify(None)
        self.assertIn('ambiguous',verdict.reason)
        other.unlink()
        (self.repo / 'Makefile').write_text('CFLAGS += -D'+self.original+'=foreign\n')
        self.commit('Override source alias from flags')
        verdict,_=self.classify(None)
        self.assertEqual(verdict.reason_code,'source-identity')


class LaneRefQueryTests(unittest.TestCase):
    def tearDown(self) -> None:
        ls.show_file.cache_clear()
        ls.blob_id.cache_clear()
        ls.build_fallback_aliases.cache_clear()
        ls.merge_base.cache_clear()

    def test_lane_scan_filters_refs_already_merged_into_base(self) -> None:
        with mock.patch.object(ls, "git", return_value="") as git:
            self.assertEqual(
                ls.lane_refs(containing="source", unmerged_into="campaign/unchain"),
                [],
            )
        self.assertIn("--contains=source", git.call_args.args)
        self.assertIn("--no-merged=campaign/unchain", git.call_args.args)
        self.assertIn("refs/heads/lane/", git.call_args.args)
        self.assertIn(
            "refs/remotes/origin/lane/burn-b-*", git.call_args.args,
        )

    def test_equal_source_blob_skips_candidate_resolution(self) -> None:
        calls = []

        def fake_git(*args, **_kwargs):
            calls.append(args)
            if args[0] == "for-each-ref":
                return "lane/history\x00deadbeef\n"
            raise AssertionError(f"unexpected Git query: {args}")

        def fake_blobs(_refs, path):
            if path == "path.c":
                return {"lane/history": ("same-blob", "base")}
            return {"lane/history": None}

        with mock.patch.object(ls, "git", side_effect=fake_git), mock.patch.object(
            ls,
            "blob_contents",
            side_effect=fake_blobs,
        ), mock.patch.object(
            ls, "show_file", return_value=None,
        ), mock.patch.object(
            ls.finalize_plateau,
            "require_guarded_candidate",
            side_effect=AssertionError("should be skipped"),
        ):
            active = ls.active_lanes_for_source(
                "campaign/unchain", "symbol", "path.c", "same-blob", "source", "base"
            )
        self.assertEqual(active, [])

    def test_blob_ids_uses_one_batch_object_query(self) -> None:
        completed = subprocess.CompletedProcess(
            [], 0, "a" * 40 + " blob\n" + "b" * 40 + " blob\n", ""
        )
        with mock.patch.object(subprocess, "run", return_value=completed) as run:
            rows = ls.blob_ids(["lane/one", "lane/two"], "src/a.c")
        self.assertEqual(rows["lane/one"], "a" * 40)
        self.assertEqual(rows["lane/two"], "b" * 40)
        self.assertEqual(run.call_count, 1)

    def test_blob_contents_uses_one_batch_object_query(self) -> None:
        first = b"one\n"
        second = b"two\n"
        output = (
            ("a" * 40 + f" blob {len(first)}\n").encode() + first + b"\n"
            + ("b" * 40 + f" blob {len(second)}\n").encode() + second + b"\n"
        )
        completed = subprocess.CompletedProcess([], 0, output, b"")
        with mock.patch.object(subprocess, "run", return_value=completed) as run:
            rows = ls.blob_contents(["lane/one", "lane/two"], "src/a.c")
        self.assertEqual(rows["lane/one"], ("a" * 40, "one\n"))
        self.assertEqual(rows["lane/two"], ("b" * 40, "two\n"))
        self.assertEqual(run.call_count, 1)

    def test_batch_source_identity_uses_one_grep_and_one_object_batch(self) -> None:
        completed = subprocess.CompletedProcess(
            [], 0, "HEAD:src/a.c\nHEAD:src/b.c\n", "",
        )
        sources = {
            "src/a.c": ("a" * 40, "void alpha(void) { }\n"),
            "src/b.c": ("b" * 40, "void beta(void) { alpha(); }\n"),
        }
        with mock.patch.object(
            subprocess, "run", return_value=completed,
        ) as run, mock.patch.object(
            ls, "blob_contents_by_path", return_value=sources,
        ) as batch:
            identities = ls.source_identity_index("HEAD", ["alpha", "beta"])
        self.assertEqual(identities["alpha"], ("src/a.c", None))
        self.assertEqual(identities["beta"], ("src/b.c", None))
        self.assertEqual(run.call_count, 1)
        batch.assert_called_once_with("HEAD", ["src/a.c", "src/b.c"])

    def test_lane_index_filters_shared_legacy_edits_by_exact_symbol(self) -> None:
        base = ls.LanePathIndex(
            base="base",
            refs_by_path={"src/a.c": (("lane/source", "a" * 40),)},
            legacy_refs_by_symbol={
                "alpha": (("lane/legacy", "b" * 40),),
            },
            common_by_branch={
                "lane/source": "c" * 40,
                "lane/legacy": "d" * 40,
            },
        )
        self.assertEqual(
            base.refs_for(["src/a.c"], symbol="alpha"),
            [("lane/legacy", "b" * 40), ("lane/source", "a" * 40)],
        )
        self.assertEqual(
            base.refs_for(["src/other.c"], symbol="beta"), [],
        )

    def test_indexed_active_scan_skips_unrelated_lane_paths(self) -> None:
        index = ls.LanePathIndex(
            base="base", refs_by_path={}, legacy_refs_by_symbol={},
            common_by_branch={},
        )
        with mock.patch.object(
            ls, "lane_refs", side_effect=AssertionError("must use index"),
        ), mock.patch.object(
            ls, "is_ancestor", side_effect=AssertionError("no candidates"),
        ):
            active = ls.active_lanes_for_source(
                "base", "alpha", "src/a.c", "blob", "commit", "source",
                index,
            )
        self.assertEqual(active, [])

class BatchSymbolScreenTests(unittest.TestCase):
    """--symbols shares one evidence scan across many targets.

    Screening a translation unit one --symbol call at a time rebuilds the whole
    lane index per call, which is why it cost minutes and got skipped. These
    tests hold the contract the batch mode has to keep: same verdicts as the
    single-symbol path, a machine-readable line per symbol, and an exit status
    that says whether everything asked about is assignable.
    """

    def test_empty_value_is_an_error_not_a_full_listing(self) -> None:
        proc = subprocess.run(
            [sys.executable, str(TOOL), "--symbols", ""],
            capture_output=True, text=True, cwd=str(TOOL.parent.parent),
        )
        self.assertEqual(proc.returncode, 2)
        self.assertIn("named nothing", proc.stderr)

    def test_unknown_symbol_fails_closed(self) -> None:
        proc = subprocess.run(
            [sys.executable, str(TOOL), "--symbols", "not_a_real_symbol_xyz"],
            capture_output=True, text=True, cwd=str(TOOL.parent.parent),
        )
        self.assertNotEqual(proc.returncode, 0)


if __name__ == "__main__":
    unittest.main()
