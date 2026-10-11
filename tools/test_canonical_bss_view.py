#!/usr/bin/env python3
"""Synthetic typed NOBITS and namespace guards; no ROM-derived fixtures."""
import copy
import hashlib
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

import canonical_bss_view as b


class Elf:
    names = ['', '.bss', '.main_bss']
    sh = [(), (0, 8, 3, 0, 0, 32, 0, 0, 16, 0)]

    def __init__(self):
        self.rows = [(name, i * 16, 16, 0x11, 1) for i, name in enumerate(b.OWNER_NAMES)]
        self.sh = copy.deepcopy(self.sh)
        self.relocs = []

    def symbols(self):
        return self.rows

    def relocations(self):
        return self.relocs


class Guards(unittest.TestCase):
    def test_exact_owner(self):
        b.owner_declarations('/* role */ void *D_800D60B0[4];\nvoid *D_800D60C0[4];')
        b.bss_layout(Elf())

    def test_owner_rejects_initializer_type_extra_and_wrong_count(self):
        source = 'void *D_800D60B0[4];void *D_800D60C0[4];'
        for changed in (source.replace('[4]', '[3]', 1), source.replace('void', 'char', 1),
                        source.replace('[4];', '[4]={0};', 1), source + 'int extra;',
                        '#if 0\n' + source + '\n#endif'):
            with self.subTest(changed=changed), self.assertRaises(b.BssViewError):
                b.owner_declarations(changed)

    def test_candidate_exact_declaration(self):
        b.candidate_declaration('extern void *D_800D60B0[4]; void f(void){use(D_800D60B0);}', b.OWNER_NAMES[0])

    def test_candidate_bad_declarations(self):
        good = 'extern void *D_800D60B0[4];'
        for source in (good * 2, good.replace('[4]', '[8]'), good.replace('void', 'char'),
                       good + 'int D_800D60B0;', '#define D_800D60B0 foo\n' + good):
            with self.subTest(source=source), self.assertRaises(b.BssViewError):
                b.candidate_declaration(source, b.OWNER_NAMES[0])

    def test_geometry_rejects_data_width_common_overlap_and_duplicates(self):
        for kind in ('data', 'wide', 'common', 'overlap', 'duplicate', 'relocation'):
            elf = Elf()
            if kind == 'data':
                h = list(elf.sh[1]); h[1] = 1; elf.sh[1] = tuple(h)
            elif kind == 'wide':
                elf.rows[0] = (b.OWNER_NAMES[0], 0, 32, 0x11, 1)
            elif kind == 'common':
                elf.rows[0] = (b.OWNER_NAMES[0], 0, 16, 0x11, 0xFFF2)
            elif kind == 'overlap':
                elf.rows.append(('other', 4, 4, 0x11, 1))
            elif kind == 'duplicate':
                elf.rows.append(elf.rows[0])
            else:
                elf.relocs.append(('.bss', 0, 2, 0))
            with self.subTest(kind=kind), self.assertRaises(b.BssViewError):
                b.bss_layout(elf)

    def linked(self):
        elf = Elf()
        elf.sh.append((0, 8, 3, 0x80085000, 0, 0x2000, 0, 0, 16, 0))
        elf.rows = [(name, 0x80086000 + i * 16, 16, 0x11, 2)
                    for i, name in enumerate(b.OWNER_NAMES)]
        elf.rows += [('D_80085A40', 0x80085A40, 0, 0, 2),
                     ('func_80000450', b.sv.ot.RESIDENT_VRAM_BASE, 4, 0x12, 1)]
        return elf

    def test_loader_retains_reserved_namespace(self):
        with mock.patch.object(b.sv, 'body_digest', return_value=b.sv.LOADER_BODY):
            self.assertEqual(b.loader_identity(b.OWNER_NAMES[0], self.linked(), 0x80086000, ''),
                             (0xFFF, 0x5C0))

    def test_loader_change_and_abs_owner_fail(self):
        with mock.patch.object(b.sv, 'body_digest', return_value='changed'), self.assertRaises(b.BssViewError):
            b.loader_identity(b.OWNER_NAMES[0], self.linked(), 0x80086000, '')
        elf = self.linked(); row = list(elf.rows[0]); row[4] = 0xFFF1; elf.rows[0] = tuple(row)
        with mock.patch.object(b.sv, 'body_digest', return_value=b.sv.LOADER_BODY), self.assertRaises(b.BssViewError):
            b.loader_identity(b.OWNER_NAMES[0], elf, 0x80086000, '')

    def test_full_collect_terminal_recheck_and_derived_identity(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            source = root / b.OWNER_SOURCE
            configured = root / b.OWNER_OBJECT
            candidate_source = root / 'candidate.c'
            candidate_path = root / 'candidate.o'
            target_path = root / 'build/mickey.us.elf'
            for path in (source, configured, candidate_source, candidate_path, target_path):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b'object')
            source.write_text('void *D_800D60B0[4];void *D_800D60C0[4];')
            candidate_source.write_text('extern void *D_800D60B0[4];')
            (root / 'mickey.us.yaml').write_text(
                '- { type: .bss, vram: 0x80086000, name: main/fx_texture_buffers }\n')
            (root / 'build/mickey.us.map').write_text(
                ' .bss 0x80086000 0x20 ' + b.OWNER_OBJECT + '\n')
            candidate = Elf(); candidate.path = candidate_path; candidate.data = b'object'
            candidate.rows = [(b.OWNER_NAMES[0], 0, 0, 0x11, 0)]
            linked = self.linked(); linked.path = target_path; linked.data = b'object'
            current = {'version': 'original'}
            capture_inputs = copy.deepcopy(current)

            def capture(resolution, directory):
                raw = Elf(); raw.path = directory / 'raw.o'; raw.path.write_bytes(b'raw')
                return raw, {'inputs': capture_inputs, 'metadata_plan': []}, lambda: dict(current), source.read_text()

            def loader(symbol, directory):
                return (None, {'preflight': 'complete'}, None,
                        {'inputs': capture_inputs}, lambda: dict(current), 'loader')

            with mock.patch.object(b, 'ROOT', root), mock.patch.object(b.sv, 'ROOT', root), \
                 mock.patch.object(b.sv.fp, 'TARGET_ELF', target_path), \
                 mock.patch.object(b, 'OWNER_SOURCE_SHA256', hashlib.sha256(source.read_bytes()).hexdigest()), \
                 mock.patch.object(b, 'LOADED', {}), mock.patch.object(b.sv, 'capture', side_effect=capture), \
                 mock.patch.object(b.sv, 'exact_report', side_effect=loader), \
                 mock.patch.object(b.rs, 'Elf', return_value=Elf()), \
                 mock.patch.object(b.sv, 'body_digest', return_value=b.sv.LOADER_BODY):
                proof = b.collect(candidate, {b.OWNER_NAMES[0]: 0xFFF}, linked,
                                  root=root, candidate_source=candidate_source)
                self.assertEqual(proof['identities'], {b.OWNER_NAMES[0]: (0xFFF, 0x5C0)})
                self.assertFalse(proof['receipt']['candidate_compilation_proved_here'])
                self.assertTrue(proof['recheck']())
                current['version'] = 'mutated dependency'
                with self.assertRaisesRegex(b.BssViewError, 'closure changed'):
                    proof['recheck']()
                current['version'] = 'original'
                candidate_path.write_bytes(b'changed')
                with self.assertRaisesRegex(b.BssViewError, 'input changed'):
                    proof['recheck']()

    def test_request_schema_rejects_bool_and_unknown(self):
        with self.assertRaises(b.BssViewError):
            b.collect(None, {b.OWNER_NAMES[0]: True}, None)
        with self.assertRaises(b.BssViewError):
            b.collect(None, ['unknown'], None)


if __name__ == '__main__':
    unittest.main()
