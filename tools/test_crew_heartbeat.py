#!/usr/bin/env python3
"""Focused tests for campaign worker heartbeat state."""

from __future__ import annotations

import argparse
import contextlib
import datetime as dt
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import crew  # noqa: E402


class HeartbeatTests(unittest.TestCase):
    def record(self, **changes: object) -> dict[str, object]:
        row: dict[str, object] = {
            "schema": 3,
            "worker": "worker-1",
            "target": "demo_symbol",
            "base": "a" * 40,
            "last_progress": "baseline compared",
            "last_progress_at": "2026-08-29T10:00:00Z",
            "last_commit": "b" * 40,
            "deadline_unix": 1787998500,
            "attempt_count": 2,
            "best_score": "192/287 words differ",
            "mismatch_class": "register-allocation",
            "eta_unix": None,
            "target_words": None,
            "candidate_words": None,
            "raw_differing_words": None,
            "relocation_masked_differing_words": None,
            "candidate_relocations": None,
            "target_relocations": None,
            "exact_relocation_identities": None,
            "promotion_state": "unmeasured",
            "state": "active",
            "updated_at": "2026-08-29T10:00:00Z",
        }
        row.update(changes)
        return row

    def schema_two_record(self, **changes: object) -> dict[str, object]:
        row = self.record(schema=2)
        for field in (*crew.MEASUREMENT_FIELDS, "promotion_state"):
            row.pop(field)
        row.update(changes)
        return row

    def legacy_record(self, **changes: object) -> dict[str, object]:
        row = self.schema_two_record(schema=1)
        for field in ("attempt_count", "best_score", "mismatch_class", "eta_unix"):
            row.pop(field)
        row.update(changes)
        return row

    def measured_record(self, **changes: object) -> dict[str, object]:
        row = self.record(
            target_words=172,
            candidate_words=172,
            raw_differing_words=100,
            relocation_masked_differing_words=8,
            candidate_relocations=21,
            target_relocations=21,
            exact_relocation_identities=20,
            promotion_state="compiled",
        )
        row.update(changes)
        if "best_score" not in changes:
            row["best_score"] = crew.measured_score(row)
        return row

    def schema_four_record(self, **changes: object) -> dict[str, object]:
        row = self.measured_record()
        measurement = {
            field: row[field] for field in (*crew.MEASUREMENT_FIELDS, "promotion_state")
        }
        row.update({
            "schema": 4,
            "current_score": crew.measured_score(measurement),
            "best_result": measurement,
            "best_mismatch_class": row["mismatch_class"],
            "best_artifact": None,
        })
        row.update(changes)
        return row

    def command_args(self, **changes: object) -> argparse.Namespace:
        values: dict[str, object] = {
            "worker": "worker-1",
            "target": None,
            "base": None,
            "deadline_unix": None,
            "progress": "candidate measured",
            "state": "active",
            "attempt_count": None,
            "best_score": None,
            "mismatch_class": None,
            "eta_unix": None,
            "wb_summary": None,
            "archive_best": False,
            "source": None,
            "candidate_object": None,
            **{field: None for field in crew.MEASUREMENT_FIELDS},
            "promotion_state": None,
        }
        values.update(changes)
        return argparse.Namespace(**values)

    def write_summary(
        self,
        root: Path,
        *,
        raw: int = 100,
        masked: int = 8,
        target_words: int = 172,
        candidate_words: int = 172,
        exact: bool = False,
        admissible: bool = False,
        symbol: str = "demo_symbol",
        relocations: dict[str, int] | None = None,
        build_claim: str | None = "match",
    ) -> Path:
        path = root / "build/wb/demo.summary.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        payload: dict[str, object] = {
            "schema": "mickey-wb-summary-v1",
            "symbol": {
                "requested": symbol,
                "target": symbol,
                "candidate": symbol,
            },
            "boundary": {"bytes": target_words * 4, "evidence": "test"},
            "comparison": {
                "target_words": target_words,
                "candidate_words": candidate_words,
                "matched_words": target_words - masked if target_words == candidate_words else None,
                "differing_words": masked,
                "raw_differing_words": raw,
                "exact": exact,
            },
            "evidence": {
                "admissible_exact_comparison": admissible,
                "promotion_proof_included": False,
            },
            "provenance": {"build_claim": build_claim},
        }
        if relocations is not None:
            payload["relocations"] = relocations
        path.write_text(json.dumps(payload), encoding="utf-8")
        return path

    def init_archive_repo(self, root: Path) -> tuple[str, Path]:
        subprocess.run(["git", "init", "-q", "-b", "lane/demo"], cwd=root, check=True)
        subprocess.run(["git", "config", "user.email", "test@example.invalid"], cwd=root, check=True)
        subprocess.run(["git", "config", "user.name", "Checkpoint Test"], cwd=root, check=True)
        (root / ".gitignore").write_text("build/\n", encoding="utf-8")
        source = root / "src/demo.c"
        source.parent.mkdir(parents=True)
        source.write_text("int demo_symbol(void) { return 1; }\n", encoding="utf-8")
        subprocess.run(["git", "add", ".gitignore", "src/demo.c"], cwd=root, check=True)
        subprocess.run(["git", "commit", "-q", "-m", "fixture"], cwd=root, check=True)
        base = subprocess.run(
            ["git", "rev-parse", "HEAD"], cwd=root, check=True, text=True,
            stdout=subprocess.PIPE,
        ).stdout.strip()
        self.write_summary(
            root, raw=5, masked=5,
            relocations={
                "candidate_relocations": 21,
                "target_relocations": 21,
                "exact_relocation_identities": 20,
            },
        )
        candidate = root / "build/src/demo.o"
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(b"candidate-object")
        return base, source

    def archive_best(self, root: Path, base: str) -> tuple[Path, dict[str, object]]:
        crew_root = root / ".git/codex-crew"
        args = self.command_args(
            target="demo_symbol", base=base, deadline_unix=2_000_000_000,
            attempt_count=1, mismatch_class="register-allocation",
            wb_summary="build/wb/demo.summary.json", archive_best=True,
            source="src/demo.c", candidate_object="build/src/demo.o",
        )
        with mock.patch.object(crew, "repository_root", return_value=root.resolve()):
            with mock.patch.object(crew, "run_git", return_value=base):
                with contextlib.redirect_stdout(io.StringIO()):
                    crew.command_heartbeat(args, crew_root)
        return crew_root, crew.load_heartbeat(crew_root / "heartbeats/worker-1.json")

    def run_checkpoint_with_summary(
        self, root: Path, args: argparse.Namespace
    ) -> dict[str, object]:
        with mock.patch.object(crew, "repository_root", return_value=root):
            with mock.patch.object(crew, "git_ignored", return_value=True):
                with mock.patch.object(crew, "run_git", return_value="c" * 40):
                    with contextlib.redirect_stdout(io.StringIO()):
                        crew.command_heartbeat(args, root / "crew")
        return crew.load_heartbeat(root / "crew/heartbeats/worker-1.json")

    def test_staleness_reports_progress_and_deadline_independently(self) -> None:
        now = dt.datetime(2026, 8, 29, 10, 30, tzinfo=dt.timezone.utc)
        reasons = crew.heartbeat_staleness(
            self.record(deadline_unix=1787998200), now, dt.timedelta(minutes=15)
        )
        self.assertTrue(any("no progress" in reason for reason in reasons))
        self.assertTrue(any("deadline passed" in reason for reason in reasons))

    def test_terminal_heartbeat_is_not_stale(self) -> None:
        now = dt.datetime(2026, 8, 30, tzinfo=dt.timezone.utc)
        reasons = crew.heartbeat_staleness(
            self.record(state="handoff"), now, dt.timedelta(minutes=1)
        )
        self.assertEqual(reasons, [])

    def test_passed_eta_is_stale_independently_of_deadline(self) -> None:
        now = dt.datetime(2026, 8, 29, 10, 5, tzinfo=dt.timezone.utc)
        reasons = crew.heartbeat_staleness(
            self.record(
                last_progress_at="2026-08-29T10:04:00Z",
                updated_at="2026-08-29T10:04:00Z",
                deadline_unix=int(now.timestamp()) + 3600,
                eta_unix=int(now.timestamp()) - 60,
            ),
            now,
            dt.timedelta(minutes=15),
        )
        self.assertEqual(reasons, ["ETA passed 1m ago"])

    def test_schema_one_record_is_readable_with_checkpoint_defaults(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.legacy_record()) + "\n")
            loaded = crew.load_heartbeat(path)
        self.assertEqual(loaded["attempt_count"], 0)
        self.assertEqual(loaded["best_score"], "not recorded")
        self.assertEqual(loaded["mismatch_class"], "unclassified")
        self.assertIsNone(loaded["eta_unix"])
        self.assertEqual(loaded["promotion_state"], "unmeasured")
        self.assertTrue(all(loaded[field] is None for field in crew.MEASUREMENT_FIELDS))

    def test_schema_two_record_is_readable_with_unmeasured_numeric_defaults(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.schema_two_record()) + "\n")
            loaded = crew.load_heartbeat(path)
        self.assertEqual(loaded["attempt_count"], 2)
        self.assertEqual(loaded["best_score"], "192/287 words differ")
        self.assertEqual(loaded["promotion_state"], "unmeasured")
        self.assertTrue(all(loaded[field] is None for field in crew.MEASUREMENT_FIELDS))

    def test_heartbeat_update_preserves_assignment_and_refreshes_commit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.record()) + "\n")
            args = argparse.Namespace(
                worker="worker-1", target=None, base=None, deadline_unix=None,
                progress="candidate improved", state="active", attempt_count=3,
                best_score="187/287 words differ", mismatch_class="frame-allocation",
                eta_unix=1787998400,
            )
            with mock.patch.object(crew, "run_git", return_value="c" * 40):
                with contextlib.redirect_stdout(io.StringIO()):
                    crew.command_heartbeat(args, root)
            updated = crew.load_heartbeat(path)
            self.assertEqual(updated["target"], "demo_symbol")
            self.assertEqual(updated["base"], "a" * 40)
            self.assertEqual(updated["last_commit"], "c" * 40)
            self.assertEqual(updated["last_progress"], "candidate improved")
            self.assertEqual(updated["attempt_count"], 3)
            self.assertEqual(updated["best_score"], "187/287 words differ")
            self.assertEqual(updated["mismatch_class"], "frame-allocation")
            self.assertEqual(updated["eta_unix"], 1787998400)

    def test_schema_one_update_migrates_to_schema_four_without_guessing_metrics(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.legacy_record()) + "\n")
            args = argparse.Namespace(
                worker="worker-1", target=None, base=None, deadline_unix=None,
                progress="baseline measured", state="active", attempt_count=1,
                best_score="200 words differ", mismatch_class="structure", eta_unix=None,
            )
            with mock.patch.object(crew, "run_git", return_value="c" * 40):
                with contextlib.redirect_stdout(io.StringIO()):
                    crew.command_heartbeat(args, root)
            raw = json.loads(path.read_text(encoding="utf-8"))
        self.assertEqual(raw["schema"], 4)
        self.assertEqual(raw["attempt_count"], 1)
        self.assertEqual(raw["promotion_state"], "unmeasured")
        self.assertEqual(raw["best_result"]["promotion_state"], "unmeasured")
        self.assertEqual(raw["current_score"], "200 words differ")
        self.assertTrue(all(raw[field] is None for field in crew.MEASUREMENT_FIELDS))

    def test_schemas_one_two_and_three_remain_readable_and_migrate(self) -> None:
        fixtures = (self.legacy_record(), self.schema_two_record(), self.measured_record())
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for index, fixture in enumerate(fixtures, 1):
                with self.subTest(schema=index):
                    path = root / "heartbeats/worker-1.json"
                    crew.atomic_write(path, json.dumps(fixture) + "\n")
                    loaded = crew.load_heartbeat(path)
                    args = self.command_args(progress=f"schema {index} migrated")
                    with mock.patch.object(crew, "run_git", return_value="c" * 40):
                        with contextlib.redirect_stdout(io.StringIO()):
                            crew.command_heartbeat(args, root)
                    raw = json.loads(path.read_text(encoding="utf-8"))
                    self.assertEqual(raw["schema"], 4)
                    self.assertEqual(raw["best_result"], loaded["best_result"])

    def test_attempt_count_cannot_regress_for_same_assignment(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.record(attempt_count=4)) + "\n")
            args = argparse.Namespace(
                worker="worker-1", target=None, base=None, deadline_unix=None,
                progress="older checkpoint", state="active", attempt_count=3,
                best_score=None, mismatch_class=None, eta_unix=None,
            )
            with self.assertRaisesRegex(crew.CrewError, "cannot decrease"):
                crew.command_heartbeat(args, root)

    def test_json_status_is_concise_and_contains_matching_checkpoint(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.measured_record()) + "\n")
            args = argparse.Namespace(
                worker=None, stale_after_minutes=15, check=True, json=True,
            )
            now = dt.datetime(2026, 8, 29, 10, 5, tzinfo=dt.timezone.utc)
            output = io.StringIO()
            with mock.patch.object(crew, "utc_now", return_value=now):
                with contextlib.redirect_stdout(output):
                    crew.command_heartbeat_status(args, root)
            self.assertEqual(len(output.getvalue().splitlines()), 1)
            report = json.loads(output.getvalue())
        self.assertTrue(report["ok"])
        self.assertEqual(report["summary"], {
            "current": 1, "malformed": 0, "stale": 0, "terminal": 0, "total": 1,
        })
        worker = report["workers"][0]
        self.assertEqual(worker["attempt_count"], 2)
        self.assertEqual(worker["target_words"], 172)
        self.assertEqual(worker["candidate_words"], 172)
        self.assertEqual(worker["raw_differing_words"], 100)
        self.assertEqual(worker["relocation_masked_differing_words"], 8)
        self.assertEqual(worker["candidate_relocations"], 21)
        self.assertEqual(worker["target_relocations"], 21)
        self.assertEqual(worker["exact_relocation_identities"], 20)
        self.assertEqual(worker["promotion_state"], "compiled")
        self.assertEqual(worker["mismatch_class"], "register-allocation")
        self.assertEqual(worker["health"], "current")

    def test_malformed_json_report_fails_closed_without_hiding_other_workers(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            directory = root / "heartbeats"
            crew.atomic_write(directory / "worker-1.json", json.dumps(self.record()) + "\n")
            crew.atomic_write(directory / "worker-2.json", "{not json\n")
            args = argparse.Namespace(
                worker=None, stale_after_minutes=15, check=False, json=True,
            )
            now = dt.datetime(2026, 8, 29, 10, 5, tzinfo=dt.timezone.utc)
            output = io.StringIO()
            with mock.patch.object(crew, "utc_now", return_value=now):
                with mock.patch("os.kill") as kill:
                    with contextlib.redirect_stdout(output):
                        with self.assertRaisesRegex(crew.CrewError, "malformed heartbeat"):
                            crew.command_heartbeat_status(args, root)
            report = json.loads(output.getvalue())
        self.assertFalse(report["ok"])
        self.assertEqual(report["summary"]["current"], 1)
        self.assertEqual(report["summary"]["malformed"], 1)
        self.assertEqual(report["workers"][1]["health"], "malformed")
        kill.assert_not_called()

    def test_stale_json_check_reports_before_failing_and_never_kills(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.record()) + "\n")
            args = argparse.Namespace(
                worker=None, stale_after_minutes=1, check=False, json=True,
            )
            now = dt.datetime(2026, 8, 29, 11, 0, tzinfo=dt.timezone.utc)
            output = io.StringIO()
            with mock.patch.object(crew, "utc_now", return_value=now):
                with mock.patch("os.kill") as kill:
                    with contextlib.redirect_stdout(output):
                        with self.assertRaisesRegex(crew.CrewError, "stale heartbeat"):
                            crew.command_heartbeat_status(args, root)
            report = json.loads(output.getvalue())
        self.assertFalse(report["ok"])
        self.assertEqual(report["summary"]["stale"], 1)
        self.assertEqual(report["workers"][0]["health"], "stale")
        kill.assert_not_called()

    def test_worker_identity_must_match_heartbeat_filename(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "worker-2.json"
            crew.atomic_write(path, json.dumps(self.record()) + "\n")
            with self.assertRaisesRegex(crew.CrewError, "expected 'worker-2'"):
                crew.load_heartbeat(path)

    def test_schema_three_rejects_noncanonical_or_unknown_checkpoint_state(self) -> None:
        malformed = (
            {"schema": True},
            {"attempt_count": True},
            {"last_progress": "two\nlines"},
            {"eta_unix": 10**100},
            {"unexpected": "field"},
        )
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "worker-1.json"
            for changes in malformed:
                with self.subTest(changes=changes):
                    crew.atomic_write(path, json.dumps(self.record(**changes)) + "\n")
                    with self.assertRaises(crew.CrewError):
                        crew.load_heartbeat(path)

    def test_schema_four_fails_closed_on_invalid_or_subordinate_best(self) -> None:
        exact = {
            "target_words": 172, "candidate_words": 172,
            "raw_differing_words": 0, "relocation_masked_differing_words": 0,
            "candidate_relocations": 21, "target_relocations": 21,
            "exact_relocation_identities": 20, "promotion_state": "object-exact",
        }
        subordinate = {
            "target_words": 172, "candidate_words": 172,
            "raw_differing_words": 101, "relocation_masked_differing_words": 9,
            "candidate_relocations": 21, "target_relocations": 21,
            "exact_relocation_identities": 20, "promotion_state": "compiled",
        }
        rows = (
            self.schema_four_record(best_result=exact, best_score="invalid"),
            self.schema_four_record(
                best_result=subordinate, best_score=crew.measured_score(subordinate)
            ),
            self.schema_four_record(
                best_result=crew.unmeasured_result(), best_score="not measured"
            ),
            self.schema_four_record(
                best_result={**self.schema_four_record()["best_result"], "extra": 1}
            ),
        )
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "worker-1.json"
            for row in rows:
                crew.atomic_write(path, json.dumps(row) + "\n")
                with self.assertRaises(crew.CrewError):
                    crew.load_heartbeat(path)

    def test_summary_import_distinguishes_100_raw_from_8_masked(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(root, raw=100, masked=8)
            args = self.command_args(
                target="demo_symbol",
                base="a" * 40,
                deadline_unix=2_000_000_000,
                attempt_count=1,
                mismatch_class="relocation-constants",
                wb_summary="build/wb/demo.summary.json",
                candidate_relocations=21,
                target_relocations=21,
                exact_relocation_identities=20,
            )
            loaded = self.run_checkpoint_with_summary(root, args)
        self.assertEqual(loaded["raw_differing_words"], 100)
        self.assertEqual(loaded["relocation_masked_differing_words"], 8)
        self.assertEqual(loaded["best_score"], (
            "8/172 relocation-masked differing words (100 raw; candidate 172)"
        ))
        self.assertEqual(loaded["promotion_state"], "compiled")

    def test_167_matched_of_172_is_stored_as_5_differences(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(root, raw=5, masked=5)
            args = self.command_args(
                target="demo_symbol",
                base="a" * 40,
                deadline_unix=2_000_000_000,
                wb_summary="build/wb/demo.summary.json",
                candidate_relocations=21,
                target_relocations=21,
                exact_relocation_identities=20,
            )
            loaded = self.run_checkpoint_with_summary(root, args)
        self.assertEqual(loaded["target_words"], 172)
        self.assertEqual(loaded["candidate_words"], 172)
        self.assertEqual(loaded["relocation_masked_differing_words"], 5)
        self.assertNotIn("167/172", str(loaded["best_score"]))

    def test_summary_can_supply_all_metrics_and_derive_object_exact(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root,
                raw=0,
                masked=0,
                exact=True,
                admissible=True,
                relocations={
                    "candidate_relocations": 6,
                    "target_relocations": 6,
                    "exact_relocation_identities": 6,
                },
            )
            args = self.command_args(
                target="demo_symbol",
                base="a" * 40,
                deadline_unix=2_000_000_000,
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, args)
        self.assertEqual(loaded["promotion_state"], "object-exact")
        self.assertEqual(loaded["raw_differing_words"], 0)
        self.assertEqual(loaded["exact_relocation_identities"], 6)

    def test_word_exact_summary_with_incomplete_identities_stays_compiled(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root,
                raw=0,
                masked=0,
                exact=True,
                admissible=True,
                relocations={
                    "candidate_relocations": 21,
                    "target_relocations": 21,
                    "exact_relocation_identities": 11,
                },
            )
            args = self.command_args(
                target="demo_symbol",
                base="a" * 40,
                deadline_unix=2_000_000_000,
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, args)
        self.assertEqual(loaded["raw_differing_words"], 0)
        self.assertEqual(loaded["exact_relocation_identities"], 11)
        self.assertEqual(loaded["promotion_state"], "compiled")

    def test_exact_summary_with_unmatched_build_claim_and_admissible_is_rejected(
        self,
    ) -> None:
        # function_preflight.workbench_summary() should never itself emit
        # admissible_exact_comparison=True alongside a non-"match" build
        # claim, but a hand-edited or stale summary can, and crew.py must not
        # trust it: exact + a forced/instrumented/unknown build is not a
        # verified match.
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root,
                raw=0,
                masked=0,
                exact=True,
                admissible=True,
                build_claim="reachability-proof",
                relocations={
                    "candidate_relocations": 6,
                    "target_relocations": 6,
                    "exact_relocation_identities": 6,
                },
            )
            with mock.patch.object(crew, "repository_root", return_value=root):
                with mock.patch.object(crew, "git_ignored", return_value=True):
                    with self.assertRaisesRegex(crew.CrewError, "unverified build_provenance"):
                        crew.workbench_summary_result(
                            "build/wb/demo.summary.json",
                            worker="worker-1",
                            target="demo_symbol",
                        )

    def test_exact_summary_with_unmatched_build_claim_stays_compiled(self) -> None:
        # Same unverified build claim, but admissible_exact_comparison is
        # already False (the ordinary case, since function_preflight folds
        # the claim into that field itself): no error, just no promotion.
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root,
                raw=0,
                masked=0,
                exact=True,
                admissible=False,
                build_claim="unknown",
                relocations={
                    "candidate_relocations": 6,
                    "target_relocations": 6,
                    "exact_relocation_identities": 6,
                },
            )
            args = self.command_args(
                target="demo_symbol",
                base="a" * 40,
                deadline_unix=2_000_000_000,
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, args)
        self.assertEqual(loaded["promotion_state"], "compiled")

    def test_exact_summary_without_provenance_block_is_unchanged(self) -> None:
        # Older summaries carry no provenance.build_claim at all; that must
        # not be treated as a denial.
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root,
                raw=0,
                masked=0,
                exact=True,
                admissible=True,
                build_claim=None,
                relocations={
                    "candidate_relocations": 6,
                    "target_relocations": 6,
                    "exact_relocation_identities": 6,
                },
            )
            args = self.command_args(
                target="demo_symbol",
                base="a" * 40,
                deadline_unix=2_000_000_000,
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, args)
        self.assertEqual(loaded["promotion_state"], "object-exact")

    def test_worse_current_checkpoint_never_replaces_best(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root, raw=5, masked=5,
                relocations={
                    "candidate_relocations": 21, "target_relocations": 21,
                    "exact_relocation_identities": 20,
                },
            )
            first = self.command_args(
                target="demo_symbol", base="a" * 40, deadline_unix=2_000_000_000,
                attempt_count=1, mismatch_class="register-allocation",
                wb_summary="build/wb/demo.summary.json",
            )
            self.run_checkpoint_with_summary(root, first)
            self.write_summary(
                root, raw=8, masked=8,
                relocations={
                    "candidate_relocations": 21, "target_relocations": 21,
                    "exact_relocation_identities": 20,
                },
            )
            second = self.command_args(
                attempt_count=2, mismatch_class="schedule",
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, second)
        self.assertEqual(loaded["relocation_masked_differing_words"], 8)
        self.assertEqual(loaded["current_score"], crew.measured_score(loaded))
        self.assertEqual(loaded["best_result"]["relocation_masked_differing_words"], 5)
        self.assertEqual(loaded["best_mismatch_class"], "register-allocation")
        self.assertEqual(loaded["mismatch_class"], "schedule")

    def test_exact_tie_retains_first_best_but_updates_current_class(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root, raw=5, masked=5,
                relocations={
                    "candidate_relocations": 21, "target_relocations": 21,
                    "exact_relocation_identities": 20,
                },
            )
            first = self.command_args(
                target="demo_symbol", base="a" * 40, deadline_unix=2_000_000_000,
                mismatch_class="first-form", wb_summary="build/wb/demo.summary.json",
            )
            self.run_checkpoint_with_summary(root, first)
            tied = self.command_args(
                attempt_count=1, mismatch_class="tied-form",
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, tied)
        self.assertEqual(loaded["mismatch_class"], "tied-form")
        self.assertEqual(loaded["best_mismatch_class"], "first-form")
        self.assertEqual(loaded["best_result"]["relocation_masked_differing_words"], 5)

    def test_exact_promotion_replaces_nonexact_best(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(
                root, raw=5, masked=5,
                relocations={
                    "candidate_relocations": 21, "target_relocations": 21,
                    "exact_relocation_identities": 20,
                },
            )
            first = self.command_args(
                target="demo_symbol", base="a" * 40, deadline_unix=2_000_000_000,
                mismatch_class="allocation", wb_summary="build/wb/demo.summary.json",
            )
            self.run_checkpoint_with_summary(root, first)
            self.write_summary(
                root, raw=0, masked=0, exact=True, admissible=True,
                relocations={
                    "candidate_relocations": 21, "target_relocations": 21,
                    "exact_relocation_identities": 21,
                },
            )
            exact = self.command_args(
                attempt_count=1, mismatch_class="exact",
                wb_summary="build/wb/demo.summary.json",
            )
            loaded = self.run_checkpoint_with_summary(root, exact)
        self.assertEqual(loaded["promotion_state"], "object-exact")
        self.assertEqual(loaded["best_result"]["promotion_state"], "object-exact")
        self.assertEqual(loaded["best_mismatch_class"], "exact")

    def test_comparator_order_and_ties_are_deterministic(self) -> None:
        baseline = {
            "target_words": 100, "candidate_words": 100,
            "raw_differing_words": 5, "relocation_masked_differing_words": 5,
            "candidate_relocations": 10, "target_relocations": 10,
            "exact_relocation_identities": 9, "promotion_state": "compiled",
        }
        self.assertFalse(crew.better_measurement(dict(baseline), dict(baseline)))
        geometry_loss = {**baseline, "candidate_words": 99, "raw_differing_words": 0,
                         "relocation_masked_differing_words": 0}
        self.assertTrue(crew.better_measurement(baseline, geometry_loss))
        masked_gain = {**baseline, "raw_differing_words": 4,
                       "relocation_masked_differing_words": 4}
        self.assertTrue(crew.better_measurement(masked_gain, baseline))
        identity_gain = {**baseline, "exact_relocation_identities": 10}
        self.assertTrue(crew.better_measurement(identity_gain, baseline))

    def test_direct_rom_exact_metrics_require_exact_words_and_relocations(self) -> None:
        exact = {
            "target_words": 108,
            "candidate_words": 108,
            "raw_differing_words": 0,
            "relocation_masked_differing_words": 0,
            "candidate_relocations": 6,
            "target_relocations": 6,
            "exact_relocation_identities": 6,
            "promotion_state": "rom-exact",
        }
        self.assertEqual(crew.validate_measured_result(exact), exact)
        for change in (
            {"raw_differing_words": 1},
            {"candidate_words": 107},
            {"exact_relocation_identities": 5},
        ):
            with self.subTest(change=change):
                with self.assertRaisesRegex(crew.CrewError, "without exact words"):
                    crew.validate_measured_result({**exact, **change})

    def test_promotion_only_update_reuses_same_assignment_exact_metrics(self) -> None:
        existing = self.measured_record(
            raw_differing_words=0,
            relocation_masked_differing_words=0,
            candidate_relocations=21,
            target_relocations=21,
            exact_relocation_identities=21,
            promotion_state="object-exact",
        )
        args = self.command_args(promotion_state="rom-exact")
        result = crew.checkpoint_result(
            args,
            target="demo_symbol",
            existing=existing,
            assignment_changed=False,
        )
        self.assertEqual(result["promotion_state"], "rom-exact")
        self.assertEqual(result["target_words"], 172)
        self.assertEqual(result["exact_relocation_identities"], 21)

    def test_partial_manual_metrics_are_rejected(self) -> None:
        args = self.command_args(target_words=172, promotion_state="compiled")
        with self.assertRaisesRegex(crew.CrewError, "incomplete"):
            crew.checkpoint_result(
                args, target="demo_symbol", existing=None, assignment_changed=False
            )

    def test_summary_conflict_with_explicit_metric_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.write_summary(root)
            args = self.command_args(
                wb_summary="build/wb/demo.summary.json",
                target_words=171,
                candidate_relocations=21,
                target_relocations=21,
                exact_relocation_identities=20,
            )
            with mock.patch.object(crew, "repository_root", return_value=root):
                with mock.patch.object(crew, "git_ignored", return_value=True):
                    with self.assertRaisesRegex(crew.CrewError, "conflicts"):
                        crew.checkpoint_result(
                            args,
                            target="demo_symbol",
                            existing=None,
                            assignment_changed=False,
                        )

    def test_summary_refuses_malformed_stale_or_wrong_target_evidence(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = self.write_summary(root)
            with mock.patch.object(crew, "repository_root", return_value=root):
                with mock.patch.object(crew, "git_ignored", return_value=True):
                    path.write_text("{not json", encoding="utf-8")
                    with self.assertRaisesRegex(crew.CrewError, "valid UTF-8 JSON"):
                        crew.workbench_summary_result(
                            "build/wb/demo.summary.json",
                            worker="worker-1",
                            target="demo_symbol",
                        )
                    self.write_summary(root, symbol="other_symbol")
                    with self.assertRaisesRegex(crew.CrewError, "assigned target"):
                        crew.workbench_summary_result(
                            "build/wb/demo.summary.json",
                            worker="worker-1",
                            target="demo_symbol",
                        )
                    self.write_summary(root)
                    payload = json.loads(path.read_text(encoding="utf-8"))
                    payload["worker"] = "worker-2"
                    path.write_text(json.dumps(payload), encoding="utf-8")
                    with self.assertRaisesRegex(crew.CrewError, "checkpoint worker"):
                        crew.workbench_summary_result(
                            "build/wb/demo.summary.json",
                            worker="worker-1",
                            target="demo_symbol",
                        )
                    self.write_summary(root)
                    os.utime(path, (1, 1))
                    with self.assertRaisesRegex(crew.CrewError, "stale"):
                        crew.workbench_summary_result(
                            "build/wb/demo.summary.json",
                            worker="worker-1",
                            target="demo_symbol",
                        )

    def test_summary_refuses_absolute_nonignored_and_symlink_paths(self) -> None:
        for path in ("/tmp/summary.json", "build/./wb/summary.json"):
            with self.subTest(path=path):
                with self.assertRaisesRegex(crew.CrewError, "repository-relative"):
                    crew.read_fresh_workbench_summary(path)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = self.write_summary(root)
            with mock.patch.object(crew, "repository_root", return_value=root):
                with mock.patch.object(crew, "git_ignored", return_value=False):
                    with self.assertRaisesRegex(crew.CrewError, "not ignored"):
                        crew.read_fresh_workbench_summary("build/wb/demo.summary.json")
                real = path.with_name("real.json")
                path.replace(real)
                path.symlink_to(real.name)
                with mock.patch.object(crew, "git_ignored", return_value=True):
                    with self.assertRaisesRegex(crew.CrewError, "symlink"):
                        crew.read_fresh_workbench_summary("build/wb/demo.summary.json")

    def test_failed_summary_ingestion_leaves_existing_record_unchanged(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            crew_root = root / "crew"
            heartbeat = crew_root / "heartbeats/worker-1.json"
            crew.atomic_write(heartbeat, json.dumps(self.record(), sort_keys=True) + "\n")
            before = heartbeat.read_bytes()
            path = self.write_summary(root)
            os.utime(path, (1, 1))
            args = self.command_args(wb_summary="build/wb/demo.summary.json")
            with mock.patch.object(crew, "repository_root", return_value=root):
                with mock.patch.object(crew, "git_ignored", return_value=True):
                    with self.assertRaisesRegex(crew.CrewError, "stale"):
                        crew.command_heartbeat(args, crew_root)
            self.assertEqual(heartbeat.read_bytes(), before)

    def test_restore_best_preserves_displaced_source_and_backup_is_recoverable(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            base, source = self.init_archive_repo(root)
            crew_root, heartbeat = self.archive_best(root, base)
            self.assertIsNotNone(heartbeat["best_artifact"])
            first_reference = heartbeat["best_artifact"]
            best_source = source.read_bytes()
            displaced = b"int demo_symbol(void) { return 99; }\n"
            source.write_bytes(displaced)
            self.write_summary(
                root, raw=8, masked=8,
                relocations={
                    "candidate_relocations": 21, "target_relocations": 21,
                    "exact_relocation_identities": 20,
                },
            )
            worse = self.command_args(
                attempt_count=2, mismatch_class="schedule",
                wb_summary="build/wb/demo.summary.json", archive_best=True,
                source="src/demo.c", candidate_object="build/src/demo.o",
            )
            with mock.patch.object(crew, "repository_root", return_value=root.resolve()):
                with mock.patch.object(crew, "run_git", return_value=base):
                    with contextlib.redirect_stdout(io.StringIO()):
                        crew.command_heartbeat(worse, crew_root)
            heartbeat = crew.load_heartbeat(crew_root / "heartbeats/worker-1.json")
            self.assertEqual(heartbeat["relocation_masked_differing_words"], 8)
            self.assertEqual(heartbeat["best_result"]["relocation_masked_differing_words"], 5)
            self.assertEqual(heartbeat["best_artifact"], first_reference)
            output = io.StringIO()
            with mock.patch.object(crew, "repository_root", return_value=root.resolve()):
                with contextlib.redirect_stdout(output):
                    crew.command_restore_best(
                        argparse.Namespace(worker="worker-1", recover_backup=None), crew_root
                    )
            receipt = json.loads(output.getvalue())
            self.assertEqual(source.read_bytes(), best_source)
            self.assertTrue(receipt["restored"])
            token = receipt["displaced_backup"]
            with mock.patch.object(crew, "repository_root", return_value=root.resolve()):
                with contextlib.redirect_stdout(io.StringIO()):
                    crew.command_restore_best(
                        argparse.Namespace(worker="worker-1", recover_backup=token), crew_root
                    )
            self.assertEqual(source.read_bytes(), displaced)

    def test_restore_refuses_archive_hash_and_manifest_path_tamper(self) -> None:
        for tamper in ("source", "manifest"):
            with self.subTest(tamper=tamper), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                base, source = self.init_archive_repo(root)
                crew_root, heartbeat = self.archive_best(root, base)
                reference = heartbeat["best_artifact"]
                artifact = crew_root / "best-artifacts/worker-1" / reference["id"]
                if tamper == "source":
                    (artifact / "source").write_bytes(b"tampered")
                    expected = "source hash drift"
                else:
                    manifest = json.loads((artifact / "manifest.json").read_text(encoding="utf-8"))
                    manifest["source_path"] = "src/other.c"
                    (artifact / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
                    expected = "manifest hash drift"
                source.write_text("worse\n", encoding="utf-8")
                with mock.patch.object(crew, "repository_root", return_value=root.resolve()):
                    with self.assertRaisesRegex(crew.CrewError, expected):
                        crew.command_restore_best(
                            argparse.Namespace(worker="worker-1", recover_backup=None), crew_root
                        )

    def test_restore_refuses_cross_worktree_branch_base_target_and_source_drift(self) -> None:
        scenarios = ("worktree", "branch", "base", "target", "symlink", "conflict")
        for scenario in scenarios:
            with self.subTest(scenario=scenario), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                base, source = self.init_archive_repo(root)
                crew_root, heartbeat = self.archive_best(root, base)
                source.write_text("worse\n", encoding="utf-8")
                expected_root = root.resolve()
                if scenario == "worktree":
                    expected_root = (root / "other-worktree").resolve()
                    expected = "different worktree"
                elif scenario == "branch":
                    subprocess.run(["git", "switch", "-q", "-c", "lane/other"], cwd=root, check=True)
                    expected = "different branch"
                elif scenario in ("base", "target"):
                    path = crew_root / "heartbeats/worker-1.json"
                    raw = json.loads(path.read_text(encoding="utf-8"))
                    raw[scenario] = ("d" * 40) if scenario == "base" else "other_symbol"
                    crew.atomic_write(path, json.dumps(raw) + "\n")
                    expected = "assignment base drift" if scenario == "base" else "target drift"
                elif scenario == "symlink":
                    real = root / "src/other.c"
                    real.write_text("other\n", encoding="utf-8")
                    source.unlink()
                    source.symlink_to(real.name)
                    expected = "symlink"
                else:
                    subprocess.run(["git", "add", "src/demo.c"], cwd=root, check=True)
                    expected = "staged path/content drift"
                with mock.patch.object(crew, "repository_root", return_value=expected_root):
                    with self.assertRaisesRegex(crew.CrewError, expected):
                        crew.command_restore_best(
                            argparse.Namespace(worker="worker-1", recover_backup=None), crew_root
                        )

    def test_reproof_is_niced_two_job_and_refreshes_checkpoint(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "src/demo.c"
            source.parent.mkdir(parents=True)
            source.write_bytes(b"best")
            heartbeat = self.schema_four_record(
                best_artifact={"id": "a" * 24, "manifest_sha256": "b" * 64}
            )
            manifest = {
                "source_path": "src/demo.c", "source_sha256": crew.sha256_bytes(b"best")
            }
            completed = subprocess.CompletedProcess(
                args=[], returncode=0, stdout="{}\n", stderr=""
            )
            args = argparse.Namespace(
                worker="worker-1", progress="restored best re-proved",
                attempt_count=3, mismatch_class="allocation",
            )
            with mock.patch.object(crew, "load_heartbeat", return_value=heartbeat):
                with mock.patch.object(crew, "load_best_artifact", return_value=(manifest, {}, root)):
                    with mock.patch.object(crew, "validate_restore_context", return_value=(root, source)):
                        with mock.patch.object(
                            crew, "read_repository_file", return_value=(b"best", 0o644, source)
                        ):
                            with mock.patch.object(crew.subprocess, "run", return_value=completed) as run:
                                with mock.patch.object(crew, "command_heartbeat") as checkpoint:
                                    crew.command_reprove_best(args, root / "crew")
            command = run.call_args.args[0]
            environment = run.call_args.kwargs["env"]
            self.assertEqual(command[:4], ["nice", "-n", "15", "tools/wb_compare.sh"])
            self.assertEqual(environment["MAKEFLAGS"], "-j2")
            self.assertEqual(environment["MICKEY_BUILD_JOBS"], "2")
            checkpoint_args = checkpoint.call_args.args[0]
            self.assertEqual(
                checkpoint_args.wb_summary, "build/wb/" + "a" * 24 + ".restore-reproof-summary.json"
            )

    def test_atomic_write_failure_preserves_previous_record_and_removes_temp(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "worker-1.json"
            path.write_text("old\n", encoding="utf-8")
            with mock.patch.object(os, "replace", side_effect=OSError("interrupted")):
                with self.assertRaises(OSError):
                    crew.atomic_write(path, "new\n")
            self.assertEqual(path.read_text(encoding="utf-8"), "old\n")
            self.assertEqual(list(root.glob(".worker-1.json.*")), [])

    def test_checkpoint_alias_preserves_heartbeat_cli(self) -> None:
        parser = crew.build_parser()
        common = [
            "--worker", "worker-1", "--target", "demo_symbol", "--base", "a" * 40,
            "--deadline-unix", "1787998500", "--progress", "runner launched",
        ]
        heartbeat = parser.parse_args(["heartbeat", *common])
        checkpoint = parser.parse_args(["checkpoint", *common])
        self.assertIs(heartbeat.function, crew.command_heartbeat)
        self.assertIs(checkpoint.function, crew.command_heartbeat)

    def test_lane_wrapper_seeds_and_teaches_structured_checkpoints(self) -> None:
        wrapper = (TOOLS / "codex_lane.sh").read_text(encoding="utf-8")
        self.assertIn("--attempt-count 0", wrapper)
        self.assertIn("--best-score \"not measured\"", wrapper)
        self.assertIn("--mismatch-class unclassified", wrapper)
        self.assertIn("tools/crew.py checkpoint", wrapper)
        self.assertIn("heartbeat-status --worker $name --json --check", wrapper)

    def test_stale_status_gives_guidance_and_never_kills(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.record()) + "\n")
            args = argparse.Namespace(worker=None, stale_after_minutes=1, check=False, json=False)
            now = dt.datetime(2026, 8, 29, 11, 0, tzinfo=dt.timezone.utc)
            output = io.StringIO()
            with mock.patch.object(crew, "utc_now", return_value=now):
                with mock.patch("os.kill") as kill:
                    with contextlib.redirect_stdout(output):
                        crew.command_heartbeat_status(args, root)
            self.assertIn("crew.py never signals or stops a process", output.getvalue())
            kill.assert_not_called()

    def test_stale_check_is_enforcing_but_still_non_destructive(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "heartbeats" / "worker-1.json"
            crew.atomic_write(path, json.dumps(self.record()) + "\n")
            args = argparse.Namespace(worker=None, stale_after_minutes=1, check=True, json=False)
            now = dt.datetime(2026, 8, 29, 11, 0, tzinfo=dt.timezone.utc)
            with mock.patch.object(crew, "utc_now", return_value=now):
                with mock.patch("os.kill") as kill:
                    with contextlib.redirect_stdout(io.StringIO()):
                        with self.assertRaisesRegex(crew.CrewError, "stale heartbeat"):
                            crew.command_heartbeat_status(args, root)
            kill.assert_not_called()


if __name__ == "__main__":
    unittest.main()
