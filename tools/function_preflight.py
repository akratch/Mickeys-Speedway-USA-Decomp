#!/usr/bin/env python3
"""One fail-closed evidence preflight for a matching-decomp function.

The report contains geometry and relocation metadata, never instruction text
or ROM bytes.  It deliberately runs before a flag sweep so a worker starts
from the right symbol identity, translation unit, build mode, owned range,
call surface, and current full-TU score.

Usage:
    tools/function_preflight.py overlay16ApplyGradient
    tools/function_preflight.py func_overlay_016_F00001E0_1873678 --json
    tools/function_preflight.py overlay16ApplyGradient --no-build
    tools/function_preflight.py overlay16ApplyGradient --analysis-only --json
"""

from __future__ import annotations

import argparse
import copy
import dataclasses
import hashlib
import json
import os
import re
import shlex
import subprocess
import sys
import time
from pathlib import Path
from typing import Iterable


REPO = Path(__file__).resolve().parent.parent
TOOLS = REPO / "tools"
ROM = REPO / "baseroms" / "mickey.us.z64"
ATLAS = REPO / "config" / "overlays.us.json"
ALIASES = REPO / "overlay_undefined_syms.us.txt"
SYMBOLS = REPO / "symbol_addrs.us.txt"
TARGET_ELF = REPO / "build" / "mickey.us.elf"
VENV_PYTHON_TARGET = ".venv/bin/python"
WB_COMPARE = TOOLS / "wb_compare.sh"
TYPE_NAMES = {2: "R_MIPS_32", 4: "R_MIPS_26", 5: "R_MIPS_HI16", 6: "R_MIPS_LO16",
              10: "R_MIPS_PC16"}
PARTIAL_EVIDENCE_EXIT = 1
WB_SUMMARY_SCHEMA = "mickey-wb-summary-v1"
WB_RELOCATION_SURFACE_MODES = {
    "fallback_static": "fallback-static",
    "promoted_linked": "promoted-linked",
}
WB_SUMMARY_MAX_AGE_SECONDS = 15 * 60

sys.path.insert(0, str(TOOLS))
import overlay_tables as ot  # noqa: E402
import function_history as fh  # noqa: E402
import proof_provenance as pp  # noqa: E402
import postprocess_audit as pa  # noqa: E402
import reloc_identity as ri  # noqa: E402
import reloc_surface as rs  # noqa: E402
import finalize_plateau as fp  # noqa: E402
_LOADED_FILTER_PROOF = pp.sha256_file(Path(__file__))
_LOADED_FILTER_IMPLEMENTATIONS = {str(Path(module.__file__)): pp.sha256_file(Path(module.__file__))
                                  for module in (pp, ri, rs, ot)}


def _check_filter_implementations():
    if (pp.sha256_file(Path(__file__)) != _LOADED_FILTER_PROOF
            or any(pp.sha256_file(Path(path)) != digest
                   for path, digest in _LOADED_FILTER_IMPLEMENTATIONS.items())):
        raise PreflightError("loaded filter-proof implementation changed")


def _require_capture_snapshot(receipt, input_snapshot, linked_bytes):
    if (receipt["linked_sha256"] != input_snapshot["linked"]
            or hashlib.sha256(linked_bytes).hexdigest() != receipt["linked_sha256"]
            or receipt["inputs"]["target_inputs"] != input_snapshot["target_inputs"]):
        raise pp.MetadataProofError("previously parsed linked/runtime ownership changed before raw capture")


class PreflightError(RuntimeError):
    """The requested identity or evidence surface is not uniquely provable."""


class StaleEvidenceError(PreflightError):
    """An otherwise valid evidence artifact is missing or needs rebuilding."""


@dataclasses.dataclass(frozen=True)
class Resolution:
    requested_symbol: str
    target_symbol: str
    candidate_symbol: str
    source: Path
    translation_unit: str
    candidate_build_dir: str
    candidate_object: Path
    target_asm: Path | None
    selection: str
    resolution_mode: str = "fallback"
    identity_evidence: str = "GLOBAL_ASM fallback"
    expected_value: int | None = None
    expected_size: int | None = None


def _relative(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def _aliases(path: Path = ALIASES) -> tuple[dict[str, str], dict[str, list[str]]]:
    forward: dict[str, str] = {}
    reverse_sets: dict[str, set[str]] = {}
    if not path.is_file():
        raise PreflightError(f"missing generated alias surface: {_relative(path)}")
    text = path.read_text(encoding="utf-8", errors="replace")
    for generated, friendly in ri.parse_linker_aliases(text):
        if not re.fullmatch(
            r"func_overlay_\d{3}_F[0-9A-Fa-f]{7}_[0-9A-Fa-f]+", generated
        ):
            continue
        old = forward.setdefault(generated, friendly)
        if old != friendly:
            raise PreflightError(
                f"generated symbol {generated} has conflicting aliases {old} and {friendly}"
            )
        reverse_sets.setdefault(friendly, set()).add(generated)
    reverse = {name: sorted(generated) for name, generated in reverse_sets.items()}
    return forward, reverse


def _resolve_names(symbol: str, alias_path: Path = ALIASES) -> tuple[str, str]:
    forward, reverse = _aliases(alias_path)
    if symbol in forward:
        return symbol, forward[symbol]
    generated = reverse.get(symbol, [])
    if len(generated) > 1:
        raise PreflightError(
            f"friendly symbol {symbol} maps to {len(generated)} generated identities: "
            + ", ".join(sorted(generated))
        )
    if len(generated) == 1:
        return generated[0], symbol
    return symbol, symbol


def _guarded_resident_fallback(symbol: str, root: Path) -> str | None:
    """The splat name a descriptive resident candidate's guard falls back to.

    A resident function renamed only in C -- ``ProcessRelocationEntry`` over
    ``func_80031A30.s`` -- has no alias row (the alias surface is overlay-only)
    and no ``<symbol>.s``, so `resolve` found no target at all and reported the
    candidate as promoted. The guard itself is the pairing, exactly as
    `finalize_plateau.guarded_candidates` and tools/progress.py read it: one
    ``#ifdef NON_MATCHING`` definition of ``symbol`` whose one ``#else``
    fallback is a generated resident name. Returns that name only when exactly
    one source carries such a guard.
    """
    found: set[str] = set()
    for source in (root / "src").rglob("*.c"):
        text = source.read_text(encoding="utf-8", errors="replace")
        if symbol not in text or "NON_MATCHING" not in text:
            continue
        try:
            candidates = fp.guarded_candidates(text, symbol)
        except fp.PlateauError:
            continue
        for candidate in candidates:
            name = Path(candidate.fallback).name
            if fp.GENERATED_RESIDENT_FALLBACK_RE.fullmatch(name):
                found.add(name[:-2])
    return found.pop() if len(found) == 1 else None


def _unique(paths: list[Path], description: str) -> Path:
    paths = sorted({path.resolve() for path in paths})
    if len(paths) != 1:
        rendered = ", ".join(_relative(path) for path in paths) or "none"
        raise PreflightError(
            f"expected one {description}, found {len(paths)} ({rendered})"
        )
    return paths[0]


def _fallback_source_for(
    target_symbol: str,
    candidate_symbol: str,
    target_asm: Path,
    root: Path = REPO,
) -> Path:
    root = root.resolve()
    relative_parent = target_asm.parent.relative_to(root / "asm" / "nonmatchings")
    direct = root / "src" / relative_parent.with_suffix(".c")
    candidates: list[Path] = []
    if direct.is_file():
        candidates.append(direct)
    if not candidates:
        for source in (root / "src").rglob("*.c"):
            text = source.read_text(encoding="utf-8", errors="replace")
            candidate_facts = pp.source_facts(text, candidate_symbol)
            target_facts = pp.source_facts(text, target_symbol)
            if candidate_facts.definitions and (
                target_facts.pragmas or target_symbol == candidate_symbol
            ):
                candidates.append(source)
    return _unique(candidates, f"canonical source for {candidate_symbol}")


def _definition_source(candidate_symbol: str, root: Path) -> Path:
    candidates: list[Path] = []
    for source in (root / "src").rglob("*.c"):
        text = source.read_text(encoding="utf-8", errors="replace")
        if pp.source_facts(text, candidate_symbol).definitions:
            candidates.append(source)
    return _unique(candidates, f"C definition source for {candidate_symbol}")


def _hex_field(row: dict[str, object], field: str, description: str) -> int:
    value = row.get(field)
    if not isinstance(value, str) or not re.fullmatch(r"0x[0-9A-Fa-f]+", value):
        raise PreflightError(f"{description} has invalid {field}: {value!r}")
    return int(value, 16)


def _overlay_promotion_evidence(
    target_symbol: str,
    candidate_symbol: str,
    source: Path,
    root: Path,
    atlas_path: Path,
) -> tuple[int, int, str]:
    generated = rs.GEN_NAME_RE.match(target_symbol)
    if not generated:
        raise PreflightError(
            f"post-promotion overlay identity requires a generated symbol, got {target_symbol}"
        )
    overlay = int(generated.group(1))
    offset = int(generated.group(2), 16)
    if not atlas_path.is_file():
        raise PreflightError(f"missing overlay atlas: {_relative(atlas_path)}")
    try:
        atlas = json.loads(atlas_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError) as error:
        raise PreflightError(f"cannot read overlay atlas: {error}") from error
    modules = [row for row in atlas.get("modules", []) if row.get("overlay") == overlay]
    if len(modules) != 1:
        raise PreflightError(f"atlas has {len(modules)} rows for overlay {overlay}")

    source_key = source.relative_to(root / "src").with_suffix("").as_posix()
    evidence: list[tuple[int, int, str, str]] = []
    module = modules[0]
    for row in module.get("text_ownership", []):
        if (
            row.get("type") == "c"
            and row.get("matched") is True
            and row.get("nonmatching") is False
            and row.get("source") == source_key
            and _hex_field(row, "offset", "text ownership row") == offset
        ):
            start = offset
            end = _hex_field(row, "end_offset", "text ownership row")
            size = _hex_field(row, "size", "text ownership row")
            evidence.append((start, end, source_key, "exact text_ownership"))
            if end - start != size:
                raise PreflightError("exact text ownership size/end fields disagree")
    for row in module.get("mixed_tu_exact_c_ranges", []):
        if (
            row.get("label") == candidate_symbol
            and row.get("source") == source_key
            and _hex_field(row, "offset", "mixed-TU exact row") == offset
        ):
            start = offset
            end = _hex_field(row, "end_offset", "mixed-TU exact row")
            size = _hex_field(row, "size", "mixed-TU exact row")
            if end - start != size:
                raise PreflightError("mixed-TU exact size/end fields disagree")
            evidence.append((start, end, source_key, "mixed_tu_exact_c_ranges"))

    geometries = {(start, end, owner) for start, end, owner, _kind in evidence}
    if not geometries:
        inner = _fully_matched_inner_geometry(
            module, overlay, offset, candidate_symbol, target_symbol, source_key, root
        )
        if inner is not None:
            return inner
    if len(geometries) != 1:
        rendered = ", ".join(
            f"{owner}:+0x{start:X}..+0x{end:X}"
            for start, end, owner in sorted(geometries)
        ) or "none"
        raise PreflightError(
            f"expected one tracked exact atlas range for {candidate_symbol}, "
            f"found {len(geometries)} ({rendered})"
        )
    start, end, _owner = next(iter(geometries))
    kinds = "+".join(sorted({kind for *_rest, kind in evidence}))
    if kinds == "exact text_ownership":
        export_offsets = sorted(
            {
                _hex_field(row, "offset", "overlay export row")
                for row in module.get("exports", [])
                if isinstance(row, dict) and "offset" in row
            }
        )
        if offset in export_offsets:
            following = [value for value in export_offsets if offset < value <= end]
            if following:
                end = following[0]
                kinds += "+export boundary"
        # A text_ownership row bounds the translation unit, not the function.
        # The two coincide only when the TU holds one function, or when a
        # later export trims the row; the first function of a multi-function
        # TU that has just become fully matched satisfies neither, and the
        # row then names the whole TU as the symbol's geometry. Authenticate
        # the symbol against the linked object in that case -- the same proof
        # an inner function already gets -- and keep the container evidence as
        # the fallback when that proof is unavailable.
        try:
            inner = _fully_matched_inner_geometry(
                module, overlay, offset, candidate_symbol, target_symbol,
                source_key, root,
            )
        except PreflightError:
            inner = None
        if inner is not None and inner[1] != end - start:
            return inner
    return rs.SYNTHETIC_VMA + start, end - start, f"overlay atlas {kinds}"


def _fully_matched_inner_geometry(
    module: dict[str, object], overlay: int, offset: int, candidate: str,
    target: str, source_key: str, root: Path,
) -> tuple[int, int, str] | None:
    """Authenticate an inner function after its last mixed-TU guard retires.

    No boundary is inferred from a TU extent or the next exported entrypoint.
    A linked function must fill its interval to the next function/owner end,
    and the entire exact-C container must already equal the baserom.
    """
    owners = [
        row for row in module.get("text_ownership", [])
        if _hex_field(row, "offset", "text ownership row") <= offset
        < _hex_field(row, "end_offset", "text ownership row")
    ]
    if len(owners) != 1:
        raise PreflightError("inner function requires one unambiguous atlas owner")
    owner = owners[0]
    if not (owner.get("type") == "c" and owner.get("matched") is True
            and owner.get("nonmatching") is False and owner.get("source") == source_key):
        return None
    start = _hex_field(owner, "offset", "exact C owner")
    end = _hex_field(owner, "end_offset", "exact C owner")
    if end - start != _hex_field(owner, "size", "exact C owner"):
        raise PreflightError("exact C owner size/end fields disagree")
    elf_path = root / "build/mickey.us.elf"
    rom_path = root / "baseroms/mickey.us.z64"
    if not elf_path.is_file() or not rom_path.is_file():
        raise PreflightError("inner function requires linked ELF and baserom evidence")
    elf = rs.Elf(elf_path)
    section_name = f".overlay_{overlay:03d}"
    section_index, section = elf.section(section_name)
    if (section is None or section[1] != 1 or not section[2] & 4
            or section[3] != rs.SYNTHETIC_VMA):
        raise PreflightError("inner function has no correctly based overlay section")
    symbols = elf.symbols()
    named = [row for row in symbols if row[0] in {candidate, target}
             and row[4] != rs.SHN_UNDEF]
    if (not any(row[0] == candidate and row[2] > 0 for row in named)
            or any(row[4] != section_index or row[3] & 0xF != rs.STT_FUNC
                   or (row[2] <= 0 and row[0] != target) for row in named)):
        raise PreflightError("inner function lacks unambiguous defined STT_FUNC evidence")
    # Linker-script aliases retain STT_FUNC/address but commonly have size 0.
    # They corroborate only the start; the actual C definition must own size.
    geometries = {(row[1], row[2]) for row in named if row[2] > 0}
    if len(geometries) != 1:
        raise PreflightError("inner function aliases have conflicting geometry")
    value, size = next(iter(geometries))
    if any(row[1] != value for row in named):
        raise PreflightError("inner function aliases have conflicting starts")
    limit = value + size
    owner_limit = rs.SYNTHETIC_VMA + end
    if value != rs.SYNTHETIC_VMA + offset or size % 4 or offset % 4 or limit > owner_limit:
        raise PreflightError("inner function geometry disagrees with its generated identity/owner")
    following = [owner_limit]
    for name, other_value, other_size, info, shndx in symbols:
        if shndx != section_index or not name or (info & 0xF) not in {rs.STT_FUNC, rs.STT_OBJECT}:
            continue
        if name in {candidate, target}:
            continue
        if other_value < limit and value < other_value + max(other_size, 1):
            raise PreflightError("inner function overlaps another function/data symbol")
        if (value < other_value <= owner_limit and other_size > 0
                and (info & 0xF) == rs.STT_FUNC):
            following.append(other_value)
    if limit != min(following):
        raise PreflightError("inner function boundary leaves unauthenticated padding/gap")
    rom_start = _hex_field(module["rom"], "start", "overlay ROM range")
    if int(target.rsplit("_", 1)[1], 16) != rom_start + offset:
        raise PreflightError("inner function generated ROM identity disagrees with atlas")
    rom = rom_path.read_bytes()
    section_bytes = elf.section_bytes(section_name)
    if end > len(section_bytes) or rom_start + end > len(rom):
        raise PreflightError("exact C owner exceeds linked/ROM extent")
    if section_bytes[start:end] != rom[rom_start + start:rom_start + end]:
        raise PreflightError("fully matched C container is not linked-ROM exact")
    return value, size, "overlay atlas exact C container+linked STT_FUNC+whole-container ROM"


def _resident_promotion_evidence(
    symbol: str,
    symbol_path: Path,
) -> tuple[int, int, str]:
    if not symbol_path.is_file():
        raise PreflightError(f"missing tracked symbol table: {_relative(symbol_path)}")
    pattern = re.compile(
        rf"^\s*{re.escape(symbol)}\s*=\s*(0x[0-9A-Fa-f]+)\s*;\s*//(?P<comment>.*)$"
    )
    rows = []
    for line in symbol_path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = pattern.match(line)
        if match:
            rows.append((int(match.group(1), 16), match.group("comment")))
    if len(rows) != 1:
        raise PreflightError(
            f"expected one tracked symbol row for promoted {symbol}, found {len(rows)}"
        )
    value, comment = rows[0]
    sizes = re.findall(r"\bsize:0x([0-9A-Fa-f]+)\b", comment)
    if "type:func" not in comment or len(sizes) != 1 or not re.search(
        r"\bmatched\s+C\b", comment
    ):
        raise PreflightError(
            f"tracked symbol row for {symbol} is not one unambiguous matched-C function"
        )
    return value, int(sizes[0], 16), "symbol_addrs matched-C function row"


def _post_promotion_resolution(
    symbol: str,
    target_symbol: str,
    candidate_symbol: str,
    root: Path,
    atlas_path: Path,
    symbol_path: Path,
) -> Resolution:
    source = _definition_source(candidate_symbol, root)
    text = source.read_text(encoding="utf-8", errors="replace")
    facts = pp.source_facts(text, candidate_symbol)
    target_facts = pp.source_facts(text, target_symbol)
    if (
        len(facts.definitions) != 1
        or facts.definitions[0].non_matching_state is not None
        or facts.pragmas
        or target_facts.pragmas
    ):
        raise PreflightError(
            f"no fallback assembly exists for {target_symbol}, and {_relative(source)} "
            "is not one unconditional promoted C definition with no matching GLOBAL_ASM"
        )
    ordinary_reason = "the selected symbol has one unconditional ordinary C definition"

    generated = rs.GEN_NAME_RE.match(target_symbol)
    if generated:
        expected_value, expected_size, evidence = _overlay_promotion_evidence(
            target_symbol, candidate_symbol, source, root, atlas_path
        )
    else:
        if target_symbol != candidate_symbol:
            raise PreflightError(
                "resident post-promotion identity cannot use different target/candidate names"
            )
        expected_value, expected_size, evidence = _resident_promotion_evidence(
            target_symbol, symbol_path
        )

    rel_source = source.relative_to(root).as_posix()
    return Resolution(
        requested_symbol=symbol,
        target_symbol=target_symbol,
        candidate_symbol=candidate_symbol,
        source=source,
        translation_unit=rel_source.removeprefix("src/").removesuffix(".c"),
        candidate_build_dir="build",
        candidate_object=root / "build" / f"{rel_source}.o",
        target_asm=None,
        selection=f"{ordinary_reason}; promoted identity proved by {evidence}",
        resolution_mode="post_promotion",
        identity_evidence=evidence,
        expected_value=expected_value,
        expected_size=expected_size,
    )


def resolve(
    symbol: str,
    root: Path = REPO,
    alias_path: Path = ALIASES,
    atlas_path: Path = ATLAS,
    symbol_path: Path = SYMBOLS,
) -> Resolution:
    root = root.resolve()
    target_symbol, candidate_symbol = _resolve_names(symbol, alias_path)
    asm_matches = list((root / "asm" / "nonmatchings").rglob(f"{target_symbol}.s"))
    if not asm_matches and target_symbol == candidate_symbol:
        generated = _guarded_resident_fallback(symbol, root)
        if generated is not None:
            target_symbol = generated
            asm_matches = list(
                (root / "asm" / "nonmatchings").rglob(f"{target_symbol}.s")
            )
    if not asm_matches:
        return _post_promotion_resolution(
            symbol,
            target_symbol,
            candidate_symbol,
            root,
            atlas_path,
            symbol_path,
        )
    target_asm = _unique(asm_matches, f"fallback assembly for {target_symbol}")
    source = _fallback_source_for(target_symbol, candidate_symbol, target_asm, root)
    text = source.read_text(encoding="utf-8", errors="replace")

    ordinary, ordinary_reason = pp.classify_source_selection(
        text,
        candidate_symbol=candidate_symbol,
        target_symbol=target_symbol,
        defines=(),
    )
    nonmatching, nonmatching_reason = pp.classify_source_selection(
        text,
        candidate_symbol=candidate_symbol,
        target_symbol=target_symbol,
        defines=("NON_MATCHING",),
    )
    if ordinary == pp.ORDINARY_C:
        build_dir = "build"
        selection = ordinary_reason
    elif nonmatching == pp.NON_MATCHING_C:
        build_dir = "build_non_matching"
        selection = nonmatching_reason
    else:
        raise PreflightError(
            f"{_relative(source)} does not select one C definition for "
            f"{candidate_symbol}; ordinary={ordinary}, NON_MATCHING={nonmatching}"
        )

    rel_source = source.relative_to(root).as_posix()
    candidate_object = root / build_dir / f"{rel_source}.o"
    return Resolution(
        requested_symbol=symbol,
        target_symbol=target_symbol,
        candidate_symbol=candidate_symbol,
        source=source,
        translation_unit=rel_source.removeprefix("src/").removesuffix(".c"),
        candidate_build_dir=build_dir,
        candidate_object=candidate_object,
        target_asm=target_asm,
        selection=selection,
    )


def _run(argv: list[str], *, capture: bool = False) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        argv,
        cwd=REPO,
        check=False,
        text=True,
        stdout=subprocess.PIPE if capture else None,
        stderr=subprocess.PIPE if capture else None,
    )


