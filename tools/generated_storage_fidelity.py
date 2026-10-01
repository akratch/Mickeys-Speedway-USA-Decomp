#!/usr/bin/env python3
"""Narrow proof for anim.c's compiler-generated local rodata in one TU pair.

This module does not assign runtime identities.  It authenticates only the
closed generated-storage shape used by ``func_800517E0`` when comparing an
original-name stock object with its configured full-TU peer.
"""

from __future__ import annotations

import struct
from typing import Any

import reloc_surface

SHT_PROGBITS = 1
SHT_RELA = 4
SHT_REL = 9
STB_LOCAL = 0
STT_SECTION = 3
SHF_WRITE = 0x1
SHF_ALLOC = 0x2
SHF_EXECINSTR = 0x4
R_MIPS_32 = 2
R_MIPS_HI16 = 5
R_MIPS_LO16 = 6

FUNCTION = "func_800517E0"
FUNCTION_START = 0x17E0
FUNCTION_SIZE = 6900
RODATA_SIZE = 0x250
FULL_RODATA_RELOCS = 142
FULL_TEXT_RODATA_GROUPS = {
    0x000: None,       # whole-TU context before the function
    0x004: "lwc1",
    0x008: "lw",
    0x204: "lwc1",
    0x208: "lwc1",
    0x20C: "lwc1",
    0x210: "lwc1",
    0x214: "lw",
    0x238: None,       # whole-TU context after the function's table
}
OWNED_GROUP_SITES = {
    0x004: (0xBC, 0xC0),
    0x008: (0xDC, 0xE4),
    0x204: (0x2E0, 0x2E4),
    0x208: (0x7C4, 0x7C8),
    0x20C: (0x880, 0x884),
    0x210: (0xC8C, 0xC90),
    0x214: (0x1484, 0x148C),
}
OWNED_TABLE_RANGES = ((0x008, 0x204), (0x214, 0x238))
UNOWNED_TABLE_RANGE = (0x238, 0x250)
ALL_TABLE_RANGES = ((0x008, 0x204), (0x214, 0x238), (0x238, 0x250))
OWNED_LITERALS = (0x004, 0x204, 0x208, 0x20C, 0x210)


class GeneratedStorageError(ValueError):
    """The generated storage is not the exact reviewed baseline geometry."""


def _fail(error_type: type[Exception], message: str) -> None:
    raise error_type(message)


def _single_section(elf: reloc_surface.Elf, name: str,
                    error_type: type[Exception]) -> tuple[int, tuple]:
    found = [(i, sh) for i, sh in enumerate(elf.sh) if elf.names[i] == name]
    if len(found) != 1:
        _fail(error_type, f"generated storage section {name!r} is missing or duplicated")
    return found[0]


def _single_symtab(elf: reloc_surface.Elf,
                   error_type: type[Exception]) -> int:
    found = [i for i, sh in enumerate(elf.sh) if sh[1] == 2]
    if len(found) != 1:
        _fail(error_type, "generated storage requires one symbol table")
    return found[0]


def _section_symbol(elf: reloc_surface.Elf, name: str, section_index: int,
                    section_size: int, error_type: type[Exception]) -> tuple[int, tuple]:
    symbols = elf.symbols()
    section_symbols = [(i, row) for i, row in enumerate(symbols)
                       if row[4] == section_index and row[3] & 0xF == STT_SECTION]
    if len(section_symbols) != 1:
        _fail(error_type, f"generated {name} section identity is not unique")
    found = [(i, row) for i, row in enumerate(symbols)
             if row[0] == name and row[4] == section_index]
    if len(found) != 1:
        _fail(error_type, f"generated {name} SECTION symbol is missing or ambiguous")
    index, row = found[0]
    if section_symbols[0] != (index, row):
        _fail(error_type, f"generated {name} SECTION symbol identity differs")
    if (row[3] >> 4 != STB_LOCAL or row[3] & 0xF != STT_SECTION
            or row[1] != 0 or row[2] != section_size):
        _fail(error_type, f"generated {name} SECTION symbol metadata differs")
    return index, row


def _rodata_symbol_closure(elf: reloc_surface.Elf, section_index: int,
                           section_symbol: tuple,
                           error_type: type[Exception]) -> tuple[dict[str, Any], ...]:
    """Require the generated section to have no named or typed aliases."""
    symbols = elf.symbols()
    owners = [row for row in symbols if row[4] == section_index]
    if owners != [section_symbol]:
        _fail(error_type, "generated storage contains an additional symbol owner")
    name, value, size, info, shndx = section_symbol
    return ({"name": name, "value": value, "size": size, "info": info,
             "section": elf.names[shndx]},)


