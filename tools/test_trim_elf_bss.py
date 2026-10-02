#!/usr/bin/env python3
"""NOBITS ownership/alignment controls using synthetic ELF metadata."""
import struct
import tempfile
import unittest
from pathlib import Path

import trim_elf_bss as tool
from reloc_surface import Elf


def fixture(path, *, size=4, old_size=16, flags=3, nobits=True, extra=False, leading=0, leading_size=None, value=0):
    names = b'\0.text\0.bss\0.shstrtab\0.symtab\0.strtab\0'
    strings = b'\0gravity\0other\0'
    symbol = lambda name, value, extent, info, section: struct.pack(
        '>IIIBBH', name, value, extent, info, 0, section)
    symbols = bytes(16) + symbol(0, 0, old_size, 3, 2)
    if leading:
        symbols += symbol(9, 0, leading if leading_size is None else leading_size, 17, 2)
    symbols += symbol(1, leading if value == 0 else value, size, 17, 2)
    if extra:
        symbols += symbol(9, 4, 4, 17, 2)
    data = bytearray(52)
    data[:16] = b'\x7fELF\x01\x02\x01' + bytes(9)
    struct.pack_into('>HHI', data, 16, 1, 8, 1)
    payloads = []
    for payload in [b'body', names, symbols, strings]:
        data.extend(bytes((-len(data)) % 4))
        payloads.append((len(data), len(payload)))
        data.extend(payload)
    data.extend(bytes((-len(data)) % 4))
    shoff = len(data)
    text, shstr, symtab, strtab = payloads
    name_offset = lambda name: names.index(name.encode() + b'\0')
    headers = [tuple([0] * 10),
        (name_offset('.text'), 1, 6, 0, text[0], text[1], 0, 0, 4, 0),
        (name_offset('.bss'), 8 if nobits else 1, flags, 0, 65536, old_size, 0, 0, 16, 0),
        (name_offset('.shstrtab'), 3, 0, 0, shstr[0], shstr[1], 0, 0, 1, 0),
        (name_offset('.symtab'), 2, 0, 0, symtab[0], symtab[1], 5, 2, 4, 16),
        (name_offset('.strtab'), 3, 0, 0, strtab[0], strtab[1], 0, 0, 1, 0)]
    for header in headers:
        data.extend(struct.pack('>10I', *header))
    struct.pack_into('>I', data, 32, shoff)
    struct.pack_into('>HHHH', data, 40, 52, 0, 0, 40)
    struct.pack_into('>HH', data, 48, len(headers), 3)
    path.write_bytes(data)


class TestTrimBss(unittest.TestCase):
    def test_nobits_metadata_only_and_idempotent(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'object.o'
            fixture(path)
            before = Elf(path)
            tool.trim(path, 'gravity', 4)
            after = Elf(path)
            _, bh = before.section('.bss')
            _, ah = after.section('.bss')
            self.assertEqual((ah[1], ah[5], ah[8]), (8, 4, 4))
            self.assertEqual(before.section_bytes('.text'), after.section_bytes('.text'))
            self.assertEqual(len(before.data), len(after.data))
            self.assertEqual(after.symbols()[1][2], 4)
            self.assertEqual(before.symbols()[2], after.symbols()[2])
            permitted = set()
            shoff = struct.unpack_from('>I', before.data, 32)[0]
            for offset in [shoff + 2 * 40 + 20, shoff + 2 * 40 + 32]:
                permitted.update(range(offset, offset + 4))
            _, symbol_header = before.section('.symtab')
            permitted.update(range(symbol_header[4] + 16 + 8, symbol_header[4] + 16 + 12))
            self.assertTrue(all(i in permitted for i, (x,y) in enumerate(zip(before.data, after.data)) if x != y))
            snapshot = path.read_bytes()
            tool.trim(path, 'gravity', 4)
            self.assertEqual(snapshot, path.read_bytes())

    def test_leading_objects_trim_to_the_last_end(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'object.o'
            fixture(path, leading=4)
            tool.trim(path, 'gravity', 4)
            _, header = Elf(path).section('.bss')
            self.assertEqual((header[5], header[8]), (8, 4))

    def test_leading_object_refusals_preserve_object(self):
        cases = [{'leading': 4, 'leading_size': 8},
                 {'leading': 4, 'value': 2},
                 {'leading': 4, 'old_size': 32}]
        for options in cases:
            with self.subTest(options=options):
                with tempfile.TemporaryDirectory() as directory:
                    path = Path(directory) / 'object.o'
                    fixture(path, **options)
                    original = path.read_bytes()
                    with self.assertRaises(ValueError):
                        tool.trim(path, 'gravity', 4)
                    self.assertEqual(original, path.read_bytes())

    def test_refusals_preserve_object(self):
        cases = [({'extra': True}, 'gravity', 4), ({'size': 0}, 'gravity', 4),
            ({'size': 6}, 'gravity', 4), ({'old_size': 32}, 'gravity', 4),
            ({'flags': 7}, 'gravity', 4), ({'nobits': False}, 'gravity', 4),
            ({}, 'wrong', 4), ({}, 'gravity', 3), ({}, 'gravity', 32)]
        for options, name, alignment in cases:
            with self.subTest(options=options, name=name, alignment=alignment):
                with tempfile.TemporaryDirectory() as directory:
                    path = Path(directory) / 'object.o'
                    fixture(path, **options)
                    original = path.read_bytes()
                    with self.assertRaises(ValueError):
                        tool.trim(path, name, alignment)
                    self.assertEqual(original, path.read_bytes())


if __name__ == '__main__':
    unittest.main()
