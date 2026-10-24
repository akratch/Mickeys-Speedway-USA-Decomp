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
import trap_names as trap_source  # noqa: E402

_LOADED_IMPLEMENTATIONS = {
    "tools/runlink_pack.py": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    "tools/overlay_tables.py": hashlib.sha256(Path(ot.__file__).read_bytes()).hexdigest(),
    "tools/trap_names.py": hashlib.sha256(Path(trap_source.__file__).read_bytes()).hexdigest(),
}

# The provider helper imports us: pin before any later lazy import, without
# changing the existing fixed metadata-producer implementation schema.
_LOADED_READONLY_PROVIDER = hashlib.sha256((REPO / "tools/link_overlay_module.py").read_bytes()).hexdigest()


def readonly_provider_identity():
    path = REPO / "tools/link_overlay_module.py"
    if hashlib.sha256(path.read_bytes()).hexdigest() != _LOADED_READONLY_PROVIDER:
        raise PackError("loaded readonly provider helper changed: link_overlay_module.py")
    return path, _LOADED_READONLY_PROVIDER


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


def parse_map(path, data=None):
    """Every placed input section: (output section, input section, address, size, file)."""
    out = []
    started = False
    cur = None
    pending = None
    head = re.compile(r"^(\.\S+)(?:\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+))?")
    one = re.compile(r"^ (\.\S+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)\s*$")
    name_only = re.compile(r"^ (\.\S+)\s*$")
    cont = re.compile(r"^\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)\s*$")
    if data is None:
        data = Path(path).read_bytes()
    for line in data.decode(errors="replace").splitlines():
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
        self.map_data = paths["map"].read_bytes()
        self.map = parse_map(paths["map"], self.map_data)
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
                compiler = Elf(p)
                target = f"build/src/{row["source"]}.c.o"
                if row["type"] == "c" and target in getattr(self, "_end_padding_plans", {}):
                    proved = getattr(self, "_end_proved_objects", {}).get(row["source"])
                    if proved is None:
                        raise PackError("compiler-end source inventory is unavailable")
                    compiler = proved
                lst.append(ObjRef(ov, p, row["source"], int(row["offset"], 16),
                                  int(row["size"], 16), row["type"], compiler))
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
    runtime_role: str | None = None  # selected ordinary input: data, ro-origin, ro-member
    runtime_origin: int | None = None  # held actual same-module origin input

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
        self._resident_functions = None
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
                    if name not in self.ort.by_name:
                        local = getattr(obj.elf, "data_local_identities", None)
                        if (getattr(obj.elf, "local_ro_provider", None) and local is not None
                                and name in local and obj.data_imports is not None
                                and obj.data_imports.get(name) == local[name]):
                            continue  # This proven LOCAL identity belongs only to its input.
                        raise PackError(f"ORT lacks its held provider identity: {name}")
                    if obj.data_imports is None or name not in obj.data_imports:
                        raise PackError(f"ORT lacks its held provider identity: {name}")
                    ident = ("ov", *obj.data_imports[name])
                    if name in targets and targets[name] != ident:
                        raise PackError(f"ORT has ambiguous held provider identities: {name}")
                    targets[name] = ident
        self._ort_ident = None
        self._resident_functions = None
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
        if RES_AUTO_RE.fullmatch(name):
            ident = self._resident_function_identity(name)
            if ident is not None:
                return Target("ident", name, "source-resident-function", ident=ident, named=True)
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

    def _resident_function_identity(self, name):
        """Qualified current function ownership, independent of encoded spelling."""
        final = [s for s in self.w.by_name.get(name, ())
                 if s.bind == STB_GLOBAL and s.shndx != SHN_UNDEF]
        qualified = [s for s in final if s.type == STT_FUNC and s.size > 0
                     and s.section in (".main", ".entry")]
        if not qualified:
            return None  # Data, ABS, boundaries and missing definitions stay compatibility.
        if len(final) != 1:
            raise PackError(f"ambiguous resident function definition: {name}")
        if self._resident_functions is None:
            hold_data_import_inputs(self.w, {
                self.w.paths["elf"]: hashlib.sha256(self.w.elf.data).hexdigest(),
                self.w.paths["map"]: hashlib.sha256(self.w.map_data).hexdigest()})
            definitions = collections.defaultdict(list)
            loaded = {}
            for mi in self.w.map:
                if (mi.outsec not in (".main", ".entry") or mi.insec != ".text"
                        or not mi.size or not mi.file.endswith(".o")):
                    continue
                path = self.w.paths["build"].parent / mi.file
                if path not in loaded:
                    loaded[path] = Elf(path)
                    hold_data_import_inputs(self.w, {
                        path: hashlib.sha256(loaded[path].data).hexdigest()})
                elf = loaded[path]
                for symbol in elf.symbols:
                    if (symbol.name and symbol.bind == STB_GLOBAL
                            and symbol.shndx not in (SHN_UNDEF, SHN_ABS, SHN_COMMON)
                            and symbol.section == ".text"):
                        definitions[symbol.name].append((mi, elf, symbol))
            self._resident_functions = definitions  # Never expose a partial index.
        choices = self._resident_functions.get(name, ())
        if len(choices) != 1:
            raise PackError(f"resident function lacks a unique source input: {name}")
        mi, elf, symbol = choices[0]
        raw = elf.section(".text")
        source = ((mi.file.startswith("build/src/") and mi.file.endswith(".c.o"))
                  or (mi.file.startswith("build/asm/") and mi.file.endswith(".s.o")))
        if (not source or int.from_bytes(elf.data[16:18], "big") != 1
                or not raw or raw["type"] != 1 or raw["flags"] & 6 != 6 or raw["addr"] != 0
                or symbol.type != STT_FUNC or symbol.size <= 0 or symbol.value < 0
                or symbol.value % 4 or symbol.size % 4
                or symbol.value + symbol.size > raw["size"]
                or len(elf.bytes_of(".text")) != raw["size"]):
            raise PackError(f"resident function lacks a bounded real source function: {name}")
        canonical = final[0]
        section = self.w.elf.section(canonical.section)
        if (not section or section["type"] != 1 or section["flags"] & 6 != 6
                or canonical.value < section["addr"]
                or canonical.value + canonical.size > section["addr"] + section["size"]
                or mi.addr % 4 or canonical.value % 4
                or symbol.value + symbol.size > mi.size or canonical.section != mi.outsec
                or canonical.value != mi.addr + symbol.value or canonical.size != symbol.size
                or not self.layout.contains(canonical.value)
                or not self.layout.contains(canonical.value + symbol.size - 1)):
            raise PackError(f"resident function source/final input geometry differs: {name}")
        start, end = canonical.value, canonical.value + symbol.size
        if any(other is not mi and other.outsec == mi.outsec and other.size
               and other.addr < end and start < other.addr + other.size for other in self.w.map):
            raise PackError(f"resident function has overlapping final input ownership: {name}")
        return ("res", mi.addr + symbol.value)

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
                    if t.kind == "section" and t.obj.runtime_role is not None:
                        # Writable compiler storage is independent of the loader's
                        # DATA/RO partition. The selected source input declares its
                        # role; its real map start supplies the RO origin.
                        if t.secname != ".data":
                            raise PackError("ordinary runtime role targets a non-data section")
                        runtime_base = (t.obj.runtime_origin if t.obj.runtime_role == "ro-member" else
                                        section_offset(t.obj, ".data") if t.obj.runtime_role == "ro-origin" else ts)
                        if runtime_base is None:
                            raise PackError("RO member has no authenticated origin")
                    else:
                        kind = t.secname if t.kind == "section" else kind_of_offset(base[2])
                        runtime_base = local_base(kind)
                    rec = Record(2, OP_LOCAL, runtime_base, site, c.type, full, name)
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


