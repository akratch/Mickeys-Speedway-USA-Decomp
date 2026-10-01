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
    def __init__(self, _path=None, *, width=1, function_size=5036, missing_low=False):
        self.names = [".text"]
        self._symbols = [
            (surface.FUNCTION, 0x100, function_size, (1 << 4) | rs.STT_FUNC, 0),
            ("gO8P1294ImpactGateReloc", 0, width, (1 << 4) | rs.STT_OBJECT, rs.SHN_UNDEF),
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

    def symbols(self):
        return list(self._symbols)

    def section(self, name):
        return (0, (0, 1, 0, 0, 0, 0, 0, 0, 0, 0)) if name == ".text" else (None, None)

    def relocations(self):
        return list(self._relocs)


class ResidentBindingSurfaceTests(unittest.TestCase):
    def _candidate(self, **elf_args):
        td = tempfile.TemporaryDirectory(dir=surface.adapter.ROOT)
        self.addCleanup(td.cleanup)
        obj = Path(td.name) / "candidate.o"
        obj.write_bytes(b"candidate")
        src = surface.adapter.ROOT / surface.SOURCE
        with mock.patch.object(surface.rs, "Elf", lambda _path: FakeElf(**elf_args)), \
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


if __name__ == "__main__":
    unittest.main()
