#!/usr/bin/env python3
"""Synthetic architectural fixtures, never ROM-derived instruction arrays."""
import sys
import struct
from unittest import mock
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import regional_schedule as r


def imm(op, a, b, value):
    return op << 26 | a << 21 | b << 16 | value & 65535


def special(fn, a=0, b=0, dest=0):
    return a << 21 | b << 16 | dest << 11 | fn


def fp(fn=2, a=2, b=4, dest=6, fmt=16):
    return 17 << 26 | fmt << 21 | b << 16 | a << 11 | dest << 6 | fn


def mtc1(src, dest):
    return 17 << 26 | 4 << 21 | src << 16 | dest << 11


class ExecutionTests(unittest.TestCase):
    def test_likely_delay_executes_only_on_taken_edge(self):
        words = [imm(21, 4, 5, 2), fp(), fp(), 0]
        out = r.analyze(words)
        self.assertEqual([1, 1], out['execution']['multiply_count_range'])
        self.assertEqual(1, len(out['branch_likely_alternatives']))
        self.assertEqual('taken-only', out['delay_slots'][0]['execution'])
        sites = [p['multiplies'][0]['offset'] for p in out['execution']['paths']]
        self.assertEqual([8, 4], sites)

    def test_branch_predicate_keeps_register_inputs_and_unresolved_status(self):
        out = r.analyze([imm(21, 4, 5, 2), fp(), fp(), 0])
        predicate = out['branch_conditions'][0]
        self.assertEqual('bnel', predicate['opcode'])
        self.assertEqual(['r4', 'r5'], predicate['register_operands'])
        self.assertEqual('not-solved', predicate['predicate_status'])
        self.assertEqual(12, predicate['taken_target'])

    def test_ordinary_delay_also_executes_on_fallthrough(self):
        out = r.analyze([imm(5, 4, 5, 2), fp(), fp(), 0])
        self.assertEqual([1, 2], out['execution']['multiply_count_range'])
        self.assertEqual([], out['branch_likely_alternatives'])

    def test_likely_target_equal_fallthrough_executes_both_on_taken(self):
        out = r.analyze([imm(21, 4, 5, 1), fp(), fp(), 0])
        self.assertEqual([1, 2], out['execution']['multiply_count_range'])
        self.assertEqual([], out['branch_likely_alternatives'])

    def test_identical_words_are_not_enough_when_branch_target_unresolved(self):
        out = r.analyze([imm(21, 4, 5, 2), fp(), fp(), 0],
                        relocations={0: ('R_MIPS_PC16', 'unresolved')})
        self.assertIsNone(out['execution']['multiply_count_range'])
        self.assertEqual([], out['branch_likely_alternatives'])
        self.assertIn('unresolved-control-target', out['execution']['limits'])

    def test_return_owns_its_delay_multiply(self):
        out = r.analyze([special(8, 31), fp(), fp()])
        self.assertEqual([1, 1], out['execution']['multiply_count_range'])
        self.assertEqual(2, out['operation_counts']['multiply:float32'])

    def test_missing_return_delay_is_incomplete(self):
        out = r.analyze([special(8, 31)])
        self.assertFalse(out['execution']['complete'])
        self.assertIn('missing-owned-delay-slot', out['execution']['limits'])

    def test_cycle_does_not_get_a_finite_dynamic_total(self):
        out = r.analyze([imm(4, 0, 0, -1), fp()])
        self.assertIsNone(out['execution']['multiply_count_range'])
        self.assertIn('cycle', out['execution']['limits'])

    def test_indirect_jump_does_not_get_a_complete_path(self):
        out = r.analyze([special(8, 4), fp()])
        self.assertIn('unresolved-control-target', out['execution']['limits'])

    def test_jalr_zero_link_is_unresolved_indirect_not_returning_call(self):
        words = [special(9, 4, dest=0), fp(), fp()]
        decoded = r.decode(words[0], 0)
        self.assertEqual('indirect', decoded['control'])
        self.assertIsNone(decoded['destination'])
        out = r.analyze(words)
        self.assertFalse(out['execution']['complete'])
        self.assertIsNone(out['execution']['multiply_count_range'])
        self.assertIn('unresolved-control-target', out['execution']['limits'])
        self.assertEqual([4], [m['offset'] for m in out['execution']['paths'][0]['multiplies']])

    def test_jalr_nonzero_link_keeps_returning_call_model(self):
        words = [special(9, 4, dest=31), 0, fp()]
        self.assertEqual('call', r.decode(words[0], 0)['control'])
        self.assertEqual([1, 1], r.analyze(words)['execution']['multiply_count_range'])

    def test_declared_extent_can_include_padding(self):
        out = r.analyze([special(8, 31), 0, 0, 0])
        self.assertEqual(16, out['owned_size'])
        self.assertEqual(3, out['opcode_counts']['nop'])
        self.assertEqual(1, out['execution']['paths'][0]['operation_counts']['nop'])

    def test_unknown_control_in_delay_slot_is_incomplete(self):
        out = r.analyze([imm(4, 4, 5, 1), special(8, 31), 0])
        self.assertIn('control-in-delay-slot', out['execution']['limits'])

    def test_path_limit_is_explicit(self):
        out = r.analyze([imm(5, 4, 5, 2), 0, 0, 0], max_paths=1)
        self.assertIn('path-limit', out['execution']['limits'])
        self.assertIsNone(out['execution']['multiply_count_range'])

    def test_window_entry_in_delay_slot_requires_owner_context(self):
        out = r.analyze([imm(21, 4, 5, 2), fp(), fp(), 0], start=4)
        self.assertIn('entry-is-delay-slot-without-branch-context', out['execution']['limits'])
        self.assertFalse(out['execution']['complete'])

    def test_unconditional_likely_does_not_claim_two_edges(self):
        out = r.analyze([imm(20, 4, 4, 2), fp(), fp(), 0])
        self.assertEqual([], out['branch_likely_alternatives'])
        self.assertEqual(1, len(out['execution']['paths']))


