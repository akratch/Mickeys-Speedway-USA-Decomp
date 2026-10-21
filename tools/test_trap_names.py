#!/usr/bin/env python3
"""tools/trap_names.py against synthetic ELF objects, maps and tables.

Every fixture is built here, in memory or in a temporary directory: no build,
no baserom, nothing read from the ROM.
"""

from __future__ import annotations

import contextlib
import io
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_tables as ot  # noqa: E402
import trap_names as tn  # noqa: E402

R_MIPS_32, R_MIPS_26, R_MIPS_HI16 = 2, 4, 5


def build_elf(symbols, relocations, *, text=b"\0" * 0x40, data=b"\0" * 8,
              extra_strings=(), sections=None):
    """A minimal big-endian ELF32 relocatable object.

    symbols:     (name, value, bind, type, section) with section a name or "UND"
    relocations: (section, offset, type, symbol name)
    sections:    optional extra (name, address, size) PROGBITS sections
    """
    names = [".text", ".data"] + [name for name, _a, _s in (sections or [])]
    payload = {".text": text, ".data": data}
    addresses = {}
    for name, address, size in sections or []:
        payload[name] = b"\0" * size
        addresses[name] = address
    rel_targets = sorted({section for section, *_ in relocations})
    order = [""] + names + [".rel" + s for s in rel_targets] + [".symtab", ".strtab", ".shstrtab"]
    index = {name: i for i, name in enumerate(order)}

    strtab = bytearray(b"\0")
    for extra in extra_strings:
        strtab += extra.encode() + b"\0"
    entries = [struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0)]
    symbol_index = {}
    locals_first = sorted(symbols, key=lambda row: row[2] != 0)
    for name, value, bind, kind, section in locals_first:
        offset = len(strtab)
        strtab += name.encode() + b"\0"
        shndx = 0 if section == "UND" else index[section]
        symbol_index.setdefault(name, len(entries))
        entries.append(struct.pack(">IIIBBH", offset, value, 0, (bind << 4) | kind, 0, shndx))
    first_global = 1 + sum(1 for row in symbols if row[2] == 0)

    rels = {}
    for section, offset, rtype, name in relocations:
        rels.setdefault(section, b"")
        rels[section] += struct.pack(">II", offset, (symbol_index[name] << 8) | rtype)

    shstr = bytearray(b"\0")
    name_offsets = {}
    for name in order[1:]:
        name_offsets[name] = len(shstr)
        shstr += name.encode() + b"\0"

    blob = bytearray(b"\0" * 52)
    headers = [(0,) * 10]
    for name in order[1:]:
        if name in payload:
            content, kind, flags, link, info, entsize = payload[name], 1, 6, 0, 0, 0
        elif name.startswith(".rel"):
            content, kind, flags = rels[name[4:]], 9, 0
            link, info, entsize = index[".symtab"], index[name[4:]], 8
        elif name == ".symtab":
            content, kind, flags = b"".join(entries), 2, 0
            link, info, entsize = index[".strtab"], first_global, 16
        elif name == ".strtab":
            content, kind, flags, link, info, entsize = bytes(strtab), 3, 0, 0, 0, 0
        else:
            content, kind, flags, link, info, entsize = bytes(shstr), 3, 0, 0, 0, 0
        while len(blob) % 4:
            blob.append(0)
        offset = len(blob)
        blob += content
        headers.append((name_offsets[name], kind, flags, addresses.get(name, 0), offset,
                        len(content), link, info, 4, entsize))
    while len(blob) % 4:
        blob.append(0)
    shoff = len(blob)
    for header in headers:
        blob += struct.pack(">10I", *header)
    struct.pack_into(">16sHHIIIIIHHHHHH", blob, 0,
                     b"\x7fELF\x01\x02\x01" + b"\0" * 9, 1, 8, 1, 0, 0, shoff, 0,
                     52, 0, 0, 40, len(headers), index[".shstrtab"])
    return bytes(blob)


def names_of(data):
    sections = tn._sections(data)
    return [s["name"] for s in tn._symbols(data, sections, tn._single_symtab(sections))]


