#!/usr/bin/env python3
"""Synthetic branch/relocation fixtures only; no ROM or compiler invocation."""
import contextlib
import io
import json
from pathlib import Path
import sys
import subprocess
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import branch_layout_report as br  # noqa: E402
import nm_ranking as nr  # noqa: E402

NOP = 0
BEQ = (4 << 26) | (8 << 21)
ADDIU = (9 << 26) | (9 << 21) | (9 << 16) | 7
JR = (31 << 21) | 8


def listing_text(words, labels=None, base=0x1000):
    labels = labels or {8: ['landing']}
    lines = ['glabel synthetic_function']
    for i, word in enumerate(words):
        lines.extend('  glabel ' + name for name in labels.get(i * 4, []))
        lines.append(f'    /* {i * 4:X} {base + i * 4:08X} {word:08X} */ synthetic')
    lines.append('endlabel synthetic_function')
    return '\n'.join(lines)


def fixture(candidate=None, symbol='landing'):
    words = [BEQ | 1, NOP, ADDIU, JR, NOP]
    listing = br.parse_listing(listing_text(words, {8: [symbol]}), 'synthetic_function')
    target = [BEQ | 0xFFFF] + words[1:]
    candidate = words if candidate is None else candidate
    streams = nr.WordStreams(base_words=candidate, target_words=target,
                             base_reloc={}, target_reloc={0: ('R_MIPS_PC16', symbol)},
                             base_size=len(candidate) * 4, target_size=len(target) * 4)
    return streams, listing


