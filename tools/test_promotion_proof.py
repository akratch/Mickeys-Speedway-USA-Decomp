#!/usr/bin/env python3
"""Focused tests for the compact post-promotion proof policy."""

from __future__ import annotations

import copy
import contextlib
import io
import json
import subprocess
import sys
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
import promotion_proof as proof  # noqa: E402


def exact_report() -> dict[str, object]:
    return {
        "schema": "mickey-function-evidence-preflight-v1",
        "requested_symbol": "friendly",
        "candidate_symbol": "friendly",
        "linked_symbol": "friendly",
        "resolution_mode": "post_promotion",
        "preflight": {"status": "complete"},
        "workbench": {
            "comparison_mode": "rom",
            "differing_words": 0,
            "target_words": 32,
            "candidate_words": 32,
            "first_mismatch": None,
            "verdict": "exact",
            "target_frame": 48,
            "candidate_frame": 48,
        },
        "relocation_comparison": {
            "target_record_count": 3,
            "candidate_record_count": 3,
            "offset_type_exact": True,
            "effective_identity_alignment_count": 3,
            "effective_identity_exact": True,
            "identity_proof_mode": "static-plus-runtime-table-and-linked-rom",
        },
    }


def module_link_report():
    report = exact_report()
    raw = copy.deepcopy(report["relocation_comparison"])
    raw.update(stable_identity_exact=True, candidate_identity_resolved_count=3)
    digests = [str(i) * 64 for i in range(1, 9)]
    inputs = {"command": "tools/ido/cc source.c", "module_link": "tools/link_overlay_module.py object.o",
              "configured": digests[0], "linked": digests[1],
              "module_link_inputs": {name: digests[0] for name in (
                  "config/runlink-ort.us.txt", "tools/link_overlay_module.py", "tools/runlink_pack.py",
                  "tools/rebind_elf_relocations.py", "tools/binutils/mips64-elf-ld")}}
    receipt = {"schema": "mickey-source-module-link-proof-v1", "source_selection": "ordinary_c",
               "raw_count": 3, "retained_count": 0, "module_relocations": 3,
               "raw_sha256": digests[2], "configured_sha256": digests[0],
               "linked_sha256": digests[1], "replayed_sha256": digests[3], "inputs": inputs,
               "allocated_sections_exact": True, "symbols_exact": True, "module_body_exact": True,
               "identities": [{"offset": off, "rtype": 4, "identity": [1, off]} for off in (0, 4, 8)],
               "link": {"schema": "mickey-source-module-link-v1", "compiler_sha256": digests[2],
                        "configured_sha256": digests[3], "linked_sha256": digests[4],
                        "prepared_sha256": digests[5], "script_sha256": digests[6],
                        "localized_sha256": digests[7],
                        "container_conversion": "zero-based-exec-to-rel-header-only",
                        "commands": [["mips64-elf-ld", "--no-check-sections", "-T", "module.ld"]]}}
    report["relocation_comparison"].update(candidate_record_count=0, original_raw_comparison=raw,
        identity_proof_mode="raw-static-with-source-module-link", source_module_link_proof=receipt)
    return report


class ModuleLinkTests(unittest.TestCase):
    def test_fresh_module_route_retains_literal_configured_count(self):
        result = proof.validate_report("friendly", module_link_report())
        self.assertEqual(0, result["configured_relocations"])
        self.assertEqual(3, result["exact_relocations"])
        self.assertEqual("raw-static-with-source-module-link", result["identity_proof_mode"])

    def test_stripped_object_or_incomplete_replay_is_not_a_module_proof(self):
        mutations = [
            lambda r: r["relocation_comparison"].pop("source_module_link_proof"),
            lambda r: r["relocation_comparison"]["original_raw_comparison"].update(stable_identity_exact=False),
            lambda r: r["relocation_comparison"]["source_module_link_proof"].update(module_body_exact=False),
            lambda r: r["relocation_comparison"]["source_module_link_proof"]["link"].update(commands=[]),
            lambda r: r["relocation_comparison"]["source_module_link_proof"]["link"].update(compiler_sha256="0" * 64),
            lambda r: r["relocation_comparison"]["source_module_link_proof"]["inputs"].update(module_link_inputs={}),
        ]
        for mutate in mutations:
            report = module_link_report()
            mutate(report)
            with self.subTest(mutate=mutate), self.assertRaises(proof.ProofError):
                proof.validate_report("friendly", report)


