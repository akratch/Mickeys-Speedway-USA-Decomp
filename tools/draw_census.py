#!/usr/bin/env python3
"""Census a function's ugen draws and emissions, per SOURCE LINE.

    tools/draw_census.py <symbol> [--proc N]   (default: resolved from the symbol) [--save out.json]
    tools/draw_census.py --compare before.json after.json

WHY THIS EXISTS

`tools/web_footprint.py` made the register-COLOUR axis a bounded measurement:
every legal force, in minutes, with the winners packed by blast radius. That
closed the colour question on dozens of functions, and then left a wall. The
functions it closed report blockers that colour cannot touch:

    "pre-colour web creation order"
    "the mask-table-scale emission-order blocker"
    "ring release-order question remains"
    "the branch-delay decline is an assembler decision"

Those are ugen and as1 decisions, not p1 colouring, and the instrument for them
already exists -- `DKWB_UGEN_SCHED=1 DKWB_UGEN_TRACE=1` emits a freelist and
emission trace in which **every row carries its source line**. The brief calls
it the most productive instrument of the campaign. Nothing wrapped it, so every
lane that wanted it re-derived the invocation, the ordinal and the parsing by
hand.

WHAT IT READS

    DKWB-FREELIST ALLOC_GP_RESULT proc=0 reg=14 emitted=23 line=133
    DKWB-FREELIST FREE|ADD|REMOVE|MOVE_END proc=0 reg=.. emitted=.. line=..
    DKWB-EMIT-V1 proc=0 block=0 emit=3 op=42 line=125 buffer=fwd fn=f_emit_rab

A **draw** is an `ALLOC_GP_RESULT` or `ALLOC_FP_RESULT`: ugen taking a register
off the free list. Because each row is line-stamped, "the target spends one more
draw at this statement" stops being an inference and becomes arithmetic -- which
is the whole point, and why L149 ("count draws, not registers") is stated the
way it is.

HOW TO USE IT ON A RESIDUAL

Save a profile, change one thing in the source, save another, and `--compare`.
The diff names the lines whose draw count or emission count moved, so a source
edit is judged by what it did to the schedule rather than by a score that mixes
every cause together. A line whose draws are unchanged did not move the
allocator, however much the score moved.

It does NOT read the target. It reads what OUR source makes the compiler do, at
line granularity; the target side still comes from `residual_map.py`.
"""
from __future__ import annotations

import argparse
import collections
import json
import os
import pathlib
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import force_lattice as fl  # noqa: E402

FIELD_RE = re.compile(r"(\w+)=([^\s]+)")
DRAW_OPS = ("ALLOC_GP_RESULT", "ALLOC_FP_RESULT")


def parse_trace(text: str, proc: int) -> dict:
    """Draws, frees and emissions for one procedure, keyed by source line."""
    draws: dict[int, list[dict]] = collections.defaultdict(list)
    frees: dict[int, int] = collections.Counter()
    emits: dict[int, list[dict]] = collections.defaultdict(list)
    order: list[dict] = []
    for raw in text.splitlines():
        if not raw.startswith(("DKWB-FREELIST", "DKWB-EMIT-V1")):
            continue
        parts = raw.split()
        fields = dict(FIELD_RE.findall(raw))
        if fields.get("proc") != str(proc):
            continue
        line = int(fields.get("line", -1))
        if raw.startswith("DKWB-EMIT-V1"):
            emits[line].append(fields)
            continue
        op = parts[1]
        if op in DRAW_OPS:
            row = {"op": op, "line": line, "reg": fields.get("reg"),
                   "emitted": int(fields.get("emitted", -1))}
            draws[line].append(row)
            order.append(row)
        elif op == "FREE":
            frees[line] += 1
    return {"draws": draws, "frees": frees, "emits": emits, "order": order}


PROC_BEGIN_RE = re.compile(r"^DKWB-PROC\s+BEGIN\s+proc=(\d+)\s*$", re.M)


def ordinal_from_symbols(symbol: str, funcs: list[tuple[str, int]], nprocs: int) -> int:
    """Procedure ordinal of `symbol` from the object's FUNC symbols.

    ugen emits procedures in ordinal order, so the Nth distinct function
    address is procedure N. That is only believed when the trace announces
    exactly as many procedures as the object has function ranges; otherwise
    the mapping is unproven and we refuse rather than census another function.
    """
    ranges = sorted({value for _, value in funcs})
    if len(ranges) != nprocs:
        raise SystemExit(
            f"draw_census: cannot resolve {symbol!r} to a procedure: the object has "
            f"{len(ranges)} function ranges but the trace has {nprocs} procedures. "
            "Pass --proc N (tools/web_footprint.py --list-procs prints the index).")
    values = [v for n, v in funcs if n == symbol]
    if not values:
        raise SystemExit(f"draw_census: {symbol!r} is not a function in the compiled object; "
                         "pass --proc N")
    return ranges.index(values[0])


def resolve_proc(symbol: str, obj: pathlib.Path, text: str) -> int:
    import allocator_trace_receipt as atr
    try:
        syms = atr.read_object_symbols(obj)
    except atr.ReceiptError as error:
        raise SystemExit(f"draw_census: cannot resolve {symbol!r} to a procedure ({error}); "
                         "pass --proc N")
    nprocs = len(set(PROC_BEGIN_RE.findall(text)))
    return ordinal_from_symbols(symbol, [(x.name, x.value) for x in syms], nprocs)


