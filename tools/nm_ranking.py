#!/usr/bin/env python3
"""Ranks every `#ifdef NON_MATCHING` function by how close its candidate C
is to the ROM target, so a fleet of workers can attack the near-misses
first instead of triaging ~300 KB of queued functions by hand.

Two independent sources feed the ranking:

1. A configured full-TU compile, the primary source. The tool expands the
   actual NON_MATCHING=1 Makefile command, compiles the original source once
   per TU, and redirects only the output object. Every compiler argument,
   relative include, source filename, and source line remains intact.
   POSTPROCESS is excluded. Each queued symbol is then compared against its
   own extracted target assembly. No isolated import or historical declaration
   substitute is accepted as a current-source measurement. Function spans
   and relocation offsets are normalized inside their objects; linked ROM
   verification remains a separate gate.

2. objdiff-cli's per-function fuzzy_match_percent, read from a report
   generated over a real `gmake NON_MATCHING=1` build tree
   (build_non_matching/) diffed against the verified `expected/build/`
   snapshot -- supplementary context (it reflects the function in its real
   linked/whole-TU context, not isolated), and only available for the
   objects objdiff-cli's ELF reader can parse and that the NON_MATCHING
   build actually produced. Pass its report with --objdiff-report; omit it
   and every function's objdiff_match_pct is simply null.

No instruction words, mnemonics, or hex ever leave this tool: every
decoded instruction word lives only in memory for one comparison and is
reduced to raw and relocation-masked counts/first-mismatch offsets before
anything is written to config/nonmatching-ranking.us.json or printed.

Usage:

    # after building both trees (see docs/nm-ranking.md):
    .venv/bin/python tools/nm_ranking.py \\
        --objdiff-report /path/to/objdiff-report.json

    # top 20 retained rows as a markdown table, with no compilation:
    .venv/bin/python tools/nm_ranking.py --show-retained --top 20 --markdown

    # remove now-matched rows from the retained snapshot without compiling:
    .venv/bin/python tools/nm_ranking.py --prune-stale

    # compile and merge only stale/new/unresolved rows:
    .venv/bin/python tools/nm_ranking.py --refresh-stale --jobs 2
"""
from __future__ import annotations

import argparse
import base64
import concurrent.futures
import dataclasses
import difflib
import hashlib
import json
import math
import os
import pathlib
import re
import shlex
import stat
import struct
import subprocess
import sys
import tempfile
from collections import Counter
from typing import Optional

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import permute_batch as pb  # noqa: E402

OBJDUMP = ROOT / "tools" / "binutils" / "mips64-elf-objdump"
OBJCOPY = ROOT / "tools" / "binutils" / "mips64-elf-objcopy"
WORK_DIR = ROOT / "build" / "nm_ranking"
DEFAULT_OUT = ROOT / "config" / "nonmatching-ranking.us.json"
DEFAULT_DOC = ROOT / "docs" / "nm-ranking.md"
DOC_BEGIN = "<!-- NM_RANKING_GENERATED_BEGIN -->"
DOC_END = "<!-- NM_RANKING_GENERATED_END -->"
SCHEMA_VERSION = 3
SOURCE_CONTEXT_VERSION = 5
SOURCE_CONTEXT_FIELD = "source_context_sha256"
HEX_SHA256_RE = re.compile(r"[0-9a-f]{64}")
BASE64URL_SHA256_RE = re.compile(r"[A-Za-z0-9_-]{43}")
GROUPED_BASE64URL_SHA256_RE = re.compile(
    r"(?:[A-Za-z0-9_-]{4}\.){10}[A-Za-z0-9_-]{3}"
)
BLAME_HEADER_RE = re.compile(r"^\^?([0-9a-f]{40})\s+\d+\s+\d+")

CATEGORY_RANK = {
    "register-only": 0,
    "schedule-only": 1,
    "other": 2,
    "reloc-mismatch": 3,
    "size-mismatch": 4,
}

SYM_RE = re.compile(
    r"^(?P<value>[0-9a-f]{8})\s+\S+\s+\S+\s+(?P<sec>\S+)\s+"
    r"(?P<size>[0-9a-f]{8})\s+(?P<name>\S+)\s*$"
)
RELOC_RE = re.compile(
    r"^([0-9a-f]{8})\s+(\S+)\s+(\S+)\s*$"
)

# Bits whose final value is supplied or adjusted by the linker. A positional
# comparison masks the union of the candidate and target relocation fields at
# each word. Unknown relocation kinds fail closed by masking nothing: without
# a defined bit field, the tool must not erase a possible code-generation
# mismatch.
RELOC_VALUE_MASKS = {
    "R_MIPS_16": 0x0000FFFF,
    "R_MIPS_26": 0x03FFFFFF,
    "R_MIPS_32": 0xFFFFFFFF,
    "R_MIPS_HI16": 0x0000FFFF,
    "R_MIPS_LO16": 0x0000FFFF,
    "R_MIPS_GPREL16": 0x0000FFFF,
    "R_MIPS_LITERAL": 0x0000FFFF,
    "R_MIPS_GOT16": 0x0000FFFF,
    "R_MIPS_CALL16": 0x0000FFFF,
}
UNKNOWN_RELOC_VALUE_MASK = 0


def objdump_text(objfile: pathlib.Path) -> str:
    return subprocess.run(
        [str(OBJDUMP), "-t", str(objfile)], capture_output=True, text=True, check=True
    ).stdout


def func_symbol_span(objfile: pathlib.Path, func: str) -> Optional[tuple[int, int]]:
    """Section-relative (offset, size) for ``func`` in an object's .text."""
    for line in objdump_text(objfile).splitlines():
        m = SYM_RE.match(line)
        if m and m.group("name") == func and m.group("sec") == ".text":
            return int(m.group("value"), 16), int(m.group("size"), 16)
    return None


def text_bytes(objfile: pathlib.Path, start: int, size: int) -> bytes:
    """Raw bytes for one function's span within an object's .text."""
    # Several queued symbols can now share one configured object and be
    # scored concurrently. A deterministic sibling filename races readers.
    with tempfile.NamedTemporaryFile(dir=objfile.parent, suffix=".text.bin", delete=False) as stream:
        tmp = pathlib.Path(stream.name)
    try:
        subprocess.run(
            [str(OBJCOPY), "-O", "binary", "--only-section=.text", str(objfile), str(tmp)],
            capture_output=True, check=True,
        )
        raw = tmp.read_bytes()
    finally:
        tmp.unlink(missing_ok=True)
    return raw[start:start + size]


def relocations(
    objfile: pathlib.Path, start: int, size: int
) -> dict[int, tuple[str, str]]:
    """Function-relative offset -> (type, value) for .text relocations."""
    out = subprocess.run(
        [str(OBJDUMP), "-r", str(objfile)], capture_output=True, text=True, check=True
    ).stdout
    result: dict[int, tuple[str, str]] = {}
    in_text = False
    for line in out.splitlines():
        if line.startswith("RELOCATION RECORDS FOR [.text]"):
            in_text = True
            continue
        if line.startswith("RELOCATION RECORDS FOR"):
            in_text = False
            continue
        if not in_text:
            continue
        m = RELOC_RE.match(line.strip())
        if m:
            offset, rtype, value = m.groups()
            absolute = int(offset, 16)
            if start <= absolute < start + size:
                result[absolute - start] = (rtype, value)
    return result


#: asm-processor's own directive, written by splat into a listing that carries
#: migrated `.late_rodata` (a TU that owns its whole `.rodata` while the
#: function is still assembled). gas rejects it as an unknown pseudo-op; the
#: section itself assembles under its own name and is never scored.
_LATE_RODATA_ALIGNMENT_RE = re.compile(
    r"^[ \t]*\.late_rodata_alignment[ \t]+\d+[ \t]*$", re.MULTILINE)


def standalone_listing(listing: str) -> str:
    """Return `listing` with the directives only asm-processor understands removed."""
    return _LATE_RODATA_ALIGNMENT_RE.sub("", listing)


def assemble_target(target_asm: pathlib.Path, target_o: pathlib.Path) -> None:
    """Assemble a prepared target when import.py did not leave target.o."""
    standalone = target_o.with_suffix(".standalone.s")
    prelude = ROOT / "tools" / "permuter" / "prelude.inc"
    standalone.write_text(
        prelude.read_text() + standalone_listing(target_asm.read_text()))
    command = shlex.split(pb.ASSEMBLER_COMMAND) + [
        str(standalone), "-o", str(target_o)
    ]
    proc = subprocess.run(
        command, cwd=ROOT, capture_output=True, text=True
    )
    if proc.returncode != 0:
        raise RuntimeError(proc.stderr.strip() or "target assembly failed")


#: Environment that means the caller is running the instrumented compiler or
#: forcing an allocator decision. Every scorer here recompiles the TU with the
#: *configured* command before measuring, which overwrites whatever object such
#: a run produced -- so the number returned describes the unforced build. A lane
#: read the same score for twelve different forces before noticing. Warn once.
_FORCED_ENV = ("CDX_FORCE", "CDX_LOG", "CDX_PROC", "CDX_DETAIL_WEB",
               "CDX_LINEAGE_TABLES", "IDO_DIR")
_forced_warning_emitted = False


def _warn_if_a_forced_object_would_be_discarded() -> None:
    global _forced_warning_emitted
    if _forced_warning_emitted:
        return
    present = [name for name in _FORCED_ENV if os.environ.get(name)]
    if not present:
        return
    _forced_warning_emitted = True
    print(
        "warning: " + ", ".join(present) + " is set, and this scorer recompiles "
        "the translation unit with the configured command before measuring.\n"
        "         Any object your instrumented or forced run produced is "
        "overwritten, so the number below\n"
        "         describes the UNFORCED build. Score a forced object against "
        "its target directly instead.",
        file=sys.stderr,
    )


def compile_configured_tu(
    source: str, command: list[str],
) -> tuple[Optional[pathlib.Path], Optional[str]]:
    """Run the fingerprinted command, redirecting only the object output.

    Keep the original source path: its quoted includes, __FILE__, __LINE__,
    and full-TU context are compiler inputs. No source timestamps are touched.
    Objects are refreshed once per TU per measurement pass; no old object is
    accepted after a failed compiler invocation.
    """
    _warn_if_a_forced_object_would_be_discarded()
    output = WORK_DIR / "configured" / f"{source}.o"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    actual = list(command)
    try:
        actual[actual.index("-o") + 1] = str(output)
        proc = subprocess.run(
            actual, cwd=ROOT, capture_output=True, text=True, timeout=120,
        )
    except (OSError, ValueError, IndexError, subprocess.TimeoutExpired) as exc:
        return None, f"configured TU compile failed for {source}: {exc}"
    log = output.with_suffix(".compile.log")
    log.write_text(proc.stdout + proc.stderr)
    if proc.returncode or not output.is_file():
        # A repo-relative path reads better, but the work directory is not
        # always under ROOT: score_symbol.py swaps in a private scratch so it
        # can run during a build, and relative_to() then raises ValueError --
        # inside the error path, so a plain compile failure surfaced as a tool
        # crash naming an unrelated exception. A lane lost three probes to it.
        try:
            where = log.relative_to(ROOT)
        except ValueError:
            where = log
        return None, (
            f"configured TU compile failed for {source}; see {where}"
        )
    return output, None


def words_of(data: bytes) -> list[int]:
    n = len(data) - (len(data) % 4)
    return [w for (w,) in struct.iter_unpack(">I", data[:n])]


