"""Generic front end: a GNU ld build (linked ELF, ``-Map`` file, objects) in, tables out.

For a splat-style build where module *n* is linked as its own output section
(``module_section``, e.g. ``.overlay_{n}``) and the resident module is linked
normally.  Inputs, all from the config's ``[build]`` table:

  elf, map             the linked ELF and its map
  module_section       output section of module n (``{n}`` is formatted)
  module_bss_section   its NOBITS section
  table_inputs         regexes for input files that are the shipped tables
  text_align           header text size alignment (default 16)
  resident_sections    output sections of the resident module
  main_sentinel        resident symbol whose HI16/LO16 references the main table patches
  runtime_objects      regexes for resident objects whose references are the runtime's own
  entry_names          {init = "...", resume = "..."}: names of module entry points ({n} formatted)
  placeholder          regex for undefined names that are placeholders, not identities
  unpatched            optional "OLD=NEW" object-path prefix rewrite locating the resident
                       objects as compiled, before any pass renamed their trap callees;
                       their callee names name the main-table targets

Identity of each relocation, in order:

1. a section symbol or a symbol the object defines: the object's own section,
   placed by the map;
2. a linked symbol in a module or resident section;
3. an absolute linked address inside the resident module;
4. a name in the ORT name list (the ROM's, or the tracked list);
5. otherwise a **placeholder**: in ``--mode hybrid`` its identity is taken
   from the shipped record at the same (site, type), and counted as borrowed;
   in ``--mode objects`` it produces no record.

``--mode oracle`` takes *every* identity from the shipped record at the same
(site, type), whatever the build says, which isolates the rules from the
naming state of the source.

Only identity is borrowed.  Table, operation, index, order, headers and the
main table are always the rules' (:mod:`runlink.rules`).
"""

from __future__ import annotations

import collections
import json
import re
from pathlib import Path

from . import codec, compare, config, mapfile, names, rules
from .codec import OP_JUMP, OP_LOCAL, OP_SYMBOL, R_MIPS_26, R_MIPS_32, R_MIPS_HI16, R_MIPS_LO16
from .elf import STB_LOCAL, STT_FILE, STT_SECTION, Elf, in_place_addends

SECTIONS = (".text", ".data", ".rodata")
SEC_KIND = {".text": rules.TEXT, ".data": rules.DATA, ".rodata": rules.RODATA, ".bss": rules.BSS,
            ".sdata": rules.DATA, ".sbss": rules.BSS, "COMMON": rules.BSS}


