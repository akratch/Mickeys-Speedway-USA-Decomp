"""Synthetic binary-owner witness acceptance and refusal controls."""

from pathlib import Path
import copy
import json
import os
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import binary_storage_witness as bw
import reloc_surface as rs
import fast_score


class Elf:
    def __init__(self, path, names, symbols, sections, rels=()):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        self.names = names
        self.rows = list(symbols)
        self.sections = sections
        self.rels = list(rels)

    def symbols(self):
        return self.rows

    def relocations(self, target=r"\.text"):
        return self.rels

    def section(self, name):
        if name not in self.names:
            return None, None
        size = len(self.sections[name])
        return self.names.index(name), (0, 1, 0, 0, 0, size, 0, 0, 1, 0)

    def section_bytes(self, name):
        return self.sections.get(name, b"")


SOURCE = """typedef struct Template {
u8 a; u8 b; u8 c; u8 d;
s16 e; s16 f; s16 g; s16 h; s16 i; s16 j;
} Template;
extern const Template proxy[16];
void caller(void) {}
"""


class BinaryOwnerTests(unittest.TestCase):
    def fixture(self, root):
        payload = bytes(range(16)) * 16  # deliberately synthetic, not game data
        rom = b"\0" * 96 + payload + b"\0" * 16
        module = {"overlay": 17, "identity": "overlay:17", "synthetic_vma": "0xF0000000",
                  "rom": {"start": "0x40", "end": "0x170", "size": "0x130"},
                  "sections": {"text": {"size": "0x20"},
                               "data_rodata": {"start": "0x60", "end": "0x160", "size": "0x100"}},
                  "text_ownership": [{"type": "c", "source": "overlays/o017/caller"}]}
        files = {
            "mickey.us.yaml": """segments:
- name: overlay_017
  start: 64
  vram: 4026531840
  dir: overlays/o017
  subsegments:
  - [64, c, caller]
  - [96, bin, owned]
  - [352, bin, reloc]
""",
            "Makefile": "# synthetic policy\n",
            "config/overlays.us.json": json.dumps({"modules": [module]}),
            "values": "proxy = 0x0;\n",
            "src/overlays/o017/caller.c": SOURCE,
            "assets/overlays/o017/owned.bin": payload,
            "baseroms/mickey.us.z64": rom,
            "build/assets/overlays/o017/owned.bin.o": b"synthetic wrapper",
            "build_non_matching/src/overlays/o017/caller.c.o": b"synthetic candidate",
            "build/mickey.us.elf": b"synthetic linked",
        }
        for relative, data in files.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data.encode() if isinstance(data, str) else data)
        # Stable ordering avoids coarse timestamp-resolution assumptions.
        for relative in files:
            stamp = 100 if not relative.startswith("build") else 200
            if relative == "build/mickey.us.elf":
                stamp = 300
            os.utime(root / relative, (stamp, stamp))
        base = "_binary_assets_overlays_o017_owned_bin"
        wrapper = Elf(root / "build/assets/overlays/o017/owned.bin.o", ["", ".data"],
                      [(base + "_start", 0, 0, 0x10, 1), (base + "_end", 256, 0, 0x10, 1)],
                      {".data": payload})
        candidate = Elf(root / "build_non_matching/src/overlays/o017/caller.c.o", ["", ".text"],
                        [("proxy", 0, 0, 0x10, 0)], {".text": b"\0" * 32},
                        [(".text", 0, rs.R_MIPS_HI16, 0), (".text", 4, rs.R_MIPS_LO16, 0)])
        linked = Elf(root / "build/mickey.us.elf", ["", ".overlay_017", ".overlay_018"],
                     [("proxy", 0, 0, 0x10, rs.SHN_ABS),
                      (base + "_start", rs.SYNTHETIC_VMA + 32, 0, 0x10, 1),
                      (base + "_end", rs.SYNTHETIC_VMA + 288, 0, 0x10, 1)],
                     {".overlay_017": b"\0" * 32 + payload, ".overlay_018": b""})
        request = {"schema_version": 1, "bindings": [{"symbol": "proxy", "candidate_source": "overlays/o017/caller",
                   "owner_source": "overlays/o017/owned", "element_type": "Template", "element_count": 16, "bias": "0x0"}]}
        proof = mock.Mock(return_value={"element_bytes": 16, "array_bytes": 256, "alignment_bytes": 2,
                                      "prepared_sha256": "synthetic", "probe_sha256": "synthetic"})
        return module, request, candidate, linked, wrapper, rom, proof

    def collect(self, root, fixture):
        module, request, candidate, linked, wrapper, rom, proof = fixture
        return bw.collect(rs, request, candidate, 0, 32, module, linked, rom,
                          root / "values", root=root, elf_loader=lambda _: wrapper, probe=proof)

    def test_independent_placement_and_compiler_extent(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td); fixture = self.fixture(root)
            identities, receipts = self.collect(root, fixture)
            self.assertEqual(identities, {"proxy": (17, 32)})
            self.assertEqual(receipts[0]["owner_bytes"], 256)
            self.assertEqual(receipts[0]["matching_credit"], 0)
            fixture[-1].assert_called_once()

    def test_refusals(self):
        for change in ("duplicate_symbol", "duplicate_owner", "wrong_section", "wrong_module",
                       "wrong_extent", "wrong_bytes", "wrong_linked_bytes", "stale_wrapper",
                       "stale_candidate", "stale_link", "bias", "layout", "unsized", "pointer_pod",
                       "conditional", "loaded_changed", "duplicate_binding", "unowned_source", "alignment", "synthetic_bias"):
            with self.subTest(change=change), tempfile.TemporaryDirectory() as td:
                root = Path(td); fixture = self.fixture(root)
                module, request, candidate, linked, wrapper, rom, proof = fixture
                if change == "duplicate_symbol": linked.rows.append(linked.rows[1])
                if change == "duplicate_owner":
                    path = root / "mickey.us.yaml"
                    path.write_text(path.read_text().replace("  - [352, bin, reloc]", "  - [352, bin, owned]\n  - [360, bin, reloc]"))
                if change in ("wrong_section", "wrong_module"):
                    row = list(linked.rows[1]); row[4] = rs.SHN_ABS if change == "wrong_section" else 2; linked.rows[1] = tuple(row)
                if change == "wrong_extent": wrapper.sections[".data"] = wrapper.sections[".data"][:-1]
                if change == "wrong_bytes": wrapper.sections[".data"] = b"x" * 256
                if change == "wrong_linked_bytes": linked.sections[".overlay_017"] = b"x" * 288
                if change == "stale_wrapper": os.utime(wrapper.path, (50, 50))
                if change == "stale_candidate": os.utime(candidate.path, (50, 50))
                if change == "stale_link": os.utime(linked.path, (150, 150))
                if change == "bias": request["bindings"][0]["bias"] = "0x4"
                if change == "synthetic_bias": (root / "values").write_text("proxy = 0xf0000000;\n")
                if change == "layout": proof.return_value["element_bytes"] = 8
                if change == "alignment": proof.return_value["alignment_bytes"] = 64
                if change in ("unsized", "pointer_pod", "conditional"):
                    path = root / "src/overlays/o017/caller.c"
                    replacement = SOURCE.replace("proxy[16]", "proxy[]") if change == "unsized" else SOURCE.replace("u8 a;", "u8 *a;")
                    if change == "conditional": replacement = "#ifdef MAYBE\n" + SOURCE + "#endif\n"
                    path.write_text(replacement)
                if change == "loaded_changed": candidate.path.write_bytes(b"changed")
                if change == "duplicate_binding": request["bindings"].append(copy.deepcopy(request["bindings"][0]))
                if change == "unowned_source": request["bindings"][0]["candidate_source"] = "overlays/o018/caller"
                with self.assertRaises(rs.SurfaceComparisonError): self.collect(root, fixture)

    def test_inputs_mutated_during_probe_are_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td); fixture = self.fixture(root)
            def mutate(*_):
                (root / "values").write_text("proxy = 0x4;\n")
                return {"element_bytes": 16, "array_bytes": 256, "alignment_bytes": 2}
            fixture[-1].side_effect = mutate
            with self.assertRaisesRegex(rs.SurfaceComparisonError, "changed during"):
                self.collect(root, fixture)

    def test_timestamp_only_change_during_probe_is_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td); fixture = self.fixture(root)
            def touch(*_):
                os.utime(root / "src/overlays/o017/caller.c", (400, 400))
                return {"element_bytes": 16, "array_bytes": 256, "alignment_bytes": 2}
            fixture[-1].side_effect = touch
            with self.assertRaisesRegex(rs.SurfaceComparisonError, "changed during"):
                self.collect(root, fixture)

    def test_identity_admission_never_repairs_count_or_offsets(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td); fixture = self.fixture(root)
            identities, _ = self.collect(root, fixture)
            candidate = [rs.SurfaceRecord(i, rs.R_MIPS_26, (0, i)) for i in (8, 12, 16)]
            candidate += [rs.SurfaceRecord(0, rs.R_MIPS_HI16, identities["proxy"]),
                          rs.SurfaceRecord(4, rs.R_MIPS_LO16, identities["proxy"])]
            target = candidate + [rs.SurfaceRecord(20, rs.R_MIPS_HI16, (17, 32)),
                                  rs.SurfaceRecord(24, rs.R_MIPS_LO16, (17, 32))]
            result = rs.compare_record_sets(target, candidate)
            self.assertFalse(result["stable_identity_exact"])
            self.assertEqual(result["candidate_record_count"], 5)
            self.assertEqual(result["target_record_count"], 7)
            moved = [rs.SurfaceRecord(record.offset + 4, record.rtype, record.identity) for record in candidate]
            self.assertFalse(rs.compare_record_sets(candidate, moved)["offset_type_exact"])