class ResolutionTests(unittest.TestCase):
    def test_resolution_retains_official_result_and_does_not_mutate_inputs(self):
        streams, listing = fixture()
        before = list(streams.target_words)
        result = br.report(streams, listing)
        self.assertEqual(streams.target_words, before)
        self.assertEqual(result['official_alignment']['aligned_immediate_only'], 1)
        self.assertEqual(result['resolved_layout_alignment']['aligned_exact'], 5)
        self.assertEqual(result['pc16_resolutions'][0]['encoded_addend_bytes'], -4)
        self.assertEqual(result['pc16_resolutions'][0]['destination'], 8)
        self.assertFalse(result['matching_proof'])

    def test_already_resolved_target_needs_no_relocation_or_score_change(self):
        streams, listing = fixture()
        streams.target_words[0] = BEQ | 1
        streams.target_reloc.clear()
        result = br.report(streams, listing)
        self.assertEqual(result['pc16_resolutions'], [])
        self.assertEqual(result['official_alignment'], result['resolved_layout_alignment'])
        self.assertEqual(result['official_alignment']['aligned_exact'], 5)

    def test_exact_binding_does_not_decode_hex_from_label_name(self):
        streams, listing = fixture(symbol='.LDEADBEEF')
        result = br.report(streams, listing)
        self.assertEqual(result['pc16_resolutions'][0]['symbol_vma'], 0x1008)

    def test_wrong_listing_destination_is_refused_not_masked(self):
        streams, listing = fixture()
        listing['words'][0] = BEQ | 2
        with self.assertRaisesRegex(ValueError, 'formula disagrees'):
            br.report(streams, listing)

    def test_wrong_label_binding_is_refused(self):
        streams, listing = fixture()
        listing['labels']['landing'] += 4
        with self.assertRaisesRegex(ValueError, 'formula disagrees'):
            br.report(streams, listing)

    def test_unknown_external_symbol_is_refused(self):
        streams, listing = fixture()
        del listing['labels']['landing']
        with self.assertRaisesRegex(ValueError, 'no exact listing binding'):
            br.report(streams, listing)

    def test_symbol_outside_owned_extent_is_refused(self):
        streams, listing = fixture()
        listing['labels']['landing'] = listing['base'] + 20
        with self.assertRaisesRegex(ValueError, 'outside the owned'):
            br.report(streams, listing)

    def test_nondefault_encoded_addend_is_checked(self):
        streams, listing = fixture()
        streams.target_words[0] = BEQ  # Encoded zero => A=0, destination S+4.
        listing['words'][0] = BEQ | 2
        resolved, records = br.resolve_internal_pc16(streams.target_words, streams.target_reloc, listing)
        self.assertEqual(records[0]['encoded_addend_bytes'], 0)
        self.assertEqual(br.branch_destination(resolved[0], 0), 12)

    def test_backward_branch_uses_signed_displacement(self):
        words = [ADDIU, NOP, BEQ | 0xFFFD, JR, NOP]
        listing = br.parse_listing(listing_text(words, {0: ['back']}), 'synthetic_function')
        raw = list(words)
        raw[2] = BEQ | 0xFFFF
        resolved, _ = br.resolve_internal_pc16(raw, {8: ('R_MIPS_PC16', 'back')}, listing)
        self.assertEqual(br.branch_destination(resolved[2], 8), 0)

    def test_out_of_signed_branch_range_is_refused(self):
        raw = [NOP] * 40000
        raw[0] = BEQ | 0xFFFF
        expected = list(raw)
        listing = {'base': 0x1000, 'words': expected, 'labels': {'far': 0x1000 + 0x20004}}
        with self.assertRaisesRegex(ValueError, 'out of range'):
            br.resolve_internal_pc16(raw, {0: ('R_MIPS_PC16', 'far')}, listing)

    def test_pc16_on_nonbranch_is_refused(self):
        streams, listing = fixture()
        streams.target_words[0] = NOP
        with self.assertRaisesRegex(ValueError, 'not a supported branch'):
            br.report(streams, listing)

    def test_candidate_pc16_is_not_silently_resolved_from_target(self):
        streams, listing = fixture()
        streams.base_reloc[0] = ('R_MIPS_PC16', 'landing')
        with self.assertRaisesRegex(ValueError, 'candidate has unresolved PC16'):
            br.report(streams, listing)

    def test_stale_nonrelocated_instruction_is_refused(self):
        streams, listing = fixture()
        listing['words'][2] ^= 1
        with self.assertRaisesRegex(ValueError, 'instruction mismatch'):
            br.report(streams, listing)

    def test_unsupported_relocation_does_not_mask_possible_mismatch(self):
        streams, listing = fixture()
        streams.target_reloc[8] = ('UNSUPPORTED_KIND', 'data')
        with self.assertRaisesRegex(ValueError, 'unsupported target relocation'):
            br.report(streams, listing)

    def test_unknown_candidate_relocation_is_refused(self):
        streams, listing = fixture()
        streams.base_reloc[8] = ('UNSUPPORTED_KIND', 'data')
        with self.assertRaisesRegex(ValueError, 'unsupported candidate relocation'):
            br.report(streams, listing)

    def test_invalid_relocation_offsets_on_either_surface_are_refused(self):
        for surface in ('base_reloc', 'target_reloc'):
            for offset in (-4, 2, 20, True):
                with self.subTest(surface=surface, offset=offset):
                    streams, listing = fixture()
                    getattr(streams, surface)[offset] = ('R_MIPS_HI16', 'data')
                    with self.assertRaisesRegex(ValueError, 'offset is unaligned or outside'):
                        br.report(streams, listing)

    def test_malformed_relocation_record_is_refused(self):
        streams, listing = fixture()
        streams.base_reloc[8] = ('R_MIPS_HI16',)
        with self.assertRaisesRegex(ValueError, 'malformed candidate relocation'):
            br.report(streams, listing)

    def test_inconsistent_stream_extent_is_refused(self):
        streams, listing = fixture()
        streams.base_words.append(NOP)
        with self.assertRaisesRegex(ValueError, 'extents do not match'):
            br.report(streams, listing)


