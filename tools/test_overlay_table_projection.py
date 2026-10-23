#!/usr/bin/env python3
"""Runtime relocation tables must not participate in module section layout."""
import unittest

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
                "source": "overlays/o001/unit", "section": ".rodata", "canonical": True,
            }],
        }
        text = atlas.render_yaml_block({"modules": [module]})
        body, tables = text.split("\n  - name: overlay_001_reloc1", 1)
        self.assertIn("bss_size: 0x20", body)
        self.assertIn("[0x1020, .rodata, unit]", body)
        self.assertNotIn("reloc1", body)
        self.assertIn("type: bin\n    start: 0x1030", tables)
        self.assertIn("\n  - name: overlay_001_reloc2", tables)
        self.assertNotIn("bss_size", tables)


if __name__ == '__main__':
    unittest.main()
