#!/usr/bin/env python3
"""Compare named source-symbol relocations across configured/raw TU objects.

This proves source-symbol correspondence and emitted instruction-field
fidelity only. It does not prove a target/runtime address or relocation
identity; callers must obtain that authority independently.
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface  # noqa: E402
import generated_storage_fidelity  # noqa: E402

SHT_PROGBITS = 1
SHT_NOBITS = 8
SHT_REL = 9
SHT_SYMTAB = 2
SHN_UNDEF = 0
STB_LOCAL, STB_GLOBAL, STB_WEAK = 0, 1, 2
STT_OBJECT, STT_FUNC, STT_SECTION = 1, 2, 3
SHF_ALLOC, SHF_WRITE, SHF_EXECINSTR = 0x2, 0x1, 0x4
R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 4, 5, 6
FIELD_MASKS = {R_MIPS_26: 0xFC000000,
               R_MIPS_HI16: 0xFFFF0000,
               R_MIPS_LO16: 0xFFFF0000}


class SourceFidelityError(ValueError):
    """The two objects cannot be compared under this narrow proof route."""


def _symtab_index(elf: reloc_surface.Elf) -> int:
    rows = [i for i, sh in enumerate(elf.sh) if sh[1] == SHT_SYMTAB]
    if len(rows) != 1:
        raise SourceFidelityError("expected exactly one ELF symbol table")
    return rows[0]


def _unique_named(symbols: list[tuple], name: str) -> tuple:
    if not name:
        raise SourceFidelityError("unnamed relocation owner is unsupported")
    found = [row for row in symbols if row[0] == name]
    if len(found) != 1:
        raise SourceFidelityError(f"owner {name!r} is missing or duplicated")
    return found[0]


def _section_owner(elf: reloc_surface.Elf, symbol: tuple) -> dict[str, Any]:
    name, value, size, info, shndx = symbol
    bind, typ = info >> 4, info & 0xF
    if typ not in (STT_FUNC, STT_OBJECT):
        label = "SECTION" if typ == STT_SECTION else f"type-{typ}"
        raise SourceFidelityError(f"owner {name!r} has unsupported {label} type")
    if bind not in (STB_LOCAL, STB_GLOBAL, STB_WEAK):
        raise SourceFidelityError(f"owner {name!r} has unsupported binding {bind}")
    if shndx == SHN_UNDEF:
        if value != 0:
            raise SourceFidelityError(f"undefined owner {name!r} has a value")
        return {"name": name, "type": typ, "binding": bind,
                "defined": False, "value": value, "size": size,
                "storage": None}
    if shndx >= len(elf.sh):
        raise SourceFidelityError(f"owner {name!r} has a reserved/invalid section")
    sh = elf.sh[shndx]
    sh_type, flags, section_size = sh[1], sh[2], sh[5]
    if sh_type not in (SHT_PROGBITS, SHT_NOBITS):
        raise SourceFidelityError(f"owner {name!r} is not in a data-bearing section")
    if not flags & SHF_ALLOC:
        raise SourceFidelityError(f"owner {name!r} is not in allocated storage")
    if typ == STT_FUNC:
        if sh_type != SHT_PROGBITS or not flags & SHF_EXECINSTR or flags & SHF_WRITE:
            raise SourceFidelityError(f"function owner {name!r} has incompatible storage")
    elif flags & SHF_EXECINSTR:
        raise SourceFidelityError(f"object owner {name!r} has executable storage")
    if size <= 0 or value < 0 or value + size > section_size:
        raise SourceFidelityError(f"defined owner {name!r} has invalid extent")
    if sh_type == SHT_PROGBITS and sh[4] + section_size > len(elf.data):
        raise SourceFidelityError(f"owner {name!r} has truncated section storage")
    return {"name": name, "type": typ, "binding": bind, "defined": True,
            "value": value, "size": size,
            "storage": (elf.names[shndx], sh_type, flags)}


def _compatible_owner(left: dict[str, Any], right: dict[str, Any]) -> None:
    if left["name"] != right["name"]:
        raise SourceFidelityError("original source owner names differ")
    if (left["type"], left["binding"]) != (right["type"], right["binding"]):
        raise SourceFidelityError(f"owner {left['name']!r} type/binding differs")
    if left["defined"] and right["defined"]:
        # Section indices and values are deliberately not compared: they are
        # layout coordinates, while the decoded REL addend is owner-relative.
        if left["size"] != right["size"]:
            raise SourceFidelityError(f"owner {left['name']!r} extent differs")
        if left["storage"] != right["storage"]:
            raise SourceFidelityError(f"owner {left['name']!r} storage differs")
    elif not left["defined"] and not right["defined"]:
        if left["size"] != right["size"]:
            raise SourceFidelityError(f"undefined owner {left['name']!r} declaration size differs")
    else:
        undefined, defined = (left, right) if not left["defined"] else (right, left)
        if undefined["size"] not in (0, defined["size"]):
            raise SourceFidelityError(f"owner {left['name']!r} declaration size differs")


def _function(elf: reloc_surface.Elf, symbols: list[tuple], name: str) -> dict[str, Any]:
    symbol = _unique_named(symbols, name)
    owner = _section_owner(elf, symbol)
    if owner["type"] != STT_FUNC or not owner["defined"]:
        raise SourceFidelityError(f"target function {name!r} is not a defined FUNC")
    _, value, size, _, shndx = symbol
    if elf.names.count(elf.names[shndx]) != 1:
        raise SourceFidelityError("owned executable section name is ambiguous")
    if value % 4 or size % 4:
        raise SourceFidelityError("target function extent is not word aligned")
    return {"symbol": symbol, "owner": owner, "section": shndx,
            "start": value, "size": size}


def _relocations(elf: reloc_surface.Elf, symtab: int,
                 function: dict[str, Any], symbols: list[tuple],
                 generated_section_owners: dict[str, dict[str, Any]] | None = None
                 ) -> list[dict[str, Any]]:
    start, end = function["start"], function["start"] + function["size"]
    text = elf.section_bytes(elf.names[function["section"]])
    pending: dict[str, list[tuple[int, int]]] = {}
    rows: list[dict[str, Any]] = []
    seen_sites: set[int] = set()
    for section_index, sh in enumerate(elf.sh):
        if sh[1] == 4 and sh[7] == function["section"]:
            raise SourceFidelityError("owned executable RELA table is unsupported")
        if sh[1] != SHT_REL or sh[7] != function["section"]:
            continue
        if sh[6] != symtab or sh[9] not in (0, 8) or sh[5] % 8:
            raise SourceFidelityError("malformed or foreign REL table")
        for pos in range(sh[4], sh[4] + sh[5], 8):
            off, r_info = struct.unpack_from(">II", elf.data, pos)
            kind, symidx = r_info & 0xFF, r_info >> 8
            if not start <= off < end:
                continue
            if kind not in FIELD_MASKS:
                if kind == 10:
                    raise SourceFidelityError("local PC16 branch relocation is unsupported")
                raise SourceFidelityError(f"unsupported relocation type {kind}")
            if off % 4 or off + 4 > end or symidx >= len(symbols):
                raise SourceFidelityError("invalid relocation site or symbol index")
            site = off - start
            if site in seen_sites:
                raise SourceFidelityError(f"duplicate relocation site +0x{site:X}")
            seen_sites.add(site)
            owner_symbol = symbols[symidx]
            if _unique_named(symbols, owner_symbol[0]) != owner_symbol:
                raise SourceFidelityError("relocation does not use the unique named owner")
            generated = ((generated_section_owners or {}).get(owner_symbol[0]))
            if generated is not None:
                if (owner_symbol != generated.get("source_symbol")
                        or owner_symbol[4] != generated.get("section_index")):
                    raise SourceFidelityError(
                        "anonymous generated owner differs from its validated SECTION symbol")
                owner = {key: value for key, value in generated.items()
                         if key not in ("source_symbol", "section_index")}
            else:
                owner = _section_owner(elf, owner_symbol)
            name = owner["name"]
            field = struct.unpack_from(">I", text, off)[0]
            opcode = field >> 26
            if kind == R_MIPS_26:
                if owner["type"] != STT_FUNC or opcode not in (2, 3):
                    raise SourceFidelityError("R_MIPS_26 is not a named function jump")
                rows.append({"site": site, "type": kind, "name": name,
                             "addend": (field & 0x03FFFFFF) << 2,
                             "owner": owner})
            elif kind == R_MIPS_HI16:
                if opcode != 15:
                    raise SourceFidelityError("R_MIPS_HI16 does not name a LUI field")
                pending.setdefault(name, []).append((site, field & 0xFFFF))
            else:
                # This narrow route accepts signed immediate address fields,
                # including integer/FP loads and stores, never R-format bits.
                if opcode not in (8, 9, 24, 25, 32, 33, 34, 35, 36, 37, 38,
                                  39, 40, 41, 42, 43, 44, 45, 46, 49, 53, 57, 61):
                    raise SourceFidelityError("R_MIPS_LO16 does not name a signed immediate field")
                low = field & 0xFFFF
                signed_low = low - 0x10000 if low & 0x8000 else low
                highs = pending.pop(name, [])
                if highs:
                    addends = {((high << 16) + signed_low) & 0xFFFFFFFF
                               for _, high in highs}
                    if len(addends) != 1:
                        raise SourceFidelityError(
                            f"ambiguous HI16/LO16 addends for owner {name!r}")
                    addend = next(iter(addends))
                    for hi_site, _ in highs:
                        rows.append({"site": hi_site, "type": R_MIPS_HI16,
                                     "name": name, "addend": addend,
                                     "owner": owner})
                else:
                    # A standalone LO16 carries its own sign-extended REL addend.
                    addend = signed_low & 0xFFFFFFFF
                rows.append({"site": site, "type": kind, "name": name,
                             "addend": addend, "owner": owner})
    if pending:
        raise SourceFidelityError("unpaired HI16 relocation owner(s): " +
                                  ", ".join(sorted(pending)))
    rows.sort(key=lambda row: (row["site"], row["type"], row["name"]))
    return rows


def _normalized_bytes(elf: reloc_surface.Elf,
                      function: dict[str, Any]) -> bytes:
    name = elf.names[function["section"]]
    section = bytearray(elf.section_bytes(name)[function["start"]:
                                                   function["start"] + function["size"]])
    for _, off, kind, _ in elf.relocations(target=re.escape(name)):
        if function["start"] <= off < function["start"] + function["size"]:
            mask = FIELD_MASKS.get(kind)
            if mask is None:
                raise SourceFidelityError(f"cannot normalize relocation type {kind}")
            at = off - function["start"]
            word = struct.unpack_from(">I", section, at)[0] & mask
            struct.pack_into(">I", section, at, word)
    return bytes(section)


def _compare_source_symbols(raw_object: Path, configured_object: Path,
                            function_name: str,
                            generated_rodata: bool = False) -> dict[str, Any]:
    """Compare original-name REL owners and instruction fields in two objects.

    ``raw_object`` is emitted from the original-name function source;
    ``configured_object`` is the current configured full-TU object. A true
    result is source fidelity only, never a target/runtime binding witness.
    """
    try:
        raw = reloc_surface.Elf(Path(raw_object))
        full = reloc_surface.Elf(Path(configured_object))
    except SystemExit as error:
        raise SourceFidelityError(str(error)) from error
    for elf in (raw, full):
        if struct.unpack_from(">H", elf.data, 16)[0] != 1:
            raise SourceFidelityError("input is not an ELF relocatable object")
        if struct.unpack_from(">H", elf.data, 18)[0] != 8:
            raise SourceFidelityError("input is not an ELF32 MIPS object")
    raw_symbols, full_symbols = raw.symbols(), full.symbols()
    raw_fn, full_fn = (_function(raw, raw_symbols, function_name),
                       _function(full, full_symbols, function_name))
    _compatible_owner(raw_fn["owner"], full_fn["owner"])
    if raw_fn["size"] != full_fn["size"]:
        raise SourceFidelityError("owned function extent differs")
    generated_summary = None
    raw_generated_owners = full_generated_owners = None
    if generated_rodata:
        generated_summary = generated_storage_fidelity.validate_generated_rodata_pair(
            raw, full, function_name, raw_fn, full_fn, SourceFidelityError)
        raw_rodata_index, _ = generated_storage_fidelity._single_section(
            raw, ".rodata", SourceFidelityError)
        full_rodata_index, _ = generated_storage_fidelity._single_section(
            full, ".rodata", SourceFidelityError)
        _, raw_rodata_symbol = generated_storage_fidelity._section_symbol(
            raw, ".rodata", raw_rodata_index, generated_storage_fidelity.RODATA_SIZE,
            SourceFidelityError)
        _, full_rodata_symbol = generated_storage_fidelity._section_symbol(
            full, ".rodata", full_rodata_index, generated_storage_fidelity.RODATA_SIZE,
            SourceFidelityError)
        raw_descriptor = {
            "name": ".rodata", "type": STT_SECTION, "binding": STB_LOCAL,
            "defined": True, "value": 0,
            "size": generated_storage_fidelity.RODATA_SIZE,
            "storage": (".rodata", SHT_PROGBITS,
                        raw.sh[raw_rodata_index][2]),
            "source_symbol": raw_rodata_symbol,
            "section_index": raw_rodata_index,
        }
        full_descriptor = {
            "name": ".rodata", "type": STT_SECTION, "binding": STB_LOCAL,
            "defined": True, "value": 0,
            "size": generated_storage_fidelity.RODATA_SIZE,
            "storage": (".rodata", SHT_PROGBITS,
                        full.sh[full_rodata_index][2]),
            "source_symbol": full_rodata_symbol,
            "section_index": full_rodata_index,
        }
        raw_generated_owners = {".rodata": raw_descriptor}
        full_generated_owners = {".rodata": full_descriptor}
    raw_symtab, full_symtab = _symtab_index(raw), _symtab_index(full)
    raw_rows = _relocations(raw, raw_symtab, raw_fn, raw_symbols,
                            raw_generated_owners)
    full_rows = _relocations(full, full_symtab, full_fn, full_symbols,
                             full_generated_owners)
    if generated_rodata and len(raw_rows) != 245:
        raise SourceFidelityError("reviewed function relocation census is not 245 records")
    raw_by_key = {(r["site"], r["type"], r["name"]): r for r in raw_rows}
    full_by_key = {(r["site"], r["type"], r["name"]): r for r in full_rows}
    if len(raw_by_key) != len(raw_rows) or len(full_by_key) != len(full_rows):
        raise SourceFidelityError("duplicate named relocation correspondence")
    if raw_by_key.keys() != full_by_key.keys():
        raise SourceFidelityError("relocation site/type/original-name rows differ")
    for key in raw_by_key:
        left, right = raw_by_key[key], full_by_key[key]
        _compatible_owner(left["owner"], right["owner"])
        if left["addend"] != right["addend"]:
            raise SourceFidelityError(
                f"effective owner-relative REL addend differs at +0x{key[0]:X}")
    raw_bytes, full_bytes = _normalized_bytes(raw, raw_fn), _normalized_bytes(full, full_fn)
    if raw_bytes != full_bytes:
        raise SourceFidelityError("normalized owned executable fields differ")
    if generated_rodata:
        raw_exact = raw.section_bytes(raw.names[raw_fn["section"]])[
            raw_fn["start"]:raw_fn["start"] + raw_fn["size"]]
        full_exact = full.section_bytes(full.names[full_fn["section"]])[
            full_fn["start"]:full_fn["start"] + full_fn["size"]]
        if raw_exact != full_exact:
            raise SourceFidelityError("generated-storage proof requires exact owned text bytes")
    report_rows = []
    for key in sorted(raw_by_key):
        row = raw_by_key[key]
        peer = full_by_key[key]
        report_rows.append({
            "site": f"+0x{row['site']:X}", "type": row["type"],
            "source_name": row["name"], "owner_relative_addend": row["addend"],
            "raw_owner": row["owner"], "configured_owner": peer["owner"]})
    return {"schema": "mickey-source-symbol-fidelity-v1",
            "status": "source-correspondence-exact",
            "function": function_name, "owned_size": raw_fn["size"],
            "relocation_count": len(report_rows),
            "relocations": report_rows,
            "normalized_executable_fields_equal": True,
            "identity_route": ("source-pair-correspondence-with-reviewed-generated-rodata"
                               if generated_rodata else
                               "named-source-owner-correspondence-only"),
            "runtime_identity_proved": False,
            "promotion_authority": False,
            **({"generated_storage": generated_summary,
                "c_origin_verified": False} if generated_rodata else {})}


def compare_source_symbols(raw_object: Path, configured_object: Path,
                           function_name: str) -> dict[str, Any]:
    """Default named-owner comparison; SECTION owners remain unsupported."""
    return _compare_source_symbols(raw_object, configured_object, function_name)


def compare_generated_rodata_source_symbols(raw_object: Path,
                                            configured_object: Path,
                                            function_name: str) -> dict[str, Any]:
    """Explicit reviewed baseline route for func_800517E0's generated data.

    This accepts no caller-provided geometry or owner map.  It proves only
    correspondence between the two supplied objects, not their C provenance.
    """
    if function_name != generated_storage_fidelity.FUNCTION:
        raise SourceFidelityError(
            "generated-rodata route is limited to func_800517E0")
    return _compare_source_symbols(raw_object, configured_object,
                                   function_name, generated_rodata=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw-object", required=True)
    parser.add_argument("--configured-object", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--anim-generated-rodata", action="store_true",
                        help="opt in to the fixed func_800517E0 generated-rodata baseline proof")
    args = parser.parse_args(argv)
    try:
        compare = (compare_generated_rodata_source_symbols
                   if args.anim_generated_rodata else compare_source_symbols)
        report = compare(Path(args.raw_object),
                         Path(args.configured_object), args.function)
    except (OSError, ValueError, struct.error, SystemExit) as error:
        report = {"schema": "mickey-source-symbol-fidelity-v1",
                  "status": "unverifiable", "function": args.function,
                  "reason": str(error), "runtime_identity_proved": False,
                  "promotion_authority": False}
        print(json.dumps(report, indent=2))
        return 1
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
