#!/usr/bin/env python3
"""Synthetic tests for tools/fp_queue_model.py.

Every graph here is hand-built, and every register in it is assigned by the
forward simulator below -- an independent, order-fixed statement of the queue
rules -- so the search is checked against the rules rather than against
itself.  Instruction words are assembled field by field from the encoders;
nothing is read from, or derived from, the ROM or any reference build.
"""
import os
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fp_queue_model as fq  # noqa: E402

A0, A1, A2, A3, T6, SP = 4, 5, 6, 7, 14, 29
FADD, FMUL = 0, 2


# ---------------------------------------------------------------------------
# Encoders.

def itype(op, rs, rt, imm):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def lwc1(ft, base, off):
    return itype(fq.OP_LWC1, base, ft, off)


def swc1(ft, base, off):
    return itype(fq.OP_SWC1, base, ft, off)


def mtc1(rt, fs):
    return (fq.OP_COP1 << 26) | (fq.COP1_MT << 21) | (rt << 16) | (fs << 11)


def fp3(funct, fd, fs, ft):
    return (fq.OP_COP1 << 26) | (fq.FMT_S << 21) | (ft << 16) | (fs << 11) | (fd << 6) | funct


def lw(rt, base, off):
    return itype(0x23, base, rt, off)


# ---------------------------------------------------------------------------
# A template graph and an order-fixed simulator of the queue rules.

def transform_template(rows=3, translation=False):
    """[(op, srcs, extra)] for `out[c] = m0*x + m1*y + m2*z (+ m3)` per row.

    op is "formal", "load", "mul", "add" or "store"; srcs index earlier
    entries.  Entries are in the natural evaluation order.
    """
    t = [("formal", (), 12), ("formal", (), 14), ("formal", (), 16)]
    for c in range(rows):
        base = len(t)
        t += [("load", (), 4 * c), ("mul", (base, 0), None),
              ("load", (), 16 + 4 * c), ("mul", (base + 2, 1), None),
              ("add", (base + 1, base + 3), None),
              ("load", (), 32 + 4 * c), ("mul", (base + 5, 2), None),
              ("add", (base + 4, base + 6), None)]
        if translation:
            t += [("load", (), 48 + 4 * c), ("add", (len(t), len(t) - 1), None)]
        t.append(("store", (len(t) - 1,), c))
    return t


def simulate(template, queue=(4, 6, 8, 10, 18)):
    """Registers for each entry under the rules, evaluating in template order."""
    queue = list(queue)
    uses = [0] * len(template)
    for _op, srcs, _x in template:
        for s in srcs:
            uses[s] += 1
    left = list(uses)
    regs = []
    for op, srcs, extra in template:
        if op == "formal":
            regs.append(extra)
            continue
        dest = None
        if op != "store":
            dest = queue.pop(0)
        for s in srcs:
            left[s] -= 1
        for s in srcs:                      # structural left operand first
            if left[s] == 0 and template[s][0] != "formal":
                queue.append(regs[s])
        regs.append(dest)
    return regs


def words_for(template, regs):
    """Encode a template whose registers are `regs` (formals from a1..a3)."""
    out = []
    formal_gpr = iter((A1, A2, A3))
    for (op, srcs, extra), reg in zip(template, regs):
        if op == "formal":
            out.append(mtc1(next(formal_gpr), reg))
        elif op == "load":
            out.append(lwc1(reg, A0, extra))
        elif op in ("mul", "add"):
            out.append(fp3(FMUL if op == "mul" else FADD, reg, regs[srcs[0]], regs[srcs[1]]))
        else:
            out.append(lw(T6, SP, 16 + 4 * extra))
            out.append(swc1(regs[srcs[0]], T6, 0))
    return out


def edited_words():
    """The hand edit, on a simulated compile with translation terms: drop each
    row's translation load and add and store the row sum instead."""
    template = transform_template(translation=True)
    regs = simulate(template)
    keep, index, t2, r2 = [], {}, [], []
    drop = set()
    for i, (op, srcs, extra) in enumerate(template):
        if op == "store":
            p = srcs[0]
            drop |= {p, template[p][1][0]}
    for i, (op, srcs, extra) in enumerate(template):
        if i in drop:
            continue
        if op == "store":
            srcs = (template[srcs[0]][1][1],)
        index[i] = len(t2)
        t2.append((op, tuple(index[s] for s in srcs), extra))
        r2.append(regs[i])
    return words_for(t2, r2), regs


# ---------------------------------------------------------------------------

