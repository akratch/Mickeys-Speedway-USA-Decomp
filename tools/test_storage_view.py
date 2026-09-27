#!/usr/bin/env python3
"""Synthetic regression fixtures for report-only storage view evidence."""
import copy
import hashlib
import json
import struct
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import storage_view as sv


def ins(op, rs=0, rt=0, immediate=0):
    return (op << 26) | (rs << 21) | (rt << 16) | (immediate & 0xffff)


def reg(fn, rs=0, rt=0, rd=0, shift=0):
    return (rs << 21) | (rt << 16) | (rd << 11) | (shift << 6) | fn


class Object:
    def __init__(self, words=(), relocs=(), symbols=None):
        self.text = b''.join(struct.pack('>I', w) for w in words)
        self.relocs = list(relocs)
        self.syms = list(symbols or [('storage', 0, 0, 16, 0), ('callee', 0, 0, 16, 0)])

    def section_bytes(self, name):
        return self.text

    def relocations(self, target=None):
        return self.relocs

    def symbols(self):
        return self.syms


def use_fixture(unsigned=False):
    # Synthetic address construction, indexed load and call delay slot.
    return Object([
        ins(15, rt=18), ins(9, rs=18, rt=18),
        reg(0, rt=25, rd=13, shift=1), reg(33, rs=18, rt=13, rd=14),
        reg(37, rs=2, rd=16), ins(3), ins(37 if unsigned else 33, rs=14, rt=4),
    ], [('.text', 0, 5, 0), ('.text', 4, 6, 0), ('.text', 20, 4, 1)])


