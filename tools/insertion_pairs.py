#!/usr/bin/env python3
"""Read a size-mismatch function's insertion pairs, and name what owns each word.

    tools/insertion_pairs.py <symbol> [--json] [--object PATH] [--trace LOG]
                             [--no-trace]

WHY THIS EXISTS

`web_footprint.py` answers "which colour" and `draw_census.py` "which draw".
Both operate at size delta 0: they move a register, never an instruction. On a
function four, eight or twelve bytes off its target, the masked-word count is
mostly L155 positional shadow -- one extra word shifts every word after it until
a missing word shifts them back -- and the question is not "which colour" but
"which IR construct emitted the extra word". Nothing answered that, so the
small-delta class was dispatched to colour lanes, which cannot move it.

WHAT IT MEASURES

  1. The same register-erased alignment `align_symbol.py` and
     `residual_map.py` use (the same edit script, so the candidate-only and
     target-only offsets agree with theirs by construction).
  2. PAIRS. Walking that script, a pair opens where the two streams stop being
     index-aligned (the first word present on one side only) and closes where
     they are index-aligned again; a pair that never closes runs to the end of
     the function. Inside a pair every positional comparison is shifted.
  3. SHADOW, per pair: positional masked mismatches inside the pair minus the
     aligned disagreement inside it (the one-sided words plus any aligned row
     that really differs). Outside every pair the two streams are
     index-aligned, so there positional and aligned counts are equal by
     construction and the tool checks that they are. `aligned residual after
     shadow` is the positional masked count less every pair's shadow -- the
     number of words that are actually wrong, and equal to align_symbol's
     aligned disagreement (its three buckets plus the one-sided words).
     One pair's shadow can come out slightly negative (-1, -2 measured): a
     shifted positional comparison happens to agree where the alignment
     accepted a differing row to win exact rows elsewhere. It is the same
     effect as align_symbol's negative displacement tax; the function-level
     identity above still holds exactly.
  4. CLASS of each one-sided word, from its encoding: move, stack-load,
     stack-store, load, store, alu, const, branch, call, frame, delay-nop, nop,
     other. Classes only; the instruction itself is never printed.
  5. OWNER of each one-sided word:
       - the source LINE, read from the candidate object's own line table
         (`.mdebug`, which the configured build carries without `-g`; `-g3`
         is not used because it changes the code on 20 of 65 objects.c
         functions). A target-only word has no candidate address, so it takes
         the line of the nearest candidate word the alignment puts beside it,
         and says so (`via neighbour`).
       - the ugen CONSTRUCT at that line: the instrumented ugen's
         `DKWB-EMIT-V1` rows carry a source line, and the `DKWB-CALL` stack at
         each emission names the handler (`iloadistore` for ILOD/ISTR,
         `loadstore` for LOD/STR, `move_to_dest` for a register copy, `jump`
         for FJP/TJP, `gen_reg_save_restore` for callee saves, ...). A word is
         owned when an emission of a compatible family is found; `own` states
         the search order and every answer carries its `basis`: `line` (its
         own line), `prologue` (a save/frame word, which ugen stamps with the
         procedure's last line), `nearest` (within three lines, because as1
         schedules across statements), `as1` (a nop), or, for a target-only
         word only, `neighbour` (our construct at the neighbour's line, no
         same-family emission near it -- the weakest basis, and counted apart).
     Anything else is `unowned`, with the reason. That is reported, not
     papered over.
  6. LABEL per pair, from a fixed vocabulary and a fixed rule (see `LABELS`):
     hoist, unrolled-loop, extra-ILOD, extra-ISTR, missing-CSE,
     split-not-copy, spill/reload, callee-save, control-flow, delay-slot,
     other, or unowned. `pair_label` states the rule; the function's label is that of
     the pair with the most shadow.

WHAT IT CANNOT DO

It reads OUR compile. The target has no trace and no line table, so a
target-only word's construct is what our code does at that line, not what
the target's compiler did there. The label is a rule over the word's class and
owner, not a proof of cause: `missing-CSE` means "a one-sided ALU/const word
owned by an expression handler", and confirming the cause is still a source
edit plus `draw_census.py --compare`. The handler is ugen's; a uopt decision
upstream of it (a CSE, a hoist, an unroll) shows only as its consequence.

REFUSALS

  - size delta 0: there are no pairs; use `residual_map.py`.
  - a force or instrumented-compiler environment without `--object`: this tool
    recompiles the configured TU, which drops the force, so the reading would
    describe the unforced build. Score the forced object with `--object` (and
    its own trace with `--trace`).

Everything above the CLI section is pure and takes word lists and text, so
`tools/test_insertion_pairs.py` needs no baserom, no toolchain and no build.
"""
from __future__ import annotations

import argparse
import collections
import contextlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

SP = 29
RA = 31

