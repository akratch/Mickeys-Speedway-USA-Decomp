#!/usr/bin/env python3
"""Diagnostic regional operation counts with MIPS delay-slot execution paths.

Reads existing objects only. It does not compile, resolve relocation identities,
prove equivalence, or supply a matching acceptance verdict. Reports are private
workbench evidence and should be written under ignored build/.
"""
from __future__ import annotations

import argparse
import collections
import json
import hashlib
import struct
from pathlib import Path

import reloc_surface as rs


LOADS = {0x20: ("lb", 1), 0x21: ("lh", 2), 0x22: ("lwl", 4),
         0x23: ("lw", 4), 0x24: ("lbu", 1), 0x25: ("lhu", 2),
         0x26: ("lwr", 4), 0x27: ("lwu", 4), 0x30: ("ll", 4),
         0x31: ("lwc1", 4), 0x34: ("lld", 8), 0x35: ("ldc1", 8),
         0x37: ("ld", 8)}
STORES = {0x28: ("sb", 1), 0x29: ("sh", 2), 0x2A: ("swl", 4),
          0x2B: ("sw", 4), 0x2C: ("sdl", 8), 0x2D: ("sdr", 8),
          0x2E: ("swr", 4), 0x38: ("sc", 4), 0x39: ("swc1", 4),
          0x3C: ("scd", 8), 0x3D: ("sdc1", 8), 0x3F: ("sd", 8)}
SPECIAL = {0: "sll", 2: "srl", 3: "sra", 4: "sllv", 6: "srlv", 7: "srav",
           8: "jr", 9: "jalr", 12: "syscall", 13: "break", 15: "sync",
           16: "mfhi", 17: "mthi", 18: "mflo", 19: "mtlo",
           20: "dsllv", 22: "dsrlv", 23: "dsrav", 24: "mult", 25: "multu",
           26: "div", 27: "divu", 28: "dmult", 29: "dmultu",
           30: "ddiv", 31: "ddivu", 32: "add", 33: "addu", 34: "sub",
           35: "subu", 36: "and", 37: "or", 38: "xor", 39: "nor",
           42: "slt", 43: "sltu", 44: "dadd", 45: "daddu", 46: "dsub",
           47: "dsubu", 56: "dsll", 58: "dsrl", 59: "dsra",
           60: "dsll32", 62: "dsrl32", 63: "dsra32"}
IMMEDIATE = {8: "addi", 9: "addiu", 10: "slti", 11: "sltiu", 12: "andi",
             13: "ori", 14: "xori", 15: "lui", 24: "daddi", 25: "daddiu"}
BRANCH = {4: "beq", 5: "bne", 6: "blez", 7: "bgtz",
          20: "beql", 21: "bnel", 22: "blezl", 23: "bgtzl"}
REGIMM = {0: "bltz", 1: "bgez", 2: "bltzl", 3: "bgezl",
          16: "bltzal", 17: "bgezal", 18: "bltzall", 19: "bgezall"}
FP = {0: "add", 1: "sub", 2: "mul", 3: "div", 4: "sqrt", 5: "abs",
      6: "mov", 7: "neg", 8: "round.l", 9: "trunc.l", 10: "ceil.l",
      11: "floor.l", 12: "round.w", 13: "trunc.w", 14: "ceil.w",
      15: "floor.w", 32: "cvt.s", 33: "cvt.d", 36: "cvt.w", 37: "cvt.l"}


