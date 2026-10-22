"""Minimal big-endian ELF32 reader: sections, symbols, REL relocations, MIPS addends."""

from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path

from .codec import R_MIPS_26, R_MIPS_32, R_MIPS_HI16, R_MIPS_LO16

SHT_SYMTAB, SHT_RELA, SHT_NOBITS, SHT_REL = 2, 4, 8, 9
STT_NOTYPE, STT_OBJECT, STT_FUNC, STT_SECTION, STT_FILE = 0, 1, 2, 3, 4
STB_LOCAL, STB_GLOBAL, STB_WEAK = 0, 1, 2
SHN_UNDEF, SHN_ABS, SHN_COMMON = 0, 0xFFF1, 0xFFF2


class ElfError(ValueError):
    pass


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

    @property
    def defined(self):
        return self.shndx != SHN_UNDEF and self.shndx < 0xFF00

    @property
    def absolute(self):
        return self.shndx == SHN_ABS


@dataclass(frozen=True)
class Rel:
    offset: int
    type: int
    sym: Symbol


def u32(d, off):
    return struct.unpack_from(">I", d, off)[0]


def sext16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


class Elf:
    def __init__(self, path, data=None):
        self.path = Path(path)
        d = self.data = data if data is not None else self.path.read_bytes()
        if d[:6] != b"\x7fELF\x01\x02":
            raise ElfError(f"{path}: not a big-endian ELF32 file")
        shoff = u32(d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from(">HHH", d, 0x2E)
        raw = [struct.unpack_from(">10I", d, shoff + i * shentsize) for i in range(shnum)]
        names = raw[shstrndx][4]
        self.sections = []
        for i, h in enumerate(raw):
            self.sections.append({"index": i, "name": self._str(names + h[0]), "type": h[1],
                                  "flags": h[2], "addr": h[3], "offset": h[4], "size": h[5],
                                  "link": h[6], "info": h[7]})
        self._by_name = {}
        for s in self.sections:
            self._by_name.setdefault(s["name"], s)
        self.symbols = []
        for s in self.sections:
            if s["type"] != SHT_SYMTAB:
                continue
            strs = self.sections[s["link"]]["offset"]
            for j in range(s["size"] // 16):
                nm, val, sz, info, _o, shndx = struct.unpack_from(">IIIBBH", d, s["offset"] + j * 16)
                self.symbols.append(Symbol(j, self._str(strs + nm), val, sz, info >> 4, info & 0xF,
                                           shndx, self.secname(shndx)))
            break

    def _str(self, off):
        return self.data[off:self.data.index(b"\0", off)].decode("latin-1")

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
        """REL relocations applying to section ``target``, in file order."""
        out = []
        for s in self.sections:
            if s["type"] not in (SHT_REL, SHT_RELA) or self.sections[s["info"]]["name"] != target:
                continue
            if s["type"] == SHT_RELA:
                raise ElfError(f"{self.path}: RELA relocations are not supported")
            for j in range(s["size"] // 8):
                off, info = struct.unpack_from(">II", self.data, s["offset"] + j * 8)
                out.append(Rel(off, info & 0xFF, self.symbols[info >> 8]))
        return out


def in_place_addends(data, rels):
    """Each REL relocation's addend, read from the stored words as a static linker would.

    A HI16 pairs with the next LO16 against the same symbol; a LO16 with no
    HI16 of its own reuses the most recent one (the MIPS AHL rule)."""
    out = []
    last_hi = {}
    for i, r in enumerate(rels):
        w = u32(data, r.offset) if r.offset + 4 <= len(data) else 0
        if r.type == R_MIPS_26:
            a = (w & 0x3FFFFFF) << 2
        elif r.type == R_MIPS_32:
            a = w - 0x100000000 if w & 0x80000000 else w
        elif r.type == R_MIPS_HI16:
            hi, lo = w & 0xFFFF, 0
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
            raise ElfError(f"unsupported relocation type {r.type}")
        out.append(a)
    return out
