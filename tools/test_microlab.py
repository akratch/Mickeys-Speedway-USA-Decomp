#!/usr/bin/env python3
"""Tests for tools/microlab.py: spec expansion and summaries on canned text; one
end-to-end probe through a tracked TU's recipe when the toolchain is present."""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))

import microlab  # noqa: E402

ROOT = TOOLS.parent

# Synthetic probe disassembly in objdump -d -z layout (our own probe code).
DUMP = """
probe.o:     file format elf32-tradbigmips

Disassembly of section .text:

00000000 <probe>:
   0:\t27bdffe8 \taddiu\tsp,sp,-24
   4:\tafbf0014 \tsw\tra,20(sp)
   8:\t00047203 \tsra\tt6,a0,0x8
   c:\t0c000000 \tjal\t0 <probe>
  10:\t31c400ff \tandi\ta0,t6,0xff
  14:\t8fbf0014 \tlw\tra,20(sp)
  18:\t27bd0018 \taddiu\tsp,sp,24
  1c:\t03e00008 \tjr\tra
  20:\t00000000 \tnop

00000024 <other>:
  24:\t03e00008 \tjr\tra
  28:\t00000000 \tnop
"""


class SpecTests(unittest.TestCase):
    def test_vars_take_the_product_in_order(self):
        rows = microlab.variants_from_spec({"vars": {"T": ["s16", "s32"], "M": ["0xFF", "0xFFU"]}})
        self.assertEqual(rows, [{"T": "s16", "M": "0xFF"}, {"T": "s16", "M": "0xFFU"},
                                {"T": "s32", "M": "0xFF"}, {"T": "s32", "M": "0xFFU"}])

    def test_shorthand_and_explicit_variants(self):
        self.assertEqual(microlab.variants_from_spec({"T": ["u8"]}), [{"T": "u8"}])
        self.assertEqual(microlab.variants_from_spec({"variants": [{"T": "u8"}, {"T": 3}]}),
                         [{"T": "u8"}, {"T": "3"}])
        self.assertEqual(microlab.variants_from_spec([{"T": "u8"}]), [{"T": "u8"}])

    def test_bad_specs_are_refused(self):
        for spec in ({}, {"vars": {"T": []}}, {"variants": "x"}, 7):
            with self.assertRaises(microlab.LabError):
                microlab.variants_from_spec(spec)

    def test_render_fills_and_refuses_missing_placeholders(self):
        self.assertEqual(microlab.render("{{ T }} x = {{M}};", {"T": "s16", "M": "1"}), "s16 x = 1;")
        with self.assertRaises(microlab.LabError):
            microlab.render("{{T}} {{U}}", {"T": "s16"})


class SummaryTests(unittest.TestCase):
    def test_disassembly_is_split_per_function(self):
        functions = microlab.parse_disassembly(DUMP)
        self.assertEqual(list(functions), ["probe", "other"])
        self.assertEqual(len(functions["probe"]), 9)
        self.assertEqual(functions["probe"][2], (0x00047203, "sra", "t6,a0,0x8"))

    def test_summary_counts_ops_and_registers(self):
        info = microlab.summarise(microlab.parse_disassembly(DUMP)["probe"])
        self.assertEqual(info["count"], 9)
        self.assertEqual(info["ops"]["addiu"], 2)
        self.assertEqual(info["registers"], ["a0", "ra", "sp", "t6"])

    def test_compare_names_what_moved(self):
        base = microlab.summarise(microlab.parse_disassembly(DUMP)["probe"])
        self.assertEqual(microlab.compare(base, base), "identical to v0")
        other_dump = DUMP.replace("31c400ff \tandi\ta0,t6,0xff", "01c02025 \tmove\ta0,t6")
        other = microlab.summarise(microlab.parse_disassembly(other_dump)["probe"])
        note = microlab.compare(base, other)
        self.assertIn("andi 1->0", note)
        self.assertIn("move 0->1", note)
        self.assertIn("+0 insns", note)


@unittest.skipUnless((ROOT / "tools/ido/cc").is_file() and (ROOT / "tools/binutils/mips64-elf-objdump").is_file(),
                     "IDO or binutils unavailable")
class EndToEndTests(unittest.TestCase):
    def test_probe_compiles_with_the_tracked_recipe(self):
        with tempfile.TemporaryDirectory() as d:
            tmp = Path(d)
            (tmp / "t.c").write_text("typedef int s32;\nextern void callee(s32 a);\n"
                                     "void probe(s32 v) { callee({{ARG}}); }\n")
            (tmp / "spec.json").write_text(json.dumps({"ARG": ["v & 0xFF", "v"]}))
            status = microlab.main(["--template", str(tmp / "t.c"), "--vars", str(tmp / "spec.json"),
                                    "--flags-from", "src/main/anim.c", "--out", str(tmp / "out"),
                                    "--json", str(tmp / "r.json"), "--function", "probe"])
            self.assertEqual(status, 0)
            report = json.loads((tmp / "r.json").read_text())
            self.assertEqual(report["source"], "src/main/anim.c")
            first, second = (v["functions"]["probe"] for v in report["variants"])
            self.assertIn("andi", first["ops"])
            self.assertNotIn("andi", second["ops"])
            self.assertTrue(second["note"].startswith("differs from v0"))

    def test_untracked_flags_source_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            tmp = Path(d)
            (tmp / "t.c").write_text("int x;\n")
            (tmp / "spec.json").write_text(json.dumps({"A": ["1"]}))
            self.assertEqual(microlab.main(["--template", str(tmp / "t.c"), "--vars", str(tmp / "spec.json"),
                                            "--flags-from", str(tmp / "t.c")]), 2)


if __name__ == "__main__":
    unittest.main()