def _build_logic_inputs(root: Path = REPO) -> tuple[Path, ...]:
    """Return build recipes whose edits Make cannot age against an object.

    Normal prerequisites (the source, every header, the splat inputs and the
    generated linker inputs) are covered by ``gmake -q`` below.  GNU Make does
    not rebuild a target merely because the recipe that produced it changed,
    so the root Makefile, its included policy files, and its checked-in
    normalization fragments need an explicit timestamp check.
    """

    inputs = [root / "Makefile"]
    inputs.extend(sorted((root / "mk").glob("**/*.mk")))
    inputs.extend(sorted((root / "config" / "normalizations").glob("*.mk")))
    return tuple(path for path in inputs if path.is_file())


def _newer_inputs(artifact: Path, inputs: Iterable[Path]) -> tuple[Path, ...]:
    if not artifact.is_file():
        return ()
    artifact_mtime = artifact.stat().st_mtime_ns
    return tuple(
        path for path in inputs if path.is_file() and path.stat().st_mtime_ns > artifact_mtime
    )


def _make_freshness_command(target: Path, *, non_matching: bool) -> list[str]:
    command = ["gmake", "--no-print-directory", "-q"]
    if non_matching:
        command.append("NON_MATCHING=1")
    command.append(_relative(target))
    return command


def _require_fresh_target(
    target: Path,
    *,
    label: str,
    non_matching: bool,
    build_logic_inputs: Iterable[Path],
) -> None:
    if not target.is_file():
        raise StaleEvidenceError(f"missing {label}: {_relative(target)}")

    newer = _newer_inputs(target, build_logic_inputs)
    if newer:
        rendered = ", ".join(_relative(path) for path in newer[:3])
        suffix = "" if len(newer) <= 3 else f" (+{len(newer) - 3} more)"
        raise StaleEvidenceError(
            f"stale {label} {_relative(target)}: newer build recipe/config input "
            f"{rendered}{suffix}"
        )

    query = _run(_make_freshness_command(target, non_matching=non_matching), capture=True)
    if query.returncode == 0:
        return
    if query.returncode == 1:
        mode = " NON_MATCHING=1" if non_matching else ""
        raise StaleEvidenceError(
            f"stale {label} {_relative(target)} according to the Make dependency graph; "
            f"run `nice -n 10 gmake -j{_build_jobs()}{mode} {_relative(target)}`"
        )
    detail = (query.stderr or query.stdout or "gmake -q failed").strip().splitlines()
    raise PreflightError(
        f"could not prove {label} freshness for {_relative(target)}: "
        f"{detail[-1] if detail else f'gmake -q exited {query.returncode}'}"
    )


def require_fresh_evidence(resolution: Resolution) -> None:
    """Fail before comparison unless both linked and full-TU artifacts are current."""

    build_logic = _build_logic_inputs()
    _require_fresh_target(
        TARGET_ELF,
        label="canonical linked ELF",
        non_matching=False,
        build_logic_inputs=build_logic,
    )
    _require_fresh_target(
        resolution.candidate_object,
        label="candidate object",
        non_matching=resolution.candidate_build_dir == "build_non_matching",
        build_logic_inputs=build_logic,
    )


def _candidate_split_stamp(resolution: Resolution) -> Path:
    return REPO / resolution.candidate_build_dir / ".splat-stamp"


def _wb_split_receipts(resolution: Resolution) -> tuple[tuple[Path, bool], ...]:
    receipts = {
        (REPO / "build" / ".splat-stamp", False),
        (
            _candidate_split_stamp(resolution),
            resolution.candidate_build_dir == "build_non_matching",
        ),
    }
    return tuple(sorted(receipts, key=lambda row: row[0].as_posix()))


def _require_fresh_wb_evidence(resolution: Resolution) -> None:
    """Prove the two inputs an assembly-mode workbench comparison consumes.

    The extracted fallback object is the target and the selected full-TU C
    object is the candidate.  The canonical linked ELF is not an input to that
    comparison.  Requiring it here made every guarded-body edit relink the ROM
    even though the ordinary branch still selected the unchanged fallback.
    """

    _require_fresh_wb_boundary(resolution)
    _require_fresh_target(
        resolution.candidate_object,
        label="candidate object",
        non_matching=resolution.candidate_build_dir == "build_non_matching",
        build_logic_inputs=_build_logic_inputs(),
    )


def _require_fresh_wb_boundary(resolution: Resolution) -> None:
    """Prove the extracted fallback came from the current split graph."""

    if resolution.target_asm is None:
        raise PreflightError(
            f"{resolution.candidate_symbol} has no extracted fallback target"
        )
    build_logic = _build_logic_inputs()
    current_receipts: list[Path] = []
    for split_stamp, non_matching in _wb_split_receipts(resolution):
        try:
            _require_fresh_target(
                split_stamp,
                label="split receipt",
                non_matching=non_matching,
                build_logic_inputs=build_logic,
            )
        except StaleEvidenceError:
            continue
        current_receipts.append(split_stamp)
    if not current_receipts:
        raise StaleEvidenceError("no current split receipt proves the extracted fallback")
    if not resolution.target_asm.is_file():
        raise StaleEvidenceError(
            f"missing extracted fallback: {_relative(resolution.target_asm)}"
        )
    asm_mtime = resolution.target_asm.stat().st_mtime_ns
    if all(asm_mtime > stamp.stat().st_mtime_ns for stamp in current_receipts):
        # A generated fallback newer than the completed split is not ordinary
        # staleness: it may be a deliberate local edit. Never overwrite it as
        # an automatic refresh.
        receipts = ", ".join(_relative(path) for path in current_receipts)
        raise PreflightError(
            f"extracted fallback {_relative(resolution.target_asm)} is newer than "
            f"every current split receipt ({receipts})"
        )


def _build_jobs() -> int:
    """Use an explicit workstation override, otherwise ADR 0004's CPU count."""

    value = os.environ.get("MICKEY_BUILD_JOBS")
    if value is None:
        return os.cpu_count() or 1
    if not re.fullmatch(r"[0-9]+", value):
        raise PreflightError("MICKEY_BUILD_JOBS must be a positive decimal integer")
    try:
        jobs = int(value)
    except ValueError as error:
        raise PreflightError("MICKEY_BUILD_JOBS must be a positive decimal integer") from error
    if jobs <= 0:
        raise PreflightError("MICKEY_BUILD_JOBS must be a positive decimal integer")
    return jobs


