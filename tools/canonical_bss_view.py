#!/usr/bin/env python3
"""Fresh typed C BSS authority for reserved-loader identities.

This deliberately starts with one reviewed, naturally laid-out owner.  It does
not infer ownership from a target relocation or an address-shaped name.  The
caller still owns candidate compilation fidelity and merging conflicting
identity evidence. Receipts and compiler artifacts remain under ignored build/.
"""
from __future__ import annotations

import hashlib
import json
import re
import tempfile
from pathlib import Path
from types import SimpleNamespace

import proof_provenance as pp
import reloc_surface as rs
import storage_view as sv

ROOT = Path(__file__).resolve().parent.parent
OWNER_SOURCE = 'src/main/fx_texture_buffers.c'
OWNER_OBJECT = 'build/src/main/fx_texture_buffers.c.o'
OWNER_INPUT = 'main/fx_texture_buffers'
OWNER_SOURCE_SHA256 = 'd1a47c1d86d6882d83ce42e8ad2aaa52ca9aaaa0de3e863301f4a17f6ef9081f'
OWNER_NAMES = ('D_800D60B0', 'D_800D60C0')
EXPECTED_DECLARATIONS = 'void*D_800D60B0[4];void*D_800D60C0[4];'
# The reviewed loader differs from storage_view's older body only by three
# func_80000450 -> amSetMuteMode identifier replacements. The actual anchor
# address and fresh exact loader proof remain mandatory below.
LOADER_BODY = '683dce5439d7a0894178ca45479a32a400ac092a109bddc029a0a72e7c4d2af7'
LOADED = {Path(m.__file__): pp.sha256_file(Path(m.__file__))
          for m in (pp, rs, sv)}
LOADED[Path(__file__)] = pp.sha256_file(Path(__file__))


class BssViewError(RuntimeError):
    pass


def need(condition, message):
    if not condition:
        raise BssViewError(message)


def clean(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)


def owner_declarations(text):
    need(re.sub(r'\s+', '', clean(text)) == EXPECTED_DECLARATIONS,
         'owner is not the reviewed two natural pointer-array definitions')


def candidate_declaration(text, name):
    """Compatibility only; the caller must bind this source to its object."""
    text = clean(text)
    pattern = r'\bextern\s+void\s*\*\s*' + re.escape(name) + r'\s*\[\s*4\s*\]\s*;'
    matches = list(re.finditer(pattern, text))
    need(len(matches) == 1, 'candidate lacks one exact void-pointer array extern')
    # A definition, shadow declaration, macro, or typedef using this owner is
    # not accepted by this narrow adapter. Ordinary expression uses are fine.
    remainder = text[:matches[0].start()] + text[matches[0].end():]
    need(not re.search(r'\b(?:void|char|short|int|long|float|double|s32|u32|u8)\s*\**\s*'
                       + re.escape(name) + r'\b', remainder), 'candidate shadows owner declaration')
    need(not re.search(r'^\s*#\s*define\s+' + re.escape(name) + r'\b', text, re.M),
         'candidate owner is a macro')


def unique(elf, name):
    rows = [row for row in elf.symbols() if row[0] == name]
    need(len(rows) == 1, 'owner symbol absent or ambiguous: ' + name)
    return rows[0]


def candidate_carrier(elf, name, aliases):
    rows = [row for row in elf.symbols() if aliases.get(row[0], row[0]) == name]
    need(len(rows) == 1, 'candidate owner carrier absent or ambiguous')
    return rows[0]


