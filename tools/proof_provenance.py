#!/usr/bin/env python3
"""Prove what source selected bytes in a workbench comparison came from.

The workbench can compare any two objects, including an ordinary build object
whose selected function was inserted by asm-processor from GLOBAL_ASM.  Equal
bytes in that case are useful diagnostic evidence, but they are not evidence
that C reproduced the function.  This tool records the distinction before
``wb_compare.sh`` invokes the comparator.

The manifest deliberately lives under ``build/wb``.  It contains hashes and
build provenance, can mention ROM-derived artifacts, and must never be tracked.
"""

from __future__ import annotations

import argparse
import bisect
import collections
import dataclasses
import hashlib
import json
import os
import pathlib
import re
import shlex
import subprocess
import sys
import tempfile
import time
from typing import Iterable, Sequence


class MetadataProofError(RuntimeError):
    """Declared metadata steps do not explain the configured object."""


def _filter_spec_file(argument: str, root: pathlib.Path) -> pathlib.Path:
    """Resolve a filter ``@SPEC_FILE`` exactly as filter_elf_relocations.py does."""
    relative = pathlib.PurePosixPath(argument[1:])
    if not argument[1:] or relative.is_absolute() or ".." in relative.parts:
        raise MetadataProofError("unsafe filter specification file")
    path = root / relative
    if path.is_symlink() or not path.is_file():
        raise MetadataProofError("missing filter specification file")
    return path


def filter_spec_files(command: str, root: pathlib.Path) -> dict[str, str]:
    """Digest every ``@SPEC_FILE`` a metadata recipe names (content is a build input)."""
    specs = {}
    for segment in command.split("&&"):
        words = shlex.split(segment)
        if len(words) >= 2 and words[1].endswith("filter_elf_relocations.py"):
            for word in words[4:]:
                if word.startswith("@"):
                    specs[word[1:]] = sha256_file(_filter_spec_file(word, root))
    return specs


def metadata_filter_plan(command: str, target: str,
                         root: pathlib.Path | None = None) -> list[tuple[str, object]]:
    """Accept only ordered renames, exact text filters and a trailing text trim.

    A filter argument may be ``@SPEC_FILE``, read the way the filter helper
    reads it: whitespace-separated specifications, ``#`` comments ignored.
    """
    root = pathlib.Path(__file__).resolve().parent.parent if root is None else root
    # These are precisely the existing canonical Makefile substitutions, not
    # arbitrary shell expansion or executable lookup.
    for token, replacement in {"$(OBJCOPY)": "tools/binutils/mips64-elf-objcopy",
                               "$(HOST_PYTHON)": ".venv/bin/python",
                               "$(TOOLS_DIR)": "tools", "$@": target}.items():
        command = command.replace(token, replacement)
    plan = []
    for segment in command.split("&&"):
        words = shlex.split(segment)
        if not words or any(token in {";", "|", "||", ">", "<"} or "$" in token for token in words):
            raise MetadataProofError("unsupported metadata command syntax")
        if words[0] == "tools/binutils/mips64-elf-objcopy" and words[-1] == target:
            args, pairs = words[1:-1], []
            while args:
                if args[0] != "--redefine-sym" or len(args) < 2 or args[1].count("=") != 1:
                    raise MetadataProofError("unsupported objcopy metadata operation")
                old, new = args[1].split("=")
                if not old or not new:
                    raise MetadataProofError("empty rename identity")
                pairs.append((old, new))
                args = args[2:]
            plan.append(("rename", pairs))
        elif (len(words) >= 5 and pathlib.Path(words[0]).name.startswith("python")
              and words[2:4] == [target, ".text"]):
            if words[1] == "tools/filter_elf_relocations.py":
                requests = []
                expanded = []
                for word in words[4:]:
                    if not word.startswith("@"):
                        expanded.append(word)
                        continue
                    for line in _filter_spec_file(word, root).read_text().splitlines():
                        expanded.extend(line.partition("#")[0].split())
                for spec in expanded:
                    try:
                        offset, kind, name = spec.split(":", 2)
                        row = (int(offset, 0), int(kind, 0), name)
                    except ValueError as error:
                        raise MetadataProofError("unsupported filter specification") from error
                    if row[0] < 0 or row[0] % 4 or row[1] not in {4, 5, 6} or not row[2] or row in requests:
                        raise MetadataProofError("invalid or duplicate filter specification")
                    requests.append(row)
                plan.append(("filter", requests))
            elif words[1] == "tools/trim_elf_section.py" and len(words) == 5:
                plan.append(("trim", int(words[4], 0)))
            else:
                raise MetadataProofError("unsupported metadata helper")
        else:
            raise MetadataProofError("unsupported ordered metadata operation")
    import reloc_identity as ri
    closure = ri.canonicalize_redefine_aliases([pair for operation, pairs in plan
                                               if operation == "rename" for pair in pairs])
    if closure.ambiguous or closure.cycles:
        raise MetadataProofError("ambiguous metadata symbol rename identities")
    return plan


