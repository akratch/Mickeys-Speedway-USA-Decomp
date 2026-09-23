#!/usr/bin/env python3
"""Focused integration tests for wb_compare.sh routing and option forwarding."""

from __future__ import annotations

import os
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class WrapperFixture:
    def __init__(self, root: pathlib.Path) -> None:
        self.root = root
        tools = root / "tools"
        binutils = tools / "binutils"
        binutils.mkdir(parents=True)
        (root / ".venv" / "bin").mkdir(parents=True)
        (root / "build_non_matching/src/overlays/o016").mkdir(parents=True)
        (root / "build").mkdir()
        (root / "src/overlays/o016").mkdir(parents=True)
        asm = root / "asm/nonmatchings/overlays/o016/example/func_overlay_016_F0000000_1.s"
        asm.parent.mkdir(parents=True)
        asm.write_text("glabel func_overlay_016_F0000000_1\n", encoding="utf-8")
        (root / "src/overlays/o016/example.c").write_text(
            "void friendly(void) {}\n", encoding="utf-8"
        )
        (root / "build_non_matching/src/overlays/o016/example.c.o").write_bytes(b"obj")
        (root / "build/mickey.us.elf").write_bytes(b"elf")

        shutil.copy2(ROOT / "tools/wb_compare.sh", tools / "wb_compare.sh")
        os.symlink(sys.executable, root / ".venv/bin/python")
        (tools / "function_preflight.py").write_text(
            "import hashlib, json, os, pathlib, sys\n"
            "class PreflightError(RuntimeError): pass\n"
            "def workbench_summary(raw, manifest, **fields):\n"
            "    payload = json.loads(raw.read_text())\n"
            "    proof = json.loads(manifest.read_text())\n"
            "    if payload.get('schema') != 'decomp-workbench-comparison-v1':\n"
            "        raise PreflightError('unexpected workbench schema '"
            "+repr(payload.get('schema')))\n"
            "    target = payload['target_instructions']\n"
            "    candidate = payload['candidate_instructions']\n"
            "    words = payload['word_mismatches']\n"
            "    boundary = fields['boundary_size'] or target * 4\n"
            "    return {'schema': 'mickey-wb-summary-v1',"
            "'symbol': {'requested': fields['requested_symbol'],"
            "'target': fields['target_symbol'], 'candidate': fields['candidate_symbol']},"
            "'mode': fields['comparison_mode'],"
            "'boundary': {'bytes': boundary, 'evidence': fields['boundary_evidence']},"
            "'comparison': {'target_words': target, 'candidate_words': candidate,"
            "'matched_words': target - words if target == candidate else None,"
            "'differing_words': words,"
            "'first_mismatch_offset': None if payload['first_divergent_row'] is None "
            "else payload['first_divergent_row'] * 4,"
            "'target_frame_bytes': None if payload['target_frame_size'] is None "
            "else abs(payload['target_frame_size']),"
            "'exact': payload['exact']},"
            "'provenance': {'exact_claim_allowed': proof['exact_claim_allowed']},"
            "'evidence': {'admissible_exact_comparison': payload['accepted'] and "
            "proof['exact_claim_allowed'], 'promotion_proof_included': False},"
            "'raw': hashlib.sha256(raw.read_bytes()).hexdigest()}\n"
            "if __name__ == '__main__':\n"
            "    if os.environ.get('PREFLIGHT_ARGS_OUT'):\n"
            "        pathlib.Path(os.environ['PREFLIGHT_ARGS_OUT']).write_text("
            "'\\n'.join(sys.argv[1:]))\n"
            "    if '--resolve-rom' in sys.argv:\n"
            "        print('friendly\\t00000000\\t000000AC\\t.text\\t'"
            "+'preflight-owned-boundary')\n"
            "    else:\n"
            "        print('func_overlay_016_F0000000_1\\tfriendly\\t'"
            "+'src/overlays/o016/example.c\\toverlays/o016/example\\t'"
            "+'build_non_matching\\tasm/nonmatchings/overlays/o016/example/'"
            "+'func_overlay_016_F0000000_1.s')\n",
            encoding="utf-8",
        )
        (tools / "proof_provenance.py").write_text(
            "raise SystemExit(0)\n", encoding="utf-8"
        )
        self._executable(
            binutils / "mips64-elf-objdump",
            "#!/bin/sh\n"
            "if [ \"$1\" = -h ]; then\n"
            "  printf '%s\\n' '  0 .text 000000ac 00000000 00000000'\n"
            "elif [ \"$1\" = -t ]; then\n"
            "  printf '%s\\n' '00000000 g F .text 000000ac friendly'\n"
            "else\n"
            "  printf '%s\\n' '00000000 g F .text 000000ac friendly'\n"
            "fi\n",
        )
        self._executable(
            binutils / "mips64-elf-as",
            "#!/bin/sh\nwhile [ \"$1\" != -o ]; do shift; done\n"
            "shift\n: > \"$1\"\n",
        )
        self._executable(binutils / "mips64-elf-objcopy", "#!/bin/sh\nexit 0\n")
        self._executable(
            root / ".venv/bin/decomp-workbench",
            "#!/bin/sh\nprintf '%s\\n' \"$@\" > \"$WB_ARGS_OUT\"\n",
        )

    @staticmethod
    def _executable(path: pathlib.Path, text: str) -> None:
        path.write_text(text, encoding="utf-8")
        path.chmod(0o755)

    def configure_summary(self, *, exact: bool = False) -> None:
        payload = {
            "schema": "decomp-workbench-comparison-v1",
            "exact": False,
            "accepted": False,
            "acceptance_basis": "mismatch",
            "verdict": "register-mismatch",
            "target_instructions": 43,
            "candidate_instructions": 43,
            "instruction_delta": 0,
            "word_mismatches": 3,
            "raw_word_mismatches": 4,
            "normalized_distance": 3,
            "opcode_mismatches": 1,
            "register_mismatches": 2,
            "fp_register_mismatches": 0,
            "aligned_total": 3,
            "aligned_structural": 0,
            "aligned_schedule": 1,
            "aligned_register": 2,
            "aligned_constant": 0,
            "first_divergent_row": 2,
            "target_frame_size": -32,
            "candidate_frame_size": -32,
            "relocation_metadata_mismatches": 0,
            "relocation_target_mismatches": 0,
            "diff_sites": [{"target_word": "not summary safe"}],
        }
        if exact:
            payload.update(
                exact=True,
                accepted=True,
                acceptance_basis="function-exact",
                verdict="exact",
                word_mismatches=0,
                raw_word_mismatches=0,
                normalized_distance=0,
                opcode_mismatches=0,
                register_mismatches=0,
                aligned_total=0,
                aligned_schedule=0,
                aligned_register=0,
                first_divergent_row=None,
                target_frame_size=None,
                candidate_frame_size=None,
            )
        self._executable(
            self.root / ".venv/bin/decomp-workbench",
            "#!/bin/sh\n"
            "printf '%s\\n' \"$@\" > \"$WB_ARGS_OUT\"\n"
            f"printf '%s\\n' '{json.dumps(payload, sort_keys=True)}'\n",
        )
        (self.root / "tools/proof_provenance.py").write_text(
            "import json, pathlib, sys\n"
            "manifest = pathlib.Path(sys.argv[sys.argv.index('--manifest') + 1])\n"
            "record = {'sha256': 'a' * 64}\n"
            "manifest.write_text(json.dumps({"
            "'schema': 'mickey-wb-proof-provenance-v1',"
            "'selection': {'classification': 'non_matching_candidate'},"
            "'exact_claim_allowed': True, 'verdict': 'c_evidence',"
            "'source': record, 'candidate_object': record, 'target_object': record"
            "}))\n",
            encoding="utf-8",
        )


