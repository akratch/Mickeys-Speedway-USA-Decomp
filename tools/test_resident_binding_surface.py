#!/usr/bin/env python3
"""Focused controls for the fixed R8 candidate binding surface."""
from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface as rs
import resident_binding_surface as surface


class FakeElf:
    def __init__(self, _path=None, *, width=1, function_size=5036, missing_low=False,
                 low_before_high=False, value=0, body_word=0):
        self.names = [".text"]
        self._symbols = [
            (surface.FUNCTION, 0x100, function_size, (1 << 4) | rs.STT_FUNC, 0),
            ("gO8P1294ImpactGateReloc", value, width, (1 << 4) | rs.STT_OBJECT, rs.SHN_UNDEF),
            ("gO8P1294ColorGateReloc", 0, 1, (1 << 4) | rs.STT_OBJECT, rs.SHN_UNDEF),
            ("gO8P1294MotionScalarReloc", 0, 4, (1 << 4) | rs.STT_OBJECT, rs.SHN_UNDEF),
        ]
        self._relocs = [
            (".text", 0x110, rs.R_MIPS_HI16, 1),
            (".text", 0x114, rs.R_MIPS_LO16, 1),
            (".text", 0x120, rs.R_MIPS_HI16, 2),
            (".text", 0x124, rs.R_MIPS_LO16, 2),
            (".text", 0x130, rs.R_MIPS_HI16, 3),
            (".text", 0x134, rs.R_MIPS_LO16, 3),
        ]
        self._relocs.extend((".text", 0x140 + 4 * index, rs.R_MIPS_32, 0)
                            for index in range(131))
        if missing_low:
            self._relocs = [(".text", 0x114, rs.R_MIPS_32, 1)
                            if row[1] == 0x114 else row for row in self._relocs]
        if low_before_high:
            self._relocs[0], self._relocs[1] = self._relocs[1], self._relocs[0]
        self.body_word = body_word

    def symbols(self):
        return list(self._symbols)

    def section(self, name):
        return (0, (0, 1, 0, 0, 0, 0, 0, 0, 0, 0)) if name == ".text" else (None, None)

    def relocations(self):
        return list(self._relocs)

    def section_bytes(self, _name):
        return bytes([self.body_word & 0xFF]) * 0x2000


