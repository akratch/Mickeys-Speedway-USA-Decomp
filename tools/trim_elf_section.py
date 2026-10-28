#!/usr/bin/env python3
"""Reduce one big-endian ELF32 section to a proved byte size.

IDO pads each translation unit's .text to a 16-byte object boundary. Overlay
functions can end on a four-byte boundary inside a larger runtime module, so
that object-only padding must not displace the following subsegment. Refuse to
discard anything but zero bytes unless the caller supplies the complete exact
discarded payload as hex or its SHA-256 digest. Explicit nonzero removal is
reserved for reviewed compiler-only material, including a duplicate data pool
whose relocations and retained linked payload are independently proved.
"""

import hashlib
import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import postprocess_guard as guard

_LOADED_COMPILER_END = hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest()


def _stack_word_store(word):
    return word >> 26 == 43 and (word >> 21) & 31 == 29 and word & 3 == 0


def _table_bound(elf, text, load_offset):
    """Locate an unchanged bounds test across only non-clobbering stack saves."""
    import runlink_pack as rp
    owners = [s for s in elf.symbols if s.section == ".text"
              and s.type == rp.STT_FUNC and s.size
              and s.value <= load_offset < s.value + s.size]
    if len(owners) != 1:
        raise ValueError("compiler table load lacks its unique actual function")
    start = owners[0].value
    branch_offset = load_offset - 16
    # The index shift is either the branch delay slot, or follows a stack
    # save in that slot. Stores cannot change the condition or index registers.
    if _stack_word_store(rp.u32(text, branch_offset)):
        branch_offset -= 4
    guard_offset = branch_offset - 4
    while guard_offset >= start and _stack_word_store(rp.u32(text, guard_offset)):
        guard_offset -= 4
    if guard_offset < start:
        raise ValueError("compiler table bounds cross their actual function")
    return rp.u32(text, guard_offset), rp.u32(text, branch_offset), branch_offset


def _compiler_storage(elf, section):
    if hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest() != _LOADED_COMPILER_END:
        raise ValueError("loaded compiler-end helper changed")
    sec = elf.section(section)
    if (struct.unpack_from(">HH", elf.data, 16) != (1, 8)
            or section not in (".data", ".rodata") or not sec or sec["type"] != 1
            or not sec["flags"] & 2 or sec["flags"] & 4 or sec["addr"] != 0
            or sec["align"] != 16):
        raise ValueError("compiler end requires aligned non-executable initialized storage")
    objects = [s for s in elf.symbols if s.section == section and s.type == 1]
    ends = []
    for symbol in objects:
        if symbol.value < 0 or symbol.value + max(1, symbol.size) > sec["size"]:
            raise ValueError("compiler end has an out-of-bounds object")
        if symbol.size:
            ends.append(symbol.value + symbol.size)
    return sec, objects, ends


def _zero_compiler_suffix(elf, section, sec, objects, ends):
    if not ends:
        raise ValueError("compiler end lacks actual sized objects or reads")
    end = max(ends)
    if (not 0 < sec["size"]-end < 16 or (end+15)//16*16 != sec["size"]
            or any(elf.bytes_of(section)[end:])
            or any(s.value >= end or s.value+s.size > end for s in objects)):
        raise ValueError("section suffix is not proved zero compiler end alignment")
    return end


def sized_object_end(elf, section):
    """Derive only the sized-object/zero-alignment boundary, without trimming.

    This does not authenticate outgoing references. A caller admitting them
    must separately prove every original relocation inside the object span.
    """
    return _zero_compiler_suffix(elf, section, *_compiler_storage(elf, section))


