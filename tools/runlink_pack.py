#!/usr/bin/env python3
"""Pack Mickey's runlink tables from compiled objects, and compare them with ROM.

Mickey's overlays ship with four tables the runtime linker (src/main/runlink.c)
reads: per module a header and two relocation tables, plus the overlay ROM
table (ORT) and the main relocation table for resident call sites.  This tool
emits all of them from the compiled objects, in the order and encoding Rare's
converter used, and classifies every way the result differs from the shipped
tables.  It is phase 1, steps 4-5, of docs/OVERLAY_STRUCTURE_SPRINT.md, and
docs/overlays.md "The runlink packer" is its user documentation.

Inputs
------
- The raw overlay objects under build/runlink_raw/ (`gmake runlink-raw`): every
  overlay source compiled by its exact configured recipe with the post-compile
  normalizations switched off, so the relocations are the compiler's own.
- The atlas, config/overlays.us.json: which object owns which module text
  range, in order.
- The ordinary build: build/mickey.us.elf and .map for resident symbols, the
  placement of every linked input section and the module images, and the
  resident objects with their `.trapsites` sidecars (tools/trap_names.py).
- config/runlink-ort.us.txt: the ORT as names, one row per line, plus the
  module entry points.  `gmake runlink-ort` generates it.  Names only.
- baseroms/mickey.us.z64, read at run time, for the comparison.  Nothing
  ROM-derived is written outside build/.

Rules (measured over all 18,542 module records; see the sprint doc)
--------------------------------------------------------------------
- Emission order: each object's .rel.text in the compiler's order, objects in
  atlas order; then the data-site records, the .rodata ones ascending, then
  the .data ones ascending.
- Table 1 holds the records whose target lies outside the module; table 2 the
  rest.  Data-site records therefore only ever reach table 2.
- SYMBOL names an ORT row.  A target inside the module is SYMBOL when it is
  exported (has an ORT row) and otherwise JUMP for a call or LOCAL for an
  address.  A section-relative (static) reference is never SYMBOL.
- LOCAL's index is a section base: 0 for text, the text size for .data, the
  text + data size for .bss, and a module rodata base for .rodata.
- JUMP's index is stale: the index of the most recent SYMBOL record in
  emission order, table 1 then table 2 (0 before the first).
- The main table: every resident trap call (R_MIPS_26 to TrapDanglingJump),
  every HI16/LO16 to the runtime's unresolved-symbol sentinel outside the
  runtime itself, in link order; then every R_MIPS_32 to TrapDanglingJump in
  resident data, as a DATA record relative to the data base.  Address-of
  references to TrapDanglingJump are the runtime's own and are excluded.

What the source cannot express yet is taken from the shipped tables and
*recorded as borrowed* (placeholder identities, blob-owned data records, data
placements and rodata bases the objects do not determine, BSS sizes).  Every
difference and every borrowing is a residual; `compare --check` fails on any
residual that config/runlink-residuals.us.json does not list and on any listed
residual that has gone, so the manifest only ever ratchets down.

Subcommands
-----------
  ort [--write | --check]         generate the ORT name list
  pack [--out DIR]                write the packed tables under build/runlink/
  emit-metadata                  emit exact individual metadata bins and hashes
  compare [--check | --write-residuals] [--verbose] [--json PATH]
  region                          byte-compare the whole overlay region

Exit status: 0 success; 1 a gate failure (a difference or borrowing not in the
manifest, a stale manifest entry, a stale name list, a region mismatch outside
the residuals); 2 missing or unusable inputs.
"""

from __future__ import annotations

import argparse
import collections
import difflib
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import overlay_tables as ot  # noqa: E402

_LOADED_IMPLEMENTATIONS = {
    "tools/runlink_pack.py": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    "tools/overlay_tables.py": hashlib.sha256(Path(ot.__file__).read_bytes()).hexdigest(),
}

VERSION = "us"

# ---------------------------------------------------------------------------
# Constants

R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6
TYPE_NAMES = {R_MIPS_32: "R_MIPS_32", R_MIPS_26: "R_MIPS_26",
              R_MIPS_HI16: "R_MIPS_HI16", R_MIPS_LO16: "R_MIPS_LO16"}
OP_SYMBOL, OP_LOCAL, OP_JUMP, OP_DATA = 0, 1, 2, 3
OP_NAMES = {OP_SYMBOL: "SYMBOL", OP_LOCAL: "LOCAL", OP_JUMP: "JUMP", OP_DATA: "DATA"}

SEL_TEXT = 0
SEL_DATA1, SEL_DATA2, SEL_BSS = 0xFFD, 0xFFE, 0xFFF
OVERLAY_VMA = 0xF0000000

TRAP = "TrapDanglingJump"
# runlink.c resolves an unloaded data reference to this address; a resident
# HI16/LO16 that names it is a reference the main table patches.
SENTINEL = "D_800D2DC4"
# The runtime linker itself references both of the above as ordinary data.
RUNTIME_OBJECTS = ("build/src/main/runlink.c.o",)
# The runtime's section bases (runlink.c: overlayTable[0].vramBase is the
# first resident function, runlinkCodeEnd is the data base, then .bss).
RESIDENT_TEXT_BASE = ("main_TEXT_START",)
RESIDENT_DATA_BASE = ("runlinkCodeEnd", "D_80078D60")
RESIDENT_BSS_BASE = ("main_BSS_START",)
RESIDENT_END = ("main_BSS_END",)
RESIDENT_SECTIONS = (".main", ".main_bss", ".entry")

SHT_SYMTAB, SHT_RELA, SHT_NOBITS, SHT_REL = 2, 4, 8, 9
STT_SECTION, STT_FILE = 3, 4
STT_FUNC = 2
STB_LOCAL, STB_GLOBAL, STB_WEAK = 0, 1, 2
SHN_UNDEF, SHN_ABS, SHN_COMMON = 0, 0xFFF1, 0xFFF2

GEN_FUNC_RE = re.compile(r"^func_overlay_(\d{3})_F([0-9A-F]{7})(?:_[0-9A-F]+)?$")
GEN_DATA_RE = re.compile(r"^[DB]_overlay_(\d{3})_F([0-9A-F]{7})(?:_[0-9A-F]+)?$")
RES_AUTO_RE = re.compile(r"^(?:func|D)_(8[0-9A-F]{7})$")
ALIAS_RE = re.compile(r"^(.+)_o(\d{3})Reloc$")
# A short `D_<hex>` is a per-module placeholder; the same spelling in two
# overlays is two different things (CLAUDE.md, "A generically-named D_ symbol").
GENERIC_DATA_RE = re.compile(r"^D_[0-9A-Fa-f]{1,5}$")
ORT_SUFFIX_RE = re.compile(r"^(.+)_ort(\d+)$")
LINKER_NAME_RE = re.compile(
    r"(?:_(?:TEXT|DATA|RODATA|BSS)_(?:START|END|SIZE)|_VRAM(?:_END)?|_ROM_(?:START|END)"
    r"|_bss_VRAM)$|^_binary_|^\.|^\$")

DEFAULTS = {
    "atlas": "config/overlays.us.json",
    "ort": "config/runlink-ort.us.txt",
    "residuals": "config/runlink-residuals.us.json",
    "elf": "build/mickey.us.elf",
    "map": "build/mickey.us.map",
    "raw": "build/runlink_raw",
    "build": "build",
    "rom": "baseroms/mickey.us.z64",
    "out": "build/runlink",
}

# The overlay region starts at the main table; it ends where the last module
# image does (docs/overlays.md section 5).
REGION_START = ot.RELOC_TABLE_BASE
# The three table blocks have fixed homes in the yaml; the main table's block
# carries its own zero pad up to the ORT.
MAIN_BLOCK = ot.ROM_TABLE_BASE - ot.RELOC_TABLE_BASE


class PackError(RuntimeError):
    """Unusable inputs; exit status 2."""


def u32(data, off):
    return struct.unpack_from(">I", data, off)[0]


def sext16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


def hexs(v):
    return "0x%X" % v


# ---------------------------------------------------------------------------
# ELF32 big-endian reader


@dataclass(frozen=True)
class Symbol:
    index: int
    name: str
    value: int
    size: int
    bind: int
    type: int
    shndx: int
    section: str


@dataclass(frozen=True)
class Rel:
    offset: int
    type: int
    sym: Symbol


