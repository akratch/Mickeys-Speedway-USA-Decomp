#!/usr/bin/env python3
"""Re-derive the verified-assembly (HASM) evidence from the baserom and build.

`verified_asm.us.txt` credits whole splat subsegments as original hand-written
assembly, and the per-routine evidence for each one is written beside its
boundary in `mickey.us.yaml`.  This tool reads neither note as evidence.  It
takes only the ledger's *list* of subsegments and the class each is filed
under, and re-derives -- from the baserom bytes, the built ELF, the linker
map, the source tree and the pinned reference farm -- whether every routine
the scoreboard counts as verified assembly carries proof of its own.

1. Coverage.  The routine set is enumerated with tools/progress.py's own
   functions (ledger subsegments x ELF function symbols), so the audit covers
   exactly what the scoreboard counts.  The count is asserted; every routine
   must be linked from an assembled `.s` object, and nothing under src/ may
   define it in C or name it in a GLOBAL_ASM pragma.

2. Reference identity (classes 1 and 2).  Each subsegment's routine run is
   covered greedily by the longest matches the farm pinned by
   tools/reference-builds.lock holds (each title's digest is checked), and a
   routine inherits the match that contains it; a routine no long match
   contains is searched for alone.  Words are compared with only the
   reference object's own relocated fields masked (R_MIPS_26, HI16/LO16, and
   the PC16/GPREL16/LITERAL halves where present).  The run's claim is
   "whole-object", "single-object-run" or "multi-segment".  A hit records the
   object path relative to the farm root, words compared and masked, and the
   object's kind: assembled from a hand-written source; carried as `hasm` by
   the reference project; GLOBAL_ASM inside a C file; extracted assembly; or
   compiled C, which refutes the claim.  A routine whose only farm match is
   GLOBAL_ASM or extracted assembly is re-tested against the reference
   project's own hand-written `.s` source, assembled with this repository's
   GNU as into a temporary directory, when such a source defines it.

3. Mechanisms (class 3, corroboration for 1 and 2).  Per routine, from the
   ROM bytes alone: odd single-precision FP operands; trapping signed
   add/sub; registers read on entry that o32 passes no argument in; callee-
   saved registers written with no save; register jumps to computed or
   constant code addresses; frame shapes; COP0/TLB/cache/eret/system
   instructions; reserved registers and other convention breaks; control
   transfers into another routine; secondary entry points; falling off the
   end.  Every category is also run over every C-matched resident function
   -- code IDO demonstrably produced -- as a negative control, and a category
   that fires there is demoted to information and carries no verdict.

4. Classification.  Per routine, which classes the re-derived evidence
   supports, checked against the ledger.  A routine with neither a mechanism
   nor reference identity is accepted on two structural grounds only: as a
   permitted library routine's built object with instructions deleted (an
   order-preserving alignment with deletions and store-register repointing
   only), or as another routine's private helper -- every reference to it
   anywhere in the resident image or the overlay export table lies inside
   one parent routine of the same subsegment, which carries evidence itself.

Findings carry offsets and short category labels only: no instruction text,
no machine words, no reference bytes.  The output is safe to paste, and the
tool never writes into the tree.

Usage:
    tools/hasm_audit.py              # human report
    tools/hasm_audit.py --json       # per-routine records (schema hasm-audit/1)
    tools/hasm_audit.py --check      # exit 1 unless every routine is supported
    tools/hasm_audit.py --check --strict   # also fail on a ledger-class mismatch
"""

from __future__ import annotations

import argparse
import array
import bisect
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict
from dataclasses import dataclass, field

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

EXPECTED_ROUTINES = 107
SCHEMA = "hasm-audit/1"
MASK32 = 0xFFFFFFFF
DEFAULT_REFS = os.environ.get(
    "REFS_ROOT", os.path.join(os.path.expanduser("~"), "Desktop", "dev", "decomp-refs"))

# --------------------------------------------------------------------------
# Register numbering.  GPRs are 0-31 and FPRs 32-63 in one bitmask, so a
# single integer carries a whole register set through the dataflow.
# --------------------------------------------------------------------------

GPR_NAMES = (
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
)
R_AT, R_V0, R_V1 = 1, 2, 3
R_K0, R_K1, R_GP, R_SP, R_FP, R_RA = 26, 27, 28, 29, 30, 31
FPR = 32


def reg_name(r):
    return GPR_NAMES[r] if r < FPR else "f%d" % (r - FPR)


def bit(r):
    return 1 << r


def mask_of(regs):
    out = 0
    for r in regs:
        out |= 1 << r
    return out


def regs_in(mask):
    return [r for r in range(64) if mask >> r & 1]


# o32 passes arguments in a0-a3 and f12/f14 (with their odd halves); any
# other caller-saved register holds nothing the callee may read on entry.
NONARG_CALLER_SAVED = mask_of([R_AT, R_V0, R_V1, *range(8, 16), 24, 25]
                              + [FPR + f for f in (*range(0, 12), *range(16, 20))])
CALLEE_SAVED = mask_of([*range(16, 24), R_FP] + [FPR + f for f in range(20, 32)])
CALL_DEFINES = mask_of([R_AT, R_V0, R_V1, *range(4, 16), 24, 25, R_RA]
                       + [FPR + f for f in range(0, 20)])

# --------------------------------------------------------------------------
# Instruction decoding (R4300, big-endian).  Numeric fields only: the tool
# never needs, and never prints, an instruction's text.
# --------------------------------------------------------------------------

OP_SPECIAL, OP_REGIMM, OP_J, OP_JAL = 0, 1, 2, 3
OP_BEQ, OP_BNE, OP_BLEZ, OP_BGTZ = 4, 5, 6, 7
OP_ADDI, OP_ADDIU, OP_SLTI, OP_SLTIU = 8, 9, 10, 11
OP_ANDI, OP_ORI, OP_XORI, OP_LUI = 12, 13, 14, 15
OP_COP0, OP_COP1, OP_COP2 = 16, 17, 18
OP_BEQL, OP_BNEL, OP_BLEZL, OP_BGTZL = 20, 21, 22, 23
OP_DADDI, OP_DADDIU, OP_LDL, OP_LDR = 24, 25, 26, 27
OP_LB, OP_LH, OP_LWL, OP_LW, OP_LBU, OP_LHU, OP_LWR, OP_LWU = 32, 33, 34, 35, 36, 37, 38, 39
OP_SB, OP_SH, OP_SWL, OP_SW, OP_SDL, OP_SDR, OP_SWR, OP_CACHE = 40, 41, 42, 43, 44, 45, 46, 47
OP_LL, OP_LWC1, OP_LWC2, OP_LLD, OP_LDC1, OP_LDC2, OP_LD = 48, 49, 50, 52, 53, 54, 55
OP_SC, OP_SWC1, OP_SWC2, OP_SCD, OP_SDC1, OP_SDC2, OP_SD = 56, 57, 58, 60, 61, 62, 63
GPR_LOADS = (OP_LB, OP_LH, OP_LWL, OP_LW, OP_LBU, OP_LHU, OP_LWR, OP_LWU, OP_LD, OP_LDL, OP_LDR)
GPR_STORES = (OP_SB, OP_SH, OP_SWL, OP_SW, OP_SDL, OP_SDR, OP_SWR, OP_SD)

FN_SLL, FN_SRL, FN_SRA = 0, 2, 3
FN_JR, FN_JALR, FN_SYSCALL, FN_BREAK, FN_SYNC = 8, 9, 12, 13, 15
FN_MFHI, FN_MTHI, FN_MFLO, FN_MTLO = 16, 17, 18, 19
FN_ADD, FN_ADDU, FN_SUB, FN_SUBU, FN_OR = 32, 33, 34, 35, 37
FN_DADD, FN_DSUB = 44, 46

FMT_S, FMT_D, FMT_W, FMT_L = 16, 17, 20, 21
COP1_MF, COP1_DMF, COP1_CF, COP1_MT, COP1_DMT, COP1_CT, COP1_BC = 0, 1, 2, 4, 5, 6, 8

CTL_BRANCH, CTL_LIKELY, CTL_JUMP, CTL_CALL, CTL_JR, CTL_ERET = (
    "branch", "likely", "jump", "call", "jr", "eret")


def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


@dataclass
class Insn:
    addr: int
    word: int
    op: int
    rs: int
    rt: int
    rd: int
    sa: int
    funct: int
    imm: int
    uses: int = 0
    defs: int = 0
    ctl: str | None = None
    target: int | None = None
    uncond: bool = False
    trapping: bool = False
    odd_single: int = 0          # odd FPR operands used at single/word width
    odd_double: int = 0          # odd FPR named at double width (illegal at FR=0)
    system: bool = False
    trap: bool = False
    wide: bool = False           # a MIPS III 64-bit operation
    store: tuple | None = None   # (stored-register mask, base, offset)
    load: tuple | None = None    # (base, offset)


def _fp_operand(ins, reg, width, *, write):
    """Record one FPR operand of a COP1 arithmetic operation."""
    r = FPR + reg
    regs = bit(r) | (bit(r + 1) if width == 64 and reg < 31 else 0)
    if write:
        ins.defs |= regs
    else:
        ins.uses |= regs
    if reg & 1:
        if width == 64:
            ins.odd_double += 1
        else:
            ins.odd_single += 1


def _decode_cop1(ins, branch_target):
    fmt, ft, fs, fd, funct = ins.rs, ins.rt, ins.rd, ins.sa, ins.funct
    if fmt in (COP1_MF, COP1_DMF):
        ins.defs, ins.uses, ins.wide = bit(ft), bit(FPR + fs), fmt == COP1_DMF
    elif fmt in (COP1_MT, COP1_DMT):
        ins.uses, ins.defs, ins.wide = bit(ft), bit(FPR + fs), fmt == COP1_DMT
    elif fmt == COP1_CF:
        ins.defs = bit(ft)
    elif fmt == COP1_CT:
        ins.uses = bit(ft)
    elif fmt == COP1_BC:
        ins.target = branch_target
        ins.ctl = CTL_LIKELY if ft & 2 else CTL_BRANCH
    elif fmt in (FMT_S, FMT_D, FMT_W, FMT_L):
        src = 32 if fmt in (FMT_S, FMT_W) else 64
        if funct >= 48:                     # compares define only the flag
            _fp_operand(ins, fs, src, write=False)
            _fp_operand(ins, ft, src, write=False)
            return
        _fp_operand(ins, fs, src, write=False)
        if funct <= 3:                      # the four binary operations
            _fp_operand(ins, ft, src, write=False)
            dst = src
        elif funct <= 7:                    # sqrt/abs/mov/neg keep the format
            dst = src
        elif funct <= 11 or funct in (33, 37):
            dst = 64                        # to long or to double
        else:
            dst = 32                        # to word or to single
        _fp_operand(ins, fd, dst, write=True)


