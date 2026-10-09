#!/usr/bin/env python3
"""Read a `DKWB_CUT_OUT` log as one row per source line.

    tools/cut_log.py <log> [--source FILE] [--proc NAME] [--range LO-HI] [--lines A,B] [--json]

The instrumented uopt (`uopt-cut-varref.patch`) appends text records while it
builds basic blocks; the log is a firehose (tens of thousands of lines for one
TU, every compile appending), and nothing in it is per source line. This folds
it back to that.

Records read (everything else is ignored):

    OP c=<counter> lim=<limit> <w0> <w1> <w2> <w3>   every ucode record at getop;
                                                     w0 byte 0 is the opcode,
                                                     0x51 (Uloc) carries the
                                                     source line in w1, 0x19
                                                     opens a procedure
    AG site=<n> op=<hex> cur=<node>                  appendgraph: a block was
                                                     closed; op=51 is a cut at a
                                                     Uloc

Per Uloc row:

* `counter`  the statement counter `c` in the Uloc's own record, i.e. what the
  statement starts with (0 right after a cut).
* `pre`      the counter in the record before it, i.e. what the cut test saw.
  The natural test is `pre >= lim`.
* `cut`      an `AG ... op=51` record sits between the previous record and this
  Uloc (the block was closed at this statement); `site` is its `AG` site.
* `forced`   the log does NOT record the override, so this comes from the range
  and line list given (or `DKWB_CUT_RANGE`/`DKWB_CUT_LINES` in the environment):
  `yes` when the line is inside the range and the outcome differs from the
  natural test, `inert` when inside the range but the same, `-` otherwise.
  A `cut` that is neither natural nor forced comes from another source (the
  varref cut); `site` tells which.

A line repeated inside one procedure (several Uloc for one statement) is one row
with `n` records; the first record decides. With `--source` a procedure is named
by the function whose body holds its first line; without it, `p<ordinal>`.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

OP_RE = re.compile(r"OP c=(-?\d+) lim=(-?\d+) ([0-9a-f]{8}) ([0-9a-f]{8})")
AG_RE = re.compile(r"AG site=(\d+) op=([0-9a-f]+)")
ULOC, UENT = 0x51, 0x21


def function_spans(text: str) -> list[tuple[str, int, int]]:
    """(name, first line, last line) of each top-level function body in C text.

    Deliberately small: skips comments, strings and preprocessor lines, and
    names a body by the identifier before the first `(` of the declaration that
    precedes its `{`. Both arms of an `#ifdef` are read, which is right: the
    physical line numbers are what Uloc carries.
    """
    spans, depth, decl, start, name = [], 0, "", 0, ""
    in_comment = False
    for no, raw in enumerate(text.splitlines(), 1):
        line = raw
        if in_comment:
            if "*/" not in line:
                continue
            line, in_comment = line.split("*/", 1)[1], False
        line = re.sub(r"/\*.*?\*/", " ", line)
        if "/*" in line:
            line, in_comment = line.split("/*", 1)[0], True
        line = re.sub(r"//.*", "", line)
        line = re.sub(r'"(\\.|[^"\\])*"', '""', line)
        if depth == 0 and line.lstrip().startswith("#"):
            continue
        for ch in line:
            if ch == "{":
                if depth == 0:
                    m = re.search(r"(\w+)\s*\(", decl)
                    name, start = (m.group(1) if m else ""), no
                depth += 1
            elif ch == "}":
                depth = max(0, depth - 1)
                if depth == 0:
                    if name:
                        spans.append((name, start, no))
                    decl, name = "", ""
            elif depth == 0:
                if ch == ";":
                    decl = ""
                else:
                    decl += ch
        if depth == 0:
            decl += " "
    return spans


def parse(log_text: str) -> list[dict]:
    """Walk the log; one dict per Uloc with its procedure ordinal."""
    rows, proc, prev_c, ag, seen = [], -1, 0, None, {}
    for raw in log_text.splitlines():
        m = OP_RE.match(raw)
        if not m:
            a = AG_RE.match(raw)
            if a and int(a.group(2), 16) == ULOC:
                ag = int(a.group(1))
            continue
        c, lim = int(m.group(1)), int(m.group(2))
        op = int(m.group(3), 16) >> 24
        if op == UENT:
            proc += 1
        elif op == ULOC:
            line = int(m.group(4), 16)
            key = (proc, line)
            if key in seen:
                seen[key]["n"] += 1
            else:
                row = {"proc": proc, "line": line, "counter": c, "pre": prev_c, "lim": lim,
                       "natural": prev_c >= lim, "cut": ag is not None, "site": ag, "n": 1}
                seen[key] = row
                rows.append(row)
        # a Uloc's own AG precedes its OP record, so clear after every OP
        ag = None
        prev_c = c
    return rows


def parse_lines(spec: str | None) -> set[int]:
    return {int(x) for x in re.split(r"[,\s]+", spec.strip()) if x} if spec else set()


def parse_range(spec: str | None) -> tuple[int, int] | None:
    if not spec:
        return None
    m = re.fullmatch(r"\s*(\d+)-(\d+)\s*", spec)
    if not m:
        raise SystemExit(f"bad --range {spec!r}; expected LO-HI")
    return int(m.group(1)), int(m.group(2))


def annotate(rows: list[dict], spans: list[tuple[str, int, int]] | None,
             rng: tuple[int, int] | None, lines: set[int]) -> list[dict]:
    first: dict[int, int] = {}
    for r in rows:
        first.setdefault(r["proc"], r["line"])
    names = {}
    for p, ln in first.items():
        hit = next((s[0] for s in spans or [] if s[1] <= ln <= s[2]), None)
        names[p] = hit or f"p{p}"
    for r in rows:
        r["procedure"] = names[r["proc"]]
        inside = rng is not None and rng[0] <= r["line"] <= rng[1] and rng[1] >= rng[0]
        r["forced"] = ("yes" if r["cut"] != r["natural"] else "inert") if inside else "-"
        r["forced_to"] = (("cut" if r["line"] in lines else "keep") if inside else None)
    return rows


def render(rows: list[dict]) -> str:
    out = [f"{'procedure':28s} {'line':>6s} {'counter':>7s} {'pre':>4s} {'lim':>4s} "
           f"{'cut':>3s} {'forced':>6s} {'site':>7s} {'n':>2s}"]
    for r in rows:
        out.append(f"{r['procedure']:28s} {r['line']:6d} {r['counter']:7d} {r['pre']:4d} {r['lim']:4d} "
                   f"{'yes' if r['cut'] else 'no':>3s} {r['forced']:>6s} "
                   f"{(str(r['site']) if r['site'] else '-'):>7s} {r['n']:2d}")
    return "\n".join(out)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("log")
    ap.add_argument("--source", help="C file, to name procedures")
    ap.add_argument("--proc", help="only this procedure (name, or p<ordinal>)")
    ap.add_argument("--range", help="DKWB_CUT_RANGE the compile ran with (default: environment)")
    ap.add_argument("--lines", help="DKWB_CUT_LINES the compile ran with (default: environment)")
    ap.add_argument("--json", action="store_true")
    ns = ap.parse_args(argv)

    path = Path(ns.log)
    if not path.is_file():
        raise SystemExit(f"no such log: {path}")
    rows = parse(path.read_text(errors="replace"))
    if not rows:
        raise SystemExit(
            f"{path}: no Uloc (op 0x51) OP records. The log needs the instrumented uopt with "
            "uopt-cut-varref.patch and DKWB_CUT_OUT set; records without the line cannot be folded.")
    spans = function_spans(Path(ns.source).read_text()) if ns.source else None
    rng = parse_range(ns.range or os.environ.get("DKWB_CUT_RANGE"))
    lines = parse_lines(ns.lines if ns.lines is not None else os.environ.get("DKWB_CUT_LINES"))
    rows = annotate(rows, spans, rng, lines)
    if ns.proc:
        rows = [r for r in rows if r["procedure"] == ns.proc]
        if not rows:
            raise SystemExit(f"no procedure {ns.proc!r} in the log")
    print(json.dumps(rows, indent=1) if ns.json else render(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
