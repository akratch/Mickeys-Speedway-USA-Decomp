#!/usr/bin/env python3
"""Link one complete C overlay with the runtime's section-relative addresses.

GNU ld resolves .text, .data, .rodata and .bss at zero independently. Only
references naming a declared runtime export may receive an external zero
binding. References to an export defined in this object use a separate zero
symbol, preserving the function/object definition; private targets link normally.
The original compiler object and ordinary linker result remain separate artifacts.
"""
from __future__ import annotations

from proof_artifacts import run_owned

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
_LOADED_MODULE_PATH = Path(__file__).resolve()
_LOADED_MODULE = hashlib.sha256(_LOADED_MODULE_PATH.read_bytes()).hexdigest()


class LinkError(RuntimeError):
    pass


def run(argv, commands=None):
    argv = [str(x) for x in argv]
    if commands is not None:
        commands.append(argv)
    result = run_owned(argv, text=True,
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


CALLBACK_PROFILE = "typed-callback-records-v1"


def callback_implementation_current():
    if (_LOADED_MODULE_PATH != (rp.REPO/"tools/link_overlay_module.py").resolve()
            or Path(__file__).resolve() != _LOADED_MODULE_PATH
            or hashlib.sha256(_LOADED_MODULE_PATH.read_bytes()).hexdigest() != _LOADED_MODULE):
        raise LinkError("loaded callback linker implementation changed")
    return True


def callback_data_shape(elf, facts):
    """Semantic C extent is distinct from unchanged ELF symbol/allocation."""
    callback_implementation_current()
    local_provider_section(elf)
    data = elf.section(".data")
    end = facts["logical_extent"]
    owners = [s for s in elf.symbols if s.shndx != rp.SHN_UNDEF and s.type not in (3, 4)]
    if (len(owners) != 1 or owners[0].name != facts["owner"]
            or owners[0].section != ".data" or owners[0].bind != rp.STB_GLOBAL
            or owners[0].type != 1 or owners[0].value != 0 or not 0 < owners[0].size <= end
            or data["align"] != 16 or not end <= data["size"] < end+16
            or data["size"] % 16 or any(elf.rels(s) for s in (".text", ".rodata"))):
        raise LinkError("callback DATA lacks its unique actual source owner/allocation")
    payload = bytearray(elf.size(".data"))
    for row, mask in enumerate(facts["masks"]):
        struct.pack_into(">H", payload, row*12+8, mask)
    if bytes(payload) != elf.bytes_of(".data"):
        raise LinkError("callback original scalar/NULL/padding bytes differ from source")
    rels = elf.rels(".data")
    expected = facts["callbacks"]
    if len(rels) != len(expected) or not rels:
        raise LinkError("callback DATA lacks its complete source pointer set")
    for rel, addend, field in zip(rels, rp.in_place_addends(elf, ".data", rels), expected):
        if (rel.offset != field["offset"] or rel.offset % 4 or rel.offset+4 > end
                or rel.type != rp.R_MIPS_32 or addend or rel.sym.name != field["name"]
                or rel.sym.shndx != rp.SHN_UNDEF or rel.sym.bind != rp.STB_GLOBAL
                or rel.sym.type != rp.STT_FUNC or rel.sym.value or rel.sym.size):
            raise LinkError("callback source field/original R32 identity differs")
    return dict(owner=owners[0].name, owner_index=owners[0].index,
                reported_symbol_extent=owners[0].size, logical_c_extent=end,
                input_allocation=elf.size(".data"), source_fields=facts)


def callback_effective_input(elf, capture, output):
    callback_implementation_current()
    import proof_provenance as provenance
    if not isinstance(capture, provenance.CallbackCapture) or not capture.current(elf):
        raise LinkError("callback input lacks its internal compiler/layout capture")
    facts = json.loads(capture.facts_json)
    shape = callback_data_shape(elf, facts)
    data = bytearray(elf.data)
    section = elf.section(".data")
    shoff = struct.unpack_from(">I", data, 32)[0]
    struct.pack_into(">I", data, shoff+section["index"]*40+20, shape["logical_c_extent"])
    Path(output).write_bytes(data)
    view = rp.Elf(output)
    if [vars(s) for s in view.symbols] != [vars(s) for s in elf.symbols]:
        raise LinkError("callback effective view changed original symbol metadata")
    view.compiler_original = elf
    view.callback_capture = capture
    view.callback_shape = shape
    sealed_shape = json.dumps(shape,sort_keys=True)
    sealed_surface = provenance._static_parsed_surface(view)
    sealed_bytes = bytes(view.data)
    path = Path(output)
    def current():
        capture.current(elf)
        if (view.compiler_original is not elf or view.callback_capture is not capture
                or bytes(view.data) != sealed_bytes or path.read_bytes() != sealed_bytes
                or provenance._static_parsed_surface(view) != sealed_surface
                or json.dumps(view.callback_shape,sort_keys=True) != sealed_shape):
            raise LinkError("callback prepared view changed after source witnessing")
        return True
    view.callback_view_current = current
    current()
    return view


def callback_text_group(providers, directory, ld, commands):
    """Real FUNCTION-bearing C prefix; no fabricated padding/zero bindings."""
    callback_implementation_current()
    if not providers or len({p.path for p in providers}) != len(providers):
        raise LinkError("callback TEXT prefix needs distinct ordinary source inputs")
    root = Path.cwd().resolve()
    names = []
    for obj in providers:
        try:
            name = obj.path.resolve().relative_to(root).as_posix()
        except ValueError as error:
            raise LinkError("callback input is outside its ordinary source graph") from error
        if not re.fullmatch(r"build/src/overlays/o[0-9]{3}/[A-Za-z_0-9]+\.c\.o", name):
            raise LinkError("callback provider is not an ordinary overlay C input")
        names.append(name)
        section = obj.section(".text")
        debug = obj.section(".mdebug")
        if (struct.unpack_from(">HH", obj.data, 16) != (1, 8)
                or not section or section["type"] != 1 or section["flags"] & 6 != 6
                or section["addr"] or not section["size"] or section["size"] % 4
                or (debug and debug["flags"] & 2)
                or any(s["flags"] & 2 and s["size"] and s["name"] not in SECTIONS+ABI_METADATA for s in obj.sections)):
            raise LinkError("callback provider has unsupported TEXT/allocated sections")
    if len({Path(n).parent for n in names}) != 1:
        raise LinkError("callback TEXT prefix crosses module source namespaces")
    script = directory/"callback-providers.ld"
    script.write_text("SECTIONS { .text 0 : SUBALIGN(4) { "+" ".join(
        json.dumps(str(p.path.resolve()))+"(.text)" for p in providers)
        +" } .data : { *(.data) } .rodata : { *(.rodata) } .bss : { *(.bss) *(COMMON) }"
        +" /DISCARD/ : { *(.mdebug) } }\n")
    output = directory/"callback-providers.elf"
    run([ld, "-r", "-T", script, "-Map", directory/"callback-providers.map",
         "-o", output, *[p.path.resolve() for p in providers]], commands)
    group = rp.Elf(output)
    if group.bytes_of(".text") != b"".join(p.bytes_of(".text") for p in providers):
        raise LinkError("ordinary callback TEXT projection changed input payload")
    base = 0
    original_rows = []
    for obj in providers:
        for symbol in obj.symbols:
            if symbol.section != ".text" or symbol.type != rp.STT_FUNC or not symbol.size:
                continue
            choices = [s for s in group.symbols if s.name == symbol.name and s.bind == symbol.bind
                       and s.type == symbol.type and s.section == ".text"
                       and s.size == symbol.size and s.value == base+symbol.value]
            if (len(choices) != 1 or symbol.value % 4 or symbol.size % 4
                    or not 0 <= symbol.value < symbol.value+symbol.size <= obj.size(".text")):
                raise LinkError("callback FUNCTION definition changed in ordinary projection")
        original_rows.extend((base+r.offset,r.type,r.sym.name,r.sym.type,r.sym.bind,addend)
            for r,addend in zip(obj.rels(".text"),rp.in_place_addends(obj,".text",obj.rels(".text"))))
        base += obj.size(".text")
    projected_rows = [(r.offset,r.type,r.sym.name,r.sym.type,r.sym.bind,addend)
        for r,addend in zip(group.rels(".text"),rp.in_place_addends(group,".text",group.rels(".text")))]
    if projected_rows != original_rows:
        raise LinkError("callback projection changed original ordered TEXT operands")
    group.callback_artifacts = (
        (script, script.read_bytes()),
        (directory/"callback-providers.map", (directory/"callback-providers.map").read_bytes()),
        (output, bytes(group.data)))
    return group


def own_ro_prefix_plan(raw, capture, source, atlas):
    """Join a complete source RO prefix to real compiler roots before projection."""
    import proof_provenance as pp
    import source_module_link_proof as proof
    if hashlib.sha256(Path(__file__).read_bytes()).hexdigest()!=_LOADED_MODULE:
        raise LinkError('loaded own RO normalizer changed')
    if not isinstance(capture, pp.OwnRoCapture):
        raise LinkError('own RO prefix requires its fresh source capture')
    capture.current()
    facts = json.loads(capture.facts_json)
    original = capture.original
    if proof.allocated(raw) != proof.allocated(original):
        raise LinkError('own RO original/prepared allocation differs')
    replay = rp.Elf(capture.directory/'raw.o')
    def symbol_rows(obj):
        return [(s.index,s.shndx,s.name,s.value,s.size,s.bind,s.type,s.section)
                for s in obj.symbols if s.type!=rp.STT_FILE]
    def rels(obj):
        return [(sec,r.offset,r.type,r.sym.index,a)
            for sec in SECTIONS[:-1] for r,a in zip(obj.rels(sec),rp.in_place_addends(obj,sec,obj.rels(sec)))]
    if (proof.allocated(raw)!=proof.allocated(replay) or symbol_rows(raw)!=symbol_rows(replay)
            or rels(raw)!=rels(replay)):
        raise LinkError('own RO configured input differs from fresh ordinary RAW replay')
    sec = raw.section('.rodata')
    if (struct.unpack_from('>HH',raw.data,16)!=(1,8) or raw.size('.data') or raw.size('.bss')
            or not sec or not raw.size('.text') or sec['type']!=1 or sec['addr'] or sec['flags']!=2
            or any(raw.rels(s) for s in ('.data','.rodata'))
            or any(s.shndx==rp.SHN_COMMON for s in raw.symbols)
            or any(s['flags']&2 and s['size'] and s['name'] not in ('.text','.rodata',*ABI_METADATA) for s in raw.sections)):
        raise LinkError('own RO requires complete readonly prefix and no DATA/BSS')
    objects = [s for s in raw.symbols if s.section=='.rodata' and s.type!=rp.STT_SECTION]
    if len(objects)!=len(facts) or any(s.bind!=rp.STB_GLOBAL or s.type!=rp.STT_OBJECT for s in objects):
        raise LinkError('own RO has extra or non-global object definitions')
    cursor=0; owners=[]
    for row in facts:
        choices=[s for s in objects if s.name==row['name']]
        if len(choices)!=1:
            raise LinkError('own RO source root is missing or ambiguous')
        symbol=choices[0]; size=row['count']*4
        payload=b''.join(struct.pack('>i',x) for x in row['values'])
        if symbol.value!=cursor or symbol.size!=size or raw.bytes_of('.rodata')[cursor:cursor+size]!=payload:
            raise LinkError('own RO source payload/extent differs from actual compiler object')
        owners.append((symbol.index,symbol.name,symbol.value,symbol.size));cursor+=size
    if cursor!=sec['size']:
        raise LinkError('own RO source roots do not cover the complete section')
    source=Path(source).as_posix()
    key=source.removeprefix('src/').removesuffix('.c')
    modules=[m for m in atlas['modules'] if any(r.get('source')==key for r in m['text_ownership'])]
    if len(modules)!=1:
        raise LinkError('own RO source graph is missing or ambiguous')
    module=modules[0]; text=module['text_ownership']; storage=module.get('data_rodata_ownership',[])
    if (len(text)!=1 or text[0].get('source')!=key or text[0].get('type')!='c'
            or int(text[0]['offset'],16)!=0 or int(text[0]['size'],16)!=raw.size('.text')
            or int(text[0]['end_offset'],16)!=raw.size('.text')):
        raise LinkError('own RO source graph is not one complete text input')
    own=[r for r in storage if r.get('source')==key]
    if (len(own)!=1 or own[0].get('type')!='c' or own[0].get('section')!='.rodata'
            or own[0].get('externalized') or int(own[0]['offset'],16)!=0
            or int(own[0]['size'],16)!=cursor or int(own[0]['end_offset'],16)!=cursor
            or any(r is not own[0] and int(r['offset'],16)<cursor for r in storage)):
        raise LinkError('own RO graph has an unowned prefix or overlapping owner')
    indices={x[0] for x in owners}; sites=[]
    for section in SECTIONS[:-1]:
        for rel,addend in zip(raw.rels(section),rp.in_place_addends(raw,section,raw.rels(section))):
            if rel.sym.section=='.rodata':
                if section!='.text' or rel.type not in (rp.R_MIPS_HI16,rp.R_MIPS_LO16) or rel.sym.index not in indices:
                    raise LinkError('own RO reference has an unsupported source role')
                if not 0<=addend<rel.sym.size:
                    raise LinkError('own RO reference addend escapes its source object')
                sites.append((section,rel.offset,rel.type,rel.sym.index,rel.sym.name,addend))
    if {s[3] for s in sites}!=indices:
        raise LinkError('own RO source root has no complete reference cohort')
    return dict(module=module['overlay'],source=source,origin=raw.size('.text'),extent=cursor,
                owners=owners,sites=sites)


def project_own_ro_prefix(raw, output, capture, source, atlas, directory, *, objcopy, rebind):
    """Change only reference metadata; ordinary final ld supplies stored operands."""
    import proof_provenance as pp
    import source_module_link_proof as proof
    raw=rp.Elf(raw); directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
    plan=own_ro_prefix_plan(raw,capture,source,atlas)
    observed=pp._static_parsed_surface(raw)
    plan_snapshot=json.dumps(plan,sort_keys=True)
    aliases={idx:f'__runlink_own_ro_{idx}' for idx,_name,_value,_size in plan['owners']}
    if set(aliases.values()) & {s.name for s in raw.symbols}:
        raise LinkError('own RO generated binding collides with an original symbol')
    prepared=directory/'bindings.o';commands=[];args=[objcopy]
    for index,name,value,size in plan['owners']:
        args += ['--add-symbol',f'{aliases[index]}={value},local']
    run(args+[raw.path,prepared],commands)
    for section in SECTIONS[:-1]:
        rows=[x for x in plan['sites'] if x[0]==section]
        if rows:
            run([sys.executable,rebind,prepared,section,*[f'{off:#x}:{name}:{aliases[index]}' for _,off,_kind,index,name,_addend in rows]],commands)
    projected=rp.Elf(prepared)
    if proof.allocated(raw)!=proof.allocated(projected):
        raise LinkError('own RO projection changed original allocated payload')
    expected={(section,off,kind):(index,name) for section,off,kind,index,name,_ in plan['sites']}
    for section in SECTIONS[:-1]:
        old,new=raw.rels(section),projected.rels(section)
        if len(old)!=len(new):
            raise LinkError('own RO projection changed relocation count')
        for a,b in zip(old,new):
            if (a.offset,a.type)!=(b.offset,b.type):
                raise LinkError('own RO projection changed ordered relocation sites')
            hit=expected.get((section,a.offset,a.type))
            if hit:
                if (b.sym.name,b.sym.value,b.sym.bind,b.sym.shndx)!=(aliases[hit[0]],a.sym.value,rp.STB_LOCAL,rp.SHN_ABS):
                    raise LinkError('own RO projected reference disagrees with source definition')
            elif (a.sym.name,a.sym.value,a.sym.size,a.sym.bind,a.sym.type,a.sym.section) != (b.sym.name,b.sym.value,b.sym.size,b.sym.bind,b.sym.type,b.sym.section):
                raise LinkError('own RO projection changed an unrelated relocation identity')
    for symbol in raw.symbols:
        if symbol.type!=rp.STT_FILE:
            choices=[s for s in projected.symbols if (s.name,s.value,s.size,s.bind,s.type,s.section)==(symbol.name,symbol.value,symbol.size,symbol.bind,symbol.type,symbol.section)]
            if len(choices)!=1:
                raise LinkError('own RO projection lost an original symbol identity')
    capture.current()
    if (pp._static_parsed_surface(raw)!=observed or raw.path.read_bytes()!=raw.data
            or json.dumps(plan,sort_keys=True)!=plan_snapshot
            or prepared.read_bytes()!=projected.data):
        raise LinkError('own RO projection observation changed')
    Path(output).write_bytes(projected.data)
    if Path(output).read_bytes()!=projected.data:
        raise LinkError('own RO copied projection differs from its loaded observation')
    return dict(plan=plan,commands=commands,raw_sha256=hashlib.sha256(raw.data).hexdigest(),
                configured_sha256=hashlib.sha256(projected.data).hexdigest())


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


def link(raw, output, exports, directory, *, ld, objcopy, rebind, local_ro_origin=None, local_ro_group=None, local_data_aggregate=False, local_ro_mixed_group=None, local_data_objects=False, local_ro_owner_profile=None, callback_data_profile=None, callback_source=None, callback_text_inputs=None):
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
    callback = None
    source_input = preserved
    if callback_data_profile is not None or callback_source is not None or callback_text_inputs:
        import proof_provenance as provenance
        if (callback_data_profile != CALLBACK_PROFILE or callback_source is None or not callback_text_inputs
                or local_ro_origin or local_ro_group or local_ro_mixed_group or local_data_objects
                or local_data_aggregate or local_ro_owner_profile):
            raise LinkError("conflicting or incomplete callback DATA profile")
        if hashlib.sha256(Path(__file__).read_bytes()).hexdigest() != _LOADED_MODULE:
            raise LinkError("loaded callback linker implementation changed")
        rp.metadata_implementations()
        compiler = rp.Elf(preserved)
        capture = provenance.capture_callback_data(compiler, Path(callback_source), Path.cwd())
        effective = callback_effective_input(compiler, capture, directory/"callback-effective.o")
        callback_providers = [rp.Elf(Path(p).resolve()) for p in callback_text_inputs]
        callback_group = callback_text_group(callback_providers, directory, ld, commands)
        callback_bindings = {}
        for field in effective.callback_shape["source_fields"]["callbacks"]:
            choices = [s for s in callback_group.symbols if s.name == field["name"]
                       and s.bind == rp.STB_GLOBAL and s.type == rp.STT_FUNC and s.section == ".text"
                       and s.size > 0 and s.value % 4 == 0 and s.size % 4 == 0
                       and 0 <= s.value < s.value+s.size <= callback_group.size(".text")]
            if len(choices) != 1:
                raise LinkError("callback lacks a unique real projected FUNCTION")
            if field["name"] not in exports.by_name:
                callback_bindings[field["name"]] = choices[0].value
        local_bindings.update(callback_bindings)
        source_input = effective.path
        callback = (capture, compiler, effective, callback_group, callback_providers)
    external, retarget = prepare(source_input, prepared, exports, objcopy, rebind, commands,
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
    if callback:
        capture, compiler, effective, callback_group, callback_providers = callback
        effective.callback_view_current()
        if (any(path.read_bytes() != observed for path,observed in callback_group.callback_artifacts)
                or any(p.path.read_bytes() != p.data for p in callback_providers)):
            raise LinkError("callback provider changed during linking")
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
    if callback:
        receipt["callback_data"] = dict(profile=CALLBACK_PROFILE, source=str(callback_source),
            shape=effective.callback_shape, capture=str(capture.directory),
            effective_sha256=hashlib.sha256(effective.data).hexdigest(),
            inputs=[dict(input=str(p.path), sha256=hashlib.sha256(p.data).hexdigest(),
                         text_size=p.size(".text")) for p in callback_providers],
            prefix_sha256=hashlib.sha256(callback_group.data).hexdigest(),
            script_sha256=hashlib.sha256(callback_group.callback_artifacts[0][1]).hexdigest(),
            map_sha256=hashlib.sha256(callback_group.callback_artifacts[1][1]).hexdigest(),
            private_bindings=callback_bindings)
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


def own_ro_prefix(raw, output, source, directory):
    """Explicit source-bound normalization; never an arbitrary value/JSON recipe."""
    import proof_provenance as pp
    root = Path.cwd().resolve()
    raw, source, directory = Path(raw).resolve(), Path(source).resolve(), Path(directory).resolve()
    relative = source.relative_to(root)
    if (relative.parts[0] != 'src' or source.suffix != '.c'
            or raw != root/'build'/(relative.as_posix()+'.o')
            or Path(output).resolve() != raw
            or hashlib.sha256(Path(__file__).read_bytes()).hexdigest()!=_LOADED_MODULE):
        raise LinkError('own RO input is not its configured source object')
    rp.metadata_implementations()
    graph_path=root/'config/overlays.us.json'; graph_bytes=graph_path.read_bytes()
    graph=json.loads(graph_bytes)
    directory.mkdir(parents=True,exist_ok=True)
    original=raw.read_bytes(); preserved=directory/'compiler.o';preserved.write_bytes(original)
    capture=pp.capture_own_ro_prefix(source,root)
    result=project_own_ro_prefix(preserved,directory/'projected.o',capture,relative,graph,directory,
        objcopy=root/'tools/binutils/mips64-elf-objcopy',rebind=root/'tools/rebind_elf_relocations.py')
    projected_path=directory/'projected.o'
    expected_projected=result['configured_sha256']
    projected_bytes=projected_path.read_bytes()
    if hashlib.sha256(projected_bytes).hexdigest()!=expected_projected:
        raise LinkError('own RO wrapper projection differs from validated loaded bytes')
    capture.current()
    if (graph_path.read_bytes()!=graph_bytes or raw.read_bytes()!=original
            or hashlib.sha256(Path(__file__).read_bytes()).hexdigest()!=_LOADED_MODULE):
        raise LinkError('own RO inputs changed before publication')
    result['capture']=str(capture.directory)
    result['graph_sha256']=hashlib.sha256(graph_bytes).hexdigest()
    result['source']=relative.as_posix()
    (directory/'receipt.json').write_text(json.dumps(result,indent=2)+'\n')
    temporary=raw.with_suffix(raw.suffix+'.own-ro-tmp')
    temporary.write_bytes(projected_bytes)
    capture.current()
    if (projected_path.read_bytes()!=projected_bytes or temporary.read_bytes()!=projected_bytes
            or raw.read_bytes()!=original or graph_path.read_bytes()!=graph_bytes
            or hashlib.sha256(Path(__file__).read_bytes()).hexdigest()!=_LOADED_MODULE):
        raise LinkError('own RO wrapper publication observation changed')
    temporary.replace(raw)
    if raw.read_bytes()!=projected_bytes:
        raise LinkError('own RO published output differs from validated loaded bytes')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("object", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--own-ro-prefix-source", type=Path)
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
    parser.add_argument("--callback-data-profile", choices=[CALLBACK_PROFILE])
    parser.add_argument("--callback-source", type=Path)
    parser.add_argument("--callback-text-inputs", nargs="+")
    args = parser.parse_args()
    try:
        if args.own_ro_prefix_source is not None:
            if (args.output or args.local_data_aggregate or args.local_data_objects or args.local_ro_owner_profile
                    or args.local_ro_origin_object or args.local_ro_provider_group or args.local_ro_mixed_provider_group
                    or args.callback_data_profile or args.callback_source or args.callback_text_inputs):
                raise LinkError('own RO prefix conflicts with a module/provider mode')
            receipt=own_ro_prefix(args.object,args.object,args.own_ro_prefix_source,args.directory)
            print(f"own RO prefix: {receipt['plan']['extent']} source bytes, {len(receipt['plan']['sites'])} references")
            return
        receipt = link(args.object, args.output or args.object,
                       rp.OrtNames.load(args.ort), args.directory,
                       ld=args.ld, objcopy=args.objcopy,
                       rebind=Path(__file__).with_name("rebind_elf_relocations.py"),
                       local_ro_origin=args.local_ro_origin_object, local_ro_group=args.local_ro_provider_group, local_ro_owner_profile=args.local_ro_owner_profile,
                       local_data_aggregate=args.local_data_aggregate, local_ro_mixed_group=args.local_ro_mixed_provider_group, local_data_objects=args.local_data_objects,
                       callback_data_profile=args.callback_data_profile, callback_source=args.callback_source,
                       callback_text_inputs=args.callback_text_inputs)
    except (LinkError, rp.PackError, OSError, ValueError, struct.error) as error:
        parser.exit(1, f"module link refused: {error}\n")
    print(f"source module link: {sum(receipt['sections'][s]['size'] for s in SECTIONS[:-1])} initialized bytes, "
          f"{len(receipt['external_exports'])} external exports, "
          f"{len(receipt['local_exports'])} references to local exports")


if __name__ == "__main__":
    from proof_artifacts import run_cli
    run_cli(main)
