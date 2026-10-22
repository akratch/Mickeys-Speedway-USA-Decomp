#!/usr/bin/env python3
"""tools/runlink generate on a synthetic GNU ld build: objects, a linked ELF, a map, a ROM.

The ROM's tables are written by hand from the format rules (runlink.synth),
so the generator is checked against the rules, not against its own encoder.
"""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from runlink import codec, compare, config, generate  # noqa: E402
from runlink.synth import (FUNC, GLOBAL, LOCAL, OBJECT, ElfBuilder, header, map_text,  # noqa: E402
                           ort_row, record, words)

R32, R26, HI16, LO16 = 2, 4, 5, 6
SYMBOL, LOCALOP, JUMP, DATA = 0, 1, 2, 3

CONFIG = r"""
name = "synth-ld"
rom = "rom.z64"
[tables]
main_table = 0x1000
main_table_block = 0x40
ort = 0x1040
ort_count = 4
headers = 0x1060
module_count = 2
modules = 0x10A0
[names]
offsets = 0x1200
blob = 0x1210
blob_end = 0x1240
[resident]
text = 0x80000450
data = 0x80001000
bss = 0x80002000
end = 0x80003000
trap_symbol = "TrapDanglingJump"
[build]
adapter = "ld"
elf = "link.elf"
map = "link.map"
module_section = ".overlay_{n}"
module_bss_section = ".overlay_{n}_bss"
table_inputs = ['_reloc\.bin\.o$']
resident_sections = [".main", ".main_bss"]
main_sentinel = "gSentinel"
[build.entry_names]
init = "_AutoInit{n:05d}"
resume = "_AutoExit{n:05d}"
"""

NAMES = ["resFunc", "_AutoExit00001", "gData", "gBss"]

# Module 1 as shipped: text 0x20, data 0x8, bss 0x10.
TEXT_IMG = words(0x0C000000,            # 0x00 jal resFunc          table 1 SYMBOL
                 0, 0x3C010000,          # 0x08 lui  %hi(.data)      LOCAL, data base
                 0x24210000,             # 0x0C addiu %lo(.data)
                 0x0C000000,             # 0x10 jal _AutoExit00001   table 2 SYMBOL (exported)
                 0x0C000006,             # 0x14 jal helper (0x18)    JUMP, stale index
                 0, 0x03E00008)          # 0x18 helper, 0x1C _AutoExit00001
DATA_IMG = words(0x18, 0x1234)           # 0x20 &helper              LOCAL, text base
T1 = [record(0, 0x00, R26, SYMBOL)]
T2 = [record(0x20, 0x08, HI16, LOCALOP), record(0x20, 0x0C, LO16, LOCALOP),
      record(1, 0x10, R26, SYMBOL), record(1, 0x14, R26, JUMP), record(0, 0x20, R32, LOCALOP)]


def fixed_text_object():
    """Module 1's code object; its HI16/LO16 pair names data.o's local mData."""
    o = ElfBuilder().section(".text", TEXT_IMG[:0x14] + words(0x0C000000) + TEXT_IMG[0x18:])
    o.section(".bss", nobits=0x10)
    o.sym("helper", 0x18, 4, GLOBAL, FUNC, ".text").sym("_AutoExit00001", 0x1C, 4, GLOBAL, FUNC, ".text")
    o.sym("resFunc").sym("mData")
    for r in [(0x00, R26, "resFunc"), (0x08, HI16, "mData"), (0x0C, LO16, "mData"),
              (0x10, R26, "_AutoExit00001"), (0x14, R26, "helper")]:
        o.rel(".text", *r)
    return o


def data_object():
    o = ElfBuilder().section(".data", words(0, 0x1234))
    o.sym("mData", 0, 8, LOCAL, OBJECT, ".data").sym("helper")
    o.rel(".data", 0, R32, "helper")
    return o.build()


def resident_object(callee):
    o = ElfBuilder().section(".text", words(0x03E00008, 0, 0, 0, 0x0C000000, 0, 0x3C010000, 0x24210000))
    o.section(".data", words(0))
    o.sym("resFunc", 0, 8, GLOBAL, FUNC, ".text").sym(callee).sym("gSentinel").sym("TrapDanglingJump")
    o.rel(".text", 0x10, R26, callee).rel(".text", 0x18, HI16, "gSentinel").rel(".text", 0x1C, LO16, "gSentinel")
    o.rel(".data", 0, R32, "TrapDanglingJump")
    return o.build()


