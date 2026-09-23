#!/usr/bin/env python3
"""Compact report safety and bounded subprocess behavior, synthetic only."""
import json
from pathlib import Path
import sys
import tempfile
import time
import unittest
from unittest.mock import patch
from contextlib import redirect_stdout
from io import StringIO

import function_probe as probe


class ProbeTests(unittest.TestCase):
    def evidence(self):
        preflight = {"schema": "mickey-function-evidence-preflight-v1", "owned_size": 16, "resolution_mode": "fallback",
                     "preflight": {"status": "complete", "action": "continue_matching", "counts": {
                         "target_relocations": 1, "candidate_static_relocations": 1,
                         "candidate_identities_resolved": 1, "candidate_identities_unresolved": 0,
                         "offset_type_aligned": 1, "stable_identities_aligned": 1,
                         "effective_identities_aligned": 1}}}
        diagnosis = {"schema": "decomp-workbench-diagnosis-v3", "comparison": {
            "exact": True, "target_instructions": 4, "candidate_instructions": 4, "words": 0,
            "relocation_metadata_mismatches": 0, "relocation_target_mismatches": 0},
            "view": {"instructions": "synthetic private evidence"},
            "lever": {"lever_class": "stack-home", "measurements": {"private": "not printed"}}}
        return preflight, diagnosis

    def test_exact_comparison_is_only_trial_candidate(self):
        report = probe.compact(*self.evidence())
        self.assertTrue(report["candidate_for_linked_trial"])
        self.assertFalse(report["promotion_proof_included"])
        self.assertNotIn("private", json.dumps(report))
        self.assertNotIn("instructions\"", json.dumps(report["mechanism"]))

    def test_partial_relocations_never_pass(self):
        preflight, diagnosis = self.evidence()
        preflight["preflight"]["status"] = "partial"
        self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])

    def test_structural_exact_with_relocation_identity_substitution_fails(self):
        for field in ("relocation_metadata_mismatches", "relocation_target_mismatches"):
            with self.subTest(field=field):
                preflight, diagnosis = self.evidence()
                diagnosis["comparison"][field] = 1
                self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])
                del diagnosis["comparison"][field]
                self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])

    def test_unresolved_unaligned_or_missing_identity_counts_fail(self):
        preflight, _ = self.evidence()
        for field in preflight["preflight"]["counts"]:
            with self.subTest(field=field):
                preflight, diagnosis = self.evidence()
                counts = preflight["preflight"]["counts"]
                counts[field] += 1
                self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])
                del counts[field]
                self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])

    def test_explicit_empty_relocation_surfaces_can_pass(self):
        preflight, diagnosis = self.evidence()
        preflight["preflight"]["counts"] = dict.fromkeys(preflight["preflight"]["counts"], 0)
        self.assertTrue(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])

    def test_already_promoted_source_needs_proof_not_guard_trial(self):
        preflight, diagnosis = self.evidence()
        preflight.update(resolution_mode="post_promotion")
        preflight["preflight"]["action"] = "run_promotion_proof"
        report = probe.compact(preflight, diagnosis)
        self.assertFalse(report["candidate_for_linked_trial"])
        self.assertEqual(report["next_action"], "run_promotion_proof")

    def test_promoted_main_selects_rom_diagnosis_without_promotion(self):
        preflight, diagnosis = self.evidence()
        preflight.update(resolution_mode="post_promotion", source="src/example.c",
                         target_symbol="example", candidate_symbol="example", linked_section=".main")
        preflight["preflight"]["action"] = "run_promotion_proof"
        calls = []
        def phase(command, directory, label, deadline):
            calls.append(command)
            return {"status": "ok"}, preflight if label == "preflight" else diagnosis
        with tempfile.TemporaryDirectory() as temporary, \
             patch.object(probe, "ROOT", Path(temporary)), \
             patch.object(probe, "run_phase", side_effect=phase), redirect_stdout(StringIO()) as output:
            self.assertEqual(probe.main(["example", "--json"]), 0)
        self.assertIn("--rom", calls[1])
        self.assertIn("--no-build", calls[1])
        self.assertFalse(json.loads(output.getvalue())["candidate_for_linked_trial"])

    def test_oversized_and_masked_only_results_never_pass(self):
        preflight, diagnosis = self.evidence()
        diagnosis["comparison"]["candidate_instructions"] = 5
        self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])
        diagnosis["comparison"].update(candidate_instructions=4, exact=False)
        self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])

    def test_exact_with_matching_build_claim_still_passes(self):
        preflight, diagnosis = self.evidence()
        diagnosis["build_provenance"] = {"claim": "match"}
        report = probe.compact(preflight, diagnosis)
        self.assertTrue(report["candidate_for_linked_trial"])
        self.assertEqual(report["build_provenance_claim"], "match")

    def test_exact_with_forced_build_claim_never_passes(self):
        preflight, diagnosis = self.evidence()
        diagnosis["build_provenance"] = {"claim": "reachability-proof"}
        report = probe.compact(preflight, diagnosis)
        self.assertFalse(report["candidate_for_linked_trial"])
        self.assertEqual(report["build_provenance_claim"], "reachability-proof")
        self.assertIn("reachability-proof", report["next_action"])

    def test_exact_with_unverified_build_claim_never_passes(self):
        preflight, diagnosis = self.evidence()
        diagnosis["build_provenance"] = {"claim": "unverified"}
        self.assertFalse(probe.compact(preflight, diagnosis)["candidate_for_linked_trial"])

    def test_exact_without_build_provenance_block_is_unconstrained(self):
        preflight, diagnosis = self.evidence()
        self.assertNotIn("build_provenance", diagnosis)
        report = probe.compact(preflight, diagnosis)
        self.assertTrue(report["candidate_for_linked_trial"])
        self.assertIsNone(report["build_provenance_claim"])

    def test_missing_evidence_never_passes(self):
        report = probe.compact(None, None)
        self.assertFalse(report["candidate_for_linked_trial"])
        self.assertEqual(report["preflight_status"], "unavailable")

    def test_unexpected_schemas_fail_closed(self):
        preflight, diagnosis = self.evidence()
        preflight["schema"] = "future"
        with self.assertRaises(ValueError):
            probe.compact(preflight, diagnosis)

    def phase(self, code, seconds=3):
        with tempfile.TemporaryDirectory(prefix="probe-test-") as temporary:
            root = Path(temporary)
            with patch.object(probe, "ROOT", root):
                return probe.run_phase([sys.executable, "-c", code], root, "fixture", time.monotonic() + seconds)

    def test_real_process_success(self):
        receipt, payload = self.phase('print("{}")')
        self.assertEqual(receipt["status"], "ok")
        self.assertEqual(payload, {})

    def test_nonzero_cannot_supply_evidence(self):
        receipt, payload = self.phase('print("{}"); raise SystemExit(7)')
        self.assertEqual(receipt["returncode"], 7)
        self.assertIsNone(payload)

    def test_malformed_report_fails(self):
        receipt, payload = self.phase('print("not JSON")')
        self.assertEqual(receipt["status"], "invalid-report")
        self.assertIsNone(payload)

    def test_timeout_is_terminal_and_no_evidence(self):
        receipt, payload = self.phase('import time; time.sleep(10)', seconds=.05)
        self.assertEqual(receipt["status"], "timeout")
        self.assertIsNone(payload)

    def test_expired_budget_does_not_launch(self):
        receipt, payload = self.phase('raise AssertionError("must not launch")', seconds=-1)
        self.assertEqual(receipt["status"], "budget-exhausted")
        self.assertIsNone(payload)


if __name__ == "__main__":
    unittest.main()