def _build_target(target: Path, *, non_matching: bool, label: str) -> None:
    """Run the Makefile's required split phase before one evidence target."""

    command = ["nice", "-n", "10", "gmake", f"-j{_build_jobs()}"]
    if non_matching:
        command.append("NON_MATCHING=1")
    build_dir = "build_non_matching" if non_matching else "build"
    # GNU Make does not consider changed recipe text when deciding whether a
    # target is current.  Detect that surface before phase one and force the
    # phase-two dependency graph only when a checked-in recipe/policy file is
    # newer than the artifact.  For the linked ELF this rebuilds every object;
    # for a NON_MATCHING candidate it rebuilds that complete translation unit.
    force = bool(_newer_inputs(target, _build_logic_inputs()))
    split = _run([*command, f"{build_dir}/.splat-stamp"], capture=True)
    if split.returncode:
        raise PreflightError(f"{label} split phase failed with exit {split.returncode}")
    target_command = [*command]
    if force:
        # Lanes intentionally share the primary checkout's virtualenv through
        # a symlink.  `--always-make` must rebuild the evidence graph after a
        # recipe edit, but it must not rerun the venv bootstrap recipe against
        # that symlink (GNU Make would try to replace its parent directory).
        # `--assume-old` exempts only this immutable host-tool prerequisite;
        # C/asm/data objects, the link, and all evidence-producing recipes are
        # still forced.
        target_command.extend(
            ["--always-make", f"--assume-old={VENV_PYTHON_TARGET}"]
        )
    result = _run([*target_command, _relative(target)], capture=True)
    if result.returncode:
        raise PreflightError(f"{label} build failed with exit {result.returncode}")


def _build(resolution: Resolution) -> None:
    """Refresh complete preflight evidence, but only when it is stale."""

    candidate_is_nonmatching = resolution.candidate_build_dir == "build_non_matching"
    build_logic = _build_logic_inputs()
    try:
        _require_fresh_target(
            resolution.candidate_object,
            label="candidate object",
            non_matching=candidate_is_nonmatching,
            build_logic_inputs=build_logic,
        )
        candidate_stale = False
    except StaleEvidenceError:
        candidate_stale = True
    try:
        _require_fresh_target(
            TARGET_ELF,
            label="canonical linked ELF",
            non_matching=False,
            build_logic_inputs=build_logic,
        )
        canonical_stale = False
    except StaleEvidenceError:
        canonical_stale = True

    if candidate_is_nonmatching and candidate_stale:
        # Both build trees consume the same extracted asm/.  A NON_MATCHING
        # split therefore has to happen first: running it after the canonical
        # link would rewrite shared assembly and immediately stale that ELF.
        _build_target(
            resolution.candidate_object,
            non_matching=True,
            label="candidate",
        )
    if canonical_stale or candidate_stale:
        # The canonical link depends on every ordinary-tree C object, including
        # an ordinary candidate. Building it last both supplies that object and
        # leaves canonical evidence newer than a NON_MATCHING split.
        _build_target(TARGET_ELF, non_matching=False, label="canonical")


def _build_wb_evidence(resolution: Resolution) -> None:
    """Refresh only the artifacts consumed by an assembly-mode comparison."""

    try:
        _require_fresh_wb_boundary(resolution)
        boundary_stale = False
    except StaleEvidenceError:
        boundary_stale = True
    candidate_nonmatching = resolution.candidate_build_dir == "build_non_matching"
    try:
        _require_fresh_target(
            resolution.candidate_object,
            label="candidate object",
            non_matching=candidate_nonmatching,
            build_logic_inputs=_build_logic_inputs(),
        )
        candidate_stale = False
    except StaleEvidenceError:
        candidate_stale = True

    if boundary_stale:
        _build_target(
            _candidate_split_stamp(resolution),
            non_matching=candidate_nonmatching,
            label="target boundary",
        )
    if boundary_stale or candidate_stale:
        _build_target(
            resolution.candidate_object,
            non_matching=candidate_nonmatching,
            label="candidate",
        )


def _symbol_geometry(elf: rs.Elf, names: tuple[str, ...]) -> tuple[str, int, int, str]:
    found: list[tuple[str, int, int, str]] = []
    for name in dict.fromkeys(names):
        for symbol, value, size, _info, shndx in elf.symbols():
            if symbol != name or shndx == rs.SHN_UNDEF or size == 0:
                continue
            section = elf.names[shndx] if shndx < len(elf.names) else ""
            found.append((name, value, size, section))
    geometries: dict[tuple[int, int, str], list[str]] = {}
    for name, value, size, section in found:
        geometries.setdefault((value, size, section), []).append(name)
    if len(geometries) != 1:
        summary = ", ".join(
            f"{'/'.join(symbols)}@{value:#x}+{size:#x}"
            for (value, size, _section), symbols in geometries.items()
        )
        raise PreflightError(
            f"linked geometry is ambiguous for {names}: found {len(geometries)} "
            f"nonzero definitions ({summary or 'none'})"
        )
    (value, size, section), symbols = next(iter(geometries.items()))
    selected = next((name for name in names if name in symbols), symbols[0])
    return selected, value, size, section


def _optional_size_annotation(
    symbol: str,
    proved_size: int,
    symbol_path: Path = SYMBOLS,
) -> str:
    """Validate a size annotation when present without making it mandatory.

    A current, uniquely owned preflight boundary is stronger than the optional
    comment field in ``symbol_addrs.us.txt``.  The comment remains an
    independent contradiction check: duplicates, malformed values, or a size
    disagreement fail rather than being silently ignored.
    """

    if not symbol_path.is_file():
        raise PreflightError(f"missing tracked symbol table: {_relative(symbol_path)}")
    pattern = re.compile(
        rf"^\s*{re.escape(symbol)}\s*=\s*0x[0-9A-Fa-f]+\s*;(?P<comment>.*)$"
    )
    rows = [
        match.group("comment")
        for line in symbol_path.read_text(
            encoding="utf-8", errors="replace"
        ).splitlines()
        if (match := pattern.match(line))
    ]
    if len(rows) > 1:
        raise PreflightError(
            f"tracked symbol table has {len(rows)} rows for {symbol}; boundary is ambiguous"
        )
    if not rows:
        return "preflight-owned-boundary"
    comment = rows[0]
    sizes = re.findall(r"\bsize:0x([0-9A-Fa-f]+)\b", comment)
    if "size:" in comment and len(sizes) != 1:
        raise PreflightError(f"tracked size annotation for {symbol} is malformed")
    if not sizes:
        return "preflight-owned-boundary"
    annotated = int(sizes[0], 16)
    if annotated != proved_size:
        raise PreflightError(
            f"tracked size annotation for {symbol} disagrees with preflight boundary: "
            f"annotation={annotated:#x}, preflight={proved_size:#x}"
        )
    return "symbol-size-annotation+preflight-owned-boundary"


def _linked_boundary(resolution: Resolution) -> dict[str, object]:
    """Return one ownership-checked linked target range for ROM comparison."""

    for path, label in (
        (TARGET_ELF, "canonical linked ELF"),
        (ROM, "baserom"),
        (ATLAS, "overlay atlas"),
    ):
        if not path.is_file():
            raise PreflightError(f"missing {label}: {_relative(path)}")
    target_elf = rs.Elf(TARGET_ELF)
    linked_name, value, size, section = _symbol_geometry(
        target_elf, (resolution.candidate_symbol, resolution.target_symbol)
    )
    _require_tracked_geometry(resolution, value, size)
    try:
        atlas = json.loads(ATLAS.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError) as error:
        raise PreflightError(f"cannot read overlay atlas: {error}") from error
    rom = ROM.read_bytes()
    context, _records = _target_context(resolution, value, size, atlas, rom)
    if context["kind"] == "resident":
        context.update(_resident_boundary(target_elf, value, size, section))
    annotation = _optional_size_annotation(resolution.requested_symbol, size)
    return {
        "linked_symbol": linked_name,
        "value": value,
        "size": size,
        "section": section,
        "evidence": annotation,
        "context": context,
    }


def _require_fresh_linked_boundary() -> None:
    _require_fresh_target(
        TARGET_ELF,
        label="canonical linked ELF",
        non_matching=False,
        build_logic_inputs=_build_logic_inputs(),
    )


def _build_linked_boundary() -> None:
    try:
        _require_fresh_linked_boundary()
        return
    except StaleEvidenceError:
        _build_target(TARGET_ELF, non_matching=False, label="canonical")


def _source_signature(source: Path, symbol: str) -> str:
    original = source.read_text(encoding="utf-8", errors="replace")
    facts = pp.source_facts(original, symbol)
    if len(facts.definitions) != 1:
        raise PreflightError(
            f"expected one C definition signature for {symbol}, found "
            f"{len(facts.definitions)}"
        )
    text = pp._mask_c(original)
    starts = [0]
    starts.extend(match.end() for match in re.finditer("\n", text))
    definition_start = starts[facts.definitions[0].line - 1]
    match = re.search(rf"\b{re.escape(symbol)}\s*\(", text[definition_start:])
    if not match:
        raise PreflightError(f"cannot find C definition signature for {symbol}")
    symbol_start = definition_start + match.start()
    symbol_end = definition_start + match.end()
    opening = text.find("(", symbol_start, symbol_end)
    closing = pp._matching_paren(text, opening)
    if closing is None:
        raise PreflightError(f"unbalanced candidate declaration for {symbol}")
    brace = closing + 1
    while brace < len(text) and text[brace].isspace():
        brace += 1
    if brace >= len(text) or text[brace] != "{":
        raise PreflightError(f"{symbol} occurrence is not a function definition")
    line_start = text.rfind("\n", 0, symbol_start) + 1
    signature = " ".join(text[line_start:brace].split())
    if not signature:
        raise PreflightError(f"empty candidate declaration for {symbol}")
    return signature


def _identity_text(identity: tuple[int, int] | None) -> str:
    try:
        return ri.format_identity(identity)
    except ri.RelocationIdentityError as error:
        raise PreflightError(str(error)) from error


def _relocation_rows(records: list[rs.SurfaceRecord]) -> list[dict[str, object]]:
    return [
        {
            "offset": f"+0x{record.offset:X}",
            "type": TYPE_NAMES.get(record.rtype, str(record.rtype)),
            "identity": _identity_text(record.identity),
            "addend": record.link_addend,
        }
        for record in records
    ]


def _relocation_evidence(
    resolution: Resolution,
    context: dict[str, object],
    runtime_records: list[rs.SurfaceRecord],
    target_elf: rs.Elf,
    linked_name: str,
    target_value: int,
    target_size: int,
    target_section: str,
) -> dict[str, list[dict[str, object]]]:
    """Keep overlay runtime records distinct from resident static tuples."""
    if context["kind"] == "overlay":
        return {
            "target_static_relocations": [],
            "runtime_overlay_records": _relocation_rows(runtime_records),
            "resident_runtime_records": [],
        }

    static_records = rs._resident_target_records(
        resolution.candidate_object,
        resolution.translation_unit,
        target_elf,
        linked_name,
        target_value,
        target_size,
        target_section,
        ALIASES,
        runtime_records,
    )
    return {
        "target_static_relocations": _relocation_rows(static_records),
        "runtime_overlay_records": [],
        "resident_runtime_records": _relocation_rows(runtime_records),
    }


def _target_context(
    resolution: Resolution,
    target_value: int,
    target_size: int,
    atlas: dict[str, object],
    rom: bytes,
) -> tuple[dict[str, object], list[rs.SurfaceRecord]]:
    generated = rs.GEN_NAME_RE.match(resolution.target_symbol)
    if generated:
        overlay = int(generated.group(1))
        offset = int(generated.group(2), 16)
        if target_value != rs.SYNTHETIC_VMA + offset:
            raise PreflightError(
                f"linked target value {target_value:#x} disagrees with generated offset {offset:#x}"
            )
        modules = [row for row in atlas["modules"] if row["overlay"] == overlay]
        if len(modules) != 1:
            raise PreflightError(f"atlas has {len(modules)} rows for overlay {overlay}")
        module = modules[0]
        owners = [
            row
            for row in module["text_ownership"]
            if int(row["offset"], 16) <= offset
            and offset + target_size <= int(row["end_offset"], 16)
        ]
        if len(owners) != 1:
            raise PreflightError(
                f"target range overlay:{overlay}:+0x{offset:X}..+0x{offset + target_size:X} "
                f"has {len(owners)} atlas owners"
            )
        owner = owners[0]
        owner_end = int(owner["end_offset"], 16)
        following = [
            row for row in module["text_ownership"] if int(row["offset"], 16) >= offset + target_size
        ]
        following.sort(key=lambda row: int(row["offset"], 16))
        next_row = following[0] if following else None
        module_runtime = ot.build_modules(ot.read_headers(rom))[overlay - 1]
        records = rs._target_runtime_records(
            rom,
            {"kind": "overlay", "overlay": overlay, "module": module_runtime},
            offset,
            target_size,
        )
        exports = [
            row for row in module["exports"] if int(row["offset"], 16) == offset
        ]
        context = {
            "kind": "overlay",
            "overlay": overlay,
            "offset": offset,
            "rom_start": module_runtime["rom_start"] + offset,
            "rom_end": module_runtime["rom_start"] + offset + target_size,
            "owner": owner,
            "owner_end": owner_end,
            "next_owner": next_row,
            "exports": exports,
        }
        return context, records

    if not (ot.RESIDENT_VRAM_BASE <= target_value < 0x90000000):
        raise PreflightError(f"resident target value is out of range: {target_value:#x}")
    offset = target_value - ot.RESIDENT_VRAM_BASE
    records = rs._target_runtime_records(
        rom, {"kind": "resident"}, offset, target_size
    )
    return {
        "kind": "resident",
        "offset": offset,
        "vram_start": target_value,
        "vram_end": target_value + target_size,
        "rom_start": target_value - ot.VRAM_ROM_DELTA,
        "rom_end": target_value - ot.VRAM_ROM_DELTA + target_size,
        "exports": [],
    }, records


def _resident_boundary(
    elf: rs.Elf, target_value: int, target_size: int, section: str
) -> dict[str, object]:
    end = target_value + target_size
    interior: list[tuple[str, int]] = []
    following: dict[int, list[str]] = {}
    for name, value, size, info, shndx in elf.symbols():
        if not name or size == 0 or shndx == rs.SHN_UNDEF or (info & 0xF) != 2:
            continue
        symbol_section = elf.names[shndx] if shndx < len(elf.names) else ""
        if symbol_section != section:
            continue
        if target_value < value < end:
            interior.append((name, value))
        elif value >= end:
            following.setdefault(value, []).append(name)
    if interior:
        rendered = ", ".join(f"{name}@0x{value:X}" for name, value in interior)
        raise PreflightError(f"resident owned range contains another function: {rendered}")
    if not following:
        return {"next_symbol": None, "next_start": None, "padding_size": None}
    next_start = min(following)
    return {
        "next_symbol": "/".join(sorted(set(following[next_start]))),
        "next_start": next_start,
        "padding_size": next_start - end,
    }


