"""Executable recipe for the bounded current-ELF JFG example in runlink-jfg.md.

This is deliberately not another general object adapter. Its admitted surface
is the demonstrated zero-based text/data module: resident calls, a single
optional local data object, zero addends, no BSS/rodata/entries/data relocations.
Everything outside the selected module remains an explicit compatibility input.
No extracted source, instruction bytes or target-specific code lives here.
"""

from __future__ import annotations

import hashlib
import json
import re
import struct
import subprocess
import sys
from dataclasses import asdict
from pathlib import Path

from . import codec, rules
from .elf import Elf, SHN_UNDEF, STB_GLOBAL, STT_FUNC, STT_OBJECT, in_place_addends

LOADED = {str(p.resolve()): hashlib.sha256(p.read_bytes()).hexdigest()
          for p in (Path(__file__), Path(__file__).with_name('__main__.py'), Path(codec.__file__),
                    Path(rules.__file__), Path(in_place_addends.__code__.co_filename))}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def unique_pairs(items):
    result = {}
    for name, value in items:
        require(name not in result, "duplicate declaration key: " + name)
        result[name] = value
    return result


def declared_rows(items, count):
    require(isinstance(items, dict), "bindings must map source names to ORT rows")
    require(all(re.fullmatch(r"[A-Za-z_]\w*", name)
                and type(row) is int and 0 <= row < count for name, row in items.items()),
            "invalid binding name/row")
    require(len(set(items.values())) == len(items), "duplicate binding row")
    return items


