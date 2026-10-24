#!/usr/bin/env python3
"""Synthetic module links distinguish exported calls from private jumps."""
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

import link_overlay_module as tool
import runlink_pack as rp
from runlink.synth import ElfBuilder, FUNC, GLOBAL, LOCAL, words

ROOT = Path(__file__).resolve().parent.parent
LD = ROOT / "tools/binutils/mips64-elf-ld"
OBJCOPY = ROOT / "tools/binutils/mips64-elf-objcopy"


def fixture(*, import_collision=False):
    # Three synthetic calls: external, same-module export, private helper.
    # The HI/LO pair addresses .rodata; two data words take local/exported
    # function addresses. This fixture contains no retail-derived bytes.
    obj = ElfBuilder().section(".text", words(
        0x0C000000, 0x0C000000, 0x0C000000, 0, 0, 0x3C010000, 0x24210000, 0))
    obj.section(".data", bytes(16)).section(".rodata", struct.pack(">f", 1.25) + bytes(12))
    obj.section(".bss", nobits=16)
    obj.sym("entry", 0, 12, GLOBAL, FUNC, ".text")
    obj.sym("exported", 12, 4, GLOBAL, FUNC, ".text")
    obj.sym("helper", 16, 4, LOCAL, FUNC, ".text")
    obj.sym("external")
    if import_collision:
        obj.sym(tool.IMPORT_PREFIX + "external", 16, 4, LOCAL, FUNC, ".text")
    for offset, name in ((0, "external"), (4, "exported"), (8, "helper")):
        obj.rel(".text", offset, rp.R_MIPS_26, name)
    obj.rel(".text", 20, rp.R_MIPS_HI16, ("section", ".rodata"))
    obj.rel(".text", 24, rp.R_MIPS_LO16, ("section", ".rodata"))
    obj.rel(".data", 0, rp.R_MIPS_32, "helper")
    obj.rel(".data", 4, rp.R_MIPS_32, "exported")
    data = bytearray(obj.build())
    shoff = struct.unpack_from(">I", data, 32)[0]
    for index, flags in ((2, 3), (3, 2)):
        struct.pack_into(">I", data, shoff + index * 40 + 8, flags)
    return bytes(data)


def local_fixture(*, provider=False, missing_first=False, addend=0, bad_type=False):
    obj = ElfBuilder().section(".data", bytes(8) if provider else words(addend, 0))
    if provider:
        obj.sym("first", 0, 0, GLOBAL, FUNC if bad_type else 1, ".data")
        obj.sym("second", 4, 0, GLOBAL, 1, ".data")
    else:
        obj.sym("first").sym("second")
        if not missing_first:
            obj.rel(".data", 0, rp.R_MIPS_32, "first")
        obj.rel(".data", 4, rp.R_MIPS_32, "second")
    data = bytearray(obj.build())
    elf = rp.Elf("synthetic", data)
    section = elf.section(".data")
    struct.pack_into(">I", data, struct.unpack_from(">I", data, 32)[0] + section["index"] * 40 + 8, 3)
    return bytes(data)


