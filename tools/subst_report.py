#!/usr/bin/env python3
"""Forward-substitution / copy-propagation report for one function.

    tools/subst_report.py <symbol> [--source FILE] [--keep LINES] [--proc NAME] [--trace LOG]

Compiles the symbol's translation unit (the tree's own `NON_MATCHING=1` recipe,
the way `fast_score.py` does, with only the compiler swapped for the
instrumented one) with `DKWB_SUBST_TRACE` and `DKWB_SUBST_PROC` set, then prints
what the instrumented uopt decided for every local, grouped by variable:

    LOCAL   a load met a store of the same variable in its own block:
            substituted (the stored expression replaces the load) or kept
    GLOBAL  copy propagation into a later block, with every reaching
            definition and the test that rejected it (`cands`)
    STORE   whether the store itself survives, i.e. whether the variable keeps
            a stack home. A local keeps its home only if its STORE is kept.

Each row carries the source lines of the use and the definition and the reason
uopt gave; the reasons are tabulated in the trace's own README (the record
format is `DKWB_SUBST_TRACE`'s: `LOCAL proc= use_line= def_line= var= decision=
reason= ...`). Variables are `<memtype><offset>:<size>` (M local frame, P
parameter, R register, S static); the parenthesised name is read from the
definition lines' source (`name = ...`) and is a hint, not a proof.

`--keep 2658,2671` sets `DKWB_SUBST_KEEP`, which forces "kept" for definitions
on those lines. **That changes the code**: it is a pricing oracle, so the report
then describes the forced compile. `--source` reports a candidate C file in
place of the tracked TU; `--trace` reads a saved log instead of compiling.
The procedure filter is the symbol's own name (uopt's entry name); `--proc`
names another when a friendly-named candidate differs.
"""

from __future__ import annotations

import argparse
import collections
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import fast_score  # noqa: E402

KINDS = ("LOCAL", "GLOBAL", "STORE")
FIELD_RE = re.compile(r"(\w+)=(\[[^\]]*\]|\S*)")


def parse_trace(text: str, proc: str | None = None) -> list[dict]:
    """Records of the three kinds as dicts (`kind` plus every key=value field)."""
    rows = []
    for raw in text.splitlines():
        kind, _, rest = raw.partition(" ")
        if kind not in KINDS:
            continue
        row = {"kind": kind, **dict(FIELD_RE.findall(rest))}
        if proc is not None and row.get("proc") != proc:
            continue
        for key in ("use_line", "def_line"):
            if key in row:
                row[key] = int(row[key])
        rows.append(row)
    return rows


def parse_cands(value: str) -> list[tuple[int, str]]:
    """`[L2658:def_not_available,L2680:ok]` -> [(2658, 'def_not_available'), (2680, 'ok')]."""
    return [(int(m.group(1)), m.group(2)) for m in re.finditer(r"L(-?\d+):(\w+)", value)]


ASSIGN_RE = re.compile(r"^\s*(?:\w+\s*:\s*)?\**\s*(\w+)\s*(?:\[[^\]]*\]\s*)*(?:\.\w+|->\w+)*\s*[-+*/&|^]?=[^=]")


def name_hint(var_rows: list[dict], source: list[str]) -> str:
    """The identifier assigned on the variable's definition lines, if they agree."""
    names = set()
    for row in var_rows:
        d = row.get("def_line")
        if isinstance(d, int) and 0 < d <= len(source):
            m = ASSIGN_RE.match(source[d - 1])
            if m:
                names.add(m.group(1))
    return next(iter(names)) if len(names) == 1 else ""


def group_by_var(rows: list[dict]) -> dict[str, list[dict]]:
    groups: dict[str, list[dict]] = collections.defaultdict(list)
    for row in rows:
        groups[row.get("var", "?")].append(row)
    return dict(sorted(groups.items(), key=lambda kv: min(
        (r.get("def_line", 0) for r in kv[1] if isinstance(r.get("def_line"), int) and r["def_line"] > 0),
        default=0)))


def src(source: list[str], line) -> str:
    if isinstance(line, int) and 0 < line <= len(source):
        return source[line - 1].strip()[:70]
    return ""


