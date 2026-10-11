#!/usr/bin/env python3
"""Authenticate a single resident anonymous float literal for search admission.

Only a direct LUI/LWC1 load is supported. Source literal, access, canonical
named owner, linker mapping and ROM independently constrain the correspondence.
This is not a promotion proof and never searches sections for matching bytes.
"""
from __future__ import annotations
import hashlib
import re
import struct
from pathlib import Path
import search_local_bindings as local
import reloc_surface as rs


def need(value, reason):
    if not value:
        raise RuntimeError('local literal proof: ' + reason)


def source_literal(ast, symbol):
    functions = [n for n in ast.ext if type(n).__name__ == 'FuncDef' and n.decl.name == symbol]
    need(len(functions) == 1, 'source function is not unique')
    values = [n for n in local.walk(functions[0].body)
              if type(n).__name__ == 'Constant' and n.type in ('float', 'double')]
    # One distinct nonzero explicit single-precision literal, possibly reused.
    # Zero can be synthesized without a pool; no other type/value is accepted.
    import math
    payloads = set()
    for node in values:
        if node.type != 'float' or not node.value.lower().endswith('f'):
            raise local.UnsupportedLocalBinding('single-float-source-literal')
        spelling = node.value[:-1]
        need(re.fullmatch(r'(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?', spelling),
             'unsupported float literal spelling')
        value = float(spelling)
        need(math.isfinite(value), 'nonfinite literal')
        if value != 0:
            payloads.add(struct.pack('>f', value).hex())
    if len(payloads) != 1:
        raise local.UnsupportedLocalBinding('single-float-source-literal')
    return {'width': 4, 'bits': next(iter(payloads))}


def load_pair(code, hi, lo):
    h, l = local.fields(code.words[hi]), local.fields(code.words[lo])
    need(h[0] == 15 and h[1] == 0 and l[0] == 49, 'literal is not a direct LUI/LWC1 load')
    need(h[2] == l[1] and code.definitions(lo, l[1]) == {hi}, 'literal base has ambiguous reaching definitions')
    return {'width': 4, 'sites': [[hi, 5], [lo, 6]],
            'fields': [code.words[hi] & 0xffff0000, code.words[lo] & 0xffff0000]}


def data_extent(elf, section, start, width):
    index, header = local.unique_payload(elf, section)
    need(header[2] & 2 and not header[2] & 5, 'literal storage is not allocated read-only data')
    data = elf.section_bytes(section)
    need(width == 4 and start >= 0 and start % width == 0 and start + width <= len(data), 'literal extent or width invalid')
    # Even a relocation that starts before the requested bytes can overlap.
    for name, offset, _, _ in elf.relocations(re.escape(section)):
        if name == section:
            need(not (offset < start + width and start < offset + 4), 'literal extent contains relocation-bearing data')
    return data[start:start + width]


def candidate_literal(path, symbol, shape):
    elf = rs.Elf(path)
    code = local.Code(elf, symbol)
    rodata, _ = local.unique_payload(elf, '.rodata')
    sites = {site for site, (_, index) in code.relocs.items() if code.symbols[index][4] == rodata}
    pairs = [p for p in local.pairs(code) if code.symbols[p[2]][4] == rodata]
    need(len(pairs) == 1, 'anonymous literal pair is missing or ambiguous')
    hi, lo, index, addend = pairs[0]
    need(sites == {hi, lo}, 'literal pair does not cover all local sites')
    _, value, _, info, _ = code.symbols[index]
    need(info & 15 == 3 and value == 0, 'literal requires zero-based section-symbol owner')
    access = load_pair(code, hi, lo)
    raw = data_extent(elf, '.rodata', addend, access['width'])
    need(raw.hex() == shape['bits'] and access['width'] == shape['width'], 'source literal differs from actual load payload')
    # Named objects over this range would supply a distinct identity, not an
    # anonymous compiler constant. Reject such overlap rather than merge it.
    need(not any(s[4] == rodata and s[3] & 15 == 1 and s[2] > 0
                 and s[1] < addend + 4 and addend < s[1] + s[2] for s in code.symbols),
         'literal overlaps a named object')
    return dict(access, shape=shape, object_sha256=local.digest(path), literal_offset=addend)


