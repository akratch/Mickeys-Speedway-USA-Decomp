#!/usr/bin/env python3
"""Synthetic metadata accounting tests; no target-derived fixtures."""
import contextlib
import copy
import hashlib
import re
import sys
import subprocess
import tempfile
import unittest
from unittest import mock
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import proof_provenance as pp


class ElfFixture:
    def __init__(self):
        self.names = ["", ".text", ".bss"]
        self.sh = [(0,) * 10, (0, 1, 6, 0, 0, 16, 0, 0, 4, 0),
                   (0, 8, 3, 0, 0, 16, 0, 0, 4, 0)]
        self.syms = [("f", 0, 12, 18, 1), (".bss", 0, 0, 3, 2),
                     ("datum", 4, 4, 17, 2), ("callee", 0, 0, 16, 0)]
        self.rels = [(".text", 0, 5, 1), (".text", 4, 6, 1), (".text", 8, 4, 3)]
        self.text = b"test" * 3 + bytes(4)

    def section(self, name):
        if name not in self.names:
            return None, None
        i = self.names.index(name)
        return i, self.sh[i]

    def section_bytes(self, name):
        return self.text if name == ".text" else b""

    def symbols(self):
        return self.syms

    def relocations(self, target=r"\.text"):
        return [row for row in self.rels if re.fullmatch(target, row[0])]


class MetadataTests(unittest.TestCase):
    def setUp(self):
        self.raw = ElfFixture()
        self.configured = copy.deepcopy(self.raw)
        self.configured.rels = self.configured.rels[2:]
        self.plan = [("filter", [(0, 5, ".bss"), (4, 6, ".bss")])]

    def test_exact_accounting(self):
        result = pp.validate_metadata_objects(self.raw, self.configured, self.plan, "f")
        self.assertEqual((result["raw_count"], result["retained_count"]), (3, 1))

    def test_bad_filter_and_duplicate(self):
        for specs in [[(0, 6, ".bss")], [(0, 5, "other")], [(12, 5, ".bss")],
                      [(0, 5, ".bss"), (0, 5, ".bss")]]:
            with self.subTest(specs=specs), self.assertRaises(pp.MetadataProofError):
                pp.validate_metadata_objects(self.raw, self.configured, [("filter", specs)], "f")

    def test_instruction_and_relocation_changes(self):
        for mutation in ("instruction", "removed", "added", "retargeted", "retained_filter"):
            obj = copy.deepcopy(self.configured)
            if mutation == "instruction":
                obj.text = b"X" + obj.text[1:]
            elif mutation == "removed":
                obj.rels = []
            elif mutation == "added":
                obj.rels.append((".text", 12, 4, 3))
            elif mutation == "retargeted":
                obj.rels = [(".text", 8, 4, 1)]
            else:
                obj.rels = self.raw.rels
            with self.subTest(mutation=mutation), self.assertRaises(pp.MetadataProofError):
                pp.validate_metadata_objects(self.raw, obj, self.plan, "f")

    def test_trim_cannot_remove_owned_or_nonzero(self):
        for end in (8, 15):
            raw = copy.deepcopy(self.raw)
            raw.text = b"x" * 16
            with self.assertRaises(pp.MetadataProofError):
                pp.validate_metadata_objects(raw, self.configured, self.plan + [("trim", end)], "f")

    def test_text_header_and_duplicate_section_changes(self):
        for index in (1, 2, 3, 8):
            bad = copy.deepcopy(self.configured)
            header = list(bad.sh[1]); header[index] += 1; bad.sh[1] = tuple(header)
            with self.subTest(index=index), self.assertRaises(pp.MetadataProofError):
                pp.validate_metadata_objects(self.raw, bad, self.plan, "f")
        bad = copy.deepcopy(self.configured)
        bad.names.append(".text"); bad.sh.append(bad.sh[1])
        with self.assertRaises(pp.MetadataProofError):
            pp.validate_metadata_objects(self.raw, bad, self.plan, "f")

    def test_trim_cannot_remove_zero_neighbor(self):
        raw, configured = copy.deepcopy(self.raw), copy.deepcopy(self.configured)
        raw.syms.append(("neighbor", 12, 4, 18, 1))
        configured.syms.append(raw.syms[-1])
        configured.text = configured.text[:12]
        with self.assertRaisesRegex(pp.MetadataProofError, "neighboring"):
            pp.validate_metadata_objects(raw, configured, self.plan + [("trim", 12)], "f")

    def test_bss_unique_named_witness(self):
        linked = copy.deepcopy(self.raw)
        linked.names[2] = ".overlay_003_bss"
        linked.sh[2] = (0, 8, 3, 0x1000, 0, 32, 0, 0, 4, 0)
        linked.syms[2] = ("datum", 0x1008, 4, 17, 2)
        self.assertEqual(pp.linked_bss_base(self.raw, self.configured, linked, 3, 0x80), (3, 0x84))
        linked.syms.append(("datum", 4, 4, 17, 0xfff1))
        self.assertEqual(pp.linked_bss_base(self.raw, self.configured, linked, 3, 0x80), (3, 0x84))
        for mutation in ("duplicate", "size", "owner", "escape"):
            bad = copy.deepcopy(linked)
            if mutation == "duplicate":
                bad.syms.append(bad.syms[2])
            elif mutation == "size":
                bad.syms[2] = ("datum", 0x1008, 8, 17, 2)
            elif mutation == "owner":
                bad.names[2] = ".overlay_004_bss"
            else:
                bad.syms[2] = ("datum", 0x1040, 4, 17, 2)
            with self.subTest(mutation=mutation), self.assertRaises(pp.MetadataProofError):
                pp.linked_bss_base(self.raw, self.configured, bad, 3, 0x80)

    def test_parser_is_narrow(self):
        command = "$(OBJCOPY) --redefine-sym a=b $@ && $(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text 0:5:.bss"
        self.assertEqual(pp.metadata_filter_plan(command, "x.o")[-1], ("filter", [(0, 5, ".bss")]))
        for bad in (command + " ; true", command.replace("0:5", "1:5"), command.replace("--redefine-sym", "--strip-all")):
            with self.assertRaises(pp.MetadataProofError):
                pp.metadata_filter_plan(bad, "x.o")


