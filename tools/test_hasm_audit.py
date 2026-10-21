#!/usr/bin/env python3
"""Synthetic tests for tools/hasm_audit.py.

Every instruction word here is assembled field by field from the encoders
below; nothing is read from, or derived from, the ROM or any reference build.
"""
import os
import struct
import tempfile
import unittest

import hasm_audit as ha

BASE = 0x80100000


def rtype(funct, rs=0, rt=0, rd=0, sa=0):
    return (rs << 21) | (rt << 16) | (rd << 11) | (sa << 6) | funct


def itype(op, rs, rt, imm):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def jtype(op, target):
    return (op << 26) | ((target >> 2) & 0x3FFFFFF)


def fpop(fmt, ft, fs, fd, funct):
    return (ha.OP_COP1 << 26) | (fmt << 21) | (ft << 16) | (fs << 11) | (fd << 6) | funct


def branch(op, rs, rt, at, target):
    return itype(op, rs, rt, (target - (at + 4)) >> 2)


NOP = 0
SP, RA, A0, A1, V0, T0, T1, S0, AT = 29, 31, 4, 5, 2, 8, 9, 16, 1
RET = rtype(ha.FN_JR, rs=RA)
ADD_S, ADD_D = 0, 0


def move(rd, rs):
    return rtype(ha.FN_OR, rs=rs, rt=0, rd=rd)


def li(rt, value):
    return itype(ha.OP_ADDIU, 0, rt, value)


def frame(size):
    return itype(ha.OP_ADDIU, SP, SP, -size)


def unframe(size):
    return itype(ha.OP_ADDIU, SP, SP, size)


def store_word(rt, base, off):
    return itype(ha.OP_SW, base, rt, off)


def load_word(rt, base, off):
    return itype(ha.OP_LW, base, rt, off)


def call(target):
    return jtype(ha.OP_JAL, target)


def labels(words, base=BASE, prog=None):
    return {label for _o, label, _d in ha.analyse_routine(words, base, prog)}


def findings(words, base=BASE, prog=None):
    return ha.analyse_routine(words, base, prog)


class DecodeTests(unittest.TestCase):
    def test_trapping_forms_only(self):
        self.assertTrue(ha.decode(rtype(ha.FN_ADD, 4, 5, 2), BASE).trapping)
        self.assertTrue(ha.decode(rtype(ha.FN_SUB, 4, 5, 2), BASE).trapping)
        self.assertTrue(ha.decode(itype(ha.OP_ADDI, 4, 2, 1), BASE).trapping)
        self.assertFalse(ha.decode(rtype(ha.FN_ADDU, 4, 5, 2), BASE).trapping)
        self.assertFalse(ha.decode(rtype(ha.FN_SUBU, 4, 5, 2), BASE).trapping)
        self.assertFalse(ha.decode(li(2, 1), BASE).trapping)

    def test_odd_single_versus_double(self):
        single = ha.decode(fpop(ha.FMT_S, 4, 2, 5, ADD_S), BASE)      # fd odd, single
        self.assertEqual(single.odd_single, 1)
        double = ha.decode(fpop(ha.FMT_D, 4, 2, 6, ADD_D), BASE)      # all even
        self.assertEqual((double.odd_single, double.odd_double), (0, 0))
        bad = ha.decode(fpop(ha.FMT_D, 4, 3, 6, ADD_D), BASE)         # odd at double width
        self.assertEqual(bad.odd_double, 1)
        widen = ha.decode(fpop(ha.FMT_S, 0, 1, 2, 33), BASE)          # single to double
        self.assertEqual((widen.odd_single, widen.odd_double), (1, 0))
        move_in = ha.decode(itype(ha.OP_LWC1, A0, 1, 0), BASE)        # 32-bit load: ambiguous
        self.assertEqual(move_in.odd_single, 0)

    def test_merge_load_is_not_a_read(self):
        ins = ha.decode(itype(ha.OP_LWL, A0, T0, 0), BASE)
        self.assertFalse(ins.uses >> T0 & 1)
        self.assertTrue(ins.defs >> T0 & 1)

    def test_targets(self):
        b = ha.decode(branch(ha.OP_BEQ, 4, 0, BASE, BASE + 0x20), BASE)
        self.assertEqual(b.target, BASE + 0x20)
        c = ha.decode(call(0x80200040), BASE)
        self.assertEqual((c.ctl, c.target), (ha.CTL_CALL, 0x80200040))

    def test_zero_is_never_defined(self):
        self.assertEqual(ha.decode(NOP, BASE).defs, 0)


