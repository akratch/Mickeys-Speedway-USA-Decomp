#!/usr/bin/env python3
"""Synthetic admission regressions: identity authority is separate from shape."""
from __future__ import annotations

import contextlib
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_batch as batch


def site(offset=4, kind=4, namespace=0, destination=128, addend=0,
         route="resident-elf-address", independent=True):
    return {"offset": offset, "rtype": kind, "identity": [namespace, destination + addend],
            "status": "resolved", "observations": [], "conflicts": {}, "addends": [addend],
            "witnesses": [{"route": route, "independent": independent,
                           "base_identity": [namespace, destination]}]}


def surface(*sites, target_count=None, exact=False):
    return {"candidate_record_count": len(sites), "candidate_identity_resolved_count": len(sites),
            "candidate_identity_unresolved_records": [], "diagnostics": {"sites": list(sites)},
            "target_record_count": len(sites) if target_count is None else target_count,
            "offset_type_exact": exact, "stable_identity_exact": exact}


class BindingRecordsTests(unittest.TestCase):
    def test_moved_site_and_count_residuals_are_admissible_not_exact(self):
        report = surface(site(12), site(28), target_count=3)
        self.assertEqual(len(batch.search_binding_records(report)), 2)
        self.assertFalse(report["offset_type_exact"])
        self.assertFalse(report["stable_identity_exact"])

    def test_exact_two_call_control(self):
        self.assertEqual(len(batch.search_binding_records(surface(site(4), site(16), exact=True))), 2)

    def test_annotation_coverage_and_placeholder_values_are_not_candidate_authority(self):
        for correlated in (False, True):
            report = surface(site(route="runtime-hilo-correlation", independent=False))
            report["target_coverage"] = "complete"
            report["placeholder_value"] = 0
            if not correlated:
                report["candidate_identity_resolved_count"] = 0
                report["candidate_identity_unresolved_records"] = [{"offset": 4, "rtype": 4}]
            with self.subTest(correlated=correlated), self.assertRaisesRegex(RuntimeError, "authority|unresolved"):
                batch.search_binding_records(report)

    def test_generated_and_local_name_routes_need_independent_proof(self):
        for route in ("generated-linker-alias", "resident-address-name", "candidate-tu-text-owner"):
            with self.subTest(route=route), self.assertRaisesRegex(RuntimeError, "authority"):
                batch.search_binding_records(surface(site(route=route)))

    def test_type_and_namespace_conflicts_refuse(self):
        wrong_namespace = site()
        wrong_namespace["witnesses"][0]["base_identity"][0] = 2
        conflicting = site()
        conflicting["conflicts"] = {"all_proposals": [[0, 128], [2, 128]]}
        for row in (site(kind=True), wrong_namespace, conflicting):
            with self.subTest(row=row), self.assertRaises(RuntimeError):
                batch.search_binding_records(surface(row))

    def test_rel_addend_must_reproduce_identity(self):
        row = site(kind=5, addend=-8)
        self.assertEqual(len(batch.search_binding_records(surface(row))), 1)
        row["addends"] = [0]
        with self.assertRaisesRegex(RuntimeError, "authority"):
            batch.search_binding_records(surface(row))

    def test_local_section_and_pc16_report_precise_missing_adapter(self):
        local = site()
        local["observations"] = ["object-local-section-symbol"]
        for row in (local, site(kind=10)):
            with self.subTest(row=row), self.assertRaisesRegex(RuntimeError, "unsupported-proof-route"):
                batch.search_binding_records(surface(row))

    def test_missing_diagnostics_unpaired_addend_and_duplicates_refuse(self):
        report = surface(site())
        del report["diagnostics"]
        unpaired = site(kind=5)
        unpaired["addends"] = []
        for value in (report, surface(unpaired), surface(site(), site())):
            with self.subTest(value=value), self.assertRaises(RuntimeError):
                batch.search_binding_records(value)


