import inspect
import unittest
import time
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

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

    def test_dry_run_recipe_pins_every_target_command(self):
        source=view.ROOT/'src/main/objects.c'; target=view.ROOT/'build/src/main/objects.c.o'
        actual=view.batch.bounded_capture(['gmake','--no-print-directory','-n','-W',
            source.relative_to(view.ROOT).as_posix(),target.relative_to(view.ROOT).as_posix()],time.monotonic()+30,check=True).stdout
        with mock.patch.object(view.batch,'bounded_capture',return_value=SimpleNamespace(stdout=actual)):
            _,_,_,_,baseline=view._recipe(source,target,time.monotonic()+30)
        view._require_recipe('color_gate',baseline)
        injected=actual+'\npython3 tools/new_unreviewed_metadata.py '+target.relative_to(view.ROOT).as_posix()+'\n'
        with mock.patch.object(view.batch,'bounded_capture',return_value=SimpleNamespace(stdout=injected)):
            _,_,_,_,changed=view._recipe(source,target,time.monotonic()+30)
        self.assertNotEqual(baseline,changed)
        with self.assertRaisesRegex(view.EvidenceError,'recipe'):
            view._require_recipe('color_gate',changed)

    def test_recipe_rejects_unknown_shell_construct_touching_target(self):
        source=view.ROOT/'src/main/objects.c'; target=view.ROOT/'build/src/main/objects.c.o'
        sr=source.relative_to(view.ROOT).as_posix(); tr=target.relative_to(view.ROOT).as_posix()
        output=f'tools/ido/cc -c -o {tr} {sr}\npython3 tools/new_unreviewed_metadata.py {tr} ; echo $HOME\n'
        with mock.patch.object(view.batch,'bounded_capture',return_value=SimpleNamespace(stdout=output)):
            with self.assertRaisesRegex(view.EvidenceError,'shell construct'):
                view._recipe(source,target,time.monotonic()+30)

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

    def test_loaded_transitive_and_executed_helpers_are_pinned(self):
        module_path=Path(view.cc.__file__)
        with mock.patch.dict(view.LOADED,{module_path:'0'*64}):
            with self.assertRaisesRegex(view.EvidenceError,'loaded evidence implementation changed'):
                view._verify_loaded_proofs()
        trim= view.ROOT/'tools/trim_elf_bss.py'
        self.assertIn(trim,view.LOADED)
        with mock.patch.dict(view.LOADED,{trim:'0'*64}):
            with self.assertRaisesRegex(view.EvidenceError,'loaded evidence implementation changed'):
                view._verify_loaded_proofs()

    def test_final_snapshot_rejects_stale_loader_owner_or_header_inputs(self):
        base={'additional_input_files':{'loader_object':'a','initialized_owner_object':'b'},
              'dependency_files':{'include/loader.h':'c'}}
        for section,key in [('additional_input_files','loader_object'),
                            ('additional_input_files','initialized_owner_object'),
                            ('dependency_files','include/loader.h')]:
            changed={**base,section:{**base[section],key:'changed'}}
            with self.subTest(section=section,key=key), self.assertRaisesRegex(view.EvidenceError,'snapshot changed'):
                view._assert_same_snapshot(base,changed,'fresh input')


if __name__ == '__main__':
    unittest.main()
