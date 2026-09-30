#!/usr/bin/env python3
"""Synthetic ELF controls for source_symbol_fidelity's narrow REL adapter."""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import source_symbol_fidelity as sf  # noqa: E402

STB_LOCAL, STB_GLOBAL = 0, 1
STT_OBJECT, STT_FUNC, STT_SECTION = 1, 2, 3
SHF_ALLOC, SHF_WRITE, SHF_EXECINSTR = 2, 1, 4
SHT_PROGBITS, SHT_REL, SHT_SYMTAB, SHT_STRTAB = 1, 9, 2, 3


def _align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & -alignment


def make_elf(path: Path, *, words: list[int], relocs: list[tuple[int, int, str]],
             owner_name: str, owner_type: int, owner_state: str,
             owner_value: int = 8, owner_size: int = 4,
             owner_binding: int = STB_GLOBAL, owner_storage: str = ".data",
             owner_section_flags: int = SHF_ALLOC | SHF_WRITE,
             extra_sections: list[tuple[str, int, int, bytes]] | None = None,
             duplicate_owner: bool = False) -> None:
    """Write a tiny big-endian ELF32 MIPS ET_REL containing a single .text."""
    text = b"".join(struct.pack(">I", word) for word in words)
    extras = list(extra_sections or [])
    sections: list[dict] = [{"name": "", "type": 0, "flags": 0,
                             "data": b"", "link": 0, "info": 0, "align": 0,
                             "entsize": 0}]
    text_idx = len(sections)
    sections.append({"name": ".text", "type": SHT_PROGBITS,
                     "flags": SHF_ALLOC | SHF_EXECINSTR, "data": text,
                     "link": 0, "info": 0, "align": 4, "entsize": 0})
    owner_idx = None
    for name, sh_type, flags, data in extras:
        if name == owner_storage:
            owner_idx = len(sections)
        sections.append({"name": name, "type": sh_type, "flags": flags,
                         "data": data, "link": 0, "info": 0,
                         "align": 4, "entsize": 0})
    if owner_state == "defined" and owner_idx is None:
        owner_idx = len(sections)
        data = bytes(max(32, owner_value + owner_size))
        sections.append({"name": owner_storage, "type": SHT_PROGBITS,
                         "flags": owner_section_flags, "data": data,
                         "link": 0, "info": 0, "align": 4, "entsize": 0})
    owner_shndx = owner_idx if owner_state == "defined" else 0

    names = ["", "source_fn", owner_name]
    if duplicate_owner:
        names.append(owner_name)
    strtab = bytearray(b"\0")
    name_offsets: dict[str, int] = {"": 0}
    for name in names[1:]:
        if name not in name_offsets:
            name_offsets[name] = len(strtab)
            strtab.extend(name.encode() + b"\0")

    symbol_rows = [(0, 0, 0, 0, 0, 0)]
    symbol_rows.append((name_offsets["source_fn"], 0, len(text),
                        (STB_GLOBAL << 4) | STT_FUNC, 0, text_idx))
    owner_value_in_elf = owner_value if owner_state == "defined" else 0
    owner_size_in_elf = owner_size if owner_state == "defined" else 0
    symbol_rows.append((name_offsets[owner_name], owner_value_in_elf,
                        owner_size_in_elf,
                        (owner_binding << 4) | owner_type, 0, owner_shndx))
    if duplicate_owner:
        symbol_rows.append((name_offsets[owner_name], owner_value_in_elf,
                            owner_size_in_elf,
                            (owner_binding << 4) | owner_type, 0, owner_shndx))
    symtab = b"".join(struct.pack(">IIIBBH", *row) for row in symbol_rows)
    symbol_index = 2
    rel_data = bytearray()
    for site, kind, rel_name in relocs:
        if rel_name != owner_name:
            raise AssertionError("fixture relocation owner mismatch")
        index = symbol_index
        info = (index << 8) | kind
        rel_data.extend(struct.pack(">II", site, info))
    rel_idx = len(sections)
    sections.append({"name": ".rel.text", "type": SHT_REL, "flags": 0,
                     "data": bytes(rel_data), "link": 0, "info": text_idx,
                     "align": 4, "entsize": 8})
    sym_idx = len(sections)
    sections.append({"name": ".symtab", "type": SHT_SYMTAB, "flags": 0,
                     "data": symtab, "link": 0, "info": 1,
                     "align": 4, "entsize": 16})
    str_idx = len(sections)
    sections.append({"name": ".strtab", "type": SHT_STRTAB, "flags": 0,
                     "data": bytes(strtab), "link": 0, "info": 0,
                     "align": 1, "entsize": 0})
    sections[rel_idx]["link"] = sym_idx
    sections[sym_idx]["link"] = str_idx
    shstr = bytearray(b"\0")
    shname_offsets = {"": 0}
    for section in sections[1:]:
        name = section["name"]
        if name not in shname_offsets:
            shname_offsets[name] = len(shstr)
            shstr.extend(name.encode() + b"\0")
    shstr_idx = len(sections)
    sections.append({"name": ".shstrtab", "type": SHT_STRTAB, "flags": 0,
                     "data": bytes(shstr), "link": 0, "info": 0,
                     "align": 1, "entsize": 0})

    output = bytearray(b"\0" * 52)
    for section in sections[1:]:
        alignment = max(1, section["align"])
        output.extend(b"\0" * (_align(len(output), alignment) - len(output)))
        section["offset"] = len(output)
        output.extend(section["data"])
    shoff = _align(len(output), 4)
    output.extend(b"\0" * (shoff - len(output)))
    for section in sections:
        output.extend(struct.pack(
            ">10I", shname_offsets.get(section["name"], 0), section["type"],
            section["flags"], 0, section.get("offset", 0), len(section["data"]),
            section["link"], section["info"], section["align"], section["entsize"]))
    ident = b"\x7fELF\x01\x02\x01\0" + b"\0" * 8
    output[:16] = ident
    struct.pack_into(">HHIIIIIHHHHHH", output, 16,
                     1, 8, 1, 0, 0, shoff, 0,
                     52, 0, 0, 40, len(sections), shstr_idx)
    path.write_bytes(output)


