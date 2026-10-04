#!/usr/bin/env python3
"""
flag_sweep.py -- compile one translation unit under the whole lattice of
compiler flag groups this project has ever needed, and rank the results by
closeness to the target bytes. Run this BEFORE any hand permutation.

Usage:
    tools/flag_sweep.py <tu.c> --function NAME [options]

Where NAME is the symbol as it appears in the *compiled candidate* object
(the C name, e.g. "ProcessRelocationEntry"). If the ROM/asm side uses a
different name -- true for any not-yet-matched function, which splat names
func_<VRAM> until it is matched -- pass that name separately with
--target-symbol.

Target bytes, in order tried (first that resolves wins; --target-asm forces
the first branch):
  1. --target-asm PATH            : assemble this .s file directly.
  2. asm/nonmatchings/**/<target-symbol>.s : the function is still
     `#pragma GLOBAL_ASM`, or under `#ifdef NON_MATCHING`, and splat still
     emits its disassembly. Assembled exactly as tools/wb_compare.sh does.
  3. <target-symbol> in build/mickey.us.elf : the function is already
     matched. Bytes come from the baserom at the symbol's ROM offset (its
     size preferring symbol_addrs.us.txt, same fallback wb_compare.sh uses).

The named function's extent must also resolve uniquely through the canonical
overlay atlas or resident symbol table. Assembly mode extracts that ELF symbol,
never the fallback file's whole section, so post-function padding is excluded.

Every candidate is an *unlinked* object, so words carrying a relocation
(a `jal`, a `%hi`/`%lo` pair, ...) do not contain final addresses and cannot
be compared literally. Both sides are masked at those words before scoring;
see score_words() and docs/flag-sweep.md for exactly what that does and does
not prove.

Candidate objects and logs are retained in a content-addressed cache under
build/flag_sweep/. ``--rescore`` requires a complete cache and never invokes
the compiler. Nothing here writes ROM-derived bytes into a tracked file, and
instruction words never reach this file, argv, or stdout as text -- only
counts and offsets do.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import contextlib
import dataclasses
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

import permute_batch as pb

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS_DIR = REPO_ROOT / "tools"
BINUTILS = TOOLS_DIR / "binutils"
AS = BINUTILS / "mips64-elf-as"
OBJDUMP = BINUTILS / "mips64-elf-objdump"
OBJCOPY = BINUTILS / "mips64-elf-objcopy"
IDO_CC = TOOLS_DIR / "ido" / "cc"
IDO_PHASES = TOOLS_DIR / "ido-phases.py"
ASM_PROCESSOR_BUILD = TOOLS_DIR / "asm-processor" / "build.py"
OBJDIFF_CLI = TOOLS_DIR / "objdiff" / "objdiff-cli"
OVERLAY_ATLAS = REPO_ROOT / "config" / "overlays.us.json"
CACHE_SCHEMA = 3


def repo_cli_path(path: Path) -> Path:
    """Resolve a CLI path against the repository root, not the caller's CWD."""
    if path.is_absolute():
        return path.resolve()
    return (REPO_ROOT / path).resolve()


def display_path(path: Path) -> str:
    """Prefer a repository-relative path, while allowing external inputs."""
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)

# Target assembly and the diagnostic asm-processor wrapper use these explicit
# assembler options. Compiler arguments come from the configured TU recipe.
ASFLAGS = ["-march=vr4300", "-32", "-mabi=32", "-G0", "-I", "include"]
ASM_PROC_ASFLAGS = ASFLAGS + ["include/asm_processor_prelude.inc"]

# ---------------------------------------------------------------------------
# The lattice. Extend by adding rows -- everything downstream (pruning,
# compiling, ranking) reads this table, nothing is hardcoded per-combo.
# ---------------------------------------------------------------------------

# (name, OPT_FLAGS tokens)
OPT_GROUP: List[Tuple[str, List[str]]] = [
    ("bare", []),
    ("-O0", ["-O0"]),
    ("-O1", ["-O1"]),
    ("-O2", ["-O2"]),
    ("-O3", ["-O3"]),
    ("-g", ["-g"]),
    ("-g3", ["-g3"]),
    ("-O2-g3", ["-O2", "-g3"]),
]

# (name, MIPSISET tokens)
ISA_GROUP: List[Tuple[str, List[str]]] = [
    ("mips1", ["-mips1", "-32"]),
    ("mips2", ["-mips2", "-32"]),
    ("mips3", ["-mips3", "-32"]),
]

# (name, extra CFLAGS tokens, requires_optimization)
# requires_optimization=True means: prune this extra when OPT_GROUP has no
# -O flag at all (bare/-g/-g3) -- a loop-unroll hint has nothing to act on
# without an optimizing pass, and no in-tree use pairs the two.
EXTRAS_GROUP: List[Tuple[str, List[str], bool]] = [
    ("none", [], False),
    ("r4300_mul", ["-Wab,-r4300_mul"], False),
    ("loopunroll0", ["-Wo,-loopunroll,0"], True),
    ("loopunroll2", ["-Wo,-loopunroll,2"], True),
    ("loopunroll4", ["-Wo,-loopunroll,4"], True),
    ("woff835", ["-woff", "835"], False),
]

# Phase-driven variants: these go through tools/ido-phases.py (a drop-in
# `cc` replacement that can run individual IDO phases with per-phase
# overrides) instead of tools/ido/cc directly, and are not crossed against
# ISA_GROUP/EXTRAS_GROUP -- the project has exactly two documented uses of
# this family (Makefile ~515-597) and both pin their own ISA. Add rows here,
# not to the cross product, if a third phase-driven family shows up.
PHASE_VARIANTS = [
    {
        "id": "phase-uopt-O1",
        "opt": ["-O2"],
        "isa": ["-mips2", "-32"],
        "extra": ["-Xphase,uopt,+", "-Xphase,uopt,-O1"],
        "use_ido_phases": True,
        "note": "uopt at -O1, rest at -O2 (setglobalintmask.c's flag group)",
    },
    {
        "id": "phase-all-O3",
        "opt": ["-O2"],
        "isa": ["-mips2", "-32"],
        "extra": [
            "-Xphase,cfe,-O3",
            "-Xphase,uopt,-O3",
            "-Xphase,ugen,-O3",
            "-Xphase,as1,-O3",
        ],
        "use_ido_phases": True,
        "note": "legacy appended -O3 phase options; UOPT retains its -O2 veto",
    },
]