class OperandTests(unittest.TestCase):
    @staticmethod
    def fixture(second=3):
        return [imm(9, 0, 4, 2), imm(9, 0, 5, second), special(24, 4, 5)]

    def test_constants_resolve_but_entry_registers_do_not(self):
        known = r.analyze(self.fixture())['execution']['paths'][0]['multiplies'][0]
        self.assertEqual([2, 3], [v['value'] for v in known['operands']])
        unknown = r.analyze([special(24, 4, 5)])['execution']['paths'][0]['multiplies'][0]
        self.assertTrue(all(v['kind'] == 'unresolved' for v in unknown['operands']))

    def test_relocated_constant_is_not_an_independent_value(self):
        out = r.analyze(self.fixture(), relocations={0: ('R_MIPS_LO16', 'address')})
        self.assertEqual('unresolved', out['execution']['paths'][0]['multiplies'][0]['operands'][0]['kind'])

    def test_memory_load_discards_old_constant(self):
        words = self.fixture()[:2] + [imm(0x23, 6, 4, 0), special(24, 4, 5)]
        out = r.analyze(words)
        self.assertEqual('unresolved', out['execution']['paths'][0]['multiplies'][0]['operands'][0]['kind'])

    def test_call_discards_constants_written_in_delay_slot(self):
        words = [3 << 26, imm(9, 0, 4, 2), imm(9, 0, 5, 3), special(24, 4, 5)]
        out = r.analyze(words)
        self.assertEqual('unresolved', out['execution']['paths'][0]['multiplies'][0]['operands'][0]['kind'])

    def test_control_register_transfers_use_fcr_namespace(self):
        read = 17 << 26 | 2 << 21 | 4 << 16 | 31 << 11
        write = 17 << 26 | 6 << 21 | 5 << 16 | 31 << 11
        self.assertEqual(['fcr31'], r.decode(read, 0)['sources'])
        self.assertEqual('r4', r.decode(read, 0)['destination'])
        self.assertEqual(['r5'], r.decode(write, 0)['sources'])
        self.assertEqual('fcr31', r.decode(write, 0)['destination'])

    def test_control_register_write_does_not_clobber_same_numbered_data_register(self):
        write = 17 << 26 | 6 << 21 | 5 << 16 | 2 << 11
        words = [imm(9, 0, 4, 2), mtc1(4, 2), write, fp()]
        value = r.analyze(words)['execution']['paths'][0]['multiplies'][0]['operands'][0]
        self.assertEqual('constant-bits', value['kind'])
        self.assertEqual(2, value['value'])

    def test_fp_transfer_preserves_bits_without_computing_float_arithmetic(self):
        words = [imm(15, 0, 4, 1), mtc1(4, 2), mtc1(4, 4), fp()]
        operands = r.analyze(words)['execution']['paths'][0]['multiplies'][0]['operands']
        self.assertEqual([65536, 65536], [v['value'] for v in operands])

    def test_wide_multiply_never_uses_only_lower_word_constant(self):
        words = self.fixture()[:2] + [special(28, 4, 5)]
        operands = r.analyze(words)['execution']['paths'][0]['multiplies'][0]['operands']
        self.assertTrue(all(v['kind'] == 'unresolved' for v in operands))

    def test_double_comparison_writes_condition_without_register_pair(self):
        out = r.analyze([fp(fn=50, fmt=17)])
        self.assertTrue(out['execution']['complete'])

    def test_double_conversion_invalidates_partner_register(self):
        words = [imm(9, 0, 4, 2), mtc1(4, 3), fp(fn=33, dest=2), fp(a=3)]
        operands = r.analyze(words)['execution']['paths'][0]['multiplies'][0]['operands']
        self.assertEqual('unresolved', operands[0]['kind'])

    def test_double_load_invalidates_partner_register(self):
        words = [imm(9, 0, 4, 2), mtc1(4, 3), imm(0x35, 5, 2, 0), fp(a=3)]
        operands = r.analyze(words)['execution']['paths'][0]['multiplies'][0]['operands']
        self.assertEqual('unresolved', operands[0]['kind'])