def relocations_of(data):
    sections = tn._sections(data)
    symtab = tn._single_symtab(sections)
    symbols = tn._symbols(data, sections, symtab)
    return [(target, offset, rtype, symbols[index]["name"])
            for target, offset, rtype, index in tn._relocations(data, sections, symtab)]


CALLER = ("caller", 0, 1, 2, ".text")


class RenameTests(unittest.TestCase):
    def test_renames_listed_undefined_references_and_records_sites(self):
        obj = build_elf(
            [CALLER, ("overlay5Spawn", 0, 1, 0, "UND"),
             ("func_overlay_009_F0000010_1866688", 0, 1, 0, "UND"),
             ("TrapDanglingJump", 0, 1, 0, "UND"), ("mmAlloc", 0, 1, 0, "UND")],
            [(".text", 0x8, R_MIPS_26, "overlay5Spawn"),
             (".text", 0x10, R_MIPS_26, "mmAlloc"),
             (".text", 0x18, R_MIPS_26, "func_overlay_009_F0000010_1866688"),
             (".text", 0x20, R_MIPS_26, "overlay5Spawn"),
             (".text", 0x28, R_MIPS_26, "TrapDanglingJump")])
        before = relocations_of(obj)
        new, record = tn.rename_object(
            obj, {"overlay5Spawn", "func_overlay_009_F0000010_1866688", "overlay7Unused"})
        self.assertEqual(names_of(new).count("TrapDanglingJump"), 3)
        self.assertNotIn("overlay5Spawn", names_of(new))
        after = relocations_of(new)
        self.assertEqual([row[:3] for row in after], [row[:3] for row in before])
        self.assertEqual([row[3] for row in after],
                         ["TrapDanglingJump", "mmAlloc", "TrapDanglingJump",
                          "TrapDanglingJump", "TrapDanglingJump"])
        self.assertEqual(tn._section_bytes(new, tn._sections(new), ".text"), b"\0" * 0x40)
        self.assertEqual(record["sites"], [
            {"offset": 0x8, "name": "overlay5Spawn"},
            {"offset": 0x18, "name": "func_overlay_009_F0000010_1866688"},
            {"offset": 0x20, "name": "overlay5Spawn"}])
        self.assertEqual(record["literal"], [0x28])

    def test_appends_the_trap_name_when_the_string_table_lacks_it(self):
        obj = build_elf([CALLER, ("overlay5Spawn", 0, 1, 0, "UND")],
                        [(".text", 0x4, R_MIPS_26, "overlay5Spawn")])
        new, record = tn.rename_object(obj, {"overlay5Spawn"})
        self.assertGreater(len(new), len(obj))
        self.assertEqual(names_of(new), ["", "caller", "TrapDanglingJump"])
        self.assertEqual(record["sites"], [{"offset": 0x4, "name": "overlay5Spawn"}])

    def test_reuses_an_existing_trap_string(self):
        obj = build_elf([CALLER, ("overlay5Spawn", 0, 1, 0, "UND")],
                        [(".text", 0x4, R_MIPS_26, "overlay5Spawn")],
                        extra_strings=("TrapDanglingJump",))
        new, _record = tn.rename_object(obj, {"overlay5Spawn"})
        self.assertEqual(len(new), len(obj))
        self.assertIn("TrapDanglingJump", names_of(new))

    def test_object_without_listed_names_is_unchanged(self):
        obj = build_elf([CALLER, ("mmAlloc", 0, 1, 0, "UND")],
                        [(".text", 0x4, R_MIPS_26, "mmAlloc")])
        new, record = tn.rename_object(obj, {"overlay5Spawn"})
        self.assertEqual(new, obj)
        self.assertEqual((record["sites"], record["literal"]), ([], []))

    def test_refuses_a_listed_name_the_object_defines(self):
        obj = build_elf([CALLER, ("overlay5Spawn", 0x10, 1, 2, ".text")], [])
        with self.assertRaisesRegex(tn.TrapError, "defines listed overlay export"):
            tn.rename_object(obj, {"overlay5Spawn"})

    def test_refuses_an_unlisted_overlay_export_spelling(self):
        obj = build_elf([CALLER, ("overlay12Draw", 0, 1, 0, "UND"),
                         ("func_overlay_003_F0000040_1859000", 0, 1, 0, "UND")],
                        [(".text", 0x4, R_MIPS_26, "overlay12Draw"),
                         (".text", 0x8, R_MIPS_26, "func_overlay_003_F0000040_1859000")])
        with self.assertRaisesRegex(tn.TrapError, "func_overlay_003_F0000040_1859000, overlay12Draw"):
            tn.rename_object(obj, {"overlay5Spawn"})

    def test_ordinary_names_are_not_mistaken_for_overlay_exports(self):
        obj = build_elf([CALLER, ("overlayTable", 0, 1, 1, "UND"),
                         ("runlinkDownloadCode", 0, 1, 0, "UND")],
                        [(".text", 0x4, R_MIPS_26, "runlinkDownloadCode")])
        tn.rename_object(obj, set())

    def test_refuses_a_listed_name_used_by_anything_but_a_text_jal(self):
        for section, rtype in ((".text", R_MIPS_HI16), (".data", R_MIPS_32)):
            obj = build_elf([CALLER, ("overlay5Spawn", 0, 1, 0, "UND")],
                            [(section, 0x4, rtype, "overlay5Spawn")])
            with self.assertRaisesRegex(tn.TrapError, "only a .text jal call site can trap"):
                tn.rename_object(obj, {"overlay5Spawn"})

    def test_command_writes_sidecar_and_a_second_run_keeps_it(self):
        with tempfile.TemporaryDirectory() as scratch:
            scratch = Path(scratch)
            obj = scratch / "unit.c.o"
            obj.write_bytes(build_elf([CALLER, ("overlay5Spawn", 0, 1, 0, "UND")],
                                      [(".text", 0x4, R_MIPS_26, "overlay5Spawn")]))
            obj.chmod(0o644)
            listing = scratch / "list.txt"
            listing.write_text("# comment\noverlay5Spawn\n")
            self.assertEqual(tn.main(["rename", str(obj), str(listing)]), 0)
            self.assertEqual(obj.stat().st_mode & 0o777, 0o644)
            self.assertIn("TrapDanglingJump", names_of(obj.read_bytes()))
            sidecar = Path(str(obj) + tn.SIDECAR_SUFFIX)
            first = json.loads(sidecar.read_text())
            self.assertEqual(first["sites"], [{"offset": 4, "name": "overlay5Spawn"}])
            self.assertEqual(tn.main(["rename", str(obj), str(listing)]), 0)
            self.assertEqual(json.loads(sidecar.read_text()), first)

    def test_command_reports_refusals_and_leaves_the_object(self):
        with tempfile.TemporaryDirectory() as scratch:
            scratch = Path(scratch)
            obj = scratch / "unit.c.o"
            original = build_elf([CALLER, ("overlay9Other", 0, 1, 0, "UND")],
                                 [(".text", 0x4, R_MIPS_26, "overlay9Other")])
            obj.write_bytes(original)
            listing = scratch / "list.txt"
            listing.write_text("overlay5Spawn\n")
            stderr = io.StringIO()
            with contextlib.redirect_stderr(stderr):
                self.assertEqual(tn.main(["rename", str(obj), str(listing)]), 1)
            self.assertIn("does not name: overlay9Other", stderr.getvalue())
            self.assertEqual(obj.read_bytes(), original)
            self.assertFalse(Path(str(obj) + tn.SIDECAR_SUFFIX).exists())


