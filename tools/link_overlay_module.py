#!/usr/bin/env python3
"""Link one complete C overlay with the runtime's section-relative addresses.

GNU ld resolves .text, .data, .rodata and .bss at zero independently. Only
references naming a declared runtime export may receive an external zero
binding. References to an export defined in this object use a separate zero
symbol, preserving the function/object definition; private targets link normally.
The original compiler object and ordinary linker result remain separate artifacts.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

import runlink_pack as rp

SECTIONS = (".text", ".data", ".rodata", ".bss")
ABI_METADATA = (".reginfo", ".MIPS.abiflags")
NAME = re.compile(r"[A-Za-z_][A-Za-z0-9_.$]*\Z")
IMPORT_PREFIX = "__runlink_import_"


class LinkError(RuntimeError):
    pass


def run(argv, commands=None):
    argv = [str(x) for x in argv]
    if commands is not None:
        commands.append(argv)
    result = subprocess.run(argv, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode:
        raise LinkError(result.stdout.strip() or f"link command exited {result.returncode}")


def local_provider_section(obj, *, readonly=False):
    """Complete ordinary provider storage; readonly is an explicit profile."""
    names = (".data", ".rodata") if readonly else (".data",)
    present = [name for name in names if obj.size(name)]
    if len(present) != 1:
        raise LinkError("LOCAL data requires one complete provider storage section")
    name = present[0]
    sec = obj.section(name)
    if (struct.unpack_from(">HH", obj.data, 16) != (1, 8)
            or sec["type"] != 1 or sec["addr"] != 0
            or sec["flags"] & 3 != (2 if name == ".rodata" else 3) or sec["flags"] & 4
            or any(obj.size(n) for n in SECTIONS if n != name)
            or any(x["flags"] & 2 and x["size"] and x["name"] not in (name, *ABI_METADATA)
                   for x in obj.sections)
            or any(x.shndx == rp.SHN_COMMON for x in obj.symbols)):
        raise LinkError("LOCAL data requires complete writable data-only inputs or explicit readonly provider")
    return name


def local_aggregate_shape(elf):
    """Actual root span and complete allocation stay distinct and unchanged."""
    owners = [s for s in elf.symbols if s.section == ".data" and s.type == 1]
    allocated = {s["name"] for s in elf.sections if s["flags"] & 2 and s["size"]}
    if (len(owners) != 1 or owners[0].bind != rp.STB_GLOBAL or owners[0].value != 0
            or not 0 < owners[0].size <= elf.size(".data")
            or any(s.name and s.section in allocated and s.type not in (3,4)
                   and s.index != owners[0].index for s in elf.symbols)):
        raise LinkError("LOCAL aggregate lacks one actual complete source root")
    root = owners[0]
    for rel, addend in zip(elf.rels(".data"), rp.in_place_addends(elf, ".data", elf.rels(".data"))):
        if (rel.type != rp.R_MIPS_32 or rel.offset % 4 or not 0 <= rel.offset <= root.size-4
                or rel.sym.shndx != rp.SHN_UNDEF or rel.sym.bind != rp.STB_GLOBAL
                or rel.sym.value or rel.sym.size or addend):
            raise LinkError("LOCAL aggregate reference escapes its actual root span")
    end = root.size
    if end != elf.size(".data"):
        import trim_elf_section as trim
        try:
            if elf.size(".data")-end != 1 or trim.sized_object_end(elf, ".data") != end:
                raise ValueError("not the bounded single-byte symbol/allocation suffix")
        except ValueError as error:
            raise LinkError("LOCAL aggregate compiler end refused: " + str(error)) from error
    return dict(root=root.name, root_symbol_extent=root.size, input_allocation=elf.size(".data"),
                compiler_object_end=end, reviewed_logical_c_extent=elf.size(".data"))


def local_objects_shape(elf):
    """Two genuine adjacent source objects, with only proved section alignment."""
    local_provider_section(elf)
    owners = sorted((symbol for symbol in elf.symbols if symbol.section == ".data"
        and symbol.bind == rp.STB_GLOBAL and symbol.type == 1 and symbol.size > 0),
        key=lambda symbol: symbol.value)
    allocated = {section["name"] for section in elf.sections if section["flags"] & 2 and section["size"]}
    if (len(owners) != 2 or owners[0].value != 0
            or owners[0].value + owners[0].size != owners[1].value
            or owners[0].size % 4
            or any(symbol.section in allocated and symbol.type not in (3, 4)
                   and symbol.index not in {owner.index for owner in owners} for symbol in elf.symbols)):
        raise LinkError("LOCAL objects require two adjacent genuine GLOBAL definitions")
    rels = elf.rels(".data")
    if [rel.offset for rel in rels] != list(range(0, owners[0].size, 4)):
        raise LinkError("LOCAL pointer object lacks its complete original R32 set")
    for rel, addend in zip(rels, rp.in_place_addends(elf, ".data", rels)):
        if (rel.type != rp.R_MIPS_32 or rel.sym.shndx != rp.SHN_UNDEF
                or rel.sym.bind != rp.STB_GLOBAL or rel.sym.type != 1
                or rel.sym.value or rel.sym.size or addend):
            raise LinkError("LOCAL pointer object has unsupported source operands")
    import trim_elf_section as trim
    try:
        end = trim.sized_object_end(elf, ".data")
    except ValueError as error:
        raise LinkError("LOCAL objects compiler end refused: " + str(error)) from error
    if end != owners[1].value + owners[1].size:
        raise LinkError("LOCAL objects do not explain the compiler end")
    return dict(objects=[dict(name=owner.name, offset=owner.value, size=owner.size) for owner in owners],
                compiler_object_end=end, input_allocation=elf.size(".data"))


def local_data_bindings(elf, provider, exports, *, aggregate=False):
    """Names bind to real bases; explicit mixed input preserves scalar bytes."""
    if local_provider_section(elf) != ".data":
        raise LinkError("LOCAL producer must retain writable DATA")
    section = local_provider_section(provider, readonly=aggregate)
    if any(provider.rels(name) for name in SECTIONS):
        raise LinkError("LOCAL data provider contains relocations")
    rels = elf.rels(".data")
    if aggregate:
        local_aggregate_shape(elf)
    elif len(rels)*4 != elf.size(".data") or elf.bytes_of(".data") != bytes(elf.size(".data")):
        raise LinkError("LOCAL data requires a complete zero-addend pointer input")
    bindings = {}
    offsets = []
    for index, rel in enumerate(rels):
        name = rel.sym.name
        if ((not aggregate and rel.offset != index*4) or rel.offset % 4
                or not 0 <= rel.offset <= elf.size(".data")-4
                or rp.u32(elf.bytes_of(".data"),rel.offset) != 0 or rel.type != rp.R_MIPS_32
                or rel.sym.shndx != rp.SHN_UNDEF or rel.sym.bind != rp.STB_GLOBAL
                or name in exports.by_name or not NAME.fullmatch(name)):
            raise LinkError("unsupported LOCAL data import or addend")
        choices = [x for x in provider.symbols if x.name == name
                   and x.bind == rp.STB_GLOBAL and x.type == 1 and x.section == section]
        if (len(choices) != 1 or not 0 <= choices[0].value < provider.size(section)
                or choices[0].value+choices[0].size > provider.size(section)):
            raise LinkError(f"LOCAL data lacks a unique bounded provider base: {name}")
        bindings[name] = choices[0].value
        offsets.append(rel.offset)
    if not offsets or offsets != sorted(set(offsets)):
        raise LinkError("LOCAL data requires ordered unique original R32 references")
    return bindings


POINTER_SCALAR_PROFILE = "pointer-array-three-s32-v1"


def pointer_scalar_shape(elf):
    """Four real complete owners; semantic C roles require the capture witness."""
    local_provider_section(elf)
    owners = sorted((s for s in elf.symbols if s.section == ".data"
                     and s.bind == rp.STB_GLOBAL and s.type == 1 and s.size > 0),
                    key=lambda s: s.value)
    allocated = {s["name"] for s in elf.sections if s["flags"] & 2 and s["size"]}
    if (len(owners) != 4 or len({s.name for s in owners}) != 4
            or any(not s.name for s in owners) or owners[0].value != 0
            or owners[0].size % 4 or any(s.size != 4 for s in owners[1:])
            or any(a.value + a.size != b.value for a, b in zip(owners, owners[1:]))
            or owners[-1].value + owners[-1].size != elf.size(".data")
            or any(s.type not in (3, 4) and s.shndx != rp.SHN_UNDEF
                   and s.index not in {o.index for o in owners} for s in elf.symbols)):
        raise LinkError("pointer/scalar DATA lacks four complete genuine owners")
    rels = elf.rels(".data")
    if [r.offset for r in rels] != list(range(0, owners[0].size, 4)):
        raise LinkError("pointer/scalar DATA lacks its complete original pointer cells")
    for rel, addend in zip(rels, rp.in_place_addends(elf, ".data", rels)):
        if (rel.type != rp.R_MIPS_32 or rel.sym.shndx != rp.SHN_UNDEF
                or rel.sym.bind != rp.STB_GLOBAL or rel.sym.type != 1
                or rel.sym.value or rel.sym.size or addend):
            raise LinkError("pointer/scalar DATA has an unsupported original operand")
    return dict(objects=[dict(name=s.name, offset=s.value, size=s.size, index=s.index)
                         for s in owners], input_allocation=elf.size(".data"))


def grouped_local_bindings(elf, providers, exports, directory, ld, commands, *, mixed=False, objects=False, owner_profile=None):
    """Ordinary whole-input link; mixed storage is an explicit separate case."""
    if len(providers) < 2 or len({p.path for p in providers}) != len(providers):
        raise LinkError("LOCAL group requires distinct ordered providers")
    if owner_profile is not None and (owner_profile != POINTER_SCALAR_PROFILE or mixed or objects):
        raise LinkError("unsupported or conflicting LOCAL owner profile")
    if mixed:
        (local_objects_shape if objects else local_aggregate_shape)(elf)
        local_provider_section(elf)
        sections = [local_provider_section(p, readonly=True) for p in providers]
        if sections != [".rodata", ".data"]:
            raise LinkError("mixed LOCAL group needs one readonly origin and one DATA member")
    else:
        owners = [s for s in elf.symbols if s.section == ".data" and s.bind == rp.STB_GLOBAL
                  and s.type == 1 and s.size]
        if owner_profile:
            pointer_scalar_shape(elf)
        elif len(owners) != 1 or owners[0].value != 0 or owners[0].size != elf.size(".data"):
            raise LinkError("LOCAL group lacks one complete sized aggregate")
        for obj in (elf, *providers):
            local_provider_section(obj)
        sections = [".data"] * len(providers)
    group_section = ".rodata" if mixed else ".data"
    if any(p.rels(n) for p in providers for n in SECTIONS):
        raise LinkError("LOCAL group provider contains relocations")
    script = directory / "providers.ld"
    script.write_text("SECTIONS { " + group_section + " 0 : { " + " ".join(
        json.dumps(str(p.path.resolve())) + "(" + section + ")" for p, section in zip(providers, sections))
        + " } /DISCARD/ : { *(*) } }\n")
    output = directory / "providers.elf"
    run([ld, "-e", "0", "-T", script, "-Map", directory / "providers.map",
         "-o", output, *[p.path.resolve() for p in providers]], commands)
    group = rp.Elf(output)
    if group.bytes_of(group_section) != b"".join(p.bytes_of(section) for p, section in zip(providers, sections)):
        raise LinkError("ordinary LOCAL group added gaps or changed provider payload")
    bindings, offsets = {}, []
    for rel, addend in zip(elf.rels(".data"), rp.in_place_addends(elf, ".data", elf.rels(".data"))):
        if (rel.type != rp.R_MIPS_32 or rel.offset % 4 or not 0 <= rel.offset <= elf.size(".data") - 4
                or rel.sym.shndx != rp.SHN_UNDEF or rel.sym.bind != rp.STB_GLOBAL
                or rel.sym.name in exports.by_name or addend != 0 or not NAME.fullmatch(rel.sym.name)):
            raise LinkError("unsupported grouped LOCAL import or addend")
        offsets.append(rel.offset)
        choices = [s for s in group.symbols if s.name == rel.sym.name and s.bind == rp.STB_GLOBAL
                   and s.type == 1 and s.section == group_section]
        if (len(choices) != 1 or not 0 <= choices[0].value < group.size(group_section)
                or choices[0].value + max(1, choices[0].size) > group.size(group_section)):
            raise LinkError("LOCAL group lacks a unique actual named base")
        actual = []
        base = 0
        for provider, section in zip(providers, sections):
            actual.extend((symbol, base, provider.size(section)) for symbol in provider.symbols
                          if symbol.name == rel.sym.name and symbol.bind == rp.STB_GLOBAL
                          and symbol.type == 1 and symbol.section == section)
            base += provider.size(section)
        if (len(actual) != 1 or not 0 <= actual[0][0].value < actual[0][2]
                or actual[0][0].value + max(1, actual[0][0].size) > actual[0][2]
                or choices[0].value != actual[0][1] + actual[0][0].value
                or choices[0].size != actual[0][0].size):
            raise LinkError("LOCAL group lacks a bounded original provider definition")
        bindings[rel.sym.name] = choices[0].value
    if not offsets or offsets != sorted(set(offsets)):
        raise LinkError("LOCAL group has missing, duplicate or unordered imports")
    return bindings, group


def binding_plan(elf, exports, local_bindings=None):
    """Resolve relocation names to explicit ORT rows, never to guessed addresses."""
    local_bindings = local_bindings or {}
    external = {}
    retarget = {}
    for section in SECTIONS[:-1]:
        for rel in elf.rels(section):
            symbol = rel.sym
            if rel.type not in rp.TYPE_NAMES:
                raise LinkError(f"unsupported relocation type {rel.type} in {section}")
            row = exports.by_name.get(symbol.name)
            if symbol.shndx == rp.SHN_UNDEF:
                if symbol.name in local_bindings:
                    continue
                if row is None or not NAME.fullmatch(symbol.name):
                    raise LinkError(f"undefined reference has no runtime export: {symbol.name}")
                external[symbol.name] = row
            elif symbol.type != rp.STT_SECTION and row is not None:
                if symbol.bind == rp.STB_LOCAL:
                    raise LinkError(f"private symbol is declared as a runtime export: {symbol.name}")
                retarget[(section, rel.offset, rel.type)] = (symbol.name, symbol.index, row)
    return external, retarget


def prepare(raw, path, exports, objcopy, rebind, commands=None, local_bindings=None):
    elf = rp.Elf(raw)
    if struct.unpack_from(">HH", elf.data, 16) != (1, 8):
        raise LinkError("module input must be a relocatable MIPS object")
    unexpected = [s["name"] for s in elf.sections
                  if s["flags"] & 2 and s["size"] and s["name"] not in SECTIONS + ABI_METADATA]
    if unexpected:
        raise LinkError("unsupported allocated sections: " + ", ".join(unexpected))
    external, retarget = binding_plan(elf, exports, local_bindings)
    if {IMPORT_PREFIX + name for name in set(external) | set(local_bindings or {})} & {s.name for s in elf.symbols}:
        raise LinkError("generated import binding collides with an input symbol")
    aliases = {index: f"__runlink_export_{index}" for _name, index, _row in retarget.values()}
    if set(aliases.values()) & {s.name for s in elf.symbols}:
        raise LinkError("generated export binding collides with an input symbol")
    if aliases:
        args = [objcopy]
        for index, alias in sorted(aliases.items()):
            args += ["--add-symbol", alias + "=0,local"]
        run(args + [raw, path], commands)
        for section in SECTIONS[:-1]:
            rows = [(key, value) for key, value in retarget.items() if key[0] == section]
            if rows:
                specs = [f"{key[1]:#x}:{name}:{aliases[index]}"
                         for key, (name, index, _row) in rows]
                run([sys.executable, rebind, path, section, *specs], commands)
    else:
        shutil.copyfile(raw, path)
    prepared = rp.Elf(path)
    for section in SECTIONS:
        if (elf.size(section) != prepared.size(section)
                or elf.bytes_of(section) != prepared.bytes_of(section)):
            raise LinkError(f"binding preparation changed allocated bytes: {section}")
    return external, retarget


def relocatable_container(linked, output):
    """Repackage an already linked zero-based image without editing its sections.

    A final GNU ld link consumes all relocation records. Its section values are
    already offsets because every section starts at zero. The next ROM link
    therefore needs only an ET_REL container, with no executable entry/program
    headers. All allocated bytes and symbol/section tables remain untouched.
    """
    elf = rp.Elf(linked)
    if struct.unpack_from(">HH", elf.data, 16) != (2, 8):
        raise LinkError("expected the ordinary MIPS executable linker product")
    if any(s["type"] in (rp.SHT_REL, rp.SHT_RELA) and s["size"] for s in elf.sections):
        raise LinkError("module linker left unresolved relocation records")
    if any(s["flags"] & 2 and s["addr"] != 0 for s in elf.sections):
        raise LinkError("module sections are not independently zero-based")
    data = bytearray(elf.data)
    struct.pack_into(">H", data, 16, 1)  # ET_REL
    struct.pack_into(">II", data, 24, 0, 0)  # no entry point or program-header offset
    struct.pack_into(">HH", data, 42, 0, 0)  # no program-header entries
    Path(output).write_bytes(data)
    converted = rp.Elf(output)
    if converted.symbols != elf.symbols:
        raise LinkError("container conversion changed linked symbols")
    for section in SECTIONS:
        if (converted.size(section) != elf.size(section)
                or converted.bytes_of(section) != elf.bytes_of(section)):
            raise LinkError("container conversion changed linked section payload")


def link(raw, output, exports, directory, *, ld, objcopy, rebind, local_ro_origin=None, local_ro_group=None, local_data_aggregate=False, local_ro_mixed_group=None, local_data_objects=False, local_ro_owner_profile=None):
    raw, output, directory = Path(raw).resolve(), Path(output).resolve(), Path(directory).resolve()
    original = raw.read_bytes()
    directory.mkdir(parents=True, exist_ok=True)
    preserved = directory / "compiler.o"
    preserved.write_bytes(original)
    prepared = directory / "bindings.o"
    commands = []
    if local_ro_owner_profile is not None and (local_ro_owner_profile != POINTER_SCALAR_PROFILE
            or not local_ro_group or local_ro_origin or local_ro_mixed_group
            or local_data_objects or local_data_aggregate):
        raise LinkError("pointer/scalar profile requires the ordinary writable provider group")
    if local_data_objects and (not local_ro_mixed_group or local_data_aggregate):
        raise LinkError("LOCAL objects require the explicit mixed provider group")
    if sum(bool(x) for x in (local_ro_origin, local_ro_group, local_ro_mixed_group)) > 1:
        raise LinkError("conflicting LOCAL provider profiles")
    if local_data_aggregate and (not local_ro_origin or local_ro_group or local_ro_mixed_group):
        raise LinkError("LOCAL aggregate requires exactly one explicit provider")
    if local_ro_origin and local_ro_group:
        raise LinkError("conflicting LOCAL provider profiles")
    providers = [rp.Elf(Path(p).resolve()) for p in (local_ro_mixed_group or local_ro_group or [])]
    provider = rp.Elf(Path(local_ro_origin).resolve()) if local_ro_origin else None
    local_bindings = local_data_bindings(rp.Elf(preserved), provider, exports, aggregate=local_data_aggregate) if provider else {}
    group = None
    if providers:
        local_bindings, group = grouped_local_bindings(rp.Elf(preserved), providers, exports,
                                                       directory, ld, commands, mixed=bool(local_ro_mixed_group), objects=local_data_objects, owner_profile=local_ro_owner_profile)
    external, retarget = prepare(preserved, prepared, exports, objcopy, rebind, commands,
                                 local_bindings)
    imports = set(external) | set(local_bindings)
    script = directory / "module.ld"
    quoted = json.dumps(str(prepared.resolve()))
    script.write_text("\n".join(f"{name} = {local_bindings.get(name, 0)};" for name in sorted(imports)) + f'''
SECTIONS {{
    .text 0 : AT(0) {{ {quoted}(.text) }}
    .data 0 : AT(SIZEOF(.text)) {{ {quoted}(.data) }}
    .rodata 0 : AT(SIZEOF(.text) + SIZEOF(.data)) {{ {quoted}(.rodata) }}
    .bss 0 (NOLOAD) : {{ {quoted}(.bss) *(COMMON) }}
    /DISCARD/ : {{ *(*) }}
}}
''')
    linked = directory / "module.elf"
    run([ld, "--no-check-sections", "-e", "0", "-T", script,
         "-Map", directory / "module.map", "-o", linked, prepared], commands)
    localized = directory / "localized.elf"
    if imports:
        args = [objcopy]
        for name in sorted(imports):
            # These are linker bindings, not definitions of the imported C
            # functions/objects. Keep their names out of canonical lookups.
            args += ["--localize-symbol", IMPORT_PREFIX + name,
                     "--redefine-sym", name + "=" + IMPORT_PREFIX + name]
        run(args + [linked, localized], commands)
    else:
        shutil.copyfile(linked, localized)
    container = directory / "module.o"
    relocatable_container(localized, container)
    if raw.read_bytes() != original:
        raise LinkError("compiler input changed during module linking")
    if group and group.path.read_bytes() != group.data:
        raise LinkError("ordinary LOCAL group changed during module linking")
    if any(p.path.read_bytes() != p.data for p in providers):
        raise LinkError("LOCAL group provider changed during module linking")
    if provider and provider.path.read_bytes() != provider.data:
        raise LinkError("LOCAL provider changed during module linking")
    shutil.copyfile(container, output)
    result = rp.Elf(output)
    body = b"".join(result.bytes_of(section) for section in SECTIONS[:-1])
    (directory / "body.bin").write_bytes(body)
    receipt = {
        "schema": "mickey-source-module-link-v1",
        "compiler_sha256": hashlib.sha256(original).hexdigest(),
        "linked_sha256": hashlib.sha256(linked.read_bytes()).hexdigest(),
        "configured_sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
        "body_sha256": hashlib.sha256(body).hexdigest(),
        "prepared_sha256": hashlib.sha256(prepared.read_bytes()).hexdigest(),
        "script_sha256": hashlib.sha256(script.read_bytes()).hexdigest(),
        "localized_sha256": hashlib.sha256(localized.read_bytes()).hexdigest(),
        "export_names_sha256": hashlib.sha256(json.dumps(exports.rows).encode()).hexdigest(),
        "commands": commands,
        "container_conversion": "zero-based-exec-to-rel-header-only",
        "sections": {name: {"size": result.size(name),
                            "sha256": hashlib.sha256(result.bytes_of(name)).hexdigest()}
                     for name in SECTIONS},
        "external_exports": external,
        "local_exports": [dict(section=key[0], offset=key[1], type=key[2],
                               name=value[0], row=value[2])
                          for key, value in sorted(retarget.items())],
    }
    if provider:
        receipt["local_data_provider"] = {
            "input": str(provider.path), "input_sha256": hashlib.sha256(provider.data).hexdigest(),
            "runtime_role": ".rodata", "origin": "input-start", "bindings": local_bindings}
        if local_data_aggregate:
            receipt["local_data_provider"]["input_section"] = local_provider_section(provider, readonly=True)
            receipt["local_data_provider"]["aggregate"] = local_aggregate_shape(rp.Elf(preserved))
    if providers:
        receipt["local_data_group"] = {
            "inputs": [{"input": str(p.path), "input_sha256": hashlib.sha256(p.data).hexdigest()}
                       for p in providers], "runtime_role": ".rodata", "origin": "first-input-start",
            "bindings": local_bindings,
            "linked_sha256": hashlib.sha256(group.data).hexdigest(),
            "script_sha256": hashlib.sha256((directory / "providers.ld").read_bytes()).hexdigest()}
    if local_ro_mixed_group:
        if local_data_objects:
            receipt["local_data_group"].update(profile="mixed-storage-objects-v1",
                objects=local_objects_shape(rp.Elf(preserved)))
        else:
            receipt["local_data_group"].update(profile="mixed-storage-v1",
                aggregate=local_aggregate_shape(rp.Elf(preserved)))
        for item, obj in zip(receipt["local_data_group"]["inputs"], providers):
            section = local_provider_section(obj, readonly=True)
            item.update(input_section=section, input_size=obj.size(section))
    if local_ro_owner_profile:
        receipt["local_data_group"].update(profile=local_ro_owner_profile,
            pointer_scalars=pointer_scalar_shape(rp.Elf(preserved)))
    (directory / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("object", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--ort", type=Path, default=Path("config/runlink-ort.us.txt"))
    parser.add_argument("--directory", type=Path, required=True)
    parser.add_argument("--ld", type=Path, default=Path("tools/binutils/mips64-elf-ld"))
    parser.add_argument("--objcopy", type=Path, default=Path("tools/binutils/mips64-elf-objcopy"))
    parser.add_argument("--local-data-aggregate", action="store_true")
    parser.add_argument("--local-data-objects", action="store_true")
    parser.add_argument("--local-ro-owner-profile", choices=[POINTER_SCALAR_PROFILE])
    parser.add_argument("--local-ro-origin-object", type=Path)
    parser.add_argument("--local-ro-provider-group", nargs="+")
    parser.add_argument("--local-ro-mixed-provider-group", nargs="+")
    args = parser.parse_args()
    try:
        receipt = link(args.object, args.output or args.object,
                       rp.OrtNames.load(args.ort), args.directory,
                       ld=args.ld, objcopy=args.objcopy,
                       rebind=Path(__file__).with_name("rebind_elf_relocations.py"),
                       local_ro_origin=args.local_ro_origin_object, local_ro_group=args.local_ro_provider_group, local_ro_owner_profile=args.local_ro_owner_profile,
                       local_data_aggregate=args.local_data_aggregate, local_ro_mixed_group=args.local_ro_mixed_provider_group, local_data_objects=args.local_data_objects)
    except (LinkError, rp.PackError, OSError, ValueError, struct.error) as error:
        parser.exit(1, f"module link refused: {error}\n")
    print(f"source module link: {sum(receipt['sections'][s]['size'] for s in SECTIONS[:-1])} initialized bytes, "
          f"{len(receipt['external_exports'])} external exports, "
          f"{len(receipt['local_exports'])} references to local exports")


if __name__ == "__main__":
    main()