def validate_metadata_objects(raw, configured, plan, symbol: str) -> dict:
    """Account for every text relocation and all allocated bytes, without editing either object."""
    import reloc_surface as rs
    def rows(elf):
        syms = elf.symbols()
        result = []
        for section, offset, kind, index in elf.relocations(r".*"):
            name, value, size, info, shndx = syms[index]
            owner = elf.names[shndx] if shndx < len(elf.names) else shndx
            result.append((section, offset, kind, name, value, size, info, owner))
        return result
    expected = rows(raw)
    symbols = lambda elf: [(name, value, size, info, elf.names[index] if index < len(elf.names) else index)
                           for name, value, size, info, index in elf.symbols()]
    expected_symbols = symbols(raw)
    removed = []
    text = raw.section_bytes(".text")
    start, size = rs._function_text_symbol(raw, [symbol])
    if rs._function_text_symbol(configured, [symbol]) != (start, size):
        raise MetadataProofError("metadata changed owned function geometry")
    for operation, arguments in plan:
        if operation == "rename":
            for old, new in arguments:
                expected = [row[:3] + (new if row[3] == old else row[3],) + row[4:] for row in expected]
                expected_symbols = [(new if row[0] == old else row[0],) + row[1:] for row in expected_symbols]
        elif operation == "filter":
            for offset, kind, name in arguments:
                matches = [row for row in expected if row[:4] == (".text", offset, kind, name)]
                if len(matches) != 1:
                    raise MetadataProofError("declared filter lacks exactly one raw relocation")
                expected.remove(matches[0])
                removed.append(matches[0])
        elif operation == "trim":
            if arguments < start + size or arguments > len(text) or any(text[arguments:]):
                raise MetadataProofError("trim would remove owned instructions or nonzero bytes")
            text_index = raw.section(".text")[0]
            if any(index == text_index and info & 15 == 2 and value + extent > arguments
                   for _name, value, extent, info, index in raw.symbols()):
                raise MetadataProofError("trim would remove a neighboring function extent")
            text = text[:arguments]
        else:
            raise MetadataProofError("unsupported metadata plan operation")
    if text != configured.section_bytes(".text"):
        raise MetadataProofError("raw/configured instruction bytes differ")
    def allocated(elf):
        result = {}
        for name, header in zip(elf.names, elf.sh):
            if not header[2] & 2:
                continue
            if name in result:
                raise MetadataProofError("duplicate allocated section ownership")
            result[name] = header
        return result
    raw_sections, configured_sections = allocated(raw), allocated(configured)
    if set(raw_sections) != set(configured_sections):
        raise MetadataProofError("allocated section ownership changed")
    for name, header in raw_sections.items():
        other = configured_sections[name]
        if name == ".text":
            if (header[1], header[2], header[3], header[8]) != (other[1], other[2], other[3], other[8]) or other[5] != len(text):
                raise MetadataProofError("text section geometry changed outside declared trim")
            continue
        if (header[1], header[2], header[3], header[5], header[8]) != (other[1], other[2], other[3], other[5], other[8]):
            raise MetadataProofError("allocated section geometry changed")
        if header[1] != 8 and raw.section_bytes(name) != configured.section_bytes(name):
            raise MetadataProofError("allocated data bytes changed")
    if collections.Counter(expected) != collections.Counter(rows(configured)):
        raise MetadataProofError("unlisted relocation removal, addition or retargeting")
    if collections.Counter(expected_symbols) != collections.Counter(symbols(configured)):
        raise MetadataProofError("unlisted symbol table change")
    owned = [row for row in rows(raw) if row[0] == ".text" and start <= row[1] < start + size]
    filtered = [row for row in removed if start <= row[1] < start + size]
    if not filtered:
        raise MetadataProofError("no declared filters belong to the requested function")
    return {"raw_count": len(owned), "retained_count": len(owned) - len(filtered),
            "filtered": [{"offset": row[1] - start, "rtype": row[2], "symbol": row[3]}
                         for row in filtered], "start": start, "size": size}


