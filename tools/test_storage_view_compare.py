#!/usr/bin/env python3
"""Synthetic controls for explicit, report-only storage accounting."""
import copy
import unittest
from unittest import mock

import storage_view_compare as sc
from test_storage_view import Object, ins


class Requests(unittest.TestCase):
    def test_no_manual_identity_or_friendly_alias(self):
        for extra in ('identity', 'alias', 'candidate_external', 'raw_object'):
            with self.assertRaises(sc.sv.ViewError):
                sc.requests([{'source_function': 'owner', 'external': 'storage', extra: 'x'}])

    def test_duplicate_name(self):
        row = {'source_function': 'owner', 'external': 'storage'}
        with self.assertRaises(sc.sv.ViewError):
            sc.requests([row, row])


class Bindings(unittest.TestCase):
    def setUp(self):
        self.proof = {'external': 'storage', 'source_function': 'owner',
                      'status': 'independent-storage-use-proved', 'promotion_acceptance': False,
                      'identity': [4093, 32], 'source_overlay': 60}
        self.linked = Object(symbols=[('storage', 0, 0, 16, sc.rs.SHN_ABS)])

    def validate(self, siblings=None):
        return sc.validate_binding(self.proof, 'storage', 'owner', 57, siblings or {}, self.linked)

    def test_reserved_namespace_not_collapsed(self):
        self.assertEqual(self.validate(), (4093, 32))
        self.proof['identity'][0] = 4094
        self.assertEqual(self.validate(), (4094, 32))

    def test_same_overlay_named_storage(self):
        self.proof.update(identity=[57, 32], source_overlay=57)
        self.assertEqual(self.validate(), (57, 32))

    def test_foreign_local_rejected(self):
        self.proof['identity'] = [60, 32]
        with self.assertRaises(sc.sv.ViewError):
            self.validate()

    def test_exact_sibling_conflict_rejected(self):
        with self.assertRaises(sc.sv.ViewError):
            self.validate({'storage': {(4095, 32)}})

    def test_actual_named_definition_conflict_rejected(self):
        self.linked.syms = [('storage', sc.rs.SYNTHETIC_VMA + 32, 4, 17, 1)]
        self.linked.names = ['', '.overlay_057_bss']
        with self.assertRaises(sc.sv.ViewError):
            self.validate()

    def test_wrong_proof_name_rejected(self):
        self.proof['external'] = 'friendly'
        with self.assertRaises(sc.sv.ViewError):
            self.validate()


class Candidate(unittest.TestCase):
    def fixture(self):
        obj = Object([ins(15, rt=2), ins(9, rs=2, rt=2, immediate=4), 0, 0],
                     [('.text', 0, 5, 1), ('.text', 4, 6, 1)],
                     [('func', 0, 8, 18, 1), ('storage', 0, 0, 16, 0)])
        obj.names = ['', '.text']
        return obj

    def test_owned_named_external(self):
        self.assertEqual(sc.candidate_sites(self.fixture(), 'func', {'storage'})[2], {'storage': 1})

    def test_only_sibling_use_rejected(self):
        obj = self.fixture()
        obj.relocs = [('.text', 8, 5, 1), ('.text', 12, 6, 1)]
        with self.assertRaises(sc.sv.ViewError):
            sc.candidate_sites(obj, 'func', {'storage'})

    def test_candidate_definition_rejected(self):
        obj = self.fixture()
        obj.syms[1] = ('storage', 8, 4, 17, 1)
        with self.assertRaises(sc.sv.ViewError):
            sc.candidate_sites(obj, 'func', {'storage'})

    def test_unpaired_low_rejected(self):
        obj = self.fixture()
        obj.relocs = [('.text', 4, 6, 1)]
        with self.assertRaises(sc.sv.ViewError):
            sc.candidate_sites(obj, 'func', {'storage'})

    def test_no_target_correlation_needed_for_moved_pair_and_addend(self):
        obj = self.fixture()
        records = sc.rs._candidate_surface_records(obj, 0, 8, [], {}, {}, set(), 57,
                                                    index_identities={1: (4093, 32)})
        self.assertEqual([r.identity for r in records], [(4093, 36), (4093, 36)])

    def test_unbound_default_record_preserved(self):
        default = [{'offset': 0, 'rtype': 5, 'identity': None},
                   {'offset': 4, 'rtype': 6, 'identity': None},
                   {'offset': 8, 'rtype': 4, 'identity': [57, 100]}]
        before = copy.deepcopy(default)
        bound = [sc.rs.SurfaceRecord(0, 5, (4093, 32)), sc.rs.SurfaceRecord(4, 6, (4093, 32))]
        result = sc.replace_records(default, bound, {'storage': [(0, 5), (4, 6)]})
        self.assertEqual(result[-1].identity, (57, 100))
        self.assertEqual(default, before)

    def test_unpaired_high_rejected(self):
        with self.assertRaises(sc.sv.ViewError):
            sc.replace_records([{'offset': 0, 'rtype': 5, 'identity': None}],
                               [sc.rs.SurfaceRecord(0, 5, None)], {'storage': [(0, 5)]})


if __name__ == '__main__':
    unittest.main()