def resident_data_callees(elf, record):
    """Authenticate named callback initializers against the configured object.

    Renaming changes only ELF symbol names. Zero .data operands alone cannot
    distinguish a callback from NULL, so bind the complete object and require
    the same bounded, zero-addend R32 relocations as the source-side receipt.
    """
    if (record.get("format") != 2
            or record.get("object_sha256") != hashlib.sha256(elf.data).hexdigest()
            or record.get("data_sha256") != hashlib.sha256(elf.bytes_of(".data")).hexdigest()):
        raise PackError("stale callback .trapsites sidecar (rebuild the object)")
    section = elf.section(".data")
    if (not section or section["type"] != 1 or section["flags"] & 3 != 3
            or section["flags"] & 4 or section["addr"] != 0):
        raise PackError("callback initializer lacks ordinary writable .data")
    sites = record.get("data_sites")
    literal = record.get("data_literal")
    if not isinstance(sites, list) or not isinstance(literal, list):
        raise PackError("invalid callback .trapsites inventory")
    names = {}
    for row in sites:
        if (not isinstance(row, dict) or set(row) != {"offset", "name"}
                or type(row["offset"]) is not int or not isinstance(row["name"], str)
                or not re.fullmatch(r"[A-Za-z_]\w*", row["name"]) or row["name"] == TRAP
                or row["offset"] in names):
            raise PackError("invalid named callback .trapsites entry")
        names[row["offset"]] = row["name"]
    if (any(type(offset) is not int for offset in literal)
            or len(set(literal)) != len(literal) or set(literal) & set(names)):
        raise PackError("invalid literal callback .trapsites entry")
    actual = []
    for rel in elf.rels(".data"):
        if rel.sym.name != TRAP:
            continue
        if (rel.type != R_MIPS_32 or rel.sym.shndx != SHN_UNDEF
                or rel.offset < 0 or rel.offset % 4 or rel.offset + 4 > section["size"]
                or u32(elf.bytes_of(".data"), rel.offset) != 0):
            raise PackError("callback initializer requires a bounded zero-addend .data R32")
        actual.append(rel.offset)
    if (len(actual) != len(set(actual))
            or set(actual) != set(names) | set(literal)):
        raise PackError("callback .trapsites relocation inventory differs")
    return names


def address_targets(dump):
    return dict(re.findall(r"^(build/asm/main/[^/]+\.s\.o): ASM_ADDRESS_EXPORT = ([A-Za-z_]\w*)$",dump,re.M))


def resident_address_names(world, item, path, elf, export):
    """Validate names from the original assembled input; no retail-site lookup."""
    import trap_names as trap
    raw_path = Path(str(path)+trap.ADDRESS_RAW_SUFFIX)
    receipt_path = Path(str(path)+trap.ADDRESS_SUFFIX)
    try:
        raw_bytes,receipt_bytes = raw_path.read_bytes(),receipt_path.read_bytes()
        receipt = json.loads(receipt_bytes)
        source_name = item.file.removeprefix("build/").removesuffix(".o")
        if (receipt.get("export") != export or receipt.get("source") != source_name
                or not re.fullmatch(r"asm/main/[^/]+\.s",source_name)
                or item.outsec != ".main" or item.insec != ".text"
                or item.size != elf.size(".text")):
            raise PackError("paired-address receipt lacks its complete selected ASM owner")
        source = world.paths["build"].parent/source_name
        source_bytes = source.read_bytes()
        if hashlib.sha256(source_bytes).hexdigest() != receipt.get("source_sha256"):
            raise PackError("stale paired-address source receipt")
        names = trap.validate_addresses(elf.data,raw_bytes,receipt)
    except (OSError,ValueError,struct.error,trap.TrapError) as error:
        raise PackError("paired-address proof refused: "+str(error)) from error
    hold_data_import_inputs(world,{path:hashlib.sha256(elf.data).hexdigest(),raw_path:hashlib.sha256(raw_bytes).hexdigest(),
        receipt_path:hashlib.sha256(receipt_bytes).hexdigest(),source:hashlib.sha256(source_bytes).hexdigest(),
        world.paths["elf"]:hashlib.sha256(world.elf.data).hexdigest(),
        world.paths["map"]:hashlib.sha256(world.map_data).hexdigest()})
    return names


def source_bss_identity(world, name):
    """Unique actual NOBITS source definition, independent of name spelling."""
    import source_module_link_proof as proof
    import proof_provenance as provenance
    choices = [s for s in world.by_name.get(name,()) if s.bind == STB_GLOBAL and s.shndx != SHN_UNDEF]
    if len(choices) != 1:
        raise PackError("data address lacks one actual canonical export: "+name)
    symbol = choices[0]
    match = re.fullmatch(r"\.overlay_(\d{3})_bss",symbol.section)
    if not match:
        raise PackError("data address is not an allocated overlay NOBITS export")
    ov = int(match.group(1))
    try:
        identity = proof.canonical_bss_export(world.elf,symbol,0,world.shipped.rom,world.shipped.modules)
    except provenance.MetadataProofError as error:
        raise PackError("data address BSS geometry refused: "+str(error)) from error
    inputs = [i for i in world.map if i.outsec == symbol.section and i.insec == ".bss"
              and i.size and i.addr <= symbol.value < i.addr+i.size]
    if len(inputs) != 1:
        raise PackError("data address lacks a unique NOBITS source input")
    item = inputs[0]
    if not re.fullmatch(r"build/asm/(?:data/)?overlays/o\d{3}/[^/]+\.s\.o",item.file):
        raise PackError("data address provider is not an actual source NOBITS object")
    root=world.paths["build"].parent
    path=root/item.file;raw=Elf(path)
    section=raw.section(".bss")
    definitions=[s for s in raw.symbols if s.name == name and s.bind == STB_GLOBAL]
    if (struct.unpack_from(">HH",raw.data,16)!=(1,8)
            or not section or section["type"] != SHT_NOBITS or section["flags"]&3 !=3
            or section["flags"]&4 or section["addr"] !=0 or section["size"] !=item.size
            or len(definitions)!=1 or definitions[0].section != ".bss"
            or definitions[0].type not in (0,1) or definitions[0].size !=symbol.size
            or not 0 <= definitions[0].value < item.size
            or definitions[0].value+max(1,definitions[0].size)>item.size
            or item.addr+definitions[0].value !=symbol.value
            or any(raw.size(s) for s in (".text",".data",".rodata"))
            or any(raw.rels(s["name"]) for s in raw.sections)):
        raise PackError("data address raw source/final NOBITS definition differs")
    source=root/item.file.removeprefix("build/").removesuffix(".o")
    payload=source.read_bytes()
    hold_data_import_inputs(world,{path:hashlib.sha256(raw.data).hexdigest(),
        source:hashlib.sha256(payload).hexdigest(),world.paths["elf"]:hashlib.sha256(world.elf.data).hexdigest(),
        world.paths["map"]:hashlib.sha256(world.map_data).hexdigest()})
    return identity


