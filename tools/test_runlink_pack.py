#!/usr/bin/env python3
"""tools/runlink_pack.py against a synthetic tree.

Every fixture is built here: big-endian ELF32 objects, a linked "ELF", a link
map, an atlas, and a ROM image whose tables were written by hand from the
format rules (not by the packer's own encoders). No build and no baserom.
"""

from __future__ import annotations

import collections
import contextlib
import hashlib
import io
import json
import struct
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_tables as ot  # noqa: E402
import runlink_pack as rp  # noqa: E402

R32, R26, HI16, LO16 = 2, 4, 5, 6
LOCAL, GLOBAL = 0, 1
NOTYPE, OBJECT, FUNC = 0, 1, 2


class ObjBuilder:
    """A minimal big-endian ELF32 file: sections, symbols, REL relocations."""

    def __init__(self):
        self.sections = []      # (name, type, addr, bytes)
        self.symbols = []       # (name, value, size, bind, type, section)
        self.relocs = collections.defaultdict(list)

    def section(self, name, data=b"", addr=0, nobits=None):
        if nobits is not None:
            self.sections.append((name, 8, addr, nobits))
        else:
            self.sections.append((name, 1, addr, bytes(data)))
        return self

    def sym(self, name, value=0, size=0, bind=GLOBAL, type=FUNC, section="UND"):
        self.symbols.append((name, value, size, bind, type, section))
        return self

    def rel(self, target, offset, rtype, symbol):
        """`symbol` is a symbol name or ("section", name)."""
        self.relocs[target].append((offset, rtype, symbol))
        return self

    def build(self):
        user = [s[0] for s in self.sections]
        rel_targets = [t for t in user if t in self.relocs]
        names = [""] + user + [".rel" + t for t in rel_targets] + [".symtab", ".strtab", ".shstrtab"]
        shstr = b"\0"
        name_off = {}
        for n in names[1:]:
            name_off[n] = len(shstr)
            shstr += n.encode() + b"\0"
        strtab = b"\0"
        str_off = {}

        def sname(n):
            nonlocal strtab
            if n not in str_off:
                str_off[n] = len(strtab)
                strtab += n.encode() + b"\0"
            return str_off[n]

        def shndx(sec):
            if sec == "UND":
                return 0
            if sec == "ABS":
                return 0xFFF1
            if sec == "COMMON":
                return 0xFFF2
            return 1 + user.index(sec)

        syms = [(0, 0, 0, 0, 0, 0)]
        sec_sym = {}
        for i, n in enumerate(user):
            sec_sym[n] = len(syms)
            syms.append((0, 0, 0, (LOCAL << 4) | 3, 0, 1 + i))
        index = {}
        ordered = [s for s in self.symbols if s[3] == LOCAL] + [s for s in self.symbols if s[3] != LOCAL]
        first_global = len(syms) + sum(1 for s in self.symbols if s[3] == LOCAL)
        for name, value, size, bind, typ, sec in ordered:
            index[name] = len(syms)
            syms.append((sname(name), value, size, (bind << 4) | typ, 0, shndx(sec)))
        symtab = b"".join(struct.pack(">IIIBBH", *s) for s in syms)
        rel_blobs = []
        for t in rel_targets:
            b = b""
            for off, rtype, symbol in self.relocs[t]:
                si = sec_sym[symbol[1]] if isinstance(symbol, tuple) else index[symbol]
                b += struct.pack(">II", off, (si << 8) | rtype)
            rel_blobs.append(b)
        out = bytearray(52)
        headers = [(0,) * 10]

        def place(blob):
            while len(out) % 4:
                out.append(0)
            off = len(out)
            out.extend(blob)
            return off

        for name, typ, addr, data in self.sections:
            if typ == 8:
                headers.append((name_off[name], 8, 3, addr, len(out), data, 0, 0, 16, 0))
            else:
                headers.append((name_off[name], 1, 6, addr, place(data), len(data), 0, 0, 16, 0))
        symtab_index = len(names) - 3
        for t, blob in zip(rel_targets, rel_blobs):
            headers.append((name_off[".rel" + t], 9, 0, 0, place(blob), len(blob),
                            symtab_index, 1 + user.index(t), 4, 8))
        headers.append((name_off[".symtab"], 2, 0, 0, place(symtab), len(symtab),
                        symtab_index + 1, first_global, 4, 16))
        headers.append((name_off[".strtab"], 3, 0, 0, place(strtab), len(strtab), 0, 0, 1, 0))
        headers.append((name_off[".shstrtab"], 3, 0, 0, place(shstr), len(shstr), 0, 0, 1, 0))
        while len(out) % 4:
            out.append(0)
        shoff = len(out)
        for h in headers:
            out.extend(struct.pack(">10I", *h))
        struct.pack_into(">4sBBBB8xHHIIIIIHHHHHH", out, 0, b"\x7fELF", 1, 2, 1, 0, 1, 8, 1, 0, 0,
                         shoff, 0, 52, 0, 0, 40, len(headers), len(headers) - 1)
        return bytes(out)


def words(*ws):
    return b"".join(struct.pack(">I", w & 0xFFFFFFFF) for w in ws)


def jal(field=0):
    return 0x0C000000 | field


def lui(hi):
    return 0x3C010000 | (hi & 0xFFFF)


def addiu(lo):
    return 0x24210000 | (lo & 0xFFFF)


def record(symidx, site, rtype, op):
    return struct.pack(">II", symidx, (site << 8) | (rtype << 4) | op)


TB, DB, BB, BE = 0x80000450, 0x80001000, 0x80002000, 0x80003000
RES = {"resFunc": 0x80000500, "resFunc2": 0x80000550, "resFunY": 0x80000580, "resData": 0x80001010,
       "TrapDanglingJump": 0x80000600, "resFirst": TB}
TS, DS, BSS = 0x40, 0x10, 0x20