@dataclasses.dataclass(frozen=True)
class Combo:
    id: str
    opt: Tuple[str, ...]
    isa: Tuple[str, ...]
    extra: Tuple[str, ...]
    use_ido_phases: bool = False
    note: str = ""


def build_lattice() -> List[Combo]:
    combos: List[Combo] = []
    for opt_name, opt_tokens in OPT_GROUP:
        # bare/-g/-g3 carry no -On token at all.
        has_opt = any(t in ("-O0", "-O1", "-O2", "-O3") for t in opt_tokens)
        for isa_name, isa_tokens in ISA_GROUP:
            for extra_name, extra_tokens, needs_opt in EXTRAS_GROUP:
                if needs_opt and not has_opt:
                    continue  # pruned: loop-unroll hint with no optimizer pass
                combos.append(
                    Combo(
                        id=f"{opt_name}_{isa_name}_{extra_name}",
                        opt=tuple(opt_tokens),
                        isa=tuple(isa_tokens),
                        extra=tuple(extra_tokens),
                    )
                )
    for pv in PHASE_VARIANTS:
        combos.append(
            Combo(
                id=pv["id"],
                opt=tuple(pv["opt"]),
                isa=tuple(pv["isa"]),
                extra=tuple(pv["extra"]),
                use_ido_phases=True,
                note=pv["note"],
            )
        )
    return combos


# ---------------------------------------------------------------------------
# Compiling
# ---------------------------------------------------------------------------


@dataclasses.dataclass
class CompileResult:
    combo: Combo
    ok: bool
    obj_path: Optional[Path]
    error: str
    elapsed: float


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def lattice_base_arguments(recipe: pb.BuildRecipe) -> List[str]:
    """Replace only the declared lattice axes; preserve other ordered inputs."""
    if not recipe.from_dry_run or not recipe.compiler_args:
        raise LookupError("no complete configured compiler recipe; refusing static fallback")
    result = []
    args = list(recipe.compiler_args)
    index = 0
    while index < len(args):
        token = args[index]
        if token == "-woff" and args[index + 1:index + 2] == ["835"]:
            index += 2
            continue
        if not (re.fullmatch(r"-(?:O[0-3]?|g[0-3]?|mips[123]|32)", token)
                or token == "-Wab,-r4300_mul"
                or re.fullmatch(r"-Wo,-loopunroll,[024]", token)):
            result.append(token)
        index += 1
    return result


def configured_recipe(tu: Path) -> pb.BuildRecipe:
    try:
        recipe = pb.build_recipe_for(tu)
        lattice_base_arguments(recipe)
        return recipe
    except (ValueError, OSError) as error:
        raise LookupError(f"cannot recover configured TU recipe: {error}") from error


def include_directories(args: Sequence[str]) -> List[Path]:
    return [REPO_ROOT / (args[index + 1] if token == "-I" else token[2:])
            for index, token in enumerate(args) if token.startswith("-I")]


def assembly_dependencies(tu: Path) -> dict:
    """Conservatively bind every GLOBAL_ASM input and literal nested include."""
    pending = [REPO_ROOT / match.group("path")
               for match in pb.GLOBAL_ASM_RE.finditer(tu.read_text())]
    seen = {}
    while pending:
        path = pending.pop().resolve()
        if str(path) in seen:
            continue
        seen[str(path)] = _sha256_file(path)
        text = path.read_text()
        if re.search(r'^\s*\.incbin\b', text, re.MULTILINE):
            raise LookupError("binary assembly inputs are not supported by this cache")
        for operand in re.findall(r'^\s*\.include\s+(.+)$', text, re.MULTILINE):
            literal = re.fullmatch(r'"([^"]+)"\s*(?:#.*)?', operand)
            if literal is None:
                raise LookupError(f"cannot bind computed assembly input {operand!r}")
            name = literal.group(1)
            hits = [directory / name for directory in [path.parent, REPO_ROOT, *include_directories(ASFLAGS)]
                    if (directory / name).is_file()]
            if not hits:
                raise LookupError(f"cannot resolve assembly include {name!r}")
            pending.extend(hits)
    return {display_path(Path(path)): digest for path, digest in sorted(seen.items())}


def _include_dependencies(tu: Path, compiler_args: Sequence[str]) -> List[Path]:
    """Conservatively discover textual inputs reachable through #include.

    The compiler runs with ``-nostdinc`` and the recovered search path, so
    recursively following literal includes captures the source-side inputs.
    Missing or computed includes fail closed because their actual dependency
    bytes cannot be bound into a reusable cache key.
    """
    if "-nostdinc" not in compiler_args or any(
        token in {"-include", "-imacros", "-isystem", "-iquote"} for token in compiler_args
    ):
        raise LookupError("unsupported implicit/forced header search; cannot bind compile inputs")
    roots = include_directories(compiler_args)
    pending = [tu.resolve()]
    seen: set[Path] = set()
    include_re = re.compile(r'^\s*#\s*include\s+(.+?)\s*$', re.MULTILINE)
    literal_re = re.compile(r'^[<"]([^>"]+)[>"](?:\s*/[/*].*)?$')
    while pending:
        path = pending.pop()
        if path in seen or not path.is_file():
            continue
        seen.add(path)
        try:
            text = path.read_text(errors="replace")
        except OSError:
            continue
        for operand in include_re.findall(text):
            literal = literal_re.fullmatch(operand)
            if literal is None:
                raise LookupError(
                    f"cannot cache computed include {operand!r} in {display_path(path)}"
                )
            name = literal.group(1)
            candidates = ([path.parent / name] if operand.startswith('"') else [])
            candidates += [root / name for root in roots]
            hit = next((candidate.resolve() for candidate in candidates
                        if candidate.is_file()), None)
            if hit is None:
                raise LookupError(
                    f"cannot resolve include {name!r} from {display_path(path)}"
                )
            if hit not in seen:
                pending.append(hit)
    return sorted(seen, key=str)