def linked_bss_base(raw, configured, linked, overlay: int, runtime_base: int, runtime_size=None) -> tuple[int, int]:
    """Derive a TU BSS base from unique matching raw/configured/linked definitions."""
    section = f".overlay_{overlay:03d}_bss"
    matches = [(i, sh) for i, (name, sh) in enumerate(zip(linked.names, linked.sh)) if name == section]
    if len(matches) != 1:
        raise MetadataProofError("ambiguous linked BSS section ownership")
    linked_index, header = matches[0]
    if header[1] != 8 or not header[2] & 2 or (runtime_size is not None and header[5] > runtime_size):
        raise MetadataProofError("linked BSS exceeds canonical runtime ownership")
    raw_index, raw_header = raw.section(".bss")
    configured_index, configured_header = configured.section(".bss")
    if raw_index is None or configured_index is None or raw_header[5] != configured_header[5]:
        raise MetadataProofError("missing or changed canonical BSS extent")
    definitions = [row for row in raw.symbols() if row[4] == raw_index and row[0] and row[0] != ".bss"]
    if not definitions:
        raise MetadataProofError("raw BSS has no named canonical ownership witness")
    bases = set()
    for name, value, size, info, _section in definitions:
        current = [row for row in configured.symbols() if row[0] == name and row[4] == configured_index]
        # Absolute diagnostic aliases are not physical section definitions.
        # Multiple real section owners remain ambiguous, even across overlays.
        final = [row for row in linked.symbols() if row[0] == name and 0 < row[4] < len(linked.sh)]
        if len(current) != 1 or len(final) != 1 or current[0][1:4] != (value, size, info) or final[0][2:] != (size, info, linked_index):
            raise MetadataProofError("ambiguous or inconsistent canonical BSS symbol witness")
        base = final[0][1] - header[3] - value
        if base < 0 or base + raw_header[5] > header[5]:
            raise MetadataProofError("raw BSS definition escapes linked section ownership")
        bases.add(base)
    if len(bases) != 1:
        raise MetadataProofError("canonical BSS definitions disagree on section placement")
    return overlay, runtime_base + next(iter(bases))


ORDINARY_C = "ordinary_c"
NON_MATCHING_C = "non_matching_candidate"
GLOBAL_ASM = "global_asm_fallback"
UNKNOWN = "unknown"

_PRAGMA_RE = re.compile(
    r'^\s*#\s*pragma\s+GLOBAL_ASM\s*\(\s*"(?P<path>[^"]+\.s)"\s*\)',
    re.MULTILINE,
)
_DIRECTIVE_RE = re.compile(
    r"^\s*#\s*(?P<kind>ifdef|ifndef|if|elif|else|endif)\b(?P<arg>.*)$"
)
_DEFINED_NM_RE = re.compile(
    r"(?:defined\s*\(\s*NON_MATCHING\s*\)|defined\s+NON_MATCHING|\bNON_MATCHING\b)"
)


@dataclasses.dataclass(frozen=True)
class Occurrence:
    line: int
    non_matching_state: bool | None


@dataclasses.dataclass(frozen=True)
class PragmaOccurrence(Occurrence):
    path: str
    symbol: str


@dataclasses.dataclass(frozen=True)
class SourceFacts:
    definitions: tuple[Occurrence, ...]
    pragmas: tuple[PragmaOccurrence, ...]


@dataclasses.dataclass
class _Conditional:
    mentions_non_matching: bool
    true_when_non_matching: bool | None


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


_LOADED_METADATA_PROOF = sha256_file(pathlib.Path(__file__))


