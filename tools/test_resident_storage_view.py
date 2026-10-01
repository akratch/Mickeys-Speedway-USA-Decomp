import inspect
import unittest

import resident_storage_view as view


class ResidentStorageViewTests(unittest.TestCase):
    def test_only_fixed_keys_and_no_identity_or_path_arguments(self):
        self.assertEqual(set(view.WITNESSES), {'impact_gate', 'color_gate', 'gravity'})
        self.assertEqual(tuple(inspect.signature(view.collect).parameters), ('key',))
        with self.assertRaisesRegex(view.EvidenceError, 'unknown fixed witness key'):
            view.collect('D_8007BF04')

    def test_recipe_fingerprint_is_closed(self):
        for key, spec in view.WITNESSES.items():
            view._require_recipe(key, spec['recipe'])
            with self.assertRaisesRegex(view.EvidenceError, 'recipe'):
                view._require_recipe(key, '0' * 64)

    def test_typed_global_owner_requires_one_unconditional_definition(self):
        self.assertEqual(view._global_object_declaration('f32 D_800CB304;\n', 'D_800CB304', 'f32'), 1)
        for invalid in (
            'extern f32 D_800CB304;\n',
            'f32 D_800CB304;\nf32 D_800CB304;\n',
            '#ifdef FEATURE\nf32 D_800CB304;\n#endif\n',
            'u32 D_800CB304;\n',
        ):
            with self.subTest(invalid=invalid), self.assertRaises(view.EvidenceError):
                view._global_object_declaration(invalid, 'D_800CB304', 'f32')

    def test_byte_view_rejects_wrong_opcode_or_nonzero_offset(self):
        # LBU rt,0(base): opcode 36; owner subobject is exactly byte zero.
        self.assertEqual(view._require_byte_zero_load(bytes.fromhex('90020000'), 0)['owner_offset'], 0)
        with self.assertRaisesRegex(view.EvidenceError, 'LBU'):
            view._require_byte_zero_load(bytes.fromhex('8c020000'), 0)
        with self.assertRaisesRegex(view.EvidenceError, 'LBU'):
            view._require_byte_zero_load(bytes.fromhex('90020001'), 0)

    def test_byte_view_rejects_out_of_section_site(self):
        with self.assertRaisesRegex(view.EvidenceError, 'outside'):
            view._require_byte_zero_load(bytes.fromhex('90020000'), 4)


if __name__ == '__main__':
    unittest.main()
