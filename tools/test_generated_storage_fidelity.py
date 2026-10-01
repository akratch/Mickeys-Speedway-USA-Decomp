#!/usr/bin/env python3
"""Fail-closed controls for the single reviewed generated-rodata baseline."""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface  # noqa: E402
import source_symbol_fidelity as sf  # noqa: E402

START = 0x17E0
SIZE = 6900
TEXT_SIZE = 0x4000
RODATA_SIZE = 0x250
HI16, LO16, R26, R32 = 5, 6, 4, 2


def _align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & -alignment


def _make_object(path: Path, *, mutations: dict | None = None) -> None:
    mutations = dict(mutations or {})
    text = bytearray(TEXT_SIZE)
    rodata = bytearray(RODATA_SIZE)
    symbols = [("", 0, 0, 0, 0, 0)]
    text_sym = len(symbols)
    symbols.append((".text", 0, TEXT_SIZE, 3, 0, 1))
    rodata_sym = len(symbols)
    symbols.append((".rodata", 0, RODATA_SIZE, 3, 0, 2))
    symbols.append(("func_800517E0", START, SIZE, 0x12, 0, 1))
    names = ["", ".text", ".rodata", "func_800517E0"]
    rel_text: list[tuple[int, int, int]] = []
    reserved = set()

    def put_word(site: int, value: int) -> None:
        struct.pack_into(">I", text, site, value)

    # Fifty-seven normal function edges.
    cursor = START + 0x1000
    for i in range(57):
        name = f"call_owner_{i}"
        sym = len(symbols); names.append(name)
        symbols.append((name, 0, 0, 0x12, 0, 0))
        put_word(cursor, 0x08000000)
        rel_text.append((cursor, (sym << 8) | R26, sym))
        reserved.add(cursor); cursor += 4

    # Eighty-seven ordinary data HI/LO groups, for the 245-record owned
    # function census after adding its seven generated-storage pairs.
    for i in range(87):
        name = f"data_owner_{i}"
        sym = len(symbols); names.append(name)
        symbols.append((name, 0, 0, 0x11, 0, 0))
        high, low = cursor, cursor + 4; cursor += 8
        put_word(high, 0x3C080000)
        put_word(low, 0x8D000000)
        rel_text.extend(((high, (sym << 8) | HI16, sym),
                         (low, (sym << 8) | LO16, sym)))
        reserved.update((high, low))

    if mutations.get("function_bytes"):
        text[START + 0x18] ^= 1

    owned_groups = {
        0x004: (0xBC, 0xC0, 49),
        0x008: (0xDC, 0xE4, 35),
        0x204: (0x2E0, 0x2E4, 49),
        0x208: (0x7C4, 0x7C8, 49),
        0x20C: (0x880, 0x884, 49),
        0x210: (0xC8C, 0xC90, 49),
        0x214: (0x1484, 0x148C, 35),
    }
    for addend, (hi_rel, lo_rel, opcode) in owned_groups.items():
        hi, lo = START + hi_rel, START + lo_rel
        put_word(hi, 0x3C080000)
        put_word(lo, (opcode << 26) | (8 << 21) | addend)
        rel_text.extend(((hi, (rodata_sym << 8) | HI16, rodata_sym),
                         (lo, (rodata_sym << 8) | LO16, rodata_sym)))
        reserved.update((hi, lo))
    # Full-TU context: one leading literal user and one trailing-table user.
    for addend, hi, lo in ((0, 0x100, 0x104), (0x238, 0x3500, 0x3504)):
        put_word(hi, 0x3C080000)
        put_word(lo, (35 << 26) | (8 << 21) | addend)
        rel_text.extend(((hi, (rodata_sym << 8) | HI16, rodata_sym),
                         (lo, (rodata_sym << 8) | LO16, rodata_sym)))

    if mutations.get("unowned_hi_register"):
        struct.pack_into(">I", text, 0x3500,
                         struct.unpack_from(">I", text, 0x3500)[0] ^ (1 << 16))
    if mutations.get("extra_unowned_site_reloc"):
        sym = next(i for i, row in enumerate(symbols) if row[0] == "data_owner_0")
        rel_text.append((0x3500, (sym << 8) | LO16, sym))

    if mutations.get("bad_load"):
        site = START + owned_groups[0x204][1]
        put_word(site, (43 << 26) | (8 << 21) | 0x204)  # SW

    table_offsets = [off for lo, hi in ((8, 0x204), (0x214, 0x238),
                                        (0x238, 0x250))
                     for off in range(lo, hi, 4)]
    for i, off in enumerate(table_offsets):
        addend = (START + i * 4) if off < 0x238 else (i - 136) * 4
        struct.pack_into(">I", rodata, off, addend)
    if mutations.get("literal_byte"):
        rodata[0x204] ^= 0x1

    rel_rodata = [(off, (text_sym << 8) | R32, text_sym)
                  for off in table_offsets]
    if mutations.get("table_missing"):
        rel_rodata.pop(10)
    if mutations.get("table_duplicate"):
        rel_rodata[10] = rel_rodata[9]
    if mutations.get("table_wrong_type"):
        rel_rodata[10] = (rel_rodata[10][0], (text_sym << 8) | LO16, text_sym)
    if mutations.get("table_reordered"):
        rel_rodata[10], rel_rodata[11] = rel_rodata[11], rel_rodata[10]
    if mutations.get("table_outside_target"):
        struct.pack_into(">I", rodata, rel_rodata[10][0], TEXT_SIZE)
    if mutations.get("unowned_head"):
        rodata[0] ^= 1
    if mutations.get("unowned_tail"):
        struct.pack_into(">I", rodata, 0x238, 4)
    if mutations.get("bad_rodata_symbol"):
        row = symbols[rodata_sym]
        symbols[rodata_sym] = (row[0], row[1], row[2], 1, row[4], row[5])
    if mutations.get("extra_rodata_owner"):
        symbols.append(("generated_extra_literal", 4, 4, 0x11, 0, 2))
        names.append("generated_extra_literal")

    # Keep the on-disk order of this owner's stream sorted by text site.  Other
    # owners remain in compiler-style grouped order and are not reordered.
    rel_text.sort(key=lambda row: row[0])
    if mutations.get("cross_boundary_pair"):
        # Move the first selected high outside the function while retaining
        # its owner and matching row multiplicity.
        for i, row in enumerate(rel_text):
            if row[2] == rodata_sym and row[1] & 0xFF == HI16:
                rel_text[i] = (0x120, row[1], row[2]); break
    if mutations.get("pending_high"):
        for i, row in enumerate(rel_text):
            if row[2] == rodata_sym and row[1] & 0xFF == LO16:
                rel_text.pop(i); break

    strings = bytearray(b"\0")
    name_offsets = {"": 0}
    for name in names[1:]:
        if name not in name_offsets:
            name_offsets[name] = len(strings)
            strings.extend(name.encode() + b"\0")
    symtab = b"".join(struct.pack(">IIIBBH", name_offsets[n], val, size,
                                  info, other, shndx)
                      for n, val, size, info, other, shndx in symbols)
    rel_text_data = b"".join(struct.pack(">II", off, info)
                              for off, info, _ in rel_text)
    rel_rodata_data = b"".join(struct.pack(">II", off, info)
                                for off, info, _ in rel_rodata)
    sections = [
        {"name":"", "type":0, "flags":0, "data":b"", "link":0, "info":0, "align":0, "entsize":0},
        {"name":".text", "type":1, "flags":6, "data":bytes(text), "link":0, "info":0, "align":4, "entsize":0},
        {"name":".rodata", "type":1, "flags":2, "data":bytes(rodata), "link":0, "info":0, "align":16, "entsize":1},
        {"name":".rel.text", "type":9, "flags":0, "data":rel_text_data, "link":5, "info":1, "align":4, "entsize":8},
        {"name":".rel.rodata", "type":9, "flags":0, "data":rel_rodata_data, "link":5, "info":2, "align":4, "entsize":8},
        {"name":".symtab", "type":2, "flags":0, "data":symtab, "link":6, "info":3, "align":4, "entsize":16},
        {"name":".strtab", "type":3, "flags":0, "data":bytes(strings), "link":0, "info":0, "align":1, "entsize":0},
    ]
    if mutations.get("bad_rel_link"):
        sections[4]["link"] = 0
    if mutations.get("bad_rel_info"):
        sections[4]["info"] = 1
    if mutations.get("bad_rel_entsize"):
        sections[4]["entsize"] = 4
    shstr = bytearray(b"\0"); shname = {"":0}
    for section in sections[1:]:
        name = section["name"]
        shname[name] = len(shstr); shstr.extend(name.encode() + b"\0")
    shname[".shstrtab"] = len(shstr); shstr.extend(b".shstrtab\0")
    sections.append({"name":".shstrtab", "type":3, "flags":0, "data":bytes(shstr),
                     "link":0, "info":0, "align":1, "entsize":0})
    if mutations.get("bad_rodata_flags"):
        sections[2]["flags"] |= 1
    if mutations.get("bad_rodata_type"):
        sections[2]["type"] = 8
    if mutations.get("bad_rodata_align"):
        sections[2]["align"] = 8
    output = bytearray(b"\0" * 52)
    for section in sections[1:]:
        alignment = max(1, section["align"])
        output.extend(b"\0" * (_align(len(output), alignment) - len(output)))
        section["offset"] = len(output); output.extend(section["data"])
    shoff = _align(len(output), 4); output.extend(b"\0" * (shoff-len(output)))
    for section in sections:
        output.extend(struct.pack(">10I", shname.get(section["name"],0), section["type"],
            section["flags"],0,section.get("offset",0),len(section["data"]),section["link"],
            section["info"],section["align"],section["entsize"]))
    output[:16] = b"\x7fELF\x01\x02\x01\0" + b"\0"*8
    struct.pack_into(">HHIIIIIHHHHHH", output,16,1,8,1,0,0,shoff,0,52,0,0,40,
                     len(sections),len(sections)-1)
    path.write_bytes(output)


