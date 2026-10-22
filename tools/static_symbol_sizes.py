#!/usr/bin/env python3
"""Give the static functions of an IDO object their sizes, from ``.mdebug``.

IDO writes no ELF symbol for a ``static`` function. asm-processor's static
conversion (``--force``, default ``--convert-statics local``) adds a local
``STT_FUNC`` for each one from the object's ``.mdebug`` symbolic table, but
with size 0, so tools that read sized function definitions (progress,
promotion proofs) cannot see them. IDO records every procedure's size as the
value of the procedure's ``stEnd`` symbol in ``.mdebug``, the same number it
puts in a global function's ``st_size``. This tool copies that value into
each unsized local ``STT_FUNC`` whose name and start match a static procedure.

Only symbol-table ``st_size`` fields change: no section contents, relocation,
instruction or symbol name is touched. An object with no such symbol is left
byte-identical. It refuses a local unsized function with no matching
procedure, or a procedure whose recorded extent leaves ``.text``.

usage: static_symbol_sizes.py OBJECT
"""

import struct
import sys

SHT_SYMTAB = 2
STT_FUNC = 2
STB_LOCAL = 0
ST_PROC = 6
ST_STATIC_PROC = 14
ST_END = 8
SC_TEXT = 1


class Failure(Exception):
    pass


def _sections(data):
    if data[:6] != b"\x7fELF\x01\x02":
        raise Failure("expected a big-endian ELF32 object")
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from(">HHH", data, 0x2E)
    sections = []
    for index in range(shnum):
        fields = struct.unpack_from(">10I", data, shoff + index * shentsize)
        sections.append({
            "name": fields[0], "type": fields[1], "offset": fields[4],
            "size": fields[5], "link": fields[6],
        })
    strtab = sections[shstrndx]
    for section in sections:
        start = strtab["offset"] + section["name"]
        section["name"] = data[start:data.index(b"\0", start)].decode("ascii")
    return sections


def static_procedures(data, mdebug):
    """{(name, start): size} for every static procedure in ``.mdebug``."""
    base = mdebug["offset"]
    header = data[base:base + mdebug["size"]]
    if len(header) < 0x60:
        raise Failure(".mdebug is shorter than its header")
    # The header's table offsets are file offsets of the object as the
    # compiler wrote it; asm-processor rewrites the object without updating
    # them. IDO writes the line table directly after the 0x60-byte header,
    # so cbLineOffset locates where the section then began.
    words = struct.unpack_from(">24I", header, 0)
    shift = base - (words[3] - 0x60)
    cb_sym_offset = words[9] + shift
    cb_ss_offset = words[15] + shift
    ifd_max, cb_fd_offset = words[18], words[19] + shift
    found = {}
    for fd in range(ifd_max):
        iss_base, _, isym_base, csym = struct.unpack_from(
            ">IIII", data, cb_fd_offset + 72 * fd + 8)

        def symbol(index):
            iss, value, packed = struct.unpack_from(
                ">III", data, cb_sym_offset + 12 * (isym_base + index))
            start = cb_ss_offset + iss_base + iss
            name = data[start:data.index(b"\0", start)].decode("latin1")
            return name, value, packed >> 26, (packed >> 21) & 0x1F

        for index in range(csym):
            name, value, st, sc = symbol(index)
            if st != ST_STATIC_PROC or sc != SC_TEXT:
                continue
            for later in range(index + 1, csym):
                end_name, end_value, end_st, _ = symbol(later)
                if end_st == ST_END and end_name == name:
                    found[(name, value)] = end_value
                    break
            else:
                raise Failure(f"static procedure {name} has no stEnd")
    return found


def size_statics(data):
    """Return (new object bytes, number of symbols sized)."""
    sections = _sections(data)
    by_name = {section["name"]: index for index, section in enumerate(sections)}
    symtab = next((s for s in sections if s["type"] == SHT_SYMTAB), None)
    if symtab is None or ".text" not in by_name:
        return data, 0
    text_index = by_name[".text"]
    text_size = sections[text_index]["size"]
    strtab = sections[symtab["link"]]
    targets = []
    for index in range(symtab["size"] // 16):
        offset = symtab["offset"] + index * 16
        name_offset, value, size, info, _, shndx = struct.unpack_from(
            ">IIIBBH", data, offset)
        if (shndx == text_index and size == 0 and info >> 4 == STB_LOCAL
                and info & 0xF == STT_FUNC):
            start = strtab["offset"] + name_offset
            name = data[start:data.index(b"\0", start)].decode("latin1")
            if name:
                targets.append((offset, name, value))
    if not targets:
        return data, 0
    if ".mdebug" not in by_name:
        raise Failure("unsized static functions but no .mdebug to size them")
    procedures = static_procedures(data, sections[by_name[".mdebug"]])
    out = bytearray(data)
    for offset, name, value in targets:
        size = procedures.get((name, value))
        if size is None:
            raise Failure(f"local function {name} at {value:#x} has no static procedure")
        if size == 0 or size % 4 or value + size > text_size:
            raise Failure(f"static procedure {name} has an invalid extent {size:#x}")
        struct.pack_into(">I", out, offset + 8, size)
    return bytes(out), len(targets)


def main(argv):
    if len(argv) != 2:
        print(__doc__.strip().splitlines()[-1], file=sys.stderr)
        return 2
    path = argv[1]
    with open(path, "rb") as handle:
        data = handle.read()
    try:
        new, count = size_statics(data)
    except Failure as error:
        print(f"static_symbol_sizes.py: {path}: {error}", file=sys.stderr)
        return 1
    if count:
        with open(path, "wb") as handle:
            handle.write(new)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
