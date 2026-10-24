#!/usr/bin/env python3
"""tools/runlink codec and config against hand-written table bytes (no ROM)."""

from __future__ import annotations

import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from runlink import codec, config, names  # noqa: E402
from runlink.synth import header, ort_row, record, words  # noqa: E402

CONFIG = """
name = "synth"
rom = "rom.z64"
[tables]
main_table = 0x100
main_table_block = 0x40
ort = 0x140
ort_count = 3
headers = 0x150
module_count = 2
modules = 0x190
[names]
offsets = 0x300
blob = 0x30C
blob_end = 0x31C
[resident]
text = 0x80000450
data = 0x80001000
bss = 0x80002000
"""


def synth_rom():
    rom = bytearray(0x400)
    rom[0:4] = b"\x80\x37\x12\x40"
    main = struct.pack(">I", 2) + record(1, 0x10, 4, 0) + record(2, 0x0, 2, 3)
    rom[0x100:0x100 + len(main)] = main
    rom[0x140:0x14C] = ort_row(0, 0x20) + ort_row(1, 0x8) + ort_row(0xFFD, 0x4)
    # Module 1: text 0x10, data 0x8, one record in each table.
    t1 = record(0, 0x0, 4, 0)
    t2 = record(0x10, 0x4, 5, 1) + record(0x10, 0x8, 6, 1)
    img = words(0x0C000000, 0x3C010000, 0x24210000, 0) + words(1, 2)
    rom[0x150:0x170] = header(0, 0x10, 0x8, 0x20, len(t1), len(t2), -1, 0x4)
    # Module 2 is empty and takes the offset after module 1.
    size1 = len(img) + len(t1) + len(t2)
    rom[0x170:0x190] = header(size1, 0, 0, 0, 0, 0)
    rom[0x190:0x190 + size1] = img + t1 + t2
    # Names: offsets parallel to the ORT; the third row repeats the first name.
    rom[0x300:0x30C] = struct.pack(">III", 0, 6, 0)
    rom[0x30C:0x31C] = b"alpha\0beta\0" + b"\0" * 5
    return bytes(rom), 0x190 + size1


class CodecTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        (root / "synth.toml").write_text(CONFIG)
        self.rom, self.end = synth_rom()
        (root / "rom.z64").write_bytes(self.rom)
        self.cfg = config.load(str(root / "synth.toml"), root)

    def tearDown(self):
        self.tmp.cleanup()

    def test_decode_fields(self):
        t = codec.decode(self.rom, self.cfg)
        self.assertEqual([(r.symidx, r.site, r.type, r.op) for r in t.main], [(1, 0x10, 4, 0), (2, 0, 2, 3)])
        self.assertEqual([(r.selector, r.offset) for r in t.ort], [(0, 0x20), (1, 0x8), (0xFFD, 0x4)])
        m1, m2 = t.modules
        self.assertEqual((m1.header.text, m1.header.data, m1.header.bss, m1.header.resume), (0x10, 8, 0x20, 4))
        self.assertEqual([r.key for r in m1.table1], [(0, 4)])
        self.assertEqual([(r.symidx, r.op) for r in m1.table2], [(0x10, 1), (0x10, 1)])
        self.assertTrue(m2.empty)
        self.assertEqual(t.names, ["alpha", "beta", "alpha"])

    def test_region_round_trip(self):
        t = codec.decode(self.rom, self.cfg)
        lo, hi = codec.region_bounds(t, self.cfg)
        self.assertEqual((lo, hi), (0x100, self.end))
        self.assertEqual(codec.encode_region(t, self.cfg), self.rom[lo:hi])

    def test_relayout_recomputes_offsets_and_sizes(self):
        t = codec.decode(self.rom, self.cfg)
        t.modules[0].table2.pop()
        mods = codec.layout_modules(t.modules)
        self.assertEqual(mods[0].header.rel2, 8)
        self.assertEqual(mods[1].header.rom, 0x18 + 8 + 8)

    def test_explicit_gap_fill_preserves_every_table_and_module_byte(self):
        tables = codec.decode(self.rom, self.cfg)
        zero = codec.encode_region(tables, self.cfg)
        filled = codec.encode_region(tables, self.cfg, gap_fill=255)
        begin = self.cfg.ort_rom + 4 * len(tables.ort) - self.cfg.main_table_rom
        end = self.cfg.headers_rom - self.cfg.main_table_rom
        self.assertGreater(end, begin)
        self.assertEqual(zero[:begin], filled[:begin])
        self.assertEqual(bytes(end-begin), zero[begin:end])
        self.assertEqual(bytes([255]) * (end-begin), filled[begin:end])
        self.assertEqual(zero[end:], filled[end:])
        for value in (-1, 256, None):
            with self.assertRaises(codec.FormatError):
                codec.encode_region(tables, self.cfg, gap_fill=value)

    def test_names_dedupe_round_trip(self):
        t = codec.decode(self.rom, self.cfg)
        blob = names.encode_names(t.names, self.cfg)
        self.assertEqual(blob, self.rom[0x300:0x300 + len(blob)])

    def test_byte_orders(self):
        v64 = bytearray(self.rom)
        v64[0::2], v64[1::2] = self.rom[1::2], self.rom[0::2]
        n64 = b"".join(self.rom[i:i + 4][::-1] for i in range(0, len(self.rom), 4))
        for img in (bytes(v64), n64):
            p = Path(self.tmp.name) / "swapped.bin"
            p.write_bytes(img)
            self.assertEqual(codec.read_rom(p), self.rom)

    def test_outside_catalog_never_decodes_omitted_header_or_ort_cells(self):
        from unittest import mock
        original = codec.Header.decode
        poisoned = bytearray(self.rom)
        poisoned[self.cfg.headers_rom:self.cfg.headers_rom + 32] = bytes([255]) * 32
        poisoned[self.cfg.ort_rom + 4:self.cfg.ort_rom + 8] = bytes([255]) * 4
        def checked(data, offset=0):
            self.assertNotEqual(offset, self.cfg.headers_rom)
            return original(data, offset)
        with mock.patch.object(codec.Header, 'decode', side_effect=checked):
            outside = codec.decode(poisoned, self.cfg, exclude_modules={1}, exclude_rows={1})
        self.assertEqual([m.number for m in outside.modules], [2])
        self.assertIsNone(outside.ort[1])
        self.assertEqual(outside.ort[0], codec.decode(self.rom, self.cfg).ort[0])

    def test_limits(self):
        with self.assertRaises(codec.FormatError):
            codec.Record(0, 1 << 24, 4, 0).encode()
        with self.assertRaises(codec.FormatError):
            codec.OrtRow(1, 1 << 20).encode()
        with self.assertRaises(codec.FormatError):
            codec.encode_main([codec.Record(0, 0, 4, 0)] * 8, 0x40)

    def test_cli_check_and_census(self):
        tool = Path(__file__).resolve().parent / "runlink"
        cfg = str(Path(self.tmp.name) / "synth.toml")
        for cmd in ("check", "census"):
            p = subprocess.run([sys.executable, str(tool), "--config", cfg, "--root", self.tmp.name, cmd],
                               capture_output=True, text=True)
            self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
        self.assertIn("re-encodes byte-identically", subprocess.run(
            [sys.executable, str(tool), "--config", cfg, "--root", self.tmp.name, "check"],
            capture_output=True, text=True).stdout)

    def test_config_by_name_and_errors(self):
        self.assertEqual(config.load("msu-us").module_count, 107)
        self.assertEqual(config.load("jfg-us").module_count, 157)
        bad = Path(self.tmp.name) / "bad.toml"
        bad.write_text("[tables]\nort = 1\n")
        with self.assertRaises(config.ConfigError):
            config.load(str(bad))


if __name__ == "__main__":
    unittest.main()