def bss_layout(elf):
    need(elf.names.count('.bss') == 1, 'owner requires one BSS section')
    index = elf.names.index('.bss')
    h = elf.sh[index]
    need(h[1] == 8 and h[2] & 3 == 3 and h[5] == 32 and h[8] >= 4,
         'owner BSS must be writable NOBITS, exactly 32 bytes')
    for pos, name in enumerate(OWNER_NAMES):
        row = unique(elf, name)
        need(row[1] == pos * 16 and row[2] == 16 and row[3] == 0x11 and row[4] == index,
             'owner pointer-array symbol extent/type/placement differs')
    extras = [r for r in elf.symbols() if r[4] == index and r[3] & 15 == 1
              and r[0] not in OWNER_NAMES and r[2]]
    need(not extras, 'overlapping or additional BSS object')
    need(not any(section == '.bss' for section, *_ in elf.relocations()),
         'NOBITS owner has relocations')


def loader_identity(name, linked, start, loader_cpp):
    need(sv.body_digest(loader_cpp, 'ResolveRelocAddress') == LOADER_BODY,
         'reviewed loader semantics changed')
    anchor = unique(linked, 'D_80085A40')
    resident = unique(linked, 'amSetMuteMode')
    need(resident[1] == sv.ot.RESIDENT_VRAM_BASE, 'resident load base changed')
    need(anchor[1] == 0x80085A40, 'resident BSS anchor changed')
    row = unique(linked, name)
    need(0 < row[4] < len(linked.names) and linked.names[row[4]] == '.main_bss'
         and row[2] == 16 and row[3] == 0x11, 'linked owner is not a typed BSS definition')
    section = linked.sh[row[4]]
    need(section[1] == 8 and section[3] <= row[1]
         and row[1] + row[2] <= section[3] + section[5],
         'linked object escapes its NOBITS section')
    expected = start + OWNER_NAMES.index(name) * 16
    need(row[1] == expected, 'linked owner disagrees with input placement')
    offset = row[1] - anchor[1]
    need(0 <= offset < 0x100000, 'reserved BSS offset outside loader domain')
    return (0xFFF, offset)


