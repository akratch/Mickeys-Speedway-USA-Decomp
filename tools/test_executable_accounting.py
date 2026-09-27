#!/usr/bin/env python3
"""Synthetic ownership tests; no game instructions or ROM-derived content."""
import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

import executable_accounting as accounting


class AccountingTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)
        (self.root / 'config').mkdir()
        (self.root / 'docs').mkdir()
        (self.root / 'docs/executable-accounting.md').write_text('reviewed boundaries')
        self.rom = bytes(256)
        self.digest = hashlib.sha1(self.rom).hexdigest()
        self.atlas = {'source': {'sha1': self.digest}, 'modules': [{
            'overlay': 1, 'sections': {'text': {'start': '0x80', 'size': '0x20'}},
            'text_ownership': [
                {'source': 'overlays/o001/body', 'offset': '0x0', 'end_offset': '0x10',
                 'size': '0x10', 'type': 'c', 'matched': True, 'nonmatching': True},
                {'source': 'overlays/o001/tail', 'offset': '0x10', 'end_offset': '0x18',
                 'size': '0x8', 'type': 'asm', 'matched': False, 'nonmatching': False},
                {'source': 'overlays/o001/looks_padding', 'offset': '0x18', 'end_offset': '0x20',
                 'size': '0x8', 'type': 'asm', 'matched': False, 'nonmatching': False}],
            'mixed_tu_exact_c_ranges': [{'offset': '0x0', 'end_offset': '0x8'}],
            'exports': [], 'entrypoints': {}}]}
        self.entries = [
            dict(kind='overlay-trailing-alignment', overlay=1, source='overlays/o001/body',
                 owner_offset='0x0', owner_size='0x10', offset='0xc', size='0x4'),
            dict(kind='overlay-padding', overlay=1, source='overlays/o001/tail',
                 owner_offset='0x10', owner_size='0x8', offset='0x10', size='0x8'),
            dict(kind='resident-trailing-alignment', symbol='body', source='main/body',
                 vram=hex(accounting.ROM_DELTA + 16), owner_size='0x10', offset='0xc', size='0x4'),
            dict(kind='resident-alignment-identity', symbol='alignment', source='30',
                 vram=hex(accounting.ROM_DELTA + 48), owner_size='0x4', offset='0x0', size='0x4')]
        self.funcs = {'body': 16, 'unknownZero': 4, 'alignment': 4, 'matched': 8, 'manual': 8}
        self.addrs = {name: accounting.ROM_DELTA + offset for name, offset in
                      [('body', 16), ('unknownZero', 40), ('alignment', 48), ('matched', 64), ('manual', 72)]}
        self.sources = dict(body='main/body', alignment='30')
        self.matched, self.manual, self.nm = {'matched'}, {'manual'}, {'body'}
        self.write()

    def write(self):
        self.manifest = {'schema_version': 1, 'rom_sha1': self.digest,
                         'review': 'docs/executable-accounting.md', 'entries': self.entries}
        (self.root / 'config/nonexecutable-ranges.us.json').write_text(json.dumps(self.manifest))
        (self.root / 'config/overlays.us.json').write_text(json.dumps(self.atlas))

    def apply(self):
        self.write()
        return accounting.apply_accounting(self.root, self.atlas, self.rom, self.funcs,
                                           self.addrs, self.matched, self.manual, self.nm, self.sources)

    def test_exact_partition_preserves_credit_and_executable_zero_content(self):
        effective, deductions = self.apply()
        self.assertEqual(effective, dict(body=12, unknownZero=4, matched=8, manual=8))
        self.assertEqual(deductions, dict(resident_nonmatching=4, resident_global_asm=4,
                                         overlay_nonmatching=4, overlay_global_asm=8, excluded_bytes=20))
        # Every byte in this synthetic ROM is zero. Only reviewed intervals go.
        # The first 12 resident bytes include its executed zero delay slot.
        self.assertEqual(sum(effective[n] for n in self.matched | self.manual), 16)
        self.assertEqual(len(effective), len(self.funcs) - 1)
        physical = sum(self.funcs.values()) + 32
        executable = sum(effective.values()) + 32 - 12
        credit = 16 + 8  # resident C/manual + overlay exact island
        nm = effective['body'] + (16 - 8 - 4)
        remaining = effective['unknownZero'] + 8  # unreviewed asm named looks_padding
        self.assertEqual(credit + nm + remaining, executable)
        self.assertEqual(physical - executable, deductions['excluded_bytes'])

    def test_no_manifest_entry_means_no_zero_name_or_asm_exclusion(self):
        self.entries = []
        effective, deductions = self.apply()
        self.assertEqual(effective, self.funcs)
        self.assertEqual(deductions['excluded_bytes'], 0)

    def test_reviewed_range_must_remain_zero_but_zero_does_not_create_range(self):
        changed = bytearray(self.rom)
        changed[140] = 1
        self.rom = bytes(changed)
        self.digest = hashlib.sha1(self.rom).hexdigest()
        self.atlas['source']['sha1'] = self.digest
        with self.assertRaisesRegex(RuntimeError, 'bytes changed'):
            self.apply()

    def test_rom_digest_must_be_current(self):
        self.rom = b'x' + self.rom[1:]
        with self.assertRaisesRegex(RuntimeError, 'ROM identity is stale'):
            self.apply()

    def test_atlas_digest_mismatch_refuses_even_without_rom(self):
        self.atlas['source']['sha1'] = '0' * 40
        self.write()
        with self.assertRaisesRegex(RuntimeError, 'identity disagrees'):
            accounting.reviewed_ranges(self.root)

    def test_duplicate_or_overlapping_exclusions_refuse(self):
        self.entries.append(copy.deepcopy(self.entries[0]))
        with self.assertRaisesRegex(RuntimeError, 'overlapping'):
            self.apply()
        self.entries[-1]['offset'], self.entries[-1]['size'] = '0x8', '0x8'
        with self.assertRaisesRegex(RuntimeError, 'overlapping'):
            self.apply()

    def test_changed_atlas_source_extent_or_category_refuses(self):
        original = copy.deepcopy(self.atlas)
        for key, value in [('source', 'renamed'), ('size', '0x14'), ('end_offset', '0x14'),
                           ('nonmatching', False), ('type', 'asm')]:
            with self.subTest(key=key):
                self.atlas = copy.deepcopy(original)
                self.atlas['modules'][0]['text_ownership'][0][key] = value
                with self.assertRaises(RuntimeError):
                    self.apply()

    def test_ambiguous_module_or_overlapping_owner_refuses(self):
        self.atlas['modules'].append(copy.deepcopy(self.atlas['modules'][0]))
        with self.assertRaisesRegex(RuntimeError, 'ambiguous'):
            self.apply()
        self.atlas['modules'].pop()
        duplicate = copy.deepcopy(self.atlas['modules'][0]['text_ownership'][0])
        duplicate['source'] = 'overlays/o001/other'
        duplicate['nonmatching'] = False
        self.atlas['modules'][0]['text_ownership'].append(duplicate)
        with self.assertRaisesRegex(RuntimeError, 'another atlas owner'):
            self.apply()

    def test_exact_island_overlap_refuses(self):
        self.atlas['modules'][0]['mixed_tu_exact_c_ranges'][0]['end_offset'] = '0x10'
        with self.assertRaisesRegex(RuntimeError, 'matched credit'):
            self.apply()

    def test_export_and_entrypoint_into_range_refuse(self):
        for key, value in [('exports', [{'offset': '0xc'}]), ('entrypoints', {'init': '0xc'})]:
            with self.subTest(key=key):
                original = self.atlas['modules'][0][key]
                self.atlas['modules'][0][key] = value
                with self.assertRaisesRegex(RuntimeError, 'entry'):
                    self.apply()
                self.atlas['modules'][0][key] = original

    def test_resident_changed_source_size_address_or_category_refuses(self):
        for mapping, key, value in [(self.sources, 'body', 'main/other'),
                                    (self.funcs, 'body', 20), (self.addrs, 'body', 1000)]:
            original = mapping[key]
            mapping[key] = value
            with self.assertRaisesRegex(RuntimeError, 'resident ownership'):
                self.apply()
            mapping[key] = original
        self.nm.clear()
        with self.assertRaisesRegex(RuntimeError, 'resident category'):
            self.apply()

    def test_resident_credit_conflict_refuses(self):
        for credited in (self.matched, self.manual):
            credited.add('body')
            with self.assertRaisesRegex(RuntimeError, 'credited bytes'):
                self.apply()
            credited.remove('body')

    def test_whole_identity_needs_its_distinct_classification(self):
        self.entries[-1]['kind'] = 'resident-trailing-alignment'
        with self.assertRaisesRegex(RuntimeError, 'distinct review'):
            self.apply()

    def test_resident_alias_or_interior_entry_refuses(self):
        self.funcs['interior'] = 4
        self.addrs['interior'] = self.addrs['body'] + 12
        with self.assertRaisesRegex(RuntimeError, 'another function'):
            self.apply()

    def test_unknown_unaligned_empty_and_non_tail_ranges_refuse(self):
        original = copy.deepcopy(self.entries)
        for key, value in [('kind', 'zero'), ('size', '0x0'), ('offset', '0xb'),
                           ('offset', '0x8'), ('source', '../other')]:
            with self.subTest(key=key, value=value):
                self.entries = copy.deepcopy(original)
                self.entries[0][key] = value
                with self.assertRaises(RuntimeError):
                    self.apply()

    def test_shared_scoreboard_uses_contract_and_rejects_stale_arithmetic(self):
        def snapshot(physical=100, excluded=20, total=80):
            (self.root / 'README.md').write_text(
                f'Physical text: {physical} bytes; reviewed nonexecutable alignment: {excluded} bytes.\n'
                f'| **Whole program** | 40 | {total} | **50.00%** |\n')
        snapshot()
        self.assertEqual(accounting.scoreboard_totals(self.root)['whole_program'], 80)
        for kw in ({'physical': 101}, {'excluded': 16}, {'total': 81}):
            snapshot(**kw)
            with self.assertRaisesRegex(RuntimeError, 'stale'):
                accounting.scoreboard_totals(self.root)

    def test_partial_check_accepts_current_tables_and_checks_contract_without_elf(self):
        import argparse
        import contextlib
        import io
        from unittest.mock import patch
        import progress
        (self.root / 'src').mkdir()
        (self.root / 'symbol_addrs.us.txt').write_text('')
        areas = {name: dict(funcs=0, bytes=0, matched=0, matched_bytes=0, named=0, unnamed=0)
                 for name, _ in progress.AREAS}
        area = dict(funcs=4, bytes=32, matched=1, matched_bytes=8, named=1, unnamed=2)
        areas[progress.AREAS[0][0]] = area
        areas['**total**'] = area.copy()
        st = dict(version='us', resolved_bytes=24, whole_text_bytes=52, resolved_pct=24/52*100,
                  physical_text_bytes=72, excluded_bytes=20, matched_bytes=8, total_bytes=32,
                  byte_pct=25, verified_asm_bytes=8, verified_asm_pct=25, n_verified_asm=1,
                  overlay_matched_bytes=8, overlay_text_bytes=20, overlay_byte_pct=40,
                  n_matched=1, n_total=4, func_pct=25, n_named_funcs=2, named_pct=50,
                  n_named=0, areas=areas, tus=[], func_msg='1 of 4 (25.00%)',
                  byte_msg='24 of 52 (46.15%)', name_msg='0 adopted')
        block = progress.render_markdown(st)
        def check(text):
            (self.root / 'README.md').write_text(progress.BEGIN + '\n' + text + '\n' + progress.END)
            with patch.object(progress, 'ROOT_DIR', str(self.root)), contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                return progress.check_partial(argparse.Namespace(version='us'))
        self.assertEqual(check(block), 0)
        self.assertEqual(check(block.replace('reviewed nonexecutable alignment: 20',
                                            'reviewed nonexecutable alignment: 16')), 1)
        self.assertEqual(check(block.replace('| Functions matched | 1 | 4 | 25.00%',
                                            '| Functions matched | 1 | 4 | 50.00%')), 1)
        self.assertEqual(check(block.replace('| Overlay C | 8 | 20 | 40.00%',
                                            '| Overlay C | 10 | 20 | 50.00%')), 1)
        self.assertEqual(check(block.replace('0 symbols are adopted', '1 symbols are adopted')), 1)


if __name__ == '__main__':
    unittest.main()