class CaptureTests(unittest.TestCase):
    def test_current_capture_and_changed_inputs(self):
        import permute_batch as batch
        for mutation in (None, "source", "header", "wrapper_header", "configured", "linked", "tool", "recipe", "failure", "timeout", "cancel"):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                paths = {"source": root / "src/a.c", "header": root / "include/a.h",
                         "wrapper_header": root / "src/local.h",
                         "configured": root / "build/src/a.c.o", "linked": root / "build/a.elf",
                         "tool": root / "tools/filter_elf_relocations.py"}
                for path in [*paths.values(), root / "Makefile", root / "baseroms/mickey.us.z64",
                             root / "config/overlays.us.json", root / "overlay_undefined_syms.us.txt", root / "symbol_addrs.us.txt",
                             root / "tools/asm-processor/build.py", root / "tools/trim_elf_section.py",
                             root / "tools/binutils/mips64-elf-objcopy"]:
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text("initial")
                paths["source"].write_text('#include "a.h"\n#include <local.h>\nvoid f(void) {}\n')
                target = "build/src/a.c.o"
                compiler = f".venv/bin/python tools/asm-processor/build.py tools/ido/cc -- tools/binutils/mips64-elf-as -- -nostdinc -I include -c -o {target} src/a.c"
                postprocess = f".venv/bin/python tools/filter_elf_relocations.py {target} .text 0:5:.bss"
                changed_recipe = False
                def execute(argv, deadline, check):
                    nonlocal changed_recipe
                    if argv[0] == "gmake":
                        return subprocess.CompletedProcess(argv, 0, compiler + "\n" + postprocess + (" extra" if changed_recipe else ""))
                    if mutation == "failure":
                        raise subprocess.CalledProcessError(7, argv, output="compile failed")
                    if mutation == "timeout":
                        raise subprocess.TimeoutExpired(argv, 1, output="partial compile")
                    if mutation == "cancel":
                        raise KeyboardInterrupt("cancelled compile")
                    Path(argv[argv.index("-o") + 1]).write_bytes(b"fresh raw object")
                    if mutation in paths:
                        paths[mutation].write_text("changed")
                    if mutation == "recipe":
                        changed_recipe = True
                    return subprocess.CompletedProcess(argv, 0, "compiled")
                with mock.patch.object(batch, "ROOT", root), mock.patch.object(batch, "bounded_capture", side_effect=execute), \
                     mock.patch.object(batch, "checked_tool_identity"), mock.patch.object(batch, "sweep_tool_identity", return_value={"compiler": "pinned"}):
                    if mutation is None:
                        raw, _plan, receipt, context = pp.capture_configured_raw(root, paths["source"], paths["configured"], paths["linked"], postprocess)
                        self.assertEqual(raw.read_bytes(), b"fresh raw object")
                        self.assertEqual(receipt["inputs"], context())
                    elif mutation in ("failure", "timeout", "cancel"):
                        expected = {"failure": subprocess.CalledProcessError, "timeout": subprocess.TimeoutExpired,
                                    "cancel": KeyboardInterrupt}[mutation]
                        with self.assertRaises(expected):
                            pp.capture_configured_raw(root, paths["source"], paths["configured"], paths["linked"], postprocess)
                        self.assertIn("compile", next((root / "build/metadata-filter-proof").glob("*/compile.log")).read_text())
                    else:
                        with self.assertRaises(pp.MetadataProofError):
                            pp.capture_configured_raw(root, paths["source"], paths["configured"], paths["linked"], postprocess)


