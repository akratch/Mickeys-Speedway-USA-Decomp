#!/usr/bin/env python3
"""Committed raw-TU scheduling only; never reads ROMs or grants matching credit.

The ordinary C gate does not call this resolver. Raw work requires an explicit
selector, a reconciled handoff and a current, reference-only authorization.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys

import yaml

YAML_PATH = 'mickey.us.yaml'
AUTH_PATH = 'config/raw-reference-authorizations.us.json'
DISPOSITIONS_PATH = 'config/raw-reference-dispositions.us.json'
SHARD_DIR = 'docs/raw-reference-handoffs'
WORK_CLASS = 'raw-reference-only'
ROM_SIZE = 0x2000000
ROM_SHA1 = '507341c0a40ca3e9a7cee969b396ee53facfb548'
OID = re.compile(r'[0-9a-f]{40}\Z')
OWNER = re.compile(r'main/[A-Za-z_][A-Za-z0-9_]*\Z')


class GateError(RuntimeError):
    pass


class MissingOwner(GateError):
    pass


def git(*args, check=True):
    p = subprocess.run(['git', *args], text=True, capture_output=True)
    if check and p.returncode:
        raise GateError(p.stderr.strip() or 'git operation failed')
    return p.stdout.strip() if p.returncode == 0 else None


def commit(ref):
    return git('rev-parse', '--verify', '--end-of-options', f'{ref}^{{commit}}')


def ancestor(a, b):
    p = subprocess.run(['git', 'merge-base', '--is-ancestor', a, b], capture_output=True)
    if p.returncode not in (0, 1):
        raise GateError('cannot compare commit ancestry')
    return p.returncode == 0


def show(ref, path):
    # All callers resolve refs to commit IDs before querying historical files.
    p = subprocess.run(['git', 'show', f'{ref}:{path}'], text=True, capture_output=True)
    return p.stdout if p.returncode == 0 else None


def owner_name(owner):
    if not isinstance(owner, str) or not OWNER.fullmatch(owner):
        raise GateError('raw reference owner must be an exact named main/ TU')
    return owner


def shard_path(owner):
    return f'{SHARD_DIR}/{owner_name(owner)}.json'


def strict_json(raw):
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise GateError(f'duplicate JSON field: {key}')
            result[key] = value
        return result
    try:
        return json.loads(raw, object_pairs_hook=unique)
    except (ValueError, TypeError) as e:
        raise GateError(f'invalid JSON: {e}') from e


def fields(value, expected, label):
    if not isinstance(value, dict) or set(value) != set(expected):
        raise GateError(f'{label}: missing or foreign fields')


def prose(value, label):
    if not isinstance(value, str) or not value.strip() or '\n' in value or len(value) > 2000:
        raise GateError(f'{label}: expected nonempty single-line prose')


def row_fields(row):
    if isinstance(row, list) and 1 <= len(row) <= 3:
        start, kind, name = (row + [None, None])[:3]
    elif isinstance(row, dict):
        start, kind, name = row.get('start'), row.get('type'), row.get('name')
    else:
        raise GateError('unsupported YAML subsegment boundary')
    if type(start) is not int or start < 0:
        raise GateError('explicit nonnegative numeric boundary required')
    return start, kind, name


def identity_text(raw, owner):
    owner_name(owner)
    class UniqueLoader(yaml.SafeLoader):
        pass
    def mapping(loader, node, deep=False):
        result = {}
        for k, v in node.value:
            key = loader.construct_object(k, deep=deep)
            if key in result:
                raise GateError('duplicate YAML mapping key')
            result[key] = loader.construct_object(v, deep=deep)
        return result
    UniqueLoader.add_constructor(yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG, mapping)
    try:
        config = yaml.load(raw, Loader=UniqueLoader)
        if not isinstance(config, dict) or config.get('sha1') != ROM_SHA1:
            raise GateError('wrong or missing configured US ROM identity')
        segments = config['segments']
        if not isinstance(segments, list):
            raise GateError('segment list required')
        segment_starts = [row_fields(s)[0] for s in segments]
        if (any(a >= b for a, b in zip(segment_starts, segment_starts[1:]))
                or any(x > ROM_SIZE for x in segment_starts)):
            raise GateError('invalid containing-segment boundaries')
        found = []
        for si, segment in enumerate(segments):
            if not isinstance(segment, dict):
                continue
            subs = segment.get('subsegments', [])
            for i, sub in enumerate(subs):
                if isinstance(sub, dict) and 'subsegments' in sub:
                    raise GateError('nested ownership layouts are unsupported in raw-reference mode')
                name = sub.get('name') if isinstance(sub, dict) else sub[2] if isinstance(sub, list) and len(sub) > 2 else None
                if name == owner:
                    found.append((si, segment, i, sub))
        if not found:
            raise MissingOwner('raw owner is missing')
        if len(found) != 1:
            raise GateError('raw owner is ambiguous')
        si, parent, i, sub = found[0]
        if sum(isinstance(x, dict) and x.get('name') == parent.get('name') for x in segments) != 1:
            raise GateError('ambiguous containing-segment name')
        if isinstance(sub, dict) and set(sub) != {'start', 'type', 'name'}:
            raise GateError('unsupported owner-row mapping or override fields')
        start, kind, _ = row_fields(sub)
        if kind != 'asm' or parent.get('type') != 'code' or parent.get('name') != 'main':
            raise GateError('owner is not a resident raw asm TU')
        if type(parent.get('start')) is not int or type(parent.get('vram')) is not int:
            raise GateError('explicit parent ROM/VRAM mapping required')
        if si + 1 >= len(segments):
            raise GateError('missing parent end boundary')
        parent_end = row_fields(segments[si + 1])[0]
        if (not 0 <= parent['start'] < parent_end <= ROM_SIZE
                or not 0 <= parent['vram'] < 2**32
                or parent['vram'] % 4
                or parent['vram'] + parent_end - parent['start'] > 2**32):
            raise GateError('invalid parent address mapping')
        subs = parent['subsegments']
        end = row_fields(subs[i + 1])[0] if i + 1 < len(subs) else parent_end
        if not parent['start'] <= start < end <= parent_end or start % 4 or end % 4:
            raise GateError('invalid raw owner extent')
        file_starts = []
        for row in subs:
            if isinstance(row, dict) and row.get('type') in ('bss', '.bss') and 'start' not in row:
                continue
            file_starts.append(row_fields(row)[0])
        if any(a >= b for a, b in zip(file_starts, file_starts[1:])):
            raise GateError('overlapping or reversed file-backed subsegments')
        if any(pos < parent['start'] or pos > parent_end for pos in file_starts):
            raise GateError('subsegment outside parent boundary')
        # Check preceding ownership as well: a reversed predecessor must not
        # create an apparently unique but overlapping target.
        if i and row_fields(subs[i - 1])[0] >= start:
            raise GateError('overlapping or reversed preceding boundary')
        dependencies = []
        seen = {parent['name']}
        follow = parent.get('follows_vram')
        while follow is not None:
            if not isinstance(follow, str) or follow in seen:
                raise GateError('invalid or cyclic parent mapping dependency')
            seen.add(follow)
            matches = [(j, x) for j, x in enumerate(segments)
                       if isinstance(x, dict) and x.get('name') == follow]
            if len(matches) != 1:
                raise GateError('ambiguous parent mapping dependency')
            j, dependency = matches[0]
            if (j + 1 >= len(segments) or type(dependency.get('start')) is not int
                    or type(dependency.get('vram')) is not int):
                raise GateError('explicit dependency mapping required')
            dependency_end = row_fields(segments[j + 1])[0]
            if (not 0 <= dependency['start'] < dependency_end <= ROM_SIZE
                    or not 0 <= dependency['vram'] < 2**32
                    or dependency['vram'] % 4
                    or dependency['vram'] + dependency_end - dependency['start'] > 2**32):
                raise GateError('invalid dependency address mapping')
            dependencies.append({'mapping': {k: v for k, v in dependency.items()
                                              if k != 'subsegments'},
                                 'rom_end': row_fields(segments[j + 1])[0]})
            follow = dependency.get('follows_vram')
        return {'source': YAML_PATH, 'owner': owner, 'rom_sha1': config['sha1'],
                'rom_start': start, 'rom_end': end, 'section': '.text',
                'parent_end': parent_end, 'mapping_dependencies': dependencies,
                'parent': {k: v for k, v in parent.items() if k != 'subsegments'}}
    except (KeyError, IndexError, TypeError, yaml.YAMLError) as e:
        raise GateError(f'invalid raw ownership YAML: {e}') from e


def identity(ref, owner):
    if show(ref, f'src/{owner_name(owner)}.c') is not None:
        raise GateError('raw owner also has a C source path; resolve ownership first')
    raw = show(ref, YAML_PATH)
    if raw is None:
        raise GateError('committed ownership YAML missing')
    return identity_text(raw, owner)


def maybe_identity(ref, owner):
    try:
        return identity(ref, owner)
    except GateError:
        return None


def ownership_state(ref, owner):
    """Keep absent and invalid ownership distinct for divergent-lane checks."""
    try:
        return identity(ref, owner)
    except MissingOwner:
        return ('absent',)
    except GateError as error:
        return ('invalid', str(error), show(ref, YAML_PATH), show(ref, f'src/{owner}.c'))


def source_pin(ref, owner):
    current = identity(ref, owner)
    for oid in git('rev-list', '--first-parent', ref, '--', YAML_PATH, f'src/{owner}.c').splitlines():
        here = maybe_identity(oid, owner)
        if here != current:
            continue
        parents = git('rev-list', '--parents', '-n', '1', oid).split()[1:]
        if not parents or maybe_identity(parents[0], owner) != here:
            return oid
    raise GateError('raw owner has no committed source history')


def handoff(raw, owner, ident):
    row = strict_json(raw)
    fields(row, ['schema_version', 'work_class', 'owner', 'identity', 'summary', 'evidence'], 'raw handoff')
    if type(row['schema_version']) is not int or row['schema_version'] != 1 or row['work_class'] != WORK_CLASS:
        raise GateError('invalid raw handoff schema/work class')
    if row['owner'] != owner or row['identity'] != ident:
        raise GateError('foreign or stale raw handoff identity')
    prose(row['summary'], 'summary')
    if not isinstance(row['evidence'], list) or not row['evidence']:
        raise GateError('raw handoff requires reconciled historical evidence')
    for evidence in row['evidence']:
        fields(evidence, ['commit', 'path', 'summary'], 'historical evidence')
        if not isinstance(evidence['commit'], str) or not OID.fullmatch(evidence['commit']):
            raise GateError('historical evidence requires full commit ID')
        path = evidence['path']
        if not isinstance(path, str) or not path.startswith('docs/') or '..' in PurePosixPath(path).parts or not path.endswith('.md') or str(PurePosixPath(path)) != path:
            raise GateError('historical evidence requires canonical documentation path')
        prose(evidence['summary'], 'evidence summary')
    return row


def validate_evidence(row, ref, owner, *, current=True):
    token = re.compile(r'(?<![A-Za-z0-9_])' + re.escape(owner.split('/')[1]) + r'(?![A-Za-z0-9_])')
    newest = {}
    for entry in row['evidence']:
        if not ancestor(entry['commit'], ref):
            raise GateError('historical evidence is not an ancestor')
        text = show(entry['commit'], entry['path'])
        if text is None or not token.search(text):
            raise GateError('historical evidence is missing or does not name raw owner')
        previous = newest.get(entry['path'])
        if previous is None or ancestor(previous, entry['commit']):
            newest[entry['path']] = entry['commit']
        elif not ancestor(entry['commit'], previous):
            raise GateError('historical document evidence has divergent histories')
    if current:
        for path, oid in newest.items():
            if show(ref, path) != show(oid, path):
                raise GateError('legacy evidence changed; reconcile a new raw handoff')


def ledger(ref, owner):
    raw = show(ref, shard_path(owner))
    if raw is None:
        raise GateError('raw owner requires a reconciled handoff; no null-ledger reset')
    row = handoff(raw, owner, identity(ref, owner))
    validate_evidence(row, ref, owner)
    return row


def ledger_pin(ref, owner):
    return git('log', '-1', '--format=%H', ref, '--', shard_path(owner))


def authorizations(raw):
    doc = strict_json(raw)
    fields(doc, ['schema_version', 'work_class', 'authorizations'], 'raw authorization document')
    if type(doc['schema_version']) is not int or doc['schema_version'] != 1 or doc['work_class'] != WORK_CLASS or not isinstance(doc['authorizations'], dict):
        raise GateError('invalid raw authorization schema/work class')
    for owner, row in doc['authorizations'].items():
        owner_name(owner)
        fields(row, ['source_commit', 'ledger_commit', 'reason'], 'raw authorization')
        for key in ('source_commit', 'ledger_commit'):
            if not isinstance(row[key], str) or not OID.fullmatch(row[key]):
                raise GateError('raw reference requires two full non-null pins')
        prose(row['reason'], 'reason')
        if len(row['reason']) > 240 or '|' in row['reason']:
            raise GateError('reason must be at most 240 characters without pipe')
    return doc['authorizations']


def validate_history(rows, base):
    for owner, row in rows.items():
        a, b = row['source_commit'], row['ledger_commit']
        if not ancestor(a, base) or not ancestor(b, base) or not (ancestor(a, b) or ancestor(b, a)):
            raise GateError('raw authorization pins have invalid ancestry')
        ident = identity(a, owner)
        if source_pin(a, owner) != a:
            raise GateError('source pin does not change exact raw owner')
        if identity(b, owner) != ident or ledger_pin(b, owner) != b:
            raise GateError('ledger pin does not identify exact ownership checkpoint')
        ledger(b, owner)


def validated_dispositions(base):
    """Use the same committed ADR 0011 decisions in assignment and landing."""
    from lane_status import claim_dispositions
    try:
        return claim_dispositions(base)
    except RuntimeError as error:
        raise GateError(str(error)) from error


def raw_dispositions(raw, base):
    """Validate exact-tip/owner decisions; these are never authorizations."""
    doc = strict_json(raw)
    fields(doc, ['schema_version', 'work_class', 'claims'], 'raw dispositions')
    if (type(doc['schema_version']) is not int or doc['schema_version'] != 1
            or doc['work_class'] != WORK_CLASS or not isinstance(doc['claims'], dict)):
        raise GateError('invalid raw disposition schema/work class')
    result = {}
    for tip, owners in doc['claims'].items():
        # Frozen private lane objects need not exist in a public clone.
        if not OID.fullmatch(tip):
            raise GateError('raw disposition requires exact commit tip')
        if not isinstance(owners, dict) or not owners:
            raise GateError('raw disposition requires owner-scoped decisions')
        for owner, row in owners.items():
            owner_name(owner)
            fields(row, ['state', 'decision_commit', 'reason'], 'raw disposition decision')
            if row['state'] not in ('rejected', 'superseded'):
                raise GateError('invalid raw disposition state')
            decision = row['decision_commit']
            if (not isinstance(decision, str) or not OID.fullmatch(decision)
                    or commit(decision) != decision or not ancestor(decision, base)):
                raise GateError('raw disposition decision must be an ancestor commit')
            prose(row['reason'], 'raw disposition reason')
            result[tip, owner] = row
    return result


def committed_raw_dispositions(base):
    raw = show(base, DISPOSITIONS_PATH)
    return {} if raw is None else raw_dispositions(raw, base)


def active_lanes(base, owner, ident, base_shard):
    # Lazy shared validation keeps ordinary C resolution unchanged.
    dispositions = validated_dispositions(base)
    raw_decisions = committed_raw_dispositions(base)
    active = []
    legacy_paths = set()
    if base_shard is not None:
        try:
            legacy_paths = {entry['path'] for entry in handoff(base_shard, owner, ident)['evidence']}
        except GateError:
            pass  # Invalid base evidence still fails in phase three.

    for line in git('for-each-ref', '--format=%(refname) %(objectname)', 'refs/heads/lane/', 'refs/remotes/origin/lane/').splitlines():
        branch, head = line.split()
        if ancestor(head, base):
            continue
        commons = git('merge-base', '--all', base, head).splitlines()
        if len(commons) != 1:
            active.append(branch)
            continue
        common = commons[0]
        # Do not filter by containing the latest source pin: older divergent
        # lanes can still own this raw target.
        lane_id, common_id = ownership_state(head, owner), ownership_state(common, owner)
        lane_shard, common_shard = show(head, shard_path(owner)), show(common, shard_path(owner))
        c_path = f'src/{owner}.c'
        c_changed = show(head, c_path) != show(common, c_path) and show(head, c_path) != show(base, c_path)
        legacy_changed = any(
            show(head, path) != show(common, path) and show(head, path) != show(base, path)
            for path in legacy_paths
        )
        disposition = dispositions.get(head)
        legacy_only_reviewed = (
            ((disposition is not None and disposition['symbol'] == owner)
             or (head, owner) in raw_decisions)
            and isinstance(lane_id, dict) and lane_id == common_id
            and show(head, c_path) == show(common, c_path)
            and lane_shard == common_shard
        )
        # A decision belongs to exactly this frozen tip and raw owner. It
        # dismisses only the conservative shared-document claim, never raw
        # ownership/C/shard changes (even if those now resemble the base).
        if legacy_only_reviewed:
            legacy_changed = False
        if (c_changed or legacy_changed or (lane_id != common_id and lane_id != ident)
                or (lane_shard != common_shard and lane_shard != base_shard)):
            active.append(branch)
    return active


def classify(base, owner):
    base = commit(base)
    owner_name(owner)
    result = {'owner': owner, 'work_class': WORK_CLASS, 'base': base,
              'state': 'stale-ledger', 'matching_credit': 0, 'active_lanes': []}
    try:
        ident = identity(base, owner)
        result['identity'] = ident
        result['source_commit'] = source_pin(base, owner)
        raw_shard = show(base, shard_path(owner))
        active = active_lanes(base, owner, ident, raw_shard)
        if active:
            result.update(state='active', active_lanes=active, reason='unintegrated raw ownership or handoff changes')
            return result
        ledger(base, owner)
        result['ledger_commit'] = ledger_pin(base, owner)
        raw = show(base, AUTH_PATH)
        if raw is None:
            raise GateError('raw authorization metadata missing')
        rows = authorizations(raw)
        validate_history(rows, base)
        row = rows.get(owner)
        if row is None:
            result.update(state='already-integrated/exhausted', reason='reference-only authorization required')
        elif row['source_commit'] != result['source_commit'] or row['ledger_commit'] != result['ledger_commit']:
            result['reason'] = 'raw source/handoff pins consumed or stale'
        else:
            result.update(state='base-only', reason=row['reason'])
    except GateError as e:
        result['reason'] = str(e)
    return result


def check_worktree(base):
    """Source-only validation; never makes worktree metadata assignable."""
    base = commit(base)
    validated_dispositions(base)
    committed_raw_dispositions(base)
    if Path(DISPOSITIONS_PATH).exists():
        raw_dispositions(Path(DISPOSITIONS_PATH).read_text(), base)
    rows = authorizations(Path(AUTH_PATH).read_text())
    validate_history(rows, base)
    yaml_text = Path(YAML_PATH).read_text()
    paths = list(Path(SHARD_DIR).rglob('*.json')) if Path(SHARD_DIR).exists() else []
    for path in paths:
        owner = path.relative_to(SHARD_DIR).as_posix().removesuffix('.json')
        if Path(f'src/{owner_name(owner)}.c').exists():
            raise GateError('raw owner also has a C source path')
        row = handoff(path.read_text(), owner, identity_text(yaml_text, owner))
        # This is a commit/history gate, not assignment. Stale but valid
        # evidence may be committed so a subsequent handoff can cite its
        # real commit ID. classify() alone enforces current reconciliation.
        validate_evidence(row, base, owner, current=False)
    return len(rows), len(paths)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base', default='HEAD')
    parser.add_argument('--check', action='store_true', required=True)
    args = parser.parse_args(argv)
    try:
        rows, shards = check_worktree(args.base)
    except (GateError, OSError, ValueError) as e:
        print(f'raw-reference: {e}', file=sys.stderr)
        return 2
    print(f'raw-reference metadata: OK ({rows} authorizations, {shards} handoffs; not assignment authorization)')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