def _inbound_references(
    context: dict[str, object], rom: bytes
) -> list[dict[str, object]]:
    target_identity = (
        int(context.get("overlay", 0)),
        int(context["offset"]),
    )
    rom_table = ot.read_rom_table(rom)
    modules = ot.build_modules(ot.read_headers(rom))
    refs: list[dict[str, object]] = []

    count, resident = ot.read_reloc_table(rom)
    if count != len(resident):
        raise PreflightError("resident relocation table count disagrees")
    resident_records = []
    for row in resident:
        resident_records.append(
            dict(
                row,
                mode=row["flags"] >> 4,
                op=row["flags"] & 0xF,
                symbol_index=row["rom_table_index"],
                relative_offset=row["call_site_offset"],
            )
        )
    attached = rs._attach_runtime_identities(
        resident_records,
        rom,
        0,
        rom_table,
        lambda row: ot.RESIDENT_VRAM_BASE + row["call_site_offset"] - ot.VRAM_ROM_DELTA,
    )
    for source, record in zip(resident_records, attached):
        if record.identity == target_identity:
            refs.append(
                {
                    "source": "resident",
                    "site": f"vram:0x{ot.RESIDENT_VRAM_BASE + source['call_site_offset']:X}",
                    "type": TYPE_NAMES.get(record.rtype, str(record.rtype)),
                }
            )

    for module in modules:
        raw = ot.read_module_relocations(rom, module, rom_table)
        prepared = [dict(row, relative_offset=row["target_offset"]) for row in raw]
        attached = rs._attach_runtime_identities(
            prepared,
            rom,
            module["overlay"],
            rom_table,
            lambda row, module=module: module["rom_start"] + row["target_offset"],
        )
        for source, record in zip(prepared, attached):
            if record.identity == target_identity:
                refs.append(
                    {
                        "source": f"overlay:{module['overlay']}",
                        "site": f"+0x{source['target_offset']:X}",
                        "type": TYPE_NAMES.get(record.rtype, str(record.rtype)),
                    }
                )
    return refs


def _summary_integer(
    payload: dict[str, object], *names: str, optional: bool = False
) -> int | None:
    values: list[tuple[str, int | None]] = []
    for name in names:
        if name not in payload:
            continue
        value = payload[name]
        if value is None and optional:
            values.append((name, None))
        elif isinstance(value, int) and not isinstance(value, bool):
            values.append((name, value))
        else:
            raise PreflightError(f"workbench summary metric {name} is not an integer")
    if not values:
        if optional:
            return None
        raise PreflightError(
            "workbench summary lacks metric " + "/".join(names)
        )
    if len({value for _name, value in values}) != 1:
        raise PreflightError(
            "workbench summary metric aliases disagree: "
            + ", ".join(name for name, _value in values)
        )
    return values[0][1]


def _summary_boolean(payload: dict[str, object], name: str) -> bool:
    value = payload.get(name)
    if not isinstance(value, bool):
        raise PreflightError(f"workbench summary field {name} is not a boolean")
    return value


def _summary_digest(payload: dict[str, object], name: str) -> str | None:
    record = payload.get(name)
    if record is None:
        return None
    if not isinstance(record, dict):
        raise PreflightError(f"workbench provenance {name} record is malformed")
    value = record.get("sha256")
    if value is not None and (
        not isinstance(value, str)
        or not re.fullmatch(r"[0-9a-f]{64}", value)
    ):
        raise PreflightError(f"workbench provenance {name} digest is malformed")
    return value


def _verified_summary_artifact(
    manifest: dict[str, object], name: str, *, root: Path = REPO
) -> Path | None:
    """Return one manifest artifact only when its declared bytes still agree."""

    record = manifest.get(name)
    if record is None:
        return None
    if not isinstance(record, dict):
        raise PreflightError(f"workbench provenance {name} record is malformed")
    raw_path = record.get("path")
    digest = record.get("sha256")
    size = record.get("size")
    # Older/minimal manifests carry a digest but no path. That is unavailable
    # relocation evidence, not permission to guess a canonical artifact.
    if raw_path is None or digest is None:
        return None
    if (
        not isinstance(raw_path, str)
        or not raw_path
        or Path(raw_path).is_absolute()
        or ".." in Path(raw_path).parts
    ):
        raise PreflightError(f"workbench provenance {name} path is unsafe")
    if not isinstance(digest, str) or not re.fullmatch(r"[0-9a-f]{64}", digest):
        raise PreflightError(f"workbench provenance {name} digest is malformed")
    if size is not None and (
        not isinstance(size, int) or isinstance(size, bool) or size < 0
    ):
        raise PreflightError(f"workbench provenance {name} size is malformed")
    path = root / raw_path
    try:
        path.resolve().relative_to(root.resolve())
    except ValueError as error:
        raise PreflightError(
            f"workbench provenance {name} artifact escapes its proof root"
        ) from error
    if not path.is_file():
        return None
    if path.is_symlink():
        raise PreflightError(f"workbench provenance {name} artifact is a symlink")
    if size is not None and path.stat().st_size != size:
        raise PreflightError(f"workbench provenance {name} size no longer agrees")
    if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
        raise PreflightError(f"workbench provenance {name} digest no longer agrees")
    return path


def _repository_summary_context() -> dict[str, str]:
    """Bind one transient summary to the lane state that produced it."""

    head = _run(["git", "rev-parse", "--verify", "HEAD"], capture=True)
    branch = _run(["git", "branch", "--show-current"], capture=True)
    if head.returncode or branch.returncode:
        raise PreflightError("cannot authenticate workbench summary repository state")
    base = head.stdout.strip()
    branch_name = branch.stdout.strip()
    if not re.fullmatch(r"[0-9a-f]{40}", base) or not branch_name:
        raise PreflightError(
            "workbench summary requires one attached branch and full base commit"
        )
    return {"base": base, "branch": branch_name}


def _summary_proof_context(
    manifest: dict[str, object], *, manifest_sha256: str
) -> dict[str, object] | None:
    """Authenticate and retain the scalar provenance needed for later pairing.

    Pairing deliberately retains hashes and repository-relative paths, not the
    objects themselves. Each path is re-read here, while the proof is being
    produced. The short freshness window and base/branch checks in the pairing
    command prevent an old receipt from being relabelled as current evidence.
    """

    paths: dict[str, Path] = {}
    records: dict[str, dict[str, object]] = {}
    for name in ("source", "candidate_object", "target_object"):
        path = _verified_summary_artifact(manifest, name)
        if path is None:
            return None
        record = manifest.get(name)
        assert isinstance(record, dict)
        digest = record.get("sha256")
        assert isinstance(digest, str)
        paths[name] = path
        records[name] = {
            "path": _relative(path),
            "sha256": digest,
            "size": path.stat().st_size,
        }
    source = paths["source"]
    try:
        source.relative_to(REPO / "src")
    except ValueError as error:
        raise PreflightError(
            "workbench summary source owner escapes the canonical source tree"
        ) from error
    repository = _repository_summary_context()
    return {
        **repository,
        "owner": _relative(source),
        "manifest_sha256": manifest_sha256,
        "artifacts": records,
        "created_unix_ns": time.time_ns(),
    }


def _authenticated_summary_relocation_comparison(
    manifest: dict[str, object],
    *,
    requested_symbol: str,
    target_symbol: str,
    candidate_symbol: str,
    comparison_mode: str,
) -> dict[str, object] | None:
    """Reproduce canonical relocation evidence from one workbench manifest.

    Assembly mode authenticates the guarded fallback/static object pairing.
    ROM mode authenticates the promoted full-TU object against the linked
    target/runtime surface. These are intentionally separate evidence modes;
    neither count is copied into or inferred from the other.
    """

    manifest_mode = manifest.get("mode")
    if manifest_mode is None:
        return None
    if manifest_mode != comparison_mode:
        raise PreflightError(
            "workbench provenance mode disagrees with summary mode"
        )
    manifest_target = manifest.get("symbol")
    manifest_candidate = manifest.get("candidate_symbol")
    if manifest_target is None or manifest_candidate is None:
        return None

    if comparison_mode == "rom":
        if (
            manifest_target != requested_symbol
            or manifest_candidate not in {requested_symbol, candidate_symbol}
        ):
            raise PreflightError(
                "ROM workbench provenance symbols disagree with requested identity"
            )
        return _authenticated_promoted_summary_relocation_comparison(
            manifest,
            requested_symbol=requested_symbol,
            target_symbol=target_symbol,
            candidate_symbol=candidate_symbol,
        )
    if manifest_target != target_symbol or manifest_candidate != candidate_symbol:
        raise PreflightError(
            "workbench provenance symbols disagree with summary identity"
        )
    if comparison_mode != "asm":
        raise PreflightError(
            f"unsupported workbench relocation evidence mode {comparison_mode!r}"
        )

    source = _verified_summary_artifact(manifest, "source")
    candidate_object = _verified_summary_artifact(manifest, "candidate_object")
    target_object = _verified_summary_artifact(manifest, "target_object")
    if source is None or candidate_object is None or target_object is None:
        return None
    try:
        source_relative = source.relative_to(REPO / "src")
        candidate_relative = candidate_object.relative_to(REPO)
    except ValueError as error:
        raise PreflightError(
            "workbench provenance relocation inputs escape canonical roots"
        ) from error
    translation_unit = source_relative.with_suffix("").as_posix()
    candidate_build = candidate_relative.parts[0]
    expected_candidate = Path(candidate_build) / "src" / f"{translation_unit}.c.o"
    if (
        candidate_build not in {"build", "build_non_matching"}
        or candidate_relative != expected_candidate
    ):
        raise PreflightError(
            "workbench provenance candidate object disagrees with source"
        )
    if not re.fullmatch(r"[A-Za-z_]\w*", requested_symbol):
        raise PreflightError("requested workbench symbol cannot name a proof artifact")
    # wb_compare.sh names the isolated artifact after the spelling requested
    # on its command line. Its internal symbol remains ``target_symbol`` and
    # is authenticated by function_surface_comparison below. Requiring the
    # generated spelling in both places incorrectly rejected friendly aliases.
    expected_target = REPO / "build/wb" / f"{requested_symbol}.target.o"
    if target_object != expected_target:
        raise PreflightError(
            "workbench provenance target object disagrees with requested symbol"
        )
    for path in (TARGET_ELF, ROM, ATLAS, ALIASES):
        if not path.is_file():
            return None

    target_elf = rs.Elf(TARGET_ELF)
    linked_name, linked_value, _linked_size, _linked_section = _symbol_geometry(
        target_elf, (candidate_symbol, target_symbol)
    )
    generated = rs.GEN_NAME_RE.match(target_symbol)
    if generated is not None:
        expected_value = rs.SYNTHETIC_VMA + int(generated.group(2), 16)
        if linked_value != expected_value:
            raise PreflightError(
                "generated target symbol offset disagrees with linked friendly identity"
            )

    redefine_aliases = _candidate_redefine_aliases(candidate_object)
    try:
        return rs.function_surface_comparison(
            requested_symbol,
            candidate_object,
            TARGET_ELF,
            rom_path=ROM,
            atlas_path=ATLAS,
            values_path=ALIASES,
            candidate_symbol=candidate_symbol,
            target_symbol=linked_name,
            source=translation_unit,
            candidate_redefine_aliases=redefine_aliases,
        )
    except rs.SurfaceComparisonError as error:
        raise PreflightError(
            f"cannot authenticate workbench relocation summary: {error}"
        ) from error


def _authenticated_promoted_summary_relocation_comparison(
    manifest: dict[str, object],
    *,
    requested_symbol: str,
    target_symbol: str,
    candidate_symbol: str,
) -> dict[str, object] | None:
    """Reproduce the promoted linked/static identity surface from ROM proof.

    The ROM comparison itself contains no relocations. Its provenance does,
    however, identify the promoted source and full-TU object. The canonical
    linked ELF and retail runtime table then provide a distinct, authenticated
    relocation surface. Absence is reported as absence; no fallback surface is
    substituted.
    """

    source = _verified_summary_artifact(manifest, "source")
    candidate_object = _verified_summary_artifact(manifest, "candidate_object")
    target_artifact = _verified_summary_artifact(manifest, "target_object")
    if source is None or candidate_object is None or target_artifact is None:
        return None
    if not re.fullmatch(r"[A-Za-z_]\w*", requested_symbol):
        raise PreflightError("requested workbench symbol cannot name a proof artifact")
    expected_target = REPO / "build/wb" / f"{requested_symbol}.target.objdump"
    if target_artifact != expected_target:
        raise PreflightError(
            "promoted workbench target artifact disagrees with requested symbol"
        )
    resolution = resolve(requested_symbol)
    if resolution.resolution_mode != "post_promotion":
        raise PreflightError(
            "ROM relocation summary requires one authenticated promoted definition"
        )
    if (
        resolution.target_symbol != target_symbol
        or resolution.candidate_symbol != candidate_symbol
    ):
        raise PreflightError(
            "promoted resolution symbols disagree with workbench summary identity"
        )
    if source != resolution.source or candidate_object != resolution.candidate_object:
        raise PreflightError(
            "promoted workbench provenance disagrees with canonical source/object owner"
        )
    for path, label in (
        (TARGET_ELF, "canonical linked ELF"),
        (ROM, "baserom"),
        (ATLAS, "overlay atlas"),
        (ALIASES, "overlay identity table"),
    ):
        if not path.is_file():
            return None
    _require_fresh_target(
        resolution.candidate_object,
        label="promoted candidate object",
        non_matching=False,
        build_logic_inputs=_build_logic_inputs(),
    )
    _require_fresh_linked_boundary()
    target_elf = rs.Elf(TARGET_ELF)
    linked_name, value, size, section = _symbol_geometry(
        target_elf, (candidate_symbol, target_symbol)
    )
    _require_tracked_geometry(resolution, value, size)
    try:
        atlas = json.loads(ATLAS.read_text(encoding="utf-8"))
        rom = ROM.read_bytes()
        _target_context(resolution, value, size, atlas, rom)
        comparison = rs.function_surface_comparison(
            requested_symbol,
            candidate_object,
            TARGET_ELF,
            rom_path=ROM,
            atlas_path=ATLAS,
            values_path=ALIASES,
            candidate_symbol=candidate_symbol,
            target_symbol=linked_name,
            source=resolution.translation_unit,
            candidate_redefine_aliases=_candidate_redefine_aliases(candidate_object),
        )
    except (json.JSONDecodeError, OSError, rs.SurfaceComparisonError) as error:
        raise PreflightError(
            f"cannot authenticate promoted linked relocation summary: {error}"
        ) from error
    return comparison


def _canonical_summary_symbols(
    requested_symbol: str,
    target_symbol: str,
    candidate_symbol: str,
    comparison_mode: str,
) -> tuple[str, str]:
    """Normalize ROM-mode friendly spellings to the canonical target identity."""

    if comparison_mode != "rom":
        return target_symbol, candidate_symbol
    resolution = resolve(requested_symbol)
    if resolution.resolution_mode != "post_promotion":
        raise PreflightError(
            "ROM workbench summary requires one authenticated promoted definition"
        )
    if target_symbol != requested_symbol or candidate_symbol not in {
        requested_symbol,
        resolution.candidate_symbol,
    }:
        raise PreflightError(
            "ROM workbench invocation symbols disagree with promoted resolution"
        )
    return resolution.target_symbol, resolution.candidate_symbol


