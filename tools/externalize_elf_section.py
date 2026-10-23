#!/usr/bin/env python3
"""Replace a proved local ELF section with an absolute relocation anchor.

This is for overlay constants whose bytes already live in a retained binary
data segment. The expected section payload is mandatory, so compiler drift
fails instead of silently discarding new data. It may be supplied directly or
as ``sha256:DIGEST``. The optional anchor defaults to zero for existing callers.
Only section and symbol metadata changes: ordinary linking applies the untouched relocations
and their original stored addends to the absolute symbol value. GNU ld treats
STT_SECTION as the owning section itself and ignores its absolute st_value;
a nonzero anchor therefore changes that existing symbol to STT_NOTYPE,
retaining its name, binding and index. Zero retains the original type.
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
        if keep:
            raise SystemExit("duplicate keep argument")
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
if target_header[2] & 4:  # SHF_EXECINSTR
    raise SystemExit(f"{path}: cannot externalize executable section {section_name!r}")

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
for offset in range(symbol_offset, symbol_offset + symbol_size, symbol_entry_size):
    st_name, st_value, st_size = struct.unpack_from(">III", data, offset)
    st_info = data[offset + 12]
    st_shndx = struct.unpack_from(">H", data, offset + 14)[0]
    if st_shndx == target_index and (st_info & 0xF) == 3:  # STT_SECTION
        section_symbol_count += 1
        struct.pack_into(">II", data, offset + 4, anchor, 0)
        struct.pack_into(">H", data, offset + 14, 0xFFF1)  # SHN_ABS
        if anchor:
            # BFD resolves STT_SECTION through section identity, discarding
            # st_value even for SHN_ABS. A plain absolute symbol supplies S.
            data[offset + 12] = st_info & 0xF0  # preserve binding; STT_NOTYPE

if section_symbol_count != 1:
    raise SystemExit(
        f"{path}: expected one {section_name} section symbol, "
        f"found {section_symbol_count}"
    )

# The absolute symbol supplies S in the ordinary ELF relocation calculation.
# Leave every relocation record and stored addend untouched; in particular,
# no HI16/LO16 instruction word is adjusted here.

# Retain the section header/name. The default contributes no bytes, because
# those bytes already live in a retained data segment. `keep` leaves the
# proved payload in place and only retargets the section symbol to ABS anchor.
if not keep:
    struct.pack_into(">I", data, target_header_offset + 20, 0)
path.write_bytes(data)