class UseTests(unittest.TestCase):
    def test_signed_indexed_argument_in_delay_slot(self):
        obj = use_fixture()
        rows = sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage'))
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual((row['width'], row['signed'], row['index_scale']), (2, True, 2))
        self.assertEqual((row['call_external'], row['argument_register']), ('callee', 4))
        self.assertFalse(row['index_bounds_proved'])
        self.assertFalse(row['complete_footprint'])

    def test_unsigned_distinguished(self):
        obj = use_fixture(unsigned=True)
        self.assertFalse(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage'))[0]['signed'])

    def test_branch_stops_observation(self):
        obj = use_fixture()
        obj.text = obj.text[:16] + struct.pack('>I', ins(4, rs=2, rt=3)) + obj.text[20:]
        self.assertEqual(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage')), [])

    def test_unknown_operation_stops_observation(self):
        obj = use_fixture()
        obj.text = obj.text[:16] + struct.pack('>I', ins(35, rs=29, rt=14)) + obj.text[20:]
        self.assertEqual(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage')), [])

    def test_relocated_load_not_interpreted_as_plain_memory_use(self):
        obj = use_fixture()
        obj.relocs.append(('.text', 24, 6, 1))
        self.assertEqual(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage')), [])

    def test_base_shift_not_an_independent_index(self):
        obj = use_fixture()
        obj.text = obj.text[:8] + struct.pack('>I', reg(0, rt=18, rd=13, shift=1)) + obj.text[12:]
        self.assertEqual(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage')), [])

    def test_nonzero_base_addend_is_not_silently_ignored(self):
        obj = use_fixture()
        obj.text = obj.text[:4] + struct.pack('>I', ins(9, rs=18, rt=18, immediate=2)) + obj.text[8:]
        self.assertEqual(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage')), [])

    def test_destination_clobber_kills_base(self):
        obj = use_fixture()
        obj.text = obj.text[:16] + struct.pack('>I', reg(37, rs=2, rd=14)) + obj.text[20:]
        self.assertEqual(sv.indexed_halfword_use(obj, sv.named_sites(obj, 'storage')), [])


class IdentityTests(unittest.TestCase):
    def test_external_must_exist_before_metadata(self):
        raw, configured = use_fixture(), use_fixture()
        raw.syms[0] = ('other', 0, 0, 16, 0)
        with self.assertRaisesRegex(sv.ViewError, 'actual compiler external'):
            sv.unchanged_external(raw, configured, 'storage')

    def test_actual_definition_not_external(self):
        obj = use_fixture()
        obj.syms[0] = ('storage', 0, 48, 17, 1)
        with self.assertRaises(sv.ViewError):
            sv.named_sites(obj, 'storage')

    def test_duplicate_names_are_not_merged(self):
        obj = use_fixture()
        obj.syms.append(obj.syms[0])
        with self.assertRaises(sv.ViewError):
            sv.named_sites(obj, 'storage')

    def test_rebind_of_witness_site_rejected(self):
        raw, configured = use_fixture(), use_fixture()
        configured.relocs[1] = ('.text', 4, 6, 1)
        with self.assertRaisesRegex(sv.ViewError, 'changed through metadata'):
            sv.unchanged_external(raw, configured, 'storage')

    def test_witness_addend_mutation_rejected(self):
        raw, configured = use_fixture(), use_fixture()
        configured.text = configured.text[:7] + b'\x02' + configured.text[8:]
        with self.assertRaisesRegex(sv.ViewError, 'storage use changed'):
            sv.unchanged_external(raw, configured, 'storage')

    def test_unchanged_external_accepted(self):
        self.assertEqual(sv.unchanged_external(use_fixture(), use_fixture(), 'storage'), [(0, 5), (4, 6)])

    def test_opcode_mutation_rejected_even_at_relocated_site(self):
        raw, configured = use_fixture(), use_fixture()
        configured.text = struct.pack('>I', ins(9, rt=18)) + configured.text[4:]
        with self.assertRaisesRegex(sv.ViewError, 'instruction bits'):
            sv.unchanged_instruction_bits(raw, configured)

    def test_relocation_field_change_is_metadata_not_instruction_change(self):
        raw, configured = use_fixture(), use_fixture()
        configured.text = configured.text[:7] + b'\x02' + configured.text[8:]
        sv.unchanged_instruction_bits(raw, configured)

    def test_nonzero_tail_not_padding(self):
        raw, configured = use_fixture(), use_fixture()
        raw.text += struct.pack('>I', ins(9, rs=1, rt=2))
        with self.assertRaisesRegex(sv.ViewError, 'executable extent'):
            sv.unchanged_instruction_bits(raw, configured)

    def test_zero_tail_permitted(self):
        raw, configured = use_fixture(), use_fixture()
        raw.text += bytes(4)
        sv.unchanged_instruction_bits(raw, configured)


class NamedConflictTests(unittest.TestCase):
    def test_abs_zero_is_not_a_storage_definition(self):
        obj = Object(symbols=[('view', 0, 0, 16, sv.rs.SHN_ABS)])
        obj.names = ['']
        sv.reject_name_conflicts(obj, 'view', (0xffd, 32), 3)

    def test_named_local_definition_conflicts_with_reserved_view(self):
        obj = Object(symbols=[('view', sv.rs.SYNTHETIC_VMA + 32, 4, 17, 1)])
        obj.names = ['', '.overlay_003']
        with self.assertRaisesRegex(sv.ViewError, 'actual named definition'):
            sv.reject_name_conflicts(obj, 'view', (0xffd, 32), 3)

    def test_actual_local_definition_can_agree(self):
        obj = Object(symbols=[('view', sv.rs.SYNTHETIC_VMA + 32, 4, 17, 1)])
        obj.names = ['', '.overlay_003']
        sv.reject_name_conflicts(obj, 'view', (3, 32), 3)

    def test_resident_is_not_reserved_even_with_same_physical_address(self):
        obj = Object(symbols=[('view', sv.ot.RESIDENT_VRAM_BASE + 32, 4, 17, 1)])
        obj.names = ['', '.main']
        with self.assertRaises(sv.ViewError):
            sv.reject_name_conflicts(obj, 'view', (0xffd, 32), 3)


class LoaderTests(unittest.TestCase):
    def setUp(self):
        self.cpp = 'void *ResolveRelocAddress(void) { return 0; }'
        self.linked = Object(symbols=[('D_80078D60', 0x1000, 4, 17, 1),
                                     ('func_80000450', sv.ot.RESIDENT_VRAM_BASE, 192, 18, 1)])
        self.owner = {'symbol': 'owned', 'address': 0x1040, 'size': 48}
        self.patch = mock.patch.object(sv, 'LOADER_BODY', sv.body_digest(self.cpp, 'ResolveRelocAddress'))
        self.patch.start()
        self.addCleanup(self.patch.stop)

    def test_selector_distinction_survives_equal_physical_addresses(self):
        one = sv.loader_view((0xffd, 64), self.owner, self.linked, self.cpp)
        two = sv.loader_view((0xffe, 64), self.owner, self.linked, self.cpp)
        self.assertEqual(one['physical_address'], two['physical_address'])
        self.assertNotEqual(one['selector'], two['selector'])

    def test_changed_loader_semantics_rejected(self):
        with self.assertRaisesRegex(sv.ViewError, 'semantics changed'):
            sv.loader_view((0xffd, 64), self.owner, self.linked, self.cpp.replace('return 0', 'return 1'))

    def test_comments_and_layout_do_not_change_reviewed_body(self):
        changed = self.cpp.replace('return 0;', '/* comment */\n return\t0 ;')
        sv.loader_view((0xffd, 64), self.owner, self.linked, changed)

    def test_overlay_local_identity_rejected(self):
        with self.assertRaisesRegex(sv.ViewError, 'reserved-data'):
            sv.loader_view((60, 64), self.owner, self.linked, self.cpp)

    def test_bss_not_promised_by_data_only_route(self):
        with self.assertRaises(sv.ViewError):
            sv.loader_view((0xfff, 64), self.owner, self.linked, self.cpp)

    def test_containment_does_not_bind_an_interior_alias(self):
        with self.assertRaisesRegex(sv.ViewError, 'start at named'):
            sv.loader_view((0xffd, 66), self.owner, self.linked, self.cpp)

    def test_changed_anchor_rejected(self):
        self.linked.syms[0] = ('D_80078D60', 0x1002, 4, 17, 1)
        with self.assertRaises(sv.ViewError):
            sv.loader_view((0xffd, 64), self.owner, self.linked, self.cpp)

    def test_duplicate_anchor_rejected(self):
        self.linked.syms.append(self.linked.syms[0])
        with self.assertRaisesRegex(sv.ViewError, 'ambiguous'):
            sv.loader_view((0xffd, 64), self.owner, self.linked, self.cpp)


class ResidentOwnerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.path = self.root / 'build/data.o'
        self.path.parent.mkdir()
        self.path.write_bytes(b'synthetic object')
        self.address = sv.ot.VRAM_ROM_DELTA + 16
        self.linked = Object(symbols=[('owned', self.address, 4, 17, 1)])
        self.linked.names = ['', '.main']
        self.linked.sh = [(), (0, 1, 2, self.address, 0, 4, 0, 0, 4, 0)]
        self.linked.text = b'abcd'
        self.owner = Object(symbols=[('owned', 0, 4, 17, 1)])
        self.owner.names = ['', '.data']
        self.header = [0, 1, 2, 0, 0, 4, 0, 0, 4, 0]
        self.owner.section = lambda _: (1, self.header)
        self.owner.text = b'abcd'
        self.rom = bytes(16) + b'abcd'
        for patch in (
            mock.patch.object(sv, 'ROOT', self.root),
            mock.patch.object(sv.rs, 'Elf', return_value=self.owner),
            mock.patch.object(sv.rs, 'linked_input_sections', return_value={('build/data.o', '.data'): (self.address, 4)}),
        ):
            patch.start()
            self.addCleanup(patch.stop)

    def test_actual_named_owner_and_retail_bytes(self):
        result = sv.resident_owner(self.linked, self.rom, 'owned')
        self.assertEqual(result['size'], 4)
        self.assertEqual(result['input_section'], '.data')

    def test_moved_input_definition_rejected(self):
        self.owner.syms[0] = ('owned', 2, 4, 17, 1)
        with self.assertRaises(sv.ViewError):
            sv.resident_owner(self.linked, self.rom, 'owned')

    def test_changed_extent_rejected(self):
        self.owner.syms[0] = ('owned', 0, 2, 17, 1)
        with self.assertRaises(sv.ViewError):
            sv.resident_owner(self.linked, self.rom, 'owned')

    def test_nobits_is_not_initialized_data(self):
        self.header[1] = 8
        with self.assertRaises(sv.ViewError):
            sv.resident_owner(self.linked, self.rom, 'owned')

    def test_data_relocation_rejected(self):
        self.owner.relocs = [('.data', 0, 2, 0)]
        with self.assertRaises(sv.ViewError):
            sv.resident_owner(self.linked, self.rom, 'owned')

    def test_edited_bytes_rejected(self):
        self.owner.text = b'abce'
        with self.assertRaises(sv.ViewError):
            sv.resident_owner(self.linked, self.rom, 'owned')

    def test_zero_size_does_not_infer_next_symbol_boundary(self):
        self.linked.syms[0] = ('owned', self.address, 0, 17, 1)
        with self.assertRaises(sv.ViewError):
            sv.resident_owner(self.linked, self.rom, 'owned')


if __name__ == '__main__':
    unittest.main()