class Build:
    def __init__(self, cfg, tables):
        b = cfg.build
        self.cfg = cfg
        self.tables = tables
        self.elf = Elf(cfg.resolve(b["elf"]))
        self.outputs, self.inputs = mapfile.parse(cfg.resolve(b["map"]))
        self.mod_fmt = b.get("module_section", ".overlay_{n}")
        self.bss_fmt = b.get("module_bss_section", ".overlay_{n}_bss")
        self.table_res = [re.compile(x) for x in b.get("table_inputs", [])]
        self.text_align = int(b.get("text_align", 16))
        self.placeholder = re.compile(b.get("placeholder", r"^(?:func|D)_[0-9A-Fa-f]+$"))
        self.resident_sections = set(b.get("resident_sections", [".main"]))
        self.sentinel = b.get("main_sentinel")
        self.runtime_res = [re.compile(x) for x in b.get("runtime_objects", [])]
        self.entry_names = b.get("entry_names", {})
        self.unpatched = b.get("unpatched")   # "OLD=NEW": rewrite the map's object path prefix
        self.per_out = collections.defaultdict(list)
        for mi in self.inputs:
            self.per_out[mi.outsec].append(mi)
        self.sec_module = {}
        for n in range(1, cfg.module_count + 1):
            self.sec_module[self.mod_fmt.format(n=n)] = (n, False)
            self.sec_module[self.bss_fmt.format(n=n)] = (n, True)
        self.by_name = collections.defaultdict(list)
        for s in self.elf.symbols:
            if s.name and s.type not in (STT_SECTION, STT_FILE):
                self.by_name[s.name].append(s)
        self.names = names.ort_names(cfg, tables)
        self.name_rows = {}
        for i, n in enumerate(self.names):
            self.name_rows.setdefault(n, i)
        self._objs = {}
        self.layout = {n: self._layout(n) for n in range(1, cfg.module_count + 1)}

    # -- layout

    def is_table(self, mi):
        return any(r.search(mi.file) for r in self.table_res)

    def _layout(self, n):
        sec = self.mod_fmt.format(n=n)
        out = self.outputs.get(sec)
        if out is None:
            return None
        ins = [mi for mi in self.per_out[sec] if mi.size and not self.is_table(mi)]
        start = out.addr
        place = {}
        text_end = 0
        data_end = None
        rodata = []
        for mi in ins:
            off = mi.addr - start
            place[(mi.file, mi.insec)] = off
            if mi.insec == ".text":
                text_end = max(text_end, off + mi.size)
        a = self.text_align
        text = (text_end + a - 1) // a * a
        for mi in ins:
            if mi.insec != ".text":
                data_end = max(data_end or text, mi.addr - start + mi.size)
                if mi.insec == ".rodata":
                    rodata.append(mi.addr - start)
        data = (data_end - text) if data_end is not None else 0
        bss_sec = self.bss_fmt.format(n=n)
        bss_out = self.outputs.get(bss_sec)
        bss = 0
        for mi in self.per_out.get(bss_sec, []):
            place[(mi.file, mi.insec)] = text + data + (mi.addr - bss_out.addr)
            bss = max(bss, mi.addr - bss_out.addr + mi.size)
        image = b""
        es = self.elf.section(sec)
        if es is not None:
            image = self.elf.bytes_of(sec)[:text + data]
        return {"start": start, "bss_start": bss_out.addr if bss_out else None, "text": text,
                "data": data, "bss": bss, "rodata": min(rodata) if rodata else None, "place": place,
                "inputs": ins, "image": image}

    def obj(self, file):
        p = self.cfg.resolve(file)
        if p not in self._objs:
            self._objs[p] = Elf(p) if p.exists() else None
        return self._objs[p]

    # -- identities

    def linked_identity(self, s):
        hit = self.sec_module.get(s.section)
        if hit:
            n, is_bss = hit
            lay = self.layout.get(n)
            if lay is None:
                return None
            if is_bss:
                return ("ov", n, s.value - lay["bss_start"] + lay["text"] + lay["data"])
            return ("ov", n, s.value - lay["start"])
        if s.section in self.resident_sections and self.resident(s.value):
            return ("res", s.value)
        return None

    def resident(self, addr):
        c = self.cfg
        lo = c.resident_text - 0x50 if c.resident_text else 0
        return lo <= addr < (c.resident_end or 0xFFFFFFFF)

    def name_identity(self, name):
        """(identity, route) for an undefined name, or (None, "placeholder")."""
        for s in self.by_name.get(name, ()):
            if s.absolute:
                continue
            ident = self.linked_identity(s)
            if ident:
                return ident, "linked"
        if not self.placeholder.match(name):
            for s in self.by_name.get(name, ()):
                if s.absolute and self.resident(s.value):
                    return ("res", s.value), "resident-address"
        row = self.name_rows.get(name)
        if row is not None and not self.placeholder.match(name):
            return rules.row_identity(self.tables.ort[row], self.cfg), "ort-name"
        return None, "placeholder"


