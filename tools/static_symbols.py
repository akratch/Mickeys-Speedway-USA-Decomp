#!/usr/bin/env python3
"""Restore compiler-recorded static function and storage symbols in an IDO object.

IDO writes no ELF definition for a ``static`` function. Its only trace in the
symbol table is a sized ``UND`` entry when the function was forward-declared,
and nothing at all otherwise, so the linked ELF cannot say where a static
function starts or ends, nor where its predecessor ends. The compiler does
record every procedure in the object's ``.mdebug`` symbolic table: a
``stStaticProc`` (or ``stProc`` for a global) symbol whose value is the
procedure's ``.text`` offset, closed by an ``stEnd`` of the same name whose
value is its size. That size is the number IDO writes into a global
function's ``st_size``, and the tool checks so for every global before it
trusts the table.

For each static ``.text`` procedure the tool writes a ``STB_LOCAL``
``STT_FUNC`` with that start and size, replacing IDO's ``UND`` placeholder of
the same name. A size-0 local function asm-processor's static conversion
already added (a TU with ``GLOBAL_ASM``) is given its size instead. Only the
symbol table, the string table and the symbol indices in relocation records
change: no section byte, relocation type, offset or addend. Compiler-recorded, unambiguous file-scope ``stStatic/scBss`` and
``stStatic/scData`` names also receive local ``STT_OBJECT`` anchors within
the real NOBITS BSS or writable PROGBITS DATA section. These
have size zero: the debug records establish addresses, not allocation extents.
Nested static variables and unemitted storage classes are not restored.
An object without missing static definitions is left byte-identical, and a
second run changes nothing.

usage: static_symbols.py [--check] OBJECT...
  --check  report what would change; write nothing
"""

from __future__ import annotations

import re
import struct
import sys

SHT_PROGBITS, SHT_SYMTAB, SHT_RELA, SHT_REL, SHT_NOBITS = 1, 2, 4, 9, 8
SHN_UNDEF = 0
STB_LOCAL, STT_NOTYPE, STT_OBJECT, STT_FUNC = 0, 0, 1, 2
ST_STATIC, ST_PROC, ST_BLOCK, ST_END, ST_FILE, ST_STATIC_PROC = 2, 6, 7, 8, 11, 14
SC_TEXT, SC_DATA, SC_BSS = 1, 2, 3
HDRR_MAGIC = 0x7009


class Failure(Exception):
    pass


def _cstring(data: bytes, offset: int) -> str:
    end = data.find(b"\0", offset)
    if end < 0:
        raise Failure(f"unterminated string at {offset:#x}")
    return data[offset:end].decode("latin1")


def _sections(data: bytes) -> list[list]:
    if data[:6] != b"\x7fELF\x01\x02":
        raise Failure("expected a big-endian ELF32 object")
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from(">HHH", data, 0x2E)
    sections = [list(struct.unpack_from(">10I", data, shoff + i * shentsize))
                for i in range(shnum)]
    names = sections[shstrndx][4]
    for section in sections:
        section.append(_cstring(data, names + section[0]))
    return sections


def _debug_files(data: bytes, mdebug: list):
    """Yield each compiler file descriptor's local symbolic records."""
    base, length = mdebug[4], mdebug[5]
    if length < 0x60:
        raise Failure(".mdebug is shorter than its header")
    magic, = struct.unpack_from(">H", data, base)
    if magic != HDRR_MAGIC:
        raise Failure(f".mdebug header magic {magic:#x}")
    words = struct.unpack_from(">24I", data, base)
    isym_max, cb_sym = words[8], words[9]
    iss_max, cb_ss = words[14], words[15]
    ifd_max, cb_fd = words[18], words[19]
    # The header holds file offsets. They are valid on the object as IDO (or
    # asm-processor, which relocates them) wrote it; anything else refuses.
    for offset, size in ((cb_sym, 12 * isym_max), (cb_ss, iss_max), (cb_fd, 72 * ifd_max)):
        if size and not base <= offset <= offset + size <= base + length:
            raise Failure(".mdebug table offsets lie outside the section")
    for fd in range(ifd_max):
        iss_base, _, isym_base, csym = struct.unpack_from(">IIII", data, cb_fd + 72 * fd + 8)
        if isym_base + csym > isym_max:
            raise Failure(".mdebug file descriptor exceeds the symbol table")
        rows = []
        for index in range(isym_base, isym_base + csym):
            iss, value, packed = struct.unpack_from(">III", data, cb_sym + 12 * index)
            rows.append((_cstring(data, cb_ss + iss_base + iss), value,
                         packed >> 26, (packed >> 21) & 0x1F))
        yield rows