def write_tree(root, text_obj):
    image = TEXT_IMG + DATA_IMG
    tables = b"".join(T1) + b"".join(T2)
    (root / "m1.o").write_bytes(text_obj)
    (root / "m1d.o").write_bytes(data_object())
    (root / "res.o").write_bytes(resident_object("TrapDanglingJump"))
    (root / "orig").mkdir(exist_ok=True)
    (root / "orig" / "res.o").write_bytes(resident_object("_AutoExit00001"))
    link = ElfBuilder()
    link.section(".main", b"\0" * 0x1000, 0x80000450).section(".main_bss", addr=0x80002000, nobits=0x100)
    link.section(".overlay_1", image + tables, 0x100000).section(".overlay_1_bss", addr=0x100100, nobits=0x10)
    for name, value, sec in (("resFunc", 0x80000450, ".main"), ("TrapDanglingJump", 0x80000500, ".main"),
                             ("gData", 0x80001000, ".main"), ("gBss", 0x80002010, ".main_bss"),
                             ("gSentinel", 0x80002020, ".main_bss"), ("helper", 0x100018, ".overlay_1"),
                             ("mData", 0x100020, ".overlay_1"),
                             ("_AutoExit00001", 0x10001C, ".overlay_1")):
        link.sym(name, value, 4, GLOBAL, FUNC, sec)
    (root / "link.elf").write_bytes(link.build())
    (root / "link.map").write_text(map_text([
        (".main", 0x80000450, 0x1000, [(".text", 0x80000450, 0x20, "res.o"), (".data", 0x80001000, 4, "res.o")]),
        (".main_bss", 0x80002000, 0x100, []),
        (".overlay_1", 0x100000, len(image) + len(tables),
         [(".text", 0x100000, 0x20, "m1.o"), (".data", 0x100020, 8, "m1d.o"),
          (".data", 0x100028, len(tables), "m1_reloc.bin.o")]),
        (".overlay_1_bss", 0x100100, 0x10, [(".bss", 0x100100, 0x10, "m1.o")]),
    ]))
    rom = bytearray(0x1300)
    rom[0:4] = b"\x80\x37\x12\x40"
    main = struct.pack(">I", 4) + record(1, 0x10, R26, SYMBOL) + record(2, 0x18, HI16, SYMBOL) \
        + record(2, 0x1C, LO16, SYMBOL) + record(1, 0x0, R32, DATA)
    rom[0x1000:0x1000 + len(main)] = main
    rom[0x1040:0x1050] = ort_row(0, 0) + ort_row(1, 0x1C) + ort_row(0xFFD, 0) + ort_row(0xFFF, 0x10)
    t1, t2 = b"".join(T1), b"".join(T2)
    size = len(image) + len(t1) + len(t2)
    rom[0x1060:0x1080] = header(0, 0x20, 8, 0x10, len(t1), len(t2), -1, 0x1C)
    rom[0x1080:0x10A0] = header(size, 0, 0, 0, 0, 0)
    rom[0x10A0:0x10A0 + size] = image + t1 + t2
    blob, offs = b"", []
    for n in NAMES:
        offs.append(len(blob))
        blob += n.encode() + b"\0"
    rom[0x1200:0x1210] = struct.pack(">4I", *offs)
    rom[0x1210:0x1210 + len(blob)] = blob
    (root / "rom.z64").write_bytes(bytes(rom))
    (root / "synth.toml").write_text(CONFIG)
    return bytes(rom)


class GenerateTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def run_gen(self, text_obj, mode, unpatched="res.o=orig/res.o"):
        rom = write_tree(self.root, text_obj)
        cfg = config.load(str(self.root / "synth.toml"), self.root)
        tables = codec.decode(rom, cfg)
        out = generate.run(cfg, rom, tables, mode, unpatched)
        return rom, cfg, tables, out

    def test_round_trip_exact(self):
        rom, cfg, tables, out = self.run_gen(fixed_text_object().build(), "hybrid")
        build, mods, main, orows, residuals, gt, stats, routes = out
        self.assertEqual([(r.category, r.offset) for r in residuals], [])
        proof = compare.region_proof(gt, tables, cfg, rom, residuals)
        self.assertTrue(proof["raw_equal"], proof)
        self.assertEqual(mods[0].header.resume, 0x1C)
        self.assertEqual(mods[0].header.bss, 0x10)
        self.assertEqual(stats["main entries named by the objects"], 1)

    def test_objects_mode_leaves_unnamed_main_rows(self):
        rom, cfg, tables, out = self.run_gen(fixed_text_object().build(), "objects")
        residuals = out[4]
        # The sentinel pair and the DATA word name no callee in the source.
        self.assertEqual(sorted((r.category, r.offset) for r in residuals),
                         [("main-row", 0x0), ("main-row", 0x18), ("main-row", 0x1C)])

    def test_order_difference_is_classified_and_patched(self):
        o = fixed_text_object()
        o.relocs[".text"] = [o.relocs[".text"][i] for i in (0, 3, 1, 2, 4)]
        rom, cfg, tables, out = self.run_gen(o.build(), "hybrid")
        residuals, gt = out[4], out[5]
        self.assertEqual(sorted(r.category for r in residuals), ["order"])
        proof = compare.region_proof(gt, tables, cfg, rom, residuals)
        self.assertFalse(proof["raw_equal"])
        self.assertTrue(proof["patched_equal"])

    def test_placeholder_needs_the_oracle(self):
        o = fixed_text_object()
        o.relocs[".text"][0] = (0x00, R26, "func_0")
        o.sym("func_0")
        rom, cfg, tables, out = self.run_gen(o.build(), "objects")
        self.assertIn(("missing", 0), [(r.category, r.offset) for r in out[4]])
        rom, cfg, tables, out = self.run_gen(o.build(), "hybrid")
        self.assertEqual(out[4], [])
        self.assertEqual(out[6]["placeholder relocations"], 1)

    def test_wrong_name_is_a_callee_difference(self):
        o = fixed_text_object()
        o.relocs[".text"][3] = (0x10, R26, "helper")        # names the wrong in-module function
        rom, cfg, tables, out = self.run_gen(o.build(), "hybrid")
        # The JUMP after it inherits the missing SYMBOL as its stale index.
        self.assertEqual(sorted((r.category, r.offset) for r in out[4]), [("callee", 0x10), ("jump-index", 0x14)])
        rom, cfg, tables, out = self.run_gen(o.build(), "oracle")
        self.assertEqual(out[4], [])


if __name__ == "__main__":
    unittest.main()