def _tool_inputs() -> List[Tuple[str, Path]]:
    """Files whose contents can affect the compiler/assembler pipeline."""
    result: List[Tuple[str, Path]] = []
    for label, root in (
        ("ido", IDO_CC.parent.resolve()),
        ("binutils", AS.parent.resolve()),
        ("asm-processor", ASM_PROCESSOR_BUILD.parent.resolve()),
    ):
        if not root.is_dir():
            continue
        for path in sorted(
            (
                p for p in root.rglob("*")
                if p.is_file()
                and "__pycache__" not in p.parts
                and p.suffix not in {".pyc", ".log"}
            ),
            key=str,
        ):
            result.append((f"{label}/{path.relative_to(root).as_posix()}", path))
    result.append(("ido-phases.py", IDO_PHASES.resolve()))
    result.extend((label, path) for label, path in (
        ("flag_sweep.py", Path(__file__).resolve()),
        ("permute_batch.py", Path(pb.__file__).resolve()),
        ("python", Path(sys.executable).resolve()),
        ("asm-prelude", REPO_ROOT / "include/asm_processor_prelude.inc"),
    ))
    return result


def compilation_cache_identity(
    tu: Path, defines: Sequence[str], lattice: Sequence[Combo],
    recipe: pb.BuildRecipe,
) -> Tuple[str, dict]:
    """Return a content-addressed compile key and its deterministic manifest.

    Scoring-only target assembly, atlas, ELF and baserom are absent. Assembly
    consumed by a TU's GLOBAL_ASM pragmas is a compiler input and is included.
    That separation is what makes an explicit rescore safe and useful.
    """
    source_inputs = []
    for path in _include_dependencies(tu, recipe.compiler_args):
        try:
            label = path.relative_to(REPO_ROOT).as_posix()
        except ValueError:
            label = f"external/{path.name}"
        source_inputs.append({"path": label, "sha256": _sha256_file(path)})
    tool_inputs = [
        {"path": label, "sha256": _sha256_file(path)}
        for label, path in _tool_inputs()
    ]
    recipe = {
        "schema": CACHE_SCHEMA,
        "python": [sys.version_info.major, sys.version_info.minor],
        "tu": display_path(tu),
        "defines": list(defines),
        "configured_compiler_args": list(recipe.compiler_args),
        "base_cflags": lattice_base_arguments(recipe),
        "assembly_inputs": assembly_dependencies(tu),
        "asm_proc_asflags": ASM_PROC_ASFLAGS,
        "lattice": [dataclasses.asdict(combo) for combo in lattice],
        "source_inputs": source_inputs,
        "tool_inputs": tool_inputs,
    }
    encoded = json.dumps(recipe, sort_keys=True, separators=(",", ":")).encode()
    key = hashlib.sha256(encoded).hexdigest()
    return key, {"cache_key": key, **recipe}


def _combo_result_path(outdir: Path) -> Path:
    return outdir / "result.json"


def write_cached_result(result: CompileResult, outdir: Path) -> None:
    outdir.mkdir(parents=True, exist_ok=True)
    object_path = outdir / "out.o"
    log_path = outdir / "compile.log"
    payload = {
        "combo": result.combo.id,
        "ok": result.ok,
        "object": "out.o" if result.ok else None,
        "object_sha256": _sha256_file(object_path) if result.ok else None,
        "log_sha256": _sha256_file(log_path) if log_path.is_file() else None,
        "error": result.error,
    }
    path = _combo_result_path(outdir)
    temporary = path.with_suffix(".json.tmp")
    temporary.write_text(json.dumps(payload, sort_keys=True, indent=2) + "\n")
    temporary.replace(path)


def load_cached_result(combo: Combo, outdir: Path) -> Optional[CompileResult]:
    path = _combo_result_path(outdir)
    if not path.is_file():
        return None
    try:
        payload = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError):
        return None
    if payload.get("combo") != combo.id or not isinstance(payload.get("ok"), bool):
        return None
    if payload["ok"]:
        obj_path = outdir / "out.o"
        if (payload.get("object") != "out.o" or not obj_path.is_file()
                or payload.get("object_sha256") != _sha256_file(obj_path)):
            return None
    else:
        obj_path = None
    log_path = outdir / "compile.log"
    expected_log = payload.get("log_sha256")
    if expected_log is not None and (
        not log_path.is_file() or _sha256_file(log_path) != expected_log
    ):
        return None
    return CompileResult(combo, payload["ok"], obj_path,
                         str(payload.get("error", "")), 0.0)


def collect_compile_results(
    tu: Path,
    lattice: Sequence[Combo],
    cache_root: Path,
    defines: Sequence[str],
    workers: int,
    *,
    rescore: bool,
    compiler_args: Sequence[str] = (),
) -> Tuple[List[CompileResult], int]:
    """Load cached rows and compile only missing rows unless rescoring."""
    results: List[CompileResult] = []
    missing: List[Combo] = []
    for combo in lattice:
        cached = load_cached_result(combo, cache_root / combo.id)
        if cached is None:
            missing.append(combo)
        else:
            results.append(cached)
    if rescore and missing:
        names = ", ".join(combo.id for combo in missing[:3])
        suffix = "..." if len(missing) > 3 else ""
        raise LookupError(
            f"cache is incomplete ({len(missing)} missing: {names}{suffix}); "
            "run once without --rescore"
        )
    if missing:
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            futures = {
                pool.submit(compile_combo, tu, combo, cache_root / combo.id, defines, compiler_args): combo
                for combo in missing
            }
            for future in concurrent.futures.as_completed(futures):
                result = future.result()
                write_cached_result(result, cache_root / result.combo.id)
                results.append(result)
    return results, len(missing)