def decode(word, offset):
    """Decode only the architectural fields used by this report; unknowns stay explicit."""
    op, a, b, c = word >> 26, word >> 21 & 31, word >> 16 & 31, word >> 11 & 31
    fn, shift, imm = word & 63, word >> 6 & 31, rs.sext16(word & 65535)
    row = {"offset": offset, "opcode": "unknown", "family": "unknown",
           "sources": [], "destination": None, "control": None}
    g = lambda r: "r%d" % r
    f = lambda r: "f%d" % r
    if op in LOADS or op in STORES:
        load = op in LOADS
        name, width = (LOADS if load else STORES)[op]
        fp = op in (0x31, 0x35, 0x39, 0x3D)
        row.update(opcode=name, family="load" if load else "store", width=width,
                   sources=[g(a)] if load else [g(a), f(b) if fp else g(b)],
                   destination=(f(b) if fp else g(b)) if load else None,
                   displacement=imm, partial=name in ("lwl", "lwr", "swl", "swr", "sdl", "sdr"))
        if op in (0x38, 0x3C):
            row["destination"] = g(b)
    elif op in IMMEDIATE:
        row.update(opcode=IMMEDIATE[op], family="integer", sources=[] if op == 15 else [g(a)],
                   destination=g(b), immediate=(word & 65535) if op in (12, 13, 14, 15) else imm)
    elif op == 0:
        name = SPECIAL.get(fn, "unknown-special")
        row.update(opcode=name, family="integer", sources=[g(a), g(b)], destination=g(c), shift=shift)
        if word == 0:
            row.update(opcode="nop", family="nop", sources=[], destination=None)
        elif fn in (24, 25, 28, 29):
            row.update(family="multiply", destination=None, multiply_kind="integer64" if fn >= 28 else "integer32")
        elif fn in (26, 27, 30, 31):
            row.update(family="divide", destination=None)
        elif fn in (8, 9):
            row.update(family="control", sources=[g(a)], destination=g(c) if fn == 9 else None,
                       control="call" if fn == 9 else ("return" if a == 31 else "indirect"))
        elif fn in (12, 13):
            row.update(family="control", control="trap", destination=None)
        elif fn in (16, 18):
            row["sources"] = ["hi" if fn == 16 else "lo"]
        elif fn in (17, 19):
            row.update(sources=[g(a)], destination="hi" if fn == 17 else "lo")
        elif fn in (0, 2, 3, 56, 58, 59, 60, 62, 63):
            row["sources"] = [g(b)]
        elif fn not in SPECIAL:
            row.update(family="unknown", destination=None)
    elif op in (2, 3):
        row.update(opcode="jal" if op == 3 else "j", family="control",
                   control="call" if op == 3 else "jump", destination="r31" if op == 3 else None,
                   target=(word & 0x03FFFFFF) << 2)
    elif op in BRANCH or (op == 1 and b in REGIMM) or (op == 17 and a == 8):
        name = BRANCH.get(op) or (REGIMM.get(b) if op == 1 else ("bc1t" if b & 1 else "bc1f") + ("l" if b & 2 else ""))
        row.update(opcode=name, family="control", control="branch", target=offset + 4 + imm * 4,
                   likely=(op in (20, 21, 22, 23) or (op == 1 and b in (2, 3, 18, 19)) or (op == 17 and bool(b & 2))),
                   sources=(["fcc"] if op == 17 else ([g(a), g(b)] if op in (4, 5, 20, 21) else [g(a)])),
                   unconditional=(op in (4, 20) and a == b))
        if op == 1 and b >= 16:
            row["control"] = "conditional-call"
    elif op == 17:
        if a in (0, 2, 4, 6):
            row.update(opcode={0: "mfc1", 2: "cfc1", 4: "mtc1", 6: "ctc1"}[a], family="transfer",
                       sources=[g(b)] if a in (4, 6) else [f(c)],
                       destination=f(c) if a in (4, 6) else g(b))
        elif a in (16, 17, 20, 21):
            suffix = {16: "s", 17: "d", 20: "w", 21: "l"}[a]
            row.update(opcode=(FP.get(fn, "compare" if fn >= 48 else "unknown-fp") + "." + suffix),
                       family="multiply" if fn == 2 else "floating",
                       sources=[f(c), f(b)] if fn <= 3 or fn >= 48 else [f(c)],
                       destination="fcc" if fn >= 48 else f(shift))
            if fn == 2 and a in (16, 17):
                row["multiply_kind"] = "float32" if a == 16 else "float64"
            elif fn <= 15 and a not in (16, 17):
                row["family"] = "unknown"
            elif fn not in FP and fn < 48:
                row["family"] = "unknown"
    return row