def capture_configured_raw(root, source, configured, linked, postprocess):
    """Fresh reproduction only: never authenticate an externally supplied raw object."""
    import permute_batch as batch
    import sweep_receipts as receipts
    if root.resolve() != batch.ROOT.resolve() or os.environ.get("PROMOTION_TRIAL", "") not in ("", "0"):
        raise MetadataProofError("unsupported raw-proof root or relaxed trial environment")
    target = configured.relative_to(root).as_posix()
    deadline = time.monotonic() + 120
    def recipe():
        output = batch.bounded_capture(
            ["gmake", "--no-print-directory", "-n", "-W", source.relative_to(root).as_posix(), target],
            deadline, check=True).stdout.replace("\\\n", " ").splitlines()
        compilers = [line.strip() for line in output if target in line and "tools/ido/cc" in line]
        metadata = [line.strip() for line in output if target in line and "tools/filter_elf_relocations.py" in line]
        if len(compilers) != 1 or len(metadata) != 1:
            raise MetadataProofError("ambiguous expanded compiler/metadata recipe")
        return compilers[0], metadata[0]
    command, expanded_postprocess = recipe()
    args = batch.compiler_arguments(command, source.relative_to(root).as_posix(), target)
    words = shlex.split(command)
    wrapped = len(words) > 1 and words[1] == "tools/asm-processor/build.py"
    dependency_args = args + (("-I", str(source.parent)) if wrapped else ())
    if words.count("-o") != 1:
        raise MetadataProofError("ambiguous configured output argument")
    plan = metadata_filter_plan(expanded_postprocess, target, root)
    def context():
        batch.checked_tool_identity()
        if sha256_file(pathlib.Path(__file__)) != _LOADED_METADATA_PROOF:
            raise MetadataProofError("loaded metadata proof implementation changed")
        if recipe() != (command, expanded_postprocess):
            raise MetadataProofError("configured compiler recipe changed during raw proof")
        inputs = {"source": sha256_file(source), "configured": sha256_file(configured),
                  "linked": sha256_file(linked), "command": command, "postprocess": expanded_postprocess,
                  "dependencies": batch.source_dependencies(source, dependency_args, deadline),
                  "wrapper_source_directory": receipts.tree_digest(source.parent) if wrapped else None,
                  "include_tree": receipts.tree_digest(root / "include"),
                  "tools": batch.sweep_tool_identity(),
                  "asm_processor": receipts.tree_digest(root / "tools/asm-processor"),
                  "makefiles": {p.relative_to(root).as_posix(): sha256_file(p)
                                for p in [root / "Makefile", *sorted((root / "mk").glob("**/*.mk"))]},
                  "proof": _LOADED_METADATA_PROOF,
                  "target_inputs": {name: sha256_file(root / name) for name in
                    ("baseroms/mickey.us.z64", "config/overlays.us.json",
                     "overlay_undefined_syms.us.txt", "symbol_addrs.us.txt")},
                  "filter_specs": filter_spec_files(expanded_postprocess, root),
                  "metadata_tools": {name: sha256_file(root / name) for name in
                    ("tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
                     "tools/binutils/mips64-elf-objcopy")}}
        assembly = {}
        for pragma in _all_pragmas(source.read_text()):
            path = root / pragma.path
            data = path.read_text()
            # This bounded capture does not guess nested assembler dependency lookup.
            for directive, name in re.findall(r'^\s*\.(include|incbin)\s+"([^"]+)"', data, re.M):
                if directive == "include" and (root / "include" / name).is_file():
                    continue  # entire canonical include tree is pinned above
                raise MetadataProofError("unsupported nested assembly dependency in raw proof")
            assembly[pragma.path] = sha256_file(path)
        inputs["assembly"] = assembly
        return inputs
    before = context()
    parent = root / "build/metadata-filter-proof"
    parent.mkdir(parents=True, exist_ok=True)
    directory = pathlib.Path(tempfile.mkdtemp(prefix="capture-", dir=parent))
    raw = directory / "raw.o"
    words[words.index("-o") + 1] = str(raw)
    (directory / "inputs.json").write_text(json.dumps(before, sort_keys=True, indent=2))
    try:
        result = batch.bounded_capture(words, deadline, check=True)
    except BaseException as failure:
        (directory / "compile.log").write_text(str(getattr(failure, "output", "")) + "\n" + repr(failure))
        raise
    (directory / "compile.log").write_text(result.stdout)
    if before != context():
        raise MetadataProofError("source, input, recipe, object or tool changed during raw proof")
    receipt = {"schema": "mickey-declared-metadata-proof-v1", "inputs": before,
               "raw_object": raw.relative_to(root).as_posix(), "raw_sha256": sha256_file(raw),
               "configured_sha256": sha256_file(configured),
               "linked_sha256": sha256_file(linked), "directory": directory.relative_to(root).as_posix()}
    return raw, plan, receipt, context