def module_refs(build, n, export, expected, mode, stats):
    lay = build.layout[n]
    refs = []
    for mi in lay["inputs"]:
        if mi.insec not in SECTIONS:
            continue
        o = build.obj(mi.file)
        if o is None:
            stats["missing objects"] += 1
            continue
        rels = o.rels(mi.insec)
        if not rels:
            continue
        adds = in_place_addends(o.bytes_of(mi.insec), rels)
        base_off = lay["place"][(mi.file, mi.insec)]
        for r, a in zip(rels, adds):
            site = base_off + r.offset
            s = r.sym
            ref = rules.Ref(mi.insec, site, r.type, name=s.name or s.section, obj=mi.file)
            e = expected.get((site, r.type))
            if mode == "oracle" and e is not None:
                ref.route = "oracle"
                take_oracle(build, n, ref, e, export)
                refs.append(ref)
                continue
            if s.type == STT_SECTION or s.defined:
                off = lay["place"].get((mi.file, s.section))
                if off is None:
                    ref.problem = "unplaced-target"
                    refs.append(ref)
                    continue
                ref.base = ("ov", n, off + (0 if s.type == STT_SECTION else s.value))
                ref.target_section = SEC_KIND.get(s.section, "")
                static = s.type == STT_SECTION or s.bind == STB_LOCAL
                ref.route = "section" if s.type == STT_SECTION else ("object-local" if static else "object-definition")
                if not static:
                    ref.row = export.row(ref.base, build.name_rows.get(s.name))
            else:
                ident, route = build.name_identity(s.name)
                ref.route = route
                if ident is not None:
                    ref.base = ident
                    ref.row = export.row(ident, build.name_rows.get(s.name))
                else:
                    stats["placeholder relocations"] += 1
                    if mode == "objects":
                        ref.problem = "placeholder"
                    elif e is None:
                        ref.problem = "placeholder-without-record"
                    else:
                        take_oracle(build, n, ref, e, export)
                    refs.append(ref)
                    continue
            ref.full = rules.ident_add(ref.base, a)
            refs.append(ref)
    return refs


def take_oracle(build, n, ref, e, export):
    """Give a reference the identity of the shipped record at its (site, type).

    Only the identity: the referenced address, and for a SYMBOL record the row
    that names it.  Table, operation and index are still the rules'."""
    ref.provenance = "oracle"
    rec = e["rec"]
    ref.full = e["full"]
    if rec.op == OP_SYMBOL:
        ref.base = rules.row_identity(build.tables.ort[rec.symidx], build.cfg)
        ref.row = export.row(ref.base, rec.symidx)
    else:
        ref.base = e["full"]
        ref.row = export.row(ref.base)


def entry_offset(build, n, kind, export):
    fmt = build.entry_names.get(kind)
    if not fmt:
        return -1, None
    name = fmt.format(n=n)
    for s in build.by_name.get(name, ()):
        ident = build.linked_identity(s)
        if ident and ident[0] == "ov" and ident[1] == n:
            return ident[2], "linked"
    row = build.name_rows.get(name)
    if row is not None:
        ident = rules.row_identity(build.tables.ort[row], build.cfg)
        if ident[0] == "ov" and ident[1] == n:
            return ident[2], "ort-name"
    return -1, None


def main_refs(build, mode, stats):
    cfg = build.cfg
    ship = {}
    for e in build.tables.main:
        base = cfg.resident_data if e.op == codec.OP_DATA else cfg.resident_text
        ship[base + e.site] = e
    out = []
    seen = set()
    for mi in build.inputs:
        if mi.outsec not in build.resident_sections or mi.insec not in (".text", ".data") or not mi.size:
            continue
        if (mi.file, mi.insec) in seen:
            continue
        seen.add((mi.file, mi.insec))
        o = build.obj(mi.file)
        if o is None:
            continue
        runtime = any(r.search(mi.file) for r in build.runtime_res)
        orig = None
        if build.unpatched:
            old, _, new = build.unpatched.partition("=")
            up = build.cfg.resolve(new + mi.file[len(old):]) if mi.file.startswith(old) else None
            if up is not None and up.exists():
                orig = {(r.offset, r.type): r.sym.name for r in Elf(up).rels(mi.insec)}
        for r in o.rels(mi.insec):
            nm, addr = r.sym.name, mi.addr + r.offset
            if mi.insec == ".text" and r.type == R_MIPS_26 and nm == cfg.trap_symbol:
                m = rules.MainRef(addr, r.type, False)
            elif (mi.insec == ".text" and r.type in (R_MIPS_HI16, R_MIPS_LO16) and nm == build.sentinel
                  and not runtime):
                m = rules.MainRef(addr, r.type, False)
            elif mi.insec == ".data" and r.type == R_MIPS_32 and nm == cfg.trap_symbol:
                m = rules.MainRef(addr, r.type, True)
            else:
                continue
            m.obj = mi.file
            callee = orig.get((r.offset, r.type)) if orig else None
            if callee and callee in build.name_rows:
                m.row = build.name_rows[callee]
                m.name = callee
                stats["main entries named by the objects"] += 1
            else:
                m.provenance = "placeholder"
            if mode == "oracle" or (m.row is None and mode == "hybrid"):
                m.row = None
                e = ship.get(addr)
                m.verbatim_row = e.symidx if e is not None else None
                m.provenance = "oracle"
                m.name = build.names[e.symidx] if e is not None and e.symidx < len(build.names) else nm
            out.append(m)
    return out