def instr_reg_mask(word: int) -> int:
    """Zero out the register-select fields of one big-endian MIPS word,
    leaving every bit that decides *what* the instruction does rather than
    *which registers* it names.

    - **SPECIAL (R-type)** zeroes rs/rt/rd but **keeps the shift amount**,
      which is an immediate and not a register: without that, `sll $t,$s,3`
      and `sll $t,$s,5` compare equal and a real difference is reported as a
      register swap.
    - **COP1** needs its own case rather than the I-type fallback. Its
      register-format layout is fmt(25-21) ft(20-16) fs(15-11) fd(10-6)
      function(5-0), so the I-type mask erased `fmt` -- which selects `.s`
      versus `.d` and is semantic -- while *keeping* fs and fd, which are
      register selectors. A pure float-ring rotation therefore read as a
      structural difference and a genuine format difference read as
      identical. Here fmt and function are kept and ft/fs/fd are zeroed.
      `BC1` (fmt 0x08) names no register at all: bits 20-16 are its
      condition/nd/tf selector and the low half is a branch offset, so it is
      left untouched.
    - **Other I-type** instructions zero rs/rt (bits 25-16) and keep the
      16-bit immediate/branch-offset. This covers `lwc1`/`swc1`, whose only
      register fields are the base and ft.
    - **J-type** (j/jal) has no register fields, so it is left untouched -- a
      differing J-type word is always a relocation or a genuine target
      difference, never a register swap."""
    op = (word >> 26) & 0x3F
    if op in (0x02, 0x03):  # j, jal
        return word
    if op == 0x00:  # SPECIAL (R-type): keep function and shamt
        return word & 0xFC0007FF
    if op == 0x11:  # COP1
        if ((word >> 21) & 0x1F) == 0x08:  # BC1: no register fields
            return word
        return word & 0xFFE0003F  # keep op, fmt, function
    return word & 0xFC00FFFF


@dataclasses.dataclass
class FuncResult:
    name: str
    file: str
    overlay: Optional[int]
    tu: str
    size_bytes: int
    differing_words: int
    first_mismatch_offset: Optional[int]
    relocation_masked_differing_words: Optional[int]
    relocation_masked_first_mismatch_offset: Optional[int]
    size_delta: int
    category: str
    objdiff_match_pct: Optional[float] = None


@dataclasses.dataclass
class RefreshPlan:
    """One immutable decision about which live identities need compilation."""

    live_by_key: dict[tuple[str, str], "pb.QueueItem"]
    current_contexts: dict[tuple[str, str], str]
    fresh_rows: dict[tuple[str, str], dict[str, object]]
    deferred_rows: dict[tuple[str, str], dict[str, object]]
    deferred_unresolved: dict[tuple[str, str], str]
    selected: list["pb.QueueItem"]
    removed: list[tuple[str, str]]
    stale_count: int
    new_count: int


class RankingDocumentError(ValueError):
    """The retained ranking cannot be consumed without guessing."""


def ranking_coverage(
    document: object,
    live_keys: set[tuple[str, str]],
    fresh_keys: set[tuple[str, str]],
) -> dict[str, object]:
    """Audit the complete queue before any ranking filters or scan limits.

    Presence alone is insufficient: unresolved and source-unproven rows cannot
    establish that an unexamined candidate would not outrank a selected row.
    Identity lists make maintenance actionable without compiling anything.
    """
    validated = validate_ranking_document(document)
    resolved = {_function_row_key(row) for row in validated["functions"]}
    unresolved = {_unresolved_row_key(row) for row in validated["unresolved_functions"]}
    retained = resolved | unresolved
    missing = live_keys - retained
    retired = retained - live_keys
    stale = (resolved & live_keys) - fresh_keys
    pending = unresolved & live_keys
    return {
        "complete": not (missing or retired or stale or pending),
        "live": len(live_keys),
        "retained": len(retained),
        "fresh": len(resolved & live_keys & fresh_keys),
        "missing": [list(key) for key in sorted(missing)],
        "retired": [list(key) for key in sorted(retired)],
        "stale": [list(key) for key in sorted(stale)],
        "unresolved": [list(key) for key in sorted(pending)],
    }


def coverage_summary(coverage: dict[str, object]) -> str:
    return " ".join(
        f"{key}={len(value) if isinstance(value, list) else value}"
        for key, value in coverage.items() if key != "complete"
    )


def source_coverage(
    document: object, current_contexts: dict[tuple[str, str], str],
    legacy_contexts: Optional[dict[tuple[str, str], str]] = None,
) -> dict[str, object]:
    """Compare all measured contexts with current selective-TU source."""
    validated = validate_ranking_document(document)
    fresh = set()
    for row in validated["functions"]:
        key = _function_row_key(row)
        measured = normalize_source_context_digest(row.get(SOURCE_CONTEXT_FIELD))
        if measured is None:
            measured = normalize_source_context_digest((legacy_contexts or {}).get(key))
        if measured is not None and measured == current_contexts.get(key):
            fresh.add(key)
    return ranking_coverage(validated, set(current_contexts), fresh)


def strip_c_comments(text: str) -> str:
    """Remove comments while preserving strings and preprocessing structure."""
    output: list[str] = []
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "*":
                state = "block"
                output.extend("  ")
                index += 2
                continue
            if char == "/" and following == "/":
                state = "line"
                output.extend("  ")
                index += 2
                continue
            if char in {'"', "'"}:
                state = "quote"
                quote = char
            output.append(char)
        elif state == "block":
            if char == "*" and following == "/":
                output.extend("  ")
                state = "code"
                index += 2
                continue
            output.append("\n" if char == "\n" else " ")
        elif state == "line":
            if char == "\n":
                output.append(char)
                state = "code"
            else:
                output.append(" ")
        else:
            output.append(char)
            if char == "\\" and following:
                output.append(following)
                index += 2
                continue
            if char == quote:
                state = "code"
        index += 1
    stripped = "".join(output)
    return "\n".join(
        line.rstrip() for line in stripped.splitlines() if line.strip()
    ) + "\n"


def source_context_digest(source_text: Optional[str], symbol: str) -> Optional[str]:
    """Digest the complete original TU, preserving all line-sensitive input.

    Blank/comment-only lines can change __LINE__ and IDO source-line metadata.
    Sibling bodies also belong to the configured full-TU compilation context.
    """
    if source_text is None:
        return None
    blocks = list(pb.iter_nonmatching_blocks(source_text))
    targets = [
        block
        for block in blocks
        if pb.block_function_names(source_text, block).count(symbol) == 1
    ]
    if len(targets) != 1:
        return None
    digest = hashlib.sha256(source_text.encode("utf-8")).digest()
    encoded = base64.urlsafe_b64encode(digest).rstrip(b"=").decode("ascii")
    return group_source_context(encoded)


def group_source_context(encoded: str) -> str:
    """Break base64url evidence into non-word-sized clean-room-safe groups."""
    if BASE64URL_SHA256_RE.fullmatch(encoded) is None:
        raise RankingDocumentError("source context is not one SHA-256 base64url value")
    return ".".join(encoded[index:index + 4] for index in range(0, len(encoded), 4))


def normalize_source_context_digest(value: object) -> Optional[str]:
    """Return canonical unpadded base64url, accepting legacy hex evidence."""
    if not isinstance(value, str):
        return None
    if GROUPED_BASE64URL_SHA256_RE.fullmatch(value):
        return value
    if BASE64URL_SHA256_RE.fullmatch(value):
        return group_source_context(value)
    if HEX_SHA256_RE.fullmatch(value):
        encoded = base64.urlsafe_b64encode(bytes.fromhex(value)).rstrip(b"=").decode("ascii")
        return group_source_context(encoded)
    return None


def current_source_contexts(
    items: list["pb.QueueItem"],
) -> dict[tuple[str, str], str]:
    """Compute current worktree evidence for an exact, duplicate-free queue."""
    source_cache: dict[str, str] = {}
    build_contexts = configured_build_contexts(items)
    contexts: dict[tuple[str, str], str] = {}
    for item in items:
        key = (item.rel_c_file, item.func)
        if key in contexts:
            raise RankingDocumentError(
                f"live queue contains duplicate identity {key[0]}:{key[1]}"
            )
        if item.rel_c_file not in source_cache:
            try:
                source_cache[item.rel_c_file] = item.c_file.read_text(
                    encoding="utf-8"
                )
            except OSError as exc:
                raise RankingDocumentError(
                    f"cannot read live source {item.rel_c_file}: {exc}"
                ) from exc
        digest = source_context_digest(source_cache[item.rel_c_file], item.func)
        if digest is None:
            raise RankingDocumentError(
                f"cannot isolate one NON_MATCHING body for {key[0]}:{key[1]}"
            )
        target = pb.find_asm_target(item)
        if target is None or not (ROOT / target).is_file():
            raise RankingDocumentError(f"missing extracted target for {item.func}; run gmake extract")
        target_digest = hashlib.sha256((ROOT / target).read_bytes()).hexdigest()
        contexts[key] = evidence_digest([digest, build_contexts[item.rel_c_file], target_digest])
    return contexts


def evidence_digest(value: object) -> str:
    raw = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    encoded = base64.urlsafe_b64encode(hashlib.sha256(raw).digest()).rstrip(b"=").decode()
    return group_source_context(encoded)


def include_directories(command: list[str]) -> list[pathlib.Path]:
    # asm-processor's assembler arguments have their own -I search path.
    # Only arguments after the second separator are passed to the compiler.
    if any(token.endswith("tools/asm-processor/build.py") for token in command):
        separators = [i for i, token in enumerate(command) if token == "--"]
        if len(separators) != 2:
            raise RankingDocumentError("configured asm-processor recipe lacks argument separators")
        command = command[separators[1] + 1:]
    directories: list[pathlib.Path] = []
    for index, token in enumerate(command):
        if token == "-I" and index + 1 < len(command):
            directories.append(ROOT / command[index + 1])
        elif token.startswith("-I") and len(token) > 2:
            directories.append(ROOT / token[2:])
    return list(dict.fromkeys(directories))


def command_file_inputs(
    command: list[str], source: str, cache: dict[pathlib.Path, str],
) -> dict[str, str]:
    """Bind literal executable/script/prelude operands to their current bytes."""
    inputs = {}
    output_index = command.index("-o") + 1
    for index, token in enumerate(command):
        if index == output_index or token == source or token.startswith("-"):
            continue
        path = ROOT / token
        if path.is_file():
            if path not in cache:
                cache[path] = hashlib.sha256(path.read_bytes()).hexdigest()
            inputs[token.replace(str(ROOT), "<ROOT>")] = cache[path]
    return inputs


def header_dependencies(
    source: pathlib.Path, directories: list[pathlib.Path],
    content_cache: Optional[dict[pathlib.Path, str]] = None,
) -> dict[str, str]:
    """Conservatively walk literal includes, including inactive branches.

    Missing includes remain explicit search facts (some belong to inactive
    SDK paths); adding such a file changes the digest. Macro includes fail
    closed because a textual walk cannot establish their dependency closure.
    """
    visited: set[pathlib.Path] = set()
    dependencies: dict[str, str] = {}
    content_cache = content_cache if content_cache is not None else {}

    def walk(path: pathlib.Path, *, initial: bool = False) -> None:
        path = pathlib.Path(os.path.normpath(path.absolute()))
        if path in visited:
            return
        visited.add(path)
        if path not in content_cache:
            content_cache[path] = path.read_text(encoding="utf-8")
        content = content_cache[path]
        if not initial:
            dependencies[path.relative_to(ROOT).as_posix()] = evidence_digest(content)
        for match in re.finditer(r'^\s*#\s*include\s+([^\n]+)', strip_c_comments(content), re.MULTILINE):
            spelling = match.group(1).strip()
            literal = re.fullmatch(r'([<"])([^>"]+)[>"]', spelling)
            if literal is None:
                raise RankingDocumentError(f"cannot resolve macro include in {path.relative_to(ROOT)}")
            quote, name = literal.groups()
            search = ([path.parent] if quote == '"' else []) + directories
            found = next((directory / name for directory in search if (directory / name).is_file()), None)
            if found is None:
                dependencies[f"missing:{path.relative_to(ROOT)}:{spelling}"] = "absent"
            else:
                walk(found)

    walk(source, initial=True)
    return dependencies


