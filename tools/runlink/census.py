"""Census of shipped tables: counts, and each generation rule tested against the ROM alone.

Every check here reads only decoded tables (and the module images, for the
stored words).  A rule that holds for every record of a game is something a
generator can rely on; a rule with exceptions lists them.
"""

from __future__ import annotations

import collections
import struct

from .codec import (OP_DATA, OP_JUMP, OP_LOCAL, OP_NAMES, OP_SYMBOL, R_MIPS_26,
                    R_MIPS_HI16, R_MIPS_LO16, TYPE_NAMES)


def stale_jump_indices(module):
    """For each JUMP record, (record, the most recent SYMBOL index before it, table 1 then 2)."""
    last = 0
    out = []
    for _t, _i, r in module.records():
        if r.op == OP_SYMBOL:
            last = r.symidx
        elif r.op == OP_JUMP:
            out.append((r, last))
    return out


def rule_checks(tables, cfg):
    """Every measured rule, as {rule: {"holds": n, "total": n, "exceptions": [...]}}."""
    res = collections.OrderedDict()

    def rule(name):
        return res.setdefault(name, {"holds": 0, "total": 0, "exceptions": []})

    def tally(name, ok, where=None):
        r = rule(name)
        r["total"] += 1
        if ok:
            r["holds"] += 1
        elif where is not None and len(r["exceptions"]) < 200:
            r["exceptions"].append(where)

    exported = collections.defaultdict(set)
    for row in tables.ort:
        exported[row.selector].add(row.offset)

    for m in tables.modules:
        h = m.header
        if m.empty:
            continue
        n = m.number
        tally("text size is a multiple of 16", h.text % 16 == 0, n)
        tally("header vramBase is 0", h.vram == 0, n)
        for t, i, r in m.records():
            where = (n, t, i, r.site)
            if r.op == OP_SYMBOL:
                if r.symidx >= len(tables.ort):
                    tally("SYMBOL index is an ORT row", False, where)
                    continue
                tally("SYMBOL index is an ORT row", True)
                sel = tables.ort[r.symidx].selector
                if t == 1:
                    tally("table 1 SYMBOL targets another module", sel != n, where)
                else:
                    tally("table 2 SYMBOL targets this module", sel == n, where)
            tally("table 1 holds only SYMBOL/DATA records", t != 1 or r.op in (OP_SYMBOL, OP_DATA), where)
            tally("table 2 holds no DATA records", t != 2 or r.op != OP_DATA, where)
            if r.op == OP_JUMP:
                tally("JUMP is R_MIPS_26", r.type == R_MIPS_26, where)
                target = (m.word(r.site) & 0x3FFFFFF) << 2 if r.site + 4 <= len(m.image) else None
                tally("JUMP target is unexported", target is not None and target not in exported[n], where)
            if r.op == OP_DATA:
                tally("DATA site lies in data", r.site < h.data, where)
        # HI16 immediately followed by LO16 of the same operation and index.
        for t, rows in ((1, m.table1), (2, m.table2)):
            for i, r in enumerate(rows):
                if r.type == R_MIPS_HI16:
                    nxt = rows[i + 1] if i + 1 < len(rows) else None
                    tally("HI16 is immediately followed by its LO16",
                          nxt is not None and nxt.type == R_MIPS_LO16 and nxt.op == r.op
                          and nxt.symidx == r.symidx, (n, t, i, r.site))
        for r, last in stale_jump_indices(m):
            tally("JUMP index = most recent SYMBOL index (table 1 then 2)", r.symidx == last,
                  (n, r.site, r.symidx, last))
        bases = {r.symidx for _t, _i, r in m.records() if r.op == OP_LOCAL}
        tally("at most four LOCAL bases", len(bases) <= 4, (n, sorted(bases)))
        extra = sorted(bases - {0, h.text, h.text + h.data})
        tally("LOCAL bases are 0, text, text+data and at most one more", len(extra) <= 1,
              (n, [hex(b) for b in extra]))
        # Text-site records precede data-site records within each table.
        for t, rows in ((1, m.table1), (2, m.table2)):
            if not rows:
                continue
            seen_data = False
            ok = True
            for r in rows:
                in_data = r.op == OP_DATA or r.site >= h.text
                if in_data:
                    seen_data = True
                elif seen_data:
                    ok = False
            tally("text-site records precede data-site records", ok, (n, t))
            sites = [r.site for r in rows]
            tally("table is sorted by site", sites == sorted(sites), (n, t))
    return res