def ort_rows(build, stats):
    rows = []
    for i, nm in enumerate(build.names):
        ident = None
        for s in build.by_name.get(nm, ()):
            ident = build.linked_identity(s) if not s.absolute else (
                ("res", s.value) if s.value >= 0x80000000 and build.resident(s.value)
                or (build.cfg.absolute_selector is not None and 0x80000000 <= s.value < build.cfg.resident_text)
                else None)
            if ident:
                break
        if ident is None:
            stats["ORT names the build does not define"] += 1
            rows.append(None)
            continue
        rows.append(codec.OrtRow(*rules.identity_row(ident, build.cfg)))
    return rows


def shipped_rodata_base(m):
    """The LOCAL base a shipped module uses beyond 0, text and text + data (its rodata base)."""
    h = m.header
    extra = sorted({r.symidx for _t, _i, r in m.records() if r.op == OP_LOCAL} - {0, h.text, h.text + h.data})
    return extra[0] if extra else None


def run(cfg, rom, tables, mode="hybrid", unpatched=None):
    build = Build(cfg, tables)
    if unpatched:
        build.unpatched = unpatched
    stats = collections.Counter()
    export = rules.ExportIndex([rules.row_identity(r, cfg) for r in tables.ort])
    mods, residuals = [], []
    routes = collections.Counter()
    for n in range(1, cfg.module_count + 1):
        sm = tables.module(n)
        lay = build.layout[n]
        expected = compare.shipped_identities(sm, tables, cfg)
        if lay is None:
            hdr = codec.Header(0, 0, 0, 0, 0, 0, 0, -1, -1)
            gm = rules.GenModule(n, hdr, b"")
        else:
            init, _ = entry_offset(build, n, "init", export)
            resume, _ = entry_offset(build, n, "resume", export)
            hdr = codec.Header(0, 0, lay["text"], lay["data"], lay["bss"], 0, 0, init, resume)
            refs = module_refs(build, n, export, expected, mode, stats)
            for r in refs:
                routes[r.route if not r.problem else r.problem] += 1
            rodata = lay["rodata"]
            shipped_rodata = shipped_rodata_base(sm)
            if rodata != shipped_rodata and mode != "objects" and shipped_rodata is not None:
                stats["rodata bases borrowed"] += 1
                rodata = shipped_rodata
            gm = rules.build_module(n, refs, hdr, lay["image"], rodata,
                                    stale_jump_for=("object", "oracle"))
            gm.refs = refs
        mods.append(gm)
        residuals += compare.compare_module(gm, expected)
        residuals += compare.compare_header(gm, sm)
        residuals += [x for x in compare.compare_unresolved(gm)]
    orows = ort_rows(build, stats)
    residuals += compare.compare_ort(orows, tables.ort, lambda i: build.names[i])
    main = rules.build_main(main_refs(build, mode, stats), cfg,
                            by_address=cfg.build.get("main_order", "link") == "address")
    residuals += compare.compare_main(main, tables.main,
                                      lambda i: build.names[i] if 0 <= i < len(build.names) else str(i))
    gt = compare.gen_tables(mods, main, orows, tables)
    return build, mods, main, orows, residuals, gt, stats, routes


# ---------------------------------------------------------------------------
# Reporting: one measurement per rule


