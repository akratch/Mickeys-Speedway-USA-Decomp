#!/usr/bin/env python3
"""Synthetic candidate ownership; no ROM data or compiler invocations."""
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_batch as pb
import finalize_plateau as fp
import nm_ranking as nm
import lane_status as ls


def guard(body, *names):
    return '#ifdef NON_MATCHING\n' + body + '\n#else\n' + ''.join(
        '#pragma GLOBAL_ASM("asm/' + n + '.s")\n' for n in names) + '#endif\n'


class GuardPairs(unittest.TestCase):
    def test_all_functions_are_enumerated_and_paired_by_name_not_order(self):
        source = guard('void first(void) {}\nvoid second(void) {}', 'second', 'first')
        block, = pb.iter_nonmatching_blocks(source)
        self.assertEqual(pb.block_function_names(source, block), ['first', 'second'])
        self.assertIsNone(pb.block_function_name(source, block))
        self.assertEqual(pb.block_fallbacks(source, block, {}),
                         {'first': 'asm/first.s', 'second': 'asm/second.s'})
        self.assertEqual(fp.require_guarded_candidate(source, 'second').fallback, 'asm/second.s')
        self.assertTrue(ls.guarded_fallback(source, 'second'))
        self.assertIsNotNone(nm.source_context_digest(source, 'second'))

    def test_runtime_else_if_is_not_another_definition(self):
        source = guard("void first(int x) {\n    if (x) {}\n    else if (x < 0) {}\n}", 'first')
        self.assertEqual(fp.require_guarded_candidate(source, 'first').fallback, 'asm/first.s')

    def test_shared_guard_needs_exact_alias_not_generated_position(self):
        source = guard('void first(void) {}\nvoid second(void) {}',
                       'func_overlay_001_F0000010_1000000', 'func_overlay_001_F0000020_1000010')
        block, = pb.iter_nonmatching_blocks(source)
        self.assertEqual(pb.block_fallbacks(source, block, {}), {})
        aliases = {'first': frozenset({'func_overlay_001_F0000020_1000010'}),
                   'second': frozenset({'func_overlay_001_F0000010_1000000'})}
        self.assertEqual(pb.block_fallbacks(source, block, aliases)['second'],
                         'asm/func_overlay_001_F0000010_1000000.s')
        self.assertTrue(ls.guarded_fallback(source, 'second', aliases['second']))

    def test_duplicate_definition_fallback_or_alias_is_ambiguous(self):
        cases = [guard('void first(void) {}\nvoid first(void) {}', 'first', 'other'),
                 guard('void first(void) {}\nvoid second(void) {}', 'first', 'first'),
                 guard('void first(void) {}\nvoid second(void) {}', 'unknown', 'other')]
        for source in cases:
            block, = pb.iter_nonmatching_blocks(source)
            self.assertEqual(pb.block_fallbacks(source, block, {}), {})
        source = guard('void first(void) {}\nvoid second(void) {}', 'one', 'two')
        block, = pb.iter_nonmatching_blocks(source)
        self.assertEqual(pb.block_fallbacks(source, block,
                         {'first': frozenset({'one'}), 'second': frozenset({'one'})}), {})

    def test_conflicting_macro_name_and_malformed_guards_refuse(self):
        text = '#define NAME first\n#define NAME second\n' + guard('void NAME(void) {}', 'first')
        block, = pb.iter_nonmatching_blocks(text)
        self.assertEqual(pb.block_function_names(text, block), [])
        valid = guard('void first(void) {}', 'first')
        for source in [valid.replace('#else', '#elif OTHER'), valid.replace('#endif', ''),
                       '#ifdef OUTER\n' + valid + '#endif\n',
                       valid.replace('#else', '#else\n#else')]:
            self.assertEqual(list(pb.iter_nonmatching_blocks(source)), [])

    def test_unique_identifier_macro_is_authenticated_but_not_header_macro(self):
        text = '#define NAME first\n' + guard('void NAME(void) {}', 'first')
        block, = pb.iter_nonmatching_blocks(text)
        self.assertEqual(pb.block_fallbacks(text, block, {}), {'first': 'asm/first.s'})
        text = '#define HEADER void first(void)\n' + guard('HEADER {}', 'first')
        block, = pb.iter_nonmatching_blocks(text)
        self.assertEqual(pb.block_fallbacks(text, block, {}), {})

    def test_promotion_refuses_shared_guard_without_touching_sibling(self):
        from test_promotion_transaction import Fixture
        with Fixture() as fixture:
            text = fixture.source.read_text().replace(
                '#else', 'int sibling(void) { return 3; }\n#else', 1).replace(
                '#endif', '#pragma GLOBAL_ASM("sibling.s")\n#endif', 1)
            fixture.source.write_text(text)
            fixture.git('add', 'src/fixture.c')
            fixture.git('commit', '-qm', 'shared guard')
            head = fixture.git('rev-parse', 'HEAD')
            with patch.object(pb, 'review_context', return_value={'status': 'unchanged', 'reason': None}), \
                 patch.object(pb, 'validate_baseline'):
                ok, error = fixture.promote()
            self.assertFalse(ok)
            self.assertIn('shared NON_MATCHING guard', error)
            self.assertEqual(fixture.source.read_text(), text)
            self.assertEqual(fixture.git('rev-parse', 'HEAD'), head)
            self.assertFalse(any(call[0] != 'git' for call in fixture.calls))