def address_export_row(world, ort, name):
    identity=source_bss_identity(world,name)
    target=next(s for s in world.by_name[name] if s.bind==STB_GLOBAL and s.shndx!=SHN_UNDEF)
    rows=[]
    for row,export in enumerate(ort.rows):
        choices=[s for s in world.by_name.get(export,()) if s.bind==STB_GLOBAL and s.shndx!=SHN_UNDEF]
        if len(choices)==1 and choices[0].section==target.section and choices[0].value==target.value:
            if source_bss_identity(world,export)==identity:rows.append(row)
    if len(rows)!=1:
        raise PackError("data address lacks its unique identity-selected ORT row")
    return rows[0]


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
        hold_data_import_inputs(world, {path: hashlib.sha256(elf.data).hexdigest()})
        rels = elf.rels(mi.insec)
        if not rels and not mi.file.endswith(".c.o"):
            continue
        sidecar = {}
        address_names = {}
        address_export = getattr(world,"_address_targets",{}).get(mi.file)
        if address_export and mi.insec == ".text":
            address_names = resident_address_names(world,mi,path,elf,address_export)
        if mi.file.endswith(".c.o"):
            sc = Path(str(path) + ".trapsites")
            if sc.exists():
                payload = sc.read_bytes()
                hold_data_import_inputs(world, {sc: hashlib.sha256(payload).hexdigest()})
                rec = json.loads(payload)
                if mi.insec == ".text":
                    if rec.get("text_sha256") != hashlib.sha256(elf.bytes_of(".text")).hexdigest():
                        problems.append(f"{mi.file}: stale .trapsites sidecar (rebuild the object)")
                    sidecar = {s["offset"]: s["name"] for s in rec.get("sites", [])}
                elif rec.get("data_sites") or rec.get("data_literal"):
                    sidecar = resident_data_callees(elf, rec)
                    if (not mi.file.startswith("build/src/main/")
                            or mi.size != elf.size(".data")):
                        raise PackError(f"{mi.file}: callback lacks a complete resident source input")
                    for offset in sidecar:
                        site = mi.addr + offset
                        owners = [other for other in resident_in
                                  if other.addr < site + 4 and site < other.addr + other.size]
                        if (len(owners) != 1 or owners[0] is not mi
                                or not layout.data <= site <= layout.bss - 4):
                            raise PackError(f"{mi.file}: callback lacks unique resident data ownership")
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
                if r.offset in address_names:
                    source_name=address_names[r.offset]
                    row=address_export_row(world,resolver.ort,source_name)
                    text_entries.append(MainEntry(row,addr-layout.text,r.type,OP_SYMBOL,addr,
                                                  source_name,"object",mi.file))
                else:
                    text_entries.append(MainEntry(-1, addr - layout.text, r.type, OP_SYMBOL, addr,
                                                  SENTINEL, "placeholder", mi.file))
            elif mi.insec == ".data" and r.type == R_MIPS_32 and name == TRAP:
                callee = sidecar.get(r.offset)
                ent = MainEntry(-1, addr - layout.data, r.type, OP_DATA, addr,
                                callee or TRAP, "placeholder", mi.file)
                if callee is not None:
                    definition = resolver._function_proxy_definition(callee)
                    if definition is None:
                        raise PackError(f"resident callback lacks an actual source function: {callee}")
                    obj, symbol = definition
                    ident = ("ov", obj.module, obj.text_offset + symbol.value)
                    target = Target("section", callee, "source-callback", obj=obj,
                                    secname=".text", value=symbol.value, named=True)
                    row = resolver.export_row(target, ident)
                    if row is None:
                        raise PackError(f"resident callback has no matching ORT export: {callee}")
                    ent.row, ent.provenance = row, "object"
                elif mi.file.endswith(".c.o"):
                    raise PackError(f"{mi.file}+{hexs(r.offset)}: resident C callback lacks a fresh named sidecar")
                data_entries.append(ent)
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


def reproduce_compiler_end(world, source):
    """Bind original code/pool records and exact ordinary metadata replay."""
    import proof_provenance as provenance
    import time
    root = world.paths["build"].parent
    configured = world.paths["build"] / "src" / (source + ".c.o")
    source_path = root / "src" / (source + ".c")
    if provenance._all_pragmas(source_path.read_text()):
        raise PackError("compiler-end source has an assembly fallback")
    raw, plan, receipt, context = provenance.capture_nonexec_producer_raw(
        root, source_path, configured, world.paths["elf"], world.paths["map"])
    original = Elf(raw)
    ordinary = world.paths["raw"] / "src" / (source + ".c.o")
    loaded_ordinary = Elf(ordinary)
    validate_fresh_data_input(loaded_ordinary, original)
    if hashlib.sha256(original.data).hexdigest() != receipt["raw_sha256"]:
        raise PackError("loaded compiler-end object differs from its compiler receipt")
    hold_data_import_inputs(world, {ordinary: hashlib.sha256(loaded_ordinary.data).hexdigest()})
    target = f"build/src/{source}.c.o"
    command = receipt["inputs"]["postprocess"]
    if receipt["inputs"]["surface_rebind"]:
        command += " && " + receipt["inputs"]["surface_rebind"]
    replay = raw.with_name("metadata-replay.o")
    provenance.replay_metadata(raw, command, target, replay, root, skip_filters=False,
                               deadline=time.monotonic()+120, plan=plan)
    loaded_configured = Elf(configured)
    configured_digest = hashlib.sha256(loaded_configured.data).hexdigest()
    if configured_digest != receipt["configured_sha256"] or configured_digest != receipt["inputs"]["configured"]:
        raise PackError("loaded compiler-end configured object differs from its receipt")
    hold_data_import_inputs(world, {configured: configured_digest})
    validate_fresh_data_input(loaded_configured, Elf(replay))
    allowed = dict(arguments for operation, arguments in plan if operation == "nonexec-end")
    if not allowed:
        raise PackError("compiler-end source lacks its explicit normalization")
    view = effective_compiler_object(original, loaded_configured, allowed)
    if getattr(view, "effective_extents", {}) != allowed:
        raise PackError("compiler-end recipe and effective source sections differ")
    hold_data_capture(world, source, receipt, context)
    if receipt["inputs"] != context() or provenance.sha256_file(raw) != receipt["raw_sha256"]:
        raise PackError("compiler-end source/context or raw object changed during replay")
    receipt["effective_sections"] = {"original_sha256": hashlib.sha256(original.data).hexdigest(),
        "extents": view.effective_extents, "replayed_sha256": provenance.sha256_file(replay)}
    world._compiler_end_receipts[source] = receipt
    return view