def _summary_relocation_surface(
    comparison: dict[str, object], *, evidence_mode: str
) -> dict[str, object]:
    """Validate and reduce one named relocation surface without inference."""

    if evidence_mode not in WB_RELOCATION_SURFACE_MODES.values():
        raise PreflightError(f"invalid relocation evidence mode {evidence_mode!r}")

    required = (
        "candidate_record_count",
        "target_record_count",
        "offset_type_alignment_count",
        "stable_identity_alignment_count",
        "candidate_identity_resolved_count",
    )
    values: dict[str, int] = {}
    for name in required:
        value = comparison.get(name)
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise PreflightError(
                f"authenticated relocation comparison has invalid {name}"
            )
        values[name] = value
    candidate = values["candidate_record_count"]
    target = values["target_record_count"]
    surface = min(candidate, target)
    if values["offset_type_alignment_count"] > surface:
        raise PreflightError(
            "authenticated relocation offset/type count exceeds its surface"
        )
    if values["stable_identity_alignment_count"] > surface:
        raise PreflightError(
            "authenticated exact relocation identities exceed their surface"
        )
    if values["stable_identity_alignment_count"] > values["offset_type_alignment_count"]:
        raise PreflightError(
            "authenticated exact relocation identities exceed offset/type alignment"
        )
    if values["candidate_identity_resolved_count"] > candidate:
        raise PreflightError(
            "authenticated resolved relocation identities exceed candidate count"
        )
    if values["stable_identity_alignment_count"] > values[
        "candidate_identity_resolved_count"
    ]:
        raise PreflightError(
            "authenticated exact relocation identities exceed resolved identities"
        )
    unresolved = comparison.get("candidate_identity_unresolved_records")
    if not isinstance(unresolved, list) or (
        values["candidate_identity_resolved_count"] + len(unresolved) != candidate
    ):
        raise PreflightError(
            "authenticated relocation unresolved diagnostics disagree with counts"
        )

    exact_identities = values["stable_identity_alignment_count"]
    complete = (
        candidate == target == values["offset_type_alignment_count"]
        and values["candidate_identity_resolved_count"] == candidate
    )
    return {
        "candidate_relocations": candidate,
        "target_relocations": target,
        "offset_type_relocations": values["offset_type_alignment_count"],
        "resolved_candidate_identities": values[
            "candidate_identity_resolved_count"
        ],
        "exact_relocation_identities": exact_identities,
        "evidence_mode": evidence_mode,
        "complete": complete,
    }


def _summary_relocations(surface: dict[str, object]) -> dict[str, int]:
    """Retain the legacy single-surface scalar view for existing consumers."""

    return {
        name: int(surface[name])
        for name in (
            "candidate_relocations",
            "target_relocations",
            "exact_relocation_identities",
        )
    }


def workbench_summary(
    raw_path: Path,
    manifest_path: Path,
    *,
    requested_symbol: str,
    target_symbol: str,
    candidate_symbol: str,
    comparison_mode: str,
    boundary_evidence: str,
    boundary_size: int | None,
) -> dict[str, object]:
    """Reduce a full comparison to stable, code-free automation evidence."""

    try:
        raw_bytes = raw_path.read_bytes()
        payload = json.loads(raw_bytes)
        manifest_bytes = manifest_path.read_bytes()
        manifest = json.loads(manifest_bytes)
    except (OSError, json.JSONDecodeError) as error:
        raise PreflightError(f"cannot read workbench summary inputs: {error}") from error
    if not isinstance(payload, dict) or payload.get("schema") != "decomp-workbench-comparison-v1":
        schema = payload.get("schema") if isinstance(payload, dict) else None
        raise PreflightError(f"unexpected workbench schema {schema!r}")
    if not isinstance(manifest, dict) or manifest.get("schema") != "mickey-wb-proof-provenance-v1":
        schema = manifest.get("schema") if isinstance(manifest, dict) else None
        raise PreflightError(f"unexpected provenance schema {schema!r}")
    selection = manifest.get("selection")
    if not isinstance(selection, dict) or not isinstance(
        selection.get("classification"), str
    ):
        raise PreflightError("workbench provenance selection is malformed")

    provenance_allowed = _summary_boolean(manifest, "exact_claim_allowed")
    comparison_exact = _summary_boolean(payload, "exact")
    comparison_accepted = _summary_boolean(payload, "accepted")
    # decomp-workbench's build_provenance block (schema
    # decomp-workbench-build-provenance-v1) says whether the compared object
    # was known to be a stock build. An exact result only claims a real
    # match when claim == "match"; a forced/instrumented/unknown build still
    # reports exact=True but is not admissible evidence of a match. Older
    # raw reports carry no such block at all -- treat that as unconstrained
    # (verified), not as a denial, so behaviour on those is unchanged.
    build_provenance = payload.get("build_provenance")
    build_claim = None
    if isinstance(build_provenance, dict):
        claim_value = build_provenance.get("claim")
        if isinstance(claim_value, str):
            build_claim = claim_value
    build_provenance_verified = build_claim is None or build_claim == "match"
    if comparison_exact and not build_provenance_verified:
        print(
            "function_preflight: workbench reported exact=True but "
            f"build_provenance.claim={build_claim!r} (not 'match'); "
            "not treating this as a verified match.",
            file=sys.stderr,
        )
    target_words = _summary_integer(payload, "target_instructions", "target_insns")
    candidate_words = _summary_integer(
        payload, "candidate_instructions", "insns"
    )
    differing_words = _summary_integer(payload, "word_mismatches", "words")
    first_row = _summary_integer(payload, "first_divergent_row", optional=True)
    target_frame = _summary_integer(
        payload, "target_frame_size", "target_frame", optional=True
    )
    candidate_frame = _summary_integer(
        payload, "candidate_frame_size", "frame", optional=True
    )
    assert target_words is not None
    assert candidate_words is not None
    assert differing_words is not None
    if first_row is not None and first_row < 0:
        raise PreflightError("workbench first divergent row is negative")
    if min(target_words, candidate_words, differing_words) < 0:
        raise PreflightError("workbench instruction metrics cannot be negative")
    proved_boundary_size = target_words * 4 if boundary_size is None else boundary_size
    if proved_boundary_size != target_words * 4:
        raise PreflightError(
            "preflight boundary size disagrees with compared target: "
            f"boundary={proved_boundary_size}, target={target_words * 4}"
        )

    target_symbol, candidate_symbol = _canonical_summary_symbols(
        requested_symbol,
        target_symbol,
        candidate_symbol,
        comparison_mode,
    )

    relocation_comparison = _authenticated_summary_relocation_comparison(
        manifest,
        requested_symbol=requested_symbol,
        target_symbol=target_symbol,
        candidate_symbol=candidate_symbol,
        comparison_mode=comparison_mode,
    )
    relocations = None
    relocation_surfaces = None
    proof_context = None
    if relocation_comparison is not None:
        surface_name = (
            "fallback_static" if comparison_mode == "asm" else "promoted_linked"
        )
        surface = _summary_relocation_surface(
            relocation_comparison,
            evidence_mode=WB_RELOCATION_SURFACE_MODES[surface_name],
        )
        relocations = _summary_relocations(surface)
        proof_context = _summary_proof_context(
            manifest,
            manifest_sha256=hashlib.sha256(manifest_bytes).hexdigest(),
        )
        if proof_context is None:
            raise PreflightError(
                "authenticated relocation surface lacks complete proof artifacts"
            )
        relocation_surfaces = {surface_name: surface}

    result = {
        "schema": WB_SUMMARY_SCHEMA,
        "symbol": {
            "requested": requested_symbol,
            "target": target_symbol,
            "candidate": candidate_symbol,
        },
        "mode": comparison_mode,
        "boundary": {
            "bytes": proved_boundary_size,
            "evidence": boundary_evidence,
        },
        "comparison": {
            "exact": comparison_exact,
            "accepted": comparison_accepted,
            "acceptance_basis": payload.get("acceptance_basis"),
            "verdict": payload.get("verdict"),
            "target_words": target_words,
            "candidate_words": candidate_words,
            "instruction_delta": _summary_integer(
                payload, "instruction_delta", "insn_delta"
            ),
            "matched_words": (
                target_words - differing_words
                if target_words == candidate_words
                else None
            ),
            "differing_words": differing_words,
            "raw_differing_words": _summary_integer(
                payload, "raw_word_mismatches", "raw"
            ),
            "normalized_distance": _summary_integer(
                payload, "normalized_distance", "norm"
            ),
            "opcode_mismatches": _summary_integer(
                payload, "opcode_mismatches", "opcodes"
            ),
            "register_mismatches": _summary_integer(
                payload, "register_mismatches", "regs"
            ),
            "fp_register_mismatches": _summary_integer(
                payload, "fp_register_mismatches", "fp"
            ),
            "aligned_differences": _summary_integer(payload, "aligned_total"),
            "aligned_structural": _summary_integer(payload, "aligned_structural"),
            "aligned_schedule": _summary_integer(payload, "aligned_schedule"),
            "aligned_register": _summary_integer(payload, "aligned_register"),
            "aligned_constant": _summary_integer(payload, "aligned_constant"),
            "first_mismatch_offset": None if first_row is None else first_row * 4,
            "target_frame_bytes": None if target_frame is None else abs(target_frame),
            "candidate_frame_bytes": (
                None if candidate_frame is None else abs(candidate_frame)
            ),
            "relocation_metadata_mismatches": _summary_integer(
                payload, "relocation_metadata_mismatches"
            ),
            "relocation_target_mismatches": _summary_integer(
                payload, "relocation_target_mismatches"
            ),
        },
        "provenance": {
            "classification": selection["classification"],
            "exact_claim_allowed": provenance_allowed,
            "build_claim": build_claim,
            "verdict": manifest.get("verdict"),
            "source_sha256": _summary_digest(manifest, "source"),
            "candidate_object_sha256": _summary_digest(
                manifest, "candidate_object"
            ),
            "target_object_sha256": _summary_digest(manifest, "target_object"),
        },
        "evidence": {
            "admissible_exact_comparison": (
                comparison_accepted and provenance_allowed and build_provenance_verified
            ),
            "promotion_proof_included": False,
            "scope": "workbench-comparison-not-canonical-promotion-proof",
            "raw_report_sha256": hashlib.sha256(raw_bytes).hexdigest(),
        },
    }
    if relocations is not None:
        result["relocations"] = relocations
        result["relocation_surfaces"] = relocation_surfaces
        result["proof_context"] = proof_context
    return result


def _validated_summary_surface(
    payload: dict[str, object], surface_name: str
) -> tuple[dict[str, object], dict[str, object]]:
    """Validate one unpaired summary and return its surface and proof context."""

    if payload.get("schema") != WB_SUMMARY_SCHEMA:
        raise PreflightError("relocation pair input has unsupported summary schema")
    expected_mode = "asm" if surface_name == "fallback_static" else "rom"
    if payload.get("mode") != expected_mode:
        raise PreflightError(
            f"{surface_name} input has wrong comparison mode {payload.get('mode')!r}"
        )
    surfaces = payload.get("relocation_surfaces")
    if not isinstance(surfaces, dict) or set(surfaces) != {surface_name}:
        raise PreflightError(
            f"{surface_name} input must contain exactly its one named surface"
        )
    surface = surfaces[surface_name]
    if not isinstance(surface, dict):
        raise PreflightError(f"{surface_name} relocation surface is malformed")
    expected_evidence_mode = WB_RELOCATION_SURFACE_MODES[surface_name]
    if surface.get("evidence_mode") != expected_evidence_mode:
        raise PreflightError(
            f"{surface_name} relocation evidence mode is inconsistent"
        )
    numeric_names = (
        "candidate_relocations",
        "target_relocations",
        "offset_type_relocations",
        "resolved_candidate_identities",
        "exact_relocation_identities",
    )
    numbers: dict[str, int] = {}
    for name in numeric_names:
        value = surface.get(name)
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise PreflightError(f"{surface_name} has invalid {name}")
        numbers[name] = value
    candidate = numbers["candidate_relocations"]
    target = numbers["target_relocations"]
    if numbers["offset_type_relocations"] > min(candidate, target):
        raise PreflightError(f"{surface_name} offset/type count exceeds its surface")
    if numbers["resolved_candidate_identities"] > candidate:
        raise PreflightError(
            f"{surface_name} resolved identities exceed candidate count"
        )
    if numbers["exact_relocation_identities"] > min(candidate, target):
        raise PreflightError(f"{surface_name} exact identities exceed its surface")
    if numbers["exact_relocation_identities"] > numbers["offset_type_relocations"]:
        raise PreflightError(
            f"{surface_name} exact identities exceed offset/type alignment"
        )
    if numbers["exact_relocation_identities"] > numbers[
        "resolved_candidate_identities"
    ]:
        raise PreflightError(
            f"{surface_name} exact identities exceed resolved identities"
        )
    complete = surface.get("complete")
    expected_complete = (
        candidate == target == numbers["offset_type_relocations"]
        and numbers["resolved_candidate_identities"] == candidate
    )
    if not isinstance(complete, bool) or complete != expected_complete:
        raise PreflightError(f"{surface_name} completeness is inconsistent")

    legacy = payload.get("relocations")
    if not isinstance(legacy, dict) or legacy != _summary_relocations(surface):
        raise PreflightError(
            f"{surface_name} legacy relocation view disagrees with named surface"
        )
    context = payload.get("proof_context")
    if not isinstance(context, dict):
        raise PreflightError(f"{surface_name} lacks authenticated proof context")
    base = context.get("base")
    branch = context.get("branch")
    owner = context.get("owner")
    created = context.get("created_unix_ns")
    manifest_digest = context.get("manifest_sha256")
    if not isinstance(base, str) or not re.fullmatch(r"[0-9a-f]{40}", base):
        raise PreflightError(f"{surface_name} proof base is malformed")
    if not isinstance(branch, str) or not branch or "\n" in branch:
        raise PreflightError(f"{surface_name} proof branch is malformed")
    if (
        not isinstance(owner, str)
        or not owner.startswith("src/")
        or Path(owner).is_absolute()
        or ".." in Path(owner).parts
    ):
        raise PreflightError(f"{surface_name} proof owner is malformed")
    if not isinstance(created, int) or isinstance(created, bool) or created < 0:
        raise PreflightError(f"{surface_name} proof timestamp is malformed")
    if not isinstance(manifest_digest, str) or not re.fullmatch(
        r"[0-9a-f]{64}", manifest_digest
    ):
        raise PreflightError(f"{surface_name} proof manifest digest is malformed")

    artifacts = context.get("artifacts")
    provenance = payload.get("provenance")
    if not isinstance(artifacts, dict) or not isinstance(provenance, dict):
        raise PreflightError(f"{surface_name} proof artifacts are malformed")
    provenance_names = {
        "source": "source_sha256",
        "candidate_object": "candidate_object_sha256",
        "target_object": "target_object_sha256",
    }
    for artifact_name, provenance_name in provenance_names.items():
        record = artifacts.get(artifact_name)
        if not isinstance(record, dict):
            raise PreflightError(
                f"{surface_name} proof lacks {artifact_name} artifact"
            )
        path = record.get("path")
        digest = record.get("sha256")
        size = record.get("size")
        if (
            not isinstance(path, str)
            or not path
            or Path(path).is_absolute()
            or ".." in Path(path).parts
            or not isinstance(digest, str)
            or not re.fullmatch(r"[0-9a-f]{64}", digest)
            or not isinstance(size, int)
            or isinstance(size, bool)
            or size < 0
        ):
            raise PreflightError(
                f"{surface_name} proof {artifact_name} artifact is malformed"
            )
        if provenance.get(provenance_name) != digest:
            raise PreflightError(
                f"{surface_name} proof {artifact_name} hash disagrees with provenance"
            )
        if artifact_name == "source" and path != owner:
            raise PreflightError(
                f"{surface_name} source artifact disagrees with proof owner"
            )
    symbols = payload.get("symbol")
    if not isinstance(symbols, dict) or any(
        not isinstance(symbols.get(name), str)
        for name in ("requested", "target", "candidate")
    ):
        raise PreflightError(f"{surface_name} symbol identity is malformed")
    evidence = payload.get("evidence")
    if not isinstance(evidence, dict) or evidence.get("promotion_proof_included") is not False:
        raise PreflightError(
            f"{surface_name} summary cannot claim canonical promotion proof"
        )
    return surface, context