class AdmissionBoundaryTests(unittest.TestCase):
    def fixture(self, directory, reports, *, changed_fields=False, changed_authority=False):
        stack = contextlib.ExitStack()
        self.addCleanup(stack.close)
        run = directory / "run"
        readiness = run / "baseline-readiness"
        readiness.mkdir(parents=True)
        (run / "baseline-capture").mkdir()
        raw_script = b"#!/bin/sh\n# synthetic original recipe\n"
        prepared_script = raw_script + b"objcopy --redefine-sym external=annotated output.o\n"
        (run / "importer-compile.sh").write_bytes(raw_script)
        (run / "baseline-capture/compile-original.sh").write_bytes(prepared_script)
        inputs = {"context": {"importer_recipe": batch.compile_script_digest(raw_script),
            "prepared": {"compile.sh": batch.compile_script_digest(prepared_script)},
            "search_binding_authority": {"elf": "before"}}}
        source, obj = b"void fixture(void) { external(); }", b"captured synthetic object"
        evidence = SimpleNamespace(source=source, object=obj, source_sha256=hashlib.sha256(source).hexdigest(),
            object_sha256=hashlib.sha256(obj).hexdigest(), prepared_inputs_json=json.dumps(inputs).encode())
        item = batch.QueueItem("fixture", directory / "src/fixture.c")
        def compile(args, deadline):
            Path(args[-1]).write_bytes(b"raw synthetic object")
            return subprocess.CompletedProcess(args, 0, "synthetic compile")
        stack.enter_context(patch.object(batch, "ROOT", directory))
        stack.enter_context(patch.object(batch, "validate_baseline"))
        stack.enter_context(patch.object(batch, "bounded_capture", side_effect=compile))
        stack.enter_context(patch.object(batch, "normalized_owned_instructions",
            side_effect=[b"fields", b"changed" if changed_fields else b"fields"]))
        stack.enter_context(patch.object(batch, "find_asm_target", return_value="fixture.s"))
        elf = stack.enter_context(patch.object(batch.reloc_surface, "Elf"))
        elf.return_value.symbols.return_value = [("fixture", 0, 0, 0, 0)]
        comparison = stack.enter_context(patch.object(batch.reloc_surface, "function_surface_comparison", side_effect=reports))
        stack.enter_context(patch.object(batch, "search_binding_authority",
            side_effect=[{"elf": "before"}, {"elf": "after" if changed_authority else "before"}]))
        return item, evidence, readiness, comparison

    def test_fresh_raw_identity_and_capture_are_both_required(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            item, evidence, directory, compare = self.fixture(root, [surface(site(12)), surface(site(12))])
            report = batch.require_search_bindings(item, evidence, directory, None)
            self.assertEqual(report["status"], "authenticated")
            self.assertEqual(compare.call_count, 2)
            self.assertEqual(compare.call_args.kwargs["candidate_redefine_aliases"], {"annotated": "external"})
            self.assertTrue(compare.call_args.kwargs["measure_size_delta"])
            self.assertEqual(json.loads((directory / "bindings/report.json").read_text())["status"], "authenticated")

    def test_unresolved_raw_identity_refuses_before_annotated_object_can_bless_it(self):
        with tempfile.TemporaryDirectory() as tmp:
            unresolved = surface(site(route="runtime-site-correlation", independent=False))
            item, evidence, directory, compare = self.fixture(Path(tmp), [unresolved, surface(site())])
            with self.assertRaisesRegex(RuntimeError, "readiness refused.*authority"):
                batch.require_search_bindings(item, evidence, directory, None)
            self.assertEqual(compare.call_count, 1)
            self.assertEqual(json.loads((directory / "bindings/report.json").read_text())["status"], "unverifiable")

    def test_changed_capture_type_or_binding_refuses(self):
        for changed in (site(kind=6), site(namespace=2)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                item, evidence, directory, _ = self.fixture(Path(tmp), [surface(site()), surface(changed)])
                with self.assertRaisesRegex(RuntimeError, "annotation changed"):
                    batch.require_search_bindings(item, evidence, directory, None)

    def test_instruction_fidelity_and_authority_drift_fail_closed(self):
        for kwargs, reason in (({"changed_fields": True}, "executable fields"),
                               ({"changed_authority": True}, "authority changed")):
            with self.subTest(kwargs=kwargs), tempfile.TemporaryDirectory() as tmp:
                item, evidence, directory, _ = self.fixture(Path(tmp), [surface(site()), surface(site())], **kwargs)
                with self.assertRaisesRegex(RuntimeError, reason):
                    batch.require_search_bindings(item, evidence, directory, None)

    def test_unsupported_route_is_retained_in_report(self):
        with tempfile.TemporaryDirectory() as tmp:
            item, evidence, directory, _ = self.fixture(Path(tmp), [surface(site(kind=10))])
            with self.assertRaises(RuntimeError):
                batch.require_search_bindings(item, evidence, directory, None)
            report = json.loads((directory / "bindings/report.json").read_text())
            self.assertEqual(report["unsupported_proof_route"], "pc16-owned-branch")

    def test_early_full_tu_adapter_failure_retains_route(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            item = batch.QueueItem("fixture", root / "src/fixture.c")
            with patch.object(batch, "_grouped_baseline_fidelity",
                    side_effect=batch.UnsupportedSearchBinding("section-local-data-owner")):
                with self.assertRaises(batch.UnsupportedSearchBinding):
                    batch.grouped_baseline_fidelity(item, root, root, {}, None)
            report = json.loads((root / "source-fidelity/binding-readiness.json").read_text())
            self.assertEqual(report["unsupported_proof_route"], "section-local-data-owner")


if __name__ == "__main__":
    unittest.main()
