#!/usr/bin/env python3
"""Report unintegrated lane commits without reading sibling worktrees.

The report is a coordination hint. A commit message such as ``Match foo`` is
shown as a *claim* until the integration lane repeats the project's exact
object, relocation, linked-range, and ROM checks (ADR 0011).

Both local lane branches and fetched Session B ``origin/lane/burn-b-*``
remote-tracking refs participate.  Fetching origin therefore makes
cross-machine ownership visible without materialising or reading a remote
worker's worktree.
"""

from __future__ import annotations

import argparse
import concurrent.futures
from functools import lru_cache
import hashlib
import json
import os
import re
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path, PurePosixPath

import finalize_plateau
import integration_base


MATCH_RE = re.compile(r"^match(?:ed)?\s+([A-Za-z_][A-Za-z0-9_]*)\b", re.I)
REMOVED_GLOBAL_ASM_RE = re.compile(
    r'^-\s*#pragma\s+GLOBAL_ASM\("(?P<path>[^"]+)"\)', re.MULTILINE,
)
DISPOSITIONS_PATH = "config/lane-claim-dispositions.us.json"
REOPEN_AUTHORIZATIONS_PATH = "config/lane-reopen-authorizations.us.json"
LEGACY_TRIAGE_PATH = "docs/matching-triage.md"
SYMBOL_TOKEN_TEMPLATE = r"(?<![A-Za-z0-9_]){symbol}(?![A-Za-z0-9_])"
FUNCTION_DEFINITION_TEMPLATE = (
    r"(?m)^[ \t]*(?:[A-Za-z_][A-Za-z0-9_]*[ \t*]+)+"
    r"{symbol}\s*\([^;{{}}]*\)\s*\{{"
)
PLATEAU_BLOCK_RE = re.compile(
    r"/\*\s*PLATEAU-HANDOFF(?::[A-Za-z_][A-Za-z0-9_]*:start)?\b"
    r".*?\*/",
    re.DOTALL,
)
PLATEAU_SUBJECT_RE = re.compile(
    r"\b(?:plateaus?|park(?:ed|ing)?)\b", re.I
)
LANE_REF_PREFIXES = (
    "refs/heads/lane/",
    "refs/remotes/origin/lane/burn-b-*",
)


