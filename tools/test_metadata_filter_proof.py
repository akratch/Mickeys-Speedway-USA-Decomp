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

    def test_filter_spec_files_expand_like_the_helper(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "specs").mkdir()
            (root / "specs/f.spec").write_text("# removed pair\n0x8:5:gA 0xc:6:gA  # tail\n\n")
            command = ("$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text "
                       "0:5:.bss @specs/f.spec")
            self.assertEqual(("filter", [(0, 5, ".bss"), (8, 5, "gA"), (12, 6, "gA")]),
                             pp.metadata_filter_plan(command, "x.o", root)[-1])
            expanded = command.replace("$(HOST_PYTHON)", ".venv/bin/python").replace(
                "$(TOOLS_DIR)", "tools").replace("$@", "x.o")
            self.assertEqual({"specs/f.spec"}, set(pp.filter_spec_files(expanded, root)))
            for bad in ("@specs/missing.spec", "@../f.spec", "@/etc/f.spec", "@"):
                with self.subTest(bad=bad), self.assertRaises(pp.MetadataProofError):
                    pp.metadata_filter_plan(command.replace("@specs/f.spec", bad), "x.o", root)
            (root / "specs/dup.spec").write_text("0:5:.bss\n")
            with self.assertRaises(pp.MetadataProofError):
                pp.metadata_filter_plan(command.replace("f.spec", "dup.spec"), "x.o", root)