class Tree:
    """One synthetic checkout: module 1 has two objects, everything else is empty.

    The shipped tables below are what the measured rules say Rare's converter
    writes for these objects; the test is that the packer derives them."""

    def __init__(self, root: Path):
        self.root = root
        self.write_objects()
        self.write_resident()
        self.write_atlas()
        self.write_rom()
        self.write_linked()

    def p(self, rel):
        path = self.root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        return path

    # -- overlay 1's raw objects

    def write_objects(self):
        a_text = words(jal(), lui(0), addiu(0x10), jal(), jal(), lui(0), addiu(4), jal())
        a = (ObjBuilder()
             .section(".text", a_text).section(".rodata", words(4, 8)).section(".bss", nobits=BSS)
             .sym("a_func", 0, 0x20, GLOBAL, FUNC, ".text")
             .rel(".text", 0x00, R26, "resFunc")
             .rel(".text", 0x04, HI16, ("section", ".bss"))
             .rel(".text", 0x08, LO16, ("section", ".bss"))
             .rel(".text", 0x0C, R26, "b_exported")
             .rel(".text", 0x10, R26, "b_static")
             .rel(".text", 0x14, HI16, "resData")
             .rel(".text", 0x18, LO16, "resData")
             .rel(".text", 0x1C, R26, "D_123")
             .rel(".rodata", 0x0, R32, ("section", ".text"))
             .rel(".rodata", 0x4, R32, ("section", ".text")))
        for n in ("resFunc", "b_exported", "b_static", "resData", "D_123"):
            a.sym(n)
        b = (ObjBuilder()
             .section(".text", words(jal(), *([0] * 7)))
             .sym("b_exported", 0, 0x10, GLOBAL, FUNC, ".text")
             .sym("b_static", 0x10, 0x10, GLOBAL, FUNC, ".text")
             .sym("a_func")
             .rel(".text", 0x00, R26, "a_func"))
        self.p("build/runlink_raw/src/overlays/o001/a.c.o").write_bytes(a.build())
        self.p("build/runlink_raw/src/overlays/o001/b.c.o").write_bytes(b.build())

    # -- resident objects the main table comes from

    def write_resident(self):
        r_text = words(0, 0, jal(), 0)
        r = (ObjBuilder().section(".text", r_text).sym("TrapDanglingJump")
             .rel(".text", 0x8, R26, "TrapDanglingJump"))
        r_bytes = r.build()
        self.p("build/src/main/r.c.o").write_bytes(r_bytes)
        sidecar = {"format": 1, "literal": [], "sites": [{"name": "b_exported", "offset": 8}],
                   "text_sha256": hashlib.sha256(r_text).hexdigest()}
        self.p("build/src/main/r.c.o.trapsites").write_text(json.dumps(sidecar))
        s = (ObjBuilder().section(".text", words(lui(0), addiu(0))).sym("D_800D2DC4")
             .rel(".text", 0, HI16, "D_800D2DC4").rel(".text", 4, LO16, "D_800D2DC4"))
        self.p("build/asm/main/s.s.o").write_bytes(s.build())
        d = (ObjBuilder().section(".data", words(0, 0)).sym("TrapDanglingJump")
             .rel(".data", 0, R32, "TrapDanglingJump"))
        self.p("build/asm/data/d.data.s.o").write_bytes(d.build())
        rt = (ObjBuilder().section(".text", words(lui(0), addiu(0), lui(0), addiu(0)))
              .sym("TrapDanglingJump").sym("D_800D2DC4")
              .rel(".text", 0, HI16, "TrapDanglingJump").rel(".text", 4, LO16, "TrapDanglingJump")
              .rel(".text", 8, HI16, "D_800D2DC4").rel(".text", 12, LO16, "D_800D2DC4"))
        self.p("build/src/main/runlink.c.o").write_bytes(rt.build())

    def write_atlas(self):
        mods = []
        for ov in range(1, ot.HEADER_COUNT + 1):
            m = {"overlay": ov, "rom": {"start": "0x%X" % (ot.MODULES_BASE + (0 if ov == 1 else 0xB0))},
                 "sections": {"text": {"size": "0x%X" % (TS if ov == 1 else 0)}},
                 "bss_size": "0x%X" % (BSS if ov == 1 else 0), "text_ownership": [],
                 "data_rodata_ownership": []}
            if ov == 1:
                m["text_ownership"] = [
                    {"offset": "0x0", "end_offset": "0x20", "size": "0x20", "type": "c",
                     "source": "overlays/o001/a"},
                    {"offset": "0x20", "end_offset": "0x40", "size": "0x20", "type": "c",
                     "source": "overlays/o001/b"}]
                m["data_rodata_ownership"] = [
                    {"offset": "0x0", "end_offset": "0x8", "size": "0x8", "type": "c",
                     "section": ".rodata", "source": "overlays/o001/a"}]
            mods.append(m)
        self.p("config/overlays.us.json").write_text(json.dumps({"modules": mods}))

    # -- the shipped ROM, by hand

    def write_rom(self):
        text = words(jal(), lui(0), addiu(0x18), jal(), jal(0x30 >> 2), lui(0), addiu(4), jal(),
                     jal(0), *([0] * 7))
        data = words(4, 8, 0, 0)
        self.image_prefix = text + data
        t1 = (record(0, 0x00, R26, 0) + record(1, 0x14, HI16, 0) + record(1, 0x18, LO16, 0)
              + record(3, 0x1C, R26, 0))
        t2 = (record(TS + DS, 0x04, HI16, 1) + record(TS + DS, 0x08, LO16, 1)
              + record(2, 0x0C, R26, 0) + record(2, 0x10, R26, 2) + record(2, 0x20, R26, 2)
              + record(0, 0x40, R32, 1) + record(0, 0x44, R32, 1) + record(2, 0x48, R32, 0))
        self.t1, self.t2 = t1, t2
        image = text + data + t1 + t2
        rom = bytearray(ot.MODULES_BASE + len(image) + 0x100)
        main = [(2, 0x80000708 - TB, (R26 << 4) | 0), (1, 0x80000800 - TB, (HI16 << 4) | 0),
                (1, 0x80000804 - TB, (LO16 << 4) | 0), (2, 0x20, (R32 << 4) | 3)]
        struct.pack_into(">I", rom, ot.RELOC_TABLE_BASE, len(main))
        for i, (row, off, flags) in enumerate(main):
            struct.pack_into(">II", rom, ot.RELOC_TABLE_BASE + 4 + 8 * i, row, (off << 8) | flags)
        ort = [(0, 0xB0), (0xFFD, 0x10), (1, 0x20), (0, 0x100)] + [(0, 0)] * (ot.ROM_TABLE_COUNT - 4)
        for i, (sel, off) in enumerate(ort):
            struct.pack_into(">I", rom, ot.ROM_TABLE_BASE + 4 * i, (sel << 20) | off)
        for ov in range(1, ot.HEADER_COUNT + 1):
            h = (0, 0, TS, DS, BSS, len(t1), len(t2), -1, 0x30) if ov == 1 else \
                (0, len(image), 0, 0, 0, 0, 0, -1, -1)
            struct.pack_into(">iiiiiHHii", rom, ot.HEADER_TABLE_BASE + 32 * (ov - 1), *h)
        rom[ot.MODULES_BASE:ot.MODULES_BASE + len(image)] = image
        self.rom = bytes(rom)
        self.p("baseroms/mickey.us.z64").write_bytes(self.rom)

    # -- the linked ELF and its map

    def write_linked(self):
        main = bytearray(BB - TB)
        e = ObjBuilder()
        e.section(".main", bytes(main), TB).section(".main_bss", addr=BB, nobits=BE - BB)
        e.section(".overlay_001", self.image_prefix + b"\0" * 0x60, rp.OVERLAY_VMA)
        e.sym("main_TEXT_START", TB, 0, GLOBAL, NOTYPE, ".main")
        e.sym("runlinkCodeEnd", DB, 0, GLOBAL, NOTYPE, ".main")
        e.sym("main_BSS_START", BB, 0, GLOBAL, NOTYPE, ".main_bss")
        e.sym("main_BSS_END", BE, 0, GLOBAL, NOTYPE, ".main_bss")
        for n, a in RES.items():
            e.sym(n, a, 0x10, GLOBAL, OBJECT if a >= DB else FUNC, ".main")
        e.sym("overlay_001_TEXT_SIZE", TS, 0, GLOBAL, NOTYPE, "ABS")
        for n, off in (("a_func", 0), ("b_exported", 0x20), ("b_static", 0x30)):
            e.sym(n, rp.OVERLAY_VMA + off, 0x10, GLOBAL, FUNC, ".overlay_001")
        e.sym("func_overlay_001_F0000020_184C400", rp.OVERLAY_VMA + 0x20, 0, GLOBAL, FUNC, ".overlay_001")
        self.p("build/mickey.us.elf").write_bytes(e.build())
        lines = ["Archive member included", "", "Linker script and memory map", "",
                 ".main           0x%08x     0x%x" % (TB, BB - TB),
                 " .text          0x%08x       0x10 build/src/main/r.c.o" % 0x80000700,
                 " .text          0x%08x        0x8 build/asm/main/s.s.o" % 0x80000800,
                 " .text          0x%08x       0x10 build/src/main/runlink.c.o" % 0x80000900,
                 " .data          0x%08x        0x8 build/asm/data/d.data.s.o" % (DB + 0x20),
                 "                0x%08x                main_TEXT_END = ." % 0x80000910,
                 "",
                 ".overlay_001    0xf0000000       0xb0 load address 0x0184c3e0",
                 " .text          0xf0000000       0x20 build/src/overlays/o001/a.c.o",
                 " .text          0xf0000020       0x20 build/src/overlays/o001/b.c.o",
                 " .data          0xf0000040       0x10 build/assets/overlays/o001/overlay_001_data_rodata.bin.o",
                 " .data          0xf0000050       0x20 build/assets/overlays/o001/overlay_001_reloc1.bin.o",
                 " .data          0xf0000070       0x40 build/assets/overlays/o001/overlay_001_reloc2.bin.o",
                 ""]
        self.p("build/mickey.us.map").write_text("\n".join(lines) + "\n")

    def args(self, *extra):
        return ["--root", str(self.root)] + list(extra)


def run(argv):
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        status = rp.main(argv)
    return status, out.getvalue() + err.getvalue()


class MetadataEmitterTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.tree = Tree(Path(self.tmp.name))
        self.assertEqual(run(self.tree.args("ort", "--write"))[0], 0)
        self.assertEqual(run(self.tree.args("compare", "--write-residuals"))[0], 0)
        self.out = self.tree.root / "build/runlink/metadata"

    def emit(self):
        return run(self.tree.args("emit-metadata"))

    def test_exact_bins_and_named_borrowing_are_reproducible(self):
        status, log = self.emit()
        self.assertEqual(status, 0, log)
        marker = self.out / "manifest.json"
        manifest_bytes = marker.read_bytes()
        manifest = json.loads(manifest_bytes)
        self.assertEqual(manifest["metadata_files"], 5)
        expected = {
            "main_reloc_table.bin": self.tree.rom[ot.RELOC_TABLE_BASE:ot.ROM_TABLE_BASE],
            "overlay_rom_table.bin": self.tree.rom[ot.ROM_TABLE_BASE:ot.HEADER_TABLE_BASE],
            "overlay_header_table.bin": self.tree.rom[ot.HEADER_TABLE_BASE:ot.MODULES_BASE],
            "overlays/o001/overlay_001_reloc1.bin": self.tree.t1,
            "overlays/o001/overlay_001_reloc2.bin": self.tree.t2,
        }
        self.assertEqual(set(manifest["files"]), set(expected))
        for name, data in expected.items():
            self.assertEqual((self.out / name).read_bytes(), data)
            self.assertEqual(manifest["files"][name], {
                "size": len(data), "sha256": hashlib.sha256(data).hexdigest()})
        ledger = (self.out / "borrowing.json").read_bytes()
        self.assertEqual(manifest["borrowing"]["sha256"], hashlib.sha256(ledger).hexdigest())
        borrowing = json.loads(ledger)["borrowed"]
        self.assertIn([1, "D_123", 1], borrowing["placeholder"]["entries"])
        self.assertEqual(borrowing["hybrid-data"]["modules"], {"1": ["0x48"]})
        self.assertFalse(manifest["proof"]["source_only"])
        self.assertFalse(manifest["proof"]["relayout"])
        for name in ("build/src/main/r.c.o.trapsites", "build/asm/data/d.data.s.o",
                     "build/runlink_raw/src/overlays/o001/a.c.o"):
            self.assertIn(name, manifest["inputs"])
        self.assertEqual(self.emit()[0], 0)
        self.assertEqual(marker.read_bytes(), manifest_bytes)

    def test_patched_equality_alone_never_emits(self):
        proof = {"raw_equal": False, "raw_differing_bytes": 4,
                 "patched_equal": True, "order_breaks": [], "region": [0, 1]}
        with mock.patch.object(rp, "region_proof", return_value=proof):
            status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertIn("exact unpatched region", log)
        self.assertFalse(self.out.exists())

    def test_listed_callee_difference_is_still_refused(self):
        obj = self.tree.root / "build/runlink_raw/src/overlays/o001/a.c.o"
        obj.write_bytes(obj.read_bytes().replace(b"resFunc\0", b"resFunY\0"))
        self.assertEqual(run(self.tree.args("compare", "--write-residuals"))[0], 0)
        status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertIn("zero differences", log)
        self.assertFalse(self.out.exists())

    def test_stale_borrowing_is_refused(self):
        path = self.tree.root / "config/runlink-residuals.us.json"
        ledger = json.loads(path.read_text())
        ledger["borrowed"].pop("placeholder")
        path.write_text(json.dumps(ledger))
        status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertIn("borrowing ledger is stale", log)
        self.assertFalse(self.out.exists())

    def test_input_change_during_packing_is_refused(self):
        pack = rp.pack_all
        def changed(world, ort):
            result = pack(world, ort)
            path = self.tree.root / "config/runlink-ort.us.txt"
            path.write_text(path.read_text() + "\n# changed during generation\n")
            return result
        with mock.patch.object(rp, "pack_all", side_effect=changed):
            status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertIn("inputs changed", log)
        self.assertFalse(self.out.exists())

    def test_missing_resident_input_is_refused(self):
        (self.tree.root / "build/asm/data/d.data.s.o").unlink()
        status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertIn("resident input is missing", log)
        self.assertFalse(self.out.exists())

    def test_failed_publication_leaves_no_completion_marker_and_can_retry(self):
        self.assertEqual(self.emit()[0], 0)
        replace = rp.os.replace
        def fail(source, target):
            if Path(target).name == "overlay_rom_table.bin":
                raise OSError("synthetic publication failure")
            replace(source, target)
        with mock.patch.object(rp.os, "replace", side_effect=fail):
            status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertFalse((self.out / "manifest.json").exists())
        self.assertEqual(self.emit()[0], 0)

    def test_unrelated_output_is_preserved(self):
        self.out.mkdir(parents=True)
        path = self.out / "keep.txt"
        path.write_text("unrelated")
        status, log = self.emit()
        self.assertEqual(status, 2, log)
        self.assertIn("unexpected file", log)
        self.assertEqual(path.read_text(), "unrelated")
        self.assertFalse((self.out / "manifest.json").exists())


class SyntheticTreeTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.tree = Tree(Path(self.tmp.name))

    def tearDown(self):
        self.tmp.cleanup()

    def pack(self):
        status, out = run(self.tree.args("ort", "--write"))
        self.assertEqual(status, 0, out)
        paths = rp.resolve_paths(type("A", (), {"root": str(self.tree.root), **{k: None for k in rp.DEFAULTS}})())
        world = rp.World(paths)
        ort = rp.OrtNames.load(paths["ort"])
        return world, rp.pack_all(world, ort)

    def test_ort_names_follow_symbols_and_number_duplicates(self):
        world, packed = self.pack()
        rows = rp.OrtNames.load(self.tree.root / "config/runlink-ort.us.txt").rows
        self.assertEqual(rows[:5], ["resFunc", "resData", "b_exported", "resFunc2", "resFirst"])
        self.assertEqual(rows[5], "func_80000450")
        self.assertEqual(rows[6], "func_80000450_ort6")
        status, out = run(self.tree.args("ort", "--check"))
        self.assertEqual(status, 0, out)

    def test_module_tables_reproduce_the_hand_written_ones(self):
        world, packed = self.pack()
        pm = packed.modules[1]
        self.assertEqual(pm.reloc_bytes(1), self.tree.t1)
        self.assertEqual(pm.reloc_bytes(2), self.tree.t2)
        prov = {r.site: r.provenance for r in pm.table1 + pm.table2}
        self.assertEqual(prov[0x1C], "placeholder")
        self.assertEqual(prov[0x48], "hybrid-data")
        self.assertEqual(prov[0x10], "object")
        self.assertEqual(packed.ort_words[:4], [0xB0, (0xFFD << 20) | 0x10, (1 << 20) | 0x20, 0x100])

    def test_borrowings_are_recorded(self):
        world, packed = self.pack()
        b = packed.borrowed
        self.assertEqual(b["placeholder"], [[1, "D_123", 1]])
        self.assertEqual(b["hybrid-data"], [[1, "0x48"]])
        self.assertEqual(b["placement"], [[1, "a", ".bss", "0x58", 2]])
        self.assertEqual(sorted(x[2] for x in b["main-placeholder"]), ["R_MIPS_32", "R_MIPS_HI16", "R_MIPS_LO16"])
        # Equal raw/atlas sums do not prove ownership: this fixture discards
        # the raw BSS and has no linked BSS input at all.
        self.assertEqual(b["bss-size"], [[1]])
        self.assertNotIn("rodata-base", b)

    def test_compare_is_clean_and_region_is_identical(self):
        world, packed = self.pack()
        self.assertEqual(rp.compare_all(packed, world), [])
        proof = rp.region_proof(packed, world, [])
        self.assertTrue(proof["raw_equal"], proof)
        status, out = run(self.tree.args("compare", "--write-residuals"))
        self.assertEqual(status, 0, out)
        status, out = run(self.tree.args("compare", "--check"))
        self.assertEqual(status, 0, out)
        status, out = run(self.tree.args("region"))
        self.assertEqual(status, 0, out)
        self.assertIn("identical", out)

    def test_main_table_excludes_the_runtime_and_names_trap_callees(self):
        world, packed = self.pack()
        got = [(e.row, e.offset, e.type, e.op, e.provenance) for e in packed.main]
        self.assertEqual(got, [(2, 0x2B8, R26, 0, "object"), (1, 0x3B0, HI16, 0, "placeholder"),
                               (1, 0x3B4, LO16, 0, "placeholder"), (2, 0x20, R32, 3, "placeholder")])

    def test_a_wrong_callee_is_classified_and_ratchets(self):
        status, out = run(self.tree.args("ort", "--write"))
        run(self.tree.args("compare", "--write-residuals"))
        # The source now calls resFunY where the shipped record targets resFunc.
        a = self.tree.root / "build/runlink_raw/src/overlays/o001/a.c.o"
        a.write_bytes(a.read_bytes().replace(b"resFunc\0", b"resFunY\0"))
        status, out = run(self.tree.args("compare", "--check", "--verbose"))
        self.assertEqual(status, 1, out)
        self.assertIn("NEW    diff callee 1 0x0", out)
        status, out = run(self.tree.args("region"))
        self.assertEqual(status, 0, out)
        status, out = run(self.tree.args("compare", "--write-residuals"))
        self.assertEqual(status, 0, out)
        status, out = run(self.tree.args("compare", "--check"))
        self.assertEqual(status, 0, out)
        man = json.loads((self.tree.root / "config/runlink-residuals.us.json").read_text())
        self.assertEqual(man["differences"]["callee"]["entries"][0][:3], [1, "0x0", "SYMBOL resFunc"])
        # Fixing it makes the manifest stale: the gate fails until it ratchets down.
        a.write_bytes(a.read_bytes().replace(b"resFunY\0", b"resFunc\0"))
        status, out = run(self.tree.args("compare", "--check"))
        self.assertEqual(status, 1, out)
        self.assertIn("GONE   diff callee 1 0x0", out)

    def test_order_differences_are_classified(self):
        world, packed = self.pack()
        pm = packed.modules[1]
        pm.table1[1], pm.table1[2] = pm.table1[2], pm.table1[1]
        residuals = rp.compare_all(packed, world)
        cats = collections.Counter(r.category for r in residuals)
        self.assertEqual(cats["order"], 1, cats)
        self.assertFalse(rp.region_proof(packed, world, [])["patched_equal"])
        self.assertTrue(rp.region_proof(packed, world, residuals)["patched_equal"])

    def test_missing_raw_objects_exit_2(self):
        (self.tree.root / "build/runlink_raw/src/overlays/o001/b.c.o").unlink()
        run(self.tree.args("ort", "--write"))
        status, out = run(self.tree.args("compare"))
        self.assertEqual(status, 2, out)
        self.assertIn("gmake runlink-raw", out)