def _mask_c(text: str) -> str:
    """Mask comments and literals while preserving offsets and newlines."""

    out = list(text)
    i = 0
    state = "code"
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line_comment"
                continue
            if char == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block_comment"
                continue
            if char == '"':
                out[i] = " "
                i += 1
                state = "string"
                continue
            if char == "'":
                out[i] = " "
                i += 1
                state = "char"
                continue
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                out[i] = " "
            i += 1
            continue
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
                continue
            if char != "\n":
                out[i] = " "
            i += 1
            continue
        else:
            delimiter = '"' if state == "string" else "'"
            if char == "\\":
                out[i] = " "
                if i + 1 < len(text):
                    if text[i + 1] != "\n":
                        out[i + 1] = " "
                    i += 2
                    continue
            if char == delimiter:
                out[i] = " "
                state = "code"
            elif char != "\n":
                out[i] = " "
            i += 1
            continue
        i += 1
    return "".join(out)


def _directive_condition(kind: str, argument: str) -> _Conditional:
    argument = argument.strip()
    if kind == "ifdef":
        mentions = argument.split(None, 1)[0] == "NON_MATCHING" if argument else False
        return _Conditional(mentions, True if mentions else None)
    if kind == "ifndef":
        mentions = argument.split(None, 1)[0] == "NON_MATCHING" if argument else False
        return _Conditional(mentions, False if mentions else None)
    mentions = bool(_DEFINED_NM_RE.search(argument))
    if not mentions:
        return _Conditional(False, None)
    compact = re.sub(r"\s+", "", argument)
    negative = (
        "!defined(NON_MATCHING)" in compact
        or compact.startswith("!NON_MATCHING")
        or compact in {"NON_MATCHING==0", "0==NON_MATCHING"}
    )
    return _Conditional(True, not negative)


def _line_non_matching_states(text: str) -> list[bool | None]:
    states: list[bool | None] = []
    stack: list[_Conditional] = []
    for line in text.splitlines(keepends=True):
        selected = [
            frame.true_when_non_matching
            for frame in stack
            if frame.mentions_non_matching
        ]
        states.append(selected[-1] if selected else None)
        match = _DIRECTIVE_RE.match(line)
        if not match:
            continue
        kind = match.group("kind")
        argument = match.group("arg")
        if kind in {"ifdef", "ifndef", "if"}:
            stack.append(_directive_condition(kind, argument))
        elif kind == "else" and stack:
            frame = stack[-1]
            if frame.mentions_non_matching and frame.true_when_non_matching is not None:
                frame.true_when_non_matching = not frame.true_when_non_matching
        elif kind == "elif" and stack:
            stack[-1] = _directive_condition("if", argument)
        elif kind == "endif" and stack:
            stack.pop()
    if not text.endswith("\n"):
        # splitlines() already emitted the final unterminated line.
        return states
    return states


def _line_starts(text: str) -> list[int]:
    starts = [0]
    starts.extend(match.end() for match in re.finditer("\n", text))
    return starts


def _line_number(starts: Sequence[int], offset: int) -> int:
    return bisect.bisect_right(starts, offset)


def _state_at(states: Sequence[bool | None], line: int) -> bool | None:
    return states[line - 1] if 0 < line <= len(states) else None


def _matching_paren(masked: str, opening: int) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "(":
            depth += 1
        elif masked[index] == ")":
            depth -= 1
            if depth == 0:
                return index
    return None


def _brace_depth(masked: str, stop: int) -> int:
    depth = 0
    for char in masked[:stop]:
        if char == "{":
            depth += 1
        elif char == "}" and depth:
            depth -= 1
    return depth