class DetectorTests(unittest.TestCase):
    def test_nonarg_register_read_on_entry(self):
        reads = [rtype(ha.FN_ADDU, T1, A0, V0), RET, NOP]
        self.assertIn("nonarg-register-read-on-entry", labels(reads))
        defined = [li(T1, 3), rtype(ha.FN_ADDU, T1, A0, V0), RET, NOP]
        self.assertNotIn("nonarg-register-read-on-entry", labels(defined))

    def test_a_call_defines_caller_saved_registers(self):
        words = [frame(0x18), store_word(RA, SP, 0x14), call(0x80200000), NOP,
                 move(V0, T0), load_word(RA, SP, 0x14), RET, unframe(0x18)]
        self.assertNotIn("nonarg-register-read-on-entry", labels(words))

    def test_delay_slot_runs_before_the_target(self):
        at = BASE
        words = [branch(ha.OP_BEQ, A0, 0, at, at + 12), li(T0, 1), li(T0, 2),
                 move(V0, T0), RET, NOP]
        self.assertNotIn("nonarg-register-read-on-entry", labels(words))

    def test_likely_delay_slot_runs_only_when_taken(self):
        at = BASE
        words = [branch(ha.OP_BEQL, A0, 0, at, at + 12), li(T0, 1), move(V0, T0),
                 RET, NOP]
        self.assertIn("nonarg-register-read-on-entry", labels(words))

    def test_callee_saved_write_without_save(self):
        bare = [li(S0, 1), RET, NOP]
        self.assertIn("callee-saved-written-unsaved", labels(bare))
        saved = [frame(0x18), store_word(S0, SP, 0x10), li(S0, 1),
                 load_word(S0, SP, 0x10), RET, unframe(0x18)]
        self.assertNotIn("callee-saved-written-unsaved", labels(saved))

    def test_frame_shapes(self):
        small = [frame(8), store_word(RA, SP, 4), call(0x80200000), NOP,
                 load_word(RA, SP, 4), RET, unframe(8)]
        self.assertIn("nonleaf-frame-below-o32-minimum", labels(small))
        leaf_unused = [frame(0x10), store_word(RA, SP, 0), load_word(RA, SP, 0),
                       RET, unframe(0x10)]
        self.assertIn("leaf-saves-unused-ra", labels(leaf_unused))
        leaf_scratch = [frame(0x10), store_word(RA, SP, 0), li(RA, 6),
                        load_word(RA, SP, 0), RET, unframe(0x10)]
        got = labels(leaf_scratch)
        self.assertIn("leaf-saves-ra", got)
        self.assertNotIn("leaf-saves-unused-ra", got)

    def test_fp_pair_save_placement(self):
        def save_fp(reg, off):
            return itype(ha.OP_SWC1, SP, reg, off)
        ido_shape = [frame(0x18), save_fp(21, 0x10), save_fp(20, 0x14), RET, unframe(0x18)]
        self.assertNotIn("callee-saved-fp-pair-misplaced", labels(ido_shape))
        swapped = [frame(0x18), save_fp(20, 0x10), save_fp(21, 0x14), RET, unframe(0x18)]
        self.assertIn("callee-saved-fp-pair-misplaced", labels(swapped))

    def test_computed_jump_into_own_code(self):
        at = BASE
        hi, lo = (at + 0x20) >> 16, (at + 0x20) & 0xFFFF
        words = [itype(ha.OP_LUI, 0, T0, hi), itype(ha.OP_ADDIU, T0, T0, lo),
                 rtype(ha.FN_SUBU, T0, A0, T0), rtype(ha.FN_JR, rs=T0), NOP,
                 NOP, NOP, NOP, RET, NOP]
        self.assertIn("computed-jump-own-code", labels(words))

    def test_table_jump_is_resolved_not_evidence(self):
        table = 0x80300000
        words = [itype(ha.OP_LUI, 0, AT, table >> 16), rtype(ha.FN_ADDU, AT, A0, AT),
                 load_word(T0, AT, table & 0xFFFF), rtype(ha.FN_JR, rs=T0), NOP,
                 li(V0, 1), RET, NOP]
        entries = {table: BASE + 20, table + 4: 0}
        prog = ha.Program(read_word=lambda v: entries.get(v))
        got = labels(words, prog=prog)
        self.assertIn("table-or-pointer-jump", got)
        self.assertNotIn("unreachable-code", got)
        self.assertNotIn("computed-jump-own-code", got)

    def test_falls_off_end_and_foreign_jumps(self):
        self.assertIn("falls-off-end", labels([li(V0, 1)]))
        prog = ha.Program(function_starts={0x80200000})
        self.assertIn("tail-jump", labels([jtype(ha.OP_J, 0x80200000), NOP], prog=prog))
        self.assertIn("jump-into-foreign-interior",
                      labels([jtype(ha.OP_J, 0x80200010), NOP], prog=prog))

    def test_system_instruction(self):
        read_status = (ha.OP_COP0 << 26) | (0 << 21) | (V0 << 16) | (12 << 11)
        self.assertIn("system-instruction", labels([read_status, RET, NOP]))

    def test_secondary_entry_point(self):
        prog = ha.Program(interior_entries={BASE: {BASE + 8}})
        got = findings([li(V0, 1), RET, li(V0, 2), RET, NOP], prog=prog)
        self.assertIn((8, "secondary-entry-point", None), got)

    def test_every_label_is_classified(self):
        known = set(ha.LEDGER_MECHANISMS) | set(ha.EXTRA_MECHANISMS) | ha.INFO_LABELS
        self.assertFalse(set(ha.LEDGER_MECHANISMS) & set(ha.EXTRA_MECHANISMS))
        self.assertFalse((set(ha.LEDGER_MECHANISMS) | set(ha.EXTRA_MECHANISMS)) & ha.INFO_LABELS)
        seen = set()
        for words in ([li(S0, 1), RET, NOP], [rtype(ha.FN_ADD, 4, 5, 2), RET, NOP],
                      [li(V0, 1)], [frame(8), call(0x80200000), NOP, RET, unframe(8)]):
            seen |= labels(words)
        self.assertTrue(seen)
        self.assertTrue(seen <= known, seen - known)


