import inspect
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

import resident_storage_bindings as bindings


class ResidentStorageBindingTests(unittest.TestCase):
    def test_registry_is_fixed_and_public_api_has_no_overrides(self):
        self.assertEqual(set(bindings.BINDINGS), {'impact_gate','color_gate','gravity'})
        self.assertEqual(tuple(inspect.signature(bindings.collect).parameters), ('key',))
        self.assertEqual(tuple(inspect.signature(bindings.recheck).parameters), ('handle',))
        for key in ('D_8007BF04','gO8P1294ImpactGateReloc','/tmp/witness.json','impact_gate:4093,12708'):
            with self.subTest(key=key), self.assertRaisesRegex(bindings.BindingError,'unknown fixed'):
                bindings.collect(key)
        with self.assertRaisesRegex(bindings.BindingError,'invalid opaque'):
            bindings.recheck('/tmp/anything')

    def test_identity_authority_is_hashed_and_matches_all_three_fixed_rows(self):
        bindings._verify_loaded()
        expected={
            'impact_gate':('gO8P1294ImpactGateReloc','D_8007BF04',(0xFFD,0x31A4),'u8',1),
            'color_gate':('gO8P1294ColorGateReloc','D_8007BF0C',(0xFFD,0x31AC),'u8',1),
            'gravity':('gO8P1294MotionScalarReloc','D_800CB304',(0xFFF,0x458C4),'f32',4),
        }
        for key,row in expected.items():
            spec=bindings.BINDINGS[key]
            self.assertEqual((spec['carrier'],spec['owner'],spec['identity'],spec['ctype'],spec['size']),row)

    def test_alias_swap_and_semantic_type_or_extent_mutation_fail_authority(self):
        original=bindings.BINDINGS['impact_gate']
        swapped={**original,'identity':bindings.BINDINGS['color_gate']['identity']}
        with mock.patch.dict(bindings.BINDINGS,{'impact_gate':swapped}):
            with self.assertRaisesRegex(bindings.BindingError,'fixed authority disagrees'):
                bindings._verify_loaded()
        self.assertEqual(bindings.BINDINGS['impact_gate'],original)

    def test_typed_carrier_rejects_wrong_size_binding_type_and_definition(self):
        spec=bindings.BINDINGS['impact_gate']
        for row in (
            ('gO8P1294ImpactGateReloc',0,4,(1<<4)|1,0),
            ('gO8P1294ImpactGateReloc',0,1,(1<<4)|2,0),
            ('gO8P1294ImpactGateReloc',0,1,(0<<4)|1,0),
            ('gO8P1294ImpactGateReloc',0,1,(1<<4)|1,1),
        ):
            elf=SimpleNamespace(symbols=lambda row=row:[row])
            with self.subTest(row=row), self.assertRaisesRegex(bindings.BindingError,'external GLOBAL UND STT_OBJECT'):
                bindings._carrier_record(elf,spec)
        elf=SimpleNamespace(symbols=lambda:[('gO8P1294ImpactGateReloc',0,1,17,0)]*2)
        with self.assertRaisesRegex(bindings.BindingError,'missing or duplicated'):
            bindings._carrier_record(elf,spec)

    def test_carrier_source_declaration_rejects_wrong_type_scope_or_conditional(self):
        spec=bindings.BINDINGS['gravity']
        with tempfile.TemporaryDirectory() as temp:
            source=Path(temp)/'source.c'
            for text in ('extern u32 gO8P1294MotionScalarReloc;\n',
                         'extern f32 otherName;\n',
                         '#ifdef FEATURE\nextern f32 gO8P1294MotionScalarReloc;\n#endif\n',
                         'extern f32 gO8P1294MotionScalarReloc;\nextern f32 gO8P1294MotionScalarReloc;\n'):
                source.write_text(text)
                with self.subTest(text=text), self.assertRaises(bindings.BindingError):
                    bindings._source_declaration(source,spec)

    def test_candidate_translation_unit_cannot_be_overridden(self):
        wrong=SimpleNamespace(source=bindings.ROOT/'src/main/objects.c',
            candidate_object=bindings.ROOT/'build_non_matching/src/main/objects.c.o')
        with mock.patch.object(bindings.fp,'resolve',return_value=wrong):
            with tempfile.TemporaryDirectory() as temp, self.assertRaisesRegex(bindings.BindingError,'fixed candidate scope'):
                bindings._candidate_capture(Path(temp))

    def test_changed_loaded_helper_fails_closed(self):
        path=Path(bindings.view.__file__)
        with mock.patch.dict(bindings._LOADED,{path:'0'*64}):
            with self.assertRaisesRegex(bindings.BindingError,'loaded binding evidence implementation changed'):
                bindings._verify_loaded()

    def test_wrong_owner_identity_or_physical_extent_is_rejected(self):
        spec=bindings.BINDINGS['color_gate']
        self.assertEqual(spec['identity'],(0xFFD,0x31AC))
        self.assertNotEqual(spec['identity'],bindings.BINDINGS['impact_gate']['identity'])
        with mock.patch.object(bindings.view,'collect',return_value={
                'status':'scoped-independent-witness-complete','resolver_admission':False,
                'matching_credit':0,'source':'src/main/objects.c',
                'semantic_view':{'type':'u8','offset':1,'extent':1,'physical_object_extent':4},
                'owner':{'symbol':'D_8007BF04','size':4,'input_section':'.data','address':1},
                'loader_view':{'selector':0xFFD,'offset':0x31A4,'physical_address':1}}), \
             mock.patch.object(bindings,'_candidate_capture') as candidate:
            with self.assertRaisesRegex(bindings.BindingError,'semantic byte view'):
                bindings.collect('color_gate')
            candidate.assert_not_called()

    def test_wrong_named_owner_and_wrong_physical_extent_are_rejected(self):
        for owner in ({'symbol':'D_8007BF0C','size':4,'input_section':'.data','address':3},
                      {'symbol':'D_8007BF04','size':8,'input_section':'.data','address':3},
                      {'symbol':'D_8007BF04','size':4,'input_section':'.bss','address':3}):
            witness={'status':'scoped-independent-witness-complete','resolver_admission':False,
                'matching_credit':0,'source':'src/main/anim.c',
                'semantic_view':{'type':'u8','offset':0,'extent':1,'physical_object_extent':4},
                'owner':owner,'loader_view':{'selector':0xFFD,'offset':0x31A4,'physical_address':3}}
            with self.subTest(owner=owner), mock.patch.object(bindings.view,'collect',return_value=witness), \
                 mock.patch.object(bindings,'_candidate_capture') as candidate, \
                 self.assertRaisesRegex(bindings.BindingError,'named initialized'):
                bindings.collect('impact_gate')
                candidate.assert_not_called()

    def test_wrong_gravity_namespace_owner_type_or_extent_is_rejected(self):
        witness={'status':'scoped-independent-witness-complete','resolver_admission':False,
            'matching_credit':0,'source':'src/main/charControl.c',
            'linked_owner':{'name':'D_800CB304','address':0x800CB304,'size':4,'section':'.main_bss'},
            'loader_view':{'selector':0xFFD,'offset':0x31A4,'physical_address':0x800CB304},
            'bss_bytes_read':False,'rom_zero_comparison_performed':False}
        with mock.patch.object(bindings.view,'collect',return_value=witness), \
             mock.patch.object(bindings,'_candidate_capture') as candidate, \
             self.assertRaisesRegex(bindings.BindingError,'typed NOBITS loader'):
            bindings.collect('gravity')
            candidate.assert_not_called()

    def test_gravity_requires_typed_nobits_without_rom_bytes(self):
        # Fixed identity is a separate namespace and cannot be swapped with a gate.
        gravity=bindings.BINDINGS['gravity']
        self.assertEqual(gravity['identity'],(0xFFF,0x458C4))
        with mock.patch.object(bindings.view,'collect',return_value={
                'status':'scoped-independent-witness-complete','resolver_admission':False,
                'matching_credit':0,'source':'src/main/charControl.c',
                'linked_owner':{'name':'D_800CB304','address':0x800CB304,'size':4,'section':'.main_bss'},
                'loader_view':{'selector':0xFFF,'offset':0x458C4,'physical_address':0x800CB304},
                'bss_bytes_read':True,'rom_zero_comparison_performed':False}), \
             mock.patch.object(bindings,'_candidate_capture') as candidate:
            with self.assertRaisesRegex(bindings.BindingError,'NOBITS'):
                bindings.collect('gravity')
            candidate.assert_not_called()

    def test_full_typed_gate_witness_is_checked_before_candidate_capture(self):
        with mock.patch.object(bindings.view,'collect',return_value={
                'status':'scoped-independent-witness-complete','resolver_admission':False,
                'matching_credit':0,'source':'src/main/anim.c',
                'semantic_view':{'type':'u8','offset':0,'extent':1,'physical_object_extent':4},
                'owner':{'symbol':'D_8007BF04','size':4,'input_section':'.data','address':3},
                'loader_view':{'selector':0xFFD,'offset':0x31A4,'physical_address':4}}), \
             mock.patch.object(bindings,'_candidate_capture') as candidate:
            with self.assertRaisesRegex(bindings.BindingError,'loader identity'):
                bindings.collect('impact_gate')
            candidate.assert_not_called()

    def test_recheck_handle_does_not_accept_arbitrary_path_or_owner(self):
        with self.assertRaisesRegex(bindings.BindingError,'invalid opaque'):
            bindings.recheck('0'*63)
        self.assertNotIn('path',inspect.signature(bindings.recheck).parameters)


if __name__=='__main__':
    unittest.main()