class Elf:
    def __init__(self, path, data=None):
        self.path = Path(path)
        d = self.data = data if data is not None else self.path.read_bytes()
        if d[:6] != b"\x7fELF\x01\x02":
            raise PackError(f"{path}: not a big-endian ELF32 file")
        shoff = u32(d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from(">HHH", d, 0x2E)
        raw = [struct.unpack_from(">10I", d, shoff + i * shentsize) for i in range(shnum)]
        strtab = raw[shstrndx]
        self.sections = []
        for i, h in enumerate(raw):
            name = self._str(strtab[4] + h[0])
            self.sections.append({"index": i, "name": name, "type": h[1], "flags": h[2],
                                  "addr": h[3], "offset": h[4], "size": h[5],
                                  "link": h[6], "info": h[7], "align": h[8]})
        self._by_name = {}
        for s in self.sections:
            self._by_name.setdefault(s["name"], s)
        self.symbols = []
        for s in self.sections:
            if s["type"] != SHT_SYMTAB:
                continue
            names = self.sections[s["link"]]["offset"]
            for j in range(s["size"] // 16):
                nm, val, sz, info, _oth, shndx = struct.unpack_from(
                    ">IIIBBH", d, s["offset"] + j * 16)
                self.symbols.append(Symbol(j, self._str(names + nm), val, sz, info >> 4,
                                           info & 0xF, shndx, self.secname(shndx)))
            break

    def _str(self, off):
        end = self.data.index(b"\0", off)
        return self.data[off:end].decode("latin-1")

    def secname(self, shndx):
        if shndx == SHN_UNDEF:
            return "UND"
        if shndx == SHN_ABS:
            return "ABS"
        if shndx == SHN_COMMON:
            return "COMMON"
        if shndx >= 0xFF00:
            return "SHN_%X" % shndx
        return self.sections[shndx]["name"]

    def section(self, name):
        return self._by_name.get(name)

    def size(self, name):
        s = self.section(name)
        return s["size"] if s else 0

    def bytes_of(self, name):
        s = self.section(name)
        if not s or s["type"] == SHT_NOBITS:
            return b""
        return self.data[s["offset"]:s["offset"] + s["size"]]

    def rels(self, target):
        """Relocations applying to section `target`, in file order."""
        out = []
        for s in self.sections:
            if s["type"] == SHT_RELA and self.sections[s["info"]]["name"] == target:
                raise PackError(f"{self.path}: RELA relocations are not supported")
            if s["type"] != SHT_REL or self.sections[s["info"]]["name"] != target:
                continue
            for j in range(s["size"] // 8):
                off, info = struct.unpack_from(">II", self.data, s["offset"] + j * 8)
                out.append(Rel(off, info & 0xFF, self.symbols[info >> 8]))
        return out


def in_place_addends(elf, secname, rels):
    """The REL addend of each relocation, as the static linker would read it.

    HI16 pairs with the next LO16 against the same symbol; a LO16 with no HI16
    of its own reuses the most recent one (the MIPS AHL rule)."""
    data = elf.bytes_of(secname)
    out = []
    last_hi = {}
    for i, r in enumerate(rels):
        w = u32(data, r.offset) if r.offset + 4 <= len(data) else 0
        if r.type == R_MIPS_26:
            a = (w & 0x3FFFFFF) << 2
        elif r.type == R_MIPS_32:
            a = w - 0x100000000 if w & 0x80000000 else w
        elif r.type == R_MIPS_HI16:
            hi = w & 0xFFFF
            lo = 0
            for nxt in rels[i + 1:]:
                if nxt.type == R_MIPS_LO16 and nxt.sym.index == r.sym.index:
                    lo = sext16(u32(data, nxt.offset))
                    break
            last_hi[r.sym.index] = hi
            a = (hi << 16) + lo
        elif r.type == R_MIPS_LO16:
            hi = last_hi.get(r.sym.index)
            a = ((hi << 16) if hi is not None else 0) + sext16(w)
        else:
            raise PackError(f"{elf.path}: unsupported relocation type {r.type} in {secname}")
        out.append(a)
    return out


# ---------------------------------------------------------------------------
# Link map


@dataclass(frozen=True)
class MapInput:
    outsec: str
    insec: str
    addr: int
    size: int
    file: str


def parse_map(path):
    """Every placed input section: (output section, input section, address, size, file)."""
    out = []
    started = False
    cur = None
    pending = None
    head = re.compile(r"^(\.\S+)(?:\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+))?")
    one = re.compile(r"^ (\.\S+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)\s*$")
    name_only = re.compile(r"^ (\.\S+)\s*$")
    cont = re.compile(r"^\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)\s*$")
    for line in Path(path).read_text(errors="replace").splitlines():
        if not started:
            started = line.startswith("Linker script and memory map")
            continue
        if line and not line[0].isspace():
            m = head.match(line)
            cur = m.group(1) if m else None
            pending = None
            continue
        if cur is None:
            continue
        m = one.match(line)
        if m:
            out.append(MapInput(cur, m.group(1), int(m.group(2), 16), int(m.group(3), 16), m.group(4)))
            pending = None
            continue
        m = name_only.match(line)
        if m:
            pending = m.group(1)
            continue
        m = cont.match(line)
        if m and pending:
            out.append(MapInput(cur, pending, int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
        pending = None
    return out


# ---------------------------------------------------------------------------
# Identities
#
# An identity is ("ov", module, offset) for an overlay address or ("res",
# address) for a resident one.  ORT rows map onto the same space.


def ident_add(ident, n):
    if ident is None:
        return None
    if ident[0] == "ov":
        return ("ov", ident[1], ident[2] + n)
    return ("res", (ident[1] + n) & 0xFFFFFFFF)


class ResidentLayout:
    def __init__(self, text, data, bss, end):
        self.text, self.data, self.bss, self.end = text, data, bss, end

    @classmethod
    def from_elf(cls, by_name):
        def pick(names):
            for n in names:
                for s in by_name.get(n, ()):
                    return s.value
            raise PackError(f"linked ELF lacks the runtime base symbol {names[0]}")
        return cls(pick(RESIDENT_TEXT_BASE), pick(RESIDENT_DATA_BASE),
                   pick(RESIDENT_BSS_BASE), pick(RESIDENT_END))

    def contains(self, addr):
        return 0x80000400 <= addr < self.end

    def row_identity(self, selector, offset):
        if selector == SEL_TEXT:
            return ("res", self.text + offset)
        if selector in (SEL_DATA1, SEL_DATA2):
            return ("res", self.data + offset)
        if selector == SEL_BSS:
            return ("res", self.bss + offset)
        return ("ov", selector, offset)

    def selector_of(self, addr):
        if addr < self.data:
            return SEL_TEXT, addr - self.text
        if addr < self.bss:
            return SEL_DATA1, addr - self.data
        return SEL_BSS, addr - self.bss


def generated_name(ident, layout, module_info):
    """The project's canonical name for an identity no symbol names."""
    if ident[0] == "res":
        addr = ident[1]
        sel, _ = layout.selector_of(addr)
        return ("func_%08X" if sel == SEL_TEXT else "D_%08X") % addr
    _, ov, off = ident
    info = module_info.get(ov)
    ts = info["text_size"] if info else 0
    ds = info["data_size"] if info else 0
    rom = info["rom_start"] if info else 0
    if off < ts:
        return "func_overlay_%03d_F%07X_%X" % (ov, off, rom + off)
    if off < ts + ds:
        return "D_overlay_%03d_F%07X_%X" % (ov, off, rom + off)
    return "B_overlay_%03d_F%07X" % (ov, off)


def parse_generated(name, layout):
    """Identity encoded in a generated name, or None."""
    m = ORT_SUFFIX_RE.match(name)
    if m:
        name = m.group(1)
    m = GEN_FUNC_RE.match(name) or GEN_DATA_RE.match(name)
    if m:
        return ("ov", int(m.group(1)), int(m.group(2), 16))
    m = RES_AUTO_RE.match(name)
    if m:
        addr = int(m.group(1), 16)
        if layout.contains(addr):
            return ("res", addr)
    return None


def is_generated(name):
    return bool(GEN_FUNC_RE.match(name) or GEN_DATA_RE.match(name) or RES_AUTO_RE.match(name)
                or ORT_SUFFIX_RE.match(name))


# ---------------------------------------------------------------------------
# The shipped tables


class Shipped:
    def __init__(self, rom):
        self.rom = rom
        self.count, self.main = ot.read_reloc_table(rom)
        self.ort = ot.read_rom_table(rom)
        self.headers = ot.read_headers(rom)
        self.modules = {m["overlay"]: m for m in ot.build_modules(self.headers)}
        self.records = {ov: ot.read_module_relocations(rom, m, self.ort)
                        for ov, m in self.modules.items()}

    def word(self, ov, off):
        return u32(self.rom, self.modules[ov]["rom_start"] + off)

    def rodata_base(self, ov):
        m = self.modules[ov]
        ts, ds = m["text_size"], m["data_size"]
        bases = {r["symbol_index"] for r in self.records[ov] if r["op"] == OP_LOCAL}
        extra = sorted(bases - {0, ts, ts + ds})
        if len(extra) > 1:
            raise PackError(f"overlay {ov}: more than one shipped rodata base")
        return extra[0] if extra else None

    def expected(self, ov, layout):
        """{(site, type): record + full target identity} for one module."""
        out = {}
        for table in (1, 2):
            rows = [r for r in self.records[ov] if r["table"] == table]
            last_hi = {}
            for i, r in enumerate(rows):
                site, t, op, idx = r["target_offset"], r["mode"], r["op"], r["symbol_index"]
                w = self.word(ov, site)
                if t == R_MIPS_HI16:
                    nxt = rows[i + 1] if i + 1 < len(rows) else None
                    lo = sext16(self.word(ov, nxt["target_offset"])) if nxt else 0
                    last_hi[(op, idx)] = w & 0xFFFF
                    a = ((w & 0xFFFF) << 16) + lo
                elif t == R_MIPS_LO16:
                    hi = last_hi.get((op, idx))
                    a = ((hi << 16) if hi is not None else 0) + sext16(w)
                elif t == R_MIPS_32:
                    a = (w - 0x100000000 if w & 0x80000000 else w) if op == OP_LOCAL else 0
                else:
                    a = 0
                if op == OP_JUMP:
                    full = ("ov", ov, (w & 0x3FFFFFF) << 2)
                elif op == OP_LOCAL:
                    full = ("ov", ov, idx + a)
                else:
                    row = self.ort[idx]
                    full = ident_add(layout.row_identity(row["overlay"], row["offset"]), a)
                key = (site, t)
                if key in out:
                    raise PackError(f"overlay {ov}: two shipped records at {hexs(site)}")
                out[key] = dict(r, full=full, position=i)
        return out


# ---------------------------------------------------------------------------
# The ORT name list


class OrtNames:
    """config/runlink-ort.us.txt: `<row> <name>` lines and `entry` lines."""

    def __init__(self, rows, entries):
        self.rows = rows            # index -> name
        self.entries = entries      # (overlay, "init"|"resume") -> name
        self.by_name = {}
        for i, n in enumerate(rows):
            if n in self.by_name:
                raise PackError(f"ORT name {n} names rows {self.by_name[n]} and {i}")
            self.by_name[n] = i

    @classmethod
    def load(cls, path):
        path = Path(path)
        if not path.exists():
            raise PackError(f"{path} is missing; run `gmake runlink-ort`")
        rows, entries = [], {}
        for ln, line in enumerate(path.read_text().splitlines(), 1):
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if parts[0] == "entry" and len(parts) == 4:
                entries[(int(parts[1]), parts[2])] = parts[3]
                continue
            if len(parts) != 2 or not parts[0].isdigit():
                raise PackError(f"{path}:{ln}: expected `<row> <name>`")
            if int(parts[0]) != len(rows):
                raise PackError(f"{path}:{ln}: row {parts[0]} out of order")
            rows.append(parts[1])
        return cls(rows, entries)

    def render(self):
        out = [ORT_HEADER]
        for i, n in enumerate(self.rows):
            out.append(f"{i} {n}")
        out.append("")
        out.append("# Module entry points: entry <overlay> init|resume <name>.")
        for (ov, kind), n in sorted(self.entries.items()):
            out.append(f"entry {ov} {kind} {n}")
        return "\n".join(out) + "\n"


ORT_HEADER = """\
# The overlay ROM table (ORT) as names: `<row> <name>`, one row per line in
# row order. A SYMBOL relocation record stores a row index; the packer
# (tools/runlink_pack.py) resolves each name to its (module, offset) from the
# build and emits the row. Rows no symbol names carry the project's canonical
# generated name (func_overlay_NNN_F..., D_/B_overlay_NNN_F..., func_8.../D_8...);
# a second row for one address takes an `_ort<row>` suffix.
#
# Generated by `gmake runlink-ort` from the shipped tables, the linked ELF's
# symbols and the overlay atlas. Names only. See docs/overlays.md "The runlink
# packer"."""


def best_name(cands):
    """Prefer a hand-given name over a generated one, a sized function, global."""
    def key(s):
        return (is_generated(s.name), not (s.type == STT_FUNC and s.size > 0),
                s.bind != STB_GLOBAL, s.name)

    return sorted(cands, key=key)


# ---------------------------------------------------------------------------
# The world: everything loaded once


class World:
    def __init__(self, paths, need_raw=True):
        self.paths = paths
        for k in ("atlas", "elf", "map", "rom"):
            if not paths[k].exists():
                hint = {"rom": "bring a baserom", "atlas": "check out the tree"}.get(
                    k, "build the tree first (gmake -j8)")
                raise PackError(f"{paths[k]} is missing; {hint}")
        self.atlas = json.loads(paths["atlas"].read_text())
        self.amods = {m["overlay"]: m for m in self.atlas["modules"]}
        self.shipped = Shipped(paths["rom"].read_bytes())
        self.elf = Elf(paths["elf"])
        self.by_name = collections.defaultdict(list)
        for s in self.elf.symbols:
            if s.name and s.type not in (STT_SECTION, STT_FILE):
                self.by_name[s.name].append(s)
        self.layout = ResidentLayout.from_elf(self.by_name)
        self.map = parse_map(paths["map"])
        self.module_info = {}
        self._linked_layout()
        self.need_raw = need_raw
        self._objects = None
        self._defs = None
        self._expected = {}

    def expected_cache(self, ov):
        """The shipped records of one module, keyed by (site, type), decoded once."""
        if ov not in self._expected:
            self._expected[ov] = (self.shipped.expected(ov, self.layout)
                                  if ov in self.shipped.modules else {})
        return self._expected[ov]

    # -- linked module layout (the "partial link" view of each module)

    def _linked_layout(self):
        per = collections.defaultdict(list)
        for mi in self.map:
            m = re.fullmatch(r"\.overlay_(\d{3})(_bss)?", mi.outsec)
            if m:
                per[(int(m.group(1)), bool(m.group(2)))].append(mi)
        for ov, am in self.amods.items():
            ins = per.get((ov, False), [])
            rom_start = int(am["rom"]["start"], 16)
            text_size = self._abs_or(f"overlay_{ov:03d}_TEXT_SIZE", None)
            if text_size is None:
                if int(am["sections"]["text"]["size"], 16) or ins:
                    raise PackError(f"linked ELF lacks overlay_{ov:03d}_TEXT_SIZE")
                text_size = 0   # an empty module has no segment at all
            reloc_starts = [mi.addr - OVERLAY_VMA for mi in ins if re.search(r"_reloc[12]\.bin\.o$", mi.file)]
            body = [mi for mi in ins if not re.search(r"_reloc[12]\.bin\.o$", mi.file)]
            end = max([mi.addr + mi.size - OVERLAY_VMA for mi in body] + [text_size])
            prefix = min(reloc_starts) if reloc_starts else end
            after = [mi for mi in body if mi.size and mi.addr - OVERLAY_VMA >= prefix]
            if after:
                raise PackError(f"overlay {ov}: {after[0].file} {after[0].insec} is linked after the relocation tables")
            bss_ins = per.get((ov, True), [])
            bss_start = self._abs_or(f"overlay_{ov:03d}_BSS_START", None)
            self.module_info[ov] = {
                "rom_start": rom_start, "text_size": text_size,
                "data_size": prefix - text_size, "prefix": prefix,
                "inputs": ins, "bss_inputs": bss_ins, "bss_vma": bss_start,
                "linked_bss": self._abs_or(f"overlay_{ov:03d}_BSS_SIZE", 0),
                "atlas_bss": int(am["bss_size"], 16),
            }

    def _abs(self, name):
        v = self._abs_or(name, None)
        if v is None:
            raise PackError(f"linked ELF lacks {name}")
        return v

    def _abs_or(self, name, default):
        for s in self.by_name.get(name, ()):
            return s.value
        return default

    def module_image_prefix(self, ov):
        """[text][data_rodata] of a module, from the linked ELF section."""
        info = self.module_info[ov]
        sec = self.elf.section(f".overlay_{ov:03d}")
        if sec is None:
            return b""
        data = self.elf.bytes_of(f".overlay_{ov:03d}")
        return data[:info["prefix"]]

    # -- raw objects

    def objects(self, ov):
        if self._objects is None:
            self._load_objects()
        return self._objects.get(ov, [])

    def _load_objects(self):
        if getattr(self, "_guarded_data_objects", None) is None:
            backend = (self.paths["build"].parent / "Makefile").is_file()
            guard_data_module_imports(self, OrtNames.load(self.paths["ort"]) if backend else None)
        admitted = self._guarded_data_objects
        if admitted is None:
            raise PackError("data-only producer inventory is incomplete")
        objects = {}
        raw = self.paths["raw"]
        missing = []
        for ov, am in sorted(self.amods.items()):
            lst = []
            for row in am["text_ownership"]:
                if row["type"] == "c":
                    p = raw / "src" / (row["source"] + ".c.o")
                else:
                    p = self.paths["build"] / "asm" / (row["source"] + ".s.o")
                    if not p.exists():
                        continue   # padding the C object's own section already covers
                if not p.exists():
                    missing.append(p)
                    continue
                lst.append(ObjRef(ov, p, row["source"], int(row["offset"], 16),
                                  int(row["size"], 16), row["type"], Elf(p)))
            lst.extend(admitted.get(ov, ()))
            objects[ov] = lst
        if missing:
            raise PackError(f"{len(missing)} raw object(s) missing, first {missing[0]}; "
                            "run `gmake runlink-raw`")
        self._objects = objects

    def defs(self):
        """Global symbols the raw overlay objects define: name -> [(obj, sym)]."""
        if self._defs is None:
            d = collections.defaultdict(list)
            for ov in sorted(self.amods):
                for o in self.objects(ov):
                    for s in o.elf.symbols:
                        if (s.name and s.bind in (STB_GLOBAL, STB_WEAK) and s.shndx != SHN_UNDEF
                                and s.shndx < 0xFF00 and s.type != STT_SECTION):
                            d[s.name].append((o, s))
            self._defs = d
        return self._defs

    # -- linked overlay symbol -> module offset

    def linked_overlay_offset(self, s):
        m = re.fullmatch(r"\.overlay_(\d{3})(_bss)?", s.section)
        if not m:
            return None
        ov = int(m.group(1))
        info = self.module_info.get(ov)
        if info is None:
            return None
        if m.group(2):
            if info["bss_vma"] is None:
                return None
            return ("ov", ov, s.value - info["bss_vma"] + info["prefix"])
        off = s.value - OVERLAY_VMA
        if 0 <= off < info["prefix"]:
            return ("ov", ov, off)
        return None


@dataclass
class ObjRef:
    module: int
    path: Path
    source: str
    text_offset: int
    text_size: int
    kind: str
    elf: Elf
    placement: dict = field(default_factory=dict)   # section -> (module offset, provenance)
    data_imports: dict | None = None  # authenticated input only: name -> actual provider identity

    @property
    def label(self):
        return self.source.split("/")[-1]


# ---------------------------------------------------------------------------
# ORT generation


def generate_ort(world):
    sh = world.shipped
    layout = world.layout
    # Candidate names per identity, from the linked ELF.
    cands = collections.defaultdict(list)
    for name, syms in world.by_name.items():
        if LINKER_NAME_RE.search(name):
            continue
        for s in syms:
            if s.section in RESIDENT_SECTIONS:
                if layout.contains(s.value):
                    cands[("res", s.value)].append(s)
            elif s.section.startswith(".overlay_"):
                ident = world.linked_overlay_offset(s)
                if ident:
                    cands[ident].append(s)
    used = collections.Counter()
    for recs in sh.records.values():
        for r in recs:
            if r["op"] == OP_SYMBOL:
                used[r["symbol_index"]] += 1
    for e in sh.main:
        used[e["rom_table_index"]] += 1
    groups = collections.defaultdict(list)
    for row in sh.ort:
        groups[layout.row_identity(row["overlay"], row["offset"])].append(row["index"])
    names = [None] * len(sh.ort)
    taken = set()
    for ident, rows in groups.items():
        rows = sorted(rows, key=lambda i: (-used[i], i))
        pool = [s.name for s in best_name(cands.get(ident, []))]
        pool = list(dict.fromkeys(pool))
        for i in rows:
            n = pool.pop(0) if pool else generated_name(ident, layout, world.module_info)
            if n in taken:
                n = f"{n}_ort{i}"
            taken.add(n)
            names[i] = n
    entries = {}
    for ov, m in sh.modules.items():
        for kind, field_name in (("init", "init_function"), ("resume", "resume_function")):
            off = m[field_name]
            if off == -1:
                continue
            ident = ("ov", ov, off)
            pool = [s.name for s in best_name(cands.get(ident, []))]
            entries[(ov, kind)] = pool[0] if pool else generated_name(ident, layout, world.module_info)
    return OrtNames(names, entries)


# ---------------------------------------------------------------------------
# Resolving a relocation's symbol


@dataclass
class Target:
    kind: str                 # "section" (an object section), "ident", or "placeholder"
    name: str
    route: str
    ident: tuple = None       # base identity for kind "ident"
    obj: ObjRef = None        # for kind "section"
    secname: str = ""
    value: int = 0            # symbol value inside that section
    named: bool = False       # a named symbol, which may be exported


class Resolver:
    def __init__(self, world, ort):
        self.w = world
        self.ort = ort
        self.layout = world.layout
        self._ort_ident = None
        self._ort_inventory = object()
        self._held_ort_targets = {}
        self._name_cache = {}

    def _sync_provider_inventory(self):
        """Namespace caches cannot outlive their atomically held providers."""
        if getattr(self.w, "_guarded_data_objects", False) is None:
            raise PackError("ORT provider inventory is unavailable or being revalidated")
        if self.w.need_raw:
            self.w.defs()  # existing lazy discovery completes before any cached return
        inventory = getattr(self.w, "_guarded_data_objects", None)
        if self._ort_inventory is inventory:
            return
        targets = {}
        for objects in (inventory or {}).values():
            for obj in objects:
                for rel in obj.elf.rels(".data"):
                    if rel.type != R_MIPS_32 or rel.sym.shndx != SHN_UNDEF:
                        continue
                    name = rel.sym.name
                    if (name not in self.ort.by_name or obj.data_imports is None
                            or name not in obj.data_imports):
                        raise PackError(f"ORT lacks its held provider identity: {name}")
                    ident = ("ov", *obj.data_imports[name])
                    if name in targets and targets[name] != ident:
                        raise PackError(f"ORT has ambiguous held provider identities: {name}")
                    targets[name] = ident
        self._ort_ident = None
        self._name_cache.clear()
        if hasattr(self, "_rbi"):
            del self._rbi
        self._held_ort_targets = targets
        self._ort_inventory = inventory

    # name -> identity for the ORT rows (the packer's ORT)
    def ort_identity(self, row):
        self._sync_provider_inventory()
        if self._ort_ident is None:
            self._ort_ident = [(self._held_ort_targets[n] if n in self._held_ort_targets
                                else self.name_identity(n)) for n in self.ort.rows]
        return self._ort_ident[row]

    def rows_by_ident(self):
        self._sync_provider_inventory()
        if not hasattr(self, "_rbi"):
            d = collections.defaultdict(list)
            for i in range(len(self.ort.rows)):
                ident = self.ort_identity(i)
                if ident:
                    d[ident].append(i)
            self._rbi = d
        return self._rbi

    def name_identity(self, name, from_ov=None):
        """Identity a name stands for, using every source except the shipped tables."""
        t = self.resolve_name(name, from_ov)
        return self.target_base(t)

    def target_base(self, t):
        if t.kind == "ident":
            return t.ident
        if t.kind == "section":
            off = section_offset(t.obj, t.secname)
            if off is None:
                return None
            return ("ov", t.obj.module, off + t.value)
        return None

    def resolve_name(self, name, from_ov=None):
        self._sync_provider_inventory()
        key = (name, from_ov)
        if key in self._name_cache:
            return self._name_cache[key]
        t = self._resolve_name(name, from_ov)
        self._name_cache[key] = t
        return t

    def _resolve_name(self, name, from_ov):
        w = self.w
        if w.need_raw:
            defs = w.defs().get(name, [])
            if defs:
                local = [d for d in defs if d[0].module == from_ov]
                pick = local or ([] if GENERIC_DATA_RE.match(name) else defs)
                if len(pick) == 1:
                    o, s = pick[0]
                    return Target("section", name, "overlay-definition", obj=o,
                                  secname=s.section, value=s.value, named=True)
        ident = parse_generated(name, self.layout)
        if ident:
            return Target("ident", name, "generated-name", ident=ident, named=True)
        # The surface's call proxy for one generated function: `<generated>Reloc`.
        if name.endswith("Reloc") and GEN_FUNC_RE.match(name[:-5]):
            definition = self._function_proxy_definition(name[:-5])
            if definition is not None:
                obj, symbol = definition
                return Target("section", name, "source-function-proxy", obj=obj,
                              secname=".text", value=symbol.value, named=True)
            # No declaration witness: explicitly retain the compatibility route.
            return Target("ident", name, "generated-proxy",
                          ident=parse_generated(name[:-5], self.layout), named=True)
        m = ALIAS_RE.match(name)
        if m:
            inner = self.resolve_name(m.group(1), from_ov)
            if inner.kind != "placeholder":
                inner = Target(inner.kind, name, "surface-alias:" + inner.route, ident=inner.ident,
                               obj=inner.obj, secname=inner.secname, value=inner.value, named=True)
                return inner
        for s in w.by_name.get(name, ()):
            if s.section in RESIDENT_SECTIONS and w.layout.contains(s.value):
                return Target("ident", name, "resident-symbol", ident=("res", s.value), named=True)
        for s in w.by_name.get(name, ()):
            if s.section == "ABS" and w.layout.contains(s.value):
                return Target("ident", name, "resident-address", ident=("res", s.value), named=True)
        for s in w.by_name.get(name, ()):
            ident = w.linked_overlay_offset(s)
            if ident and not LINKER_NAME_RE.search(name):
                return Target("ident", name, "linked-overlay", ident=ident, named=True)
        return Target("placeholder", name, "placeholder")

    def _function_proxy_definition(self, name):
        """An exact-name function witness; no coordinates come from its spelling."""
        choices = self.w.defs().get(name, []) if self.w.need_raw else []
        if not choices:
            return None
        if len(choices) != 1:
            raise PackError(f"ambiguous function proxy provider: {name}")
        obj, symbol = choices[0]
        raw = obj.elf.section(".text")
        if (symbol.bind != STB_GLOBAL or symbol.type != STT_FUNC or symbol.section != ".text"
                or not raw or raw["type"] != 1 or raw["flags"] & 6 != 6 or raw["addr"] != 0
                or symbol.size <= 0 or symbol.value < 0
                or symbol.value % 4 or symbol.size % 4
                or symbol.value + symbol.size > raw["size"]
                or len(obj.elf.bytes_of(".text")) != raw["size"]):
            raise PackError(f"function proxy lacks a bounded real function: {name}")
        proc = (f"build/src/{obj.source}.c.o" if obj.kind == "c"
                else f"build/asm/{obj.source}.s.o")
        info = self.w.module_info[obj.module]
        inputs = [mi for mi in info["inputs"] if mi.file == proc and mi.insec == ".text" and mi.size]
        final = [s for s in self.w.by_name.get(name, ())
                 if s.bind == STB_GLOBAL and s.shndx != SHN_UNDEF]
        if (len(inputs) != 1 or inputs[0].outsec != f".overlay_{obj.module:03d}"
                or inputs[0].addr - OVERLAY_VMA != obj.text_offset
                or symbol.value + symbol.size > inputs[0].size
                or len(final) != 1 or final[0].type != STT_FUNC
                or final[0].section != inputs[0].outsec
                or final[0].value != inputs[0].addr + symbol.value
                or final[0].size != symbol.size):
            raise PackError(f"function proxy raw/final input geometry differs: {name}")
        start, end = final[0].value, final[0].value + symbol.size
        if any(mi is not inputs[0] and mi.size and mi.addr < end and start < mi.addr + mi.size
               for mi in info["inputs"]):
            raise PackError(f"function proxy has overlapping final input ownership: {name}")
        return obj, symbol

    def export_row(self, t, base):
        """The ORT row a named target is exported through, or None."""
        if not t.named or base is None:
            return None
        r = self.ort.by_name.get(t.name)
        if r is not None and self.ort_identity(r) == base:
            return r
        rows = self.rows_by_ident().get(base, [])
        return rows[0] if rows else None

    def name_of(self, ident):
        """A readable name for an identity: an ORT row's, a symbol's, or generated."""
        self._sync_provider_inventory()
        if ident is None:
            return "?"
        if ident in self._name_cache:
            return self._name_cache[ident]
        rows = self.rows_by_ident().get(ident)
        if rows:
            n = self.ort.rows[rows[0]]
        else:
            n = self._symbol_name(ident) or generated_name(ident, self.layout, self.w.module_info)
        self._name_cache[ident] = n
        return n

    def _symbol_name(self, ident):
        if not hasattr(self, "_sym_index"):
            idx = collections.defaultdict(list)
            for name, syms in self.w.by_name.items():
                if LINKER_NAME_RE.search(name):
                    continue
                for s in syms:
                    if s.section in RESIDENT_SECTIONS:
                        idx[("res", s.value)].append(s)
                    else:
                        i2 = self.w.linked_overlay_offset(s)
                        if i2:
                            idx[i2].append(s)
            self._sym_index = idx
        c = self._sym_index.get(ident)
        return best_name(c)[0].name if c else None


def section_offset(obj, secname):
    if secname == ".text":
        return obj.text_offset
    p = obj.placement.get(secname)
    return p[0] if p else None


# ---------------------------------------------------------------------------
# Packing one module


@dataclass
class CReloc:
    obj: ObjRef
    section: str
    offset: int
    type: int
    sym: Symbol
    addend: int
    target: Target

    def site(self):
        base = section_offset(self.obj, self.section)
        return None if base is None else base + self.offset


@dataclass
class Record:
    table: int
    op: int
    symidx: int
    site: int
    type: int
    full: tuple = None
    name: str = ""
    provenance: str = "object"   # "object", "placeholder", "hybrid-data"
    route: str = ""
    obj: str = ""

    @property
    def key(self):
        return (self.site, self.type)

    def encode(self):
        return struct.pack(">II", self.symidx & 0xFFFFFFFF,
                           ((self.site & 0xFFFFFF) << 8) | (self.type << 4) | self.op)


@dataclass
class PackedModule:
    overlay: int
    table1: list
    table2: list
    text_size: int
    data_size: int
    bss_size: int
    rodata_base: int
    init: int
    resume: int
    image_prefix: bytes
    notes: list
    borrowed: dict          # category -> list of entries
    unresolved: list        # C relocations that produced no record

    def reloc_bytes(self, table):
        return b"".join(r.encode() for r in (self.table1 if table == 1 else self.table2))


def collect_relocs(world, resolver, obj):
    out = []
    for secname in (".text", ".rodata", ".data"):
        if obj.kind != "c" and secname != ".text":
            continue
        rels = obj.elf.rels(secname)
        if not rels:
            continue
        adds = in_place_addends(obj.elf, secname, rels)
        for r, a in zip(rels, adds):
            s = r.sym
            if s.type == STT_SECTION:
                t = Target("section", s.section, "section", obj=obj, secname=s.section, value=0)
            elif s.shndx != SHN_UNDEF and s.shndx < 0xFF00:
                t = Target("section", s.name,
                           "object-local" if s.bind == STB_LOCAL else "object-definition",
                           obj=obj, secname=s.section, value=s.value, named=s.bind != STB_LOCAL)
            elif s.shndx == SHN_COMMON:
                t = Target("placeholder", s.name, "common")
            elif obj.data_imports is not None:
                if s.name not in obj.data_imports:
                    raise PackError(f"admitted data input lacks its actual import identity: {obj.source}: {s.name}")
                t = Target("ident", s.name, "data-source-export",
                           ident=("ov", *obj.data_imports[s.name]), named=True)
            else:
                t = resolver.resolve_name(s.name, obj.module)
            out.append(CReloc(obj, secname, r.offset, r.type, s, a, t))
    return out


def linked_bss_output(world, ov):
    """Authenticate the allocated BSS output independently of retail size."""
    info = world.module_info[ov]
    output = world.elf.section(f".overlay_{ov:03d}_bss")
    if (not output or output["type"] != SHT_NOBITS
            or output["flags"] & 3 != 3 or output["flags"] & 4
            or output["addr"] != info["bss_vma"]
            or output["addr"] != OVERLAY_VMA + info["prefix"]
            or output["size"] != info["linked_bss"]):
        return None
    return output


def linked_bss_placement(world, obj):
    """Accept an actual BSS input, never a whole-module guess for a raw object.

    Legacy objects may emit duplicate BSS which their configured rules discard
    or shorten. Such raw sections have no faithful linked input and must keep
    their named borrowing. The final typed section and input map establish
    ownership only when the complete raw section survives at the runtime body's
    BSS boundary, with the exact declared extent and no overlapping contributor.
    """
    info = world.module_info[obj.module]
    raw = obj.elf.section(".bss")
    output = linked_bss_output(world, obj.module)
    if not raw or not output or not raw["size"]:
        return None
    if (raw["type"] != SHT_NOBITS or raw["flags"] & 3 != 3
            or raw["flags"] & 4):
        return None
    inputs = [mi for mi in info["bss_inputs"] if mi.size]
    source = f"build/src/{obj.source}.c.o"
    owned = [mi for mi in inputs if mi.file == source and mi.insec == ".bss"]
    if len(owned) != 1:
        return None
    item = owned[0]
    end = item.addr + item.size
    if (item.size != raw["size"] or item.addr < output["addr"]
            or end > output["addr"] + output["size"]
            or any(other is not item and other.addr < end
                   and item.addr < other.addr + other.size for other in inputs)):
        return None
    return item.addr - OVERLAY_VMA


def header_bss(world, ov):
    """(size, source-owned): use faithful source layout or explicit atlas fallback.

    A matching sum is insufficient: discarded/trimmed/duplicate raw producers
    can have the same size as a retained reserve. Every nonempty map input must
    have exactly one raw C owner, and every raw allocated BSS section must
    survive. The initialized body must also be completely source-produced:
    a faithful partial BSS allocation beside retained data cannot establish
    that the source accounts for the module's entire storage contract.
    Only ordinary input alignment may separate contributors; neither
    a configured trailing reserve nor an unexplained gap establishes ownership.
    Atlas size is a comparison/fallback value, never the size of proved BSS.
    """
    info = world.module_info[ov]
    fallback = (info["atlas_bss"], False)
    objects = world.objects(ov)
    inputs = [mi for mi in info["bss_inputs"] if mi.size]
    owners = []
    for obj in objects:
        if any(s.shndx == SHN_COMMON and s.size for s in obj.elf.symbols):
            return fallback
        if any(s["size"] and s["flags"] & 2 and s["type"] == SHT_NOBITS
               and s["name"] != ".bss" for s in obj.elf.sections):
            return fallback
        if obj.elf.size(".bss"):
            if obj.kind != "c" or linked_bss_placement(world, obj) is None:
                return fallback
            owners.append(obj)
    if not owners:
        # No source allocation cannot disprove an existing retail reserve.
        # Zero-sized output sections need not have NOBITS/ALLOC flags in GNU ld.
        output = world.elf.section(f".overlay_{ov:03d}_bss")
        if (info["atlas_bss"] or info["linked_bss"] or inputs
                or (output and (output["size"]
                    or output["addr"] != info["bss_vma"]
                    or output["addr"] != OVERLAY_VMA + info["prefix"]))):
            return fallback
        return 0, True
    output = linked_bss_output(world, ov)
    if not output or len(inputs) != len(owners):
        return fallback
    body = {}
    for obj in objects:
        if obj.kind != "c":
            return fallback
        if any(s["size"] and s["flags"] & 2 and s["name"] not in
               (".text", ".data", ".rodata", ".bss", ".reginfo", ".MIPS.abiflags")
               for s in obj.elf.sections):
            return fallback
        for section in (".text", ".data", ".rodata"):
            raw = obj.elf.section(section)
            if raw and raw["size"]:
                key = (f"build/src/{obj.source}.c.o", section)
                if key in body:
                    return fallback
                body[key] = raw
    cursor = OVERLAY_VMA
    for item in sorted(info["inputs"], key=lambda mi: mi.addr):
        if not item.size or re.search(r"_reloc[12]\.bin\.o$", item.file):
            continue
        raw = body.pop((item.file, item.insec), None)
        if (raw is None or not raw["flags"] & 2 or raw["type"] == SHT_NOBITS
                or raw["size"] != item.size):
            return fallback
        alignment = raw["align"]
        if (alignment < 1 or alignment & (alignment - 1)
                or item.addr != ((cursor + alignment - 1) & -alignment)):
            return fallback
        cursor = item.addr + item.size
    if body or cursor != OVERLAY_VMA + info["prefix"]:
        return fallback
    by_source = {f"build/src/{obj.source}.c.o": obj for obj in owners}
    if len(by_source) != len(owners):
        return fallback
    cursor = output["addr"]
    for item in sorted(inputs, key=lambda mi: mi.addr):
        obj = by_source.pop(item.file, None)
        if obj is None or item.insec != ".bss":
            return fallback
        alignment = obj.elf.section(".bss")["align"]
        if alignment < 1 or alignment & (alignment - 1):
            return fallback
        if item.addr != ((cursor + alignment - 1) & -alignment):
            return fallback
        cursor = item.addr + item.size
    if by_source or cursor != output["addr"] + output["size"]:
        return fallback
    return output["size"], True


def place_sections(world, resolver, relocs_by_module):
    """Decide where each object's .data/.rodata/.bss sits in its module.

    The build's own placement (the linked input section, or an atlas
    data-ownership row) wins; otherwise the section is placed from the shipped
    tables by the references into it, and recorded as borrowed."""
    evidence = collections.defaultdict(collections.Counter)
    for ov, relocs in relocs_by_module.items():
        exp = world.expected_cache(ov)
        for c in relocs:
            if c.section != ".text":
                continue
            t = c.target
            if t.kind != "section" or t.secname == ".text":
                continue
            e = exp.get((c.obj.text_offset + c.offset, c.type))
            if not e or e["full"] is None or e["full"][0] != "ov" or e["full"][1] != t.obj.module:
                continue
            evidence[(t.obj.path, t.secname)][e["full"][2] - t.value - c.addend] += 1
    borrowed = collections.defaultdict(list)
    for ov in sorted(relocs_by_module):
        info = world.module_info[ov]
        am = world.amods[ov]
        atlas_rows = {}
        for row in am.get("data_rodata_ownership", []):
            if row.get("externalized"):
                continue   # the range a normalization moved out, not a section start
            atlas_rows[(row["source"], row.get("section", ".data"))] = (
                int(row["offset"], 16), int(row["size"], 16))
        linked = {}
        for mi in info["inputs"]:
            if mi.insec in (".data", ".rodata") and mi.size and mi.addr - OVERLAY_VMA >= info["text_size"]:
                linked[(mi.file, mi.insec)] = mi.addr - OVERLAY_VMA
        for o in world.objects(ov):
            for secname in (".data", ".rodata", ".bss"):
                if not o.elf.size(secname):
                    continue
                proc = f"build/src/{o.source}.c.o"
                if (proc, secname) in linked:
                    o.placement[secname] = (linked[(proc, secname)], "link")
                    continue
                if secname == ".bss":
                    offset = linked_bss_placement(world, o)
                    if offset is not None:
                        o.placement[secname] = (offset, "link")
                        continue
                row = atlas_rows.get((o.source, secname))
                # A row that covers the whole section, short of its trailing
                # alignment pad, places it; a row for part of a section (one
                # externalized table among other literals) does not.
                if row and 0 <= o.elf.size(secname) - row[1] < 16:
                    o.placement[secname] = (info["text_size"] + row[0], "atlas")
                    continue
                ev = evidence.get((o.path, secname))
                if ev:
                    p, n = ev.most_common(1)[0]
                    o.placement[secname] = (p, "borrowed")
                    borrowed["placement"].append([ov, o.label, secname, hexs(p), n])
                    continue
                p = place_by_data_sites(world, o, secname)
                if p is None:
                    p = place_by_content(world, o, secname)
                if p is not None:
                    o.placement[secname] = (p, "borrowed")
                    borrowed["placement"].append([ov, o.label, secname, hexs(p), 0])
    return borrowed


def place_by_content(world, o, secname):
    """Place a section by its own bytes inside the module's linked data image.

    Relocated words are masked and trailing zero words dropped; only a unique
    match inside the data region counts."""
    data = o.elf.bytes_of(secname)
    if not data:
        return None
    masked = {r.offset for r in o.elf.rels(secname)}
    words = [None if i in masked else u32(data, i) for i in range(0, len(data) - len(data) % 4, 4)]
    while words and words[-1] == 0:
        words.pop()
    if not any(w for w in words if w):
        return None
    info = world.module_info[o.module]
    image = world.module_image_prefix(o.module)
    ts = info["text_size"]
    hits = []
    for p in range(ts, len(image) - 4 * len(words) + 1, 4):
        if all(w is None or u32(image, p + 4 * k) == w for k, w in enumerate(words)):
            hits.append(p)
            if len(hits) > 1:
                return None
    return hits[0] if hits else None


def place_by_data_sites(world, o, secname):
    """Place a section with relocations of its own onto the shipped data-site records."""
    rels = o.elf.rels(secname) if secname != ".bss" else []
    if not rels:
        return None
    ov = o.module
    exp = world.expected_cache(ov)
    ts = world.module_info[ov]["text_size"]
    sites = {(k[0], k[1]) for k in exp if k[0] >= ts}
    counts = collections.Counter()
    for (s, t) in sites:
        for r in rels:
            if r.type == t:
                counts[s - r.offset] += 1
    if not counts:
        return None
    best = counts.most_common(2)
    if len(best) > 1 and best[0][1] == best[1][1]:
        return None
    return best[0][0]


def pack_module(world, resolver, ov, relocs):
    sh = world.shipped
    info = world.module_info[ov]
    ts, ds = info["text_size"], info["data_size"]
    exp = world.expected_cache(ov)
    ship_rodata = sh.rodata_base(ov) if ov in sh.modules else None
    notes = []
    borrowed = collections.defaultdict(list)
    c_rodata = [o.placement[".rodata"][0] for o in world.objects(ov) if ".rodata" in o.placement]
    if ship_rodata is not None:
        rodata0 = ship_rodata
        if not c_rodata or min(c_rodata) != ship_rodata:
            borrowed["rodata-base"].append([ov])
    else:
        rodata0 = ts
    bss_base = ts + ds

    def local_base(kind):
        return {"text": 0, ".text": 0, ".data": ts, ".rodata": rodata0, ".bss": bss_base}[kind]

    def kind_of_offset(off):
        if off < ts:
            return ".text"
        if off >= bss_base:
            return ".bss"
        return ".rodata" if off >= rodata0 and rodata0 > ts else ".data"

    text_recs, data_recs, unresolved = [], [], []
    for c in relocs:
        site = c.site()
        if site is None:
            unresolved.append((c, "unplaced-site"))
            continue
        t = c.target
        e = exp.get((site, c.type))
        if t.kind == "placeholder":
            if e is None:
                unresolved.append((c, "placeholder-without-record"))
                continue
            rec = Record(e["table"], e["op"], e["symbol_index"], site, c.type, e["full"],
                         t.name, "placeholder", t.route, c.obj.label)
        else:
            base = resolver.target_base(t)
            if base is None:
                unresolved.append((c, "unplaced-target"))
                continue
            full = ident_add(base, c.addend)
            row = resolver.export_row(t, base)
            name = t.name if t.named else resolver.name_of(full)
            if base[0] == "ov" and base[1] == ov:
                if row is not None:
                    rec = Record(2, OP_SYMBOL, row, site, c.type, full, name)
                elif c.type == R_MIPS_26:
                    rec = Record(2, OP_JUMP, 0, site, c.type, full, name)
                else:
                    kind = t.secname if t.kind == "section" else kind_of_offset(base[2])
                    rec = Record(2, OP_LOCAL, local_base(kind), site, c.type, full, name)
            else:
                if row is None:
                    unresolved.append((c, "unexported-target"))
                    rec = Record(1, OP_SYMBOL, -1, site, c.type, full, name)
                else:
                    rec = Record(1, OP_SYMBOL, row, site, c.type, full, name)
            rec.route, rec.obj = t.route, c.obj.label
            if rec.op == OP_SYMBOL and c.type in (R_MIPS_26, R_MIPS_32) and c.addend:
                notes.append(f"{c.obj.label}+{hexs(c.offset)}: SYMBOL {TYPE_NAMES[c.type]} with addend {c.addend}")
        (text_recs if c.section == ".text" else data_recs).append(rec)
    # Blob-owned data: the shipped data-site records no object produces.
    produced = {r.key for r in text_recs + data_recs}
    owned = []
    for o in world.objects(ov):
        for secname in (".data", ".rodata"):
            if secname in o.placement:
                owned.append((o.placement[secname][0], o.placement[secname][0] + o.elf.size(secname)))
    for key, e in exp.items():
        site = key[0]
        if key in produced or site < ts:
            continue
        if any(a <= site < b for a, b in owned):
            continue
        data_recs.append(Record(e["table"], e["op"], e["symbol_index"], site, key[1], e["full"],
                                resolver.name_of(e["full"]), "hybrid-data", "shipped", "blob"))
        borrowed["hybrid-data"].append([ov, hexs(site)])
    rank = lambda r: (0 if r.site >= rodata0 else 1, r.site)  # noqa: E731
    data_recs.sort(key=rank)
    ordered = text_recs + data_recs
    t1 = [r for r in ordered if r.table == 1]
    t2 = [r for r in ordered if r.table == 2]
    last = 0
    for r in t1 + t2:
        if r.op == OP_SYMBOL:
            last = r.symidx
        elif r.op == OP_JUMP and r.provenance == "object":
            r.symidx = last
    init = resume = -1
    for kind in ("init", "resume"):
        n = resolver.ort.entries.get((ov, kind))
        if n is None:
            continue
        ident = resolver.name_identity(n, ov)
        if not ident or ident[0] != "ov" or ident[1] != ov:
            notes.append(f"entry {kind} {n} does not resolve inside the module")
            continue
        if kind == "init":
            init = ident[2]
        else:
            resume = ident[2]
    bss, source_bss = header_bss(world, ov)
    if not source_bss:
        borrowed["bss-size"].append([ov])
    return PackedModule(ov, t1, t2, ts, ds, bss, rodata0, init, resume,
                        world.module_image_prefix(ov), notes, dict(borrowed), unresolved)


# ---------------------------------------------------------------------------
# The main relocation table


@dataclass
class MainEntry:
    row: int
    offset: int
    type: int
    op: int
    site: int                 # absolute address of the patched word
    name: str
    provenance: str = "object"
    obj: str = ""

    def encode(self):
        return struct.pack(">II", self.row & 0xFFFFFFFF,
                           ((self.offset & 0xFFFFFF) << 8) | (self.type << 4) | self.op)


def pack_main(world, resolver):
    layout = world.layout
    text_entries, data_entries, borrowed, problems = [], [], collections.defaultdict(list), []
    seen = set()
    resident_in = [mi for mi in world.map if mi.outsec in (".main", ".entry")
                   and mi.insec in (".text", ".data") and mi.size]
    ship_by_site = {}
    for e in world.shipped.main:
        if (e["flags"] & 0xF) == OP_DATA:
            ship_by_site[layout.data + e["call_site_offset"]] = e
        else:
            ship_by_site[layout.text + e["call_site_offset"]] = e
    for mi in resident_in:
        path = world.paths["build"].parent / mi.file
        if mi.file in seen and mi.insec == ".text":
            continue
        if not path.exists() or not mi.file.endswith(".o"):
            continue
        elf = Elf(path)
        rels = elf.rels(mi.insec)
        if not rels:
            continue
        sidecar = {}
        if mi.insec == ".text" and mi.file.endswith(".c.o"):
            sc = Path(str(path) + ".trapsites")
            if sc.exists():
                rec = json.loads(sc.read_text())
                if rec.get("text_sha256") != hashlib.sha256(elf.bytes_of(".text")).hexdigest():
                    problems.append(f"{mi.file}: stale .trapsites sidecar (rebuild the object)")
                sidecar = {s["offset"]: s["name"] for s in rec.get("sites", [])}
        for r in rels:
            name = r.sym.name
            addr = mi.addr + r.offset
            if mi.insec == ".text" and r.type == R_MIPS_26 and name == TRAP:
                callee = sidecar.get(r.offset)
                ent = MainEntry(-1, addr - layout.text, r.type, OP_SYMBOL, addr, callee or TRAP,
                                obj=mi.file)
                if callee is None:
                    ent.provenance = "placeholder"
                else:
                    ident = resolver.name_identity(callee)
                    row = resolver.ort.by_name.get(callee)
                    if row is None and ident is not None:
                        rows = resolver.rows_by_ident().get(ident, [])
                        row = rows[0] if rows else None
                    if row is None:
                        problems.append(f"{mi.file}+{hexs(r.offset)}: trap callee {callee} has no ORT row")
                        ent.provenance = "placeholder"
                    else:
                        ent.row = row
                text_entries.append(ent)
            elif (mi.insec == ".text" and r.type in (R_MIPS_HI16, R_MIPS_LO16) and name == SENTINEL
                  and mi.file not in RUNTIME_OBJECTS):
                text_entries.append(MainEntry(-1, addr - layout.text, r.type, OP_SYMBOL, addr,
                                              SENTINEL, "placeholder", mi.file))
            elif mi.insec == ".data" and r.type == R_MIPS_32 and name == TRAP:
                data_entries.append(MainEntry(-1, addr - layout.data, r.type, OP_DATA, addr,
                                              TRAP, "placeholder", mi.file))
        if mi.insec == ".text":
            seen.add(mi.file)
    entries = text_entries + data_entries
    for ent in entries:
        if ent.provenance == "placeholder":
            e = ship_by_site.get(ent.site)
            if e is not None:
                ent.row = e["rom_table_index"]
                ent.name = resolver.ort.rows[ent.row]
            borrowed["main-placeholder"].append([hexs(ent.offset), OP_NAMES[ent.op], TYPE_NAMES[ent.type]])
    return entries, dict(borrowed), problems


# ---------------------------------------------------------------------------
# Packing everything


@dataclass
class Packed:
    modules: dict
    main: list
    ort_words: list
    borrowed: dict
    problems: list
    resolver: object = None
    relocs: dict = None


def projected_data_rows(module):
    """Default YAML ownership: leading C data or explicit canonical fixed rows."""
    end = 0
    for row in module.get("data_rodata_ownership", []):
        normal = row.get("type") == "c" and not row.get("externalized")
        leading = (normal and "section" not in row and "trial_function" not in row
                   and int(row["offset"], 16) == end)
        fixed = (normal and row.get("canonical") is True
                 and row.get("section") in (".data", ".rodata"))
        if leading:
            end = int(row["end_offset"], 16)
        if leading or fixed:
            yield row


def hold_data_import_inputs(world, inputs):
    """Merge held observations without ever blessing a newer conflicting digest."""
    snapshot = getattr(world, "data_import_inputs", {})
    for path, digest in inputs.items():
        if path in snapshot and snapshot[path] != digest:
            raise PackError(f"conflicting held data-only input: {path}")
        snapshot[path] = digest
    world.data_import_inputs = snapshot


def remember_data_import_inputs(world, source):
    root = world.paths["build"].parent
    files = (world.paths["raw"] / "src" / (source + ".c.o"),
             world.paths["build"] / "src" / (source + ".c.o"),
             root / "src" / (source + ".c"))
    hold_data_import_inputs(world, {path: hashlib.sha256(path.read_bytes()).hexdigest()
                                    for path in files})


def hold_data_capture(world, source, receipt, context):
    """Keep every receipt's original context until the complete guard finishes."""
    import copy
    root = world.paths["build"].parent
    expected = copy.deepcopy(receipt["inputs"])
    held = getattr(world, "data_import_contexts", [])
    held.append((expected, context))
    world.data_import_contexts = held
    for group in ("dependencies", "makefiles", "module_link_inputs", "metadata_tools"):
        hold_data_import_inputs(world, {root / name: digest
                                        for name, digest in expected.get(group, {}).items()})
    hold_data_import_inputs(world, {
        root / "src" / (source + ".c"): expected["source"],
        world.paths["build"] / "src" / (source + ".c.o"): expected["configured"],
    })
    hold_data_import_inputs(world, {root / name: digest for name, digest
                                    in expected["generated_metadata"]["inputs"].items()})


def check_data_import_holds(world):
    for expected, context in getattr(world, "data_import_contexts", []):
        if expected != context():
            raise PackError("held data-only capture context changed after validation")
    for path, digest in getattr(world, "data_import_inputs", {}).items():
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            raise PackError(f"held data-only input changed after validation: {path}")


def data_import_input(world, ov, row, source, raw):
    """Authenticate one complete data input; no inferred placement or tail trim."""
    import proof_provenance as provenance
    remember_data_import_inputs(world, source)
    source_path = world.paths["build"].parent / "src" / (source + ".c")
    if provenance._all_pragmas(source_path.read_text()):
        raise PackError(f"data-only input contains an assembly fallback: {source}")
    configured = Elf(world.paths["build"] / "src" / (source + ".c.o"))
    size = raw.size(".data")
    def require(test, why):
        if not test:
            raise PackError(f"data-only module {source}: {why}")
    require(row.get("section", ".data") == ".data" and size > 0
            and size == int(row["size"], 16), "not one complete canonical .data input")
    for obj in (raw, configured):
        require(all(obj.size(s) == 0 for s in (".text", ".rodata", ".bss")),
                "mixed runtime sections are unsupported")
        require(not any(s.shndx == SHN_COMMON and s.size for s in obj.symbols),
                "COMMON allocation is unsupported")
        require(not any(s["size"] and s["flags"] & 2
                        and s["name"] not in (".data", ".reginfo", ".MIPS.abiflags")
                        for s in obj.sections), "unexpected allocated section")
        sec = obj.section(".data")
        require(sec and sec["type"] == 1 and sec["flags"] & 3 == 3
                and not sec["flags"] & 4 and sec["size"] == size, "invalid data storage")
    require(raw.bytes_of(".data") == configured.bytes_of(".data"),
            "raw/configured initialized bytes differ")
    items = [mi for mi in world.map if mi.file == f"build/src/{source}.c.o"
             and mi.insec == ".data" and mi.size]
    require(len(items) == 1, "missing or duplicate final source input")
    item = items[0]
    start = item.addr - OVERLAY_VMA
    require(item.outsec == f".overlay_{ov:03d}" and item.size == size
            and start == world.module_info[ov]["text_size"] + int(row["offset"], 16),
            "source row/final input geometry differs")
    require(not any(mi is not item and mi.size and mi.addr < item.addr + size
                    and item.addr < mi.addr + mi.size
                    for mi in world.module_info[ov]["inputs"]), "overlapping final contributor")
    require(world.elf.bytes_of(f".overlay_{ov:03d}")[start:start + size]
            == configured.bytes_of(".data"), "final initialized input bytes differ")
    return configured, start


def reproduce_data_provider(world, source):
    """Fresh ordinary-provider compile, sharing the producer capture machinery."""
    import proof_provenance as provenance
    root = world.paths["build"].parent
    configured = world.paths["build"] / "src" / (source + ".c.o")
    raw, _plan, receipt, context = provenance.capture_data_provider_raw(
        root, root / "src" / (source + ".c"), configured,
        world.paths["elf"], world.paths["map"])
    fresh = Elf(raw)
    validate_fresh_data_input(Elf(world.paths["raw"] / "src" / (source + ".c.o")), fresh)
    validate_fresh_data_input(Elf(configured), fresh)
    hold_data_capture(world, source, receipt, context)
    receipts = getattr(world, "data_import_provider_receipts", {})
    receipts[source] = receipt
    world.data_import_provider_receipts = receipts
    if receipt["inputs"] != context():
        raise PackError("provider source/context changed during fresh compilation")
    return fresh


def data_import_definition(world, name):
    """A real raw C definition and unique map input, never an encoded address."""
    choices = [s for s in world.by_name.get(name, ()) if s.bind == STB_GLOBAL
               and s.type == 1 and s.size > 0 and s.shndx not in (SHN_ABS, SHN_UNDEF)]
    if len(choices) != 1:
        raise PackError(f"data-only import has no unique sized canonical definition: {name}")
    symbol = choices[0]
    match = re.fullmatch(r"\.overlay_(\d{3})", symbol.section)
    if not match:
        raise PackError(f"data-only import target is not initialized overlay C data: {name}")
    ov = int(match.group(1))
    hits = []
    for row in projected_data_rows(world.amods[ov]):
        source = row["source"]
        for item in world.module_info[ov]["inputs"]:
            if (item.file == f"build/src/{source}.c.o" and item.insec == ".data"
                    and item.addr <= symbol.value
                    and symbol.value + symbol.size <= item.addr + item.size):
                hits.append((row, source))
    if len(hits) != 1:
        raise PackError(f"data-only import target lacks a unique canonical source input: {name}")
    row, source = hits[0]
    raw = Elf(world.paths["raw"] / "src" / (source + ".c.o"))
    _configured, start = data_import_input(world, ov, row, source, raw)
    if any(raw.rels(s) for s in (".text", ".data", ".rodata")):
        raise PackError(f"data-only import provider has unsupported relocations: {name}")
    providers = getattr(world, "data_import_providers", {})
    if source not in providers:
        providers[source] = reproduce_data_provider(world, source)
    world.data_import_providers = providers
    fresh = providers[source]
    definitions = [s for s in fresh.symbols if s.name == name and s.section == ".data"
                   and s.bind == STB_GLOBAL and s.type == 1 and s.size > 0]
    if len(definitions) != 1:
        raise PackError(f"data-only import is not defined by its raw C input: {name}")
    own = definitions[0]
    if (own.size != symbol.size or own.value + start != symbol.value - OVERLAY_VMA
            or own.value < 0 or own.value + own.size > raw.size(".data")):
        raise PackError(f"data-only raw definition/final geometry differs: {name}")
    return ov, start + own.value


def validate_data_local_prefix(world, ov, row, source, raw, start):
    """A complete aggregate at runtime DATA zero may point inside itself.

    The actual source definition and final map establish the target before any
    retained record is consulted. This does not infer a runtime section role
    from the compiler's section name, or admit arbitrary interior input groups.
    """
    size = raw.size(".data")
    owners = [s for s in raw.symbols if s.section == ".data" and s.type == 1
              and s.bind == STB_GLOBAL and s.size]
    if (int(row["offset"], 16) != 0 or len(owners) != 1
            or owners[0].value != 0 or owners[0].size != size):
        raise PackError(f"data-only LOCAL prefix lacks one complete sized owner: {source}")
    owner = owners[0]
    definitions = [s for s in world.by_name.get(owner.name, ())
                   if s.bind == STB_GLOBAL and s.shndx != SHN_UNDEF]
    if (len(definitions) != 1 or definitions[0].section != f".overlay_{ov:03d}"
            or definitions[0].type != 1 or definitions[0].size != size
            or definitions[0].value != OVERLAY_VMA + start):
        raise PackError(f"data-only LOCAL prefix lacks its unique final owner: {source}")
    rels = raw.rels(".data")
    identities = {}
    offsets = []
    for rel, addend in zip(rels, in_place_addends(raw, ".data", rels)):
        if (rel.type != R_MIPS_32 or rel.sym.index != owner.index or rel.offset % 4
                or not 0 <= rel.offset <= size - 4 or not 0 <= addend < size
                or rel.offset in offsets):
            raise PackError(f"unsupported data-only LOCAL prefix relocation: {source}")
        offsets.append(rel.offset)
        identities[(start + rel.offset, rel.type)] = ("ov", ov, start + owner.value + addend)
    if offsets != sorted(offsets):
        raise PackError(f"unordered data-only LOCAL prefix relocations: {source}")
    expected = {key: value for key, value in world.expected_cache(ov).items()
                if start <= key[0] < start + size}
    if set(identities) != set(expected) or not identities:
        raise PackError(f"data-only LOCAL prefix: missing or extra runtime records: {source}")
    if list(identities) != sorted(expected, key=lambda key: expected[key]["position"]):
        raise PackError(f"data-only LOCAL prefix runtime order differs: {source}")
    for key, identity in identities.items():
        record = expected[key]
        if (record["op"] != OP_LOCAL or record["table"] != 2
                or record["symbol_index"] != world.module_info[ov]["text_size"]
                or record["full"] != identity):
            raise PackError(f"data-only LOCAL source/runtime identity mismatch: {source}+{key[0]:#x}")
    return len(rels)


def validate_data_imports(world, exports, ov, row, source, raw):
    """Complete ORT pointer inputs or a bounded same-owner LOCAL data prefix."""
    import source_module_link_proof as proof
    configured, start = data_import_input(world, ov, row, source, raw)
    rels = raw.rels(".data")
    if any(configured.rels(s) for s in (".text", ".data", ".rodata")):
        raise PackError(f"configured data-only input retains relocations: {source}")
    if any(rel.sym.shndx != SHN_UNDEF for rel in rels):
        return validate_data_local_prefix(world, ov, row, source, raw, start)
    if (len(rels) * 4 != raw.size(".data")
            or raw.bytes_of(".data") != bytes(raw.size(".data"))):
        raise PackError(f"data-only module {source}: not a complete pure ORT pointer input")
    identities = proof.source_identities(raw, configured, world.elf, exports, ov)
    expected = {key: value for key, value in world.expected_cache(ov).items()
                if start <= key[0] < start + raw.size(".data")}
    if len(expected) != len(rels):
        raise PackError(f"data-only module {source}: missing or extra runtime records")
    for index, rel in enumerate(rels):
        if (rel.offset != index * 4 or rel.type != R_MIPS_32
                or rel.sym.shndx != SHN_UNDEF or rel.sym.name not in exports.by_name):
            raise PackError(f"data-only module {source}: unsupported LOCAL/group/import relocation")
        target = data_import_definition(world, rel.sym.name)
        record = expected.get((start + rel.offset, rel.type))
        if (identities[(rel.offset, rel.type)] != target or record is None
                or record["op"] != OP_SYMBOL
                or record["symbol_index"] != exports.by_name[rel.sym.name]
                or record["full"] != ("ov", *target)
                or record["table"] != (2 if target[0] == ov else 1)):
            raise PackError(f"data-only source/runtime identity mismatch: {source}+{rel.offset:#x}")
    return len(rels)


def validate_fresh_data_input(ordinary_raw, fresh):
    """Zero payloads do not excuse a stale raw symbol/relocation surface."""
    import source_module_link_proof as proof
    def relocations(obj):
        return [(section, rel.offset, rel.type, rel.sym.name, rel.sym.value,
                 rel.sym.size, rel.sym.bind, rel.sym.type, rel.sym.section)
                for section in (".text", ".data", ".rodata") for rel in obj.rels(section)]
    if (proof.allocated(ordinary_raw) != proof.allocated(fresh)
            or proof.symbols(ordinary_raw) != proof.symbols(fresh)
            or relocations(ordinary_raw) != relocations(fresh)):
        raise PackError("stale ordinary raw data-only source evidence")


def reproduce_data_import(world, source):
    """Reuse fresh compiler/context capture; replay the unchanged ordinary helper."""
    import shlex
    import subprocess
    import proof_provenance as provenance
    import source_module_link_proof as proof
    root = world.paths["build"].parent
    configured = world.paths["build"] / "src" / (source + ".c.o")
    raw, _plan, receipt, context = provenance.capture_data_module_producer_raw(
        root, root / "src" / (source + ".c"), configured, world.paths["elf"], world.paths["map"])
    words = shlex.split(receipt["inputs"]["module_link"])
    target = configured.relative_to(root).as_posix()
    if (words.count(target) != 1 or "--output" in words
            or len(words) < 2 or words[1] != "tools/link_overlay_module.py"):
        raise PackError("ambiguous data-only source-module recipe")
    flags = {"--ort": "config/runlink-ort.us.txt", "--ld": "tools/binutils/mips64-elf-ld",
             "--objcopy": "tools/binutils/mips64-elf-objcopy"}
    for flag, value in flags.items():
        if words.count(flag) != 1 or words[words.index(flag) + 1] != value:
            raise PackError("data-only module recipe uses unbound tools or exports")
    directory = root / receipt["directory"] / "module"
    if words.count("--directory") != 1:
        raise PackError("ambiguous data-only artifact directory")
    words[words.index("--directory") + 1] = str(directory)
    words[words.index(target)] = str(raw)
    replayed = raw.with_name("module-replayed.o")
    words += ["--output", str(replayed)]
    result = subprocess.run(words, cwd=root, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=120)
    if result.returncode:
        raise PackError("data-only ordinary module link failed: " + result.stdout.strip())
    link = json.loads((directory / "receipt.json").read_text())
    checks = {"compiler.o": "compiler_sha256", "module.elf": "linked_sha256",
              "bindings.o": "prepared_sha256", "module.ld": "script_sha256",
              "localized.elf": "localized_sha256"}
    if (link.get("schema") != "mickey-source-module-link-v1"
            or link.get("container_conversion") != "zero-based-exec-to-rel-header-only"
            or link.get("compiler_sha256") != provenance.sha256_file(raw)
            or link.get("configured_sha256") != provenance.sha256_file(replayed)
            or any(link.get(field) != provenance.sha256_file(directory / name)
                   for name, field in checks.items())):
        raise PackError("data-only ordinary helper receipt does not bind its products")
    actual, reproduced = Elf(configured), Elf(replayed)
    ordinary_raw = Elf(world.paths["raw"] / "src" / (source + ".c.o"))
    fresh = Elf(raw)
    validate_fresh_data_input(ordinary_raw, fresh)
    if (proof.allocated(actual) != proof.allocated(reproduced)
            or proof.symbols(actual) != proof.symbols(reproduced)):
        raise PackError("fresh data-only source link differs from configured bytes or symbols")
    if receipt["inputs"] != context():
        raise PackError("data-only source/context changed during reproduction")
    return Elf(raw), receipt, context


def guard_data_module_imports(world, exports):
    """Revalidate atomically before publishing complete data-only producers."""
    if getattr(world, "_data_guard_active", False):
        raise PackError("recursive data-only producer discovery")
    world._guarded_data_objects = None
    world._objects = world._defs = None
    world._data_guard_active = True
    try:
        admitted = _guard_data_module_imports(world, exports)
    finally:
        world._data_guard_active = False
    world._guarded_data_objects = admitted


def _guard_data_module_imports(world, exports):
    """Existing fresh guard; build a private inventory until every hold passes."""
    import postprocess_audit as audit
    import proof_provenance as provenance
    import source_module_link_proof as proof
    root = world.paths["build"].parent
    if not (root / "Makefile").is_file():
        return {}  # synthetic trees without a configured backend have no selected inputs
    metadata_implementations()
    if (world.elf.data != world.paths["elf"].read_bytes()
            or world.map != parse_map(world.paths["map"])):
        raise PackError("parsed data-only World ELF/map no longer match the supplied inputs")
    world.data_import_inputs = {}
    world.data_import_contexts = []
    world.data_import_providers = {}
    world.data_import_provider_receipts = {}
    import permute_batch as batch
    files = {root / name for name in ("Makefile", "tools/proof_provenance.py",
             "tools/source_module_link_proof.py", "tools/postprocess_audit.py",
             "tools/static_symbols.py", ".venv/bin/python")}
    files.update((root / "mk").rglob("*.mk"))
    for directory in ("tools/ido", "tools/binutils"):
        files.update(path for path in (root / directory).rglob("*") if path.is_file())
    files.update((root / "tools/asm-processor").rglob("*.py"))
    hold_data_import_inputs(world, {path: hashlib.sha256(path.read_bytes()).hexdigest()
                                    for path in files})
    hold_data_import_inputs(world, {path: digest for path, digest
                                    in batch._LOADED_IMPLEMENTATIONS.values()})
    dump = audit.run_make_database()
    outputs = []
    admitted = collections.defaultdict(list)
    try:
        for ov, module in world.amods.items():
            text = {r["source"] for r in module["text_ownership"] if r["type"] == "c"}
            sources = {r["source"] for r in module.get("data_rodata_ownership", [])
                       if r.get("type") == "c"} - text
            for source in sorted(sources):
                if not proof.selected(f"build/src/{source}.c.o", dump):
                    continue
                rows = [r for r in projected_data_rows(module) if r["source"] == source]
                if len(rows) != 1:
                    raise PackError(f"data-only module lacks one canonical input: {source}")
                raw, receipt, context = reproduce_data_import(world, source)
                hold_data_capture(world, source, receipt, context)
                validate_data_imports(world, exports, ov, rows[0], source, raw)
                targets = {rel.sym.name: data_import_definition(world, rel.sym.name)
                           for rel in raw.rels(".data") if rel.sym.shndx == SHN_UNDEF}
                start = world.module_info[ov]["text_size"] + int(rows[0]["offset"], 16)
                path = world.paths["raw"] / "src" / (source + ".c.o")
                admitted[ov].append(ObjRef(ov, path, source, 0, 0, "c", raw,
                    {".data": (start, "link")}, targets))
                outputs.append((raw, receipt))
        check_data_import_holds(world)
        for receipt in world.data_import_provider_receipts.values():
            receipt["allocated_symbols_ordered_relocations_exact"] = True
            (root / receipt["directory"] / "proof.json").write_text(
                json.dumps(receipt, sort_keys=True, indent=2))
        for raw, receipt in outputs:
            receipt.update(data_only_runtime_imports=len(raw.rels(".data")),
                provider_inputs={os.path.relpath(path, root): digest
                                 for path, digest in world.data_import_inputs.items()})
            (root / receipt["directory"] / "proof.json").write_text(
                json.dumps(receipt, sort_keys=True, indent=2))
    except (OSError, ValueError, provenance.MetadataProofError) as error:
        raise PackError(f"data-only module proof refused: {error}") from error
    return dict(admitted)


def pack_all(world, ort):
    guard_data_module_imports(world, ort)
    resolver = Resolver(world, ort)
    relocs = {}
    for ov in sorted(world.amods):
        relocs[ov] = [c for o in world.objects(ov) for c in collect_relocs(world, resolver, o)]
    borrowed = collections.defaultdict(list)
    for k, v in place_sections(world, resolver, relocs).items():
        borrowed[k].extend(v)
    modules = {}
    for ov in sorted(world.amods):
        pm = pack_module(world, resolver, ov, relocs[ov])
        modules[ov] = pm
        for k, v in pm.borrowed.items():
            borrowed[k].extend(v)
    ph = collections.Counter()
    for ov in sorted(world.amods):
        for c in relocs[ov]:
            if c.target.kind == "placeholder":
                ph[(ov, c.target.name)] += 1
    for (ov, name), n in sorted(ph.items()):
        borrowed["placeholder"].append([ov, name, n])
    main, mb, problems = pack_main(world, resolver)
    for k, v in mb.items():
        borrowed[k].extend(v)
    ort_words = []
    for i, n in enumerate(ort.rows):
        ident = resolver.ort_identity(i)
        if ident is None:
            problems.append(f"ORT row {i} name {n} does not resolve")
            ort_words.append(None)
            continue
        if ident[0] == "res":
            sel, off = world.layout.selector_of(ident[1])
        else:
            sel, off = ident[1], ident[2]
        ort_words.append((sel << 20) | (off & 0xFFFFF))
    return Packed(modules, main, ort_words, dict(borrowed), problems, resolver, relocs)


def encode_headers(packed, world, table_override=None):
    """The header table and module images, in module order."""
    headers, images = [], []
    pos = 0
    for ov in range(1, ot.HEADER_COUNT + 1):
        pm = packed.modules.get(ov)
        if pm is None:
            raise PackError(f"overlay {ov} is not in the atlas")
        t1, t2 = (table_override(pm) if table_override else (pm.reloc_bytes(1), pm.reloc_bytes(2)))
        image = pm.image_prefix + t1 + t2
        headers.append(struct.pack(">iiiiiHHii", 0, pos, pm.text_size, pm.data_size, pm.bss_size,
                                   len(t1), len(t2), pm.init, pm.resume))
        images.append(image)
        pos += len(image)
    return b"".join(headers), images


def encode_main(entries):
    body = struct.pack(">I", len(entries)) + b"".join(e.encode() for e in entries)
    if len(body) > MAIN_BLOCK:
        raise PackError(f"main table of {len(entries)} entries overflows its {MAIN_BLOCK}-byte block")
    return body + b"\0" * (MAIN_BLOCK - len(body))


def encode_ort(words):
    return b"".join(struct.pack(">I", w if w is not None else 0) for w in words)


def encode_region(packed, world, table_override=None, main_override=None):
    main = encode_main(main_override if main_override is not None else packed.main)
    headers, images = encode_headers(packed, world, table_override)
    return main + encode_ort(packed.ort_words) + headers + b"".join(images)


# ---------------------------------------------------------------------------
# Comparison


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


def compare_module(pm, exp, resolver):
    """Classify every way the packed module's tables differ from the shipped ones."""
    out = []
    got = {}
    for r in pm.table1 + pm.table2:
        if r.key in got:
            out.append(Residual("duplicate", pm.overlay, r.site, "", r.name))
        got[r.key] = r
    for key, r in got.items():
        e = exp.get(key)
        if e is None:
            out.append(Residual("extra", pm.overlay, r.site, "no record", f"{OP_NAMES[r.op]} {r.name}"))
            continue
        en = resolver.name_of(e["full"])
        if r.full != e["full"]:
            out.append(Residual(target_category(r, pm), pm.overlay, r.site, f"{e['op_name']} {en}",
                                f"{OP_NAMES[r.op]} {r.name}" + (f" ({fmt_ident(r.full)})" if r.full else "")))
        elif r.op != e["op"]:
            out.append(Residual("op", pm.overlay, r.site, f"{e['op_name']} {en}", f"{OP_NAMES[r.op]} {r.name}"))
        elif r.table != e["table"]:
            out.append(Residual("table", pm.overlay, r.site, f"table {e['table']}", f"table {r.table}"))
        elif (r.symidx & 0xFFFFFFFF) != e["symbol_index"]:
            cat = "jump-index" if r.op == OP_JUMP else ("row" if r.op == OP_SYMBOL else "base")
            out.append(Residual(cat, pm.overlay, r.site, f"{e['symbol_index']}", f"{r.symidx}"))
    for key, e in exp.items():
        if key not in got:
            out.append(Residual("missing", pm.overlay, key[0], f"{e['op_name']} {resolver.name_of(e['full'])}",
                                "no record"))
    flagged = {x.offset for x in out}
    for table, rows in ((1, pm.table1), (2, pm.table2)):
        ship = [k for k, e in sorted(exp.items(), key=lambda kv: kv[1]["position"]) if e["table"] == table]
        mine = [r.key for r in rows]
        common = set(ship) & set(mine)
        a = [k for k in ship if k in common]
        b = [k for k in mine if k in common]
        sm = difflib.SequenceMatcher(a=a, b=b, autojunk=False)
        in_lcs = set()
        for blk in sm.get_matching_blocks():
            in_lcs.update(b[blk.b:blk.b + blk.size])
        for k in b:
            if k not in in_lcs and k[0] not in flagged:
                out.append(Residual("order", pm.overlay, k[0], f"table {table} position "
                                    f"{exp[k]['position']}", f"position {mine.index(k)}"))
    return out


def target_category(r, pm):
    """Split a wrong-target residual by what has to change to fix it."""
    if r.site >= pm.text_size:
        return "table-entry"     # a switch or pointer table entry in data
    if r.route in ("section", "object-local"):
        return "pool-layout"     # the object's own literal/static data sits elsewhere
    if r.type == R_MIPS_26:
        return "callee"          # the C calls a different function
    return "data-ref"            # the C addresses a different object or offset


def fmt_ident(ident):
    if ident is None:
        return "?"
    if ident[0] == "res":
        return "resident %s" % hexs(ident[1])
    return "overlay %d %s" % (ident[1], hexs(ident[2]))


def compare_all(packed, world):
    res = []
    resolver = packed.resolver
    sh = world.shipped
    for ov, pm in sorted(packed.modules.items()):
        exp = world.expected_cache(ov)
        res.extend(compare_module(pm, exp, resolver))
        m = sh.modules[ov]
        for fld, mine in (("text_size", pm.text_size), ("data_size", pm.data_size),
                          ("bss_size", pm.bss_size), ("init_function", pm.init),
                          ("resume_function", pm.resume)):
            if m[fld] != mine:
                res.append(Residual("header", ov, 0, f"{fld} {m[fld]}", f"{mine}"))
        image = world.shipped.rom[m["rom_start"]:m["rom_start"] + m["text_size"] + m["data_size"]]
        if image != pm.image_prefix:
            first = next((i for i in range(min(len(image), len(pm.image_prefix)))
                          if image[i] != pm.image_prefix[i]), min(len(image), len(pm.image_prefix)))
            res.append(Residual("image", ov, first, f"{len(image)} bytes", f"{len(pm.image_prefix)} bytes"))
        for c, why in pm.unresolved:
            if why == "unexported-target":
                continue    # still emitted, and classified by its target above
            if why == "unplaced-site":
                # The section holding the site has no placement; offset is section-relative.
                res.append(Residual("unplaced", ov, c.offset, f"{c.obj.label} {c.section}",
                                    f"{TYPE_NAMES[c.type]} {c.target.name}"))
                continue
            res.append(Residual("unresolved", ov, (c.site() or 0), why, f"{c.obj.label} {c.target.name}"))
    for i, w in enumerate(packed.ort_words):
        e = sh.ort[i]
        want = (e["overlay"] << 20) | e["offset"]
        if w != want:
            res.append(Residual("ort", 0, i, resolver.name_of(world.layout.row_identity(e["overlay"], e["offset"])),
                                f"{resolver.ort.rows[i]}"))
    ship_main = {(e["call_site_offset"], (e["flags"] >> 4) & 0xF, e["flags"] & 0xF): e for e in sh.main}
    mine_main = {(x.offset, x.type, x.op): x for x in packed.main}
    for k, x in mine_main.items():
        e = ship_main.get(k)
        if e is None:
            res.append(Residual("main-extra", 0, x.offset, "no entry", x.name))
        elif e["rom_table_index"] != x.row:
            res.append(Residual("main-row", 0, x.offset, resolver.ort.rows[e["rom_table_index"]], x.name))
    for k, e in ship_main.items():
        if k not in mine_main:
            res.append(Residual("main-missing", 0, k[0], resolver.ort.rows[e["rom_table_index"]], "no entry"))
    order_ship = [(e["call_site_offset"], (e["flags"] >> 4) & 0xF, e["flags"] & 0xF) for e in sh.main]
    order_mine = [(x.offset, x.type, x.op) for x in packed.main]
    if [k for k in order_ship if k in mine_main] != [k for k in order_mine if k in ship_main]:
        res.append(Residual("main-order", 0, 0, "shipped order", "packed order"))
    return res


# ---------------------------------------------------------------------------
# The residual manifest

BORROWED_DOC = {
    "placeholder": "identity of a symbol the source names only by placeholder (module, symbol, sites)",
    "hybrid-data": "data-site record in blob-owned data, taken from the shipped table (module, site)",
    "placement": "object data section placed from the shipped references (module, object, section, offset, evidence)",
    "rodata-base": "module rodata base the objects do not determine (module)",
    "bss-size": "header BSS size without a complete faithful linked source allocation, from the atlas (module)",
    "main-placeholder": "main-table entry whose target the resident source does not name (site, op, type)",
}
DIFF_DOC = {
    "callee": "the C calls a different function than the shipped record targets",
    "data-ref": "the C addresses a different object or offset than the shipped record",
    "pool-layout": "an object's own literal or static data sits at a different offset",
    "table-entry": "a switch or pointer table entry targets a different address",
    "op": "same target, different operation (SYMBOL/LOCAL/JUMP)",
    "table": "same target and operation, other table",
    "row": "same target, different ORT row",
    "base": "same target, different LOCAL base",
    "jump-index": "JUMP record whose stale index differs",
    "order": "record emitted at a different position than shipped",
    "extra": "the objects produce a record the shipped table lacks",
    "missing": "the shipped table has a record the objects do not produce",
    "duplicate": "two records at one site",
    "unplaced": "a relocation in an object section no evidence places (offset is section-relative)",
    "unresolved": "a relocation the packer could not resolve",
    "header": "a header field differs",
    "image": "module text/data bytes differ",
    "ort": "an ORT row resolves to a different address",
    "main-extra": "main-table entry the shipped table lacks",
    "main-missing": "shipped main-table entry not produced",
    "main-row": "main-table entry names a different ORT row",
    "main-order": "main table emitted in a different order",
}


def entry_sort_key(e):
    out = []
    for x in e:
        if isinstance(x, int):
            out.append((0, x, ""))
        elif isinstance(x, str) and re.fullmatch(r"0x[0-9A-F]+", x):
            out.append((0, int(x, 16), ""))
        else:
            out.append((1, 0, str(x)))
    return out


def manifest_from(residuals, borrowed):
    diffs = collections.defaultdict(list)
    for r in sorted(residuals, key=lambda r: (r.category, r.module, r.offset)):
        diffs[r.category].append([r.module, hexs(r.offset), r.expected, r.got])
    out = {
        "schema": 1,
        "generated_by": "tools/runlink_pack.py compare --write-residuals",
        "about": ("Residuals of the runlink packer against the shipped tables. "
                  "`gmake check-runlink-tables` fails on any difference or borrowing "
                  "not listed here and on any listed entry that no longer occurs; "
                  "regenerate with `gmake runlink-residuals` only to record progress."),
        "totals": {"differences": len(residuals),
                   "borrowed_records": sum(e[2] for e in borrowed.get("placeholder", []))
                   + len(borrowed.get("hybrid-data", [])) + len(borrowed.get("main-placeholder", []))},
        "differences": {k: {"about": DIFF_DOC.get(k, ""), "count": len(v), "entries": v}
                        for k, v in sorted(diffs.items())},
        "borrowed": {},
    }
    for k in sorted(borrowed):
        v = sorted(borrowed[k], key=entry_sort_key)
        if k == "hybrid-data":
            by = collections.defaultdict(list)
            for ov, site in v:
                by[str(ov)].append(site)
            out["borrowed"][k] = {"about": BORROWED_DOC[k], "count": len(v), "modules": dict(by)}
        else:
            out["borrowed"][k] = {"about": BORROWED_DOC.get(k, ""), "count": len(v), "entries": v}
    return out


def manifest_keys(man):
    keys = set()
    for cat, blk in man.get("differences", {}).items():
        for e in blk.get("entries", []):
            keys.add(("diff", cat, e[0], e[1]))
    for cat, blk in man.get("borrowed", {}).items():
        if "modules" in blk:
            for ov, sites in blk["modules"].items():
                for s in sites:
                    keys.add(("borrowed", cat, int(ov), s))
        for e in blk.get("entries", []):
            keys.add(("borrowed", cat) + tuple(e))
    return keys


def render_manifest(man):
    """JSON with one entry per line, so diffs read as entries."""
    lines = ["{"]
    top = list(man.items())
    for ti, (k, v) in enumerate(top):
        comma = "," if ti + 1 < len(top) else ""
        if k in ("differences", "borrowed"):
            lines.append(f'  "{k}": {{')
            cats = list(v.items())
            for ci, (cat, blk) in enumerate(cats):
                ccomma = "," if ci + 1 < len(cats) else ""
                lines.append(f'    "{cat}": {{')
                fields = list(blk.items())
                for fi, (fk, fv) in enumerate(fields):
                    fcomma = "," if fi + 1 < len(fields) else ""
                    if fk == "entries":
                        lines.append(f'      "entries": [')
                        for ei, e in enumerate(fv):
                            lines.append("        " + json.dumps(e) + ("," if ei + 1 < len(fv) else ""))
                        lines.append("      ]" + fcomma)
                    elif fk == "modules":
                        lines.append('      "modules": {')
                        ms = list(fv.items())
                        for mi, (mk, mv) in enumerate(ms):
                            lines.append(f'        "{mk}": ' + json.dumps(mv) + ("," if mi + 1 < len(ms) else ""))
                        lines.append("      }" + fcomma)
                    else:
                        lines.append(f'      "{fk}": ' + json.dumps(fv) + fcomma)
                lines.append("    }" + ccomma)
            lines.append("  }" + comma)
        else:
            lines.append(f'  "{k}": ' + json.dumps(v) + comma)
    lines.append("}")
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# Region proof


def region_proof(packed, world, residuals):
    """Byte-compare the packed overlay region with the shipped one.

    Two images: the packer's own, and the packer's with exactly the residual
    records replaced by the shipped ones (a listed difference or a borrowed
    record is the only thing allowed to differ).  The second must equal the
    shipped region byte for byte."""
    sh = world.shipped
    end = max([REGION_START] + [m["rom_end"] for m in sh.modules.values()])
    rom_region = sh.rom[REGION_START:end]
    raw = encode_region(packed, world)
    flagged = collections.defaultdict(set)
    for r in residuals:
        flagged[r.module].add(r.offset)

    order_breaks = []

    def patched_tables(pm):
        exp = world.expected_cache(pm.overlay)
        mine = {r.key: r for r in pm.table1 + pm.table2}
        out = []
        for table, rows_mine in ((1, pm.table1), (2, pm.table2)):
            rows = sorted((e for e in exp.values() if e["table"] == table), key=lambda e: e["position"])
            # Outside the residuals the packer's own order must be the shipped one.
            kept = [(e["target_offset"], e["mode"]) for e in rows
                    if (e["target_offset"], e["mode"]) in mine and e["target_offset"] not in flagged[pm.overlay]]
            ours = [r.key for r in rows_mine if r.key in exp and r.site not in flagged[pm.overlay]
                    and exp[r.key]["table"] == table]
            if kept != ours:
                order_breaks.append((pm.overlay, table))
            b = b""
            for e in rows:
                k = (e["target_offset"], e["mode"])
                r = mine.get(k)
                if r is None or e["target_offset"] in flagged[pm.overlay]:
                    b += struct.pack(">II", e["symbol_index"], (e["target_offset"] << 8) | e["flags"])
                else:
                    b += r.encode()
            out.append(b)
        return out[0], out[1]

    ship_main = {}
    for e in sh.main:
        ship_main[(e["call_site_offset"], (e["flags"] >> 4) & 0xF, e["flags"] & 0xF)] = e
    mine_main = {(x.offset, x.type, x.op): x for x in packed.main}
    main_patched = []
    for e in sh.main:
        k = (e["call_site_offset"], (e["flags"] >> 4) & 0xF, e["flags"] & 0xF)
        x = mine_main.get(k)
        if x is None or x.offset in flagged[0]:
            x = MainEntry(e["rom_table_index"], k[0], k[1], k[2], 0, "")
        main_patched.append(x)
    patched = encode_region(packed, world, table_override=patched_tables, main_override=main_patched)
    if [k for k in [(e["call_site_offset"], (e["flags"] >> 4) & 0xF, e["flags"] & 0xF) for e in sh.main]
            if k in mine_main] != [k for k in [(x.offset, x.type, x.op) for x in packed.main] if k in ship_main]:
        order_breaks.append((0, "main"))
    return {
        "order_breaks": order_breaks,
        "region": [REGION_START, end],
        "raw_equal": raw == rom_region,
        "raw_differing_bytes": diff_bytes(raw, rom_region),
        "patched_equal": patched == rom_region and not order_breaks,
        "patched_differing_bytes": diff_bytes(patched, rom_region),
        "patched_first_difference": first_diff(patched, rom_region),
    }


def diff_bytes(a, b):
    n = sum(1 for x, y in zip(a, b) if x != y)
    return n + abs(len(a) - len(b))


def first_diff(a, b):
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return REGION_START + i
    return None if len(a) == len(b) else REGION_START + min(len(a), len(b))


# ---------------------------------------------------------------------------
# CLI


def resolve_paths(args):
    root = Path(args.root).resolve() if args.root else REPO
    p = {}
    for k, v in DEFAULTS.items():
        given = getattr(args, k, None)
        p[k] = Path(given) if given else root / v
        if not p[k].is_absolute():
            p[k] = root / p[k]
    return p


def summarize(residuals, borrowed, packed):
    cats = collections.Counter(r.category for r in residuals)
    recs = sum(len(pm.table1) + len(pm.table2) for pm in packed.modules.values())
    prov = collections.Counter(r.provenance for pm in packed.modules.values()
                               for r in pm.table1 + pm.table2)
    lines = [f"packed {len(packed.modules)} modules, {recs} module records "
             f"({prov['object']} from the objects, {prov['placeholder']} placeholder, "
             f"{prov['hybrid-data']} hybrid data), "
             f"{len(packed.main)} main-table entries, {len(packed.ort_words)} ORT rows"]
    lines.append("differences: " + (", ".join(f"{k} {v}" for k, v in sorted(cats.items())) or "none"))
    parts = []
    for k in sorted(borrowed):
        v = borrowed[k]
        if k == "placeholder":
            parts.append(f"placeholder {sum(e[2] for e in v)} records ({len(v)} symbols)")
        else:
            parts.append(f"{k} {len(v)}")
    lines.append("borrowed: " + (", ".join(parts) or "none"))
    return lines


def cmd_ort(args):
    paths = resolve_paths(args)
    world = World(paths, need_raw=False)
    ort = generate_ort(world)
    text = ort.render()
    if args.write:
        paths["ort"].write_text(text)
        print(f"wrote {paths['ort'].relative_to(REPO) if paths['ort'].is_relative_to(REPO) else paths['ort']}"
              f" ({len(ort.rows)} rows, {len(ort.entries)} entry points)")
        return 0
    cur = paths["ort"].read_text() if paths["ort"].exists() else ""
    if cur != text:
        print("runlink ORT name list is stale; run `gmake runlink-ort`", file=sys.stderr)
        for line in list(difflib.unified_diff(cur.splitlines(), text.splitlines(), "tracked", "generated",
                                              lineterm=""))[:40]:
            print(line, file=sys.stderr)
        return 1
    print(f"runlink ORT name list current ({len(ort.rows)} rows)")
    return 0


def load_and_pack(args):
    paths = resolve_paths(args)
    world = World(paths)
    ort = OrtNames.load(paths["ort"])
    if len(ort.rows) != len(world.shipped.ort):
        raise PackError(f"{paths['ort']}: {len(ort.rows)} rows, the shipped ORT has {len(world.shipped.ort)}")
    packed = pack_all(world, ort)
    return paths, world, packed


def cmd_pack(args):
    paths, world, packed = load_and_pack(args)
    out = Path(args.out) if args.out else paths["out"]
    out.mkdir(parents=True, exist_ok=True)
    headers, images = encode_headers(packed, world)
    (out / "main_reloc_table.bin").write_bytes(encode_main(packed.main))
    (out / "overlay_rom_table.bin").write_bytes(encode_ort(packed.ort_words))
    (out / "overlay_header_table.bin").write_bytes(headers)
    mdir = out / "modules"
    mdir.mkdir(exist_ok=True)
    for ov, img in zip(range(1, len(images) + 1), images):
        (mdir / f"overlay_{ov:03d}.bin").write_bytes(img)
    print(f"wrote {out} (main table, ORT, headers, {len(images)} module images)")
    for p in packed.problems:
        print("problem:", p, file=sys.stderr)
    return 1 if packed.problems else 0


def metadata_input_paths(paths, world):
    """The files the metadata producer reads, including resident trap sidecars."""
    inputs = {paths[k] for k in ("atlas", "ort", "residuals", "rom", "elf", "map")}
    for ov in world.amods:
        inputs.update(obj.path for obj in world.objects(ov))
    for mi in world.map:
        if (mi.outsec not in (".main", ".entry") or mi.insec not in (".text", ".data")
                or not mi.size or not mi.file.endswith(".o")):
            continue
        path = world.paths["build"].parent / mi.file
        if not path.is_file():
            raise PackError(f"metadata resident input is missing: {path}")
        inputs.add(path)
        sidecar = Path(str(path) + ".trapsites")
        if mi.insec == ".text" and mi.file.endswith(".c.o") and sidecar.exists():
            inputs.add(sidecar)
    inputs.update(getattr(world, "data_import_inputs", {}))
    return inputs


def metadata_digests(inputs, root):
    result = {}
    for path in sorted(inputs):
        data = path.read_bytes()
        result[os.path.relpath(path, root)] = {
            "size": len(data), "sha256": hashlib.sha256(data).hexdigest()}
    return result


def metadata_implementations():
    for name, digest in _LOADED_IMPLEMENTATIONS.items():
        if hashlib.sha256((REPO / name).read_bytes()).hexdigest() != digest:
            raise PackError(f"loaded metadata implementation changed: {name}")
    return dict(_LOADED_IMPLEMENTATIONS)


def metadata_artifacts(packed, world):
    """Use the ordinary encoders at the final link's existing bin granularity."""
    headers, _images = encode_headers(packed, world)
    files = {"main_reloc_table.bin": encode_main(packed.main),
             "overlay_rom_table.bin": encode_ort(packed.ort_words),
             "overlay_header_table.bin": headers}
    for ov, module in sorted(packed.modules.items()):
        for table in (1, 2):
            data = module.reloc_bytes(table)
            if data:
                files[f"overlays/o{ov:03d}/overlay_{ov:03d}_reloc{table}.bin"] = data
    return files


def emit_metadata(args):
    """Publish exact hybrid metadata; the manifest is the last completion marker."""
    root = Path(args.root).resolve() if args.root else REPO
    paths = resolve_paths(args)
    # Discover inputs, then reload them after the initial snapshot. The probe's
    # cached ELF objects must not stand in for the bytes bound by that snapshot.
    probe = World(paths)
    guard_data_module_imports(probe, OrtNames.load(paths["ort"]))
    inputs = metadata_input_paths(paths, probe)
    before = metadata_digests(inputs, root)
    implementation = metadata_implementations()
    _paths, world, packed = load_and_pack(args)

    def unchanged():
        if (metadata_input_paths(paths, world) != inputs
                or metadata_digests(inputs, root) != before
                or metadata_implementations() != implementation):
            raise PackError("metadata inputs changed during generation")

    unchanged()
    differences = compare_all(packed, world)
    proof = region_proof(packed, world, differences)
    if (packed.problems or differences or proof["raw_equal"] is not True
            or proof["raw_differing_bytes"] != 0 or proof["order_breaks"]):
        raise PackError("metadata requires zero differences/problems and an exact unpatched region")
    borrowing = manifest_from(differences, packed.borrowed)
    expected = json.loads(paths["residuals"].read_text())
    if manifest_keys(borrowing) != manifest_keys(expected):
        raise PackError("metadata borrowing ledger is stale; run the normal table gates and ratchet it")
    files = metadata_artifacts(packed, world)
    ledger = render_manifest(borrowing).encode()
    revision = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                              text=True, capture_output=True)
    manifest = {
        "schema": "mickey-generated-metadata-v1",
        "source_revision": revision.stdout.strip() if revision.returncode == 0 else None,
        "implementation": implementation, "inputs": before,
        "proof": {"raw_equal": True, "raw_differing_bytes": 0, "differences": 0,
                  "region": proof["region"], "source_only": False, "relayout": False},
        "metadata_files": len(files), "metadata_bytes": sum(map(len, files.values())),
        "files": {name: {"size": len(data), "sha256": hashlib.sha256(data).hexdigest()}
                  for name, data in sorted(files.items())},
        "borrowing": {"path": "borrowing.json", "sha256": hashlib.sha256(ledger).hexdigest(),
                      "borrowed_records": borrowing["totals"]["borrowed_records"],
                      "entries_by_kind": {k: v["count"] for k, v in borrowing["borrowed"].items()}},
    }
    files["borrowing.json"] = ledger
    files["manifest.json"] = (json.dumps(manifest, indent=2, sort_keys=True) + "\n").encode()
    out = paths["out"] if args.out else paths["out"] / "metadata"
    if any(path.resolve().is_relative_to(out.resolve()) for path in inputs):
        raise PackError("metadata output overlaps a producer input")
    if out.is_symlink() or (out.exists() and not out.is_dir()):
        raise PackError("metadata output is not an ordinary directory")
    if out.exists():
        for path in out.rglob("*"):
            if path.is_symlink() or (path.is_file() and path.relative_to(out).as_posix() not in files):
                raise PackError("metadata output contains an unexpected file")
        marker = out / "manifest.json"
        if marker.exists() and json.loads(marker.read_text()).get("schema") != manifest["schema"]:
            raise PackError("metadata output has an unrelated manifest")
    out.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".metadata-", dir=out.parent) as temporary:
        stage = Path(temporary)
        for name, data in files.items():
            target = stage / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        unchanged()
        out.mkdir(exist_ok=True)
        # Invalidate the old marker before replacing any payload. A failed
        # publication cannot leave a valid-looking completion manifest behind.
        marker = out / "manifest.json"
        if marker.exists():
            os.replace(marker, stage / "previous-manifest.json")
        for name in sorted(files.keys() - {"manifest.json"}):
            target = out / name
            target.parent.mkdir(parents=True, exist_ok=True)
            os.replace(stage / name, target)
        unchanged()
        os.replace(stage / "manifest.json", marker)
    print(f"wrote exact metadata: {manifest['metadata_files']} bins, {manifest['metadata_bytes']} bytes; "
          f"{manifest['borrowing']['borrowed_records']} borrowed records remain; {out / 'manifest.json'}")
    return 0


def cmd_emit_metadata(args):
    try:
        return emit_metadata(args)
    except (OSError, ValueError) as error:
        raise PackError(f"metadata emission failed: {error}") from error


def cmd_compare(args):
    paths, world, packed = load_and_pack(args)
    residuals = compare_all(packed, world)
    man = manifest_from(residuals, packed.borrowed)
    for line in summarize(residuals, packed.borrowed, packed):
        print(line)
    for p in packed.problems:
        print("problem:", p)
    if args.verbose:
        for ov, pm in sorted(packed.modules.items()):
            for n in pm.notes:
                print(f"  note         o{ov:03d}: {n}")
        for r in sorted(residuals, key=lambda r: (r.category, r.module, r.offset)):
            print(f"  {r.category:12s} o{r.module:03d} +{hexs(r.offset)}: expected {r.expected}; got {r.got}")
    if args.json:
        Path(args.json).write_text(json.dumps(man, indent=1) + "\n")
    if args.write_residuals:
        paths["residuals"].write_text(render_manifest(man))
        print(f"wrote {paths['residuals']}")
        return 0
    if not args.check:
        return 0
    if not paths["residuals"].exists():
        print(f"FAIL: {paths['residuals']} is missing; run `gmake runlink-residuals`")
        return 1
    want = manifest_keys(json.loads(paths["residuals"].read_text()))
    have = manifest_keys(json.loads(render_manifest(man)))
    new, gone = sorted(have - want, key=str), sorted(want - have, key=str)
    if packed.problems:
        print(f"FAIL: {len(packed.problems)} input problem(s) above")
    for k in new[:50]:
        print("  NEW   ", " ".join(str(x) for x in k))
    for k in gone[:50]:
        print("  GONE  ", " ".join(str(x) for x in k))
    if new or gone or packed.problems:
        print(f"FAIL check-runlink-tables: {len(new)} residual(s) not in the manifest, "
              f"{len(gone)} listed residual(s) no longer occur")
        if gone and not new:
            print("  progress: run `gmake runlink-residuals` to ratchet the manifest down")
        return 1
    print(f"OK  runlink tables: every difference and borrowing is a listed residual "
          f"({len(residuals)} differences, {len(have) - len(residuals)} borrowed entries)")
    return 0


def cmd_region(args):
    paths, world, packed = load_and_pack(args)
    residuals = compare_all(packed, world)
    proof = region_proof(packed, world, residuals)
    print(f"overlay region {hexs(proof['region'][0])}-{hexs(proof['region'][1])}")
    print(f"  packer output: {'identical' if proof['raw_equal'] else 'differs'}"
          f" ({proof['raw_differing_bytes']} bytes differ)")
    print(f"  with the {len(residuals)} listed differences replaced by the shipped records:"
          f" {'identical' if proof['patched_equal'] else 'differs'}"
          f" ({proof['patched_differing_bytes']} bytes differ)")
    if proof["order_breaks"]:
        print(f"  record order differs outside the residuals in {proof['order_breaks']}")
    if not proof["patched_equal"]:
        if proof["patched_first_difference"] is not None:
            print(f"  first difference outside the residuals at ROM {hexs(proof['patched_first_difference'])}")
        return 1
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", help="repository root (default: this checkout)")
    for k in DEFAULTS:
        ap.add_argument(f"--{k}", help=f"override {DEFAULTS[k]}")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("ort", help="generate the ORT name list")
    g = p.add_mutually_exclusive_group()
    g.add_argument("--write", action="store_true")
    g.add_argument("--check", action="store_true")
    sub.add_parser("pack", help="write the packed tables under build/runlink/")
    sub.add_parser("emit-metadata", help="emit exact table bins and a hashed hybrid borrowing manifest")
    p = sub.add_parser("compare", help="classify every difference from the shipped tables")
    g = p.add_mutually_exclusive_group()
    g.add_argument("--check", action="store_true", help="fail on residuals the manifest does not list")
    g.add_argument("--write-residuals", action="store_true", help="rewrite the residual manifest")
    p.add_argument("--verbose", "-v", action="store_true")
    p.add_argument("--json", help="also write the would-be manifest to this path")
    sub.add_parser("region", help="byte-compare the whole overlay region")
    args = ap.parse_args(argv)
    try:
        return {"ort": cmd_ort, "pack": cmd_pack, "emit-metadata": cmd_emit_metadata,
                "compare": cmd_compare, "region": cmd_region}[args.cmd](args)
    except PackError as e:
        print(f"runlink_pack: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