def procedures(data: bytes, mdebug: list) -> dict[str, tuple[int, int, bool]]:
    """``{name: (start, size, is_static)}`` for every ``.text`` procedure."""
    found: dict[str, tuple[int, int, bool]] = {}
    for rows in _debug_files(data, mdebug):
        for index, (name, value, st, sc) in enumerate(rows):
            if st not in (ST_PROC, ST_STATIC_PROC) or sc != SC_TEXT:
                continue
            size = next((v for n, v, s, _ in rows[index + 1:] if s == ST_END and n == name), None)
            if size is None:
                raise Failure(f"procedure {name} has no stEnd")
            if name in found:
                raise Failure(f"procedure {name} is recorded twice")
            found[name] = (value, size, st == ST_STATIC_PROC)
    return found


def _storage_anchors(data: bytes, mdebug: list, storage_class: int,
                     label: str) -> dict[str, int]:
    """File-scope static names and compiler offsets, without inferred extents."""
    found: dict[str, int] = {}
    occurrences: dict[str, int] = {}
    files = list(_debug_files(data, mdebug))
    if storage_class == SC_DATA:
        for rows in files:
            for name, _value, st, _sc in rows:
                if st == ST_STATIC:
                    occurrences[name] = occurrences.get(name, 0) + 1
    for rows in files:
        if not any(st == ST_STATIC and sc == storage_class for _, _, st, sc in rows):
            continue
        scopes = []
        for name, value, st, sc in rows:
            if st == ST_STATIC and sc == storage_class and storage_class != SC_DATA:
                occurrences[name] = occurrences.get(name, 0) + 1
            if st == ST_STATIC and sc == storage_class:
                if (len(scopes) == 1 and scopes[0][0] == ST_FILE
                        and re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name)):
                    if name in found:
                        raise Failure(f"static {label} anchor {name} is recorded twice")
                    found[name] = value
            if sc == SC_TEXT:
                if st in (ST_FILE, ST_PROC, ST_STATIC_PROC, ST_BLOCK):
                    scopes.append((st, name))
                elif st == ST_END:
                    if not scopes or scopes[-1][1] != name:
                        raise Failure(f"ambiguous .mdebug scope for static {label} anchors")
                    scopes.pop()
        if scopes:
            raise Failure(f"unclosed .mdebug scope for static {label} anchors")
    for name in found:
        if occurrences[name] != 1:
            raise Failure(f"static {label} anchor {name} has ambiguous nested names")
    return found


def bss_anchors(data: bytes, mdebug: list) -> dict[str, int]:
    """Unambiguous file-scope stStatic/scBss names and compiler offsets only."""
    return _storage_anchors(data, mdebug, SC_BSS, "BSS")


def data_anchors(data: bytes, mdebug: list) -> dict[str, int]:
    """Unambiguous file-scope stStatic/scData names and compiler offsets only."""
    return _storage_anchors(data, mdebug, SC_DATA, "DATA")


def _symbols(data: bytes, symtab: list) -> list[list]:
    return [list(struct.unpack_from(">IIIBBH", data, offset))
            for offset in range(symtab[4], symtab[4] + symtab[5], 16)]