class SummaryTests(unittest.TestCase):
    def test_control_demotes_a_label(self):
        f = [(0, "trapping-add-sub", None), (4, "frame-not-first", None),
             (8, "table-or-pointer-jump", None)]
        mech, info = ha.summarise_findings(f, {"frame-not-first": 7})
        self.assertEqual([m["label"] for m in mech], ["trapping-add-sub"])
        why = {i["label"]: i["why_info"] for i in info}
        self.assertIn("control", why["frame-not-first"])
        self.assertIn("not an IDO-impossible", why["table-or-pointer-jump"])

    def test_classes_and_verdicts(self):
        hand = {"kind": "handwritten-source"}
        glob = {"kind": "global-asm"}
        comp = {"kind": "compiled-c"}
        mech = [{"label": "odd-fp-single"}]
        self.assertEqual(ha.derive_classes(hand, [], None, None), [1])
        self.assertEqual(ha.derive_classes(glob, mech, None, None), [2, 3])
        self.assertEqual(ha.derive_classes(None, [], None, {"parent": "p"}), [3])
        self.assertEqual(ha.derive_classes(None, [], None, {"parent": None}), [])
        self.assertEqual(ha.verdict_for(2, [2, 3], glob), "supported")
        self.assertEqual(ha.verdict_for(1, [3], None), "supported-other-class")
        self.assertEqual(ha.verdict_for(3, [], None), "unsupported")
        self.assertEqual(ha.verdict_for(1, [1], comp), "refuted")


