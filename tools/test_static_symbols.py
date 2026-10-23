#!/usr/bin/env python3
"""Synthetic-object tests for tools/static_symbols.py.

The fixture is a hand-built IDO-shaped relocatable: an exported ``entry``
followed by two static functions, ``helper`` (forward-declared, so IDO left a
sized ``UND`` placeholder) and ``tail`` (no symbol at all), plus a relocation
against an undefined ``callee`` whose symbol index must survive the rewrite.
Its .text is zero filler; no ROM content.
"""

from __future__ import annotations

import pathlib
import struct
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import static_symbols as ss  # noqa: E402

STB_GLOBAL = 1
R_MIPS_26 = 4


def _pack_symbol(name, value, size, info, shndx):
    return struct.pack(">IIIBBH", name, value, size, info, 0, shndx)


def build_object(*, statics=(("helper", 0x10, 0x10), ("tail", 0x20, 0x8)),
                 placeholder=True, entry_size=0x10, asmproc_local=False):
    text = bytes(0x30)
    strings = bytearray(b"\0")

    def string(name):
        offset = len(strings)
        strings.extend(name.encode() + b"\0")
        return offset

    # [null, .text section, (asm-processor's size-0 local)] | globals
    symbols = [_pack_symbol(0, 0, 0, 0, 0), _pack_symbol(0, 0, len(text), 3, 1)]
    if asmproc_local:
        symbols.append(_pack_symbol(string("helper"), 0x10, 0, ss.STT_FUNC, 1))
    first_global = len(symbols)
    if placeholder and not asmproc_local:
        symbols.append(_pack_symbol(string("helper"), 0, 0x10, STB_GLOBAL << 4, 0))
    symbols.append(_pack_symbol(string("entry"), 0, entry_size, STB_GLOBAL << 4 | ss.STT_FUNC, 1))
    callee = len(symbols)
    symbols.append(_pack_symbol(string("callee"), 0, 0, STB_GLOBAL << 4 | ss.STT_FUNC, 0))
    rel = struct.pack(">II", 0x4, callee << 8 | R_MIPS_26)

    # .mdebug: header, local symbols, local strings, one file descriptor.
    procs = [("entry", 0, entry_size, ss.ST_PROC)] + [
        (name, start, size, ss.ST_STATIC_PROC) for name, start, size in statics]
    local_strings = bytearray(b"\0")
    local_syms = b""
    for name, start, size, st in procs:
        iss = len(local_strings)
        local_strings.extend(name.encode() + b"\0")
        local_syms += struct.pack(">III", iss, start, st << 26 | ss.SC_TEXT << 21)
        local_syms += struct.pack(">III", iss, size, ss.ST_END << 26 | ss.SC_TEXT << 21)

    shstr = b"\0.text\0.rel.text\0.mdebug\0.symtab\0.strtab\0.shstrtab\0"
    names = {n: shstr.index(n.encode() + b"\0") for n in
             (".text", ".rel.text", ".mdebug", ".symtab", ".strtab", ".shstrtab")}
    body = bytearray(0x34)
    placed = {}

    def put(name, blob):
        body.extend(bytes(-len(body) % 4))
        placed[name] = (len(body), len(blob))
        body.extend(blob)

    put(".text", text)
    put(".rel.text", rel)
    mdebug_at = len(body) + (-len(body) % 4)
    sym_at = mdebug_at + 0x60
    ss_at = sym_at + len(local_syms)
    fd_at = ss_at + len(local_strings) + (-(ss_at + len(local_strings)) % 4)
    header = [0] * 24
    header[0] = ss.HDRR_MAGIC << 16
    header[8], header[9] = len(local_syms) // 12, sym_at
    header[14], header[15] = len(local_strings), ss_at
    header[18], header[19] = 1, fd_at
    fd = bytearray(72)
    struct.pack_into(">IIII", fd, 8, 0, 0, 0, len(local_syms) // 12)
    mdebug = (struct.pack(">24I", *header) + local_syms + local_strings
              + bytes(fd_at - ss_at - len(local_strings)) + fd)
    put(".mdebug", mdebug)
    put(".symtab", b"".join(symbols))
    put(".strtab", bytes(strings))
    put(".shstrtab", shstr)
    body.extend(bytes(-len(body) % 4))
    shoff = len(body)
    order = [".text", ".rel.text", ".mdebug", ".symtab", ".strtab", ".shstrtab"]
    kinds = {".text": 1, ".rel.text": ss.SHT_REL, ".mdebug": 0x70000005,
             ".symtab": ss.SHT_SYMTAB, ".strtab": 3, ".shstrtab": 3}
    links = {".rel.text": (4, 1), ".symtab": (5, first_global)}
    body.extend(bytes(40))
    for name in order:
        offset, length = placed[name]
        link, info = links.get(name, (0, 0))
        body.extend(struct.pack(">10I", names[name], kinds[name], 0, 0, offset, length,
                                link, info, 4, {".symtab": 16, ".rel.text": 8}.get(name, 0)))
    ident = b"\x7fELF\x01\x02\x01" + bytes(9)
    body[:0x34] = ident + struct.pack(">HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0,
                                      0x34, 0, 0, 40, len(order) + 1, len(order))
    return bytes(body)


def symbols(data):
    sections = ss._sections(data)
    index = next(i for i, s in enumerate(sections) if s[1] == ss.SHT_SYMTAB)
    symtab, strtab = sections[index], sections[sections[index][6]]
    strings = data[strtab[4]:strtab[4] + strtab[5]]
    rows = [(ss._cstring(strings, row[0]), row[1], row[2], row[3], row[5])
            for row in ss._symbols(data, symtab)]
    return rows, symtab[7], sections


def relocation_targets(data):
    rows, _first_global, sections = symbols(data)
    rel = next(s for s in sections if s[1] == ss.SHT_REL)
    return [rows[struct.unpack_from(">I", data, offset + 4)[0] >> 8][0]
            for offset in range(rel[4], rel[4] + rel[5], 8)]


def section_bytes(data, name):
    section = next(s for s in ss._sections(data) if s[10] == name)
    return data[section[4]:section[4] + section[5]]


class StaticSymbolsTest(unittest.TestCase):
    def test_exported_function_followed_by_statics(self):
        original = build_object()
        data, added = ss.add_static_symbols(original)
        self.assertEqual(added, ["helper", "tail"])
        rows, first_global, _ = symbols(data)
        local = (ss.STB_LOCAL << 4) | ss.STT_FUNC
        self.assertIn(("helper", 0x10, 0x10, local, 1), rows[:first_global])
        self.assertIn(("tail", 0x20, 0x8, local, 1), rows[:first_global])
        # The placeholder is gone; every global follows every local.
        self.assertEqual([row[0] for row in rows[first_global:]], ["entry", "callee"])
        self.assertTrue(all(row[3] >> 4 == ss.STB_LOCAL for row in rows[:first_global]))
        # The relocation still names its symbol, and no section byte moved.
        self.assertEqual(relocation_targets(data), ["callee"])
        for name in (".text", ".mdebug"):
            self.assertEqual(section_bytes(data, name), section_bytes(original, name))
        rel_before, rel_after = (section_bytes(d, ".rel.text") for d in (original, data))
        self.assertEqual(rel_before[:4], rel_after[:4])
        self.assertEqual(rel_before[7], rel_after[7])

    def test_second_run_changes_nothing(self):
        data, _ = ss.add_static_symbols(build_object())
        self.assertEqual(ss.add_static_symbols(data), (data, []))

    def test_object_without_statics_is_byte_identical(self):
        original = build_object(statics=(), placeholder=False)
        self.assertEqual(ss.add_static_symbols(original), (original, []))

    def test_asm_processor_size_zero_local_is_sized(self):
        data, added = ss.add_static_symbols(build_object(asmproc_local=True))
        self.assertEqual(added, ["helper", "tail"])
        rows, first_global, _ = symbols(data)
        self.assertEqual([row[:3] for row in rows[:first_global] if row[0] == "helper"],
                         [("helper", 0x10, 0x10)])

    def test_global_disagreeing_with_mdebug_refuses(self):
        # entry's st_size says 0x14 but its procedure ends at 0x10.
        data = bytearray(build_object())
        sections = ss._sections(bytes(data))
        symtab = next(s for s in sections if s[1] == ss.SHT_SYMTAB)
        for offset in range(symtab[4], symtab[4] + symtab[5], 16):
            if struct.unpack_from(">I", data, offset + 8)[0] == 0x10 and data[offset + 12] == 0x12:
                struct.pack_into(">I", data, offset + 8, 0x14)
        with self.assertRaisesRegex(ss.Failure, "disagrees"):
            ss.add_static_symbols(bytes(data))

    def test_overlapping_static_refuses(self):
        with self.assertRaisesRegex(ss.Failure, "overlap"):
            ss.add_static_symbols(build_object(statics=(("helper", 0x8, 0x10),)))


if __name__ == "__main__":
    unittest.main()