# --------------------------------------------------------------------------
# Word classes. Opcode numbers only; no instruction text anywhere.
# --------------------------------------------------------------------------

LOAD_OPS = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x31, 0x35, 0x37}
STORE_OPS = {0x28, 0x29, 0x2A, 0x2B, 0x2E, 0x39, 0x3D, 0x3F}
BRANCH_OPS = {0x01, 0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17}
OP_JUMP, OP_CALL = 0x02, 0x03
OP_ADD_IMM, OP_ADD_IMM_U, OP_OR_IMM, OP_UPPER = 0x08, 0x09, 0x0D, 0x0F
OP_SPECIAL, OP_COP1 = 0x00, 0x11
FN_JR, FN_JALR = 0x08, 0x09
FN_ADD, FN_ADDU, FN_OR = 0x20, 0x21, 0x25
COP1_MOVE_FROM, COP1_MOVE_TO, COP1_BRANCH = 0x00, 0x04, 0x08
COP1_FMT_MOVE = 0x06

CLASSES = ("move", "stack-load", "stack-store", "load", "store", "alu",
           "const", "branch", "call", "frame", "delay-nop", "nop", "other")


def is_control_transfer(word: int) -> bool:
    """A word with a delay slot after it."""
    op = (word >> 26) & 0x3F
    if op in BRANCH_OPS or op in (OP_JUMP, OP_CALL):
        return True
    if op == OP_SPECIAL and (word & 0x3F) in (FN_JR, FN_JALR):
        return True
    return op == OP_COP1 and ((word >> 21) & 0x1F) == COP1_BRANCH


def classify_word(word: int, previous: int | None = None) -> str:
    """One of CLASSES for one encoded word; `previous` decides delay-nop."""
    if word == 0:
        return "delay-nop" if previous is not None and \
            is_control_transfer(previous) else "nop"
    op = (word >> 26) & 0x3F
    rs, rt = (word >> 21) & 0x1F, (word >> 16) & 0x1F
    if op in LOAD_OPS:
        return "stack-load" if rs == SP else "load"
    if op in STORE_OPS:
        return "stack-store" if rs == SP else "store"
    if op in BRANCH_OPS or op == OP_JUMP:
        return "branch"
    if op == OP_CALL:
        return "call"
    if op == OP_SPECIAL:
        funct = word & 0x3F
        rd = (word >> 11) & 0x1F
        if funct == FN_JR:
            return "frame" if rs == RA else "branch"
        if funct == FN_JALR:
            return "call"
        if funct in (FN_ADD, FN_ADDU, FN_OR) and rd and (rs == 0 or rt == 0):
            return "move"
        return "alu"
    if op in (OP_ADD_IMM, OP_ADD_IMM_U):
        if rs == SP and rt == SP:
            return "frame"
        if rs == 0:
            return "const"
        return "alu"
    if op == OP_UPPER or (op == OP_OR_IMM and rs == 0):
        return "const"
    if 0x0A <= op <= 0x0E:
        return "alu"
    if op == OP_COP1:
        sub = rs
        if sub == COP1_BRANCH:
            return "branch"
        if sub in (COP1_MOVE_FROM, COP1_MOVE_TO):
            return "move"
        if sub >= 0x10 and (word & 0x3F) == COP1_FMT_MOVE:
            return "move"
        return "alu"
    return "other"


# --------------------------------------------------------------------------
# Pairs, from the alignment script.
# --------------------------------------------------------------------------

def pairs_from_script(script: list[tuple[str, int, int]]) -> list[dict]:
    """Group one-sided words into pairs of index misalignment.

    `script` is `align_symbol._banded_edit_script` output: (op, i, j) with
    "delete" = candidate-only (candidate index i) and "insert" = target-only
    (target index j). A pair opens at the first one-sided word after an
    index-aligned stretch and closes when the running shift (candidate-only
    minus target-only) is back to zero. `lo`/`hi` are POSITIONAL indices: the
    compared index range whose two words are not each other's alignment
    partners. `hi` is None for a pair that runs to the end.
    """
    pairs: list[dict] = []
    shift = 0
    current = None
    for step, (op, i, j) in enumerate(script):
        if op in ("delete", "insert"):
            if current is None:
                # Index-aligned up to here, so i == j.
                current = {"lo": i, "hi": None, "closed": False, "steps": []}
                pairs.append(current)
            shift += 1 if op == "delete" else -1
            current["steps"].append(step)
            if shift == 0:
                # Index-aligned again from the next word on both sides. A
                # further one-sided word right here opens a NEW pair: the two
                # are separate shifts that happen to touch.
                current["hi"] = i + 1 if op == "delete" else j + 1
                current["closed"] = True
                current = None
        elif current is not None:
            current["steps"].append(step)
    return pairs


