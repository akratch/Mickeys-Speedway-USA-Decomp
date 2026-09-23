#!/usr/bin/env python3
"""Batch decomp-permuter runner over the NON_MATCHING queue.

docs/adr/0007-matching-tools.md: decomp-permuter runs only as a *bounded
batch job*, never inside an agent's own reasoning loop. This is that job
runner. It does not reason about candidates; it imports each queued
function, runs permuter.py under a wall-clock cap, and reports what came
back. See docs/permute-batch.md for the day-to-day usage writeup.

QUEUE DISCOVERY

Primary source: config/overlays.us.json's per-module `text_ownership` rows
(written by tools/overlay_atlas.py), each carrying a mechanically-derived
`nonmatching` flag (true iff the C source for that range still has an
`#ifdef NON_MATCHING` guard -- see overlay_atlas.is_nonmatching_source).
Cross-checked, and supplemented for anything the atlas doesn't cover yet,
by scanning src/**/*.c directly for `#ifdef NON_MATCHING` blocks.

THE OVERLAY NAMING QUIRK

splat auto-names every overlay function it disassembles from the ROM
`func_overlay_MMM_FOOOOOOO_ROMADDR` (the module shares one synthetic VMA
with every other overlay, so spimdisasm can't derive a unique name from the
address alone -- see overlay_atlas.py's SYNTHETIC_VMA comment). The C
symbol is the friendly name a human already gave it
(`overlay1GetEntry`). A queued function's `#ifdef NON_MATCHING` branch
already defines the friendly name; its `#else` branch's
`#pragma GLOBAL_ASM("asm/nonmatchings/.../func_overlay_...s")` still points
at the auto name. decomp-permuter's import.py derives the function name it
hunts for in the C from the target .s file's `glabel` line, so handing it
the auto-named .s directly fails ("not found in base.c"). This script
works around that without editing import.py (out of this lane's ownership)
or the project's own asm/ output (gitignored, never written to): it copies
the target .s into the function's scratch dir and renames the
glabel/endlabel pair to the friendly name -- a label rename, metadata only,
exactly the kind of thing the project's own POSTPROCESS
`objcopy --redefine-sym` steps already do to compiled objects (permitted
under docs/adr/0002-no-post-compile-instruction-editing.md). No instruction
word is touched and nothing under asm/ is modified in place.

THE NON_MATCHING DEFINE

A queued function's C file still carries both branches
(`#ifdef NON_MATCHING <candidate> #else <pragma> #endif`); the project's
own Makefile selects the candidate branch by building with
`-DNON_MATCHING` (see Makefile's `NON_MATCHING ?= 0` escape hatch). This
script's generated per-function permuter_settings.toml adds that same
define, so the unmodified C file, not a hand-edited copy, is what
import.py preprocesses -- consistent for a file with more than one queued
function in it too, since the C preprocessor (not this script) resolves
every `#ifdef NON_MATCHING` block in the translation unit the same way.

PROMOTION

When a run reaches score 0, this script does not stop at reporting it: an
exact permuter candidate is still just a candidate (ADR 0007's own
closing line) until it is compiled by the project's real toolchain, linked
at its real address, and byte-compared -- so:

  1. extract the winning candidate's function body (its output-0-*/source.c
     is the whole pruned translation unit, not just the function) and
     splice it into the real C file in place of the `#ifdef NON_MATCHING`
     wrapper, dropping the ifdef/else/pragma/endif;
  2. `gmake -jN` and `gmake verify` (byte-identical ROM rebuild);
  3. `tools/promotion_proof.py <symbol> --json` for complete post-promotion
     linked-range, frame and relocation count/type/offset/identity proof;
  4. regenerate and check derived metadata, scoreboard, docs and cleanroom;
     optionally commit through an isolated index. Failure rolls back owned
     source and derived changes; conflicting independent edits are preserved
     with recovery evidence. An unsuccessful promotion never earns credit.

This intentionally does not touch a POSTPROCESS Makefile rule automatically
-- whether one is still needed after promotion is a per-function judgment
call (see docs/permute-batch.md and the pilot commit for the worked
example); --apply-makefile-cleanup is opt-in and only ever removes a rule
that becomes a dead reference to a symbol the object no longer emits.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import contextlib
import dataclasses
import fcntl
import hashlib
import json
import os
import re
import shlex
import shutil
import signal
import subprocess
import sys
import threading
import time
import tomllib
import uuid
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))
import reloc_surface  # noqa: E402
import sweep_receipts  # noqa: E402
import promotion_transaction  # noqa: E402
_LOADED_IMPLEMENTATIONS = {
    name: (Path(path), hashlib.sha256(Path(path).read_bytes()).hexdigest())
    for name, path in (("runner", __file__), ("receipts", sweep_receipts.__file__),
                       ("promotion", promotion_transaction.__file__),
                       ("relocations", reloc_surface.__file__),
                       ("relocation_identity", reloc_surface.ri.__file__),
                       ("overlay_table", reloc_surface.ot.__file__))
}
ATLAS_PATH = ROOT / "config" / "overlays.us.json"
PERMUTER_DIR = ROOT / "tools" / "permuter"
IMPORT_PY = PERMUTER_DIR / "import.py"
PERMUTER_PY = PERMUTER_DIR / "permuter.py"
PYTHON = ROOT / ".venv" / "bin" / "python"
BUILD_PERMUTER = ROOT / "build" / "permuter"
SUMMARY_JSON = BUILD_PERMUTER / "summary.json"
SUMMARY_TXT = BUILD_PERMUTER / "summary.txt"
RANKING_PATH = ROOT / "config" / "nonmatching-ranking.us.json"
MANDATORY_PERMUTER_ARGS = (
    "--stop-on-zero", "--quiet", "--stack-diffs", "--no-ignore-branch-targets",
)
BASEROM = ROOT / "baseroms" / f"mickey.us.z64"
OBJCOPY = ROOT / "tools" / "binutils" / "mips64-elf-objcopy"
DEFAULT_INTEGRATION_REF = "campaign/unchain"
CANCEL_EVENT = threading.Event()


# Flags that shape codegen and therefore must match the real per-file build
# exactly (the same set tools/permute.sh recovers). Anything else in the cc
# line (-I, -D, -c, -o ...) is reproduced by BASE_CC_ARGS/INCLUDES already.
CODEGEN_FLAG_RE = re.compile(
    r"-mips[0-9]|-O[0-9]|-Wo,[^ ]*|-Wab,[^ ]*|-g[0-9]?(?= |$)|-(?:32|n32|64)(?= |$)"
)
# Post-compile ELF passes the scratch cannot replicate: they are digest-guarded
# against the *matched* bytes and would abort (or lie) on a permuted object.
# trim_elf_section only trims section padding and never touches a function's
# words, so it is harmless to skip.
UNREPLICABLE_POSTPROC_RE = re.compile(r"\.py\b")

NON_MATCHING_BLOCK_RE = re.compile(
    r"#ifdef NON_MATCHING\b(?P<body>.*?)#else\b(?P<else_>.*?)#endif\b",
    re.DOTALL,
)
PP_DIRECTIVE_RE = re.compile(
    r"^[ \t]*#[ \t]*(?P<kind>if|ifdef|ifndef|elif|else|endif)\b"
    r"(?P<argument>[^\n]*)",
    re.MULTILINE,
)
FUNC_DEF_RE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*"
    r"\([^;{]*\)\s*\{",
    re.MULTILINE,
)
GLOBAL_ASM_RE = re.compile(r'#pragma\s+GLOBAL_ASM\("(?P<path>[^"]+)"\)')
GLABEL_RE = re.compile(r"^\s*(glabel|endlabel|jlabel|dlabel)\s+(\S+)", re.MULTILINE)
LEXICAL_NOISE_RE = re.compile(
    r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'",
    re.DOTALL,
)

# Flag groups this script knows about, keyed by a classifier over the
# source path relative to src/. Mirrors tools/permuter_settings.toml's own
# header comment and the root Makefile's OPT_FLAGS/MIPSISET defaults plus the
# per-directory overrides in Makefile (main/%) and mk/overlays.mk (overlays/%).
FLAG_GROUP_DEFAULT = ("-O2", "-mips1", "-32")  # libultra project default
FLAG_GROUP_OVERLAY = ("-O2", "-mips2", "-32")  # src/main/**, src/overlays/**
FLAG_GROUP_O2_G3 = ("-O2", "-g3", "-mips2", "-32")  # LIBULTRA_O2_G3_TUS

BASE_CC_ARGS = (
    "-c -non_shared -G 0 -Xcpluscomm -fullwarn -woff 649,838 -nostdinc "
    "-D_LANGUAGE_C -D_FINALROM -DTARGET_N64 -DVERSION_us -D_MIPS_SZLONG=32"
)
INCLUDES = "-I . -I include -I include/libc -I include/PR -I assets"
ASSEMBLER_COMMAND = (
    "tools/binutils/mips64-elf-as -march=vr4300 -32 -mabi=32 -G0 -I include"
)

PRESERVE_MACROS = """[preserve_macros]
"g[DS]P.*" = "void"
"gDma.*" = "void"
"gDkr.*" = "void"
"fast3d_cmd" = "void"
"OS_PHYSICAL_TO_K0" = "void *"
"_SHIFTL" = "unsigned int"
"""


# --------------------------------------------------------------------------
# Queue discovery
# --------------------------------------------------------------------------


@dataclasses.dataclass
class QueueItem:
    func: str
    c_file: Path  # absolute path
    overlay: Optional[int] = None
    source: Optional[str] = None  # atlas "source" string, e.g. overlays/o001/overlay1GetEntry

    @property
    def rel_c_file(self) -> str:
        return str(self.c_file.relative_to(ROOT))


@dataclasses.dataclass(frozen=True)
class NonMatchingBlock:
    body: str
    fallback: str
    start: int
    end: int
    body_start: int
    body_end: int
    fallback_start: int
    fallback_end: int


def iter_nonmatching_blocks(source_text: str):
    """Yield top-level ``#ifdef NON_MATCHING`` body/fallback pairs.

    The old non-greedy regex stopped at the first nested ``#else`` or
    ``#endif``. Several candidates legitimately contain feature switches,
    so their GLOBAL_ASM fallback was invisible to queue isolation.
    """
    stack: list[dict[str, object]] = []
    completed: list[tuple[int, NonMatchingBlock]] = []
    for match in PP_DIRECTIVE_RE.finditer(source_text):
        kind = match.group("kind")
        argument = match.group("argument").strip()
        if kind in ("if", "ifdef", "ifndef"):
            stack.append(
                {
                    "target": kind == "ifdef" and argument == "NON_MATCHING",
                    "start": match.start(),
                    "body_start": match.end(),
                    "body_end": None,
                    "fallback_start": None,
                }
            )
            continue
        if not stack:
            continue
        current = stack[-1]
        if kind in ("else", "elif"):
            if current["target"] and current["body_end"] is None:
                current["body_end"] = match.start()
                current["fallback_start"] = match.end()
            continue
        if kind == "endif":
            current = stack.pop()
            if not current["target"]:
                continue
            body_end = current["body_end"]
            fallback_start = current["fallback_start"]
            if body_end is None or fallback_start is None:
                continue
            completed.append(
                (
                    int(current["start"]),
                    NonMatchingBlock(
                        body=source_text[int(current["body_start"]):int(body_end)],
                        fallback=source_text[int(fallback_start):match.start()],
                        start=int(current["start"]),
                        end=match.end(),
                        body_start=int(current["body_start"]),
                        body_end=int(body_end),
                        fallback_start=int(fallback_start),
                        fallback_end=match.start(),
                    ),
                )
            )
    for _, block in sorted(completed, key=lambda pair: pair[0]):
        yield block


def block_function_name(source_text: str, block: NonMatchingBlock) -> Optional[str]:
    """Return the C symbol defined by a NON_MATCHING candidate body.

    A few shared implementations spell the definition through a simple
    object-like macro. Resolve that one identifier without attempting to
    duplicate the C preprocessor.
    """
    fn = FUNC_DEF_RE.search(block.body)
    if fn is None:
        return None
    name = fn.group("name")
    for _ in range(8):
        macro = re.search(
            rf"^[ \t]*#[ \t]*define[ \t]+{re.escape(name)}[ \t]+"
            r"(?P<replacement>[A-Za-z_][A-Za-z0-9_]*)[ \t]*(?:/\*.*\*/)?$",
            source_text,
            re.MULTILINE,
        )
        if macro is None:
            break
        name = macro.group("replacement")
    return name


def unconditional_function_names(source_text: str) -> set[str]:
    """Return definitions whose declaration starts outside preprocessor guards.

    This is deliberately conservative scheduling evidence, not a C parser.
    Comments and literals are position-preservingly masked so their contents
    cannot look like definitions; every ``#if`` family raises guard depth.
    """
    def mask_noise(match: re.Match[str]) -> str:
        return "".join("\n" if char == "\n" else " " for char in match.group(0))

    text = LEXICAL_NOISE_RE.sub(mask_noise, source_text)
    directives = list(PP_DIRECTIVE_RE.finditer(text))
    names = set()
    depth = 0
    directive_index = 0
    for function in FUNC_DEF_RE.finditer(text):
        while (
            directive_index < len(directives)
            and directives[directive_index].start() < function.start()
        ):
            kind = directives[directive_index].group("kind")
            if kind in ("if", "ifdef", "ifndef"):
                depth += 1
            elif kind == "endif" and depth:
                depth -= 1
            directive_index += 1
        if depth == 0:
            names.add(function.group("name"))
    return names


def _libultra_o2_g3_tus() -> set[str]:
    """Parse Makefile's LIBULTRA_O2_G3_TUS := ... continuation list."""
    text = (ROOT / "Makefile").read_text(errors="replace")
    m = re.search(r"LIBULTRA_O2_G3_TUS\s*:=\s*(.*?)(?<!\\)\n", text, re.DOTALL)
    if not m:
        return set()
    raw = m.group(1).replace("\\\n", " ")
    return set(raw.split())


_O2_G3_TUS = None


@dataclasses.dataclass(frozen=True)
class BuildRecipe:
    """What the project's real build does to one TU's object: the codegen
    flags on its cc line and any post-compile objcopy chain. Recovered from
    `gmake -n -W <source> <obj>` (make treats the source as changed without
    modifying its timestamp, avoiding the silent -mips1 false floor
    docs/matching-triage.md records)."""

    flags: tuple[str, ...]
    objcopy_steps: tuple[str, ...]  # shell fragments, real object path intact
    skipped_postproc: tuple[str, ...]  # digest-guarded passes not replicated
    from_dry_run: bool
    compiler_args: tuple[str, ...] = ()  # complete IDO tail, excluding input/output


def compiler_arguments(line: str, source: str, obj: str) -> tuple[str, ...]:
    """Preserve every real IDO flag, define and include in its original order."""
    words = shlex.split(line)
    cc = words.index("tools/ido/cc")
    wrapped = (cc == 2 and words[1] == "tools/asm-processor/build.py"
               and Path(words[0]).name.startswith("python"))
    if cc != 0 and not wrapped:
        raise ValueError("unsupported compiler wrapper; cannot prove scratch command fidelity")
    tail = words[cc + 1:]
    if wrapped:
        # asm-processor: cc -- assembler and its arguments -- IDO arguments.
        if not tail or tail[0] != "--":
            raise ValueError("unsupported asm-processor compiler command")
        tail = tail[tail.index("--", 1) + 1:]
    if any(token in {"&&", "||", ";", "|", ">", "2>"} or "$" in token for token in tail):
        raise ValueError("unsupported shell syntax in IDO argument tail")
    if source not in tail or "-o" not in tail:
        raise ValueError("cannot identify IDO input/output arguments")
    output_index = tail.index("-o")
    if output_index + 1 >= len(tail) or tail[output_index + 1] != obj:
        raise ValueError("unexpected IDO output path")
    del tail[output_index:output_index + 2]
    tail.remove(source)
    return tuple(tail)


def build_recipe_for(c_file: Path, deadline: Optional[float] = None) -> BuildRecipe:
    # Re-read each time: promotions and operator edits can change per-file
    # flags during a long batch. A path-only cache hid those changes.
    obj = f"build/{c_file.relative_to(ROOT).as_posix()}.o"
    dry = bounded_capture(
        ["gmake", "-n", "-W", c_file.relative_to(ROOT).as_posix(), obj], deadline, check=True,
    ).stdout
    flags: tuple[str, ...] = ()
    objcopy_steps: list[str] = []
    skipped: list[str] = []
    compiler_args: tuple[str, ...] = ()
    # gmake echoes the recipe verbatim, so the cc command arrives as
    # "... tools/ido/cc -- <as> -- \" + a continuation line carrying the
    # flags and the object path. Join continuations before parsing.
    dry = dry.replace("\\\n", " ")
    for line in dry.splitlines():
        if obj not in line:
            continue
        if "objcopy" in line and "tools/ido/cc" not in line:
            # A shell conditional in the recipe (`if [ ... ]; then <step>; fi`)
            # applies its step only when the test holds; the scratch cannot
            # evaluate that against the real object, and copying the fragment
            # verbatim leaves a stray `fi` that breaks compile.sh (the
            # overlay_001_tail scratch died on it). Record the whole
            # conditional as not replicable instead.
            if re.search(r"(^|[;&\s])if\s+\[", line) or re.search(r";\s*fi(\s|$)", line):
                skipped.append(line.strip())
                continue
            for seg in (s.strip() for s in line.split("&&")):
                if not seg:
                    continue
                if UNREPLICABLE_POSTPROC_RE.search(seg):
                    skipped.append(seg)
                elif "objcopy" in seg:
                    objcopy_steps.append(seg)
                else:
                    skipped.append(seg)
        elif "tools/ido/cc" in line and not flags:
            found = CODEGEN_FLAG_RE.findall(line)
            if any(f.startswith("-mips") for f in found):
                flags = tuple(dict.fromkeys(found))
                compiler_args = compiler_arguments(line, c_file.relative_to(ROOT).as_posix(), obj)
    if flags:
        recipe = BuildRecipe(flags, tuple(objcopy_steps), tuple(skipped), True, compiler_args)
    else:
        print(
            f"WARNING: could not recover real compile flags for {obj}; "
            f"falling back to the static flag group (may search the wrong ISA)",
            file=sys.stderr,
        )
        recipe = BuildRecipe(flag_group_for(c_file), tuple(objcopy_steps), tuple(skipped), False)
    return recipe


def replicate_objcopy(scratch: Path, recipe: BuildRecipe, c_file: Path, out_dir: Path,
                      alias_history: Optional[list] = None) -> None:
    """Append the TU's post-compile objcopy chain to the scratch's compile.sh,
    retargeted at the scratch object, so the scratch object == the real
    per-TU object (workbench improvement-backlog #9). Records what was and
    was not replicated in <out_dir>/recipe.txt."""
    obj = f"build/{c_file.relative_to(ROOT).as_posix()}.o"
    csh = scratch / "compile.sh"
    lines = [f"flags: {' '.join(recipe.flags)} ({'gmake -n' if recipe.from_dry_run else 'static group'})"]
    # Whole-token match only: a plain substring replace would also rewrite
    # `build/x.c.o.syms`-style arguments into nonexistent paths, and a step
    # that does not mention the object at all cannot be retargeted (it would
    # reach the scratch still pointing at the real build tree).
    mention = re.compile(r"(?<![\w./-])(?:\./)?" + re.escape(obj) + r"(?![\w.-])")
    if recipe.objcopy_steps and csh.is_file():
        with open(csh, "a") as f:
            f.write("\n")
            for step in recipe.objcopy_steps:
                if not mention.search(step):
                    lines.append(f"skipped (does not name the object): {step}")
                    continue
                remapped = mention.sub('"$OUTPUT"', step)
                if alias_history is not None:
                    words = shlex.split(remapped)
                    if (not words or Path(words[0]).resolve() != OBJCOPY.resolve()
                            or words[-1] != "$OUTPUT"
                            or any(word in {"&&", "||", ";", "|"} for word in words)):
                        raise RuntimeError("unsupported scratch objcopy alias invocation")
                    remaining = words[1:-1]
                    additions, removals = [], []
                    while remaining:
                        if remaining[0] == "--redefine-sym" and len(remaining) >= 2:
                            remaining = remaining[2:]
                        elif remaining[0].startswith("--redefine-sym="):
                            remaining = remaining[1:]
                        elif remaining[0] in {"--add-symbol", "--remove-section"} or remaining[0].startswith(("--add-symbol=", "--remove-section=")):
                            option, separator, value = remaining[0].partition("=")
                            if separator:
                                remaining = remaining[1:]
                            elif len(remaining) >= 2:
                                value, remaining = remaining[1], remaining[2:]
                            else:
                                raise RuntimeError("missing scratch objcopy metadata argument")
                            if option == "--add-symbol":
                                match = re.fullmatch(r"([^=,]+)=(?:(\.[A-Za-z0-9_.$]+):)?(?:0[xX][0-9a-fA-F]+|[0-9]+),global", value)
                                if match is None:
                                    raise RuntimeError("unsupported scratch added symbol declaration")
                                name, section = match.groups()
                                reloc_surface.ri.canonicalize_redefine_aliases([(name, name)])
                                additions.append((name, section))
                            else:
                                if not re.fullmatch(r"\.[A-Za-z0-9_.$]+", value):
                                    raise RuntimeError("unsupported scratch removed section declaration")
                                removals.append(value)
                        else:
                            raise RuntimeError("unsupported scratch objcopy alias operation")
                    pairs = reloc_surface.ri.parse_objcopy_redefine_pairs(remapped)
                    alias_history.append({"renames": tuple(pairs), "additions": tuple(additions),
                                          "removals": tuple(removals)})
                f.write(remapped + "\n")
                lines.append(f"replicated: {remapped}")
    for s in recipe.skipped_postproc:
        lines.append(f"skipped (not replicable): {s}")
    (out_dir / "recipe.txt").write_text("\n".join(lines) + "\n")


def flag_group_for(c_file: Path) -> tuple[str, ...]:
    global _O2_G3_TUS
    rel = c_file.relative_to(ROOT / "src").as_posix()
    if rel.startswith("main/") or rel.startswith("overlays/"):
        return FLAG_GROUP_OVERLAY
    if rel.startswith("libultra/"):
        if _O2_G3_TUS is None:
            _O2_G3_TUS = _libultra_o2_g3_tus()
        stem = Path(rel).stem
        if stem in _O2_G3_TUS:
            return FLAG_GROUP_O2_G3
    return FLAG_GROUP_DEFAULT


def discover_queue_from_atlas() -> list[QueueItem]:
    if not ATLAS_PATH.is_file():
        return []
    data = json.loads(ATLAS_PATH.read_text())
    items = []
    for mod in data.get("modules", []):
        overlay = mod.get("overlay")
        for row in mod.get("text_ownership", []):
            if not row.get("nonmatching"):
                continue
            source = row.get("source")
            if not source:
                continue
            func = source.rsplit("/", 1)[-1]
            c_file = ROOT / "src" / f"{source}.c"
            if not c_file.is_file():
                continue
            text = c_file.read_text(errors="replace")
            body_funcs = {
                name
                for block in iter_nonmatching_blocks(text)
                if (name := block_function_name(text, block)) is not None
            }
            # An atlas row identifies a C translation unit, not necessarily
            # a function. Consolidated overlay TUs therefore have source
            # basenames such as overlay_001 while their queued functions are
            # found by the source scan below. Do not invent a queue symbol.
            if func not in body_funcs:
                continue
            items.append(
                QueueItem(func=func, c_file=c_file, overlay=overlay, source=source)
            )
    return items


