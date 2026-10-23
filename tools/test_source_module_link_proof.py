#!/usr/bin/env python3
"""Source-derived module identities never borrow the target site's identity."""
from pathlib import Path
from types import SimpleNamespace
import tempfile
import unittest

import proof_provenance as pp
import runlink_pack as rp
import source_module_link_proof as proof
from test_link_overlay_module import fixture
from runlink.synth import ElfBuilder, GLOBAL, OBJECT, NOTYPE, words
import struct


def canonical(extra=()):
    def symbol(name, value, section=".main"):
        return SimpleNamespace(name=name, value=value, bind=rp.STB_GLOBAL,
                               shndx=1, section=section)
    return SimpleNamespace(symbols=[
        symbol("main_TEXT_START", 0x80000450), symbol("runlinkCodeEnd", 0x80001000),
        symbol("main_BSS_START", 0x80002000, ".main_bss"),
        symbol("main_BSS_END", 0x80003000, ".main_bss"),
        *[symbol(name, value, section) for name, value, section in extra],
    ])


class SourceIdentityTests(unittest.TestCase):
    def test_local_sections_and_exports_resolve_without_a_target_table(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "raw.o"
            path.write_bytes(fixture())
            raw = rp.Elf(path)
            actual = proof.source_identities(raw, raw,
                canonical([("external", 0x80000500, ".main")]),
                rp.OrtNames(["exported", "external"], {}), 1)
            self.assertEqual({(0, 4): (0, 0xB0), (4, 4): (1, 12), (8, 4): (1, 16),
                              (20, 5): (1, 48), (24, 6): (1, 48),
                              (32, 2): (1, 16), (36, 2): (1, 12)}, actual)

    def test_missing_or_shadowed_canonical_export_is_refused(self):
        cases = [[], [("external", 0, "ABS")],
                 [("external", 0, "ABS"), ("external", 0x80000500, ".main")]]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "raw.o"
            path.write_bytes(fixture())
            raw = rp.Elf(path)
            for symbols in cases:
                with self.subTest(symbols=symbols), self.assertRaises(pp.MetadataProofError):
                    proof.source_identities(raw, raw, canonical(symbols),
                        rp.OrtNames(["exported", "external"], {}), 1)


class CrossOverlayBssTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)

    def inputs(self, *, legacy=False, name="external", offset=4, size=0,
               flags=3, capacity=16, kind=8, symbol_type=OBJECT,
               symbol_section=".overlay_002_bss", symbol_bind=GLOBAL, duplicate=False,
               duplicate_section=False, missing=False, corrupt_body=False,
               body_addr=rp.OVERLAY_VMA, addend=0):
        body, tables = b"body" * 8, b"tables.." * 2
        self.rom = body + tables
        self.modules = {2: {"overlay": 2, "rom_start": 0, "rom_end": 48,
                           "text_size": 16, "data_size": 16,
                           "reloc1_size": 8, "reloc2_size": 8, "bss_size": 16}}
        data = self.rom if legacy else body
        if corrupt_body:
            data = b"!" + data[1:]
        physical_prefix = 48 if legacy else 32
        obj = ElfBuilder().section(".overlay_002", data, addr=body_addr)
        obj.section(".overlay_002_bss", addr=rp.OVERLAY_VMA + physical_prefix, nobits=capacity)
        if duplicate_section:
            obj.section(".overlay_002_bss", addr=rp.OVERLAY_VMA + physical_prefix, nobits=capacity)
        for marker, value in (("main_TEXT_START", 0x80000450), ("runlinkCodeEnd", 0x80001000),
                              ("main_BSS_START", 0x80002000), ("main_BSS_END", 0x80003000)):
            obj.sym(marker, value, 0, GLOBAL, NOTYPE, "ABS")
        if not missing:
            obj.sym(name, rp.OVERLAY_VMA + physical_prefix + offset, size,
                    symbol_bind, symbol_type, symbol_section)
        if duplicate:
            obj.sym(name, rp.OVERLAY_VMA + physical_prefix + offset, size,
                    symbol_bind, symbol_type, symbol_section)
        data = bytearray(obj.build())
        shoff = struct.unpack_from(">I", data, 32)[0]
        struct.pack_into(">I", data, shoff + 2 * 40 + 4, kind)
        struct.pack_into(">I", data, shoff + 2 * 40 + 8, flags)
        path = self.root / "canonical.elf"
        path.write_bytes(data)
        raw = ElfBuilder().section(".text", words(0x3C010000, 0x24210000 | (addend & 0xFFFF)))
        raw.sym(name, bind=GLOBAL, type=OBJECT)
        raw.rel(".text", 0, rp.R_MIPS_HI16, name)
        raw.rel(".text", 4, rp.R_MIPS_LO16, name)
        source = self.root / "raw.o"
        source.write_bytes(raw.build())
        return rp.Elf(source), rp.Elf(path), rp.OrtNames([name], {})

    def resolve(self, **kwargs):
        raw, linked, exports = self.inputs(**kwargs)
        return proof.source_identities(raw, raw, linked, exports, 1,
                                       rom=self.rom, runtime_modules=self.modules)

    def test_actual_bss_section_defines_identity_before_or_after_tables(self):
        for legacy in (False, True):
            with self.subTest(legacy=legacy):
                self.assertEqual({(0, 5): (2, 40), (4, 6): (2, 40)},
                                 self.resolve(legacy=legacy, addend=4))

    def test_encoded_name_never_supplies_the_overlay_or_offset(self):
        self.assertEqual({(0, 5): (2, 36), (4, 6): (2, 36)},
                         self.resolve(name="B_overlay_099_F1234567"))
        with self.assertRaises(pp.MetadataProofError):
            self.resolve(name="B_overlay_002_F0000024", offset=-4)

    def test_unfaithful_definitions_sections_and_payloads_are_refused(self):
        for kwargs in ({"missing": True}, {"duplicate": True}, {"duplicate_section": True},
                       {"symbol_section": "ABS"}, {"symbol_type": 2}, {"symbol_bind": 0},
                       {"offset": -1}, {"offset": 16}, {"size": 32},
                       {"flags": 0}, {"flags": 1}, {"flags": 2}, {"flags": 7},
                       {"kind": 1}, {"capacity": 0}, {"capacity": 8},
                       {"body_addr": rp.OVERLAY_VMA + 16}, {"corrupt_body": True},
                       {"addend": -8}, {"addend": 12}):
            with self.subTest(kwargs=kwargs), self.assertRaises(pp.MetadataProofError):
                self.resolve(**kwargs)

    def test_physical_capacity_cannot_wrap_the_32_bit_address_space(self):
        raw, canonical_elf, exports = self.inputs(capacity=0x20000000)
        self.modules[2]["bss_size"] = 0x20000000
        with self.assertRaises(pp.MetadataProofError):
            proof.source_identities(raw, raw, canonical_elf, exports, 1,
                                   rom=self.rom, runtime_modules=self.modules)

    def test_missing_or_inconsistent_runtime_geometry_is_refused(self):
        raw, canonical_elf, exports = self.inputs()
        with self.assertRaises(pp.MetadataProofError):
            proof.source_identities(raw, raw, canonical_elf, exports, 1)
        for field, value in (("bss_size", 0), ("rom_end", 47), ("rom_end", 64),
                             ("text_size", -1), ("overlay", 3)):
            raw, canonical_elf, exports = self.inputs()
            self.modules[2][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(pp.MetadataProofError):
                proof.source_identities(raw, raw, canonical_elf, exports, 1,
                                        rom=self.rom, runtime_modules=self.modules)


if __name__ == '__main__':
    unittest.main()