def effective_compiler_object(raw, configured, allowed=None, *, literal_proofs=None, root=None):
    """Explicit semantic view; original compiler bytes/records remain attached."""
    import trim_elf_section as trim
    data = bytearray(raw.data)
    extents = {}
    for section in (".data", ".rodata"):
        size = configured.size(section)
        if size and size < raw.size(section):
            try:
                if section in (literal_proofs or {}):
                    import proof_provenance as provenance
                    end = provenance.validate_literal_proof(raw, literal_proofs[section], root)
                else:
                    end = trim.compiler_end(raw, section)
            except (ValueError, struct.error, RuntimeError) as error:
                raise PackError("unproved compiler end: " + str(error)) from error
            if end != size or (allowed or {}).get(section) != end:
                raise PackError("configured section lacks its declared proved compiler end")
            shoff = struct.unpack_from(">I", data, 32)[0]
            struct.pack_into(">I", data, shoff + raw.section(section)["index"]*40 + 20, end)
            extents[section] = end
    if not extents:
        return raw
    view = Elf(raw.path, bytes(data))
    view.compiler_original = raw
    view.effective_extents = extents
    return view


def data_import_input(world, ov, row, source, raw, *, linked_local=False, storage=".data"):
    """Authenticate one complete data input; no inferred placement or tail trim."""
    import proof_provenance as provenance
    remember_data_import_inputs(world, source)
    source_path = world.paths["build"].parent / "src" / (source + ".c")
    if provenance._all_pragmas(source_path.read_text()):
        raise PackError(f"data-only input contains an assembly fallback: {source}")
    configured = Elf(world.paths["build"] / "src" / (source + ".c.o"))
    hold_data_import_inputs(world, {configured.path: hashlib.sha256(configured.data).hexdigest()})
    ordinary_path = world.paths["raw"] / "src" / (source + ".c.o")
    if raw.path == ordinary_path:
        hold_data_import_inputs(world, {ordinary_path: hashlib.sha256(getattr(raw, "compiler_original", raw).data).hexdigest()})
    target = f"build/src/{source}.c.o"
    literal = getattr(world, "_literal_array_plans", {}).get(target)
    literal_proofs = None
    if literal:
        providers = getattr(world, "data_import_providers", {})
        if source not in providers:
            providers[source] = reproduce_data_provider(world, source)
            world.data_import_providers = providers
        fresh = providers[source]
        validate_fresh_data_input(getattr(raw, "compiler_original", raw),
                                  getattr(fresh, "compiler_original", fresh))
        literal_proofs = getattr(fresh, "literal_end_proofs", None)
        if not literal_proofs:
            raise PackError("literal input lacks its fresh source proof")
    raw = effective_compiler_object(raw, configured, getattr(world, "_end_padding_plans", {}).get(target),
        literal_proofs=literal_proofs, root=world.paths["build"].parent)
    size = raw.size(storage)
    def require(test, why):
        if not test:
            raise PackError(f"data-only module {source}: {why}")
    require(storage in (".data", ".rodata") and row.get("section", ".data") == storage and size > 0
            and size == int(row["size"], 16), "not one complete canonical initialized input")
    for obj in (raw, configured):
        require(all(obj.size(s) == 0 for s in (".text", ".data", ".rodata", ".bss") if s != storage),
                "mixed runtime sections are unsupported")
        require(not any(s.shndx == SHN_COMMON and s.size for s in obj.symbols),
                "COMMON allocation is unsupported")
        require(not any(s["size"] and s["flags"] & 2
                        and s["name"] not in (storage, ".reginfo", ".MIPS.abiflags")
                        for s in obj.sections), "unexpected allocated section")
        sec = obj.section(storage)
        require(sec and sec["type"] == 1 and sec["flags"] & 3 == (2 if storage == ".rodata" else 3)
                and not sec["flags"] & 4 and sec["size"] == size, "invalid data storage")
    require(linked_local or raw.bytes_of(storage) == configured.bytes_of(storage),
            "raw/configured initialized bytes differ")
    items = [mi for mi in world.map if mi.file == f"build/src/{source}.c.o"
             and mi.insec == storage and mi.size]
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
            == configured.bytes_of(storage), "final initialized input bytes differ")
    return configured, start


def reproduce_data_provider(world, source):
    """Fresh ordinary-provider compile, sharing the producer capture machinery."""
    import proof_provenance as provenance
    root = world.paths["build"].parent
    configured = world.paths["build"] / "src" / (source + ".c.o")
    raw, plan, receipt, context = provenance.capture_data_provider_raw(
        root, root / "src" / (source + ".c"), configured,
        world.paths["elf"], world.paths["map"])
    fresh = Elf(raw)
    if hashlib.sha256(fresh.data).hexdigest() != receipt["raw_sha256"]:
        raise PackError("loaded ordinary provider differs from its compiler receipt")
    validate_fresh_data_input(Elf(world.paths["raw"] / "src" / (source + ".c.o")), fresh)
    original = fresh
    loaded_configured = Elf(configured)
    configured_digest = hashlib.sha256(loaded_configured.data).hexdigest()
    if configured_digest != receipt["configured_sha256"] or configured_digest != receipt["inputs"]["configured"]:
        raise PackError("loaded ordinary provider differs from its configured receipt")
    hold_data_import_inputs(world, {configured: configured_digest})
    allowed = {arguments[0]: arguments[1] for operation, arguments in plan
               if operation in ("nonexec-end", "literal-end")}
    literal_proofs = receipt.get("literal_arrays")
    fresh = effective_compiler_object(fresh, loaded_configured, allowed,
        literal_proofs=literal_proofs, root=root)
    if getattr(fresh, "effective_extents", None):
        literal_section = next(iter(literal_proofs)) if literal_proofs else None
        expected = ([("literal-end", (literal_section, fresh.size(literal_section), f"src/{source}.c"))]
                    if literal_proofs else [("nonexec-end", (".data", fresh.size(".data")))])
        if plan != expected:
            raise PackError("effective ordinary provider lacks its declared compiler-end recipe")
        if literal_proofs:
            fresh.literal_end_proofs = literal_proofs
        receipt["effective_sections"] = {"original_sha256": hashlib.sha256(original.data).hexdigest(),
                                         "extents": fresh.effective_extents}
    elif plan:
        raise PackError("unexpected ordinary provider metadata profile")
    validate_fresh_data_input(loaded_configured, fresh)
    hold_data_capture(world, source, receipt, context)
    receipts = getattr(world, "data_import_provider_receipts", {})
    receipts[source] = receipt
    world.data_import_provider_receipts = receipts
    if receipt["inputs"] != context() or provenance.sha256_file(raw) != receipt["raw_sha256"]:
        raise PackError("provider source/context or original object changed during fresh compilation")
    return fresh