def _read_fresh_relocation_summary(
    path: Path,
    *,
    surface_name: str,
    root: Path,
    now_ns: int,
    max_age_seconds: int,
) -> tuple[dict[str, object], dict[str, object], bytes]:
    try:
        resolved = path.resolve(strict=True)
        resolved.relative_to((root / "build").resolve())
    except (OSError, ValueError) as error:
        raise PreflightError(
            f"{surface_name} summary must be one existing file under build/"
        ) from error
    if path.is_symlink() or not resolved.is_file():
        raise PreflightError(f"{surface_name} summary cannot be a symlink")
    raw = resolved.read_bytes()
    if len(raw) > 1024 * 1024:
        raise PreflightError(f"{surface_name} summary is unexpectedly large")
    try:
        payload = json.loads(raw)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise PreflightError(f"{surface_name} summary is malformed JSON") from error
    if not isinstance(payload, dict):
        raise PreflightError(f"{surface_name} summary is not one JSON object")
    surface, context = _validated_summary_surface(payload, surface_name)
    created = int(context["created_unix_ns"])
    age = now_ns - created
    if age < 0 or age > max_age_seconds * 1_000_000_000:
        raise PreflightError(f"{surface_name} summary is stale")
    mtime_age = now_ns - resolved.stat().st_mtime_ns
    if mtime_age < 0 or mtime_age > max_age_seconds * 1_000_000_000:
        raise PreflightError(f"{surface_name} summary file is stale")
    return payload, context, raw


def compose_relocation_summaries(
    fallback_path: Path,
    promoted_path: Path,
    *,
    root: Path = REPO,
    now_ns: int | None = None,
    max_age_seconds: int = WB_SUMMARY_MAX_AGE_SECONDS,
    repository_context: dict[str, str] | None = None,
) -> dict[str, object]:
    """Compose fresh fallback-static and promoted-linked proof receipts."""

    if fallback_path.resolve() == promoted_path.resolve():
        raise PreflightError("relocation pair inputs are duplicates")
    current_ns = time.time_ns() if now_ns is None else now_ns
    fallback, fallback_context, fallback_raw = _read_fresh_relocation_summary(
        fallback_path,
        surface_name="fallback_static",
        root=root,
        now_ns=current_ns,
        max_age_seconds=max_age_seconds,
    )
    promoted, promoted_context, promoted_raw = _read_fresh_relocation_summary(
        promoted_path,
        surface_name="promoted_linked",
        root=root,
        now_ns=current_ns,
        max_age_seconds=max_age_seconds,
    )
    if hashlib.sha256(fallback_raw).digest() == hashlib.sha256(promoted_raw).digest():
        raise PreflightError("relocation pair inputs have duplicate contents")
    for field in ("base", "branch", "owner"):
        if fallback_context[field] != promoted_context[field]:
            raise PreflightError(f"relocation pair has cross-{field} evidence")
    if fallback.get("symbol") != promoted.get("symbol"):
        raise PreflightError("relocation pair has cross-symbol evidence")
    fallback_boundary = fallback.get("boundary")
    promoted_boundary = promoted.get("boundary")
    if (
        not isinstance(fallback_boundary, dict)
        or not isinstance(promoted_boundary, dict)
        or fallback_boundary.get("bytes") != promoted_boundary.get("bytes")
    ):
        raise PreflightError("relocation pair has inconsistent owned boundary")
    current_repository = (
        _repository_summary_context()
        if repository_context is None
        else repository_context
    )
    for field in ("base", "branch"):
        if fallback_context[field] != current_repository.get(field):
            raise PreflightError(
                f"relocation pair {field} is stale for the current lane"
            )
    fallback_surface = fallback["relocation_surfaces"]["fallback_static"]
    promoted_surface = promoted["relocation_surfaces"]["promoted_linked"]
    assert isinstance(fallback_surface, dict)
    assert isinstance(promoted_surface, dict)
    if (
        fallback_surface["candidate_relocations"]
        != promoted_surface["candidate_relocations"]
        or fallback_surface["target_relocations"]
        != promoted_surface["target_relocations"]
    ):
        raise PreflightError(
            "relocation pair candidate/target surfaces have inconsistent geometry"
        )

    result = copy.deepcopy(fallback)
    result["relocation_surfaces"] = {
        "fallback_static": copy.deepcopy(fallback_surface),
        "promoted_linked": copy.deepcopy(promoted_surface),
    }
    # Preserve the fallback-static scalar view for old checkpoint consumers.
    # A promoted linked result must never upgrade an inexact static object into
    # an object-exact claim through the compatibility field.
    result["relocations"] = _summary_relocations(fallback_surface)
    result["proof_context"] = {
        "base": fallback_context["base"],
        "branch": fallback_context["branch"],
        "owner": fallback_context["owner"],
        "created_unix_ns": current_ns,
        "surface_proofs": {
            "fallback_static": copy.deepcopy(fallback_context),
            "promoted_linked": copy.deepcopy(promoted_context),
        },
    }
    result["relocation_pair"] = {
        "fallback_summary_sha256": hashlib.sha256(fallback_raw).hexdigest(),
        "promoted_summary_sha256": hashlib.sha256(promoted_raw).hexdigest(),
        "legacy_surface": "fallback_static",
        "promotion_policy": "unchanged",
    }
    return result


def _render_workbench_summary_human(report: dict[str, object]) -> None:
    symbols = report["symbol"]
    assert isinstance(symbols, dict)
    print(
        "identity: "
        f"{symbols['requested']} ({symbols['target']} -> {symbols['candidate']})"
    )
    surfaces = report.get("relocation_surfaces")
    if not isinstance(surfaces, dict):
        print("relocation surfaces: unavailable")
        return
    print("relocation surfaces:")
    for name in ("fallback_static", "promoted_linked"):
        surface = surfaces.get(name)
        if not isinstance(surface, dict):
            continue
        state = "complete" if surface["complete"] else "partial"
        print(
            f"  {surface['evidence_mode']}: "
            f"candidate={surface['candidate_relocations']} "
            f"target={surface['target_relocations']} "
            f"offset/type={surface['offset_type_relocations']} "
            f"resolved={surface['resolved_candidate_identities']} "
            f"exact identities={surface['exact_relocation_identities']} "
            f"completeness={state}"
        )


def _workbench(resolution: Resolution, *, no_build: bool = False) -> dict[str, object]:
    """Run the workbench oracle for one resolution.

    ``no_build`` is forwarded to ``wb_compare.sh --rom``.  Without it the
    ROM oracle refreshes ``build/mickey.us.z64`` from the linked ELF, and two
    concurrent proofs then rewrite and read that one shared ROM at the same
    time: the reader dumps a half-written image and the comparison fails with
    exit 2.  A ``--no-build`` proof only reads, and requires the ROM to be
    strictly newer than the ELF, so it is safe to run in parallel once the
    caller has refreshed the build.
    """
    if resolution.resolution_mode == "post_promotion":
        command = [
            str(WB_COMPARE),
            *(["--no-build"] if no_build else []),
            "--rom",
            resolution.candidate_symbol,
            "--json",
            "--color",
            "never",
        ]
        comparison_mode = "rom"
    else:
        command = [
            str(WB_COMPARE),
            "--no-build",
            resolution.requested_symbol,
            "--json",
            "--color",
            "never",
        ]
        comparison_mode = "asm"
    result = _run(command, capture=True)
    if result.returncode:
        raise PreflightError(f"wb_compare failed with exit {result.returncode}")
    try:
        payload = json.loads(result.stdout)
    except json.JSONDecodeError as error:
        raise PreflightError("wb_compare did not return one JSON report") from error
    if payload.get("schema") != "decomp-workbench-comparison-v1":
        raise PreflightError(f"unexpected workbench report schema: {payload.get('schema')!r}")
    required = ("words", "target_instructions", "candidate_instructions", "first_divergent_row")
    missing = [key for key in required if key not in payload]
    if missing:
        raise PreflightError("workbench report lacks: " + ", ".join(missing))
    return {
        "comparison_mode": comparison_mode,
        "differing_words": payload["words"],
        "target_words": payload["target_instructions"],
        "candidate_words": payload["candidate_instructions"],
        "matched_words": (
            payload["target_instructions"] - payload["words"]
            if payload["target_instructions"] == payload["candidate_instructions"]
            else None
        ),
        "first_mismatch": (
            None
            if payload["first_divergent_row"] is None
            else f"+0x{4 * payload['first_divergent_row']:X}"
        ),
        "verdict": payload.get("verdict"),
        "target_frame": (
            abs(payload["target_frame_size"])
            if isinstance(payload.get("target_frame_size"), int)
            else None
        ),
        "candidate_frame": (
            abs(payload["candidate_frame_size"])
            if isinstance(payload.get("candidate_frame_size"), int)
            else None
        ),
    }


def _require_tracked_geometry(
    resolution: Resolution, target_value: int, target_size: int
) -> None:
    if resolution.resolution_mode != "post_promotion":
        return
    if resolution.expected_value is None or resolution.expected_size is None:
        raise PreflightError("post-promotion resolution lacks tracked geometry")
    if (target_value, target_size) != (
        resolution.expected_value,
        resolution.expected_size,
    ):
        raise PreflightError(
            "linked geometry disagrees with tracked post-promotion evidence: "
            f"linked={target_value:#x}+{target_size:#x}, "
            f"tracked={resolution.expected_value:#x}+{resolution.expected_size:#x}"
        )


def _original_static_surface(comparison):
    """Keep configured counts literal; select explicitly authenticated raw evidence."""
    if comparison.get("declared_metadata_proof") is not None:
        return comparison["original_raw_comparison"]
    return comparison


def _require_static_relocation_evidence(
    resolution: Resolution, comparison: dict[str, object]
) -> None:
    """Apply the relocation proof available in each resolution mode.

    A fallback candidate must resolve every static identity before it can be
    compared with the extracted target. After promotion, the fallback no
    longer exists and overlay proxy names need not all resolve; the canonical
    object can still prove the complete relocation shape while the relocated
    ROM oracle proves the final bytes. This is the same limitation reported by
    the post-promotion human/JSON output, not permission for a candidate lane.
    """
    comparison = _original_static_surface(comparison)
    candidate_count = int(comparison["candidate_record_count"])
    resolved_count = int(comparison["candidate_identity_resolved_count"])
    if resolution.resolution_mode != "post_promotion":
        if resolved_count != candidate_count:
            raise PreflightError(
                "candidate static relocation identity is unresolved at "
                f"{candidate_count - resolved_count} site(s)"
            )
        return

    target_count = int(comparison["target_record_count"])
    if candidate_count != target_count or comparison.get("offset_type_exact") is not True:
        raise PreflightError(
            "post-promotion static relocation shape disagrees with the tracked target: "
            f"candidate={candidate_count}, target={target_count}, "
            f"offset/type exact={comparison.get('offset_type_exact')!r}"
        )


def _preflight_evidence_status(
    resolution: Resolution, comparison: dict[str, object]
) -> dict[str, object]:
    """Summarize complete or partial evidence without inventing identities."""

    configured_comparison = comparison
    comparison = _original_static_surface(comparison)
    candidate_count = int(comparison["candidate_record_count"])
    resolved_count = int(comparison["candidate_identity_resolved_count"])
    target_count = int(comparison["target_record_count"])
    if not 0 <= resolved_count <= candidate_count:
        raise PreflightError(
            "candidate static relocation identity counts are inconsistent: "
            f"resolved={resolved_count}, candidate={candidate_count}"
        )
    unresolved_count = candidate_count - resolved_count
    raw_sites = comparison.get("candidate_identity_unresolved_records")
    if not isinstance(raw_sites, list):
        raise PreflightError(
            "relocation comparison lacks candidate unresolved-identity diagnostics"
        )
    sites: list[dict[str, object]] = []
    for row in raw_sites:
        if not isinstance(row, dict):
            raise PreflightError("candidate unresolved-identity diagnostic is malformed")
        offset = row.get("offset")
        rtype = row.get("rtype")
        if (
            not isinstance(offset, int)
            or isinstance(offset, bool)
            or offset < 0
            or not isinstance(rtype, int)
            or isinstance(rtype, bool)
        ):
            raise PreflightError("candidate unresolved-identity diagnostic is malformed")
        sites.append(
            {
                "offset": f"+0x{offset:X}",
                "type": TYPE_NAMES.get(rtype, str(rtype)),
            }
        )
    if len(sites) != unresolved_count:
        raise PreflightError(
            "candidate unresolved-identity diagnostics disagree with the count: "
            f"diagnostics={len(sites)}, unresolved={unresolved_count}"
        )

    # Post-promotion candidates retain the existing stronger shape gate. Their
    # unresolved static names are complete only when the linked ROM and retail
    # runtime table have already supplied exact effective identities.
    if resolution.resolution_mode == "post_promotion" or unresolved_count == 0:
        _require_static_relocation_evidence(resolution, comparison)
    effective_complete = (
        resolution.resolution_mode == "post_promotion"
        and comparison.get("effective_identity_exact") is True
    )
    partial = unresolved_count > 0 and not effective_complete
    counts = {
        "target_relocations": target_count,
        "candidate_static_relocations": candidate_count,
        "offset_type_aligned": int(comparison["offset_type_alignment_count"]),
        "stable_identities_aligned": int(
            comparison["stable_identity_alignment_count"]
        ),
        "effective_identities_aligned": int(
            comparison.get(
                "effective_identity_alignment_count",
                comparison["stable_identity_alignment_count"],
            )
        ),
        "candidate_identities_resolved": resolved_count,
        "candidate_identities_unresolved": unresolved_count,
    }
    diagnostics: list[dict[str, object]] = []
    if configured_comparison.get("declared_metadata_proof") is not None:
        counts["candidate_static_relocations"] = configured_comparison["candidate_record_count"]
        counts["original_raw_static_relocations"] = candidate_count
        counts["declared_removed_relocations"] = len(configured_comparison["declared_metadata_proof"]["filtered"])
    if unresolved_count:
        diagnostics.append(
            {
                "code": "candidate_static_relocation_identity_unresolved",
                "severity": "error" if partial else "warning",
                "count": unresolved_count,
                "sites": sites,
                "message": (
                    "candidate static relocation runtime identity is incomplete; "
                    "no identity was inferred"
                    if partial
                    else "static names remain unresolved; exact post-promotion "
                    "linked/runtime evidence supplies the effective identities"
                ),
            }
        )
    if partial:
        action = (
            "restore_linked_runtime_identity_proof"
            if resolution.resolution_mode == "post_promotion"
            else "resolve_candidate_static_relocation_identities"
        )
        status = "partial"
    else:
        action = (
            "run_promotion_proof"
            if resolution.resolution_mode == "post_promotion"
            else "continue_matching"
        )
        status = "complete"
    return {
        "status": status,
        "action": action,
        "counts": counts,
        "diagnostics": diagnostics,
    }


