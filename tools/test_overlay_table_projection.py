#!/usr/bin/env python3
"""Runtime relocation tables must not participate in module section layout."""
import unittest
from unittest import mock

import overlay_atlas as atlas


class TableProjectionTests(unittest.TestCase):
    def test_tables_follow_the_body_as_standalone_rom_segments(self):
        module = {
            "overlay": 1, "rom": {"start": "0x1000", "size": "0x40"},
            "bss_size": "0x20",
            "sections": {
                "text": {"start": "0x1000", "size": "0x20"},
                "data_rodata": {"start": "0x1020", "size": "0x10"},
                "reloc1": {"start": "0x1030", "size": "0x8"},
                "reloc2": {"start": "0x1038", "size": "0x8"},
            },
            "text_ownership": [{"offset": "0x0", "type": "c", "source": "overlays/o001/unit"}],
            "data_rodata_ownership": [{
                "offset": "0x0", "end_offset": "0x10", "size": "0x10",
                "type": "c", "source": "overlays/o001/unit",
                "section": ".rodata", "canonical": True,
            }],
        }
        text = atlas.render_yaml_block({"modules": [module]})
        body, tables = text.split("\n  - name: overlay_001_reloc1", 1)
        self.assertIn("bss_size: 0x20", body)
        self.assertIn("[0x1020, .rodata, unit]", body)
        self.assertNotIn("reloc1", body)
        self.assertIn("{ type: bss, vram: 0xF0000030, name: overlay_001_bss_reserve }", body)
        self.assertIn("type: bin\n    start: 0x1030", tables)
        self.assertIn("\n  - name: overlay_001_reloc2", tables)
        self.assertNotIn("bss_size", tables)
        # A canonical owner can retain its earlier trial producer as
        # provenance: the default projection and credit remain canonical.
        module["data_rodata_ownership"][0]["trial_function"] = "candidate"
        self.assertEqual(atlas.render_yaml_block({"modules": [module]}), text)
        self.assertEqual(atlas.data_rodata_ownership_totals([module]),
                         {"owned_data_rodata_bytes": 0x10,
                          "diagnostic_data_rodata_bytes": 0})


class DataOwnershipCreditTests(unittest.TestCase):
    def row(self, start, end, **flags):
        return dict(offset=hex(start), end_offset=hex(end), size=hex(end - start),
                    type="c", source="overlays/o001/unit", **flags)

    def totals(self, *rows):
        return atlas.data_rodata_ownership_totals([
            {"data_rodata_ownership": list(rows)}])

    def test_only_leading_data_and_explicit_canonical_fixed_rows_earn_credit(self):
        totals = self.totals(
            self.row(0, 0x10), self.row(0x10, 0x20),
            self.row(0x40, 0x50, section=".data", canonical=True),
            self.row(0x60, 0x80, section=".rodata", canonical=True),
            self.row(0x80, 0xA0, section=".data"),
            self.row(0xA0, 0xB0, section=".rodata"))
        self.assertEqual(totals, {"owned_data_rodata_bytes": 0x50,
                                  "diagnostic_data_rodata_bytes": 0x30})

    def test_trial_only_and_externalized_rows_do_not_earn_credit(self):
        for flags in ({"trial_function": "candidate"}, {"externalized": True},
                      {"trial_function": "candidate", "externalized": True}):
            for canonical in (False, True):
                with self.subTest(flags=flags, canonical=canonical):
                    totals = self.totals(self.row(0, 0x20, section=".rodata",
                                                  canonical=canonical, **flags))
                    owned = 0x20 if canonical and not flags.get("externalized") else 0
                    self.assertEqual(totals, {"owned_data_rodata_bytes": owned,
                                              "diagnostic_data_rodata_bytes": 0x20 - owned})

    def test_unmarked_fixed_or_nonleading_rows_are_diagnostic(self):
        for flags in ({}, {"canonical": True},
                      {"canonical": True, "section": ".text"}):
            with self.subTest(flags=flags):
                self.assertEqual(self.totals(self.row(0x20, 0x30, **flags)),
                                 {"owned_data_rodata_bytes": 0,
                                  "diagnostic_data_rodata_bytes": 0x10})

    def test_module_leading_ranges_are_independent_and_inputs_are_unchanged(self):
        import copy

        modules = [{"data_rodata_ownership": [self.row(0, 0x10)]},
                   {"data_rodata_ownership": [self.row(0, 0x20)]}, {}]
        before = copy.deepcopy(modules)
        self.assertEqual(atlas.data_rodata_ownership_totals(modules),
                         {"owned_data_rodata_bytes": 0x30,
                          "diagnostic_data_rodata_bytes": 0})
        self.assertEqual(modules, before)


    def test_unselected_modules_keep_the_same_projection(self):
        module = {
            "overlay": 2, "rom": {"start": "0x1000", "size": "0x30"},
            "bss_size": "0x10",
            "sections": {
                "text": {"start": "0x1000", "size": "0x20"},
                "data_rodata": {"start": "0x1020", "size": "0x0"},
                "reloc1": {"start": "0x1020", "size": "0x8"},
                "reloc2": {"start": "0x1028", "size": "0x8"},
            },
            "text_ownership": [{"offset": "0x0", "type": "c", "source": "overlays/o002/unit"}],
        }
        actual = atlas.render_yaml_block({"modules": [module]})
        with mock.patch.object(atlas, "UNOWNED_BSS_RESERVE_OVERLAYS", frozenset()):
            self.assertEqual(actual, atlas.render_yaml_block({"modules": [module]}))
        self.assertNotIn("type: bss", actual)


if __name__ == '__main__':
    unittest.main()