def counts(rows):
    result = collections.Counter()
    for row in rows:
        key = row["family"]
        if key in ("load", "store"):
            key += ":%d%s" % (row["width"], "-partial" if row["partial"] else "")
        elif key == "multiply":
            key += ":" + row["multiply_kind"]
        result[key] += 1
    return dict(sorted(result.items()))


def constant(value):
    return {"kind": "constant-bits", "value": value & 0xFFFFFFFF,
            "definition_offsets": []}


def operand(register, state):
    return state.get(register, {"kind": "unresolved", "register": register})


def apply(row, state, relocated=False):
    """Conservative local constant propagation; memory contents are never inferred."""
    state = dict(state)
    name, dest = row["opcode"], row["destination"]
    values = [operand(r, state) for r in row["sources"]]
    result = None
    if relocated:
        pass
    elif name == "lui":
        result = constant(row["immediate"] << 16)
    elif name in ("mtc1", "mfc1", "mov.s") and values and values[0]["kind"] == "constant-bits":
        result = values[0]
    elif name in ("ori", "andi", "xori", "addiu") and values[0]["kind"] == "constant-bits":
        a, b = values[0]["value"], row["immediate"]
        result = constant({"ori": lambda: a | b, "andi": lambda: a & b,
                           "xori": lambda: a ^ b, "addiu": lambda: a + b}[name]())
    elif name in ("addu", "or") and all(v["kind"] == "constant-bits" for v in values):
        a, b = [v["value"] for v in values]
        result = constant(a + b if name == "addu" else a | b)
    if row["family"] == "unknown" or row["control"] in ("call", "conditional-call"):
        state.clear()
    if dest:
        state.pop(dest, None)
        writes_pair = (name.endswith(".d") or name == "ldc1" or
                       name.startswith(("cvt.d.", "cvt.l.", "round.l.", "trunc.l.", "ceil.l.", "floor.l.")))
        if dest.startswith("f") and dest[1:].isdigit() and writes_pair:
            state.pop("f%d" % (int(dest[1:]) + 1), None)
        if result is not None:
            origins = {row["offset"]}
            for value in values:
                origins.update(value.get("definition_offsets", []))
            state[dest] = {**result, "definition_offsets": sorted(origins)}
    if row["family"] in ("multiply", "divide") and row["opcode"] in SPECIAL.values():
        state.pop("hi", None)
        state.pop("lo", None)
    state["r0"] = constant(0)
    return state


