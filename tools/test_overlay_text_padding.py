#!/usr/bin/env python3
"""Module-end alignment requires a reviewed pad and an aligned final TU."""

import copy
import unittest

from overlay_atlas import compiler_text_padding


class CompilerTextPaddingTests(unittest.TestCase):
    def setUp(self):
        self.module = {
            "overlay": 3,
            "sections": {"text": {"size": "0x40"}},
            "text_ownership": [
                {"type": "c", "source": "overlays/o003/overlay_003",
                 "offset": "0x0", "end_offset": "0x34", "size": "0x34"},
                {"type": "asm", "source": "overlays/o003/alignment",
                 "offset": "0x34", "end_offset": "0x40", "size": "0xC"},
            ],
        }
        self.reviewed = {(3, "overlays/o003/alignment", 0x34, 0xC)}

    def test_uses_reviewed_identity_without_a_padding_name_convention(self):
        self.assertEqual(compiler_text_padding(self.module, self.reviewed),
                         "overlays/o003/alignment")
        self.assertIsNone(compiler_text_padding(self.module, set()))

    def test_unaligned_final_translation_unit_keeps_its_padding_input(self):
        module = copy.deepcopy(self.module)
        module["text_ownership"][0]["offset"] = "0xC"
        module["text_ownership"][0]["size"] = "0x28"
        module["text_ownership"].insert(0, {
            "type": "c", "source": "overlays/o003/init",
            "offset": "0x0", "end_offset": "0xC", "size": "0xC",
        })
        self.assertIsNone(compiler_text_padding(module, self.reviewed))

    def test_aligned_final_translation_unit_supplies_its_padding(self):
        module = copy.deepcopy(self.module)
        module["text_ownership"][0]["offset"] = "0x10"
        module["text_ownership"][0]["size"] = "0x24"
        module["text_ownership"].insert(0, {
            "type": "c", "source": "overlays/o003/init",
            "offset": "0x0", "end_offset": "0x10", "size": "0x10",
        })
        self.assertEqual(compiler_text_padding(module, self.reviewed),
                         "overlays/o003/alignment")

    def test_only_exact_module_end_alignment_is_absorbed(self):
        for change in ("gap", "interior", "size", "alignment"):
            with self.subTest(change=change):
                module = copy.deepcopy(self.module)
                if change == "gap":
                    module["text_ownership"][0]["end_offset"] = "0x30"
                elif change == "interior":
                    module["sections"]["text"]["size"] = "0x50"
                elif change == "size":
                    module["text_ownership"][1]["size"] = "0x8"
                else:
                    module["sections"]["text"]["size"] = "0x44"
                    module["text_ownership"][1]["end_offset"] = "0x44"
                self.assertIsNone(compiler_text_padding(module, self.reviewed))


if __name__ == "__main__":
    unittest.main()