class DecodeTests(unittest.TestCase):
    def test_fp_events(self):
        self.assertEqual(fq.decode(lwc1(8, A0, 20)), fq.Op("load", 8, (), A0, 20))
        self.assertEqual(fq.decode(swc1(6, T6, 0)).kind, "store")
        self.assertEqual(fq.decode(swc1(6, T6, 0)).srcs, (6,))
        self.assertEqual(fq.decode(mtc1(A1, 12)), fq.Op("mtc1", 12, ()))
        self.assertEqual(fq.decode(fp3(FADD, 18, 6, 10)), fq.Op("arith", 18, (6, 10)))
        self.assertIsNone(fq.decode(lw(T6, SP, 16)))
        self.assertEqual(fq.decode(lwc1(4, A0, -4)).offset, -4)

    def test_dataflow_follows_redefinition(self):
        words = [mtc1(A1, 12), lwc1(4, A0, 0), fp3(FMUL, 6, 4, 12),
                 lwc1(4, A0, 16), fp3(FADD, 8, 6, 4), swc1(8, T6, 0)]
        nodes = fq.dataflow(words)
        self.assertEqual([n.kind for n in nodes],
                         ["formal", "temp", "temp", "temp", "temp", "store"])
        self.assertEqual(nodes[4].srcs, (2, 3))      # the second $f4, not the first
        self.assertEqual(nodes[5].srcs, (4,))

    def test_undefined_register_is_out_of_scope(self):
        with self.assertRaises(fq.OutOfScope):
            fq.dataflow([fp3(FADD, 4, 6, 8)])


class ModelTests(unittest.TestCase):
    def nodes(self, template, regs):
        return fq.dataflow(words_for(template, regs))

    def test_simulated_compile_fits(self):
        t = transform_template()
        self.assertIsNotNone(fq.fits(self.nodes(t, simulate(t))))

    def test_simulated_compile_with_translation_fits(self):
        t = transform_template(translation=True)
        self.assertIsNotNone(fq.fits(self.nodes(t, simulate(t))))

    def test_hand_edit_fits_no_order_and_extras_restore_it(self):
        words, compiled = edited_words()
        nodes = fq.dataflow(words)
        self.assertFalse(fq.writes_uopt_first(nodes))
        self.assertIsNone(fq.fits(nodes))
        self.assertFalse(any(fq.fits(nodes, start=s) for s in fq.start_orders(nodes)))
        extra = fq.insert_extras(nodes)
        fit = fq.fits(extra)
        self.assertIsNotNone(fit)
        # The extra values land where the compiled translation load and sum were.
        t = transform_template(translation=True)
        want = [compiled[i] for i, (op, s, x) in enumerate(t)
                if (op == "load" and x is not None and x >= 48) or
                (op == "add" and t[s[0]][0] == "load" and t[s[0]][2] >= 48)]
        got = [fit.extra_regs[i] for i in fit.order if extra[i].kind == "extra"]
        self.assertEqual(got, want)

    def test_delete_translations_matches_the_hand_edit(self):
        t = transform_template(translation=True)
        edited = fq.delete_translations(self.nodes(t, simulate(t)))
        words, _ = edited_words()
        by_hand = fq.dataflow(words)
        self.assertEqual([(n.dest, n.srcs, n.kind) for n in edited],
                         [(n.dest, n.srcs, n.kind) for n in by_hand])
        self.assertIsNone(fq.delete_translations(self.nodes(transform_template(),
                                                            simulate(transform_template()))))

    def test_a_shuffled_register_is_rejected(self):
        t = transform_template()
        regs = simulate(t)
        last_store = max(i for i, e in enumerate(t) if e[0] == "store")
        final_sum = t[last_store][1][0]
        first_load = next(i for i, e in enumerate(t) if e[0] == "load")
        regs[first_load], regs[final_sum] = regs[final_sum], regs[first_load]
        self.assertIsNone(fq.fits(self.nodes(t, regs)))

    def test_start_orders_count(self):
        nodes = self.nodes(transform_template(), simulate(transform_template()))
        # 5 registers free; $f0/$f2 optionally inserted anywhere.
        self.assertEqual(sum(1 for _ in fq.start_orders(nodes)), 120 + 2 * 720 + 5040)

    def test_a_multi_use_value_returns_at_its_last_use(self):
        # One value read by three products, as a reloaded formal is.
        t = [("formal", (), 12), ("load", (), 0)]
        for c in range(3):
            base = len(t)
            t += [("load", (), 4 + 4 * c), ("mul", (base, 1), None),
                  ("mul", (base + 1, 0), None), ("store", (base + 2,), c)]
        regs = simulate(t, queue=(4, 6, 8, 10, 16, 18))     # only $f12 is a formal
        self.assertIsNotNone(fq.fits(self.nodes(t, regs)))

    def test_a_coloured_local_outside_the_queue(self):
        t = transform_template(rows=1)
        regs = simulate(t)
        regs[-2] = 0                    # the row sum coloured into $f0 by uopt
        nodes = self.nodes(t, regs)
        self.assertTrue(fq.writes_uopt_first(nodes))
        self.assertIsNotNone(fq.fits(nodes))

    def test_a_queue_register_given_to_uopt(self):
        t = transform_template()
        regs = simulate(t, queue=(4, 6, 8, 10))     # $f18 out of the queue
        regs[-2] = 18                               # the last row sum lives there
        nodes = self.nodes(t, regs)
        self.assertEqual(fq.explain(nodes), "queue, uopt-owned $f18")


class InputTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        os.makedirs(os.path.join(self.root, "baseroms"))
        self.saved = fq.ROOT_DIR
        fq.ROOT_DIR = self.root

    def tearDown(self):
        fq.ROOT_DIR = self.saved
        self.tmp.cleanup()

    def write(self, words, offset=0x400, name="synth"):
        with open(os.path.join(self.root, "symbol_addrs.us.txt"), "w") as fh:
            fh.write("%s = 0x80100000; // type:func size:0x%X ROM 0x%x\n"
                     % (name, 4 * len(words), offset))
            fh.write("other = 0x80001000; // type:func size:0x10\n")
        blob = bytearray(offset + 4 * len(words) + 16)
        struct.pack_into(">%dI" % len(words), blob, offset, *words)
        with open(os.path.join(self.root, "baseroms", "mickey.us.z64"), "wb") as fh:
            fh.write(bytes(blob))

    def test_read_symbol(self):
        self.write([0, 0])
        self.assertEqual(fq.read_symbol("synth"), (0x400, 8))
        self.assertEqual(fq.read_symbol("other"), (0x1000 - 0x400 + 0x1000, 16))

    def run_rom(self):
        import contextlib
        import io
        with contextlib.redirect_stdout(io.StringIO()) as out:
            rc = fq.main(["--rom", "synth"])
        return rc, out.getvalue()

    def test_rom_mode_on_the_hand_edit(self):
        words, _ = edited_words()
        self.write(words)
        rc, out = self.run_rom()
        self.assertEqual(rc, 0, out)
        self.assertIn("0 of 6600 queue start orders fit", out)
        self.assertIn("fewest rows that need the extra pair: 0, 1", out)

    def test_rom_mode_on_a_compile(self):
        t = transform_template()
        self.write(words_for(t, simulate(t)))
        rc, out = self.run_rom()
        self.assertEqual(rc, 1, out)

    def test_rom_mode_without_a_rom(self):
        os.rmdir(os.path.join(self.root, "baseroms"))
        self.write_symbols_only()
        rc, _ = self.run_rom()
        self.assertEqual(rc, 2)

    def write_symbols_only(self):
        with open(os.path.join(self.root, "symbol_addrs.us.txt"), "w") as fh:
            fh.write("synth = 0x80100000; // type:func size:0x8 ROM 0x400\n")

    def test_object_function_words(self):
        t = transform_template(translation=True)
        words = words_for(t, simulate(t))
        path = os.path.join(self.root, "x.o")
        with open(path, "wb") as fh:
            fh.write(elf_object([0] * 2 + words, [("pad", 0, 8), ("guMtxXFMF", 8, 4 * len(words))]))
        self.assertEqual(fq.object_function_words(path, "guMtxXFMF"), words)
        self.assertEqual(len(fq.object_function_words(path)), 2 + len(words))

    def test_reference_mode(self):
        t = transform_template(translation=True)
        path = os.path.join(self.root, "mtxxfmf.o")
        with open(path, "wb") as fh:
            fh.write(elf_object(words_for(t, simulate(t)), [("guMtxXFMF", 0, 0)]))
        saved = fq.locate_reference
        fq.locate_reference = lambda *a: ([path], 1, self.root)
        try:
            import contextlib
            import io
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(fq.main(["--reference"]), 0)
            fq.locate_reference = lambda *a: ([], None, None)
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(fq.main(["--reference"]), 2)
        finally:
            fq.locate_reference = saved


def elf_object(words, symbols):
    """A minimal big-endian ELF32 relocatable: .text, .symtab, .strtab."""
    text = struct.pack(">%dI" % len(words), *words)
    strtab = b"\0"
    syms = [struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0)]
    for name, value, size in symbols:
        syms.append(struct.pack(">IIIBBH", len(strtab), value, size, 0x12, 0, 1))
        strtab += name.encode() + b"\0"
    symtab = b"".join(syms)
    shstr = b"\0.text\0.symtab\0.strtab\0.shstrtab\0"
    names = {n: shstr.index(n.encode()) for n in (".text", ".symtab", ".strtab", ".shstrtab")}
    body = text + symtab + strtab + shstr
    offs = [52, 52 + len(text), 52 + len(text) + len(symtab),
            52 + len(text) + len(symtab) + len(strtab)]
    shoff = 52 + len(body)
    header = b"\x7fELF" + bytes([1, 2, 1]) + bytes(9)
    header += struct.pack(">HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0, 52, 0, 0, 40, 5, 4)
    sh = [bytes(40),
          struct.pack(">IIIIIIIIII", names[".text"], 1, 6, 0, offs[0], len(text), 0, 0, 4, 0),
          struct.pack(">IIIIIIIIII", names[".symtab"], 2, 0, 0, offs[1], len(symtab), 3, 1, 4, 16),
          struct.pack(">IIIIIIIIII", names[".strtab"], 3, 0, 0, offs[2], len(strtab), 0, 0, 1, 0),
          struct.pack(">IIIIIIIIII", names[".shstrtab"], 3, 0, 0, offs[3], len(shstr), 0, 0, 1, 0)]
    return header + body + b"".join(sh)


if __name__ == "__main__":
    unittest.main()