class ResidentBindingSurfaceTests(unittest.TestCase):
    def _candidate(self, **elf_args):
        td = tempfile.TemporaryDirectory(dir=surface.adapter.ROOT)
        self.addCleanup(td.cleanup)
        obj = Path(td.name) / "candidate.o"
        obj.write_bytes(b"candidate")
        src = surface.adapter.ROOT / surface.SOURCE
        graph = [{"site": i * 4, "type": rs.R_MIPS_32, "name": "proxy",
                  "addend": 0, "owner": {"name": "proxy"}}
                 for i in range(137)]
        with mock.patch.object(surface.rs, "Elf", lambda _path: FakeElf(**elf_args)), \
             mock.patch.object(surface.sf, "_function", side_effect=lambda elf, symbols, name: {
                 "symbol": symbols[0], "owner": {}, "section": 0, "start": 0x100,
                 "size": next(row[2] for row in symbols if row[0] == name)}), \
             mock.patch.object(surface.sf, "_symtab_index", return_value=1), \
             mock.patch.object(surface.sf, "_relocations", return_value=graph), \
             mock.patch.object(surface, "_sha", side_effect=lambda p: (
                 surface.SOURCE_SHA256 if Path(p) == src else "a" * 64)):
            return surface._candidate_carriers(
                obj, surface.FUNCTION, 8, surface.SOURCE)

    def test_actual_candidate_requires_each_typed_complete_pair(self):
        result = self._candidate()
        self.assertEqual((result["function_size"], result["relocation_count"]), (5036, 137))
        self.assertEqual(set(result["carriers"]), {
            "gO8P1294ImpactGateReloc", "gO8P1294ColorGateReloc",
            "gO8P1294MotionScalarReloc"})

    def test_rejects_wrong_carrier_width(self):
        with self.assertRaisesRegex(surface.BindingSurfaceError, "type, width"):
            self._candidate(width=4)

    def test_rejects_wrong_function_extent(self):
        with self.assertRaisesRegex(surface.BindingSurfaceError, "size"):
            self._candidate(function_size=5040)

    def test_rejects_incomplete_hi_lo_pair(self):
        with self.assertRaisesRegex(surface.BindingSurfaceError, "complete HI16/LO16 pair"):
            self._candidate(missing_low=True)

    def test_rejects_lo_before_hi_in_actual_rel_table_order(self):
        with self.assertRaisesRegex(surface.BindingSurfaceError, "unpaired LO16"):
            self._candidate(low_before_high=True)

    def test_rejects_nonzero_undefined_carrier_value(self):
        with self.assertRaisesRegex(surface.BindingSurfaceError, "type, width, or binding"):
            self._candidate(value=4)

    def test_changed_owned_body_with_same_geometry_fails_stock_configured_link(self):
        with tempfile.TemporaryDirectory(dir=surface.adapter.ROOT) as td:
            root = Path(td)
            supplied = root / "candidate.o"; supplied.write_bytes(b"changed-body-object")
            configured = root / "configured.o"; configured.write_bytes(b"original-object")
            raw = root / "raw.o"; raw.write_bytes(b"original-stock-c")
            source = root / "overlay_008.c"; source.write_bytes(b"source")
            with mock.patch.object(surface.adapter, "ROOT", root), \
                 mock.patch.object(surface, "_sha", side_effect=lambda p: (
                     surface.SOURCE_SHA256 if Path(p) == source
                     else __import__('hashlib').sha256(Path(p).read_bytes()).hexdigest())):
                candidate = {"sha256": surface._sha(supplied), "path": supplied,
                             "function_size": 5036, "function_bytes_sha256": "changed-bytes",
                             "ordered_relocations": [{"site": 0, "type": 5, "name": "proxy"}],
                             "source_sha256": surface.SOURCE_SHA256}
                packet = {"candidate_stock_c": {
                    "configured_object_sha256": surface._sha(configured),
                    "object_sha256": surface._sha(raw), "function_size": 5036,
                    "function_bytes_sha256": "original-bytes",
                    "ordered_relocations": [{"site": 0, "type": 5, "name": "proxy"}],
                    "source_fidelity": {"runtime_identity_proved": False,
                                        "promotion_authority": False},
                    "source_sha256": surface.SOURCE_SHA256,
                    "preprocessed_self_context": {"status": "unchanged"},
                    "recipe_fingerprint": "recipe"},
                    "artifacts": {"configured_object": "configured.o", "stock_c_object": "raw.o"}}
                with self.assertRaisesRegex(surface.BindingSurfaceError, "not the freshly authenticated"):
                    surface._authenticate_candidate_report(candidate, packet)

    def test_exact_stock_configured_candidate_proof_is_admitted(self):
        with tempfile.TemporaryDirectory(dir=surface.adapter.ROOT) as td:
            root = Path(td)
            supplied = root / "candidate.o"; supplied.write_bytes(b"same-object")
            configured = root / "configured.o"; configured.write_bytes(b"same-object")
            raw = root / "raw.o"; raw.write_bytes(b"stock-c")
            source = root / "overlay_008.c"; source.write_bytes(b"source")
            graph = [{"site": 0, "type": rs.R_MIPS_HI16, "name": "proxy",
                      "addend": 0, "owner": {"name": "proxy", "type": 1}}]
            with mock.patch.object(surface.adapter, "ROOT", root):
                candidate = {"sha256": surface._sha(supplied), "path": supplied,
                             "function_size": 5036, "function_bytes_sha256": "owned-bytes",
                             "ordered_relocations": graph,
                             "source_sha256": surface.SOURCE_SHA256}
                packet = {"candidate_stock_c": {
                    "configured_object_sha256": surface._sha(configured),
                    "object_sha256": surface._sha(raw), "function_size": 5036,
                    "function_bytes_sha256": "owned-bytes",
                    "ordered_relocations": graph,
                    "source_fidelity": {"runtime_identity_proved": False,
                                        "promotion_authority": False},
                    "source_sha256": surface.SOURCE_SHA256,
                    "preprocessed_self_context": {"status": "unchanged"},
                    "recipe_fingerprint": "recipe"},
                    "artifacts": {"configured_object": "configured.o", "stock_c_object": "raw.o"}}
                surface._authenticate_candidate_report(candidate, packet)

    def test_rejects_other_scope_before_parsing(self):
        with self.assertRaisesRegex(surface.BindingSurfaceError, "scoped"):
            surface._candidate_carriers(Path("/missing"), surface.FUNCTION, 7, surface.SOURCE)

    def test_joint_alias_graph_clears_only_correlation_ambiguity(self):
        with tempfile.TemporaryDirectory() as td:
            values = Path(td) / "symbols.txt"
            values.write_text("gO8P1294ImpactGateReloc = overlayAlias;\n")
            imported = {"gO8P1294ImpactGateReloc": (0xFFD, 0x31A4)}
            evidence = {"gO8P1294ImpactGateReloc": [
                {"independent": False, "base_identity": [8, 0x31A4]}]}
            identities, ambiguous = {}, {"gO8P1294ImpactGateReloc"}
            rs._merge_independent_resident_bindings(
                imported, identities, ambiguous, evidence, values, None)
            self.assertEqual(identities["overlayAlias"], (0xFFD, 0x31A4))
            self.assertNotIn("gO8P1294ImpactGateReloc", ambiguous)

    def test_joint_alias_graph_rejects_independent_peer_conflict(self):
        with tempfile.TemporaryDirectory() as td:
            values = Path(td) / "symbols.txt"
            values.write_text("gO8P1294ImpactGateReloc = overlayAlias;\n")
            imported = {"gO8P1294ImpactGateReloc": (0xFFD, 0x31A4)}
            evidence = {"overlayAlias": [
                {"independent": True, "base_identity": [8, 0x31A4]}]}
            with self.assertRaisesRegex(rs.SurfaceComparisonError, "conflict"):
                rs._merge_independent_resident_bindings(
                    imported, {}, set(), evidence, values, None)

    def test_helper_import_pin_checked_before_capture(self):
        helper = Path(surface.__file__).resolve()
        with mock.patch.dict(surface._LOADED, {helper: "0" * 64}), \
             mock.patch.object(surface.adapter, "collect") as capture:
            with self.assertRaisesRegex(surface.BindingSurfaceError, "changed since import"):
                surface.collect_candidate(Path("/missing"), surface.FUNCTION, 8, surface.SOURCE)
            capture.assert_not_called()

    def test_helper_pin_rechecked_after_final_physical_capture(self):
        with tempfile.TemporaryDirectory(dir=surface.adapter.ROOT) as td:
            root = Path(td); candidate = root / "candidate.o"; candidate.write_bytes(b"candidate")
            helper = Path(surface.__file__).resolve(); expected = surface._LOADED[helper]
            def late_change(_handle):
                surface._LOADED[helper] = "0" * 64
            receipt = {"schema": "mickey-r8-resident-binding-surface-v1",
                       "candidate": {"path": candidate.relative_to(surface.adapter.ROOT).as_posix(),
                                     "sha256": surface._sha(candidate), "source": surface.SOURCE,
                                     "source_sha256": surface.SOURCE_SHA256},
                       "recheck_handles": ["opaque"]}
            try:
                with mock.patch.object(surface.adapter, "ROOT", surface.adapter.ROOT), \
                     mock.patch.object(surface.adapter, "recheck", side_effect=late_change):
                    with self.assertRaisesRegex(surface.BindingSurfaceError, "changed since import"):
                        surface.recheck_candidate(receipt)
            finally:
                surface._LOADED[helper] = expected


if __name__ == "__main__":
    unittest.main()