def _augment_runtime_identity_evidence(
    resolution: Resolution,
    comparison: dict[str, object],
    workbench: dict[str, object],
) -> dict[str, object]:
    """Separate compile-time names from identities proved after promotion.

    Overlay proxy names can intentionally collapse several runtime identities
    onto one link value.  Once a promoted function is instruction-exact in the
    linked ROM and its relocation offsets/types are exact, the unchanged retail
    runtime table proves those remaining identities even though the ordinary
    object cannot spell them.  Keep that evidence distinct from static symbol
    resolution instead of either under-counting the proof or pretending the
    object names were more informative than they are.
    """
    configured_comparison = comparison
    comparison = dict(_original_static_surface(comparison))
    if configured_comparison.get("declared_metadata_proof") is not None:
        comparison["declared_metadata_proof"] = configured_comparison["declared_metadata_proof"]
    target_count = int(comparison["target_record_count"])
    linked_exact = (
        resolution.resolution_mode == "post_promotion"
        and comparison.get("offset_type_exact") is True
        and int(comparison["candidate_record_count"]) == target_count
        and workbench.get("differing_words") == 0
        and workbench.get("target_words") == workbench.get("candidate_words")
    )
    result = ri.augment_effective_identity(comparison, linked_exact=linked_exact)
    if configured_comparison.get("declared_metadata_proof") is None:
        return result
    result.pop("declared_metadata_proof")
    return dict(configured_comparison, original_raw_comparison=result,
                identity_proof_mode=result["identity_proof_mode"],
                effective_identity_exact=result["effective_identity_exact"],
                effective_identity_alignment_count=result["effective_identity_alignment_count"])


def _candidate_redefine_aliases(candidate_object: Path) -> dict[str, str]:
    """Map postprocessed object names back to their compile-time identities."""
    target = _relative(candidate_object)
    command = pa.postprocess_commands(pa.run_make_database()).get(target)
    if command is None:
        return {}
    try:
        pairs = ri.parse_objcopy_redefine_pairs(command)
        closure = ri.canonicalize_redefine_aliases(pairs)
    except ri.RelocationIdentityError as error:
        raise PreflightError(
            f"cannot parse candidate objcopy aliases for {target}: {error}"
        ) from error
    if closure.cycles:
        rendered = ", ".join(" -> ".join(cycle) for cycle in closure.cycles)
        raise PreflightError(
            f"candidate objcopy aliases for {target} contain a cycle: {rendered}"
        )
    # Conflicting-source destinations are deliberately absent. Cycles are
    # rejected above; conflicts reach the ordinary unresolved-identity gate
    # below instead of being assigned guessed provenance.
    return closure.resolved


def _signed_hex(value: int) -> str:
    sign = "+" if value >= 0 else "-"
    return f"{sign}0x{abs(value):X}"


def _tracked_overlay_alias_identities(
    symbol: str, alias_path: Path = ALIASES
) -> list[tuple[int, int]]:
    """Return every generated overlay identity assigned to one friendly name."""

    forward, _reverse = _aliases(alias_path)
    identities: set[tuple[int, int]] = set()
    for generated, friendly in forward.items():
        if symbol not in (generated, friendly):
            continue
        match = rs.GEN_NAME_RE.match(generated)
        if match:
            identities.add((int(match.group(1)), int(match.group(2), 16)))
    return sorted(identities)


def _surface_error_diagnostic(
    error: rs.SurfaceComparisonError,
    resolution: Resolution,
    atlas: dict[str, object],
    target_value: int,
    target_size: int,
    *,
    alias_path: Path = ALIASES,
) -> str:
    """Explain consolidated-TU failures without weakening the closed gate.

    ``reloc_surface`` deliberately stops at the first ownership or identity
    contradiction.  Its terse errors are appropriate for a library, but they
    hide the common cause in a consolidated candidate object: earlier guarded
    functions changed size, shifting every later definition.  Add the two
    geometries here while preserving the original failure and verdict.
    """

    original = str(error)
    generated = rs.GEN_NAME_RE.match(resolution.target_symbol)
    if not generated:
        return original

    try:
        owner = rs.resolve_overlay_ownership(
            resolution.candidate_object,
            atlas,
            overlay_hint=int(generated.group(1)),
            source=resolution.translation_unit,
        )
        if owner is None:
            return original
        module, row = owner
        overlay = int(module["overlay"])
        row_start = int(row["offset"], 16)
        row_end = int(row["end_offset"], 16)
        candidate_elf = rs.Elf(resolution.candidate_object)
        candidate_start, candidate_size, _section = rs._unique_symbol(
            candidate_elf, resolution.candidate_symbol, require_text=True
        )
        target_start = target_value - rs.SYNTHETIC_VMA
        target_local_start = target_start - row_start
        candidate_end = candidate_start + candidate_size
    except (OSError, KeyError, TypeError, ValueError, rs.SurfaceComparisonError):
        return original

    owner_size = row_end - row_start
    prefix_drift = candidate_start - target_local_start
    owner_text = (
        f"overlay:{overlay}:+0x{row_start:X}..+0x{row_end:X} "
        f"({_relative(resolution.source)})"
    )

    if original == "candidate function escapes TU ownership":
        overrun = max(0, candidate_end - owner_size)
        return (
            f"{original}: consolidated candidate {resolution.candidate_symbol} occupies "
            f"TU .text+0x{candidate_start:X}..+0x{candidate_end:X}, while the linked "
            f"target belongs at TU+0x{target_local_start:X}.."
            f"+0x{target_local_start + target_size:X} inside {owner_text}; "
            f"preceding candidate code shifted this function by {_signed_hex(prefix_drift)}"
            f" and the candidate exceeds the owner by 0x{overrun:X}. Repair or isolate "
            "the earlier size drift before relocation comparison; preflight will not "
            "reinterpret bytes outside the atlas owner"
        )

    ambiguous = re.fullmatch(
        r"candidate relocation symbol ([A-Za-z_]\w*) has ambiguous runtime identity",
        original,
    )
    if ambiguous:
        symbol = ambiguous.group(1)
        try:
            aliases = _tracked_overlay_alias_identities(symbol, alias_path)
            text_index, _text = candidate_elf.section(".text")
            definitions = {
                (overlay, row_start + value, value)
                for name, value, _size, _info, shndx in candidate_elf.symbols()
                if name == symbol and shndx == text_index
            }
        except (OSError, ValueError, rs.SurfaceComparisonError):
            aliases = []
            definitions = set()
        if len(aliases) == 1 and len(definitions) == 1:
            alias_overlay, alias_offset = aliases[0]
            compiled_overlay, compiled_offset, object_offset = next(iter(definitions))
            disagreement = compiled_offset - alias_offset
            return (
                f"{original}: tracked alias identity is overlay:{alias_overlay}:"
                f"+0x{alias_offset:X}, but the same definition in consolidated candidate "
                f"{resolution.translation_unit} lands at overlay:{compiled_overlay}:"
                f"+0x{compiled_offset:X} (TU .text+0x{object_offset:X}, disagreement "
                f"{_signed_hex(disagreement)}). This is candidate prefix-layout drift, "
                "not a second target identity; repair earlier shared-TU sizes before "
                "preflight can authenticate relocations"
            )
        return (
            f"{original} in consolidated candidate {resolution.translation_unit}; "
            f"the target range remains uniquely bounded by {owner_text}, but candidate "
            "symbol provenance does not select one runtime identity. Preflight remains "
            "closed; use wb_compare.sh only for scalar source-shape diagnostics"
        )

    return original


def _declared_filter_comparison(resolution, comparison, context, records, target_elf, linked_name, input_snapshot):
    """Compose fresh raw proof with exact declared metadata removal, never missing guessed sites."""
    if (resolution.resolution_mode != "post_promotion" or context["kind"] != "overlay"
            or comparison.get("offset_type_exact") is True):
        return comparison, None
    command = pa.postprocess_commands(pa.run_make_database()).get(_relative(resolution.candidate_object), "")
    if "filter_elf_relocations.py" not in command:
        return comparison, None
    _check_filter_implementations()
    try:
        raw_path, plan, receipt, current_context = pp.capture_configured_raw(
            REPO, resolution.source, resolution.candidate_object, TARGET_ELF, command)
        _check_filter_implementations()
        receipt["proof_implementations"] = dict(_LOADED_FILTER_IMPLEMENTATIONS)
        _require_capture_snapshot(receipt, input_snapshot, target_elf.data)
        raw, configured = rs.Elf(raw_path), rs.Elf(resolution.candidate_object)
        accounting = pp.validate_metadata_objects(raw, configured, plan, resolution.candidate_symbol)
        raw_comparison = rs.function_surface_comparison(
            resolution.requested_symbol, raw_path, TARGET_ELF, rom_path=ROM, atlas_path=ATLAS,
            values_path=ALIASES, candidate_symbol=resolution.candidate_symbol,
            target_symbol=linked_name, source=resolution.translation_unit,
            measure_size_delta=resolution.resolution_mode != "post_promotion")
        total = len(records)
        removed = {(row["offset"], row["rtype"]) for row in accounting["filtered"]}
        unresolved = {(row["offset"], row["rtype"]) for row in raw_comparison["candidate_identity_unresolved_records"]}
        if (raw_comparison["candidate_record_count"] != total or not raw_comparison["offset_type_exact"]
                or not unresolved <= removed or raw_comparison["stable_identity_alignment_count"] != total - len(unresolved)
                or accounting["retained_count"] != comparison["candidate_record_count"]):
            raise pp.MetadataProofError("raw compiler relocation surface is not exact outside declared filters")
        # Two independent routes prove a filtered site's identity:
        #   * a named symbol the raw surface resolved: the raw comparison
        #     above already aligned it with the shipped tuple (every resolved
        #     raw record agrees, since the alignment count equals the resolved
        #     count), exactly as for any unfiltered site;
        #   * the anonymous `.bss` section symbol, which the raw surface
        #     cannot resolve: its base is derived from the linked BSS
        #     definitions below.
        # A filtered site proved by neither is refused.
        bss_keys = {(row["offset"], row["rtype"]) for row in accounting["filtered"]
                    if row["symbol"] == ".bss"}
        if any(row["rtype"] not in (5, 6) for row in accounting["filtered"]
               if row["symbol"] == ".bss"):
            raise pp.MetadataProofError("unsupported filtered runtime identity; only canonical BSS HI/LO is accounted")
        if (removed - bss_keys) & unresolved:
            raise pp.MetadataProofError(
                "unsupported filtered runtime identity; a named filtered site the raw "
                "surface cannot resolve has no independent identity")
        expected = {(row.offset, row.rtype): row.identity for row in records}
        proved = {key: expected.get(key) for key in removed - bss_keys}
        routes = {key: "raw-static" for key in removed - bss_keys}
        if bss_keys:
            overlay = context["overlay"]
            module = ot.build_modules(ot.read_headers(ROM.read_bytes()))[overlay - 1]
            identity = pp.linked_bss_base(raw, configured, target_elf, overlay,
                                         module["text_size"] + module["data_size"], module["bss_size"])
            bss_records = rs._candidate_surface_records(raw, accounting["start"], accounting["size"], records,
                                                       {".bss": identity}, {}, set(), overlay)
            bss_proved = {(row.offset, row.rtype): row.identity for row in bss_records}
            proved.update({key: bss_proved.get(key) for key in bss_keys})
            routes.update({key: "linked-bss-base" for key in bss_keys})
        if any(proved.get(key) is None or proved.get(key) != expected.get(key) for key in removed):
            raise pp.MetadataProofError("filtered raw addend disagrees with canonical/runtime identity")
        if receipt["inputs"] != current_context() or receipt["raw_sha256"] != pp.sha256_file(raw_path):
            raise pp.MetadataProofError("raw proof inputs or object changed during validation")
        receipt.update(accounting)
        receipt["filtered_identities"] = [{"offset": offset, "rtype": kind, "identity": proved[(offset, kind)],
                                            "route": routes[(offset, kind)]}
                                           for offset, kind in sorted(removed)]
        receipt["source_selection"] = pp.classify_source_selection(
            resolution.source.read_text(), candidate_symbol=resolution.candidate_symbol,
            target_symbol=resolution.target_symbol,
            defines={token[2:].split("=", 1)[0] for token in shlex.split(receipt["inputs"]["command"])
                     if token.startswith("-D") and len(token) > 2})[0]
        if receipt["source_selection"] != pp.ORDINARY_C:
            raise pp.MetadataProofError("raw proof does not select ordinary C")
        (REPO / receipt["directory"] / "proof.json").write_text(json.dumps(receipt, sort_keys=True, indent=2))
        raw_comparison.update(candidate_identity_resolved_count=total,
                              candidate_identity_unresolved_records=[], stable_identity_alignment_count=total,
                              stable_identity_exact=True,
                              candidate_surface_source="fresh-raw-before-declared-metadata")
        return dict(comparison, original_raw_comparison=raw_comparison,
                    declared_metadata_proof=receipt), (receipt, current_context)
    except (pp.MetadataProofError, OSError, ValueError, RuntimeError) as error:
        raise PreflightError("declared metadata accounting failed: " + str(error)) from error