class ListTests(unittest.TestCase):
    def test_load_names_skips_comments_and_refuses_junk(self):
        with tempfile.TemporaryDirectory() as scratch:
            path = Path(scratch) / "list.txt"
            path.write_text("# header\n\noverlay5Spawn  # trailing\nfunc_overlay_001_F0000000_1\n")
            self.assertEqual(tn.load_names(path), ["overlay5Spawn", "func_overlay_001_F0000000_1"])
            path.write_text("overlay5Spawn\noverlay5Spawn\n")
            with self.assertRaisesRegex(tn.TrapError, "duplicate"):
                tn.load_names(path)
            path.write_text("0x80001234\n")
            with self.assertRaisesRegex(tn.TrapError, "not a symbol name"):
                tn.load_names(path)

    def test_callee_name_prefers_one_unique_friendly_name(self):
        census = {"overlay5Spawn": 1, "func_overlay_005_F0000010_1": 1, "shared": 2,
                  "func_overlay_005_F0000020_2": 1, "a5": 1, "b5": 1}
        self.assertEqual(tn.callee_name(["func_overlay_005_F0000010_1", "overlay5Spawn"], census),
                         "overlay5Spawn")
        self.assertEqual(tn.callee_name(["func_overlay_005_F0000020_2", "shared"], census),
                         "func_overlay_005_F0000020_2")
        self.assertEqual(tn.callee_name(["func_overlay_005_F0000020_2", "a5", "b5"], census),
                         "func_overlay_005_F0000020_2")
        with self.assertRaisesRegex(tn.TrapError, "no unambiguous name"):
            tn.callee_name(["shared"], census)

    def test_render_list_is_sorted_unique_names_after_the_header(self):
        text = tn.render_list(["b", "a", "b"])
        self.assertTrue(text.startswith(tn.LIST_HEADER))
        self.assertEqual(text[len(tn.LIST_HEADER):], "a\nb\n")