def compiler_end(elf, section):
    """Derive the existing bounded DATA or proved compiler-pool end."""
    import runlink_pack as rp
    sec, objects, ends = _compiler_storage(elf, section)
    incoming = []
    for source in elf.sections:
        rels = elf.rels(source["name"])
        for rel, addend in zip(rels, rp.in_place_addends(elf, source["name"], rels)):
            if rel.sym.section == section:
                if source["name"] != ".text" or section != ".rodata":
                    raise ValueError("unsupported incoming compiler-pool reference")
                incoming.append((rel, addend))
    outgoing = elf.rels(section)
    if section == ".data":
        if outgoing or incoming or not ends:
            raise ValueError("compiler DATA end lacks actual sized objects")
    else:
        text = elf.bytes_of(".text")
        cells = {}
        for rel, addend in zip(outgoing, rp.in_place_addends(elf, section, outgoing)):
            if (rel.type != rp.R_MIPS_32 or rel.offset % 4 or rel.sym.section != ".text"
                    or rel.sym.value + addend < 0 or (rel.sym.value + addend) % 4
                    or rel.offset in cells):
                raise ValueError("unsupported compiler jump-table cell")
            target = rel.sym.value + addend
            functions = [s for s in elf.symbols if s.section == ".text"
                         and s.type == rp.STT_FUNC and s.size and s.value <= target < s.value+s.size]
            if len(functions) != 1:
                raise ValueError("compiler table target lacks its unique actual function")
            cells[rel.offset] = target
        covered, paired = set(), set()
        for rel, base in incoming:
            if rel.type == rp.R_MIPS_HI16:
                continue
            if rel.type != rp.R_MIPS_LO16 or rel.offset < 20 or rel.offset % 4:
                raise ValueError("unsupported compiler-pool load")
            word = rp.u32(text, rel.offset)
            hi = [(r,a) for r,a in incoming if r.type == rp.R_MIPS_HI16 and a == base
                  and r.offset < rel.offset]
            if not hi:
                raise ValueError("compiler-pool load lacks its actual high relocation")
            high_word = rp.u32(text, hi[-1][0].offset)
            if high_word >> 26 != 15 or (high_word >> 16)&31 != (word >> 21)&31:
                raise ValueError("compiler-pool load lacks its actual address register")
            paired.add(hi[-1][0].offset)
            if word >> 26 == 49:
                end = base + 4
            elif word >> 26 == 35:
                guard, branch, branch_offset = _table_bound(elf, text, rel.offset)
                shift = rp.u32(text, rel.offset-12)
                high = rp.u32(text, rel.offset-8)
                add = rp.u32(text, rel.offset-4)
                count, index, scaled = guard & 65535, (guard >> 21)&31, (shift >> 11)&31
                displacement = branch & 65535
                if displacement & 32768: displacement -= 65536
                if (guard >> 26 != 11 or (guard >> 16)&31 != 1 or count == 0
                        or branch >> 26 != 4 or (branch >> 21)&31 != 1 or (branch >> 16)&31
                        or branch_offset+4+displacement*4 <= rel.offset+4
                        or shift >> 26 or shift & 63 or (shift >> 6)&31 != 2
                        or (shift >> 16)&31 != index or high >> 26 != 15
                        or (high >> 16)&31 != 1 or add >> 26 or add & 63 != 33
                        or (add >> 11)&31 != 1 or {(add >> 21)&31,(add >> 16)&31} != {1,scaled}
                        or (word >> 21)&31 != 1 or hi[-1][0].offset != rel.offset-8):
                    raise ValueError("unsupported compiler table bounds/load sequence")
                end = base + count*4
                if set(range(base,end,4)) - set(cells):
                    raise ValueError("compiler table load exceeds actual R32 cells")
            else:
                raise ValueError("unsupported compiler-pool load width")
            if base < 0 or end > sec["size"]:
                raise ValueError("compiler-pool read exceeds its section")
            covered.update(range(base,end))
            ends.append(end)
        if paired != {r.offset for r,a in incoming if r.type == rp.R_MIPS_HI16}:
            raise ValueError("unpaired compiler-pool address relocation")
        if not ends or covered != set(range(max(ends))) or any(
                not set(range(site,site+4)) <= covered for site in cells):
            raise ValueError("compiler-pool extent has unexplained reads or cells")
    return _zero_compiler_suffix(elf, section, sec, objects, ends)