def account(pairs: list[dict], script: list[tuple[str, int, int]],
            differing_steps: set[int], positional: list[int],
            n_min: int, extra: int,
            naming_steps: set[int] | None = None) -> dict:
    """Split the positional count into per-pair shadow and aligned residual.

    `differing_steps` are script steps whose aligned pair really differs
    (naming, immediate or structural); one-sided steps always count.
    `naming_steps`, a subset, are the register-naming-only rows: inside a
    pair those are the candidates for a free list the one-sided word rotated
    (the overlay 58 reading), so they are counted separately.
    `positional` is the masked mismatch index list over range(n_min), and
    `extra` the length difference, which the ranking adds to its count.
    """
    in_pair_steps: set[int] = set()
    positional_set = set(positional)
    for pair in pairs:
        steps = set(pair["steps"])
        in_pair_steps |= steps
        aligned_in = sum(1 for s in steps
                         if script[s][0] in ("delete", "insert")
                         or s in differing_steps)
        hi = pair["hi"] if pair["hi"] is not None else n_min
        positional_in = sum(1 for k in positional_set if pair["lo"] <= k < hi)
        if pair["hi"] is None:
            positional_in += extra
        pair["positional_in"] = positional_in
        pair["aligned_in"] = aligned_in
        pair["naming_in"] = len(steps & (naming_steps or set()))
        pair["shadow"] = positional_in - aligned_in
        pair["span_words"] = hi - pair["lo"] + (extra if pair["hi"] is None else 0)
    covered = set()
    for pair in pairs:
        hi = pair["hi"] if pair["hi"] is not None else n_min
        covered.update(range(pair["lo"], hi))
    aligned_out = sum(1 for s in differing_steps if s not in in_pair_steps)
    positional_out = sum(1 for k in positional_set if k not in covered)
    total_positional = len(positional) + extra
    shadow = sum(p["shadow"] for p in pairs)
    return {
        "positional": total_positional,
        "aligned_outside": aligned_out,
        "positional_outside": positional_out,
        "outside_agrees": aligned_out == positional_out,
        "shadow": shadow,
        "aligned_after_shadow": total_positional - shadow,
        "naming_in_pairs": sum(p["naming_in"] for p in pairs),
    }


# --------------------------------------------------------------------------
# Owners: line table and ugen trace.
# --------------------------------------------------------------------------

ADDR_RE = re.compile(r"^\s*([0-9a-f]+):\t")
LINE_RE = re.compile(r"^(\S[^:]*):(\d+)(?: \(discriminator \d+\))?$")


def parse_line_table(objdump_text: str, start: int, size: int,
                     source_name: str) -> dict[int, int | str]:
    """Function-relative offset -> source line, from `objdump -d -l` text.

    Only the address column and the file:line headers are read; the
    instruction text is never kept. A line from a file other than the TU
    (a header) is kept as "name:line" so it is never matched against the
    TU's trace lines.
    """
    table: dict[int, int | str] = {}
    current: int | str | None = None
    for raw in objdump_text.splitlines():
        m = LINE_RE.match(raw)
        if m:
            path, line = m.group(1), int(m.group(2))
            current = line if pathlib.Path(path).name == source_name \
                else f"{pathlib.Path(path).name}:{line}"
            continue
        m = ADDR_RE.match(raw)
        if m and current is not None:
            address = int(m.group(1), 16)
            if start <= address < start + size:
                table[address - start] = current
    return table


FIELD_RE = re.compile(r"(\w+)=(\S+)")
DIRECTIVE_EMITTERS = {"f_emit_dir0", "f_emit_dir1", "f_emit_dir2",
                      "f_emit_dir_ll", "f_demit_dir0", "f_demit_dir1",
                      "f_demit_dir2", "f_define_label", "f_emit_alias",
                      "f_emit_regmask", "f_demit_mask", "f_demit_frame",
                      "f_emit_symbol", "f_emit_optimize_level", "f_emit_pic"}
MEMORY_PARENTS = {"f_iloadistore", "f_loadstore", "f_unaligned_loadstore"}


def emitter_family(fn: str, stack: list[str]) -> str | None:
    """Instruction family of one ugen emission, or None for a directive."""
    if fn in DIRECTIVE_EMITTERS:
        return None
    frames = set(stack)
    if "f_gen_reg_save_restore" in frames:
        return "save"
    if "f_gen_entry_exit" in frames:
        return "frame"
    if frames & MEMORY_PARENTS or fn in ("f_emit_rab", "f_emit_rob",
                                         "f_demit_rob_"):
        return "memory"
    if fn in ("f_emit_rill", "f_emit_rrll", "f_emit_ll", "f_emit_rllb",
              "f_emit_branch_rrll", "f_emit_branch_rill"):
        return "branch"
    if fn == "f_emit_a":
        return "call"
    if fn in ("f_emit_ri_", "f_emit_ra", "f_emit_rfi"):
        return "const"
    if fn == "f_emit_rr" and "f_move_to_dest" in frames:
        return "move"
    return "alu"