class WrapperRoutingTests(unittest.TestCase):
    def test_summary_json_is_stable_concise_and_provenance_guarded(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            wrapper = WrapperFixture(fixture)
            wrapper.configure_summary()
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--summary-json",
                    "friendly",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(1, len(result.stdout.splitlines()))
            report = json.loads(result.stdout)
            self.assertEqual("mickey-wb-summary-v1", report["schema"])
            self.assertEqual("friendly", report["symbol"]["candidate"])
            self.assertEqual(43, report["comparison"]["target_words"])
            self.assertEqual(40, report["comparison"]["matched_words"])
            self.assertEqual(8, report["comparison"]["first_mismatch_offset"])
            self.assertEqual(32, report["comparison"]["target_frame_bytes"])
            self.assertTrue(report["provenance"]["exact_claim_allowed"])
            self.assertFalse(report["evidence"]["admissible_exact_comparison"])
            self.assertFalse(report["evidence"]["promotion_proof_included"])
            self.assertNotIn("diff_sites", result.stdout)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertIn("--json", arguments)
            self.assertIn("--color", arguments)
            self.assertIn("never", arguments)

    def test_exact_summary_accepts_null_mismatch_and_frame_fields(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            wrapper = WrapperFixture(fixture)
            wrapper.configure_summary(exact=True)
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(fixture / "args.txt")

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--summary-json",
                    "friendly",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads(result.stdout)
            self.assertIsNone(report["comparison"]["first_mismatch_offset"])
            self.assertIsNone(report["comparison"]["target_frame_bytes"])
            self.assertTrue(report["evidence"]["admissible_exact_comparison"])

    def test_summary_json_rejects_an_unexpected_workbench_schema(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            wrapper = WrapperFixture(fixture)
            wrapper.configure_summary()
            WrapperFixture._executable(
                fixture / ".venv/bin/decomp-workbench",
                "#!/bin/sh\nprintf '%s\\n' '{\"schema\":\"future-schema\"}'\n",
            )
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(fixture / "args.txt")

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--summary-json",
                    "friendly",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 2)
            self.assertIn("unexpected workbench schema", result.stderr)
            self.assertNotIn("diff_sites", result.stderr)

    def test_summary_json_is_not_available_for_diagnosis_reports(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--diagnose",
                    "--summary-json",
                    "friendly",
                ],
                cwd=fixture,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 2)
            self.assertIn("not diagnosis reports", result.stderr)

    def test_json_stdout_is_not_prefixed_by_provenance_receipts(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            (fixture / "tools/proof_provenance.py").write_text(
                "print('proof receipt')\n", encoding="utf-8"
            )
            WrapperFixture._executable(
                fixture / ".venv/bin/decomp-workbench",
                "#!/bin/sh\nprintf '%s\\n' '{\"schema\":\"comparison\"}'\n",
            )
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(fixture / "args.txt")

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, '{"schema":"comparison"}\n')
            self.assertIn("proof receipt", result.stderr)

    def test_resolves_friendly_symbol_and_nonmatching_tree_automatically(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)
            preflight_args = fixture / "preflight-args.txt"
            env["PREFLIGHT_ARGS_OUT"] = str(preflight_args)

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertEqual(arguments[0], "compare")
            self.assertIn(
                "build_non_matching/src/overlays/o016/example.c.o", arguments
            )
            self.assertIn("--function", arguments)
            self.assertIn("friendly", arguments)
            self.assertEqual(arguments[-3], "--json")
            self.assertEqual(arguments[-2:], ["--build-env", "stock"])
            self.assertEqual(
                ["friendly", "--resolve-wb"],
                preflight_args.read_text(encoding="utf-8").splitlines(),
            )

    def test_caller_declared_build_env_is_not_overridden(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "friendly",
                    "--json",
                    "--build-env",
                    "forced",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertEqual(arguments[-2:], ["--build-env", "forced"])
            self.assertEqual(arguments.count("--build-env"), 1)

    def test_scratch_candidate_symbol_env_skips_auto_stock_declaration(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)
            env["WB_CANDIDATE_SYMBOL"] = "friendly"

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertNotIn("--build-env", arguments)

    def test_forcing_environment_variable_without_declaration_is_refused(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)
            env["CDX_LOG"] = "trace.log"

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 2)
            self.assertIn("CDX_LOG", result.stderr)
            self.assertIn("--build-env forced", result.stderr)
            self.assertFalse(args_out.exists())

    def test_forcing_environment_variable_with_explicit_declaration_proceeds(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)
            env["CDX_LOG"] = "trace.log"

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "friendly",
                    "--json",
                    "--build-env",
                    "forced",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertEqual(arguments[-2:], ["--build-env", "forced"])

    def test_no_build_is_forwarded_to_preflight_not_workbench(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            preflight_args = fixture / "preflight-args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)
            env["PREFLIGHT_ARGS_OUT"] = str(preflight_args)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--no-build",
                    "friendly",
                    "--json",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(
                ["friendly", "--resolve-wb", "--no-build"],
                preflight_args.read_text(encoding="utf-8").splitlines(),
            )
            self.assertNotIn(
                "--no-build", args_out.read_text(encoding="utf-8").splitlines()
            )

    def test_diagnose_mode_forwards_diagnostic_options_after_symbol(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--diagnose",
                    "func_overlay_016_F0000000_1",
                    "--trace",
                    "trace.log",
                    "--trace-proc",
                    "3",
                    "--terse",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            arguments = args_out.read_text(encoding="utf-8").splitlines()
            self.assertEqual(arguments[0], "diagnose")
            tail = arguments[arguments.index("--objdump") + 2 :]
            self.assertEqual(
                tail,
                [
                    "--trace", "trace.log", "--trace-proc", "3", "--terse",
                    "--build-env", "stock",
                ],
            )

    def test_preflight_freshness_failure_stops_before_workbench(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            (fixture / "tools/function_preflight.py").write_text(
                "import sys\n"
                "print('stale candidate object', file=sys.stderr)\n"
                "raise SystemExit(2)\n",
                encoding="utf-8",
            )
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [str(fixture / "tools/wb_compare.sh"), "friendly", "--json"],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 2)
            self.assertIn("stale candidate object", result.stderr)
            self.assertFalse(args_out.exists())

    def test_rom_no_build_rejects_same_tick_artifacts(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            WrapperFixture(fixture)
            candidate_rom = fixture / "build/mickey.us.z64"
            candidate_rom.write_bytes(b"rom")
            elf = fixture / "build/mickey.us.elf"
            timestamp_ns = max(elf.stat().st_mtime_ns, candidate_rom.stat().st_mtime_ns)
            os.utime(elf, ns=(timestamp_ns, timestamp_ns))
            os.utime(candidate_rom, ns=(timestamp_ns, timestamp_ns))
            args_out = fixture / "args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--rom",
                    "--no-build",
                    "friendly",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 2)
            self.assertIn("is not strictly newer", result.stderr)
            self.assertIn("omit --no-build", result.stderr)
            self.assertFalse(args_out.exists())

    def test_rom_summary_uses_preflight_boundary_without_size_annotation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = pathlib.Path(directory)
            wrapper = WrapperFixture(fixture)
            wrapper.configure_summary()
            candidate_rom = fixture / "build/mickey.us.z64"
            candidate_rom.write_bytes(b"rom")
            (fixture / "baseroms").mkdir()
            (fixture / "baseroms/mickey.us.z64").write_bytes(b"target")
            elf = fixture / "build/mickey.us.elf"
            elf_time = elf.stat().st_mtime_ns
            os.utime(candidate_rom, ns=(elf_time + 1_000_000, elf_time + 1_000_000))
            (fixture / "symbol_addrs.us.txt").write_text("", encoding="utf-8")
            args_out = fixture / "args.txt"
            preflight_args = fixture / "preflight-args.txt"
            env = os.environ.copy()
            env["WB_ARGS_OUT"] = str(args_out)
            env["PREFLIGHT_ARGS_OUT"] = str(preflight_args)

            result = subprocess.run(
                [
                    str(fixture / "tools/wb_compare.sh"),
                    "--rom",
                    "--no-build",
                    "--summary-json",
                    "friendly",
                ],
                cwd=fixture,
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )

            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads(result.stdout)
            self.assertEqual("rom", report["mode"])
            self.assertEqual(0xAC, report["boundary"]["bytes"])
            self.assertEqual(
                "preflight-owned-boundary", report["boundary"]["evidence"]
            )
            self.assertEqual(
                ["friendly", "--resolve-rom", "--no-build"],
                preflight_args.read_text(encoding="utf-8").splitlines(),
            )


if __name__ == "__main__":
    unittest.main()
