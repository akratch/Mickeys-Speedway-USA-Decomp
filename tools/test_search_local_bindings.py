#!/usr/bin/env python3
"""Synthetic switch-table proof fixtures; no game words or extracted payload."""
from __future__ import annotations

import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_batch as batch
import search_local_bindings as local
from test_search_bindings import site, surface

SOURCE = b'''typedef signed short Half; typedef unsigned char Byte;
void fixture(Byte *object) {
    switch (*(Half *)(object + 12)) {
    case 5: case 7: object[0] = 1; break;
    case 8: object[0] = 2; break;
    default: break;
    }
}
'''


class TableProofTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.shape = batch.local_table_shape(SOURCE, 'fixture')
        self.serial = 0

    def assemble(self, text, output=None):
        self.serial += 1
        source = self.root / ('assembly-' + str(self.serial) + '.s')
        output = output or source.with_suffix('.o')
        output.parent.mkdir(parents=True, exist_ok=True)
        source.write_text(text)
        subprocess.run([str(batch.ROOT / 'tools/binutils/mips64-elf-as'), '-32', '-o', str(output), str(source)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return output

    def body(self, table, *, padding=0, load='lh', origin=-5, count=4,
             nops=False, extra='', target=False):
        gap = 'nop\n' if nops else ''
        glob = '.globl default_case,case_a,case_b\n' if target else ''
        return ('.set noreorder\n.text\n.space ' + str(padding) + '\n' + glob +
            '.globl fixture\n.ent fixture\nfixture:\n'
            'or $16,$4,$0\n' + load + ' $8,12($16)\n' +
            f'addiu $9,$8,{origin}\n' + gap + f'sltiu $10,$9,{count}\n' +
            'beq $10,$0,default_case\nsll $9,$9,2\n' +
            f'lui $11,%hi({table})\n' + gap + 'addu $11,$11,$9\n' +
            f'lw $9,%lo({table})($11)\n' + gap + 'jr $9\nnop\n' +
            'case_a:\naddiu $2,$0,1\njr $31\nnop\n'
            'case_b:\naddiu $2,$0,2\njr $31\nnop\n'
            'default_case:\njr $31\nnop\n' + extra + '.end fixture\n')

    def candidate(self, *, padding=0, table_padding=0, entries=None, extra='', **kwargs):
        entries = entries or ['case_a', 'default_case', 'case_a', 'case_b']
        table = '.rodata' + ('+' + str(table_padding) if table_padding else '')
        body = self.body(table, padding=padding, extra=extra, **kwargs)
        body += '.section .rodata\n.space ' + str(table_padding) + '\n'
        body += ''.join('.word ' + label + '\n' for label in entries)
        return self.assemble(body)

    def test_function_relative_table_fidelity_allows_tu_and_data_base_shift(self):
        small = local.candidate_table(self.candidate(), 'fixture', self.shape)
        full = local.candidate_table(self.candidate(padding=80, table_padding=32), 'fixture', self.shape)
        local.fidelity(small, full)
        self.assertNotEqual(small['table_offset'], full['table_offset'])
        self.assertEqual(small['partition'], [0, 1, 0, 2])

    def test_moved_instruction_sites_are_a_search_shape_residual(self):
        baseline = local.candidate_table(self.candidate(), 'fixture', self.shape)
        moved = local.candidate_table(self.candidate(nops=True), 'fixture', self.shape)
        self.assertEqual(baseline['partition'], moved['partition'])
        self.assertNotEqual(baseline['sites'], moved['sites'])
        with self.assertRaisesRegex(RuntimeError, 'fidelity'):
            local.fidelity(baseline, moved)

    def test_dispatch_domain_and_origin_refuse(self):
        for kwargs in ({'load': 'lhu'}, {'origin': -4}, {'count': 5}):
            with self.subTest(kwargs=kwargs), self.assertRaises(RuntimeError):
                local.candidate_table(self.candidate(**kwargs), 'fixture', self.shape)

    def test_case_default_partition_and_escape_refuse(self):
        for entries in (['case_a', 'case_a', 'case_a', 'case_b'],
                        ['case_a', 'default_case', 'case_a', 'fixture+4096']):
            with self.subTest(entries=entries), self.assertRaises(RuntimeError):
                local.candidate_table(self.candidate(entries=entries), 'fixture', self.shape)

    def test_raw_full_destination_mismatch_refuses_even_same_partition(self):
        first = local.candidate_table(self.candidate(), 'fixture', self.shape)
        second = copy.deepcopy(first)
        second['entries'] = [x + 4 for x in second['entries']]
        with self.assertRaisesRegex(RuntimeError, 'entries'):
            local.fidelity(first, second)

    def test_second_table_owner_and_bad_hilo_addend_refuse(self):
        for extra in ('lui $12,%hi(.rodata)\nlw $12,%lo(.rodata)($12)\n',
                      'lui $12,%hi(.rodata+4)\nlw $12,%lo(.rodata+4)($12)\n'):
            with self.subTest(extra=extra), self.assertRaisesRegex(RuntimeError, 'multiple'):
                local.candidate_table(self.candidate(extra=extra), 'fixture', self.shape)
        text = self.body('.rodata+8') + '.section .rodata\n.word case_a,default_case,case_a,case_b\n'
        with self.assertRaisesRegex(RuntimeError, 'extent'):
            local.candidate_table(self.assemble(text), 'fixture', self.shape)

    def test_register_clobber_and_missing_guard_refuse(self):
        for before, after in (('addu $11,$11,$9', 'addu $11,$11,$8'),
                              ('sltiu $10,$9,4', 'slti $10,$9,4'),
                              ('jr $9', 'jr $8')):
            text = self.body('.rodata').replace(before, after)
            text += '.section .rodata\n.word case_a,default_case,case_a,case_b\n'
            with self.subTest(before=before), self.assertRaises(RuntimeError):
                local.candidate_table(self.assemble(text), 'fixture', self.shape)

    def test_source_type_parameter_scale_case_and_second_switch_refuse(self):
        for source in (SOURCE.replace(b'signed short', b'unsigned short'),
                       SOURCE.replace(b'Byte *object', b'int *object'),
                       SOURCE.replace(b'case 7', b'case 5'),
                       SOURCE.replace(b'default: break;', b'default: switch(object[0]) { default: break; }')):
            with self.subTest(source=source), self.assertRaises(RuntimeError):
                batch.local_table_shape(source, 'fixture')

    def target(self):
        target_object = self.assemble(self.body('named_table', target=True), self.root / 'build/src/main/fixture.c.o')
        table = ('.section .rodata\n.globl named_table\n.type named_table,@object\nnamed_table:\n'
                 '.word case_a,default_case,case_a,case_b\n.size named_table,.-named_table\n')
        owner = self.assemble(table, self.root / 'build/asm/data/200.rodata.s.o')
        script = self.root / 'layout.ld'
        script.write_text('SECTIONS { .main 0x81000000 : { ' + str(target_object) + '(.text) . = 0x200; ' + str(owner) + '(.rodata) } }')
        linked = self.root / 'build/mickey.us.elf'
        subprocess.run([str(batch.ROOT / 'tools/binutils/mips64-elf-ld'), '-m', 'elf32ebmip', '-T', str(script),
                        '-o', str(linked), str(target_object), str(owner)], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.root / 'build/mickey.us.map').write_text(' .rodata 0x81000200 0x10 build/asm/data/200.rodata.s.o\n')
        (self.root / 'mickey.us.yaml').write_text('segments:\n  - name: main\n    type: code\n    start: 0\n    vram: 0x81000000\n    subsegments:\n      - [0, c, main/fixture]\n      - [0x200, rodata]\n      - [0x210, rodata]\n')
        rom = self.root / 'baseroms/mickey.us.z64'
        rom.parent.mkdir()
        rom.write_bytes(local.rs.Elf(linked).section_bytes('.main'))
        (self.root / 'links.txt').write_text('')
        return target_object, owner, linked

    def test_named_target_owner_rom_and_case_partition_positive(self):
        self.target()
        path = self.candidate(nops=True, padding=32, table_padding=16)
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root / 'links.txt'):
            proof = local.authenticate(self.root, 'fixture', 'main/fixture', path, self.shape)
        self.assertEqual(proof['candidate']['partition'], proof['target']['partition'])
        self.assertNotEqual(proof['candidate']['dispatch']['hi'], proof['target']['dispatch']['hi'])
        self.assertEqual(len(proof['records']), 2)

    def test_target_owner_namespace_and_rom_negatives(self):
        self.target()
        path = self.candidate()
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root / 'links.txt'):
            with self.assertRaisesRegex(RuntimeError, 'namespace'):
                local.authenticate(self.root, 'fixture', 'overlays/unit', path, self.shape, overlay=2)
            rom = self.root / 'baseroms/mickey.us.z64'
            data = bytearray(rom.read_bytes()); data[-1] ^= 1; rom.write_bytes(data)
            with self.assertRaisesRegex(RuntimeError, 'ROM'):
                local.authenticate(self.root, 'fixture', 'main/fixture', path, self.shape)

    def test_target_case_partition_and_yaml_owner_must_agree(self):
        self.target()
        path = self.candidate(entries=['case_a', 'default_case', 'case_b', 'case_a'])
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root / 'links.txt'):
            with self.assertRaisesRegex(RuntimeError, 'destination partition'):
                local.authenticate(self.root, 'fixture', 'main/fixture', path, self.shape)
            yaml = self.root / 'mickey.us.yaml'
            yaml.write_text(yaml.read_text().replace('[0x200, rodata]', '[0x200, rodata, other]'))
            with self.assertRaisesRegex(RuntimeError, 'owners disagree'):
                local.authenticate(self.root, 'fixture', 'main/fixture', self.candidate(), self.shape)

    def test_target_pc16_actual_addend_must_keep_default_partition(self):
        target, owner, linked = self.target()
        self.assemble(self.body('named_table', target=True).replace(
            'beq $10,$0,default_case', 'beq $10,$0,default_case+4'), target)
        subprocess.run([str(batch.ROOT / 'tools/binutils/mips64-elf-ld'), '-m', 'elf32ebmip', '-T',
                        str(self.root / 'layout.ld'), '-o', str(linked), str(target), str(owner)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (self.root / 'baseroms/mickey.us.z64').write_bytes(local.rs.Elf(linked).section_bytes('.main'))
        with patch.object(local.rs, 'REPO', self.root), patch.object(local.rs, 'LINK_SYMS', self.root / 'links.txt'):
            with self.assertRaisesRegex(RuntimeError, 'delay slot|partition'):
                local.authenticate(self.root, 'fixture', 'main/fixture', self.candidate(), self.shape)

    def test_external_witnesses_still_required_and_counts_may_move(self):
        proof = {'records': [[8, 5, [0, 512]], [16, 6, [0, 512]]]}
        local_rows = [site(8, 5), site(16, 6)]
        for row in local_rows:
            row['observations'] = ['object-local-section-symbol']
        report = surface(site(28), *local_rows, target_count=8)
        self.assertEqual(len(batch.local_search_records(report, proof)), 3)
        report['diagnostics']['sites'][0]['witnesses'][0]['independent'] = False
        with self.assertRaisesRegex(RuntimeError, 'authority'):
            batch.local_search_records(report, proof)

    def test_stale_original_source_and_candidate_proof_refuse(self):
        out = self.root / 'prepared'; out.mkdir()
        (out / 'source-groups.preprocessed.c').write_bytes(SOURCE)
        item = batch.QueueItem('fixture', batch.ROOT / 'src/main/fixture.c')
        with self.assertRaisesRegex(RuntimeError, 'stale'):
            batch.local_table_proof(item, self.candidate(), SOURCE, out,
                                   {'context': {'original_preprocessed': 'incorrect'}})


if __name__ == '__main__':
    unittest.main()