class LinkedInputTests(unittest.TestCase):
    def test_overlay_functions_reads_overlay_sections_only(self):
        elf = build_elf(
            [("overlay5Spawn", 0xF0000010, 1, 2, ".overlay_005"),
             ("func_overlay_005_F0000010_1", 0xF0000010, 1, 2, ".overlay_005"),
             ("overlay_005_VRAM", 0xF0000000, 1, 0, ".overlay_005"),
             ("mmAlloc", 0x80001000, 1, 2, ".text")],
            [], sections=[(".overlay_005", 0xF0000000, 0x40)])
        functions, census = tn.overlay_functions(elf)
        self.assertEqual(sorted(functions[(5, 0x10)]),
                         ["func_overlay_005_F0000010_1", "overlay5Spawn"])
        self.assertNotIn((5, 0), functions)
        self.assertEqual(census["mmAlloc"], 1)

    def test_resident_text_bases_parses_linker_map_rows(self):
        text = (" .text          0x80004340     0x7a10 build/src/main/objects.c.o\n"
                " .text          0x00000000        0x0 build/asm/header.s.o\n"
                " .text          0xf0000000      0x7b0 build/src/overlays/o001/a.c.o\n"
                " .text          0x80015540      0x950 build/asm/main/trackasm.s.o\n")
        self.assertEqual(tn.resident_text_bases(text), {
            "build/src/main/objects.c.o": (0x80004340, 0x7A10),
            "build/asm/main/trackasm.s.o": (0x80015540, 0x950)})

    def test_table_sites_keeps_only_records_on_a_trap_jal(self):
        rom = bytearray(ot.ROM_TABLE_BASE + 4 * ot.ROM_TABLE_COUNT)
        struct.pack_into(">I", rom, ot.RELOC_TABLE_BASE, 2)
        for number, (ort, offset) in enumerate(((7, 0x100), (8, 0x200))):
            struct.pack_into(">II", rom, ot.RELOC_TABLE_BASE + 4 + 8 * number,
                             ort, (offset << 8) | 0x42)
        site = ot.RESIDENT_VRAM_BASE + 0x100 - ot.VRAM_ROM_DELTA
        struct.pack_into(">I", rom, site, ot.TRAP_DANGLING_JUMP_WORD)
        struct.pack_into(">I", rom, ot.ROM_TABLE_BASE + 4 * 7, (5 << 20) | 0x10)
        self.assertEqual(tn.table_sites(bytes(rom)), {
            ot.RESIDENT_VRAM_BASE + 0x100: {"record": 0, "ort": 7, "overlay": 5, "offset": 0x10}})