class Inventory(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.addCleanup(patch.stopall)
        patch.object(pb, 'ROOT', self.root).start()
        patch.object(pb, 'ATLAS_PATH', self.root / 'atlas.json').start()

    def write(self, path, text):
        full = self.root / path
        full.parent.mkdir(parents=True, exist_ok=True)
        full.write_text(text)
        return full

    def test_second_function_gets_own_fallback_and_full_tu_context(self):
        text = guard('void first(void) {}\nvoid second(void) {}', 'second', 'first')
        path = self.write('src/main/test.c', text)
        queue = pb.discover_queue()
        self.assertEqual([x.func for x in queue], ['first', 'second'])
        second = queue[1]
        self.assertEqual(pb.find_asm_target(second), 'asm/second.s')
        self.assertNotEqual(nm.source_context_digest(text, 'second'),
                            nm.source_context_digest(text.replace('first(void)', 'first(int x)'), 'second'))

    def test_include_and_shared_macro_are_visible_but_unresolved(self):
        self.write('src/shared.inc', '#define HEADER void included(void)\nHEADER {}\n')
        self.write('src/overlays/o001/a.c', guard('#include "src/shared.inc"', 'included'))
        self.write('src/overlays/o002/b.c', '#define NAME other\n' + guard('#include "src/shared.inc"', 'other'))
        rows = pb.candidate_inventory()
        unresolved = [x for x in rows if x['status'] == 'guarded-candidate-unresolved']
        self.assertEqual(len(unresolved), 2)
        self.assertEqual({x['overlay'] for x in unresolved}, {1, 2})
        self.assertTrue(all(x['source_includes'] == ['src/shared.inc'] for x in unresolved))
        self.assertEqual(pb.discover_queue(), [])

    def test_bare_fallback_ordinary_c_and_padding_are_separate(self):
        self.write('src/main/test.c', 'void plain(void) {}\n#pragma GLOBAL_ASM("asm/missing.s")\n')
        self.write('atlas.json', json.dumps({'modules': [{'overlay': 1, 'text_ownership': [
            {'source': 'overlays/o001/overlay_001_padding', 'type': 'asm',
             'offset': '0x10', 'size': '0x4'}]}]}))
        rows = pb.candidate_inventory()
        self.assertEqual({x['status'] for x in rows}, {'ordinary-c', 'bare-fallback', 'padding-owner'})
        self.assertEqual(pb.discover_queue(), [])

    def test_duplicate_byte_ownership_never_enters_queue(self):
        for path in ['src/main/a.c', 'src/main/b.c']:
            self.write(path, guard('void duplicate(void) {}', 'duplicate'))
        self.assertEqual(pb.discover_queue(), [])
        rows = pb.candidate_inventory()
        self.assertEqual({x['status'] for x in rows}, {'ambiguous-ownership'})

    def test_duplicate_guards_are_visible_not_silently_deduplicated(self):
        text = guard('void duplicate(void) {}', 'duplicate')
        self.write('src/main/a.c', text + text)
        self.assertEqual(pb.discover_queue(), [])
        self.assertEqual({x['status'] for x in pb.candidate_inventory()}, {'ambiguous-ownership'})


if __name__ == '__main__':
    unittest.main()