class ReportTests(unittest.TestCase):
    def test_loaded_implementations_and_parsed_snapshot(self):
        import function_preflight as fp
        original = pp.sha256_file
        for module in (fp.ri, fp.rs):
            with mock.patch.object(pp, "sha256_file", side_effect=lambda path: "changed" if str(path) == module.__file__ else original(path)):
                with self.assertRaises(fp.PreflightError):
                    fp._check_filter_implementations()
        linked = b"linked fixture"
        digest = hashlib.sha256(linked).hexdigest()
        snapshot = {"linked": digest, "target_inputs": {"rom": "original", "atlas": "original"}}
        receipt = {"linked_sha256": digest, "inputs": {"target_inputs": snapshot["target_inputs"]}}
        fp._require_capture_snapshot(receipt, snapshot, linked)
        with self.assertRaises(pp.MetadataProofError):
            fp._require_capture_snapshot(receipt, snapshot, b"older parsed ELF")
        for field in ("rom", "atlas"):
            changed = copy.deepcopy(receipt)
            changed["inputs"]["target_inputs"][field] = "changed"
            with self.assertRaises(pp.MetadataProofError):
                fp._require_capture_snapshot(changed, snapshot, linked)

    def test_explicit_raw_counts_and_malformed_receipts(self):
        import promotion_proof as proof
        from test_promotion_proof import exact_report
        report = exact_report()
        raw = copy.deepcopy(report["relocation_comparison"])
        raw["identity_proof_mode"] = "raw-static-with-declared-metadata-filters"
        metadata = {"schema": "mickey-declared-metadata-proof-v1", "raw_count": 3, "retained_count": 1,
                    "source_selection": "ordinary_c", "filtered": [{"offset": 0, "rtype": 5, "symbol": ".bss"}, {"offset": 4, "rtype": 6, "symbol": ".bss"}],
                    "filtered_identities": [{"offset": 0, "rtype": 5, "identity": [3, 8]}, {"offset": 4, "rtype": 6, "identity": [3, 8]}],
                    "raw_sha256": "a" * 64, "configured_sha256": "b" * 64, "linked_sha256": "c" * 64,
                    "inputs": {"command": "cc", "postprocess": "filter", "configured": "b" * 64, "linked": "c" * 64}}
        report["relocation_comparison"].update(candidate_record_count=1, original_raw_comparison=raw,
                                                declared_metadata_proof=metadata, identity_proof_mode=raw["identity_proof_mode"])
        result = proof.validate_report("friendly", report)
        self.assertEqual((result["exact_relocations"], result["configured_relocations"]), (3, 1))
        for key, bad in (("filtered", None), ("filtered_identities", []), ("raw_count", 2), ("retained_count", 2),
                         ("source_selection", "global_asm_fallback"), ("raw_sha256", "bad"), ("inputs", {})):
            altered = copy.deepcopy(report)
            altered["relocation_comparison"]["declared_metadata_proof"][key] = bad
            with self.subTest(key=key), self.assertRaises(proof.ProofError):
                proof.validate_report("friendly", altered)
        for mutation in ("outer_count", "floating_type", "floating_identity_offset"):
            altered = copy.deepcopy(report)
            relocation = altered["relocation_comparison"]
            if mutation == "outer_count":
                relocation["target_record_count"] = 4
            elif mutation == "floating_type":
                relocation["declared_metadata_proof"]["filtered"][0]["rtype"] = 5.0
            else:
                relocation["declared_metadata_proof"]["filtered_identities"][0]["offset"] = 0.0
            with self.subTest(mutation=mutation), self.assertRaises(proof.ProofError):
                proof.validate_report("friendly", altered)


    def test_named_filtered_sites_need_a_raw_static_identity(self):
        """overlay86ScaledVectorPosition filters a named HI/LO pair, not `.bss`.

        The raw surface resolves the named pair statically, so its identity is
        the shipped tuple it aligned with; a named site the raw surface left
        unresolved has no independent identity and is still refused.
        """
        import collections
        import function_preflight as fp
        Record = collections.namedtuple("Record", "offset rtype identity")
        records = [Record(0x18, 5, (86, 0x40)), Record(0x24, 6, (86, 0x40)),
                   Record(0x84, 4, (86, 0))]
        accounting = {"raw_count": 3, "retained_count": 1, "start": 0, "size": 0xDC,
                      "filtered": [{"offset": 0x18, "rtype": 5, "symbol": "gVectors"},
                                   {"offset": 0x24, "rtype": 6, "symbol": "gVectors"}]}

        def run(unresolved):
            raw = {"candidate_record_count": 3, "offset_type_exact": True,
                   "candidate_identity_unresolved_records": unresolved,
                   "stable_identity_alignment_count": 3 - len(unresolved)}
            with tempfile.TemporaryDirectory() as td:
                root = Path(td)
                (root / "capture").mkdir()
                source = root / "f.c"
                source.write_text("void f(void) {}\n")
                resolution = fp.Resolution("f", "f", "f", source, "f", "build",
                                           root / "f.c.o", None, "promoted",
                                           resolution_mode="post_promotion")
                receipt = {"inputs": {"command": "cc"}, "raw_sha256": "r",
                           "directory": "capture"}
                with contextlib.ExitStack() as stack:
                    patch = lambda *a, **k: stack.enter_context(mock.patch.object(*a, **k))
                    patch(fp, "REPO", root)
                    patch(fp.pa, "run_make_database", return_value=None)
                    patch(fp.pa, "postprocess_commands",
                          return_value={fp._relative(resolution.candidate_object):
                                        "tools/filter_elf_relocations.py"})
                    patch(fp, "_check_filter_implementations")
                    patch(fp, "_require_capture_snapshot")
                    patch(fp.pp, "capture_configured_raw",
                          return_value=(root / "raw.o", [], receipt, lambda: {"command": "cc"}))
                    patch(fp.rs, "Elf", return_value=None)
                    patch(fp.pp, "validate_metadata_objects", return_value=dict(accounting))
                    patch(fp.rs, "function_surface_comparison", return_value=raw)
                    patch(fp.pp, "sha256_file", return_value="r")
                    patch(fp.pp, "classify_source_selection", return_value=(fp.pp.ORDINARY_C,))
                    bss = patch(fp.pp, "linked_bss_base")
                    _comparison, binding = fp._declared_filter_comparison(
                        resolution, {"offset_type_exact": False, "candidate_record_count": 1},
                        {"kind": "overlay", "overlay": 86}, records, mock.Mock(data=b""), "f", {})
                    self.assertFalse(bss.called)
                    return binding[0]

        proved = run([])
        self.assertEqual([(0x18, (86, 0x40), "raw-static"), (0x24, (86, 0x40), "raw-static")],
                         [(row["offset"], row["identity"], row["route"])
                          for row in proved["filtered_identities"]])
        with self.assertRaisesRegex(fp.PreflightError, "no independent identity"):
            run([{"offset": 0x18, "rtype": 5}, {"offset": 0x24, "rtype": 6}])

    def test_named_filter_sites_are_valid_receipt_rows(self):
        import promotion_proof as proof
        from test_promotion_proof import exact_report
        report = exact_report()
        raw = copy.deepcopy(report["relocation_comparison"])
        raw["identity_proof_mode"] = "raw-static-with-declared-metadata-filters"
        metadata = {"schema": "mickey-declared-metadata-proof-v1", "raw_count": 3, "retained_count": 1,
                    "source_selection": "ordinary_c",
                    "filtered": [{"offset": 0, "rtype": 5, "symbol": "gVectors"},
                                 {"offset": 4, "rtype": 6, "symbol": "gVectors"}],
                    "filtered_identities": [{"offset": 0, "rtype": 5, "identity": [3, 8]},
                                            {"offset": 4, "rtype": 6, "identity": [3, 8]}],
                    "raw_sha256": "a" * 64, "configured_sha256": "b" * 64, "linked_sha256": "c" * 64,
                    "inputs": {"command": "cc", "postprocess": "filter", "configured": "b" * 64,
                               "linked": "c" * 64}}
        report["relocation_comparison"].update(candidate_record_count=1, original_raw_comparison=raw,
                                                declared_metadata_proof=metadata,
                                                identity_proof_mode=raw["identity_proof_mode"])
        self.assertEqual("exact", proof.validate_report("friendly", report)["verdict"])
        for symbol, rtype in ((".bss", 4), ("", 5), (None, 5)):
            altered = copy.deepcopy(report)
            row = altered["relocation_comparison"]["declared_metadata_proof"]["filtered"][0]
            row.update(symbol=symbol, rtype=rtype)
            altered["relocation_comparison"]["declared_metadata_proof"]["filtered_identities"][0]["rtype"] = rtype
            with self.subTest(symbol=symbol, rtype=rtype), self.assertRaises(proof.ProofError):
                proof.validate_report("friendly", altered)

if __name__ == "__main__":
    unittest.main()