def target_literal(root, symbol, source, candidate, shape):
    import yaml
    root = Path(root)
    linked_path = root / 'build/mickey.us.elf'
    object_path = rs.resolve_resident_target_object(candidate, source)
    linked, obj = rs.Elf(linked_path), rs.Elf(object_path)
    code = local.Code(obj, symbol)
    value, size, section = rs._unique_symbol(linked, symbol)
    need(section == '.main', 'literal target is outside resident namespace')
    local.unique_payload(linked, section)
    rs._resident_target_records(candidate, source, linked, symbol, value, size, section, rs.LINK_SYMS)
    # Select by FP access and unique canonical named owner, never payload scan.
    possible = []
    for hi, lo, index, addend in local.pairs(code):
        if local.fields(code.words[lo])[0] != 49:
            continue
        name, _, _, _, shndx = code.symbols[index]
        need(addend == 0 and (shndx == 0 or obj.names[shndx] == '.rodata'), 'target FP literal lacks a direct named owner')
        access = load_pair(code, hi, lo)
        owners = [s for s in linked.symbols() if s[0] == name]
        need(len(owners) == 1, 'target literal owner is ambiguous')
        owner = owners[0]
        need(owner[2] >= 4 and owner[3] & 15 == 1 and owner[4] == linked.section(section)[0],
             'target literal owner extent/type/namespace differs')
        possible.append((owner, access))
    need(len(possible) == 1, 'target FP literal access is missing or ambiguous')
    owner, access = possible[0]
    name, address, owner_extent, _, _ = owner
    extent = access['width']
    map_path = root / 'build/mickey.us.map'
    owners = []
    for line in map_path.read_text().splitlines():
        match = re.fullmatch(r'\s+\.rodata\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+(build/\S+\.o)\s*', line)
        if match:
            base, length = int(match[1], 16), int(match[2], 16)
            if base <= address and address + extent <= base + length:
                owners.append((base, root / match[3]))
    need(len(owners) == 1, 'target literal linker owner is ambiguous')
    base, owner_path = owners[0]
    owned = rs.Elf(owner_path)
    ri, _ = local.unique_payload(owned, '.rodata')
    symbols = [s for s in owned.symbols() if s[0] == name]
    need(len(symbols) == 1 and symbols[0][1] + base == address and symbols[0][2] == owner_extent
         and symbols[0][3] & 15 == 1 and symbols[0][4] == ri, 'canonical literal owner disagrees with link map')
    payload = data_extent(owned, '.rodata', symbols[0][1], extent)
    yaml_path = root / 'mickey.us.yaml'
    layouts = [s for s in yaml.safe_load(yaml_path.read_text())['segments']
               if isinstance(s, dict) and '.' + s.get('name', '') == section]
    need(len(layouts) == 1 and layouts[0].get('type') == 'code', 'literal has no unique YAML namespace')
    layout = layouts[0]
    rom_start = address - layout['vram'] + layout['start']
    subs = [s for s in layout['subsegments'] if isinstance(s, list) and len(s) >= 2 and type(s[0]) is int]
    spans = [(s, subs[i + 1][0]) for i, s in enumerate(subs[:-1])
             if s[0] <= rom_start and rom_start + extent <= subs[i + 1][0] and s[1] in ('rodata', '.rodata')]
    need(len(spans) == 1, 'literal lacks unique YAML rodata ownership')
    sub, _ = spans[0]
    expected = ('build/src/' + sub[2] + '.c.o') if sub[1] == '.rodata' else ('build/asm/data/' + (sub[2] if len(sub) > 2 else format(sub[0], 'X')) + '.rodata.s.o')
    need(owner_path.relative_to(root).as_posix() == expected, 'YAML and linker literal owners disagree')
    offset = address - linked.section(section)[1][3]
    linked_payload = linked.section_bytes(section)[offset:offset + extent]
    rom_path = root / 'baseroms/mickey.us.z64'
    need(payload == linked_payload == rom_path.read_bytes()[rom_start:rom_start + extent]
         and payload.hex() == shape['bits'], 'literal owner/link/ROM/source payload differs')
    return {'identity': [0, address - rs.ot.RESIDENT_VRAM_BASE], 'name': name, 'access': access,
            'authority': {str(p.relative_to(root)): local.digest(p) for p in (linked_path, object_path, owner_path, map_path, yaml_path, rom_path)}}


def authenticate(root, symbol, source, candidate, shape, overlay=None):
    need(overlay is None and not source.startswith('overlays/'), 'literal crosses overlay namespace')
    current = candidate_literal(candidate, symbol, shape)
    target = target_literal(root, symbol, source, candidate, shape)
    need(local.digest(candidate) == current['object_sha256'], 'candidate changed during proof')
    need(all(local.digest(Path(root) / p) == h for p, h in target['authority'].items()), 'literal authority changed during proof')
    return {'schema': 'mickey-resident-float-literal-v1', 'candidate': current, 'target': target,
            'records': [[site, kind, target['identity']] for site, kind in current['sites']]}


def fidelity(left, right):
    for key in ('sites', 'fields', 'width', 'shape'):
        need(left[key] == right[key], 'raw/full-TU literal fidelity differs: ' + key)


def source_ast(source, parser_context):
    import sys
    import candidate_context
    # Carried macro definitions must be proved inactive, not blindly stripped.
    text, _, _ = candidate_context.inactive_seed_prelude(source)
    sys.path.insert(0, str(Path(__file__).resolve().parent / 'permuter'))
    from src import ast_util
    with parser_context():
        return ast_util.parse_c(text, from_import=True)


def fidelity_proofs(left, right):
    need(left.get('schema') == right.get('schema'), 'local binding proof routes differ')
    schema = left.get('schema')
    if schema == 'mickey-resident-float-literal-v1':
        fidelity(left['candidate'], right['candidate'])
    elif schema == 'mickey-resident-switch-table-v1':
        local.fidelity(left['candidate'], right['candidate'])
    else:
        raise RuntimeError('unsupported local binding proof schema')


def prove_source_binding(root, item, path, source, out_dir, inputs, parser_context):
    import sweep_receipts
    full_source = sweep_receipts.owned_bytes(out_dir, 'source-groups.preprocessed.c')
    expected = inputs['context'].get('original_preprocessed')
    need(expected and hashlib.sha256(full_source).hexdigest() == expected,
         'original preprocessor authority is missing or stale')
    emitted_ast, full_ast = (source_ast(s, parser_context) for s in (source, full_source))
    helper = local
    try:
        shape = local.source_switch(emitted_ast, item.func)
        full_shape = local.source_switch(full_ast, item.func)
    except local.UnsupportedLocalBinding as error:
        if error.route != 'single-switch-source-owner':
            raise
        shape = source_literal(emitted_ast, item.func)
        full_shape = source_literal(full_ast, item.func)
        helper = None
    need(shape == full_shape, 'prepared/full-TU source shape differs')
    authenticate_binding = authenticate if helper is None else helper.authenticate
    return authenticate_binding(root, item.func,
        item.rel_c_file.removeprefix('src/').removesuffix('.c'), path, shape, item.overlay)