def data_import_definition(world, name, *, base_only=False, provider_source=None, readonly_provider=False):
    """A real raw C definition and unique map input, never an encoded address."""
    choices = [s for s in world.by_name.get(name, ()) if s.bind == STB_GLOBAL
               and s.type == 1 and (base_only or s.size > 0)
               and s.shndx not in (SHN_ABS, SHN_UNDEF)]
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
            if (item.file == f"build/src/{source}.c.o" and item.insec in ((".data", ".rodata") if readonly_provider else (".data",))
                    and item.insec == row.get("section", ".data")
                    and item.addr <= symbol.value < item.addr + item.size
                    and symbol.value + symbol.size <= item.addr + item.size):
                hits.append((row, source, item.insec))
    if len(hits) != 1:
        raise PackError(f"data-only import target lacks a unique canonical source input: {name}")
    row, source, storage = hits[0]
    if provider_source is not None and source != provider_source:
        raise PackError(f"LOCAL data target is not in its declared provider: {name}")
    raw = Elf(world.paths["raw"] / "src" / (source + ".c.o"))
    _configured, start = data_import_input(world, ov, row, source, raw, storage=storage)
    if any(raw.rels(s) for s in (".text", ".data", ".rodata")):
        raise PackError(f"data-only import provider has unsupported relocations: {name}")
    providers = getattr(world, "data_import_providers", {})
    if source not in providers:
        providers[source] = reproduce_data_provider(world, source)
    world.data_import_providers = providers
    fresh = providers[source]
    definitions = [s for s in fresh.symbols if s.name == name and s.section == storage
                   and s.bind == STB_GLOBAL and s.type == 1 and (base_only or s.size > 0)]
    if len(definitions) != 1:
        raise PackError(f"data-only import is not defined by its raw C input: {name}")
    own = definitions[0]
    if (own.size != symbol.size or own.value + start != symbol.value - OVERLAY_VMA
            or own.value < 0 or own.value + max(1, own.size) > raw.size(storage)):
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


def validate_cross_input_local(world, exports, ov, row, source, raw):
    """A complete pointer input names actual bases in one runtime RO origin."""
    import source_module_link_proof as proof
    grouped = getattr(raw, "local_ro_providers", None)
    aggregate = getattr(raw, "local_ro_aggregate", None)
    provider_sources = [name.removeprefix("build/src/").removesuffix(".c.o")
                        for name in (grouped or (raw.local_ro_provider,))]
    mixed = getattr(raw, "local_ro_mixed_group", False)
    if mixed:
        if not grouped or len(grouped) != 2 or not aggregate:
            raise PackError("mixed LOCAL group lacks its declared complete providers")
        roles, origins, sections = getattr(world, "_plain_data_declarations", ({}, {}, {}))
        member = grouped[1]
        if (member not in getattr(world, "_literal_array_plans", {})
                or roles.get(member) != "ro-member" or origins.get(member) != grouped[0]
                or sections.get(member) != ".rodata"):
            raise PackError("mixed LOCAL group lacks its source-proved literal RO member")
        member_role = plain_data_role_context(world, {member: roles[member]},
            {member: origins[member]}, {member: sections[member]})[member]
    configured, start = data_import_input(world, ov, row, source, raw, linked_local=True)
    rels = raw.rels(".data")
    if grouped or aggregate:
        owners = [s for s in raw.symbols if s.section == ".data" and s.bind == STB_GLOBAL
                  and s.type == 1 and s.size]
        if aggregate:
            import link_overlay_module as module_link
            try:
                if module_link.local_aggregate_shape(raw) != aggregate:
                    raise ValueError("aggregate receipt shape differs")
            except (ValueError, module_link.LinkError) as error:
                raise PackError("LOCAL aggregate shape refused: " + str(error)) from error
        final = world.by_name.get(owners[0].name, ()) if len(owners) == 1 else ()
        if (len(owners) != 1 or owners[0].value != 0 or (not aggregate and owners[0].size != raw.size(".data"))
                or len(final) != 1 or final[0].bind != STB_GLOBAL or final[0].type != 1
                or final[0].section != f".overlay_{ov:03d}" or final[0].size != owners[0].size
                or final[0].value != OVERLAY_VMA + start):
            raise PackError("grouped LOCAL data lacks its complete raw/final aggregate")
    if (not rels or (not (grouped or aggregate) and (len(rels) * 4 != raw.size(".data")
            or raw.bytes_of(".data") != bytes(raw.size(".data"))))
            or any(configured.rels(s) for s in (".text", ".data", ".rodata"))):
        raise PackError("LOCAL data is not a complete zero-addend pointer input")
    targets = {}
    offsets = []
    for index, (rel, addend) in enumerate(zip(rels, in_place_addends(raw, ".data", rels))):
        if ((not (grouped or aggregate) and rel.offset != index * 4) or rel.offset % 4
                or not 0 <= rel.offset <= raw.size(".data") - 4 or addend != 0
                or rel.type != R_MIPS_32 or rel.sym.shndx != SHN_UNDEF
                or rel.sym.bind != STB_GLOBAL or rel.sym.name in exports.by_name):
            raise PackError("unsupported cross-input LOCAL data import")
        offsets.append(rel.offset)
        target = data_import_definition(world, rel.sym.name, base_only=True,
                                        provider_source=None if grouped else provider_sources[0], readonly_provider=bool(aggregate))
        if target[0] != ov:
            raise PackError("LOCAL data provider escapes its module")
        targets[rel.sym.name] = target
    if offsets != sorted(set(offsets)):
        raise PackError("unordered or duplicate grouped LOCAL sites")
    origin = None
    end = None
    allowed = set()
    for provider_index, provider_source in enumerate(provider_sources):
        # Authenticate even a complete declared member not named by a relocation.
        rows = [r for r in projected_data_rows(world.amods[ov]) if r["source"] == provider_source]
        if len(rows) != 1:
            raise PackError("LOCAL provider lacks one canonical input")
        ordinary = Elf(world.paths["raw"] / "src" / (provider_source + ".c.o"))
        storage = rows[0].get("section", ".data") if aggregate else ".data"
        if aggregate:
            import link_overlay_module as module_link
            try:
                if module_link.local_provider_section(ordinary, readonly=True) != storage:
                    raise ValueError("provider storage/row differs")
            except (ValueError, module_link.LinkError) as error:
                raise PackError("LOCAL readonly provider refused: " + str(error)) from error
        _provider_configured, provider_start = data_import_input(world, ov, rows[0], provider_source, ordinary, storage=storage)
        providers = getattr(world, "data_import_providers", {})
        if provider_source not in providers:
            providers[provider_source] = reproduce_data_provider(world, provider_source)
        world.data_import_providers = providers
        provider = providers[provider_source]
        if any(provider.rels(n) for n in (".text", ".data", ".rodata")):
            raise PackError("LOCAL provider has relocations")
        if origin is None:
            origin = provider_start
        elif grouped and provider_start != end:
            raise PackError("LOCAL provider group is not contiguous in declared order")
        end = provider_start + provider.size(storage)
        if mixed:
            expected_storage = ".rodata" if provider_index == 0 else ".data"
            definitions = [symbol for symbol in provider.symbols if symbol.section == storage
                           and symbol.bind == STB_GLOBAL and symbol.type == 1]
            if (storage != expected_storage or not definitions
                    or (provider_index == 0 and min(symbol.value for symbol in definitions) != 0)
                    or (provider_index == 1 and (member_role["origin"] != origin
                        or member_role["input_start"] != provider_start
                        or not getattr(provider, "literal_end_proofs", None)))):
                raise PackError("mixed LOCAL group provider storage/runtime origin differs")
        allowed.update((ov, provider_start + symbol.value) for symbol in provider.symbols
                       if symbol.section == storage and symbol.bind == STB_GLOBAL and symbol.type == 1
                       and 0 <= symbol.value < provider.size(storage))
    if any(target not in allowed for target in targets.values()):
        raise PackError("LOCAL target is outside its declared provider group")
    identities = proof.source_identities(raw, configured, world.elf, exports, ov,
                                         local_data_identities=targets)
    expected = {k: v for k, v in world.expected_cache(ov).items()
                if start <= k[0] < start + raw.size(".data")}
    if (set(expected) != {(start + r.offset, r.type) for r in rels}
            or [start + r.offset for r in rels] !=
               [k[0] for k in sorted(expected, key=lambda k: expected[k]["position"])]):
        raise PackError("LOCAL data runtime count/order differs")
    for rel in rels:
        target = targets[rel.sym.name]
        record = expected[(start + rel.offset, rel.type)]
        if (identities[(rel.offset, rel.type)] != target
                or u32(configured.bytes_of(".data"), rel.offset) != target[1] - origin
                or record["op"] != OP_LOCAL or record["table"] != 2
                or record["symbol_index"] != origin or record["full"] != ("ov", *target)):
            raise PackError("LOCAL data source/provider/runtime identity differs")
    raw.data_local_identities = targets
    return len(rels)