def handler_of(stack: list[str]) -> str:
    """The innermost frame that is not an emitter: the ugen construct."""
    for frame in reversed(stack):
        if frame.startswith(("f_emit", "f_demit", "f_dw_emit")):
            continue
        return frame[2:] if frame.startswith("f_") else frame
    return "unknown"


def parse_trace(text: str) -> dict[int, dict]:
    """Per procedure: instruction emissions and draws, keyed by source line.

    Emissions sharing one (block, emit) index are one instruction emitted
    through a wrapper (a branch helper calling the plain emitter); only the
    first is kept.
    """
    procs: dict[int, dict] = {}
    stack: list[str] = []
    for raw in text.splitlines():
        if raw.startswith("DKWB-CALL "):
            parts = raw.split()
            if len(parts) < 3:
                continue
            depth = int(parts[1])
            del stack[depth - 1:]
            if parts[2] == ">" and len(parts) > 3:
                stack.append(parts[3])
            continue
        if raw.startswith("DKWB-EMIT-V1"):
            fields = dict(FIELD_RE.findall(raw))
            fn = fields.get("fn", "")
            family = emitter_family(fn, stack)
            if family is None:
                continue
            proc = procs.setdefault(int(fields.get("proc", -1)),
                                    {"emits": collections.defaultdict(list),
                                     "draws": collections.Counter(),
                                     "seen": set()})
            key = (fields.get("block"), fields.get("emit"))
            if key in proc["seen"]:
                continue
            proc["seen"].add(key)
            proc["emits"][int(fields.get("line", -1))].append(
                {"family": family, "handler": handler_of(stack)})
            continue
        if raw.startswith("DKWB-FREELIST"):
            parts = raw.split()
            if len(parts) > 1 and parts[1] in ("ALLOC_GP_RESULT",
                                               "ALLOC_FP_RESULT"):
                fields = dict(FIELD_RE.findall(raw))
                proc = procs.setdefault(int(fields.get("proc", -1)),
                                        {"emits": collections.defaultdict(list),
                                         "draws": collections.Counter(),
                                         "seen": set()})
                proc["draws"][int(fields.get("line", -1))] += 1
    for proc in procs.values():
        proc.pop("seen", None)
    return procs


def pick_proc(procs: dict[int, dict], lines: set[int]) -> tuple[int | None, str]:
    """The procedure whose emissions fall on this function's lines.

    The trace's ordinal is the Ucode procedure order and carries no name, so
    it is matched by line: the function's line table gives its lines, and the
    procedure with the most emissions on those lines, strictly more than any
    other, is it.
    """
    if not lines:
        return None, "no line table for this function"
    lo, hi = min(lines), max(lines)
    scores = []
    for index, proc in procs.items():
        hits = sum(len(rows) for line, rows in proc["emits"].items()
                   if lo <= line <= hi)
        if hits:
            scores.append((hits, index))
    if not scores:
        return None, "no traced procedure emits on this function's lines"
    scores.sort(reverse=True)
    if len(scores) > 1 and scores[0][0] == scores[1][0]:
        return None, "two traced procedures tie on this function's lines"
    return scores[0][1], "matched by line"


COMPATIBLE = {
    "move": {"move", "alu"},
    "stack-load": {"memory", "save", "frame"},
    "stack-store": {"memory", "save", "frame"},
    "load": {"memory"},
    "store": {"memory"},
    "alu": {"alu", "move", "const"},
    "const": {"const", "memory", "alu", "call"},
    "branch": {"branch"},
    "call": {"call"},
    "frame": {"frame", "save"},
    "other": {"alu", "memory", "move", "const", "branch", "call", "frame",
              "save"},
}


NEAREST_LINES = 3


def _construct(rows: list[dict]) -> tuple[str, int]:
    handlers = collections.Counter(r["handler"] for r in rows)
    return sorted(handlers.items(), key=lambda kv: (-kv[1], kv[0]))[0]