def main():
    if len(sys.argv) not in (4, 5, 6):
        raise SystemExit(
            "usage: trim_elf_section.py OBJECT SECTION NEW_SIZE "
            "[EXPECTED_DISCARDED_HEX_OR_SHA256 | --literal-array-end SOURCE]"
        )

    path = pathlib.Path(sys.argv[1])
    section_name = sys.argv[2]
    new_size = int(sys.argv[3], 0)
    compiler_padding = len(sys.argv) == 5 and sys.argv[4] == "--compiler-end-padding"
    literal_source = sys.argv[5] if len(sys.argv) == 6 and sys.argv[4] == "--literal-array-end" else None
    if (len(sys.argv) == 6 and literal_source is None
            or len(sys.argv) == 5 and sys.argv[4] == "--literal-array-end"):
        raise SystemExit("unsupported end profile arguments")
    expected_discarded = None
    expected_digest = None
    if len(sys.argv) == 5 and not compiler_padding:
        if sys.argv[4].startswith("sha256:"):
            expected_digest = sys.argv[4][7:].lower()
            if len(expected_digest) != 64 or any(c not in "0123456789abcdef" for c in expected_digest):
                raise SystemExit("expected sha256: followed by 64 hexadecimal digits")
            if section_name == ".text":
                raise SystemExit("digest removal is restricted to non-executable data sections")
        else:
            expected_discarded = bytes.fromhex(sys.argv[4])
    data = bytearray(path.read_bytes())
    if compiler_padding:
        import runlink_pack as rp
        try:
            proved_end = compiler_end(rp.Elf(path, data), section_name)
        except (ValueError, rp.PackError, struct.error) as error:
            guard.fail(str(error))
        if proved_end != new_size:
            guard.fail("requested size differs from source-derived compiler end")
    literal = None
    if literal_source is not None:
        import proof_provenance as provenance
        import runlink_pack as rp
        root = pathlib.Path.cwd()
        source = root / literal_source
        if (pathlib.Path(literal_source).is_absolute() or ".." in source.relative_to(root).parts
                or not literal_source.startswith("src/") or not literal_source.endswith(".c")):
            guard.fail("literal end requires the actual configured project source")
        try:
            literal = provenance.prove_literal_arrays(rp.Elf(path, bytes(data)), source, root)
            if literal["section"] != section_name or literal["end"] != new_size:
                raise provenance.MetadataProofError("requested size differs from literal-derived end")
        except (provenance.MetadataProofError, ValueError, rp.PackError, struct.error) as error:
            guard.fail(str(error))
    if data[:6] != b"\x7fELF\x01\x02":
        raise SystemExit(f"{path}: expected a big-endian ELF32 object")

    section_header_offset = struct.unpack_from(">I", data, 0x20)[0]
    section_header_size = struct.unpack_from(">H", data, 0x2E)[0]
    section_count = struct.unpack_from(">H", data, 0x30)[0]
    string_table_index = struct.unpack_from(">H", data, 0x32)[0]

    def header(index):
        offset = section_header_offset + index * section_header_size
        return offset, struct.unpack_from(">10I", data, offset)

    _, string_header = header(string_table_index)
    string_offset, string_size = string_header[4], string_header[5]
    strings = data[string_offset : string_offset + string_size]

    for index in range(section_count):
        header_offset, values = header(index)
        name_offset, file_offset, old_size = values[0], values[4], values[5]
        end = strings.find(b"\0", name_offset)
        name = strings[name_offset:end].decode("ascii")
        if name != section_name:
            continue
        # The requested size is the ownership row's extent: what the module says
        # this translation unit occupies. A candidate whose codegen is a different
        # size therefore trips one of the three guards below, and the delta is the
        # measurement the promotion trial wants -- not a build failure with no
        # number in it. `text-size-differs (+N)` means the candidate's section is N
        # bytes longer than the module owns; negative, N bytes shorter.
        delta = old_size - new_size
        if new_size > old_size:
            guard.fail(
                f"{path}: cannot grow {name} from {old_size:#x} to {new_size:#x}",
                kind=f"{name.lstrip('.')}-size-differs ({delta:+d} bytes)",
            )
        discarded = data[file_offset + new_size : file_offset + old_size]
        if expected_digest is not None:
            if values[2] & 4:
                guard.fail("digest removal is restricted to non-executable data sections")
            actual_digest = hashlib.sha256(discarded).hexdigest()
            if actual_digest != expected_digest:
                guard.fail(
                    f"{path}: discarded {name} SHA-256 mismatch: "
                    f"expected {expected_digest}, got {actual_digest}",
                    kind=f"{name.lstrip('.')}-payload-differs ({delta:+d} bytes)",
                )
        if expected_discarded is not None and discarded != expected_discarded:
            guard.fail(
                f"{path}: discarded {name} payload changed: expected "
                f"{expected_discarded.hex()}, got {discarded.hex()}",
                kind=f"{name.lstrip('.')}-payload-differs ({delta:+d} bytes)",
            )
        if expected_discarded is None and expected_digest is None and any(discarded):
            guard.fail(
                f"{path}: refusing to trim nonzero bytes from {name}",
                kind=f"{name.lstrip('.')}-size-differs ({delta:+d} bytes)",
            )
        if literal is not None:
            if path.read_bytes() != bytes(data):
                guard.fail("loaded literal raw input changed before metadata publication")
            try:
                provenance.validate_literal_proof(rp.Elf(path, bytes(data)), literal, root)
            except (provenance.MetadataProofError, OSError, ValueError) as error:
                guard.fail(str(error))
        struct.pack_into(">I", data, header_offset + 0x14, new_size)
        path.write_bytes(data)
        break
    else:
        guard.fail(f"{path}: section {section_name!r} not found",
                   kind="section-missing")


if __name__ == "__main__":
    from proof_artifacts import run_cli
    run_cli(main)
