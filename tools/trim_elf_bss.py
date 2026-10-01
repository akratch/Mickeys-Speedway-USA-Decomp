#!/usr/bin/env python3
"""Trim compiler-only NOBITS tail alignment around one proved typed BSS object.

This changes only the section size and alignment metadata. Callers must prove
original storage ownership and the object's C type; object size alone does not
establish a retail allocation. No executable or initialized bytes are changed.
"""
import argparse
from pathlib import Path
import struct

from reloc_surface import Elf, STT_OBJECT


def trim(path, symbol_name, alignment):
    path = Path(path)
    obj = Elf(path)
    if obj.data[:6] != b'\x7fELF\x01\x02':
        raise ValueError('expected big-endian ELF32')
    if struct.unpack_from('>HH', obj.data, 16) != (1, 8):
        raise ValueError('expected relocatable MIPS object')
    if alignment <= 0 or alignment & (alignment - 1):
        raise ValueError('alignment must be a positive power of two')
    matches = [i for i, name in enumerate(obj.names) if name == '.bss']
    if len(matches) != 1:
        raise ValueError('requires exactly one .bss section')
    index, header = obj.section('.bss')
    if header[1] != 8 or header[2] != 3:
        raise ValueError('requires writable allocated non-executable NOBITS')
    old_size, old_alignment = header[5], header[8]
    if old_alignment <= 0 or old_alignment & (old_alignment - 1):
        raise ValueError('invalid original section alignment')
    if alignment > old_alignment:
        raise ValueError('cannot increase alignment')
    objects = []
    section_symbols = []
    for ordinal, (name, value, size, info, shndx) in enumerate(obj.symbols()):
        if shndx != index:
            continue
        kind = info & 15
        if kind == 3 and value == 0 and size in (0, old_size):
            section_symbols.append((ordinal, size))
            continue
        if kind != STT_OBJECT:
            raise ValueError('unproved non-object symbol in BSS')
        objects.append((name, value, size))
    if len(objects) != 1 or objects[0][0] != symbol_name:
        raise ValueError('requires sole named typed object')
    _, value, size = objects[0]
    if value != 0 or size <= 0 or size % alignment:
        raise ValueError('object must start at zero and occupy aligned extent')
    if old_size != ((size + old_alignment - 1) & -old_alignment):
        raise ValueError('tail is not precisely compiler section alignment')
    if any(section == '.bss' for section, *_ in obj.relocations(target=r'.*')):
        raise ValueError('relocations stored in NOBITS are unsupported')
    data = bytearray(obj.data)
    shoff = struct.unpack_from('>I', data, 32)[0]
    shentsize = struct.unpack_from('>H', data, 46)[0]
    if shentsize != 40:
        raise ValueError('unexpected section header size')
    struct.pack_into('>I', data, shoff + index * shentsize + 20, size)
    struct.pack_into('>I', data, shoff + index * shentsize + 32, alignment)
    tables = [h for h in obj.sh if h[1] == 2]
    if len(tables) != 1 or tables[0][9] != 16:
        raise ValueError('requires one standard symbol table')
    for ordinal, old_symbol_size in section_symbols:
        if old_symbol_size:
            struct.pack_into('>I', data, tables[0][4] + ordinal * 16 + 8, size)
    if path.read_bytes() != obj.data:
        raise ValueError('object changed while proving metadata')
    path.write_bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('symbol')
    parser.add_argument('alignment', type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        trim(args.object, args.symbol, args.alignment)
    except (ValueError, OSError, IndexError, struct.error) as exc:
        parser.exit(1, f'refused BSS trim: {exc}\n')


if __name__ == '__main__':
    main()
