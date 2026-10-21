"""Fixture tests for tools/private_rodata_plan.py (synthetic ELF; no build)."""

import hashlib
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import private_rodata_plan as prp  # noqa: E402

TEXT_BASE = 0x80100000
HI = 0x3C000000  # only the low halfword (the in-place addend) is read
LO = 0x8C000000


def build_elf(rodata_words, object_symbols=("D_80200004",)):
    """A minimal big-endian ELF32 REL object: .text, .rodata and their RELs."""
    text = b"".join(struct.pack(">I", w) for w in (
        HI, LO | 0x4,      # literal at private +0x4
        HI, LO | 0x8,      # first table at private +0x8
        HI, LO | 0x0,      # the kept prefix word
        HI, LO | 0x10,     # second table at private +0x10
        0, 0, 0, 0))       # case bodies at +0x20, +0x24, +0x28
    rodata = b"".join(struct.pack(">I", w) for w in rodata_words)

    names = b"\0.text\0.rodata\0" + b"".join(n.encode() + b"\0" for n in object_symbols)
    def name_offset(name):
        return names.index(name.encode() + b"\0")
    symtab = bytes(16)
    symtab += struct.pack(">IIIBBH", name_offset(".text"), 0, 0, prp.STT_SECTION, 0, 1)
    symtab += struct.pack(">IIIBBH", name_offset(".rodata"), 0, 0, prp.STT_SECTION, 0, 2)
    for name in object_symbols:
        symtab += struct.pack(">IIIBBH", name_offset(name), 0, 0, 0x10, 0, 0)

    rel_text = b"".join(struct.pack(">II", site, (2 << 8) | kind) for site, kind in (
        (0x00, prp.R_MIPS_HI16), (0x04, prp.R_MIPS_LO16),
        (0x08, prp.R_MIPS_HI16), (0x0C, prp.R_MIPS_LO16),
        (0x10, prp.R_MIPS_HI16), (0x14, prp.R_MIPS_LO16),
        (0x18, prp.R_MIPS_HI16), (0x1C, prp.R_MIPS_LO16)))
    rel_rodata = b"".join(struct.pack(">II", site, (1 << 8) | prp.R_MIPS_32)
                          for site in (0x8, 0xC, 0x10))

    sections = [
        (".text", 1, 6, text, 0, 0, 16, 0),
        (".rodata", 1, 2, rodata, 0, 0, 16, 0),
        (".rel.text", prp.SHT_REL, 0, rel_text, 5, 1, 4, 8),
        (".rel.rodata", prp.SHT_REL, 0, rel_rodata, 5, 2, 4, 8),
        (".symtab", prp.SHT_SYMTAB, 0, symtab, 6, 3, 4, 16),
        (".strtab", 3, 0, names, 0, 0, 1, 0),
    ]
    shstrtab = b"\0" + b"".join(s[0].encode() + b"\0" for s in sections) + b".shstrtab\0"
    sections.append((".shstrtab", 3, 0, shstrtab, 0, 0, 1, 0))

    body = bytearray(52)
    headers = [bytes(40)]
    for name, kind, flags, data, link, info, align, entsize in sections:
        while len(body) % 4:
            body.append(0)
        headers.append(struct.pack(">10I", shstrtab.index(name.encode() + b"\0"),
                                   kind, flags, 0, len(body), len(data), link,
                                   info, align, entsize))
        body += data
    while len(body) % 4:
        body.append(0)
    shoff = len(body)
    struct.pack_into(">4sBBBB8xHHIIIIIHHHHHH", body, 0, b"\x7fELF", 1, 2, 1, 0,
                     1, 8, 1, 0, 0, shoff, 0, 52, 0, 0, 40, len(headers),
                     len(headers) - 1)
    return bytes(body) + b"".join(headers)


ROD = [0x3C23D70A, 0x3F800000, 0x20, 0x24, 0x28, 0, 0, 0]


def listing(table=(0x80100020, 0x80100024), literal=0x3F800000):
    rows = [
        ("D_80200004", [(0x80200004, 0x3C888889, ".float 0.0167")]),
        ("D_80200008", [(0x80200008, literal, ".float 1.0")]),
        ("jtbl_8020000C", [(0x8020000C + 4 * i, w, f".word .L{w:08X}")
                           for i, w in enumerate(table)]),
        ("jtbl_80200040", [(0x80200040, 0x80100028, ".word .L80100028")]),
    ]
    out = []
    for name, lines in rows:
        out.append(f"dlabel {name}")
        for address, word, operand in lines:
            out.append(f"    /* {address - 0x80000000:X} {address:08X} {word:08X} */ {operand}")
        out.append(f"enddlabel {name}")
    return "\n".join(out) + "\n"


RANGES = [(0x4, 0x10, "D_80200008"), (0x10, 0x14, "jtbl_80200040")]


class PrivateRodataPlanTests(unittest.TestCase):
    def plan(self, rodata=ROD, text=None, **kwargs):
        return prp.plan(build_elf(rodata, **kwargs), text or listing(),
                        TEXT_BASE, 0x4, RANGES, "test")

    def test_labels_rebinds_anchor_and_digest(self):
        result = self.plan()
        self.assertEqual(result["labels"], [(".L80100020", 0x20), (".L80100024", 0x24),
                                            (".L80100028", 0x28)])
        self.assertEqual(result["checked"], {"literal": 1, "table": 3, "padding": 3})
        self.assertEqual(result["kept_sites"], [0x10, 0x14])
        self.assertEqual(result["rebinds"], [
            (0x00, ".rodata", "D_80200004"), (0x04, ".rodata", "D_80200004"),
            (0x08, ".rodata", "D_80200004"), (0x0C, ".rodata", "D_80200004"),
            (0x18, ".rodata", "testAnchor_jtbl_80200040"),
            (0x1C, ".rodata", "testAnchor_jtbl_80200040")])
        self.assertEqual(result["anchors"], {"testAnchor_jtbl_80200040": 0x80200030})
        discarded = b"".join(struct.pack(">I", w) for w in ROD[1:])
        self.assertEqual(result["digest"], hashlib.sha256(discarded).hexdigest())
        fragment = prp.render(result, 0x4)
        self.assertIn("--add-symbol .L80100020=.text:0x20,global", fragment)
        self.assertIn("--add-symbol testAnchor_jtbl_80200040=0x80200030,local $@", fragment)
        self.assertIn("0x18:.rodata:testAnchor_jtbl_80200040", fragment)
        self.assertTrue(fragment.endswith("sha256:" + result["digest"]))

    def test_unreferenced_shipped_symbol_becomes_an_anchor(self):
        # rebind_elf_relocations.py can bind only to a name the object holds.
        result = self.plan(object_symbols=())
        self.assertEqual(result["anchors"]["testAnchor_D_80200008"], 0x80200004)
        self.assertEqual(result["anchors"]["testAnchor_jtbl_8020000C"], 0x80200004)

    def test_refuses_a_case_label_that_differs(self):
        with self.assertRaisesRegex(prp.PlanError, "private case"):
            self.plan(text=listing(table=(0x80100020, 0x80100028)))

    def test_refuses_a_literal_that_differs(self):
        with self.assertRaisesRegex(prp.PlanError, "private literal"):
            self.plan(text=listing(literal=0x40000000))

    def test_refuses_uncovered_nonzero_bytes(self):
        with self.assertRaisesRegex(prp.PlanError, "no range covers"):
            self.plan(rodata=ROD[:6] + [1, 0])


if __name__ == "__main__":
    unittest.main()
