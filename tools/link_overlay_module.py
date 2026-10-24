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


def local_data_bindings(elf, provider, exports):
    """One complete pointer input names bases in one runtime-RO-origin input."""
    for obj in (elf, provider):
        if struct.unpack_from(">HH", obj.data, 16) != (1, 8):
            raise LinkError("LOCAL data requires relocatable MIPS inputs")
        sec = obj.section(".data")
        if (not sec or not sec["size"] or sec["type"] != 1 or sec["addr"] != 0
                or sec["flags"] & 3 != 3 or sec["flags"] & 4
                or any(obj.size(name) for name in (".text", ".rodata", ".bss"))
                or any(x["flags"] & 2 and x["size"]
                       and x["name"] not in (".data", *ABI_METADATA) for x in obj.sections)
                or any(x.shndx == rp.SHN_COMMON for x in obj.symbols)):
            raise LinkError("LOCAL data requires complete writable data-only inputs")
    if any(provider.rels(name) for name in SECTIONS):
        raise LinkError("LOCAL data provider contains relocations")
    rels = elf.rels(".data")
    if len(rels) * 4 != elf.size(".data") or elf.bytes_of(".data") != bytes(elf.size(".data")):
        raise LinkError("LOCAL data requires a complete zero-addend pointer input")
    bindings = {}
    for index, rel in enumerate(rels):
        name = rel.sym.name
        if (rel.offset != index * 4 or rel.type != rp.R_MIPS_32
                or rel.sym.shndx != rp.SHN_UNDEF or rel.sym.bind != rp.STB_GLOBAL
                or name in exports.by_name or not NAME.fullmatch(name)):
            raise LinkError("unsupported LOCAL data import or addend")
        choices = [x for x in provider.symbols if x.name == name
                   and x.bind == rp.STB_GLOBAL and x.type == 1 and x.section == ".data"]
        if (len(choices) != 1 or not 0 <= choices[0].value < provider.size(".data")
                or choices[0].value + choices[0].size > provider.size(".data")):
            raise LinkError(f"LOCAL data lacks a unique bounded provider base: {name}")
        bindings[name] = choices[0].value
    return bindings


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


def link(raw, output, exports, directory, *, ld, objcopy, rebind, local_ro_origin=None):
    raw, output, directory = Path(raw).resolve(), Path(output).resolve(), Path(directory).resolve()
    original = raw.read_bytes()
    directory.mkdir(parents=True, exist_ok=True)
    preserved = directory / "compiler.o"
    preserved.write_bytes(original)
    prepared = directory / "bindings.o"
    commands = []
    provider = rp.Elf(Path(local_ro_origin).resolve()) if local_ro_origin else None
    local_bindings = local_data_bindings(rp.Elf(preserved), provider, exports) if provider else {}
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
    parser.add_argument("--local-ro-origin-object", type=Path)
    args = parser.parse_args()
    try:
        receipt = link(args.object, args.output or args.object,
                       rp.OrtNames.load(args.ort), args.directory,
                       ld=args.ld, objcopy=args.objcopy,
                       rebind=Path(__file__).with_name("rebind_elf_relocations.py"),
                       local_ro_origin=args.local_ro_origin_object)
    except (LinkError, rp.PackError, OSError, ValueError, struct.error) as error:
        parser.exit(1, f"module link refused: {error}\n")
    print(f"source module link: {sum(receipt['sections'][s]['size'] for s in SECTIONS[:-1])} initialized bytes, "
          f"{len(receipt['external_exports'])} external exports, "
          f"{len(receipt['local_exports'])} references to local exports")


if __name__ == "__main__":
    main()