class SourceSymbolFidelityTests(unittest.TestCase):
    def test_moved_raw_full_lo_site_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory),
                words=[0x3C080000, 0x25080000, 0x25080000],
                raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                full_relocs=[(0, 5, "owner"), (8, 6, "owner")],
                owner_type=STT_OBJECT)
            with self.assertRaisesRegex(sf.SourceFidelityError, "site/type"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_lo_site_survives_high_pairing(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory),
                words=[0x3C080000, 0x25080000],
                raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                owner_type=STT_OBJECT)
            report = sf.compare_source_symbols(raw, full, "source_fn")
            self.assertEqual([(r['site'], r['type']) for r in report['relocations']],
                             [('+0x0', 5), ('+0x4', 6)])

    def test_rela_and_missing_function_payload_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            for missing in (False, True):
                raw, full = self.make_pair(Path(directory), words=[0x0C000000],
                    raw_relocs=[(0, 4, "owner")])
                elf = sf.reloc_surface.Elf(full)
                data = bytearray(full.read_bytes())
                shoff = struct.unpack_from('>I', data, 0x20)[0]
                index, _ = elf.section('.text' if missing else '.rel.text')
                struct.pack_into('>I', data, shoff + index * 40 + (16 if missing else 4),
                                 len(data) + 4 if missing else 4)
                full.write_bytes(data)
                with self.subTest(missing=missing), self.assertRaises(sf.SourceFidelityError):
                    sf.compare_source_symbols(raw, full, 'source_fn')

    def test_duplicate_owned_section_name_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            raw, full = self.make_pair(path, words=[0x0C000000],
                raw_relocs=[(0, 4, "owner")],
                full_extra=[(".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR,
                             struct.pack(">I", 0x24020002))])
            with self.assertRaisesRegex(sf.SourceFidelityError, "ambiguous"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_lo16_on_non_immediate_instruction_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory), words=[0],
                raw_relocs=[(0, 6, "owner")], owner_type=STT_OBJECT)
            with self.assertRaisesRegex(sf.SourceFidelityError, "signed immediate"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_nobits_function_storage_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory), words=[0x0C000000],
                raw_relocs=[(0, 4, "owner")])
            elf = sf.reloc_surface.Elf(full)
            data = bytearray(full.read_bytes())
            shoff = struct.unpack_from(">I", data, 0x20)[0]
            index, _ = elf.section(".text")
            struct.pack_into(">I", data, shoff + index * 40 + 4, sf.SHT_NOBITS)
            full.write_bytes(data)
            with self.assertRaisesRegex(sf.SourceFidelityError, "storage"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def make_pair(self, directory: Path, *, words: list[int],
                  raw_relocs: list[tuple[int, int, str]],
                  full_relocs: list[tuple[int, int, str]] | None = None,
                  owner_name: str = "owner", owner_type: int = STT_FUNC,
                  raw_state: str = "undefined", full_state: str = "defined",
                  raw_value: int = 8, full_value: int = 8,
                  raw_size: int = 4, full_size: int = 4,
                  raw_binding: int = STB_GLOBAL,
                  full_binding: int = STB_GLOBAL,
                  raw_storage: str = ".data", full_storage: str = ".data",
                  raw_storage_flags: int = SHF_ALLOC | SHF_WRITE,
                  full_storage_flags: int = SHF_ALLOC | SHF_WRITE,
                  full_extra: list[tuple[str, int, int, bytes]] | None = None,
                  duplicate_full: bool = False):
        if owner_type == STT_FUNC and raw_storage == ".data":
            raw_storage = ".init"
        if owner_type == STT_FUNC and full_storage == ".data":
            full_storage = ".init"
            if full_storage_flags == SHF_ALLOC | SHF_WRITE:
                full_storage_flags = SHF_ALLOC | SHF_EXECINSTR
        raw, full = directory / "raw.o", directory / "full.o"
        make_elf(raw, words=words, relocs=raw_relocs,
                 owner_name=owner_name, owner_type=owner_type,
                 owner_state=raw_state, owner_value=raw_value,
                 owner_size=raw_size, owner_binding=raw_binding,
                 owner_storage=raw_storage, owner_section_flags=raw_storage_flags)
        make_elf(full, words=words,
                 relocs=full_relocs if full_relocs is not None else raw_relocs,
                 owner_name=owner_name, owner_type=owner_type,
                 owner_state=full_state, owner_value=full_value,
                 owner_size=full_size, owner_binding=full_binding,
                 owner_storage=full_storage, owner_section_flags=full_storage_flags,
                 extra_sections=full_extra, duplicate_owner=duplicate_full)
        return raw, full

    def run_pair(self, **kwargs):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory), **kwargs)
            return sf.compare_source_symbols(raw, full, "source_fn")

    def test_named_function_undefined_to_defined_and_address_masking(self):
        report = self.run_pair(words=[0x0C000000],
                               raw_relocs=[(0, 4, "owner")])
        self.assertEqual("source-correspondence-exact", report["status"])
        self.assertEqual(1, report["relocation_count"])
        self.assertFalse(report["runtime_identity_proved"])
        self.assertFalse(report["promotion_authority"])

    def test_named_object_addend_survives_section_index_and_value_change(self):
        words = [0x3C080000, 0x25080004]
        relocs = [(0, 5, "owner"), (4, 6, "owner")]
        report = self.run_pair(words=words, raw_relocs=relocs,
                               owner_type=STT_OBJECT, raw_state="defined",
                               full_state="defined", raw_value=8, full_value=24,
                               raw_size=4, full_size=4,
                               full_extra=[(".rodata", SHT_PROGBITS,
                                            SHF_ALLOC, bytes(8))])
        self.assertEqual("source-correspondence-exact", report["status"])
        self.assertEqual([4, 4], [r["owner_relative_addend"]
                                  for r in report["relocations"]])
        self.assertNotEqual(report["relocations"][0]["raw_owner"]["value"],
                            report["relocations"][0]["configured_owner"]["value"])

    def test_changed_hi_lo_addend_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            raw, full = self.make_pair(
                path, words=[0x3C080000, 0x25080004],
                raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                full_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                owner_type=STT_OBJECT, raw_state="defined", full_state="defined")
            # Change only the effective LO16 addend in the configured object.
            data = bytearray(full.read_bytes())
            elf = sf.reloc_surface.Elf(full)
            text_idx, text_sh = elf.section(".text")
            struct.pack_into(">I", data, text_sh[4] + 4, 0x25080008)
            full.write_bytes(data)
            with self.assertRaisesRegex(sf.SourceFidelityError, "addend differs"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_changed_r_mips_26_addend_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            raw, full = self.make_pair(path, words=[0x0C000000],
                                       raw_relocs=[(0, 4, "owner")])
            data = bytearray(full.read_bytes())
            elf = sf.reloc_surface.Elf(full)
            _, section = elf.section(".text")
            struct.pack_into(">I", data, section[4], 0x0C000004)
            full.write_bytes(data)
            with self.assertRaisesRegex(sf.SourceFidelityError, "addend differs"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_mismatched_owner_type_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            raw, full = self.make_pair(path, words=[0x0C000000],
                                       raw_relocs=[(0, 4, "owner")],
                                       owner_type=STT_FUNC)
            data = bytearray(full.read_bytes())
            elf = sf.reloc_surface.Elf(full)
            symbols = elf.symbols()
            # Rebuild by changing the owner type in the full symbol table.
            symtab_idx = sf._symtab_index(elf)
            sh = elf.sh[symtab_idx]
            rows = list(range(sh[4], sh[4] + sh[5], sh[9]))
            owner_row = rows[2]
            data[owner_row + 12] = (STB_GLOBAL << 4) | STT_OBJECT
            full.write_bytes(data)
            with self.assertRaises(sf.SourceFidelityError):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_mismatched_storage_is_rejected(self):
        with self.assertRaisesRegex(sf.SourceFidelityError, "storage differs"):
            self.run_pair(words=[0x3C080000, 0x25080004],
                          raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                          owner_type=STT_OBJECT, raw_state="defined",
                          full_state="defined", raw_storage=".data",
                          full_storage=".rodata",
                          full_storage_flags=SHF_ALLOC)
        with self.assertRaisesRegex(sf.SourceFidelityError, "storage differs"):
            self.run_pair(words=[0x3C080000, 0x25080004],
                          raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                          owner_type=STT_OBJECT, raw_state="defined",
                          full_state="defined", raw_storage=".data",
                          full_storage=".other_data",
                          full_storage_flags=SHF_ALLOC | SHF_WRITE)

    def test_owner_binding_and_defined_extent_mismatches_are_rejected(self):
        with self.assertRaisesRegex(sf.SourceFidelityError, "type/binding"):
            self.run_pair(words=[0x3C080000, 0x25080004],
                          raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                          owner_type=STT_OBJECT, raw_state="defined",
                          full_state="defined", full_binding=2)
        with self.assertRaisesRegex(sf.SourceFidelityError, "extent differs"):
            self.run_pair(words=[0x3C080000, 0x25080004],
                          raw_relocs=[(0, 5, "owner"), (4, 6, "owner")],
                          owner_type=STT_OBJECT, raw_state="defined",
                          full_state="defined", raw_size=4, full_size=8)

    def test_duplicate_owner_name_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory), words=[0x0C000000],
                                       raw_relocs=[(0, 4, "owner")],
                                       duplicate_full=True)
            with self.assertRaisesRegex(sf.SourceFidelityError, "duplicated"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_section_symbol_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory), words=[0x0C000000],
                                       raw_relocs=[(0, 4, "owner")])
            data = bytearray(full.read_bytes())
            elf = sf.reloc_surface.Elf(full)
            off = elf.sh[sf._symtab_index(elf)][4] + 2 * 16
            data[off + 12] = (STB_GLOBAL << 4) | STT_SECTION
            full.write_bytes(data)
            with self.assertRaisesRegex(sf.SourceFidelityError, "SECTION"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_pc16_local_branch_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            raw, full = self.make_pair(Path(directory), words=[0x10000000],
                                       raw_relocs=[(0, 10, "owner")])
            with self.assertRaisesRegex(sf.SourceFidelityError, "PC16"):
                sf.compare_source_symbols(raw, full, "source_fn")

    def test_ambiguous_hi_lo_pair_is_rejected(self):
        words = [0x3C080000, 0x3C090001, 0x25080000]
        relocs = [(0, 5, "owner"), (4, 5, "owner"), (8, 6, "owner")]
        with self.assertRaisesRegex(sf.SourceFidelityError, "ambiguous HI16/LO16"):
            self.run_pair(words=words, raw_relocs=relocs,
                          owner_type=STT_OBJECT, raw_state="defined",
                          full_state="defined")

    def test_unpaired_hi16_is_rejected(self):
        with self.assertRaisesRegex(sf.SourceFidelityError, "unpaired HI16"):
            self.run_pair(words=[0x3C080000],
                          raw_relocs=[(0, 5, "owner")],
                          owner_type=STT_OBJECT, raw_state="defined",
                          full_state="defined")

    def test_non_relocation_instruction_drift_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            raw, full = self.make_pair(path, words=[0x0C000000, 0x24020001],
                                       raw_relocs=[(0, 4, "owner")])
            data = bytearray(full.read_bytes())
            elf = sf.reloc_surface.Elf(full)
            _, section = elf.section(".text")
            struct.pack_into(">I", data, section[4] + 4, 0x24020002)
            full.write_bytes(data)
            with self.assertRaisesRegex(sf.SourceFidelityError,
                                        "normalized owned executable fields differ"):
                sf.compare_source_symbols(raw, full, "source_fn")


if __name__ == "__main__":
    unittest.main()