def collect(resolution: Resolution, *, no_build: bool = False) -> dict[str, object]:
    for path, label in (
        (TARGET_ELF, "canonical linked ELF"),
        (resolution.candidate_object, "candidate object"),
        (ROM, "baserom"),
        (ATLAS, "overlay atlas"),
    ):
        if not path.is_file():
            raise PreflightError(f"missing {label}: {_relative(path)}")

    input_snapshot = {"linked": pp.sha256_file(TARGET_ELF),
                      "target_inputs": {name: pp.sha256_file(REPO / name) for name in
                        ("baseroms/mickey.us.z64", "config/overlays.us.json",
                         "overlay_undefined_syms.us.txt", "symbol_addrs.us.txt")}}
    target_elf = rs.Elf(TARGET_ELF)
    linked_name, target_value, target_size, section = _symbol_geometry(
        target_elf, (resolution.candidate_symbol, resolution.target_symbol)
    )
    _require_tracked_geometry(resolution, target_value, target_size)
    atlas_bytes = ATLAS.read_bytes()
    atlas = json.loads(atlas_bytes)
    rom = ROM.read_bytes()
    if (hashlib.sha256(target_elf.data).hexdigest() != input_snapshot["linked"]
            or hashlib.sha256(atlas_bytes).hexdigest() != input_snapshot["target_inputs"]["config/overlays.us.json"]
            or hashlib.sha256(rom).hexdigest() != input_snapshot["target_inputs"]["baseroms/mickey.us.z64"]):
        raise PreflightError("linked/runtime ownership changed while loading proof inputs")
    context, runtime_records = _target_context(
        resolution, target_value, target_size, atlas, rom
    )
    if context["kind"] == "resident":
        context.update(
            _resident_boundary(target_elf, target_value, target_size, section)
        )
    candidate_redefine_aliases = _candidate_redefine_aliases(
        resolution.candidate_object
    )
    try:
        # A candidate longer than its target is measured (Track B); only a
        # post-promotion resolution keeps the strict TU-ownership bound.
        comparison = rs.function_surface_comparison(
            resolution.requested_symbol,
            resolution.candidate_object,
            TARGET_ELF,
            rom_path=ROM,
            atlas_path=ATLAS,
            values_path=ALIASES,
            candidate_symbol=resolution.candidate_symbol,
            target_symbol=linked_name,
            source=resolution.translation_unit,
            candidate_redefine_aliases=candidate_redefine_aliases,
            measure_size_delta=resolution.resolution_mode != "post_promotion",
        )
    except rs.SurfaceComparisonError as error:
        raise PreflightError(
            _surface_error_diagnostic(
                error, resolution, atlas, target_value, target_size
            )
        ) from error
    comparison, filter_binding = _declared_filter_comparison(
        resolution, comparison, context, runtime_records, target_elf, linked_name, input_snapshot)
    relocation_evidence = _relocation_evidence(
        resolution,
        context,
        runtime_records,
        target_elf,
        linked_name,
        target_value,
        target_size,
        section,
    )
    inbound = _inbound_references(context, rom)
    workbench = _workbench(resolution, no_build=no_build)
    if filter_binding is not None:
        receipt, current_context = filter_binding
        _check_filter_implementations()
        if (pp.sha256_file(Path(__file__)) != _LOADED_FILTER_PROOF
                or receipt["inputs"] != current_context()
                or receipt["raw_sha256"] != pp.sha256_file(REPO / receipt["raw_object"])):
            raise PreflightError("declared metadata proof became stale before linked comparison")
    comparison = _augment_runtime_identity_evidence(
        resolution, comparison, workbench
    )
    preflight = _preflight_evidence_status(resolution, comparison)
    try:
        source_history = [
            dataclasses.asdict(row)
            for row in fh.guarded_body_history(
                resolution.source, resolution.candidate_symbol, root=REPO
            )
        ]
        source_history_status = "ok"
    except fh.HistoryError as error:
        source_history = []
        source_history_status = f"unavailable: {error}"

    result: dict[str, object] = {
        "schema": "mickey-function-evidence-preflight-v1",
        "requested_symbol": resolution.requested_symbol,
        "target_symbol": resolution.target_symbol,
        "candidate_symbol": resolution.candidate_symbol,
        "source": _relative(resolution.source),
        "translation_unit": resolution.translation_unit,
        "candidate_build_dir": resolution.candidate_build_dir,
        "candidate_object": _relative(resolution.candidate_object),
        "candidate_selection": resolution.selection,
        "resolution_mode": resolution.resolution_mode,
        "identity_evidence": resolution.identity_evidence,
        "candidate_signature": _source_signature(
            resolution.source, resolution.candidate_symbol
        ),
        "source_history": source_history,
        "source_history_status": source_history_status,
        "linked_symbol": linked_name,
        "linked_section": section,
        "owned_size": target_size,
        "context": context,
        "exports": context["exports"],
        "callers": [row for row in inbound if row["type"] == "R_MIPS_26"],
        "inbound_references": inbound,
        # Compatibility for existing JSON consumers. New consumers should use
        # the explicitly named static/runtime fields below.
        "runtime_relocations": _relocation_rows(runtime_records),
        **relocation_evidence,
        "relocation_comparison": comparison,
        "workbench": workbench,
        "preflight": preflight,
    }
    return result


def _render_preflight_status(preflight: dict[str, object]) -> None:
    status = preflight["status"]
    suffix = " (non-exact)" if status == "partial" else ""
    print(f"status: {status}{suffix}")
    print(f"action: {preflight['action']}")
    counts = preflight["counts"]
    print(
        "preflight relocation counts: "
        f"target={counts['target_relocations']} "
        f"candidate={counts['candidate_static_relocations']} "
        f"offset/type={counts['offset_type_aligned']} "
        f"resolved={counts['candidate_identities_resolved']} "
        f"unresolved={counts['candidate_identities_unresolved']}"
    )
    for diagnostic in preflight["diagnostics"]:
        print(
            f"diagnostic [{diagnostic['severity']}] {diagnostic['code']}: "
            f"{diagnostic['message']}"
        )
        for site in diagnostic["sites"]:
            print(f"  {site['offset']} {site['type']} identity=unresolved")


def _render_human(report: dict[str, object]) -> None:
    def print_relocation(row: dict[str, object]) -> None:
        addend = row["addend"]
        suffix = "" if addend is None else f" addend={addend:+#x}"
        print(
            f"  {row['offset']} {row['type']} identity={row['identity']}"
            f"{suffix}"
        )

    context = report["context"]
    print(f"identity: {report['target_symbol']} -> {report['candidate_symbol']}")
    print(
        f"resolution: {report['resolution_mode']} "
        f"({report['identity_evidence']})"
    )
    _render_preflight_status(report["preflight"])
    print(
        f"candidate: {report['source']} [{report['candidate_build_dir']}] "
        f"{report['candidate_signature']}"
    )
    history = report.get("source_history", [])
    history_status = report.get("source_history_status", "not collected")
    print(f"source history (guarded-body changes): {history_status}")
    if history:
        for row in history:
            print(f"  {str(row['commit'])[:10]} {row['subject']}")
    else:
        print("  none")
    if context["kind"] == "overlay":
        print(
            f"owned range: overlay:{context['overlay']}:+0x{context['offset']:X}.."
            f"+0x{context['offset'] + report['owned_size']:X} "
            f"(ROM 0x{context['rom_start']:X}..0x{context['rom_end']:X})"
        )
        owner = context["owner"]
        next_owner = context["next_owner"]
        if next_owner:
            print(
                f"boundary: owner ends +0x{context['owner_end']:X}; next "
                f"{next_owner['type']} range {next_owner['source']} starts "
                f"+0x{int(next_owner['offset'], 16):X}"
            )
        else:
            print(f"boundary: owner ends +0x{context['owner_end']:X}; end of overlay text")
    else:
        print(
            f"owned range: resident 0x{context['vram_start']:X}..0x{context['vram_end']:X} "
            f"(ROM 0x{context['rom_start']:X}..0x{context['rom_end']:X})"
        )
        if context["next_symbol"] is None:
            print("boundary: no following resident function in the linked section")
        else:
            print(
                f"boundary: next {context['next_symbol']} starts "
                f"0x{context['next_start']:X}; padding=0x{context['padding_size']:X}"
            )

    exports = report["exports"]
    print("exports: " + (", ".join(str(row["rom_table_index"]) for row in exports) or "none"))
    callers = report["callers"]
    print(f"callers: {len(callers)}")
    for row in callers:
        print(f"  {row['source']} {row['site']} {row['type']}")
    noncall = [row for row in report["inbound_references"] if row["type"] != "R_MIPS_26"]
    if noncall:
        print(f"other inbound references: {len(noncall)}")
        for row in noncall:
            print(f"  {row['source']} {row['site']} {row['type']}")
    if context["kind"] == "overlay":
        tuples = report["runtime_overlay_records"]
        print(f"runtime overlay records: {len(tuples)}")
        for row in tuples:
            print_relocation(row)
    else:
        tuples = report["target_static_relocations"]
        print(f"target static relocations: {len(tuples)}")
        for row in tuples:
            print_relocation(row)
        resident_runtime = report["resident_runtime_records"]
        print(
            f"resident runtime records: {len(resident_runtime)} "
            "(sparse startup table; zero is valid)"
        )
        print("runtime overlay records: not applicable (resident function)")
    reloc = report["relocation_comparison"]
    print(
        "candidate static relocation surface: "
        f"target={reloc['target_surface_source']} "
        f"offset/type={reloc['offset_type_alignment_count']}/"
        f"{reloc['target_record_count']} identity="
        f"{reloc['stable_identity_alignment_count']}/"
        f"{reloc['target_record_count']} static + "
        f"{reloc.get('linked_runtime_identity_alignment_count', 0)} "
        "linked-runtime = "
        f"{reloc.get('effective_identity_alignment_count', reloc['stable_identity_alignment_count'])}/"
        f"{reloc['target_record_count']} effective"
    )
    wb = report["workbench"]
    matched = "n/a" if wb["matched_words"] is None else f"{wb['matched_words']}/{wb['target_words']}"
    target_frame = "n/a" if wb["target_frame"] is None else f"0x{wb['target_frame']:X}"
    candidate_frame = (
        "n/a" if wb["candidate_frame"] is None else f"0x{wb['candidate_frame']:X}"
    )
    print(
        f"workbench [{wb['comparison_mode']}]: matched={matched} "
        f"differing={wb['differing_words']} "
        f"target={wb['target_words']} candidate={wb['candidate_words']} "
        f"first={wb['first_mismatch'] or 'none'} verdict={wb['verdict']} "
        f"frame={target_frame}/{candidate_frame}"
    )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol", nargs="?")
    parser.add_argument("--json", action="store_true")
    parser.add_argument(
        "--compose-relocation-summaries",
        nargs=2,
        type=Path,
        metavar=("FALLBACK_JSON", "PROMOTED_JSON"),
        help=(
            "compose fresh fallback-static and promoted-linked summary proofs; "
            "both inputs must be under build/"
        ),
    )
    parser.add_argument(
        "--no-build",
        action="store_true",
        help="require existing canonical and candidate artifacts instead of updating them",
    )
    parser.add_argument(
        "--analysis-only",
        action="store_true",
        help=(
            "return success for structured partial evidence; status remains "
            "partial and non-exact"
        ),
    )
    resolution_mode = parser.add_mutually_exclusive_group()
    resolution_mode.add_argument(
        "--resolve-wb",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    resolution_mode.add_argument(
        "--resolve-rom",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    args = parser.parse_args(argv)
    try:
        if args.compose_relocation_summaries is not None:
            if args.symbol is not None or args.no_build or args.analysis_only:
                raise PreflightError(
                    "relocation-summary composition does not accept a symbol, "
                    "--no-build, or --analysis-only"
                )
            report = compose_relocation_summaries(
                args.compose_relocation_summaries[0],
                args.compose_relocation_summaries[1],
            )
            if args.json:
                print(json.dumps(report, indent=2, sort_keys=True))
            else:
                _render_workbench_summary_human(report)
            return 0
        if args.symbol is None:
            raise PreflightError(
                "provide one symbol or --compose-relocation-summaries"
            )
        if (args.resolve_wb or args.resolve_rom) and args.analysis_only:
            raise PreflightError(
                "--analysis-only is not valid with an internal resolution mode"
            )
        resolution = resolve(args.symbol)
        if args.resolve_wb:
            if resolution.target_asm is None:
                raise PreflightError(
                    f"{resolution.candidate_symbol} is already promoted and has no "
                    "GLOBAL_ASM target; use `tools/wb_compare.sh --rom "
                    f"{resolution.candidate_symbol}`"
                )
            if not args.no_build:
                _build_wb_evidence(resolution)
                resolution = resolve(args.symbol)
            _require_fresh_wb_evidence(resolution)
            fields = (
                resolution.target_symbol,
                resolution.candidate_symbol,
                _relative(resolution.source),
                resolution.translation_unit,
                resolution.candidate_build_dir,
                _relative(resolution.target_asm),
            )
            if any("\t" in field or "\n" in field for field in fields):
                raise PreflightError("resolved paths cannot be represented safely")
            print("\t".join(fields))
            return 0
        if args.resolve_rom:
            if not args.no_build:
                _build_linked_boundary()
            _require_fresh_linked_boundary()
            if not args.no_build:
                # Resolution can authenticate linked container bytes, aliases,
                # and section boundaries. A rebuild invalidates that evidence
                # even when the selected function keeps its value and size.
                resolution = resolve(args.symbol)
            boundary = _linked_boundary(resolution)
            fields = (
                str(boundary["linked_symbol"]),
                f"{int(boundary['value']):X}",
                f"{int(boundary['size']):X}",
                str(boundary["section"]),
                str(boundary["evidence"]),
            )
            if any("\t" in field or "\n" in field for field in fields):
                raise PreflightError("resolved boundary cannot be represented safely")
            print("\t".join(fields))
            return 0
        if not args.no_build:
            _build(resolution)
            resolution = resolve(args.symbol)
        require_fresh_evidence(resolution)
        report = collect(resolution, no_build=args.no_build)
    except (OSError, ValueError, rs.SurfaceComparisonError, PreflightError) as error:
        parser.error(str(error))
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        _render_human(report)
    if report["preflight"]["status"] == "partial" and not args.analysis_only:
        return PARTIAL_EVIDENCE_EXIT
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