def compile_combo(
    tu: Path, combo: Combo, outdir: Path, defines: Sequence[str],
    compiler_args: Sequence[str],
) -> CompileResult:
    outdir.mkdir(parents=True, exist_ok=True)
    obj_path = outdir / "out.o"
    log_path = outdir / "compile.log"
    obj_path.unlink(missing_ok=True)

    cc_tokens = (
        [sys.executable, str(IDO_PHASES)] if combo.use_ido_phases else [str(IDO_CC)]
    )
    cmd = (
        [sys.executable, str(ASM_PROCESSOR_BUILD)]
        + cc_tokens
        + ["--", str(AS)]
        + ASM_PROC_ASFLAGS
        + ["--"]
        + list(compiler_args)
        + [f"-D{d}" for d in defines]
        + list(combo.opt)
        + list(combo.isa)
        + list(combo.extra)
        + ["-o", str(obj_path), display_path(tu)]
    )

    start = time.monotonic()
    try:
        proc = subprocess.run(
            cmd,
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            timeout=120,
        )
    except subprocess.TimeoutExpired:
        return CompileResult(combo, False, None, "timed out", time.monotonic() - start)
    elapsed = time.monotonic() - start

    log_path.write_text((proc.stdout or "") + (proc.stderr or ""))

    if proc.returncode != 0 or not obj_path.exists():
        # Keep only the last couple of lines -- IDO errors/warnings are project
        # source, not ROM content, so this is fine to surface, but there is no
        # reason to spray the whole log into the ranked table.
        tail = "\n".join((proc.stderr or proc.stdout or "").strip().splitlines()[-3:])
        return CompileResult(combo, False, None, tail or "compile failed", elapsed)

    return CompileResult(combo, True, obj_path, "", elapsed)


# ---------------------------------------------------------------------------
# Target/candidate byte extraction
# ---------------------------------------------------------------------------

RELOC_MASK = {
    "R_MIPS_26": 0x03FFFFFF,
    "R_MIPS_HI16": 0x0000FFFF,
    "R_MIPS_LO16": 0x0000FFFF,
    "R_MIPS_GPREL16": 0x0000FFFF,
    "R_MIPS_GOT16": 0x0000FFFF,
    "R_MIPS_CALL16": 0x0000FFFF,
    "R_MIPS_16": 0x0000FFFF,
    "R_MIPS_32": 0xFFFFFFFF,
}
DEFAULT_RELOC_MASK = 0x0000FFFF


def _run(cmd: Sequence[str]) -> str:
    return subprocess.run(
        list(map(str, cmd)), cwd=REPO_ROOT, capture_output=True, text=True, check=True
    ).stdout


def elf_symbol(obj_path: Path, name: str) -> Optional[Tuple[int, int, str]]:
    """(offset_or_vram, size, section) for an exact symbol name, or None."""
    out = _run([OBJDUMP, "-t", obj_path])
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 5 or parts[-1] != name:
            continue
        try:
            size = int(parts[-2], 16)
            addr = int(parts[0], 16)
        except ValueError:
            continue
        section = parts[-3]
        return addr, size, section
    return None


def dump_section(obj_path: Path, section: str, out_bin: Path) -> bytes:
    subprocess.run(
        [str(OBJCOPY), "-O", "binary", f"--only-section={section}", str(obj_path), str(out_bin)],
        cwd=REPO_ROOT,
        check=True,
        capture_output=True,
    )
    if not out_bin.exists():
        return b""
    return out_bin.read_bytes()


def words_from_bytes(data: bytes) -> List[int]:
    n = len(data) // 4
    if n == 0:
        return []
    return list(struct.unpack(">%dI" % n, data[: n * 4]))


