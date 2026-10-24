"""The generation rules: resolved references in, Rare's tables out.

This module knows nothing about a game or a build.  A front end (an adapter,
or :mod:`runlink.generate`) turns each relocation of a module's objects into a
:class:`Ref` -- where the patched word is, which address it refers to, and
whether that address is exported -- in *emission order*, and these functions
apply the rules measured on Mickey's Speedway USA (18,542 records) and
confirmed against Jet Force Gemini's shipped tables (27,515 records):

1. **Emission order.**  Text-site records in the order the objects list them
   (each object's ``.rel.text`` as the compiler wrote it, objects in link
   order); then the data-site records, ``.rodata`` sites ascending, then
   ``.data`` sites ascending.
2. **Table split.**  Table 1 holds the records whose referenced symbol lies
   outside the module; table 2 holds the rest.  Each table keeps emission
   order.
3. **Operation.**  Outside the module: SYMBOL, indexed by the ORT row.  Inside:
   SYMBOL when the referenced symbol is exported (has an ORT row) and the
   reference is by name; otherwise JUMP for an R_MIPS_26 and LOCAL for
   anything else.
4. **LOCAL base.**  The section start of the target, in the layout
   ``[text][data][rodata][bss]``: 0, the text size, the module's rodata base,
   and text + data.
5. **Stale JUMP index.**  A JUMP's index is the index of the most recent
   SYMBOL record, table 1 then table 2, or 0 before the first.
6. **HI16 pairing.**  Every HI16 is immediately followed by a LO16 of the
   same operation and index (checked, not enforced: the compiler's order
   already satisfies it).
7. **Main table.**  The resident module's references to module symbols, in
   link order; text sites relative to the resident text base and the DATA
   entries (R_MIPS_32 words in resident data) after them, relative to the
   resident data base.
"""

from __future__ import annotations

import collections
from dataclasses import dataclass, field

from .codec import (OP_DATA, OP_JUMP, OP_LOCAL, OP_SYMBOL, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16,
                    Header, Record)

TEXT, DATA, RODATA, BSS = ".text", ".data", ".rodata", ".bss"


# ---------------------------------------------------------------------------
# Identities: ("ov", module, offset) for a module address, ("res", address)
# for a resident one, ("sel", selector, offset) for a reserved selector the
# runtime does not map.


def ident_add(ident, n):
    if ident is None:
        return None
    if ident[0] == "res":
        return ("res", (ident[1] + n) & 0xFFFFFFFF)
    return (ident[0], ident[1], ident[2] + n)


def row_identity(row, cfg):
    """The address an ORT row names."""
    sel, off = row.selector, row.offset
    if sel == 0:
        return ("res", cfg.resident_text + off)
    if sel in cfg.data_selectors and cfg.resident_data is not None:
        return ("res", cfg.resident_data + off)
    if sel == cfg.bss_selector and cfg.resident_bss is not None:
        return ("res", cfg.resident_bss + off)
    if sel == cfg.absolute_selector:
        return ("res", cfg.absolute_base + off)
    if sel in cfg.reserved_selectors:
        return ("sel", sel, off)
    return ("ov", sel, off)


def identity_row(ident, cfg):
    """The (selector, offset) an ORT row needs to name an identity."""
    if ident[0] == "ov":
        return ident[1], ident[2]
    if ident[0] == "sel":
        return ident[1], ident[2]
    addr = ident[1]
    if cfg.absolute_selector is not None and addr < cfg.resident_text - 0x50:
        return cfg.absolute_selector, addr - cfg.absolute_base
    if cfg.resident_data is not None and addr < cfg.resident_data:
        return 0, addr - cfg.resident_text
    if cfg.resident_bss is not None and addr >= cfg.resident_bss:
        return cfg.bss_selector, addr - cfg.resident_bss
    if cfg.resident_data is None:
        return 0, addr - cfg.resident_text
    return cfg.data_selectors[0], addr - cfg.resident_data


class ExportIndex:
    """identity -> ORT rows, from the identities each row names."""

    def __init__(self, identities):
        self.rows = collections.defaultdict(list)
        for i, ident in enumerate(identities):
            if ident is not None:
                self.rows[ident].append(i)

    def row(self, ident, prefer=None):
        rows = self.rows.get(ident, ())
        if prefer is not None and prefer in rows:
            return prefer
        return rows[0] if rows else None

    def exported(self, ident):
        return ident in self.rows


# ---------------------------------------------------------------------------
# Module records


@dataclass
class Ref:
    """One relocation of a module object, resolved as far as the inputs allow."""
    section: str                 # the site's section: .text, .data or .rodata
    site: int                    # module offset of the patched word (None: unplaced)
    type: int                    # ELF relocation type
    base: tuple = None           # identity of the referenced symbol, before the addend
    full: tuple = None           # identity of the referenced address
    row: int = None              # ORT row the reference is exported through, if any
    target_section: str = ""     # the target's section, for a LOCAL base
    name: str = ""               # for reports
    obj: str = ""
    route: str = ""              # how the front end resolved the identity
    verbatim: Record = None      # a record taken as is from elsewhere (placeholder identity)
    verbatim_table: int = 0
    provenance: str = "object"   # object | placeholder | hybrid-data | oracle
    problem: str = ""            # set when the front end could not place or resolve it
    runtime_local_base: int | None = None  # authenticated loader role, independent of ELF section


@dataclass
class Gen:
    """One generated record and where it came from."""
    table: int
    rec: Record
    full: tuple = None
    name: str = ""
    provenance: str = "object"
    route: str = ""
    obj: str = ""

    @property
    def key(self):
        return self.rec.key

    @property
    def site(self):
        return self.rec.site