def measure(tables, mods, main, orows, residuals, stats, routes):
    """One number per rule, over the shipped tables."""
    cats = collections.Counter(r.category for r in residuals)
    by_mod = collections.defaultdict(lambda: collections.defaultdict(set))
    for r in residuals:
        by_mod[r.module][r.category].add(r.offset)
    out = collections.OrderedDict()
    shipped_n = sum(len(m.table1) + len(m.table2) for m in tables.modules)

    # Surface: does an object relocation exist at each shipped (site, type)?
    cover = collections.Counter()
    total = collections.Counter()
    extra = collections.Counter()
    ident = collections.Counter()
    for m, gm in zip(tables.modules, mods):
        refs = getattr(gm, "refs", [])
        keys = {(r.site, r.type): r for r in refs if r.site is not None}
        ship = {r.key for _t, _i, r in m.records()}
        for _t, _i, r in m.records():
            k = ("text" if r.site < m.header.text else "data", codec.TYPE_NAMES[r.type])
            total[k] += 1
            if r.key in keys:
                cover[k] += 1
                ref = keys[r.key]
                if ref.route == "oracle":
                    ident["oracle: identity taken from the shipped record"] += 1
                elif ref.provenance == "oracle":
                    ident["placeholder: identity borrowed from the shipped record"] += 1
                elif ref.problem == "placeholder":
                    ident["placeholder: no identity, no record generated"] += 1
                else:
                    ident["determined by the build (" + ref.route + ")"] += 1
        for key, ref in keys.items():
            if key not in ship:
                extra[("text" if key[0] < m.header.text else "data", codec.TYPE_NAMES[key[1]])] += 1
    out["shipped module records"] = shipped_n
    out["shipped records with an object relocation at their (site, type)"] = {
        f"{a} {b}": f"{cover[(a, b)]}/{total[(a, b)]}" for a, b in sorted(total)}
    out["  total"] = f"{sum(cover.values())}/{shipped_n}"
    out["object relocations with no shipped record"] = {f"{a} {b}": n for (a, b), n in sorted(extra.items())}
    out["identity of the covered records"] = dict(sorted(ident.items()))

    # Per-record rules, on the covered records.
    common = sum(cover.values())
    wrong_addr = sum(cats[c] for c in ("callee", "data-ref", "pool-layout", "table-entry"))
    out["covered records: address agrees"] = f"{common - wrong_addr}/{common}"
    for c, label in (("table", "table split"), ("op", "operation"), ("row", "SYMBOL row"),
                     ("base", "LOCAL base"), ("jump-index", "stale JUMP index")):
        out[f"covered records: {label} differs"] = cats[c]

    # JUMP index: is each wrong one explained by a missing upstream SYMBOL?
    explained = 0
    wrong = 0
    jumps = 0
    for m, gm in zip(tables.modules, mods):
        gen_keys = {g.key for g in gm.gens()}
        last_key = None
        gen_by_key = {g.key: g for g in gm.gens()}
        for _t, _i, r in m.records():
            if r.op == OP_SYMBOL:
                last_key = r.key
            elif r.op == OP_JUMP and r.key in gen_by_key:
                jumps += 1
                if gen_by_key[r.key].rec.symidx != r.symidx:
                    wrong += 1
                    if last_key is not None and last_key not in gen_keys:
                        explained += 1
                    elif last_key is None:
                        pass
    out["generated JUMPs with the shipped stale index"] = f"{jumps - wrong}/{jumps}"
    out["  wrong ones whose shipped predecessor SYMBOL is missing"] = f"{explained}/{wrong}"

    # Order, per table, on the records both sides have.
    agree = collections.Counter()
    for m, gm in zip(tables.modules, mods):
        for t, ship_rows, gen_rows in ((1, m.table1, gm.table1), (2, m.table2, gm.table2)):
            if not ship_rows:
                continue
            gk = {g.key for g in gen_rows}
            sk = {r.key for r in ship_rows}
            a = [r.key for r in ship_rows if r.key in gk]
            b = [g.key for g in gen_rows if g.key in sk]
            sorted_ship = [r.site for r in ship_rows] == sorted(r.site for r in ship_rows)
            cls = "site-sorted" if sorted_ship else "not site-sorted"
            agree[(cls, "tables")] += 1
            if a == b:
                agree[(cls, "order agrees")] += 1
    for cls in ("site-sorted", "not site-sorted"):
        out[f"shipped tables {cls}: generated order agrees"] = (
            f"{agree[(cls, 'order agrees')]}/{agree[(cls, 'tables')]}")
    out["records out of order"] = cats["order"]

    # Headers.
    hdr = collections.Counter()
    present = [m for m in tables.modules if not m.empty]
    for r in residuals:
        if r.category == "header":
            hdr[r.expected.split()[0]] += 1
    out["header fields that agree"] = {f: f"{len(present) - hdr[label]}/{len(present)}"
                                       for f, label in compare.HEADER_FIELDS
                                       for f in [label]}
    out["module images (text+data bytes) that agree"] = f"{len(present) - cats['image']}/{len(present)}"

    # Main table and ORT.
    ship_keys = {(e.site, e.type, e.op) for e in tables.main}
    gen_keys = {(g.rec.site, g.rec.type, g.rec.op) for g in main}
    out["main table: shipped entries the objects produce"] = f"{len(ship_keys & gen_keys)}/{len(ship_keys)}"
    out["main table: entries with no shipped entry"] = len(gen_keys - ship_keys)
    out["main table: order agrees"] = "no" if cats["main-order"] else "yes"
    named = sum(1 for g in main if g.provenance == "object")
    out["main table: rows named by the objects"] = f"{named}/{len(main)}"
    out["main table: rows that differ"] = cats["main-row"]
    out["ORT rows reproduced from the names and the build"] = f"{len(tables.ort) - cats['ort']}/{len(tables.ort)}"
    record_cats = {"duplicate", "extra", "callee", "data-ref", "pool-layout", "table-entry", "op", "table",
                   "row", "base", "jump-index", "missing", "order", "unplaced", "unresolved"}
    with_records = [m for m in tables.modules if m.table1 or m.table2]
    out["modules whose tables regenerate exactly"] = (
        f"{sum(1 for m in with_records if not record_cats & set(by_mod[m.number]))}/{len(with_records)}")
    out["modules with no difference at all"] = (
        f"{sum(1 for m in present if not by_mod[m.number])}/{len(present)}")
    out["differences by category"] = dict(sorted(cats.items()))
    out["stats"] = dict(stats)
    return out