def git(*args: str, check: bool = True) -> str:
    result = subprocess.run(
        ["git", *args], text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if check and result.returncode != 0:
        message = result.stderr.strip() or "git command failed"
        raise RuntimeError(f"git {' '.join(args)}: {message}")
    return result.stdout


def has_global_asm(ref: str, symbol: str) -> bool:
    """True when ``src`` at ``ref`` names a fallback file for ``symbol``.

    A renamed overlay symbol's fallback keeps splat's generated file name, so
    the names the build redefines to ``symbol`` are probed as well.
    """
    patterns = [f"{symbol}.s"] + [
        f"{name}.s" for name in sorted(symbol_fallback_aliases(ref, symbol))
    ]
    command = ["git", "grep", "-q", "-F"]
    for pattern in patterns:
        command.extend(("-e", pattern))
    result = subprocess.run(
        [*command, ref, "--", "src"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if result.returncode not in (0, 1):
        raise RuntimeError(f"git grep failed for {ref}:{symbol}")
    return result.returncode == 0


def has_text(ref: str, text: str) -> bool:
    result = subprocess.run(
        ["git", "grep", "-q", "-F", text, ref, "--", "src"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if result.returncode not in (0, 1):
        raise RuntimeError(f"git grep failed for {ref}:{text}")
    return result.returncode == 0


@lru_cache(maxsize=2048)
def show_file(ref: str, path: str) -> str | None:
    result = subprocess.run(
        ["git", "show", f"{ref}:{path}"], text=True,
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
    )
    if result.returncode == 0:
        return result.stdout
    if result.returncode == 128:
        return None
    raise RuntimeError(f"git show failed for {ref}:{path}")


@lru_cache(maxsize=2048)
def blob_id(ref: str, path: str) -> str | None:
    result = subprocess.run(
        ["git", "rev-parse", "--verify", f"{ref}:{path}"], text=True,
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
    )
    if result.returncode == 0:
        return result.stdout.strip()
    if result.returncode == 128:
        return None
    raise RuntimeError(f"git rev-parse failed for {ref}:{path}")


@lru_cache(maxsize=4096)
def merge_base(left: str, right: str) -> str:
    value = git("merge-base", left, right).strip()
    if not re.fullmatch(r"[0-9a-f]{40}", value):
        raise RuntimeError(f"cannot resolve one merge base for {left} and {right}")
    return value


def blob_ids(refs: list[str], path: str) -> dict[str, str | None]:
    """Resolve one path for many refs with one Git object-database process."""
    if not refs:
        return {}
    queries = "".join(f"{ref}:{path}\n" for ref in refs)
    result = subprocess.run(
        ["git", "cat-file", "--batch-check=%(objectname) %(objecttype)"],
        input=queries,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise RuntimeError(f"git cat-file batch failed: {result.stderr.strip()}")
    lines = result.stdout.splitlines()
    if len(lines) != len(refs):
        raise RuntimeError("git cat-file batch returned the wrong row count")
    resolved: dict[str, str | None] = {}
    for ref, line in zip(refs, lines, strict=True):
        if line.endswith(" missing"):
            resolved[ref] = None
            continue
        fields = line.split()
        if len(fields) != 2 or fields[1] != "blob":
            raise RuntimeError(f"{ref}:{path} did not resolve to one blob")
        resolved[ref] = fields[0]
    return resolved


def blob_contents(
    refs: list[str], path: str,
) -> dict[str, tuple[str, str] | None]:
    """Read one source path from many committed refs in one Git process."""
    if not refs:
        return {}
    queries = "".join(f"{ref}:{path}\n" for ref in refs).encode()
    result = subprocess.run(
        ["git", "cat-file", "--batch"], input=queries,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        detail = result.stderr.decode(errors="replace").strip()
        raise RuntimeError(f"git cat-file batch failed: {detail}")
    data = result.stdout
    cursor = 0
    resolved: dict[str, tuple[str, str] | None] = {}
    for ref in refs:
        newline = data.find(b"\n", cursor)
        if newline < 0:
            raise RuntimeError("git cat-file batch omitted an object header")
        header = data[cursor:newline].decode("ascii", errors="replace")
        cursor = newline + 1
        if header.endswith(" missing"):
            resolved[ref] = None
            continue
        fields = header.split()
        if len(fields) != 3 or fields[1] != "blob":
            raise RuntimeError(f"{ref}:{path} did not resolve to one blob")
        try:
            size = int(fields[2], 10)
        except ValueError as error:
            raise RuntimeError("git cat-file returned an invalid blob size") from error
        end = cursor + size
        if end >= len(data) or data[end:end + 1] != b"\n":
            raise RuntimeError("git cat-file batch returned a truncated blob")
        try:
            text = data[cursor:end].decode("utf-8")
        except UnicodeDecodeError as error:
            raise RuntimeError(f"{ref}:{path} is not UTF-8 source") from error
        resolved[ref] = (fields[0], text)
        cursor = end + 1
    if cursor != len(data):
        raise RuntimeError("git cat-file batch returned unexpected trailing data")
    return resolved


def blob_contents_by_path(
    ref: str, paths: list[str],
) -> dict[str, tuple[str, str] | None]:
    """Read many paths from one committed ref in one Git process."""
    if not paths:
        return {}
    queries = "".join(f"{ref}:{path}\n" for path in paths).encode()
    result = subprocess.run(
        ["git", "cat-file", "--batch"], input=queries,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        detail = result.stderr.decode(errors="replace").strip()
        raise RuntimeError(f"git cat-file batch failed: {detail}")
    data = result.stdout
    cursor = 0
    resolved: dict[str, tuple[str, str] | None] = {}
    for path in paths:
        newline = data.find(b"\n", cursor)
        if newline < 0:
            raise RuntimeError("git cat-file batch omitted an object header")
        header = data[cursor:newline].decode("ascii", errors="replace")
        cursor = newline + 1
        if header.endswith(" missing"):
            resolved[path] = None
            continue
        fields = header.split()
        if len(fields) != 3 or fields[1] != "blob":
            raise RuntimeError(f"{ref}:{path} did not resolve to one blob")
        try:
            size = int(fields[2], 10)
        except ValueError as error:
            raise RuntimeError("git cat-file returned an invalid blob size") from error
        end = cursor + size
        if end >= len(data) or data[end:end + 1] != b"\n":
            raise RuntimeError("git cat-file batch returned a truncated blob")
        try:
            text = data[cursor:end].decode("utf-8")
        except UnicodeDecodeError as error:
            raise RuntimeError(f"{ref}:{path} is not UTF-8 source") from error
        resolved[path] = (fields[0], text)
        cursor = end + 1
    if cursor != len(data):
        raise RuntimeError("git cat-file batch returned unexpected trailing data")
    return resolved


def exact_symbol_pattern(symbol: str) -> re.Pattern[str]:
    return re.compile(
        SYMBOL_TOKEN_TEMPLATE.format(symbol=re.escape(symbol)), re.MULTILINE,
    )


def exact_symbol_rows(text: str | None, symbol: str) -> list[str]:
    if text is None:
        return []
    token = exact_symbol_pattern(symbol)
    return [line for line in text.splitlines() if token.search(line)]


def legacy_evidence_signature(
    text: str | None, symbol: str,
) -> tuple[tuple[str, ...], tuple[str, ...]]:
    """Select this symbol's rows and generated blocks from the shared ledger."""
    if text is None:
        return (), ()
    marker = re.escape(f"plateau-handoff:{symbol}")
    pattern = re.compile(
        rf"<!-- {marker}:start -->.*?<!-- {marker}:end -->\n?",
        re.DOTALL,
    )
    return tuple(exact_symbol_rows(text, symbol)), tuple(pattern.findall(text))


def malformed_legacy_marker(text: str | None, symbol: str) -> bool:
    """Reject a target generated block whose paired markers are damaged."""
    if text is None:
        return False
    start = f"<!-- plateau-handoff:{symbol}:start -->"
    end = f"<!-- plateau-handoff:{symbol}:end -->"
    if start not in text and end not in text:
        return False
    _rows, blocks = legacy_evidence_signature(text, symbol)
    return text.count(start) != 1 or text.count(end) != 1 or len(blocks) != 1


@lru_cache(maxsize=4096)
def source_identity(ref: str, symbol: str) -> tuple[str | None, str | None]:
    """Return the one committed definition path, or a fail-closed reason."""
    output = git(
        "grep", "-l", "-w", "-e", symbol, ref, "--", "src", check=False,
    )
    definition = re.compile(
        FUNCTION_DEFINITION_TEMPLATE.format(symbol=re.escape(symbol)),
        re.DOTALL,
    )
    paths = []
    for row in output.splitlines():
        path = row.split(":", 1)[1] if ":" in row else row
        text = show_file(ref, path)
        if text is not None and definition.search(text):
            paths.append(path)
    paths = sorted(set(paths))
    if len(paths) == 1:
        return paths[0], None
    if not paths:
        return None, f"no exact committed source definition for {symbol}"
    return None, "ambiguous exact source definitions: " + ", ".join(paths)


def source_identity_index(
    ref: str, symbols: list[str],
) -> dict[str, tuple[str | None, str | None]]:
    """Resolve many exact definitions with one grep and one object batch."""
    ordered = list(dict.fromkeys(symbols))
    if not ordered:
        return {}
    command = ["git", "grep", "-l", "-w"]
    for symbol in ordered:
        command.extend(("-e", symbol))
    command.extend((ref, "--", "src"))
    result = subprocess.run(
        command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode not in (0, 1):
        detail = result.stderr.strip() or "git grep failed"
        raise RuntimeError(f"batch source identity: {detail}")
    paths = sorted({
        row.split(":", 1)[1] if ":" in row else row
        for row in result.stdout.splitlines() if row
    })
    objects = blob_contents_by_path(ref, paths)
    found: dict[str, list[str]] = {symbol: [] for symbol in ordered}
    token = re.compile(
        r"(?<![A-Za-z0-9_])(?:"
        + "|".join(re.escape(symbol) for symbol in sorted(ordered, key=len, reverse=True))
        + r")(?![A-Za-z0-9_])"
    )
    for path in paths:
        item = objects[path]
        if item is None:
            continue
        text = item[1]
        candidates = {match.group(0) for match in token.finditer(text)}
        for symbol in candidates:
            definition = re.compile(
                FUNCTION_DEFINITION_TEMPLATE.format(symbol=re.escape(symbol)),
                re.DOTALL,
            )
            if definition.search(text):
                found[symbol].append(path)
    identities: dict[str, tuple[str | None, str | None]] = {}
    for symbol in ordered:
        definitions = sorted(set(found[symbol]))
        if len(definitions) == 1:
            identities[symbol] = (definitions[0], None)
        elif not definitions:
            identities[symbol] = (
                None, f"no exact committed source definition for {symbol}",
            )
        else:
            identities[symbol] = (
                None, "ambiguous exact source definitions: "
                + ", ".join(definitions),
            )
    return identities


def guarded_fallback(
    text: str, symbol: str, aliases: frozenset[str] | set[str] = frozenset(),
) -> bool:
    """Recognise an exact-symbol NON_MATCHING body and its one fallback.

    Delegates to `finalize_plateau.guarded_candidates`, the depth-aware
    reader the plateau tool itself uses. The regex this replaced paired the
    target ``#ifdef NON_MATCHING`` with the first ``#else``/``#endif`` after
    it, so any candidate body carrying its own ``#if ... #else ... #endif``
    (a variant switch) read as having no fallback, and the symbol was
    classified already-integrated/exhausted while still NON_MATCHING.
    ``aliases`` are the splat names the build renames to ``symbol``
    (`build_fallback_aliases`).
    """
    try:
        candidates = finalize_plateau.guarded_candidates(text, symbol, aliases)
    except finalize_plateau.PlateauError:
        return False
    return len(candidates) == 1


@lru_cache(maxsize=256)
def build_fallback_aliases(ref: str) -> dict[tuple[str, str], frozenset[str]]:
    """The build's generated-name -> symbol renames at ``ref``, per source."""
    return finalize_plateau.fallback_aliases(
        show_file(ref, finalize_plateau.OVERLAY_RULES_PATH)
    )


def source_fallback_aliases(ref: str, path: str, symbol: str) -> frozenset[str]:
    return build_fallback_aliases(ref).get((path, symbol), frozenset())


def symbol_fallback_aliases(ref: str, symbol: str) -> frozenset[str]:
    """Every splat name the build renames to ``symbol``, from any source."""
    names: set[str] = set()
    for (_source, built), generated in build_fallback_aliases(ref).items():
        if built == symbol:
            names.update(generated)
    return frozenset(names)


def has_plateau_handoff(text: str, symbol: str) -> bool:
    return bool(plateau_handoff_signature(text, symbol))


def plateau_handoff_signature(text: str, symbol: str) -> tuple[str, ...]:
    """Return only this symbol's inline handoff blocks, byte for byte."""
    token = exact_symbol_pattern(symbol)
    found = []
    for block in PLATEAU_BLOCK_RE.findall(text):
        if re.search(rf"\bsymbol:\s*{re.escape(symbol)}\s*(?:\n|$)", block):
            found.append(block)
        elif block.startswith(f"/* PLATEAU-HANDOFF:{symbol}:start") and token.search(block):
            found.append(block)
    return tuple(found)


def shard_path(symbol: str) -> str:
    return finalize_plateau.handoff_shard_path(symbol)


def validated_shard_source(text: str | None, symbol: str) -> str | None:
    if text is None:
        return None
    try:
        return finalize_plateau.handoff_shard_source(text, symbol)
    except finalize_plateau.PlateauError as error:
        raise RuntimeError(str(error)) from error


@lru_cache(maxsize=4096)
def path_history(ref: str, path: str) -> tuple[tuple[str, str], ...]:
    """Return one path's commit/subject history, shared across symbols."""
    raw = git("log", "--format=%H%x00%s%x1e", ref, "--", path)
    records: list[tuple[str, str]] = []
    for record in raw.split("\x1e"):
        record = record.strip("\n")
        if not record:
            continue
        commit, subject = record.split("\0", 1)
        records.append((commit, subject))
    return tuple(records)


def target_history_record(
    ref: str, symbol: str, paths: list[str], *, require_plateau: bool,
) -> tuple[str, str] | None:
    if not paths:
        return None
    if len(paths) == 1:
        records = path_history(ref, paths[0])
    else:
        raw = git(
            "log", "--format=%H%x00%s%x1e", ref, "--", *paths,
        )
        parsed: list[tuple[str, str]] = []
        for record in raw.split("\x1e"):
            record = record.strip("\n")
            if not record:
                continue
            parsed.append(tuple(record.split("\0", 1)))
        records = tuple(parsed)
    token = exact_symbol_pattern(symbol)
    for commit, subject in records:
        names_target = bool(token.search(subject))
        changes_target = any(
            target_guard_changed(commit, symbol, path) for path in paths
        )
        if not names_target and not changes_target:
            continue
        if require_plateau and "plateau" not in subject.lower():
            continue
        return commit, subject
    return None


def target_history_commit(
    ref: str, symbol: str, paths: list[str], *, require_plateau: bool,
) -> str | None:
    record = target_history_record(
        ref, symbol, paths, require_plateau=require_plateau,
    )
    return record[0] if record else None


@lru_cache(maxsize=16384)
def guarded_candidate_region(ref: str, path: str, symbol: str) -> str | None:
    """Return one exact committed NON_MATCHING region for history ownership."""
    text = show_file(ref, path)
    if text is None:
        return None
    try:
        candidate = finalize_plateau.require_guarded_candidate(text, symbol)
    except finalize_plateau.PlateauError:
        return None
    lines = text.splitlines(keepends=True)
    return "".join(lines[candidate.ifdef_line:candidate.endif_line + 1])


@lru_cache(maxsize=16384)
def target_guard_changed(commit: str, symbol: str, path: str) -> bool:
    """Detect target-owned history even when a commit subject uses a nickname."""
    parent = first_parent(commit)
    current = guarded_candidate_region(commit, path, symbol)
    previous = (
        guarded_candidate_region(parent, path, symbol)
        if parent is not None else None
    )
    return current != previous and (current is not None or previous is not None)


def path_plateau_record(ref: str, path: str) -> tuple[str, str] | None:
    """Return the latest plateau commit for a single-candidate source path."""
    for commit, subject in path_history(ref, path):
        if PLATEAU_SUBJECT_RE.search(subject):
            return commit, subject
    return None


@lru_cache(maxsize=4096)
def commit_paths(commit: str) -> frozenset[str]:
    """Return paths owned by one commit without repeated diff-tree calls."""
    return frozenset(git(
        "diff-tree", "--no-commit-id", "--name-only", "-r", commit,
    ).splitlines())


def ledger_source_plateau_record(
    ref: str, symbol: str, source_path: str,
) -> tuple[str, str] | None:
    """Find exact-symbol plateau evidence committed with a mixed-TU source.

    Older consolidated translation units often used a shortened generated
    name (or only an overlay number) in the commit subject.  The exact symbol
    still appears in the triage row changed by that commit.  Requiring the
    same commit to change both that exact row and the owning source path keeps
    this fallback unambiguous without treating every path-wide plateau as
    evidence for every guarded function in the file.
    """
    raw = git(
        "log", "--format=%H%x00%s%x1e", ref, "--", LEGACY_TRIAGE_PATH,
    )
    token = exact_symbol_pattern(symbol)
    for record in raw.split("\x1e"):
        record = record.strip("\n")
        if not record:
            continue
        commit, subject = record.split("\0", 1)
        if not PLATEAU_SUBJECT_RE.search(subject):
            continue
        changed = commit_paths(commit)
        if source_path not in changed or LEGACY_TRIAGE_PATH not in changed:
            continue
        triage = show_file(commit, LEGACY_TRIAGE_PATH)
        if triage is None:
            continue
        parent = first_parent(commit)
        previous = show_file(parent, LEGACY_TRIAGE_PATH) if parent else None
        if legacy_evidence_signature(triage, symbol) == legacy_evidence_signature(
            previous, symbol,
        ):
            continue
        rows = (line for line in triage.splitlines() if token.search(line))
        if any(PLATEAU_SUBJECT_RE.search(line) for line in rows):
            return commit, subject
    return None


def shard_source_plateau_record(
    ref: str, symbol: str, source_path: str,
) -> tuple[str, str] | None:
    """Find a plateau commit that changed one source and its exact shard."""
    ledger_path = shard_path(symbol)
    raw = git("log", "--format=%H%x00%s%x1e", ref, "--", ledger_path)
    for record in raw.split("\x1e"):
        record = record.strip("\n")
        if not record:
            continue
        commit, subject = record.split("\0", 1)
        if not PLATEAU_SUBJECT_RE.search(subject):
            continue
        changed = commit_paths(commit)
        if source_path not in changed or ledger_path not in changed:
            continue
        shard = show_file(commit, ledger_path)
        if shard is None:
            continue
        try:
            recorded_source = finalize_plateau.handoff_shard_source(shard, symbol)
        except finalize_plateau.PlateauError:
            continue
        if recorded_source == source_path:
            return commit, subject
    return None


def newest_related_commit(commits: list[str]) -> str | None:
    """Choose the descendant when exact evidence exists in both ledgers."""
    if not commits:
        return None
    selected = commits[0]
    for commit in commits[1:]:
        if is_ancestor(selected, commit):
            selected = commit
    return selected


@lru_cache(maxsize=4096)
def first_parent(commit: str) -> str | None:
    fields = git("rev-list", "--parents", "-n", "1", commit).split()
    return fields[1] if len(fields) > 1 else None


@lru_cache(maxsize=32)
def blame_line_commits(ref: str, path: str) -> tuple[str, ...]:
    """Map current line numbers to commits with one reusable blame."""
    blame = git("blame", "--line-porcelain", ref, "--", path)
    commits: list[str] = []
    current_commit: str | None = None
    for line in blame.splitlines():
        header = re.fullmatch(r"\^?([0-9a-f]{40}) \d+ \d+(?: \d+)?", line)
        if header:
            current_commit = header.group(1)
        elif line.startswith("\t"):
            if current_commit is None:
                raise RuntimeError("git blame omitted a line commit")
            commits.append(current_commit)
    return tuple(commits)


def latest_legacy_evidence_commit(ref: str, symbol: str) -> str | None:
    """Find the newest commit owning a current target row or block line."""
    text = show_file(ref, LEGACY_TRIAGE_PATH)
    if text is None:
        return None
    token = exact_symbol_pattern(symbol)
    start = f"<!-- plateau-handoff:{symbol}:start -->"
    end = f"<!-- plateau-handoff:{symbol}:end -->"
    selected_lines: set[int] = set()
    inside = False
    for index, line in enumerate(text.splitlines(), 1):
        if line == start:
            inside = True
        if inside or token.search(line):
            selected_lines.add(index)
        if line == end:
            inside = False
    if not selected_lines:
        return None

    line_commits = blame_line_commits(ref, LEGACY_TRIAGE_PATH)
    if len(line_commits) != len(text.splitlines()):
        raise RuntimeError("git blame returned the wrong legacy-ledger line count")
    commits = [line_commits[line - 1] for line in sorted(selected_lines)]
    return newest_related_commit(list(dict.fromkeys(commits)))


@lru_cache(maxsize=8192)
def latest_path_commit(ref: str, path: str, *, exclude: str | None = None) -> str | None:
    args = ["log", "-1", "--format=%H", ref]
    if exclude:
        args.extend(("--not", exclude))
    args.extend(("--", path))
    value = git(*args).strip()
    return value or None


@lru_cache(maxsize=16384)
def is_ancestor(older: str, newer: str) -> bool:
    result = subprocess.run(
        ["git", "merge-base", "--is-ancestor", older, newer],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if result.returncode not in (0, 1):
        raise RuntimeError(f"cannot compare commits {older} and {newer}")
    return result.returncode == 0


def removed_global_asms(commit: str) -> list[str]:
    patch = git(
        "show", "--format=", "--no-ext-diff", "--unified=0", commit,
        "--", "src",
    )
    return sorted(set(REMOVED_GLOBAL_ASM_RE.findall(patch)))


def source_paths(commit: str) -> list[str]:
    paths = git(
        "diff-tree", "--no-commit-id", "--name-only", "-r", commit,
        "--", "src",
    ).splitlines()
    return sorted(path for path in paths if path.endswith((".c", ".h")))


@dataclass
class Claim:
    symbol: str
    commit: str
    committed_at: str
    subject: str
    state: str
    source_paths: list[str]
    reason: str | None = None
    decision_commit: str | None = None


@dataclass
class Lane:
    branch: str
    head: str
    ahead: int
    claims: list[Claim]


@dataclass
class Assignment:
    symbol: str
    state: str
    source_path: str | None
    source_commit: str | None
    ledger_commit: str | None
    active_lanes: list[str]
    reason: str
    reason_code: str | None = None


def lane_refs(
    *, containing: str | None = None, unmerged_into: str | None = None
) -> list[tuple[str, str]]:
    """Return committed local and fetched cross-machine lane tips."""
    rows = []
    args = ["for-each-ref", "--format=%(refname:short)%00%(objectname)"]
    if containing:
        args.append(f"--contains={containing}")
    if unmerged_into:
        args.append(f"--no-merged={unmerged_into}")
    args.extend(LANE_REF_PREFIXES)
    output = git(*args)
    for line in output.splitlines():
        branch, head = line.split("\0", 1)
        rows.append((branch, head))
    return rows


@dataclass(frozen=True)
class LanePathIndex:
    """Committed lane deltas indexed once for a complete queue scan."""

    base: str
    refs_by_path: dict[str, tuple[tuple[str, str], ...]]
    legacy_refs_by_symbol: dict[str, tuple[tuple[str, str], ...]]
    common_by_branch: dict[str, str]

    def refs_for(
        self, paths: list[str], *, symbol: str,
    ) -> list[tuple[str, str]]:
        selected: dict[str, str] = {}
        for path in paths:
            for branch, head in self.refs_by_path.get(path, ()):
                selected[branch] = head
        for branch, head in self.legacy_refs_by_symbol.get(symbol, ()):
            selected[branch] = head
        return sorted(selected.items())


def _lane_delta(
    base: str, ref: tuple[str, str],
) -> tuple[str, str, str, tuple[str, ...]]:
    branch, head = ref
    common = merge_base(base, branch)
    paths = tuple(sorted(set(git(
        "diff", "--name-only", "--no-renames", common, branch,
    ).splitlines())))
    return branch, head, common, paths


def legacy_evidence_map(
    text: str | None, symbols: set[str],
) -> dict[str, tuple[tuple[str, ...], tuple[str, ...]]]:
    """Parse exact-symbol legacy evidence once for a branch snapshot."""
    if text is None:
        return {}
    rows: dict[str, list[str]] = {}
    token = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
    for line in text.splitlines():
        for symbol in set(token.findall(line)) & symbols:
            rows.setdefault(symbol, []).append(line)
    blocks: dict[str, list[str]] = {}
    block_re = re.compile(
        r"<!-- plateau-handoff:(?P<symbol>[A-Za-z_][A-Za-z0-9_]*):start -->"
        r".*?<!-- plateau-handoff:(?P=symbol):end -->\n?",
        re.DOTALL,
    )
    for match in block_re.finditer(text):
        symbol = match.group("symbol")
        if symbol in symbols:
            blocks.setdefault(symbol, []).append(match.group(0))
    return {
        symbol: (tuple(rows.get(symbol, ())), tuple(blocks.get(symbol, ())))
        for symbol in set(rows) | set(blocks)
    }


def build_lane_path_index(
    base: str, symbols: list[str], *, jobs: int = 1,
) -> LanePathIndex:
    """Index lane-owned committed paths without opening lane worktrees."""
    refs = lane_refs(unmerged_into=base)
    if jobs > 1 and len(refs) > 1:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as executor:
            deltas = list(executor.map(lambda ref: _lane_delta(base, ref), refs))
    else:
        deltas = [_lane_delta(base, ref) for ref in refs]
    refs_by_path: dict[str, list[tuple[str, str]]] = {}
    legacy_candidates: list[tuple[str, str]] = []
    common_by_branch: dict[str, str] = {}
    for branch, head, common, paths in deltas:
        common_by_branch[branch] = common
        for path in paths:
            if path == LEGACY_TRIAGE_PATH:
                legacy_candidates.append((branch, head))
            else:
                refs_by_path.setdefault(path, []).append((branch, head))

    legacy_refs_by_symbol: dict[str, list[tuple[str, str]]] = {}
    if legacy_candidates:
        branches = [branch for branch, _head in legacy_candidates]
        common_refs = sorted({common_by_branch[branch] for branch in branches})
        lane_objects = blob_contents(branches, LEGACY_TRIAGE_PATH)
        common_objects = blob_contents(common_refs, LEGACY_TRIAGE_PATH)
        symbol_set = set(symbols)
        evidence_cache: dict[str, dict[
            str, tuple[tuple[str, ...], tuple[str, ...]]
        ]] = {}

        def evidence(item: tuple[str, str] | None) -> dict[
            str, tuple[tuple[str, ...], tuple[str, ...]]
        ]:
            if item is None:
                return {}
            blob, text = item
            if blob not in evidence_cache:
                evidence_cache[blob] = legacy_evidence_map(text, symbol_set)
            return evidence_cache[blob]

        for branch, head in legacy_candidates:
            lane_map = evidence(lane_objects[branch])
            common_map = evidence(common_objects[common_by_branch[branch]])
            for symbol in set(lane_map) | set(common_map):
                if lane_map.get(symbol) != common_map.get(symbol):
                    legacy_refs_by_symbol.setdefault(symbol, []).append(
                        (branch, head)
                    )
    return LanePathIndex(
        base=base,
        refs_by_path={
            path: tuple(sorted(path_refs))
            for path, path_refs in refs_by_path.items()
        },
        legacy_refs_by_symbol={
            symbol: tuple(sorted(symbol_refs))
            for symbol, symbol_refs in legacy_refs_by_symbol.items()
        },
        common_by_branch=common_by_branch,
    )


def active_lanes_for_source(
    base: str, symbol: str, base_path: str, base_blob: str,
    base_source_commit: str, base_text: str | None = None,
    lane_index: LanePathIndex | None = None,
    dispositions: dict[str, dict[str, str]] | None = None,
) -> list[str]:
    """Return lanes with committed work on this exact guarded candidate.

    A translation unit may contain many independently assignable NON_MATCHING
    functions.  Comparing only the whole-file blob made an old edit to any one
    function reserve every other function in that file.  Compare the validated
    target guard instead, while still failing closed when a lane removes or
    damages the guard, or adds a target handoff.
    """
    active = []
    dispositions = dispositions or {}
    target_shard_path = shard_path(symbol)
    if lane_index is not None:
        if lane_index.base != base:
            raise RuntimeError(
                f"lane path index is for {lane_index.base}, expected {base}"
            )
        refs = [
            ref for ref in lane_index.refs_for(
                [base_path, target_shard_path], symbol=symbol,
            )
            if is_ancestor(base_source_commit, ref[0])
        ]
    else:
        refs = lane_refs(
            containing=base_source_commit, unmerged_into=base
        )
    if not refs:
        # Identical to the all() test below over an empty branch list; it only
        # skips the three base reads that test would otherwise make first.
        return []
    branches = [branch for branch, _head in refs]
    objects = blob_contents(branches, base_path)
    legacy_objects = blob_contents(branches, LEGACY_TRIAGE_PATH)
    shard_objects = blob_contents(branches, target_shard_path)
    if base_text is None:
        base_text = show_file(base, base_path)
    base_legacy_evidence = legacy_evidence_signature(
        show_file(base, LEGACY_TRIAGE_PATH), symbol,
    )
    base_shard = show_file(base, target_shard_path)
    if all(
        objects[branch] is not None
        and objects[branch][0] == base_blob
        and legacy_evidence_signature(
            legacy_objects[branch][1]
            if legacy_objects[branch] is not None else None,
            symbol,
        ) == base_legacy_evidence
        and (
            shard_objects[branch][1]
            if shard_objects[branch] is not None else None
        ) == base_shard
        for branch in branches
    ):
        return []

    # A lane can legitimately predate a handoff-only repair committed on the
    # integration branch. Comparing that old lane directly with today's base
    # made the base-owned repair look like lane-owned work and reserved the
    # target on every historical ref. Determine ownership relative to each
    # lane's merge base first; the cache bounds this to one Git query per lane
    # across a complete ready-queue scan.
    common_by_branch = {
        branch: (
            lane_index.common_by_branch[branch]
            if lane_index is not None else merge_base(base, branch)
        )
        for branch in branches
    }
    common_refs = sorted(set(common_by_branch.values()))
    common_objects = blob_contents(common_refs, base_path)
    common_legacy_objects = blob_contents(common_refs, LEGACY_TRIAGE_PATH)
    common_shard_objects = blob_contents(common_refs, target_shard_path)
    try:
        base_candidate = finalize_plateau.require_guarded_candidate(
            base_text or "", symbol
        )
    except finalize_plateau.PlateauError:
        base_candidate = None
    base_lines = (base_text or "").splitlines(keepends=True)
    base_region = (
        "".join(base_lines[base_candidate.ifdef_line:base_candidate.endif_line + 1])
        if base_candidate is not None else None
    )
    base_handoff = plateau_handoff_signature(base_text or "", symbol)
    for branch, head in refs:
        disposition = dispositions.get(head)
        if disposition is not None and disposition["symbol"] == symbol:
            # A reviewed frozen tip may retain a historical target guard for
            # audit even after canonical explicitly rejected or superseded it.
            # Keep the ref, but do not let that adjudicated guard reserve the
            # target forever.
            continue
        # The contains filter already excludes old refs that predate the
        # current target source. Most retained lane refs have not changed this
        # path at all; compare its blob before paying for a full symbol/path
        # resolution on the small differing remainder.
        lane_object = objects[branch]
        lane_legacy_object = legacy_objects[branch]
        lane_shard_object = shard_objects[branch]
        lane_legacy = lane_legacy_object[1] if lane_legacy_object is not None else None
        lane_shard = lane_shard_object[1] if lane_shard_object is not None else None
        common = common_by_branch[branch]
        common_object = common_objects[common]
        common_legacy_object = common_legacy_objects[common]
        common_shard_object = common_shard_objects[common]
        common_legacy = (
            common_legacy_object[1] if common_legacy_object is not None else None
        )
        common_shard = (
            common_shard_object[1] if common_shard_object is not None else None
        )
        lane_legacy_signature = legacy_evidence_signature(lane_legacy, symbol)
        common_legacy_signature = legacy_evidence_signature(common_legacy, symbol)
        if (
            lane_legacy_signature != common_legacy_signature
            and lane_legacy_signature != base_legacy_evidence
        ):
            active.append(branch)
            continue
        if lane_shard != common_shard and lane_shard != base_shard:
            active.append(branch)
            continue
        if lane_object is None:
            if common_object is not None:
                active.append(branch)
            continue
        lane_blob, lane_text = lane_object
        if lane_blob == base_blob:
            continue
        if common_object is not None and lane_blob == common_object[0]:
            # Only base changed this path after the branch point.
            continue
        if base_region is None:
            active.append(branch)
            continue
        try:
            lane_candidate = finalize_plateau.require_guarded_candidate(
                lane_text, symbol
            )
        except finalize_plateau.PlateauError:
            active.append(branch)
            continue
        lane_lines = lane_text.splitlines(keepends=True)
        lane_region = "".join(
            lane_lines[
                lane_candidate.ifdef_line:lane_candidate.endif_line + 1
            ]
        )
        common_text = common_object[1] if common_object is not None else None
        try:
            common_candidate = finalize_plateau.require_guarded_candidate(
                common_text or "", symbol
            )
        except finalize_plateau.PlateauError:
            common_candidate = None
        if common_candidate is not None:
            common_lines = (common_text or "").splitlines(keepends=True)
            common_region = "".join(
                common_lines[
                    common_candidate.ifdef_line:common_candidate.endif_line + 1
                ]
            )
            if (
                lane_region == common_region
                and plateau_handoff_signature(lane_text, symbol)
                == plateau_handoff_signature(common_text or "", symbol)
            ):
                # This lane changed another function in a mixed TU. A later
                # base-only repair of this target must not turn that unrelated
                # historical path delta into target ownership.
                continue
        if lane_region != base_region:
            active.append(branch)
            continue
        if plateau_handoff_signature(lane_text, symbol) != base_handoff:
            active.append(branch)
    return sorted(active)


ASSIGNMENT_CACHE_SCHEMA = 2
ASSIGNMENT_CACHE_DIR = PurePosixPath("build/cache/lane-assignment")
ASSIGNMENT_CACHE_FILE = "entries.json"
# Keys kept per symbol. A symbol's key moves only when its own evidence moves,
# so the newest few cover the integration base and the lane bases that
# dispatch_check is pointed at, and the file stays bounded by the queue.
ASSIGNMENT_CACHE_KEYS_PER_SYMBOL = 4
# The classifier's own code is part of every key: a logic change must never be
# answered from a verdict an older rule produced.
_CACHE_CODE_FILES = ("lane_status.py", "finalize_plateau.py")


def path_anchor(ref: str, path: str) -> str | None:
    """The commit `git log <ref> -- <path>` starts from, or None.

    Every history walk the settled classifier makes over ``path`` (the
    target's history commit, the path's plateau record, the shard's and the
    legacy ledger's last-change commits, the legacy ledger's blame) is a
    function of this commit alone: default history simplification follows
    one TREESAME line from ``ref`` down to it, so the walk from ``ref`` and
    the walk from the anchor list the same commits in the same order.
    """
    return latest_path_commit(ref, path)


class AssignmentCache:
    """Content-keyed store for the base-derived phases of `assignment_status`.

    Measured on 2026-09-23: classifying the 244-function queue took two to
    three and a half minutes, nearly all of it walking git history to
    re-derive each symbol's source and ledger pins. Phases 1 and 3
    (`_pre_active_status`, `_settled_status`) read only the base commit, so
    they are stored; phase 2, lane ownership, also reads lane refs that move
    independently of the base, and is recomputed on every call.

    Until 2026-09-23 the key carried the integration base commit, so every
    merge batch -- which moves the base but touches few symbols' evidence --
    refilled the whole queue cold (119.6 s measured after one empty commit).
    The key now names exactly what phases 1 and 3 read, and not the base:

    * schema and the classifier's code (`_CACHE_CODE_FILES`);
    * the symbol, its source path, the source blob, and the source path's
      last-change commit (`path_anchor`; the history `target_history_commit`
      and `path_plateau_record` walk);
    * the handoff shard path, its blob, and -- when the shard exists -- its
      last-change commit (what `shard_source_plateau_record` walks and
      `latest_path_commit` returns for it). A missing shard is never walked;
    * the legacy triage ledger's blob and last-change commit (its rows, its
      blame, and `ledger_source_plateau_record`);
    * this symbol's own row of the reopen-authorization file, and whether
      the file as a whole validates at this base (`reopen_authorizations`
      fails closed for every symbol when any row is invalid). The validation
      verdict costs about 30 s on the real file, so it is itself stored,
      keyed by the file's blob and the base term below. Keying on the file's
      blob instead would send the whole queue cold on every pin renewal,
      which is exactly when `authorize_reopen.py --verify` runs;
    * the build's fallback aliases for this (source, symbol) pair, which is
      the only thing read from ``mk/overlays.mk``;
    * the base commit itself, only when some authorization pin is not an
      ancestor of the authorization file's last-change commit. Then the
      pins' ancestry to the base is not implied by the anchor and has to be
      re-proved per base; otherwise a base that contains the anchor contains
      every pin.

    Everything else phases 1 and 3 compute is a pure function of commit ids
    (ancestry, first parents, guarded regions at a commit). So a merge that
    leaves a symbol's files alone leaves its key alone and hits.

    Entries are invalidated by key only, never by age. They live in one file
    under ``build/`` (gitignored); each symbol keeps its newest
    `ASSIGNMENT_CACHE_KEYS_PER_SYMBOL` keys, and `save` merges with whatever
    another process wrote meanwhile rather than overwriting it.
    """

    def __init__(self, base: str, directory: Path) -> None:
        self.base = base
        self.base_commit = git("rev-parse", "--verify", f"{base}^{{commit}}").strip()
        self.directory = directory
        self.path = directory / ASSIGNMENT_CACHE_FILE
        tools = Path(__file__).resolve().parent
        code = hashlib.sha256()
        for name in _CACHE_CODE_FILES:
            code.update(name.encode() + b"\0" + (tools / name).read_bytes())
        self.code = code.hexdigest()
        self.blobs: dict[str, str] = {}
        listing = git(
            "ls-tree", "-r", "--full-tree", self.base_commit, "--",
            "src", str(PurePosixPath(finalize_plateau.HANDOFF_SHARD_DIR)),
            REOPEN_AUTHORIZATIONS_PATH, LEGACY_TRIAGE_PATH,
        )
        for line in listing.splitlines():
            meta, _, name = line.partition("\t")
            fields = meta.split()
            if len(fields) == 3 and fields[1] == "blob":
                self.blobs[name] = fields[2]
        self.anchors: dict[str, str | None] = {}
        self.aliases = build_fallback_aliases(self.base_commit)
        self.authorization_rows: dict[str, dict] = {}
        self.base_term = self._authorization_base_term()
        self.entries: dict[str, dict] = {}
        self.order: dict[str, list[str]] = {}
        self.validity: dict[str, list] = {}
        self._validity: str | None = None
        self.hits = self.misses = 0
        self.dirty = False
        self._load()

    def _load(self) -> None:
        try:
            document = json.loads(self.path.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            return
        if (
            isinstance(document, dict)
            and document.get("schema") == ASSIGNMENT_CACHE_SCHEMA
            and isinstance(document.get("entries"), dict)
            and isinstance(document.get("order"), dict)
        ):
            self.entries = document["entries"]
            self.order = document["order"]
            validity = document.get("validity")
            self.validity = validity if isinstance(validity, dict) else {}

    def _authorization_base_term(self) -> str | None:
        """None when the authorization anchor implies every pin's ancestry."""
        path = REOPEN_AUTHORIZATIONS_PATH
        if path not in self.blobs:
            return None
        anchor = self.anchor(path)
        try:
            rows = parse_reopen_authorizations(
                show_file(self.base_commit, path) or "", label=path,
            )
        except RuntimeError:
            return self.base_commit
        self.authorization_rows = rows
        pins = sorted({
            pin for row in rows.values()
            for pin in (row["source_commit"], row["ledger_commit"])
            if pin is not None
        })
        if not pins or anchor is None:
            return None if not pins else self.base_commit
        result = subprocess.run(
            ["git", "rev-list", "--count", *pins, "--not", anchor],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        )
        if result.returncode != 0 or result.stdout.strip() != "0":
            return self.base_commit
        return None

    def authorization_validity(self) -> str:
        """The authorization file's validation verdict at this base.

        ``"valid"``, or ``"invalid: <error>"``. Stored per (schema, code,
        file blob, base term), so it is re-proved only when one of those moves.
        """
        if self._validity is not None:
            return self._validity
        material = json.dumps([
            ASSIGNMENT_CACHE_SCHEMA, self.code,
            self.blobs.get(REOPEN_AUTHORIZATIONS_PATH), self.base_term,
        ])
        key = hashlib.sha256(material.encode()).hexdigest()
        stored = self.validity.get(key)
        if isinstance(stored, list) and len(stored) == 2:
            verdict = stored[1]
        else:
            try:
                reopen_authorizations(self.base)
                verdict = "valid"
            except RuntimeError as error:
                verdict = f"invalid: {error}"
        self.validity = {key: [0, verdict], **{
            k: [v[0] + 1, v[1]] for k, v in self.validity.items()
            if k != key and isinstance(v, list) and len(v) == 2 and v[0] < 3
        }}
        if stored is None or stored != [0, verdict]:
            self.dirty = True
        self._validity = verdict
        return verdict

    def anchor(self, path: str) -> str | None:
        if path not in self.anchors:
            self.anchors[path] = path_anchor(self.base_commit, path)
        return self.anchors[path]

    def prefetch(self, pairs: list[tuple[str, str]], *, jobs: int = 8) -> None:
        """Resolve the anchors `key` needs for (symbol, source path) pairs.

        One ``git log -1`` per path, run in parallel; a shard that does not
        exist at the base needs none.
        """
        wanted = {LEGACY_TRIAGE_PATH}
        for symbol, path in pairs:
            wanted.add(path)
            shard = shard_path(symbol)
            if shard in self.blobs:
                wanted.add(shard)
        wanted = {
            path for path in wanted
            if path not in self.anchors and path in self.blobs
        }
        if not wanted:
            return
        with concurrent.futures.ThreadPoolExecutor(max(1, jobs)) as pool:
            ordered = sorted(wanted)
            for path, value in zip(
                ordered,
                pool.map(lambda p: path_anchor(self.base_commit, p), ordered),
            ):
                self.anchors[path] = value

    def _file_terms(self, path: str) -> list[str | None]:
        blob = self.blobs.get(path)
        return [path, blob, self.anchor(path) if blob is not None else None]

    def key(self, symbol: str, path: str) -> str:
        material = json.dumps([
            ASSIGNMENT_CACHE_SCHEMA, self.code, symbol,
            *self._file_terms(path),
            *self._file_terms(shard_path(symbol)),
            *self._file_terms(LEGACY_TRIAGE_PATH),
            self.authorization_rows.get(symbol),
            self.authorization_validity(),
            sorted(self.aliases.get((path, symbol), frozenset())),
            self.base_term,
        ])
        return hashlib.sha256(material.encode()).hexdigest()

    def get(self, key: str) -> dict | None:
        record = self.entries.get(key)
        if record is None:
            self.misses += 1
        else:
            self.hits += 1
        return record

    def put(self, key: str, record: dict, symbol: str) -> None:
        self.entries[key] = record
        keys = [k for k in self.order.get(symbol, []) if k != key]
        self.order[symbol] = [key, *keys]
        self.dirty = True

    def touch(self, symbol: str, key: str) -> None:
        """Record that ``key`` is ``symbol``'s newest key, for pruning."""
        keys = self.order.get(symbol, [])
        if keys[:1] != [key]:
            self.order[symbol] = [key, *(k for k in keys if k != key)]
            self.dirty = True

    def save(self) -> None:
        if not self.dirty:
            return
        # Merge with what another process may have saved since we loaded.
        on_disk = AssignmentCache.__new__(AssignmentCache)
        on_disk.path, on_disk.entries, on_disk.order = self.path, {}, {}
        on_disk.validity = {}
        on_disk._load()
        entries = dict(on_disk.entries, **self.entries)
        order: dict[str, list[str]] = {}
        for symbol in sorted(set(on_disk.order) | set(self.order)):
            mine = self.order.get(symbol, [])
            theirs = on_disk.order.get(symbol, [])
            merged = list(dict.fromkeys(
                k for k in (*mine, *theirs) if k in entries
            ))
            order[symbol] = merged[:ASSIGNMENT_CACHE_KEYS_PER_SYMBOL]
        validity = dict(on_disk.validity, **self.validity)
        validity = dict(sorted(
            validity.items(), key=lambda item: item[1][0],
        )[:ASSIGNMENT_CACHE_KEYS_PER_SYMBOL])
        live = {k for keys in order.values() for k in keys}
        entries = {k: v for k, v in entries.items() if k in live}
        self.directory.mkdir(parents=True, exist_ok=True)
        temporary = self.path.with_suffix(f".{os.getpid()}.tmp")
        temporary.write_text(json.dumps({
            "schema": ASSIGNMENT_CACHE_SCHEMA,
            "entries": entries,
            "order": order,
            "validity": validity,
        }, sort_keys=True), encoding="utf-8")
        os.replace(temporary, self.path)
        # Schema-1 files were named for a base commit; nothing reads them.
        for stale in self.directory.glob("*.json"):
            if stale != self.path:
                try:
                    stale.unlink()
                except OSError:
                    pass
        self.entries, self.order = entries, order
        self.dirty = False


def default_cache_dir() -> Path:
    return Path(git("rev-parse", "--show-toplevel").strip()) / ASSIGNMENT_CACHE_DIR


def cached_assignment_status(
    base: str, symbol: str, *,
    identity: tuple[str | None, str | None],
    lane_index: LanePathIndex | None,
    cache: AssignmentCache,
) -> Assignment:
    """`assignment_status`, with phases 1 and 3 served from `cache`.

    Returns exactly what `assignment_status` returns for the same inputs;
    tools/test_lane_status.py holds that equivalence.
    """
    path, identity_error = identity
    if identity_error or path is None:
        return assignment_status(
            base, symbol, identity=identity, lane_index=lane_index,
        )
    key = cache.key(symbol, path)
    record = cache.get(key)
    if record is not None:
        cache.touch(symbol, key)
    if record is not None and record.get("early") is not None:
        return Assignment(**record["early"])
    text: str | None = None
    if record is None:
        early, text, current_blob, base_source_commit = _pre_active_status(
            base, symbol, path, None,
        )
        if early is not None:
            cache.put(key, {"early": asdict(early)}, symbol)
            return early
    else:
        current_blob = record["source_blob"]
        base_source_commit = record["base_source_commit"]
    active = _active_status(
        base, symbol, path, current_blob, base_source_commit, text, lane_index,
    )
    if record is None:
        try:
            settled = _settled_status(base, symbol, path, text, base_source_commit)
        except Exception:
            # The uncached path never reaches phase 3 for an owned target, so
            # an error there must not replace an `active` verdict.
            if active is not None:
                return active
            raise
        record = {
            "early": None, "source_blob": current_blob,
            "base_source_commit": base_source_commit,
            "settled": asdict(settled),
        }
        cache.put(key, record, symbol)
    if active is not None:
        return active
    return Assignment(**record["settled"])


@dataclass(frozen=True)
class AssignmentContext:
    """Shared immutable evidence for many assignment classifications.

    Pass ``cache`` to `build` to serve the base-derived phases from an
    `AssignmentCache`; call `save` when done so the next run is warm.
    """

    base: str
    identities: dict[str, tuple[str | None, str | None]]
    lane_index: LanePathIndex
    cache: AssignmentCache | None = None

    @classmethod
    def build(
        cls, base: str, symbols: list[str], *, jobs: int = 1,
        cache: AssignmentCache | None = None,
    ) -> "AssignmentContext":
        identities = source_identity_index(base, symbols)
        if cache is not None:
            cache.prefetch([
                (symbol, path) for symbol, (path, error) in identities.items()
                if path is not None and error is None
            ], jobs=max(jobs, 8))
        return cls(
            base=base,
            identities=identities,
            lane_index=build_lane_path_index(base, symbols, jobs=jobs),
            cache=cache,
        )

    def classify(self, base: str, symbol: str) -> Assignment:
        if base != self.base:
            raise RuntimeError(
                f"assignment context is for {self.base}, expected {base}"
            )
        identity = self.identities.get(symbol)
        if identity is None:
            raise RuntimeError(f"assignment context does not contain {symbol}")
        if self.cache is not None:
            return cached_assignment_status(
                base, symbol, identity=identity, lane_index=self.lane_index,
                cache=self.cache,
            )
        return assignment_status(
            base, symbol, identity=identity, lane_index=self.lane_index,
        )

    def save(self) -> None:
        if self.cache is not None:
            self.cache.save()


def assignment_status(
    base: str, symbol: str, *,
    identity: tuple[str | None, str | None] | None = None,
    lane_index: LanePathIndex | None = None,
) -> Assignment:
    """Classify whether one exact target is safe to assign.

    Only ``base-only`` is assignable. Every other state is deliberately
    fail-closed so stale evidence cannot become duplicate matching work.

    The classification runs in three phases, split so that `AssignmentCache`
    can store the two that depend on the base commit alone:

    1. `_pre_active_status` -- source identity, blob and target history commit;
    2. `_active_status` -- unintegrated lane ownership, which depends on lane
       refs as well as the base, and so is never cached;
    3. `_settled_status` -- plateau, ledger and reopen-pin evidence.
    """
    path, identity_error = identity or source_identity(base, symbol)
    early, text, current_blob, base_source_commit = _pre_active_status(
        base, symbol, path, identity_error,
    )
    if early is not None:
        return early
    active = _active_status(
        base, symbol, path, current_blob, base_source_commit, text, lane_index,
    )
    if active is not None:
        return active
    return _settled_status(base, symbol, path, text, base_source_commit)


def _pre_active_status(
    base: str, symbol: str, path: str | None, identity_error: str | None,
) -> tuple[Assignment | None, str | None, str | None, str | None]:
    """Phase 1: return (early verdict, source text, source blob, source commit)."""
    if identity_error or path is None:
        return Assignment(
            symbol, "stale-ledger", path, None, None, [],
            identity_error or "source identity is unavailable",
            "source-identity",
        ), None, None, None
    text = show_file(base, path)
    current_blob = blob_id(base, path)
    if text is None or current_blob is None:
        return Assignment(
            symbol, "stale-ledger", path, None, None, [],
            "exact source path is absent from the base object",
            "source-missing",
        ), text, current_blob, None

    base_source_commit = target_history_commit(
        base, symbol, [path], require_plateau=False,
    ) or latest_path_commit(base, path)
    if base_source_commit is None:
        return Assignment(
            symbol, "stale-ledger", path, None, None, [],
            "exact source path has no committed history",
            "history-missing",
        ), text, current_blob, None
    return None, text, current_blob, base_source_commit


def _active_status(
    base: str, symbol: str, path: str, current_blob: str,
    base_source_commit: str, text: str | None,
    lane_index: LanePathIndex | None,
) -> Assignment | None:
    """Phase 2: the `active` verdict, or None when no lane owns the target."""
    active = active_lanes_for_source(
        base, symbol, path, current_blob, base_source_commit, text,
        lane_index, claim_dispositions(base),
    )
    if active:
        return Assignment(
            symbol, "active", path, None, None, active,
            "an unintegrated lane has a different committed target guard or handoff",
            "lane-owned",
        )
    return None


def _settled_status(
    base: str, symbol: str, path: str, text: str | None,
    base_source_commit: str,
) -> Assignment:
    """Phase 3: the verdict for a target no unintegrated lane owns."""
    if text is None:
        text = show_file(base, path)
        if text is None:
            raise RuntimeError(f"{base}:{path} vanished during classification")
    try:
        reopen_authorization = reopen_authorizations(base).get(symbol)
    except RuntimeError as error:
        return Assignment(
            symbol, "stale-ledger", path, None, None, [], str(error),
            "reopen-authorization-invalid",
        )

    if not guarded_fallback(
        text, symbol, source_fallback_aliases(base, path, symbol),
    ):
        return Assignment(
            symbol, "already-integrated/exhausted", path, None, None, [],
            "base has a committed definition without this target's fallback",
            "matched-or-promoted",
        )

    source_record = target_history_record(
        base, symbol, [path], require_plateau=False,
    )
    named_plateau_record = (
        source_record
        if source_record and PLATEAU_SUBJECT_RE.search(source_record[1])
        else None
    )
    # Historical standalone candidate files often predate the structured EOF
    # handoff marker and use friendly names only in commit subjects. A plateau
    # or park commit on a path containing exactly one NON_MATCHING guard is
    # still unambiguous target evidence; treating it as fresh work caused the
    # ready queue to reassign already exhausted o57/o79/o22 routes.
    guard_count = len(re.findall(r"#\s*ifdef\s+NON_MATCHING\b", text))
    path_record = (
        path_plateau_record(base, path)
        if guard_count == 1
        else None
    )
    triage_text = show_file(base, LEGACY_TRIAGE_PATH)
    if malformed_legacy_marker(triage_text, symbol):
        return Assignment(
            symbol, "stale-ledger", path, None,
            latest_legacy_evidence_commit(base, symbol), [],
            "malformed target-specific handoff block in the legacy triage ledger",
            "legacy-invalid",
        )
    ledger_rows = exact_symbol_rows(triage_text, symbol)
    legacy_has_symbol = bool(ledger_rows)
    legacy_marks_plateau = any(
        PLATEAU_SUBJECT_RE.search(line) for line in ledger_rows
    )
    legacy_source_record = (
        ledger_source_plateau_record(base, symbol, path)
        if guard_count > 1 and legacy_marks_plateau
        else None
    )
    target_shard_path = shard_path(symbol)
    shard_text = show_file(base, target_shard_path)
    try:
        shard_source = validated_shard_source(shard_text, symbol)
    except RuntimeError as error:
        return Assignment(
            symbol, "stale-ledger", path, None,
            latest_path_commit(base, target_shard_path), [], str(error),
            "shard-invalid",
        )
    if shard_source is not None and shard_source != path:
        return Assignment(
            symbol, "stale-ledger", path, None,
            latest_path_commit(base, target_shard_path), [],
            f"symbol handoff shard records source {shard_source}, expected {path}",
            "shard-source-mismatch",
        )
    shard_record = (
        shard_source_plateau_record(base, symbol, path)
        if shard_source is not None
        else None
    )
    if (
        not has_plateau_handoff(text, symbol)
        and named_plateau_record is None
        and path_record is None
        and legacy_source_record is None
        and shard_source is None
    ):
        return Assignment(
            symbol, "base-only", path, None, None, [],
            "base retains the fallback and has no committed plateau handoff",
            "ready",
        )

    source_commit = (
        named_plateau_record[0]
        if named_plateau_record is not None
        else path_record[0]
        if path_record is not None
        else legacy_source_record[0]
        if legacy_source_record is not None
        else shard_record[0]
        if shard_record is not None
        else None
    )
    ledger_commits = []
    if shard_source is not None:
        shard_commit = latest_path_commit(base, target_shard_path)
        if shard_commit is not None:
            ledger_commits.append(shard_commit)
    if legacy_has_symbol:
        legacy_commit = latest_legacy_evidence_commit(base, symbol)
        if legacy_commit is not None:
            ledger_commits.append(legacy_commit)
    current_ledger_commits = (
        [commit for commit in ledger_commits if is_ancestor(source_commit, commit)]
        if source_commit is not None
        else []
    )
    ledger_commit = newest_related_commit(
        current_ledger_commits or ledger_commits
    )
    if source_commit is None:
        return Assignment(
            symbol, "stale-ledger", path, None, ledger_commit, [],
            "source has a plateau handoff but no target-named plateau commit",
            "prose-needs-remeasurement",
        )
    if ledger_commit is None:
        if reopen_authorization is not None:
            authorized_source = reopen_authorization["source_commit"]
            authorized_ledger = reopen_authorization["ledger_commit"]
            latest_authorizable_source = base_source_commit
            if guard_count == 1:
                latest_authorizable_source = (
                    latest_path_commit(base, path) or base_source_commit
                )
            if (
                authorized_source == latest_authorizable_source
                and authorized_ledger is None
                and is_ancestor(source_commit, authorized_source)
            ):
                return Assignment(
                    symbol, "base-only", path, source_commit, None, [],
                    "source plateau without structured handoff is explicitly "
                    "reopened for one authenticated maintenance pass: "
                    f"{reopen_authorization['reason']}",
                    "authorized-reopen",
                )
            if authorized_ledger is None:
                return Assignment(
                    symbol, "stale-ledger", path, source_commit, None, [],
                    "null-ledger reopen authorization is stale because the "
                    "latest target guard or evidence commit no longer matches "
                    "its pinned source commit",
                    "reopen-authorization-stale",
                )
        return Assignment(
            symbol, "stale-ledger", path, source_commit, None, [],
            "source plateau is missing exact-symbol handoff ledger evidence",
            "prose-needs-remeasurement",
        )
    if not is_ancestor(source_commit, ledger_commit):
        if reopen_authorization is not None:
            authorized_source = reopen_authorization["source_commit"]
            authorized_ledger = reopen_authorization["ledger_commit"]
            if (
                source_commit == authorized_source
                and ledger_commit == authorized_ledger
            ):
                return Assignment(
                    symbol, "base-only", path, source_commit, ledger_commit, [],
                    "stale structured evidence is explicitly reopened for one "
                    "authenticated maintenance pass: "
                    f"{reopen_authorization['reason']}",
                    "authorized-reopen",
                )
        return Assignment(
            symbol, "stale-ledger", path, source_commit, ledger_commit, [],
            "triage evidence predates the committed source plateau",
            "stale-structured-evidence",
        )
    if reopen_authorization is not None:
        authorized_source = reopen_authorization["source_commit"]
        authorized_ledger = reopen_authorization["ledger_commit"]
        if (
            source_commit == authorized_source
            and ledger_commit == authorized_ledger
        ):
            return Assignment(
                symbol, "base-only", path, source_commit, ledger_commit, [],
                "current plateau is explicitly reopened for one authenticated "
                f"mechanism: {reopen_authorization['reason']}",
                "authorized-reopen",
            )
        return Assignment(
            symbol, "already-integrated/exhausted", path, source_commit,
            ledger_commit, [],
            "reopen authorization is stale because the current source or "
            "handoff commit no longer matches its pinned pair",
            "reopen-authorization-stale",
        )
    return Assignment(
        symbol, "already-integrated/exhausted", path, source_commit,
        ledger_commit, [], "source plateau and triage evidence are current",
        "current-plateau",
    )


def unique_commits(branch: str, base: str) -> list[tuple[str, str, str]]:
    raw = git(
        "log", "--no-merges", "--format=%H%x00%cI%x00%s%x1e",
        branch, "--not", base,
    )
    rows = []
    for record in raw.split("\x1e"):
        record = record.strip("\n")
        if not record:
            continue
        commit, committed_at, subject = record.split("\0", 2)
        rows.append((commit, committed_at, subject.strip()))
    return rows


@lru_cache(maxsize=128)
def claim_dispositions(base: str) -> dict[str, dict[str, str]]:
    raw = git("show", f"{base}:{DISPOSITIONS_PATH}", check=False)
    if not raw:
        return {}
    try:
        document = json.loads(raw)
    except json.JSONDecodeError as error:
        raise RuntimeError(f"{base}:{DISPOSITIONS_PATH}: {error}") from error
    if document.get("schema_version") != 1:
        raise RuntimeError(f"{base}:{DISPOSITIONS_PATH}: unsupported schema")
    claims = document.get("claims")
    if not isinstance(claims, dict):
        raise RuntimeError(f"{base}:{DISPOSITIONS_PATH}: claims must be an object")
    for commit, row in claims.items():
        if not re.fullmatch(r"[0-9a-f]{40}", commit):
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: invalid commit {commit!r}"
            )
        if not isinstance(row, dict):
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: {commit} must be an object"
            )
        if row.get("state") not in {"rejected", "superseded"}:
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: {commit} has invalid state"
            )
        if not isinstance(row.get("symbol"), str) or not row["symbol"]:
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: {commit} needs a symbol"
            )
        if not isinstance(row.get("reason"), str) or not row["reason"]:
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: {commit} needs a reason"
            )
        decision_commit = row.get("decision_commit")
        if not isinstance(decision_commit, str) or not re.fullmatch(
            r"[0-9a-f]{40}", decision_commit
        ):
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: {commit} needs a full "
                "decision_commit"
            )
        decision_is_ancestor = subprocess.run(
            ["git", "merge-base", "--is-ancestor", decision_commit, base],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if decision_is_ancestor.returncode != 0:
            raise RuntimeError(
                f"{base}:{DISPOSITIONS_PATH}: decision {decision_commit} "
                f"is not an ancestor of {base}"
            )
    return claims


def parse_reopen_authorizations(
    raw: str, *, label: str,
) -> dict[str, dict[str, str | None]]:
    """Validate structure only; pins need not exist or authorize assignment."""
    try:
        document = json.loads(raw)
    except json.JSONDecodeError as error:
        raise RuntimeError(
            f"{label}: {error}"
        ) from error
    if not isinstance(document, dict) or set(document) != {
        "schema_version", "authorizations",
    }:
        raise RuntimeError(
            f"{label}: invalid top-level schema"
        )
    if type(document["schema_version"]) is not int or document["schema_version"] != 1:
        raise RuntimeError(
            f"{label}: schema_version must be the integer 1"
        )
    authorizations = document["authorizations"]
    if not isinstance(authorizations, dict):
        raise RuntimeError(
            f"{label}: authorizations must be an object"
        )
    expected_fields = {"source_commit", "ledger_commit", "reason"}
    for symbol, row in authorizations.items():
        if not isinstance(symbol, str) or not re.fullmatch(
            r"[A-Za-z_][A-Za-z0-9_]*", symbol
        ):
            raise RuntimeError(
                f"{label}: invalid symbol {symbol!r}"
            )
        if not isinstance(row, dict) or set(row) != expected_fields:
            raise RuntimeError(
                f"{label}: {symbol} has invalid fields"
            )
        source_commit = row["source_commit"]
        if not isinstance(source_commit, str) or not re.fullmatch(
            r"[0-9a-f]{40}", source_commit
        ):
            raise RuntimeError(
                f"{label}: {symbol} needs a "
                "full source_commit"
            )
        ledger_commit = row["ledger_commit"]
        if ledger_commit is not None and (
            not isinstance(ledger_commit, str)
            or not re.fullmatch(r"[0-9a-f]{40}", ledger_commit)
        ):
            raise RuntimeError(
                f"{label}: {symbol} needs a full "
                "ledger_commit or null"
            )
        reason = row["reason"]
        if (
            not isinstance(reason, str) or not reason.strip()
            or len(reason) > 240 or "\n" in reason or "|" in reason
        ):
            raise RuntimeError(
                f"{label}: {symbol} needs one nonempty reason of at most "
                "240 characters, without newline or pipe"
            )
    return authorizations


@lru_cache(maxsize=128)
def reopen_authorizations(base: str) -> dict[str, dict[str, str | None]]:
    """Load commit-pinned, one-shot authorizations for current plateaus."""
    raw = git("show", f"{base}:{REOPEN_AUTHORIZATIONS_PATH}", check=False)
    if not raw:
        return {}
    authorizations = parse_reopen_authorizations(
        raw, label=f"{base}:{REOPEN_AUTHORIZATIONS_PATH}",
    )
    for symbol, row in authorizations.items():
        source_commit = row["source_commit"]
        ledger_commit = row["ledger_commit"]
        if not is_ancestor(source_commit, base):
            raise RuntimeError(
                f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} source_commit "
                f"{source_commit} is not an ancestor of {base}"
            )
        if ledger_commit is not None:
            if not is_ancestor(ledger_commit, base):
                raise RuntimeError(
                    f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} "
                    f"ledger_commit {ledger_commit} is not an ancestor of {base}"
                )
            if not (
                is_ancestor(source_commit, ledger_commit)
                or is_ancestor(ledger_commit, source_commit)
            ):
                raise RuntimeError(
                    f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} "
                    "source_commit and ledger_commit are unrelated"
                )
        source_path, identity_error = source_identity(source_commit, symbol)
        if identity_error or source_path is None:
            raise RuntimeError(
                f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} source_commit "
                "does not identify exactly one source definition"
            )
        if ledger_commit is not None:
            shard_text = show_file(ledger_commit, shard_path(symbol))
            try:
                shard_source = validated_shard_source(shard_text, symbol)
            except RuntimeError as error:
                raise RuntimeError(
                    f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} "
                    f"ledger_commit has invalid handoff evidence: {error}"
                ) from error
            if shard_source is None:
                # No shard at that commit. `classify` derives a ledger from a
                # structured shard OR from legacy inline evidence, so demanding
                # a shard here rejects a ledger the deriver itself produced,
                # and the symbol becomes structurally unauthorizable: the
                # null-ledger branch is only reached when the DERIVED ledger is
                # null, which it is not. That contradiction blocked
                # func_8003E8D8 -- 560 bytes at one differing word -- until
                # 2026-09-09. Accept a legacy-evidence ledger by the same test
                # the deriver used.
                rows, blocks = legacy_evidence_signature(
                    show_file(ledger_commit, LEGACY_TRIAGE_PATH), symbol,
                )
                if not rows and not blocks:
                    raise RuntimeError(
                        f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} "
                        "ledger_commit carries neither a handoff shard nor "
                        "legacy evidence for this symbol"
                    )
            elif shard_source != source_path:
                raise RuntimeError(
                    f"{base}:{REOPEN_AUTHORIZATIONS_PATH}: {symbol} "
                    "ledger_commit does not identify the authorized source path"
                )
    return authorizations