class RecipeTests(unittest.TestCase):
    """The shipped POSTPROCESS forms beyond rename/filter/trim (synthetic)."""

    def test_parser_reads_every_shipped_metadata_form(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "specs").mkdir()
            (root / "specs/r.spec").write_text("0x44:.rodata:gTable  # jump table\n")
            (root / "specs/f.spec").write_text("0x78:5:.rodata 0x80:6:.rodata\n")
            command = (
                "$(OBJCOPY) --redefine-sym a=b --add-symbol gTable=0x38,global $@ && "
                "$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text @specs/r.spec 0x4C:.rodata:gTable && "
                "$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text @specs/f.spec && "
                "$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .data sha256:00 0x10 && "
                "$(OBJCOPY) --remove-section=.rel.data --remove-section .rodata $@ && "
                "$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x178 00000000")
            plan = pp.metadata_filter_plan(command, "x.o", root)
            self.assertEqual([
                ("rename", [("a", "b")]), ("add-symbol", ["gTable=0x38,global"]),
                ("rebind", (".text", [(0x44, ".rodata", "gTable"), (0x4C, ".rodata", "gTable")])),
                ("filter", [(0x78, 5, ".rodata"), (0x80, 6, ".rodata")]),
                ("externalize", (".data", "sha256:00", 0x10)),
                ("remove-section", [".rel.data", ".rodata"]), ("trim", 0x178)], plan)
            expanded = command.replace("$(HOST_PYTHON)", ".venv/bin/python").replace(
                "$(TOOLS_DIR)", "tools").replace("$(OBJCOPY)", "tools/binutils/mips64-elf-objcopy").replace("$@", "x.o")
            self.assertEqual({"specs/r.spec", "specs/f.spec"}, set(pp.filter_spec_files(expanded, root)))
            for bad in ("--remove-section=.text", "--remove-section .rel.text", "--strip-all",
                        "--add-symbol broken"):
                with self.subTest(bad=bad), self.assertRaises(pp.MetadataProofError):
                    pp.metadata_filter_plan("$(OBJCOPY) %s $@" % bad, "x.o", root)
            for bad in ("externalize_elf_section.py $@ .text sha256:00",
                        "rebind_elf_relocations.py $@ .text 0x2:a:b",
                        "rebind_elf_relocations.py $@ .text 0x4:a:a",
                        "unknown_helper.py $@ .text 0"):
                with self.subTest(bad=bad), self.assertRaises(pp.MetadataProofError):
                    pp.metadata_filter_plan("$(HOST_PYTHON) $(TOOLS_DIR)/" + bad, "x.o", root)

    def test_accounting_plan_carries_later_renames_and_freed_sections(self):
        plan = [("filter", [(0, 5, "gLocal"), (4, 6, "gLocal"), (8, 5, ".rodata"), (12, 6, ".rodata")]),
                ("rename", [("gLocal", "D_C8"), ("other", "D_0")]),
                ("rename", [("D_C8", "D_final")]),
                ("remove-section", [".rodata", ".gptab.data"]), ("trim", 0x10)]
        self.assertEqual({".rodata"}, pp.deferred_sections(plan))
        self.assertEqual([("filter", [(0, 5, "D_final"), (4, 6, "D_final"), (8, 5, ".rodata"),
                                      (12, 6, ".rodata")]),
                          ("remove-section", [".rodata"])], pp.filter_accounting_plan(plan))
        # A rename *before* the filter is already the filter's own spelling,
        # and one objcopy invocation renames simultaneously (a=b, b=c: a->b).
        self.assertEqual([("filter", [(0, 5, "b")])], pp.filter_accounting_plan(
            [("rename", [("x", "y")]), ("filter", [(0, 5, "a")]), ("rename", [("a", "b"), ("b", "c")])]))

    def test_removed_section_is_accounted_after_the_filters(self):
        raw = ElfFixture()
        raw.names.append(".rodata")
        raw.sh.append((0, 1, 2, 0, 0, 8, 0, 0, 4, 0))
        raw.syms.append((".rodata", 0, 0, 3, 3))
        raw.rels = raw.rels[2:] + [(".text", 0, 5, 4), (".text", 4, 6, 4)]
        configured = copy.deepcopy(raw)
        del configured.names[3], configured.sh[3], configured.syms[4]
        configured.rels = configured.rels[:1]
        plan = [("filter", [(0, 5, ".rodata"), (4, 6, ".rodata")]), ("remove-section", [".rodata"])]
        result = pp.validate_metadata_objects(raw, configured, plan, "f")
        self.assertEqual((3, 1), (result["raw_count"], result["retained_count"]))
        with self.assertRaisesRegex(pp.MetadataProofError, "still a relocation target"):
            pp.validate_metadata_objects(raw, configured, plan[1:], "f")
        with self.assertRaises(pp.MetadataProofError):
            pp.validate_metadata_objects(raw, configured, plan[:1], "f")

    def test_replay_runs_every_step_but_the_filters_in_order(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "tools").mkdir()
            log = root / "log.txt"
            (root / "tools/step.py").write_text(
                "import sys\nopen(sys.argv[1], 'a').write(' '.join(sys.argv[2:]) + '\\n')\n")
            (root / "raw.o").write_text("")
            python = sys.executable
            command = " && ".join([
                "%s tools/step.py x.o objcopy-a" % python,
                "%s tools/filter_elf_relocations.py x.o .text 0:5:.rodata" % python,
                "%s tools/step.py x.o rebind" % python,
            ])
            out = pp.replay_metadata(root / "raw.o", command, "x.o", root / "u.o", root,
                                     skip_filters=True, deadline=__import__("time").monotonic() + 30)
            self.assertEqual("objcopy-a\nrebind\n", out.read_text())
            failing = "%s -c 'raise SystemExit(3)' x.o" % python
            with self.assertRaisesRegex(pp.MetadataProofError, "replay step failed"):
                pp.replay_metadata(root / "raw.o", failing, "x.o", root / "u.o", root,
                                   skip_filters=True, deadline=__import__("time").monotonic() + 30)
        # A removal the filters make possible is deferred; nothing else is.
        self.assertEqual(["objcopy", "--remove-section=.gptab", "x.o"], pp._replay_words(
            ["objcopy", "--remove-section=.rodata", "--remove-section=.gptab", "x.o"], {".rodata"}))
        self.assertIsNone(pp._replay_words(["objcopy", "--remove-section", ".rodata", "x.o"], {".rodata"}))


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
                             root / "tools/rebind_elf_relocations.py", root / "tools/externalize_elf_section.py",
                             root / "tools/postprocess_guard.py", root / "tools/binutils/mips64-elf-objcopy"]:
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


    def test_named_filtered_sites_need_a_replay_static_identity(self):
        """A named filtered site is proved by the unfiltered replay's static surface.

        overlay86ScaledVectorPosition filters a named HI/LO pair, not `.bss`.
        The replay (every recipe step but the filters) resolves the pair; its
        identity must be the shipped tuple. A named site it cannot resolve, or
        resolves elsewhere, is refused. A *retained* site the static surface
        cannot name is left to the linked-ROM route, as in any function.
        """
        import collections
        import function_preflight as fp
        Record = collections.namedtuple("Record", "offset rtype identity")
        records = [Record(0x18, 5, (86, 0x40)), Record(0x24, 6, (86, 0x40)),
                   Record(0x84, 4, (86, 0))]
        accounting = {"raw_count": 3, "retained_count": 1, "start": 0, "size": 0xDC,
                      "filtered": [{"offset": 0x18, "rtype": 5, "symbol": "gVectors"},
                                   {"offset": 0x24, "rtype": 6, "symbol": "gVectors"}]}

        def run(filtered_identity, retained_identity=(86, 0)):
            identities = [{"offset": 0x18, "rtype": 5, "identity": filtered_identity},
                          {"offset": 0x24, "rtype": 6, "identity": filtered_identity},
                          {"offset": 0x84, "rtype": 4, "identity": retained_identity}]
            unresolved = [{"offset": row["offset"], "rtype": row["rtype"]}
                          for row in identities if row["identity"] is None]
            raw = {"candidate_record_count": 3, "offset_type_exact": True,
                   "candidate_identity_unresolved_records": unresolved,
                   "stable_identity_alignment_count": 3 - len(unresolved),
                   "candidate_identities": identities}
            with tempfile.TemporaryDirectory() as td:
                root = Path(td)
                (root / "capture").mkdir()
                source = root / "f.c"
                source.write_text("void f(void) {}\n")
                resolution = fp.Resolution("f", "f", "f", source, "f", "build",
                                           root / "f.c.o", None, "promoted",
                                           resolution_mode="post_promotion")
                receipt = {"inputs": {"command": "cc", "postprocess": "filter"},
                           "raw_sha256": "r", "directory": "capture"}
                with contextlib.ExitStack() as stack:
                    patch = lambda *a, **k: stack.enter_context(mock.patch.object(*a, **k))
                    patch(fp, "REPO", root)
                    patch(fp.pa, "run_make_database", return_value=None)
                    patch(fp.pa, "postprocess_commands",
                          return_value={fp._relative(resolution.candidate_object):
                                        "tools/filter_elf_relocations.py"})
                    patch(fp, "_check_filter_implementations")
                    patch(fp, "_require_capture_snapshot")
                    patch(fp, "_candidate_redefine_aliases", return_value={})
                    patch(fp.pp, "capture_configured_raw",
                          return_value=(root / "raw.o", [], receipt, lambda: {"command": "cc", "postprocess": "filter"}))
                    replay = patch(fp.pp, "replay_metadata", return_value=root / "capture/unfiltered.o")
                    patch(fp.rs, "Elf", return_value=None)
                    patch(fp.pp, "validate_metadata_objects", return_value=dict(accounting))
                    patch(fp.rs, "function_surface_comparison", return_value=copy.deepcopy(raw))
                    patch(fp.pp, "sha256_file", return_value="r")
                    patch(fp.pp, "classify_source_selection", return_value=(fp.pp.ORDINARY_C,))
                    bss = patch(fp.pp, "linked_bss_base")
                    comparison, binding = fp._declared_filter_comparison(
                        resolution, {"offset_type_exact": False, "candidate_record_count": 1},
                        {"kind": "overlay", "overlay": 86}, records, mock.Mock(data=b""), "f", {})
                    self.assertFalse(bss.called)
                    self.assertTrue(replay.call_args.kwargs["skip_filters"])
                    return binding[0], comparison["original_raw_comparison"]

        proved, surface = run([86, 0x40])
        self.assertEqual([(0x18, (86, 0x40), "replay-static"), (0x24, (86, 0x40), "replay-static")],
                         [(row["offset"], row["identity"], row["route"])
                          for row in proved["filtered_identities"]])
        self.assertEqual((3, 3, True), (surface["stable_identity_alignment_count"],
                                        surface["candidate_identity_resolved_count"],
                                        surface["stable_identity_exact"]))
        # A retained site the static surface cannot name stays unresolved.
        _proved, surface = run([86, 0x40], retained_identity=None)
        self.assertEqual([{"offset": 0x84, "rtype": 4}], surface["candidate_identity_unresolved_records"])
        self.assertEqual((2, 2, False), (surface["stable_identity_alignment_count"],
                                         surface["candidate_identity_resolved_count"],
                                         surface["stable_identity_exact"]))
        with self.assertRaisesRegex(fp.PreflightError, "no independent identity"):
            run(None)
        with self.assertRaisesRegex(fp.PreflightError, "disagrees with canonical/runtime identity"):
            run([86, 0])

    def test_removed_section_sites_are_proved_by_retail_content(self):
        """overlay91UpdateTimeline filters a `.rodata` HI/LO, then removes `.rodata`.

        Its jump table's bytes live in retail's module data; the site is
        proved only where the ROM holds the section's exact words (relocated
        against the TU's text, as retail stores them).
        """
        import collections
        import function_preflight as fp
        Record = collections.namedtuple("Record", "offset rtype identity")

        class Raw:
            names = ["", ".text", ".rodata"]
            def __init__(self, table):
                self.table = table
            def section(self, name):
                return (self.names.index(name), None) if name in self.names else (None, None)
            def section_bytes(self, name):
                if name == ".text":
                    # lui at,0x0 ; lw at,0x4(at) -- addend 4 into the table
                    return bytes.fromhex("3c010000" "8c210004")
                return self.table
            def symbols(self):
                return [("", 0, 0, 0, 0), (".text", 0, 0, 3, 1), (".rodata", 0, 0, 3, 2)]
            def relocations(self, target=r"\.text"):
                if re.fullmatch(target, ".text"):
                    return [(".text", 0, 5, 2), (".text", 4, 6, 2)]
                return [(".rodata", 0, 2, 1), (".rodata", 4, 2, 1)]

        table = bytes.fromhex("0000008c" "000000c4")
        module_data = bytes.fromhex("000000d8" "00000110")  # TU base 0x4C applied
        rom = bytes(0x100) + bytes(0x80) + module_data + bytes(0x10)
        module = {"rom_start": 0x100, "text_size": 0x80, "data_size": 0x20}
        keys = {(0, 5), (4, 6)}
        accounting = {"start": 0, "size": 8}
        with tempfile.TemporaryDirectory() as td, contextlib.ExitStack() as stack:
            rom_path = Path(td) / "rom.z64"
            rom_path.write_bytes(rom)
            stack.enter_context(mock.patch.object(fp, "ROM", rom_path))
            stack.enter_context(mock.patch.object(fp.ot, "read_headers"))
            stack.enter_context(mock.patch.object(fp.ot, "build_modules", return_value=[module] * 91))

            def prove(raw, shipped):
                expected = {(0, 5): shipped, (4, 6): shipped}
                records = [Record(0, 5, shipped), Record(4, 6, shipped)]
                return fp._removed_section_identities(raw, accounting, records, 91, 0x4C,
                                                      {".rodata"}, keys, expected)

            self.assertEqual({(0, 5): (91, 0x84), (4, 6): (91, 0x84)}, prove(Raw(table), (91, 0x84)))
            with self.assertRaisesRegex(pp.MetadataProofError, "not the retail module data"):
                prove(Raw(bytes.fromhex("0000008c" "000000c8")), (91, 0x84))
            with self.assertRaisesRegex(pp.MetadataProofError, "initialized data"):
                prove(Raw(table), (91, 0x14))

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