def source_facts(text: str, symbol: str) -> SourceFacts:
    """Find global definitions and GLOBAL_ASM fallbacks for ``symbol``."""

    states = _line_non_matching_states(text)
    starts = _line_starts(text)
    masked = _mask_c(text)
    definitions: list[Occurrence] = []
    pattern = re.compile(rf"\b{re.escape(symbol)}\s*\(")
    for match in pattern.finditer(masked):
        if _brace_depth(masked, match.start()) != 0:
            continue
        opening = masked.find("(", match.start(), match.end())
        closing = _matching_paren(masked, opening)
        if closing is None:
            continue
        tail = closing + 1
        while tail < len(masked) and masked[tail].isspace():
            tail += 1
        if tail >= len(masked) or masked[tail] != "{":
            continue
        line = _line_number(starts, match.start())
        definitions.append(Occurrence(line, _state_at(states, line)))

    pragmas: list[PragmaOccurrence] = []
    for match in _PRAGMA_RE.finditer(text):
        path = match.group("path")
        pragma_symbol = pathlib.PurePosixPath(path).stem
        if pragma_symbol != symbol:
            continue
        line = _line_number(starts, match.start())
        pragmas.append(
            PragmaOccurrence(line, _state_at(states, line), path, pragma_symbol)
        )
    return SourceFacts(tuple(definitions), tuple(pragmas))


def _all_pragmas(text: str) -> tuple[PragmaOccurrence, ...]:
    states = _line_non_matching_states(text)
    starts = _line_starts(text)
    result: list[PragmaOccurrence] = []
    for match in _PRAGMA_RE.finditer(text):
        path = match.group("path")
        line = _line_number(starts, match.start())
        result.append(
            PragmaOccurrence(
                line,
                _state_at(states, line),
                path,
                pathlib.PurePosixPath(path).stem,
            )
        )
    return tuple(result)


def classify_source_selection(
    text: str,
    *,
    candidate_symbol: str,
    target_symbol: str,
    defines: Iterable[str],
) -> tuple[str, str]:
    """Classify the bytes selected by the effective preprocessor mode."""

    defined = set(defines)
    candidate = source_facts(text, candidate_symbol)
    target = source_facts(text, target_symbol)
    definitions = candidate.definitions
    pragmas = tuple({p.path: p for p in (*candidate.pragmas, *target.pragmas)}.values())
    # Overlay sources commonly give the C body a friendly name while the
    # fallback keeps splat's auto-name.  A single opposite NON_MATCHING branch
    # in the same one-function TU is the selected function even though the two
    # labels differ.
    if definitions and not pragmas and candidate_symbol != target_symbol:
        opposite = [
            pragma
            for pragma in _all_pragmas(text)
            if pragma.non_matching_state is False
        ]
        if len(opposite) == 1:
            pragmas = tuple(opposite)
    nm_enabled = "NON_MATCHING" in defined

    active_definitions = [
        item
        for item in definitions
        if item.non_matching_state is None or item.non_matching_state == nm_enabled
    ]
    active_pragmas = [
        item
        for item in pragmas
        if item.non_matching_state is None or item.non_matching_state == nm_enabled
    ]

    if len(active_definitions) == 1 and not active_pragmas:
        if active_definitions[0].non_matching_state is True:
            return NON_MATCHING_C, "-DNON_MATCHING selects the guarded C body"
        return ORDINARY_C, "the selected symbol has one active ordinary C definition"
    if active_pragmas and not active_definitions:
        return GLOBAL_ASM, "the effective preprocessor branch selects GLOBAL_ASM"
    if active_definitions and active_pragmas:
        return UNKNOWN, "both C and GLOBAL_ASM appear active for the selected symbol"
    if definitions:
        return UNKNOWN, "the C definition is inactive under the discovered compiler defines"
    return UNKNOWN, "no active C definition or matching GLOBAL_ASM could be tied to the symbol"