def discover_queue_from_source_scan() -> list[QueueItem]:
    """Fallback / supplement: scan src/**/*.c directly for #ifdef
    NON_MATCHING blocks, independent of the atlas. Catches anything not
    (yet) reflected in config/overlays.us.json (non-overlay sources, or a
    tree where the atlas hasn't been regenerated since a conversion)."""
    items = []
    for c_file in sorted(ROOT.glob("src/**/*.c")):
        text = c_file.read_text(errors="replace")
        if "#ifdef NON_MATCHING" not in text:
            continue
        for block in iter_nonmatching_blocks(text):
            func = block_function_name(text, block)
            if func is None:
                continue
            overlay = None
            mo = re.search(r"src/overlays/o(\d+)/", str(c_file))
            if mo:
                overlay = int(mo.group(1))
            items.append(QueueItem(func=func, c_file=c_file, overlay=overlay))
    return items


def discover_queue() -> list[QueueItem]:
    """Union of the atlas rows and the direct source scan, de-duplicated by
    (c_file, func). The atlas is authoritative when both agree; the scan
    catches anything the atlas doesn't have a row for yet."""
    by_key: dict[tuple[str, str], QueueItem] = {}
    for it in discover_queue_from_atlas():
        by_key[(it.rel_c_file, it.func)] = it
    for it in discover_queue_from_source_scan():
        by_key.setdefault((it.rel_c_file, it.func), it)
    return sorted(by_key.values(), key=lambda it: (it.rel_c_file, it.func))


def read_excluded_functions(paths: list[Path]) -> set[str]:
    """Read explicit scheduling exclusions, one function name per line.

    Exclusions are operator routing, never match evidence.  Require every
    requested file and every non-comment line to be valid so a typo cannot
    silently change the queue.
    """
    excluded: set[str] = set()
    for path in paths:
        try:
            lines = path.read_text().splitlines()
        except OSError as exc:
            raise ValueError(f"cannot read exclusion file {path}: {exc}") from exc
        for line_number, raw in enumerate(lines, 1):
            name = raw.strip()
            if not name or name.startswith("#"):
                continue
            if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
                raise ValueError(
                    f"invalid function name in {path}:{line_number}: {name!r}"
                )
            excluded.add(name)
    return excluded


