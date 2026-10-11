"""Direct storage authority refuses address-only and shadowed definitions."""
import unittest
import tempfile
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

import canonical_data_owner as owner
import reloc_surface as rs


class DirectOwnerTests(unittest.TestCase):
    def test_duplicate_yaml_owner_key_fails_closed(self):
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "duplicate YAML"):
            owner.yaml_config("segments: []\nsegments: []\n", rs.SurfaceComparisonError)

    def inputs(self):
        symbols = [("state", rs.SYNTHETIC_VMA + 16, 4, 0x11, 1)]
        linked = SimpleNamespace(names=["", ".overlay_064"], symbols=lambda: symbols)
        carriers = [("state", 0, 0, 0x11, rs.SHN_UNDEF)]
        candidate = SimpleNamespace(symbols=lambda: carriers)
        return symbols, carriers, linked, candidate

    def collect(self, linked, candidate):
        return owner.collect(rs, {"overlay": 64}, candidate, "state", linked, b"")

    def test_address_only_assignment_is_not_a_definition(self):
        symbols, _, linked, candidate = self.inputs()
        symbols[:] = [("state", 16, 4, 0x11, rs.SHN_ABS)]
        self.assertIsNone(self.collect(linked, candidate))

    def test_same_synthetic_address_in_other_overlay_is_not_borrowed(self):
        _, _, linked, candidate = self.inputs()
        linked.names[1] = ".overlay_063"
        self.assertIsNone(self.collect(linked, candidate))

    def test_duplicate_definition_fails(self):
        symbols, _, linked, candidate = self.inputs()
        symbols.append(symbols[0])
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "ambiguous"):
            self.collect(linked, candidate)

    def test_zero_extent_is_not_storage(self):
        symbols, _, linked, candidate = self.inputs()
        symbols[0] = ("state", rs.SYNTHETIC_VMA + 16, 0, 0x11, 1)
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "sized"):
            self.collect(linked, candidate)

    def test_absolute_alias_cannot_override_direct_owner(self):
        symbols, _, linked, candidate = self.inputs()
        symbols.append(("state", 0, 0, 0, rs.SHN_ABS))
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "absolute alias"):
            self.collect(linked, candidate)

    def test_candidate_local_definition_cannot_borrow_external_storage(self):
        _, carriers, linked, candidate = self.inputs()
        carriers[0] = ("state", 0, 4, 0x11, 1)
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "shadows"):
            self.collect(linked, candidate)

    def test_mismatched_candidate_extent_fails(self):
        _, carriers, linked, candidate = self.inputs()
        carriers[0] = ("state", 0, 8, 0x11, rs.SHN_UNDEF)
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "shadows"):
            self.collect(linked, candidate)

    def test_no_owned_c_input_fails_even_with_same_name(self):
        _, _, linked, candidate = self.inputs()
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "input owner"):
            self.collect(linked, candidate)


class DirectOwnerResolverTests(unittest.TestCase):
    def resolve(self, *, conflicting=False, enabled=True):
        from test_reloc_surface import OverlayDataIdentityTests
        with tempfile.TemporaryDirectory() as directory:
            candidate, canonical, linked, module, values = OverlayDataIdentityTests().fixture(
                Path(directory), name="state")
            values.write_text("")
            linked._symbols = [row for row in linked._symbols if row[4] != rs.SHN_ABS]
            receipts = [] if enabled else None
            evidence = {"state": [{"route": "matched-canonical-sibling", "independent": True,
                                    "base_identity": [7, 0xFC4]}]} if conflicting else {}
            proof = {"identity": (7, 0xFC0), "owner": "overlays/o007/owner",
                     "size": 4, "capture_sha256": "test", "recheck": lambda: None}
            with mock.patch.object(owner, "collect", return_value=proof) as collect:
                result = rs._stable_overlay_data_identities(
                    values, candidate, module, linked, 0, 0x40,
                    root=values.parent, elf_loader=lambda _: canonical, rom=b"fixture",
                    target_records=[rs.SurfaceRecord(0, rs.R_MIPS_HI16, (7, 0xFC4), 0),
                                    rs.SurfaceRecord(4, rs.R_MIPS_LO16, (7, 0xFC4), 0)],
                    evidence=evidence, direct_storage_receipts=receipts)
            return result, receipts, collect.call_count

    def test_owner_authority_survives_conflicting_target_site_correlation(self):
        result, receipts, calls = self.resolve()
        self.assertEqual(result, ({"state": (7, 0xFC0)}, set()))
        self.assertEqual(len(receipts), 1)
        self.assertEqual(calls, 1)

    def test_independent_conflict_is_never_overwritten(self):
        with self.assertRaisesRegex(rs.SurfaceComparisonError, "independent identity"):
            self.resolve(conflicting=True)

    def test_route_is_opt_in(self):
        _, receipts, calls = self.resolve(enabled=False)
        self.assertIsNone(receipts)
        self.assertEqual(calls, 0)


if __name__ == "__main__":
    unittest.main()
