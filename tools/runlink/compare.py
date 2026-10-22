"""Compare generated tables with shipped ones and classify every difference.

A difference is a :class:`Residual` keyed by (category, module, offset).  The
categories, in the order they are tested per record:

  duplicate     two generated records at one (site, type)
  extra         a generated record the shipped table lacks
  callee / data-ref / pool-layout / table-entry
                the record refers to a different address (split by what has
                to change: the call, a data reference, an object's own
                literal layout, or a data-site table entry)
  op            same address, different operation
  table         same address and operation, other table
  row / base / jump-index
                same address, operation and table, different index
  missing       a shipped record no generated record matches
  order         the record sits at a different position (outside the
                longest common subsequence of the shipped order)
  header        a header field differs (offset 0, field named)
  image         module text+data bytes differ (offset = first difference)
  unplaced / unresolved
                a relocation the front end could not place or resolve
  ort           an ORT row names a different address (module 0, offset = row)
  main-extra / main-missing / main-row / main-order
                the same tests for the main table (module 0, offset = site)
"""

from __future__ import annotations

import difflib
import struct
from dataclasses import dataclass

from .codec import (OP_JUMP, OP_LOCAL, OP_NAMES, R_MIPS_26, R_MIPS_32, R_MIPS_HI16, R_MIPS_LO16,
                    TYPE_NAMES, encode_region, region_bounds)
from .rules import ident_add, row_identity


def sext16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


def hexs(v):
    return "0x%X" % v


@dataclass(frozen=True)
class Residual:
    category: str
    module: int
    offset: int
    expected: str
    got: str

    @property
    def key(self):
        return (self.category, self.module, self.offset)


def shipped_identities(module, tables, cfg):
    """{(site, type): {table, position, rec, full}} for one shipped module.

    ``full`` is the address the record makes the site refer to, with the
    addend read from the stored words the way the runtime reads it."""
    out = {}
    img = module.image

    def word(off):
        return struct.unpack_from(">I", img, off)[0] if off + 4 <= len(img) else 0

    for table, rows in ((1, module.table1), (2, module.table2)):
        last_hi = {}
        for i, r in enumerate(rows):
            w = word(r.site)
            if r.type == R_MIPS_HI16:
                nxt = rows[i + 1] if i + 1 < len(rows) else None
                lo = sext16(word(nxt.site)) if nxt else 0
                last_hi[(r.op, r.symidx)] = w & 0xFFFF
                a = ((w & 0xFFFF) << 16) + lo
            elif r.type == R_MIPS_LO16:
                hi = last_hi.get((r.op, r.symidx))
                a = ((hi << 16) if hi is not None else 0) + sext16(w)
            elif r.type == R_MIPS_32:
                a = (w - 0x100000000 if w & 0x80000000 else w) if r.op == OP_LOCAL else 0
            else:
                a = 0
            if r.op == OP_JUMP:
                full = ("ov", module.number, (w & 0x3FFFFFF) << 2)
            elif r.op == OP_LOCAL:
                full = ("ov", module.number, r.symidx + a)
            elif r.symidx < len(tables.ort):
                full = ident_add(row_identity(tables.ort[r.symidx], cfg), a)
            else:
                full = None
            if r.key in out:
                raise ValueError(f"module {module.number}: two shipped records at {r.site:#x}")
            out[r.key] = {"table": table, "position": i, "rec": r, "full": full}
    return out


def default_classify(gen, mod):
    """Split a wrong-address difference by what has to change to fix it."""
    if gen.site >= mod.header.text:
        return "table-entry"
    if gen.route in ("section", "object-local"):
        return "pool-layout"
    if gen.rec.type == R_MIPS_26:
        return "callee"
    return "data-ref"


def fmt_ident(ident):
    if ident is None:
        return "?"
    if ident[0] == "res":
        return "resident %s" % hexs(ident[1])
    if ident[0] == "sel":
        return "selector %s %s" % (hexs(ident[1]), hexs(ident[2]))
    return "overlay %d %s" % (ident[1], hexs(ident[2]))