def _table_relocations(elf: reloc_surface.Elf, symtab_index: int,
                       rodata_index: int, text_section_symbol: int,
                       function: dict[str, Any],
                       error_type: type[Exception]) -> list[dict[str, int]]:
    rel_tables = [(i, sh) for i, sh in enumerate(elf.sh)
                  if sh[1] == SHT_REL and sh[7] == rodata_index]
    rela_tables = [sh for sh in elf.sh if sh[1] == SHT_RELA and sh[7] == rodata_index]
    if rela_tables or len(rel_tables) != 1:
        _fail(error_type, "generated .rodata must have one REL table and no RELA table")
    rel_index, sh = rel_tables[0]
    if (elf.names[rel_index] != ".rel.rodata"
            or sh[6] != symtab_index or sh[9] != 8 or sh[8] != 4
            or sh[5] % 8):
        _fail(error_type, "generated .rodata REL table header is invalid")
    if sh[4] < 0 or sh[4] + sh[5] > len(elf.data):
        _fail(error_type, "generated .rodata REL table is truncated")
    if sh[5] // 8 != FULL_RODATA_RELOCS:
        _fail(error_type, "generated .rodata REL row count differs from reviewed extent")
    storage = elf.section_bytes(".rodata")
    start, end = function["start"], function["start"] + function["size"]
    rows = []
    previous = -1
    seen = set()
    for pos in range(sh[4], sh[4] + sh[5], 8):
        offset, info = struct.unpack_from(">II", elf.data, pos)
        kind, symidx = info & 0xFF, info >> 8
        if kind != R_MIPS_32 or symidx != text_section_symbol:
            _fail(error_type, "generated .rodata edge owner or relocation type differs")
        if offset % 4 or offset + 4 > RODATA_SIZE or offset <= previous or offset in seen:
            _fail(error_type, "generated .rodata relocation sites are unordered or invalid")
        previous = offset
        seen.add(offset)
        addend = struct.unpack_from(">I", storage, offset)[0]
        if addend % 4 or addend + 4 > len(elf.section_bytes(".text")):
            _fail(error_type, "generated .rodata target is outside aligned .text bounds")
        in_owned_table = any(lo <= offset < hi for lo, hi in OWNED_TABLE_RANGES)
        in_unowned_table = UNOWNED_TABLE_RANGE[0] <= offset < UNOWNED_TABLE_RANGE[1]
        if not (in_owned_table or in_unowned_table):
            _fail(error_type, "generated .rodata relocation lies outside reviewed tables")
        if (offset < 4) or (offset == 4):
            _fail(error_type, "generated literal/head context has an unexpected relocation")
        if in_owned_table and not start <= addend < end:
            _fail(error_type, "owned generated table edge escapes the selected function")
        rows.append({"site": offset, "type": kind,
                     "target_owner": ".text SECTION", "addend": addend,
                     "owned_by_function": in_owned_table})
    expected_sites = [off for lo, hi in ALL_TABLE_RANGES
                      for off in range(lo, hi, 4)]
    if [row["site"] for row in rows] != expected_sites:
        _fail(error_type, "generated table coverage has a gap or extra edge")
    return rows


