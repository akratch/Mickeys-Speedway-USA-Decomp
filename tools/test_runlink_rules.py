#!/usr/bin/env python3
"""tools/runlink generation rules, comparer and census on synthetic records."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parent))
from runlink import census, compare, rules  # noqa: E402
from runlink.adapters import msu  # noqa: E402
from runlink.codec import (OP_DATA, OP_JUMP, OP_LOCAL, OP_SYMBOL, R_MIPS_26, R_MIPS_32,  # noqa: E402
                           R_MIPS_HI16, R_MIPS_LO16, Header, Module, OrtRow, Record, Tables)

CFG = SimpleNamespace(resident_text=0x80000450, resident_data=0x80001000, resident_bss=0x80002000,
                      data_selectors=(0xFFD, 0xFFE), bss_selector=0xFFF,
                      reserved_selectors=(0xFFC, 0xFFD, 0xFFE, 0xFFF),
                      absolute_selector=0xFFC, absolute_base=0x80000000, trap_address=None,
                      modules_rom=0)

TEXT, DATA = 0x40, 0x20     # module 1: text 0x40, data 0x20 (rodata from 0x50), bss 0x10


def ref(section, site, rtype, base, row=None, tsec="", **kw):
    return rules.Ref(section, site, rtype, base=base, full=base, row=row, target_section=tsec, **kw)


def module_refs():
    own = lambda off: ("ov", 1, off)  # noqa: E731
    return [
        ref(".text", 0x00, R_MIPS_26, ("res", 0x80000500), row=7),          # table 1 SYMBOL
        ref(".text", 0x08, R_MIPS_HI16, own(0x48), tsec=".data"),         # LOCAL, data base
        ref(".text", 0x0C, R_MIPS_LO16, own(0x48), tsec=".data"),
        ref(".text", 0x04, R_MIPS_26, own(0x20)),                          # JUMP (unexported)
        ref(".text", 0x10, R_MIPS_26, own(0x30), row=9),                   # table 2 SYMBOL (exported)
        ref(".text", 0x14, R_MIPS_26, own(0x20)),                          # JUMP after it
        ref(".data", 0x44, R_MIPS_32, own(0x20), tsec=".text"),            # data site, LOCAL text
        ref(".rodata", 0x54, R_MIPS_32, own(0x60), tsec=".bss"),           # rodata site, LOCAL bss
        ref(".rodata", 0x50, R_MIPS_32, own(0x52), tsec=".rodata"),        # rodata site, LOCAL rodata
    ]


class RulesTests(unittest.TestCase):
    def build(self, refs=None):
        hdr = Header(0, 0, TEXT, DATA, 0x10, 0, 0, -1, -1)
        return rules.build_module(1, refs or module_refs(), hdr, b"\0" * (TEXT + DATA), rodata_base=0x50)

    def test_authenticated_runtime_base_is_independent_of_compiler_and_global_ro(self):
        refs = [ref(".text", 0x08, R_MIPS_HI16, ("ov", 1, 0x58),
                    tsec=".data", runtime_local_base=0x50),
                ref(".text", 0x0C, R_MIPS_LO16, ("ov", 1, 0x58),
                    tsec=".data", runtime_local_base=0x50)]
        hdr = Header(0, 0, TEXT, DATA, 0x10, 0, 0, -1, -1)
        result = rules.build_module(1, refs, hdr, bytes(TEXT + DATA), rodata_base=0x54)
        self.assertEqual([g.rec.symidx for g in result.table2], [0x50, 0x50])
        self.assertEqual([g.full for g in result.table2], [("ov", 1, 0x58)] * 2)
        for item in refs:
            item.runtime_local_base = 0
        zero = self.build(refs)
        self.assertEqual([g.rec.symidx for g in zero.table2], [0, 0])
        self.assertEqual(zero.notes, [])
        exported = ref(".text", 0x08, R_MIPS_32, ("ov", 1, 0x58), row=7,
                       tsec=".data", runtime_local_base=TEXT + DATA + 1)
        self.assertEqual(self.build([exported]).table2[0].rec.symidx, 7)
        refs[0].runtime_local_base = TEXT + DATA + 1
        with self.assertRaisesRegex(ValueError, "outside"):
            self.build(refs)

    def test_msu_adapter_preserves_source_runtime_roles_and_origins(self):
        obj = SimpleNamespace(module=1, runtime_role="data", runtime_origin=0x50,
                              placement={".data": (0x58, "link")})
        target = SimpleNamespace(kind="section", obj=obj, secname=".data")
        rp = SimpleNamespace(section_offset=lambda item, section: item.placement[section][0])
        self.assertEqual(msu.runtime_local_base(rp, target, 1, TEXT), TEXT)
        obj.runtime_role = "ro-origin"
        self.assertEqual(msu.runtime_local_base(rp, target, 1, TEXT), 0x58)
        obj.runtime_role = "ro-member"
        self.assertEqual(msu.runtime_local_base(rp, target, 1, TEXT), 0x50)
        self.assertIsNone(msu.runtime_local_base(rp, target, 2, TEXT))
        obj.runtime_origin = None
        with self.assertRaisesRegex(ValueError, "lacks"):
            msu.runtime_local_base(rp, target, 1, TEXT)
        obj.runtime_role = "unexpected"
        with self.assertRaisesRegex(ValueError, "unsupported"):
            msu.runtime_local_base(rp, target, 1, TEXT)
        obj.runtime_role = None
        self.assertIsNone(msu.runtime_local_base(rp, target, 1, TEXT))

    def test_split_order_and_operation(self):
        m = self.build()
        self.assertEqual([(g.site, g.rec.op) for g in m.table1], [(0x00, OP_SYMBOL)])
        # Text records keep emission order (0x04 after 0x0C); data sites follow,
        # rodata ascending first, then data.
        self.assertEqual([g.site for g in m.table2], [0x08, 0x0C, 0x04, 0x10, 0x14, 0x50, 0x54, 0x44])
        self.assertEqual([g.rec.op for g in m.table2],
                         [OP_LOCAL, OP_LOCAL, OP_JUMP, OP_SYMBOL, OP_JUMP, OP_LOCAL, OP_LOCAL, OP_LOCAL])

    def test_local_bases(self):
        m = self.build()
        base = {g.site: g.rec.symidx for g in m.table2 if g.rec.op == OP_LOCAL}
        self.assertEqual(base, {0x08: TEXT, 0x0C: TEXT, 0x44: 0, 0x50: 0x50, 0x54: TEXT + DATA})

    def test_stale_jump_index(self):
        m = self.build()
        jumps = [g.rec.symidx for g in m.table2 if g.rec.op == OP_JUMP]
        # The first JUMP inherits table 1's last SYMBOL (7); the second, table 2's (9).
        self.assertEqual(jumps, [7, 9])

    def test_verbatim_record_keeps_its_index(self):
        refs = module_refs()
        refs[3] = rules.Ref(".text", 0x04, R_MIPS_26, verbatim=Record(123, 0, R_MIPS_26, OP_JUMP),
                            verbatim_table=2, provenance="placeholder")
        m = self.build(refs)
        self.assertEqual([g.rec.symidx for g in m.table2 if g.rec.op == OP_JUMP], [123, 9])
        m = rules.build_module(1, refs, Header(0, 0, TEXT, DATA, 0x10), b"", 0x50,
                               stale_jump_for=("object", "placeholder"))
        self.assertEqual([g.rec.symidx for g in m.table2 if g.rec.op == OP_JUMP], [7, 9])

    def test_unexported_outside_target_and_problems(self):
        refs = [ref(".text", 0, R_MIPS_26, ("ov", 2, 0x10)),
                rules.Ref(".text", 4, R_MIPS_26, problem="placeholder"),
                rules.Ref(".text", None, R_MIPS_26, base=("ov", 1, 0))]
        m = self.build(refs)
        self.assertEqual(m.table1[0].rec.symidx, -1)
        self.assertEqual(sorted(w for _r, w in m.unresolved), ["placeholder", "unexported-target", "unplaced-site"])

    def test_pairing_check(self):
        refs = [ref(".text", 8, R_MIPS_HI16, ("ov", 1, 0x48), tsec=".data"),
                ref(".text", 0, R_MIPS_26, ("ov", 1, 0x20)),
                ref(".text", 12, R_MIPS_LO16, ("ov", 1, 0x48), tsec=".data")]
        self.assertTrue(self.build(refs).notes)

    def test_identities(self):
        for row, ident in ((OrtRow(0, 0x10), ("res", 0x80000460)), (OrtRow(0xFFD, 4), ("res", 0x80001004)),
                           (OrtRow(0xFFF, 8), ("res", 0x80002008)), (OrtRow(5, 0x30), ("ov", 5, 0x30)),
                           (OrtRow(0xFFC, 0x300), ("res", 0x80000300))):
            self.assertEqual(rules.row_identity(row, CFG), ident)
            self.assertEqual(rules.identity_row(ident, CFG), (row.selector, row.offset))

    def test_main_table(self):
        refs = [rules.MainRef(0x80000470, R_MIPS_26, row=3),
                rules.MainRef(0x80001010, R_MIPS_32, data=True, row=4),
                rules.MainRef(0x80000460, R_MIPS_26, row=5)]
        link = rules.build_main(refs, CFG)
        self.assertEqual([(g.rec.site, g.rec.op) for g in link], [(0x20, OP_SYMBOL), (0x10, OP_SYMBOL), (0x10, OP_DATA)])
        addr = rules.build_main(refs, CFG, by_address=True)
        self.assertEqual([g.rec.site for g in addr], [0x10, 0x20, 0x10])


class CompareTests(unittest.TestCase):
    def shipped(self, records):
        m = Module(1, Header(0, 0, 0x10, 0x10, 0, 0, 0), b"\0" * 0x20,
                   [r for t, r in records if t == 1], [r for t, r in records if t == 2])
        return m, Tables([], [OrtRow(0, 0), OrtRow(1, 0x8)], [m])

    def gen(self, records):
        m = rules.GenModule(1, Header(0, 0, 0x10, 0x10, 0, 0, 0), b"\0" * 0x20)
        for t, r, full in records:
            (m.table1 if t == 1 else m.table2).append(rules.Gen(t, r, full))
        return m

    def test_categories(self):
        sm, tables = self.shipped([(1, Record(0, 0x0, R_MIPS_26, OP_SYMBOL)),
                                   (2, Record(1, 0x4, R_MIPS_26, OP_SYMBOL)),
                                   (2, Record(1, 0x8, R_MIPS_26, OP_JUMP)),
                                   (2, Record(0x10, 0x10, R_MIPS_32, OP_LOCAL)),
                                   (2, Record(0, 0xC, R_MIPS_26, OP_JUMP))])
        exp = compare.shipped_identities(sm, tables, CFG)
        gm = self.gen([(1, Record(0, 0x0, R_MIPS_26, OP_SYMBOL), ("res", 0x80000450)),
                       (2, Record(0, 0x8, R_MIPS_26, OP_JUMP), ("ov", 1, 0)),        # jump-index
                       (2, Record(0, 0x10, R_MIPS_32, OP_LOCAL), ("ov", 1, 0x10)),   # base (0 vs 0x10)
                       (2, Record(1, 0x4, R_MIPS_26, OP_SYMBOL), ("ov", 1, 0x8)),    # order
                       (2, Record(0, 0x18, R_MIPS_26, OP_JUMP), ("ov", 1, 0))])      # extra
        cats = sorted((r.category, r.offset) for r in compare.compare_module(gm, exp))
        self.assertIn(("jump-index", 0x8), cats)
        self.assertIn(("extra", 0x18), cats)
        self.assertIn(("missing", 0xC), cats)
        self.assertIn(("order", 0x4), cats)

    def test_header_and_main(self):
        sm, _t = self.shipped([])
        gm = self.gen([])
        gm.header = Header(0, 0, 0x10, 0x10, 8, 0, 0)
        self.assertEqual([r.category for r in compare.compare_header(gm, sm)], ["header"])
        ship = [Record(1, 0x10, R_MIPS_26, OP_SYMBOL), Record(2, 0x20, R_MIPS_26, OP_SYMBOL)]
        gens = [rules.Gen(0, Record(2, 0x20, R_MIPS_26, OP_SYMBOL)), rules.Gen(0, Record(9, 0x10, R_MIPS_26, OP_SYMBOL))]
        cats = sorted(r.category for r in compare.compare_main(gens, ship))
        self.assertEqual(cats, ["main-order", "main-row"])


class CensusTests(unittest.TestCase):
    def test_rules_and_exceptions(self):
        img = b"".join(x.to_bytes(4, "big") for x in (0x0C000004, 0x0C000004, 0, 0, 0, 0, 0, 0))
        m = Module(1, Header(0, 0, 0x10, 0x10, 0, 0, 0), img,
                   [Record(0, 0x0, R_MIPS_26, OP_SYMBOL)],
                   [Record(1, 0x8, R_MIPS_HI16, OP_LOCAL), Record(1, 0xC, R_MIPS_LO16, OP_LOCAL),
                    Record(0, 0x4, R_MIPS_26, OP_JUMP),     # stale index 0: holds
                    Record(5, 0x0, R_MIPS_26, OP_JUMP)])    # 5: an exception
        t = Tables([], [OrtRow(2, 0), OrtRow(1, 0x30)], [m])
        res = census.rule_checks(t, CFG)
        j = res["JUMP index = most recent SYMBOL index (table 1 then 2)"]
        self.assertEqual((j["holds"], j["total"]), (1, 2))
        self.assertEqual(res["HI16 is immediately followed by its LO16"]["holds"], 1)
        self.assertEqual(res["table is sorted by site"]["holds"], 1)    # table 1 yes, table 2 no
        c = census.counts(t, CFG)
        self.assertEqual(c["module records"], 5)
        self.assertEqual(c["ORT rows no SYMBOL/JUMP/main record names"], 1)


if __name__ == "__main__":
    unittest.main()