def regional_paths(rows, start, end, relocated=(), max_paths=128, max_steps=4096):
    """Enumerate acyclic region traversals; a loop or unknown edge remains incomplete."""
    by_offset = {r["offset"]: r for r in rows}
    paths, limits = [], set()
    pending = [(start, None, {}, [], [], [], frozenset())]
    while pending and len(paths) < max_paths:
        pc, continuation, state, executed, decisions, multiplies, seen = pending.pop()
        key = (pc, continuation)
        if not start <= pc < end:
            paths.append({"exit": pc, "complete": True, "decisions": decisions,
                          "operation_counts": counts([by_offset[x] for x in executed]),
                          "multiplies": multiplies})
            continue
        if key in seen or len(executed) >= max_steps:
            reason = "cycle" if key in seen else "step-limit"
            limits.add(reason)
            paths.append({"exit": pc, "complete": False, "reason": reason,
                          "decisions": decisions, "multiplies": multiplies})
            continue
        row = by_offset[pc]
        if continuation is not None and row["control"]:
            limits.add("control-in-delay-slot")
            paths.append({"exit": pc, "complete": False, "reason": "control-in-delay-slot",
                          "decisions": decisions, "multiplies": multiplies})
            continue
        seen = seen | {key}
        executed = executed + [pc]
        state.setdefault("r0", constant(0))
        if row["family"] == "multiply":
            inputs = [operand(r, state) for r in row["sources"]]
            if row["multiply_kind"] in ("float64", "integer64"):
                inputs = [{"kind": "unresolved", "reason": "wide-operand-not-propagated",
                           "register": r} for r in row["sources"]]
            multiplies = multiplies + [{"offset": pc, "kind": row["multiply_kind"],
                                        "operands": inputs}]
        state = apply(row, state, pc in relocated)
        def enqueue(dest, after=None, decision=None):
            pending.append((dest, after, state, executed,
                            decisions + ([decision] if decision else []), multiplies, seen))
        if continuation is not None:
            if by_offset.get(pc - 4, {}).get("control") == "call":
                state = {"r0": constant(0)}
            # -1 is an authenticated return; -2 denotes an unresolved jump.
            if continuation == -2:
                limits.add("unresolved-control-target")
                paths.append({"exit": None, "complete": False,
                              "reason": "unresolved-control-target", "decisions": decisions,
                              "multiplies": multiplies})
            else:
                enqueue(continuation)
        elif row["control"] in ("branch", "jump", "call", "return", "indirect", "conditional-call") and pc + 4 not in by_offset:
            limits.add("missing-owned-delay-slot")
            paths.append({"exit": pc, "complete": False, "reason": "missing-owned-delay-slot",
                          "decisions": decisions, "multiplies": multiplies})
        elif row["control"] == "branch":
            target = row.get("target")
            if target is None:
                limits.add("unresolved-control-target")
                paths.append({"exit": None, "complete": False,
                              "reason": "unresolved-control-target", "decisions": decisions,
                              "multiplies": multiplies})
                continue
            enqueue(pc + 4, target, {"branch": pc, "outcome": "taken"})
            if not row["unconditional"]:
                enqueue(pc + 8 if row["likely"] else pc + 4,
                        None if row["likely"] else pc + 8,
                        {"branch": pc, "outcome": "not-taken"})
        elif row["control"] in ("jump", "call", "return", "indirect"):
            after = {"call": pc + 8, "return": -1, "indirect": -2,
                     "jump": row.get("target") if row.get("target") is not None else -2}[row["control"]]
            enqueue(pc + 4, after)
        elif row["control"] in ("trap", "conditional-call") or row["family"] == "unknown":
            limits.add("unsupported-execution-semantics")
            paths.append({"exit": pc, "complete": False,
                          "reason": "unsupported-execution-semantics", "decisions": decisions,
                          "multiplies": multiplies})
        else:
            enqueue(pc + 4)
    if pending:
        limits.add("path-limit")
    complete = not limits and all(p["complete"] for p in paths)
    totals = [len(p["multiplies"]) for p in paths]
    return {"complete": complete, "limits": sorted(limits), "paths": paths,
            "multiply_count_range": [min(totals), max(totals)] if complete and totals else None,
            "condition": "per traversal from the selected entry; calls assumed to return; branch outcomes are not solved"}