def profile(symbol: str, proc: int | None, keep: pathlib.Path | None = None) -> dict:
    """Compile once with the ugen trace on and reduce it to a per-line census."""
    command = fl.replace_compiler(fl.compile_command(symbol),
                                  fl.INSTRUMENTED / "cc")
    work = keep or pathlib.Path(tempfile.mkdtemp())
    work.mkdir(parents=True, exist_ok=True)
    command[command.index("-o") + 1] = str(work / "candidate.o")
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(("CDX_", "DKWB_"))}
    env.update(DKWB_UGEN_SCHED="1", DKWB_UGEN_TRACE="1")
    result = subprocess.run(command, env=env, capture_output=True, text=True,
                            cwd=fl.ROOT, timeout=900)
    text = result.stdout + result.stderr
    if keep:
        (work / "ugen.log").write_text(text)
    if result.returncode:
        raise SystemExit(f"draw_census: compile failed (exit {result.returncode})")
    if proc is None:
        proc = resolve_proc(symbol, work / "candidate.o", text)
        print(f"draw_census: {symbol} is procedure {proc}", file=sys.stderr)
    parsed = parse_trace(text, proc)
    if not parsed["order"] and not parsed["emits"]:
        raise SystemExit(
            f"draw_census: no proc={proc} rows in the trace. The ordinal is the "
            "Ucode procedure order, not the source order; "
            "tools/web_footprint.py --list-procs prints the index.")
    return {
        "symbol": symbol, "proc": proc,
        "draws_by_line": {str(k): len(v) for k, v in parsed["draws"].items()},
        "frees_by_line": {str(k): v for k, v in parsed["frees"].items()},
        "emits_by_line": {str(k): len(v) for k, v in parsed["emits"].items()},
        "draw_order": [r["reg"] for r in parsed["order"]],
        "total_draws": len(parsed["order"]),
        "total_emits": sum(len(v) for v in parsed["emits"].values()),
    }


def _int_keys(mapping: dict) -> dict[int, int]:
    return {int(k): v for k, v in (mapping or {}).items()}


def render(data: dict) -> str:
    draws, emits = _int_keys(data["draws_by_line"]), _int_keys(data["emits_by_line"])
    frees = _int_keys(data["frees_by_line"])
    out = [f"{data['symbol']}  proc={data['proc']}",
           f"  {data['total_draws']} draws, {data['total_emits']} emissions",
           f"  draw order: {' '.join(data['draw_order'])}",
           "",
           "  line   draws  frees  emits"]
    for line in sorted(set(draws) | set(emits) | set(frees)):
        out.append(f"  {line:>5} {draws.get(line,0):>7} {frees.get(line,0):>6} "
                   f"{emits.get(line,0):>6}")
    return "\n".join(out)


def compare(before: dict, after: dict) -> str:
    """Per-line movement between two profiles of the same function.

    A source edit that leaves every line's draw count unchanged did not move the
    allocator, whatever it did to the score -- and a score can move for reasons
    that have nothing to do with the edit.
    """
    out = [f"{before['symbol']}: draws {before['total_draws']} -> "
           f"{after['total_draws']}, emissions {before['total_emits']} -> "
           f"{after['total_emits']}", ""]
    if before["draw_order"] != after["draw_order"]:
        out.append("  draw order CHANGED")
        out.append(f"    before: {' '.join(before['draw_order'])}")
        out.append(f"    after:  {' '.join(after['draw_order'])}")
    else:
        out.append("  draw order unchanged")
    out.append("")
    out.append("  line   draws       emits")
    moved = 0
    b_d, a_d = _int_keys(before["draws_by_line"]), _int_keys(after["draws_by_line"])
    b_e, a_e = _int_keys(before["emits_by_line"]), _int_keys(after["emits_by_line"])
    for line in sorted(set(b_d) | set(a_d) | set(b_e) | set(a_e)):
        dd = a_d.get(line, 0) - b_d.get(line, 0)
        de = a_e.get(line, 0) - b_e.get(line, 0)
        if dd or de:
            moved += 1
            out.append(f"  {line:>5} {b_d.get(line,0):>3}->{a_d.get(line,0):<3}"
                       f"{dd:+3d}   {b_e.get(line,0):>3}->{a_e.get(line,0):<3}{de:+3d}")
    if not moved:
        out.append("  no line moved -- this edit did not change the schedule")
    return "\n".join(out)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Census a function's ugen draws and emissions per source line.")
    parser.add_argument("symbol", nargs="?")
    parser.add_argument("--proc", type=int, default=None,
                        help="procedure ordinal (default: resolved from the symbol name)")
    parser.add_argument("--save", type=pathlib.Path, default=None)
    parser.add_argument("--keep", type=pathlib.Path, default=None,
                        help="retain the object and the raw trace here")
    parser.add_argument("--compare", nargs=2, type=pathlib.Path, default=None,
                        metavar=("BEFORE", "AFTER"))
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    if args.compare:
        before, after = (json.loads(p.read_text()) for p in args.compare)
        if before["symbol"] != after["symbol"]:
            raise SystemExit("draw_census: --compare needs two profiles of the "
                             f"same function ({before['symbol']} vs {after['symbol']})")
        print(compare(before, after))
        return 0
    if not args.symbol:
        parser.error("a symbol is required unless --compare is used")

    data = profile(args.symbol, args.proc, args.keep)
    if args.save:
        args.save.write_text(json.dumps(data, indent=2) + "\n")
    print(json.dumps(data, indent=2) if args.json else render(data))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