def _text_rodata_groups(elf: reloc_surface.Elf, symtab_index: int,
                        text_index: int, rodata_symbol: int,
                        function: dict[str, Any],
                        error_type: type[Exception]) -> list[dict[str, Any]]:
    rel_tables = [(i, sh) for i, sh in enumerate(elf.sh)
                  if sh[1] == SHT_REL and sh[7] == text_index]
    rela_tables = [sh for sh in elf.sh if sh[1] == SHT_RELA and sh[7] == text_index]
    if rela_tables or len(rel_tables) != 1:
        _fail(error_type, "generated references require one .text REL table and no RELA")
    rel_index, sh = rel_tables[0]
    if (elf.names[rel_index] != ".rel.text"
            or sh[6] != symtab_index or sh[9] != 8 or sh[8] != 4
            or sh[5] % 8):
        _fail(error_type, "generated .text REL table header is invalid")
    if sh[4] + sh[5] > len(elf.data):
        _fail(error_type, "generated .text REL table is truncated")
    text = elf.section_bytes(".text")
    pending: list[tuple[int, int, int]] = []
    groups = []
    seen_sites = set()
    all_site_counts: dict[int, int] = {}
    previous_rodata_site = -1
    for pos in range(sh[4], sh[4] + sh[5], 8):
        offset, info = struct.unpack_from(">II", elf.data, pos)
        kind, symidx = info & 0xFF, info >> 8
        # The complete REL table is parsed in its on-disk order.  Other
        # symbol owners may be emitted in compiler groups, so only the
        # `.rodata` owner's own subsequence is required to be ordered.
        if offset % 4 or offset + 4 > len(text) or symidx >= len(elf.symbols()):
            _fail(error_type, "full .text REL stream has an invalid row")
        all_site_counts[offset] = all_site_counts.get(offset, 0) + 1
        if symidx != rodata_symbol:
            continue
        if offset <= previous_rodata_site:
            _fail(error_type, "generated .rodata reference rows are unordered")
        previous_rodata_site = offset
        if kind not in (R_MIPS_HI16, R_MIPS_LO16):
            _fail(error_type, "generated .rodata reference uses an unsupported relocation")
        if offset % 4 or offset + 4 > len(text) or offset in seen_sites:
            _fail(error_type, "generated .rodata reference site is invalid or duplicated")
        seen_sites.add(offset)
        word = struct.unpack_from(">I", text, offset)[0]
        opcode, low = word >> 26, word & 0xFFFF
        if kind == R_MIPS_HI16:
            if opcode != 15:
                _fail(error_type, "generated .rodata HI16 is not a LUI")
            pending.append((offset, low, word))
            continue
        if len(pending) != 1:
            _fail(error_type, "generated .rodata HI/LO graph has pending or ambiguous highs")
        if opcode not in (35, 49):
            _fail(error_type, "generated .rodata low use is not an approved word/FP load")
        hi_site, high, hi_word = pending.pop()
        signed_low = low - 0x10000 if low & 0x8000 else low
        addend = ((high << 16) + signed_low) & 0xFFFFFFFF
        kind_name = FULL_TEXT_RODATA_GROUPS.get(addend, "unsupported")
        if kind_name == "unsupported":
            _fail(error_type, "generated .rodata HI/LO addend is outside reviewed ranges")
        expected_opcode = {"lwc1": 49, "lw": 35}.get(kind_name)
        if expected_opcode is not None and opcode != expected_opcode:
            _fail(error_type, "generated .rodata range uses the wrong load width")
        start, end = function["start"], function["start"] + function["size"]
        inside_hi = start <= hi_site < end
        inside_lo = start <= offset < end
        owned = addend in OWNED_GROUP_SITES
        if inside_hi != inside_lo or inside_hi != owned:
            _fail(error_type, "generated .rodata pair crosses or escapes its ownership boundary")
        if owned:
            expected_hi, expected_lo = OWNED_GROUP_SITES[addend]
            if (hi_site - start, offset - start) != (expected_hi, expected_lo):
                _fail(error_type, "selected generated HI/LO group sites differ")
        groups.append({"addend": addend, "owner": ".rodata SECTION",
                       "hi_site": hi_site, "lo_site": offset,
                       "hi_word": hi_word, "lo_word": word,
                       "lo_opcode": opcode,
                       "owned": owned})
    if pending:
        _fail(error_type, "generated .rodata HI16 remains unpaired")
    if len(groups) != len(FULL_TEXT_RODATA_GROUPS):
        _fail(error_type, "full translation unit does not have nine generated pairs")
    if {row["addend"] for row in groups} != set(FULL_TEXT_RODATA_GROUPS):
        _fail(error_type, "full translation-unit generated addend set differs")
    if any(all_site_counts.get(site, 0) != 1
           for group in groups for site in (group["hi_site"], group["lo_site"])):
        _fail(error_type, "generated HI/LO use site has a conflicting full-TU relocation")
    if sum(row["owned"] for row in groups) != 7:
        _fail(error_type, "selected function generated pair count differs")
    return groups


def _inspect(elf: reloc_surface.Elf, function: dict[str, Any],
             error_type: type[Exception]) -> tuple[dict[str, Any], list[dict[str, Any]], list[dict[str, Any]]]:
    text_index, text_header = _single_section(elf, ".text", error_type)
    rodata_index, rodata_header = _single_section(elf, ".rodata", error_type)
    symtab = _single_symtab(elf, error_type)
    symtab_header = elf.sh[symtab]
    if (symtab_header[9] != 16 or symtab_header[5] % 16
            or symtab_header[6] >= len(elf.sh)
            or elf.sh[symtab_header[6]][1] != 3
            or symtab_header[4] + symtab_header[5] > len(elf.data)):
        _fail(error_type, "generated-storage symbol table geometry is invalid")
    if (text_header[1] != SHT_PROGBITS or not text_header[2] & SHF_ALLOC
            or not text_header[2] & SHF_EXECINSTR or text_header[2] & SHF_WRITE):
        _fail(error_type, "generated target .text section has incompatible geometry")
    if (rodata_header[1] != SHT_PROGBITS or rodata_header[5] != RODATA_SIZE
            or rodata_header[8] != 16 or rodata_header[9] != 1
            or not rodata_header[2] & SHF_ALLOC
            or rodata_header[2] & (SHF_WRITE | SHF_EXECINSTR)):
        _fail(error_type, "generated .rodata header differs from reviewed baseline")
    if rodata_header[4] + rodata_header[5] > len(elf.data):
        _fail(error_type, "generated .rodata payload is truncated")
    if len(elf.section_bytes(".rodata")) != RODATA_SIZE:
        _fail(error_type, "generated .rodata payload has the wrong extent")
    _, text_sym = _section_symbol(elf, ".text", text_index,
                                  text_header[5], error_type)
    text_symbol_index = next(i for i, row in enumerate(elf.symbols())
                             if row == text_sym)
    rodata_symbol_index, rodata_sym = _section_symbol(
        elf, ".rodata", rodata_index, RODATA_SIZE, error_type)
    symbol_closure = _rodata_symbol_closure(
        elf, rodata_index, rodata_sym, error_type)
    section_descriptor = {"name": ".rodata", "type": STT_SECTION,
                          "binding": STB_LOCAL, "defined": True,
                          "value": 0, "size": RODATA_SIZE,
                          "storage": (".rodata", SHT_PROGBITS, rodata_header[2]),
                          "symbol_closure": symbol_closure}
    table_rows = _table_relocations(elf, symtab, rodata_index,
                                    text_symbol_index, function, error_type)
    groups = _text_rodata_groups(elf, symtab, text_index,
                                 rodata_symbol_index, function, error_type)
    return section_descriptor, table_rows, groups