class GeneratedStorageFidelityTests(unittest.TestCase):
    def pair(self, directory: Path, mutation: str | None = None):
        raw, full = directory/"raw.o", directory/"full.o"
        _make_object(raw)
        _make_object(full, mutations={mutation: True} if mutation else {})
        return raw, full

    def test_default_refuses_section_owner_but_fixed_opt_in_passes(self):
        with tempfile.TemporaryDirectory() as d:
            raw, full = self.pair(Path(d))
            with self.assertRaisesRegex(sf.SourceFidelityError, "SECTION"):
                sf.compare_source_symbols(raw, full, "func_800517E0")
            report = sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")
            self.assertEqual(245, report["relocation_count"])
            self.assertEqual(142, report["generated_storage"]["table_relocation_count"])
            self.assertEqual(142, len(report["generated_storage"]["ordered_table_edges"]))
            self.assertEqual(9, len(report["generated_storage"]["full_tu_hi_lo_group_rows"]))
            self.assertEqual(6, sum(not edge["owned_by_function"]
                                    for edge in report["generated_storage"]["ordered_table_edges"]))
            self.assertEqual(7, report["generated_storage"]["owned_hi_lo_groups"])
            self.assertEqual(2, report["generated_storage"]["unowned_hi_lo_groups"])
            self.assertFalse(report["c_origin_verified"])
            self.assertFalse(report["runtime_identity_proved"])
            self.assertFalse(report["promotion_authority"])

    def test_only_fixed_function_can_use_opt_in(self):
        with tempfile.TemporaryDirectory() as d:
            raw, full = self.pair(Path(d))
            with self.assertRaisesRegex(sf.SourceFidelityError, "limited"):
                sf.compare_generated_rodata_source_symbols(raw, full, "other")

    def test_table_completeness_and_storage_mutations_fail_closed(self):
        for mutation, reason in (("literal_byte", "complete generated .rodata bytes"),
                                 ("unowned_head", "complete generated .rodata bytes"),
                                 ("unowned_tail", "complete generated .rodata bytes|target is outside"),
                                 ("table_missing", "row count"),
                                 ("table_duplicate", "unordered or invalid"),
                                 ("table_wrong_type", "edge owner or relocation type"),
                                 ("table_reordered", "unordered or invalid"),
                                 ("table_outside_target", "target is outside"),
                                 ("function_bytes", "owned executable bytes")):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as d:
                raw, full = self.pair(Path(d), mutation)
                with self.assertRaisesRegex(sf.SourceFidelityError, reason):
                    sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")

    def test_wrong_storage_section_shape_fails(self):
        for mutation, reason in (("bad_rodata_flags", "header differs"),
                                 ("bad_rodata_type", "header differs"),
                                 ("bad_rodata_align", "header differs")):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as d:
                raw, full = self.pair(Path(d), mutation)
                with self.assertRaisesRegex(sf.SourceFidelityError, reason):
                    sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")

    def test_section_owner_and_rel_table_linkage_are_pinned(self):
        for mutation, reason in (("bad_rodata_symbol", "section identity|SECTION symbol metadata"),
                                 ("bad_rel_link", "REL table header"),
                                 ("bad_rel_info", "REL table"),
                                 ("bad_rel_entsize", "REL table header")):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as d:
                raw, full = self.pair(Path(d), mutation)
                with self.assertRaisesRegex(sf.SourceFidelityError, reason):
                    sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")

    def test_unowned_generated_uses_compare_complete_instruction_words(self):
        with tempfile.TemporaryDirectory() as d:
            raw, full = self.pair(Path(d), "unowned_hi_register")
            with self.assertRaisesRegex(sf.SourceFidelityError,
                                        "complete generated HI/LO graph differs"):
                sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")

    def test_generated_rodata_symbol_closure_rejects_extra_named_owner(self):
        with tempfile.TemporaryDirectory() as d:
            raw, full = self.pair(Path(d), "extra_rodata_owner")
            with self.assertRaisesRegex(sf.SourceFidelityError,
                                        "additional symbol owner"):
                sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")

    def test_generated_use_sites_reject_other_full_tu_relocation_owner(self):
        with tempfile.TemporaryDirectory() as d:
            raw, full = self.pair(Path(d), "extra_unowned_site_reloc")
            with self.assertRaisesRegex(sf.SourceFidelityError,
                                        "conflicting full-TU relocation"):
                sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")

    def test_unsupported_access_and_invalid_pair_graph_fail(self):
        for mutation, reason in (("bad_load", "approved word/FP load"),
                                 ("cross_boundary_pair", "ownership boundary"),
                                 ("pending_high", "HI/LO addend|unpaired")):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as d:
                raw, full = self.pair(Path(d), mutation)
                with self.assertRaises(sf.SourceFidelityError):
                    sf.compare_generated_rodata_source_symbols(raw, full, "func_800517E0")


if __name__ == "__main__":
    unittest.main()