def validate_data_imports(world, exports, ov, row, source, raw):
    """Complete ORT pointer inputs or a bounded same-owner LOCAL data prefix."""
    import source_module_link_proof as proof
    if getattr(raw, "local_ro_provider", None):
        return validate_cross_input_local(world, exports, ov, row, source, raw)
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


def load_mixed_group_provider(world, name, expected):
    """Bind the one parsed provider observation to its captured input digest."""
    path = world.paths["build"].parent / name
    provider = Elf(path)
    observed = hashlib.sha256(provider.data).hexdigest()
    if observed != expected:
        raise PackError("loaded mixed LOCAL provider differs from captured input")
    hold_data_import_inputs(world, {path: observed})
    return provider


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
    provider_inputs = provenance.local_ro_provider_inputs(receipt["inputs"]["module_link"], root)
    declared = link.get("local_data_provider")
    group = link.get("local_data_group")
    if len(provider_inputs) > 1:
        mixed = provenance.local_ro_mixed_group(receipt["inputs"]["module_link"], root)
        loaded_providers = ([load_mixed_group_provider(world, name,
            receipt["inputs"]["module_link_inputs"].get(name)) for name in provider_inputs]
            if mixed else [])
        expected_inputs = [{"input": str((root / name).resolve()),
            "input_sha256": (hashlib.sha256(loaded_providers[index].data).hexdigest()
                             if mixed else provenance.sha256_file(root / name))}
            for index, name in enumerate(provider_inputs)]
        if mixed:
            import link_overlay_module as module_link
            try:
                shape = module_link.local_aggregate_shape(fresh)
                sections = [module_link.local_provider_section(provider, readonly=True)
                            for provider in loaded_providers]
            except module_link.LinkError as error:
                raise PackError("mixed LOCAL group storage refused: " + str(error)) from error
            if sections != [".rodata", ".data"]:
                raise PackError("mixed LOCAL group storage order differs")
            for item, provider, section in zip(expected_inputs, loaded_providers, sections):
                item.update(input_section=section, input_size=provider.size(section))
            if group is None or group.get("profile") != "mixed-storage-v1" or group.get("aggregate") != shape:
                raise PackError("mixed LOCAL group lacks its actual root/storage receipt")
            fresh.local_ro_aggregate = shape
            fresh.local_ro_mixed_group = True
        elif group and ("profile" in group or "aggregate" in group):
            raise PackError("undeclared mixed LOCAL group profile")
        if (declared or not group or group.get("inputs") != expected_inputs
                or group.get("runtime_role") != ".rodata" or group.get("origin") != "first-input-start"
                or group.get("linked_sha256") != provenance.sha256_file(directory / "providers.elf")
                or group.get("script_sha256") != provenance.sha256_file(directory / "providers.ld")):
            raise PackError("ordinary LOCAL group lacks its bound ordered providers")
        fresh.local_ro_providers = provider_inputs
        fresh.local_ro_provider = provider_inputs[0]
    elif provider_inputs:
        provider_input = provider_inputs[0]
        provider = root / provider_input
        if (group or not declared or declared.get("input") != str(provider.resolve())
                or declared.get("input_sha256") != provenance.sha256_file(provider)
                or declared.get("runtime_role") != ".rodata" or declared.get("origin") != "input-start"):
            raise PackError("ordinary LOCAL data link lacks its bound provider declaration")
        fresh.local_ro_provider = provider_input
        if provenance.local_data_aggregate(receipt["inputs"]["module_link"], root):
            import link_overlay_module as module_link
            try:
                shape = module_link.local_aggregate_shape(fresh)
                storage = module_link.local_provider_section(Elf(provider), readonly=True)
            except module_link.LinkError as error:
                raise PackError("mixed LOCAL declaration refused: " + str(error)) from error
            if declared.get("aggregate") != shape or declared.get("input_section") != storage:
                raise PackError("mixed LOCAL receipt root/storage differs")
            fresh.local_ro_aggregate = shape
        elif "aggregate" in declared or "input_section" in declared:
            raise PackError("undeclared mixed LOCAL profile")
    elif declared or group:
        raise PackError("undeclared LOCAL provider in ordinary module receipt")
    return fresh, receipt, context



def plain_data_targets(make_dump):
    """Explicit proof-only selections and roles; neither changes a build recipe."""
    selected, roles = set(), {}
    for target, role in re.findall(r"^(\S+): PLAIN_DATA_PROOF = (.*)$", make_dump, re.M):
        if (role.strip() != "1" or not re.fullmatch(
                r"build/src/overlays/o[0-9]{3}/[^/]+\.c\.o", target)):
            raise PackError(f"invalid ordinary data proof selection: {target}")
        selected.add(target)
    for target, role in re.findall(r"^(\S+): PLAIN_DATA_RUNTIME_ROLE = (.*)$", make_dump, re.M):
        role = role.strip()
        if role not in ("data", "ro-origin", "ro-member") or (target in roles and roles[target] != role):
            raise PackError(f"invalid or duplicate ordinary data runtime role: {target}")
        roles[target] = role
    if selected != set(roles):
        raise PackError("ordinary data proof requires exactly one runtime role per selection")
    return roles



