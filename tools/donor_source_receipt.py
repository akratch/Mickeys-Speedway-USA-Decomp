#!/usr/bin/env python3
"""Pin a named donor source query without confusing it with object identity.

This is a lexical availability report, not preprocessing, correspondence,
provenance approval, match proof, or assignment authority. Output belongs in
ignored evidence. Only committed Git blobs are read; source text is not emitted.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import subprocess

SCHEMA = 'mickey-donor-source-v1'
SYMBOL = re.compile(r'[A-Za-z_][A-Za-z0-9_]*\Z')
LEXICAL = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*', re.S)


def git(repo: Path, *args: str) -> bytes:
    proc = subprocess.run(['git', '-C', str(repo), *args], capture_output=True)
    if proc.returncode:
        raise ValueError(proc.stderr.decode(errors='replace').strip())
    return proc.stdout


def commit(repo: Path, ref: str) -> str:
    if ref.startswith('-'):
        raise ValueError('revision may not be an option')
    return git(repo, 'rev-parse', '--verify', ref + '^{commit}').decode().strip()


def source(repo: Path, revision: str, path: str) -> tuple[str | None, bytes | None]:
    p = PurePosixPath(path)
    if p.is_absolute() or '..' in p.parts or ':' in path or not path or str(p) != path:
        raise ValueError('source path must be normalized and repository relative')
    entry = git(repo, 'ls-tree', '-z', revision, '--', path)
    if not entry:
        return None, None
    records = entry.rstrip(b'\0').split(b'\0')
    if len(records) != 1:
        raise ValueError('source path is not one file')
    metadata, actual = records[0].split(b'\t', 1)
    mode, kind, oid = metadata.split()
    if actual.decode() != path or kind != b'blob' or mode not in (b'100644', b'100755'):
        raise ValueError('source must be a regular committed file')
    return oid.decode(), git(repo, 'cat-file', 'blob', oid.decode())


def classify(data: bytes | None, path: str, symbol: str) -> dict:
    if not SYMBOL.fullmatch(symbol):
        raise ValueError('invalid counterpart symbol')
    if data is None:
        return {'availability': 'absent-path', 'definition_lines': [], 'fallback_lines': []}
    text = data.decode('utf-8', errors='strict').replace('\\\n', '')
    def mask(match):
        return re.sub(r'[^\n]', ' ', match.group())
    # Literal contents must not manufacture declarations or directives.
    visible = LEXICAL.sub(mask, text)
    comments_removed = LEXICAL.sub(lambda m: mask(m) if m.group().startswith(('/',)) else m.group(), text)
    fallback = re.compile(r'^\s*#\s*pragma\s+GLOBAL_ASM\s*\(\s*"([^"\n]+)"\s*\)', re.M)
    fallbacks = [m for m in fallback.finditer(comments_removed)
                 if PurePosixPath(m[1]).name == symbol + '.s'
                 and visible[m.start():m.end()].lstrip().startswith('#')]
    # Remove directives and their macro replacement text before looking for C.
    code = re.sub(r'^[ \t]*#[^\n]*', lambda m: ' ' * len(m.group()), visible, flags=re.M)
    definition = re.compile(r'^[ \t]*(?:[A-Za-z_][A-Za-z0-9_]*[ \t*]+)+'
                            + re.escape(symbol) + r'\s*\([^;{}]*\)\s*\{', re.M)
    definitions = list(definition.finditer(code))
    if Path(path).suffix.lower() in ('.s', '.asm'):
        availability = 'tracked-assembly-unreviewed'
    elif definitions:
        availability = 'c-text-and-fallback' if fallbacks else 'c-text-present'
    elif fallbacks:
        availability = 'global-asm-only'
    else:
        availability = 'no-direct-definition'
    return {'availability': availability,
            'definition_lines': [text.count('\n', 0, m.start()) + 1 for m in definitions],
            'fallback_lines': [text.count('\n', 0, m.start()) + 1 for m in fallbacks],
            'conditional_directives': len(re.findall(r'^\s*#\s*(?:if|ifdef|ifndef|elif|else|endif)\b', visible, re.M)),
            'configured_availability': 'not-evaluated',
            'assembly_origin': 'not-authenticated' if availability == 'tracked-assembly-unreviewed' else 'not-applicable'}


def make_receipt(target_repo: Path, target_ref: str, target_path: str,
                 target_symbol: str, reference_repo: Path, reference_ref: str,
                 reference_path: str, counterpart: str, basis: str) -> dict:
    if not SYMBOL.fullmatch(target_symbol) or not basis.strip():
        raise ValueError('target symbol and correspondence basis are required')
    target_revision = commit(target_repo, target_ref)
    reference_revision = commit(reference_repo, reference_ref)
    target_blob, target_bytes = source(target_repo, target_revision, target_path)
    if target_bytes is None:
        raise ValueError('target source does not exist')
    target_query = classify(target_bytes, target_path, target_symbol)
    if target_query['availability'] not in ('c-text-present', 'c-text-and-fallback', 'global-asm-only'):
        raise ValueError('target symbol has no direct definition or exact fallback in the pinned file')
    reference_blob, reference_bytes = source(reference_repo, reference_revision, reference_path)
    return {'schema': SCHEMA,
            'target': {'commit': target_revision, 'path': target_path,
                       'blob': target_blob, 'symbol': target_symbol},
            'reference_source': {'commit': reference_revision, 'path': reference_path,
                                 'blob': reference_blob, 'symbol': counterpart},
            'correspondence_basis': basis,
            'correspondence_verified': False,
            'query': classify(reference_bytes, reference_path, counterpart),
            'object_identity': None,
            'scope': 'one named counterpart in one committed source file; no include expansion or version/flag/TU matrix',
            'matrix_exhausted': False, 'assignment_authorized': False,
            'tool_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target-repo', type=Path, default=Path('.'))
    parser.add_argument('--target-ref', default='HEAD')
    parser.add_argument('--target-path', required=True)
    parser.add_argument('--target-symbol', required=True)
    parser.add_argument('--reference-repo', type=Path, required=True)
    parser.add_argument('--reference-ref', required=True)
    parser.add_argument('--reference-path', required=True)
    parser.add_argument('--counterpart', required=True)
    parser.add_argument('--basis', required=True)
    args = parser.parse_args()
    try:
        result = make_receipt(**vars(args))
    except (ValueError, UnicodeError) as error:
        parser.exit(2, str(error) + '\n')
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
