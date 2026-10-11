"""Physical ownership must not erase distinct loader namespaces or conflicts."""
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace

import reloc_surface as rs
import storage_identity as storage
import canonical_bss_view as bss


class ReservedNamespaceTests(unittest.TestCase):
    name = "D_800D60B0"
    physical = 0x800D60B0

    def proof(self):
        return {"identities": {self.name: (0xFFF, 0x50670)},
                "receipt": {"physical_addresses": {self.name: self.physical}}}

    def test_explicit_view_preserves_reserved_selector(self):
        self.assertEqual(storage.scoped_resident_identity(
            rs, self.name, (0, self.physical - rs.ot.RESIDENT_VRAM_BASE), self.proof()),
            (0xFFF, 0x50670))

    def test_physical_disagreement_is_not_reinterpreted(self):
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "physical address"):
            storage.scoped_resident_identity(rs, self.name, (0, 4), self.proof())

    def test_same_address_does_not_authorize_other_selector(self):
        proof = self.proof()
        proof["identities"][self.name] = (0, self.physical - rs.ot.RESIDENT_VRAM_BASE)
        with self.assertRaises(rs.SurfaceComparisonError):
            storage.scoped_resident_identity(
                rs, self.name, (0, self.physical - rs.ot.RESIDENT_VRAM_BASE), proof)

    def test_unrequested_names_retain_original_identity(self):
        self.assertEqual(storage.scoped_resident_identity(rs, "other", (0, 4), self.proof()), (0, 4))

    def test_renamed_bss_carrier_needs_authenticated_map(self):
        row = ("annotated", 0, 16, 0x11, rs.SHN_UNDEF)
        elf = SimpleNamespace(symbols=lambda: [row])
        with self.assertRaises(bss.BssViewError):
            bss.candidate_carrier(elf, self.name, {})
        self.assertEqual(bss.candidate_carrier(elf, self.name, {"annotated": self.name}), row)

    def test_renamed_bss_carrier_cannot_hide_original_shadow(self):
        elf = SimpleNamespace(symbols=lambda: [("annotated", 0, 16, 0x11, rs.SHN_UNDEF),
                                               (self.name, 0, 16, 0x11, 1)])
        with self.assertRaises(bss.BssViewError):
            bss.candidate_carrier(elf, self.name, {"annotated": self.name})

    def test_resolver_records_scoped_identity_before_alias_resolution(self):
        candidate = SimpleNamespace(
            symbols=lambda: [(self.name, 0, 0, 0x11, rs.SHN_UNDEF)],
            section=lambda _: (1, None))
        linked = SimpleNamespace(symbols=lambda: [(self.name, self.physical, 16, 0x11, 2)])
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "values"
            path.write_text("")
            evidence = {}
            identities, ambiguous = rs._stable_symbol_identities(
                path, candidate, 64, 0, linked, evidence=evidence,
                reserved_bss_proof=self.proof())
        self.assertEqual(identities, {self.name: (0xFFF, 0x50670)})
        self.assertFalse(ambiguous)
        self.assertTrue(all(row["base_identity"] == [0xFFF, 0x50670]
                            for row in evidence[self.name]))


if __name__ == "__main__":
    unittest.main()