def generate(cfg, elf_path, declaration_path, out):
    """Generate selected bytes/records and the compatibility catalog, without a link."""
    require(cfg.name == "jfg-us", "this recipe is admitted only for JFG US")
    elf_path, declaration_path, out = map(Path, (elf_path, declaration_path, out))
    require(not out.exists(), "output already exists; preserve it and choose a fresh directory")
    inputs = {str(p.resolve()): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in (elf_path, declaration_path, cfg.rom, cfg.path, cfg.resolve(cfg.build['elf']))}
    inputs.update(LOADED)
    require(all(hashlib.sha256(Path(p).read_bytes()).hexdigest() == h for p, h in inputs.items()),
            "loaded implementation/input changed")
    spec = json.loads(declaration_path.read_text(), object_pairs_hook=unique_pairs)
    require(isinstance(spec, dict) and set(spec) == {"schema", "module", "imports", "exports"}
            and spec["schema"] == "jfg-current-module-example-v1", "invalid recipe declaration")
    number = spec["module"]
    require(type(number) is int and 1 <= number <= cfg.module_count, "invalid existing module")
    imports = declared_rows(spec["imports"], cfg.ort_count)
    exports = declared_rows(spec["exports"], cfg.ort_count)
    require(exports and not (set(imports) & set(exports))
            and not (set(imports.values()) & set(exports.values())), "overlapping/empty bindings")
    rom = codec.read_rom(cfg.rom)
    require(hashlib.sha1(rom).hexdigest() == cfg.rom_sha1, "wrong compatibility ROM")
    outside = codec.decode(rom, cfg, exclude_modules={number}, exclude_rows=set(exports.values()))
    require(not any(row is not None and row.selector == number for row in outside.ort),
            "declarations omit a selected module's existing exported row")
    require(all(outside.names[row] == name for name, row in {**imports, **exports}.items()),
            "binding name differs from the compatibility registry")
    current = Elf(elf_path)
    require(struct.unpack_from('>HH', current.data, 16) == (1, 8), "expected MIPS ET_REL input")
    require(current.size('.bss') == current.size('.rodata') == 0, "BSS/rodata are outside this recipe")
    sections = {s['name']: s for s in current.sections}
    require(len(sections) == len(current.sections), "duplicate ELF sections")
    for section in current.sections:
        if section['flags'] & 2 and section['size']:
            require(section['name'] in ('.text', '.data', '.reginfo', '.MIPS.abiflags'),
                    "unsupported allocated section: " + section['name'])
    for name in ('.text', '.data'):
        section = current.section(name)
        require(section is not None and section['addr'] == 0 and section['type'] == 1
                and section['flags'] == (6 if name == '.text' else 3)
                and section['size'] % 8 == 0, "expected ordinary zero-based aligned " + name)
    require(current.size('.text') > 0 and not current.rels('.data'), "unsupported empty text/data relocations")
    definitions = {}
    for symbol in current.symbols:
        if symbol.type in (STT_FUNC, STT_OBJECT) and symbol.defined and symbol.section in ('.text', '.data'):
            require(symbol.name not in definitions and symbol.size > 0
                    and re.fullmatch(r'[A-Za-z_.$][\w.$]*', symbol.name)
                    and 0 <= symbol.value < symbol.value + symbol.size <= current.size(symbol.section),
                    "ambiguous/unsized/out-of-range definition: " + symbol.name)
            definitions[symbol.name] = symbol
    functions = sorted((s for s in definitions.values() if s.section == '.text'), key=lambda s: s.value)
    end = 0
    for symbol in functions:
        require(symbol.type == STT_FUNC and symbol.value == end, "text is not completely covered by sized functions")
        end += symbol.size
    require(end == current.size('.text'), "unowned text/alignment is outside this recipe")
    require({s.name for s in functions if s.bind == STB_GLOBAL} == set(exports),
            "exports must own every actual global function")
    data_objects = [s for s in definitions.values() if s.section == '.data']
    require((not current.size('.data') and not data_objects)
            or (len(data_objects) == 1 and data_objects[0].type == STT_OBJECT
                and data_objects[0].value == 0 and data_objects[0].size == 4
                and current.size('.data') == 8
                and current.bytes_of('.data')[4:] == bytes(4)),
            "only the demonstrated one-word datum plus ordinary alignment is admitted")
    resident = Elf(cfg.resolve(cfg.build['elf']))
    provider_ids = {}
    for name, row in imports.items():
        matches = [s for s in resident.symbols if s.name == name and s.defined]
        require(len(matches) == 1 and matches[0].type == STT_FUNC
                and matches[0].size > 0 and matches[0].section == '.main',
                "import lacks one sized allocated resident function: " + name)
        identity = ('res', matches[0].value)
        require(rules.row_identity(outside.ort[row], cfg) == identity, "import registry/provider disagreement")
        provider_ids[name] = identity
    rels = current.rels('.text')
    require(len({r.offset for r in rels}) == len(rels), "duplicate relocation site")
    addends = in_place_addends(current.bytes_of('.text'), rels)
    refs = []
    used_imports = set()
    for rel, addend in zip(rels, addends):
        require(rel.offset % 4 == 0 and rel.offset + 4 <= current.size('.text') and addend == 0,
                "unaligned/out-of-range/nonzero-addend reference is outside this recipe")
        if rel.type == codec.R_MIPS_26 and rel.sym.shndx == SHN_UNDEF:
            require(struct.unpack_from('>I', current.bytes_of('.text'), rel.offset)[0] >> 26 == 3,
                    "resident reference is not the demonstrated JAL encoding")
            require(rel.sym.name in imports, "undeclared resident call: " + rel.sym.name)
            identity = provider_ids[rel.sym.name]
            row = imports[rel.sym.name]
            used_imports.add(rel.sym.name)
        else:
            require(rel.type in (codec.R_MIPS_HI16, codec.R_MIPS_LO16)
                    and rel.sym.defined and rel.sym.section == '.data'
                    and rel.sym.value == 0 and data_objects,
                    "unsupported reference; no guessed identity or operand patch is permitted")
            identity, row = ('ov', number, current.size('.text')), None
        refs.append(rules.Ref('.text', rel.offset, rel.type, base=identity, full=identity,
                              row=row, target_section=rel.sym.section, name=rel.sym.name,
                              route='declared-current-ELF'))
    require(used_imports == set(imports), "unused import declaration")
    header = codec.Header(text=current.size('.text'), data=current.size('.data'))
    generated = rules.build_module(number, refs, header,
                                   current.bytes_of('.text') + current.bytes_of('.data'))
    require(not generated.notes and not generated.unresolved, "unresolved/pairing notes")
    module = codec.Module(number, header, generated.image,
                          [g.rec for g in generated.table1], [g.rec for g in generated.table2])
    outside.modules = codec.layout_modules(sorted(outside.modules + [module], key=lambda m: m.number))
    module = next(m for m in outside.modules if m.number == number)
    for name, row in exports.items():
        symbol = definitions[name]
        outside.ort[row] = codec.OrtRow(number, symbol.value)
    headers, _images = codec.encode_modules(outside.modules)
    out.mkdir(parents=True)
    products = {'text.bin': current.bytes_of('.text'), 'data.bin': current.bytes_of('.data'),
                'rt1.bin': b''.join(r.encode() for r in module.table1),
                'rt2.bin': b''.join(r.encode() for r in module.table2),
                'headers.bin': headers, 'ort.bin': codec.encode_ort(outside.ort)}
    for name, payload in products.items():
        (out / name).write_bytes(payload)
    require(all(hashlib.sha256(Path(p).read_bytes()).hexdigest() == h for p, h in inputs.items()),
            "input changed during generation")
    receipt = {'schema': 'jfg-current-module-recipe-v1', 'module': number,
               'header': asdict(module.header), 'inputs': inputs,
               'exports': {name: {'row': row, 'offset': definitions[name].value,
                                  'size': definitions[name].size} for name, row in exports.items()},
               'definitions': {name: {'section': s.section, 'offset': s.value, 'size': s.size,
                                       'global': s.bind == STB_GLOBAL, 'function': s.type == STT_FUNC}
                               for name, s in definitions.items()},
               'relocations': [{'site': r.site, 'type': r.type, 'name': r.name,
                                'identity': r.full} for r in refs],
               'products': {name: hashlib.sha256(payload).hexdigest() for name, payload in products.items()},
               'outside_authority': 'unselected modules/records/headers, registry names/rows, resident homes',
               'source_only': False, 'general_adapter': False,
               'support': 'resident calls and optional one-word local datum; no BSS/rodata/entries/nonzero addends'}
    (out / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    return receipt


def link_rom(cfg, out, receipt, cross='tools/binutils/mips64-elf-', expect_retail=False):
    """Ordinary final link for the documented pinned JFG script, never execution."""
    from dataclasses import replace
    from . import compare, mapfile
    out = Path(out).resolve()
    require(not re.search(r'[\s"\\]', str(out)), "bounded linker recipe needs a simple output path")
    pin = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=cfg.root, text=True).strip()
    require(pin == 'efd5abb1c79636e297b831f7c2d5bf47eac39c0c', "final-link recipe requires the documented JFG pin")
    holds = dict(receipt['inputs'])
    for name, expected in receipt['products'].items():
        require(hashlib.sha256((out/name).read_bytes()).hexdigest() == expected, "generated product changed")
        holds[str(out/name)] = expected
    tools = {name: str(cfg.resolve(cross)) + name for name in ('as', 'ld', 'objcopy')}
    commands = []

    def hold(path):
        path = Path(path).resolve()
        value = hashlib.sha256(path.read_bytes()).hexdigest()
        require(str(path) not in holds or holds[str(path)] == value, "changed input: " + str(path))
        holds[str(path)] = value

    def run(label, args):
        args = [str(a) for a in args]
        commands.append(args)
        (out/'commands.json').write_text(json.dumps(commands, indent=2)+'\n')
        with (out/(label+'.log')).open('w') as log:
            result = subprocess.run(args, cwd=cfg.root, stdout=log, stderr=subprocess.STDOUT)
        require(result.returncode == 0, label + ' failed; see private command log')

    wrap = []
    for section, filename in (('.text', 'text.bin'), ('.data', 'data.bin')):
        wrap.append(section)
        cursor = 0
        symbols = sorted(((name, s) for name, s in receipt['definitions'].items()
                          if s['section'] == section), key=lambda item: item[1]['offset'])
        for name, symbol in symbols:
            start, size = symbol['offset'], symbol['size']
            require(start >= cursor, "overlapping definitions cannot be wrapped")
            if start > cursor:
                wrap.append(f'.incbin "{out/filename}", {cursor}, {start-cursor}')
            wrap += [('.globl ' if symbol['global'] else '.local ') + name,
                     '.type ' + name + (',@function' if symbol['function'] else ',@object'), name+':',
                     f'.incbin "{out/filename}", {start}, {size}', f'.size {name}, .-{name}']
            cursor = start + size
        remaining = (out/filename).stat().st_size - cursor
        if remaining:
            wrap.append(f'.incbin "{out/filename}", {cursor}, {remaining}')
    for name in ('rt1', 'rt2', 'headers', 'ort'):
        wrap += [f'.section .recipe_{name},"a"', f'.incbin "{out/(name+".bin")}"']
    (out/'packed.s').write_text('\n'.join(wrap)+'\n')
    for path in (*tools.values(), cfg.root/'tools/CopyRom.py', cfg.root/'tools/n64crc'):
        hold(path)
    hold(compare.__file__)
    hold(mapfile.__file__)
    hold(out/'packed.s')
    run('assemble-wrapper', [tools['as'], '-march=vr4300', '-32', '-G0', out/'packed.s', '-o', out/'packed.o'])
    hold(out/'packed.o')
    original_script = cfg.root/'ver/jfg.us.ld'
    hold(original_script)
    script = original_script.read_text()
    number, packed = receipt['module'], out/'packed.o'
    replacements = {f'build/src/overlays/o{number}/overlay_{number}.c.o({section})': f'{packed}({section})'
                    for section in ('.text', '.data', '.rodata', '.bss')}
    replacements.update({f'build/assets/overlays/o{number}/overlay_{number}_reloc.bin.o(.data)':
                         f'{packed}(.recipe_rt1);\n        {packed}(.recipe_rt2)',
                         'build/assets/overlayTable.bin.o(.data)':f'{packed}(.recipe_headers)',
                         'build/assets/overlayRomTable.bin.o(.data)':f'{packed}(.recipe_ort)'})
    for old, new in replacements.items():
        require(script.count(old) == 1, "unsupported/ambiguous script selector: " + old)
        script = script.replace(old, new)
    padding = cfg.root/'build/assets/padding.bin.o'
    hold(padding)
    original = codec.read_rom(cfg.rom)
    old_elf = Elf(cfg.resolve(cfg.build['elf']))
    old_symbols = {s.name:s for s in old_elf.symbols if s.defined or s.absolute}
    require('padding_ROM_START' in old_symbols, "missing actual terminal-fill boundary")
    padding_bytes = Elf(padding).bytes_of('.data')
    require(padding_bytes == original[old_symbols['padding_ROM_START'].value:],
            "compatibility padding object differs from the original tail")
    prefix = padding_bytes.rstrip(b'\xff')
    require(prefix == bytes(len(prefix)), "unsupported nonzero terminal-fill prefix")
    cartridge_size = cfg.rom.stat().st_size
    old = 'build/assets/padding.bin.o(.data);'
    require(script.count(old) == 1, "terminal fill selector is ambiguous")
    script = script.replace(old, f'ASSERT({cartridge_size} - padding_ROM_START > {len(prefix)}, "tail overflow");\n'
        f'        . += {len(prefix)};\n        FILL(0xFFFFFFFF);\n        BYTE(0xFF);\n'
        f'        . += {cartridge_size} - padding_ROM_START - {len(prefix)+1};')
    for name in set(re.findall(r'(build/[^\s();]+\.o)\(', script)):
        hold(cfg.root/name)
    scripts = [cfg.root/'ver/symbols'/name for name in ('undefined_syms.txt', 'undefined_funcs_auto.us.txt',
        'undefined_syms_auto.us.txt', 'libultra_undefined_syms.us.txt', 'undefined_syms.us.txt')]
    for path in scripts:
        hold(path)
    (out/'final.ld').write_text(script)
    hold(out/'final.ld')
    command = [tools['ld'], '-T', out/'final.ld']
    for path in scripts:
        command += ['-T', path]
    run('final-link', command + ['-Map', out/'final.map', '-o', out/'final.elf'])
    run('objcopy', [tools['objcopy'], '-O', 'binary', f'--pad-to={cartridge_size}', '--gap-fill=0xFF',
                    out/'final.elf', out/'final.bin'])
    run('copy-rom', [sys.executable, 'tools/CopyRom.py', out/'final.bin', out/'final.z64'])
    run('crc', ['tools/n64crc', out/'final.z64'])
    before, result = (out/'final.bin').read_bytes(), (out/'final.z64').read_bytes()
    require(len(before) == len(result) == len(original), "cartridge extent changed")
    require(before[:16] == result[:16] and before[24:] == result[24:],
            "post-objcopy mutation outside checksum header")
    final = Elf(out/'final.elf')
    symbols = {s.name:s for s in final.symbols if s.defined or s.absolute}
    boundaries = ('symbolsTable_offsets_ROM_START', 'symbolsTable_offsets_ROM_END',
                  'symbolsTable_symbol_names_ROM_START', 'symbolsTable_symbol_names_ROM_END')
    require(all(name in symbols and name in old_symbols for name in boundaries),
            "missing actual linker name-table boundary")
    allowed_prefix = set(range(16, 24))
    hold(cfg.resolve(cfg.build['map']))
    _, inputs = mapfile.parse(cfg.resolve(cfg.build['map']))
    boundary_sites = []
    for item in inputs:
        if item.outsec != '.main' or item.insec != '.text':
            continue
        path = cfg.resolve(item.file)
        hold(path)
        raw = Elf(path)
        rels = [r for r in raw.rels(item.insec) if r.sym.name in boundaries]
        for rel, addend in zip(rels, in_place_addends(raw.bytes_of(item.insec), rels)):
            if rel.type != codec.R_MIPS_LO16:
                continue
            require(addend == 0, "unsupported boundary addend")
            site = item.addr + rel.offset - cfg.resident_rom_delta
            old_word, new_word = (struct.unpack_from('>I', data, site)[0] for data in (original, result))
            require(old_word >> 16 == new_word >> 16
                    and old_word & 0xFFFF == old_symbols[rel.sym.name].value & 0xFFFF
                    and new_word & 0xFFFF == symbols[rel.sym.name].value & 0xFFFF,
                    "resident boundary reference exceeds the demonstrated LO16-only relayout")
            allowed_prefix.update(range(site, site+4))
            if old_word != new_word:
                boundary_sites.append(site)
    cursor = 0
    for limit in sorted(allowed_prefix | {cfg.main_table_rom}):
        require(result[cursor:limit] == original[cursor:limit],
                "resident/prefix mutation outside actual boundary relocations")
        cursor = limit + 1
    dynamic = replace(cfg, name_offsets_rom=symbols[boundaries[0]].value,
                      names_rom=symbols[boundaries[2]].value, names_end=symbols[boundaries[3]].value)
    decoded = codec.decode(result, dynamic)
    selected = decoded.module(number)
    require(asdict(selected.header) == receipt['header'], "linked header differs from current source")
    require(selected.image == (out/'text.bin').read_bytes() + (out/'data.bin').read_bytes(), "linked source bytes differ")
    require(b''.join(r.encode() for r in selected.table1) == (out/'rt1.bin').read_bytes()
            and b''.join(r.encode() for r in selected.table2) == (out/'rt2.bin').read_bytes(),
            "linked ordered runtime records differ")
    require({key:value['full'] for key, value in compare.shipped_identities(selected, decoded, dynamic).items()}
            == {(r['site'], r['type']):tuple(r['identity']) for r in receipt['relocations']},
            "final runtime identities differ from actual current ELF references")
    require(result[cfg.headers_rom:cfg.headers_rom+32*cfg.module_count] == (out/'headers.bin').read_bytes()
            and result[cfg.ort_rom:cfg.ort_rom+4*cfg.ort_count] == (out/'ort.bin').read_bytes(), "linked tables differ")
    old_catalog = codec.decode(original, cfg)
    require(decoded.main == old_catalog.main and decoded.names == old_catalog.names, "resident records/names changed")
    require(result[cfg.main_table_rom:cfg.ort_rom] == original[cfg.main_table_rom:cfg.ort_rom],
            "resident table block/padding changed")
    for module in decoded.modules:
        if module.number == number:
            continue
        old_module = old_catalog.module(module.number)
        fields = ('vram', 'text', 'data', 'bss', 'rel1', 'rel2', 'init', 'resume')
        require(module.image == old_module.image and module.table1 == old_module.table1
                and module.table2 == old_module.table2
                and all(getattr(module.header, key) == getattr(old_module.header, key) for key in fields),
                "unselected module changed: " + str(module.number))
    require(result[dynamic.name_offsets_rom:dynamic.name_offsets_rom+(cfg.names_rom-cfg.name_offsets_rom)]
            == original[cfg.name_offsets_rom:cfg.names_rom]
            and result[dynamic.names_rom:dynamic.names_end] == original[cfg.names_rom:cfg.names_end], "name payload changed")
    old_end = cfg.modules_rom + max(m.header.rom + m.header.image_size for m in old_catalog.modules)
    new_end = cfg.modules_rom + max(m.header.rom + m.header.image_size for m in decoded.modules)
    require(result[new_end:dynamic.name_offsets_rom] == original[old_end:cfg.name_offsets_rom]
            and result[dynamic.names_end:symbols['padding_ROM_START'].value]
            == original[cfg.names_end:old_symbols['padding_ROM_START'].value], "compatibility boundary gap changed")
    tail = symbols['padding_ROM_START'].value
    require(result[tail:tail+len(prefix)] == prefix
            and result[tail+len(prefix):] == b'\xff' * (cartridge_size-tail-len(prefix)),
            "terminal compatibility prefix/fill differs")
    for name, export in receipt['exports'].items():
        require((symbols[name].section, symbols[name].value, symbols[name].size)
                == (f'.overlay_{number}', export['offset'], export['size']), "genuine export definition changed")
    if expect_retail:
        require(result == original, "baseline final ROM is not byte-identical")
    require(all(hashlib.sha256(Path(p).read_bytes()).hexdigest() == h for p, h in holds.items()),
            "held input changed during final link")
    report = {'schema':'jfg-current-module-link-v1', 'module':number,
              'rom_sha1':hashlib.sha1(result).hexdigest(), 'retail_byte_exact':result == original,
              'resident_boundary_changed_words':len(boundary_sites),
              'unselected_modules_exact':cfg.module_count-1, 'runtime_identities_exact':True,
              'post_objcopy':'checksum header only', 'inputs':holds,
              'terminal_zero_prefix':len(prefix), 'compatibility_gaps_exact':True,
              'outside_authority':receipt['outside_authority'], 'target_execution':False}
    (out/'link-receipt.json').write_text(json.dumps(report, indent=2)+'\n')
    return report