def plain_data_origins(dump):
    origins = {}
    for target, origin in re.findall(r"^(\S+): PLAIN_DATA_RO_ORIGIN_OBJECT = (.*)$", dump, re.M):
        origin = origin.strip()
        if (not re.fullmatch(r"build/src/overlays/o[0-9]{3}/[^/]+\.c\.o", origin)
                or ".." in Path(origin).parts or (target in origins and origins[target] != origin)):
            raise PackError("invalid ordinary RO member origin declaration")
        origins[target] = origin
    return origins


def plain_data_origin_sections(dump):
    """Only explicit readonly origin admission; DATA remains the old default."""
    sections = {}
    for target, section in re.findall(r"^(\S+): PLAIN_DATA_RO_ORIGIN_SECTION = (.*)$", dump, re.M):
        if section.strip() != ".rodata":
            raise PackError("unsupported ordinary RO origin section declaration")
        sections[target] = ".rodata"
    return sections


def plain_data_role_context(world, roles, origins=None, origin_sections=None):
    """Bind declared roles to actual map starts, keeping compiler sections intact."""
    inputs = parse_map(world.paths["map"])
    origins = origins or {}
    origin_sections = origin_sections or {}
    if (set(origin_sections) - set(origins)
            or any(roles.get(target) != "ro-member" or section != ".rodata"
                   or target not in getattr(world, "_literal_array_plans", {})
                   for target, section in origin_sections.items())):
        raise PackError("readonly RO origin requires one literal-array ro-member")
    if set(origins) != {target for target, role in roles.items() if role == "ro-member"}:
        raise PackError("RO member requires exactly one declared origin input")
    observed = {}
    for target, role in roles.items():
        items = [item for item in inputs if item.file == target
                 and item.insec == ".data" and item.size]
        if len(items) != 1 or not re.fullmatch(r"\.overlay_[0-9]{3}", items[0].outsec):
            raise PackError(f"ordinary runtime role lacks its unique actual data input: {target}")
        item = items[0]
        ov = int(item.outsec.rsplit("_", 1)[1])
        start = item.addr - OVERLAY_VMA
        origin = start if role == "ro-origin" else world.module_info[ov]["text_size"]
        if role == "ro-member":
            storage = origin_sections.get(target, ".data")
            choices = [i for i in inputs if i.file == origins[target] and i.insec == storage and i.size]
            if len(choices) != 1 or choices[0].outsec != item.outsec or choices[0].addr > item.addr:
                raise PackError("RO member lacks its bounded same-module origin")
            if storage == ".rodata" and choices[0].addr + choices[0].size != item.addr:
                raise PackError("readonly RO origin must immediately precede its literal member")
            origin = choices[0].addr - OVERLAY_VMA
        observed[target] = {"role": role, "input_start": start, "size": item.size,
            "origin": origin}
        if target in origin_sections:
            observed[target].update(origin_section=storage, origin_size=choices[0].size,
                                    origin_input=origins[target])
    return observed


def refresh_plain_role_context(world):
    import postprocess_audit as audit
    dump = audit.run_make_database()
    return plain_data_role_context(world, plain_data_targets(dump), plain_data_origins(dump),
                                   plain_data_origin_sections(dump))


def validate_plain_roles(admitted):
    """A declared runtime RO origin is unique and follows all selected DATA."""
    for ov, objects in admitted.items():
        origins = [obj for obj in objects if obj.runtime_role == "ro-origin"]
        if len(origins) > 1:
            raise PackError(f"overlay {ov}: duplicate ordinary data RO origins")
        if origins:
            origin = section_offset(origins[0], ".data")
            if any(obj.runtime_role == "data"
                   and section_offset(obj, ".data") + obj.elf.size(".data") > origin
                   for obj in objects):
                raise PackError(f"overlay {ov}: ordinary DATA conflicts with its RO origin")


def validate_plain_data(world, ov, row, source, *, storage=".data"):
    """Authenticate a complete ordinary C input, without a module-link surrogate."""
    import proof_provenance as provenance
    path = world.paths["raw"] / "src" / (source + ".c.o")
    ordinary = Elf(path)
    if storage == ".rodata":
        helper, held = readonly_provider_identity()
        import link_overlay_module as module_link
        if Path(module_link.__file__).resolve() != helper.resolve():
            raise PackError("readonly provider helper namespace differs")
        readonly_provider_identity()
        hold_data_import_inputs(world, {helper: held})
        try:
            if module_link.local_provider_section(ordinary, readonly=True) != storage:
                raise module_link.LinkError("origin is not readonly storage")
        except module_link.LinkError as error:
            raise PackError("readonly RO origin storage refused: " + str(error)) from error
    configured, start = data_import_input(world, ov, row, source, ordinary, storage=storage)
    provider = getattr(world, "data_import_providers", {}).get(source)
    ordinary = effective_compiler_object(ordinary, configured,
        getattr(world, "_end_padding_plans", {}).get(f"build/src/{source}.c.o"),
        literal_proofs=getattr(provider, "literal_end_proofs", None), root=world.paths["build"].parent)
    text = (world.paths["build"].parent / "src" / (source + ".c")).read_text()
    if re.search(r"\b(?:asm|__asm|__asm__)\b", provenance._mask_c(text)):
        raise PackError(f"ordinary data input contains inline assembly: {source}")
    for obj in (ordinary, configured):
        if int.from_bytes(obj.data[16:18], "big") != 1:
            raise PackError(f"ordinary data input is not relocatable C: {source}")
        if any(obj.rels(section["name"]) for section in obj.sections):
            raise PackError(f"ordinary data input has relocations: {source}")
        if any(sym.name and sym.bind in (STB_GLOBAL, STB_WEAK)
               and sym.type not in (STT_SECTION, STT_FILE)
               and (sym.bind != STB_GLOBAL or sym.type != 1 or sym.section != storage)
               for sym in obj.symbols):
            raise PackError(f"ordinary data input has non-object/import definitions: {source}")
    definitions = [sym for sym in ordinary.symbols
                   if sym.name and sym.bind == STB_GLOBAL and sym.type == 1
                   and sym.section == storage]
    if not definitions or len({sym.name for sym in definitions}) != len(definitions):
        raise PackError(f"ordinary data input lacks unique named objects: {source}")
    if storage == ".rodata" and min(symbol.value for symbol in definitions) != 0:
        raise PackError("readonly RO origin lacks its actual first GLOBAL object at zero")
    providers = getattr(world, "data_import_providers", {})
    if source not in providers:
        providers[source] = reproduce_data_provider(world, source)
    world.data_import_providers = providers
    for symbol in definitions:
        # IDO can give a genuine char[] definition size zero. This proves its
        # base, never a fabricated extent; the complete input is proved above.
        target = data_import_definition(world, symbol.name, base_only=True,
                                        provider_source=source, readonly_provider=(storage == ".rodata"))
        if target != (ov, start + symbol.value):
            raise PackError(f"ordinary data definition moved: {symbol.name}")
    return providers[source], start


