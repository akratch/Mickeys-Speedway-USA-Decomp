"""Synthetic fixtures for tests: big-endian ELF32 objects and hand-written table bytes.

Nothing here reads a ROM.  Table bytes are written with ``struct`` straight
from the format description, not through :mod:`runlink.codec`, so a test
that decodes them checks the codec against the format rather than against
itself.
"""

from __future__ import annotations

import collections
import struct

LOCAL, GLOBAL = 0, 1
NOTYPE, OBJECT, FUNC, SECTION = 0, 1, 2, 3


class ElfBuilder:
    """Sections, symbols and REL relocations, written as a relocatable or 'linked' ELF."""

    def __init__(self):
        self.sections = []      # (name, sh_type, addr, data-or-size)
        self.symbols = []       # (name, value, size, bind, type, section)
        self.relocs = collections.defaultdict(list)

    def section(self, name, data=b"", addr=0, nobits=None):
        self.sections.append((name, 8, addr, nobits) if nobits is not None else (name, 1, addr, bytes(data)))
        return self

    def sym(self, name, value=0, size=0, bind=GLOBAL, type=FUNC, section="UND"):
        self.symbols.append((name, value, size, bind, type, section))
        return self

    def rel(self, target, offset, rtype, symbol):
        """``symbol`` is a symbol name, or ("section", name) for a section symbol."""
        self.relocs[target].append((offset, rtype, symbol))
        return self

    def build(self) -> bytes:
        user = [s[0] for s in self.sections]
        rel_targets = [t for t in user if t in self.relocs]
        names = [""] + user + [".rel" + t for t in rel_targets] + [".symtab", ".strtab", ".shstrtab"]
        shstr, name_off = b"\0", {}
        for n in names[1:]:
            name_off[n] = len(shstr)
            shstr += n.encode() + b"\0"
        strtab, str_off = b"\0", {}

        def sname(n):
            nonlocal strtab
            if n not in str_off:
                str_off[n] = len(strtab)
                strtab += n.encode() + b"\0"
            return str_off[n]

        def shndx(sec):
            if sec == "UND":
                return 0
            if sec == "ABS":
                return 0xFFF1
            return 1 + user.index(sec)

        syms = [(0, 0, 0, 0, 0, 0)]
        sec_sym = {}
        for i, n in enumerate(user):
            sec_sym[n] = len(syms)
            syms.append((0, 0, 0, (LOCAL << 4) | SECTION, 0, 1 + i))
        index = {}
        ordered = [s for s in self.symbols if s[3] == LOCAL] + [s for s in self.symbols if s[3] != LOCAL]
        first_global = len(syms) + sum(1 for s in self.symbols if s[3] == LOCAL)
        for name, value, size, bind, typ, sec in ordered:
            index[name] = len(syms)
            syms.append((sname(name), value, size, (bind << 4) | typ, 0, shndx(sec)))
        symtab = b"".join(struct.pack(">IIIBBH", *s) for s in syms)
        rel_blobs = []
        for t in rel_targets:
            rel_blobs.append(b"".join(
                struct.pack(">II", off, ((sec_sym[s[1]] if isinstance(s, tuple) else index[s]) << 8) | rt)
                for off, rt, s in self.relocs[t]))
        out = bytearray(52)
        headers = [(0,) * 10]

        def place(blob):
            while len(out) % 4:
                out.append(0)
            off = len(out)
            out.extend(blob)
            return off

        for name, typ, addr, data in self.sections:
            if typ == 8:
                headers.append((name_off[name], 8, 3, addr, len(out), data, 0, 0, 16, 0))
            else:
                headers.append((name_off[name], 1, 6, addr, place(data), len(data), 0, 0, 16, 0))
        symtab_index = len(names) - 3
        for t, blob in zip(rel_targets, rel_blobs):
            headers.append((name_off[".rel" + t], 9, 0, 0, place(blob), len(blob),
                            symtab_index, 1 + user.index(t), 4, 8))
        headers.append((name_off[".symtab"], 2, 0, 0, place(symtab), len(symtab),
                        symtab_index + 1, first_global, 4, 16))
        headers.append((name_off[".strtab"], 3, 0, 0, place(strtab), len(strtab), 0, 0, 1, 0))
        headers.append((name_off[".shstrtab"], 3, 0, 0, place(shstr), len(shstr), 0, 0, 1, 0))
        while len(out) % 4:
            out.append(0)
        shoff = len(out)
        for h in headers:
            out.extend(struct.pack(">10I", *h))
        struct.pack_into(">4sBBBB8xHHIIIIIHHHHHH", out, 0, b"\x7fELF", 1, 2, 1, 0, 1, 8, 1, 0, 0,
                         shoff, 0, 52, 0, 0, 40, len(headers), len(headers) - 1)
        return bytes(out)


def words(*ws) -> bytes:
    return b"".join(struct.pack(">I", w & 0xFFFFFFFF) for w in ws)


def record(symidx, site, rtype, op) -> bytes:
    """One table record, written from the format: index, then site << 8 | type << 4 | op."""
    return struct.pack(">II", symidx & 0xFFFFFFFF, (site << 8) | (rtype << 4) | op)


def header(rom, text, data, bss, rel1, rel2, init=-1, resume=-1) -> bytes:
    return struct.pack(">iiiiiHHii", 0, rom, text, data, bss, rel1, rel2, init, resume)


def ort_row(selector, offset) -> bytes:
    return struct.pack(">I", (selector << 20) | offset)


def map_text(outputs):
    """A GNU ld map file: ``outputs`` is [(name, addr, size, [(insec, addr, size, file)])]."""
    lines = ["Archive member included", "", "Linker script and memory map", ""]
    for name, addr, size, ins in outputs:
        lines.append(f"{name:<16}0x{addr:08x} 0x{size:x}")
        for insec, a, sz, f in ins:
            lines.append(f" {insec:<15}0x{a:08x} 0x{sz:x} {f}")
        lines.append("")
    return "\n".join(lines) + "\n"
