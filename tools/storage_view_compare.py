#!/usr/bin/env python3
"""Report-only candidate accounting with fresh, explicitly named storage proofs.

This consumes storage_view receipts produced in this invocation, never arbitrary
identity assertions. Default comparison and promotion acceptance are unchanged.
"""
import argparse
import json
from pathlib import Path

import storage_view as sv

rs, fp, pp, ot = sv.rs, sv.fp, sv.pp, sv.ot


def requests(value):
    sv.require(isinstance(value, list) and value, 'nonempty witness request list required')
    names = set()
    for row in value:
        sv.require(isinstance(row, dict) and set(row) == {'source_function', 'external'},
                   'witness request must contain only source_function and external')
        sv.require(all(isinstance(v, str) and v for v in row.values()), 'invalid witness name')
        sv.require(row['external'] not in names, 'duplicate requested external')
        names.add(row['external'])
    return value


def candidate_sites(elf, function, names):
    start, size, _ = rs._unique_symbol(elf, function, require_text=True)
    symbols = elf.symbols()
    indexes = {}
    sites = {}
    for name in names:
        matches = [(i, s) for i, s in enumerate(symbols) if s[0] == name]
        sv.require(len(matches) == 1 and matches[0][1][4] == rs.SHN_UNDEF,
                   'candidate must use one undefined named external: ' + name)
        indexes[name] = matches[0][0]
        owned = [(off - start, kind) for _, off, kind, index in elf.relocations()
                 if start <= off < start + size and index == indexes[name]]
        sv.require(owned and all(k in (rs.R_MIPS_HI16, rs.R_MIPS_LO16) for _, k in owned),
                   'named candidate storage use missing or unsupported: ' + name)
        pending = 0
        for _, kind in owned:
            if kind == rs.R_MIPS_HI16:
                pending += 1
            else:
                sv.require(pending > 0, 'unpaired candidate LO16: ' + name)
                pending = 0
        sv.require(pending == 0, 'unpaired candidate HI16: ' + name)
        sites[name] = owned
    return start, size, indexes, sites


def validate_binding(proof, name, function, overlay, siblings, linked):
    sv.require(proof['external'] == name and proof['source_function'] == function
               and proof['status'] in ('independent-storage-use-proved', 'independent-address-view-proved')
               and proof['promotion_acceptance'] is False, 'unexpected storage proof')
    identity = tuple(proof['identity'])
    sv.require(identity[0] in rs.RESERVED_SELECTORS or
               identity[0] == proof['source_overlay'] == overlay,
               'foreign overlay-local storage imports are forbidden')
    sv.require(siblings.get(name, {identity}) == {identity}, 'candidate overlay named siblings conflict')
    sv.reject_name_conflicts(linked, name, identity, overlay)
    return identity


def replace_records(default, bound, owned):
    keys = {site for sites in owned.values() for site in sites}
    sv.require(len(keys) == sum(map(len, owned.values())), 'duplicate candidate relocation site')
    replacement = {(r.offset, r.rtype): r for r in bound if (r.offset, r.rtype) in keys}
    sv.require(set(replacement) == keys and all(r.identity is not None for r in replacement.values()),
               'explicit storage use has malformed or unresolved HI/LO pairing')
    records = []
    for row in default:
        key = (row['offset'], row['rtype'])
        old = rs.SurfaceRecord(*key, tuple(row['identity']) if row['identity'] is not None else None)
        records.append(replacement.get(key, old))
    sv.require(keys <= {(r.offset, r.rtype) for r in records}, 'default candidate surface lost explicit sites')
    return records


def collect(symbol, candidate, source, witness_requests):
    witness_requests = requests(witness_requests)
    names = {r['external'] for r in witness_requests}
    atlas = json.loads(fp.ATLAS.read_text())
    elf, linked, rom = rs.Elf(candidate), rs.Elf(fp.TARGET_ELF), fp.ROM.read_bytes()
    owner = rs.resolve_overlay_ownership(candidate, atlas, source=source)
    sv.require(owner is not None, 'storage comparison requires an overlay owner')
    module, _ = owner
    overlay = module['overlay']
    start, size, indexes, sites = candidate_sites(elf, symbol, names)
    pins = {p: pp.sha256_file(p) for p in (Path(__file__), candidate, fp.TARGET_ELF, fp.ROM, fp.ATLAS,
                                           rs.LINK_SYMS, sv.ROOT / 'build/mickey.us.map')}
    default = rs.function_surface_comparison(symbol, candidate, fp.TARGET_ELF,
                    source=source, include_candidate_identities=True, include_diagnostics=True)
    target_start = int(default['target_offset'], 16)
    target_size = int(default['target_size'], 16)
    runtime = ot.build_modules(ot.read_headers(rom))[overlay - 1]
    target = rs._target_runtime_records(rom, {'kind': 'overlay', 'overlay': overlay, 'module': runtime},
                                        target_start, target_size)
    siblings = rs._matched_overlay_relocation_witnesses(module, linked, rom, runtime, names,
                    exclude_range=(target_start, target_start + target_size))
    session = sv.WitnessSession()
    proofs, identities = [], {}
    for request in witness_requests:
        sv.require(request['source_function'] != symbol, 'candidate cannot witness itself')
        proof = sv.collect(request['source_function'], request['external'], session=session)
        identities[indexes[request['external']]] = validate_binding(proof, request['external'],
                    request['source_function'], overlay, siblings, linked)
        proofs.append(proof)
    # Only explicit names use independent bases here. No target-site identity
    # or synthetic numeric assignment is provided to this recomputation.
    bound = rs._candidate_surface_records(elf, start, size, [], {}, {}, set(), overlay,
                                         index_identities=identities)
    records = replace_records(default['candidate_identities'], bound, sites)
    session.check()
    sv.require(all(pp.sha256_file(p) == digest for p, digest in pins.items()),
               'candidate or comparison inputs changed during proof')
    return {'schema': 'mickey-storage-view-comparison-v1', 'promotion_acceptance': False,
            'status': 'diagnostic-only', 'default_comparison': default,
            'independent_storage_comparison': rs.compare_record_sets(target, records),
            'storage_proofs': proofs,
            'witness_reuse': {'scope': 'this invocation only',
                              'fresh_function_captures': session.capture_count,
                              'shared_function_reuses': session.reuse_count},
            'candidate_sha256': pins[candidate],
            'limits': 'Explicit storage bases only; no inferred type, subobject, bound, semantics, or promotion bridge.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('symbol')
    parser.add_argument('--candidate-object', required=True, type=Path)
    parser.add_argument('--source', required=True, help='atlas source key, without src/ or .c')
    parser.add_argument('--witnesses', required=True, type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(collect(args.symbol, args.candidate_object, args.source,
                                 json.loads(args.witnesses.read_text())), indent=2))
        return 0
    except (sv.ViewError, fp.PreflightError, pp.MetadataProofError, rs.SurfaceComparisonError,
            OSError, RuntimeError, ValueError) as error:
        parser.exit(1, 'storage_view_compare: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