class PairAndRegionTests(unittest.TestCase):
    def test_shifted_branch_destination_matches_paired_instruction(self):
        words = [BEQ | 1, NOP, ADDIU, JR, NOP]
        streams, listing = fixture([ADDIU] + words)
        result = br.report(streams, listing)
        branch = result['branches'][0]
        self.assertEqual(branch['candidate_offset'], 4)
        self.assertEqual(branch['candidate_destination'], 12)
        self.assertEqual(branch['status'], 'destination_matches_instruction_pair')

    def test_wrong_candidate_destination_is_reported_not_excused_as_relocation(self):
        streams, listing = fixture([BEQ | 2, NOP, ADDIU, JR, NOP])
        result = br.report(streams, listing)
        self.assertEqual(result['branches'][0]['status'], 'destination_differs_from_instruction_pair')
        self.assertEqual(result['resolved_layout_alignment']['aligned_immediate_only'], 1)

    def test_matching_destination_does_not_hide_different_branch_condition(self):
        streams, listing = fixture([(5 << 26) | (8 << 21) | 1, NOP, ADDIU, JR, NOP])
        result = br.report(streams, listing)
        self.assertEqual(result['branches'][0]['status'], 'destination_matches_instruction_pair')
        self.assertFalse(result['branches'][0]['branch_kind_matches'])
        self.assertFalse(result['matching_proof'])

    def test_unrelocated_stack_immediate_is_not_a_branch_artifact(self):
        streams, listing = fixture()
        load = (0x23 << 26) | (29 << 21) | (4 << 16)
        streams.target_words[2] = load | 16
        listing['words'][2] = load | 16
        streams.base_words[2] = load | 20
        result = br.report(streams, listing)
        self.assertEqual(result['resolved_immediate_categories'], {'stack_displacement': 1})

    def test_candidate_branch_paired_with_nonbranch_is_visible(self):
        streams, listing = fixture()
        streams.base_words[2] = BEQ
        result = br.report(streams, listing)
        self.assertTrue(any(row['status'] == 'candidate_branch_without_target_branch' for row in result['branches']))

    def test_regions_partition_streams_and_report_real_start_shift(self):
        streams, listing = fixture([ADDIU, BEQ | 1, NOP, ADDIU, JR, NOP])
        regions = [{'name': 'first', 'target_span': [0, 8], 'candidate_span': [0, 12]},
                   {'name': 'second', 'target_span': [8, 20], 'candidate_span': [12, 24]}]
        result = br.report(streams, listing, regions)
        self.assertEqual(result['regions'][0]['size_delta_bytes'], 4)
        self.assertEqual(result['regions'][1]['start_shift_bytes'], 4)
        self.assertEqual(result['regions'][1]['resolved_alignment']['aligned_exact'], 3)

    def test_region_gap_overlap_or_partial_coverage_is_refused(self):
        for spans in [([4, 20], [0, 20]), ([0, 16], [0, 20]), ([0, True], [0, 20])]:
            with self.subTest(spans=spans), self.assertRaises(ValueError):
                br.validate_regions([{'name': 'part', 'target_span': spans[0], 'candidate_span': spans[1]}], 20, 20)

    def test_regimm_trap_is_not_misread_as_branch(self):
        self.assertIsNone(br.branch_destination((1 << 26) | (8 << 16) | 1, 0))


class ListingAndCliTests(unittest.TestCase):
    def test_duplicate_label_is_refused(self):
        text = listing_text([BEQ | 1, NOP, ADDIU], {8: ['landing', 'landing']})
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            br.parse_listing(text, 'synthetic_function')

    def test_noncontiguous_listing_is_refused(self):
        text = listing_text([BEQ | 1, NOP, ADDIU]).replace('00001004', '0000100C')
        with self.assertRaisesRegex(ValueError, 'contiguous'):
            br.parse_listing(text, 'synthetic_function')

    def test_failed_object_reader_is_a_json_refusal(self):
        output = io.StringIO()
        error = subprocess.CalledProcessError(1, ['objdump', 'invalid.o'])
        with patch.object(br, 'object_stream', side_effect=error), contextlib.redirect_stdout(output):
            status = br.main(['--symbol', 'synthetic_function', '--target-object', 'invalid.o',
                              '--candidate-object', 'candidate.o', '--target-listing', 'listing.s'])
        self.assertEqual(status, 2)
        self.assertEqual(json.loads(output.getvalue())['status'], 'refused')

    def test_cli_refusal_is_json_and_nonzero(self):
        streams, _ = fixture()
        with tempfile.TemporaryDirectory() as directory:
            listing = Path(directory) / 'fixture.s'
            listing.write_text(listing_text([BEQ | 2, NOP, ADDIU, JR, NOP]))
            values = [(streams.target_words, streams.target_reloc, 20),
                      (streams.base_words, streams.base_reloc, 20)]
            output = io.StringIO()
            with patch.object(br, 'object_stream', side_effect=values), contextlib.redirect_stdout(output):
                status = br.main(['--symbol', 'synthetic_function', '--target-object', 'target.o',
                                  '--candidate-object', 'candidate.o', '--target-listing', str(listing)])
            result = json.loads(output.getvalue())
            self.assertEqual(status, 2)
            self.assertEqual(result['status'], 'refused')
            self.assertNotIn('resolved_layout_alignment', result)


if __name__ == '__main__':
    unittest.main()