def ref_object(words, masks=None, symbols=(), path="t/build/x.s.o"):
    text = struct.pack(">%dI" % len(words), *words)
    return ha.RefObject("t", path, text, masks or [ha.MASK32] * len(words), list(symbols))


class StubSources:
    def __init__(self, kind="handwritten-source"):
        self._kind = kind

    def kind(self, obj, symbol, offset=None):
        return self._kind

    def handwritten_source_defining(self, title, symbol):
        return None


class FarmTests(unittest.TestCase):
    def test_prefilter_covers_relocated_fields(self):
        cases = [(call(0x80200000), ha.RELOC_MASKS[4]),
                 (itype(ha.OP_LUI, 0, T0, 0x1234), ha.RELOC_MASKS[5]),
                 (itype(ha.OP_ADDIU, T0, T0, 0x10), ha.RELOC_MASKS[6]),
                 (load_word(T0, T0, 0x10), ha.RELOC_MASKS[6]),
                 (branch(ha.OP_BEQ, 4, 0, BASE, BASE + 8), ha.RELOC_MASKS[10])]
        for word, mask in cases:
            self.assertEqual(ha.prefilter(word) & ~mask & ha.MASK32, 0)
        self.assertNotEqual(ha.prefilter(ha.SEP_WORD), ha.SEP_WORD)

    def test_masked_search(self):
        body = [li(V0, 1), call(0x80200000), NOP, RET, NOP, li(T0, 2)]
        ref_words = list(body)
        ref_words[1] = call(0x80000000)            # the linker's field differs
        masks = [ha.MASK32] * len(body)
        masks[1] = ha.RELOC_MASKS[4]
        farm = ha.Farm("", [ref_object([li(A0, 9)]), ref_object(ref_words, masks)])
        hits = farm.search(body)
        self.assertEqual([(h[0].path, h[1]) for h in hits], [("t/build/x.s.o", 0)])
        unmasked = ha.Farm("", [ref_object(ref_words)])
        self.assertEqual(unmasked.search(body), [])

    def test_cover_and_identity(self):
        a = [li(V0, k) for k in range(1, 21)]
        b = [li(T0, k) for k in range(1, 21)]
        farm = ha.Farm("", [ref_object(a, symbols=[(0, 80, "first")], path="t/build/a.s.o"),
                            ref_object(b, symbols=[(0, 80, "second")], path="t/build/b.s.o")])
        ident = ha.Identifier(farm, StubSources(), "", "")
        run = a + b
        record, per = ident.identify_subsegment(run, [("one", 0, 80), ("two", 80, 80)])
        self.assertEqual(record["claim"], "multi-segment")
        self.assertEqual(per["one"][0]["object"], "t/build/a.s.o")
        self.assertEqual(per["two"][0]["symbol"], "second")
        whole, per = ident.identify_subsegment(a, [("one", 0, 80)])
        self.assertEqual(whole["claim"], "whole-object")

    def test_short_compiled_match_cannot_refute(self):
        a = [li(V0, 1), li(V0, 2), RET, NOP]
        farm = ha.Farm("", [ref_object(a, symbols=[(0, 16, "f")], path="t/build/f.c.o")])
        ident = ha.Identifier(farm, StubSources("compiled-c"), "", "")
        _rec, per = ident.identify_subsegment(a, [("f", 0, 16)])
        self.assertEqual(per["f"], [])

    def test_deletion_alignment(self):
        ref = [li(V0, 1), li(V0, 2), li(V0, 3), store_word(5, A0, 0), li(V0, 4), RET, NOP]
        mine = [li(V0, 1), li(V0, 3), store_word(6, A0, 0), li(V0, 4), RET, NOP]
        got = ha.align_deletions(mine, ref, [ha.MASK32] * len(ref))
        self.assertEqual((got["kept"], got["repointed_stores"], got["deleted"]), (5, 1, 1))
        inserted = [li(V0, 1), li(V0, 9), li(V0, 2)]
        self.assertIsNone(ha.align_deletions(inserted, ref, [ha.MASK32] * len(ref)))