def collect(candidate_elf, names, target_elf, *, root=ROOT, candidate_source=None,
            candidate_aliases=None):
    """Return ``identities``, JSON ``receipt`` and a terminal ``recheck``.

    Names is a unique sequence of reviewed owner names, or a mapping from each
    name to the requested selector (only integer 0xFFF). No requested offset is
    accepted: the actual canonical definition and loader derive it.
    ``candidate_source`` must be the caller's already compilation-bound source;
    this helper validates its declaration compatibility, not that compilation.
    All captures are invocation-local; no persistent receipt is imported.
    """
    root = Path(root).resolve()
    need(root == ROOT.resolve() and root == sv.ROOT.resolve(), 'proof must run in its own checkout')
    requested = list(names)
    aliases = dict(candidate_aliases or {})
    need(requested and len(requested) == len(set(requested))
         and set(requested) <= set(OWNER_NAMES), 'unsupported or duplicate owner request')
    if isinstance(names, dict):
        need(all(type(v) is int and v == 0xFFF for v in names.values()),
             'only the reserved BSS namespace is supported')
    need(candidate_source is not None, 'candidate source compilation binding is required')
    candidate_source = Path(candidate_source).resolve()
    candidate_path = Path(candidate_elf.path).resolve()
    target_path = Path(target_elf.path).resolve()
    need(target_path == sv.fp.TARGET_ELF.resolve(), 'target must be canonical linked ELF')
    source, configured = root / OWNER_SOURCE, root / OWNER_OBJECT
    yaml = root / 'mickey.us.yaml'
    before = {str(p): pp.sha256_file(p) for p in
              (source, configured, yaml, candidate_source, candidate_path, target_path)}
    need(before[str(source)] == OWNER_SOURCE_SHA256, 'reviewed owner source pin changed')
    need(hashlib.sha256(candidate_elf.data).hexdigest() == before[str(candidate_path)]
         and hashlib.sha256(target_elf.data).hexdigest() == before[str(target_path)],
         'caller supplied stale ELF objects')
    need(all(pp.sha256_file(p) == h for p, h in LOADED.items()), 'loaded proof changed')
    owner_declarations(source.read_text())
    for name in requested:
        candidate_declaration(candidate_source.read_text(), name)
        row = candidate_carrier(candidate_elf, name, aliases)
        need(row[4] == rs.SHN_UNDEF and row[3] >> 4 == 1 and row[3] & 15 in (0, 1)
             and row[2] in (0, 16), 'candidate is not a unique undefined data carrier')
    directory = root / 'build/canonical-bss-view'
    directory.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='owner-', dir=directory))
    owner_dir = out / 'owner'; owner_dir.mkdir()
    raw, owner_receipt, owner_current, owner_cpp = sv.capture(
        SimpleNamespace(source=source, candidate_object=configured), owner_dir)
    # Preprocessing must not replace the reviewed declarations.
    owner_declarations(re.sub(r'^\s*#.*$', '', owner_cpp, flags=re.M))
    need(not owner_receipt['metadata_plan'], 'BSS owner must be untouched stock C')
    bss_layout(raw); bss_layout(rs.Elf(configured))
    _, loader_proof, _, loader_receipt, loader_current, loader_cpp = sv.exact_report(
        'ResolveRelocAddress', out / 'loader')
    placements = rs.linked_input_sections(root / 'build/mickey.us.map')
    need((OWNER_OBJECT, '.bss') in placements, 'owner absent from canonical input map')
    start, size = placements[(OWNER_OBJECT, '.bss')]
    need(size == 32 and start % 4 == 0, 'mapped BSS owner extent differs')
    map_rows = re.findall(r'^\s+\.bss\s+0x[0-9a-fA-F]+\s+0x[0-9a-fA-F]+\s+'
                          + re.escape(OWNER_OBJECT) + r'\s*$',
                          (root / 'build/mickey.us.map').read_text(), re.M)
    need(len(map_rows) == 1, 'duplicate or missing owner map placement')
    yaml_pattern = (r'^\s*-\s*\{\s*type:\s*\.bss,\s*vram:\s*(?i:0x' + format(start, 'X')
                    + r'),\s*name:\s*' + re.escape(OWNER_INPUT) + r'\s*\}\s*$')
    need(len(re.findall(yaml_pattern, yaml.read_text(), re.M)) == 1,
         'YAML does not uniquely carve the actual BSS input')
    identities = {name: loader_identity(name, target_elf, start, loader_cpp) for name in requested}
    artifacts = {str(Path(raw.path)): pp.sha256_file(Path(raw.path))}

    def recheck():
        need(all(pp.sha256_file(Path(p)) == h for p, h in before.items()), 'BSS proof input changed')
        need(all(pp.sha256_file(p) == h for p, h in LOADED.items()), 'loaded proof changed')
        need(all(pp.sha256_file(Path(p)) == h for p, h in artifacts.items()), 'captured owner object changed')
        need(owner_current() == owner_receipt['inputs'], 'owner capture closure changed')
        need(loader_current() == loader_receipt['inputs'], 'loader capture closure changed')
        return True

    recheck()
    receipt = {'schema': 'mickey-canonical-bss-view-v1', 'status': 'independent-owner-complete',
               'namespace_preserved': True, 'identities': identities, 'matching_credit': 0,
               'owner_source': OWNER_SOURCE, 'owner_extent': 32, 'owner_address': start,
               'physical_addresses': {name: unique(target_elf, name)[1] for name in requested},
               'inputs': before, 'owner_capture': owner_receipt, 'loader_capture': loader_receipt,
               'loader_proof': loader_proof, 'candidate_compilation_proved_here': False,
               'candidate_declaration_compatible': True, 'bss_bytes_read': False}
    receipt['candidate_aliases'] = aliases
    path = out / 'receipt.json'; path.write_text(json.dumps(receipt, indent=2, sort_keys=True) + '\n')
    recheck()
    return {'identities': identities, 'receipt': receipt, 'receipt_path': path, 'recheck': recheck}