def exclude_resolved_on_ref(
    queue: list[QueueItem], ref: str
) -> tuple[list[QueueItem], list[QueueItem], Optional[str]]:
    """Drop stale-lane queue entries already resolved on ``ref``.

    Long-running lane worktrees intentionally keep their own source and build
    state, but Git refs are shared.  Consult the integration ref's version of
    the same source file so a lane created before a promotion cannot spend a
    later batch rediscovering an already-canonical exact function.

    If the ref or path is unavailable, retain the item: absence of scheduling
    evidence must never discard matching work.
    """
    if not ref:
        return queue, [], None
    resolved_ref = subprocess.run(
        ["git", "rev-parse", "--verify", "--quiet", f"{ref}^{{commit}}"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    if resolved_ref.returncode != 0:
        print(f"note: integration ref {ref!r} is unavailable; stale-lane filter disabled")
        return queue, [], None
    ref_oid = resolved_ref.stdout.strip()

    fallback_scan = subprocess.run(
        ["git", "grep", "-h", "-E", "GLOBAL_ASM", ref_oid, "--", "src"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    if fallback_scan.returncode not in (0, 1):
        print(
            f"note: could not index fallbacks on {ref!r} ({ref_oid[:12]}); "
            "stale-lane filter disabled"
        )
        return queue, [], ref_oid
    fallbacks_on_ref = {
        match.group("path")
        for match in GLOBAL_ASM_RE.finditer(fallback_scan.stdout)
    }

    ref_state_by_file: dict[str, Optional[tuple[set[str], set[str]]]] = {}
    kept = []
    skipped = []
    for item in queue:
        rel = item.rel_c_file
        if rel not in ref_state_by_file:
            shown = subprocess.run(
                ["git", "show", f"{ref_oid}:{rel}"],
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                check=False,
            )
            if shown.returncode != 0:
                ref_state_by_file[rel] = None
            else:
                text = shown.stdout
                unresolved = {
                    name
                    for block in iter_nonmatching_blocks(text)
                    if (name := block_function_name(text, block)) is not None
                }
                defined = unconditional_function_names(text)
                ref_state_by_file[rel] = (unresolved, defined)
        ref_state = ref_state_by_file[rel]
        if ref_state is None:
            kept.append(item)
            continue
        unresolved, defined = ref_state
        if item.func not in defined or item.func in unresolved:
            # A moved/new/macro-defined function is not evidence of a
            # canonical resolution. Retain it and let normal proof decide.
            kept.append(item)
            continue

        # The same friendly definition is unguarded at the old path. Before
        # calling it resolved, ensure its stable assembly fallback did not
        # move elsewhere or survive in an unrecognized guard form.
        asm_target = find_asm_target(item)
        if not asm_target:
            kept.append(item)
            continue
        if asm_target not in fallbacks_on_ref:
            skipped.append(item)
        else:
            kept.append(item)
    return kept, skipped, ref_oid


def find_asm_target(item: QueueItem) -> Optional[str]:
    """The GLOBAL_ASM path for this specific function's #else branch,
    read straight out of the C file text (works whether or not the tree
    has been `gmake extract`-ed with NON_MATCHING=0 active, since the
    pragma string is present in the source either way)."""
    text = item.c_file.read_text(errors="replace")
    for block in iter_nonmatching_blocks(text):
        if block_function_name(text, block) == item.func:
            am = GLOBAL_ASM_RE.search(block.fallback)
            if am:
                return am.group("path")
    return None


# --------------------------------------------------------------------------
# Per-function permuter run
# --------------------------------------------------------------------------


@dataclasses.dataclass
class RunResult:
    func: str
    c_file: str
    overlay: Optional[int]
    ok: bool  # ran without infrastructure error
    base_score: Optional[int] = None
    best_score: Optional[int] = None
    zero_found: bool = False
    promoted: bool = False
    promote_error: Optional[str] = None
    error: Optional[str] = None
    seconds: float = 0.0
    flags: Optional[str] = None  # real codegen flags the scratch compiled with
    replicated_objcopy: int = 0  # post-compile objcopy steps appended to compile.sh
    extended: bool = False  # score-trend extension run happened
    stopped_flat: bool = False  # stopped early: no improvement by --flat-minutes
    stopped_batch: bool = False  # whole-batch deadline ended this search
    commit_error: Optional[str] = None
    annotated_relocs: int = 0  # overlay target sites given symbolic relocations
    receipt_key: Optional[str] = None
    resumed: bool = False
    busy: bool = False
    deep_skipped: bool = False
    scratch_path: Optional[str] = None
    artifact_bundle: Optional[str] = None
    context_review: Optional[dict] = None
    original_base_score: Optional[int] = None
    seed_score: Optional[int] = None
    seed_parent_score: Optional[int] = None
    seed_proof: Optional[dict] = None
    search_gain: Optional[int] = None


def remaining_timeout(deadline: Optional[float], cap: float = 120) -> float:
    if CANCEL_EVENT.is_set():
        raise RuntimeError("batch cancelled")
    remaining = cap if deadline is None else min(cap, deadline - time.monotonic())
    if remaining <= 0:
        raise TimeoutError("whole-batch deadline reached")
    return remaining


def bounded_capture(args: list[str], deadline: Optional[float], *, check: bool = False,
                    cap: float = 120, env: Optional[dict] = None, cwd: Optional[Path] = None):
    """Capture a preparation command and terminate its entire group on timeout."""
    timeout = remaining_timeout(deadline, cap)
    end = time.monotonic() + timeout
    proc = subprocess.Popen(args, cwd=ROOT if cwd is None else cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True, start_new_session=True, env=env)
    finished = False
    failure = None
    partial_output = ""
    try:
        while True:
            if CANCEL_EVENT.is_set():
                raise RuntimeError("batch cancelled")
            remaining = end - time.monotonic()
            if remaining <= 0:
                raise subprocess.TimeoutExpired(args, timeout)
            try:
                output, _ = proc.communicate(timeout=min(1, remaining))
                break
            except subprocess.TimeoutExpired as pending:
                partial_output = pending.output or partial_output
                continue
        finished = True
    except BaseException as error:
        failure = error
        raise
    finally:
        try:
            if not finished or proc.returncode:
                stop_process_group(proc)
                drained, _ = proc.communicate()
                partial_output = drained or partial_output
        finally:
            if failure is not None:
                # communicate() returns cumulative output: retain the final
                # drain, not duplicated timeout snapshots, on the same error.
                failure.output = partial_output
            if proc.stdout is not None:
                proc.stdout.close()
    result = subprocess.CompletedProcess(args, proc.returncode, output, output)
    if check:
        result.check_returncode()
    return result


def stop_process_group(proc: subprocess.Popen) -> None:
    """Stop only our launched session, including workers after parent failure.

    Darwin's killpg(2) fails with EPERM, not ESRCH, when any member of the
    group cannot be signalled -- which includes a member that has exited and
    not yet been reaped. The signal still reaches every live member. So a
    search whose last worker is a zombie at the moment of cleanup raised
    PermissionError out of here and failed a finished, successful run; on a
    loaded machine that was often enough to fail tests/test_sweep_receipts
    RunnerTests intermittently (measured 2026-09-23: 3 failures in 6
    concurrent runs of the class under load, all "[Errno 1] Operation not
    permitted"). EPERM is therefore "signalled what could be signalled":
    carry on to the wait and the SIGKILL sweep, which is what ends the group.
    """
    try:
        os.killpg(proc.pid, signal.SIGTERM)
    except ProcessLookupError:
        proc.wait()
        return
    except PermissionError:
        pass
    try:
        proc.wait(timeout=15)
    except subprocess.TimeoutExpired:
        pass
    finally:
        # The parent can exit before a worker that ignores TERM. Reaping the
        # parent alone is not evidence that its process group has stopped.
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except (ProcessLookupError, PermissionError):
            pass
        proc.wait()


def sweep_tool_identity() -> dict:
    return {
        "ido": sweep_receipts.tree_digest(ROOT / "tools/ido"),
        "binutils": sweep_receipts.tree_digest(ROOT / "tools/binutils"),
        "permuter": sweep_receipts.tree_digest(PERMUTER_DIR, source_only=True),
        "runner": sweep_receipts.file_digest(Path(__file__)),
        "receipts": sweep_receipts.file_digest(Path(sweep_receipts.__file__)),
        "promotion": sweep_receipts.file_digest(Path(promotion_transaction.__file__)),
        "python": sweep_receipts.file_digest(PYTHON.resolve()),
        "python_version": sys.version,
        "candidate_context": context_tool_identity(),
        "loaded_modules": {name: value[1] for name, value in _LOADED_IMPLEMENTATIONS.items()},
    }


_CONTEXT_IDENTITY_LOCK = threading.Lock()
_CONTEXT_IDENTITY_PIN = None
_TOOL_IDENTITY_LOCK = threading.Lock()
_PROCESS_TOOLS_PIN = None


def context_tool_identity() -> dict:
    global _CONTEXT_IDENTITY_PIN
    with _CONTEXT_IDENTITY_LOCK:
        try:
            import candidate_context
            current = candidate_context.identity()
        except (ImportError, OSError) as error:
            current = {"unavailable": type(error).__name__}
        digest = sweep_receipts.digest(current)
        if _CONTEXT_IDENTITY_PIN is None:
            _CONTEXT_IDENTITY_PIN = digest
        elif _CONTEXT_IDENTITY_PIN != digest:
            raise RuntimeError("context comparator/parser changed in this process; restart the runner")
        return current


def checked_tool_identity() -> dict:
    """Loaded code cannot truthfully claim the identity of subsequently edited files."""
    global _PROCESS_TOOLS_PIN
    with _TOOL_IDENTITY_LOCK:
        for name, (path, imported_digest) in _LOADED_IMPLEMENTATIONS.items():
            if hashlib.sha256(path.read_bytes()).hexdigest() != imported_digest:
                raise RuntimeError(f"loaded {name} implementation changed; restart the runner")
        current = sweep_tool_identity()
        digest = sweep_receipts.digest(current)
        if _PROCESS_TOOLS_PIN is None:
            _PROCESS_TOOLS_PIN = digest
        elif _PROCESS_TOOLS_PIN != digest:
            raise RuntimeError("tools changed in this process; restart the runner")
        return current


@dataclasses.dataclass(frozen=True)
class PreparedBaseline:
    """Explicit immutable evidence from the first actual search compilation."""
    symbol: str
    canonical_source_sha256: str
    tools_sha256: str
    source: bytes
    object: bytes
    source_sha256: str
    object_sha256: str
    returncode: int
    recipe_json: bytes
    dependencies_json: bytes
    capture_binding_json: bytes
    prepared_inputs_json: bytes


def source_dependencies(source: Path, arguments: tuple[str, ...], deadline=None) -> dict:
    """Conservative literal include closure, with inactive/missing paths retained."""
    directories = []
    for index, argument in enumerate(arguments):
        if argument == "-I-":
            raise RuntimeError("unsupported split include search freshness")
        elif argument == "-I" and index + 1 < len(arguments):
            directories.append(ROOT / arguments[index + 1])
        elif argument.startswith("-I") and len(argument) > 2:
            directories.append(ROOT / argument[2:])
        elif argument.startswith(("-include", "-imacros", "-isystem", "-iquote", "-idirafter", "-iprefix", "-iwithprefix")):
            raise RuntimeError("unsupported forced/system include freshness")
    records, visited = {}, set()
    total_bytes = 0
    lexical = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*', re.S)
    def walk(path, initial=False):
        nonlocal total_bytes
        remaining_timeout(deadline)
        # Do not resolve away a symlink before the owned read authenticates
        # every lookup component. The spelling can affect nested includes and
        # __FILE__ even when the ultimate file bytes are identical.
        relative = path.relative_to(ROOT).as_posix()
        if path in visited:
            return
        visited.add(path)
        data = sweep_receipts.owned_bytes(ROOT.resolve(), relative, limit=4 * 1024 * 1024)
        total_bytes += len(data)
        if len(visited) > 4096 or total_bytes > 32 * 1024 * 1024:
            raise RuntimeError("include freshness closure exceeds bounded limits")
        if not initial:
            records[relative] = hashlib.sha256(data).hexdigest()
        text = data.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
        if re.search(r"\?\?[=/'()!<>-]", text):
            raise RuntimeError("cannot authenticate trigraph include freshness")
        if re.search(r"\\[ \t\v\f]+\n", text):
            raise RuntimeError("cannot authenticate dialect-dependent spaced include continuation")
        text = text.replace("\\\n", "")
        text = lexical.sub(lambda m: " " + "\n" * m.group().count("\n")
                           if m.group().startswith(("/*", "//")) else m.group(), text)
        if re.search(r"(?m)^\s*%:", text):
            raise RuntimeError("cannot authenticate digraph include directive freshness")
        for match in re.finditer(r'^\s*#\s*(include[A-Za-z_0-9]*|import)\b([^\n]*)', text, re.MULTILINE):
            if match.group(1) != "include":
                raise RuntimeError("unsupported include-family directive freshness")
            if "-nostdinc" not in arguments:
                raise RuntimeError("implicit system include search is not authenticated")
            spelling = match.group(2).strip()
            literal = re.fullmatch(r'"([^"\n]+)"|<([^>\n]+)>', spelling)
            if literal is None:
                raise RuntimeError("cannot authenticate macro include freshness")
            quoted, angled = literal.groups()
            name = quoted if quoted is not None else angled
            search = ([path.parent] if quoted is not None else []) + directories
            found = next((directory / name for directory in search if (directory / name).is_file()), None)
            if found is None:
                records[f"missing:{relative}:{spelling}"] = "absent"
            else:
                walk(found)
    walk(source, True)
    return records


def captured_baseline(item: QueueItem, out_dir: Path, inputs: dict, deadline=None) -> PreparedBaseline:
    context = inputs["context"]
    if context["identity"]["symbol"] != item.func or context["identity"]["source"] != item.rel_c_file:
        raise RuntimeError("prepared baseline belongs to another symbol/source")
    capture = out_dir / "baseline-capture"
    metadata = json.loads(sweep_receipts.owned_bytes(capture, "capture.json"))
    binding = {"inputs_sha256": sweep_receipts.digest(inputs), "run_id": out_dir.name}
    if metadata.get("binding") != binding:
        raise RuntimeError("actual compiler capture belongs to another prepared run")
    evidence = PreparedBaseline(item.func, context["source"], sweep_receipts.digest(context["tools"]),
        sweep_receipts.owned_bytes(capture, "compiled.c", limit=4 * 1024 * 1024),
        sweep_receipts.owned_bytes(capture, "compiled.o"), metadata["source_sha256"],
        metadata["object_sha256"], metadata["returncode"],
        json.dumps(context["recipe"], sort_keys=True).encode(),
        json.dumps(context["dependencies"], sort_keys=True).encode(),
        json.dumps(binding, sort_keys=True).encode(),
        json.dumps(inputs, sort_keys=True).encode())
    validate_baseline(item, evidence, deadline)
    return evidence


def validate_baseline(item: QueueItem, evidence: PreparedBaseline | None, deadline=None) -> None:
    if not isinstance(evidence, PreparedBaseline) or evidence.symbol != item.func:
        raise RuntimeError("missing authenticated prepared baseline for this symbol")
    binding = json.loads(evidence.capture_binding_json)
    inputs = json.loads(evidence.prepared_inputs_json)
    context = inputs.get("context") if isinstance(inputs, dict) else None
    if (not isinstance(binding, dict) or not isinstance(context, dict)
            or binding.get("inputs_sha256") != sweep_receipts.digest(inputs)
            or not isinstance(binding.get("run_id"), str)
            or re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,127}", binding["run_id"]) is None
            or not isinstance(context.get("identity"), dict)
            or context["identity"].get("symbol") != item.func
            or context["identity"].get("source") != item.rel_c_file
            or context.get("source") != evidence.canonical_source_sha256
            or sweep_receipts.digest(context.get("tools")) != evidence.tools_sha256
            or json.dumps(context.get("recipe"), sort_keys=True).encode() != evidence.recipe_json
            or json.dumps(context.get("dependencies"), sort_keys=True).encode() != evidence.dependencies_json):
        raise RuntimeError("prepared capture binding is missing or belongs to another context")
    if (type(evidence.returncode) is not int or evidence.returncode != 0 or not evidence.object
            or hashlib.sha256(evidence.source).hexdigest() != evidence.source_sha256
            or hashlib.sha256(evidence.object).hexdigest() != evidence.object_sha256):
        raise RuntimeError("first actual compiler capture is unsuccessful or digest-mismatched")
    if sweep_receipts.file_digest(item.c_file) != evidence.canonical_source_sha256:
        raise RuntimeError("canonical TU changed since initial prepared baseline")
    if sweep_receipts.digest(checked_tool_identity()) != evidence.tools_sha256:
        raise RuntimeError("tools changed since initial prepared baseline")
    remaining_timeout(deadline)
    recipe = build_recipe_for(item.c_file, deadline)
    if json.dumps(dataclasses.asdict(recipe), sort_keys=True).encode() != evidence.recipe_json:
        raise RuntimeError("configured recipe changed since initial prepared baseline")
    if json.dumps(source_dependencies(item.c_file, recipe.compiler_args, deadline), sort_keys=True).encode() != evidence.dependencies_json:
        raise RuntimeError("header context changed since initial prepared baseline")


def review_context(item: QueueItem, evidence: PreparedBaseline | None, winner: bytes, deadline=None) -> dict:
    try:
        comparator_identity = context_tool_identity()
    except RuntimeError as error:
        comparator_identity = {"unavailable": str(error)}
    report = {"schema": "mickey-prepared-context-review-v1", "status": "unverifiable",
              "symbol": item.func, "canonical_source_sha256": getattr(evidence, "canonical_source_sha256", None),
              "baseline_source_sha256": hashlib.sha256(evidence.source).hexdigest() if evidence else None,
              "winner_source_sha256": hashlib.sha256(winner).hexdigest(),
              "capture_binding": json.loads(evidence.capture_binding_json) if evidence else None,
              "comparator_identity": comparator_identity, "comparison": None, "reason": None}
    try:
        validate_baseline(item, evidence, deadline)
        import candidate_context
        comparison = candidate_context.compare_context(evidence.source, winner, item.func)
        if (comparison.get("schema") != "mickey-candidate-context-v1"
                or comparison.get("symbol") != item.func
                or comparison.get("baseline_sha256") != report["baseline_source_sha256"]
                or comparison.get("winner_sha256") != report["winner_source_sha256"]
                or comparison.get("status") not in {"unchanged", "changed", "unverifiable"}):
            raise RuntimeError("context comparator returned unbound or malformed evidence")
        report.update(status=comparison["status"], comparison=comparison, reason=comparison.get("reason"))
    except Exception as error:
        report["reason"] = str(error)
    return report


def retain_context(directory: Path, evidence: PreparedBaseline | None, winner: bytes, report: dict) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    if evidence is not None:
        (directory / "baseline.c").write_bytes(evidence.source)
        (directory / "baseline.o").write_bytes(evidence.object)
        (directory / "prepared-inputs.json").write_bytes(evidence.prepared_inputs_json)
        sweep_receipts.atomic_json(directory / "capture.json", {
            "returncode": evidence.returncode, "source_sha256": evidence.source_sha256,
            "object_sha256": evidence.object_sha256,
            "binding": json.loads(evidence.capture_binding_json)})
    (directory / "winner.c").write_bytes(winner)
    sweep_receipts.atomic_json(directory / "report.json", report)


def require_search_context(item, evidence, directory, deadline):
    """Retain an authenticated self-comparison before spending random-search time."""
    report = review_context(item, evidence, evidence.source, deadline)
    retain_context(directory, evidence, evidence.source, report)
    if report["status"] != "unchanged":
        raise RuntimeError("baseline context readiness refused: " + str(report.get("reason")))
    # review_context records errors; recheck outside its report boundary too.
    validate_baseline(item, evidence, deadline)
    return report


def compile_script_digest(raw: bytes) -> str:
    """Normalize generated lane plumbing, never compiler arguments/literals."""
    text = raw.decode("utf-8")
    text = text.replace("\ncd " + shlex.quote(str(ROOT)) + "\n", "\ncd .\n")
    text = re.sub(r"(?m)^" + re.escape(str(ROOT / "tools/binutils/mips64-elf-objcopy")) + r"(?= )",
                  "tools/binutils/mips64-elf-objcopy", text)
    return sweep_receipts.digest(text)


def receipt_inputs(item: QueueItem, scratch: Path, settings: Path, target: Path,
                   recipe: BuildRecipe, search: dict) -> dict:
    """Fingerprint the actual importer output, not guessed header dependencies.

    Preparing a resumed entry still preprocesses and compiles its baseline.
    That small cost proves its effective source and relocation annotation are
    unchanged before skipping a much longer search. Source, target and settings
    bytes are never rewritten for hashing: path literals can affect codegen.
    """
    if not recipe.from_dry_run or not recipe.compiler_args:
        raise RuntimeError("cannot identify a sweep with fallback compile flags")
    raw = target.read_text()
    addresses = re.findall(r"^\s*/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{8})\s+", raw, re.MULTILINE)
    if not addresses:
        raise ValueError("target has no ROM/text address identity")
    rom, vma = (int(value, 16) for value in addresses[0])
    offset = vma - 0xF0000000 if item.overlay is not None else vma
    if offset < 0:
        raise ValueError("overlay target has an invalid synthetic address")

    def compile_digest(path: Path) -> str:
        # Only importer-generated cwd and our executable command prefixes are
        # path plumbing. Never substitute inside source, -D values, shell
        # strings, include arguments or arbitrary user commands.
        return compile_script_digest(path.read_bytes())

    context = {
        "preparation_contract": SOURCE_GROUP_CONTRACT,
        "importer_recipe": compile_digest(scratch.parent / "importer-compile.sh"),
        "source_group_plan": sweep_receipts.file_digest(scratch.parent / "source-groups.json"),
        "identity": {"symbol": item.func, "source": item.rel_c_file,
                     "overlay": item.overlay, "section": ".text", "offset": offset,
                     "rom_offset": rom},
        "source": sweep_receipts.file_digest(item.c_file),
        "prepared": {name: compile_digest(scratch / name) if name == "compile.sh"
                     else sweep_receipts.file_digest(scratch / name)
                     for name in ("base.c", "compile.sh", "target.s", "settings.toml")},
        "settings": sweep_receipts.file_digest(settings),
        "recipe": json.loads(json.dumps(dataclasses.asdict(recipe))),
        "tools": sweep_tool_identity(),
        "dependencies": source_dependencies(item.c_file, recipe.compiler_args),
    }
    # Whole-ROM hash is cheap here and also pins the relocation metadata read
    # by annotation. The receipt never contains ROM bytes.
    if item.overlay is not None:
        context["rom"] = sweep_receipts.file_digest(BASEROM)
    baseline_hashes = {"baseline/" + name: sweep_receipts.file_digest(scratch / name)
                       for name in ("base.c", "compile.sh", "target.s", "settings.toml")}
    # base.o is importer/annotation OUTPUT, not a search input. IDO embeds
    # the unique preparation path in .mdebug, so hashing it prevents identical
    # searches from resuming. It remains preserved and independently verified
    # by the bundle's member hash; search source/recipe/target are pinned here.
    baseline_hashes.update({"baseline/tu.c": context["source"],
                           "baseline/permuter_settings.toml": context["settings"],
                           "baseline/recipe.json": hashlib.sha256(
                               json.dumps(dataclasses.asdict(recipe), sort_keys=True).encode()).hexdigest()})
    context["baseline_hashes"] = baseline_hashes
    return {"schema": sweep_receipts.SCHEMA, "context": context, "search": search,
            "baseline_hashes": baseline_hashes}


def write_settings_toml(out_path: Path, flags: tuple[str, ...],
                        recipe: Optional[BuildRecipe] = None) -> None:
    # The optional legacy form remains available to standalone callers. The
    # batch runner always passes its recovered complete recipe.
    args = list(recipe.compiler_args) if recipe and recipe.compiler_args else [
        *shlex.split(BASE_CC_ARGS), *shlex.split(INCLUDES), *flags]
    compiler_command = shlex.join(["tools/ido/cc", *args, "-DNON_MATCHING"])
    text = (
        'compiler_type = "ido"\n'
        f"compiler_command = {json.dumps(compiler_command)}\n"
        f'assembler_command = "{ASSEMBLER_COMMAND}"\n\n'
        'objdump_command = "tools/binutils/mips64-elf-objdump -drz -m mips:4300"\n\n'
        f"{PRESERVE_MACROS}\n"
        "[decompme.compilers]\n"
        '"tools/ido/cc" = "ido5.3"\n'
    )
    out_path.write_text(text)


def prepare_target_asm(item: QueueItem, out_dir: Path) -> Path:
    """Copy the function's target .s into the scratch area, renaming its
    glabel/endlabel pair from splat's ROM-derived auto name to the
    friendly C name import.py expects to find. See module docstring:
    "THE OVERLAY NAMING QUIRK". A pure label rename -- metadata, never an
    instruction word -- and never written back into asm/ itself."""
    asm_rel = find_asm_target(item)
    if asm_rel is None:
        raise RuntimeError(
            f"no #pragma GLOBAL_ASM(...) found for {item.func} in {item.rel_c_file}"
        )
    asm_path = ROOT / asm_rel
    if not asm_path.is_file():
        raise RuntimeError(
            f"target asm {asm_rel} does not exist -- run `gmake extract` first"
        )
    text = asm_path.read_text(errors="replace")
    labels = [m.group(2) for m in GLABEL_RE.finditer(text)]
    label_set = set(labels)
    # The file stem is splat's symbol for the owned function. Auxiliary
    # labels (local branches and jump-table/data labels) must retain their
    # identities and relocations. Fall back to the C symbol for resident
    # assembly that already uses its friendly name.
    if asm_path.stem in label_set:
        auto_name = asm_path.stem
    elif item.func in label_set:
        auto_name = item.func
    else:
        raise RuntimeError(
            f"{asm_rel}: neither target stem {asm_path.stem!r} nor "
            f"C symbol {item.func!r} is defined"
        )
    if auto_name != item.func:
        text = re.sub(
            r"\b" + re.escape(auto_name) + r"\b", item.func, text
        )
    target = out_dir / "target.s"
    target.write_text(text)
    return target


def validate_annotation_target(target: Path, notes: list[str], out_dir: Path, deadline=None):
    """Prove target metadata and all owned words through an ordinary diagnostic link."""
    reports = [json.loads(note[len("target-proof: "):]) for note in notes if note.startswith("target-proof: ")]
    if len(reports) != 1:
        raise RuntimeError("annotation lacks one complete target-proof description")
    proof = reports[0]
    elf = reloc_surface.Elf(target)
    actual = sorted([[offset, kind] for _section, offset, kind, _symbol in elf.relocations()])
    if actual != proof["records"]:
        raise RuntimeError("assembled target static relocation coverage differs from runtime records")
    linked = out_dir / ("annotation-proof-" + uuid.uuid4().hex + ".elf")
    arguments = [str(ROOT / "tools/binutils/mips64-elf-ld"), "-m", "elf32ebmip",
                 "-Ttext", "0", "-e", "0", "-o", str(linked), str(target)]
    for name, value in sorted(proof["values"].items()):
        if re.fullmatch(r"__ov[A-Za-z0-9_]+", name) is None or type(value) is not int:
            raise RuntimeError("invalid diagnostic target stored-value assignment")
        arguments.extend(["--defsym", f"{name}=0x{value:08X}"])
    command = bounded_capture(arguments, deadline, check=True)
    (out_dir / "annotation-proof.log").write_text(command.stdout)
    actual = reloc_surface.Elf(linked).section_bytes(".text")[:proof["size"]]
    expected = BASEROM.read_bytes()[proof["rom_start"]:proof["rom_start"] + proof["size"]]
    if len(actual) != proof["size"] or actual != expected:
        raise RuntimeError("annotated target operands do not reconstruct exact owned ROM bytes")


def annotation_aliases(renames: dict[str, str], alias_history, symbols,
                       symbol_sections=None) -> dict[str, str]:
    """Carry proved identities through simultaneous invocations in execution order."""
    groups = [group if isinstance(group, dict) else {"renames": group, "additions": (), "removals": ()}
              for group in alias_history]
    pairs = [pair for group in groups for pair in group["renames"]]
    closure = reloc_surface.ri.canonicalize_redefine_aliases(pairs)
    if closure.cycles or any(
            None in {renames.get(source) for source in sources}
            or len({renames.get(source) for source in sources}) != 1
            for _destination, sources in closure.conflicts):
        raise RuntimeError("ambiguous scratch objcopy alias provenance")
    current = {name: name for name in symbols if name}
    sections = dict(symbol_sections or {})
    def require_unambiguous(values):
        origins = {}
        for original, name in values.items():
            origins.setdefault(name, []).append(original)
        for originals in origins.values():
            if len(originals) > 1:
                identities = {renames.get(original) for original in originals}
                if None in identities or len(identities) != 1:
                    raise RuntimeError("scratch objcopy aliases merge unproved or distinct runtime identities")
    for index, group in enumerate(groups):
        mapping = dict(group["renames"])
        if len(mapping) != len(group["renames"]):
            raise RuntimeError("duplicate scratch objcopy source alias")
        if group["removals"]:
            if symbol_sections is None or any(section in {".text", ".rel.text"} for section in group["removals"]):
                raise RuntimeError("cannot preserve owned annotation surface through section removal")
            current = {original: name for original, name in current.items()
                       if sections.get(original) not in group["removals"]}
        current = {original: mapping.get(name, name) for original, name in current.items()}
        # Objcopy appends new symbols after renaming the input symbols. Their
        # scalar values do not authenticate a preexisting runtime identity.
        for ordinal, (name, section) in enumerate(group["additions"]):
            if section in group["removals"]:
                raise RuntimeError("added symbol belongs to a removed section")
            origin = ("added", index, ordinal, name)
            current[origin], sections[origin] = name, section
        require_unambiguous(current)
    composed = {}
    for original, destination in renames.items():
        if original not in current:
            raise RuntimeError("annotation rename lacks original candidate symbol")
        name = current[original]
        if name in composed and composed[name] != destination:
            raise RuntimeError("scratch aliases collapse distinct runtime identities")
        composed[name] = destination
    require_unambiguous({original: composed.get(name, name) for original, name in current.items()})
    return composed


def annotate_overlay_scratch(item: QueueItem, scratch: Path, out_dir: Path,
                             batch_deadline: Optional[float] = None,
                             alias_history=()) -> int:
    """Annotate owned scratch transactionally; retain evidence and never rebuild to undo."""
    if item.overlay is None or not BASEROM.is_file():
        return 0
    paths = [scratch / name for name in ("target.s", "target.o", "compile.sh", "base.o")]
    if not paths[0].is_file() or not paths[3].is_file():
        return 0
    remaining_timeout(batch_deadline)
    snapshots = {}
    for path in paths:
        if path.is_symlink() or (path.exists() and not path.is_file()):
            raise RuntimeError("annotation refuses non-regular scratch file: " + str(path))
        snapshots[path] = (path.read_bytes(), path.stat().st_mode & 0o7777) if path.exists() else None
    attempt = out_dir / ("annotation-attempt-" + uuid.uuid4().hex)
    attempt.mkdir()
    for path, saved in snapshots.items():
        if saved is not None:
            (attempt / ("before-" + path.name)).write_bytes(saved[0])
    (attempt / "before.json").write_text(json.dumps({path.name: None if saved is None else {
        "mode": saved[1], "sha256": hashlib.sha256(saved[0]).hexdigest()}
        for path, saved in snapshots.items()}, sort_keys=True))
    diagnostic = out_dir / "annotation.txt"
    if diagnostic.is_file():
        (attempt / "before-annotation.txt").write_bytes(diagnostic.read_bytes())
    try:
        result = _annotate_overlay_scratch(item, scratch, out_dir, batch_deadline, alias_history)
        remaining_timeout(batch_deadline)
        return result
    except BaseException as error:
        details = ["not annotated: target proof failed or scratch preparation failed",
                   type(error).__name__ + ": " + str(error)]
        for field in ("output", "stderr"):
            output = getattr(error, field, None)
            if output:
                details.append(field + ": " + (output.decode(errors="replace") if isinstance(output, bytes) else str(output)))
        recovery_errors = []
        # Evidence capture must not prevent restoration, even if its write fails.
        for path, saved in snapshots.items():
            try:
                if path.is_file() and not path.is_symlink():
                    (attempt / ("failed-" + path.name)).write_bytes(path.read_bytes())
            except OSError as failure:
                recovery_errors.append("evidence " + path.name + ": " + str(failure))
            try:
                if saved is None:
                    if path.exists() or path.is_symlink():
                        path.unlink()
                else:
                    temporary = path.with_name(".annotation-restore-" + uuid.uuid4().hex)
                    temporary.write_bytes(saved[0])
                    temporary.chmod(saved[1])
                    os.replace(temporary, path)
            except OSError as failure:
                recovery_errors.append("restore " + path.name + ": " + str(failure))
        details.extend(recovery_errors)
        message = "\n".join(details) + "\n"
        try:
            (attempt / "failure.txt").write_text(message)
            diagnostic.write_text(message + "evidence: " + str(attempt) + "\n")
        except OSError as failure:
            recovery_errors.append("diagnostic: " + str(failure))
        if recovery_errors:
            raise RuntimeError("annotation recovery needs review: " + str(attempt) + "; "
                               + "; ".join(recovery_errors)) from error
        if (not isinstance(error, Exception) or isinstance(error, (TimeoutError, subprocess.TimeoutExpired))
                or CANCEL_EVENT.is_set()):
            raise
        remaining_timeout(batch_deadline)  # cancellation/deadline remain terminal after restoration
        return 0


def _annotate_overlay_scratch(item: QueueItem, scratch: Path, out_dir: Path,
                              batch_deadline: Optional[float] = None,
                              alias_history=()) -> int:
    """Give an overlay function's permuter target the relocations the shipped
    module says are there, and rename the candidate's placeholders to match.

    Without this the target .s assembles with no relocations at all -- the
    module ships unrelocated, so splat's disassembly carries stored addends,
    not addresses -- while the candidate object carries honest `R_MIPS_26` and
    `%hi`/`%lo` references to placeholder externs.  decomp-permuter's scorer
    ignores a symbol-name difference only when the *target* line also carries
    a relocation, so every relocation site in an overlay function was scored
    as a full insertion+deletion pair and an overlay candidate two words from
    the ROM scored in the hundreds.  See tools/reloc_surface.py's
    `permuter_annotation` for the derivation and docs/reloc-surface.md for the
    model it rests on.

    Returns the number of placeholder symbols renamed; zero can still mean
    complete, independently proved target-only annotation.
    Any failure leaves the scratch exactly as import.py wrote it: an
    unannotated overlay run is the previous behaviour, not a broken one.
    """
    if item.overlay is None or not BASEROM.is_file():
        return 0
    target_s = scratch / "target.s"
    base_o = scratch / "base.o"
    if not target_s.is_file() or not base_o.is_file():
        return 0
    asm_rel = find_asm_target(item)
    names = [item.func]
    if asm_rel:
        names.append(Path(asm_rel).stem)
    original = target_s.read_text(errors="replace")
    text, renames, notes = reloc_surface.permuter_annotation(
        original, base_o, names, item.overlay, BASEROM.read_bytes())
    if alias_history and renames:
        base_elf = reloc_surface.Elf(base_o)
        renames = annotation_aliases(renames, alias_history,
                                    [row[0] for row in base_elf.symbols()],
                                    {row[0]: base_elf.names[row[4]] if 0 < row[4] < len(base_elf.names) else None
                                     for row in base_elf.symbols()})
    if text == original:
        (out_dir / "annotation.txt").write_text(
            "not annotated: no site the module relocation table names\n"
            + "\n".join(notes) + "\n")
        return 0
    target_s.write_text(text)
    # The annotated target must still assemble -- that is the check that the
    # rewritten operands are real assembler syntax, not just plausible text.
    bounded_capture(
        shlex.split(ASSEMBLER_COMMAND) + [str(target_s), "-o", str(scratch / "target.o")],
        batch_deadline, check=True)
    validate_annotation_target(scratch / "target.o", notes, out_dir, batch_deadline)
    csh = scratch / "compile.sh"
    if csh.is_file() and renames:
        # objcopy refuses two --redefine-sym arguments that share a target
        # name, so aliases of one link value (two externs the surface values
        # identically) go in successive invocations rather than one.
        rounds: list[dict[str, str]] = []
        for old_name, new_name in sorted(renames.items()):
            for group in rounds:
                if new_name not in group.values():
                    group[old_name] = new_name
                    break
            else:
                rounds.append({old_name: new_name})
        with open(csh, "a") as f:
            # replicate_objcopy only terminates its last line when it wrote
            # one at all; a TU whose POSTPROCESS is unreplicable leaves
            # compile.sh ending mid-line, and an appended command would be
            # swallowed by the compiler invocation above it.
            if not csh.read_text().endswith("\n"):
                f.write("\n")
            for group in rounds:
                args = " ".join(f"--redefine-sym {o}={n}"
                                for o, n in sorted(group.items()))
                f.write(f'{OBJCOPY} {args} "$OUTPUT"\n')
        # Refresh base.o through the amended recipe so the scratch's own base
        # object carries the canonical names too.
        bounded_capture(["bash", str(csh), str(scratch / "base.c"), "-o", str(base_o)],
                        batch_deadline, check=True)
    (out_dir / "annotation.txt").write_text(
        "\n".join(notes + [f"{k} -> {v}" for k, v in sorted(renames.items())]) + "\n")
    return len(renames)


SOURCE_GROUP_CONTRACT = "original-coordinate-sameline-v1"


def preserve_source_groups(ast, source: str, symbol: str, nodes) -> dict:
    """Retain supported original statement groups, before importer serialization.

    Only an isolated importer calls this; vendor modules and canonical C are
    never modified. Coordinates must identify one physical preprocessed line.
    Unsupported same-line control shapes fail closed instead of being joined.
    """
    functions = [n for n in ast.ext if isinstance(n, nodes.FuncDef)
                 and n.decl.name == symbol]
    if len(functions) != 1:
        raise ValueError("source grouping requires one selected function")
    locations = {}
    filename, logical = "<source>", 1
    for physical, line in enumerate(source.splitlines(), 1):
        directive = re.fullmatch(r'\s*#\s*(?:line\s+)?(\d+)(?:\s+"([^"\n]+)")?(?:\s+\d+)*\s*', line)
        if directive:
            logical = int(directive[1])
            filename = directive[2] or filename
            continue
        locations.setdefault((filename, logical), []).append(physical)
        logical += 1

    def key(node):
        coord = node.coord
        if coord is None:
            raise ValueError("source grouping has missing coordinates")
        result = (coord.file, coord.line)
        if len(locations.get(result, [])) != 1:
            raise ValueError(f"source grouping has ambiguous line coordinates: {result!r}")
        return result

    def single_line(node, line):
        if isinstance(node, (nodes.Typename, nodes.TypeDecl)) and (
                node.coord is None or node.coord.line == 0):
            return bool(node.children()) and all(single_line(c, line) for _, c in node.children())
        return key(node) == line and all(single_line(c, line) for _, c in node.children())

    def simple(node, line):
        if not isinstance(node, (nodes.Assignment, nodes.FuncCall, nodes.UnaryOp,
                                 nodes.Return, nodes.Break, nodes.Continue,
                                 nodes.EmptyStatement)):
            return False
        return single_line(node, line)

    # Compound coordinates mark only the opening brace. Retain lexical closing
    # lines too, so `} next_statement;` cannot disappear from group detection.
    lexical = "\n".join("" if line.lstrip().startswith("#") else line
                        for line in source.splitlines())
    openings, closing, stack, statement_ends = {}, {}, [], {}
    closing_offsets = {}
    parentheses = 0
    position, physical_line = 0, 1
    token_re = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*|[{}();]', re.S)
    for token in token_re.finditer(lexical):
        physical_line += lexical.count("\n", position, token.start())
        position = token.start()
        if token[0] == "{":
            openings.setdefault(physical_line, []).append(token.start())
            stack.append(token.start())
        elif token[0] == "}":
            if not stack:
                raise ValueError("ambiguous original compound boundary")
            opening = stack.pop()
            closing[opening] = physical_line
            closing_offsets[opening] = token.start()
        elif token[0] == "(":
            parentheses += 1
        elif token[0] == ")":
            parentheses -= 1
        elif token[0] == ";" and parentheses == 0:
            column = token.start() - lexical.rfind("\n", 0, token.start())
            statement_ends.setdefault(physical_line, []).append(column)

    inline_compounds = set()
    inline_do_controls = set()
    standalone_inline_do = set()

    def whole_line_do(node):
        """Bind a complete standalone do statement to one physical line.

        Nested braces have no reliable parser columns. Compare every maximal
        C token of the existing AST with the entire original physical line;
        do not infer an opener or absorb a neighboring statement. Unsupported
        spellings remain measurement-required, with the original AST intact.
        """
        if not isinstance(node.stmt, nodes.Compound):
            return False
        line = key(node)
        if not single_line(node, line):
            return False
        pending = [node]
        while pending:
            current = pending.pop()
            if isinstance(current, (nodes.Pragma, nodes.Label, nodes.Case,
                                    nodes.Default, nodes.Switch)):
                return False
            pending.extend(child for _, child in current.children())
        from perm_pycparser import c_generator
        maximal = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*'
                             r'|[A-Za-z_][A-Za-z_0-9]*|(?:\d|\.\d)[A-Za-z_0-9.]*'
                             r'|>>=|<<=|\.\.\.|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[+*/%&|^!-]='
                             r'|[^\s]', re.S)
        def tokens(text):
            return [m.group() for m in maximal.finditer(text)
                    if not m.group().startswith(("/*", "//"))]
        physical = source.splitlines()[locations[line][0] - 1]
        return tokens(physical) == tokens(c_generator.CGenerator().visit(node))

    def lexical_lines(n):
        if id(n) in standalone_inline_do:
            return locations[key(n)]
        values = []
        if n.coord is not None and n.coord.line > 0:
            values.extend(locations.get(key(n), []))
        if isinstance(n, nodes.Compound):
            candidates = openings.get(locations[key(n)][0], [])
            if len(candidates) != 1 or candidates[0] not in closing:
                raise ValueError("ambiguous original compound boundary")
            values.append(closing[candidates[0]])
        for _, child in n.children():
            values.extend(lexical_lines(child))
        return values

    def whole_line_compound(node):
        line = key(node)
        physical = locations[line][0]
        # The vendor parser records compound column 1, not the brace column.
        # Require a unique physical opener instead of inventing a column map.
        candidates = openings.get(physical, [])
        return (len(candidates) == 1 and closing.get(candidates[0]) == physical
                and bool(node.block_items)
                and all((simple(item, line) or isinstance(item, nodes.Decl)
                         and single_line(item, line)) for item in node.block_items))

    def groupable(node, line):
        return (id(node) in inline_compounds or id(node) in standalone_inline_do
                or simple(node, line))

    def do_tail_line(node):
        """Prove the complete `} while (...);` punctuation is on one line."""
        if not isinstance(node.stmt, nodes.Compound):
            return None
        candidates = openings.get(locations[key(node.stmt)][0], [])
        if len(candidates) != 1 or candidates[0] not in closing_offsets:
            return None
        close = closing_offsets[candidates[0]]
        tail = lexical[close + 1:]
        prefix = re.match(r"[^\S\n]*while[^\S\n]*\(", tail)
        if prefix is None:
            return None
        depth = 1
        for token in token_re.finditer(tail, prefix.end()):
            if token[0] == "(":
                depth += 1
            elif token[0] == ")":
                depth -= 1
                if depth == 0:
                    end = re.match(r"[^\S\n]*;", tail[token.end():])
                    if end is not None and "\n" not in tail[:token.end() + end.end()]:
                        return closing[candidates[0]]
                    return None
        return None

    def inline_do(node, line):
        return (isinstance(node, nodes.DoWhile) and key(node) == line
                and single_line(node.cond, line)
                and do_tail_line(node) == locations[line][0]
                and isinstance(node.stmt, nodes.Compound)
                and key(node.stmt) == line and whole_line_compound(node.stmt))

    def tail_composites(items):
        """Recognize one physical do-tail plus complete following siblings."""
        found, covered = {}, set()
        i = 0
        while i + 1 < len(items):
            left = items[i]
            if not isinstance(left, nodes.DoWhile) or not isinstance(left.stmt, nodes.Compound):
                i += 1
                continue
            line = key(items[i + 1])
            physical = locations[line][0]
            body = left.stmt
            candidates = openings.get(locations[key(body)][0], [])
            if (key(left) == line or len(candidates) != 1
                    or closing.get(candidates[0]) != physical
                    or do_tail_line(left) != physical
                    or not single_line(left.cond, line)
                    or not body.block_items
                    or max(value for child in body.block_items for value in lexical_lines(child)) >= physical):
                i += 1
                continue
            end = i + 1
            while end < len(items) and key(items[end]) == line:
                node = items[end]
                if not (simple(node, line) or inline_do(node, line)):
                    raise ValueError("unsupported do-tail composite sibling")
                if simple(node, line) and not any(column >= node.coord.column for column in
                                                 statement_ends.get(physical, [])):
                    raise ValueError("incomplete do-tail composite sibling endpoint")
                if isinstance(node, nodes.DoWhile):
                    inline_do_controls.add(id(node))
                end += 1
            found[i] = (end, line)
            covered.update((id(a), id(b)) for a, b in zip(items[i:end], items[i + 1:end]))
            i = end
        return found, covered

    groups = []
    def walk(node, parent=None):
        if isinstance(node, nodes.Pragma) and "_permuter" in node.string:
            raise ValueError("preexisting permuter pragma in selected function")
        if id(node) in inline_do_controls:
            return
        if (isinstance(node, nodes.DoWhile) and isinstance(parent, nodes.Compound)
                and whole_line_do(node)):
            standalone_inline_do.add(id(node))
            return
        # A macro-expanded standalone block may include declarations and its
        # closing brace on one physical line. Wrap the entire existing block
        # from its parent's statement list; its lexical scope stays intact.
        # Partial openers and control-owned bodies still request measurement.
        if (isinstance(node, nodes.Compound) and isinstance(parent, nodes.Compound)
                and whole_line_compound(node)):
            inline_compounds.add(id(node))
            return
        # Groups spanning a control/label boundary are not sibling runs. Do
        # not silently classify their original layout as ungrouped merely
        # because the generator will move the child to a different line.
        if isinstance(node, (nodes.If, nodes.For, nodes.While, nodes.DoWhile,
                             nodes.Switch, nodes.Label)):
            branches = ([node.iftrue, node.iffalse] if isinstance(node, nodes.If)
                        else [node.stmt])
            for branch in branches:
                if (branch is not None and not isinstance(branch, nodes.Compound)
                        and key(branch) == key(node)):
                    raise ValueError("unsupported same-line unbraced control group")
        if isinstance(node, (nodes.Case, nodes.Default)):
            statements = node.stmts or []
            if (statements and key(statements[0]) == key(node)) or any(
                    key(a) == key(b) for a, b in zip(statements, statements[1:])):
                raise ValueError("unsupported same-line case group")
        composites, composite_pairs = (tail_composites(node.block_items or [])
                                       if isinstance(node, nodes.Compound) else ({}, set()))
        for _, child in list(node.children()):
            walk(child, node)
        if not isinstance(node, nodes.Compound):
            return
        items = node.block_items or []
        if items and key(items[0]) == key(node):
            raise ValueError("unsupported same-line compound opener group")
        for left, right in zip(items, items[1:]):
            if (id(left), id(right)) in composite_pairs:
                continue
            if key(left) != key(right):
                # Parser children omit closing parentheses and semicolons.
                # A preceding real terminator on the next statement's line
                # proves an unpreserved endpoint group, regardless of AST kind.
                if any(column < right.coord.column for column in
                       statement_ends.get(locations[key(right)][0], [])):
                    raise ValueError("same-line lexical statement endpoint requires measurement")
                if max(lexical_lines(left)) >= locations[key(right)][0]:
                    raise ValueError("multiline statement overlaps next statement group")
        output, i = [], 0
        while i < len(items):
            if i in composites:
                end, line = composites[i]
                # Enter immediately before the previous body's closing brace;
                # the vendor then joins its while and all following siblings.
                items[i].stmt.block_items.append(nodes.Pragma("_permuter sameline start", coord=items[i].cond.coord))
                output.extend(items[i:end])
                output.append(nodes.Pragma("_permuter sameline end", coord=items[end - 1].coord))
                groups.append({"line": line[1], "statements": end - i,
                               "control": "DoWhileTailComposite"})
                i = end
                continue
            first = items[i]
            line = key(first)
            end = i + 1
            while end < len(items) and key(items[end]) == line:
                end += 1
            if (end == i + 1 and id(first) not in inline_compounds
                    and id(first) not in standalone_inline_do):
                output.append(first)
                i = end
                continue
            batch = items[i:end]
            tail = batch[-1]
            inner = None
            if not all(groupable(n, line) for n in batch):
                if (not all(groupable(n, line) for n in batch[:-1])
                        or not isinstance(tail, (nodes.DoWhile, nodes.If))):
                    raise ValueError("unsupported same-line statement group")
                inner = tail.stmt if isinstance(tail, nodes.DoWhile) else tail.iftrue
                if (not isinstance(inner, nodes.Compound) or key(inner) != line
                        or not inner.block_items
                        or key(inner.block_items[0]) == line):
                    raise ValueError("unsupported same-line control body")
                if isinstance(tail, nodes.If):
                    def condition_same(n):
                        return key(n) == line and all(condition_same(c) for _, c in n.children())
                    if not condition_same(tail.cond):
                        raise ValueError("multiline same-line condition")
            start = nodes.Pragma("_permuter sameline start", coord=first.coord)
            stop = nodes.Pragma("_permuter sameline end", coord=tail.coord)
            output.extend([start, *batch])
            if inner is None:
                output.append(stop)
            else:
                inner.block_items.insert(0, stop)
            groups.append({"line": line[1], "statements": len(batch),
                           "control": (type(tail).__name__ if inner else
                                       "DoWhile" if any(id(n) in standalone_inline_do for n in batch) else
                                       "Compound" if any(id(n) in inline_compounds for n in batch) else None)})
            i = end
        node.block_items = output
    walk(functions[0].body)
    return {"contract": SOURCE_GROUP_CONTRACT, "symbol": symbol, "groups": groups}


IDO_IMPORT_TYPE_OPERATORS = frozenset({"__builtin_classof", "__builtin_alignof"})


@contextlib.contextmanager
def ido_import_parser():
    """Parse IDO type operators without evaluating or hiding their operands.

    Only the importer child uses this dialect. Normal extraction may remove
    unrelated function bodies; retained operators require broader parser support
    and are rejected before prepared source can reach the baseline compiler.
    The existing grammar/table handles the same type-only production as alignof.
    """
    from src import ast_util
    from perm_pycparser import c_ast
    from perm_pycparser.c_lexer import CLexer
    from perm_pycparser.c_parser import CParser

    class TypeLexer(CLexer):
        keyword_map = dict(CLexer.keyword_map, **{
            name: "_ALIGNOF" for name in IDO_IMPORT_TYPE_OPERATORS})

    class TypeParser(CParser):
        def __init__(self):
            super().__init__(lexer=TypeLexer)

        def p_unary_expression_3(self, production):
            if production[1] in IDO_IMPORT_TYPE_OPERATORS:
                if len(production) != 5 or not isinstance(production[3], c_ast.Typename):
                    raise ValueError("IDO import operator requires an explicit type operand")
                production[0] = c_ast.UnaryOp(production[1], production[3],
                                              self._token_coord(production, 1))
            else:
                super().p_unary_expression_3(production)

    # Keep the vendor grammar and cached parse table exactly unchanged, while
    # binding this one semantic action to the original operator spelling.
    TypeParser.p_unary_expression_3.__doc__ = CParser.p_unary_expression_3.__doc__
    original = ast_util.CParser
    ast_util.CParser = TypeParser
    try:
        yield
    finally:
        ast_util.CParser = original


def reject_retained_ido_operators(node):
    """Never send importer-only syntax into unextended candidate/context parsers."""
    from perm_pycparser import c_ast
    pending = [node]
    while pending:
        current = pending.pop()
        if isinstance(current, c_ast.UnaryOp) and current.op in IDO_IMPORT_TYPE_OPERATORS:
            raise ValueError("IDO type operator survives in retained target/context")
        pending.extend(child for _, child in current.children())


def grouped_import_main(symbol: str, plan_path: str, argv: list[str]) -> None:
    """Process-local adapter using the vendor's existing sameline contract."""
    import runpy
    sys.path.insert(0, str(PERMUTER_DIR))
    from src import ast_util
    from perm_pycparser import c_ast
    original = ast_util.parse_c
    original_to_c, original_to_c_raw = ast_util.to_c, ast_util.to_c_raw
    original_argv = sys.argv
    seen = []
    def parse(source, from_import=False):
        ast = original(source, from_import=from_import)
        if from_import:
            Path(plan_path).with_suffix(".preprocessed.c").write_text(source)
            ast, plan = prepare_source_groups(ast, source, symbol, c_ast)
            seen.append(plan)
            if len(seen) != 1:
                raise ValueError("multiple original importer parses")
            sweep_receipts.atomic_json(Path(plan_path), seen[0])
        return ast
    def checked_emitter(emitter):
        def emit(node, *args, **kwargs):
            reject_retained_ido_operators(node)
            return emitter(node, *args, **kwargs)
        return emit
    try:
        ast_util.parse_c = parse
        ast_util.to_c = checked_emitter(original_to_c)
        ast_util.to_c_raw = checked_emitter(original_to_c_raw)
        sys.argv = [str(IMPORT_PY), *argv]
        with ido_import_parser():
            runpy.run_path(str(IMPORT_PY), run_name="__main__")
        if len(seen) != 1:
            raise ValueError("importer did not expose original source coordinates")
    finally:
        ast_util.parse_c = original
        ast_util.to_c, ast_util.to_c_raw = original_to_c, original_to_c_raw
        sys.argv = original_argv


def preserve_wide_do_spans(ast, source, symbol, nodes):
    """Map original physical spans to live AST slots by exact lexical equality.

    Compound columns are deliberately unused. A whole-function token witness
    binds the generated AST's statement-list slots to the original source.
    Only bridges between distinct multiline do bodies activate this fallback.
    """
    from perm_pycparser import c_generator
    source = source.replace("\r\n", "\n").replace("\r", "\n")
    if len(source) > 4 * 1024 * 1024:
        raise ValueError("wide span source exceeds bound")
    functions = [n for n in ast.ext if isinstance(n, nodes.FuncDef) and n.decl.name == symbol]
    if len(functions) != 1:
        raise ValueError("wide span requires one function")
    function = functions[0]
    pending, compounds, do_bodies = [function.body], [], set()
    count = 0
    while pending:
        node = pending.pop()
        count += 1
        if count > 100000:
            raise ValueError("wide span AST exceeds bound")
        if isinstance(node, (nodes.Pragma, nodes.If, nodes.For, nodes.While,
                             nodes.Switch, nodes.Label, nodes.Case, nodes.Default)):
            raise ValueError("unsupported wide span control or pragma")
        if isinstance(node, nodes.DoWhile):
            if not isinstance(node.stmt, nodes.Compound):
                raise ValueError("unsupported unbraced wide do body")
            do_bodies.add(id(node.stmt))
        if isinstance(node, nodes.Compound):
            compounds.append(node)
        pending.extend(child for _, child in node.children())

    # Maximal operators matter: `+ +` must never match `++`. Literals and
    # comments are single tokens, so their braces cannot identify AST slots.
    token_re = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|/\*.*?\*/|//[^\n]*'
                          r'|[A-Za-z_][A-Za-z_0-9]*|(?:\d|\.\d)[A-Za-z_0-9.]*'
                          r'|>>=|<<=|\.\.\.|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[+*/%&|^!-]='
                          r'|[^\s]', re.S)

    def tokens(text):
        rows = []
        physical, offset = 1, 0
        for match in token_re.finditer(text):
            physical += text.count("\n", offset, match.start())
            offset = match.start()
            value = match.group()
            if value.startswith(("/*", "//")):
                continue
            rows.append((value, physical))
            if len(rows) > 250000:
                raise ValueError("wide span token count exceeds bound")
        return rows

    original = tokens(source)
    generator = c_generator.CGenerator()
    generated = tokens(generator.visit(function))
    needle = [value for value, _ in generated]
    # Linear exact subsequence matching, requiring one entire definition.
    failure = [0] * len(needle)
    j = 0
    for i in range(1, len(needle)):
        while j and needle[i] != needle[j]:
            j = failure[j - 1]
        if needle[i] == needle[j]:
            j += 1
        failure[i] = j
    matches, j = [], 0
    for i, (value, _) in enumerate(original):
        while j and value != needle[j]:
            j = failure[j - 1]
        if value == needle[j]:
            j += 1
        if j == len(needle):
            matches.append(i + 1 - j)
            j = failure[j - 1]
    if len(matches) != 1:
        raise ValueError("wide span lacks unique whole-function lexical correspondence")
    original = original[matches[0]:matches[0] + len(needle)]

    saved, anchors, originally_empty = {}, {}, set()
    for compound in compounds:
        if compound.block_items is None:
            originally_empty.add(id(compound))
        items = list(compound.block_items or [])
        saved[id(compound)] = items
        marked = []
        for index in range(len(items) + 1):
            number = len(anchors)
            anchors[number] = (compound, index)
            marked.append(nodes.Pragma(f"_permuter source_slot_{number}"))
            if index < len(items):
                marked.append(items[index])
        compound.block_items = marked
    try:
        marked = generator.visit(function)
    finally:
        for compound in compounds:
            compound.block_items = None if id(compound) in originally_empty else saved[id(compound)]
    # Count exact token boundaries on each side of the temporary slot markers.
    pieces = re.split(r"(?m)^[ \t]*#pragma _permuter source_slot_(\d+)[ \t]*$", marked)
    witnessed, positions = [], {}
    for index, piece in enumerate(pieces):
        if index % 2:
            positions.setdefault(len(witnessed), []).append(int(piece))
        else:
            witnessed.extend(value for value, _ in tokens(piece))
    if witnessed != needle or sum(map(len, positions.values())) != len(anchors):
        raise ValueError("wide span probe changed searchable AST tokens")

    root_start = next(pos for pos, ids in positions.items()
                      if any(anchors[number] == (function.body, 0) for number in ids))
    root_end = next(pos for pos, ids in positions.items()
                    if any(anchors[number] == (function.body, len(saved[id(function.body)])) for number in ids))
    spans, first = [], 0
    while first < len(original):
        end = first + 1
        while end < len(original) and original[end][1] == original[first][1]:
            end += 1
        # Function signature/outer opening brace layout is not a body group.
        if end > root_start and first < root_end and len({line for _, line in generated[first:end]}) > 1:
            if len(positions.get(first, [])) != 1 or len(positions.get(end, [])) != 1:
                raise ValueError("wide physical line lacks unique statement-list boundary slots")
            spans.append((first, end, original[first][1], positions[first][0], positions[end][0]))
        first = end
    def bridge(span):
        first, end, line, start_id, end_id = span
        left, _ = anchors[start_id]
        right, _ = anchors[end_id]
        if left is right or id(left) not in do_bodies or id(right) not in do_bodies:
            return False
        left_slots = [pos for pos, ids in positions.items() for number in ids if anchors[number][0] is left]
        right_slots = [pos for pos, ids in positions.items() for number in ids if anchors[number][0] is right]
        # First body's opening precedes the span; last body's closing follows it.
        return (original[min(left_slots) - 1][1] < line
                and original[max(right_slots)][1] > line)
    if not any(bridge(span) for span in spans):
        raise ValueError("no supported wide multiline do-body bridge")
    events = {}
    for first, end, line, start_id, end_id in spans:
        events.setdefault(start_id, []).append("start")
        events.setdefault(end_id, []).append("end")
    by_slot = {(id(compound), index): number for number, (compound, index) in anchors.items()}
    for compound in compounds:
        items, output = saved[id(compound)], []
        for index in range(len(items) + 1):
            for event in sorted(events.get(by_slot[id(compound), index], [])):
                output.append(nodes.Pragma("_permuter sameline " + event))
            if index < len(items):
                output.append(items[index])
        compound.block_items = None if not output and id(compound) in originally_empty else output
    return {"contract": SOURCE_GROUP_CONTRACT, "symbol": symbol, "status": "preserved",
            "groups": [{"line": line, "tokens": end - first, "control": "LexicallyBoundDoSpan"}
                       for first, end, line, _, _ in spans]}


def prepare_source_groups(ast, source, symbol, nodes):
    import copy
    working = copy.deepcopy(ast)
    try:
        plan = preserve_source_groups(working, source, symbol, nodes)
        plan["status"] = "preserved" if plan["groups"] else "ungrouped"
        return working, plan
    except ValueError as error:
        try:
            working = copy.deepcopy(ast)
            plan = preserve_wide_do_spans(working, source, symbol, nodes)
            return working, plan
        except ValueError:
            pass
        # Unsupported syntax is not evidence of compiler divergence. Retain
        # the original AST and demand an actual baseline measurement instead.
        return ast, {"contract": SOURCE_GROUP_CONTRACT, "symbol": symbol,
                     "status": "measurement-required", "reason": str(error), "groups": []}


def run_import(item: QueueItem, out_dir: Path, settings_path: Path, target_asm: Path,
               batch_deadline: Optional[float] = None) -> Path:
    root_nonmatchings = ROOT / "nonmatchings" / item.func
    remaining_timeout(batch_deadline)
    def preserve(path: Path, label: str):
        if path.exists() or path.is_symlink():
            path.rename(out_dir / f"{label}-{uuid.uuid4().hex}")
    preserve(root_nonmatchings, "preexisting-import")
    log_path = out_dir / "import.log"
    try:
        proc = bounded_capture(
            [str(PYTHON), "-c",
             "import sys; sys.path.insert(0, sys.argv.pop(1)); "
             "import permute_batch as p; "
             "p.grouped_import_main(sys.argv[1], sys.argv[2], sys.argv[3:])",
             str(ROOT / "tools"), item.func, str(out_dir / "source-groups.json"),
             str(item.c_file), str(target_asm), "--settings", str(settings_path)], batch_deadline,
        )
        log_path.write_text(proc.stdout)
        if (proc.returncode != 0 or not root_nonmatchings.is_dir()
                or not (out_dir / "source-groups.json").is_file()):
            raise RuntimeError(
                f"import.py failed for {item.func} (see {log_path.relative_to(ROOT)})"
            )
    except BaseException as exc:
        preserve(root_nonmatchings, "failed-import")
        if not log_path.exists():
            log_path.write_text(f"{type(exc).__name__}: {exc}\n")
        raise
    scratch = out_dir / "scratch"
    preserve(scratch, "preexisting-scratch")
    shutil.move(str(root_nonmatchings), str(scratch))
    (out_dir / "importer-compile.sh").write_bytes(sweep_receipts.owned_bytes(scratch, "compile.sh"))
    # Keep the empty parent directory. Removing it races another concurrent
    # import.py between its os.makedirs("nonmatchings") and per-function
    # os.mkdir calls, producing a sporadic FileNotFoundError at --jobs > 1.
    return scratch


def _improved_over_base(scratch: Path, log_path: Path) -> bool:
    """True if some output-*/score.txt is strictly below the base score.
    The permuter also writes equal-score outputs, which are not progress."""
    text = log_path.read_text(errors="replace") if log_path.is_file() else ""
    m = re.search(r"base score = (\d+)", text)
    base = int(m.group(1)) if m else None
    for d in scratch.glob("output-*"):
        f = d / "score.txt"
        try:
            score = int(f.read_text().strip())
        except (OSError, ValueError):
            continue
        if base is None or score < base:
            return True
    return False


def wait_for_headroom(
    threshold: float,
    label: str = "",
    batch_deadline: Optional[float] = None,
) -> None:
    """Block until the 1-minute load average is under `threshold`. The
    machine froze under an unthrottled fleet (load ~20 on 14 cores); every
    compile-heavy launch here gates on headroom first."""
    if batch_deadline is not None and time.monotonic() >= batch_deadline:
        raise TimeoutError("whole-batch deadline reached before launch")
    if threshold <= 0:
        return
    waited = 0
    while True:
        if batch_deadline is not None and time.monotonic() >= batch_deadline:
            raise TimeoutError("whole-batch deadline reached while waiting for headroom")
        try:
            load = os.getloadavg()[0]
        except OSError:
            return
        if load < threshold:
            return
        if waited == 0:
            print(f"[headroom] load {load:.1f} >= {threshold:.1f}; waiting {label}".rstrip())
        CANCEL_EVENT.wait(remaining_timeout(batch_deadline, 15))
        remaining_timeout(batch_deadline)
        waited += 15


def run_permuter(
    scratch: Path,
    out_dir: Path,
    minutes: int,
    threads: int,
    extra_args: list[str],
    log_name: str = "permuter.log",
    flat_minutes: int = 0,
    batch_deadline: Optional[float] = None,
) -> tuple[Optional[int], float, bool, bool]:
    """Run one search; return score, elapsed, flat-stop, batch-stop.

    The per-function deadline remains the normal stopping condition.  The
    optional absolute batch deadline is the outer safety bound: every active
    process group observes it, so a long queue cannot turn a bounded search
    into an unbounded campaign.
    """
    log_path = out_dir / log_name
    # Both stack offsets and branch destinations are executable semantics.
    # The vendor CLI ignores branch targets by default, unlike its Options
    # dataclass: leaving that default would report false zeros for wrong CFG
    # destinations. Keep these mandatory arguments bound into receipt inputs.
    args = [
        "nice", "-n", "15",
        str(PYTHON),
        "-u",  # A capped child must not lose its baseline score in a file buffer.
        str(PERMUTER_PY),
        *MANDATORY_PERMUTER_ARGS,
        "-j",
        str(threads),
    ]
    args += [*(arg for arg in extra_args if arg not in MANDATORY_PERMUTER_ARGS), str(scratch)]
    start = time.monotonic()
    with open(log_path, "w") as log_f:
        # permuter.py -j N forks worker processes; killing only the parent on
        # timeout reparents the idle workers to PID 1 (observed: three batches
        # of orphans after one morning of sweeping). Run the search in its own
        # session and kill the whole process group at the cap.
        proc = subprocess.Popen(
            args,
            cwd=ROOT,
            stdout=log_f,
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        # Early stop when flat: a search that has produced no improvement at
        # all after `flat_minutes` almost never does later (measured on the
        # first sweep day: every 20-minute run that was flat at 6 minutes was
        # still flat at 20). Improvements appear as output-* dirs, so poll
        # for one; the extension heuristic covers the descending case.
        deadline = time.monotonic() + minutes * 60
        if batch_deadline is not None:
            deadline = min(deadline, batch_deadline)
        flat_deadline = time.monotonic() + flat_minutes * 60 if flat_minutes > 0 else None
        stop_reason = None
        try:
            while True:
                if CANCEL_EVENT.is_set():
                    raise RuntimeError("batch cancelled")
                now = time.monotonic()
                wake_at = deadline
                if flat_deadline is not None:
                    wake_at = min(wake_at, flat_deadline)
                poll_seconds = max(0.05, min(1.0, wake_at - now))
                try:
                    returncode = proc.wait(timeout=poll_seconds)
                    break
                except subprocess.TimeoutExpired:
                    pass
                now = time.monotonic()
                if now >= deadline:
                    stop_reason = "batch" if batch_deadline is not None and now >= batch_deadline else "cap"
                    break
                if flat_deadline is not None and now >= flat_deadline:
                    if not _improved_over_base(scratch, out_dir / log_name):
                        stop_reason = "flat"
                        break
                    flat_deadline = None
        finally:
            # Cleanup also runs on Ctrl-C or an unexpected monitoring failure.
            stop_process_group(proc)
    elapsed = time.monotonic() - start
    text = log_path.read_text(errors="replace")
    m = re.search(r"base score = (\d+)", text)
    base_score = int(m.group(1)) if m else None
    if stop_reason is None and returncode != 0:
        raise RuntimeError(f"permuter exited {returncode}; see {log_path}")
    if base_score is None and stop_reason != "batch":
        raise RuntimeError(f"permuter produced no base score; see {log_path}")
    return base_score, elapsed, stop_reason == "flat", stop_reason == "batch"


def best_output_dir(scratch: Path) -> Optional[Path]:
    candidates = []
    for d in scratch.glob("output-*"):
        if d.is_symlink() or not d.is_dir():
            continue
        parts = d.name.split("-")
        if len(parts) >= 2 and parts[1].lstrip("-").isdigit():
            candidates.append((int(parts[1]), d))
    if not candidates:
        return None
    candidates.sort(key=lambda t: t[0])
    return candidates[0][1]


# --------------------------------------------------------------------------
# Promotion: splice a zero-score candidate into the real C file
# --------------------------------------------------------------------------


def extract_function_text(source_text: str, func: str) -> str:
    m = None
    for cand in FUNC_DEF_RE.finditer(source_text):
        if cand.group("name") == func:
            m = cand
            break
    if m is None:
        raise RuntimeError(f"could not find {func}() in candidate source.c")
    start = m.start()
    depth = 0
    i = m.end() - 1  # at the opening brace
    while True:
        if source_text[i] == "{":
            depth += 1
        elif source_text[i] == "}":
            depth -= 1
            if depth == 0:
                break
        i += 1
    end = i + 1
    return source_text[start:end]


def retire_plateau_marker(text: str, func: str) -> str:
    """Remove owned metadata; trim only a newly exposed blank EOF suffix."""
    marker = re.compile(r"^/\* PLATEAU-HANDOFF:" + re.escape(func) +
                        r":start\n.*?\* PLATEAU-HANDOFF:" + re.escape(func) +
                        r":end\n \*/(?:\n|\Z)", re.S | re.M)
    for match in reversed(list(marker.finditer(text))):
        prefix, suffix = text[:match.start()], text[match.end():]
        if not suffix.strip(" \t\r\n"):
            # The metadata made its preceding separator an interior blank
            # line. Once retired, preserve the final content line and its
            # newline, without introducing whitespace-only lines at EOF.
            text = re.sub(r"\n(?:[ \t]*\n)*[ \t]*\Z", "\n", prefix)
        else:
            text = prefix + suffix
    return text


# A --jobs > 1 batch runs several functions' permuter searches concurrently,
# but promotion (splice + `gmake` + `gmake verify`) mutates the one shared
# working tree and build/ directory this lane owns -- two threads promoting
# at once would race on the same objects and ELF. Only the search itself is
# parallel; promotion is serialized across the whole batch.
PROMOTE_LOCK = threading.Lock()


def promote(item: QueueItem, winning_source: Path, jobs: int,
            batch_deadline: Optional[float] = None, commit: bool = False, *,
            evidence: PreparedBaseline | None = None, winner_bytes: bytes | None = None) -> tuple[bool, Optional[str]]:
    """Splice the winning candidate into the real C file, rebuild, and
    verify byte-identity. Roll back owned writes on failure, preserving
    conflicting independent edits for recovery. Returns (promoted, error)."""
    try:
        while not PROMOTE_LOCK.acquire(timeout=remaining_timeout(batch_deadline, 0.25)):
            pass
        try:
            remaining_timeout(batch_deadline)
            frozen = winner_bytes if winner_bytes is not None else sweep_receipts.owned_bytes(
                winning_source.parent, winning_source.name, limit=4 * 1024 * 1024)
            if not isinstance(frozen, bytes):
                raise RuntimeError("winner must be immutable bytes")
            return _promote_locked(item, frozen, jobs, batch_deadline, commit, evidence)
        finally:
            PROMOTE_LOCK.release()
    except Exception as exc:
        return False, str(exc)


TRIAL_JSON = ROOT / "build" / "promotion-trial.json"


def trial_explanation(func: str) -> Optional[str]:
    """`tools/promotion_trial.py`'s verdict for one function, if it has run.

    A permuter score of 0 says the candidate's *instruction schedule* matches
    the target the scratch was built from. For an overlay that is necessary
    and not sufficient: the module's data is placed by the runtime, so a
    promotion can still move a word the scratch never modelled -- a datum
    landing at a different module offset, a POSTPROCESS normalization the
    scratch could not replicate -- and `gmake verify` is the only thing that
    sees it. When that happens the function stays a candidate, and the trial's
    class is the explanation to carry forward rather than "promotion failed".
    """
    if not TRIAL_JSON.is_file():
        return None
    try:
        rows = json.loads(TRIAL_JSON.read_text()).get("results", [])
    except (OSError, ValueError):
        return None
    for row in rows:
        if row.get("func") != func:
            continue
        parts = [f"promotion-trial class: {row.get('klass')}"]
        if row.get("in_range_words") is not None:
            parts.append(f"{row['in_range_words']} word(s) differ in range")
        if row.get("out_of_range_bytes"):
            parts.append(f"{row['out_of_range_bytes']} byte(s) out of range")
        if row.get("cause"):
            parts.append(str(row["cause"]))
        return "; ".join(parts)
    return None


HANDOFF_DIR = ROOT / "docs" / "matching-triage-handoffs"


def _promote_locked(item: QueueItem, winner: bytes, jobs: int,
                    deadline: Optional[float], commit: bool,
                    prepared: PreparedBaseline | None = None) -> tuple[bool, Optional[str]]:
    source = item.c_file
    shard = HANDOFF_DIR / f"{item.func}.md"
    atlas = ATLAS_PATH
    yaml = ROOT / "mickey.us.yaml"
    symbols = ROOT / "overlay_undefined_syms.us.txt"
    donors = ROOT / "config/overlay-donors.us.json"
    readme = ROOT / "README.md"
    paths = [source, shard, atlas, yaml, symbols, donors, readme]
    rel_paths = [str(path.relative_to(ROOT)) for path in paths]
    evidence = BUILD_PERMUTER / item.func / "promotions" / uuid.uuid4().hex
    review = review_context(item, prepared, winner, deadline)
    retain_context(evidence / "context-review", prepared, winner, review)
    if review["status"] != "unchanged":
        return False, f"candidate declaration context {review['status']}: {review['reason']}; evidence: {evidence.relative_to(ROOT)}"
    journal = promotion_transaction.FileJournal(ROOT, paths, evidence)
    initial_head = None
    initial_ref = "HEAD"
    initial_index = None
    main_index = None
    detached_git = None
    expected_tree = None
    expected_index = None
    expected_message = None
    cleanup_deadline = None

    def run(args, *, outputs=(), env=None, cap=600):
        remaining_timeout(deadline)
        journal.check()
        try:
            result = bounded_capture(args, deadline, cap=cap, env=env)
        finally:
            # Capture partial generator writes even when timeout/cancellation
            # interrupted the command. Other paths keep their owned snapshot.
            journal.capture(list(outputs))
        journal.check()
        if result.returncode:
            raise RuntimeError(f"{' '.join(args)} failed:\n{result.stdout[-3000:]}")
        return result.stdout.strip()

    def cleanup_git(args, env=None):
        # Cleanup has a short independent grace; it must still run once the
        # search deadline has expired or cancellation has been requested.
        remaining = min(5, cleanup_deadline - time.monotonic())
        if remaining <= 0:
            raise TimeoutError("promotion cleanup grace expired")
        result = subprocess.run(["git", *args], cwd=ROOT, capture_output=True,
                                text=True, timeout=remaining, env=env)
        if result.returncode:
            raise RuntimeError(f"rollback git {' '.join(args)} failed: {result.stderr[-1000:]}")
        return result.stdout.strip()

    def synchronize_index(expected, target, command, unchanged=None, publish=None, finalize=None,
                          locked_copy=None):
        # Git's index.lock spans the comparison AND publication. Commands
        # update a separate copy, so errors leave the real index untouched.
        context = (contextlib.nullcontext(locked_copy) if locked_copy is not None
                   else promotion_transaction.locked_index(main_index))
        with context as copy:
            env = dict(os.environ, GIT_INDEX_FILE=str(copy))
            observed = command(["ls-files", "--stage", "--", *rel_paths], env=env)
            if observed == unchanged:
                if publish is not None:
                    publish()
                return
            if observed != expected:
                raise RuntimeError("concurrent promotion-path index edits preserved")
            if publish is not None:
                publish()
            command(["reset", "-q", target, "--",
                     *[str(path.relative_to(ROOT)) for path in journal.changed()]], env=env)
            if finalize is not None:
                finalize()

    try:
        initial_head = run(["git", "rev-parse", "HEAD"])
        initial_ref = run(["git", "rev-parse", "--symbolic-full-name", "HEAD"])
        main_index = Path(run(["git", "rev-parse", "--path-format=absolute", "--git-path", "index"]))
        if commit and initial_ref == "HEAD":
            raise RuntimeError("--commit requires an attached lane branch")
        if Path(str(main_index) + ".lock").exists():
            raise RuntimeError("existing index lock; refusing to mutate promotion source")
        initial_index = run(["git", "ls-files", "--stage", "--", *rel_paths])
        if commit and run(["git", "diff", "HEAD", "--name-only", "--", *rel_paths]):
            raise RuntimeError("promotion paths already differ from HEAD; preserve them before --commit")
        original = source.read_text()
        validate_baseline(item, prepared, deadline)
        function = extract_function_text(winner.decode("utf-8"), item.func)
        block = next((block for block in iter_nonmatching_blocks(original)
                      if block_function_name(original, block) == item.func), None)
        if block is None:
            raise RuntimeError(f"could not locate {item.func}'s NON_MATCHING block")
        new_text = original[:block.start] + function + original[block.end:]
        new_text = retire_plateau_marker(new_text, item.func)
        remaining_timeout(deadline)
        journal.write(source, new_text.encode())
        if shard.exists():
            journal.write(shard, None)
        if item.overlay is not None:
            run(["gmake", "overlay-atlas-write"], outputs=[atlas, yaml])
            run(["gmake", f"-j{jobs}", f"build/{item.rel_c_file}.o"])
            run(["gmake", "overlay-syms"], outputs=[symbols])
            run([str(PYTHON), "tools/refresh_atlas_digest.py"], outputs=[donors])
            # Generation may normalize a built object's symbol table. That
            # transient object is not proof that the configured recipe can
            # reproduce it. Retain it, then build the affected TU from an
            # absent object and check the generated surface without mutation.
            object_target = f"build/{item.rel_c_file}.o"
            object_path = ROOT / object_target
            if (not object_path.is_file() or object_path.is_symlink()
                    or object_path.resolve() != ROOT.resolve() / object_target):
                raise RuntimeError("promotion object is missing or has symlinked ownership")
            remaining_timeout(deadline)
            journal.check()
            object_path.rename(evidence / "after-symbol-generation.o")
            try:
                run(["gmake", f"-j{jobs}", object_target])
            finally:
                # Keep partial rebuild evidence even after a failed compiler
                # or expired/cancelled run. Never restore an exact-looking
                # generator object over the configured result.
                if (object_path.is_file() and not object_path.is_symlink()
                        and object_path.resolve() == ROOT.resolve() / object_target):
                    shutil.copy2(object_path, evidence / "configured-rebuild.o")
            if (not object_path.is_file() or object_path.is_symlink()
                    or object_path.resolve() != ROOT.resolve() / object_target
                    or object_path.stat().st_size == 0):
                raise RuntimeError("configured promotion rebuild produced no object")
            run(["gmake", "check-overlay-syms"])
        run(["gmake", f"-j{jobs}"], cap=1800)
        run(["gmake", f"-j{jobs}", "verify"])
        # Resident C edits do not invalidate the split stamp. Remove the now
        # orphaned fallback through the normal lifecycle before post-promotion
        # resolution; a retained .s would incorrectly select fallback mode.
        run(["gmake", "prune-asm"])
        run([str(PYTHON), "tools/promotion_proof.py", item.func, "--json"], cap=300)
        run(["gmake", "scoreboard"], outputs=[readme])
        if item.overlay is not None:
            run(["gmake", "check-overlay-syms"])
            run(["gmake", "overlay-atlas"])
        run(["gmake", "check-scoreboard"])
        run(["gmake", "check-docs"])
        run(["gmake", "cleanroom"])
        if run(["git", "rev-parse", "HEAD"]) != initial_head:
            raise RuntimeError("HEAD changed during promotion; manual review required")
        if commit:
            # A private index contains HEAD plus exactly this transaction's
            # paths. Unrelated staged work never enters the match commit.
            changed = [str(path.relative_to(ROOT)) for path in journal.changed()]
            common = run(["git", "rev-parse", "--path-format=absolute", "--git-common-dir"])
            hooks = run(["git", "rev-parse", "--path-format=absolute", "--git-path", "hooks"])
            detached_git = evidence / "commit.git"
            detached_git.mkdir()
            (detached_git / "commondir").write_text(common + "\n")
            (detached_git / "HEAD").write_text(initial_head + "\n")
            env = dict(os.environ, GIT_INDEX_FILE=str(evidence / "commit.index"),
                       GIT_DIR=str(detached_git), GIT_COMMON_DIR=common, GIT_WORK_TREE=str(ROOT))
            # Detached HEAD changes conditional config evaluation and lacks
            # the originating worktree's config.worktree. Never silently drop
            # signing, identity, filters or policy consumed by hooks. Compare
            # without printing configuration values, which may be sensitive.
            if run(["git", "config", "--null", "--list"]) != run(
                    ["git", "config", "--null", "--list"], env=env):
                raise RuntimeError("origin and detached Git configuration differ; unsupported commit context")
            run(["git", "read-tree", initial_head], env=env)
            run(["git", "add", "-A", "--", *changed], env=env)
            run(["git", "diff", "--cached", "--check"], env=env)
            expected_tree = run(["git", "write-tree"], env=env)
            expected_index = run(["git", "ls-files", "--stage", "--", *rel_paths], env=env)
            if run(["git", "ls-files", "--stage", "--", *rel_paths]) != initial_index:
                raise RuntimeError("promotion-path index entries changed concurrently")
            message = (f"Match {item.func} (permuter)\n\n"
                       "Untouched configured compiler output passed linked-range and ROM proof;\n"
                       "derived symbols, scoreboard, documentation and cleanroom gates passed.")
            expected_message = message
            # Pin the effective hook directory as seen by the real worktree,
            # including per-worktree or conditional configuration overrides.
            run(["git", "-c", f"core.hooksPath={hooks}", "commit", "-q", "-m", message], env=env)
            candidate = run(["git", "rev-parse", "HEAD"], env=env)
            if (run(["git", "show", "-s", "--format=%T", candidate]) != expected_tree
                    or run(["git", "show", "-s", "--format=%P", candidate]) != initial_head
                    or run(["git", "show", "-s", "--format=%B", candidate]) != expected_message):
                raise RuntimeError("hooked commit differs from the proved tree, parent or message")
            def publish():
                # Normal checkout also requires this real index.lock. Hold it
                # across the HEAD check, ref CAS and index reconciliation so
                # a checkout cannot redirect the index to another branch.
                if run(["git", "symbolic-ref", "HEAD"]) != initial_ref:
                    raise RuntimeError("checked-out branch changed during promotion")
                run(["git", "update-ref", initial_ref, candidate, initial_head])
            def finalize():
                remaining_timeout(deadline)
                journal.check()
            synchronize_index(initial_index, candidate,
                              lambda args, env: run(["git", *args], env=env),
                              publish=publish, finalize=finalize)
            # Lock release is the committed transaction's completion point.
            # A later checkout/cancel is new activity, not grounds to undo it.
            return True, None
        remaining_timeout(deadline)
        journal.check()
        return True, None
    except BaseException as exc:
        reason = f"{type(exc).__name__}: {exc}"
        cleanup_deadline = time.monotonic() + 30
        try:
            detached_tip = None
            if detached_git is not None and (detached_git / "HEAD").exists():
                detached_tip = cleanup_git(["--git-dir", str(detached_git), "rev-parse", "HEAD"])
                if detached_tip != initial_head:
                    recovery = f"refs/sweep-recovery/{uuid.uuid4().hex}"
                    cleanup_git(["update-ref", recovery, detached_tip])
                    reason += f"; failed promotion commit retained at {recovery}"
            if journal.changed():
                # Same bytes do not establish ownership after a checkout:
                # another branch may commit exactly our candidate text. Hold
                # checkout exclusion through branch/ref/index AND file recovery.
                with promotion_transaction.locked_index(main_index) as recovery_index:
                    if cleanup_git(["rev-parse", "--symbolic-full-name", "HEAD"]) != initial_ref:
                        raise RuntimeError("selected branch changed; files and index preserved for manual recovery")
                    current_head = cleanup_git(["rev-parse", initial_ref])
                    if current_head != initial_head:
                        # Undo only our exact detached candidate on the pinned
                        # branch; never undo a foreign or concurrent commit.
                        tree = cleanup_git(["show", "-s", "--format=%T", current_head])
                        parents = cleanup_git(["show", "-s", "--format=%P", current_head])
                        message = cleanup_git(["show", "-s", "--format=%B", current_head])
                        if (current_head != detached_tip or expected_tree is None or tree != expected_tree
                                or parents != initial_head or message != expected_message):
                            changed = cleanup_git(["diff", initial_head, current_head,
                                                   "--name-only", "--", *rel_paths])
                            if changed:
                                raise RuntimeError("foreign HEAD changed promotion paths; files and commits preserved for review")
                            reason += "; unrelated concurrent commit preserved"
                        else:
                            def restore_ref():
                                cleanup_git(["update-ref", initial_ref, initial_head, current_head])
                            try:
                                synchronize_index(expected_index, initial_head, cleanup_git,
                                                  unchanged=initial_index, publish=restore_ref,
                                                  locked_copy=recovery_index)
                            except Exception as index_error:
                                reason += f"; {index_error}"
                    conflicts = journal.rollback(cleanup_deadline)
                    if conflicts:
                        reason += "; concurrent edits need manual rollback: " + ", ".join(conflicts)
        except BaseException as rollback_error:
            reason += f"; rollback needs review: {rollback_error}"
        reason += (f"; evidence: {evidence.relative_to(ROOT)}; "
                   "run gmake extract and rebuild before reusing restored fallback artifacts")
        return False, reason


def _best(scratch: Path) -> tuple[Optional[Path], Optional[int]]:
    best_dir = best_output_dir(scratch)
    if best_dir is None:
        return None, None
    score = sweep_receipts.owned_bytes(scratch, best_dir.name + "/score.txt")
    return best_dir, int(score.decode().strip())


_IMPORT_LOCK = threading.Lock()

_CAPTURE_BASELINE = r'''
import hashlib, json, os, stat, subprocess, sys, tempfile
from pathlib import Path
recipe, destination, binding, *args = sys.argv[1:]
if len(args) != 3 or args[1] != '-o':
    raise RuntimeError('unsupported permuter compiler invocation')
root = Path(destination)
def read_temp(name):
    path = Path(name)
    if path.parent.resolve() != Path(tempfile.gettempdir()).resolve() or not path.name.startswith('permuter'):
        raise RuntimeError('compiler input/output is not a permuter temporary file')
    fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)
    with os.fdopen(fd, 'rb') as stream:
        info = os.fstat(stream.fileno())
        if not stat.S_ISREG(info.st_mode) or info.st_size > 120 * 1024 * 1024:
            raise RuntimeError('unsupported compiler temporary file')
        return stream.read(120 * 1024 * 1024 + 1)
source = read_temp(args[0])
(root / 'compiled.c').write_bytes(source)
code = subprocess.run(['bash', recipe, *args]).returncode
obj = read_temp(args[2])
(root / 'compiled.o').write_bytes(obj)
if read_temp(args[0]) != source:
    raise RuntimeError('baseline input changed during compiler invocation')
(root / 'capture.json').write_text(json.dumps({'returncode': code, 'binding': json.loads(binding),
    'source_sha256': hashlib.sha256(source).hexdigest(),
    'object_sha256': hashlib.sha256(obj).hexdigest()}))
sys.exit(code if code else (0 if obj else 1))
'''


def install_baseline_capture(scratch: Path, out_dir: Path, baseline: dict, inputs: dict) -> Path:
    """Capture the first synchronous search compilation, not importer's different AST."""
    capture = out_dir / "baseline-capture"
    capture.mkdir(mode=0o700)
    recipe = capture / "compile-original.sh"
    recipe.write_bytes(baseline["baseline/compile.sh"])
    helper = capture / "capture.py"
    helper.write_text(_CAPTURE_BASELINE)
    quote = shlex.quote
    wrapper = ("#!/bin/sh\nif mkdir " + quote(str(capture / "claimed")) + " 2>/dev/null; then\n"
               + "exec " + " ".join(map(quote, [str(PYTHON), str(helper), str(recipe), str(capture),
                   json.dumps({"inputs_sha256": sweep_receipts.digest(inputs), "run_id": out_dir.name})]))
               + ' "$@"\nelse\nexec bash ' + quote(str(recipe)) + ' "$@"\nfi\n')
    (scratch / "compile.sh").write_text(wrapper)
    (scratch / "compile.sh").chmod(0o755)
    return capture


def seed_context_compatible(parent: dict, current: dict) -> bool:
    """Only generated lane cwd and prior runner implementation may differ.

    Actual source/header/target/settings and external compiler/search tools remain
    exact. Fresh local captures prove declaration context separately.
    """
    old = json.loads(json.dumps(parent))
    new = json.loads(json.dumps(current))
    implementations = {"runner", "receipts", "promotion", "candidate_context", "loaded_modules"}
    for context in (old["context"], new["context"]):
        context.pop("seed", None)
        context["tools"] = {k: v for k, v in context["tools"].items() if k not in implementations}
        context["baseline_hashes"].pop("baseline/compile.sh", None)
    for value in (old, new):
        value.pop("search", None)
        value["baseline_hashes"].pop("baseline/compile.sh", None)
    return old == new


def seed_stage_settings(raw: bytes) -> tuple[bytes, dict]:
    """Resolve only the generated local objdump executable for isolated cwd."""
    text = raw.decode("utf-8")
    settings = tomllib.loads(text)
    configured = settings.get("objdump_command")
    if not isinstance(configured, str):
        raise RuntimeError("seed stage requires the configured local objdump command")
    command = shlex.split(configured)
    known = "tools/binutils/mips64-elf-objdump"
    executable = ROOT / known
    if not command or command[0] not in (known, str(executable)):
        raise RuntimeError("seed stage objdump is not the pinned local tool")
    effective = shlex.join([str(executable), *command[1:]])
    pattern = r'(?m)^objdump_command[ \t]*=[ \t]*"(?:[^"\\\n]|\\.)*"[ \t]*(?:#[^\n]*)?$'
    replaced, count = re.subn(pattern, lambda _: "objdump_command = " + json.dumps(effective), text)
    if count != 1 or tomllib.loads(replaced) != {**settings, "objdump_command": effective}:
        raise RuntimeError("unsupported generated seed-stage settings syntax")
    return replaced.encode(), {"original": command, "effective": [str(executable), *command[1:]],
        "original_settings_sha256": hashlib.sha256(raw).hexdigest(),
        "effective_settings_sha256": hashlib.sha256(replaced.encode()).hexdigest(),
        "executable_sha256": sweep_receipts.file_digest(executable)}


def measure_seed_stage(item, directory, baseline, inputs, source, deadline):
    """Compile/strict-score once with this lane's recipe, never saved scripts."""
    directory.mkdir(mode=0o700)
    scratch = directory / "scratch"
    scratch.mkdir()
    for name in ("base.c", "compile.sh", "target.s", "target.o", "settings.toml"):
        (scratch / name).write_bytes(baseline["baseline/" + name])
    settings, plumbing = seed_stage_settings(baseline["baseline/settings.toml"])
    (directory / "settings-original.toml").write_bytes(baseline["baseline/settings.toml"])
    (scratch / "settings.toml").write_bytes(settings)
    sweep_receipts.atomic_json(directory / "settings-plumbing.json", plumbing)
    (scratch / "base.c").write_bytes(source)
    install_baseline_capture(scratch, directory, baseline, inputs)
    command = [str(PYTHON), "-u", str(PERMUTER_PY), *MANDATORY_PERMUTER_ARGS,
               "-j", "1", "--debug", str(scratch)]
    try:
        output = bounded_capture(command, deadline, cwd=directory)
    except BaseException as error:
        partial = getattr(error, "output", None) or getattr(error, "stdout", None) or ""
        if isinstance(partial, bytes):
            partial = partial.decode("utf-8", errors="replace")
        (directory / "debug.log").write_text(
            f"{partial}\n[seed stage failure] {type(error).__name__}: {error}\n")
        raise
    (directory / "debug.log").write_text(output.stdout + f"\n[seed stage exit] {output.returncode}\n")
    output.check_returncode()
    scores = re.findall(r"base score = (\d+)", output.stdout)
    if len(scores) != 1:
        raise RuntimeError("compile-only seed stage did not report exactly one strict baseline score")
    return captured_baseline(item, directory, inputs, deadline), int(scores[0])


_FIDELITY_LOCK = threading.Lock()


def normalized_owned_instructions(path, symbol):
    """Address-field normalization only; callers separately prove identities."""
    elf = reloc_surface.Elf(path)
    start, size, _ = reloc_surface._unique_symbol(elf, symbol, require_text=True)
    data = bytearray(elf.section_bytes(".text")[start:start + size])
    if len(data) != size or size % 4:
        raise RuntimeError("incomplete grouped baseline function bytes")
    seen = set()
    for _, offset, kind, _ in elf.relocations():
        if not start <= offset < start + size:
            continue
        site = offset - start
        masks = {4: 0xFC000000, 5: 0xFFFF0000, 6: 0xFFFF0000}
        if kind not in masks or site % 4 or site in seen or site + 4 > size:
            raise RuntimeError("unsupported grouped baseline relocation encoding")
        seen.add(site)
        word = int.from_bytes(data[site:site + 4], "big") & masks[kind]
        data[site:site + 4] = word.to_bytes(4, "big")
    return bytes(data)


def raw_source_relocations(path, symbol):
    """Original source-symbol correspondence, NOT a ROM/runtime identity proof.

    Undefined/absolute unique symbols only. Defined/section-local symbols need
    independently authenticated runtime identities or a future data adapter.
    REL addends are decoded before address fields can be ignored.
    """
    elf = reloc_surface.Elf(path)
    start, size, _ = reloc_surface._unique_symbol(elf, symbol, require_text=True)
    symbols = elf.symbols()
    names = [row[0] for row in symbols]
    data = elf.section_bytes(".text")
    raw, pending, addends = [], {}, {}
    for _, offset, kind, index in elf.relocations():
        if not start <= offset < start + size:
            continue
        if index >= len(symbols) or offset % 4 or offset + 4 > start + size:
            raise RuntimeError("invalid source relocation geometry")
        name, value, extent, info, section = symbols[index]
        if (not name or names.count(name) != 1 or section not in (0, 0xFFF1)
                or kind not in (4, 5, 6)):
            raise RuntimeError("source relocation needs independent defined-symbol identity")
        position = offset - start
        if any(row[0] == position for row in raw):
            raise RuntimeError("duplicate source relocation site")
        field = int.from_bytes(data[offset:offset + 4], "big")
        identity = (name, value, extent, info, section)
        raw.append((position, kind, identity))
        if kind == 4:
            addends[position] = (field & 0x03FFFFFF) << 2
        elif kind == 5:
            pending.setdefault(index, []).append((position, field & 0xFFFF))
        else:
            low = reloc_surface.sext16(field & 0xFFFF)
            highs = pending.pop(index, [])
            values = {((high << 16) + low) & 0xFFFFFFFF for _, high in highs}
            if len(values) > 1:
                raise RuntimeError("ambiguous shared HI source relocation addend")
            value = next(iter(values)) if values else low
            addends[position] = value
            for high_position, _ in highs:
                addends[high_position] = value
    if pending:
        raise RuntimeError("unpaired HI source relocation")
    return sorted((*row, addends[row[0]]) for row in raw)


def grouped_baseline_fidelity(item, out_dir, scratch, inputs, deadline):
    while not _FIDELITY_LOCK.acquire(timeout=remaining_timeout(deadline, 0.25)):
        pass
    try:
        return _grouped_baseline_fidelity(item, out_dir, scratch, inputs, deadline)
    finally:
        _FIDELITY_LOCK.release()


def _grouped_baseline_fidelity(item, out_dir, scratch, inputs, deadline):
    """Prove the actual initial emitter against a freshly configured full TU.

    Unsupported syntax requests measurement, not automatic refusal. Runtime
    identities and raw original-source correspondence are distinct proof routes.
    """
    plan_path = out_dir / "source-groups.json"
    if not plan_path.is_file() or plan_path.is_symlink():
        raise RuntimeError("missing authenticated original source grouping plan")
    plan_bytes = sweep_receipts.owned_bytes(out_dir, "source-groups.json")
    plan_hash = hashlib.sha256(plan_bytes).hexdigest()
    if inputs.get("context", {}).get("source_group_plan") != plan_hash:
        raise RuntimeError("original source grouping plan changed since prepared inputs")
    plan = json.loads(plan_bytes)
    if (not isinstance(plan, dict) or plan.get("contract") != SOURCE_GROUP_CONTRACT
            or plan.get("symbol") != item.func or not isinstance(plan.get("groups"), list)):
        raise RuntimeError("invalid source grouping plan")
    status = plan.get("status")
    if not ((status == "ungrouped" and not plan["groups"])
            or (status == "preserved" and bool(plan["groups"]))
            or (status == "measurement-required" and not plan["groups"]
                and isinstance(plan.get("reason"), str) and bool(plan["reason"]))):
        raise RuntimeError("invalid source grouping status or inconsistent groups")
    if status == "ungrouped":
        return
    directory = out_dir / "source-fidelity"
    directory.mkdir()
    target = ROOT / "build_non_matching" / (item.rel_c_file + ".o")
    if target.resolve() != ROOT.resolve() / "build_non_matching" / (item.rel_c_file + ".o"):
        raise RuntimeError("symlinked full-TU fidelity object path")
    if target.exists() or target.is_symlink():
        if (not target.is_file() or target.is_symlink()
                or target.resolve() != ROOT / "build_non_matching" / (item.rel_c_file + ".o")):
            raise RuntimeError("nonregular full-TU fidelity object")
        target.rename(directory / "previous-full-tu.o")
    try:
        output = bounded_capture([str(PYTHON), "tools/function_preflight.py", item.func,
                                  "--json", "--analysis-only"], deadline)
    except BaseException as error:
        partial = getattr(error, "output", None) or ""
        if isinstance(partial, bytes):
            partial = partial.decode("utf-8", errors="replace")
        (directory / "preflight.log").write_text(
            f"{partial}\n[fidelity preflight failure] {type(error).__name__}: {error}\n")
        raise
    (directory / "preflight.log").write_text(output.stdout)
    output.check_returncode()
    evidence = json.loads(output.stdout)
    if evidence.get("candidate_object") != str(target.relative_to(ROOT)):
        raise RuntimeError("grouped full-TU preflight is incomplete")
    authority_paths = (ROOT / "build/mickey.us.elf", BASEROM, ATLAS_PATH,
                       reloc_surface.LINK_SYMS)
    authority = {str(path): sweep_receipts.file_digest(path) for path in authority_paths}
    full = directory / "full-tu.o"
    full.write_bytes(sweep_receipts.owned_bytes(ROOT, str(target.relative_to(ROOT))))
    baseline = {"baseline/" + name: sweep_receipts.owned_bytes(scratch, name)
                for name in ("base.c", "compile.sh", "target.s", "target.o", "settings.toml")}
    capture, score = measure_seed_stage(item, directory / "emitted", baseline, inputs,
                                       baseline["baseline/base.c"], deadline)
    validate_baseline(item, capture, deadline)
    emitted = directory / "emitted.o"
    emitted.write_bytes(capture.object)
    raw_script = sweep_receipts.owned_bytes(out_dir, "importer-compile.sh")
    if compile_script_digest(raw_script) != inputs["context"]["importer_recipe"]:
        raise RuntimeError("current importer recipe no longer matches prepared inputs")
    raw_recipe = directory / "raw-compile.sh"
    raw_recipe.write_bytes(raw_script)
    raw_source = directory / "raw-emitted.c"
    raw_source.write_bytes(capture.source)
    raw_emitted = directory / "raw-emitted.o"
    # The vendor script resolves OUTPUT before invoking IDO. Match its normal
    # NamedTemporaryFile contract on hosts whose realpath requires existence.
    raw_emitted.touch(exist_ok=False)
    result = bounded_capture(["bash", str(raw_recipe), str(raw_source), "-o", str(raw_emitted)], deadline)
    (directory / "raw-compile.log").write_text(result.stdout)
    result.check_returncode()
    if (raw_source.read_bytes() != capture.source or raw_recipe.read_bytes() != raw_script
            or sweep_receipts.owned_bytes(out_dir, "importer-compile.sh") != raw_script):
        raise RuntimeError("raw emitted input or current importer recipe changed")
    if normalized_owned_instructions(raw_emitted, item.func) != normalized_owned_instructions(emitted, item.func):
        raise RuntimeError("scratch postprocessing changed actual emitted instruction fields")
    reports, normalized = [], []
    for path in (full, raw_emitted):
        try:
            report = reloc_surface.function_surface_comparison(item.func, path,
                ROOT / "build/mickey.us.elf", source=item.rel_c_file.removeprefix("src/").removesuffix(".c"),
                overlay_hint=item.overlay, target_symbol=evidence["linked_symbol"])
        except (reloc_surface.SurfaceComparisonError, ValueError, KeyError) as error:
            report = {"status": "unresolved", "reason": str(error)}
        reports.append(report)
        sweep_receipts.atomic_json(directory / (path.stem + "-relocations.json"), report)
        normalized.append(normalized_owned_instructions(path, item.func))
    runtime_exact = all(report.get("offset_type_exact") is True
                        and report.get("stable_identity_exact") is True for report in reports)
    if not runtime_exact:
        source_records = [raw_source_relocations(path, item.func) for path in (full, raw_emitted)]
        sweep_receipts.atomic_json(directory / "source-relocations.json", source_records)
        if source_records[0] != source_records[1]:
            raise RuntimeError("actual emitted raw source-symbol relocation correspondence differs")
    success = normalized[0] == normalized[1]
    sweep_receipts.atomic_json(directory / "report.json", {
        "contract": SOURCE_GROUP_CONTRACT, "source_fidelity_exact": success,
        "identity_route": "runtime-identities" if runtime_exact else "raw-source-symbols-not-runtime-proof",
        "exact": success,
        "owned_bytes": len(normalized[0]), "strict_score": score,
        "full_tu_sha256": sweep_receipts.file_digest(full),
        "emitted_sha256": capture.object_sha256, "relocations": reports})
    if not success:
        raise RuntimeError("prepared source grouping does not reproduce configured full-TU instructions")
    if authority != {str(path): sweep_receipts.file_digest(path) for path in authority_paths}:
        raise RuntimeError("grouped baseline runtime authority changed during comparison")
    if sweep_receipts.file_digest(plan_path) != plan_hash:
        raise RuntimeError("source grouping plan changed during fidelity measurement")
    checked_tool_identity()
    validate_baseline(item, capture, deadline)
    return capture, score


SEED_FIDELITY_CONTRACT = "mickey-seed-emission-v1"


def group_seed_source(source, symbol):
    """Reconstruct consumed emission markers from immutable seed coordinates."""
    import candidate_context
    sys.path.insert(0, str(PERMUTER_DIR))
    from src import ast_util
    from perm_pycparser import c_ast
    text, definitions, macro_rows = candidate_context.inactive_seed_prelude(source)
    ast = ast_util.parse_c(text, from_import=True)
    ast, plan = prepare_source_groups(ast, text, symbol, c_ast)
    if definitions:
        # Add carriage only AFTER grouping at the original physical coordinates.
        # Candidate reparses these pragmas; to_c restores the full definitions.
        prelude = ("#pragma _permuter latedefine start\n"
                   + "".join("#pragma _permuter " + value + "\n" for value in definitions)
                   + "#pragma _permuter latedefine end\n")
        ast.ext[:0] = ast_util.parse_c(prelude, from_import=True).ext
        plan["inactive_macro_prelude"] = {
            "source_sha256": hashlib.sha256(source).hexdigest(),
            "definitions": macro_rows,
        }
    # Do NOT call to_c: that consumes sameline pragmas before Candidate reparses.
    return ast_util.to_c_raw(ast).encode(), plan


def prepare_seed_layout(item, directory, source, deadline):
    directory.mkdir(mode=0o700)
    original = directory / "original.c"
    original.write_bytes(source)
    command = ("import sys; from pathlib import Path; sys.path.insert(0,sys.argv[1]); "
               "import permute_batch as p; d=Path(sys.argv[2]); "
               "source=p.sweep_receipts.owned_bytes(d,'original.c'); "
               "prepared,plan=p.group_seed_source(source,sys.argv[3]); "
               "(d/'prepared.c').write_bytes(prepared); "
               "p.sweep_receipts.atomic_json(d/'plan.json',plan)")
    checked_tool_identity()
    output = bounded_capture([str(PYTHON), "-c", command, str(ROOT / "tools"),
                              str(directory), item.func], deadline)
    (directory / "prepare.log").write_text(output.stdout)
    output.check_returncode()
    if sweep_receipts.owned_bytes(directory, "original.c") != source:
        raise RuntimeError("immutable seed changed during layout preparation")
    prepared = sweep_receipts.owned_bytes(directory, "prepared.c")
    plan = sweep_receipts.owned_bytes(directory, "plan.json")
    checked_tool_identity()
    return prepared, plan


def seed_object_fidelity(item, original, emitted):
    """Source fidelity only: all owned fields and independent relocation identity."""
    paths = (original, emitted)
    fields = [normalized_owned_instructions(path, item.func) for path in paths]
    if not fields[0] or fields[0] != fields[1]:
        raise RuntimeError("seed emission changed owned instruction fields or geometry")
    try:
        records = [raw_source_relocations(path, item.func) for path in paths]
        if records[0] != records[1]:
            raise RuntimeError("seed emission changed source relocation identity")
        count = len(records[0])
        route = "raw-source-symbols-not-runtime-proof"
    except RuntimeError as error:
        # Defined symbols require independent runtime authority; equal synthetic
        # values or section-relative spellings are not an identity witness.
        reports = [reloc_surface.function_surface_comparison(item.func, path,
            ROOT / "build/mickey.us.elf",
            source=item.rel_c_file.removeprefix("src/").removesuffix(".c"),
            overlay_hint=item.overlay) for path in paths]
        if not all(r.get("offset_type_exact") is True
                   and r.get("stable_identity_exact") is True for r in reports):
            raise RuntimeError("seed emission relocation identity is unproved") from error
        if reports[0]["candidate_record_count"] != reports[1]["candidate_record_count"]:
            raise RuntimeError("seed emission relocation count differs")
        count = reports[0]["candidate_record_count"]
        route = "runtime-identities"
    return {"owned_bytes": len(fields[0]), "relocation_count": count,
            "identity_route": route, "source_fidelity_exact": True}


def prove_seed_emission(item, directory, baseline, inputs, source, seed, deadline):
    directory.mkdir(mode=0o700)
    recipe = baseline["baseline/compile.sh"]
    files = {"original.c": source, "compile.sh": recipe, "emitted.c": seed.source,
             "emitted.o": seed.object}
    for name, data in files.items():
        (directory / name).write_bytes(data)
    original = directory / "original.o"
    original.touch(exist_ok=False)
    authority_paths = (ROOT / "build/mickey.us.elf", BASEROM, ATLAS_PATH, reloc_surface.LINK_SYMS)
    authority = {str(p): sweep_receipts.file_digest(p) for p in authority_paths}
    validate_baseline(item, seed, deadline)
    try:
        output = bounded_capture(["bash", str(directory / "compile.sh"),
            str(directory / "original.c"), "-o", str(original)], deadline)
    except BaseException as error:
        (directory / "compile.log").write_text(str(getattr(error, "output", "")) + "\n" + str(error))
        raise
    (directory / "compile.log").write_text(output.stdout)
    output.check_returncode()
    original_bytes = sweep_receipts.owned_bytes(directory, "original.o")
    report = seed_object_fidelity(item, original, directory / "emitted.o")
    if any(sweep_receipts.owned_bytes(directory, name) != data for name, data in files.items()):
        raise RuntimeError("seed fidelity inputs changed during proof")
    if sweep_receipts.owned_bytes(directory, "original.o") != original_bytes:
        raise RuntimeError("seed original object changed during proof")
    if authority != {str(p): sweep_receipts.file_digest(p) for p in authority_paths}:
        raise RuntimeError("seed relocation authority changed during proof")
    checked_tool_identity()
    validate_baseline(item, seed, deadline)
    report.update(contract=SEED_FIDELITY_CONTRACT,
        inputs_sha256=sweep_receipts.digest(inputs),
        original_source_sha256=hashlib.sha256(source).hexdigest(),
        original_object_sha256=hashlib.sha256(original_bytes).hexdigest(),
        emitted_source_sha256=seed.source_sha256, emitted_object_sha256=seed.object_sha256,
        recipe_sha256=hashlib.sha256(recipe).hexdigest())
    sweep_receipts.atomic_json(directory / "report.json", report)
    return original_bytes, report


def validate_seed_layout(item, baseline, inputs, source):
    identity = inputs["search"]["seed"]
    plan = json.loads(baseline["seed/plan.json"])
    if (identity.get("preparation_contract") != SEED_FIDELITY_CONTRACT
            or identity.get("source_sha256") != hashlib.sha256(source).hexdigest()
            or identity.get("prepared_source_sha256") != hashlib.sha256(baseline["seed/prepared.c"]).hexdigest()
            or identity.get("plan_sha256") != hashlib.sha256(baseline["seed/plan.json"]).hexdigest()
            or not isinstance(plan, dict) or plan.get("contract") != SOURCE_GROUP_CONTRACT
            or plan.get("symbol") != item.func or not isinstance(plan.get("groups"), list)
            or not ((plan.get("status") == "ungrouped" and not plan["groups"])
                or (plan.get("status") == "preserved" and bool(plan["groups"]))
                or (plan.get("status") == "measurement-required" and not plan["groups"]
                    and isinstance(plan.get("reason"), str) and bool(plan["reason"])))):
        raise RuntimeError("seed layout evidence changed or is invalid")


def validate_seed_search(item, out_dir, seed, capture, inputs, deadline):
    directory = out_dir / "seed-search-fidelity"
    directory.mkdir(mode=0o700)
    frozen = {"measured.o": seed.object, "search.o": capture.object}
    for name, data in frozen.items():
        (directory / name).write_bytes(data)
    authority_paths = (ROOT / "build/mickey.us.elf", BASEROM, ATLAS_PATH, reloc_surface.LINK_SYMS)
    authority = {str(p): sweep_receipts.file_digest(p) for p in authority_paths}
    validate_baseline(item, capture, deadline)
    report = seed_object_fidelity(item, directory / "measured.o", directory / "search.o")
    if any(sweep_receipts.owned_bytes(directory, name) != data for name, data in frozen.items()):
        raise RuntimeError("seed search objects changed during comparison")
    if authority != {str(p): sweep_receipts.file_digest(p) for p in authority_paths}:
        raise RuntimeError("seed search relocation authority changed")
    checked_tool_identity()
    validate_baseline(item, capture, deadline)
    report.update(contract=SEED_FIDELITY_CONTRACT, inputs_sha256=sweep_receipts.digest(inputs),
                  measured_object_sha256=seed.object_sha256, search_object_sha256=capture.object_sha256)
    sweep_receipts.atomic_json(directory / "report.json", report)
    return report


def prepare_seed(item, out_dir, baseline, inputs, parent, source, result, deadline):
    validate_seed_layout(item, baseline, inputs, source)
    baseline.update({"seed/parent.json": json.dumps(parent, sort_keys=True).encode(),
                     "seed/source.c": source})
    if any(hashlib.sha256(baseline[name]).hexdigest() != expected
           for name, expected in inputs["baseline_hashes"].items()):
        raise RuntimeError("fresh preparation changed before seed measurement")
    canonical, original_score = measure_seed_stage(item, out_dir / "canonical-measurement",
        baseline, inputs, baseline["baseline/base.c"], deadline)
    result.original_base_score = original_score
    selection = inputs["search"]["seed"].get("selection")
    result.seed_parent_score = selection["selected_score"] if selection is not None else parent["result"]["best_score"]
    if canonical.source_sha256 != parent["result"]["context_review"]["baseline_source_sha256"]:
        raise RuntimeError("fresh actual canonical compiler input differs from parent baseline")
    report = review_context(item, canonical, source, deadline)
    if report["status"] != "unchanged":
        retain_context(out_dir / "context-review", canonical, source, report)
        result.context_review = report
        raise RuntimeError("saved seed declarations differ from fresh actual canonical input")
    prepared_source = baseline["seed/prepared.c"]
    seed, score = measure_seed_stage(item, out_dir / "seed-measurement", baseline,
                                     inputs, prepared_source, deadline)
    # External scorer/target identities are pinned before accepting the parent.
    # A regressed re-emission is not a fresh seed or permission to search.
    if score != result.seed_parent_score:
        raise RuntimeError("seed score differs from authenticated parent; refusing search")
    original_object, fidelity = prove_seed_emission(item, out_dir / "seed-fidelity",
        baseline, inputs, source, seed, deadline)
    baseline.update({"seed/original.o": original_object,
                     "seed/fidelity.json": json.dumps(fidelity, sort_keys=True).encode()})
    compiled_report = review_context(item, canonical, seed.source, deadline)
    if compiled_report["status"] != "unchanged":
        raise RuntimeError("compiled seed declarations differ from fresh canonical input")
    result.seed_score = score
    result.best_score = score
    result.seed_proof = {"status": "pending-search", "seed": inputs["search"]["seed"],
        "original_score": original_score, "seed_score": score, "fidelity": fidelity,
        "fresh_baseline_sha256": canonical.source_sha256,
        "compiled_source_sha256": seed.source_sha256,
        "compiled_object_sha256": seed.object_sha256,
        "parent_comparison": report, "compiled_comparison": compiled_report}
    baseline.update({"seed/compiled.c": seed.source, "seed/compiled.o": seed.object})
    validate_seed_layout(item, baseline, inputs, source)
    return canonical, seed


def preserve_search_artifacts(store, out_dir, scratch, baseline, result, deadline, inputs):
    """Save untouched compiler evidence, including retryable partial attempts."""
    files = dict(baseline)
    if result.seed_proof is not None:
        files["seed/proof.json"] = json.dumps(result.seed_proof, sort_keys=True).encode()
    errors = []
    for name in ("baseline.c", "winner.c", "report.json"):
        try:
            files["context/" + name] = sweep_receipts.owned_bytes(out_dir, "context-review/" + name)
        except Exception as error:
            errors.append(f"context evidence {name}: {error}")
    try:
        # The original capture is retained among attempt files. Use the frozen
        # source/object pair checked before promotion, not mutable late reads.
        capture = out_dir / "context-review"
        metadata = json.loads(sweep_receipts.owned_bytes(capture, "capture.json"))
        for name, frozen_name in (("compiled.c", "baseline.c"), ("compiled.o", "baseline.o")):
            files["baseline/" + name] = sweep_receipts.owned_bytes(capture, frozen_name)
        if (metadata["returncode"] != 0 or not files["baseline/compiled.o"]
                or hashlib.sha256(files["baseline/compiled.c"]).hexdigest() != metadata["source_sha256"]
                or hashlib.sha256(files["baseline/compiled.o"]).hexdigest() != metadata["object_sha256"]):
            raise RuntimeError("actual search baseline capture is incomplete")
        files["baseline/measurement.json"] = json.dumps({**metadata, "score": (
            result.original_base_score if result.seed_score is not None else result.base_score),
            "kind": "first synchronous permuter baseline; importer base.c/base.o are unpaired preparation artifacts"}).encode()
    except Exception as error:
        errors.append(str(error))
    try:
        best_dir, _ = _best(scratch)
        score = result.best_score
        if score is not None and (result.seed_score is not None or result.base_score is None or score < result.base_score):
            source = files.get("context/winner.c") or sweep_receipts.owned_bytes(
                out_dir, (best_dir / "source.c").relative_to(out_dir).as_posix())
            files["best/source.c"] = source
            # The permuter retains source, score and diff, not an object.
            # Compile exactly those bytes once through the saved full recipe.
            compile_dir = out_dir / ("artifact-compile-" + uuid.uuid4().hex)
            compile_dir.mkdir(mode=0o700)
            candidate = compile_dir / "source.c"
            obj = compile_dir / "object.o"
            compile_script = compile_dir / "compile.sh"
            candidate.write_bytes(source)
            compile_script.write_bytes(files["baseline/compile.sh"])
            # The importer's realpath-based recipe expects an existing output,
            # exactly as its Compiler's NamedTemporaryFile provides.
            obj.touch(exist_ok=False)
            remaining_timeout(deadline)
            command = bounded_capture(["bash", str(compile_script), str(candidate),
                                       "-o", str(obj)], deadline, check=True)
            (out_dir / "artifact-compile.log").write_text(command.stdout)
            if sweep_receipts.owned_bytes(compile_dir, candidate.name) != source:
                raise RuntimeError("saved best source changed during compilation")
            files["best/object.o"] = sweep_receipts.owned_bytes(compile_dir, obj.name)
            if not files["best/object.o"]:
                raise RuntimeError("saved best compiler produced an empty object")
            files["best/measurement.json"] = json.dumps({"search_score": score,
                "kind": "unchanged saved source recompiled at a diagnostic path; not the original scored object",
                "path_sensitive_equivalence_proved": False}).encode()
        else:
            files["best/source.c"] = files["baseline/compiled.c"]
            files["best/object.o"] = files["baseline/compiled.o"]
            files["best/measurement.json"] = files["baseline/measurement.json"]
    except Exception as error:
        errors.append(str(error))
        output = getattr(error, "output", None)
        if output:
            (out_dir / "artifact-compile.log").write_bytes(output.encode() if isinstance(output, str) else output)
    attempts, copy_errors = sweep_receipts.attempt_files(out_dir,
        byte_budget=sweep_receipts.MAX_PAYLOAD_BYTES - sum(map(len, files.values())) - 65536,
        entry_budget=sweep_receipts.MAX_ARTIFACT_ENTRIES - len(files) - 1,
        deadline=time.monotonic() + 5)
    files.update(attempts)
    errors.extend(copy_errors)
    complete = not errors and sweep_receipts.REQUIRED_ARTIFACTS <= files.keys()
    if not complete:
        result.ok = False
        result.error = (result.error + "; " if result.error else "") + "artifact preservation incomplete: " + "; ".join(errors)
        if deadline is not None and time.monotonic() >= deadline:
            result.stopped_batch = True
    files["artifact-status.json"] = json.dumps({"complete": complete, "errors": errors,
                                                "search_error": result.error}).encode()
    result.artifact_bundle = store.save_bundle(files, complete=complete, inputs=inputs)


def run_one(item: QueueItem, minutes: int, permuter_threads: int, build_jobs: int, apply: bool,
            extra_args: list[str], load_threshold: float = 0.0, extend_minutes: int = 0,
            commit: bool = False, flat_minutes: int = 0,
            annotate_overlays: bool = True,
            batch_deadline: Optional[float] = None, resume: bool = False,
            receipt_store: Optional[sweep_receipts.ReceiptStore] = None,
            deep: bool = False, seed_receipt: Optional[str] = None,
            seed_candidate: Optional[str] = None) -> RunResult:
    # Keep every meaningful attempt. Reimporting must not erase an earlier
    # best candidate, especially when a later run fails before scoring.
    out_dir = BUILD_PERMUTER / item.func / "runs" / uuid.uuid4().hex
    out_dir.mkdir(parents=True, exist_ok=True)
    result = RunResult(func=item.func, c_file=item.rel_c_file, overlay=item.overlay, ok=False)
    start = time.monotonic()
    store = receipt_store or sweep_receipts.ReceiptStore.for_repo(ROOT)
    baseline = {}
    preservation_started = False
    try:
        if seed_receipt and (deep or extra_args or extend_minutes):
            raise ValueError("receipt seeding does not accept --deep, extensions or forwarded permuter arguments")
        if seed_candidate is not None and not seed_receipt:
            raise ValueError("--seed-candidate requires --seed-receipt")
        parent, seed_source, selection = store.select_seed(seed_receipt, seed_candidate) if seed_receipt else (None, None, None)
        remaining_timeout(batch_deadline)
        checked_tool_identity()
        source_hash = sweep_receipts.file_digest(item.c_file)
        recipe = build_recipe_for(item.c_file, batch_deadline)
        if not recipe.from_dry_run or not recipe.compiler_args:
            raise RuntimeError("no supported complete IDO recipe; refusing a guessed scratch command")
        initial_dependencies = source_dependencies(item.c_file, recipe.compiler_args, batch_deadline)
        result.flags = " ".join(recipe.flags)
        result.replicated_objcopy = len(recipe.objcopy_steps)
        settings_path = out_dir / "permuter_settings.toml"
        write_settings_toml(settings_path, recipe.flags, recipe=recipe)
        target_asm = prepare_target_asm(item, out_dir)
        # import.py uses nonmatchings/<symbol> internally, even when two TUs
        # happen to define the same symbol. Serialize only this short step.
        while not _IMPORT_LOCK.acquire(timeout=remaining_timeout(batch_deadline, 0.25)):
            pass
        try:
            scratch = run_import(item, out_dir, settings_path, target_asm, batch_deadline)
        finally:
            _IMPORT_LOCK.release()
        result.scratch_path = str(scratch)
        alias_history: list = []
        replicate_objcopy(scratch, recipe, item.c_file, out_dir,
                          alias_history if annotate_overlays and item.overlay is not None else None)
        if annotate_overlays:
            result.annotated_relocs = annotate_overlay_scratch(item, scratch, out_dir, batch_deadline, alias_history)
        inputs = receipt_inputs(item, scratch, settings_path, target_asm, recipe, {
            "minutes": minutes, "threads": permuter_threads, "extra_args": extra_args,
            "extend_minutes": extend_minutes, "flat_minutes": flat_minutes,
            "annotate_overlays": annotate_overlays,
            "mandatory_args": list(MANDATORY_PERMUTER_ARGS),
        })
        if source_hash != inputs["context"]["source"]:
            raise RuntimeError("source changed during sweep preparation; retry against stable input")
        if initial_dependencies != inputs["context"]["dependencies"]:
            raise RuntimeError("headers changed during sweep preparation")
        checked_tool_identity()
        readiness = grouped_baseline_fidelity(item, out_dir, scratch, inputs, batch_deadline)
        if parent is not None:
            if not seed_context_compatible(parent["inputs"], inputs):
                raise RuntimeError("seed source, ownership, headers, recipe, target or external tools are stale")
            prepared_seed, seed_plan = prepare_seed_layout(item, out_dir / "seed-layout",
                                                         seed_source, batch_deadline)
            baseline.update({"seed/prepared.c": prepared_seed, "seed/plan.json": seed_plan})
            seed_identity = {"receipt": seed_receipt,
                             "bundle": parent["result"]["artifact_bundle"],
                             "source_sha256": hashlib.sha256(seed_source).hexdigest(),
                             "preparation_contract": SEED_FIDELITY_CONTRACT,
                             "prepared_source_sha256": hashlib.sha256(prepared_seed).hexdigest(),
                             "plan_sha256": hashlib.sha256(seed_plan).hexdigest(),
                             "target_object_sha256": hashlib.sha256(
                                 sweep_receipts.owned_bytes(scratch, "target.o")).hexdigest()}
            if selection is not None:
                seed_identity["selection"] = selection
                baseline["seed/selection.json"] = json.dumps(selection, sort_keys=True).encode()
            inputs["search"]["seed"] = seed_identity
            inputs["context"]["seed"] = seed_identity
        result.receipt_key = sweep_receipts.digest(inputs)
        with store.claim(result.receipt_key) as acquired:
            if not acquired:
                result.busy = True
            elif deep and not store.descending(inputs["context"]):
                result.deep_skipped = True
            elif resume and (previous := store.completed(result.receipt_key)) is not None:
                result.resumed = True
                result.ok = True
                result.base_score = previous["result"]["base_score"]
                result.best_score = previous["result"]["best_score"]
                result.scratch_path = previous["result"].get("scratch_path")
                result.artifact_bundle = previous["result"]["artifact_bundle"]
                result.context_review = previous["result"]["context_review"]
                for field in ("original_base_score", "seed_score", "seed_parent_score", "seed_proof", "search_gain"):
                    setattr(result, field, previous["result"].get(field))
            else:
                # Freeze baseline bytes before an extension replaces base.c.
                for name in ("base.c", "base.o", "compile.sh", "target.s", "target.o", "settings.toml"):
                    baseline["baseline/" + name] = sweep_receipts.owned_bytes(scratch, name)
                baseline["baseline/tu.c"] = sweep_receipts.owned_bytes(ROOT, item.rel_c_file)
                baseline["baseline/recipe.json"] = json.dumps(dataclasses.asdict(recipe), sort_keys=True).encode()
                baseline["baseline/permuter_settings.toml"] = sweep_receipts.owned_bytes(out_dir, settings_path.name)
                if any(hashlib.sha256(baseline[name]).hexdigest() != expected
                       for name, expected in inputs["baseline_hashes"].items()):
                    raise RuntimeError("prepared baseline changed before readiness measurement")
                canonical = seed = None
                if parent is not None:
                    canonical, seed = prepare_seed(item, out_dir, baseline, inputs,
                        parent, seed_source, result, batch_deadline)
                    validate_seed_layout(item, baseline, inputs, seed_source)
                    (scratch / "base.c").write_bytes(baseline["seed/prepared.c"])
                install_baseline_capture(scratch, out_dir, baseline, inputs)
                launch_inputs = {name: sweep_receipts.owned_bytes(scratch, name)
                                 for name in ("base.c", "compile.sh", "target.s", "target.o", "settings.toml")}
                run_prepared(item, scratch, out_dir, result, minutes, permuter_threads,
                             build_jobs, apply, extra_args, load_threshold, extend_minutes,
                             commit, flat_minutes, batch_deadline, prepared_inputs=inputs,
                             canonical_evidence=canonical, seed_evidence=seed, seed_artifacts=baseline,
                             readiness_evidence=readiness if parent is None else None,
                             launch_inputs=launch_inputs)
                # Concurrent promotion in another slot can change this TU.
                # Such a search remains useful evidence, but cannot suppress
                # a future run against the newly changed source.
                if not result.promoted and sweep_receipts.file_digest(item.c_file) != source_hash:
                    result.error = "source changed while searching; receipt is retryable"
                    result.ok = False
                preservation_started = True
                preserve_search_artifacts(store, out_dir, scratch, baseline, result, batch_deadline, inputs)
                if sweep_tool_identity() != inputs["context"]["tools"]:
                    result.error = "tools changed while searching; receipt is retryable"
                    result.ok = False
                result.seconds = time.monotonic() - start
                store.record(inputs, dataclasses.asdict(result))
    except Exception as e:  # noqa: BLE001 -- report, don't crash the batch
        result.ok = False
        result.error = str(e)
        if batch_deadline is not None and time.monotonic() >= batch_deadline:
            result.stopped_batch = True
        # Even failures before identity preparation retain their actual files.
        # They never enter the reusable context index.
        try:
            if preservation_started:
                raise RuntimeError("preservation already attempted; refusing a second full scan")
            files, errors = sweep_receipts.attempt_files(out_dir,
                byte_budget=sweep_receipts.MAX_PAYLOAD_BYTES - sum(map(len, baseline.values())) - 65536,
                deadline=time.monotonic() + 5)
            failure_inputs = {"schema": sweep_receipts.SCHEMA,
                              "context": {"unprepared_run": out_dir.name, "func": item.func}}
            result.artifact_bundle = store.save_bundle({**baseline, **files}, complete=False, inputs=failure_inputs)
            store.record(failure_inputs, dataclasses.asdict(result))
        except Exception as preservation_error:
            result.error += f"; durable preservation failed: {preservation_error}; local evidence: {out_dir}"
    result.seconds = time.monotonic() - start
    sweep_receipts.atomic_json(out_dir / "result.json", dataclasses.asdict(result))
    return result


def run_prepared(item: QueueItem, scratch: Path, out_dir: Path, result: RunResult,
                 minutes: int, permuter_threads: int, build_jobs: int, apply: bool,
                 extra_args: list[str], load_threshold: float, extend_minutes: int,
                 commit: bool, flat_minutes: int, batch_deadline: Optional[float], *,
                 prepared_inputs: dict | None = None,
                 canonical_evidence: PreparedBaseline | None = None,
                 seed_evidence: PreparedBaseline | None = None,
                 seed_artifacts: dict | None = None,
                 readiness_evidence: tuple[PreparedBaseline, int | None] | None = None,
                 launch_inputs: dict[str, bytes] | None = None) -> None:
    prepared = canonical_evidence
    try:
        if seed_evidence is not None and sweep_receipts.owned_bytes(scratch, "base.c") != seed_artifacts["seed/prepared.c"]:
            raise RuntimeError("prepared seed source changed before search")
        if seed_artifacts is not None:
            expected_source = seed_artifacts["seed/prepared.c"] if seed_evidence is not None else seed_artifacts["baseline/base.c"]
            if sweep_receipts.owned_bytes(scratch, "base.c") != expected_source:
                raise RuntimeError("prepared baseline source changed before search")
            for name in ("target.s", "target.o", "settings.toml"):
                if "baseline/" + name in seed_artifacts and sweep_receipts.owned_bytes(scratch, name) != seed_artifacts["baseline/" + name]:
                    raise RuntimeError("prepared baseline input changed before search: " + name)
        # Keep the current launch files pinned across compile-only readiness and
        # the potentially long load wait. The installed capture wrapper itself
        # is part of this snapshot; archived/foreign scripts are never invoked.
        launch_files = launch_inputs if launch_inputs is not None else {name: sweep_receipts.owned_bytes(scratch, name)
                        for name in ("base.c", "compile.sh", "target.s", "target.o", "settings.toml")
                        if (scratch / name).exists()}
        if seed_evidence is not None:
            require_search_context(item, prepared, out_dir / "baseline-readiness", batch_deadline)
            require_search_context(item, seed_evidence, out_dir / "seed-readiness", batch_deadline)
        else:
            if readiness_evidence is None:
                if seed_artifacts is not None:
                    readiness_evidence = measure_seed_stage(item, out_dir / "baseline-measurement",
                        seed_artifacts, prepared_inputs, seed_artifacts["baseline/base.c"], batch_deadline)
                else:
                    # Direct callers may already own an authenticated capture.
                    readiness_evidence = (captured_baseline(item, out_dir, prepared_inputs, batch_deadline), None)
            prepared = readiness_evidence[0]
            require_search_context(item, prepared, out_dir / "baseline-readiness", batch_deadline)
        wait_for_headroom(load_threshold, f"before permuting {item.func}", batch_deadline)
        validate_baseline(item, prepared, batch_deadline)
        if seed_evidence is not None:
            validate_baseline(item, seed_evidence, batch_deadline)
        if any(sweep_receipts.owned_bytes(scratch, name) != data for name, data in launch_files.items()):
            raise RuntimeError("prepared search input changed during baseline readiness")
        if seed_evidence is not None and hashlib.sha256(sweep_receipts.owned_bytes(
                scratch, "target.o")).hexdigest() != prepared_inputs["search"]["seed"]["target_object_sha256"]:
            raise RuntimeError("seed target object changed before search")
        base_score, elapsed, stopped_flat, stopped_batch = run_permuter(
            scratch, out_dir, minutes, permuter_threads, extra_args,
            flat_minutes=flat_minutes, batch_deadline=batch_deadline)
        result.base_score = base_score
        result.stopped_flat = stopped_flat
        result.stopped_batch = stopped_batch
        result.ok = True
        try:
            if any(sweep_receipts.owned_bytes(scratch, name) != data for name, data in launch_files.items()):
                raise RuntimeError("prepared search input changed during search")
            search_capture = captured_baseline(item, out_dir, prepared_inputs, batch_deadline)
            if seed_evidence is not None:
                if (search_capture.source != seed_evidence.source or base_score != result.seed_score):
                    raise RuntimeError("new search baseline differs from independently measured seed")
                search_fidelity = validate_seed_search(item, out_dir, seed_evidence,
                    search_capture, prepared_inputs, batch_deadline)
                if hashlib.sha256(sweep_receipts.owned_bytes(scratch, "target.o")).hexdigest() != prepared_inputs["search"]["seed"]["target_object_sha256"]:
                    raise RuntimeError("seed target object changed during search")
                validate_baseline(item, prepared, batch_deadline)
                seed_artifacts.update({"seed/search.c": search_capture.source,
                                       "seed/search.o": search_capture.object,
                                       "seed/search-fidelity.json": json.dumps(search_fidelity, sort_keys=True).encode()})
                result.seed_proof.update(status="validated", search_object_sha256=search_capture.object_sha256)
                result.seed_proof["search_fidelity"] = search_fidelity
            else:
                if (search_capture.source != readiness_evidence[0].source
                        or readiness_evidence[1] is not None and base_score != readiness_evidence[1]):
                    raise RuntimeError("actual search baseline differs from context readiness measurement")
                prepared = search_capture
        except Exception as error:
            if seed_evidence is None:
                prepared = None
            raise RuntimeError(f"prepared baseline unverifiable: {error}") from error

        best_dir, best_score = _best(scratch)
        if seed_evidence is not None and (best_score is None or best_score >= base_score):
            best_dir, best_score = scratch, base_score
            (scratch / "source.c").write_bytes(seed_evidence.source)
        result.best_score = best_score
        # Score-trend extension: if the run hit the cap and its best result
        # landed in the final third of the window, the search was still
        # descending -- re-seed from the best candidate and run once more.
        # A search that found its best early and then sat is not extended
        # (docs/permute-batch.md: search time is not the fix for a plateau).
        if (not stopped_batch
                and (batch_deadline is None or time.monotonic() < batch_deadline)
                and extend_minutes > 0 and best_dir is not None
                and best_score not in (None, 0)
                and elapsed >= minutes * 60 * 0.95):
            age = time.time() - best_dir.stat().st_mtime
            if age < minutes * 60 / 3:
                if seed_artifacts is None:
                    raise RuntimeError("extension requires frozen current preparation artifacts")
                if any(sweep_receipts.owned_bytes(scratch, name) != data for name, data in launch_files.items()):
                    raise RuntimeError("prepared search input changed before extension")
                extension_source = sweep_receipts.owned_bytes(best_dir, "source.c")
                extension_prepared, _ = prepare_seed_layout(item, out_dir / "extension-layout",
                                                           extension_source, batch_deadline)
                extension, extension_score = measure_seed_stage(item, out_dir / "extension-measurement",
                    seed_artifacts, prepared_inputs, extension_prepared, batch_deadline)
                extension_report = review_context(item, prepared, extension.source, batch_deadline)
                retain_context(out_dir / "extension-readiness", prepared, extension.source, extension_report)
                if extension_report["status"] != "unchanged":
                    raise RuntimeError("extension context readiness refused: " + str(extension_report.get("reason")))
                if extension_score != best_score:
                    raise RuntimeError("extension emission score differs from saved winner")
                prove_seed_emission(item, out_dir / "extension-fidelity", seed_artifacts,
                    prepared_inputs, extension_source, extension, batch_deadline)
                extension_dir = out_dir / "extension-search"
                extension_dir.mkdir(mode=0o700)
                (scratch / "base.c").write_bytes(extension_prepared)
                install_baseline_capture(scratch, extension_dir, seed_artifacts, prepared_inputs)
                # Carry the original target/settings expectations across the
                # measurement as well; do not bless a drifted file by taking
                # another mutable snapshot after that subprocess returns.
                extension_files = {**launch_files, "base.c": extension_prepared,
                                   "compile.sh": sweep_receipts.owned_bytes(scratch, "compile.sh")}
                wait_for_headroom(load_threshold, f"before extending {item.func}", batch_deadline)
                validate_baseline(item, prepared, batch_deadline)
                validate_baseline(item, extension, batch_deadline)
                if any(sweep_receipts.owned_bytes(scratch, name) != data for name, data in extension_files.items()):
                    raise RuntimeError("prepared extension input changed during readiness")
                result.extended = True
                actual_score, _, _, extension_stopped_batch = run_permuter(
                    scratch, extension_dir, extend_minutes, permuter_threads, extra_args,
                    "permuter-extend.log", batch_deadline=batch_deadline)
                result.stopped_batch = extension_stopped_batch
                if any(sweep_receipts.owned_bytes(scratch, name) != data for name, data in extension_files.items()):
                    raise RuntimeError("prepared extension input changed during search")
                actual_extension = captured_baseline(item, extension_dir, prepared_inputs, batch_deadline)
                if actual_extension.source != extension.source or actual_score != extension_score:
                    raise RuntimeError("actual extension baseline differs from readiness measurement")
                validate_seed_search(item, extension_dir, extension, actual_extension,
                                     prepared_inputs, batch_deadline)
                best_dir, best_score = _best(scratch)
                result.best_score = best_score
        # A base score of zero means the candidate already scores exact in the
        # scratch (only ownership/relocation can still fail verify): promote
        # it as-is instead of reporting "no improvement".
        if base_score == 0 and best_dir is None:
            best_dir, best_score = scratch, 0
            if prepared is not None:
                (scratch / "source.c").write_bytes(seed_evidence.source if seed_evidence else prepared.source)
        result.best_score = best_score
        if seed_evidence is not None:
            result.search_gain = result.seed_score - best_score
        frozen_winner = (sweep_receipts.owned_bytes(best_dir, "source.c", limit=4 * 1024 * 1024)
                         if best_dir is not None and (seed_evidence is not None or best_score == 0 or
                             best_score is not None and best_score < base_score)
                         and (best_dir / "source.c").exists()
                         else prepared.source if prepared is not None else b"")
        result.context_review = review_context(item, prepared, frozen_winner, batch_deadline)
        retain_context(out_dir / "context-review", prepared, frozen_winner, result.context_review)
        if best_score == 0 and best_dir is not None:
            result.zero_found = True
            if apply and not result.stopped_batch:
                if batch_deadline is not None and time.monotonic() >= batch_deadline:
                    result.stopped_batch = True
                else:
                    wait_for_headroom(
                        load_threshold, f"before promoting {item.func}", batch_deadline)
                    promoted, err = promote(item, best_dir / "source.c", build_jobs,
                                            batch_deadline, commit=commit, evidence=prepared,
                                            winner_bytes=frozen_winner)
                    result.promoted = promoted
                    result.promote_error = err
    except Exception as e:  # noqa: BLE001 -- report, don't crash the batch
        result.ok = False
        result.error = str(e)
        if batch_deadline is not None and time.monotonic() >= batch_deadline:
            result.stopped_batch = True
    finally:
        if result.context_review is None:
            if prepared is None:
                try:
                    # A child can fail after a successful synchronous compile
                    # but before returning its score. Recover only authenticated
                    # evidence; this never clears the failure or enables apply.
                    prepared = captured_baseline(item, out_dir, prepared_inputs, batch_deadline)
                except Exception as error:
                    result.error = ((result.error + "; ") if result.error else "") + \
                        f"prepared baseline recovery unavailable: {error}"
            frozen_winner = seed_evidence.source if seed_evidence is not None else prepared.source if prepared is not None else b""
            if seed_evidence is not None:
                result.best_score = result.seed_score
            try:
                best_dir, score = _best(scratch)
                if seed_evidence is not None:
                    if best_dir is not None and score is not None and score < result.seed_score:
                        # Freeze the source before changing its paired metric.
                        # Failed/flat/regressing attempts remain separate files.
                        frozen_winner = sweep_receipts.owned_bytes(best_dir, "source.c", limit=4 * 1024 * 1024)
                        result.best_score = score
                elif score is not None:
                    result.best_score = score
                if seed_evidence is None and best_dir is not None and score is not None and (result.base_score is None or score < result.base_score):
                    frozen_winner = sweep_receipts.owned_bytes(best_dir, "source.c", limit=4 * 1024 * 1024)
            except (OSError, ValueError):
                pass
            result.context_review = review_context(item, prepared, frozen_winner, batch_deadline)
            retain_context(out_dir / "context-review", prepared, frozen_winner, result.context_review)


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------


def parse_args(argv: list[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--overlay", type=int, help="restrict to one overlay number")
    p.add_argument("--resident-only", action="store_true",
                   help="skip overlay functions")
    p.add_argument("--overlays-only", action="store_true",
                   help="run only overlay functions (the relocation-annotated "
                        "scratch makes their score meaningful; see "
                        "docs/permute-batch.md)")
    p.add_argument("--no-overlay-annotate", action="store_true",
                   help="do not give an overlay target the module's own "
                        "relocation sites (the pre-annotation behaviour, kept "
                        "as an escape hatch and for before/after measurement)")
    p.add_argument("--function", action="append", default=None,
                   help="restrict to these function names (repeatable)")
    p.add_argument("--seed-receipt", help="continue one --function from a validated immutable receipt winner")
    p.add_argument("--seed-candidate", type=sweep_receipts.seed_candidate_name,
                   help="with --seed-receipt: select a retained output-SCORE-ORDINAL source/score pair, not the parent's best")
    p.add_argument(
        "--exclude-file",
        action="append",
        default=[],
        type=Path,
        metavar="PATH",
        help="skip function names listed one per line in PATH; repeatable. This is "
        "operator scheduling only and never marks a function matched",
    )
    p.add_argument("--limit", type=int, help="cap the number of functions processed")
    p.add_argument("--minutes", type=int, default=20, help="per-function wall-clock cap (default: 20)")
    p.add_argument(
        "--max-total-minutes",
        type=int,
        default=120,
        help="whole-batch wall-clock cap across the queue (default: 120; must be positive)",
    )
    p.add_argument("--jobs", type=int, default=1, help="concurrent functions (default: 1)")
    p.add_argument(
        "--permuter-threads",
        type=int,
        help="threads per permuter.py instance (-j); default: split (ncpu - 2) across --jobs",
    )
    p.add_argument(
        "--build-jobs",
        type=int,
        default=6,
        help="gmake -jN to use when promoting a zero-score match (default: 6, the "
        "integration-build cap that keeps the workstation responsive)",
    )
    p.add_argument(
        "--order",
        choices=["ranking", "trial", "queue"],
        default="ranking",
        help="ranking: closest first by config/nonmatching-ranking.us.json differing_words "
        "(unranked last, then by name); trial: closest first by "
        "build/promotion-trial.json's in-range word count, which is the linked-ROM "
        "measurement and the right order for overlays (a candidate the trial could not "
        "link runs last); queue: discovery order (default: ranking)",
    )
    p.add_argument(
        "--resume",
        action="store_true",
        help="after preparing the baseline, skip completed searches with identical "
        "source/compiler/permuter/settings receipts in Git's common directory",
    )
    p.add_argument(
        "--extend-minutes",
        type=int,
        default=0,
        help="if a run hits its cap while its score was still descending (best result in "
        "the last third of the window), re-seed from the best candidate and run this many "
        "more minutes once (default: 0 = off)",
    )
    p.add_argument(
        "--flat-minutes",
        type=int,
        default=6,
        help="stop a search early when it has produced no improvement at all by this "
        "many minutes (default: 6; 0 disables). Flat-at-six searches were flat-at-twenty "
        "on every measured run",
    )
    p.add_argument(
        "--load-threshold",
        type=float,
        default=9.0,
        help="wait until the 1-min load average is below this before each permuter launch "
        "and each promotion build (default: 9.0; 0 disables)",
    )
    p.add_argument(
        "--integration-ref",
        default=DEFAULT_INTEGRATION_REF,
        help="local ref used to skip stale lane candidates already resolved "
        f"there (default: {DEFAULT_INTEGRATION_REF}; no fetch is performed)",
    )
    p.add_argument(
        "--no-integration-ref-filter",
        action="store_true",
        help="disable the stale-lane integration-ref filter",
    )
    p.add_argument(
        "--commit",
        action="store_true",
        help="with --apply: git-commit each verified promotion and its derived metadata "
        "through an isolated index as 'Match <fn> (permuter)'",
    )
    p.add_argument(
        "--apply",
        action="store_true",
        help="on a zero-score result, splice the winning candidate into the C file, "
        "rebuild, and verify byte-identity before reporting it matched",
    )
    p.add_argument(
        "--deep",
        action="store_true",
        help="second pass: search only exact source/tool contexts with a durable "
        "descending receipt (0 < best < base); pair with longer --minutes/--extend-minutes",
    )
    p.add_argument("--list", action="store_true", help="print the discovered queue and exit")
    p.add_argument(
        "permuter_args",
        nargs=argparse.REMAINDER,
        help="extra args forwarded to permuter.py, after --",
    )
    args = p.parse_args(argv)
    if args.seed_candidate is not None and not args.seed_receipt:
        p.error("--seed-candidate requires --seed-receipt")
    if args.seed_receipt:
        if not re.fullmatch(r"[0-9a-f]{64}", args.seed_receipt):
            p.error("--seed-receipt requires a lowercase SHA256 receipt key")
        if (not args.function or len(set(args.function)) != 1 or args.jobs != 1
                or args.deep or args.permuter_args or args.extend_minutes):
            p.error("--seed-receipt requires one --function, --jobs 1, no --deep, extension or forwarded arguments")
    return args


def ncpu() -> int:
    return os.cpu_count() or 4


def main(argv: list[str]) -> int:
    # Separate worktrees remain independent. Two batches in this worktree
    # would share importer scratch, generated objects and summary files.
    if parse_args(argv).list:
        return run_batch(argv)
    lock_path = subprocess.check_output(
        ["git", "rev-parse", "--git-path", "mickey-permute-batch.lock"],
        cwd=ROOT, text=True).strip()
    path = Path(lock_path)
    if not path.is_absolute():
        path = ROOT / path
    with path.open("a") as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            print("another permuter batch owns this worktree; use a disjoint lane", file=sys.stderr)
            return 2
        previous_handlers = {}
        CANCEL_EVENT.clear()
        if threading.current_thread() is threading.main_thread():
            for sig in (signal.SIGINT, signal.SIGTERM):
                previous_handlers[sig] = signal.getsignal(sig)
                signal.signal(sig, lambda _sig, _frame: CANCEL_EVENT.set())
        try:
            result = run_batch(argv)
            return 130 if CANCEL_EVENT.is_set() else result
        finally:
            for sig, handler in previous_handlers.items():
                signal.signal(sig, handler)


def run_batch(argv: list[str]) -> int:
    args = parse_args(argv)
    extra_args = args.permuter_args
    if extra_args and extra_args[0] == "--":
        extra_args = extra_args[1:]

    queue = discover_queue()
    if args.overlay is not None:
        queue = [it for it in queue if it.overlay == args.overlay]
    if args.resident_only:
        queue = [it for it in queue if it.overlay is None]
    if args.overlays_only:
        queue = [it for it in queue if it.overlay is not None]
    if args.function is not None:
        wanted = set(args.function)
        queue = [it for it in queue if it.func in wanted]
        missing = wanted - {it.func for it in queue}
        if missing:
            print(f"--function: not in the live queue: {', '.join(sorted(missing))}")
    try:
        excluded_names = read_excluded_functions(args.exclude_file)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    excluded_items = [it for it in queue if it.func in excluded_names]
    if excluded_items:
        queue = [it for it in queue if it.func not in excluded_names]
        print(
            f"exclude-file: skipping {len(excluded_items)} "
            "explicitly parked function(s)"
        )
        if args.list or args.function is not None:
            for item in excluded_items:
                print(f"  excluded: {item.func}  ({item.rel_c_file})")
    resolved_elsewhere: list[QueueItem] = []
    integration_oid = None
    if not args.no_integration_ref_filter:
        queue, resolved_elsewhere, integration_oid = exclude_resolved_on_ref(
            queue, args.integration_ref
        )
    if resolved_elsewhere:
        pinned = integration_oid[:12] if integration_oid else "unknown"
        print(
            f"integration-ref: skipping {len(resolved_elsewhere)} function(s) already "
            f"resolved on {args.integration_ref} ({pinned})"
        )
        if args.list or args.function is not None:
            for item in resolved_elsewhere:
                print(f"  resolved: {item.func}  ({item.rel_c_file})")
    resolved_keys = {(it.rel_c_file, it.func) for it in resolved_elsewhere}
    if args.order == "trial":
        # The trial's in-range word count is the linked-ROM distance, which is
        # what "closest" means for an overlay; the static ranking is derived
        # from the object and does not see the link.
        trial: dict[str, tuple[int, int]] = {}
        if TRIAL_JSON.is_file():
            for row in json.loads(TRIAL_JSON.read_text()).get("results", []):
                words = row.get("in_range_words")
                if isinstance(words, int) and not row.get("error"):
                    trial[row["func"]] = (words, row.get("out_of_range_bytes") or 0)
        else:
            print(f"note: {TRIAL_JSON.relative_to(ROOT)} does not exist; "
                  "--order trial degenerates to name order")
        queue.sort(key=lambda it: (trial.get(it.func, (10**9, 0)), it.func))
        unmeasured = sum(1 for it in queue if it.func not in trial)
        if unmeasured:
            print(f"note: {unmeasured} queued function(s) have no trial row; they run last")
    elif args.order == "ranking":
        rank: dict[str, tuple[int, int]] = {}
        if RANKING_PATH.is_file():
            for row in json.loads(RANKING_PATH.read_text()).get("functions", []):
                words = row.get("differing_words")
                if isinstance(words, int):
                    rank[row["name"]] = (words, row.get("size_bytes") or 0)
        queue.sort(key=lambda it: (rank.get(it.func, (10**9, 0)), it.func))
        unranked = sum(1 for it in queue if it.func not in rank)
        if unranked:
            print(f"note: {unranked} queued function(s) have no ranking row; they run last")
    prior = json.loads(SUMMARY_JSON.read_text()).get("results", []) if SUMMARY_JSON.is_file() else []
    if args.resume or args.deep:
        print("receipt selection: checked after baseline preparation; local summary rows do not skip work")
    if args.limit is not None and (args.list or not (args.resume or args.deep)):
        queue = queue[: args.limit]
    if args.commit and not args.apply:
        print("--commit requires --apply", file=sys.stderr)
        return 2
    if args.max_total_minutes <= 0:
        print("--max-total-minutes must be positive", file=sys.stderr)
        return 2
    if args.minutes <= 0 or args.extend_minutes < 0 or args.flat_minutes < 0:
        print("--minutes must be positive; extension and flat caps cannot be negative", file=sys.stderr)
        return 2
    if args.jobs <= 0 or (args.permuter_threads is not None and args.permuter_threads <= 0):
        print("search and thread counts must be positive", file=sys.stderr)
        return 2
    if args.limit is not None and args.limit <= 0:
        print("--limit must be positive", file=sys.stderr)
        return 2

    if args.list:
        print(f"{len(queue)} queued function(s):")
        for it in queue:
            ov = f"o{it.overlay:03d}" if it.overlay is not None else "?"
            print(f"  [{ov}] {it.func}  ({it.rel_c_file})")
        if not queue:
            print("nothing to do.")
        return 0
    if args.seed_receipt and len(queue) != 1:
        print("--seed-receipt requires exactly one live filtered target", file=sys.stderr)
        return 2
    if not queue:
        print("0 queued function(s):")
        print("nothing to do.")
        if (args.resume or args.deep) and prior and resolved_keys:
            known = {f.name for f in dataclasses.fields(RunResult)}
            retained = [
                RunResult(**{k: v for k, v in row.items() if k in known})
                for row in prior
                if (row.get("c_file"), row.get("func")) not in resolved_keys
            ]
            if len(retained) != len(prior):
                BUILD_PERMUTER.mkdir(parents=True, exist_ok=True)
                write_summary(retained, final=True)
                print(
                    f"summary: removed {len(prior) - len(retained)} row(s) resolved "
                    f"on {args.integration_ref}"
                )
        return 0

    jobs = max(1, args.jobs)
    permuter_threads = args.permuter_threads
    if permuter_threads is None:
        budget = max(ncpu() - 2, 1)
        permuter_threads = max(1, budget // jobs)

    BUILD_PERMUTER.mkdir(parents=True, exist_ok=True)
    results: list[RunResult] = []
    current_results: list[RunResult] = []
    if (args.resume or args.deep) and prior:
        # Carry the earlier results forward so summary.json stays the whole
        # sweep's record; a function about to be re-run keeps only its new row.
        known = {f.name for f in dataclasses.fields(RunResult)}
        rerun = {it.func for it in queue}
        for row in prior:
            key = (row.get("c_file"), row.get("func"))
            if row.get("func") not in rerun and key not in resolved_keys:
                results.append(RunResult(**{k: v for k, v in row.items() if k in known}))

    print(
        f"Running {len(queue)} function(s), {jobs} concurrent, "
        f"{permuter_threads} permuter thread(s) each, {args.minutes} min cap each, "
        f"{args.max_total_minutes} min whole-batch cap"
        + (", apply=on" if args.apply else ", apply=off (report only)")
    )

    batch_deadline = time.monotonic() + args.max_total_minutes * 60
    scheduled = 0
    attempted = 0

    def counts_against_limit(result: RunResult) -> bool:
        return not (result.resumed or result.busy or result.deep_skipped)

    if jobs == 1:
        for it in queue:
            if CANCEL_EVENT.is_set() or time.monotonic() >= batch_deadline:
                break
            if args.limit is not None and attempted >= args.limit:
                break
            scheduled += 1
            r = run_one(it, args.minutes, permuter_threads, args.build_jobs, args.apply, extra_args,
                        args.load_threshold, args.extend_minutes, args.commit, args.flat_minutes,
                        not args.no_overlay_annotate, batch_deadline, args.resume,
                        deep=args.deep, seed_receipt=args.seed_receipt,
                        seed_candidate=args.seed_candidate)
            results.append(r)
            current_results.append(r)
            attempted += counts_against_limit(r)
            print_result(r)
            write_summary(results)
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            queue_iter = iter(queue)
            futures: dict[concurrent.futures.Future[RunResult], QueueItem] = {}

            def submit_next() -> bool:
                nonlocal scheduled
                if CANCEL_EVENT.is_set() or time.monotonic() >= batch_deadline:
                    return False
                # Reserve room for every in-flight preparation: each may
                # become a real search once its receipt has been checked.
                if args.limit is not None and attempted + len(futures) >= args.limit:
                    return False
                try:
                    item = next(queue_iter)
                except StopIteration:
                    return False
                future = pool.submit(
                    run_one, item, args.minutes, permuter_threads, args.build_jobs, args.apply, extra_args,
                    args.load_threshold, args.extend_minutes, args.commit, args.flat_minutes,
                    not args.no_overlay_annotate, batch_deadline, args.resume,
                    deep=args.deep,
                )
                futures[future] = item
                scheduled += 1
                return True

            for _ in range(min(jobs, len(queue))):
                if not submit_next():
                    break
            while futures:
                done, _ = concurrent.futures.wait(
                    futures, return_when=concurrent.futures.FIRST_COMPLETED)
                for fut in done:
                    futures.pop(fut)
                    r = fut.result()
                    results.append(r)
                    current_results.append(r)
                    attempted += counts_against_limit(r)
                    print_result(r)
                    write_summary(results)
                    submit_next()

    if scheduled < len(queue):
        reason = "search limit" if args.limit is not None and attempted >= args.limit else "batch cap"
        print(
            f"{reason}: stopped after preparing {scheduled}/{len(queue)} function(s); "
            "use --resume for the remainder"
        )

    write_summary(results, final=True)
    print(f"\nSummary written to {SUMMARY_JSON.relative_to(ROOT)} and {SUMMARY_TXT.relative_to(ROOT)}")
    print_table(results)
    return 1 if any(r.error or r.promote_error or r.commit_error for r in current_results) else 0


def print_result(r: RunResult) -> None:
    if r.error:
        print(f"[{r.func}] ERROR: {r.error}")
        return
    if r.resumed:
        status = "completed receipt (search skipped)"
    elif r.busy:
        status = "identical search active elsewhere (retry later)"
    elif r.deep_skipped:
        status = "no current descending receipt (deep search skipped)"
    elif r.promoted:
        status = "MATCHED"
    elif r.zero_found:
        status = "zero-found"
    elif r.stopped_batch:
        status = "stopped by batch cap"
    elif r.best_score is None or (r.base_score is not None and r.best_score >= r.base_score):
        status = "flat" if r.stopped_flat else "no improvement"
    else:
        status = f"improved{' (extended)' if r.extended else ''}"
    annotated = (f" reloc-annotated={r.annotated_relocs}"
                 if r.annotated_relocs else "")
    context = f" context={r.context_review['status']}" if r.context_review else ""
    seed = (f" original={r.original_base_score} seed={r.seed_score} new-gain={r.search_gain}"
            if r.seed_score is not None else "")
    print(
        f"[{r.func}] base={r.base_score} best={r.best_score} "
        f"{status}{annotated}{context}{seed} ({r.seconds:.0f}s)"
    )


def print_table(results: list[RunResult]) -> None:
    if not results:
        return
    print(f"\n{'function':<32} {'base':>6} {'best':>6}  {'zero':<5} {'matched':<8} {'time':>7}")
    for r in results:
        print(
            f"{r.func:<32} {str(r.base_score):>6} {str(r.best_score):>6}  "
            f"{'yes' if r.zero_found else 'no':<5} {'yes' if r.promoted else 'no':<8} "
            f"{r.seconds:>6.0f}s"
        )
    n = len(results)
    n_zero = sum(1 for r in results if r.zero_found)
    n_matched = sum(1 for r in results if r.promoted)
    n_err = sum(1 for r in results if r.error)
    print(
        f"\n{n} run, {n_zero} zero-score ({n_zero / n:.0%}), "
        f"{n_matched} promoted to matched ({n_matched / n:.0%}), {n_err} errored"
    )


def write_summary(results: list[RunResult], final: bool = False) -> None:
    payload = {
        "generated_by": "tools/permute_batch.py",
        "final": final,
        "results": [dataclasses.asdict(r) for r in results],
        "totals": {
            "run": len(results),
            "zero_found": sum(1 for r in results if r.zero_found),
            "promoted": sum(1 for r in results if r.promoted),
            "errored": sum(1 for r in results if r.error),
        },
    }
    sweep_receipts.atomic_json(SUMMARY_JSON, payload)
    lines = [
        f"{'function':<32} {'base':>6} {'best':>6}  {'zero':<5} {'matched':<8} {'time':>7}"
    ]
    for r in results:
        lines.append(
            f"{r.func:<32} {str(r.base_score):>6} {str(r.best_score):>6}  "
            f"{'yes' if r.zero_found else 'no':<5} {'yes' if r.promoted else 'no':<8} "
            f"{r.seconds:>6.0f}s"
        )
    SUMMARY_TXT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