def section_relocs(obj_path: Path, section: str) -> Dict[int, int]:
    """word-index (relative to `section`'s start) -> bits to mask before compare."""
    out = _run([OBJDUMP, "-r", obj_path])
    cur = None
    result: Dict[int, int] = {}
    for line in out.splitlines():
        m = re.match(r"RELOCATION RECORDS FOR \[(.+)\]:", line)
        if m:
            cur = m.group(1)
            continue
        if cur != section:
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        try:
            off = int(parts[0], 16)
        except ValueError:
            continue
        rtype = parts[1]
        if not rtype.startswith("R_MIPS"):
            continue
        result[off // 4] = RELOC_MASK.get(rtype, DEFAULT_RELOC_MASK)
    return result


def get_candidate(obj_path: Path, function: str, workdir: Path) -> Tuple[List[int], Dict[int, int], str]:
    """candidate words, function-local reloc mask, section name."""
    sym = elf_symbol(obj_path, function)
    if sym is None:
        raise LookupError(f"symbol {function!r} not found in {obj_path}")
    addr, size, section = sym
    if size == 0:
        raise LookupError(f"symbol {function!r} in {obj_path} has size 0 (no -O0 frame?)")
    raw = dump_section(obj_path, section, workdir / "cand_section.bin")
    func_bytes = raw[addr : addr + size]
    words = words_from_bytes(func_bytes)
    relocs = section_relocs(obj_path, section)
    start_word = addr // 4
    end_word = start_word + len(words)
    local = {k - start_word: v for k, v in relocs.items() if start_word <= k < end_word}
    return words, local, section


ASM_TARGET_HEADER = '.set noat\n.set noreorder\n.include "macro.inc"\n.section .text, "ax"\n'


class OwnershipError(LookupError):
    """A function's canonical target extent cannot be proved uniquely."""


@dataclasses.dataclass(frozen=True)
class CanonicalOwnership:
    kind: str
    description: str
    target_start: int
    required_size: Optional[int] = None
    overlay: Optional[int] = None
    row_start: Optional[int] = None
    row_end: Optional[int] = None
    synthetic_vma: Optional[int] = None


OVERLAY_SYMBOL_RE = re.compile(
    r"^func_overlay_(\d+)_F([0-9A-Fa-f]{7,8})_[0-9A-Fa-f]+$"
)


def _source_key(tu: Path) -> Optional[str]:
    try:
        relative = tu.resolve().relative_to((REPO_ROOT / "src").resolve())
    except ValueError:
        return None
    text = relative.as_posix()
    return text[:-2] if text.endswith(".c") else text


def symbol_addrs_record(name: str) -> Optional[Tuple[int, int]]:
    pattern = re.compile(
        r"^%s\s*=\s*0x([0-9A-Fa-f]+)\s*;.*\bsize:0x([0-9A-Fa-f]+)\b"
        % re.escape(name),
        re.MULTILINE,
    )
    matches = pattern.findall(SYMBOL_ADDRS.read_text())
    if len(matches) > 1:
        raise OwnershipError(
            f"{name!r} has {len(matches)} symbol_addrs ownership records"
        )
    if not matches:
        return None
    address, size = matches[0]
    return int(address, 16), int(size, 16)


def resolve_canonical_ownership(
    tu: Path,
    target_symbol: str,
    elf_path: Path,
    atlas_path: Path = OVERLAY_ATLAS,
) -> CanonicalOwnership:
    """Resolve one authoritative function owner or fail closed.

    Overlay identity is the unique atlas row containing the target's encoded
    or linked module offset. Resident identity comes from the canonical symbol
    table, including its reviewed size. Candidate object size is never used to
    decide how many target words belong to the function.
    """
    source = _source_key(tu)
    encoded = OVERLAY_SYMBOL_RE.fullmatch(target_symbol)
    source_overlay = None
    if source:
        match = re.match(r"overlays/o(\d{3})/", source)
        if match:
            source_overlay = int(match.group(1))

    if encoded or source_overlay is not None:
        try:
            atlas = json.loads(atlas_path.read_text())
        except (OSError, json.JSONDecodeError) as exc:
            raise OwnershipError(f"cannot read canonical overlay atlas: {exc}") from exc
        overlay = int(encoded.group(1)) if encoded else source_overlay
        if source_overlay is not None and overlay != source_overlay:
            raise OwnershipError(
                f"target names overlay {overlay}, but {display_path(tu)} belongs to "
                f"overlay {source_overlay}"
            )
        modules = [m for m in atlas.get("modules", []) if m.get("overlay") == overlay]
        if len(modules) != 1:
            raise OwnershipError(
                f"overlay {overlay} has {len(modules)} canonical atlas module rows"
            )
        module = modules[0]
        if encoded:
            target_start = int(encoded.group(2), 16)
        else:
            symbol = elf_symbol(elf_path, target_symbol) if elf_path.is_file() else None
            if symbol is None:
                raise OwnershipError(
                    f"cannot locate friendly overlay target {target_symbol!r} in "
                    f"{display_path(elf_path)}"
                )
            value, _size, _section = symbol
            target_start = value - int(module["synthetic_vma"], 16)
        owners = []
        for row in module.get("text_ownership", []):
            start = int(row["offset"], 16)
            end = int(row["end_offset"], 16)
            if start <= target_start < end:
                owners.append(row)
        if len(owners) != 1:
            raise OwnershipError(
                f"overlay {overlay} offset {target_start:#x} has {len(owners)} "
                "canonical text owners"
            )
        row = owners[0]
        if source is not None and row.get("source") != source:
            raise OwnershipError(
                f"atlas owner {row.get('source')!r} disagrees with TU {source!r}"
            )
        return CanonicalOwnership(
            kind="overlay",
            description=(
                f"atlas overlay {overlay} {row['offset']}..{row['end_offset']} "
                f"({row.get('source')})"
            ),
            target_start=target_start,
            overlay=overlay,
            row_start=int(row["offset"], 16),
            row_end=int(row["end_offset"], 16),
            synthetic_vma=int(module["synthetic_vma"], 16),
        )

    record = symbol_addrs_record(target_symbol)
    if record is None:
        raise OwnershipError(
            f"{target_symbol!r} has no unique overlay-atlas or symbol_addrs owner"
        )
    address, size = record
    if size <= 0 or size % 4:
        raise OwnershipError(
            f"{target_symbol!r} has invalid canonical size {size:#x}"
        )
    return CanonicalOwnership(
        kind="resident",
        description=f"symbol_addrs.us.txt {address:#x} size {size:#x}",
        target_start=address,
        required_size=size,
    )


def find_nonmatching_asm(target_symbol: str) -> Optional[Path]:
    matches = list((REPO_ROOT / "asm" / "nonmatchings").rglob(f"{target_symbol}.s"))
    if len(matches) > 1:
        raise OwnershipError(
            f"{target_symbol!r} has {len(matches)} fallback assembly files"
        )
    return matches[0] if matches else None


def assemble_target_asm(
    asm_path: Path,
    target_symbol: str,
    ownership: CanonicalOwnership,
    workdir: Path,
) -> Tuple[List[int], Dict[int, int]]:
    src = workdir / "target.s"
    src.write_text(ASM_TARGET_HEADER + asm_path.read_text())
    obj = workdir / "target.o"
    subprocess.run(
        [str(AS)] + ASFLAGS + ["-o", str(obj), str(src)],
        cwd=REPO_ROOT,
        check=True,
        capture_output=True,
    )
    symbol = elf_symbol(obj, target_symbol)
    if symbol is None:
        raise OwnershipError(
            f"target assembly does not define {target_symbol!r}"
        )
    address, symbol_size, section = symbol
    if symbol_size <= 0 or symbol_size % 4:
        raise OwnershipError(
            f"target symbol {target_symbol!r} has invalid size {symbol_size:#x}"
        )
    owned_size = ownership.required_size or symbol_size
    if ownership.required_size is not None and symbol_size != ownership.required_size:
        raise OwnershipError(
            f"target symbol size {symbol_size:#x} disagrees with canonical "
            f"ownership {ownership.required_size:#x}"
        )
    if ownership.kind == "overlay":
        assert ownership.row_start is not None and ownership.row_end is not None
        if ownership.target_start + owned_size > ownership.row_end:
            raise OwnershipError(
                f"target range {ownership.target_start:#x}.."
                f"{ownership.target_start + owned_size:#x} escapes atlas owner "
                f"{ownership.row_start:#x}..{ownership.row_end:#x}"
            )
    raw = dump_section(obj, section, workdir / "target_section.bin")
    if address + owned_size > len(raw):
        raise OwnershipError(
            f"target section is too short for owned range {address:#x}+{owned_size:#x}"
        )
    words = words_from_bytes(raw[address : address + owned_size])
    relocs = section_relocs(obj, section)
    start_word = address // 4
    end_word = start_word + len(words)
    local = {k - start_word: v for k, v in relocs.items()
             if start_word <= k < end_word}
    return words, local


SYMBOL_ADDRS = REPO_ROOT / "symbol_addrs.us.txt"


def symbol_addrs_size(name: str) -> Optional[int]:
    record = symbol_addrs_record(name)
    return record[1] if record else None


def section_vma_lma(elf_path: Path, section: str) -> Tuple[int, int]:
    """(VMA, LMA) of a section, from `objdump -h`.

    LMA is where the loader/ROM puts a section's bytes; VMA is where the
    program sees them at runtime. They agree for resident code (the whole
    ROM is memory-mapped 1:1 modulo a fixed offset) and *disagree* for
    overlays, which splat gives a synthetic shared VMA (0xF0000000 and up,
    since every overlay's code loads at the same runtime address) while LMA
    still names each overlay's own distinct ROM location. Deriving the ROM
    offset from LMA-VMA per section, rather than assuming the resident
    segment's fixed 0x7FFFF400, is what makes ELF-range mode work for
    overlay functions too.
    """
    out = _run([OBJDUMP, "-h", elf_path])
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 6 and parts[1] == section:
            return int(parts[3], 16), int(parts[4], 16)
    raise LookupError(f"section {section!r} not found in {elf_path}")


def rom_target_bytes(
    target_symbol: str, elf_path: Path, ownership: CanonicalOwnership
) -> Tuple[List[int], int]:
    """(target words, vram) read straight from the baserom, ELF-range mode."""
    sym = elf_symbol(elf_path, target_symbol)
    if sym is None:
        raise LookupError(f"{target_symbol!r} not found in {elf_path}")
    vram, elf_size, section = sym
    size = ownership.required_size or elf_size
    if size == 0:
        raise OwnershipError(f"{target_symbol!r} has no nonzero owned size")
    if ownership.kind == "overlay":
        assert ownership.synthetic_vma is not None
        module_offset = vram - ownership.synthetic_vma
        if module_offset != ownership.target_start:
            raise OwnershipError(
                f"linked overlay offset {module_offset:#x} disagrees with canonical "
                f"offset {ownership.target_start:#x}"
            )
        assert ownership.row_end is not None
        if module_offset + size > ownership.row_end:
            raise OwnershipError("linked target escapes canonical atlas owner")
    elif vram != ownership.target_start or elf_size != size:
        raise OwnershipError(
            f"linked resident symbol geometry {vram:#x}/{elf_size:#x} disagrees "
            f"with canonical ownership {ownership.target_start:#x}/{size:#x}"
        )
    vma, lma = section_vma_lma(elf_path, section)
    rom_off = lma + (vram - vma)
    baserom = REPO_ROOT / "baseroms" / "mickey.us.z64"
    with open(baserom, "rb") as f:
        f.seek(rom_off)
        data = f.read(size)
    return words_from_bytes(data), vram


def resolve_target(
    tu: Path,
    target_symbol: str,
    target_asm: Optional[Path],
    workdir: Path,
    elf_path: Path,
) -> Tuple[List[int], Dict[int, int], str, CanonicalOwnership]:
    """Target words, local reloc masks, mode, and canonical ownership."""
    ownership = resolve_canonical_ownership(tu, target_symbol, elf_path)
    if target_asm is not None:
        words, relocs = assemble_target_asm(
            target_asm, target_symbol, ownership, workdir
        )
        return words, relocs, f"asm:{display_path(target_asm)}", ownership

    found = find_nonmatching_asm(target_symbol)
    if found is not None:
        words, relocs = assemble_target_asm(found, target_symbol, ownership, workdir)
        return words, relocs, f"asm:{found.relative_to(REPO_ROOT)}", ownership

    if elf_path.exists():
        try:
            words, vram = rom_target_bytes(target_symbol, elf_path, ownership)
            return (words, {},
                    f"rom:0x{vram:08X} (build/mickey.us.elf + baserom)",
                    ownership)
        except LookupError:
            pass

    raise LookupError(
        f"could not resolve target bytes for {target_symbol!r}: no "
        f"asm/nonmatchings/**/{target_symbol}.s, and it is not a symbol in "
        f"{display_path(elf_path)}. "
        "Pass --target-asm explicitly."
    )


# ---------------------------------------------------------------------------
# Scoring -- pure function of word arrays, exercised directly by
# tests/test_flag_sweep.py with synthetic data, no compiler involved.
# ---------------------------------------------------------------------------


@dataclasses.dataclass(frozen=True)
class Score:
    exact: bool
    size_delta: int  # bytes; candidate - target
    diff_words: int  # words differing after masking, plus any length mismatch
    first_mismatch: Optional[int]  # byte offset of the first differing word, or None


def score_words(
    target: Sequence[int],
    candidate: Sequence[int],
    masks: Optional[Dict[int, int]] = None,
) -> Score:
    """
    Compare two word arrays word-by-word. `masks` maps a word index to the
    bits that must be ignored at that word (a relocation site: the bits a
    linker or assembler would still have to fill in, so an unlinked object's
    placeholder there is not a real disagreement).

    Extra words on either side beyond the shorter length each count as one
    more differing word, which is what lets a size-only regression still
    show up in `diff_words` even when every shared word matches.
    """
    masks = masks or {}
    tlen, clen = len(target), len(candidate)
    n = min(tlen, clen)
    diff = 0
    first: Optional[int] = None
    for i in range(n):
        m = masks.get(i, 0)
        tw = target[i] & ~m & 0xFFFFFFFF
        cw = candidate[i] & ~m & 0xFFFFFFFF
        if tw != cw:
            diff += 1
            if first is None:
                first = i * 4
    extra = abs(tlen - clen)
    diff += extra
    if extra and first is None:
        first = n * 4
    size_delta = (clen - tlen) * 4
    exact = size_delta == 0 and diff == 0
    return Score(exact, size_delta, diff, first)


def rank_key(score: Score):
    return (
        0 if score.exact else 1,
        score.diff_words,
        abs(score.size_delta),
        -(score.first_mismatch if score.first_mismatch is not None else 1 << 30),
    )


# ---------------------------------------------------------------------------
# objdiff (optional, best-effort -- tools/objdiff/objdiff-cli is not part of
# this lane and may not exist; never treat its absence as an error)
# ---------------------------------------------------------------------------


def objdiff_match_percent(target_obj: Path, candidate_obj: Path, function: str) -> Optional[float]:
    if not OBJDIFF_CLI.exists():
        return None
    try:
        out = subprocess.run(
            [str(OBJDIFF_CLI), "diff", "-1", str(target_obj), "-2", str(candidate_obj), "-o", function],
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            timeout=30,
        ).stdout
    except Exception:
        return None
    m = re.search(r"(\d+(?:\.\d+)?)\s*%", out)
    return float(m.group(1)) if m else None


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("tu", type=Path, help="C translation unit to sweep")
    p.add_argument("--recipe-tu", type=Path,
                   help="explicit configured in-tree TU supplying context for an external candidate")
    p.add_argument("--function", required=True, help="symbol name in the compiled candidate object")
    p.add_argument(
        "--target-symbol",
        default=None,
        help="symbol/file name on the ROM/asm side, if different from --function "
        "(e.g. a not-yet-matched function's splat name func_80031A30)",
    )
    p.add_argument("--target-asm", type=Path, default=None, help="assemble this .s file as the target directly")
    p.add_argument(
        "--define",
        action="append",
        default=[],
        help="extra -D define, repeatable (e.g. --define NON_MATCHING)",
    )
    p.add_argument("--jobs", type=int, default=None, help="parallel compile workers (default: ncpu-2)")
    p.add_argument("--limit", type=int, default=None, help="only show the top N rows")
    p.add_argument(
        "--rescore",
        action="store_true",
        help="reuse the complete content-addressed compile cache; never compile",
    )
    p.add_argument(
        "--keep",
        action="store_true",
        help="deprecated compatibility option; sweep artifacts are always retained",
    )
    p.add_argument("--objdiff", action="store_true", help="also print objdiff-cli's match %% for the top row, if installed")
    p.add_argument("--json", action="store_true", help="emit scalar summary JSON; human logs go to stderr")
    p.add_argument(
        "--elf",
        type=Path,
        default=REPO_ROOT / "build" / "mickey.us.elf",
        help="linked ELF to use for ROM-range target resolution",
    )
    return p.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    if args.json:
        output = sys.stdout
        with contextlib.redirect_stdout(sys.stderr):
            return run_sweep(args, output)
    return run_sweep(args)


def summary_report(scored, failed, cache_key, compiled_count, total):
    """Report search coverage separately from masked score and match proof."""
    return {
        "schema": "mickey-flag-sweep-summary-v1",
        "cache_key": cache_key,
        "total_combinations": total,
        "compiled_combinations": compiled_count,
        "scored_combinations": len(scored),
        "failed_combinations": sorted(row[0].id for row in failed),
        "complete": len(scored) == total and not failed,
        "scope": "unlinked-relocation-masked-search-not-promotion-proof",
        "promotion_proof_included": False,
        "ranked": [{"combination": combo.id,
                    "flags": combo.opt + combo.isa + combo.extra,
                    "compiler": "ido-phases.py" if combo.use_ido_phases else "cc",
                    "masked_exact": score.exact,
                    "size_delta": score.size_delta,
                    "differing_words": score.diff_words,
                    "first_mismatch_offset": score.first_mismatch}
                   for combo, score, _error, _elapsed in scored],
    }


def run_sweep(args, report_stream=None) -> int:
    tu = repo_cli_path(args.tu)
    if not tu.exists():
        print(f"flag_sweep: no such file: {tu}", file=sys.stderr)
        return 2

    target_asm = repo_cli_path(args.target_asm) if args.target_asm is not None else None
    elf_path = repo_cli_path(args.elf)

    target_symbol = args.target_symbol or args.function

    text = tu.read_text()
    defines = list(args.define)
    if "#ifdef NON_MATCHING" in text and not any(d.split("=")[0] == "NON_MATCHING" for d in defines):
        defines.append("NON_MATCHING")
        print("flag_sweep: TU has #ifdef NON_MATCHING; auto-adding -DNON_MATCHING", file=sys.stderr)

    lattice = build_lattice()
    ncpu = os.cpu_count() or 4
    workers = args.jobs or max(1, ncpu - 2)

    try:
        recipe_tu = repo_cli_path(args.recipe_tu) if args.recipe_tu else tu
        recipe = configured_recipe(recipe_tu)
        cache_key, cache_manifest = compilation_cache_identity(tu, defines, lattice, recipe)
    except LookupError as e:
        print(f"flag_sweep: {e}", file=sys.stderr)
        return 2
    sweep_root = REPO_ROOT / "build" / "flag_sweep" / "cache" / cache_key
    sweep_root.mkdir(parents=True, exist_ok=True)
    manifest_path = sweep_root / "manifest.json"
    manifest_text = json.dumps(cache_manifest, sort_keys=True, indent=2) + "\n"
    if manifest_path.is_file() and manifest_path.read_text() != manifest_text:
        print("flag_sweep: cache manifest disagrees with its content key", file=sys.stderr)
        return 2
    manifest_path.write_text(manifest_text)

    print(
        f"flag_sweep: {len(lattice)} combos, {workers} workers, "
        f"target={target_symbol!r}, cache={cache_key[:12]}"
    )

    # Target artifacts are separate from the compile cache: target geometry
    # may be corrected and rescored without invalidating any candidate object.
    target_dir = REPO_ROOT / "build" / "flag_sweep" / "targets" / cache_key
    target_dir.mkdir(parents=True, exist_ok=True)
    try:
        target_words, target_relocs, target_mode, ownership = resolve_target(
            tu, target_symbol, target_asm, target_dir, elf_path
        )
    except (LookupError, subprocess.CalledProcessError) as e:
        print(f"flag_sweep: {e}", file=sys.stderr)
        return 2
    print(
        f"flag_sweep: target = {target_mode}, {len(target_words)} words; "
        f"owner = {ownership.description}"
    )

    t0 = time.monotonic()
    try:
        results, compiled_count = collect_compile_results(
            tu, lattice, sweep_root, defines, workers, rescore=args.rescore,
            compiler_args=lattice_base_arguments(recipe),
        )
        try:
            final_recipe = configured_recipe(recipe_tu)
            final_key, _ = compilation_cache_identity(tu, defines, lattice, final_recipe)
        except (LookupError, OSError):
            final_key = None
        if final_key != cache_key:
            for combo in lattice:
                _combo_result_path(sweep_root / combo.id).unlink(missing_ok=True)
            raise LookupError("compiler context changed during sweep; cache receipts invalidated")
    except LookupError as e:
        print(f"flag_sweep: {e}", file=sys.stderr)
        return 2
    compile_elapsed = time.monotonic() - t0

    rows = []
    for r in results:
        if not r.ok:
            rows.append((r.combo, None, r.error, r.elapsed))
            continue
        try:
            cand_words, cand_relocs, _section = get_candidate(r.obj_path, args.function, r.obj_path.parent)
        except LookupError as e:
            rows.append((r.combo, None, str(e), r.elapsed))
            continue
        masks = dict(target_relocs)
        for k, v in cand_relocs.items():
            masks[k] = masks.get(k, 0) | v
        sc = score_words(target_words, cand_words, masks)
        rows.append((r.combo, sc, "", r.elapsed))

    scored = [row for row in rows if row[1] is not None]
    failed = [row for row in rows if row[1] is None]
    lattice_order = {combo.id: index for index, combo in enumerate(lattice)}
    scored.sort(key=lambda row: (rank_key(row[1]), lattice_order[row[0].id]))

    if args.limit:
        scored_shown = scored[: args.limit]
    else:
        scored_shown = scored

    print()
    print(f"{'combo':<26}{'exact':<7}{'Δsize':>8}{'diffwords':>11}{'firstmiss':>11}   flags")
    for combo, sc, _err, _elapsed in scored_shown:
        flags = " ".join(combo.opt + combo.isa + combo.extra)
        cc = "ido-phases.py" if combo.use_ido_phases else "cc"
        fm = "-" if sc.first_mismatch is None else f"+0x{sc.first_mismatch:x}"
        print(
            f"{combo.id:<26}{'YES' if sc.exact else 'no':<7}{sc.size_delta:>+8}{sc.diff_words:>11}{fm:>11}   {cc} {flags}"
        )

    if scored:
        best = scored[0][0]
        print()
        print("Best diagnostic flags (require target proof and consumer impact review before adoption):")
        cc_line = "CC := $(IDO_PHASES)" if best.use_ido_phases else ""
        if cc_line:
            print(f"  {cc_line}")
        print(f"  OPT_FLAGS := {' '.join(best.opt) or '(none)'}")
        print(f"  MIPSISET := {' '.join(best.isa)}")
        if best.extra:
            print(f"  CFLAGS += {' '.join(best.extra)}")
        if scored[0][1].exact:
            print("  -> BYTE-IDENTICAL under this tool's masked word comparison.")
            print("     Confirm with tools/wb_compare.sh before declaring it matched.")

    if failed:
        print(
            f"\n{len(failed)} combo(s) failed to compile or extract; "
            "inspect their retained cache logs."
        )

    if args.objdiff and scored:
        best_combo = scored[0][0]
        best_obj = sweep_root / best_combo.id / "out.o"
        pct = objdiff_match_percent(target_dir / "target.o", best_obj, args.function)
        if pct is not None:
            print(f"\nobjdiff-cli match for the top row: {pct:.2f}%")
        else:
            print("\nobjdiff-cli not available or gave no parseable output; skipped.")

    print(
        f"\n{compiled_count} combos compiled, {len(lattice) - compiled_count} reused "
        f"in {compile_elapsed:.1f}s ({workers} workers)"
    )
    print(f"cache: {sweep_root.relative_to(REPO_ROOT)}")
    if report_stream is not None:
        print(json.dumps(summary_report(scored, failed, cache_key, compiled_count, len(lattice)),
                         indent=2, sort_keys=True), file=report_stream)
    return 0 if scored else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
