#!/usr/bin/env python3
"""Synthetic-object tests for tools/static_symbol_sizes.py."""

import pathlib
import struct
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import static_symbol_sizes as sss  # noqa: E402


def build_object(local_value=0, local_size=0, procedure=("helper", 0, 0x10),
                 mdebug_shift=0):
    """A minimal ELF32 BE object: .text, .symtab, .strtab, .shstrtab, .mdebug.

    The .mdebug header's table offsets point ``mdebug_shift`` bytes before
    where the tables really are, as after asm-processor rewrites an object.
    """
    text = bytes(0x20)
    strtab = b"\0helper\0entry\0"
    symbols = b"".join([
        struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0),
        struct.pack(">IIIBBH", 1, local_value, local_size, (0 << 4) | 2, 0, 1),
        struct.pack(">IIIBBH", 8, 0x10, 0x10, (1 << 4) | 2, 0, 1),
    ])
    shstrtab = b"\0.text\0.symtab\0.strtab\0.shstrtab\0.mdebug\0"
    names = {".text": 1, ".symtab": 7, ".strtab": 15, ".shstrtab": 23, ".mdebug": 33}

    name, start, size = procedure
    ss = b"\0" + name.encode() + b"\0"
    local_syms = b"".join([
        struct.pack(">III", 1, start, (sss.ST_STATIC_PROC << 26) | (sss.SC_TEXT << 21)),
        struct.pack(">III", 1, size, (sss.ST_END << 26) | (sss.SC_TEXT << 21)),
    ])
    layout = [(".text", text), (".symtab", symbols), (".strtab", strtab),
              (".shstrtab", shstrtab)]
    offset = 0x34
    placed = {}
    body = b""
    for sec, data in layout:
        placed[sec] = (offset + len(body), len(data))
        body += data
        body += bytes((-len(body)) % 4)
    mdebug_offset = offset + len(body)
    sym_off = mdebug_offset + 0x60
    ss_off = sym_off + len(local_syms)
    fd_off = ss_off + len(ss)
    fd = bytearray(72)
    struct.pack_into(">IIII", fd, 8, 0, 0, 0, 2)
    header = [0] * 24
    header[3] = mdebug_offset + 0x60 - mdebug_shift
    header[9] = sym_off - mdebug_shift
    header[15] = ss_off - mdebug_shift
    header[18] = 1
    header[19] = fd_off - mdebug_shift
    mdebug = struct.pack(">24I", *header) + local_syms + ss + bytes(fd)
    placed[".mdebug"] = (mdebug_offset, len(mdebug))
    body += mdebug
    body += bytes((-len(body)) % 4)
    shoff = offset + len(body)
    order = [".text", ".symtab", ".strtab", ".shstrtab", ".mdebug"]
    types = {".text": 1, ".symtab": 2, ".strtab": 3, ".shstrtab": 3, ".mdebug": 0x70000005}
    headers = struct.pack(">10I", *([0] * 10))
    for sec in order:
        off, length = placed[sec]
        link = 3 if sec == ".symtab" else 0
        headers += struct.pack(">10I", names[sec], types[sec], 0, 0, off, length,
                               link, 0, 4, 16 if sec == ".symtab" else 0)
    ident = b"\x7fELF\x01\x02\x01" + bytes(9)
    elf_header = ident + struct.pack(">HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0,
                                     0x34, 0, 0, 40, len(order) + 1, 4)
    return elf_header + body + headers


def local_size(data):
    sections = sss._sections(data)
    symtab = next(s for s in sections if s["type"] == sss.SHT_SYMTAB)
    return struct.unpack_from(">I", data, symtab["offset"] + 16 + 8)[0]


class StaticSymbolSizesTest(unittest.TestCase):
    def test_sizes_static_function_from_stend(self):
        data, count = sss.size_statics(build_object(mdebug_shift=0x40))
        self.assertEqual(count, 1)
        self.assertEqual(local_size(data), 0x10)

    def test_only_symbol_size_changes(self):
        original = build_object()
        data, _ = sss.size_statics(original)
        changed = [i for i in range(len(data)) if data[i] != original[i]]
        self.assertTrue(changed)
        self.assertLessEqual(max(changed) - min(changed), 3)

    def test_sized_object_is_untouched(self):
        original = build_object(local_size=0x10)
        data, count = sss.size_statics(original)
        self.assertEqual((data, count), (original, 0))

    def test_refuses_function_without_procedure(self):
        with self.assertRaises(sss.Failure):
            sss.size_statics(build_object(procedure=("other", 0, 0x10)))

    def test_refuses_extent_past_text(self):
        with self.assertRaises(sss.Failure):
            sss.size_statics(build_object(procedure=("helper", 0, 0x40)))


if __name__ == "__main__":
    unittest.main()
