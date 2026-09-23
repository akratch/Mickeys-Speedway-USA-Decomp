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


class LaneStatusAssignmentTests(unittest.TestCase):
    def setUp(self) -> None:
        ls.show_file.cache_clear()
        ls.blob_id.cache_clear()
        ls.guarded_candidate_region.cache_clear()
        ls.target_guard_changed.cache_clear()
        ls.reopen_authorizations.cache_clear()
        ls.claim_dispositions.cache_clear()
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)
        self.command("git", "init", "-q", "-b", "campaign/unchain")
        self.command("git", "config", "user.email", "lane-status@example.invalid")
        self.command("git", "config", "user.name", "Lane Status Test")
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
        ls.guarded_candidate_region.cache_clear()
        ls.target_guard_changed.cache_clear()
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

    def test_base_only_is_the_only_assignable_state(self) -> None:
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
        self.commit("Refine guarded candidate evidence")

        result, report = self.status()
        assignment = report["assignment"]
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertEqual(assignment["state"], "stale-ledger")
        self.assertEqual(
            assignment["reason_code"], "reopen-authorization-stale",
        )
        self.assertEqual(assignment["source_commit"], source_commit)

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
        self.assertEqual(assignment["source_commit"], plateau_commit)
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
        self.assertEqual(cache.hits, 1, "same base commit must hit")
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
            [f"{cache.base_commit}.json"],
        )

    def test_key_covers_code_shard_and_authorization(self) -> None:
        self.current_plateau()
        previous = Path.cwd()
        os.chdir(self.repo)
        try:
            cache = ls.AssignmentCache("campaign/unchain", self.cache_dir())
        finally:
            os.chdir(previous)
        path = SOURCE_PATH.as_posix()
        base_key = cache.key(SYMBOL, path)
        self.assertEqual(base_key, cache.key(SYMBOL, path))
        probe = ls.AssignmentCache.__new__(ls.AssignmentCache)
        probe.__dict__.update(cache.__dict__, code="0" * 64)
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        probe.__dict__.update(cache.__dict__, base_commit="e" * 40)
        self.assertNotEqual(base_key, probe.key(SYMBOL, path))
        for name in (SHARD_PATH.as_posix(), ls.REOPEN_AUTHORIZATIONS_PATH, path):
            with self.subTest(name):
                probe.__dict__.update(
                    cache.__dict__, blobs=dict(cache.blobs, **{name: "f" * 40}))
                self.assertNotEqual(base_key, probe.key(SYMBOL, path))

    def test_a_corrupt_cache_file_is_ignored(self) -> None:
        self.current_plateau()
        _, cache = self.classify(self.cache_dir())
        cache.path.write_text("{not json", encoding="utf-8")
        uncached, _ = self.classify(None)
        again, cache = self.classify(self.cache_dir())
        self.assertEqual((cache.hits, cache.misses), (0, 1))
        self.assertEqual(again, uncached)


class LaneRefQueryTests(unittest.TestCase):
    def tearDown(self) -> None:
        ls.show_file.cache_clear()
        ls.blob_id.cache_clear()
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