def counts(tables, cfg, rom=None):
    """Plain counts of everything decoded."""
    out = collections.OrderedDict()
    mods = tables.modules
    out["modules"] = len(mods)
    out["empty modules"] = sum(1 for m in mods if m.empty)
    out["module records"] = sum(len(m.table1) + len(m.table2) for m in mods)
    out["table 1 records"] = sum(len(m.table1) for m in mods)
    out["table 2 records"] = sum(len(m.table2) for m in mods)
    by = collections.Counter()
    for m in mods:
        for t, _i, r in m.records():
            by[(t, OP_NAMES.get(r.op, r.op), TYPE_NAMES.get(r.type, r.type))] += 1
    out["records by table/op/type"] = {f"{t} {o} {ty}": n for (t, o, ty), n in sorted(by.items(), key=str)}
    data_sites = collections.Counter()
    for m in mods:
        for t, _i, r in m.records():
            if r.op == OP_DATA or r.site >= m.header.text:
                data_sites[(t, OP_NAMES.get(r.op, r.op))] += 1
    out["data-site records by table/op"] = {f"{t} {o}": n for (t, o), n in sorted(data_sites.items(), key=str)}
    out["main table entries"] = len(tables.main)
    mt = collections.Counter((OP_NAMES.get(e.op, e.op), TYPE_NAMES.get(e.type, e.type)) for e in tables.main)
    out["main table by op/type"] = {f"{o} {t}": n for (o, t), n in sorted(mt.items(), key=str)}
    if rom is not None and cfg.trap_address is not None:
        jal = 0x0C000000 | ((cfg.trap_address & 0x0FFFFFFF) >> 2)
        hits = 0
        for e in tables.main:
            if e.op == OP_DATA:
                continue
            rom_off = resident_rom_offset(cfg, cfg.resident_text + e.site)
            if rom_off is not None and struct.unpack_from(">I", rom, rom_off)[0] == jal:
                hits += 1
        out["main table sites holding jal trap"] = hits
        main_sites = [e.site for e in tables.main if e.op != OP_DATA]
        out["main table text sites ascending"] = main_sites == sorted(main_sites)
    out["ORT rows"] = len(tables.ort)
    sel = collections.Counter()
    for r in tables.ort:
        sel["resident" if r.selector == 0 else (f"{r.selector:#x}" if r.selector in cfg.reserved_selectors
                                                else "module")] += 1
    out["ORT rows by selector"] = dict(sorted(sel.items()))
    used = set()
    for m in mods:
        for _t, _i, r in m.records():
            if r.op in (OP_SYMBOL, OP_DATA, OP_JUMP):
                used.add(r.symidx)
    for e in tables.main:
        used.add(e.symidx)
    out["ORT rows no SYMBOL/JUMP/main record names"] = sum(1 for i in range(len(tables.ort)) if i not in used)
    if tables.names is not None:
        out["ROM symbol names"] = len(tables.names)
        out["distinct ROM symbol names"] = len(set(tables.names))
    chain = 0
    pos = 0
    for m in mods:
        if m.header.rom == pos:
            chain += 1
        pos = m.header.rom + m.header.image_size
    out["headers chained back to back"] = f"{chain}/{len(mods)}"
    out["region end"] = hex(cfg.modules_rom + pos)
    return out


def resident_rom_offset(cfg, vram):
    """ROM offset of a resident address, using the config's resident load offset."""
    if cfg.resident_rom_delta is None:
        return None
    return vram - cfg.resident_rom_delta
