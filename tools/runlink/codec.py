"""Rare's runlink tables: decode from a ROM image, encode back to bytes.

The format (shared by Mickey's Speedway USA and Jet Force Gemini):

- **main table**: a u32 entry count, then that many 8-byte records, padded
  with zeros to the end of its block.  Each record patches a site in the
  resident module.
- **ORT** (overlay ROM table): u32 per row, ``selector << 20 | offset``.  A
  selector is a module number (0 = resident) or a reserved section selector.
- **headers**: 0x20 bytes per module, module 1 first.  Module 0 (resident) is
  synthesized at run time and has no ROM row.
- **module images**, back to back, each ``[text][data][table 1][table 2]``.
- optionally, a **symbol-name table**: u32 offsets parallel to the ORT, into a
  blob of NUL-terminated names (JFG ships one; MSU does not).

A record is ``u32 symbolIndex; u32 site << 8 | type << 4 | op``.  ``type`` is
the ELF MIPS relocation number (2 R_MIPS_32, 4 R_MIPS_26, 5 HI16, 6 LO16);
``op`` is Rare's operation (0 SYMBOL, 1 LOCAL, 2 JUMP, 3 DATA).

Every function here is a pure transformation between bytes and the
dataclasses below; nothing reads paths except :func:`read_rom`.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass, field, replace

R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6
TYPE_NAMES = {R_MIPS_32: "R_MIPS_32", R_MIPS_26: "R_MIPS_26",
              R_MIPS_HI16: "R_MIPS_HI16", R_MIPS_LO16: "R_MIPS_LO16"}
OP_SYMBOL, OP_LOCAL, OP_JUMP, OP_DATA = 0, 1, 2, 3
OP_NAMES = {OP_SYMBOL: "SYMBOL", OP_LOCAL: "LOCAL", OP_JUMP: "JUMP", OP_DATA: "DATA"}

HEADER_SIZE = 0x20
RECORD_SIZE = 8


class FormatError(ValueError):
    """The bytes do not decode as the configured tables."""


@dataclass
class Record:
    """One relocation record (module table or main table)."""
    symidx: int
    site: int
    type: int
    op: int

    @property
    def key(self):
        return (self.site, self.type)

    @property
    def flags(self):
        return (self.type << 4) | self.op

    def encode(self) -> bytes:
        if not 0 <= self.site < 1 << 24:
            raise FormatError(f"record site {self.site:#x} does not fit 24 bits")
        return struct.pack(">II", self.symidx & 0xFFFFFFFF,
                           (self.site << 8) | ((self.type & 0xF) << 4) | (self.op & 0xF))

    @classmethod
    def decode(cls, data: bytes, off: int = 0) -> "Record":
        sym, info = struct.unpack_from(">II", data, off)
        return cls(sym, info >> 8, (info >> 4) & 0xF, info & 0xF)

    def describe(self) -> str:
        return (f"{OP_NAMES.get(self.op, f'op{self.op}')} "
                f"{TYPE_NAMES.get(self.type, f'type{self.type}')} "
                f"site={self.site:#x} index={self.symidx}")


@dataclass
class Header:
    vram: int = 0
    rom: int = 0          # offset from the module region base
    text: int = 0
    data: int = 0         # data + rodata, one ROM-backed span
    bss: int = 0
    rel1: int = 0         # bytes
    rel2: int = 0         # bytes
    init: int = -1
    resume: int = -1

    FIELDS = ("vram", "rom", "text", "data", "bss", "rel1", "rel2", "init", "resume")

    def encode(self) -> bytes:
        return struct.pack(">iiiiiHHii", self.vram, self.rom, self.text, self.data, self.bss,
                           self.rel1, self.rel2, self.init, self.resume)

    @classmethod
    def decode(cls, data: bytes, off: int = 0) -> "Header":
        return cls(*struct.unpack_from(">iiiiiHHii", data, off))

    @property
    def image_size(self):
        return self.text + self.data + self.rel1 + self.rel2


@dataclass
class OrtRow:
    selector: int
    offset: int

    def encode(self) -> bytes:
        if not 0 <= self.offset < 1 << 20:
            raise FormatError(f"ORT offset {self.offset:#x} does not fit 20 bits")
        return struct.pack(">I", (self.selector << 20) | self.offset)

    @classmethod
    def decode(cls, data: bytes, off: int = 0) -> "OrtRow":
        w = struct.unpack_from(">I", data, off)[0]
        return cls(w >> 20, w & 0xFFFFF)


@dataclass
class Module:
    number: int
    header: Header
    image: bytes          # [text][data]
    table1: list = field(default_factory=list)
    table2: list = field(default_factory=list)

    @property
    def empty(self):
        return self.header.image_size == 0 and self.header.bss == 0

    def records(self):
        """(table, position, record) in table 1 then table 2 order."""
        for t, rows in ((1, self.table1), (2, self.table2)):
            for i, r in enumerate(rows):
                yield t, i, r

    def word(self, off: int) -> int:
        return struct.unpack_from(">I", self.image, off)[0]


@dataclass
class Tables:
    main: list            # [Record]
    ort: list             # [OrtRow]
    modules: list         # [Module], module 1 first
    names: list = None    # [str] parallel to ort, when the ROM ships them

    def module(self, n: int) -> Module:
        return self.modules[n - 1]


# ---------------------------------------------------------------------------
# Decoding


def _records(data, start, size, what):
    if size % RECORD_SIZE:
        raise FormatError(f"{what}: {size:#x} bytes is not a whole number of records")
    return [Record.decode(data, start + i) for i in range(0, size, RECORD_SIZE)]


def decode(rom: bytes, cfg) -> Tables:
    """Decode every table the config locates."""
    main_rom = cfg.main_table_rom
    count = struct.unpack_from(">I", rom, main_rom)[0]
    if 4 + count * RECORD_SIZE > cfg.main_table_block:
        raise FormatError(f"main table count {count} overflows its {cfg.main_table_block:#x}-byte block")
    main = _records(rom, main_rom + 4, count * RECORD_SIZE, "main table")
    ort = [OrtRow.decode(rom, cfg.ort_rom + 4 * i) for i in range(cfg.ort_count)]
    modules = []
    for i in range(cfg.module_count):
        h = Header.decode(rom, cfg.headers_rom + HEADER_SIZE * i)
        start = cfg.modules_rom + h.rom
        image = rom[start:start + h.text + h.data]
        r1 = start + h.text + h.data
        t1 = _records(rom, r1, h.rel1, f"module {i + 1} table 1")
        t2 = _records(rom, r1 + h.rel1, h.rel2, f"module {i + 1} table 2")
        modules.append(Module(i + 1, h, image, t1, t2))
    names = decode_names(rom, cfg) if cfg.names_rom is not None else None
    return Tables(main, ort, modules, names)


def decode_names(rom: bytes, cfg) -> list:
    """The ROM's symbol-name table: offsets parallel to the ORT into a NUL-terminated blob."""
    out = []
    base, end = cfg.names_rom, cfg.names_end
    for i in range(cfg.ort_count):
        off = struct.unpack_from(">I", rom, cfg.name_offsets_rom + 4 * i)[0]
        a = base + off
        if not base <= a < end:
            raise FormatError(f"name offset {off:#x} of row {i} lies outside the name blob")
        z = rom.index(b"\0", a)
        out.append(rom[a:z].decode("latin-1"))
    return out


