#!/usr/bin/env python3
"""Stack-slot request/owner trace of one function: uopt's slot allocator, per procedure.

    tools/slot_trace.py <symbol> [--proc N] [--source FILE] [--candidates] [--raw] [--keep DIR]

`~/Desktop/dev/ido-slottrace/cc` is the toolchain whose uopt prints
`DKWB-SLOT event=...` rows to **stderr** when `DKWB_UOPT_SLOT_TRACE` is set;
there is no file path to give it. This wrapper takes the compile command from
the build (never typed), swaps in that compiler, sets the variable, captures
stderr, resolves the symbol to its procedure ordinal (from the object's
function symbols, as `draw_census.py` does) and prints one line per slot
request in the order uopt made them:

    req 3  spill  owner 0x10062198 size 4 index 15 reserve 8 (spilltemps)
        -> slot 0x1005e588 index 0 offset -8 new  frame 4 -> 8   [2 candidates]

`owner` is the request's identity (the pseudo or home the slot is for), `path`
the reason it asked, `reuse`/`new` whether a free slot was shared, and `frame`
the running frame size before and after. `--candidates` adds the rejected
candidates with their reason; `--raw` prints the procedure's rows unparsed.
`--source FILE` traces a candidate C file in place of the tracked TU (a
relative path is taken from the caller's cwd). The trace is not an identity
gate: this toolchain's object is for reading, not for scoring.
"""
from __future__ import annotations

import argparse
import collections
import os
import pathlib
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

TOOLCHAIN = pathlib.Path(os.path.expanduser("~/Desktop/dev/ido-slottrace"))
ROW_RE = re.compile(r"^DKWB-SLOT\s+(.*)$")
FIELD_RE = re.compile(r"(\w+)=(\S+)")


def parse_rows(text: str) -> list[dict]:
    """Every `DKWB-SLOT` row as a dict of its fields (`event` included)."""
    rows = []
    for line in text.splitlines():
        m = ROW_RE.match(line.strip())
        if m:
            rows.append(dict(FIELD_RE.findall(m.group(1))))
    return rows


def procedure_count(rows: list[dict]) -> int:
    return len({r["proc"] for r in rows if r.get("event") == "procedure"})


def by_request(rows: list[dict], proc: int) -> list[dict]:
    """Join request, candidate and chosen rows of one procedure by request number."""
    p = str(proc)
    joined: dict[str, dict] = collections.OrderedDict()
    for r in rows:
        if r.get("proc") != p or "request" not in r:
            continue
        entry = joined.setdefault(r["request"], {"request": r["request"], "candidates": []})
        event = r["event"]
        if event == "request":
            entry["request_row"] = r
        elif event == "candidate":
            entry["candidates"].append(r)
        elif event == "chosen":
            entry["chosen"] = r
    return [e for e in joined.values() if "request_row" in e]


def render(symbol: str, proc: int, rows: list[dict], candidates: bool) -> str:
    out = [f"{symbol}  proc {proc}  slot requests in order"]
    entries = by_request(rows, proc)
    if not entries:
        out.append("  (no slot request in this procedure)")
    for e in entries:
        q = e["request_row"]
        out.append(f"req {q['request']}  {q.get('path', '?')}  owner {q.get('owner', '?')} "
                   f"size {q.get('size', '?')} index {q.get('index', '?')} "
                   f"reserve {q.get('reserve', '?')} ({q.get('caller', '?')})")
        c = e.get("chosen")
        if c:
            out.append(f"    -> slot {c['slot']} index {c['index']} offset {c['offset']} "
                       f"{'reuse' if c.get('reused') == '1' else 'new'}  frame {c['before']} -> "
                       f"{c['after']}   [{len(e['candidates'])} candidates]")
        else:
            out.append(f"    -> no slot chosen   [{len(e['candidates'])} candidates]")
        if candidates:
            for k in e["candidates"]:
                out.append(f"       candidate index {k.get('index')} size {k.get('size')} "
                           f"available {k.get('available')}: {k.get('reason')}")
    homes = [r for r in rows if r.get("proc") == str(proc) and r.get("event") == "home"]
    if homes:
        out.append(f"{len(homes)} home row(s); `--raw` prints them")
    return "\n".join(out) + "\n"


def compile_trace(symbol: str, work: pathlib.Path, candidate: pathlib.Path | None) -> tuple[str, pathlib.Path]:
    import force_lattice as fl
    command = fl.replace_compiler(fl.compile_command(symbol), TOOLCHAIN / "cc")
    if candidate is not None:
        command[-1] = str(candidate)
    obj = work / "slot.o"
    command[command.index("-o") + 1] = str(obj)
    env = {k: v for k, v in os.environ.items() if not k.startswith(("CDX_", "DKWB_"))}
    env["DKWB_UOPT_SLOT_TRACE"] = "1"
    result = subprocess.run(command, env=env, capture_output=True, text=True,
                            cwd=fl.ROOT, timeout=900)
    if result.returncode:
        raise SystemExit(f"slot_trace: compile failed (exit {result.returncode}):\n"
                         f"{result.stderr[-1500:]}")
    return result.stderr, obj


def resolve_proc(symbol: str, obj: pathlib.Path, nprocs: int) -> int:
    import allocator_trace_receipt as atr
    import draw_census as dc
    syms = atr.read_object_symbols(obj)
    return dc.ordinal_from_symbols(symbol, [(s.name, s.value) for s in syms], nprocs)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("symbol")
    parser.add_argument("--proc", type=int, help="procedure ordinal (default: from the symbol)")
    parser.add_argument("--source", type=pathlib.Path,
                        help="trace this candidate C file in place of the tracked TU")
    parser.add_argument("--candidates", action="store_true", help="print rejected candidates too")
    parser.add_argument("--raw", action="store_true", help="print the procedure's rows unparsed")
    parser.add_argument("--keep", type=pathlib.Path, help="keep the object and stderr here")
    args = parser.parse_args(argv)

    candidate = args.source.resolve() if args.source else None
    if candidate is not None and not candidate.is_file():
        raise SystemExit(f"slot_trace: no such --source: {candidate}")
    if not (TOOLCHAIN / "cc").is_file():
        raise SystemExit(f"slot_trace: no slot-trace toolchain at {TOOLCHAIN}")
    work = args.keep or pathlib.Path(tempfile.mkdtemp())
    work.mkdir(parents=True, exist_ok=True)
    text, obj = compile_trace(args.symbol, work, candidate)
    if args.keep:
        (work / "slot.stderr").write_text(text)
    rows = parse_rows(text)
    if not rows:
        print("slot_trace: no DKWB-SLOT rows; the toolchain's uopt predates the trace",
              file=sys.stderr)
        return 2
    proc = args.proc if args.proc is not None else resolve_proc(args.symbol, obj,
                                                              procedure_count(rows))
    if args.raw:
        keep = [r for r in rows if r.get("proc") == str(proc)]
        for r in keep:
            print("DKWB-SLOT " + " ".join(f"{k}={v}" for k, v in r.items()))
        return 0
    sys.stdout.write(render(args.symbol, proc, rows, args.candidates))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