class ReportPolicyTests(unittest.TestCase):
    def test_exact_post_promotion_report_becomes_compact_receipt(self) -> None:
        receipt = proof.validate_report("friendly", exact_report())

        self.assertEqual("mickey-promotion-proof-v1", receipt["schema"])
        self.assertEqual("exact", receipt["verdict"])
        self.assertEqual(32, receipt["exact_words"])
        self.assertEqual(48, receipt["frame_size"])
        self.assertEqual(3, receipt["exact_relocations"])

    def test_leaf_function_with_no_frame_or_relocations_can_be_exact(self) -> None:
        report = exact_report()
        report["workbench"]["target_frame"] = None
        report["workbench"]["candidate_frame"] = None
        report["relocation_comparison"].update(
            {
                "target_record_count": 0,
                "candidate_record_count": 0,
                "effective_identity_alignment_count": 0,
                "identity_proof_mode": "static",
            }
        )

        receipt = proof.validate_report("friendly", report)

        self.assertIsNone(receipt["frame_size"])
        self.assertEqual(0, receipt["exact_relocations"])

    def test_workbench_native_exact_word_verdict_is_accepted(self) -> None:
        report = exact_report()
        report["workbench"]["verdict"] = "instruction-words-identical"
        self.assertEqual("exact", proof.validate_report("friendly", report)["verdict"])

    def test_fallback_mode_is_not_a_promotion_proof(self) -> None:
        report = exact_report()
        report["resolution_mode"] = "fallback"
        with self.assertRaisesRegex(proof.ProofError, "not post-promotion"):
            proof.validate_report("friendly", report)

    def test_partial_preflight_is_not_a_promotion_proof(self) -> None:
        report = exact_report()
        report["preflight"]["status"] = "partial"
        with self.assertRaisesRegex(proof.ProofError, "not complete"):
            proof.validate_report("friendly", report)

    def test_every_exactness_surface_fails_closed(self) -> None:
        mutations = (
            ("linked words", lambda row: row["workbench"].update(differing_words=1)),
            (
                "exact-word verdict",
                lambda row: row["workbench"].update(verdict="register-mismatch"),
            ),
            ("frame", lambda row: row["workbench"].update(candidate_frame=32)),
            (
                "relocation counts",
                lambda row: row["relocation_comparison"].update(candidate_record_count=2),
            ),
            (
                "offsets/types",
                lambda row: row["relocation_comparison"].update(offset_type_exact=False),
            ),
            (
                "identities",
                lambda row: row["relocation_comparison"].update(
                    effective_identity_exact=False
                ),
            ),
        )
        for message, mutate in mutations:
            with self.subTest(message=message):
                report = copy.deepcopy(exact_report())
                mutate(report)
                with self.assertRaisesRegex(proof.ProofError, message):
                    proof.validate_report("friendly", report)


class CommandTests(unittest.TestCase):
    def test_preflight_command_forwards_no_build_and_parses_json(self) -> None:
        calls: list[tuple[list[str], dict[str, object]]] = []

        def runner(command: list[str], **kwargs: object) -> subprocess.CompletedProcess[str]:
            calls.append((command, kwargs))
            return subprocess.CompletedProcess(command, 0, json.dumps(exact_report()), "")

        report = proof.run_preflight("friendly", no_build=True, runner=runner)

        self.assertEqual(exact_report(), report)
        self.assertEqual(
            [sys.executable, str(proof.PREFLIGHT), "friendly", "--json", "--no-build"],
            calls[0][0],
        )
        self.assertEqual(proof.REPO, calls[0][1]["cwd"])

    def test_canonical_commands_are_explicitly_niced_and_two_job(self) -> None:
        commands = [command for _label, command in proof.canonical_commands()]

        self.assertEqual(
            [
                ["nice", "-n", "10", "gmake", "-j2", "verify"],
                ["nice", "-n", "10", "gmake", "-j2", "check-overlay-syms"],
            ],
            commands,
        )

    def test_canonical_proofs_stop_on_first_failure(self) -> None:
        calls: list[list[str]] = []

        def runner(command: list[str], **_kwargs: object) -> subprocess.CompletedProcess[str]:
            calls.append(command)
            return subprocess.CompletedProcess(command, 1)

        with contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(proof.ProofError, "full-ROM identity"):
                proof.run_canonical_proofs(runner=runner)
        self.assertEqual(1, len(calls))


if __name__ == "__main__":
    unittest.main()