def compare_module(mod, expected, name_of=fmt_ident, classify=default_classify):
    out = []
    got = {}
    for g in mod.gens():
        if g.key in got:
            out.append(Residual("duplicate", mod.number, g.site, "", g.name))
        got[g.key] = g
    for key, g in got.items():
        e = expected.get(key)
        r = g.rec
        if e is None:
            out.append(Residual("extra", mod.number, g.site, "no record", f"{OP_NAMES[r.op]} {g.name}"))
            continue
        er = e["rec"]
        en = name_of(e["full"])
        if g.full != e["full"]:
            out.append(Residual(classify(g, mod), mod.number, g.site, f"{OP_NAMES[er.op]} {en}",
                                f"{OP_NAMES[r.op]} {g.name}" + (f" ({fmt_ident(g.full)})" if g.full else "")))
        elif r.op != er.op:
            out.append(Residual("op", mod.number, g.site, f"{OP_NAMES[er.op]} {en}", f"{OP_NAMES[r.op]} {g.name}"))
        elif g.table != e["table"]:
            out.append(Residual("table", mod.number, g.site, f"table {e['table']}", f"table {g.table}"))
        elif (r.symidx & 0xFFFFFFFF) != er.symidx:
            cat = "jump-index" if r.op == OP_JUMP else ("base" if r.op == OP_LOCAL else "row")
            out.append(Residual(cat, mod.number, g.site, f"{er.symidx}", f"{r.symidx}"))
    for key, e in expected.items():
        if key not in got:
            out.append(Residual("missing", mod.number, key[0], f"{OP_NAMES[e['rec'].op]} {name_of(e['full'])}",
                                "no record"))
    flagged = {x.offset for x in out}
    for table, rows in ((1, mod.table1), (2, mod.table2)):
        ship = [k for k, e in sorted(expected.items(), key=lambda kv: kv[1]["position"]) if e["table"] == table]
        mine = [g.key for g in rows]
        common = set(ship) & set(mine)
        a = [k for k in ship if k in common]
        b = [k for k in mine if k in common]
        sm = difflib.SequenceMatcher(a=a, b=b, autojunk=False)
        in_lcs = set()
        for blk in sm.get_matching_blocks():
            in_lcs.update(b[blk.b:blk.b + blk.size])
        for k in b:
            if k not in in_lcs and k[0] not in flagged:
                out.append(Residual("order", mod.number, k[0], f"table {table} position "
                                    f"{expected[k]['position']}", f"position {mine.index(k)}"))
    return out


HEADER_FIELDS = (("text", "text_size"), ("data", "data_size"), ("bss", "bss_size"),
                 ("init", "init_function"), ("resume", "resume_function"))


def compare_header(mod, shipped):
    out = []
    for fld, label in HEADER_FIELDS:
        a, b = getattr(shipped.header, fld), getattr(mod.header, fld)
        if a != b:
            out.append(Residual("header", mod.number, 0, f"{label} {a}", f"{b}"))
    if mod.image is not None and shipped.image != mod.image:
        a, b = shipped.image, mod.image
        first = next((i for i in range(min(len(a), len(b))) if a[i] != b[i]), min(len(a), len(b)))
        out.append(Residual("image", mod.number, first, f"{len(a)} bytes", f"{len(b)} bytes"))
    return out


def compare_unresolved(mod):
    out = []
    for r, why in mod.unresolved:
        if why == "unexported-target":
            continue          # still emitted, and classified by its address
        if why == "unplaced-site":
            out.append(Residual("unplaced", mod.number, r.site if r.site is not None else 0,
                                f"{r.obj} {r.section}", f"{TYPE_NAMES.get(r.type, r.type)} {r.name}"))
            continue
        out.append(Residual("unresolved", mod.number, r.site or 0, why, f"{r.obj} {r.name}"))
    return out


def compare_ort(rows, shipped_rows, name_of_row=lambda i: str(i)):
    out = []
    for i, (w, e) in enumerate(zip(rows, shipped_rows)):
        if w is None or (w.selector, w.offset) != (e.selector, e.offset):
            out.append(Residual("ort", 0, i, f"{e.selector:#x}:{e.offset:#x}",
                                f"{name_of_row(i)}" + ("" if w is None else f" {w.selector:#x}:{w.offset:#x}")))
    for i in range(len(rows), len(shipped_rows)):
        out.append(Residual("ort", 0, i, "row", "no row"))
    return out


def compare_main(gens, shipped, name_of_row=lambda i: str(i)):
    out = []
    ship = {(e.site, e.type, e.op): e for e in shipped}
    mine = {(g.rec.site, g.rec.type, g.rec.op): g for g in gens}
    for k, g in mine.items():
        e = ship.get(k)
        if e is None:
            out.append(Residual("main-extra", 0, g.rec.site, "no entry", g.name))
        elif e.symidx != (g.rec.symidx & 0xFFFFFFFF):
            out.append(Residual("main-row", 0, g.rec.site, name_of_row(e.symidx), g.name))
    for k, e in ship.items():
        if k not in mine:
            out.append(Residual("main-missing", 0, k[0], name_of_row(e.symidx), "no entry"))
    a = [(e.site, e.type, e.op) for e in shipped if (e.site, e.type, e.op) in mine]
    b = [(g.rec.site, g.rec.type, g.rec.op) for g in gens if (g.rec.site, g.rec.type, g.rec.op) in ship]
    if a != b:
        out.append(Residual("main-order", 0, 0, "shipped order", "generated order"))
    return out