def configured_tool_digest() -> str:
    """Fingerprint compiler binaries and the code that prepares comparisons."""
    paths = [ROOT / "tools" / name for name in (
        "nm_ranking.py", "permute_batch.py", "finalize_plateau.py", "ido-phases.py",
        "permuter/import.py", "permuter/prelude.inc",
        "binutils/mips64-elf-as", "binutils/mips64-elf-objdump",
        "binutils/mips64-elf-objcopy",
    )]
    paths.extend(sorted((ROOT / "tools/ido").glob("*")))
    paths.extend(sorted((ROOT / "tools/asm-processor").glob("*.py")))
    paths.extend(sorted((ROOT / "tools/permuter/src").rglob("*.py")))
    records = {}
    for path in paths:
        if not path.is_file():
            raise RankingDocumentError(f"missing configured tool {path.relative_to(ROOT)}")
        records[path.relative_to(ROOT).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    return evidence_digest(records)


def assembly_dependencies(source: pathlib.Path, command: list[str]) -> dict[str, str]:
    """Bind all pragma inputs, including conservative inactive fallbacks.

    Full-TU asm-processor compilation can consume other functions' assembly,
    not just the selected target. Hash literal nested include candidates too;
    unresolved includes fail closed instead of inventing an input closure.
    """
    separators = [i for i, token in enumerate(command) if token == "--"]
    assembler = command[separators[0] + 1:separators[1]] if len(separators) == 2 else []
    directories = [ROOT, *include_directories(assembler)]
    dependencies: dict[str, str] = {}
    visited: set[pathlib.Path] = set()
    def walk(path: pathlib.Path) -> None:
        path = pathlib.Path(os.path.normpath(path.absolute()))
        if path in visited:
            return
        visited.add(path)
        raw = path.read_bytes()
        dependencies[path.relative_to(ROOT).as_posix()] = hashlib.sha256(raw).hexdigest()
        for match in re.finditer(r'^\s*\.include\s+"([^"]+)"', raw.decode("utf-8"), re.MULTILINE):
            found = [directory / match.group(1) for directory in [path.parent, *directories]
                     if (directory / match.group(1)).is_file()]
            if not found:
                raise RankingDocumentError(f"missing assembly include from {path.relative_to(ROOT)}")
            for included in found:
                walk(included)
    for match in pb.GLOBAL_ASM_RE.finditer(strip_c_comments(source.read_text(encoding="utf-8"))):
        walk(ROOT / match.group("path"))
    return dependencies


def configured_build_contexts(items: list["pb.QueueItem"]) -> dict[str, str]:
    """Expand each TU recipe once and bind flags, tools, and header closure.

    No source mtime is touched and no compiler runs. Recipe expansion is
    repeated at publication so a cached measurement cannot conceal a flag or
    header edit that overlaps compilation.
    """
    if not items:
        return {}
    tool_digest = configured_tool_digest()
    unique = {item.rel_c_file: item for item in items}
    commands = configured_compile_commands(list(unique.values()))
    content_cache: dict[pathlib.Path, str] = {}
    command_files: dict[pathlib.Path, str] = {}
    def context(item: "pb.QueueItem") -> tuple[str, str]:
        command = commands[item.rel_c_file]
        directories = include_directories(command)
        headers = header_dependencies(item.c_file, directories, content_cache)
        assembly = assembly_dependencies(item.c_file, command)
        portable = [token.replace(str(ROOT), "<ROOT>") for token in command]
        return item.rel_c_file, evidence_digest({
            "command": portable, "headers": headers, "tools": tool_digest,
            "assembly": assembly,
            "command_files": command_file_inputs(command, item.rel_c_file, command_files),
            "target_assembler": pb.ASSEMBLER_COMMAND,
        })
    return dict(map(context, unique.values()))


def configured_compile_commands(items: list["pb.QueueItem"]) -> dict[str, list[str]]:
    """Expand every distinct TU in one make invocation, with no mtime writes."""
    sources = list(dict.fromkeys(item.rel_c_file for item in items))
    arguments = ["gmake", "--no-print-directory", "-n", "NON_MATCHING=1"]
    for source in sources:
        arguments.extend(["-W", source])
    arguments.extend(f"build_non_matching/{source}.o" for source in sources)
    proc = subprocess.run(arguments, cwd=ROOT, capture_output=True, text=True, timeout=120)
    if proc.returncode:
        raise RankingDocumentError(proc.stderr.strip() or "cannot expand configured TU recipes")
    wanted = set(sources)
    commands: dict[str, list[str]] = {}
    for line in proc.stdout.replace("\\\n", " ").splitlines():
        if "tools/asm-processor/build.py" not in line:
            continue
        command = shlex.split(line)
        matches = wanted.intersection(command)
        if len(matches) != 1:
            raise RankingDocumentError("configured recipe lacks one exact source identity")
        source = matches.pop()
        if source in commands:
            raise RankingDocumentError(f"duplicate configured recipe for {source}")
        try:
            command[command.index("-o") + 1] = "build/nm_ranking/context.o"
        except (ValueError, IndexError) as exc:
            raise RankingDocumentError(f"configured recipe lacks output for {source}") from exc
        commands[source] = command
    if set(commands) != wanted:
        raise RankingDocumentError(f"missing configured recipes: {sorted(wanted - set(commands))}")
    return commands


def blamed_source_lines(ref: str, path: str) -> list[tuple[str, str]]:
    """Return each tracked ranking line with its porcelain-blame commit."""
    proc = subprocess.run(
        ["git", "blame", "--line-porcelain", ref, "--", path],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    if proc.returncode != 0:
        raise RankingDocumentError(
            proc.stderr.strip() or f"cannot blame ranking evidence at {ref}:{path}"
        )
    result: list[tuple[str, str]] = []
    commit: Optional[str] = None
    for line in proc.stdout.splitlines():
        header = BLAME_HEADER_RE.match(line)
        if header:
            commit = header.group(1)
        elif line.startswith("\t"):
            if commit is None:
                raise RankingDocumentError(
                    f"blame source line lacks a commit in {path}"
                )
            result.append((commit, line[1:]))
            commit = None
    return result


def ranking_evidence_commits(
    ref: str, path: str
) -> dict[tuple[str, str], str]:
    """Map each metric row to embedded-context or legacy measurement blame."""
    differing_commits: dict[tuple[str, str], str] = {}
    context_commits: dict[tuple[str, str], str] = {}
    symbol: Optional[str] = None
    file_name: Optional[str] = None
    for commit, line in blamed_source_lines(ref, path):
        stripped = line.strip()
        if stripped.startswith('"name":'):
            symbol = str(json.loads(stripped.split(":", 1)[1].rstrip(",")))
            file_name = None
        elif stripped.startswith('"file":'):
            file_name = str(json.loads(stripped.split(":", 1)[1].rstrip(",")))
        elif stripped.startswith('"differing_words":'):
            if symbol is None or file_name is None:
                raise RankingDocumentError(
                    f"ranking measurement lacks file/symbol context in {path}"
                )
            key = (file_name, symbol)
            if key in differing_commits:
                raise RankingDocumentError(
                    f"duplicate blamed ranking identity {file_name}:{symbol}"
                )
            differing_commits[key] = commit
        elif stripped.startswith(f'"{SOURCE_CONTEXT_FIELD}":'):
            if symbol is None or file_name is None:
                raise RankingDocumentError(
                    f"ranking context lacks file/symbol identity in {path}"
                )
            context_commits[(file_name, symbol)] = commit
    return {
        key: context_commits.get(key, commit)
        for key, commit in differing_commits.items()
    }


def git_source(ref: str, path: str) -> Optional[str]:
    proc = subprocess.run(
        ["git", "show", f"{ref}:{path}"],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    return proc.stdout if proc.returncode == 0 else None


def legacy_source_contexts(
    ref: str, ranking_path: pathlib.Path, document: object
) -> dict[tuple[str, str], str]:
    """Recover source evidence for schema-1 rows without trusting their age.

    Git blame identifies the commit that introduced each retained measurement.
    A digest from that commit can prove a legacy row still describes the exact
    current selective-TU source. Missing/ambiguous evidence simply leaves the
    row stale; it never becomes an optimistic cache hit.
    """
    validated = validate_ranking_document(document)
    try:
        relative = ranking_path.resolve().relative_to(ROOT.resolve()).as_posix()
        commits = ranking_evidence_commits(ref, relative)
    except (ValueError, RankingDocumentError):
        return {}
    functions = validated["functions"]
    assert isinstance(functions, list)
    contexts: dict[tuple[str, str], str] = {}
    source_cache: dict[tuple[str, str], Optional[str]] = {}
    for row in functions:
        key = _function_row_key(row)
        if isinstance(row, dict) and row.get(SOURCE_CONTEXT_FIELD) is not None:
            continue
        commit = commits.get(key)
        if commit is None:
            continue
        source_key = (commit, key[0])
        if source_key not in source_cache:
            source_cache[source_key] = git_source(commit, key[0])
        digest = source_context_digest(source_cache[source_key], key[1])
        if digest is not None:
            contexts[key] = digest
    return contexts


def _plain_int(value: object, field: str, *, minimum: Optional[int] = None) -> int:
    if type(value) is not int:
        raise RankingDocumentError(f"{field} must be an integer")
    if minimum is not None and value < minimum:
        raise RankingDocumentError(f"{field} must be >= {minimum}")
    return value


def _plain_string(value: object, field: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise RankingDocumentError(f"{field} must be a non-empty string")
    if any(ord(character) < 0x20 or ord(character) == 0x7F for character in value):
        raise RankingDocumentError(f"{field} must not contain control characters")
    return value


def _source_symbol_identity(
    file_name: object, symbol: object, field: str
) -> tuple[str, str]:
    source = _plain_string(file_name, f"{field}.file")
    name = _plain_string(symbol, f"{field}.symbol")
    path = pathlib.PurePosixPath(source)
    if (
        path.is_absolute()
        or ".." in path.parts
        or path.suffix != ".c"
        or source != path.as_posix()
        or not path.parts
        or path.parts[0] != "src"
    ):
        raise RankingDocumentError(
            f"{field}.file must be a relative C source path"
        )
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
        raise RankingDocumentError(f"{field}.symbol is not a C identifier")
    return source, name


def _function_row_key(row: object) -> tuple[str, str]:
    if not isinstance(row, dict):
        raise RankingDocumentError("function row is not an object")
    return _source_symbol_identity(row.get("file"), row.get("name"), "function")


def _unresolved_row_key(row: object) -> tuple[str, str]:
    if not isinstance(row, list) or len(row) != 2:
        raise RankingDocumentError(
            "unresolved row needs [[file, name], diagnostic] identity"
        )
    key, diagnostic = row
    if (
        not isinstance(key, list)
        or len(key) != 2
        or not isinstance(diagnostic, str)
    ):
        raise RankingDocumentError(
            "unresolved row needs [[file, name], diagnostic] identity"
        )
    identity = _source_symbol_identity(key[0], key[1], "unresolved")
    _plain_string(diagnostic, "unresolved diagnostic")
    return identity


def validate_ranking_document(
    document: object, *, require_consistent_counts: bool = True
) -> dict[str, object]:
    """Validate the complete persisted schema and exact row identities.

    Pruning may deliberately repair stale derived counts, so its input opts out
    of count consistency while retaining every structural/type/identity check.
    Documentation generation and checking always require full consistency.
    """
    if not isinstance(document, dict):
        raise RankingDocumentError("ranking root is not an object")

    schema_version = document.get("schema_version", 1)
    schema_version = _plain_int(schema_version, "schema_version", minimum=1)
    if schema_version not in (1, 2, SCHEMA_VERSION):
        raise RankingDocumentError(
            f"schema_version {schema_version} is unsupported"
        )
    if schema_version >= 2:
        context_version = _plain_int(
            document.get("source_context_version"),
            "source_context_version",
            minimum=1,
        )
        if context_version not in (1, 2, 3, 4, SOURCE_CONTEXT_VERSION):
            raise RankingDocumentError(
                f"source_context_version {context_version} is unsupported"
            )
        context_coverage = _plain_int(
            document.get("source_context_coverage"),
            "source_context_coverage",
            minimum=0,
        )
    else:
        context_coverage = 0
    if schema_version >= 3:
        relocation_masked_coverage = _plain_int(
            document.get("relocation_masked_coverage"),
            "relocation_masked_coverage",
            minimum=0,
        )
    else:
        relocation_masked_coverage = 0

    queue_size = _plain_int(document.get("queue_size"), "queue_size", minimum=0)
    resolved = _plain_int(document.get("resolved"), "resolved", minimum=0)
    unresolved_count = _plain_int(
        document.get("unresolved"), "unresolved", minimum=0
    )
    coverage = _plain_int(
        document.get("objdiff_match_pct_coverage"),
        "objdiff_match_pct_coverage",
        minimum=0,
    )
    if type(document.get("objdiff_report_used")) is not bool:
        raise RankingDocumentError("objdiff_report_used must be a boolean")

    functions = document.get("functions")
    unresolved = document.get("unresolved_functions")
    if not isinstance(functions, list) or not isinstance(unresolved, list):
        raise RankingDocumentError(
            "ranking needs functions and unresolved_functions lists"
        )

    function_keys: list[tuple[str, str]] = []
    measured_coverage = 0
    measured_context_coverage = 0
    measured_relocation_masked_coverage = 0
    for index, row in enumerate(functions):
        key = _function_row_key(row)
        assert isinstance(row, dict)
        function_keys.append(key)
        prefix = f"functions[{index}]"
        overlay = row.get("overlay")
        tu = _plain_string(row.get("tu"), f"{prefix}.tu")
        source_parts = pathlib.PurePosixPath(key[0]).parts
        if overlay is not None:
            overlay = _plain_int(overlay, f"{prefix}.overlay", minimum=0)
            expected_tu = f"overlays/o{overlay:03d}"
            if len(source_parts) < 3 or "/".join(source_parts[1:3]) != expected_tu:
                raise RankingDocumentError(
                    f"{prefix}.overlay does not match its source path"
                )
        else:
            expected_tu = source_parts[1] if len(source_parts) >= 2 else ""
        if tu != expected_tu:
            raise RankingDocumentError(
                f"{prefix}.tu is {tu!r}, expected {expected_tu!r} from identity"
            )
        size = _plain_int(row.get("size_bytes"), f"{prefix}.size_bytes", minimum=0)
        if size % 4:
            raise RankingDocumentError(f"{prefix}.size_bytes must be word-aligned")
        raw_differing = _plain_int(
            row.get("differing_words"), f"{prefix}.differing_words", minimum=0
        )
        first = row.get("first_mismatch_offset")
        if first is not None:
            first = _plain_int(first, f"{prefix}.first_mismatch_offset", minimum=0)
            if first % 4:
                raise RankingDocumentError(
                    f"{prefix}.first_mismatch_offset must be word-aligned"
                )
        delta = _plain_int(row.get("size_delta"), f"{prefix}.size_delta")
        if delta % 4:
            raise RankingDocumentError(f"{prefix}.size_delta must be word-aligned")
        if size + delta < 0:
            raise RankingDocumentError(f"{prefix}.size_delta makes base size negative")
        if (raw_differing == 0) != (first is None):
            raise RankingDocumentError(
                f"{prefix}.first_mismatch_offset disagrees with differing_words"
            )
        masked_differing = row.get("relocation_masked_differing_words")
        masked_first = row.get("relocation_masked_first_mismatch_offset")
        if schema_version >= 3:
            if "relocation_masked_differing_words" not in row:
                raise RankingDocumentError(
                    f"{prefix}.relocation_masked_differing_words is required"
                )
            if "relocation_masked_first_mismatch_offset" not in row:
                raise RankingDocumentError(
                    f"{prefix}.relocation_masked_first_mismatch_offset is required"
                )
            if masked_differing is None:
                if masked_first is not None:
                    raise RankingDocumentError(
                        f"{prefix}.relocation_masked_first_mismatch_offset needs "
                        "relocation-masked evidence"
                    )
            else:
                masked_differing = _plain_int(
                    masked_differing,
                    f"{prefix}.relocation_masked_differing_words",
                    minimum=0,
                )
                if masked_differing > raw_differing:
                    raise RankingDocumentError(
                        f"{prefix}.relocation_masked_differing_words exceeds "
                        "differing_words"
                    )
                if masked_first is not None:
                    masked_first = _plain_int(
                        masked_first,
                        f"{prefix}.relocation_masked_first_mismatch_offset",
                        minimum=0,
                    )
                    if masked_first % 4:
                        raise RankingDocumentError(
                            f"{prefix}.relocation_masked_first_mismatch_offset "
                            "must be word-aligned"
                        )
                if (masked_differing == 0) != (masked_first is None):
                    raise RankingDocumentError(
                        f"{prefix}.relocation_masked_first_mismatch_offset "
                        "disagrees with relocation_masked_differing_words"
                    )
                if first is not None and masked_first is not None and masked_first < first:
                    raise RankingDocumentError(
                        f"{prefix}.relocation_masked_first_mismatch_offset "
                        "precedes the raw first mismatch"
                    )
                measured_relocation_masked_coverage += 1
        _plain_string(row.get("category"), f"{prefix}.category")
        pct = row.get("objdiff_match_pct")
        if pct is not None:
            if isinstance(pct, bool) or not isinstance(pct, (int, float)):
                raise RankingDocumentError(
                    f"{prefix}.objdiff_match_pct must be a number or null"
                )
            if not math.isfinite(pct) or not 0 <= pct <= 100:
                raise RankingDocumentError(
                    f"{prefix}.objdiff_match_pct must be between 0 and 100"
                )
            measured_coverage += 1
        context_digest = row.get(SOURCE_CONTEXT_FIELD)
        if context_digest is not None:
            valid_context = (
                isinstance(context_digest, str)
                and (
                    HEX_SHA256_RE.fullmatch(context_digest)
                    if schema_version >= 2 and context_version == 1
                    else BASE64URL_SHA256_RE.fullmatch(context_digest)
                    if schema_version >= 2 and context_version == 2
                    else GROUPED_BASE64URL_SHA256_RE.fullmatch(context_digest)
                )
            )
            if not valid_context:
                raise RankingDocumentError(
                    f"{prefix}.{SOURCE_CONTEXT_FIELD} must use the schema's SHA-256 encoding"
                )
            measured_context_coverage += 1

    unresolved_keys = [_unresolved_row_key(row) for row in unresolved]
    all_keys = function_keys + unresolved_keys
    if len(set(all_keys)) != len(all_keys):
        raise RankingDocumentError("ranking contains duplicate function identities")

    if require_consistent_counts:
        expected = {
            "resolved": len(functions),
            "unresolved": len(unresolved),
            "queue_size": len(functions) + len(unresolved),
            "objdiff_match_pct_coverage": measured_coverage,
        }
        actual = {
            "resolved": resolved,
            "unresolved": unresolved_count,
            "queue_size": queue_size,
            "objdiff_match_pct_coverage": coverage,
        }
        for field, expected_value in expected.items():
            if actual[field] != expected_value:
                raise RankingDocumentError(
                    f"{field} is {actual[field]}, expected {expected_value} from rows"
                )
        if schema_version >= 2 and context_coverage != measured_context_coverage:
            raise RankingDocumentError(
                f"source_context_coverage is {context_coverage}, expected "
                f"{measured_context_coverage} from rows"
            )
        if (
            schema_version >= 3
            and relocation_masked_coverage != measured_relocation_masked_coverage
        ):
            raise RankingDocumentError(
                f"relocation_masked_coverage is {relocation_masked_coverage}, "
                f"expected {measured_relocation_masked_coverage} from rows"
            )
    return document


def row_from_result(result: FuncResult, context_digest: str) -> dict[str, object]:
    return {
        "name": result.name,
        "file": result.file,
        "overlay": result.overlay,
        "tu": result.tu,
        "size_bytes": result.size_bytes,
        "objdiff_match_pct": result.objdiff_match_pct,
        "differing_words": result.differing_words,
        "first_mismatch_offset": result.first_mismatch_offset,
        "relocation_masked_differing_words": (
            result.relocation_masked_differing_words
        ),
        "relocation_masked_first_mismatch_offset": (
            result.relocation_masked_first_mismatch_offset
        ),
        "size_delta": result.size_delta,
        "category": result.category,
        SOURCE_CONTEXT_FIELD: context_digest,
    }


def row_sort_key(row: dict[str, object]) -> tuple[int, int, int, str, str]:
    masked = row.get("relocation_masked_differing_words")
    return (
        CATEGORY_RANK.get(str(row["category"]), 99),
        int(masked) if masked is not None else int(row["differing_words"]),
        int(row["differing_words"]),
        str(row["file"]),
        str(row["name"]),
    )


def make_ranking_document(
    functions: list[dict[str, object]],
    unresolved: list[list[object]],
    *,
    objdiff_report_used: bool,
) -> dict[str, object]:
    functions = [dict(row) for row in functions]
    for row in functions:
        row.setdefault("relocation_masked_differing_words", None)
        row.setdefault("relocation_masked_first_mismatch_offset", None)
    functions = sorted(functions, key=row_sort_key)
    unresolved = sorted(
        unresolved,
        key=lambda row: (str(row[0][0]), str(row[0][1])),
    )
    document: dict[str, object] = {
        "schema_version": SCHEMA_VERSION,
        "source_context_version": SOURCE_CONTEXT_VERSION,
        "queue_size": len(functions) + len(unresolved),
        "resolved": len(functions),
        "unresolved": len(unresolved),
        "objdiff_report_used": objdiff_report_used,
        "objdiff_match_pct_coverage": sum(
            row.get("objdiff_match_pct") is not None for row in functions
        ),
        "source_context_coverage": sum(
            row.get(SOURCE_CONTEXT_FIELD) is not None for row in functions
        ),
        "relocation_masked_coverage": sum(
            row.get("relocation_masked_differing_words") is not None
            for row in functions
        ),
        "functions": functions,
        "unresolved_functions": unresolved,
    }
    validate_ranking_document(document)
    return document


def plan_incremental_refresh(
    document: object,
    live_items: list["pb.QueueItem"],
    current_contexts: dict[tuple[str, str], str],
    legacy_contexts: dict[tuple[str, str], str],
    *,
    limit: Optional[int] = None,
) -> RefreshPlan:
    """Classify retained rows and select only stale/new identities to build."""
    validated = validate_ranking_document(document)
    functions = validated["functions"]
    unresolved = validated["unresolved_functions"]
    assert isinstance(functions, list)
    assert isinstance(unresolved, list)
    schema_version = int(validated.get("schema_version", 1))

    live_by_key: dict[tuple[str, str], pb.QueueItem] = {}
    for item in live_items:
        key = (item.rel_c_file, item.func)
        if key in live_by_key:
            raise RankingDocumentError(
                f"live queue contains duplicate identity {key[0]}:{key[1]}"
            )
        live_by_key[key] = item
    if set(live_by_key) != set(current_contexts):
        raise RankingDocumentError(
            "live queue and current source-context identities disagree"
        )

    fresh_rows: dict[tuple[str, str], dict[str, object]] = {}
    stale_function_rows: dict[tuple[str, str], dict[str, object]] = {}
    stale_order: list[tuple[str, str]] = []
    all_retained_keys: set[tuple[str, str]] = set()
    for raw in functions:
        assert isinstance(raw, dict)
        key = _function_row_key(raw)
        all_retained_keys.add(key)
        if key not in live_by_key:
            continue
        embedded = normalize_source_context_digest(raw.get(SOURCE_CONTEXT_FIELD))
        measured = embedded or normalize_source_context_digest(legacy_contexts.get(key))
        if measured == current_contexts[key]:
            migrated = dict(raw)
            migrated[SOURCE_CONTEXT_FIELD] = current_contexts[key]
            if (
                schema_version >= 3
                and migrated.get("relocation_masked_differing_words") is not None
            ):
                fresh_rows[key] = migrated
            else:
                # The raw measurement is still source-proven, but schema v3
                # needs one compile to derive relocation-masked evidence.
                # Preserve its context if --limit defers that backfill so the
                # retained raw metric does not become falsely stale.
                stale_function_rows[key] = migrated
                stale_order.append(key)
        else:
            stale = dict(raw)
            stale.pop(SOURCE_CONTEXT_FIELD, None)
            stale_function_rows[key] = stale
            stale_order.append(key)

    stale_unresolved: dict[tuple[str, str], str] = {}
    for raw in unresolved:
        key = _unresolved_row_key(raw)
        all_retained_keys.add(key)
        if key not in live_by_key:
            continue
        stale_unresolved[key] = str(raw[1])
        stale_order.append(key)

    new_keys = [
        key for key in live_by_key if key not in all_retained_keys
    ]
    stale_order.extend(new_keys)
    selected_keys = stale_order[:limit] if limit is not None else stale_order
    selected_set = set(selected_keys)

    deferred_rows = {
        key: row
        for key, row in stale_function_rows.items()
        if key not in selected_set
    }
    deferred_unresolved = {
        key: diagnostic
        for key, diagnostic in stale_unresolved.items()
        if key not in selected_set
    }
    for key in new_keys:
        if key not in selected_set:
            deferred_unresolved[key] = (
                "incremental refresh pending: bounded --limit deferred this "
                "new queue identity"
            )

    return RefreshPlan(
        live_by_key=live_by_key,
        current_contexts=current_contexts,
        fresh_rows=fresh_rows,
        deferred_rows=deferred_rows,
        deferred_unresolved=deferred_unresolved,
        selected=[live_by_key[key] for key in selected_keys],
        removed=sorted(all_retained_keys - set(live_by_key)),
        stale_count=len(stale_order) - len(new_keys),
        new_count=len(new_keys),
    )


def merge_incremental_results(
    document: object,
    plan: RefreshPlan,
    results: list[FuncResult],
    *,
    objdiff_report_used: bool,
) -> dict[str, object]:
    """Merge a successful selected batch; every live identity remains visible."""
    selected_keys = {
        (item.rel_c_file, item.func) for item in plan.selected
    }
    result_by_key = {(result.file, result.name): result for result in results}
    if set(result_by_key) != selected_keys:
        raise RankingDocumentError(
            "incremental result identities disagree with the selected refresh batch"
        )

    rows = list(plan.fresh_rows.values()) + list(plan.deferred_rows.values())
    rows.extend(
        row_from_result(result_by_key[key], plan.current_contexts[key])
        for key in selected_keys
    )
    unresolved = [
        [[key[0], key[1]], diagnostic]
        for key, diagnostic in plan.deferred_unresolved.items()
    ]
    merged = make_ranking_document(
        rows,
        unresolved,
        objdiff_report_used=(
            objdiff_report_used
            or bool(validate_ranking_document(document)["objdiff_report_used"])
        ),
    )
    merged_keys = {
        _function_row_key(row) for row in merged["functions"]
    } | {
        _unresolved_row_key(row) for row in merged["unresolved_functions"]
    }
    if merged_keys != set(plan.live_by_key):
        raise RankingDocumentError(
            "incremental merge does not cover the exact live queue"
        )
    return merged


def prune_stale_document(
    document: object, live_keys: set[tuple[str, str]]
) -> tuple[dict[str, object], list[tuple[str, str]], list[tuple[str, str]]]:
    """Drop retained rows whose exact source/symbol identity is no longer queued.

    This deliberately does not invent measurements for newly queued functions.
    Their identities are returned as ``unranked`` so the caller can report that
    a full regeneration is still needed.
    """
    document = validate_ranking_document(
        document, require_consistent_counts=False
    )
    functions = document.get("functions")
    unresolved = document.get("unresolved_functions")
    assert isinstance(functions, list)
    assert isinstance(unresolved, list)

    function_keys = [_function_row_key(row) for row in functions]
    unresolved_keys = [_unresolved_row_key(row) for row in unresolved]
    all_keys = function_keys + unresolved_keys

    retained_functions = [
        row for row, key in zip(functions, function_keys) if key in live_keys
    ]
    retained_unresolved = [
        row for row, key in zip(unresolved, unresolved_keys) if key in live_keys
    ]
    retained_keys = {key for key in all_keys if key in live_keys}
    removed = sorted(set(all_keys) - live_keys)
    unranked = sorted(live_keys - retained_keys)

    pruned = dict(document)
    pruned["functions"] = retained_functions
    pruned["unresolved_functions"] = retained_unresolved
    pruned["resolved"] = len(retained_functions)
    pruned["unresolved"] = len(retained_unresolved)
    pruned["queue_size"] = len(retained_functions) + len(retained_unresolved)
    pruned["objdiff_match_pct_coverage"] = sum(
        1
        for row in retained_functions
        if isinstance(row, dict) and row.get("objdiff_match_pct") is not None
    )
    if int(pruned.get("schema_version", 1)) >= 2:
        pruned["source_context_coverage"] = sum(
            1
            for row in retained_functions
            if isinstance(row, dict) and row.get(SOURCE_CONTEXT_FIELD) is not None
        )
    if int(pruned.get("schema_version", 1)) >= 3:
        pruned["relocation_masked_coverage"] = sum(
            1
            for row in retained_functions
            if isinstance(row, dict)
            and row.get("relocation_masked_differing_words") is not None
        )
    validate_ranking_document(pruned)
    return pruned, removed, unranked


def write_text_atomic(path: pathlib.Path, content: str) -> None:
    """Replace one text file only after its complete temp file is written."""
    path.parent.mkdir(parents=True, exist_ok=True)
    output_mode = stat.S_IMODE(path.stat().st_mode) if path.exists() else 0o644
    temporary: Optional[pathlib.Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary = pathlib.Path(stream.name)
            stream.write(content)
            stream.flush()
        temporary.chmod(output_mode)
        temporary.replace(path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def write_json_atomic(path: pathlib.Path, document: dict[str, object]) -> None:
    """Validate and atomically replace one persisted ranking document."""
    validate_ranking_document(document)
    write_text_atomic(path, json.dumps(document, indent=2) + "\n")


def write_texts_transactionally(entries: list[tuple[pathlib.Path, str]]) -> None:
    """Stage every text output, then replace all with rollback on an error."""
    staged: list[tuple[pathlib.Path, pathlib.Path]] = []
    originals: dict[pathlib.Path, Optional[tuple[str, int]]] = {}
    replaced: list[pathlib.Path] = []
    try:
        for path, content in entries:
            path.parent.mkdir(parents=True, exist_ok=True)
            if path.exists():
                originals[path] = (
                    path.read_text(encoding="utf-8"),
                    stat.S_IMODE(path.stat().st_mode),
                )
                output_mode = originals[path][1]
            else:
                originals[path] = None
                output_mode = 0o644
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                dir=path.parent,
                prefix=f".{path.name}.",
                suffix=".tmp",
                delete=False,
            ) as stream:
                temporary = pathlib.Path(stream.name)
                stream.write(content)
                stream.flush()
                os.fsync(stream.fileno())
            temporary.chmod(output_mode)
            staged.append((path, temporary))

        for path, temporary in staged:
            temporary.replace(path)
            replaced.append(path)
    except Exception:
        for path in reversed(replaced):
            original = originals[path]
            if original is None:
                path.unlink(missing_ok=True)
            else:
                content, mode = original
                write_text_atomic(path, content)
                path.chmod(mode)
        raise
    finally:
        for _path, temporary in staged:
            temporary.unlink(missing_ok=True)


def publish_ranking_document(
    path: pathlib.Path,
    document: dict[str, object],
    documentation_path: pathlib.Path,
) -> None:
    """Validate and publish JSON plus its canonical generated documentation.

    All validation/rendering happens before either destination changes. Each
    replacement is atomic; the documentation is coupled only for the canonical
    ranking path, matching the existing command contract.
    """
    validate_ranking_document(document)
    expected_doc: Optional[str] = None
    if path.resolve() == DEFAULT_OUT.resolve():
        _, expected_doc = expected_document_text(document, documentation_path)
    json_text = json.dumps(document, indent=2) + "\n"
    entries = [(path, json_text)]
    if expected_doc is not None:
        entries.append((documentation_path, expected_doc))
    write_texts_transactionally(entries)


def missing_permuter_inputs() -> list[pathlib.Path]:
    required = [
        ROOT / "tools" / "permuter" / "import.py",
        ROOT / "tools" / "permuter" / "prelude.inc",
    ]
    return [path for path in required if not path.is_file()]


def report_missing_permuter(missing: list[pathlib.Path], out: pathlib.Path) -> int:
    print(
        "error: decomp-permuter is not installed; refusing to overwrite "
        f"{out}",
        file=sys.stderr,
    )
    for path in missing:
        print(f"  missing: {path.relative_to(ROOT)}", file=sys.stderr)
    print(
        "install decomp-permuter under tools/permuter as described in "
        "README.md, then rerun this command",
        file=sys.stderr,
    )
    return 2


def process_items(
    queue: list["pb.QueueItem"], jobs: int
) -> tuple[list[FuncResult], list[tuple[tuple[str, str], str]]]:
    results: list[FuncResult] = []
    errors: list[tuple[tuple[str, str], str]] = []
    commands = configured_compile_commands(queue)
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        sources = list(commands)
        compiled = dict(zip(sources, pool.map(
            lambda source: compile_configured_tu(source, commands[source]), sources,
        )))
        def measure(item: "pb.QueueItem") -> tuple[Optional[FuncResult], Optional[str]]:
            candidate, error = compiled[item.rel_c_file]
            if candidate is None:
                return None, error
            return process_item(item, candidate)
        for item, (result, error) in zip(queue, pool.map(measure, queue)):
            if result is not None:
                results.append(result)
            if error is not None:
                errors.append(((item.rel_c_file, item.func), error))
    return results, errors


def _markdown_code(value: object) -> str:
    text = str(value).replace("\\", "\\\\").replace("|", "\\|")
    text = text.replace("`", "\\`")
    return f"`{text}`"


def _markdown_text(value: object) -> str:
    return str(value).replace("\\", "\\\\").replace("|", "\\|")


def render_ranking_markdown(document: object) -> str:
    """Render the authoritative human-readable snapshot from validated JSON."""
    document = validate_ranking_document(document)
    functions = document["functions"]
    unresolved = document["unresolved_functions"]
    assert isinstance(functions, list)
    assert isinstance(unresolved, list)

    queue_size = int(document["queue_size"])
    schema_version = int(document.get("schema_version", 1))
    resolved = int(document["resolved"])
    unresolved_count = int(document["unresolved"])
    coverage = int(document["objdiff_match_pct_coverage"])
    context_coverage = sum(
        isinstance(row, dict) and row.get(SOURCE_CONTEXT_FIELD) is not None
        for row in functions
    )
    relocation_masked_coverage = sum(
        isinstance(row, dict)
        and row.get("relocation_masked_differing_words") is not None
        for row in functions
    )
    total_size = sum(int(row["size_bytes"]) for row in functions)
    overlays = sorted(
        {int(row["overlay"]) for row in functions if row["overlay"] is not None}
    )
    resident_tus = sorted(
        {str(row["tu"]) for row in functions if row["overlay"] is None}
    )

    lines = [
        "## Current generated ranking snapshot",
        "",
        "> Generated by `tools/nm_ranking.py --write-doc` from",
        "> `config/nonmatching-ranking.us.json`. Do not edit this region by hand;",
        "> `--check-doc` and `gmake check-docs` fail on any drift.",
        "",
        f"The snapshot contains **{queue_size:,} queued identities**: "
        f"**{resolved:,} resolved measurements** and **{unresolved_count:,} "
        f"unresolved identities**. Resolved target size totals **{total_size:,} "
        f"bytes ({total_size / 1024:.1f} KiB)**.",
        "",
        f"Resolved rows span **{len(overlays):,} overlays** and "
        f"**{len(resident_tus):,} resident TU "
        f"{'group' if len(resident_tus) == 1 else 'groups'}**"
        + (f" ({', '.join(_markdown_code(tu) for tu in resident_tus)})" if resident_tus else "")
        + ".",
        "",
        "A supplementary objdiff report "
        + ("was supplied" if document["objdiff_report_used"] else "was not supplied")
        + f"; `objdiff_match_pct` covers **{coverage:,} / {resolved:,}** resolved rows.",
        "",
        f"Persisted configured-TU input evidence covers **{context_coverage:,} / "
        f"{resolved:,}** resolved rows. Rows without it are retained legacy or "
        "bounded-refresh measurements and must be treated as requiring reproof.",
        "",
    ]
    if schema_version >= 3:
        lines.extend([
            f"Relocation-masked mismatch evidence covers **{relocation_masked_coverage:,} "
            f"/ {resolved:,}** resolved rows. The raw count preserves literal object "
            "differences; the masked count removes only known linker-owned fields to "
            "expose the remaining code-generation mismatch. Neither replaces linked "
            "byte-identity proof.",
            "",
        ])
    lines.extend([
        "### Category distribution",
        "",
        "| Category | Count | Share of resolved |",
        "|---|---:|---:|",
    ])

    category_counts = Counter(str(row["category"]) for row in functions)
    category_order: list[str] = []
    for row in functions:
        category = str(row["category"])
        if category not in category_order:
            category_order.append(category)
    for category in category_order:
        count = category_counts[category]
        share = count * 100 / resolved if resolved else 0.0
        lines.append(f"| {_markdown_code(category)} | {count:,} | {share:.1f}% |")

    lines.extend([
        "",
        "### Differing-word thresholds",
        "",
        "| Threshold | Count |",
        "|---|---:|",
    ])
    for threshold in (5, 10, 20):
        count = sum(int(row["differing_words"]) <= threshold for row in functions)
        label = "raw " if schema_version >= 3 else ""
        lines.append(
            f"| {label}`differing_words <= {threshold}` | {count:,} |"
        )
        if schema_version >= 3:
            masked_count = sum(
                row.get("relocation_masked_differing_words") is not None
                and int(row["relocation_masked_differing_words"]) <= threshold
                for row in functions
            )
            lines.append(
                "| relocation-masked "
                f"`differing_words <= {threshold}` | {masked_count:,} |"
            )

    lines.extend([
        "",
        "### Complete ranked queue",
        "",
        "Rank is the persisted `functions` array order. The exact identity is",
        "(`file`, `symbol`), so repeated symbol spellings in different translation",
        "units remain distinct.",
        "",
    ])
    if schema_version >= 3:
        lines.extend([
            "| Rank | File | Symbol | Overlay/TU | Category | Target bytes | Raw diff | Masked diff | Raw first | Masked first | Size delta | Objdiff% |",
            "|---:|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|",
        ])
    else:
        lines.extend([
            "| Rank | File | Symbol | Overlay/TU | Category | Target bytes | Diff words | First mismatch | Size delta | Objdiff% |",
            "|---:|---|---|---|---|---:|---:|---:|---:|---:|",
        ])
    for rank, row in enumerate(functions, 1):
        location = (
            f"o{int(row['overlay']):03d}"
            if row["overlay"] is not None
            else str(row["tu"])
        )
        first = row["first_mismatch_offset"]
        pct = row["objdiff_match_pct"]
        cells = [
            str(rank),
            _markdown_code(row["file"]),
            _markdown_code(row["name"]),
            _markdown_code(location),
            _markdown_code(row["category"]),
            f"{int(row['size_bytes']):,}",
            f"{int(row['differing_words']):,}",
        ]
        if schema_version >= 3:
            masked = row.get("relocation_masked_differing_words")
            masked_first = row.get("relocation_masked_first_mismatch_offset")
            cells.extend([
                "—" if masked is None else f"{int(masked):,}",
                "—" if first is None else f"{int(first):,}",
                "—" if masked_first is None else f"{int(masked_first):,}",
            ])
        else:
            cells.append("—" if first is None else f"{int(first):,}")
        cells.extend([
            f"{int(row['size_delta']):,}",
            "—" if pct is None else json.dumps(pct, allow_nan=False),
        ])
        lines.append("| " + " | ".join(cells) + " |")

    lines.extend([
        "",
        "### Unresolved identities",
        "",
    ])
    if unresolved:
        lines.extend([
            "| File | Symbol | Diagnostic |",
            "|---|---|---|",
        ])
        for key, diagnostic in sorted(
            unresolved, key=lambda row: (row[0][0], row[0][1])
        ):
            lines.append(
                f"| {_markdown_code(key[0])} | {_markdown_code(key[1])} | "
                f"{_markdown_text(diagnostic)} |"
            )
    else:
        lines.append("None.")
    return "\n".join(lines) + "\n"


def replace_generated_markdown(document_text: str, generated: str) -> str:
    """Replace exactly one marked region while preserving authored text."""
    if document_text.count(DOC_BEGIN) != 1 or document_text.count(DOC_END) != 1:
        raise RankingDocumentError(
            "documentation needs exactly one generated begin/end marker"
        )
    begin = document_text.index(DOC_BEGIN)
    end = document_text.index(DOC_END)
    if begin >= end:
        raise RankingDocumentError("documentation generated markers are reversed")
    begin_content = begin + len(DOC_BEGIN)
    return (
        document_text[:begin_content]
        + "\n"
        + generated
        + DOC_END
        + document_text[end + len(DOC_END):]
    )


def expected_document_text(
    ranking_document: object, documentation_path: pathlib.Path
) -> tuple[str, str]:
    current = documentation_path.read_text(encoding="utf-8")
    expected = replace_generated_markdown(
        current, render_ranking_markdown(ranking_document)
    )
    return current, expected


def tu_category(rel_c_file: str) -> str:
    parts = pathlib.PurePosixPath(rel_c_file).parts
    # src/overlays/oNNN/... -> overlays/oNNN ; src/main/... -> main ; src/libultra/... -> libultra
    if len(parts) >= 3 and parts[0] == "src" and parts[1] == "overlays":
        return f"overlays/{parts[2]}"
    if len(parts) >= 2 and parts[0] == "src":
        return parts[1]
    return "?"


def masked_mismatch_positions(
    base_words: list[int],
    target_words: list[int],
    base_reloc: dict[int, tuple[str, str]],
    target_reloc: dict[int, tuple[str, str]],
) -> list[int]:
    """Word indices that still differ once linker-owned fields are masked.

    The masking rule lives here alone. `mismatch_evidence` reduces this to a
    count, and `tools/force_lattice.py` bins the same positions into windows to
    locate an intervention's blast radius; a second copy of the rule would let
    the two disagree about what a residual word even is.
    """
    n = min(len(base_words), len(target_words))
    positions: list[int] = []
    for index in range(n):
        base_word = base_words[index]
        target_word = target_words[index]
        if base_word == target_word:
            continue
        offset = index * 4
        value_mask = 0
        for relocation in (base_reloc.get(offset), target_reloc.get(offset)):
            if relocation is not None:
                value_mask |= RELOC_VALUE_MASKS.get(
                    relocation[0], UNKNOWN_RELOC_VALUE_MASK
                )
        compare_mask = (~value_mask) & 0xFFFFFFFF
        if (base_word & compare_mask) != (target_word & compare_mask):
            positions.append(index)
    return positions


def mismatch_evidence(
    base_words: list[int],
    target_words: list[int],
    base_reloc: dict[int, tuple[str, str]],
    target_reloc: dict[int, tuple[str, str]],
) -> tuple[int, Optional[int], int, Optional[int]]:
    """Return raw and linker-field-masked mismatch counts/first offsets.

    Relocation payload bits are masked at a positional union of both objects'
    relocation surfaces. Words beyond the shared length remain mismatches in
    both views because no relocation can explain a missing instruction.
    """
    n = min(len(base_words), len(target_words))
    raw_positions = [i for i in range(n) if base_words[i] != target_words[i]]
    masked_positions = masked_mismatch_positions(
        base_words, target_words, base_reloc, target_reloc
    )

    extra = abs(len(base_words) - len(target_words))

    def summarize(positions: list[int]) -> tuple[int, Optional[int]]:
        count = len(positions) + extra
        if positions:
            return count, positions[0] * 4
        if extra:
            return count, n * 4
        return 0, None

    raw_count, raw_first = summarize(raw_positions)
    masked_count, masked_first = summarize(masked_positions)
    return raw_count, raw_first, masked_count, masked_first


def classify(
    base_size: int,
    target_size: int,
    base_words: list[int],
    target_words: list[int],
    base_reloc: dict[int, tuple[str, str]],
    target_reloc: dict[int, tuple[str, str]],
) -> tuple[str, int, Optional[int], int, Optional[int]]:
    """Return category plus raw and relocation-masked mismatch evidence."""
    size_delta = base_size - target_size
    n = min(len(base_words), len(target_words))
    diff_positions = [i for i in range(n) if base_words[i] != target_words[i]]
    (
        differing_words,
        first_mismatch_offset,
        relocation_masked_differing_words,
        relocation_masked_first_mismatch_offset,
    ) = mismatch_evidence(base_words, target_words, base_reloc, target_reloc)

    evidence = (
        differing_words,
        first_mismatch_offset,
        relocation_masked_differing_words,
        relocation_masked_first_mismatch_offset,
    )

    if size_delta != 0:
        return ("size-mismatch", *evidence)

    if not diff_positions:
        return ("other", *evidence)

    if Counter(base_words) == Counter(target_words):
        return ("schedule-only", *evidence)

    if all(
        instr_reg_mask(base_words[i]) == instr_reg_mask(target_words[i])
        for i in diff_positions
    ):
        return ("register-only", *evidence)

    if relocation_masked_differing_words == 0:
        return ("reloc-mismatch", *evidence)

    return ("other", *evidence)


@dataclasses.dataclass
class WordStreams:
    """Both instruction streams for one symbol, with their relocation surfaces.

    Split out of `process_item` so that a tool which needs to *align* the two
    streams reads them by exactly the path the ranking scores them by. Two
    lanes wrote their own extractor into scratch and both had to rediscover
    that `objdump -r` prints relocation offsets in section coordinates while a
    hand-rolled reader indexes from the function's base -- see the header of
    `tools/score_symbol.py`. There is now one reader.
    """

    base_words: list[int]
    target_words: list[int]
    base_reloc: dict[int, tuple[str, str]]
    target_reloc: dict[int, tuple[str, str]]
    base_size: int
    target_size: int


def word_streams(
    item: "pb.QueueItem", base_o: pathlib.Path,
) -> tuple[Optional[WordStreams], Optional[str]]:
    """Assemble the target and read both streams out of the two objects."""
    safe = item.func.replace("/", "_")
    out_dir = WORK_DIR / safe
    out_dir.mkdir(parents=True, exist_ok=True)
    target_o = out_dir / "target.o"
    try:
        target_asm = pb.prepare_target_asm(item, out_dir)
        target_o.unlink(missing_ok=True)
        assemble_target(target_asm, target_o)
    except Exception as exc:  # noqa: BLE001 - per-function failure is retained
        return None, f"{item.func} ({item.rel_c_file}): {exc}"
    target_span = func_symbol_span(target_o, item.func)
    base_span = func_symbol_span(base_o, item.func)
    if target_span is None or base_span is None:
        return None, f"{item.func} ({item.rel_c_file}): couldn't find .text symbol in base.o/target.o"
    target_start, target_size = target_span
    base_start, base_size = base_span

    return WordStreams(
        base_words=words_of(text_bytes(base_o, base_start, base_size)),
        target_words=words_of(text_bytes(target_o, target_start, target_size)),
        base_reloc=relocations(base_o, base_start, base_size),
        target_reloc=relocations(target_o, target_start, target_size),
        base_size=base_size,
        target_size=target_size,
    ), None


def process_item(
    item: "pb.QueueItem", base_o: pathlib.Path,
) -> tuple[Optional[FuncResult], Optional[str]]:
    """Compare one owned symbol from the already compiled configured TU."""
    streams, error = word_streams(item, base_o)
    if streams is None:
        return None, error
    base_words = streams.base_words
    target_words = streams.target_words
    base_reloc = streams.base_reloc
    target_reloc = streams.target_reloc
    base_size = streams.base_size
    target_size = streams.target_size

    (
        category,
        differing_words,
        first_mismatch_offset,
        relocation_masked_differing_words,
        relocation_masked_first_mismatch_offset,
    ) = classify(
        base_size, target_size, base_words, target_words, base_reloc, target_reloc
    )

    result = FuncResult(
        name=item.func,
        file=item.rel_c_file,
        overlay=item.overlay,
        tu=tu_category(item.rel_c_file),
        size_bytes=target_size,
        differing_words=differing_words,
        first_mismatch_offset=first_mismatch_offset,
        relocation_masked_differing_words=relocation_masked_differing_words,
        relocation_masked_first_mismatch_offset=(
            relocation_masked_first_mismatch_offset
        ),
        size_delta=base_size - target_size,
        category=category,
    )
    return result, None


def load_objdiff_pct(report_path: Optional[pathlib.Path]) -> dict[str, float]:
    if report_path is None:
        return {}
    data = json.loads(report_path.read_text())
    out: dict[str, float] = {}
    for unit in data.get("units", []):
        for fn in unit.get("functions", []):
            name = fn.get("name")
            pct = fn.get("fuzzy_match_percent")
            if name is not None and pct is not None:
                # A deduplicated/weak symbol can appear in more than one
                # unit; keep the lowest score seen, the more conservative
                # (worse-case) one for triage purposes.
                if name not in out or pct < out[name]:
                    out[name] = pct
    return out


def sort_key(r: FuncResult):
    return (
        CATEGORY_RANK.get(r.category, 99),
        (
            r.relocation_masked_differing_words
            if r.relocation_masked_differing_words is not None
            else r.differing_words
        ),
        r.differing_words,
        r.file,
        r.name,
    )


def print_table(results: list[FuncResult], top: Optional[int], markdown: bool) -> None:
    rows = sorted(results, key=sort_key)
    if top is not None:
        rows = rows[:top]
    headers = [
        "name", "overlay/TU", "size", "objdiff%", "raw_diff",
        "masked_diff", "raw_first", "masked_first", "size_delta", "category",
    ]

    def fmt_row(r: FuncResult) -> list[str]:
        loc = f"o{r.overlay:03d}" if r.overlay is not None else r.tu
        pct = f"{r.objdiff_match_pct:.1f}" if r.objdiff_match_pct is not None else "-"
        raw_first = (
            str(r.first_mismatch_offset)
            if r.first_mismatch_offset is not None
            else "-"
        )
        masked_first = (
            str(r.relocation_masked_first_mismatch_offset)
            if r.relocation_masked_first_mismatch_offset is not None
            else "-"
        )
        return [
            r.name, loc, str(r.size_bytes), pct, str(r.differing_words),
            (
                str(r.relocation_masked_differing_words)
                if r.relocation_masked_differing_words is not None
                else "-"
            ),
            raw_first, masked_first,
            str(r.size_delta), r.category,
        ]

    data_rows = [fmt_row(r) for r in rows]

    if markdown:
        print("| " + " | ".join(headers) + " |")
        print("|" + "|".join("---" for _ in headers) + "|")
        for row in data_rows:
            print("| " + " | ".join(row) + " |")
        return

    widths = [len(h) for h in headers]
    for row in data_rows:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))
    def line(cells: list[str]) -> str:
        return "  ".join(c.ljust(widths[i]) for i, c in enumerate(cells))
    print(line(headers))
    print(line(["-" * w for w in widths]))
    for row in data_rows:
        print(line(row))


def retained_results(document: object) -> list[FuncResult]:
    """Recover display-only rows from a validated retained snapshot."""
    validated = validate_ranking_document(document)
    functions = validated["functions"]
    assert isinstance(functions, list)
    return [
        FuncResult(
            name=str(row["name"]),
            file=str(row["file"]),
            overlay=row["overlay"] if isinstance(row["overlay"], int) else None,
            tu=str(row["tu"]),
            size_bytes=int(row["size_bytes"]),
            differing_words=int(row["differing_words"]),
            first_mismatch_offset=(
                int(row["first_mismatch_offset"])
                if isinstance(row["first_mismatch_offset"], int)
                else None
            ),
            relocation_masked_differing_words=(
                int(row["relocation_masked_differing_words"])
                if isinstance(
                    row.get("relocation_masked_differing_words"), int
                )
                else None
            ),
            relocation_masked_first_mismatch_offset=(
                int(row["relocation_masked_first_mismatch_offset"])
                if isinstance(
                    row.get("relocation_masked_first_mismatch_offset"), int
                )
                else None
            ),
            size_delta=int(row["size_delta"]),
            category=str(row["category"]),
            objdiff_match_pct=(
                float(row["objdiff_match_pct"])
                if isinstance(row["objdiff_match_pct"], (int, float))
                else None
            ),
        )
        for row in functions
        if isinstance(row, dict)
    ]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--objdiff-report", type=pathlib.Path, default=None,
                     help="objdiff-cli 'report generate' JSON, run against build_non_matching/ "
                          "vs expected/build/ (see docs/nm-ranking.md). Optional: supplies "
                          "objdiff_match_pct where available.")
    ap.add_argument("--jobs", type=int, default=8,
                     help="parallel configured TU compiles (default 8)")
    ap.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT,
                     help=f"where to write the ranking JSON (default {DEFAULT_OUT})")
    ap.add_argument(
        "--doc",
        type=pathlib.Path,
        default=DEFAULT_DOC,
        help=f"generated Markdown document (default {DEFAULT_DOC})",
    )
    doc_mode = ap.add_mutually_exclusive_group()
    doc_mode.add_argument(
        "--write-doc",
        action="store_true",
        help="validate --out and update only the marked region in --doc",
    )
    doc_mode.add_argument(
        "--check-doc",
        action="store_true",
        help="validate --out and fail if the marked region in --doc has drifted",
    )
    ap.add_argument(
        "--top",
        type=int,
        default=None,
        help="only print the top N rows (requires --show-retained or --refresh-stale)",
    )
    ap.add_argument(
        "--markdown",
        action="store_true",
        help="print the table as markdown (requires --show-retained or --refresh-stale)",
    )
    ap.add_argument("--no-table", action="store_true", help="skip printing the table")
    ap.add_argument(
        "--show-retained",
        action="store_true",
        help="display the validated --out snapshot without compiling candidates",
    )
    ap.add_argument("--limit", type=int, default=None,
                     help="process at most N queue items; with --refresh-stale, "
                          "deferred rows remain explicitly unproven")
    maintenance_mode = ap.add_mutually_exclusive_group()
    maintenance_mode.add_argument(
        "--check-freshness", action="store_true",
        help="without compiling, fail if live identities or source contexts "
             "differ from the complete retained ranking",
    )
    ap.add_argument(
        "--json", action="store_true",
        help="emit machine-readable coverage (requires --check-freshness)",
    )
    maintenance_mode.add_argument(
        "--check-retired", action="store_true",
        help="without compiling, fail only if the retained ranking lists a "
             "function that is no longer queued (already matched). Narrower "
             "than --check-freshness, which also fails on stale scores and so "
             "cannot gate ordinary commits",
    )
    maintenance_mode.add_argument(
        "--prune-stale",
        action="store_true",
        help="without compiling, remove rows from --out whose exact file/symbol "
             "identity is no longer in the current NON_MATCHING queue; validates "
             "the retained document and reports newly queued unranked functions",
    )
    maintenance_mode.add_argument(
        "--refresh-stale",
        action="store_true",
        help="validate and merge only changed, newly queued, and previously "
             "unresolved identities; retain proven-fresh measurements",
    )
    ap.add_argument(
        "--evidence-ref",
        default="HEAD",
        help="Git ref used only to migrate legacy rows without embedded source "
             "evidence (default HEAD)",
    )
    args = ap.parse_args()

    if args.json and not args.check_freshness:
        ap.error("--json requires --check-freshness")
    if args.check_freshness:
        if any((args.show_retained, args.write_doc, args.check_doc,
                args.objdiff_report is not None, args.limit is not None,
                args.top is not None, args.markdown, args.no_table)):
            ap.error("--check-freshness cannot be combined with display, "
                     "documentation, or compilation options")
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            contexts = current_source_contexts(pb.discover_queue())
            legacy = legacy_source_contexts(args.evidence_ref, args.out, document)
            coverage = source_coverage(document, contexts, legacy)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(f"ranking freshness: {exc}", file=sys.stderr)
            return 2
        if args.json:
            print(json.dumps(coverage, indent=2, sort_keys=True))
        else:
            print("ranking freshness: " + coverage_summary(coverage))
        if not coverage["complete"]:
            print("ranking freshness: run tools/nm_ranking.py --refresh-stale "
                  "to measure all stale/new/unresolved rows", file=sys.stderr)
        return 0 if coverage["complete"] else 1

    if args.check_retired:
        # Deliberately narrower than --check-freshness. A *stale score* is a
        # normal consequence of editing a candidate-bearing TU, so gating on
        # it would fail nearly every commit. A *retired row* is different: it
        # names a function that has been matched and is no longer queued, so
        # the row is not out of date, it is wrong. Planning reads this file to
        # choose targets, and a retired row invites re-deriving a match that
        # already landed -- which is exactly what happened on 2026-09-10, when
        # a sweep for work stranded on unmerged branches reported a function
        # as still queued hours after it was matched, briefly making a closed
        # problem look like an open systemic one.
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            contexts = current_source_contexts(pb.discover_queue())
            legacy = legacy_source_contexts(args.evidence_ref, args.out, document)
            coverage = source_coverage(document, contexts, legacy)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(f"ranking retired-check: {exc}", file=sys.stderr)
            return 2
        retired = coverage["retired"]
        if not retired:
            print(f"ranking: no retired rows ({coverage['retained']} retained, "
                  f"{coverage['live']} live)")
            return 0
        print(f"ranking lists {len(retired)} function(s) that are no longer "
              f"queued (already matched):", file=sys.stderr)
        for key in retired[:10]:
            print(f"  {'::'.join(str(part) for part in key)}", file=sys.stderr)
        if len(retired) > 10:
            print(f"  (and {len(retired) - 10} more)", file=sys.stderr)
        print("run tools/nm_ranking.py --refresh-stale", file=sys.stderr)
        return 1

    if args.jobs < 1:
        print("error: --jobs must be at least 1", file=sys.stderr)
        return 2
    if args.limit is not None and args.limit < 1:
        print("error: --limit must be at least 1", file=sys.stderr)
        return 2
    if (
        (args.top is not None or args.markdown)
        and not args.show_retained
        and not args.refresh_stale
    ):
        print(
            "error: --top/--markdown require --show-retained for a "
            "compile-free snapshot view (or --refresh-stale)",
            file=sys.stderr,
        )
        return 2

    if args.show_retained:
        incompatible = []
        if args.objdiff_report is not None:
            incompatible.append("--objdiff-report")
        if args.limit is not None:
            incompatible.append("--limit")
        if args.no_table:
            incompatible.append("--no-table")
        if args.write_doc:
            incompatible.append("--write-doc")
        if args.check_doc:
            incompatible.append("--check-doc")
        if args.prune_stale:
            incompatible.append("--prune-stale")
        if args.refresh_stale:
            incompatible.append("--refresh-stale")
        if args.evidence_ref != "HEAD":
            incompatible.append("--evidence-ref")
        if incompatible:
            print(
                "error: --show-retained cannot be combined with "
                + ", ".join(incompatible),
                file=sys.stderr,
            )
            return 2
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            results = retained_results(document)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to display retained ranking: {exc}",
                file=sys.stderr,
            )
            return 2
        print_table(results, args.top, args.markdown)
        return 0

    if args.write_doc or args.check_doc:
        incompatible = []
        if args.objdiff_report is not None:
            incompatible.append("--objdiff-report")
        if args.limit is not None:
            incompatible.append("--limit")
        if args.top is not None:
            incompatible.append("--top")
        if args.markdown:
            incompatible.append("--markdown")
        if args.no_table:
            incompatible.append("--no-table")
        if args.prune_stale:
            incompatible.append("--prune-stale")
        if args.refresh_stale:
            incompatible.append("--refresh-stale")
        if args.evidence_ref != "HEAD":
            incompatible.append("--evidence-ref")
        if incompatible:
            print(
                "error: documentation mode cannot be combined with "
                + ", ".join(incompatible),
                file=sys.stderr,
            )
            return 2
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            current_doc, expected_doc = expected_document_text(document, args.doc)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to process ranking documentation: {exc}",
                file=sys.stderr,
            )
            return 2
        if args.write_doc:
            write_text_atomic(args.doc, expected_doc)
            print(f"updated generated ranking in {args.doc}", file=sys.stderr)
            return 0
        if current_doc != expected_doc:
            print(
                f"error: generated ranking documentation is stale: {args.doc}",
                file=sys.stderr,
            )
            sys.stderr.writelines(
                difflib.unified_diff(
                    current_doc.splitlines(keepends=True),
                    expected_doc.splitlines(keepends=True),
                    fromfile=str(args.doc),
                    tofile=f"{args.doc} (generated)",
                )
            )
            return 1
        print(f"generated ranking documentation is current: {args.doc}")
        return 0

    if args.refresh_stale:
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            validate_ranking_document(document)
            live_items = pb.discover_queue()
            initial_contexts = current_source_contexts(live_items)
            legacy_context = legacy_source_contexts(
                args.evidence_ref, args.out, document
            )
            plan = plan_incremental_refresh(
                document,
                live_items,
                initial_contexts,
                legacy_context,
                limit=args.limit,
            )
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to refresh {args.out}: {exc}",
                file=sys.stderr,
            )
            return 2

        if plan.selected:
            missing = missing_permuter_inputs()
            if missing:
                return report_missing_permuter(missing, args.out)
            WORK_DIR.mkdir(parents=True, exist_ok=True)
            results, errors = process_items(plan.selected, args.jobs)
            if errors:
                print(
                    f"error: {len(errors)}/{len(plan.selected)} incremental "
                    f"item(s) failed; refusing to overwrite {args.out}",
                    file=sys.stderr,
                )
                for _key, error in errors[:5]:
                    print(f"  {error}", file=sys.stderr)
                return 2
        else:
            results = []

        # A refresh may overlap source edits. Re-discover the complete queue,
        # not only the selected subset, and reject any membership or context
        # movement so no measurement is attached to a different source state.
        try:
            final_items = pb.discover_queue()
            final_contexts = current_source_contexts(final_items)
            final_keys = {(item.rel_c_file, item.func) for item in final_items}
            if final_keys != set(plan.live_by_key):
                raise RankingDocumentError(
                    "live NON_MATCHING queue changed during incremental refresh"
                )
            if final_contexts != plan.current_contexts:
                raise RankingDocumentError(
                    "source context changed during incremental refresh"
                )
            pct_by_name = load_objdiff_pct(args.objdiff_report)
            for result in results:
                result.objdiff_match_pct = pct_by_name.get(result.name)
            merged = merge_incremental_results(
                document,
                plan,
                results,
                objdiff_report_used=args.objdiff_report is not None,
            )
            publish_ranking_document(args.out, merged, args.doc)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to publish incremental refresh: {exc}",
                file=sys.stderr,
            )
            return 2

        deferred = len(plan.deferred_rows) + len(plan.deferred_unresolved)
        print(
            f"incremental refresh: selected={len(plan.selected)} "
            f"fresh-retained={len(plan.fresh_rows)} deferred={deferred} "
            f"removed={len(plan.removed)} new={plan.new_count}",
            file=sys.stderr,
        )
        if deferred:
            print(
                "note: bounded refresh left deferred identities explicitly "
                "unproven; rerun without --limit to finish",
                file=sys.stderr,
            )
        if not args.no_table:
            print_table(results, args.top, args.markdown)
        return 0

    if args.prune_stale:
        incompatible = []
        if args.objdiff_report is not None:
            incompatible.append("--objdiff-report")
        if args.limit is not None:
            incompatible.append("--limit")
        if args.top is not None:
            incompatible.append("--top")
        if args.markdown:
            incompatible.append("--markdown")
        if incompatible:
            print(
                "error: --prune-stale cannot be combined with "
                + ", ".join(incompatible),
                file=sys.stderr,
            )
            return 2
        try:
            document = json.loads(args.out.read_text(encoding="utf-8"))
            live_keys = {
                (item.rel_c_file, item.func) for item in pb.discover_queue()
            }
            pruned, removed, unranked = prune_stale_document(document, live_keys)
            expected_doc: Optional[str] = None
            if args.out.resolve() == DEFAULT_OUT.resolve():
                _, expected_doc = expected_document_text(pruned, args.doc)
            write_json_atomic(args.out, pruned)
            if expected_doc is not None:
                write_text_atomic(args.doc, expected_doc)
        except (OSError, json.JSONDecodeError, RankingDocumentError) as exc:
            print(
                f"error: refusing to prune {args.out}: {exc}", file=sys.stderr
            )
            return 2
        print(
            f"pruned {len(removed)} stale row(s); "
            f"retained {pruned['queue_size']} ranked/unresolved row(s)",
            file=sys.stderr,
        )
        if unranked:
            print(
                f"note: {len(unranked)} live queue item(s) are absent from the "
                "snapshot and remain unranked; run a full ranking pass to add them",
                file=sys.stderr,
            )
        return 0

    # The permuter is intentionally installed separately from this repository.
    # Fail before touching the committed queue when that dependency is absent;
    # otherwise every item reports the same import error and a successful exit
    # silently replaces a useful ranking with an empty one.
    missing = missing_permuter_inputs()
    if missing:
        return report_missing_permuter(missing, args.out)

    WORK_DIR.mkdir(parents=True, exist_ok=True)

    queue = pb.discover_queue()
    if args.limit:
        queue = queue[: args.limit]

    initial_contexts = current_source_contexts(queue)
    results, errors = process_items(queue, args.jobs)

    # A full ranking pass can overlap matching work for hours. Re-scan the
    # canonical source immediately before publishing so functions promoted
    # while this process was compiling do not survive as stale ranking rows.
    # Filter the original queue too, preserving --limit and the reported
    # queue/resolution counts for the exact set this invocation processed.
    live_keys = {
        (item.rel_c_file, item.func)
        for item in pb.discover_queue_from_source_scan()
    }
    queue = [
        item for item in queue
        if (item.rel_c_file, item.func) in live_keys
    ]
    results = [
        result for result in results
        if (result.file, result.name) in live_keys
    ]
    errors = [(key, error) for key, error in errors if key in live_keys]

    pct_by_name = load_objdiff_pct(args.objdiff_report)
    for r in results:
        r.objdiff_match_pct = pct_by_name.get(r.name)

    results.sort(key=sort_key)

    if queue and not results:
        print(
            f"error: 0/{len(queue)} candidates resolved; refusing to overwrite "
            f"{args.out}",
            file=sys.stderr,
        )
        if errors:
            print(f"first failure: {errors[0][1]}", file=sys.stderr)
        return 2

    final_contexts = current_source_contexts(queue)
    if final_contexts != initial_contexts:
        print(
            "error: source context changed during full ranking; refusing to "
            f"overwrite {args.out}",
            file=sys.stderr,
        )
        return 2
    out_doc = make_ranking_document(
        [
            row_from_result(r, final_contexts[(r.file, r.name)])
            for r in results
        ],
        [[list(key), error] for key, error in errors],
        objdiff_report_used=args.objdiff_report is not None,
    )
    publish_ranking_document(args.out, out_doc, args.doc)

    print(f"{len(results)}/{len(queue)} queued functions resolved "
          f"({len(errors)} could not be measured from configured TUs)", file=sys.stderr)
    counts = Counter(r.category for r in results)
    print("category counts: " + ", ".join(f"{k}={v}" for k, v in sorted(counts.items())),
          file=sys.stderr)

    if not args.no_table:
        print_table(results, args.top, args.markdown)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