class ComparisonTests(unittest.TestCase):
    def test_extra_static_multiply_can_have_same_execution_count(self):
        target = [fp(), 0]
        candidate = [imm(21, 4, 5, 2), fp(), fp(), 0]
        out = r.compare(target, candidate)
        self.assertEqual('same-count-range-not-equivalence', out['multiply_execution_relation'])
        self.assertEqual('unresolved', out['multiply_operand_relation'])
        self.assertTrue(out['diagnostic_only'])

    def test_genuine_extra_executed_multiply(self):
        target = OperandTests.fixture()
        out = r.compare(target, target + [special(24, 4, 5)])
        self.assertEqual('candidate-has-more-on-every-enumerated-traversal', out['multiply_execution_relation'])

    def test_equal_counts_different_known_operands(self):
        out = r.compare(OperandTests.fixture(3), OperandTests.fixture(4))
        self.assertEqual('same-count-range-not-equivalence', out['multiply_execution_relation'])
        self.assertEqual('different-constant-input-sequences', out['multiply_operand_relation'])

    def test_operand_order_is_not_silently_normalized(self):
        a = OperandTests.fixture()
        b = a[:2] + [special(24, 5, 4)]
        self.assertEqual('different-constant-input-sequences', r.compare(a, b)['multiply_operand_relation'])

    def test_windows_use_independent_offsets(self):
        out = r.compare([0, fp(), 0], [fp(), 0], target_window=(4, 12))
        self.assertEqual([], out['alignment']['insertions'])
        self.assertEqual([], out['alignment']['deletions'])
        self.assertEqual(-4, out['size_delta'])

    def test_frame_sizes_and_load_widths(self):
        a = [imm(9, 29, 29, -32), imm(0x21, 4, 5, 0), imm(0x2B, 4, 5, 0)]
        b = [imm(9, 29, 29, -40)] + a[1:]
        out = r.compare(a, b)
        self.assertEqual(8, out['frame_delta'])
        self.assertEqual(1, out['candidate']['operation_counts']['load:2'])
        self.assertEqual(1, out['candidate']['operation_counts']['store:4'])

    def test_invalid_windows_and_limits_fail(self):
        for kwargs in ({'start': 1}, {'end': 8}, {'start': 4}, {'max_paths': 0}):
            with self.subTest(kwargs=kwargs), self.assertRaises(ValueError):
                r.analyze([0], **kwargs)


