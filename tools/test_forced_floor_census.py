#!/usr/bin/env python3
"""Synthetic floor evidence: chronology, source binding and route scope."""
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import forced_floor_census as ffc


class FloorEvidence(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = pathlib.Path(self.tmp.name)
        self.source = 'src/example.c'
        path = self.root / self.source
        path.parent.mkdir()
        path.write_text('void example(void) {}\n')
        self.rank = {'name': 'example', 'file': self.source, 'overlay': 9,
                     'size_bytes': 400, 'size_delta': 0,
                     'relocation_masked_differing_words': 20,
                     'source_context_sha256': 'current-context'}
        self.receipt = dict(symbol='example', source=self.source,
                            source_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                            source_context_sha256='current-context', overlay=9,
                            target_size_bytes=400, search_scope='every-colour',
                            result='exhausted', floor=14)

    def line(self, **changes):
        return '<!-- colour-exhaustion-v1 ' + json.dumps(dict(self.receipt, **changes)) + ' -->'

    def census(self, text, *, context='current-context', committed=True):
        with patch.object(ffc, 'ROOT', self.root), \
             patch.object(ffc, 'current_contexts', return_value={(self.source, 'example'): context}), \
             patch.object(ffc, 'receipt_commit', return_value='a' * 40 if committed else None):
            return ffc.census(ranking={'example': self.rank},
                              shards={'example': ('docs/example.md', text)},
                              sources={}, commits=False)[0]

    def test_prose_is_advisory_even_with_exhaustion(self):
        row = self.census('Colour floor 14; no zero-scoring force.')
        self.assertTrue(row.proved)
        self.assertEqual(row.status, 'needs-review')
        self.assertIsNone(row.commit)
        self.assertEqual(ffc.colour_exhausted([row]), {})

    def test_prepended_newer_and_appended_history_cannot_establish_binding(self):
        new = '#### 2026-09-20\nColour floor 4; zero winners.'
        old = '#### 2026-09-10\nColour floor 20; zero winners.'
        a, b = self.census(new + '\n' + old), self.census(old + '\n' + new)
        self.assertEqual((a.status, a.claims), ('needs-review', [4, 20]))
        self.assertEqual((a.status, a.claims), (b.status, b.claims))

    def test_unrelated_section_proof_cannot_contaminate_claim(self):
        row = self.census('Colour floor 14.\n\nA separate old probe had zero winners.')
        self.assertFalse(row.proved)
        self.assertEqual(row.status, 'needs-review')

    def test_current_receipt_has_exact_evidence_commit(self):
        row = self.census(self.line())
        self.assertEqual(row.status, 'colour-exhausted')
        self.assertTrue(row.bound)
        self.assertEqual(row.commit, 'a' * 40)
        self.assertEqual(list(ffc.colour_exhausted([row])), ['example'])

    def test_source_change_with_unchanged_score_needs_review(self):
        (self.root / self.source).write_text('void example(void) { /* changed */ }\n')
        row = self.census(self.line())
        self.assertEqual(row.status, 'needs-review')
        self.assertEqual(row.reason, 'source changed')

    def test_header_flags_compiler_or_target_context_change_needs_review(self):
        row = self.census(self.line(), context='changed-context')
        self.assertEqual(row.status, 'needs-review')
        self.assertIn('context changed', row.reason)

    def test_stale_ranking_cannot_authenticate_context(self):
        self.rank['source_context_sha256'] = 'stale'
        self.assertEqual(self.census(self.line()).status, 'needs-review')

    def test_uncommitted_or_duplicate_line_cannot_be_evidence(self):
        self.assertEqual(self.census(self.line(), committed=False).status, 'needs-review')

    def test_conflicting_receipts_refuse_in_either_order(self):
        a, b = self.line(), self.line(floor=12)
        for text in [a + '\n' + b, b + '\n' + a]:
            row = self.census(text)
            self.assertEqual(row.status, 'needs-review')
            self.assertEqual(row.reason, 'conflicting current receipts')

    def test_stale_and_current_receipts_require_explicit_reconciliation(self):
        row = self.census(self.line() + '\n' + self.line(source_context_sha256='old'))
        self.assertEqual(row.status, 'needs-review')

    def test_wrong_owner_scope_size_or_result_refuses(self):
        for changes in [dict(symbol='other'), dict(source='src/other.c'),
                        dict(overlay=10), dict(target_size_bytes=404),
                        dict(search_scope='one-web'), dict(floor=-1),
                        dict(result='zero-floor'), dict(floor=True)]:
            with self.subTest(changes=changes):
                self.assertEqual(self.census(self.line(**changes)).status, 'needs-review')

    def test_valid_zero_floor_is_not_exhaustion(self):
        row = self.census(self.line(floor=0, result='zero-floor'))
        self.assertEqual(row.status, 'zero-floor')
        self.assertEqual(ffc.colour_exhausted([row]), {})

    def test_malformed_record_is_advisory(self):
        for payload in ['{', 'null', '{}']:
            self.assertEqual(self.census('<!-- colour-exhaustion-v1 ' + payload + ' -->').status,
                             'needs-review')

    def test_document_labels_prose_and_never_presents_last_touch_as_evidence(self):
        row = self.census('Colour floor 14; zero winners.')
        doc = ffc.render_doc([row])
        self.assertIn('0 queued functions', doc)
        self.assertIn('needs-review', doc)
        self.assertIn('evidence commit', doc)
        self.assertIn('Legacy prose is advisory', doc)


class CommitBinding(unittest.TestCase):
    def test_blame_tracks_receipt_not_later_shard_edit(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            def git(*args):
                return subprocess.check_output(['git', *args], cwd=root, text=True).strip()
            git('init', '-q')
            git('config', 'user.name', 'Fixture')
            git('config', 'user.email', 'fixture@example.invalid')
            path = root / 'receipt.md'
            line = '<!-- colour-exhaustion-v1 {} -->'
            path.write_text(line + '\n')
            git('add', 'receipt.md'); git('commit', '-qm', 'receipt')
            first = git('rev-parse', 'HEAD')
            path.write_text('New unrelated section\n' + line + '\n')
            git('add', 'receipt.md'); git('commit', '-qm', 'unrelated')
            import nm_ranking as nm
            with patch.object(nm, 'ROOT', root):
                self.assertEqual(ffc.receipt_commit('receipt.md', line), first)
                self.assertIsNone(ffc.receipt_commit('receipt.md', 'missing'))


if __name__ == '__main__':
    unittest.main()