def guard_data_module_imports(world, exports):
    """Revalidate atomically before publishing complete data-only producers."""
    if getattr(world, "_data_guard_active", False):
        raise PackError("recursive data-only producer discovery")
    world._guarded_data_objects = None
    world._objects = world._defs = None
    world._plain_data_declarations = ({}, {}, {})
    world._data_guard_active = True
    try:
        admitted = _guard_data_module_imports(world, exports)
    except BaseException:
        world._plain_data_declarations = ({}, {}, {})
        raise
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
    world._end_proved_objects = {}
    world._compiler_end_receipts = {}
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
    world._address_targets = address_targets(dump)
    end_plans = {target: provenance.metadata_filter_plan(command, target, root)
                 for target, command in audit.postprocess_commands(dump).items()
                 if "--compiler-end-padding" in command or "--literal-array-end" in command}
    world._end_padding_plans = {target: {arguments[0]: arguments[1] for operation, arguments in plan
                                      if operation in ("nonexec-end", "literal-end")}
                               for target, plan in end_plans.items()}
    world._literal_array_plans = {target: plan for target, plan in end_plans.items()
                                 if any(operation == "literal-end" for operation, arguments in plan)}
    if world._literal_array_plans:
        hold_data_import_inputs(world, {root/name: digest for name, digest
                                        in provenance.literal_profile_inputs(root).items()})
    outputs = []
    plain_targets = plain_data_targets(dump)
    member_origins = plain_data_origins(dump)
    origin_sections = plain_data_origin_sections(dump)
    if origin_sections:
        helper = root / "tools/link_overlay_module.py"
        hold_data_import_inputs(world, {helper: hashlib.sha256(helper.read_bytes()).hexdigest()})
    if (set(origin_sections) - set(member_origins)
            or any(plain_targets.get(target) != "ro-member" for target in origin_sections)):
        raise PackError("readonly origin section requires exactly one ro-member origin")
    world._plain_data_declarations = (plain_targets, member_origins, origin_sections)
    role_context = {}
    remaining_plain = set(plain_targets)
    admitted = collections.defaultdict(list)
    try:
        for ov, module in world.amods.items():
            text = {r["source"] for r in module["text_ownership"] if r["type"] == "c"}
            sources = {r["source"] for r in module.get("data_rodata_ownership", [])
                       if r.get("type") == "c"} - text
            for source in sorted(sources):
                target = f"build/src/{source}.c.o"
                plain = target in plain_targets
                linked = proof.selected(target, dump)
                if plain and linked:
                    raise PackError(f"ordinary data proof also selects module linking: {source}")
                if not plain and not linked:
                    continue
                rows = [r for r in projected_data_rows(module) if r["source"] == source]
                if len(rows) != 1:
                    raise PackError(f"data-only module lacks one canonical input: {source}")
                if plain:
                    raw, start = validate_plain_data(world, ov, rows[0], source)
                    role_context[target] = plain_data_role_context(world, {target: plain_targets[target]},
                        {target: member_origins[target]} if target in member_origins else {},
                        {target: origin_sections[target]} if target in origin_sections else {})[target]
                    path = world.paths["raw"] / "src" / (source + ".c.o")
                    admitted[ov].append(ObjRef(ov, path, source, 0, 0, "c", raw,
                        {".data": (start, "link")}, {}, plain_targets[target], role_context[target]["origin"]))
                    receipt = world.data_import_provider_receipts[source]
                    receipt["ordinary_data_runtime"] = {"role": plain_targets[target],
                        "input_start": start, "origin": role_context[target]["origin"]}
                    if target in member_origins:
                        origin_source = member_origins[target].removeprefix("build/src/").removesuffix(".c.o")
                        origin_rows = [r for r in projected_data_rows(module) if r["source"] == origin_source]
                        if len(origin_rows) != 1:
                            raise PackError("RO member origin lacks one canonical C owner")
                        if target in origin_sections:
                            _fresh, origin_start = validate_plain_data(world, ov, origin_rows[0], origin_source,
                                                                       storage=".rodata")
                            receipt["ordinary_data_runtime"].update(
                                origin_section=".rodata", origin_source=origin_source,
                                origin_size=_fresh.size(".rodata"))
                        else:
                            origin_raw = Elf(world.paths["raw"] / "src" / (origin_source + ".c.o"))
                            _cfg, origin_start = data_import_input(world, ov, origin_rows[0], origin_source, origin_raw)
                            if origin_source not in world.data_import_providers:
                                world.data_import_providers[origin_source] = reproduce_data_provider(world, origin_source)
                        if origin_start != role_context[target]["origin"]:
                            raise PackError("RO member origin geometry changed")
                    remaining_plain.remove(target)
                    continue
                raw, receipt, context = reproduce_data_import(world, source)
                hold_data_capture(world, source, receipt, context)
                validate_data_imports(world, exports, ov, rows[0], source, raw)
                targets = getattr(raw, "data_local_identities", None)
                if targets is None:
                    targets = {rel.sym.name: data_import_definition(world, rel.sym.name)
                               for rel in raw.rels(".data") if rel.sym.shndx == SHN_UNDEF}
                start = world.module_info[ov]["text_size"] + int(rows[0]["offset"], 16)
                path = world.paths["raw"] / "src" / (source + ".c.o")
                admitted[ov].append(ObjRef(ov, path, source, 0, 0, "c", raw,
                    {".data": (start, "link")}, targets))
                outputs.append((raw, receipt))
        for ov, module in world.amods.items():
            for row in module["text_ownership"]:
                source = row["source"]
                if row["type"] == "c" and f"build/src/{source}.c.o" in world._end_padding_plans:
                    world._end_proved_objects[source] = reproduce_compiler_end(world, source)
        if remaining_plain:
            raise PackError("ordinary data proof selection lacks one canonical data input: "
                            + ", ".join(sorted(remaining_plain)))
        validate_plain_roles(admitted)
        if plain_targets:
            world.data_import_contexts.append((plain_data_role_context(world, plain_targets, member_origins, origin_sections),
                lambda: refresh_plain_role_context(world)))
        check_data_import_holds(world)
        for receipt in list(world.data_import_provider_receipts.values()) + list(world._compiler_end_receipts.values()):
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
    check_data_import_holds(world)
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
        export = getattr(world, "_address_targets", {}).get(mi.file)
        if export and mi.insec == ".text":
            names = resident_address_names(world, mi, path, Elf(path), export)
            for name in set(names.values()):
                source_bss_identity(world, name)
        sidecar = Path(str(path) + ".trapsites")
        if mi.file.endswith(".c.o") and sidecar.exists():
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