def collect(base: str, symbol_filter: str | None) -> list[Lane]:
    git("rev-parse", "--verify", base)
    dispositions = claim_dispositions(base)
    lanes = []
    for branch, head in lane_refs(unmerged_into=base):
        commits = unique_commits(branch, base)
        if not commits:
            continue
        claims = []
        for commit, committed_at, subject in commits:
            match = MATCH_RE.match(subject)
            if not match:
                continue
            symbol = match.group(1)
            if symbol_filter and symbol != symbol_filter:
                continue
            reason = None
            decision_commit = None
            disposition = dispositions.get(commit)
            if disposition:
                if disposition["symbol"] != symbol:
                    raise RuntimeError(
                        f"{base}:{DISPOSITIONS_PATH}: {commit} names "
                        f"{disposition['symbol']}, subject names {symbol}"
                    )
                state = disposition["state"]
                reason = disposition["reason"]
                decision_commit = disposition["decision_commit"]
            else:
                base_fallback = has_global_asm(base, symbol)
                lane_fallback = has_global_asm(branch, symbol)
                # Friendly C names and splat's generated fallback names can
                # differ. A match commit records the authoritative association
                # by deleting the exact GLOBAL_ASM path, so use that path when
                # the friendly-name probe cannot see the fallback.
                fallback_paths = removed_global_asms(commit)
                if not base_fallback and fallback_paths:
                    base_fallback = any(
                        has_text(base, f'GLOBAL_ASM("{path}")')
                        for path in fallback_paths
                    )
                    lane_fallback = any(
                        has_text(branch, f'GLOBAL_ASM("{path}")')
                        for path in fallback_paths
                    )
                if base_fallback and not lane_fallback:
                    state = "pending"
                elif not base_fallback:
                    state = "already-in-base"
                else:
                    state = "claim-only"
            claims.append(Claim(
                symbol=symbol,
                commit=commit,
                committed_at=committed_at,
                subject=subject,
                state=state,
                source_paths=source_paths(commit),
                reason=reason,
                decision_commit=decision_commit,
            ))
        if symbol_filter and not claims:
            continue
        lanes.append(Lane(
            branch=branch,
            head=head,
            ahead=len(commits),
            claims=claims,
        ))
    return sorted(lanes, key=lambda lane: lane.branch)


