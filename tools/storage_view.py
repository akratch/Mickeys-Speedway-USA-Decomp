#!/usr/bin/env python3
"""Independently reproduce a named reserved-storage use; report only.

This does not accept candidate aliases, caller-supplied objects or target sites.
It neither changes relocation resolution nor supplies promotion acceptance.
Raw receipts and reports belong under ignored build/.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shlex
import shutil
import struct
import sys
import tempfile
import time
from pathlib import Path

import function_preflight as fp
import overlay_tables as ot
import permute_batch as batch
import proof_provenance as pp
import reloc_surface as rs
import sweep_receipts as receipts

ROOT = Path(__file__).resolve().parent.parent
LOADED_PROOFS = {Path(module.__file__): pp.sha256_file(Path(module.__file__))
                 for module in (fp, ot, batch, pp, rs, receipts)}
LOADED_PROOFS[Path(__file__)] = pp.sha256_file(Path(__file__))
# Reviewed preprocessed ResolveRelocAddress body. A semantic change requires
# reviewing the loader rule again, never silently updating this digest.
LOADER_BODY = "84294dfe85ab136afe936f8b20e06272af1c6a3ea367b8f419987e93def25620"


class ViewError(RuntimeError):
    pass


def require(condition, reason):
    if not condition:
        raise ViewError(reason)


def function_body(text, symbol):
    text = re.sub(r'^\s*#.*$', '', pp._mask_c(text), flags=re.M)
    matches = list(re.finditer(r'\b' + re.escape(symbol) + r'\s*\([^;{}]*\)\s*\{', text))
    require(len(matches) == 1, 'function body unavailable or ambiguous')
    start = text.index('{', matches[0].start())
    depth = 0
    for end in range(start, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return re.sub(r'\s+', '', text[start:end + 1])
    raise ViewError('unbalanced function body')


def body_digest(text, symbol):
    return hashlib.sha256(function_body(text, symbol).encode()).hexdigest()


def capture(resolution, directory):
    """Fresh configured compilation + full metadata replay, with input pins.

    Reuses the existing recipe parser, runner, dependency closure, tool identity,
    replay and allocated-section/symbol/relocation fidelity checks. Filters and
    externalization are deliberately outside this report's current scope.
    """
    source = resolution.source
    configured = resolution.candidate_object
    source_rel = source.relative_to(ROOT).as_posix()
    target = configured.relative_to(ROOT).as_posix()
    deadline = time.monotonic() + 120

    def recipe():
        lines = batch.bounded_capture(['gmake', '--no-print-directory', '-n', '-W', source_rel, target],
                                      deadline, check=True).stdout.replace('\\\n', ' ').splitlines()
        compilers = [x.strip() for x in lines if target in x and 'tools/ido/cc' in x]
        metadata = [x.strip() for x in lines if target in x and 'tools/ido/cc' not in x
                    and ('objcopy' in x or 'trim_elf_section.py' in x)]
        require(len(compilers) == 1 and len(metadata) <= 1, 'ambiguous configured recipe')
        return compilers[0], metadata[0] if metadata else ''

    command, postprocess = recipe()
    words = shlex.split(command)
    require(words.count('-o') == 1, 'ambiguous compiler output')
    args = batch.compiler_arguments(command, source_rel, target)
    wrapped = len(words) > 1 and words[1] == 'tools/asm-processor/build.py'
    dependency_args = args + (('-I', str(source.parent)) if wrapped else ())
    plan = pp.metadata_filter_plan(postprocess, target, ROOT) if postprocess else []
    require(all(op in {'rename', 'add-symbol', 'rebind', 'remove-section', 'trim'} for op, _ in plan),
            'storage report does not support this metadata operation')

    def context():
        batch.checked_tool_identity()
        require(all(pp.sha256_file(path) == digest for path, digest in LOADED_PROOFS.items()),
                'loaded storage-proof implementation changed')
        require(recipe() == (command, postprocess), 'configured recipe changed')
        require(pp.sha256_file(Path(pp.__file__)) == pp._LOADED_METADATA_PROOF,
                'loaded provenance implementation changed')
        dependencies = batch.source_dependencies(source, dependency_args, deadline)
        require(not any(x.startswith('missing:') for x in dependencies), 'missing compiler dependency')
        assembly = {}
        for pragma in pp._all_pragmas(source.read_text()):
            path = ROOT / pragma.path
            for directive, name in re.findall(r'^\s*\.(include|incbin)\s+"([^"]+)"', path.read_text(), re.M):
                require(directive == 'include' and (ROOT / 'include' / name).is_file(),
                        'unsupported nested assembly dependency')
            assembly[pragma.path] = pp.sha256_file(path)
        return {'source': pp.sha256_file(source), 'configured': pp.sha256_file(configured),
                'linked': pp.sha256_file(fp.TARGET_ELF), 'command': command, 'postprocess': postprocess,
                'dependencies': dependencies, 'assembly': assembly,
                'source_directory': receipts.tree_digest(source.parent),
                'include_tree': receipts.tree_digest(ROOT / 'include'),
                'tools': batch.sweep_tool_identity(),
                'asm_processor': receipts.tree_digest(ROOT / 'tools/asm-processor'),
                'makefiles': {p.relative_to(ROOT).as_posix(): pp.sha256_file(p)
                              for p in [ROOT / 'Makefile', *sorted((ROOT / 'mk').glob('**/*.mk'))]},
                'filter_specs': pp.filter_spec_files(postprocess, ROOT),
                'target_inputs': {p: pp.sha256_file(ROOT / p) for p in (
                    'baseroms/mickey.us.z64', 'config/overlays.us.json', 'overlay_undefined_syms.us.txt',
                    'symbol_addrs.us.txt', 'build/mickey.us.map')},
                'proof_tools': {p: pp.sha256_file(ROOT / p) for p in (
                    'tools/storage_view.py', 'tools/function_preflight.py', 'tools/proof_provenance.py',
                    'tools/reloc_surface.py', 'tools/overlay_tables.py', 'tools/reloc_identity.py',
                    'tools/rebind_elf_relocations.py', 'tools/trim_elf_section.py',
                    'tools/binutils/mips64-elf-objcopy')}}

    before = context()
    raw = directory / 'raw.o'
    words[words.index('-o') + 1] = str(raw)
    result = batch.bounded_capture(words, deadline, check=True)
    (directory / 'compile.log').write_text(result.stdout)
    require(before == context(), 'inputs changed during compilation')
    replay = directory / 'replayed.o'
    if postprocess:
        pp.replay_metadata(raw, postprocess, target, replay, ROOT,
                           skip_filters=False, deadline=deadline, plan=plan)
    else:
        shutil.copyfile(raw, replay)
    rs._reserved_witness_fidelity(rs.Elf(replay), rs.Elf(configured))
    unchanged_instruction_bits(rs.Elf(raw), rs.Elf(configured))
    # Retain actual IDO-preprocessed source for reviewed loader semantics.
    cpp = batch.bounded_capture(['tools/ido/cc', *[a for a in args if a != '-c'], '-E', source_rel],
                                deadline, check=True).stdout
    (directory / 'preprocessed.c').write_text(cpp)
    require(before == context(), 'inputs changed during replay/preprocessing')
    receipt = {'schema': 'mickey-storage-view-capture-v1', 'inputs': before,
               'raw_object': raw.relative_to(ROOT).as_posix(), 'raw_sha256': pp.sha256_file(raw),
               'replayed_sha256': pp.sha256_file(replay),
               'preprocessed_sha256': hashlib.sha256(cpp.encode()).hexdigest(), 'metadata_plan': plan}
    (directory / 'capture.json').write_text(json.dumps(receipt, indent=2) + '\n')
    return rs.Elf(raw), receipt, context, cpp


def unchanged_instruction_bits(raw, configured):
    """Metadata may relocate fields and remove zero alignment, never opcodes."""
    a, b = raw.section_bytes('.text'), configured.section_bytes('.text')
    require(len(a) >= len(b) and len(b) % 4 == 0 and not any(a[len(b):]),
            'metadata changed executable extent')
    masks = {}
    for _, offset, kind, _ in raw.relocations():
        mask = {2: 0, 4: 0xFC000000, 5: 0xFFFF0000, 6: 0xFFFF0000}.get(kind)
        require(mask is not None and offset % 4 == 0 and 0 <= offset < len(b)
                and offset not in masks, 'unsupported raw executable relocation geometry')
        masks[offset] = mask
    for offset in range(0, len(b), 4):
        original = struct.unpack_from('>I', a, offset)[0]
        replayed = struct.unpack_from('>I', b, offset)[0]
        require(not ((original ^ replayed) & masks.get(offset, 0xFFFFFFFF)),
                'metadata changed compiler instruction bits')


def named_sites(elf, name):
    symbols = elf.symbols()
    matches = [x for x in symbols if x[0] == name]
    require(len(matches) == 1 and matches[0][4] == rs.SHN_UNDEF, 'witness must be one actual compiler external')
    return [(offset, kind) for section, offset, kind, index in elf.relocations()
            if symbols[index][0] == name]


def unchanged_external(raw, configured, name):
    sites = named_sites(raw, name)
    require(sites and sites == named_sites(configured, name), 'named external changed through metadata')
    a, b = raw.section_bytes('.text'), configured.section_bytes('.text')
    for offset, kind in sites:
        require(offset % 4 == 0 and 0 <= offset <= min(len(a), len(b)) - 4,
                'named storage relocation outside text')
        require(kind in (rs.R_MIPS_HI16, rs.R_MIPS_LO16, rs.R_MIPS_32), 'unsupported named storage relocation')
        require(a[offset:offset + 4] == b[offset:offset + 4], 'named storage use changed through metadata')
    return sites


def indexed_halfword_use(elf, sites, owned_end=None):
    """Recognize only a small straight-line address-to-load/call witness.

    Stops at control flow (after an ordinary direct call's delay slot), any
    unsupported operation, or another relocation. It proves one observed use,
    never a complete footprint, index range or absence of writes elsewhere.
    """
    text = elf.section_bytes('.text')
    relocations = {off: (kind, elf.symbols()[idx][0]) for _, off, kind, idx in elf.relocations()}
    output = []
    end = len(text) if owned_end is None else owned_end
    require(0 <= end <= len(text) and end % 4 == 0, 'invalid owned use boundary')
    for hi, kind in sites:
        if (kind != rs.R_MIPS_HI16 or (hi + 4, rs.R_MIPS_LO16) not in sites
                or hi < 0 or hi + 8 > end):
            continue
        h, lo = struct.unpack_from('>II', text, hi)
        base = (h >> 16) & 31
        if (h >> 26 != 15 or lo >> 26 != 9 or (lo >> 21) & 31 != base
                or base == 0 or ((lo >> 16) & 31) == 0
                or h & 0xffff or lo & 0xffff):
            continue
        base = (lo >> 16) & 31
        known = {base: ('base', None)}
        pending_call = None
        for pc in range(hi + 8, min(hi + 48, end), 4):
            word = struct.unpack_from('>I', text, pc)[0]
            op, a, b, d = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
            sh, fn = (word >> 6) & 31, word & 63
            if pc in relocations and op != 3:
                break
            if op == 3 and relocations.get(pc, (None,))[0] == rs.R_MIPS_26 and pending_call is None:
                known.pop(31, None)  # JAL writes RA before its delay slot.
                pending_call = (pc, relocations[pc][1])
            elif op == 0 and fn == 0 and a == 0:
                is_index = b not in known and b != 0
                known.pop(d, None)
                if is_index:
                    known[d] = ('index', sh)
            elif op == 0 and fn == 33:
                left, right = known.get(a), known.get(b)
                if left and right and {left[0], right[0]} == {'base', 'index'}:
                    known[d] = ('address', left[1] if left[0] == 'index' else right[1])
                else:
                    known.pop(d, None)
            elif op == 0 and fn == 37 and (a == 0 or b == 0):
                source = known.get(b if a == 0 else a)
                known.pop(d, None)
                if source:
                    known[d] = source
            elif op in (33, 37):  # signed/unsigned halfword load
                address = known.get(a)
                if address and address[0] == 'address':
                    row = {'load_offset': pc, 'width': 2, 'signed': op == 33,
                           'index_scale': 1 << address[1], 'displacement': rs.sext16(word & 0xffff),
                           'destination_register': b, 'complete_footprint': False,
                           'index_bounds_proved': False}
                    if pending_call and pc == pending_call[0] + 4:
                        row.update(call_offset=pending_call[0], call_external=pending_call[1],
                                   argument_register=b if 4 <= b <= 7 else None)
                    output.append(row)
                known.pop(b, None)
            elif word != 0:
                break
            known.pop(0, None)
            if pending_call and pc == pending_call[0] + 4:
                break
    return output


def unique_symbol(elf, name):
    symbols = [s for s in elf.symbols() if s[0] == name and s[4] != rs.SHN_UNDEF]
    require(len(symbols) == 1, 'missing or ambiguous named definition: ' + name)
    return symbols[0]


def owned_external_sites(raw, configured, name, function, expected_size):
    all_sites = unchanged_external(raw, configured, name)
    a, b = unique_symbol(raw, function), unique_symbol(configured, function)
    for elf, symbol in ((raw, a), (configured, b)):
        require(0 < symbol[4] < len(elf.names) and elf.names[symbol[4]] == '.text'
                and symbol[3] & 15 == rs.STT_FUNC and symbol[2] == expected_size
                and 0 <= symbol[1] <= len(elf.section_bytes('.text')) - symbol[2],
                'selected function compiler boundary unavailable')
    require(a[1:3] == b[1:3], 'selected function boundary changed through metadata')
    start, size = a[1:3]
    sites = [(offset, kind) for offset, kind in all_sites if start <= offset < start + size]
    require(sites, 'external is not used by selected function')
    return sites, start + size


def reject_name_conflicts(linked, name, identity, overlay):
    """Do not replace a real named definition with a synthetic-name witness."""
    for other, value, _, _, section in linked.symbols():
        if other != name or section in (rs.SHN_UNDEF, rs.SHN_ABS):
            continue
        require(0 < section < len(linked.names), 'invalid named definition section')
        section_name = linked.names[section]
        if section_name == '.overlay_%03d' % overlay:
            defined = (overlay, value - rs.SYNTHETIC_VMA)
        elif section_name == '.overlay_%03d_bss' % overlay:
            defined = (overlay, value - rs.SYNTHETIC_VMA)
        elif section_name in ('.main', '.main_bss'):
            defined = (0, value - ot.RESIDENT_VRAM_BASE)
        else:
            raise ViewError('external conflicts with another named storage owner')
        require(defined == identity, 'external conflicts with actual named definition')


def resident_owner(elf, rom, name):
    symbol = unique_symbol(elf, name)
    _, address, size, info, section = symbol
    require(0 < section < len(elf.names) and elf.names[section] == '.main' and size > 0
            and info & 15 == 1, 'resident owner must be a sized initialized object')
    matches = [(obj, sec, start, extent) for (obj, sec), (start, extent) in rs.linked_input_sections().items()
               if start <= address and address + size <= start + extent]
    require(len(matches) == 1, 'resident input owner unavailable or ambiguous')
    obj, sec, start, extent = matches[0]
    path = ROOT / obj
    require(path.resolve().is_relative_to((ROOT / 'build').resolve()), 'unsafe resident owner path')
    owner = rs.Elf(path)
    definition = unique_symbol(owner, name)
    _, offset, own_size, own_info, own_section = definition
    require(0 < own_section < len(owner.names) and owner.names[own_section] == sec and offset == address - start and own_size == size
            and own_info & 15 == 1, 'resident input symbol contradicts named linked owner')
    _, header = owner.section(sec)
    require(header[1] != 8 and header[5] == extent, 'unsupported or contradictory resident section')
    require(not any(offset <= off < offset + size for _, off, _, _ in owner.relocations(re.escape(sec))),
            'resident data view contains unresolved input relocations')
    value = owner.section_bytes(sec)[offset:offset + size]
    linked_offset = address - elf.sh[section][3]
    linked = elf.section_bytes(elf.names[section])[linked_offset:linked_offset + size]
    rom_offset = address - ot.VRAM_ROM_DELTA
    require(len(value) == size and value == linked == rom[rom_offset:rom_offset + size],
            'named resident owner does not reproduce retail bytes')
    return {'symbol': name, 'address': address, 'size': size, 'input_object': obj,
            'input_section': sec, 'section_alignment': header[8], 'object_sha256': pp.sha256_file(path),
            'bytes_sha256': hashlib.sha256(value).hexdigest()}


def loader_view(identity, owner, linked, cpp):
    require(body_digest(cpp, 'ResolveRelocAddress') == LOADER_BODY, 'reviewed loader semantics changed')
    selector, offset = identity
    require(selector in (0xffd, 0xffe), 'only initialized reserved-data views supported')
    anchor = unique_symbol(linked, 'D_80078D60')
    resident = unique_symbol(linked, 'func_80000450')
    require(resident[1] == ot.RESIDENT_VRAM_BASE, 'resident loader base changed')
    require(0 <= offset < 0x100000, 'reserved export offset out of range')
    require(anchor[1] + offset == owner['address'], 'reserved view does not start at named resident owner')
    return {'resident_base_requirement': 'resident module loaded at canonical resident base',
            'selector': selector, 'offset': offset, 'anchor': anchor[0], 'anchor_address': anchor[1],
            'physical_address': owner['address'], 'resident_owner': owner['symbol'],
            'namespace_preserved': True, 'loader_body_sha256': LOADER_BODY}


def exact_report(symbol, directory):
    resolution = fp.resolve(symbol)
    require(resolution.resolution_mode == 'post_promotion', 'source function is not canonical exact C')
    fp.require_fresh_evidence(resolution)
    original = resolution.source.read_text()
    facts = pp.source_facts(original, resolution.candidate_symbol)
    require(len(facts.definitions) == 1 and not facts.pragmas, 'witness function is not ordinary C')
    report = fp.collect(resolution, no_build=True)
    require(report['preflight']['status'] == 'complete' and report['workbench']['differing_words'] == 0,
            'source owner lacks complete linked/runtime proof')
    directory.mkdir()
    raw, receipt, current, cpp = capture(resolution, directory)
    return resolution, report, raw, receipt, current, cpp


def collect(function, external, owner_name=None, *, freshness_checks=None):
    parent = ROOT / 'build/storage-views'
    parent.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix='proof-', dir=parent))
    resolution, report, raw, capture_receipt, current, _ = exact_report(function, directory / 'witness')
    require(report['context']['kind'] == 'overlay', 'external witness must be an overlay function')
    configured = rs.Elf(resolution.candidate_object)
    sites, owned_end = owned_external_sites(raw, configured, external, resolution.candidate_symbol,
                                          report['owned_size'])
    linked, rom = rs.Elf(fp.TARGET_ELF), fp.ROM.read_bytes()
    atlas = json.loads(fp.ATLAS.read_text())
    overlay = report['context']['overlay']
    module = next(m for m in atlas['modules'] if m['overlay'] == overlay)
    rows = [r for r in module['text_ownership'] if r['source'] == resolution.translation_unit.removeprefix('src/').removesuffix('.c')]
    require(len(rows) == 1, 'witness owner unavailable')
    narrowed = dict(module, text_ownership=rows)
    witnesses = rs._matched_overlay_relocation_witnesses(narrowed, linked, rom,
                    ot.build_modules(ot.read_headers(rom))[overlay - 1], {external})
    identities = witnesses.get(external, set())
    require(len(identities) == 1, 'independent external identity missing or conflicting')
    identity = next(iter(identities))
    other_witnesses = rs._matched_overlay_relocation_witnesses(module, linked, rom,
                       ot.build_modules(ot.read_headers(rom))[overlay - 1], {external})
    require(other_witnesses.get(external) == {identity}, 'independent named witnesses conflict')
    reject_name_conflicts(linked, external, identity, overlay)
    # An independently exact consumer can establish the address used by an
    # actual source name. It does not thereby establish object size/type.
    require(identity[0] == overlay or identity[0] in rs.RESERVED_SELECTORS,
            'only local overlay or reserved storage uses are supported')
    if identity[0] == overlay:
        runtime_module = ot.build_modules(ot.read_headers(rom))[overlay - 1]
        require(runtime_module['text_size'] <= identity[1] <
                runtime_module['text_size'] + runtime_module['data_size'] + runtime_module['bss_size'],
                'local witness is not inside runtime storage')
    result = {'schema': 'mickey-storage-view-v1', 'status': 'independent-storage-use-proved',
              'promotion_acceptance': False, 'source_function': function, 'source_overlay': overlay,
              'external': external, 'identity': list(identity), 'external_relocations': len(sites),
              'owned_storage': None, 'reserved_view': None,
              'observed_uses': indexed_halfword_use(raw, sites, owned_end),
              'type_and_subobject_status': 'access facts only; no inferred C effective type, alias, or index bound',
              'source_preflight': report, 'captures': [capture_receipt]}
    if owner_name is not None:
        owner = resident_owner(linked, rom, owner_name)
        _, loader_report, _, loader_receipt, loader_current, loader_cpp = exact_report('ResolveRelocAddress', directory / 'loader')
        view = loader_view(identity, owner, linked, loader_cpp)
        require(pp.sha256_file(ROOT / owner['input_object']) == owner['object_sha256'],
                'resident data owner changed during proof')
        require(loader_receipt['inputs'] == loader_current(), 'loader evidence changed during view proof')
        result.update(status='independent-address-view-proved', owned_storage=owner,
                      reserved_view=view, loader_preflight=loader_report)
        result['captures'].append(loader_receipt)
    require(capture_receipt['inputs'] == current(), 'evidence changed during view proof')
    if freshness_checks is not None:
        freshness_checks.append(lambda: require(capture_receipt['inputs'] == current(),
                                               'storage witness changed after proof'))
        if owner_name is not None:
            freshness_checks.append(lambda: require(loader_receipt['inputs'] == loader_current(),
                                                   'loader witness changed after proof'))
    (directory / 'report.json').write_text(json.dumps(result, indent=2) + '\n')
    result['saved_report'] = (directory / 'report.json').relative_to(ROOT).as_posix()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-function', required=True)
    parser.add_argument('--external', required=True)
    parser.add_argument('--resident-owner', help='also prove a full named resident DATA1/DATA2 view')
    args = parser.parse_args()
    try:
        print(json.dumps(collect(args.source_function, args.external, args.resident_owner), indent=2))
        return 0
    except (ViewError, fp.PreflightError, pp.MetadataProofError, rs.SurfaceComparisonError, OSError, RuntimeError) as error:
        print('storage_view: ' + str(error), file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