@dataclass
class GenModule:
    number: int
    header: Header
    image: bytes
    table1: list = field(default_factory=list)     # [Gen]
    table2: list = field(default_factory=list)
    unresolved: list = field(default_factory=list)  # [(Ref, reason)]
    notes: list = field(default_factory=list)

    def gens(self):
        return self.table1 + self.table2


def local_bases(text, data, rodata_base):
    rod = text if rodata_base is None else rodata_base
    return {TEXT: 0, DATA: text, RODATA: rod, BSS: text + data}


def section_of_offset(off, text, data, rodata_base):
    if off < text:
        return TEXT
    if off >= text + data:
        return BSS
    if rodata_base is not None and rodata_base > text and off >= rodata_base:
        return RODATA
    return DATA


def build_module(number, refs, header, image, rodata_base=None, extra_data=(),
                 data_order=(RODATA, DATA), stale_jump_for=("object",)):
    """Apply rules 1-5 to one module.

    ``refs`` are in emission order for text sites; data-site refs may come in
    any order.  ``extra_data`` are data-site Gens taken from elsewhere (the
    shipped table, for data no object owns yet); they are ordered with the
    rest.  A JUMP gets the stale index only when its provenance is in
    ``stale_jump_for``; other JUMPs keep the index they came with."""
    text, data = header.text, header.data
    bases = local_bases(text, data, rodata_base)
    mod = GenModule(number, header, image)
    text_gens, data_gens = [], []
    for r in refs:
        if r.problem:
            mod.unresolved.append((r, r.problem))
            continue
        if r.site is None:
            mod.unresolved.append((r, "unplaced-site"))
            continue
        if r.verbatim is not None:
            g = Gen(r.verbatim_table, Record(r.verbatim.symidx, r.site, r.type, r.verbatim.op),
                    r.full, r.name, r.provenance, r.route, r.obj)
        elif r.base is None:
            mod.unresolved.append((r, "unplaced-target"))
            continue
        elif r.base[0] == "ov" and r.base[1] == number:
            if r.row is not None:
                op, idx = OP_SYMBOL, r.row
            elif r.type == R_MIPS_26:
                op, idx = OP_JUMP, 0
            else:
                if r.runtime_local_base is not None:
                    if not 0 <= r.runtime_local_base <= text + data:
                        raise ValueError("resolved runtime LOCAL base is outside the initialized module")
                    op, idx = OP_LOCAL, r.runtime_local_base
                else:
                    kind = r.target_section or section_of_offset(r.base[2], text, data, rodata_base)
                    op, idx = OP_LOCAL, bases[kind]
            g = Gen(2, Record(idx, r.site, r.type, op), r.full, r.name, r.provenance, r.route, r.obj)
        else:
            if r.row is None:
                mod.unresolved.append((r, "unexported-target"))
            g = Gen(1, Record(-1 if r.row is None else r.row, r.site, r.type, OP_SYMBOL),
                    r.full, r.name, r.provenance, r.route, r.obj)
        (text_gens if r.section == TEXT else data_gens).append(g)
    data_gens.extend(extra_data)
    rod = text if rodata_base is None else rodata_base
    rank = {k: i for i, k in enumerate(data_order)}

    def data_key(g):
        kind = RODATA if g.site >= rod else DATA
        return (rank.get(kind, len(rank)), g.site)

    data_gens.sort(key=data_key)
    ordered = text_gens + data_gens
    mod.table1 = [g for g in ordered if g.table == 1]
    mod.table2 = [g for g in ordered if g.table == 2]
    apply_stale_jump(mod, stale_jump_for)
    mod.notes.extend(check_pairing(mod))
    return mod


def apply_stale_jump(mod, provenances=("object",)):
    last = 0
    for g in mod.table1 + mod.table2:
        if g.rec.op == OP_SYMBOL:
            last = g.rec.symidx
        elif g.rec.op == OP_JUMP and g.provenance in provenances:
            g.rec.symidx = last


def check_pairing(mod):
    out = []
    for t, rows in ((1, mod.table1), (2, mod.table2)):
        for i, g in enumerate(rows):
            if g.rec.type != R_MIPS_HI16:
                continue
            nxt = rows[i + 1].rec if i + 1 < len(rows) else None
            if nxt is None or nxt.type != R_MIPS_LO16 or nxt.op != g.rec.op or nxt.symidx != g.rec.symidx:
                out.append(f"table {t} position {i}: HI16 at {g.site:#x} is not followed by its LO16")
    return out


# ---------------------------------------------------------------------------
# The main table


@dataclass
class MainRef:
    """One resident reference to a module symbol, in link order."""
    site: int            # absolute resident address of the patched word
    type: int
    data: bool = False   # site in resident data: a DATA entry relative to the data base
    row: int = None      # ORT row of the target (None: unknown)
    name: str = ""
    obj: str = ""
    provenance: str = "object"
    verbatim_row: int = None   # row taken from elsewhere when the source does not name it


def build_main(refs, cfg, by_address=False):
    """Text entries, then DATA entries, each in the given (link) order or by address."""
    out = []
    text = [r for r in refs if not r.data]
    data = [r for r in refs if r.data]
    if by_address:
        text.sort(key=lambda r: r.site)
        data.sort(key=lambda r: r.site)
    for r in text + data:
        base = cfg.resident_data if r.data else cfg.resident_text
        row = r.row if r.row is not None else r.verbatim_row
        out.append(Gen(0, Record(-1 if row is None else row, r.site - base, r.type,
                                 OP_DATA if r.data else OP_SYMBOL),
                       None, r.name, r.provenance, "", r.obj))
    return out