def decode(word, addr):
    op = word >> 26
    rs, rt, rd = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
    sa, funct, imm = (word >> 6) & 31, word & 63, sext16(word & 0xFFFF)
    ins = Insn(addr, word, op, rs, rt, rd, sa, funct, imm)
    branch_target = (addr + 4 + (imm << 2)) & MASK32

    if op == OP_SPECIAL:
        if funct in (FN_SLL, FN_SRL, FN_SRA, 56, 58, 59, 60, 62, 63):
            ins.uses, ins.defs, ins.wide = bit(rt), bit(rd), funct >= 56
        elif funct in (4, 6, 7, 20, 22, 23):
            ins.uses, ins.defs, ins.wide = bit(rs) | bit(rt), bit(rd), funct >= 20
        elif funct == FN_JR:
            ins.uses, ins.ctl = bit(rs), CTL_JR
        elif funct == FN_JALR:
            ins.uses, ins.defs, ins.ctl = bit(rs), bit(rd), CTL_CALL
        elif funct in (FN_SYSCALL, FN_SYNC):
            ins.system = True
        elif funct in (FN_MFHI, FN_MFLO):
            ins.defs = bit(rd)
        elif funct in (FN_MTHI, FN_MTLO):
            ins.uses = bit(rs)
        elif 24 <= funct <= 31:
            ins.uses, ins.wide = bit(rs) | bit(rt), funct >= 28
        elif 32 <= funct <= 47:
            ins.uses, ins.defs = bit(rs) | bit(rt), bit(rd)
            ins.trapping = funct in (FN_ADD, FN_SUB, FN_DADD, FN_DSUB)
            ins.wide = funct >= 44
        elif 48 <= funct <= 54:
            ins.uses, ins.trap = bit(rs) | bit(rt), True
    elif op == OP_REGIMM:
        ins.uses = bit(rs)
        if rt in (0, 1, 2, 3, 16, 17, 18, 19):
            ins.target = branch_target
            if rt >= 16:
                ins.ctl, ins.defs = CTL_CALL, bit(R_RA)
            else:
                ins.ctl = CTL_LIKELY if rt in (2, 3) else CTL_BRANCH
                ins.uncond = rs == 0 and rt in (1, 3)
        elif 8 <= rt <= 14:
            ins.trap = True
    elif op in (OP_J, OP_JAL):
        ins.target = ((addr + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
        ins.ctl = CTL_JUMP if op == OP_J else CTL_CALL
        if op == OP_JAL:
            ins.defs = bit(R_RA)
    elif op in (OP_BEQ, OP_BNE, OP_BEQL, OP_BNEL):
        ins.uses, ins.target = bit(rs) | bit(rt), branch_target
        ins.ctl = CTL_LIKELY if op >= OP_BEQL else CTL_BRANCH
        ins.uncond = op in (OP_BEQ, OP_BEQL) and rs == rt
    elif op in (OP_BLEZ, OP_BGTZ, OP_BLEZL, OP_BGTZL):
        ins.uses, ins.target = bit(rs), branch_target
        ins.ctl = CTL_LIKELY if op >= OP_BEQL else CTL_BRANCH
        ins.uncond = op in (OP_BLEZ, OP_BLEZL) and rs == 0
    elif OP_ADDI <= op <= OP_XORI or op in (OP_DADDI, OP_DADDIU):
        ins.uses, ins.defs = bit(rs), bit(rt)
        ins.trapping = op in (OP_ADDI, OP_DADDI)
        ins.wide = op in (OP_DADDI, OP_DADDIU)
    elif op == OP_LUI:
        ins.defs = bit(rt)
    elif op == OP_COP0:
        ins.system = True
        if rs in (0, 1):
            ins.defs = bit(rt)
        elif rs in (4, 5):
            ins.uses = bit(rt)
        elif rs == 16 and funct == 24:
            ins.ctl = CTL_ERET
        elif rs == 8:
            ins.ctl, ins.target = CTL_BRANCH, branch_target
    elif op == OP_COP1:
        _decode_cop1(ins, branch_target)
    elif op in (OP_COP2, OP_LWC2, OP_SWC2, OP_LDC2, OP_SDC2):
        ins.system = True
    elif op in GPR_LOADS:
        # lwl/lwr architecturally merge into rt, but a pair overwrites it
        # whole, so the merge input is deliberately not counted as a read.
        ins.uses, ins.defs, ins.load = bit(rs), bit(rt), (rs, imm)
        ins.wide = op in (OP_LWU, OP_LD, OP_LDL, OP_LDR)
    elif op in GPR_STORES:
        ins.uses, ins.store = bit(rs) | bit(rt), (bit(rt), rs, imm)
        ins.wide = op in (OP_SD, OP_SDL, OP_SDR)
    elif op == OP_CACHE:
        ins.uses, ins.system = bit(rs), True
    elif op in (OP_LL, OP_LLD):
        ins.uses, ins.defs, ins.system = bit(rs), bit(rt), True
    elif op in (OP_SC, OP_SCD):
        ins.uses, ins.defs, ins.system = bit(rs) | bit(rt), bit(rt), True
    elif op in (OP_LWC1, OP_LDC1):
        ins.uses, ins.load = bit(rs), (rs, imm)
        ins.defs = bit(FPR + rt)
        if op == OP_LDC1:
            ins.defs |= bit(FPR + rt + 1) if rt < 31 else 0
            ins.odd_double = rt & 1
    elif op in (OP_SWC1, OP_SDC1):
        regs = bit(FPR + rt)
        if op == OP_SDC1:
            regs |= bit(FPR + rt + 1) if rt < 31 else 0
            ins.odd_double = rt & 1
        ins.uses, ins.store = bit(rs) | regs, (regs, rs, imm)
    ins.defs &= ~1  # writes to $zero are discarded
    return ins


def decode_words(words, base):
    return [decode(w, base + 4 * i) for i, w in enumerate(words)]


# --------------------------------------------------------------------------
# Control flow.  Delay slots are separate nodes: a branch's delay slot runs
# before either successor, a likely branch's only on the taken edge, and a
# call's effect lands after its delay slot.
# --------------------------------------------------------------------------

EXIT_RETURN, EXIT_TAIL, EXIT_REGJUMP, EXIT_ERET, EXIT_FALL, EXIT_OUT = (
    "return", "tail", "register-jump", "eret", "fall", "out")
ANY_TARGET = "any"


@dataclass
class Cfg:
    succ: list
    delay_of: dict          # delay-slot index -> controlling index
    call_return: set        # delay slots whose successor sees a call's effect
    reachable: set          # from the entry, extra entries and jump targets
    from_entry: set         # from the entry alone
    exits: list             # (index, exit kind) reachable from the entry


def _resolve(insns, i, reg, depth=0):
    """Symbolic value of `reg` just before insns[i], walking back linearly.

    Returns (kind, value): 'const' (value known), 'loaded' (the word was read
    from memory; value is the load address when that is constant), 'sum' (a
    known constant plus something unknown; value is the constant), or
    'unknown'.  Linear and depth-bounded: a heuristic for classifying a
    register jump's target, never a proof of a value.
    """
    if reg == 0:
        return ("const", 0)
    if depth > 12:
        return ("unknown", None)
    for j in range(i - 1, -1, -1):
        d = insns[j]
        if not d.defs >> reg & 1:
            continue
        if d.load is not None:
            kind, val = _resolve(insns, j, d.load[0], depth + 1)
            addr = (val + d.load[1]) & MASK32 if kind in ("const", "sum") else None
            return ("loaded", addr)
        if d.op == OP_LUI:
            return ("const", (d.word & 0xFFFF) << 16)
        if d.op in (OP_ADDIU, OP_ADDI, OP_ORI):
            kind, val = _resolve(insns, j, d.rs, depth + 1)
            low = d.word & 0xFFFF if d.op == OP_ORI else d.imm
            if kind == "const":
                return ("const", ((val | low) if d.op == OP_ORI else (val + low)) & MASK32)
            if kind == "sum":
                return ("sum", (val + low) & MASK32)
            return ("unknown", None)
        if d.op == OP_SPECIAL and d.funct in (FN_ADDU, FN_ADD, FN_SUBU, FN_SUB, FN_OR):
            a = _resolve(insns, j, d.rs, depth + 1)
            b = _resolve(insns, j, d.rt, depth + 1)
            if d.funct == FN_OR and b == ("const", 0):
                return a
            if d.funct == FN_OR and a == ("const", 0):
                return b
            if a[0] == "const" and b[0] == "const":
                if d.funct in (FN_ADDU, FN_ADD):
                    return ("const", (a[1] + b[1]) & MASK32)
                if d.funct in (FN_SUBU, FN_SUB):
                    return ("const", (a[1] - b[1]) & MASK32)
                return ("unknown", None)
            if d.funct != FN_OR and a[0] in ("const", "sum"):
                return ("sum", a[1])
            if d.funct in (FN_ADDU, FN_ADD) and b[0] in ("const", "sum"):
                return ("sum", b[1])
            return ("unknown", None)
        return ("unknown", None)
    return ("unknown", None)


def classify_register_jumps(insns, read_word=None):
    """{index: (label, targets)} for every register jump that is not a return.

    targets is a list of instruction indices (a resolved jump table), the
    marker ANY_TARGET (a jump computed into the routine's own code), or None.
    """
    n = len(insns)
    start = insns[0].addr if insns else 0
    end = start + 4 * n
    out = {}
    for i, ins in enumerate(insns):
        if ins.ctl != CTL_JR or ins.rs == R_RA:
            continue
        kind, val = _resolve(insns, i, ins.rs)
        targets = None
        if kind == "loaded":
            label = "table-or-pointer-jump"
            if val is not None and read_word is not None:
                found = []
                for k in range(512):
                    w = read_word(val + 4 * k)
                    if w is None or not (start <= w < end) or w % 4:
                        break
                    found.append((w - start) // 4)
                targets = found or None
        elif kind == "sum" and val is not None and start <= val <= end:
            label, targets = "computed-jump-own-code", ANY_TARGET
        elif kind == "const":
            label = "register-jump-to-constant"
        elif kind == "sum":
            label = "computed-jump"
        else:
            label = "register-jump-unresolved"
        out[i] = (label, targets)
    return out


def build_cfg(insns, jr_targets=None, entries=()):
    n = len(insns)
    start = insns[0].addr if insns else 0
    end = start + 4 * n
    jr_targets = jr_targets or {}

    def node(addr):
        if addr is not None and start <= addr < end and not (addr - start) % 4:
            return (addr - start) // 4
        return None

    delay_of = {}
    for i, ins in enumerate(insns):
        if i in delay_of:
            continue                      # a control word in a delay slot is inert
        if ins.ctl and ins.ctl != CTL_ERET and i + 1 < n:
            delay_of[i + 1] = i
    succ = [[] for _ in range(n)]
    call_return = set()

    def after(i):
        return i if i < n else EXIT_FALL

    for i, ins in enumerate(insns):
        if i in delay_of:
            ci = delay_of[i]
            c = insns[ci]
            tgt = node(c.target)
            if c.ctl == CTL_CALL:
                succ[i] = [after(ci + 2)]
                call_return.add(i)
            elif c.ctl == CTL_JR:
                if c.rs == R_RA:
                    succ[i] = [EXIT_RETURN]
                else:
                    t = jr_targets.get(ci)
                    if t == ANY_TARGET:
                        succ[i] = list(range(n)) + [EXIT_REGJUMP]
                    elif t:
                        succ[i] = sorted(set(t))
                    else:
                        succ[i] = [EXIT_REGJUMP]
            elif c.ctl == CTL_JUMP:
                succ[i] = [tgt if tgt is not None else EXIT_TAIL]
            elif c.ctl == CTL_LIKELY:
                succ[i] = [tgt if tgt is not None else EXIT_OUT]
            else:
                taken = tgt if tgt is not None else EXIT_OUT
                succ[i] = [taken] if c.uncond else [taken, after(ci + 2)]
            continue
        if ins.ctl == CTL_ERET:
            succ[i] = [EXIT_ERET]
        elif ins.ctl and i + 1 >= n:
            succ[i] = [EXIT_FALL]
        elif ins.ctl == CTL_LIKELY and not ins.uncond:
            succ[i] = [i + 1, after(i + 2)]
        else:
            succ[i] = [after(i + 1)]

    def reach(roots):
        seen, stack = set(), [r for r in roots if 0 <= r < n]
        while stack:
            i = stack.pop()
            if i in seen:
                continue
            seen.add(i)
            stack.extend(s for s in succ[i] if isinstance(s, int))
        return seen

    from_entry = reach([0] if n else [])
    reachable = reach([0, *entries] if n else [])
    exits = [(i, s) for i in sorted(from_entry) for s in succ[i] if isinstance(s, str)]
    return Cfg(succ, delay_of, call_return, reachable, from_entry, exits)


def forward(insns, cfg, init, transfer):
    """Forward may-dataflow (union join) from the entry; returns IN[]."""
    state = [None] * len(insns)
    if not insns:
        return state
    state[0] = init
    work = [0]
    while work:
        i = work.pop()
        out = transfer(i, state[i])
        for s in cfg.succ[i]:
            if not isinstance(s, int):
                continue
            new = out if state[s] is None else state[s] | out
            if new != state[s]:
                state[s] = new
                work.append(s)
    return state


# --------------------------------------------------------------------------
# Mechanism detectors.  Each yields (offset, label, detail): offset relative
# to the routine, detail a count or register names, never instruction text.
# --------------------------------------------------------------------------

def detect_odd_fp(insns, cfg):
    out = []
    for i, ins in enumerate(insns):
        if i not in cfg.reachable:
            continue
        if ins.odd_single:
            out.append((4 * i, "odd-fp-single", ins.odd_single))
        if ins.odd_double:
            out.append((4 * i, "odd-fp-double", ins.odd_double))
    return out


def detect_trapping(insns, cfg):
    return [(4 * i, "trapping-add-sub", None) for i, ins in enumerate(insns)
            if ins.trapping and i in cfg.reachable]


def detect_system(insns, cfg):
    out = []
    for i in sorted(cfg.reachable):
        ins = insns[i]
        if ins.system:
            out.append((4 * i, "system-instruction", None))
        if ins.trap:
            out.append((4 * i, "trap-instruction", None))
        if ins.wide:
            out.append((4 * i, "64-bit-instruction", None))
    return out


def detect_entry_reads(insns, cfg):
    """Non-argument registers read where no definition can reach.

    The value read is then the caller's on every path.  "No definition can
    reach" is a may-analysis of definitions, so a loop's back edge or an
    infeasible path can only suppress a finding, never invent one.
    """
    def transfer(i, defined):
        out = defined | insns[i].defs
        if i in cfg.call_return:
            out |= CALL_DEFINES
        return out

    state = forward(insns, cfg, 0, transfer)
    out = []
    for i, ins in enumerate(insns):
        if state[i] is None:
            continue
        hit = ins.uses & ~state[i] & NONARG_CALLER_SAVED
        if hit:
            out.append((4 * i, "nonarg-register-read-on-entry",
                        ",".join(reg_name(r) for r in regs_in(hit))))
    return out


def detect_unsaved_writes(insns, cfg):
    """Callee-saved registers written where no save of them can reach."""
    def saves(ins):
        if ins.store and ins.store[1] == R_SP:
            return ins.store[0] & CALLEE_SAVED
        return 0

    state = forward(insns, cfg, 0, lambda i, saved: saved | saves(insns[i]))
    out, seen = [], 0
    for i, ins in enumerate(insns):
        if state[i] is None:
            continue
        hit = ins.defs & CALLEE_SAVED & ~state[i] & ~seen
        if hit:
            seen |= hit
            out.append((4 * i, "callee-saved-written-unsaved",
                        ",".join(reg_name(r) for r in regs_in(hit))))
    return out


def detect_frame(insns, cfg):
    """Frame shapes, over every reachable instruction."""
    out = []
    reach = sorted(cfg.reachable)
    calls = [i for i in reach if insns[i].ctl == CTL_CALL]
    allocs = [i for i in reach if insns[i].op == OP_ADDIU and insns[i].rs == R_SP
              and insns[i].rt == R_SP and insns[i].imm < 0]
    ra_saves = [i for i in reach if insns[i].store and insns[i].store[1] == R_SP
                and insns[i].store[0] >> R_RA & 1]
    if allocs:
        first, size = allocs[0], -insns[allocs[0]].imm
        if first != 0:
            out.append((4 * first, "frame-not-first", None))
        if size % 8:
            out.append((4 * first, "frame-misaligned", None))
        if calls and size < 0x18:
            out.append((4 * first, "nonleaf-frame-below-o32-minimum", None))
        if len(allocs) > 1:
            out.append((4 * allocs[1], "second-frame-allocation", None))
    elif calls:
        out.append((4 * calls[0], "nonleaf-without-frame", None))
    # IDO saves ra in a leaf only when it allocates ra as a scratch register
    # (the negative control has both cases), so only a save with no other
    # write of ra is a shape IDO cannot produce.
    ra_scratch = any(insns[i].defs >> R_RA & 1 and insns[i].ctl != CTL_CALL
                     and not (insns[i].load and insns[i].load[0] == R_SP) for i in reach)
    if not calls and ra_saves:
        out.append((4 * ra_saves[0], "leaf-saves-ra" if ra_scratch
                    else "leaf-saves-unused-ra", None))
    if calls and not ra_saves:
        out.append((4 * calls[0], "nonleaf-ra-unsaved", None))
    # Callee-saved FP saved a word at a time.  IDO -mips1 does that too, but
    # big-endian, an even/odd pair lands odd half first at an 8-aligned slot
    # with the even half 4 above it; any other placement, or an odd half
    # saved alone, is not an IDO frame.
    fp_slots = {}
    for i in reach:
        ins = insns[i]
        if (ins.store and ins.store[1] == R_SP and ins.op == OP_SWC1
                and ins.store[0] & CALLEE_SAVED):
            out.append((4 * i, "callee-saved-fp-saved-by-word", None))
            fp_slots.setdefault(ins.rt, (i, ins.imm))
    for reg, (i, slot) in sorted(fp_slots.items()):
        if not reg & 1:
            continue
        even = fp_slots.get(reg - 1)
        if even is None or slot % 8 or even[1] != slot + 4:
            out.append((4 * i, "callee-saved-fp-pair-misplaced", None))
    return out


def detect_conventions(insns, cfg):
    """Register-convention and delay-slot shapes."""
    out = []
    for i in sorted(cfg.reachable):
        ins = insns[i]
        if (ins.uses | ins.defs) & mask_of([R_K0, R_K1, R_GP]):
            out.append((4 * i, "reserved-register", None))
        if ins.defs >> R_SP & 1 and not (ins.op == OP_ADDIU and ins.rs == R_SP):
            out.append((4 * i, "sp-nonstandard-write", None))
        if (ins.defs >> R_RA & 1 and ins.ctl != CTL_CALL
                and not (ins.load and ins.load[0] == R_SP)):
            out.append((4 * i, "ra-nonstandard-write", None))
        if i in cfg.delay_of and ins.ctl:
            out.append((4 * i, "control-in-delay-slot", None))
    return out


def detect_foreign_control(insns, cfg, function_starts):
    """Transfers that leave the routine, and paths that run off its end."""
    start = insns[0].addr if insns else 0
    end = start + 4 * len(insns)
    out = []
    for i in sorted(cfg.reachable):
        ins = insns[i]
        if ins.target is None or i in cfg.delay_of:
            continue
        inside = start <= ins.target < end
        if ins.ctl in (CTL_BRANCH, CTL_LIKELY) and not inside:
            out.append((4 * i, "branch-out-of-routine", None))
        elif ins.ctl == CTL_JUMP and not inside:
            out.append((4 * i, "tail-jump" if ins.target in function_starts
                        else "jump-into-foreign-interior", None))
        elif ins.ctl == CTL_CALL and ins.op != OP_SPECIAL and ins.target not in function_starts:
            out.append((4 * i, "call-to-non-entry", None))
    for i, kind in cfg.exits:
        if kind == EXIT_FALL:
            out.append((4 * i, "falls-off-end", None))
    return out


def detect_dead_code(insns, cfg, jumps):
    """Non-zero words no path reaches.

    Only claimed when every register jump in the routine was resolved: an
    unresolved jump could be what reaches them.  Trailing zero words are
    alignment, not code.
    """
    if any(t is None for _l, t in jumps.values()):
        return []
    return [(4 * i, "unreachable-code", None) for i, ins in enumerate(insns)
            if i not in cfg.reachable and ins.word != 0]


# Labels the ledger header names as class-3 mechanisms, then the further
# shapes this audit adds.  Each must also pass the negative control.
LEDGER_MECHANISMS = {
    "odd-fp-single": "odd single-precision FP registers",
    "trapping-add-sub": "trapping signed add/sub",
    "nonarg-register-read-on-entry": "arguments outside a0-a3/f12/f14",
    "callee-saved-written-unsaved": "callee-saved registers clobbered unsaved",
    "computed-jump-own-code": "computed jump into its own code",
    "frame-not-first": "non-IDO prologue",
    "frame-misaligned": "non-IDO prologue",
    "nonleaf-frame-below-o32-minimum": "non-IDO prologue",
    "second-frame-allocation": "non-IDO prologue",
    "nonleaf-without-frame": "non-IDO prologue",
    "leaf-saves-ra": "non-IDO prologue",
    "leaf-saves-unused-ra": "non-IDO prologue",
    "nonleaf-ra-unsaved": "non-IDO prologue",
    "callee-saved-fp-saved-by-word": "non-IDO prologue",
    "callee-saved-fp-pair-misplaced": "non-IDO prologue",
}
EXTRA_MECHANISMS = {
    "odd-fp-double": "odd register named at double width",
    "system-instruction": "COP0/TLB/cache/eret/system instruction",
    "register-jump-to-constant": "register jump to a constant code address",
    "computed-jump": "register jump to an arithmetically computed address",
    "reserved-register": "k0/k1/gp in use",
    "sp-nonstandard-write": "sp set other than by a frame adjustment",
    "ra-nonstandard-write": "ra set other than by a call or its restore",
    "control-in-delay-slot": "branch or jump in a delay slot",
    "branch-out-of-routine": "conditional branch into another routine",
    "jump-into-foreign-interior": "jump into another routine's interior",
    "tail-jump": "jump to another routine's entry",
    "falls-off-end": "falls through into the next routine",
    "secondary-entry-point": "entered at an interior address from outside",
    "unreachable-code": "instructions no path reaches",
}
INFO_LABELS = {
    "trap-instruction", "64-bit-instruction", "table-or-pointer-jump",
    "register-jump-unresolved", "call-to-non-entry",
}


@dataclass
class Program:
    """What routine analysis needs to know about the rest of the image."""
    function_starts: set = field(default_factory=set)
    read_word: object = None                 # vram -> word or None
    interior_entries: dict = field(default_factory=dict)  # start -> {addr}


def analyse_routine(words, base, prog=None):
    """All findings for one routine: [(offset, label, detail)], sorted."""
    prog = prog or Program()
    insns = decode_words(words, base)
    if not insns:
        return []
    jumps = classify_register_jumps(insns, prog.read_word)
    end = base + 4 * len(insns)
    interior = sorted(a for a in prog.interior_entries.get(base, ())
                      if base < a < end and not (a - base) % 4)
    cfg = build_cfg(insns, {i: t for i, (_l, t) in jumps.items()},
                    [(a - base) // 4 for a in interior])
    findings = []
    findings += detect_odd_fp(insns, cfg)
    findings += detect_trapping(insns, cfg)
    findings += detect_system(insns, cfg)
    findings += detect_entry_reads(insns, cfg)
    findings += detect_unsaved_writes(insns, cfg)
    findings += [(4 * i, label, None) for i, (label, _t) in jumps.items()
                 if i in cfg.reachable]
    findings += detect_frame(insns, cfg)
    findings += detect_conventions(insns, cfg)
    findings += detect_foreign_control(insns, cfg, prog.function_starts)
    findings += [(a - base, "secondary-entry-point", None) for a in interior]
    findings += detect_dead_code(insns, cfg, jumps)
    findings.sort(key=lambda f: (f[0], f[1]))
    return findings


# --------------------------------------------------------------------------
# The resident image: segments, word access, and a cross-reference index.
# --------------------------------------------------------------------------

@dataclass
class Segment:
    rom_start: int
    rom_end: int
    vram: int


class Image:
    def __init__(self, rom, segments):
        self.rom = rom
        self.segments = sorted(segments, key=lambda s: s.vram)

    def rom_of(self, vram):
        for s in self.segments:
            if s.vram <= vram < s.vram + (s.rom_end - s.rom_start):
                return s.rom_start + (vram - s.vram)
        return None

    def read_word(self, vram):
        r = self.rom_of(vram)
        if r is None or vram % 4 or r + 4 > len(self.rom):
            return None
        return struct.unpack_from(">I", self.rom, r)[0]

    def words(self, vram, size):
        r = self.rom_of(vram)
        if r is None:
            raise ValueError("address %#x is outside the resident image" % vram)
        n = size // 4
        return list(struct.unpack_from(">%dI" % n, self.rom, r))

    def vram_range(self):
        lo = min(s.vram for s in self.segments)
        hi = max(s.vram + s.rom_end - s.rom_start for s in self.segments)
        return lo, hi


@dataclass
class Xref:
    source: int      # vram of the referencing word (overlay exports: None)
    kind: str        # call / jump / branch / address / data-word / overlay-export


def build_xrefs(image, functions, overlay_exports=()):
    """{target vram: [Xref]} over the whole resident image.

    Code references come from decoding every resident function's words
    (calls, jumps, branches, and lui/addiu or lui/ori address pairs).  Data
    references are every aligned word outside the functions whose value lies
    in the resident address range; overlay references are the export-table
    rows that name a resident address.
    """
    lo, hi = image.vram_range()
    refs = defaultdict(list)
    covered = []
    for start, size in functions:
        covered.append((start, start + size))
        insns = decode_words(image.words(start, size), start)
        pending = {}
        for k, ins in enumerate(insns):
            if ins.target is not None and lo <= ins.target < hi:
                kind = {CTL_CALL: "call", CTL_JUMP: "jump"}.get(ins.ctl, "branch")
                refs[ins.target].append(Xref(ins.addr, kind))
            if ins.op in (OP_ADDIU, OP_ORI) and ins.rs in pending:
                hi16, at = pending[ins.rs]
                if k - at <= 16:
                    low = ins.word & 0xFFFF if ins.op == OP_ORI else ins.imm
                    value = ((hi16 | low) if ins.op == OP_ORI else hi16 + low) & MASK32
                    if lo <= value < hi:
                        refs[value].append(Xref(ins.addr, "address"))
            for r in regs_in(ins.defs & MASK32):
                pending.pop(r, None)
            if ins.op == OP_LUI:
                pending[ins.rt] = ((ins.word & 0xFFFF) << 16, k)
    covered.sort()
    starts = [c[0] for c in covered]
    for seg in image.segments:
        for r in range(seg.rom_start, seg.rom_end - 3, 4):
            vram = seg.vram + (r - seg.rom_start)
            i = bisect.bisect_right(starts, vram) - 1
            if i >= 0 and covered[i][0] <= vram < covered[i][1]:
                continue
            value = struct.unpack_from(">I", image.rom, r)[0]
            if lo <= value < hi and not value % 4:
                refs[value].append(Xref(vram, "data-word"))
    for value in overlay_exports:
        refs[value].append(Xref(None, "overlay-export"))
    return refs


def raw_references(blob, base, targets):
    """{target: [offset]} for raw words or lui/addiu|ori pairs naming a target.

    Used on the overlay module images, which can hold a resident address as
    an unrelocated constant: an absolute resident address needs no patching.
    """
    targets = set(targets)
    out = defaultdict(list)
    n = len(blob) // 4
    words = struct.unpack_from(">%dI" % n, blob, 0)
    pending = {}
    for k, w in enumerate(words):
        if w in targets:
            out[w].append(base + 4 * k)
        op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
        if op in (OP_ADDIU, OP_ORI) and rs in pending and k - pending[rs][1] <= 16:
            low = w & 0xFFFF if op == OP_ORI else sext16(w & 0xFFFF)
            hi16 = pending[rs][0]
            value = ((hi16 | low) if op == OP_ORI else hi16 + low) & MASK32
            if value in targets:
                out[value].append(base + 4 * k)
        if op == OP_LUI:
            pending[rt] = ((w & 0xFFFF) << 16, k)
    return out


def interior_entries(functions, xrefs):
    """{start: {interior vram}} entered by a control transfer from outside."""
    out = defaultdict(set)
    for start, size in functions:
        end = start + size
        for a in range(start + 4, end, 4):
            for x in xrefs.get(a, ()):
                outside = x.source is None or not (start <= x.source < end)
                if outside and x.kind in ("call", "jump", "branch", "overlay-export"):
                    out[start].add(a)
    return out


# --------------------------------------------------------------------------
# Reference farm: pinned titles, ELF objects, masked search.
# --------------------------------------------------------------------------

RELOC_MASKS = {
    4: 0xFC000000,   # R_MIPS_26
    5: 0xFFFF0000,   # R_MIPS_HI16
    6: 0xFFFF0000,   # R_MIPS_LO16
    7: 0xFFFF0000,   # R_MIPS_GPREL16
    8: 0xFFFF0000,   # R_MIPS_LITERAL
    10: 0xFFFF0000,  # R_MIPS_PC16
}
SEP_WORD = 0xFFFFFFFF   # never produced by prefilter(): op 0x3F keeps 16 bits
GLOBAL_ASM_MARK = ".NON_MATCHING"


def prefilter(word):
    """A superset mask every relocation's field falls inside, by opcode."""
    op = word >> 26
    if op in (OP_J, OP_JAL):
        return word & 0xFC000000
    if op == OP_SPECIAL:
        return word
    if op in (OP_COP0, OP_COP1, OP_COP2):
        return word & 0xFFFF0000 if (word >> 21) & 31 == 8 else word
    return word & 0xFFFF0000


@dataclass
class RefObject:
    title: str
    path: str                # relative to the farm root
    text: bytes
    masks: list
    symbols: list            # [(offset, size, name)] sorted by offset
    irregular: bool = False  # a relocation outside prefilter()'s superset

    def symbol_at(self, offset):
        names = [s[2] for s in self.symbols if s[0] == offset]
        plain = [n for n in names if not n.endswith(GLOBAL_ASM_MARK)]
        return (plain or names or [None])[0]

    def marked_global_asm(self, offset):
        """asm-processor names every GLOBAL_ASM function `<name>.NON_MATCHING`."""
        return any(s[0] == offset and s[2].endswith(GLOBAL_ASM_MARK) for s in self.symbols)

    def symbol_containing(self, offset):
        best = None
        for s in self.symbols:
            if s[0] <= offset and not s[2].endswith(GLOBAL_ASM_MARK):
                best = s
        return best


def parse_object(blob, title="", path=""):
    """RefObject from an ELF32 relocatable object, or None without .text."""
    import reference_build_digest as rbd
    end, is64, sections = rbd._sections(blob)
    if is64:
        return None
    text_index = next((i for i, s in enumerate(sections) if s[0] == ".text"), None)
    if text_index is None or sections[text_index][1] == 8:
        return None
    _n, _t, off, size, _l, _e = sections[text_index]
    text = blob[off:off + size]
    if len(text) % 4:
        text += bytes(4 - len(text) % 4)
    if not text:
        return None
    masks = [MASK32] * (len(text) // 4)
    irregular = False
    for name, stype, roff, rsize, _link, entsize in sections:
        if name != ".rel.text" or stype != 9:
            continue
        for k in range(roff, roff + rsize, entsize or 8):
            r_offset, info = struct.unpack_from(end + "II", blob, k)
            if r_offset // 4 >= len(masks):
                continue
            mask = RELOC_MASKS.get(info & 0xFF, 0)
            masks[r_offset // 4] &= mask
            word = struct.unpack_from(">I", text, r_offset & ~3)[0]
            if prefilter(word) & ~mask & MASK32:
                irregular = True
    symbols = []
    for name, stype, soff, ssize, link, entsize in sections:
        if stype != 2 or not entsize:
            continue
        stroff, strsize = sections[link][2], sections[link][3]
        strtab = blob[stroff:stroff + strsize]
        for k in range(soff, soff + ssize, entsize):
            nameoff, value, sym_size, info, _other, shndx = struct.unpack_from(
                end + "IIIBBH", blob, k)
            if shndx != text_index or (info & 0xF) in (3, 4):
                continue
            stop = strtab.find(b"\0", nameoff)
            sym = strtab[nameoff:stop if stop >= 0 else None].decode("ascii", "replace")
            if sym and not sym.startswith((".L", "$")):
                symbols.append((value, sym_size, sym))
    symbols.sort()
    return RefObject(title, path, text, masks, symbols, irregular)


def parse_lock(path):
    """[(title, [object roots], commit, digest)] from tools/reference-builds.lock."""
    titles, current = [], None
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.split("#", 1)[0].strip()
            m = re.match(r"^\[(.+)\]$", line)
            if m:
                current = [m.group(1), [], None, None]
                titles.append(current)
            elif current and "=" in line:
                key, value = (p.strip() for p in line.split("=", 1))
                if key == "object_roots":
                    current[1] = value.split()
                elif key == "commit":
                    current[2] = value
                elif key == "digest":
                    current[3] = value
    return [tuple(t) for t in titles]


class Farm:
    """Every pinned reference object, with a prefiltered search index."""

    def __init__(self, refs_root, objects):
        self.refs_root = refs_root
        self.objects = objects
        self.commits = {}
        self.errors = 0
        # One prefiltered word stream with a separator after every object,
        # and every word value's positions in it.
        self.norm = array.array("I")
        self.starts = []
        for obj in objects:
            self.starts.append(len(self.norm))
            self.norm.extend(prefilter(w) for w in
                             struct.unpack(">%dI" % (len(obj.text) // 4), obj.text))
            self.norm.append(SEP_WORD)
        self.word_pos = defaultdict(list)
        for k, v in enumerate(self.norm):
            self.word_pos[v].append(k)
        self.irregular = [o for o in objects if o.irregular]

    def locate(self, pos):
        i = bisect.bisect_right(self.starts, pos) - 1
        return i, pos - self.starts[i]

    def candidates(self, norm):
        """Start positions whose prefiltered words equal `norm`."""
        anchor, best = 0, None
        for k, v in enumerate(norm[:16]):
            lst = self.word_pos.get(v)
            if not lst:
                return
            if best is None or len(lst) < len(best):
                anchor, best = k, lst
        want = array.array("I", norm)
        n = len(norm)
        for c in best:
            start = c - anchor
            if start >= 0 and self.norm[start:start + n] == want:
                yield start

    @classmethod
    def load(cls, refs_root, lock_path):
        """Every object under the lock's roots, and each title's digest check.

        The digest is tools/reference_build_digest.py's aggregate over the
        objects' mining surface, so a farm that is not the pinned one is
        reported rather than silently compared against.
        """
        import reference_build_digest as rbd
        objects, commits, digests, errors = [], {}, {}, 0
        for title, roots, commit, digest in parse_lock(lock_path):
            commits[title] = commit
            checkout = os.path.join(refs_root, title)
            paths = []
            for root in roots:
                for dirpath, dirnames, filenames in os.walk(os.path.join(checkout, root)):
                    dirnames.sort()
                    paths.extend(os.path.join(dirpath, fn) for fn in sorted(filenames)
                                 if fn.endswith(".o"))
            aggregate = hashlib.sha256()
            for full in sorted(paths, key=lambda q: os.path.relpath(q, checkout)):
                rel = os.path.relpath(full, checkout)
                try:
                    with open(full, "rb") as fh:
                        blob = fh.read()
                    aggregate.update(("%s %s\n" % (rel, rbd.mining_surface(full))).encode())
                    obj = parse_object(blob, title, os.path.relpath(full, refs_root))
                except Exception:
                    errors += 1
                    continue
                if obj is not None:
                    objects.append(obj)
            digests[title] = {"objects": len(paths),
                              "ok": bool(paths) and aggregate.hexdigest() == digest}
        farm = cls(refs_root, objects)
        farm.commits, farm.digests, farm.errors = commits, digests, errors
        return farm

    def search(self, words, limit=64):
        """[(object, word offset)] where `words` match under the object's masks."""
        if not words:
            return []
        hits = []
        for start in self.candidates([prefilter(w) for w in words]):
            i, at = self.locate(start)
            obj = self.objects[i]
            if exact_match(words, obj, at):
                hits.append((obj, at))
                if len(hits) >= limit:
                    break
        for obj in self.irregular:
            n = len(obj.masks)
            for at in range(0, n - len(words) + 1):
                if exact_match(words, obj, at) and (obj, at) not in hits:
                    hits.append((obj, at))
        return hits


def exact_match(words, obj, at):
    if at + len(words) > len(obj.masks):
        return False
    for k, w in enumerate(words):
        mask = obj.masks[at + k]
        ref = struct.unpack_from(">I", obj.text, 4 * (at + k))[0]
        if (w ^ ref) & mask:
            return False
    return True


def masked_count(obj, at, n):
    return sum(1 for m in obj.masks[at:at + n] if m != MASK32)


# Reference object kinds, strongest first.  The first two say the reference
# project holds the routine as hand-written assembly.
KIND_RANK = {
    "handwritten-source": 0,     # assembled from a hand-written .s source
    "reference-hasm": 1,         # extracted, but filed as hasm by that project
    "global-asm": 2,             # GLOBAL_ASM/INCLUDE_ASM inside a C file
    "extracted-asm": 3,          # splat output the project has not decompiled
    "compiled-c": 4,             # a C function the reference compiler built
}
HANDWRITTEN_KINDS = {"handwritten-source", "reference-hasm"}
EXCLUDED_SOURCE_DIRS = {"asm", "build", "tools", ".git", "expected", "assets", "extracted"}


class SourceIndex:
    """Hand-written .s sources and C sources of each reference checkout."""

    def __init__(self, refs_root, commits):
        self.refs_root = refs_root
        self.commits = commits
        self._asm = {}
        self._label_cache = {}
        self._c_text = {}
        self._kind = {}

    def asm_sources(self, title):
        if title not in self._asm:
            by_stem = defaultdict(list)
            top = os.path.join(self.refs_root, title)
            for dirpath, dirnames, filenames in os.walk(top):
                # Splat output, build trees and tool test fixtures are not
                # anyone's hand-written source, at whatever depth.
                dirnames[:] = sorted(d for d in dirnames if d not in EXCLUDED_SOURCE_DIRS)
                for fn in filenames:
                    if fn.endswith(".s"):
                        by_stem[fn[:-2]].append(os.path.join(dirpath, fn))
            self._asm[title] = by_stem
        return self._asm[title]

    def defines(self, path, symbol):
        key = (path, symbol)
        if key not in self._label_cache:
            try:
                with open(path, encoding="utf-8", errors="replace") as fh:
                    text = fh.read()
            except OSError:
                text = ""
            pat = re.compile(
                r"^\s*(?:(?:glabel|LEAF|XLEAF|NESTED|leaf|\.globl|\.global|\.ent)"
                r"\b[\s(]*%s\b|%s:)" % (re.escape(symbol), re.escape(symbol)), re.M)
            self._label_cache[key] = bool(pat.search(text))
        return self._label_cache[key]

    def handwritten_source_for(self, obj, symbol):
        stem = os.path.basename(obj.path)
        for suffix in (".o", ".s", ".c"):
            if stem.endswith(suffix):
                stem = stem[:-len(suffix)]
        for path in self.asm_sources(obj.title).get(stem, []):
            if symbol is None or self.defines(path, symbol):
                return path
        return None

    def handwritten_source_defining(self, title, symbol):
        for paths in self.asm_sources(title).values():
            for path in paths:
                if self.defines(path, symbol):
                    return path
        return None

    def c_source_text(self, obj):
        """The C source of a `.c.o` object at the pinned commit, else HEAD."""
        if obj.path not in self._c_text:
            self._c_text[obj.path] = self._find_c_source(obj)
        return self._c_text[obj.path]

    def _find_c_source(self, obj):
        rel = os.path.relpath(os.path.join(self.refs_root, obj.path),
                              os.path.join(self.refs_root, obj.title))
        rel = rel[:-2] if rel.endswith(".o") else rel
        parts = rel.split(os.sep)
        candidates = [os.sep.join(parts[k:]) for k in range(len(parts))]
        repo = os.path.join(self.refs_root, obj.title)
        commit = self.commits.get(obj.title)
        for cand in candidates:
            if commit:
                got = subprocess.run(["git", "-C", repo, "show", "%s:%s" % (commit, cand)],
                                     capture_output=True)
                if got.returncode == 0:
                    return got.stdout.decode("utf-8", "replace"), "pinned"
            path = os.path.join(repo, cand)
            if os.path.isfile(path):
                with open(path, encoding="utf-8", errors="replace") as fh:
                    return fh.read(), "checkout"
        return None, None

    def kind(self, obj, symbol, offset=None):
        key = (obj.path, symbol, offset)
        if key not in self._kind:
            self._kind[key] = self._classify(obj, symbol, offset)
        return self._kind[key]

    def _classify(self, obj, symbol, offset):
        parts = obj.path.split(os.sep)
        if self.handwritten_source_for(obj, symbol) and not obj.path.endswith(".c.o"):
            return "handwritten-source"
        if "hasm" in parts:
            return "reference-hasm"
        if obj.path.endswith(".s.o"):
            return "extracted-asm"
        if offset is not None and obj.marked_global_asm(offset):
            return "global-asm"
        text, _where = self.c_source_text(obj)
        if text is not None and symbol:
            if re.search(r"GLOBAL_ASM\s*\(\s*\"[^\"]*\b%s\.s\"" % re.escape(symbol), text) or \
               re.search(r"INCLUDE_ASM\s*\([^)]*\b%s\s*\)" % re.escape(symbol), text):
                return "global-asm"
        return "compiled-c"


def assemble_source(path, repo, workdir):
    """Assemble a reference hand-written source with this repo's GNU as."""
    as_bin = os.path.join(ROOT_DIR, "tools", "binutils", "mips64-elf-as")
    if not os.access(as_bin, os.X_OK):
        return None
    out = os.path.join(workdir, hashlib.sha1(path.encode()).hexdigest() + ".o")
    if not os.path.exists(out):
        cmd = [as_bin, "-march=vr4300", "-32", "-mabi=32", "-G0",
               "-I", repo, "-I", os.path.join(repo, "include"),
               "-I", os.path.dirname(path),
               "--defsym", "_MIPS_SIM=1", "--defsym", "mips=1", "-o", out, path]
        if subprocess.run(cmd, capture_output=True).returncode != 0:
            return None
    with open(out, "rb") as fh:
        return fh.read()


# --------------------------------------------------------------------------
# Edited library objects: Mickey's routine as a reference routine with
# instructions deleted.
# --------------------------------------------------------------------------

def align_deletions(words, ref_words, ref_masks):
    """Order-preserving alignment of `words` onto a reference routine.

    Allowed operations: keep a reference word (equal under its mask),
    delete a reference word, or keep a store whose only change is the stored
    register's field (bits 16-20).  Returns a dict of counts, or None when
    `words` cannot be produced from the reference by those operations alone.
    """
    n, m = len(words), len(ref_words)
    INF = 10 ** 9
    # cost[i][j]: aligning words[i:] onto ref[j:]
    cost = [[INF] * (m + 1) for _ in range(n + 1)]
    move = [[None] * (m + 1) for _ in range(n + 1)]
    for j in range(m + 1):
        cost[n][j], move[n][j] = m - j, "del"
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            best, how = cost[i][j + 1] + 1, "del"
            w, r, mask = words[i], ref_words[j], ref_masks[j]
            if not (w ^ r) & mask and cost[i + 1][j + 1] < best:
                best, how = cost[i + 1][j + 1], "keep"
            elif (r >> 26) in (OP_SWC1, OP_SW, OP_SDC1) and not (w ^ r) & mask & ~0x001F0000 \
                    and cost[i + 1][j + 1] + 0.5 < best:
                best, how = cost[i + 1][j + 1] + 0.5, "repoint"
            cost[i][j], move[i][j] = best, how
    if cost[0][0] >= INF:
        return None
    i = j = 0
    counts = Counter()
    while i < n or j < m:
        how = move[i][j]
        if how is None:
            return None
        counts[how] += 1
        if how == "del":
            j += 1
        else:
            i += 1
            j += 1
    return {"kept": counts["keep"], "repointed_stores": counts["repoint"],
            "deleted": counts["del"], "reference_words": m}


def find_edited_library(words, farm, sources, top=6):
    """The reference routine `words` is an instruction-deleted copy of."""
    n = len(words)
    if n < 8:
        return None
    norm = [prefilter(w) for w in words]
    votes = Counter()
    for k in range(n - 2):
        for hits, start in enumerate(farm.candidates(norm[k:k + 3])):
            if hits >= 200:
                break
            i, at = farm.locate(start)
            sym = farm.objects[i].symbol_containing(4 * at)
            if sym:
                votes[(i, sym)] += 1
    found = []
    for (i, sym), _v in votes.most_common(top):
        obj = farm.objects[i]
        offset, size, name = sym
        if not size:
            nxt = [s[0] for s in obj.symbols if s[0] > offset]
            size = (nxt[0] if nxt else len(obj.text)) - offset
        if size // 4 > 2 * n or size // 4 <= n:
            continue
        lo = offset // 4
        ref = list(struct.unpack_from(">%dI" % (size // 4), obj.text, offset))
        result = align_deletions(words, ref, obj.masks[lo:lo + size // 4])
        if result is None or result["kept"] + result["repointed_stores"] != n:
            continue
        if result["deleted"] > n // 2:
            continue
        result.update(object=obj.path, symbol=name, kind=sources.kind(obj, name, offset))
        found.append(result)
    if not found:
        return None
    found.sort(key=lambda r: (r["deleted"], KIND_RANK[r["kind"]], r["object"]))
    best = dict(found[0])
    best["alternatives"] = [{"object": r["object"], "symbol": r["symbol"], "kind": r["kind"],
                             "deleted": r["deleted"]} for r in found[1:4]]
    return best


# --------------------------------------------------------------------------
# Identity: cover each subsegment's routine run with the longest masked
# matches the farm holds, then attribute each routine to the match that
# contains it.
# --------------------------------------------------------------------------

MIN_IDENTITY_UNMASKED = 6   # docs/modules.md 1.2's word bar for a match
CONTEXT_UNMASKED = 16       # a run long enough to vouch for a short routine in it


def trailing_nonzero(words):
    k = len(words)
    while k > 1 and words[k - 1] == 0:
        k -= 1
    return k


class Identifier:
    def __init__(self, farm, sources, refs_root, workdir):
        self.farm, self.sources = farm, sources
        self.refs_root, self.workdir = refs_root, workdir

    def describe(self, obj, at, n, via, segment_words=None):
        sym = obj.symbol_at(4 * at)
        cont = obj.symbol_containing(4 * at)
        rec = {"object": obj.path, "symbol": sym or (cont[2] if cont else None),
               "kind": self.sources.kind(obj, sym or (cont[2] if cont else None), 4 * at),
               "offset": "%#x" % (4 * at), "words": n, "masked": masked_count(obj, at, n),
               "boundary_agrees": sym is not None, "via": via}
        if segment_words is not None:
            rec["segment_words"] = segment_words
        return rec

    @staticmethod
    def best_of(cands):
        return sorted(cands, key=lambda c: (KIND_RANK[c["kind"]], -c.get("segment_words", 0),
                                            c["object"]))

    def longest_at(self, words, p):
        """(length, hits) of the longest prefix of words[p:] the farm holds."""
        if not self.farm.search(words[p:p + 1], limit=1):
            return 0, []
        lo, hi = 1, len(words) - p
        if self.farm.search(words[p:p + hi], limit=1):
            lo = hi
        while hi - lo > 1:
            mid = (lo + hi) // 2
            if self.farm.search(words[p:p + mid], limit=1):
                lo = mid
            else:
                hi = mid
        return lo, self.farm.search(words[p:p + lo], limit=64)

    def cover(self, words):
        """Greedy cover of `words` by maximal farm matches: [(start, length, hits)]."""
        segments, p = [], 0
        while p < len(words):
            n, hits = self.longest_at(words, p)
            if n == 0:
                p += 1
                continue
            segments.append((p, n, hits))
            p += n
        return segments

    def try_handwritten_source(self, obj, symbol, words):
        """Re-test a GLOBAL_ASM/extracted match against that project's .s source."""
        path = self.sources.handwritten_source_defining(obj.title, symbol) if symbol else None
        if not path:
            return None
        repo = os.path.join(self.refs_root, obj.title)
        blob = assemble_source(path, repo, self.workdir)
        if blob is None:
            return None
        built = parse_object(blob, obj.title, os.path.relpath(path, self.refs_root))
        if built is None:
            return None
        at = next((s[0] for s in built.symbols if s[2] == symbol), None)
        if at is None or at % 4 or not exact_match(words, built, at // 4):
            return None
        head = subprocess.run(["git", "-C", repo, "rev-parse", "HEAD"], capture_output=True,
                              text=True).stdout.strip()
        return {"object": built.path, "symbol": symbol, "kind": "handwritten-source",
                "offset": "%#x" % at, "words": len(words),
                "masked": masked_count(built, at // 4, len(words)), "boundary_agrees": True,
                "via": "assembled-from-source", "source_commit": head[:12]}

    def identify_subsegment(self, run_words, spans):
        """(run record, {routine: [identity candidates, best first]}).

        spans: [(name, byte offset in the run, byte size)].
        """
        trimmed = trailing_nonzero(run_words)
        segments = self.cover(run_words[:trimmed])
        per_routine = {}
        for name, off, size in spans:
            w0 = off // 4
            words = run_words[w0:w0 + size // 4]
            k = trailing_nonzero(words)
            cands = []
            for p, n, hits in segments:
                if not (p <= w0 and w0 + k <= p + n):
                    continue
                for obj, at in hits:
                    seg_unmasked = n - masked_count(obj, at, n)
                    own_unmasked = k - masked_count(obj, at + w0 - p, k)
                    # A short routine borrows a long run's context; a short
                    # run lends none.
                    if seg_unmasked < CONTEXT_UNMASKED and own_unmasked < MIN_IDENTITY_UNMASKED:
                        continue
                    cands.append(self.describe(obj, at + w0 - p, k, "segment", n))
            if not cands:
                for obj, at in self.farm.search(words[:k], limit=32):
                    d = self.describe(obj, at, k, "routine")
                    tail = struct.unpack_from(">%dI" % (len(obj.masks) - at - k), obj.text,
                                              4 * (at + k))
                    whole = at == 0 and not any(tail)
                    if k - d["masked"] >= MIN_IDENTITY_UNMASKED or whole:
                        d["whole_object"] = whole
                        d["weak"] = k - d["masked"] < MIN_IDENTITY_UNMASKED
                        cands.append(d)
            # Compiled C refutes only on a routine-sized, boundary-aligned
            # match; anything shorter is a coincidence of common idioms.
            cands = [c for c in cands if c["kind"] != "compiled-c" or (
                c["words"] - c["masked"] >= MIN_IDENTITY_UNMASKED and c["boundary_agrees"])]
            cands = self.best_of(cands)
            if cands and cands[0]["kind"] in ("global-asm", "extracted-asm"):
                obj = next(o for o in self.farm.objects if o.path == cands[0]["object"])
                got = self.try_handwritten_source(obj, cands[0]["symbol"], words[:k])
                if got:
                    cands.insert(0, got)
            per_routine[name] = cands
        covered = sum(n for _p, n, _h in segments)
        claim = "none"
        long_segments = [sg for sg in segments if sg[1] >= MIN_IDENTITY_UNMASKED]
        if len(segments) == 1 and segments[0][1] == trimmed:
            p, n, hits = segments[0]
            whole = any(at == 0 and not any(struct.unpack_from(
                ">%dI" % (len(o.masks) - n), o.text, 4 * n)) for o, at in hits)
            claim = "whole-object" if whole else "single-object-run"
        elif long_segments:
            claim = "multi-segment"
        run = {
            "words": trimmed, "covered_words": covered, "claim": claim,
            "segments": [{"run_offset": "%#x" % (4 * p), "words": n,
                          "matches": sorted({o.path for o, _a in hits})[:6]}
                         for p, n, hits in segments if n >= MIN_IDENTITY_UNMASKED],
            "short_segments": sum(1 for _p, n, _h in segments if n < MIN_IDENTITY_UNMASKED),
        }
        return run, per_routine


# --------------------------------------------------------------------------
# The ledger, the tree and the build.
# --------------------------------------------------------------------------

def parse_ledger(text):
    """[(subsegment, ledger class)] in file order.

    Class 3: the entries in the block a comment line naming "Evidence class
    3" opens, up to the next blank line.  Class 2: entries the header's
    paragraph "2." names.  Every other entry is class 1.
    """
    entries, class3, header2 = [], set(), set()
    in3, para = False, None
    names_re = re.compile(r"\b((?:main|libultra)/[A-Za-z0-9_]+|entrypoint)\b")
    for raw in text.splitlines():
        line = raw.strip()
        if line.startswith("#"):
            body = line.lstrip("#").strip()
            m = re.match(r"^([123])\.\s", body)
            if m:
                para = m.group(1)
            if re.search(r"Evidence class 3", body):
                in3 = True
            if para == "2":
                header2.update(names_re.findall(body))
            continue
        if not line:
            in3 = False
            para = None
            continue
        name = line.split("#", 1)[0].strip()
        if name:
            entries.append(name)
            if in3:
                class3.add(name)
    out = []
    for name in entries:
        cls = 3 if name in class3 else 2 if name in header2 else 1
        out.append((name, cls))
    return out


STRIP_COMMENTS = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
PRAGMA_RE = re.compile(r"#\s*pragma\s+GLOBAL_ASM\s*\(\s*\"([^\"]+)\"\s*\)")
INCLUDE_ASM_RE = re.compile(r"INCLUDE_ASM\s*\(([^)]*)\)")


def scan_sources(src_dir, names, root=None):
    """C definitions and GLOBAL_ASM references to `names` under src_dir.

    Returns {name: [(relative path, line, 'c-definition'|'global-asm')]}.
    """
    root = root or os.path.dirname(src_dir)
    names = set(names)
    found = defaultdict(list)
    ident = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")
    for dirpath, _dirs, files in os.walk(src_dir):
        for fn in sorted(files):
            if not fn.endswith((".c", ".h", ".inc")):
                continue
            path = os.path.join(dirpath, fn)
            rel = os.path.relpath(path, root)
            with open(path, encoding="utf-8", errors="replace") as fh:
                raw = fh.read()
            text = STRIP_COMMENTS.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), raw)
            for m in ident.finditer(text):
                if m.group(1) not in names:
                    continue
                depth, k = 0, m.end() - 1
                while k < len(text):
                    if text[k] == "(":
                        depth += 1
                    elif text[k] == ")":
                        depth -= 1
                        if depth == 0:
                            break
                    k += 1
                rest = text[k + 1:k + 200].lstrip()
                if rest.startswith("{"):
                    line = text.count("\n", 0, m.start()) + 1
                    found[m.group(1)].append((rel, line, "c-definition"))
            if not fn.endswith(".c"):
                continue
            for m in PRAGMA_RE.finditer(text):
                target = m.group(1)
                stem = os.path.splitext(os.path.basename(target))[0]
                hits = {stem} & names
                asm_path = os.path.join(root, target)
                if os.path.isfile(asm_path):
                    with open(asm_path, encoding="utf-8", errors="replace") as fh:
                        for lm in re.finditer(r"^\s*(?:glabel|alabel)\s+(\S+)", fh.read(), re.M):
                            if lm.group(1) in names:
                                hits.add(lm.group(1))
                line = text.count("\n", 0, m.start()) + 1
                for name in hits:
                    found[name].append((rel, line, "global-asm"))
            for m in INCLUDE_ASM_RE.finditer(text):
                for arg in (a.strip() for a in m.group(1).split(",")):
                    if arg in names:
                        line = text.count("\n", 0, m.start()) + 1
                        found[arg].append((rel, line, "global-asm"))
    return found


MAP_TEXT_RE = re.compile(r"^\s\.text\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S+)")


def parse_map_text(map_text):
    """[(start, end, object)] for every .text input section in a GNU ld map."""
    out = []
    for line in map_text.splitlines():
        m = MAP_TEXT_RE.match(line)
        if m:
            start, size = int(m.group(1), 16), int(m.group(2), 16)
            if size:
                out.append((start, start + size, m.group(3)))
    out.sort()
    return out


def object_at(sections, addr):
    i = bisect.bisect_right([s[0] for s in sections], addr) - 1
    if i >= 0 and sections[i][0] <= addr < sections[i][1]:
        return sections[i][2]
    return None


# --------------------------------------------------------------------------
# Helper relationships and verdicts.
# --------------------------------------------------------------------------

def verify_helper(routine, siblings, xrefs, falls_into, raw_overlay=()):
    """Is `routine` reachable only from one parent routine of its subsegment?

    siblings: [(name, start, end)] of the subsegment.  falls_into: the name
    of the routine whose path runs off its end into this one, if any.
    Returns {parent, references, ...} or a dict with 'parent': None and why.
    """
    name, start, end = routine
    sources = []
    for a in range(start, end, 4):
        for x in xrefs.get(a, ()):
            if x.source is not None and start <= x.source < end:
                continue
            sources.append((a, x))
    owners = set()
    detail = []
    for a, x in sources:
        owner = None
        if x.source is not None:
            for sname, s, e in siblings:
                if s <= x.source < e:
                    owner = sname
        owners.add(owner)
        detail.append({"target": "+%#x" % (a - start), "kind": x.kind,
                       "from": owner or "outside the subsegment"})
    for rom_offset in raw_overlay:
        owners.add(None)
        detail.append({"target": "+0x0", "kind": "overlay-raw-constant",
                       "from": "overlay image ROM %#x" % rom_offset})
    if falls_into:
        owners.add(falls_into)
        detail.append({"target": "+0x0", "kind": "fall-through", "from": falls_into})
    if not detail:
        return {"parent": None, "why": "no reference at all", "references": []}
    if len(owners) != 1 or None in owners:
        return {"parent": None, "why": "referenced from more than one place",
                "references": detail}
    return {"parent": owners.pop(), "references": detail}


def summarise_findings(findings, control):
    """Group findings by label; mark which can carry a verdict."""
    groups = defaultdict(list)
    for off, label, detail in findings:
        groups[label].append((off, detail))
    mechanisms, info = [], []
    for label in sorted(groups):
        rows = groups[label]
        regs = sorted({d for _o, d in rows if isinstance(d, str)})
        count = sum(d if isinstance(d, int) else 1 for _o, d in rows)
        rec = {"label": label, "count": count, "sites": len(rows),
               "offsets": ["+%#x" % o for o, _d in rows]}
        if regs:
            order = {reg_name(k): k for k in range(64)}
            rec["registers"] = sorted({r for d in regs for r in d.split(",")},
                                      key=lambda r: order.get(r, 99))
        listed = label in LEDGER_MECHANISMS
        candidate = listed or label in EXTRA_MECHANISMS
        clean = control.get(label, 0) == 0
        rec["ledger_listed"] = listed
        rec["control_functions"] = control.get(label, 0)
        if candidate and clean:
            mechanisms.append(rec)
        else:
            rec["why_info"] = ("fires in IDO-compiled control code" if candidate
                               else "not an IDO-impossible shape")
            info.append(rec)
    return mechanisms, info


def derive_classes(identity, mechanisms, edited, helper):
    classes = []
    if identity and identity["kind"] in HANDWRITTEN_KINDS:
        classes.append(1)
    if identity and identity["kind"] != "compiled-c" and mechanisms:
        classes.append(2)
    if mechanisms or edited or (helper and helper.get("parent")):
        classes.append(3)
    return classes


def verdict_for(ledger_class, classes, identity):
    if identity and identity["kind"] == "compiled-c":
        return "refuted"
    if not classes:
        return "unsupported"
    if ledger_class in classes:
        return "supported"
    return "supported-other-class"


# --------------------------------------------------------------------------
# The audit.
# --------------------------------------------------------------------------

def load_tree(version):
    """Everything progress.py reads, enumerated with progress.py itself."""
    import progress
    import yaml
    sym_path = os.path.join(ROOT_DIR, "symbol_addrs.%s.txt" % version)
    elf_path = os.path.join(ROOT_DIR, "build", "mickey.%s.elf" % version)
    yaml_path = os.path.join(ROOT_DIR, "mickey.%s.yaml" % version)
    adopted = progress.get_adopted_symbol_addresses(sym_path)
    all_funcs, func_addrs, _ph, _al = progress.get_elf_functions(
        elf_path, progress.find_objdump(os.path.join(ROOT_DIR, "tools")), adopted)
    labelled = progress.get_asm_labelled_names(os.path.join(ROOT_DIR, "asm"))
    matched = set(all_funcs) - labelled
    index = progress.subsegment_index(yaml_path)
    ledger_path = os.path.join(ROOT_DIR, "verified_asm.%s.txt" % version)
    subsegments = progress.get_verified_asm_subsegments(ledger_path)
    verified = {n for n, a in func_addrs.items()
                if n not in matched and progress.subsegment_of(a, index) in subsegments}
    with open(yaml_path, encoding="utf-8") as fh:
        cfg = yaml.safe_load(fh)
    starts = []
    for seg in cfg["segments"]:
        if isinstance(seg, dict) and isinstance(seg.get("start"), int):
            starts.append(seg["start"])
        elif isinstance(seg, list) and seg and isinstance(seg[0], int):
            starts.append(seg[0])
    segments, main_vram = [], None
    for seg in cfg["segments"]:
        if not (isinstance(seg, dict) and seg.get("type") == "code"
                and isinstance(seg.get("vram"), int)):
            continue
        end = min(s for s in starts if s > seg["start"])
        segments.append(Segment(seg["start"], end, seg["vram"]))
        if seg.get("name") == "main":
            main_vram = seg["vram"]
    with open(ledger_path, encoding="utf-8") as fh:
        ledger = parse_ledger(fh.read())
    return {
        "all_funcs": all_funcs, "func_addrs": func_addrs, "matched": matched,
        "index": index, "subsegments": subsegments, "verified": verified,
        "segments": segments, "main_vram": main_vram, "ledger": ledger,
        "subsegment_of": progress.subsegment_of,
    }


def run_audit(args):
    workdir = tempfile.mkdtemp(prefix="hasm-audit-")
    try:
        return _run_audit(args, workdir)
    finally:
        shutil.rmtree(workdir, ignore_errors=True)


def _run_audit(args, workdir):
    tree = load_tree(args.version)
    with open(os.path.join(ROOT_DIR, "baseroms", "mickey.%s.z64" % args.version), "rb") as fh:
        rom = fh.read()
    image = Image(rom, tree["segments"])
    all_funcs, func_addrs = tree["all_funcs"], tree["func_addrs"]
    functions = sorted((func_addrs[n], all_funcs[n]) for n in all_funcs
                       if n in func_addrs and all_funcs[n] >= 4
                       and image.rom_of(func_addrs[n]) is not None)
    starts = {s for s, _z in functions}

    # Overlay export rows for overlay 0 name resident addresses as offsets
    # from the main segment's base (1159 of 1161 land on function entries).
    import overlay_tables
    exports = [tree["main_vram"] + r["offset"] for r in overlay_tables.read_rom_table(rom)
               if r["overlay"] == 0]
    xrefs = build_xrefs(image, functions, exports)
    prog = Program(starts, image.read_word, interior_entries(functions, xrefs))

    # Negative control: every C-matched resident function.
    control = Counter()
    control_examples = defaultdict(list)
    control_names = sorted(n for n in tree["matched"] if n in func_addrs
                           and all_funcs.get(n, 0) >= 4 and image.rom_of(func_addrs[n]))
    for name in control_names:
        a = func_addrs[name]
        labels = {l for _o, l, _d in analyse_routine(image.words(a, all_funcs[name]), a, prog)}
        for l in labels:
            control[l] += 1
            if len(control_examples[l]) < 4:
                control_examples[l].append(name)

    # Coverage.
    verified = sorted(tree["verified"], key=lambda n: func_addrs[n])
    with open(os.path.join(ROOT_DIR, "build", "mickey.%s.map" % args.version),
              encoding="utf-8", errors="replace") as fh:
        map_sections = parse_map_text(fh.read())
    in_src = scan_sources(os.path.join(ROOT_DIR, "src"), verified, ROOT_DIR)
    ledger = dict(tree["ledger"])
    coverage_failures = []
    if len(verified) != args.expected:
        coverage_failures.append("counted %d verified-asm routines, expected %d"
                                 % (len(verified), args.expected))
    missing = [s for s in ledger if s not in tree["subsegments"]]
    if missing:
        coverage_failures.append("ledger parse disagrees with progress.py: %s" % missing)

    # Routines grouped by subsegment, in address order.
    by_sub = defaultdict(list)
    for name in verified:
        by_sub[tree["subsegment_of"](func_addrs[name], tree["index"])].append(name)

    farm = None
    sources = None
    if not args.no_references:
        farm = Farm.load(args.refs_root, os.path.join(ROOT_DIR, "tools", "reference-builds.lock"))
        sources = SourceIndex(args.refs_root, farm.commits)

    ident = Identifier(farm, sources, args.refs_root, workdir) if farm else None

    subsegment_records = []
    routine_records = []
    for sub, cls in tree["ledger"]:
        names = by_sub.get(sub, [])
        if not names:
            coverage_failures.append("ledger subsegment %s holds no counted routine" % sub)
            continue
        first, last = func_addrs[names[0]], func_addrs[names[-1]] + all_funcs[names[-1]]
        spans = [(n, func_addrs[n] - first, all_funcs[n]) for n in names]
        if ident:
            run, per_routine = ident.identify_subsegment(image.words(first, last - first), spans)
        else:
            run, per_routine = {"segments": [], "claim": "not searched"}, {n: [] for n in names}
        subsegment_records.append({
            "name": sub, "ledger_class": cls,
            "rom": "%#x-%#x" % (image.rom_of(first), image.rom_of(last)),
            "routines": names, "run": run,
        })
        for n in names:
            a, size = func_addrs[n], all_funcs[n]
            findings = analyse_routine(image.words(a, size), a, prog)
            mechanisms, info = summarise_findings(findings, control)
            cands = per_routine[n]
            identity = cands[0] if cands else None
            record = {
                "name": n, "vram": "%#x" % a, "rom": "%#x" % image.rom_of(a),
                "size": "%#x" % size, "words": size // 4, "subsegment": sub,
                "ledger_class": cls,
                "linked_from": object_at(map_sections, a),
                "source_references": [{"file": f, "line": l, "kind": kd}
                                      for f, l, kd in in_src.get(n, [])],
                "identity": identity,
                "identity_alternatives": len(cands) - 1 if cands else 0,
                "identity_builds": sorted({c["object"].split(os.sep)[0] for c in cands}),
                "mechanisms": mechanisms, "info": info,
                "edited_library": None, "helper": None,
            }
            if not mechanisms and not identity and farm:
                record["edited_library"] = find_edited_library(
                    image.words(a, size), farm, sources)
            routine_records.append(record)

    # Helper relationships, once every sibling's own evidence is known.
    by_name = {r["name"]: r for r in routine_records}
    for sub in subsegment_records:
        siblings = [(n, func_addrs[n], func_addrs[n] + all_funcs[n]) for n in sub["routines"]]
        for k, n in enumerate(sub["routines"]):
            rec = by_name[n]
            if rec["mechanisms"] or rec["identity"] or rec["edited_library"]:
                continue
            prev = sub["routines"][k - 1] if k else None
            falls = None
            if prev:
                pf = [l for l in by_name[prev]["info"] + by_name[prev]["mechanisms"]
                      if l["label"] == "falls-off-end"]
                falls = prev if pf else None
            raw = raw_references(rom[overlay_tables.MODULES_BASE:overlay_tables.MODULES_END],
                                 overlay_tables.MODULES_BASE,
                                 range(siblings[k][1], siblings[k][2], 4))
            hits = sorted(o for v in raw.values() for o in v)
            helper = verify_helper(siblings[k], siblings, xrefs, falls, hits)
            helper["overlay_images_scanned"] = True
            if helper.get("parent"):
                parent = by_name[helper["parent"]]
                helper["parent_has_evidence"] = bool(
                    parent["mechanisms"] or parent["identity"] or parent["edited_library"])
                if not helper["parent_has_evidence"]:
                    helper["why"] = "the parent carries no evidence of its own"
                    helper["parent"] = None
            rec["helper"] = helper

    for rec in routine_records:
        classes = derive_classes(rec["identity"], rec["mechanisms"],
                                 rec["edited_library"], rec["helper"])
        rec["derived_classes"] = classes
        rec["verdict"] = verdict_for(rec["ledger_class"], classes, rec["identity"])
        problems = []
        if not (rec["linked_from"] or "").endswith(".s.o"):
            problems.append("not linked from an assembled .s object")
        if rec["source_references"]:
            problems.append("named by C source under src/")
        rec["coverage_problems"] = problems
        if problems:
            coverage_failures.append("%s: %s" % (rec["name"], "; ".join(problems)))

    verdicts = Counter(r["verdict"] for r in routine_records)
    flags = []

    def flag(kind, subject, text):
        flags.append({"kind": kind, "subject": subject, "text": text})

    for title, row in (farm.digests.items() if farm else ()):
        if not row["ok"]:
            flag("farm", title, "reference objects do not match the lock's pinned digest; "
                 "identity evidence from this title is not reproducible")
    for sub in subsegment_records:
        claim = sub["run"]["claim"]
        if farm and sub["ledger_class"] in (1, 2) and claim not in ("whole-object",
                                                                   "single-object-run"):
            flag("mismatch", sub["name"],
                 "ledger class %d claims one object or one object's routine run; the "
                 "re-derived run is %s" % (sub["ledger_class"], claim))
        if farm and sub["ledger_class"] == 3 and claim in ("whole-object", "single-object-run"):
            flag("mismatch", sub["name"], "ledger class 3 claims no reference identity; the "
                 "whole run matches (%s)" % claim)
    for rec in routine_records:
        ident = rec["identity"]
        if rec["verdict"] == "supported-other-class":
            flag("mismatch", rec["name"], "ledger class %d not supported; re-derived class %s"
                 % (rec["ledger_class"], ",".join(map(str, rec["derived_classes"]))))
        elif rec["verdict"] != "supported":
            flag("missing", rec["name"], "%s (ledger class %d)" % (rec["verdict"],
                                                                  rec["ledger_class"]))
        if ident and ident.get("weak") and not rec["mechanisms"]:
            flag("weak", rec["name"], "identity rests on %d unmasked word(s) of a whole tiny "
                 "object, with no mechanism" % (ident["words"] - ident["masked"]))
        if rec["edited_library"] and not rec["mechanisms"] and not ident:
            flag("weak", rec["name"], "carried only by the edited-library reproduction "
                 "(no IDO-impossible instruction of its own)")
        if rec["helper"] and rec["helper"].get("parent") and not rec["mechanisms"] and not ident:
            flag("weak", rec["name"], "carried only as %s's private helper"
                 % rec["helper"]["parent"])
        if ident and ident.get("via") == "assembled-from-source":
            flag("note", rec["name"], "identity is to %s assembled from the reference checkout "
                 "at %s, not to a pinned farm object" % (ident["object"],
                                                       ident.get("source_commit")))
    return {
        "schema": SCHEMA,
        "inputs": {
            "rom_sha1": hashlib.sha1(rom).hexdigest(),
            "ledger": "verified_asm.%s.txt" % args.version,
            "elf": "build/mickey.%s.elf" % args.version,
            "reference_lock": "tools/reference-builds.lock",
            "reference_objects": len(farm.objects) if farm else 0,
            "reference_unreadable": farm.errors if farm else 0,
            "reference_commits": farm.commits if farm else {},
            "reference_digests": farm.digests if farm else {},
        },
        "coverage": {"expected": args.expected, "counted": len(verified),
                     "ok": not coverage_failures, "failures": coverage_failures},
        "control": {
            "functions": len(control_names),
            "labels": {l: {"functions": c, "examples": control_examples[l]}
                       for l, c in sorted(control.items())},
        },
        "subsegments": subsegment_records,
        "routines": routine_records,
        "flags": flags,
        "summary": {"routines": len(routine_records), "verdicts": dict(sorted(verdicts.items()))},
    }


# --------------------------------------------------------------------------
# Reporting.
# --------------------------------------------------------------------------

def evidence_line(rec):
    parts = []
    ident = rec["identity"]
    if ident:
        parts.append("ref %s %s %d/%dm" % (ident["kind"], ident["object"], ident["words"],
                                          ident["masked"]))
    for m in rec["mechanisms"]:
        parts.append("%s x%d" % (m["label"], m["count"]))
    if rec["edited_library"]:
        e = rec["edited_library"]
        parts.append("edited-library %s:%s -%d" % (e["object"], e["symbol"], e["deleted"]))
    if rec["helper"] and rec["helper"].get("parent"):
        parts.append("helper of %s" % rec["helper"]["parent"])
    return "; ".join(parts) or "none"


def render_report(audit):
    out = []
    cov = audit["coverage"]
    out.append("hasm-audit: %d routines (expected %d), %d reference objects, "
               "%d control functions" % (cov["counted"], cov["expected"],
                                         audit["inputs"]["reference_objects"],
                                         audit["control"]["functions"]))
    out.append("coverage: %s" % ("ok" if cov["ok"] else "FAIL"))
    for f in cov["failures"]:
        out.append("  - %s" % f)
    out.append("negative control (labels that fire in IDO-compiled C; demoted):")
    for label, row in audit["control"]["labels"].items():
        out.append("  %-34s %4d  e.g. %s" % (label, row["functions"], ", ".join(row["examples"])))
    out.append("")
    out.append("%-28s %-28s %s %-22s %s" % ("routine", "subsegment", "L", "verdict", "evidence"))
    for rec in audit["routines"]:
        out.append("%-28s %-28s %d %-22s %s" % (
            rec["name"], rec["subsegment"], rec["ledger_class"],
            rec["verdict"] + ("" if rec["verdict"] != "supported-other-class"
                              else "(%s)" % ",".join(map(str, rec["derived_classes"]))),
            evidence_line(rec)))
    out.append("")
    out.append("subsegment runs against the farm:")
    for sub in audit["subsegments"]:
        run = sub["run"]
        segs = run.get("segments", [])
        out.append("  %-28s L%d %-18s %4d/%-4d words covered; %s" % (
            sub["name"], sub["ledger_class"], run["claim"], run.get("covered_words", 0),
            run.get("words", 0), "; ".join("%s+%d %s" % (g["run_offset"], g["words"],
                                                         ",".join(g["matches"][:2]))
                                           for g in segs[:3]) or "-"))
    out.append("")
    out.append("flags:")
    for f in audit["flags"]:
        out.append("  - [%s] %s: %s" % (f["kind"], f["subject"], f["text"]))
    if not audit["flags"]:
        out.append("  none")
    out.append("")
    out.append("verdicts: %s" % ", ".join("%s=%d" % kv for kv in audit["summary"]["verdicts"].items()))
    return "\n".join(out)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--version", default="us")
    ap.add_argument("--refs-root", default=DEFAULT_REFS,
                    help="reference farm checkout root (default $REFS_ROOT, else "
                         "~/Desktop/dev/decomp-refs)")
    ap.add_argument("--expected", type=int, default=EXPECTED_ROUTINES)
    ap.add_argument("--no-references", action="store_true",
                    help="skip the reference farm (mechanism evidence only)")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--check", action="store_true",
                    help="exit 1 if coverage fails or any routine is unsupported or refuted")
    ap.add_argument("--strict", action="store_true",
                    help="with --check, also fail on any ledger mismatch flag (a routine "
                         "supported only by another class, or a class-1/2 subsegment that is "
                         "not one reference object or routine run)")
    args = ap.parse_args(argv)
    audit = run_audit(args)
    if args.json:
        print(json.dumps(audit, indent=1))
    else:
        print(render_report(audit))
    if args.check:
        bad = [r["name"] for r in audit["routines"] if r["verdict"] in ("unsupported", "refuted")]
        if args.strict:
            bad += [f["subject"] for f in audit["flags"] if f["kind"] == "mismatch"]
        if bad or not audit["coverage"]["ok"]:
            print("hasm-audit: FAIL (%s%s)" % (
                ", ".join(bad) or "no routine", "" if audit["coverage"]["ok"]
                else "; coverage failure"), file=sys.stderr)
            return 1
        print("hasm-audit: OK", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