class ObjectBoundaryTests(unittest.TestCase):
    class Elf:
        def __init__(self, word, relocation=(), size=16):
            self.word, self.relocation, self.size = word, relocation, size

        def section(self, name):
            return 1, None

        def symbols(self):
            return [("owned", 8, self.size, r.rs.STT_FUNC, 1),
                    ("local", 16, 0, 0, 1), ("external", 0, 0, 0, 0)]

        def section_bytes(self, name):
            return bytes(8) + struct.pack(">IIII", self.word, 0, special(8, 31), 0)

        def relocations(self, target=r"\.text"):
            self.requested_relocation_target = target
            return list(self.relocation)

    def test_local_pc16_uses_symbol_definition_not_placeholder_offset(self):
        fixture = self.Elf(imm(4, 4, 5, -1), [(".text", 8, r.rs.R_MIPS_PC16, 1)])
        with mock.patch.object(r.rs, 'Elf', return_value=fixture):
            words, relocs, targets = r.load_function('fixture.o', 'owned')
        self.assertEqual(4, len(words))
        self.assertEqual({0: 8}, targets)
        self.assertEqual('R_MIPS_PC16', relocs[0][0])

    def test_other_section_relocations_at_same_offsets_are_ignored(self):
        fixture = self.Elf(imm(4, 4, 5, -1), [
            (".rodata", 8, r.rs.R_MIPS_26, 2),
            (".text", 8, r.rs.R_MIPS_PC16, 1),
            (".data", 12, r.rs.R_MIPS_LO16, 2)])
        with mock.patch.object(r.rs, 'Elf', return_value=fixture):
            _, relocs, targets = r.load_function('fixture.o', 'owned')
        self.assertEqual(r"\.text", fixture.requested_relocation_target)
        self.assertEqual({0: ('R_MIPS_PC16', 'local')}, relocs)
        self.assertEqual({0: 8}, targets)

    def test_raw_local_jump_is_rebased_for_multifunction_object(self):
        fixture = self.Elf(2 << 26 | 4)
        with mock.patch.object(r.rs, 'Elf', return_value=fixture):
            _, _, targets = r.load_function('fixture.o', 'owned')
        self.assertEqual({0: 8}, targets)

    def test_undefined_branch_symbol_remains_unresolved(self):
        fixture = self.Elf(imm(4, 4, 5, 1), [(".text", 8, r.rs.R_MIPS_PC16, 2)])
        with mock.patch.object(r.rs, 'Elf', return_value=fixture):
            words, relocs, targets = r.load_function('fixture.o', 'owned')
        self.assertEqual({}, targets)
        self.assertFalse(r.analyze(words, relocations=relocs, control_targets=targets)['execution']['complete'])

    def test_invalid_function_extent_and_relocation_fail(self):
        for fixture in (self.Elf(0, size=20),
                        self.Elf(0, [(".text", 9, r.rs.R_MIPS_PC16, 1)]),
                        self.Elf(0, [(".text", 8, r.rs.R_MIPS_PC16, 99)])):
            with self.subTest(fixture=fixture), mock.patch.object(r.rs, 'Elf', return_value=fixture):
                with self.assertRaises(ValueError):
                    r.load_function('fixture.o', 'owned')


if __name__ == '__main__':
    unittest.main()
