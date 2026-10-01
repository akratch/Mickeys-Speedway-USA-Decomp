#!/usr/bin/env python3
"""Availability is target-specific and never proves a donor or build."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

import donor_source_receipt as receipt


class ShapeTests(unittest.TestCase):
    def classify(self, text, symbol='wanted', path='src/peer.c'):
        return receipt.classify(text.encode(), path, symbol)

    def test_other_function_c_does_not_make_requested_fallback_available(self):
        result = self.classify('void other(void) {}\n#pragma GLOBAL_ASM("asm/wanted.s")\n')
        self.assertEqual(result['availability'], 'global-asm-only')

    def test_comments_literals_and_macros_cannot_invent_definition(self):
        result = self.classify('/* void wanted(void) {} */\n'
                               'char *s = "void wanted(void) {}";\n'
                               '#define X void wanted(void) {}\n')
        self.assertEqual(result['availability'], 'no-direct-definition')

    def test_prototype_and_call_are_not_definition(self):
        result = self.classify('void wanted(void);\nvoid other(void) { wanted(); }\n')
        self.assertEqual(result['availability'], 'no-direct-definition')

    def test_disabled_source_is_not_configured_availability(self):
        result = self.classify('#if 0\nvoid wanted(void) {}\n#else\n'
                               '#pragma GLOBAL_ASM("asm/wanted.s")\n#endif\n')
        self.assertEqual(result['availability'], 'c-text-and-fallback')
        self.assertEqual(result['configured_availability'], 'not-evaluated')
        self.assertEqual(result['conditional_directives'], 3)

    def test_matching_basename_is_exact(self):
        self.assertEqual(self.classify('#pragma GLOBAL_ASM("asm/unwanted.s")\n')['availability'],
                         'no-direct-definition')

    def test_tracked_assembly_does_not_authenticate_origin(self):
        result = self.classify('# generated output\n', path='asm/wanted.s')
        self.assertEqual(result['availability'], 'tracked-assembly-unreviewed')
        self.assertEqual(result['assembly_origin'], 'not-authenticated')

    def test_line_spliced_macro_remains_a_directive(self):
        result = self.classify('#define BODY \\\nvoid wanted(void) {}\n')
        self.assertEqual(result['availability'], 'no-direct-definition')

    def test_missing_and_unsupported_indirection_are_distinct(self):
        self.assertEqual(receipt.classify(None, 'src/peer.c', 'wanted')['availability'], 'absent-path')
        self.assertEqual(self.classify('#include "body.inc"\n')['availability'], 'no-direct-definition')


class GitTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.repo = Path(self.tmp.name)
        self.run_git('init', '-q')
        self.run_git('config', 'user.name', 'Fixture')
        self.run_git('config', 'user.email', 'fixture@example.invalid')
        (self.repo / 'peer.c').write_text('#pragma GLOBAL_ASM("asm/wanted.s")\n')
        self.run_git('add', 'peer.c')
        self.run_git('commit', '-qm', 'fallback')
        self.old = self.run_git('rev-parse', 'HEAD').strip()
        (self.repo / 'peer.c').write_text('void wanted(void) {}\n')
        self.run_git('commit', '-qam', 'source')
        self.new = self.run_git('rev-parse', 'HEAD').strip()

    def run_git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.repo), *args], text=True)

    def make(self, ref):
        return receipt.make_receipt(self.repo, 'HEAD', 'peer.c', 'wanted', self.repo,
                                    ref, 'peer.c', 'wanted', 'fixture-only pairing')

    def test_source_version_is_not_object_version(self):
        old, new = self.make(self.old), self.make(self.new)
        self.assertEqual(old['query']['availability'], 'global-asm-only')
        self.assertEqual(new['query']['availability'], 'c-text-present')
        self.assertIsNone(new['object_identity'])
        self.assertFalse(new['matrix_exhausted'])
        self.assertFalse(new['assignment_authorized'])
        self.assertFalse(new['correspondence_verified'])
        self.assertNotIn(str(self.repo), json.dumps(new))

    def test_dirty_files_do_not_supply_evidence(self):
        (self.repo / 'peer.c').write_text('uncommitted replacement')
        self.assertEqual(self.make('HEAD')['query']['availability'], 'c-text-present')

    def test_deleted_source_can_be_queried_at_old_pin(self):
        self.run_git('rm', '-q', 'peer.c')
        self.run_git('commit', '-qm', 'delete')
        oid, data = receipt.source(self.repo, self.old, 'peer.c')
        self.assertIsNotNone(oid)
        self.assertIn(b'GLOBAL_ASM', data)
        self.assertEqual(receipt.source(self.repo, 'HEAD', 'peer.c'), (None, None))

    def test_changed_target_produces_new_pin(self):
        first = self.make(self.old)
        (self.repo / 'peer.c').write_text('void wanted(void) { return; }\n')
        self.run_git('commit', '-qam', 'target changes')
        second = self.make(self.old)
        self.assertNotEqual(first['target']['blob'], second['target']['blob'])
        self.assertNotEqual(first['target']['commit'], second['target']['commit'])

    def test_wrong_target_symbol_is_refused(self):
        with self.assertRaises(ValueError):
            receipt.make_receipt(self.repo, 'HEAD', 'peer.c', 'unrelated', self.repo,
                                 self.old, 'peer.c', 'wanted', 'fixture')

    def test_path_escape_and_symlinks_refused(self):
        for path in ('../peer.c', '/peer.c', './peer.c', 'peer.c:other'):
            with self.assertRaises(ValueError):
                receipt.source(self.repo, self.old, path)
        (self.repo / 'alias.c').symlink_to('peer.c')
        self.run_git('add', 'alias.c')
        self.run_git('commit', '-qm', 'symlink')
        with self.assertRaises(ValueError):
            receipt.source(self.repo, 'HEAD', 'alias.c')


if __name__ == '__main__':
    unittest.main()