def own(word_class: str, line, proc: dict | None,
        bounds: tuple[int, int] | None = None) -> dict:
    """Owner of one one-sided word: a line plus a construct, or unowned.

    Tried in this order, and the one that answered is `basis`:

      as1       a nop: the assembler's fill, owned once its line is known.
      line      a compatible emission stamped with the word's own line.
      prologue  a stack or frame word on the function's first or last line
                (`bounds`), matched to the save/frame emissions wherever they
                are stamped: ugen emits the prologue and epilogue on reaching
                the END of the procedure, so the trace stamps them with the
                last line while the line table puts the prologue on the first.
      nearest   a compatible emission within NEAREST_LINES lines, nearest
                first (lower line on a tie): as1 schedules across statements,
                so the line table can stamp a word with a neighbouring
                statement's line.
    Otherwise the word is unowned and `reason` says why.
    """
    def unowned(reason):
        return {"owned": False, "line": line, "construct": None,
                "basis": None, "reason": reason}

    if line is None:
        return unowned("no line")
    if word_class in ("nop", "delay-nop"):
        return {"owned": True, "line": line, "construct": "as1",
                "basis": "as1", "reason": "assembler fill"}
    if not isinstance(line, int):
        return unowned("line outside the TU source")
    if proc is None:
        return unowned("no trace")

    def compatible(at):
        return [r for r in proc["emits"].get(at, [])
                if r["family"] in COMPATIBLE[word_class]]

    def owned(rows, basis, reason, at):
        construct, count = _construct(rows)
        return {"owned": True, "line": line, "construct": construct,
                "basis": basis, "construct_line": at,
                "reason": reason.format(count=count, total=len(rows), at=at),
                "draws_on_line": proc["draws"].get(at, 0)}

    rows = compatible(line)
    if rows:
        return owned(rows, "line", "{count} of {total} compatible emissions",
                     line)
    if bounds and line in bounds and \
            word_class in ("stack-load", "stack-store", "frame"):
        rows = [r for rs in proc["emits"].values() for r in rs
                if r["family"] in ("save", "frame")]
        if rows:
            return owned(rows, "prologue",
                         "prologue/epilogue, traced at the procedure's end",
                         line)
    for distance in range(1, NEAREST_LINES + 1):
        for at in (line - distance, line + distance):
            if bounds and not bounds[0] <= at <= bounds[1]:
                continue
            rows = compatible(at)
            if rows:
                return owned(rows, "nearest",
                             "no compatible emission on its line; nearest at "
                             "line {at}", at)
    return unowned(f"no compatible emission within {NEAREST_LINES} lines")


# The label rule, in full. Order in LABELS breaks ties.
LABELS = ("hoist", "unrolled-loop", "extra-ILOD", "extra-ISTR", "missing-CSE",
          "split-not-copy", "spill/reload", "callee-save", "control-flow",
          "delay-slot", "other", "unowned")
CLASS_LABEL = {
    "load": "extra-ILOD", "store": "extra-ISTR",
    "stack-load": "spill/reload", "stack-store": "spill/reload",
    "move": "split-not-copy", "alu": "missing-CSE", "const": "missing-CSE",
    "branch": "control-flow", "delay-nop": "delay-slot", "nop": "delay-slot",
    "call": "other", "frame": "other", "other": "other",
}


def word_label(word: dict) -> str:
    if not word["owner"]["owned"]:
        return "unowned"
    if word["owner"]["construct"] == "gen_reg_save_restore":
        return "callee-save"
    return CLASS_LABEL[word["class"]]


def pair_label(pair: dict) -> str:
    """The pair's label. The rule, applied in this order:

      hoist          a closed pair whose candidate-only and target-only words
                     have the same class multiset (nops aside): the same kind
                     of instruction sits at a different place. Decided before
                     ownership, because it does not depend on it.
      unrolled-loop  one side carries two or more one-sided branches of the
                     same register-erased shape: a duplicated loop test.
      otherwise      the majority of the per-word labels (CLASS_LABEL, or
                     `unowned` for a word nothing owns). A tie goes to the
                     candidate side's word -- what our source emits and can
                     delete -- then to LABELS order.
    """
    def classes(side):
        return collections.Counter(w["class"] for w in pair["words"]
                                   if w["side"] == side
                                   and w["class"] not in ("nop", "delay-nop"))
    cand, targ = classes("candidate"), classes("target")
    if pair["closed"] and cand and cand == targ:
        return "hoist"
    for side in ("candidate", "target"):
        branches = collections.Counter(
            w["shape"] for w in pair["words"]
            if w["side"] == side and w["class"] == "branch")
        if any(n > 1 for n in branches.values()):
            return "unrolled-loop"
    votes = collections.Counter(word_label(w) for w in pair["words"])
    first = {}
    for w in pair["words"]:
        first.setdefault(word_label(w), 0 if w["side"] == "candidate" else 1)
    return sorted(votes.items(),
                  key=lambda kv: (-kv[1], first[kv[0]],
                                  LABELS.index(kv[0])))[0][0]


def function_label(pairs: list[dict]) -> str:
    """The label of the pair that costs the most shadow.

    That pair is where the positional count comes from, so it is where a lane
    starts; ties go to the pair with more one-sided words, then LABELS order.
    """
    if not pairs:
        return "unowned"
    best = sorted(pairs, key=lambda p: (-p["shadow"], -len(p["words"]),
                                        LABELS.index(p["label"])))[0]
    return best["label"]


# --------------------------------------------------------------------------
# CLI. Everything below needs the build; nothing above it does.
# --------------------------------------------------------------------------

FORCED_ENV_PREFIXES = ("CDX_", "DKWB_")