def main(cfg, rom, tables, args):
    if cfg.build.get("adapter") == "msu":
        if args.mode != "hybrid":
            raise config.ConfigError(
                "MSU adapter supports only --mode hybrid; "
                f"--mode {args.mode} is not implemented")
        from .adapters import msu
        rp, world, ort, mods, main_g, residuals, gt = msu.generate(cfg, rom, tables)
        stats, routes, orows = collections.Counter(), collections.Counter(), None
        cats = collections.Counter(r.category for r in residuals)
        print("differences: " + ", ".join(f"{k} {v}" for k, v in sorted(cats.items())))
    else:
        build, mods, main_g, orows, residuals, gt, stats, routes = run(cfg, rom, tables, args.mode,
                                                                       getattr(args, "unpatched", None))
        m = measure(tables, mods, main_g, orows, residuals, stats, routes)
        for k, v in m.items():
            if isinstance(v, dict):
                print(f"{k}:")
                for kk, vv in v.items():
                    print(f"    {kk:40s} {vv}")
            else:
                print(f"{k:60s} {v}")
        if args.json:
            Path(args.json).write_text(json.dumps({"measure": m, "residuals": [vars(r) for r in residuals]},
                                                  indent=0, default=str) + "\n")
    proof = compare.region_proof(gt, tables, cfg, rom, residuals)
    print(f"region {proof['region'][0]:#x}-{proof['region'][1]:#x}: generated "
          f"{'identical' if proof['raw_equal'] else 'differs'} ({proof['raw_differing_bytes']} bytes); "
          f"with the {len(residuals)} residuals replaced: "
          f"{'identical' if proof['patched_equal'] else 'differs'} ({proof['patched_differing_bytes']} bytes)")
    if proof["order_breaks"]:
        print(f"  order differs outside the residuals in {len(proof['order_breaks'])} tables")
    if args.verbose:
        for r in sorted(residuals, key=lambda r: (r.category, r.module, r.offset))[:args.limit]:
            print(f"  {r.category:12s} m{r.module:03d} +{r.offset:#x}: expected {r.expected}; got {r.got}")
    if args.out:
        Path(args.out).write_bytes(codec.encode_region(gt, cfg))
    return 0 if proof["patched_equal"] else 1
