#!/usr/bin/env python3
"""Reviewed nonexecutable ranges; content is a validation, never classification.

The manifest is an explicit ownership decision against one ROM. An arbitrary
zero word, asm owner, name, or ELF function is not an exclusion. This module
never grants matched credit or changes the build's physical section extents.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

ROM_DELTA = 0x7FFFF400  # resident mapping in mickey.us.yaml / overlay_tables
OVERLAY_KEYS = {'kind', 'overlay', 'source', 'owner_offset', 'owner_size', 'offset', 'size'}
RESIDENT_KEYS = {'kind', 'symbol', 'source', 'vram', 'owner_size', 'offset', 'size'}
KINDS = {'overlay-padding', 'overlay-trailing-alignment',
         'resident-trailing-alignment', 'resident-alignment-identity'}


def number(value):
    if not isinstance(value, str) or not re.fullmatch(r'0x[0-9a-fA-F]+', value):
        raise RuntimeError('nonexecutable range requires a hexadecimal extent')
    return int(value, 16)


def reviewed_ranges(root, atlas=None, rom=None):
    """Validate the explicit contract, optionally checking authenticated bytes.

    Without ROM/ELF this validates schema and atlas ownership only, suitable
    for CI and consumers of the committed scoreboard. Live resident ownership
    and category proof is additionally required by apply_accounting().
    """
    root = Path(root)
    data = json.loads((root / 'config/nonexecutable-ranges.us.json').read_text())
    if set(data) != {'schema_version', 'rom_sha1', 'review', 'entries'} or type(data['schema_version']) is not int or data['schema_version'] != 1:
        raise RuntimeError('unsupported nonexecutable range contract')
    if not re.fullmatch(r'[0-9a-f]{40}', data['rom_sha1']):
        raise RuntimeError('invalid nonexecutable ROM identity')
    if data['review'] != 'docs/executable-accounting.md' or not (root / data['review']).is_file():
        raise RuntimeError('missing nonexecutable boundary review')
    if atlas is None:
        atlas = json.loads((root / 'config/overlays.us.json').read_text())
    if atlas['source']['sha1'] != data['rom_sha1']:
        raise RuntimeError('nonexecutable ROM identity disagrees with atlas')
    if rom is not None and hashlib.sha1(rom).hexdigest() != data['rom_sha1']:
        raise RuntimeError('nonexecutable ROM identity is stale')
    if not isinstance(data['entries'], list):
        raise RuntimeError('nonexecutable entries must be a list')
    modules = {m['overlay']: m for m in atlas['modules']}
    if len(modules) != len(atlas['modules']):
        raise RuntimeError('ambiguous nonexecutable overlay identity')
    ranges = []
    for raw in data['entries']:
        if not isinstance(raw, dict) or raw.get('kind') not in KINDS:
            raise RuntimeError('unknown nonexecutable classification')
        overlay = raw['kind'].startswith('overlay-')
        if set(raw) != (OVERLAY_KEYS if overlay else RESIDENT_KEYS):
            raise RuntimeError('invalid nonexecutable entry fields')
        source = raw['source']
        if not isinstance(source, str) or not source or source.startswith('/') or '..' in source.split('/'):
            raise RuntimeError('invalid nonexecutable owner source')
        offset, size, owner_size = (number(raw[k]) for k in ('offset', 'size', 'owner_size'))
        if size <= 0 or owner_size <= 0 or any(x % 4 for x in (offset, size, owner_size)):
            raise RuntimeError('unaligned or empty nonexecutable extent')
        if overlay:
            if type(raw['overlay']) is not int or raw['overlay'] not in modules:
                raise RuntimeError('missing nonexecutable overlay owner')
            module = modules[raw['overlay']]
            owner_offset = number(raw['owner_offset'])
            owners = [o for o in module['text_ownership'] if o['source'] == source
                      and number(o['offset']) == owner_offset and number(o['size']) == owner_size]
            if len(owners) != 1:
                raise RuntimeError('changed or ambiguous nonexecutable atlas ownership')
            owner = owners[0]
            for other in module['text_ownership']:
                if other is not owner and offset < number(other['end_offset']) and number(other['offset']) < offset + size:
                    raise RuntimeError('nonexecutable range overlaps another atlas owner')
            if number(owner['end_offset']) != owner_offset + owner_size:
                raise RuntimeError('inconsistent nonexecutable owner extent')
            if offset < owner_offset or offset + size != owner_offset + owner_size:
                raise RuntimeError('nonexecutable range is not the reviewed owner tail')
            if raw['kind'] == 'overlay-padding':
                if owner['type'] != 'asm' or owner['matched'] or owner['nonmatching'] or offset != owner_offset:
                    raise RuntimeError('padding owner became executable or credited')
                category = 'global_asm'
            else:
                if owner['type'] != 'c' or not owner['matched'] or not owner['nonmatching']:
                    raise RuntimeError('trailing padding conflicts with matched credit/category')
                category = 'nonmatching'
            for island in module.get('mixed_tu_exact_c_ranges', []):
                if offset < number(island['end_offset']) and number(island['offset']) < offset + size:
                    raise RuntimeError('nonexecutable range overlaps matched credit')
            if any(offset <= number(e['offset']) < offset + size for e in module.get('exports', [])):
                raise RuntimeError('nonexecutable range has an exported entry')
            for entry in module.get('entrypoints', {}).values():
                if entry is not None and offset <= number(entry) < offset + size:
                    raise RuntimeError('nonexecutable range has an entrypoint')
            start = number(module['sections']['text']['start']) + offset
            if offset + size > number(module['sections']['text']['size']):
                raise RuntimeError('nonexecutable range exceeds overlay text')
            identity = ('overlay', raw['overlay'])
        else:
            vram = number(raw['vram'])
            if vram % 4 or offset + size != owner_size:
                raise RuntimeError('nonexecutable resident extent is not a tail')
            if not isinstance(raw['symbol'], str) or not re.fullmatch(r'[A-Za-z_]\w*', raw['symbol']):
                raise RuntimeError('invalid nonexecutable resident symbol')
            whole = raw['kind'] == 'resident-alignment-identity'
            if whole != (offset == 0):
                raise RuntimeError('whole-symbol alignment requires its distinct review')
            start = vram - ROM_DELTA + offset
            if start < 0:
                raise RuntimeError('nonexecutable resident address is outside ROM')
            category = 'global_asm' if whole else 'nonmatching'
            identity = ('resident',)
        if rom is not None and (start + size > len(rom) or any(rom[start:start + size])):
            raise RuntimeError('reviewed nonexecutable bytes changed or exceed ROM')
        if any(start < r['rom_end'] and r['rom_start'] < start + size for r in ranges):
            raise RuntimeError('duplicate or overlapping nonexecutable ranges')
        ranges.append(dict(raw, bytes=size, category=category, identity=identity,
                           rom_start=start, rom_end=start + size))
    return ranges


def apply_accounting(root, atlas, rom, funcs, addrs, matched, verified, nonmatching, sources):
    """Return effective resident sizes and deductions after category validation."""
    ranges = reviewed_ranges(root, atlas, rom)
    effective = dict(funcs)
    deductions = {'resident_nonmatching': 0, 'resident_global_asm': 0,
                  'overlay_nonmatching': 0, 'overlay_global_asm': 0}
    for row in ranges:
        if row['identity'][0] == 'resident':
            symbol = row['symbol']
            if (funcs.get(symbol) != number(row['owner_size'])
                    or addrs.get(symbol) != number(row['vram']) or sources.get(symbol) != row['source']):
                raise RuntimeError('changed nonexecutable resident ownership/extent')
            if symbol in matched or symbol in verified:
                raise RuntimeError('nonexecutable resident range conflicts with credited bytes')
            if (symbol in nonmatching) != (row['category'] == 'nonmatching'):
                raise RuntimeError('changed nonexecutable resident category')
            lo = number(row['vram']) + number(row['offset'])
            for name, addr in addrs.items():
                if name != symbol and addr < lo + row['bytes'] and lo < addr + funcs.get(name, 0):
                    raise RuntimeError('nonexecutable resident range overlaps another function')
            effective[symbol] -= row['bytes']
            if effective[symbol] == 0:
                del effective[symbol]
            prefix = 'resident'
        else:
            prefix = 'overlay'
        deductions[prefix + '_' + row['category']] += row['bytes']
    deductions['excluded_bytes'] = sum(deductions.values())
    return effective, deductions


def scoreboard_totals(root):
    """Read the committed score snapshot, verifying its exclusion contract.

    Triage already consumes README's credited-byte snapshot. Derive its
    denominator from the same physical extent and reviewed ranges, rather than
    carrying an independent constant or requiring an ELF in read-only CI.
    """
    root = Path(root)
    ranges = reviewed_ranges(root)
    excluded = sum(r['bytes'] for r in ranges)
    text = (root / 'README.md').read_text()
    whole = re.findall(r'^\| \*\*Whole program\*\* \| ([\d,]+) \| ([\d,]+) \|', text, re.M)
    physical = re.findall(r'^Physical text: ([\d,]+) bytes; reviewed nonexecutable alignment: ([\d,]+) bytes\.', text, re.M)
    if len(whole) != 1 or len(physical) != 1:
        raise RuntimeError('scoreboard lacks one executable/physical accounting snapshot')
    credit, executable = [int(x.replace(',', '')) for x in whole[0]]
    extent, reported_excluded = [int(x.replace(',', '')) for x in physical[0]]
    if reported_excluded != excluded or executable != extent - excluded or not 0 <= credit <= executable:
        raise RuntimeError('scoreboard nonexecutable accounting is stale')
    return {'resolved_bytes': credit, 'whole_program': extent - excluded,
            'physical_text_bytes': extent, 'excluded_bytes': excluded}