def _relative(path: pathlib.Path, root: pathlib.Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def _artifact(path: pathlib.Path | None, root: pathlib.Path, kind: str) -> dict[str, object] | None:
    if path is None:
        return None
    record: dict[str, object] = {"path": _relative(path, root), "kind": kind}
    if path.is_file():
        record.update(sha256=sha256_file(path), size=path.stat().st_size)
    else:
        record.update(sha256=None, size=None)
    return record


def _find_source(root: pathlib.Path, symbols: Sequence[str]) -> pathlib.Path | None:
    matches: set[pathlib.Path] = set()
    for path in (root / "src").rglob("*.c"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for symbol in symbols:
            facts = source_facts(text, symbol)
            if facts.definitions or facts.pragmas:
                matches.add(path)
                break
    return next(iter(matches)) if len(matches) == 1 else None


def _discover_compile_command(
    root: pathlib.Path,
    source: pathlib.Path,
    *,
    non_matching: bool,
) -> tuple[str | None, list[str], str | None]:
    rel_source = _relative(source, root)
    build_dir = "build_non_matching" if non_matching else "build"
    target = f"{build_dir}/{rel_source}.o"
    command = [
        "gmake",
        "--no-print-directory",
        "-n",
        "-W",
        rel_source,
    ]
    if non_matching:
        command.append("NON_MATCHING=1")
    command.append(target)
    try:
        result = subprocess.run(
            command,
            cwd=root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=120,
            check=False,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        return None, [], str(error)
    if result.returncode != 0:
        return None, [], (result.stderr.strip() or "gmake dry-run failed")
    logical = result.stdout.replace("\\\n", " ").splitlines()
    candidates = [
        line.strip()
        for line in logical
        if rel_source in line
        and ("tools/asm-processor/build.py" in line or "tools/ido/cc" in line)
    ]
    if len(candidates) != 1:
        return None, [], f"expected one compile command, found {len(candidates)}"
    compile_command = candidates[0]
    try:
        tokens = shlex.split(compile_command)
    except ValueError as error:
        return compile_command, [], str(error)
    if "--" in tokens:
        tokens = tokens[tokens.index("--", tokens.index("--") + 1) + 1 :]
    flags: list[str] = []
    skip_next = False
    for token in tokens:
        if skip_next:
            skip_next = False
            continue
        if token == "-o":
            skip_next = True
            continue
        if token == "-c" or token == rel_source:
            continue
        flags.append(token)
    return compile_command, flags, None


def _object_symbols(path: pathlib.Path, objdump: pathlib.Path) -> tuple[set[str], str | None]:
    if not path.is_file():
        return set(), "object does not exist"
    result = subprocess.run(
        [str(objdump), "-t", str(path)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        return set(), result.stderr.strip() or "objdump symbol-table read failed"
    symbols: set[str] = set()
    for line in result.stdout.splitlines():
        fields = line.split()
        if len(fields) < 6:
            continue
        # GNU objdump's function row is ADDRESS BIND F SECTION SIZE NAME.
        # An undefined reference with the requested spelling must not tie the
        # candidate bytes to a C definition.
        if fields[-4] != "F" or fields[-3] == "*UND*":
            continue
        try:
            size = int(fields[-2], 16)
        except ValueError:
            continue
        if size:
            symbols.add(fields[-1])
    return symbols, None


def build_manifest(
    *,
    root: pathlib.Path,
    mode: str,
    source: pathlib.Path | None,
    symbol: str,
    candidate_symbol: str,
    candidate_build_dir: pathlib.Path,
    candidate_object: pathlib.Path | None,
    target_object: pathlib.Path | None,
    candidate_artifact: pathlib.Path,
    target_artifact: pathlib.Path,
    objdump: pathlib.Path,
) -> dict[str, object]:
    reasons: list[str] = []
    if source is None:
        source = _find_source(root, (candidate_symbol, symbol))
    source_text = None
    if source is None or not source.is_file():
        classification = UNKNOWN
        classification_reason = "source path could not be resolved uniquely"
        compile_command = None
        compiler_flags: list[str] = []
        compile_error = "source unavailable"
    else:
        source_text = source.read_text(encoding="utf-8", errors="replace")
        requested_nm = candidate_build_dir.name == "build_non_matching"
        compile_command, compiler_flags, compile_error = _discover_compile_command(
            root, source, non_matching=requested_nm
        )
        defines = {
            flag[2:].split("=", 1)[0]
            for flag in compiler_flags
            if flag.startswith("-D") and len(flag) > 2
        }
        # A failed dry-run must not silently turn the standard NON_MATCHING
        # tree into a fallback classification; its directory is part of the
        # public build contract and supplies the conservative define here.
        if requested_nm:
            defines.add("NON_MATCHING")
        classification, classification_reason = classify_source_selection(
            source_text,
            candidate_symbol=candidate_symbol,
            target_symbol=symbol,
            defines=defines,
        )

    if candidate_object is None and source is not None:
        rel_source = _relative(source, root)
        candidate_object = candidate_build_dir / f"{rel_source}.o"
        if not candidate_object.is_absolute():
            candidate_object = root / candidate_object

    candidate_symbols: set[str] = set()
    symbol_error = None
    if candidate_object is not None:
        candidate_symbols, symbol_error = _object_symbols(candidate_object, objdump)
    if candidate_symbol not in candidate_symbols:
        reasons.append("candidate object does not define the selected candidate symbol")
        if symbol_error:
            reasons.append(symbol_error)

    if classification not in {ORDINARY_C, NON_MATCHING_C}:
        reasons.append(classification_reason)
    if source is None:
        reasons.append("candidate symbol cannot be tied to one source path")
    elif candidate_object is not None and candidate_object.is_file():
        if candidate_object.stat().st_mtime_ns < source.stat().st_mtime_ns:
            reasons.append("candidate object predates its source path")
    else:
        reasons.append("candidate object is unavailable for hashing")

    # A guarded candidate is C evidence only when the actual Make expansion
    # confirms that the defining branch was enabled.
    if classification == NON_MATCHING_C:
        if compile_command is None or "-DNON_MATCHING" not in compiler_flags:
            reasons.append("NON_MATCHING C selection lacks a discovered -DNON_MATCHING compile")

    admissible = not reasons
    source_record = None
    if source is not None:
        source_record = {
            "path": _relative(source, root),
            "sha256": sha256_file(source) if source.is_file() else None,
        }
    manifest: dict[str, object] = {
        "schema": "mickey-wb-proof-provenance-v1",
        "mode": mode,
        "symbol": symbol,
        "candidate_symbol": candidate_symbol,
        "source": source_record,
        "selection": {
            "classification": classification,
            "reason": classification_reason,
        },
        "compiler": {
            "command": compile_command,
            "flags": compiler_flags,
            "discovery_error": compile_error,
        },
        "candidate_object": _artifact(candidate_object, root, "elf_object"),
        "target_object": _artifact(
            target_object if target_object is not None else target_artifact,
            root,
            "elf_object" if target_object is not None else "rom_objdump",
        ),
        "candidate_artifact": _artifact(candidate_artifact, root, "comparison_input"),
        "target_artifact": _artifact(target_artifact, root, "comparison_input"),
        "exact_claim_allowed": admissible,
        "exact_guard": "none" if admissible else "assert_exact_false",
        "verdict": "c_evidence" if admissible else "not_c_evidence",
        "reasons": reasons,
    }
    return manifest


def _parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=pathlib.Path, default=pathlib.Path.cwd())
    parser.add_argument("--mode", choices=("asm", "rom"), required=True)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--candidate-symbol", required=True)
    parser.add_argument("--source", type=pathlib.Path)
    parser.add_argument("--candidate-build-dir", type=pathlib.Path, required=True)
    parser.add_argument("--candidate-object", type=pathlib.Path)
    parser.add_argument("--target-object", type=pathlib.Path)
    parser.add_argument("--candidate-artifact", type=pathlib.Path, required=True)
    parser.add_argument("--target-artifact", type=pathlib.Path, required=True)
    parser.add_argument("--objdump", type=pathlib.Path, required=True)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    args = _parse_args(argv)
    root = args.root.resolve()

    def rooted(path: pathlib.Path | None) -> pathlib.Path | None:
        if path is None or path.is_absolute():
            return path
        return root / path

    manifest = build_manifest(
        root=root,
        mode=args.mode,
        source=rooted(args.source),
        symbol=args.symbol,
        candidate_symbol=args.candidate_symbol,
        candidate_build_dir=rooted(args.candidate_build_dir),
        candidate_object=rooted(args.candidate_object),
        target_object=rooted(args.target_object),
        candidate_artifact=rooted(args.candidate_artifact),
        target_artifact=rooted(args.target_artifact),
        objdump=rooted(args.objdump),
    )
    manifest_path = rooted(args.manifest)
    assert manifest_path is not None
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = manifest_path.with_suffix(manifest_path.suffix + ".tmp")
    temporary.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    os.replace(temporary, manifest_path)

    selection = manifest["selection"]
    assert isinstance(selection, dict)
    source = manifest["source"]
    source_path = source["path"] if isinstance(source, dict) else "unresolved"
    verdict = "C-EVIDENCE" if manifest["exact_claim_allowed"] else "NOT-C-EVIDENCE"
    print(
        "proof-provenance: "
        f"{verdict} {selection['classification']} source={source_path} "
        f"symbol={args.candidate_symbol} manifest={_relative(manifest_path, root)}",
        file=sys.stderr,
    )
    if not manifest["exact_claim_allowed"]:
        print(
            "proof-provenance: exact claim blocked; "
            + "; ".join(str(reason) for reason in manifest["reasons"]),
            file=sys.stderr,
        )
        return 3
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