def forced_environment() -> list[str]:
    import nm_ranking as nr
    names = [n for n in nr._FORCED_ENV if os.environ.get(n)]
    names += [n for n in os.environ
              if n.startswith(FORCED_ENV_PREFIXES) and os.environ.get(n)
              and n not in names]
    return sorted(names)


@contextlib.contextmanager
def _isolated_workdir(nr):
    previous = nr.WORK_DIR
    scratch = pathlib.Path(tempfile.mkdtemp(prefix="insertion-pairs-"))
    nr.WORK_DIR = scratch
    try:
        yield scratch
    finally:
        nr.WORK_DIR = previous
        shutil.rmtree(scratch, ignore_errors=True)


def line_table(obj: pathlib.Path, start: int, size: int, source: str) -> dict:
    import nm_ranking as nr
    text = subprocess.run(
        [str(nr.OBJDUMP), "-d", "-l", f"--start-address={start}",
         f"--stop-address={start + size}", str(obj)],
        capture_output=True, text=True, check=True).stdout
    return parse_line_table(text, start, size, pathlib.Path(source).name)


def compile_traced(item, work: pathlib.Path
                   ) -> tuple[pathlib.Path | None, str | None, str]:
    """Compile the TU once with the traced ugen: (object, trace text, note)."""
    import force_lattice as fl
    compiler = fl.INSTRUMENTED / "cc"
    if not compiler.exists():
        return None, None, "no instrumented compiler installed"
    command = fl.replace_compiler(fl.compile_command(item.func), compiler)
    out = work / (item.rel_c_file.replace("/", "_") + ".traced.o")
    command[command.index("-o") + 1] = str(out)
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(FORCED_ENV_PREFIXES)}
    env.update(DKWB_UGEN_SCHED="1", DKWB_UGEN_TRACE="1")
    result = subprocess.run(command, env=env, capture_output=True, text=True,
                            cwd=fl.ROOT, timeout=900)
    if result.returncode:
        return None, None, f"traced compile failed (exit {result.returncode})"
    return out, result.stdout + result.stderr, "traced"


def identity_gate(item, traced_obj: pathlib.Path, words: list[int]) -> bool:
    """The traced object's words for this function equal the scored ones."""
    import nm_ranking as nr
    span = nr.func_symbol_span(traced_obj, item.func)
    if span is None:
        return False
    return nr.words_of(nr.text_bytes(traced_obj, *span)) == words