def elf_object(text_words, relocs=(), symbols=()):
    """A minimal big-endian ELF32 relocatable: .text, .rel.text, .symtab."""
    text = struct.pack(">%dI" % len(text_words), *text_words)
    rel = b"".join(struct.pack(">II", off, typ) for off, typ in relocs)
    strtab = b"\0"
    syms = [struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0)]
    for value, size, name in symbols:
        syms.append(struct.pack(">IIIBBH", len(strtab), value, size, 0x12, 0, 1))
        strtab += name.encode() + b"\0"
    symtab = b"".join(syms)
    names = [b"", b".text", b".rel.text", b".symtab", b".strtab", b".shstrtab"]
    shstr = b"\0".join(names) + b"\0"
    offs, pos = [], 0
    for n in names:
        offs.append(pos)
        pos += len(n) + 1
    body = [text, rel, symtab, strtab, shstr]
    data, starts = b"", []
    cursor = 52
    for chunk in body:
        starts.append(cursor + len(data))
        data += chunk + bytes(-len(chunk) % 4)
    shoff = 52 + len(data)
    header = (b"\x7fELF\x01\x02\x01" + bytes(9)
              + struct.pack(">HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0, 52, 0, 0, 40, 6, 5))
    shdrs = [bytes(40)]
    spec = [(1, 1, 0, 0, 0), (2, 9, 3, 1, 8), (3, 2, 4, 0, 16), (4, 3, 0, 0, 0), (5, 3, 0, 0, 0)]
    for k, (ni, typ, link, info, ent) in enumerate(spec):
        shdrs.append(struct.pack(">IIIIIIIIII", offs[ni], typ, 0, 0, starts[k],
                                 len(body[k]), link, info, 4, ent))
    return header + data + b"".join(shdrs)


class ElfTests(unittest.TestCase):
    def test_parse_object(self):
        blob = elf_object([li(V0, 1), call(0x80000000), NOP, RET, NOP],
                          relocs=[(4, 4)],
                          symbols=[(0, 20, "f"), (0, 20, "f.NON_MATCHING")])
        obj = ha.parse_object(blob, "t", "t/build/f.c.o")
        self.assertEqual(len(obj.masks), 5)
        self.assertEqual(obj.masks[1], ha.RELOC_MASKS[4])
        self.assertEqual(obj.symbol_at(0), "f")
        self.assertTrue(obj.marked_global_asm(0))
        self.assertFalse(obj.irregular)


class TreeTests(unittest.TestCase):
    LEDGER = "\n".join([
        "# header",
        "# 1. Byte-identical to an object (main/alpha: routines two to five).",
        "# 2. Proved by mechanism and identity.  main/beta: routine one.",
        "# 3. Mechanism only.",
        "",
        "entrypoint",
        "main/alpha",
        "main/beta",
        "",
        "# Evidence class 3 (mechanism, no reference identity).",
        "main/gamma",
        "",
        "libultra/delta",
        "",
    ])

    def test_parse_ledger(self):
        self.assertEqual(ha.parse_ledger(self.LEDGER), [
            ("entrypoint", 1), ("main/alpha", 1), ("main/beta", 2),
            ("main/gamma", 3), ("libultra/delta", 1)])

    def test_parse_lock(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "lock")
            with open(path, "w") as fh:
                fh.write("# comment\n[one]\ncommit = abc\nobject_roots = build lib/b\n"
                         "digest = d1\n[two]\nobject_roots = out\n")
            self.assertEqual(ha.parse_lock(path), [
                ("one", ["build", "lib/b"], "abc", "d1"), ("two", ["out"], None, None)])

    def test_scan_sources(self):
        with tempfile.TemporaryDirectory() as tmp:
            src = os.path.join(tmp, "src")
            os.makedirs(os.path.join(src, "main"))
            with open(os.path.join(src, "main", "a.c"), "w") as fh:
                fh.write("void alpha(void);\n/* alpha(void) { */\n"
                         "int beta(int x,\n         int y) {\n    return alpha();\n}\n"
                         '#pragma GLOBAL_ASM("asm/nonmatchings/main/gamma.s")\n')
            found = ha.scan_sources(src, {"alpha", "beta", "gamma"}, tmp)
        self.assertNotIn("alpha", found)
        self.assertEqual(found["beta"], [(os.path.join("src", "main", "a.c"), 3, "c-definition")])
        self.assertEqual(found["gamma"][0][2], "global-asm")

    def test_map_sections(self):
        text = (" .text          0x80001000      0x40 build/asm/main/x.s.o\n"
                " .text          0x80001040      0x20 build/src/main/y.c.o\n")
        sections = ha.parse_map_text(text)
        self.assertEqual(ha.object_at(sections, 0x80001010), "build/asm/main/x.s.o")
        self.assertEqual(ha.object_at(sections, 0x80001050), "build/src/main/y.c.o")
        self.assertIsNone(ha.object_at(sections, 0x80002000))


class HelperTests(unittest.TestCase):
    SIBLINGS = [("parent", 0x1000, 0x1040), ("helper", 0x1040, 0x1048)]

    def test_only_the_parent_refers(self):
        xrefs = {0x1040: [ha.Xref(0x1030, "jump")]}
        got = ha.verify_helper(self.SIBLINGS[1], self.SIBLINGS, xrefs, None)
        self.assertEqual(got["parent"], "parent")

    def test_a_reference_from_outside_breaks_it(self):
        xrefs = {0x1040: [ha.Xref(0x1030, "jump"), ha.Xref(0x9000, "call")]}
        self.assertIsNone(ha.verify_helper(self.SIBLINGS[1], self.SIBLINGS, xrefs, None)["parent"])
        overlay = ha.verify_helper(self.SIBLINGS[1], self.SIBLINGS,
                                   {0x1040: [ha.Xref(0x1030, "jump")]}, None, [0x5000])
        self.assertIsNone(overlay["parent"])
        self.assertIsNone(ha.verify_helper(self.SIBLINGS[1], self.SIBLINGS, {}, None)["parent"])

    def test_raw_references(self):
        target = 0x80101234
        words = [itype(ha.OP_LUI, 0, T0, (target + 0x8000) >> 16),
                 itype(ha.OP_ADDIU, T0, T0, target & 0xFFFF), target]
        blob = struct.pack(">3I", *words)
        got = ha.raw_references(blob, 0x100, [target])
        self.assertEqual(sorted(got[target]), [0x104, 0x108])

    def test_xrefs_and_interior_entries(self):
        a_base, b_base = 0x80000400, 0x80000420
        a = [call(b_base + 8), NOP, RET, NOP, NOP, NOP, NOP, NOP]
        b = [li(V0, 1), RET, li(V0, 2), RET, NOP, NOP, NOP, NOP]
        rom = bytes(0x400) + struct.pack(">16I", *(a + b))
        image = ha.Image(rom, [ha.Segment(0x400, 0x440, a_base)])
        funcs = [(a_base, 0x20), (b_base, 0x20)]
        xrefs = ha.build_xrefs(image, funcs)
        self.assertEqual([x.kind for x in xrefs[b_base + 8]], ["call"])
        self.assertEqual(ha.interior_entries(funcs, xrefs)[b_base], {b_base + 8})


if __name__ == "__main__":
    unittest.main()