class BindingTests(unittest.TestCase):
    def test_unknown_external_is_refused_without_editing_input(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "raw.o"
            path.write_bytes(fixture())
            with self.assertRaisesRegex(tool.LinkError, "no runtime export: external"):
                tool.binding_plan(rp.Elf(path), rp.OrtNames(["exported"], {}))
            self.assertEqual(fixture(), path.read_bytes())

    @unittest.skipUnless(LD.is_file() and OBJCOPY.is_file(), "needs MIPS binutils")
    def test_export_bindings_preserve_private_targets_and_section_bases(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            raw, output = root / "raw.o", root / "configured.o"
            raw.write_bytes(fixture())
            exports = rp.OrtNames(["exported", "external"], {})
            receipt = tool.link(raw, output, exports, root / "module", ld=LD,
                                objcopy=OBJCOPY, rebind=ROOT / "tools/rebind_elf_relocations.py")
            result = rp.Elf(output)
            text, data = result.bytes_of(".text"), result.bytes_of(".data")
            self.assertEqual([0, 0, 4], [rp.u32(text, offset) & 0x3FFFFFF for offset in (0, 4, 8)])
            self.assertEqual((16, 0), (rp.u32(data, 0), rp.u32(data, 4)))
            self.assertEqual((0, 0), (rp.u32(text, 20) & 0xFFFF, rp.u32(text, 24) & 0xFFFF))
            self.assertEqual(16, result.size(".data"))
            self.assertEqual(16, result.size(".rodata"))
            self.assertEqual(16, result.size(".bss"))
            self.assertEqual(1, struct.unpack_from(">H", result.data, 16)[0])
            self.assertFalse(any(result.rels(section) for section in tool.SECTIONS[:-1]))
            self.assertEqual({"external": 1}, receipt["external_exports"])
            self.assertEqual(2, len(receipt["local_exports"]))
            localized = (root / "module/localized.elf").read_bytes()
            permitted = set(range(16, 18)) | set(range(24, 32)) | set(range(42, 46))
            self.assertEqual(len(localized), len(result.data))
            self.assertTrue(all(i in permitted for i, pair in enumerate(zip(localized, result.data))
                                if pair[0] != pair[1]))
            self.assertEqual(fixture(), raw.read_bytes())
            self.assertTrue(any(s.name == "exported" and s.value == 12 and s.bind == GLOBAL
                                for s in result.symbols))
            self.assertFalse(any(s.name == "external" for s in result.symbols))
            self.assertTrue(any(s.name == tool.IMPORT_PREFIX + "external"
                                and s.bind == LOCAL and s.shndx == rp.SHN_ABS and s.value == 0
                                for s in result.symbols))
            ordinary = rp.Elf(root / "module/module.elf")
            self.assertTrue(any(s.name == "external" and s.bind == GLOBAL
                                for s in ordinary.symbols))

    @unittest.skipUnless(LD.is_file() and OBJCOPY.is_file(), "needs MIPS binutils")
    @unittest.skipUnless(LD.is_file() and OBJCOPY.is_file(), "needs MIPS binutils")
    def test_local_provider_named_bases_link_normally_and_remain_private_bindings(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            raw, provider, output = root / "raw.o", root / "provider.o", root / "configured.o"
            raw.write_bytes(local_fixture())
            provider.write_bytes(local_fixture(provider=True))
            receipt = tool.link(raw, output, rp.OrtNames([], {}), root / "module", ld=LD,
                objcopy=OBJCOPY, rebind=ROOT / "tools/rebind_elf_relocations.py",
                local_ro_origin=provider)
            result = rp.Elf(output)
            self.assertEqual((0, 4), struct.unpack(">II", result.bytes_of(".data")))
            self.assertEqual({"first": 0, "second": 4}, receipt["local_data_provider"]["bindings"])
            self.assertEqual(local_fixture(), raw.read_bytes())
            self.assertEqual(local_fixture(provider=True), provider.read_bytes())
            self.assertFalse(any(s.name in ("first", "second") for s in result.symbols))
            self.assertTrue(any(s.name == tool.IMPORT_PREFIX + "second" and s.value == 4
                                and s.bind == LOCAL for s in result.symbols))

    def test_local_provider_refuses_missing_zero_pointer_addend_and_function_base(self):
        exports = rp.OrtNames([], {})
        provider = rp.Elf("provider", local_fixture(provider=True))
        good = rp.Elf("raw", local_fixture())
        for raw in (local_fixture(missing_first=True), local_fixture(addend=1)):
            with self.assertRaises(tool.LinkError):
                tool.local_data_bindings(rp.Elf("bad", raw), provider, exports)
        with self.assertRaisesRegex(tool.LinkError, "provider base"):
            tool.local_data_bindings(good, rp.Elf("bad", local_fixture(provider=True, bad_type=True)), exports)

    @unittest.skipUnless(LD.is_file() and OBJCOPY.is_file(), "needs MIPS binutils")
    def test_changed_loaded_local_provider_refuses_before_configured_publication(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            raw, provider, output = root / "raw.o", root / "provider.o", root / "configured.o"
            raw.write_bytes(local_fixture())
            provider.write_bytes(local_fixture(provider=True))
            ordinary_run = tool.run
            def mutate_after_link(argv, commands=None):
                result = ordinary_run(argv, commands)
                if str(argv[0]) == str(LD):
                    data = bytearray(provider.read_bytes())
                    data[rp.Elf(provider).section(".data")["offset"]] ^= 1
                    provider.write_bytes(data)
                return result
            with mock.patch.object(tool, "run", side_effect=mutate_after_link), \
                    self.assertRaisesRegex(tool.LinkError, "provider changed"):
                tool.link(raw, output, rp.OrtNames([], {}), root / "module", ld=LD,
                    objcopy=OBJCOPY, rebind=ROOT / "tools/rebind_elf_relocations.py",
                    local_ro_origin=provider)
            self.assertFalse(output.exists())

    def test_local_provider_requires_the_ordinary_zero_based_raw_namespace(self):
        provider = bytearray(local_fixture(provider=True))
        section = rp.Elf("provider", provider).section(".data")
        shoff = struct.unpack_from(">I", provider, 32)[0]
        struct.pack_into(">I", provider, shoff + section["index"] * 40 + 12, 0x100)
        with self.assertRaisesRegex(tool.LinkError, "complete writable"):
            tool.local_data_bindings(rp.Elf("raw", local_fixture()),
                                    rp.Elf("bad", provider), rp.OrtNames([], {}))

    def test_import_namespace_collision_is_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            raw = root / "raw.o"
            original = fixture(import_collision=True)
            raw.write_bytes(original)
            with self.assertRaisesRegex(tool.LinkError, "import binding collides"):
                tool.link(raw, root / "configured.o", rp.OrtNames(["exported", "external"], {}),
                          root / "module", ld=LD, objcopy=OBJCOPY,
                          rebind=ROOT / "tools/rebind_elf_relocations.py")
            self.assertEqual(original, raw.read_bytes())


if __name__ == "__main__":
    unittest.main()