def read_rom(path) -> bytes:
    """A ROM image in big-endian (.z64) order; .v64/.n64 byte orders are swapped."""
    with open(path, "rb") as fh:
        d = fh.read()
    magic = d[:4]
    if magic == b"\x80\x37\x12\x40":
        return d
    if magic == b"\x37\x80\x40\x12":       # .v64, byte-swapped halfwords
        b = bytearray(d)
        b[0::2], b[1::2] = d[1::2], d[0::2]
        return bytes(b)
    if magic == b"\x40\x12\x37\x80":       # .n64, little-endian words
        return b"".join(d[i:i + 4][::-1] for i in range(0, len(d), 4))
    return d


# ---------------------------------------------------------------------------
# Encoding


def encode_main(records, block: int) -> bytes:
    body = struct.pack(">I", len(records)) + b"".join(r.encode() for r in records)
    if len(body) > block:
        raise FormatError(f"main table of {len(records)} entries overflows its {block:#x}-byte block")
    return body + b"\0" * (block - len(body))


def encode_ort(rows) -> bytes:
    return b"".join(r.encode() for r in rows)


def layout_modules(modules):
    """Assign each header's ROM offset and table sizes from its image and tables.

    Modules are packed back to back in number order; an empty module takes
    the offset of the next one.  Returns new Module objects."""
    out = []
    pos = 0
    for m in modules:
        t1 = b"".join(r.encode() for r in m.table1)
        t2 = b"".join(r.encode() for r in m.table2)
        h = replace(m.header, rom=pos, rel1=len(t1), rel2=len(t2))
        if len(m.image) != h.text + h.data:
            raise FormatError(f"module {m.number}: image is {len(m.image):#x} bytes, "
                              f"header says text+data {h.text + h.data:#x}")
        out.append(replace(m, header=h))
        pos += h.image_size
    return out


def encode_modules(modules) -> tuple:
    """(header block, image block) for modules whose headers are already laid out."""
    headers = b"".join(m.header.encode() for m in modules)
    images = b"".join(m.image + b"".join(r.encode() for r in m.table1)
                      + b"".join(r.encode() for r in m.table2) for m in modules)
    return headers, images


def encode_region(tables: Tables, cfg, relayout=True) -> bytes:
    """Main table block, ORT, headers and images, contiguous as both games ship them.

    Requires the config's blocks to be back to back (main, ORT, headers,
    modules), which holds for MSU US and JFG US."""
    mods = layout_modules(tables.modules) if relayout else tables.modules
    headers, images = encode_modules(mods)
    main = encode_main(tables.main, cfg.main_table_block)
    ort = encode_ort(tables.ort)
    gap_ort = cfg.ort_rom - (cfg.main_table_rom + len(main))
    gap_hdr = cfg.headers_rom - (cfg.ort_rom + len(ort))
    gap_mod = cfg.modules_rom - (cfg.headers_rom + len(headers))
    if min(gap_ort, gap_hdr, gap_mod) < 0:
        raise FormatError("the configured blocks overlap")
    return (main + b"\0" * gap_ort + ort + b"\0" * gap_hdr + headers + b"\0" * gap_mod + images)


def region_bounds(tables: Tables, cfg):
    end = max([cfg.modules_rom] + [cfg.modules_rom + m.header.rom + m.header.image_size
                                    for m in tables.modules])
    return cfg.main_table_rom, end
