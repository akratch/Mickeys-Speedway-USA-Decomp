#!/usr/bin/env python3
"""Synthetic literal ownership fixtures; no game payloads or identities."""
import copy
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import permute_batch as batch
import search_literal_bindings as literal
import search_local_bindings as local

SOURCE = b'float fixture(float x) { return x + 1.25f; }'


class LiteralTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.serial = 0
        self.shape = literal.source_literal(batch.local_binding_ast(SOURCE), 'fixture')

    def assemble(self, *, padding=0, value='1.25', load='lwc1', section='.rodata', extra='', named=False, broken=False):
        self.serial += 1
        path = self.root / f'{self.serial}.s'
        owner = 'literal_owner' if named else f'.rodata+{padding}'
        text = ('.set noreorder\n.text\n.globl fixture\n.ent fixture\nfixture:\n'
                f'lui $8,%hi({owner})\n' + ('addiu $8,$0,0\n' if broken else '') +
                f'{load} $f0,%lo({owner})($8)\njr $31\nnop\n.end fixture\n'
                f'.section {section}\n.space {padding}\n')
        if named:
            text += '.globl literal_owner\n.type literal_owner,@object\nliteral_owner:\n'
        text += ('.word other\n' if value == 'relocation' else f'.float {value}\n') + extra
        if named:
            text += '.size literal_owner,.-literal_owner\n'
        path.write_text(text)
        obj = path.with_suffix('.o')
        subprocess.run([str(batch.ROOT/'tools/binutils/mips64-elf-as'), '-32', '-mips2', '-o', str(obj), str(path)], check=True, capture_output=True)
        return obj

    def target(self):
        code = self.root / 'build/src/main/fixture.c.o'
        owner = self.root / 'build/asm/data/200.rodata.s.o'
        code.parent.mkdir(parents=True); owner.parent.mkdir(parents=True)
        instructions = ('.set noreorder\n.text\n.globl fixture\n.ent fixture\nfixture:\n'
                        'lui $8,%hi(literal_owner)\nlwc1 $f0,%lo(literal_owner)($8)\n'
                        'jr $31\nnop\n.end fixture\n')
        for obj, source in ((code, instructions), (owner,
                '.section .rodata\n.globl literal_owner\n.type literal_owner,@object\n'
                'literal_owner:\n.float 1.25\n.space 12\n.size literal_owner,.-literal_owner\n')):
            asm = obj.with_suffix('.s'); asm.write_text(source)
            subprocess.run([str(batch.ROOT/'tools/binutils/mips64-elf-as'), '-32', '-o', str(obj), str(asm)], check=True, capture_output=True)
        script = self.root/'layout.ld'
        script.write_text('SECTIONS { .main 0x81000000 : { ' + str(code) + '(.text) . = 0x200; ' + str(owner) + '(.rodata) } }')
        linked = self.root/'build/mickey.us.elf'
        subprocess.run([str(batch.ROOT/'tools/binutils/mips64-elf-ld'), '-m', 'elf32ebmip', '-T', str(script), '-o', str(linked), str(code), str(owner)], check=True, capture_output=True)
        (self.root/'build/mickey.us.map').write_text(' .rodata 0x81000200 0x10 build/asm/data/200.rodata.s.o\n')
        (self.root/'mickey.us.yaml').write_text('segments:\n  - name: main\n    type: code\n    start: 0\n    vram: 0x81000000\n    subsegments:\n      - [0, c, main/fixture]\n      - [0x200, rodata]\n      - [0x210, rodata]\n')
        rom = self.root/'baseroms/mickey.us.z64'; rom.parent.mkdir()
        rom.write_bytes(local.rs.Elf(linked).section_bytes('.main'))
        (self.root/'links.txt').write_text('')
        return code, owner, linked

    def test_target_owner_link_yaml_rom_and_source_positive(self):
        self.target()
        candidate = self.assemble(padding=32)
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root/'links.txt'):
            proof = literal.authenticate(self.root, 'fixture', 'main/fixture', candidate, self.shape)
        self.assertEqual(len(proof['records']), 2)
        self.assertEqual(proof['target']['name'], 'literal_owner')

    def test_target_wrong_rom_and_yaml_refuse(self):
        self.target(); candidate = self.assemble()
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root/'links.txt'):
            rom = self.root/'baseroms/mickey.us.z64'
            data = bytearray(rom.read_bytes()); data[0x200] ^= 1; rom.write_bytes(data)
            with self.assertRaisesRegex(RuntimeError, 'payload differs'):
                literal.authenticate(self.root, 'fixture', 'main/fixture', candidate, self.shape)
            data[0x200] ^= 1; rom.write_bytes(data)
            config = self.root/'mickey.us.yaml'
            config.write_text(config.read_text().replace('[0x200, rodata]', '[0x200, rodata, foreign]'))
            with self.assertRaisesRegex(RuntimeError, 'owners disagree'):
                literal.authenticate(self.root, 'fixture', 'main/fixture', candidate, self.shape)

    def test_target_ambiguous_map_refuses(self):
        self.target(); candidate = self.assemble()
        path = self.root/'build/mickey.us.map'; path.write_text(path.read_text()*2)
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root/'links.txt'):
            with self.assertRaisesRegex(RuntimeError, 'ambiguous'):
                literal.authenticate(self.root, 'fixture', 'main/fixture', candidate, self.shape)

    def test_pool_shift_keeps_access_and_value(self):
        a = literal.candidate_literal(self.assemble(), 'fixture', self.shape)
        b = literal.candidate_literal(self.assemble(padding=32), 'fixture', self.shape)
        literal.fidelity(a, b)
        self.assertNotEqual(a['literal_offset'], b['literal_offset'])

    def test_value_change_refuses(self):
        with self.assertRaisesRegex(RuntimeError, 'payload'):
            literal.candidate_literal(self.assemble(value='2.5'), 'fixture', self.shape)

    def test_double_width_refuses(self):
        with self.assertRaisesRegex(RuntimeError, 'LUI/LWC1'):
            literal.candidate_literal(self.assemble(load='ldc1'), 'fixture', self.shape)

    def test_relocation_payload_refuses(self):
        with self.assertRaisesRegex(RuntimeError, 'relocation-bearing'):
            literal.candidate_literal(self.assemble(value='relocation'), 'fixture', self.shape)

    def test_named_owner_is_not_anonymous(self):
        with self.assertRaisesRegex(RuntimeError, 'section-symbol|named object'):
            literal.candidate_literal(self.assemble(named=True), 'fixture', self.shape)

    def test_base_redefinition_refuses(self):
        with self.assertRaisesRegex(RuntimeError, 'reaching'):
            literal.candidate_literal(self.assemble(broken=True), 'fixture', self.shape)

    def test_extent_alignment_width_and_write_refuse(self):
        obj = self.assemble()
        elf = literal.rs.Elf(obj)
        for start, width in ((1, 4), (0, 8), (len(elf.section_bytes('.rodata')), 4)):
            with self.subTest(start=start, width=width), self.assertRaises(RuntimeError):
                literal.data_extent(elf, '.rodata', start, width)
        header = elf.section('.rodata')[1]
        replacement = list(header); replacement[2] |= 1
        with patch.object(elf, 'section', return_value=(elf.section('.rodata')[0], replacement)):
            with self.assertRaisesRegex(RuntimeError, 'read-only'):
                literal.data_extent(elf, '.rodata', 0, 4)

    def test_access_and_site_fidelity_refuse(self):
        a = literal.candidate_literal(self.assemble(), 'fixture', self.shape)
        for key, value in (('sites', [[0, 5], [8, 6]]), ('width', 8), ('fields', [1, 2])):
            b = copy.deepcopy(a); b[key] = value
            with self.subTest(key=key), self.assertRaises(RuntimeError):
                literal.fidelity(a, b)

    def test_multiple_or_nonfloat_source_refuses(self):
        for source in (b'float fixture(float x){return x+1.25f+2.5f;}', b'float fixture(float x){return x+1.25;}'):
            with self.assertRaises(local.UnsupportedLocalBinding):
                literal.source_literal(batch.local_binding_ast(source), 'fixture')

    def test_inactive_macro_prelude_validated_not_blindly_removed(self):
        source = b'#define UNUSED(x) (x)\n' + SOURCE
        self.assertEqual(self.shape, literal.source_literal(batch.local_binding_ast(source), 'fixture'))
        with self.assertRaises(Exception):
            batch.local_binding_ast(b'#define VALUE 1.25f\nfloat fixture(float x){return x+VALUE;}')

    def test_unknown_equal_schemas_refuse(self):
        with self.assertRaisesRegex(RuntimeError, 'unsupported.*schema'):
            batch.local_binding_fidelity({'schema': 'unrecognized'}, {'schema': 'unrecognized'})

    def test_routes_cannot_mix(self):
        with self.assertRaisesRegex(RuntimeError, 'routes differ'):
            batch.local_binding_fidelity({'schema':'mickey-resident-float-literal-v1'}, {'schema':'mickey-resident-switch-table-v1'})


if __name__ == '__main__':
    unittest.main()