def validate_generated_rodata_pair(raw: reloc_surface.Elf,
                                   configured: reloc_surface.Elf,
                                   function_name: str,
                                   raw_function: dict[str, Any],
                                   configured_function: dict[str, Any],
                                   error_type: type[Exception]) -> dict[str, Any]:
    """Validate the one reviewed anonymous-rodata source-fidelity baseline."""
    if function_name != FUNCTION:
        _fail(error_type, "generated-rodata policy is limited to func_800517E0")
    for function in (raw_function, configured_function):
        if (function["start"], function["size"]) != (FUNCTION_START, FUNCTION_SIZE):
            _fail(error_type, "generated-rodata function boundary differs from reviewed baseline")
    raw_text_index, raw_text = _single_section(raw, ".text", error_type)
    full_text_index, full_text = _single_section(configured, ".text", error_type)
    if (raw_text[1], raw_text[2], raw_text[5], raw_text[8], raw_text[9]) != (
            full_text[1], full_text[2], full_text[5], full_text[8], full_text[9]):
        _fail(error_type, "raw/configured .text section geometry differs")
    if raw_function["start"] != configured_function["start"]:
        _fail(error_type, "raw/configured selected function placement differs")
    raw_descriptor, raw_table, raw_groups = _inspect(raw, raw_function, error_type)
    full_descriptor, full_table, full_groups = _inspect(
        configured, configured_function, error_type)
    if raw_descriptor != full_descriptor:
        _fail(error_type, "raw/configured generated section owner differs")
    if raw.section_bytes(".rodata") != configured.section_bytes(".rodata"):
        _fail(error_type, "raw/configured complete generated .rodata bytes differ")
    if raw_table != full_table:
        _fail(error_type, "raw/configured complete ordered generated table edges differ")
    if raw_groups != full_groups:
        _fail(error_type, "raw/configured complete generated HI/LO graph differs")
    raw_owned = raw.section_bytes(".text")[FUNCTION_START:FUNCTION_START + FUNCTION_SIZE]
    full_owned = configured.section_bytes(".text")[FUNCTION_START:FUNCTION_START + FUNCTION_SIZE]
    if raw_owned != full_owned:
        _fail(error_type, "raw/configured owned executable bytes differ")
    return {"policy": "anim-func-800517E0-generated-rodata-v1",
            "section": ".rodata", "section_size": RODATA_SIZE,
            "table_relocation_count": len(raw_table),
            "table_ranges": [{"start": lo, "end": hi,
                              "owned_by_function": (lo, hi) in OWNED_TABLE_RANGES}
                             for lo, hi in ALL_TABLE_RANGES],
            "rodata_header": {"type": SHT_PROGBITS,
                              "flags": raw_descriptor["storage"][2],
                              "size": RODATA_SIZE, "alignment": 16,
                              "entry_size": 1},
            "ordered_table_edges": raw_table,
            "full_tu_hi_lo_group_rows": raw_groups,
            "owned_literal_addends": list(OWNED_LITERALS),
            "full_tu_hi_lo_groups": len(raw_groups),
            "owned_hi_lo_groups": sum(row["owned"] for row in raw_groups),
            "unowned_hi_lo_groups": sum(not row["owned"] for row in raw_groups),
            "whole_rodata_bytes_equal": True,
            "complete_ordered_table_edges_equal": True,
            "owned_text_bytes_equal": True,
            "runtime_identity_proved": False,
            "promotion_authority": False}