def render(rows: list[dict], source: list[str]) -> str:
    out = []
    for var, items in group_by_var(rows).items():
        hint = name_hint(items, source)
        out.append(f"{var}" + (f"  ({hint})" if hint else ""))
        for r in sorted(items, key=lambda r: ({"LOCAL": 0, "GLOBAL": 1, "STORE": 2}[r["kind"]],
                                              r.get("use_line", r.get("def_line", 0)))):
            if r["kind"] == "STORE":
                out.append(f"  STORE  def L{r['def_line']:<5d} {r['decision']:<8s} {r['reason']}"
                           f"  [lval_av={r.get('lval_av')} store_av={r.get('store_av')} "
                           f"dead_after={r.get('dead_after')}]   {src(source, r['def_line'])}")
                continue
            d = r["def_line"]
            extra = (f"rhs={r.get('rhs')} uses={r.get('rhs_uses')}" if r["kind"] == "LOCAL"
                     else f"rhs={r.get('rhs')}")
            out.append(f"  {r['kind']:<6s} use L{r['use_line']:<5d} def {'L' + str(d) if d >= 0 else '-':<6s} "
                       f"{r['decision']:<11s} {r['reason']}  [{extra}]   {src(source, r['use_line'])}")
            if r["kind"] == "GLOBAL" and parse_cands(r.get("cands", "")):
                out.append("           cands: " + ", ".join(f"L{l}:{v}" for l, v in parse_cands(r["cands"])))
    tally = collections.Counter((r["kind"], r["decision"]) for r in rows)
    out.append("")
    out.append("summary: " + ", ".join(f"{k} {d} {n}" for (k, d), n in sorted(tally.items())))
    return "\n".join(out)


def compile_trace(symbol: str, candidate: Path | None, proc: str, keep: str | None,
                  work: Path, with_proc: bool = True) -> Path:
    source = fast_score.tracked_source_for(symbol)
    cand = candidate or (fast_score.ROOT / source)
    trace, obj = work / "subst.trace", work / "subst.o"
    args = fast_score.use_compiler(
        fast_score.rewrite_io(fast_score.configured_cc_args(source), cand, source, obj),
        fast_score.compiler_for(True))
    env = {k: v for k, v in os.environ.items() if not k.startswith("DKWB_SUBST_")}
    env["DKWB_SUBST_TRACE"] = str(trace)
    if with_proc:
        env["DKWB_SUBST_PROC"] = proc
    if keep:
        env["DKWB_SUBST_KEEP"] = keep
    r = subprocess.run(args, cwd=fast_score.ROOT, env=env, capture_output=True, text=True)
    if r.returncode:
        raise SystemExit("compile failed:\n" + r.stderr[-1500:])
    return trace


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--source", help="candidate C file in place of the tracked TU")
    ap.add_argument("--keep", help="DKWB_SUBST_KEEP lines (forces 'kept'; changes the code)")
    ap.add_argument("--proc", help="uopt entry name (default: the symbol)")
    ap.add_argument("--trace", help="read a saved DKWB_SUBST_TRACE log instead of compiling")
    ns = ap.parse_args(argv)

    candidate = Path(ns.source).resolve() if ns.source else None
    if candidate is not None and not candidate.is_file():
        raise SystemExit(f"no such --source: {candidate}")
    proc = ns.proc or ns.symbol
    if ns.trace:
        text = Path(ns.trace).read_text()
    else:
        with tempfile.TemporaryDirectory(prefix="subst-report-") as tmp:
            trace = compile_trace(ns.symbol, candidate, proc, ns.keep, Path(tmp))
            text = trace.read_text() if trace.is_file() else ""
    rows = parse_trace(text, proc)
    if not rows:
        names = sorted({r.get("proc") for r in parse_trace(text)})
        sys.stderr.write(
            f"subst_report: no records for procedure {proc!r}. "
            + (f"The log names: {', '.join(n for n in names if n)}; pass --proc."
               if names else "The log is empty: the instrumented uopt may predate "
                             "DKWB_SUBST_TRACE (see docs/LANE_BRIEF.md, Instruments).") + "\n")
        return 1
    tu = candidate or fast_score.ROOT / fast_score.tracked_source_for(ns.symbol)
    if ns.keep:
        print(f"DKWB_SUBST_KEEP={ns.keep}: the code below is the FORCED compile")
    print(render(rows, tu.read_text().splitlines()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
