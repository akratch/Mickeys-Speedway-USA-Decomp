#!/usr/bin/env python3
"""Replace a proved local ELF section with an absolute relocation anchor.

This is for overlay constants whose bytes already live in a retained binary
data segment. The expected section payload is mandatory, so compiler drift
fails instead of silently discarding new data. It may be supplied directly or
as ``sha256:DIGEST``. The optional anchor defaults to zero for existing callers.
"""

import hashlib
import pathlib
import struct
import sys


if len(sys.argv) not in (4, 5, 6):
    raise SystemExit(
        "usage: externalize_elf_section.py OBJECT SECTION "
        "EXPECTED_HEX_OR_SHA256 "
        "[ABSOLUTE_ANCHOR] [keep]"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
expected_spec = sys.argv[3]
keep = False
anchor_text = None
for extra in sys.argv[4:]:
    if extra == "keep":
        keep = True
    elif anchor_text is None:
        anchor_text = extra
    else:
        raise SystemExit(f"unexpected argument {extra!r}")
anchor = int(anchor_text, 0) if anchor_text is not None else 0
if not 0 <= anchor < 0x8000:
    raise SystemExit(
        f"absolute anchor must fit the supported positive MIPS LO16 range: "
        f"{anchor:#x}"
    )
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit(f"{path}: expected a big-endian ELF32 object")

section_header_offset = struct.unpack_from(">I", data, 0x20)[0]
section_header_size = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
string_table_index = struct.unpack_from(">H", data, 0x32)[0]


def section_header(index):
    offset = section_header_offset + index * section_header_size
    return offset, struct.unpack_from(">10I", data, offset)


_, string_header = section_header(string_table_index)
strings = data[string_header[4] : string_header[4] + string_header[5]]
target_index = None
target_header_offset = None
target_header = None
symtab_header = None

for index in range(section_count):
    header_offset, values = section_header(index)
    name_end = strings.find(b"\0", values[0])
    name = strings[values[0] : name_end].decode("ascii")
    if name == section_name:
        target_index = index
        target_header_offset = header_offset
        target_header = values
    if values[1] == 2:  # SHT_SYMTAB
        symtab_header = values

if target_index is None or target_header is None:
    raise SystemExit(f"{path}: section {section_name!r} not found")
if symtab_header is None:
    raise SystemExit(f"{path}: symbol table not found")

payload = bytes(data[target_header[4] : target_header[4] + target_header[5]])
if expected_spec.startswith("sha256:"):
    expected_digest = expected_spec.removeprefix("sha256:").lower()
    if len(expected_digest) != 64 or any(
        character not in "0123456789abcdef" for character in expected_digest
    ):
        raise SystemExit("expected sha256: followed by 64 hexadecimal digits")
    payload_matches = hashlib.sha256(payload).hexdigest() == expected_digest
else:
    expected = bytes.fromhex(expected_spec)
    payload_matches = payload == expected
if not payload_matches:
    raise SystemExit(
        f"{path}: {section_name} payload changed: "
        f"expected {expected_spec}, got sha256:{hashlib.sha256(payload).hexdigest()}"
    )

symbol_offset = symtab_header[4]
symbol_size = symtab_header[5]
symbol_entry_size = symtab_header[9] or 16
section_symbol_count = 0
section_symbol_index = None
for index, offset in enumerate(
    range(symbol_offset, symbol_offset + symbol_size, symbol_entry_size)
):
    st_name, st_value, st_size = struct.unpack_from(">III", data, offset)
    st_info = data[offset + 12]
    st_shndx = struct.unpack_from(">H", data, offset + 14)[0]
    if st_shndx == target_index and (st_info & 0xF) == 3:  # STT_SECTION
        section_symbol_count += 1
        section_symbol_index = index
        struct.pack_into(">II", data, offset + 4, 0, 0)
        struct.pack_into(">H", data, offset + 14, 0xFFF1)  # SHN_ABS

if section_symbol_count != 1:
    raise SystemExit(
        f"{path}: expected one {section_name} section symbol, "
        f"found {section_symbol_count}"
    )

# ELF32/MIPS REL records carry their addends in the relocated instruction.
# Once the section symbol becomes ABS zero, move the requested overlay-local
# anchor into each LO16 addend. Restrict this helper to the simple positive
# range where the paired HI16 word cannot change; fail instead of attempting a
# lossy generic HI16 carry rewrite.
if anchor:
    hi_count = 0
    lo_count = 0
    for index in range(section_count):
        _, relocation_header = section_header(index)
        if relocation_header[1] != 9:  # SHT_REL
            continue
        relocated_section_index = relocation_header[7]
        _, relocated_header = section_header(relocated_section_index)
        entry_size = relocation_header[9] or 8
        if entry_size != 8:
            raise SystemExit(
                f"{path}: unsupported ELF32 REL entry size {entry_size}"
            )
        start = relocation_header[4]
        end = start + relocation_header[5]
        for offset in range(start, end, entry_size):
            relocation_offset, relocation_info = struct.unpack_from(
                ">II", data, offset
            )
            if relocation_info >> 8 != section_symbol_index:
                continue
            relocation_type = relocation_info & 0xFF
            if relocation_type == 5:  # R_MIPS_HI16
                hi_count += 1
                continue
            if relocation_type != 6:  # R_MIPS_LO16
                raise SystemExit(
                    f"{path}: unsupported relocation type {relocation_type} "
                    f"against anchored {section_name}"
                )
            if relocation_offset + 4 > relocated_header[5]:
                raise SystemExit(
                    f"{path}: relocation offset {relocation_offset:#x} "
                    "falls outside its target section"
                )
            word_offset = relocated_header[4] + relocation_offset
            word = struct.unpack_from(">I", data, word_offset)[0]
            low_addend = word & 0xFFFF
            if low_addend & 0x8000:
                low_addend -= 0x10000
            anchored_addend = low_addend + anchor
            if not 0 <= anchored_addend < 0x8000:
                raise SystemExit(
                    f"{path}: {section_name} addend {low_addend:#x} plus "
                    f"anchor {anchor:#x} needs an HI16 carry rewrite"
                )
            struct.pack_into(
                ">I", data, word_offset,
                (word & 0xFFFF0000) | anchored_addend,
            )
            lo_count += 1
    if hi_count != lo_count or lo_count == 0:
        raise SystemExit(
            f"{path}: expected paired HI16/LO16 relocations against "
            f"{section_name}, found {hi_count} HI16 and {lo_count} LO16"
        )

# Retain the section header/name. The default contributes no bytes, because
# those bytes already live in a retained data segment. `keep` leaves the
# proved payload in place and only retargets the section symbol to ABS zero.
if not keep:
    struct.pack_into(">I", data, target_header_offset + 20, 0)
path.write_bytes(data)