def add_static_symbols(data: bytes) -> tuple[bytes, list[str]]:
    """Return (new object bytes, names given local definitions or unsized storage anchors)."""
    sections = _sections(data)
    by_name = {section[10]: index for index, section in enumerate(sections)}
    if ".mdebug" not in by_name or ".text" not in by_name:
        return data, []
    procs = procedures(data, sections[by_name[".mdebug"]])
    statics = {name: row for name, row in procs.items() if row[2]}
    storage = (("BSS", ".bss", SHT_NOBITS, bss_anchors(data, sections[by_name[".mdebug"]])),
               ("DATA", ".data", SHT_PROGBITS, data_anchors(data, sections[by_name[".mdebug"]])))
    anchors: dict[str, int] = {}
    anchor_sections: dict[str, int] = {}
    anchor_labels: dict[str, str] = {}
    for label, section_name, section_type, found in storage:
        if not found:
            continue
        if anchors.keys() & found.keys():
            raise Failure("static BSS and DATA anchor names collide")
        if struct.unpack_from(">H", data, 0x10)[0] != 1:
            raise Failure(f"static {label} anchors require a relocatable object")
        if sum(section[10] == section_name for section in sections) != 1:
            raise Failure(f"static {label} anchors require one unambiguous {section_name} section")
        section_index = by_name[section_name]
        section = sections[section_index]
        if section[1] != section_type or not section[2] & 2:
            kind = "NOBITS" if label == "BSS" else "PROGBITS"
            raise Failure(f"static {label} anchors require an allocated {kind} {section_name} section")
        if label == "DATA" and (section[3] != 0 or section[2] & 7 != 3):
            raise Failure("static DATA anchors require zero-based writable nonexec .data")
        for name, offset in found.items():
            if not 0 <= offset < section[5]:
                raise Failure(f"static {label} anchor {name} lies outside {section_name}")
            anchors[name] = offset
            anchor_sections[name] = section_index
            anchor_labels[name] = label
    if not statics and not anchors:
        return data, []
    if statics.keys() & anchors.keys():
        raise Failure("static procedure and storage anchor names collide")
    text_index = by_name[".text"]
    text_size = sections[text_index][5]
    symtab_index = next(i for i, s in enumerate(sections) if s[1] == SHT_SYMTAB)
    symtab = sections[symtab_index]
    strtab = sections[symtab[6]]
    strings = bytearray(data[strtab[4]:strtab[4] + strtab[5]])
    symbols = _symbols(data, symtab)
    names = [_cstring(strings, row[0]) for row in symbols]
    for name in anchors:
        if names.count(name) > 1:
            raise Failure(f"static {anchor_labels[name]} anchor {name} conflicts with duplicate symbols")

    extents = []
    for name, row in zip(names, symbols):
        if (statics and row[5] == text_index and row[3] & 0xF == STT_FUNC
                and row[3] >> 4 != STB_LOCAL):
            # A GLOBAL_ASM function has no procedure of its own name.
            proc = procs.get(name)
            if proc is not None and (proc[2] or proc[:2] != (row[1], row[2])):
                raise Failure(f"global {name} disagrees with its .mdebug procedure")
            extents.append((row[1], row[2], name))
    for name, (start, size, _static) in statics.items():
        if size == 0 or size % 4 or start % 4 or start + size > text_size:
            raise Failure(f"static procedure {name} has an invalid extent "
                          f"{start:#x}+{size:#x}")
        extents.append((start, size, name))
    extents.sort()
    for (start, size, name), (later, _, other) in zip(extents, extents[1:]):
        if start + size > later:
            raise Failure(f"procedures {name} and {other} overlap")

    referenced = set()
    for section in sections:
        if section[1] in (SHT_REL, SHT_RELA) and section[6] == symtab_index:
            step = 8 if section[1] == SHT_REL else 12
            for offset in range(section[4], section[4] + section[5], step):
                referenced.add(struct.unpack_from(">I", data, offset + 4)[0] >> 8)

    first_global = symtab[7]
    locals_, globals_ = [], []
    added: list[str] = []
    pending = dict(statics)
    pending_anchors = dict(anchors)
    for index, (name, row) in enumerate(zip(names, symbols)):
        if name in statics:
            start, size, _ = statics[name]
            if row[5] == SHN_UNDEF and row[3] & 0xF == STT_NOTYPE and index >= first_global:
                if index in referenced:
                    raise Failure(f"a relocation names static {name}'s UND placeholder")
                continue  # IDO's placeholder; replaced below
            if (row[5] == text_index and row[3] == (STB_LOCAL << 4 | STT_FUNC)
                    and row[1] == start and row[2] in (0, size)):
                if row[2] == 0:
                    added.append(name)
                row = [row[0], start, size, row[3], row[4], row[5]]
                pending.pop(name)
            else:
                raise Failure(f"static {name} conflicts with an existing symbol")
        elif name in anchors:
            if (row[5] != anchor_sections[name] or row[3] != STT_OBJECT
                    or row[1] != anchors[name]
                    or row[1] + row[2] > sections[anchor_sections[name]][5]):
                raise Failure(f"static {anchor_labels[name]} anchor {name} conflicts with an existing symbol")
            pending_anchors.pop(name)
        (locals_ if index < first_global else globals_).append((index, row))
    placeholders = {name: row[0] for name, row in zip(names, symbols)
                    if name in pending and row[5] == SHN_UNDEF}
    for name in sorted(pending, key=lambda n: statics[n][0]):
        if name in placeholders:
            name_offset = placeholders[name]
        else:
            name_offset = len(strings)
            strings += name.encode("latin1") + b"\0"
        start, size, _ = statics[name]
        locals_.append((None, [name_offset, start, size,
                               STB_LOCAL << 4 | STT_FUNC, 0, text_index]))
        added.append(name)
    for name in sorted(pending_anchors, key=lambda n: (anchors[n], n)):
        name_offset = len(strings)
        strings += name.encode("latin1") + b"\0"
        locals_.append((None, [name_offset, anchors[name], 0,
                              STB_LOCAL << 4 | STT_OBJECT, 0, anchor_sections[name]]))
        added.append(name)
    if not added:
        return data, []

    ordered = locals_ + globals_
    remap = {old: new for new, (old, _row) in enumerate(ordered) if old is not None}
    table = b"".join(struct.pack(">IIIBBH", *row) for _old, row in ordered)

    out = bytearray(data)
    for section in sections:
        if section[1] in (SHT_REL, SHT_RELA) and section[6] == symtab_index:
            step = 8 if section[1] == SHT_REL else 12
            for offset in range(section[4], section[4] + section[5], step):
                info, = struct.unpack_from(">I", out, offset + 4)
                struct.pack_into(">I", out, offset + 4, remap[info >> 8] << 8 | info & 0xFF)

    def place(index: int, blob: bytes, align: int) -> None:
        section = sections[index]
        if len(blob) <= section[5]:
            offset = section[4]
        else:  # grow: move the table to the end of the file
            out.extend(bytes(-len(out) % align))
            offset = len(out)
            out.extend(bytes(len(blob)))
        out[offset:offset + len(blob)] = blob
        section[4], section[5] = offset, len(blob)

    place(symtab_index, table, 4)
    place(symtab[6], bytes(strings), 1)
    symtab[7] = len(locals_)
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, = struct.unpack_from(">H", data, 0x2E)
    for index in (symtab_index, symtab[6]):
        struct.pack_into(">10I", out, shoff + index * shentsize, *sections[index][:10])
    return bytes(out), sorted(added)


def main(argv: list[str]) -> int:
    check = "--check" in argv
    paths = [arg for arg in argv if arg != "--check"]
    if not paths:
        print(__doc__.strip().splitlines()[-2], file=sys.stderr)
        return 2
    status = 0
    for path in paths:
        with open(path, "rb") as handle:
            data = handle.read()
        try:
            new, added = add_static_symbols(data)
        except Failure as error:
            print(f"static_symbols.py: {path}: {error}", file=sys.stderr)
            status = 1
            continue
        if check:
            if added:
                print(f"{path}: {' '.join(added)}")
        elif new != data:
            with open(path, "wb") as handle:
                handle.write(new)
    return status


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