class ProveTests(unittest.TestCase):
    OBJ = "build/src/main/unit.c.o"

    def inputs(self, **changes):
        sites = {0x80001008: {"record": 0, "ort": 7, "overlay": 5, "offset": 0x10},
                 0x80001020: {"record": 1, "ort": 8, "overlay": 6, "offset": 0x0},
                 0x80002004: {"record": 2, "ort": 9, "overlay": 5, "offset": 0x10}}
        functions = {(5, 0x10): ["overlay5Spawn", "func_overlay_005_F0000010_1"],
                     (6, 0x0): ["func_overlay_006_F0000000_2"]}
        census = {name: 1 for names in functions.values() for name in names}
        bases = {self.OBJ: (0x80001000, 0x100), "build/asm/main/x.s.o": (0x80002000, 0x10)}
        record = {"format": 1, "text_sha256": "aa", "_current_text_sha256": "aa", "literal": [],
                  "sites": [{"offset": 0x8, "name": "overlay5Spawn"},
                            {"offset": 0x20, "name": "func_overlay_006_F0000000_2"}]}
        listed = ["func_overlay_006_F0000000_2", "overlay5Spawn"]
        inbound = {vram: (site["overlay"], site["offset"]) for vram, site in sites.items()}
        values = dict(sites=sites, functions=functions, census=census, bases=bases,
                      sidecars={self.OBJ: record}, listed=listed, inbound=inbound)
        values.update(changes)
        return values

    def prove(self, **changes):
        return tn.prove(**self.inputs(**changes))

    def test_consistent_inputs_pass_and_count_assembly_sites(self):
        errors, stats = self.prove()
        self.assertEqual(errors, [])
        self.assertEqual((stats["named"], stats["sites"], stats["assembly"], stats["names"]),
                         (2, 3, 1, 2))

    def test_a_c_callee_the_table_does_not_target_fails(self):
        values = self.inputs()
        values["sidecars"][self.OBJ]["sites"][1]["name"] = "overlay5Spawn"
        errors, _stats = tn.prove(**values)
        self.assertEqual(len(errors), 1)
        self.assertIn("C calls overlay5Spawn, but main-table record 1", errors[0])

    def test_a_literal_trap_call_in_c_fails(self):
        values = self.inputs()
        values["sidecars"][self.OBJ]["sites"].pop()
        values["sidecars"][self.OBJ]["literal"] = [0x20]
        errors, _stats = tn.prove(**values)
        self.assertEqual(len(errors), 1)
        self.assertIn("calls TrapDanglingJump literally", errors[0])

    def test_a_stale_or_missing_sidecar_fails(self):
        values = self.inputs()
        values["sidecars"][self.OBJ]["_current_text_sha256"] = "bb"
        errors, _stats = tn.prove(**values)
        self.assertTrue(any("stale" in error for error in errors))
        errors, _stats = self.prove(sidecars={})
        self.assertEqual(sum("not named by any fresh" in error for error in errors), 2)

    def test_a_named_site_the_table_does_not_trap_fails(self):
        values = self.inputs()
        values["sidecars"][self.OBJ]["sites"].append({"offset": 0x40, "name": "overlay5Spawn"})
        errors, _stats = tn.prove(**values)
        self.assertEqual(errors, [f"{self.OBJ}+0x40: C calls overlay5Spawn, but no shipped "
                                  "main-table record traps this site"])

    def test_a_stale_list_fails(self):
        errors, _stats = self.prove(listed=["overlay5Spawn", "overlay9Gone"])
        self.assertEqual(len(errors), 1)
        self.assertIn("missing func_overlay_006_F0000000_2", errors[0])
        self.assertIn("not a trap target overlay9Gone", errors[0])

    def test_atlas_disagreement_fails(self):
        values = self.inputs()
        values["inbound"][0x80001020] = (6, 0x4)
        errors, _stats = tn.prove(**values)
        self.assertEqual(errors, ["main-table record 1: atlas resident_inbound disagrees "
                                  "with the shipped ORT row"])


if __name__ == "__main__":
    unittest.main()