class LinkedBssTests(unittest.TestCase):
    def fixture(self):
        raw = (ObjBuilder().section(".text", words(lui(0), addiu(0), *([0] * 14)))
               .section(".data", bytes(0x10))
               .section(".bss", nobits=0x20))
        obj = rp.ObjRef(1, Path("raw/a.c.o"), "overlays/o001/a", 0, 0x40, "c",
                        rp.Elf("raw/a.c.o", raw.build()))
        output = ObjBuilder().section(".overlay_001_bss", addr=rp.OVERLAY_VMA + 0x50,
                                      nobits=0x20)
        info = dict(text_size=0x40, data_size=0x10, prefix=0x50, bss_vma=rp.OVERLAY_VMA + 0x50,
                    linked_bss=0x20, atlas_bss=0x20, inputs=[
                        rp.MapInput(".overlay_001", ".text", rp.OVERLAY_VMA,
                                    0x40, "build/src/overlays/o001/a.c.o"),
                        rp.MapInput(".overlay_001", ".data", rp.OVERLAY_VMA + 0x40,
                                    0x10, "build/src/overlays/o001/a.c.o")], bss_inputs=[
                        rp.MapInput(".overlay_001_bss", ".bss", rp.OVERLAY_VMA + 0x50,
                                    0x20, "build/src/overlays/o001/a.c.o")])
        world = SimpleNamespace(module_info={1: info}, elf=rp.Elf("linked", output.build()),
            amods={1: {}}, objects=lambda ov: [obj],
            # Deliberately contradictory shipped-site evidence cannot supply
            # the map-proved source placement.
            expected_cache=lambda ov: {(0, HI16): {"full": ("ov", 1, 0x58)}})
        target = rp.Target("section", "section", "section", obj=obj, secname=".bss")
        relocs = {1: [rp.CReloc(obj, ".text", 0, HI16, None, 0, target)]}
        return world, obj, relocs

    def test_actual_input_map_places_complete_and_interior_source_bss(self):
        for interior in (False, True):
            with self.subTest(interior=interior):
                world, obj, relocs = self.fixture()
                expected = 0x60 if interior else 0x50
                if interior:
                    info = world.module_info[1]
                    info["linked_bss"] = info["atlas_bss"] = 0x30
                    world.elf.section(".overlay_001_bss")["size"] = 0x30
                    info["bss_inputs"] = [
                        rp.MapInput(".overlay_001_bss", ".bss", rp.OVERLAY_VMA + 0x50,
                                    0x10, "build/src/overlays/o001/b.c.o"),
                        rp.MapInput(".overlay_001_bss", ".bss", rp.OVERLAY_VMA + 0x60,
                                    0x20, "build/src/overlays/o001/a.c.o")]
                borrowed = rp.place_sections(world, None, relocs)
                self.assertEqual(obj.placement[".bss"], (expected, "link"))
                self.assertNotIn("placement", borrowed)

    def test_header_uses_complete_source_allocation_and_can_outgrow_atlas(self):
        for atlas_size in (0, 0x10, 0x20, 0x40):
            with self.subTest(atlas_size=atlas_size):
                world, obj, _relocs = self.fixture()
                world.module_info[1]["atlas_bss"] = atlas_size
                self.assertEqual(rp.header_bss(world, 1), (0x20, True))
                # Exercise the serialized-header producer as well as its helper.
                world.shipped = SimpleNamespace(modules={})
                world.module_image_prefix = lambda ov: bytes(0x50)
                resolver = SimpleNamespace(ort=SimpleNamespace(entries={}))
                packed = rp.pack_module(world, resolver, 1, [])
                self.assertEqual(packed.bss_size, 0x20)
                self.assertNotIn("bss-size", packed.borrowed)

    def test_header_requires_all_body_and_bss_contributors(self):
        cases = ("discarded", "trimmed", "duplicate", "overlap", "wrong-producer",
                 "gap", "trailing-reserve", "initialized", "executable", "wrong-boundary",
                 "wrong-extent", "blob-body", "trimmed-body", "body-gap", "missing-body",
                 "duplicate-object", "extra-raw-bss", "common", "other-nobits")
        for case in cases:
            with self.subTest(case=case):
                world, obj, _relocs = self.fixture()
                info = world.module_info[1]
                item = info["bss_inputs"][0]
                output = world.elf.section(".overlay_001_bss")
                if case == "discarded":
                    info["bss_inputs"] = []
                elif case == "trimmed":
                    info["bss_inputs"] = [rp.MapInput(item.outsec, item.insec, item.addr, 0x10, item.file)]
                elif case == "duplicate":
                    info["bss_inputs"] *= 2
                elif case == "overlap":
                    info["bss_inputs"].append(rp.MapInput(item.outsec, item.insec, item.addr,
                                                        item.size, "build/src/overlays/o001/b.c.o"))
                elif case == "wrong-producer":
                    info["bss_inputs"] = [rp.MapInput(item.outsec, item.insec, item.addr,
                                                     item.size, "build/asm/reserve.s.o")]
                elif case in ("gap", "trailing-reserve"):
                    output["size"] = info["linked_bss"] = 0x30
                    if case == "gap":
                        info["bss_inputs"] = [rp.MapInput(item.outsec, item.insec, item.addr + 0x10,
                                                         item.size, item.file)]
                elif case == "initialized":
                    output["type"] = 1
                elif case == "executable":
                    output["flags"] |= 4
                elif case == "wrong-boundary":
                    output["addr"] = info["bss_vma"] = rp.OVERLAY_VMA + 0xB0
                elif case == "wrong-extent":
                    info["linked_bss"] = 0x40
                elif case in ("blob-body", "trimmed-body", "body-gap", "missing-body"):
                    body = info["inputs"][1]
                    if case == "missing-body":
                        info["inputs"].pop()
                    else:
                        info["inputs"][1] = rp.MapInput(body.outsec, body.insec,
                            body.addr + (0x10 if case == "body-gap" else 0),
                            8 if case == "trimmed-body" else body.size,
                            "build/assets/retained.bin.o" if case == "blob-body" else body.file)
                elif case == "duplicate-object":
                    world.objects = lambda ov: [obj, obj]
                elif case == "extra-raw-bss":
                    extra = rp.ObjRef(1, Path("raw/b.c.o"), "overlays/o001/b", 0, 0, "c", obj.elf)
                    world.objects = lambda ov: [obj, extra]
                elif case == "common":
                    obj.elf = rp.Elf("common", (ObjBuilder().sym("reserve", 4, 8, GLOBAL,
                                                                  OBJECT, "COMMON")).build())
                elif case == "other-nobits":
                    obj.elf.sections.append(dict(name=".sbss", size=8, flags=3, type=8))
                self.assertEqual(rp.header_bss(world, 1), (0x20, False))

    def test_header_allows_only_actual_input_alignment_between_owners(self):
        world, obj, _relocs = self.fixture()
        extra = rp.ObjRef(1, Path("raw/b.c.o"), "overlays/o001/b", 0x40, 0, "c",
                         rp.Elf("raw/b.c.o", ObjBuilder().section(".bss", nobits=0x10).build()))
        extra.elf.section(".bss")["align"] = 0x20
        world.objects = lambda ov: [obj, extra]
        info = world.module_info[1]
        info["linked_bss"] = 0x40
        world.elf.section(".overlay_001_bss")["size"] = 0x40
        info["bss_inputs"].append(rp.MapInput(".overlay_001_bss", ".bss",
            rp.OVERLAY_VMA + 0x80, 0x10, "build/src/overlays/o001/b.c.o"))
        self.assertEqual(rp.header_bss(world, 1), (0x40, True))

    def test_zero_bss_has_no_reserve_or_unaccounted_producer(self):
        world, obj, _relocs = self.fixture()
        info = world.module_info[1]
        obj.elf.section(".bss")["size"] = 0
        info["atlas_bss"] = info["linked_bss"] = 0
        info["bss_inputs"] = []
        output = world.elf.section(".overlay_001_bss")
        output["size"], output["type"], output["flags"] = 0, 1, 1
        self.assertEqual(rp.header_bss(world, 1), (0, True))
        info["atlas_bss"] = 0x20
        self.assertEqual(rp.header_bss(world, 1), (0x20, False))
        info["atlas_bss"] = 0
        info["linked_bss"] = 0x10
        self.assertEqual(rp.header_bss(world, 1), (0, False))

    def test_unproved_legacy_bss_keeps_its_named_borrowing(self):
        cases = ("discarded", "trimmed", "duplicate", "overlap", "wrong-producer",
                 "tables-before-bss", "wrong-extent", "initialized", "executable")
        for case in cases:
            with self.subTest(case=case):
                world, obj, relocs = self.fixture()
                info = world.module_info[1]
                item = info["bss_inputs"][0]
                output = world.elf.section(".overlay_001_bss")
                if case == "discarded":
                    info["bss_inputs"] = []
                elif case == "trimmed":
                    info["bss_inputs"] = [rp.MapInput(item.outsec, item.insec, item.addr, 0x10, item.file)]
                elif case == "duplicate":
                    info["bss_inputs"] *= 2
                elif case == "overlap":
                    info["bss_inputs"].append(rp.MapInput(item.outsec, item.insec, item.addr,
                                                        item.size, "build/src/overlays/o001/b.c.o"))
                elif case == "wrong-producer":
                    info["bss_inputs"] = [rp.MapInput(item.outsec, item.insec, item.addr,
                                                     item.size, "build/src/overlays/o001/b.c.o")]
                elif case == "tables-before-bss":
                    output["addr"] = info["bss_vma"] = rp.OVERLAY_VMA + 0xB0
                elif case == "wrong-extent":
                    info["linked_bss"] = 0x40
                elif case == "initialized":
                    output["type"] = 1
                elif case == "executable":
                    output["flags"] |= 4
                borrowed = rp.place_sections(world, None, relocs)
                self.assertEqual(obj.placement[".bss"], (0x58, "borrowed"))
                self.assertEqual(borrowed["placement"], [[1, "a", ".bss", "0x58", 1]])


class DataOnlyImportGuardTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.paths = {"build": self.root / "build", "raw": self.root / "raw"}
        self.source = "overlays/o001/pointers"
        self.provider = "overlays/o001/values"
        for source in (self.source, self.provider):
            path = self.root / "src" / (source + ".c")
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("/* synthetic C input */\n")
        self.row = dict(type="c", canonical=True, section=".data", source=self.source,
                        offset="0x0", size="0x8", end_offset="0x8")
        provider_row = dict(type="c", canonical=True, section=".data", source=self.provider,
                            offset="0x8", size="0x8", end_offset="0x10")
        self.exports = rp.OrtNames(["first", "second"], {})
        self.provider_compile = mock.patch.object(rp, "reproduce_data_provider",
            side_effect=lambda world, source: rp.Elf(world.paths["raw"] / "src" / (source + ".c.o")))
        self.provider_mock = self.provider_compile.start()
        self.addCleanup(self.provider_compile.stop)
        self.provider_bytes = struct.pack(">ff", 1.0, 2.0)
        self.write(self.paths["raw"] / "src" / (self.provider + ".c.o"), self.provider_object())
        self.write(self.paths["build"] / "src" / (self.provider + ".c.o"), self.provider_object())
        configured = ObjBuilder().section(".data", bytes(8)).sym(
            "pointers", 0, 8, GLOBAL, OBJECT, ".data")
        self.write(self.paths["build"] / "src" / (self.source + ".c.o"), configured)
        linked = (ObjBuilder().section(".overlay_001", bytes(24) + self.provider_bytes,
                                      addr=rp.OVERLAY_VMA)
                  .sym("first", rp.OVERLAY_VMA + 24, 4, GLOBAL, OBJECT, ".overlay_001")
                  .sym("second", rp.OVERLAY_VMA + 28, 4, GLOBAL, OBJECT, ".overlay_001"))
        for name, value in (("main_TEXT_START", 0x80000400), ("runlinkCodeEnd", 0x80001000),
                            ("main_BSS_START", 0x80002000), ("main_BSS_END", 0x80003000)):
            linked.sym(name, value, 0, GLOBAL, NOTYPE, "ABS")
        self.linked = rp.Elf("linked", linked.build())
        items = [rp.MapInput(".overlay_001", ".data", rp.OVERLAY_VMA + 16, 8,
                             "build/src/" + self.source + ".c.o"),
                 rp.MapInput(".overlay_001", ".data", rp.OVERLAY_VMA + 24, 8,
                             "build/src/" + self.provider + ".c.o")]
        expected = {(16 + 4 * i, R32): dict(op=rp.OP_SYMBOL, symbol_index=i,
                    full=("ov", 1, 24 + 4 * i), table=2) for i in range(2)}
        self.world = SimpleNamespace(paths=self.paths, elf=self.linked, map=items,
            module_info={1: dict(text_size=16, inputs=items)},
            amods={1: dict(data_rodata_ownership=[self.row, provider_row])},
            by_name={s.name: [s] for s in self.linked.symbols if s.name},
            expected_cache=lambda _ov: expected)

    def provider_object(self):
        return (ObjBuilder().section(".data", self.provider_bytes)
                .sym("first", 0, 4, GLOBAL, OBJECT, ".data")
                .sym("second", 4, 4, GLOBAL, OBJECT, ".data"))

    def write(self, path, builder):
        data = bytearray(builder.build())
        elf = rp.Elf("fixture", data)
        shoff = struct.unpack_from(">I", data, 0x20)[0]
        for section in elf.sections:
            if section["name"] == ".data":
                struct.pack_into(">I", data, shoff + section["index"] * 40 + 8, 3)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return rp.Elf(path)

    def raw(self, swapped=False, local=False):
        builder = (ObjBuilder().section(".data", bytes(8))
                   .sym("first").sym("second"))
        names = ["second", "first"] if swapped else ["first", "second"]
        for i, name in enumerate(names):
            builder.rel(".data", i * 4, R32, ("section", ".data") if local else name)
        return self.write(self.paths["raw"] / "src" / (self.source + ".c.o"), builder)

    def test_complete_pointer_input_uses_qualified_raw_definitions(self):
        self.assertEqual(rp.validate_data_imports(
            self.world, self.exports, 1, self.row, self.source, self.raw()), 2)

    def test_authenticated_data_sources_and_providers_enter_metadata_dependencies(self):
        rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, self.raw())
        self.world.objects = lambda _ov: []
        self.world.paths.update({name: self.root / name for name in
                                 ("atlas", "ort", "residuals", "rom", "elf", "map")})
        inputs = rp.metadata_input_paths(self.world.paths, self.world)
        for source in (self.source, self.provider):
            self.assertIn(self.paths["raw"] / "src" / (source + ".c.o"), inputs)
            self.assertIn(self.root / "src" / (source + ".c"), inputs)

    def test_swapped_same_width_targets_are_refused_despite_equal_zero_payload(self):
        correct, swapped = self.raw(), self.raw(swapped=True)
        self.assertEqual(correct.bytes_of(".data"), swapped.bytes_of(".data"))
        with self.assertRaisesRegex(rp.PackError, "source/runtime identity mismatch"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, swapped)

    def test_missing_actual_provider_definition_is_refused(self):
        self.world.by_name.pop("first")
        with self.assertRaisesRegex((rp.PackError, RuntimeError), "canonical definition"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, self.raw())

    def test_stale_provider_raw_bytes_and_duplicate_map_input_are_refused(self):
        self.provider_bytes = struct.pack(">ff", 9.0, 2.0)
        self.write(self.paths["raw"] / "src" / (self.provider + ".c.o"), self.provider_object())
        with self.assertRaisesRegex(rp.PackError, "raw/configured initialized bytes differ"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, self.raw())
        self.world.map.append(self.world.map[0])
        with self.assertRaisesRegex(rp.PackError, "duplicate final source input"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, self.raw())

    def test_distinct_export_names_share_one_bound_provider_compile(self):
        self.assertEqual(rp.validate_data_imports(
            self.world, self.exports, 1, self.row, self.source, self.raw()), 2)
        self.provider_mock.assert_called_once_with(self.world, self.provider)

    def test_changed_provider_source_with_old_objects_is_freshly_compiled_and_refused(self):
        import proof_provenance as provenance
        self.provider_compile.stop()
        self.world.paths.update(elf=self.root / "linked.elf", map=self.root / "linked.map")
        source = self.root / "src" / (self.provider + ".c")
        source.write_text("float renamed = 1.0f, second = 2.0f;\n")
        fresh = self.write(self.root / "fresh-provider.o", ObjBuilder().section(
            ".data", self.provider_bytes).sym("renamed", 0, 4, GLOBAL, OBJECT, ".data").sym(
                "second", 4, 4, GLOBAL, OBJECT, ".data"))
        def capture(_root, current_source, *_arguments):
            self.assertEqual(current_source.read_text(), source.read_text())
            return fresh.path, [], {}, lambda: {}
        with mock.patch.object(provenance, "capture_data_provider_raw", side_effect=capture) as compiler:
            with self.assertRaisesRegex(rp.PackError, "stale ordinary raw"):
                rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, self.raw())
            compiler.assert_called_once()

    def test_fresh_compile_refuses_stale_raw_targets_with_identical_pointer_bytes(self):
        ordinary = self.raw()
        fresh = self.raw(swapped=True)
        self.assertEqual(ordinary.bytes_of(".data"), fresh.bytes_of(".data"))
        rp.validate_fresh_data_input(ordinary, ordinary)
        with self.assertRaisesRegex(rp.PackError, "stale ordinary raw"):
            rp.validate_fresh_data_input(ordinary, fresh)

    def test_assembly_fallback_is_not_a_genuine_data_c_input(self):
        (self.root / "src" / (self.source + ".c")).write_text(
            '#pragma GLOBAL_ASM("ignored.s")\n')
        with self.assertRaisesRegex(rp.PackError, "assembly fallback"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, self.raw())

    def test_same_input_local_case_is_not_silently_accepted(self):
        with self.assertRaisesRegex(rp.PackError, "LOCAL prefix lacks one complete sized owner"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source,
                                     self.raw(local=True))

    def local_prefix(self, *, missing_first=False, target_addend=16,
                     owner_size=32, target_section=False):
        self.world.data_import_inputs = {}  # each call constructs a fresh fixture
        payload = words(1, 2, 3, 0, 4, 5, 6, target_addend)
        self.row.update(size="0x20", end_offset="0x20")
        builder = ObjBuilder().section(".data", payload).sym(
            "frames", 0, owner_size, GLOBAL, OBJECT, ".data")
        if not missing_first:
            builder.rel(".data", 12, R32, ("section", ".data") if target_section else "frames")
        builder.rel(".data", 28, R32, "frames")
        raw = self.write(self.paths["raw"] / "src" / (self.source + ".c.o"), builder)
        self.write(self.paths["build"] / "src" / (self.source + ".c.o"),
                   ObjBuilder().section(".data", payload).sym(
                       "frames", 0, owner_size, GLOBAL, OBJECT, ".data"))
        linked = rp.Elf("linked", ObjBuilder().section(".overlay_001", bytes(16) + payload,
            addr=rp.OVERLAY_VMA).sym("frames", rp.OVERLAY_VMA + 16, owner_size,
                                    GLOBAL, OBJECT, ".overlay_001").build())
        item = rp.MapInput(".overlay_001", ".data", rp.OVERLAY_VMA + 16, 32,
                           "build/src/" + self.source + ".c.o")
        self.world.elf = linked
        self.world.by_name = {"frames": [s for s in linked.symbols if s.name == "frames"]}
        self.world.map = self.world.module_info[1]["inputs"] = [item]
        self.local_expected = {(16 + site, R32): dict(op=rp.OP_LOCAL, symbol_index=16,
            full=("ov", 1, 16 + addend), table=2, position=i)
            for i, (site, addend) in enumerate(((12, 0), (28, 16)))}
        self.world.expected_cache = lambda _ov: self.local_expected
        return raw

    def test_local_prefix_uses_real_owner_and_runtime_data_base(self):
        raw = self.local_prefix()
        self.assertEqual(rp.validate_data_imports(
            self.world, self.exports, 1, self.row, self.source, raw), 2)
        self.provider_mock.assert_not_called()

    def test_local_first_pointer_to_null_loses_identity_despite_identical_payload(self):
        correct = self.local_prefix()
        missing = self.local_prefix(missing_first=True)
        self.assertEqual(correct.bytes_of(".data"), missing.bytes_of(".data"))
        with self.assertRaisesRegex(rp.PackError, "missing or extra runtime records"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, missing)
        with self.assertRaisesRegex(rp.PackError, "stale ordinary raw"):
            rp.validate_fresh_data_input(correct, missing)

    def test_local_target_cannot_escape_or_use_unproved_section_interior(self):
        for arguments in ({"target_addend": 32}, {"target_section": True}):
            with self.subTest(arguments=arguments):
                raw = self.local_prefix(**arguments)
                with self.assertRaisesRegex(rp.PackError, "unsupported data-only LOCAL"):
                    rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, raw)

    def test_local_pointer_site_must_be_word_aligned(self):
        raw = self.local_prefix()
        rels = raw.rels(".data")
        with mock.patch.object(raw, "rels", side_effect=lambda section:
                [rp.Rel(13, R32, rels[0].sym), rels[1]] if section == ".data" else []):
            with self.assertRaisesRegex(rp.PackError, "unsupported data-only LOCAL"):
                rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, raw)

    def test_local_owner_must_cover_the_complete_input_and_be_unique_in_final(self):
        raw = self.local_prefix(owner_size=16)
        with self.assertRaisesRegex(rp.PackError, "one complete sized owner"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, raw)
        raw = self.local_prefix()
        self.world.by_name["frames"] *= 2
        with self.assertRaisesRegex(rp.PackError, "unique final owner"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, raw)

    def test_local_runtime_role_and_targets_are_independently_checked(self):
        for change in ({"symbol_index": 32}, {"op": rp.OP_SYMBOL},
                       {"table": 1}, {"full": ("ov", 1, 17)}):
            with self.subTest(change=change):
                raw = self.local_prefix()
                self.local_expected[(28, R32)].update(change)
                with self.assertRaisesRegex(rp.PackError, "LOCAL source/runtime identity mismatch"):
                    rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, raw)

    def test_local_runtime_record_order_is_not_discarded(self):
        raw = self.local_prefix()
        self.local_expected[(28, R32)]["position"] = 2
        with self.assertRaisesRegex(rp.PackError, "runtime order differs"):
            rp.validate_data_imports(self.world, self.exports, 1, self.row, self.source, raw)


class DataOnlyProducerDiscoveryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.tree = Tree(Path(self.temp.name))
        self.assertEqual(run(self.tree.args("ort", "--write"))[0], 0)
        self.paths = rp.resolve_paths(type("Args", (), {
            "root": str(self.tree.root), **{key: None for key in rp.DEFAULTS}})())
        # A configured backend makes lazy discovery read the actual ORT input.
        self.tree.p("Makefile").write_text("all:\n\t@:\n")
        self.world = rp.World(self.paths)
        self.exports = rp.OrtNames.load(self.paths["ort"])
        self.source = "overlays/o001/pointers"
        builder = (ObjBuilder().section(".data", bytes(8))
                   .sym("pointers", 0, 8, GLOBAL, OBJECT, ".data")
                   .sym("b_exported").rel(".data", 0, R32, "b_exported")
                   .rel(".data", 4, R32, "b_exported"))
        path = self.tree.p("build/runlink_raw/src/" + self.source + ".c.o")
        path.write_bytes(builder.build())
        self.obj = rp.ObjRef(1, path, self.source, 0, 0, "c",
            rp.Elf("fresh-authenticated-capture", builder.build()),
            {".data": (0x48, "link")}, {"b_exported": (1, 0x20)})

    def test_objects_before_pack_discover_only_published_inputs_with_current_exports(self):
        with mock.patch.object(rp, "_guard_data_module_imports", return_value={1: [self.obj]}) as guard:
            objects = self.world.objects(1)
            self.assertIn(self.obj, objects)
            self.assertEqual(guard.call_args.args[1].rows, self.exports.rows)
            self.assertIn((self.obj, next(s for s in self.obj.elf.symbols
                                         if s.name == "pointers")), self.world.defs()["pointers"])
            self.assertIn(self.obj.path, rp.metadata_input_paths(self.paths, self.world))
            self.assertNotEqual(self.obj.path, self.obj.elf.path)
            rp.pack_all(self.world, self.exports)
            self.assertEqual(guard.call_count, 2)
            self.assertIs(guard.call_args.args[1], self.exports)

    def test_failed_warm_revalidation_clears_previous_objects_and_definitions(self):
        failure = rp.PackError("changed source or provider")
        with mock.patch.object(rp, "_guard_data_module_imports",
                side_effect=[{1: [self.obj]}, failure, failure]):
            self.assertIn(self.obj, self.world.objects(1))
            self.world.defs()
            with self.assertRaisesRegex(rp.PackError, "changed source or provider"):
                rp.guard_data_module_imports(self.world, self.exports)
            self.assertIsNone(self.world._guarded_data_objects)
            self.assertIsNone(self.world._objects)
            self.assertIsNone(self.world._defs)
            with self.assertRaisesRegex(rp.PackError, "changed source or provider"):
                self.world.objects(1)

    def test_recursion_cannot_expose_a_half_built_inventory(self):
        with mock.patch.object(rp, "_guard_data_module_imports",
                side_effect=lambda world, _exports: world.objects(1)):
            with self.assertRaisesRegex(rp.PackError, "recursive data-only producer"):
                self.world.objects(1)
        self.assertFalse(self.world._data_guard_active)
        self.assertIsNone(self.world._guarded_data_objects)
        self.assertIsNone(self.world._objects)

    def test_missing_text_input_never_leaves_a_partial_inventory_on_retry(self):
        (self.paths["raw"] / "src/overlays/o001/a.c.o").unlink()
        with mock.patch.object(rp, "_guard_data_module_imports", return_value={1: [self.obj]}):
            for _attempt in range(2):
                with self.assertRaisesRegex(rp.PackError, "raw object.*missing"):
                    self.world.objects(1)
                self.assertIsNone(self.world._objects)
                self.assertIsNone(self.world._defs)

    def test_generated_looking_import_uses_provider_identity_and_never_spelling_fallback(self):
        name = "D_overlay_001_F0001234_00000000"
        builder = ObjBuilder().section(".data", bytes(4)).sym(name).rel(".data", 0, R32, name)
        self.obj.elf = rp.Elf("authenticated-capture", builder.build())
        self.obj.data_imports = {name: (1, 0x48)}
        resolver = mock.Mock()
        reference = rp.collect_relocs(self.world, resolver, self.obj)[0]
        self.assertEqual(reference.target.ident, ("ov", 1, 0x48))
        self.assertEqual(reference.target.route, "data-source-export")
        resolver.resolve_name.assert_not_called()
        self.obj.data_imports = {}
        with self.assertRaisesRegex(rp.PackError, "lacks its actual import identity"):
            rp.collect_relocs(self.world, resolver, self.obj)
        resolver.resolve_name.assert_not_called()

    def held_ort_resolver(self, *, missing=False, conflicting=False):
        name = "D_overlay_001_F0001234_00000000"
        builder = ObjBuilder().section(".data", bytes(4)).sym(name).rel(".data", 0, R32, name)
        self.obj.elf = rp.Elf("authenticated-capture", builder.build())
        self.obj.data_imports = {} if missing else {name: (1, 0x48)}
        self.exports = rp.OrtNames([name], {})
        self.paths["ort"].write_text(self.exports.render())
        objects = [self.obj]
        if conflicting:
            objects.append(rp.ObjRef(1, self.obj.path, "overlays/o001/other", 0, 0,
                "c", self.obj.elf, self.obj.placement, {name: (1, 0x4C)}))
        with mock.patch.object(rp, "_guard_data_module_imports", return_value={1: objects}):
            rp.guard_data_module_imports(self.world, self.exports)
        return name, rp.Resolver(self.world, self.exports)

    def test_ort_held_provider_bypasses_coordinate_spelling_without_reordering_name_resolution(self):
        name, resolver = self.held_ort_resolver()
        self.assertEqual(resolver.ort_identity(0), ("ov", 1, 0x48))
        self.assertEqual(resolver.rows_by_ident()[("ov", 1, 0x48)], [0])
        # General name resolution deliberately keeps its compatibility behavior.
        self.assertEqual(resolver.resolve_name(name).route, "generated-name")
        self.assertEqual(resolver.name_identity(name), ("ov", 1, 0x1234))

    def test_ort_missing_or_conflicting_held_provider_never_uses_spelling(self):
        for arguments, message in (({"missing": True}, "lacks its held provider"),
                                   ({"conflicting": True}, "ambiguous held provider")):
            with self.subTest(arguments=arguments):
                _name, resolver = self.held_ort_resolver(**arguments)
                with self.assertRaisesRegex(rp.PackError, message):
                    resolver.ort_identity(0)

    def test_revalidated_inventory_refreshes_ort_grouping_and_cached_definition_before_return(self):
        _name, resolver = self.held_ort_resolver()
        self.assertEqual(resolver.ort_identity(0), ("ov", 1, 0x48))
        resolver.rows_by_ident()
        old_target = resolver.resolve_name("b_exported", 1)
        self.obj.data_imports = {_name: (1, 0x4C)}
        with mock.patch.object(rp, "_guard_data_module_imports", return_value={1: [self.obj]}):
            rp.guard_data_module_imports(self.world, self.exports)
        # No ORT call may be needed to invalidate a stale Target/ObjRef cache.
        new_target = resolver.resolve_name("b_exported", 1)
        self.assertIsNot(new_target.obj, old_target.obj)
        self.assertEqual(resolver.ort_identity(0), ("ov", 1, 0x4C))
        self.assertNotIn(("ov", 1, 0x48), resolver.rows_by_ident())
        self.assertEqual(resolver.rows_by_ident()[("ov", 1, 0x4C)], [0])

    def test_failed_revalidation_refuses_all_previous_namespace_cache_paths(self):
        _name, resolver = self.held_ort_resolver()
        resolver.ort_identity(0)
        resolver.rows_by_ident()
        resolver.resolve_name("b_exported", 1)
        resolver.name_of(("ov", 1, 0x48))
        with mock.patch.object(rp, "_guard_data_module_imports", side_effect=rp.PackError("stale provider")):
            with self.assertRaisesRegex(rp.PackError, "stale provider"):
                rp.guard_data_module_imports(self.world, self.exports)
        for call in (lambda: resolver.ort_identity(0), resolver.rows_by_ident,
                     lambda: resolver.resolve_name("b_exported", 1),
                     lambda: resolver.name_of(("ov", 1, 0x48))):
            with self.assertRaisesRegex(rp.PackError, "ORT provider inventory is unavailable"):
                call()