def analyse(item, obj: pathlib.Path, procs: dict | None, trace_note: str,
            traced_obj: pathlib.Path | None = None) -> dict:
    """Pairs, shadow, classes, owners and labels for one symbol in `obj`.

    `procs` is `parse_trace` output (or None). With `traced_obj`, the trace is
    used only if that object's words for this function equal `obj`'s -- the
    identity gate; a supplied trace with no object is used ungated and says so.
    """
    import align_symbol as als
    import frame_census as fc
    import nm_ranking as nr

    streams, error = nr.word_streams(item, obj)
    if streams is None:
        raise SystemExit(f"insertion_pairs: {error}")
    base, target = streams.base_words, streams.target_words
    size_delta = streams.base_size - streams.target_size
    if size_delta == 0:
        raise SystemExit(
            f"insertion_pairs: {item.func} has size delta 0 in this object; "
            "there is no insertion to pair. Use tools/residual_map.py.")
    if procs and traced_obj is not None:
        if identity_gate(item, traced_obj, base):
            trace_note = "identity gate passed"
        else:
            procs, trace_note = None, ("identity gate FAILED: the traced "
                                       "object differs here; trace discarded")

    b_reloc, t_reloc = streams.base_reloc, streams.target_reloc
    masked = als.reloc_masked

    def key(words, reloc, other):
        return [nr.instr_reg_mask(masked(w, i * 4, reloc, other))
                for i, w in enumerate(words)]

    b_key, t_key = key(base, b_reloc, t_reloc), key(target, t_reloc, b_reloc)
    script = als._banded_edit_script(b_key, t_key)
    buckets = {"naming": 0, "immediate": 0, "structural": 0}
    differing: set[int] = set()
    naming: set[int] = set()
    for step, (op, i, j) in enumerate(script):
        if op not in ("equal", "replace"):
            continue
        bw = masked(base[i], i * 4, b_reloc, t_reloc)
        tw = masked(target[j], j * 4, t_reloc, b_reloc)
        if op == "equal" and bw == tw:
            continue
        differing.add(step)
        if nr.instr_reg_mask(bw) == nr.instr_reg_mask(tw):
            buckets["naming"] += 1
            naming.add(step)
        elif als.instr_imm_mask(bw) == als.instr_imm_mask(tw):
            buckets["immediate"] += 1
        else:
            buckets["structural"] += 1

    positional = nr.masked_mismatch_positions(base, target, b_reloc, t_reloc)
    n_min = min(len(base), len(target))
    extra = abs(len(base) - len(target))
    pairs = pairs_from_script(script)
    totals = account(pairs, script, differing, positional, n_min, extra,
                     naming)

    span = nr.func_symbol_span(obj, item.func)
    lines = line_table(obj, span[0], span[1], item.rel_c_file) if span else {}
    procs = procs or {}
    int_lines = {v for v in lines.values() if isinstance(v, int)}
    bounds = (min(int_lines), max(int_lines)) if int_lines else None
    proc_index, proc_note = pick_proc(procs, int_lines) if procs else \
        (None, "no trace")
    proc = procs.get(proc_index) if proc_index is not None else None

    # Neighbour candidate index for each target-only step: the candidate word
    # aligned just before it, else just after.
    last_cand: list[int | None] = []
    seen = None
    for op, i, j in script:
        if op in ("equal", "replace", "delete"):
            seen = i
        last_cand.append(seen)

    for pair in pairs:
        words = []
        for step in pair["steps"]:
            op, i, j = script[step]
            if op == "delete":
                prev = base[i - 1] if i else None
                klass = classify_word(base[i], prev)
                line = lines.get(i * 4)
                words.append({"side": "candidate", "offset": i * 4,
                              "class": klass, "shape": b_key[i],
                              "owner": own(klass, line, proc, bounds), "via": "own"})
            elif op == "insert":
                prev = target[j - 1] if j else None
                klass = classify_word(target[j], prev)
                near = last_cand[step]
                if near is None:
                    near = i if i < len(base) else None
                line = lines.get(near * 4) if near is not None else None
                owner = own(klass, line, proc, bounds)
                if not owner["owned"] and isinstance(line, int) and proc \
                        and proc["emits"].get(line):
                    # Our side emits nothing for this word and nothing of its
                    # family nearby; the line still names what our code does
                    # where the target has it. Kept, and marked as such.
                    rows = proc["emits"][line]
                    construct, _ = _construct(rows)
                    owner = {"owned": True, "line": line,
                             "construct": construct, "basis": "neighbour",
                             "construct_line": line,
                             "reason": "neighbour line; no same-family "
                                       "emission near it",
                             "draws_on_line": proc["draws"].get(line, 0)}
                words.append({"side": "target", "offset": j * 4,
                              "class": klass, "shape": t_key[j],
                              "owner": owner, "via": "neighbour"})
        pair["words"] = words
        pair["label"] = pair_label(pair)

    # No stack adjustment on a side means no frame: size 0, not unknown.
    frames = (fc.census(base)["frame_size"] or 0,
              fc.census(target)["frame_size"] or 0)
    frame_delta = frames[0] - frames[1]
    owning_lines = sorted({str(w["owner"]["line"]) for p in pairs
                           for w in p["words"] if w["owner"]["owned"]})
    owned = all(w["owner"]["owned"] for p in pairs for w in p["words"])
    return {
        "symbol": item.func,
        "file": item.rel_c_file,
        "size_bytes": streams.target_size,
        "size_delta": size_delta,
        "candidate_frame": frames[0],
        "target_frame": frames[1],
        "frame_delta": frame_delta,
        "buckets": buckets,
        "trace": trace_note,
        "proc": proc_index,
        "proc_note": proc_note,
        "line_table_words": len(lines),
        **totals,
        "pairs": [{
            "positional_lo": p["lo"] * 4,
            "positional_hi": p["hi"] * 4 if p["hi"] is not None else None,
            "closed": p["closed"],
            "span_words": p["span_words"],
            "positional_in": p["positional_in"],
            "aligned_in": p["aligned_in"],
            "naming_in": p["naming_in"],
            "shadow": p["shadow"],
            "label": p["label"],
            "words": [{k: v for k, v in w.items() if k != "shape"}
                      for w in p["words"]],
        } for p in pairs],
        "one_sided_words": sum(len(p["words"]) for p in pairs),
        "owned": owned,
        "owning_lines": len(owning_lines),
        "label": function_label(pairs),
        "classes": dict(collections.Counter(w["class"] for p in pairs
                                            for w in p["words"])),
        "basis": dict(collections.Counter(w["owner"].get("basis") or "unowned"
                                          for p in pairs for w in p["words"])),
    }