# ---------------------------------------------------------------------------
# Byte proof


def region_proof(gen_tables, shipped, cfg, rom, residuals):
    """Byte-compare the generated region with the ROM.

    ``gen_tables`` is a codec.Tables built from the generated records.  Two
    images: the generated one, and the generated one with exactly the
    records and main entries a residual names replaced by the shipped ones.
    The second must equal the ROM byte for byte (and the record order must
    agree outside the residuals)."""
    lo, hi = region_bounds(shipped, cfg)
    want = rom[lo:hi]
    raw = encode_region(gen_tables, cfg)
    flagged = {}
    header_fix = {}
    for r in residuals:
        flagged.setdefault(r.module, set()).add(r.offset)
        if r.category == "ort":
            header_fix.setdefault("ort", set()).add(r.offset)
        if r.category in ("header", "image"):
            header_fix.setdefault(r.module, set()).add(r.category)
    order_breaks = []
    patched_modules = []
    from .codec import Module
    for gm, sm in zip(gen_tables.modules, shipped.modules):
        fl = flagged.get(gm.number, set())
        mine = {r.key: r for r in gm.table1 + gm.table2}
        tabs = []
        for t, ship_rows, gen_rows in ((1, sm.table1, gm.table1), (2, sm.table2, gm.table2)):
            kept = [r.key for r in ship_rows if r.key in mine and r.site not in fl]
            ship_keys = {r.key for r in ship_rows}
            ours = [r.key for r in gen_rows if r.key in ship_keys and r.site not in fl]
            if kept != ours:
                order_breaks.append((gm.number, t))
            tabs.append([r if (r.site in fl or r.key not in mine) else mine[r.key] for r in ship_rows])
        hdr, img = gm.header, gm.image
        if "header" in header_fix.get(gm.number, ()):
            from dataclasses import replace as _replace
            hdr = _replace(gm.header, text=sm.header.text, data=sm.header.data, bss=sm.header.bss,
                           init=sm.header.init, resume=sm.header.resume)
        if "image" in header_fix.get(gm.number, ()):
            img = sm.image
        patched_modules.append(Module(gm.number, hdr, img, tabs[0], tabs[1]))
    mine_main = {(r.site, r.type, r.op): r for r in gen_tables.main}
    fl0 = flagged.get(0, set())
    main = [mine_main.get((e.site, e.type, e.op), e) if e.site not in fl0 else e for e in shipped.main]
    a = [(e.site, e.type, e.op) for e in shipped.main if (e.site, e.type, e.op) in mine_main]
    b = [(r.site, r.type, r.op) for r in gen_tables.main if (r.site, r.type, r.op) in {(e.site, e.type, e.op) for e in shipped.main}]
    if a != b:
        order_breaks.append((0, "main"))
    from .codec import Tables
    ort_fix = header_fix.get("ort", set())
    ort = [shipped.ort[i] if i in ort_fix else row for i, row in enumerate(gen_tables.ort)]
    patched = encode_region(Tables(main, ort, patched_modules), cfg)

    def ndiff(x, y):
        return sum(1 for p, q in zip(x, y) if p != q) + abs(len(x) - len(y))

    first = next((lo + i for i, (p, q) in enumerate(zip(patched, want)) if p != q),
                 None if len(patched) == len(want) else lo + min(len(patched), len(want)))
    return {"region": (lo, hi), "raw_equal": raw == want, "raw_differing_bytes": ndiff(raw, want),
            "patched_equal": patched == want and not order_breaks,
            "patched_differing_bytes": ndiff(patched, want), "first_difference": first,
            "order_breaks": order_breaks}


def gen_tables(gen_modules, main_gens, ort_rows, shipped):
    """A codec.Tables from generated modules (number order), main Gens and ORT rows."""
    from .codec import Module, OrtRow, Tables
    mods = []
    for gm in gen_modules:
        mods.append(Module(gm.number, gm.header, gm.image,
                           [g.rec for g in gm.table1], [g.rec for g in gm.table2]))
    ort = [r if r is not None else OrtRow(0, 0) for r in ort_rows]
    return Tables([g.rec for g in main_gens], ort, mods, shipped.names)