def print_assignment(assignment: Assignment) -> None:
    details = []
    if assignment.source_path:
        details.append(f"source={assignment.source_path}")
    if assignment.source_commit:
        details.append(f"source-commit={assignment.source_commit[:12]}")
    if assignment.ledger_commit:
        details.append(f"ledger-commit={assignment.ledger_commit[:12]}")
    if assignment.active_lanes:
        details.append("lanes=" + ",".join(assignment.active_lanes))
    suffix = " " + " ".join(details) if details else ""
    print(
        f"assignment: {assignment.state} {assignment.symbol}{suffix}\n"
        f"  reason: {assignment.reason}"
    )


def print_text(
    base: str, lanes: list[Lane], pending_only: bool,
    assignment: Assignment | None,
) -> None:
    print(f"base: {base}")
    if assignment:
        print_assignment(assignment)
    for lane in lanes:
        claims = [c for c in lane.claims if not pending_only or c.state == "pending"]
        if pending_only and not claims:
            continue
        print(f"{lane.branch} {lane.head[:12]} ahead={lane.ahead}")
        for claim in claims:
            paths = ",".join(claim.source_paths) or "-"
            print(
                f"  {claim.state:15s} {claim.symbol:42s} "
                f"{claim.commit[:12]} {paths}"
            )
            if claim.reason:
                print(
                    f"    decision: {claim.decision_commit[:12]} "
                    f"reason: {claim.reason}"
                )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--base",
        help="canonical Git ref (default: freshest linear integration ref)",
    )
    parser.add_argument("--symbol", help="Show claims for one exact symbol")
    parser.add_argument(
        "--symbols",
        help=(
            "Screen many symbols under ONE shared evidence scan: a comma-"
            "separated list, or '-' to read one symbol per line from stdin. "
            "Prints 'verdict<TAB>symbol<TAB>source' per line. Exits 0 if every "
            "symbol is base-only, 1 otherwise. Per-symbol --symbol calls each "
            "rebuild the whole lane index, which is why screening a translation "
            "unit that way costs minutes; this shares one index across all of "
            "them."
        ),
    )
    parser.add_argument(
        "--no-cache", action="store_true",
        help=(
            "with --symbols: classify without the content-keyed assignment "
            "cache under build/cache/lane-assignment/ (neither read nor "
            "written). The cache serves only the base-derived phases; lane "
            "ownership is recomputed every run either way."
        ),
    )
    parser.add_argument("--pending-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    parser.add_argument(
        "--check-reopen-schema", action="store_true",
        help="check worktree reopen JSON structure only, without Git/history checks",
    )
    args = parser.parse_args()

    if args.check_reopen_schema:
        if args.base is not None or args.symbol or args.pending_only or args.json:
            parser.error("--check-reopen-schema cannot be combined with assignment options")
        path = Path(REOPEN_AUTHORIZATIONS_PATH)
        try:
            rows = parse_reopen_authorizations(
                path.read_text(encoding="utf-8"), label=str(path),
            )
        except (OSError, UnicodeError, RuntimeError) as error:
            print(f"lane_status: {error}", file=sys.stderr)
            return 2
        print(f"reopen schema: OK ({len(rows)} entries; not assignment authorization)")
        return 0

    if args.symbols is not None:
        if args.symbols.strip() == "-":
            wanted = [line.strip() for line in sys.stdin if line.strip()]
        else:
            wanted = [name.strip() for name in args.symbols.split(",") if name.strip()]
        if not wanted:
            print("lane_status: --symbols named nothing", file=sys.stderr)
            return 2
        try:
            if args.base is None:
                args.base = integration_base.resolve(Path.cwd())
            cache = (
                None if args.no_cache
                else AssignmentCache(args.base, default_cache_dir())
            )
            context = AssignmentContext.build(
                args.base, wanted, jobs=getattr(args, "jobs", 4), cache=cache)
        except RuntimeError as error:
            print(f"lane_status: {error}", file=sys.stderr)
            return 2
        results = []
        for name in wanted:
            try:
                results.append(context.classify(args.base, name))
            except RuntimeError as error:
                print(f"lane_status: {name}: {error}", file=sys.stderr)
                return 2
        context.save()
        if cache is not None:
            print(f"lane_status: assignment cache {cache.hits} hit(s), "
                  f"{cache.misses} miss(es)", file=sys.stderr)
        if args.json:
            print(json.dumps(
                {"base": args.base,
                 "assignments": [asdict(item) for item in results]},
                indent=2, sort_keys=True))
        else:
            for item in results:
                print(f"{item.state}\t{item.symbol}\t{item.source_path or ''}")
        return 0 if all(item.state == "base-only" for item in results) else 1

    try:
        if args.base is None:
            args.base = integration_base.resolve(Path.cwd())
        lanes = collect(args.base, args.symbol)
        assignment = assignment_status(args.base, args.symbol) if args.symbol else None
    except RuntimeError as error:
        print(f"lane_status: {error}", file=sys.stderr)
        return 2

    if args.pending_only:
        lanes = [
            Lane(
                branch=lane.branch,
                head=lane.head,
                ahead=lane.ahead,
                claims=[claim for claim in lane.claims if claim.state == "pending"],
            )
            for lane in lanes
        ]
        lanes = [lane for lane in lanes if lane.claims]

    if args.json:
        print(json.dumps({
            "base": args.base,
            "assignment": asdict(assignment) if assignment else None,
            "lanes": [asdict(lane) for lane in lanes],
        }, indent=2, sort_keys=True))
    else:
        print_text(args.base, lanes, args.pending_only, assignment)
    if assignment and assignment.state != "base-only":
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