def measure(symbol: str, object_path: pathlib.Path | None = None,
            trace_path: pathlib.Path | None = None,
            use_trace: bool = True) -> dict:
    import nm_ranking as nr
    import permute_batch as pb
    queue = {entry.func: entry for entry in pb.discover_queue()}
    if symbol not in queue:
        raise SystemExit(f"insertion_pairs: {symbol} is not in the NON_MATCHING "
                         "queue (already matched, or never queued)")
    item = queue[symbol]
    with _isolated_workdir(nr) as work:
        if object_path is None:
            commands = nr.configured_compile_commands([item])
            obj, error = nr.compile_configured_tu(item.rel_c_file,
                                                  commands[item.rel_c_file])
            if obj is None:
                raise SystemExit(f"insertion_pairs: {error}")
        else:
            obj = pathlib.Path(object_path)
            if not obj.is_file():
                raise SystemExit(f"insertion_pairs: no such object {obj}")
        refuse_delta_zero(item, obj)
        procs, traced_obj, note = None, None, "not requested"
        if trace_path is not None:
            procs = parse_trace(pathlib.Path(trace_path).read_text(
                errors="replace"))
            note = "supplied trace (not identity-gated)"
        elif use_trace:
            traced_obj, text, note = compile_traced(item, work)
            procs = parse_trace(text) if text else None
        return analyse(item, obj, procs, note, traced_obj)


def refuse_delta_zero(item, obj: pathlib.Path) -> None:
    import nm_ranking as nr
    streams, error = nr.word_streams(item, obj)
    if streams is None:
        raise SystemExit(f"insertion_pairs: {error}")
    if streams.base_size == streams.target_size:
        raise SystemExit(
            f"insertion_pairs: {item.func} has size delta 0 in this object; "
            "there is no insertion to pair. Use tools/residual_map.py.")


def render(data: dict) -> str:
    fd = data["frame_delta"]
    b = data["buckets"]
    out = [f"{data['symbol']}  ({data['file']})",
           f"  {data['size_bytes']} bytes, size delta {data['size_delta']:+d}, "
           f"frame delta {fd:+d}",
           f"  aligned rows: naming {b['naming']}, immediate {b['immediate']}, "
           f"structural {b['structural']}, one-sided {data['one_sided_words']}",
           f"  positional masked {data['positional']}, shadow {data['shadow']}, "
           f"aligned residual after shadow {data['aligned_after_shadow']} "
           f"({data['naming_in_pairs']} of it naming rows inside pairs)",
           f"  outside pairs: aligned {data['aligned_outside']} vs positional "
           f"{data['positional_outside']}"
           + ("" if data["outside_agrees"] else "   <-- DISAGREE"),
           f"  trace: {data['trace']}; proc {data['proc']} ({data['proc_note']})",
           ""]
    for n, pair in enumerate(data["pairs"], 1):
        hi = pair["positional_hi"]
        span = (f"+0x{pair['positional_lo']:X}..+0x{hi:X}" if hi is not None
                else f"+0x{pair['positional_lo']:X}..end")
        out.append(f"  pair {n}: {span} ({pair['span_words']} words, "
                   f"{'closed' if pair['closed'] else 'open to end'})  "
                   f"label {pair['label']}")
        out.append(f"    positional {pair['positional_in']}, aligned "
                   f"{pair['aligned_in']} ({pair['naming_in']} naming), "
                   f"shadow {pair['shadow']}")
        for w in pair["words"]:
            o = w["owner"]
            where = f"line {o['line']}" if o["line"] is not None else "no line"
            what = o["construct"] if o["owned"] else "unowned"
            extra = (f", {o['draws_on_line']} draws on line"
                     if o.get("draws_on_line") is not None else "")
            basis = o.get("basis") or "unowned"
            out.append(f"    {w['side']:<9} +0x{w['offset']:X}  {w['class']:<11}"
                       f" {where} [{basis}]: {what} -- {o['reason']}{extra}")
    out.append("")
    out.append("  ownership basis: " + ", ".join(
        f"{k} {v}" for k, v in sorted(data["basis"].items())))
    out.append(f"  function label {data['label']}; "
               f"{'owned' if data['owned'] else 'NOT fully owned'}; "
               f"{data['owning_lines']} owning line(s)")
    return "\n".join(out)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Pair a size-mismatch function's one-sided words, subtract "
                    "their positional shadow, and name what owns each word.")
    parser.add_argument("symbol")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--object", type=pathlib.Path, default=None,
                        help="read this object instead of recompiling the TU; "
                             "required under a force or instrumented env")
    parser.add_argument("--trace", type=pathlib.Path, default=None,
                        help="use this ugen trace (DKWB_UGEN_TRACE output) "
                             "instead of compiling one")
    parser.add_argument("--no-trace", action="store_true",
                        help="skip the traced compile; every non-nop word "
                             "is then unowned")
    args = parser.parse_args(argv)
    forced = forced_environment()
    if forced and args.object is None:
        print("error: " + ", ".join(forced) + " is set; this tool recompiles "
              "the configured TU, which drops any force, so the reading would "
              "describe the unforced build. Pass --object <forced.o> (and "
              "--trace <its log>) to read that object directly.",
              file=sys.stderr)
        return 2
    data = measure(args.symbol, args.object, args.trace, not args.no_trace)
    print(json.dumps(data, indent=2) if args.json else render(data))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