class FunctionProxyAuthorityTests(unittest.TestCase):
    def provider(self, *, raw_type=FUNC, raw_size=4, raw_bind=GLOBAL, raw_value=4, raw_addr=0,
                 final_type=FUNC, final_size=4, final_value=None,
                 final_section=".overlay_001", map_size=8, map_section=".overlay_001"):
        self.name = "func_overlay_099_F0001234_00000000"
        self.proxy = self.name + "Reloc"
        raw = rp.Elf("raw/provider.o", ObjBuilder().section(".text", words(0, 0), addr=raw_addr).sym(
            self.name, raw_value, raw_size, raw_bind, raw_type, ".text").build())
        obj = rp.ObjRef(1, raw.path, "overlays/o001/provider", 0x20, 8, "c", raw)
        symbol = next(s for s in raw.symbols if s.name == self.name)
        final = rp.Elf("final", ObjBuilder().section(".overlay_001", bytes(0x30),
            addr=rp.OVERLAY_VMA).sym(self.name,
                rp.OVERLAY_VMA + 0x24 if final_value is None else final_value,
                final_size, GLOBAL, final_type, final_section).build())
        self.definitions = {self.name: [(obj, symbol)]}
        item = rp.MapInput(map_section, ".text", rp.OVERLAY_VMA + 0x20, map_size,
                           "build/src/overlays/o001/provider.c.o")
        self.world = SimpleNamespace(need_raw=True,
            layout=rp.ResidentLayout(TB, DB, BB, BE), defs=lambda: self.definitions,
            _guarded_data_objects={}, module_info={1: {"inputs": [item]}},
            by_name={self.name: [s for s in final.symbols if s.name == self.name]})
        return rp.Resolver(self.world, rp.OrtNames([], {}))

    def test_proxy_uses_actual_qualified_function_not_embedded_module_or_coordinates(self):
        resolver = self.provider()
        target = resolver.resolve_name(self.proxy)
        self.assertEqual(target.route, "source-function-proxy")
        self.assertEqual(resolver.target_base(target), ("ov", 1, 0x24))
        self.assertEqual(target.obj.source, "overlays/o001/provider")

    def test_present_invalid_function_never_falls_back_to_coordinates(self):
        for arguments in ({"raw_type": OBJECT}, {"raw_size": 0}, {"raw_bind": 2},
                          {"raw_size": 8},
                          {"raw_value": 5, "raw_size": 1, "final_size": 1,
                           "final_value": rp.OVERLAY_VMA + 0x25},
                          {"raw_size": 1, "final_size": 1}, {"raw_addr": 4}):
            with self.subTest(arguments=arguments):
                resolver = self.provider(**arguments)
                with self.assertRaisesRegex(rp.PackError, "bounded real function"):
                    resolver.resolve_name(self.proxy)

    def test_ambiguous_function_definitions_refuse(self):
        resolver = self.provider()
        self.definitions[self.name] *= 2
        with self.assertRaisesRegex(rp.PackError, "ambiguous function proxy"):
            resolver.resolve_name(self.proxy)

    def test_stale_or_wrong_module_final_geometry_refuses(self):
        for arguments in ({"final_size": 8}, {"final_value": rp.OVERLAY_VMA + 0x28},
                          {"final_type": OBJECT}, {"final_section": "ABS"},
                          {"map_size": 4}, {"map_section": ".overlay_002"}):
            with self.subTest(arguments=arguments):
                resolver = self.provider(**arguments)
                with self.assertRaisesRegex(rp.PackError, "raw/final input geometry"):
                    resolver.resolve_name(self.proxy)

    def test_duplicate_or_overlapping_input_ownership_refuses(self):
        resolver = self.provider()
        self.world.module_info[1]["inputs"] *= 2
        with self.assertRaisesRegex(rp.PackError, "raw/final input geometry"):
            resolver.resolve_name(self.proxy)
        resolver = self.provider()
        self.world.module_info[1]["inputs"].append(rp.MapInput(".overlay_001", ".text",
            rp.OVERLAY_VMA + 0x24, 4, "build/src/overlays/o001/other.c.o"))
        with self.assertRaisesRegex(rp.PackError, "overlapping final input"):
            resolver.resolve_name(self.proxy)

    def test_absent_definition_remains_visibly_compatibility_after_cache_refresh(self):
        resolver = self.provider()
        self.assertEqual(resolver.resolve_name(self.proxy).route, "source-function-proxy")
        self.definitions.clear()
        self.world._guarded_data_objects = {}  # successful catalogue revalidation
        target = resolver.resolve_name(self.proxy)
        self.assertEqual(target.route, "generated-proxy")
        self.assertEqual(resolver.target_base(target), ("ov", 99, 0x1234))


class HeldDataCaptureTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.world = SimpleNamespace(paths={"build": self.root / "build"})

    def receipt(self, dependency):
        return {"inputs": {"source": "source-hash", "configured": "object-hash",
            "dependencies": {"include/shared.h": dependency},
            "generated_metadata": {"inputs": {}}, "wrapper_source_directory": "early-tree"}}

    def test_later_capture_cannot_overwrite_a_conflicting_held_dependency(self):
        early = self.receipt("early-hash")
        rp.hold_data_capture(self.world, "overlays/o001/first", early, lambda: early["inputs"])
        later = self.receipt("later-hash")
        with self.assertRaisesRegex(rp.PackError, "conflicting held"):
            rp.hold_data_capture(self.world, "overlays/o001/second", later, lambda: later["inputs"])
        self.assertEqual(self.world.data_import_inputs[self.root / "include/shared.h"], "early-hash")

    def test_end_of_guard_rechecks_earlier_context_even_when_latest_context_is_valid(self):
        early, later = self.receipt("same-hash"), self.receipt("same-hash")
        rp.hold_data_capture(self.world, "overlays/o001/first", early, lambda: early["inputs"])
        rp.hold_data_capture(self.world, "overlays/o001/second", later, lambda: later["inputs"])
        early["inputs"]["wrapper_source_directory"] = "changed-after-validation"
        with self.assertRaisesRegex(rp.PackError, "held data-only capture context changed"):
            rp.check_data_import_holds(self.world)


class UnitTests(unittest.TestCase):
    def test_in_place_addends_pair_hi_with_the_next_lo_and_reuse_it(self):
        b = (ObjBuilder().section(".text", words(lui(1), addiu(0x8000), addiu(4), jal(3), 0x12345678))
             .sym("x").rel(".text", 0, HI16, "x").rel(".text", 4, LO16, "x").rel(".text", 8, LO16, "x")
             .rel(".text", 12, R26, "x").rel(".text", 16, R32, "x"))
        elf = rp.Elf("x.o", b.build())
        rels = elf.rels(".text")
        self.assertEqual(rp.in_place_addends(elf, ".text", rels),
                         [0x10000 - 0x8000, 0x10000 - 0x8000, 0x10004, 12, 0x12345678])

    def test_parse_map_reads_wrapped_input_lines(self):
        with tempfile.TemporaryDirectory() as t:
            p = Path(t) / "m.map"
            p.write_text("Linker script and memory map\n\n.main 0x80000450 0x100\n"
                         " .text.long_section_name\n                0x80000460       0x20 build/x.o\n"
                         " .text          0x80000480       0x10 build/y.o\n")
            got = rp.parse_map(p)
        self.assertEqual([(m.insec, m.addr, m.size, m.file) for m in got],
                         [(".text.long_section_name", 0x80000460, 0x20, "build/x.o"),
                          (".text", 0x80000480, 0x10, "build/y.o")])

    def test_generated_names_round_trip(self):
        layout = rp.ResidentLayout(TB, DB, BB, BE)
        info = {3: {"text_size": 0x100, "data_size": 0x20, "rom_start": 0x1000}}
        for ident in (("ov", 3, 0x40), ("ov", 3, 0x110), ("ov", 3, 0x200), ("res", 0x80000500), ("res", 0x80002010)):
            name = rp.generated_name(ident, layout, info)
            self.assertEqual(rp.parse_generated(name, layout), ident, name)
            self.assertEqual(rp.parse_generated(name + "_ort12", layout), ident)
        self.assertEqual(rp.generated_name(("ov", 3, 0x110), layout, info), "D_overlay_003_F0000110_1110")
        self.assertEqual(rp.generated_name(("ov", 3, 0x200), layout, info), "B_overlay_003_F0000200")

    def test_ort_list_round_trips_and_refuses_duplicates(self):
        o = rp.OrtNames(["a", "b"], {(1, "resume"): "b"})
        with tempfile.TemporaryDirectory() as t:
            p = Path(t) / "ort.txt"
            p.write_text(o.render())
            back = rp.OrtNames.load(p)
        self.assertEqual(back.rows, ["a", "b"])
        self.assertEqual(back.entries, {(1, "resume"): "b"})
        with self.assertRaises(rp.PackError):
            rp.OrtNames(["a", "a"], {})

    def test_manifest_keys_cover_differences_and_borrowings(self):
        res = [rp.Residual("callee", 4, 0x10, "SYMBOL f", "SYMBOL g")]
        bor = {"placeholder": [[4, "D_10", 2]], "hybrid-data": [[4, "0x80"]]}
        keys = rp.manifest_keys(json.loads(rp.render_manifest(rp.manifest_from(res, bor))))
        self.assertEqual(keys, {("diff", "callee", 4, "0x10"), ("borrowed", "placeholder", 4, "D_10", 2),
                                ("borrowed", "hybrid-data", 4, "0x80")})

    def test_record_encoding(self):
        r = rp.Record(2, rp.OP_JUMP, 7, 0x1234, R26)
        self.assertEqual(r.encode(), struct.pack(">II", 7, (0x1234 << 8) | 0x42))


if __name__ == "__main__":
    unittest.main()
