#!/usr/bin/env python3
"""Tests for the insertion-pair reader.

Nothing here compiles and no fixture is ROM-derived: every word is encoded from
its fields by the helpers below, the line table is written in the shape
`objdump -l` prints with placeholder instruction columns, and the trace rows are
in the grammar the instrumented ugen emits. What is pinned is the reading:

  1. a pair opens at the first one-sided word and closes where the streams are
     index-aligned again; two shifts that touch are two pairs, not one;
  2. positional = shadow + aligned residual, exactly, and outside every pair
     the positional and aligned counts agree -- the L155 subtraction is exact;
  3. a word's class comes from its fields, and a nop's class from its
     predecessor (delay slot or not);
  4. ownership needs a line AND a compatible emission (on the line, within
     three lines, or -- for a prologue word -- among the end-stamped
     save/frame rows), and says which; anything else is `unowned`;
  5. the label rule and its tie-breaks;
  6. the refusals: size delta 0, and a force environment without --object.
"""
import os
import pathlib
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import align_symbol as als  # noqa: E402
import insertion_pairs as ip  # noqa: E402

SP, RA, T0, T1, T2, A0 = 29, 31, 8, 9, 10, 4


def itype(op, rs, rt, imm=0):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def rtype(rs, rt, rd, funct, shamt=0):
    return (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | funct


LOAD32, STORE32, ADDIMM, UPPER, BRANCH_EQ, CALL = 0x23, 0x2B, 0x09, 0x0F, 0x04, 0x03
FN_OR, FN_ADDU, FN_JR, FN_SUBU = 0x25, 0x21, 0x08, 0x23


def load(rt, base, off):
    return itype(LOAD32, base, rt, off)


def store(rt, base, off):
    return itype(STORE32, base, rt, off)


def add_imm(rt, rs, imm):
    return itype(ADDIMM, rs, rt, imm)


class ClassifyTests(unittest.TestCase):
    def test_stack_and_plain_memory_are_distinct(self):
        self.assertEqual(ip.classify_word(load(T0, SP, 0x18)), "stack-load")
        self.assertEqual(ip.classify_word(load(T0, A0, 4)), "load")
        self.assertEqual(ip.classify_word(store(T0, SP, 0x18)), "stack-store")
        self.assertEqual(ip.classify_word(store(T0, A0, 4)), "store")

    def test_register_copy_is_a_move_and_real_ops_are_alu(self):
        self.assertEqual(ip.classify_word(rtype(A0, 0, T0, FN_OR)), "move")
        self.assertEqual(ip.classify_word(rtype(0, A0, T0, FN_ADDU)), "move")
        self.assertEqual(ip.classify_word(rtype(A0, T1, T0, FN_ADDU)), "alu")
        self.assertEqual(ip.classify_word(rtype(A0, T1, T0, FN_SUBU)), "alu")

    def test_constants_frame_and_calls(self):
        self.assertEqual(ip.classify_word(add_imm(T0, 0, 5)), "const")
        self.assertEqual(ip.classify_word(itype(UPPER, 0, T0, 0x8000)), "const")
        self.assertEqual(ip.classify_word(add_imm(SP, SP, -0x20)), "frame")
        self.assertEqual(ip.classify_word(add_imm(T0, A0, 4)), "alu")
        self.assertEqual(ip.classify_word(rtype(RA, 0, 0, FN_JR)), "frame")
        self.assertEqual(ip.classify_word(CALL << 26), "call")
        self.assertEqual(ip.classify_word(itype(BRANCH_EQ, T0, T1, 3)), "branch")

    def test_a_nop_is_a_delay_nop_only_after_a_control_transfer(self):
        branch = itype(BRANCH_EQ, T0, T1, 3)
        self.assertEqual(ip.classify_word(0, branch), "delay-nop")
        self.assertEqual(ip.classify_word(0, load(T0, A0, 0)), "nop")
        self.assertEqual(ip.classify_word(0, None), "nop")

    def test_every_class_has_a_label_and_a_compatibility_set(self):
        for klass in ip.CLASSES:
            self.assertIn(ip.CLASS_LABEL[klass], ip.LABELS)
            if klass not in ("nop", "delay-nop"):
                self.assertIn(klass, ip.COMPATIBLE)


def script_for(base, target):
    return als._banded_edit_script(base, target)


class PairTests(unittest.TestCase):
    def test_one_extra_then_one_missing_is_one_closed_pair(self):
        target = list(range(100, 112))
        base = target[:3] + [999] + target[3:8] + target[9:]
        pairs = ip.pairs_from_script(script_for(base, target))
        self.assertEqual(len(pairs), 1)
        self.assertTrue(pairs[0]["closed"])
        self.assertEqual(pairs[0]["lo"], 3)
        self.assertEqual(pairs[0]["hi"], 9)

    def test_touching_shifts_are_two_pairs(self):
        # candidate-only at 2, target-only at 5 and 6: the first closes the
        # shift, the second opens a new one that runs to the end.
        target = list(range(100, 112))
        base = target[:2] + [999] + target[2:5] + target[7:]
        pairs = ip.pairs_from_script(script_for(base, target))
        self.assertEqual(len(pairs), 2)
        self.assertTrue(pairs[0]["closed"])
        self.assertFalse(pairs[1]["closed"])
        self.assertIsNone(pairs[1]["hi"])
        self.assertEqual(pairs[0]["hi"], pairs[1]["lo"])

    def test_a_single_insertion_runs_to_the_end(self):
        target = list(range(100, 110))
        base = target[:4] + [999] + target[4:]
        pairs = ip.pairs_from_script(script_for(base, target))
        self.assertEqual(len(pairs), 1)
        self.assertFalse(pairs[0]["closed"])
        self.assertEqual(pairs[0]["lo"], 4)

    def test_no_one_sided_word_no_pair(self):
        target = list(range(10))
        base = list(target)
        base[4] = 55
        self.assertEqual(ip.pairs_from_script(script_for(base, target)), [])


def accounting(base, target):
    script = script_for(base, target)
    differing = {s for s, (op, i, j) in enumerate(script)
                 if op in ("equal", "replace") and base[i] != target[j]}
    n = min(len(base), len(target))
    positional = [k for k in range(n) if base[k] != target[k]]
    pairs = ip.pairs_from_script(script)
    totals = ip.account(pairs, script, differing, positional, n,
                        abs(len(base) - len(target)))
    aligned = len(differing) + sum(1 for op, _, _ in script
                                   if op in ("delete", "insert"))
    return pairs, totals, aligned


class AccountingTests(unittest.TestCase):
    def test_shadow_plus_aligned_is_the_positional_count(self):
        target = list(range(100, 140))
        base = target[:5] + [999] + target[5:20] + target[21:]
        base[30] = 777  # one real difference outside the pair
        pairs, totals, aligned = accounting(base, target)
        self.assertEqual(totals["aligned_after_shadow"], aligned)
        self.assertEqual(totals["positional"],
                         totals["shadow"] + totals["aligned_after_shadow"])
        self.assertTrue(totals["outside_agrees"])
        self.assertEqual(totals["aligned_outside"], 1)
        # 15 shifted words inside the pair, 2 of them one-sided.
        self.assertEqual(pairs[0]["shadow"], pairs[0]["positional_in"] - 2)

    def test_an_open_pair_carries_the_length_difference(self):
        target = list(range(100, 130))
        base = target[:10] + [999] + target[10:]
        pairs, totals, aligned = accounting(base, target)
        self.assertEqual(pairs[0]["positional_in"], 21)  # 20 shifted + 1 extra
        self.assertEqual(pairs[0]["aligned_in"], 1)
        self.assertEqual(totals["aligned_after_shadow"], aligned)
        self.assertEqual(totals["aligned_after_shadow"], 1)


LINE_TABLE = """
obj.o:     file format elf32-tradbigmips

Disassembly of section .text:

00000100 <fn>:
fn():
some/dir/unit.c:40
 100:\tWORD\tPLACEHOLDER
 104:\tWORD\tPLACEHOLDER
some/dir/other.h:7
 108:\tWORD\tPLACEHOLDER
some/dir/unit.c:42 (discriminator 1)
 10c:\tWORD\tPLACEHOLDER
 200:\tWORD\tPLACEHOLDER
"""


class LineTableTests(unittest.TestCase):
    def test_offsets_are_function_relative_and_headers_kept_apart(self):
        table = ip.parse_line_table(LINE_TABLE, 0x100, 0x10, "unit.c")
        self.assertEqual(table, {0x0: 40, 0x4: 40, 0x8: "other.h:7", 0xC: 42})


def call(depth, name=None):
    return f"DKWB-CALL {depth} > {name}" if name else f"DKWB-CALL {depth} <"


def emit(line, emit_index, fn, proc=0, block=0):
    return (f"DKWB-EMIT-V1 proc={proc} block={block} emit={emit_index} op=1 "
            f"line={line} buffer=fwd fn={fn}")


TRACE = "\n".join([
    call(1, "f_main"), call(2, "f_eval"), call(3, "f_iloadistore"),
    call(4, "f_emit_rab"), emit(10, 1, "f_emit_rab"), call(4),
    call(3), call(3, "f_move_to_dest"), call(4, "f_emit_rr"),
    emit(11, 2, "f_emit_rr"), call(4), call(3),
    call(3, "f_jump"), call(4, "f_emit_branch_rrll"),
    emit(12, 3, "f_emit_branch_rrll"), call(5, "f_emit_rrll"),
    emit(12, 3, "f_emit_rrll"), call(5), call(4), call(3),
    call(3, "f_emit_dir2"), emit(12, 4, "f_emit_dir2"), call(3),
    call(3, "f_gen_entry_exit"), call(4, "f_gen_reg_save_restore"),
    call(5, "f_demit_rob_"), emit(19, 65000, "f_demit_rob_"), call(5),
    call(4), call(3),
    "DKWB-FREELIST ALLOC_GP_RESULT proc=0 reg=14 emitted=1 line=10",
    "DKWB-FREELIST ALLOC_GP proc=0 reg=14 emitted=1 line=10",
    emit(10, 1, "f_emit_rab", proc=1),
])


class TraceTests(unittest.TestCase):
    def setUp(self):
        self.procs = ip.parse_trace(TRACE)

    def test_families_and_handlers_come_from_the_call_stack(self):
        emits = self.procs[0]["emits"]
        self.assertEqual(emits[10], [{"family": "memory",
                                      "handler": "iloadistore"}])
        self.assertEqual(emits[11], [{"family": "move",
                                      "handler": "move_to_dest"}])
        self.assertEqual(emits[19], [{"family": "save",
                                      "handler": "gen_reg_save_restore"}])

    def test_a_wrapped_emission_is_one_instruction_and_directives_none(self):
        self.assertEqual(len(self.procs[0]["emits"][12]), 1)
        self.assertEqual(self.procs[0]["emits"][12][0]["family"], "branch")

    def test_only_result_rows_are_draws(self):
        self.assertEqual(self.procs[0]["draws"][10], 1)

    def test_the_procedure_is_picked_by_line_and_a_tie_refuses(self):
        self.assertEqual(ip.pick_proc(self.procs, {10, 11, 12, 19})[0], 0)
        self.assertIsNone(ip.pick_proc(self.procs, {10})[0])
        self.assertIsNone(ip.pick_proc(self.procs, {500})[0])
        self.assertIsNone(ip.pick_proc(self.procs, set())[0])


class OwnershipTests(unittest.TestCase):
    def setUp(self):
        self.proc = ip.parse_trace(TRACE)[0]

    def test_a_compatible_emission_on_the_line_owns_the_word(self):
        owner = ip.own("load", 10, self.proc)
        self.assertTrue(owner["owned"])
        self.assertEqual(owner["construct"], "iloadistore")
        self.assertEqual(owner["basis"], "line")

    def test_the_nearest_compatible_line_owns_it_and_says_so(self):
        owner = ip.own("load", 11, self.proc)
        self.assertTrue(owner["owned"])
        self.assertEqual(owner["basis"], "nearest")
        self.assertEqual(owner["construct_line"], 10)

    def test_nothing_compatible_within_reach_is_unowned_not_guessed(self):
        owner = ip.own("load", 15, self.proc)
        self.assertFalse(owner["owned"])
        self.assertIsNone(owner["construct"])
        self.assertIsNone(owner["basis"])
        # the search stays inside the function
        self.assertFalse(ip.own("load", 11, self.proc,
                                bounds=(11, 19))["owned"])

    def test_no_line_no_trace_or_header_line_is_unowned(self):
        self.assertFalse(ip.own("alu", None, self.proc)["owned"])
        self.assertFalse(ip.own("alu", 10, None)["owned"])
        self.assertFalse(ip.own("alu", "other.h:7", self.proc)["owned"])

    def test_a_nop_is_the_assemblers(self):
        owner = ip.own("delay-nop", 12, None)
        self.assertTrue(owner["owned"])
        self.assertEqual(owner["construct"], "as1")

    def test_a_prologue_save_is_matched_to_the_end_stamped_rows(self):
        self.assertFalse(ip.own("stack-store", 5, self.proc)["owned"])
        owner = ip.own("stack-store", 5, self.proc, bounds=(5, 19))
        self.assertTrue(owner["owned"])
        self.assertEqual(owner["basis"], "prologue")
        self.assertEqual(owner["construct"], "gen_reg_save_restore")
        # but not a body line with no memory emission within reach
        self.assertFalse(ip.own("stack-store", 15, self.proc,
                                bounds=(5, 19))["owned"])


def word(side, klass, shape=0, owned=True, construct="eval"):
    return {"side": side, "class": klass, "shape": shape,
            "owner": {"owned": owned, "construct": construct if owned else None}}


class LabelTests(unittest.TestCase):
    def test_same_class_both_sides_of_a_closed_pair_is_a_hoist(self):
        pair = {"closed": True, "words": [word("candidate", "load", 1),
                                          word("target", "load", 2)]}
        self.assertEqual(ip.pair_label(pair), "hoist")
        pair["closed"] = False
        self.assertEqual(ip.pair_label(pair), "extra-ILOD")

    def test_two_same_shape_branches_on_one_side_are_an_unrolled_loop(self):
        pair = {"closed": False, "words": [word("candidate", "branch", 5),
                                           word("candidate", "alu", 7),
                                           word("candidate", "branch", 5)]}
        self.assertEqual(ip.pair_label(pair), "unrolled-loop")

    def test_a_single_branch_with_repeated_alu_is_control_flow(self):
        pair = {"closed": False, "words": [word("candidate", "branch", 5),
                                           word("candidate", "alu", 7),
                                           word("candidate", "alu", 7),
                                           word("target", "branch", 6)]}
        self.assertEqual(ip.pair_label(pair), "missing-CSE")
        pair["words"] = [word("candidate", "branch", 5)]
        self.assertEqual(ip.pair_label(pair), "control-flow")

    def test_a_tie_goes_to_the_candidate_side(self):
        pair = {"closed": True, "words": [word("target", "store"),
                                          word("candidate", "alu")]}
        self.assertEqual(ip.pair_label(pair), "missing-CSE")

    def test_unowned_words_vote_unowned(self):
        pair = {"closed": False, "words": [word("candidate", "alu",
                                                owned=False)]}
        self.assertEqual(ip.pair_label(pair), "unowned")

    def test_a_callee_save_construct_overrides_the_class(self):
        pair = {"closed": False, "words": [word(
            "candidate", "stack-store", construct="gen_reg_save_restore")]}
        self.assertEqual(ip.pair_label(pair), "callee-save")

    def test_the_function_takes_the_label_of_the_most_shadow(self):
        pairs = [{"shadow": 1, "words": [1, 2, 3], "label": "hoist"},
                 {"shadow": 20, "words": [1], "label": "control-flow"}]
        self.assertEqual(ip.function_label(pairs), "control-flow")
        self.assertEqual(ip.function_label([]), "unowned")


class RefusalTests(unittest.TestCase):
    def test_a_force_environment_without_an_object_refuses(self):
        with mock.patch.dict(os.environ, {"CDX_FORCE": "p1:w1=c2"}), \
                mock.patch.object(ip, "measure") as measure, \
                mock.patch("sys.stderr"):
            self.assertEqual(ip.main(["func_x"]), 2)
            measure.assert_not_called()

    def test_a_trace_environment_without_an_object_refuses(self):
        with mock.patch.dict(os.environ, {"DKWB_UGEN_TRACE": "1"}), \
                mock.patch.object(ip, "measure") as measure, \
                mock.patch("sys.stderr"):
            self.assertEqual(ip.main(["func_x"]), 2)
            measure.assert_not_called()

    def test_size_delta_zero_refuses(self):
        import nm_ranking as nr

        class Streams:
            base_size = target_size = 64

        class Item:
            func = "func_x"
        with mock.patch.object(nr, "word_streams",
                               return_value=(Streams(), None)):
            with self.assertRaises(SystemExit) as caught:
                ip.refuse_delta_zero(Item(), pathlib.Path("unused.o"))
        self.assertIn("size delta 0", str(caught.exception))


if __name__ == "__main__":
    unittest.main()