def analyze(words, *, start=0, end=None, relocations=None, control_targets=None,
            max_paths=128, max_steps=4096):
    end = len(words) * 4 if end is None else end
    if start % 4 or end % 4 or not 0 <= start < end <= len(words) * 4:
        raise ValueError("window must be a nonempty aligned range inside the owned function")
    if max_paths < 1 or max_steps < 1:
        raise ValueError("path and step limits must be positive")
    relocations, control_targets = relocations or {}, control_targets or {}
    rows = [decode(word, i * 4) for i, word in enumerate(words)]
    for row in rows:
        if row["offset"] in relocations and row["control"] in ("branch", "jump"):
            row["target"] = control_targets.get(row["offset"])
        elif row["control"] == "jump":
            row["target"] = control_targets.get(row["offset"], row["target"])
    window = [r for r in rows if start <= r["offset"] < end]
    delays, alternatives = [], []
    for row in rows:
        pc = row["offset"]
        if row["control"] in ("branch", "jump", "call", "return", "indirect", "conditional-call"):
            if start <= pc + 4 < end:
                delays.append({"offset": pc + 4, "owner": pc,
                               "execution": "taken-only" if row.get("likely") else "both-outcomes-or-unconditional"})
            if (row.get("likely") and row["control"] == "branch"
                    and not row.get("unconditional") and row.get("target") is not None
                    and start <= pc < end - 8 and row.get("target") != pc + 8
                    and words[(pc + 4) // 4] == words[(pc + 8) // 4]
                    and rows[(pc + 4) // 4]["family"] == "multiply"):
                alternatives.append({"branch": pc, "taken_site": pc + 4,
                    "not_taken_site": pc + 8, "same_incoming_register_operands": True,
                    "scope": "one operation on each outgoing edge of this branch evaluation; not a whole-loop count"})
    frames = [-r["immediate"] for r in rows[:16] if r["opcode"] in ("addiu", "daddiu")
              and r["destination"] == "r29" and r["sources"] == ["r29"] and r["immediate"] < 0]
    execution = regional_paths(rows, start, end, relocations, max_paths, max_steps)
    if any(slot["offset"] == start and slot["owner"] < start for slot in delays):
        execution["complete"] = False
        execution["multiply_count_range"] = None
        execution["limits"] = sorted(set(execution["limits"]) | {"entry-is-delay-slot-without-branch-context"})
    return {"owned_size": len(words) * 4, "window": [start, end],
            "frame_size": frames[0] if len(frames) == 1 else None,
            "operation_counts": counts(window),
            "opcode_counts": dict(sorted(collections.Counter(r["opcode"] for r in window).items())),
            "multiply_sites": [{"offset": r["offset"], "kind": r["multiply_kind"],
                                "register_operands": r["sources"]} for r in window if r["family"] == "multiply"],
            "delay_slots": delays, "branch_likely_alternatives": alternatives,
            "branch_conditions": [{"offset": row["offset"], "opcode": row["opcode"],
                "register_operands": row["sources"], "taken_target": row.get("target"),
                "likely": row.get("likely", False),
                "predicate_status": "unconditional" if row.get("unconditional") else "not-solved"}
                for row in rows if row["control"] == "branch"
                and (start <= row["offset"] < end or start <= row["offset"] + 4 < end)],
            "execution": execution,
            "operand_policy": "only local immediate constants resolved; memory, entry values and relocation names remain unresolved"}


def compare(target_words, candidate_words, *, target_window=None, candidate_window=None,
            target_relocations=None, candidate_relocations=None,
            target_control_targets=None, candidate_control_targets=None,
            max_paths=128, max_steps=4096):
    import align_symbol as al
    import nm_ranking as nr
    target_relocations = target_relocations or {}
    candidate_relocations = candidate_relocations or {}
    tw = target_window or (0, len(target_words) * 4)
    cw = candidate_window or (0, len(candidate_words) * 4)
    target = analyze(target_words, start=tw[0], end=tw[1], relocations=target_relocations,
                     control_targets=target_control_targets, max_paths=max_paths, max_steps=max_steps)
    candidate = analyze(candidate_words, start=cw[0], end=cw[1], relocations=candidate_relocations,
                        control_targets=candidate_control_targets, max_paths=max_paths, max_steps=max_steps)
    def shifted(relocs, window):
        return {k - window[0]: v for k, v in relocs.items() if window[0] <= k < window[1]}
    aligned = al.align(nr.WordStreams(candidate_words[cw[0] // 4:cw[1] // 4],
        target_words[tw[0] // 4:tw[1] // 4], shifted(candidate_relocations, cw),
        shifted(target_relocations, tw), cw[1] - cw[0], tw[1] - tw[0]))
    tr, cr = target["execution"]["multiply_count_range"], candidate["execution"]["multiply_count_range"]
    relation = "unresolved-path-counts"
    if tr is not None and cr is not None:
        relation = ("candidate-has-more-on-every-enumerated-traversal" if cr[0] > tr[1] else
                    "target-has-more-on-every-enumerated-traversal" if tr[0] > cr[1] else
                    "same-count-range-not-equivalence" if tr == cr else "overlapping-count-ranges")
    def signatures(report):
        output = []
        for path in report["execution"]["paths"]:
            for multiply in path["multiplies"]:
                if any(v["kind"] != "constant-bits" for v in multiply["operands"]):
                    return None
            output.append([(m["kind"], [v["value"] for v in m["operands"]]) for m in path["multiplies"]])
        return output if report["execution"]["complete"] else None
    ts, cs = signatures(target), signatures(candidate)
    return {"schema_version": 1, "diagnostic_only": True, "target": target, "candidate": candidate,
            "size_delta": len(candidate_words) * 4 - len(target_words) * 4,
            "frame_delta": candidate["frame_size"] - target["frame_size"]
                if candidate["frame_size"] is not None and target["frame_size"] is not None else None,
            "alignment": aligned, "multiply_execution_relation": relation,
            "multiply_operand_relation": "unresolved" if ts is None or cs is None else
                ("same-constant-input-sequences-not-equivalence" if ts == cs else "different-constant-input-sequences"),
            "warning": "Alignment gaps are not missing semantic operations. Path ranges overapproximate feasible branch outcomes; no equivalence or matching verdict."}


def load_function(path, symbol):
    elf = rs.Elf(Path(path))
    index, _ = elf.section(".text")
    matches = [(v, size) for name, v, size, info, sec in elf.symbols()
               if name == symbol and sec == index and info & 15 == rs.STT_FUNC and size > 0]
    if len(matches) != 1:
        raise ValueError("expected one positive-sized .text function: " + symbol)
    start, size = matches[0]
    raw = elf.section_bytes(".text")
    if start % 4 or size % 4 or start + size > len(raw):
        raise ValueError("function extent escapes aligned .text")
    words = list(struct.unpack(">%dI" % (size // 4), raw[start:start + size]))
    relocs, destinations = {}, {}
    symbols = elf.symbols()
    names = {rs.R_MIPS_26: "R_MIPS_26", rs.R_MIPS_HI16: "R_MIPS_HI16",
             rs.R_MIPS_LO16: "R_MIPS_LO16", rs.R_MIPS_PC16: "R_MIPS_PC16"}
    for _, offset, kind, symbol_index in elf.relocations():
        if not start <= offset < start + size:
            continue
        if symbol_index >= len(symbols):
            raise ValueError("relocation symbol index escapes the symbol table")
        name, value, _, _, section = symbols[symbol_index]
        local = offset - start
        if local % 4:
            raise ValueError("relocation is not on an instruction boundary")
        if local in relocs:
            raise ValueError("duplicate relocation at one instruction")
        relocs[local] = (names.get(kind, "R_MIPS_%d" % kind), name)
        if section == index and kind in (rs.R_MIPS_PC16, rs.R_MIPS_26):
            word = words[local // 4]
            dest = value + (rs.sext16(word & 65535) * 4 + 4 if kind == rs.R_MIPS_PC16 else (word & 0x03FFFFFF) * 4)
            if start <= dest < start + size and dest % 4 == 0:
                destinations[local] = dest - start
    for local, word in enumerate(words):
        offset = local * 4
        if word >> 26 == 2 and offset not in relocs:
            destinations[offset] = ((word & 0x03FFFFFF) << 2) - start
    return words, relocs, destinations


def window(value):
    try:
        first, last = value.split(":")
        return int(first, 0), int(last, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("use START:END with function-relative byte offsets") from exc


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol")
    parser.add_argument("--target-object", type=Path, required=True)
    parser.add_argument("--candidate-object", type=Path, required=True)
    parser.add_argument("--target-window", type=window)
    parser.add_argument("--candidate-window", type=window)
    parser.add_argument("--max-paths", type=int, default=128)
    parser.add_argument("--max-steps", type=int, default=4096)
    args = parser.parse_args(argv)
    try:
        target, tr, tc = load_function(args.target_object, args.symbol)
        candidate, cr, cc = load_function(args.candidate_object, args.symbol)
        report = compare(target, candidate, target_window=args.target_window,
                         candidate_window=args.candidate_window, target_relocations=tr,
                         candidate_relocations=cr, target_control_targets=tc,
                         candidate_control_targets=cc, max_paths=args.max_paths, max_steps=args.max_steps)
    except (OSError, ValueError, rs.SurfaceComparisonError) as error:
        parser.error(str(error))
    report["inputs"] = {"symbol": args.symbol,
        "target_object_sha256": hashlib.sha256(args.target_object.read_bytes()).hexdigest(),
        "candidate_object_sha256": hashlib.sha256(args.candidate_object.read_bytes()).hexdigest(),
        "authority": "selected ELF symbol extents; caller must authenticate these objects against the owned target"}
    print(json.dumps(report, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