class CompilerLayoutTests(unittest.TestCase):
    def test_forced_environment_is_refused_before_compilation(self):
        with mock.patch.dict(os.environ, {"CDX_TEST_ONLY": "1"}):
            with self.assertRaisesRegex(rs.SurfaceComparisonError, "forced/instrumented"):
                bw.layout_proof(rs, Path("."), Path("unused"), None, {})

    @unittest.skipUnless((rs.REPO / "tools/ido/cc").is_file(), "stock IDO unavailable")
    def test_stock_preprocessed_probe_proves_extents_and_preserves_text(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            source = root / "src/overlays/o017/caller.c"
            source.parent.mkdir(parents=True)
            source.write_text("typedef unsigned char u8; typedef signed short s16;\n" + SOURCE)
            compiler_root = root / "tools/ido"
            compiler_root.parent.mkdir(parents=True)
            compiler_root.symlink_to((rs.REPO / "tools/ido").resolve())
            obj = root / "build_non_matching/src/overlays/o017/caller.c.o"
            obj.parent.mkdir(parents=True)
            args = [str(compiler_root / "cc"), "-c", "-O2", "-mips2", "-32", "-non_shared", "-G", "0",
                    "-o", str(obj), source.relative_to(root).as_posix()]
            baseline = subprocess.run(args, cwd=root, capture_output=True)
            self.assertEqual(baseline.returncode, 0, baseline.stderr.decode())
            candidate = rs.Elf(obj)
            binding = {"element_type": "Template", "element_count": 16, "symbol": "proxy"}
            with mock.patch.object(rs, "_callee_build_dependencies", return_value=({source, compiler_root / "cc"}, None)), \
                 mock.patch.object(fast_score, "configured_cc_args", return_value=args):
                proof = bw.layout_proof(rs, root, source, candidate, binding)
            self.assertEqual(proof["element_bytes"], 16)
            self.assertEqual(proof["array_bytes"], 256)
            self.assertEqual(proof["alignment_bytes"], 2)
            self.assertEqual(len(proof["prepared_sha256"]), 64)
            self.assertEqual(obj.read_bytes(), candidate.data)
            captures = list((root / "build/binary-storage-witnesses").glob("*/consumer.i"))
            self.assertEqual(len(captures), 1)
            self.assertNotIn(b"binary_witness_", captures[0].read_bytes())


if __name__ == "__main__":
    unittest.main()
