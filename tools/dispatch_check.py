#!/usr/bin/env python3
"""Check a planned lane fan-out, and emit the facts each brief should carry.

    tools/dispatch_check.py <lane>=<sym>[,<sym>...] [<lane>=<sym>...]
    tools/dispatch_check.py --plan plan.json

Two jobs, both aimed at waste that has actually happened rather than waste
that might.

**Refuse an unsound plan.** A symbol assigned to two lanes gets worked twice
and merged twice, and the second merge is the one that conflicts. A symbol that
is no longer queued gets a lane spent re-deriving a match that already landed.
Both occurred while planning campaign 2; both were caught by hand, which is not
a control.

**Emit the measured facts.** Every brief so far was hand-assembled from the
ranking and the handoff shards, and that is where the errors came from: two
lanes were told to spend their first phase partitioning relocation artifacts
out of a residual, when the artifact count was already stored and turned out to
be 0.4% of the number. A brief that carries measured values does not invite a
lane to re-derive them, and a lane that is handed `masked=1834, artifact=7`
does not spend a phase discovering `7`.

Exit status is 0 when the plan is sound, 1 when it is not.
"""
from __future__ import annotations

import argparse
import collections
import json
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

REPO = pathlib.Path(__file__).resolve().parents[1]
RANKING = REPO / "config" / "nonmatching-ranking.us.json"


def check_doc_paths(paths: list[str]) -> list[str]:
    """Refuse a brief that cites a document which does not exist.

    Three lanes in one campaign were sent to
    `docs/lastmile-block-budget-globals.md`; the file is
    `docs/lastmile-block-budget.md`. The name was copied out of a lane report
    and never opened. A brief that names a missing file spends the lane's
    first minutes proving the coordinator wrong, and teaches it to distrust
    the rest of the brief -- which is the part that matters.
    """
    problems = []
    for raw in paths:
        if (REPO / raw).is_file():
            continue
        stem = pathlib.Path(raw).name.split("-")[0]
        near = sorted(
            q.relative_to(REPO).as_posix()
            for q in (REPO / "docs").rglob("*.md")
            if q.name.split("-")[0] == stem
        )[:4]
        hint = f" Did you mean: {', '.join(near)}?" if near else ""
        problems.append(f"brief cites a missing document: {raw}.{hint}")
    return problems


SHARD_DIR = REPO / "docs" / "matching-triage-handoffs"


def prior_work(symbol: str) -> str:
    """State how much has already been tried, in bytes, never as a dash.

    A dispatch of mine wrote "--" in this column for four targets and the lane
    read it as "no shard". All four carried six to sixteen kilobytes of
    recorded attempts, and the lane spent cycles rediscovering them. A second
    dispatch repeated it. The number is cheap to look up and impossible to
    misread, so the tool looks it up.
    """
    path = SHARD_DIR / f"{symbol}.md"
    if not path.is_file():
        return "prior work: no shard -- genuinely untouched"
    size = path.stat().st_size
    sections = sum(
        1 for line in path.read_text(encoding="utf-8", errors="replace").splitlines()
        if line.startswith("## ")
    )
    weight = ("thin" if size < 3000 else
              "moderate" if size < 10000 else
              "HEAVY -- read it before forming a hypothesis")
    return (f"prior work: shard {size:,} B, {sections} recorded pass(es) "
            f"({weight})")


def queued_rows() -> dict[str, dict]:
    document = json.loads(RANKING.read_text(encoding="utf-8"))
    return {row["name"]: row for row in document["functions"]}


def closure_facts() -> dict[str, dict]:
    """Closure date and newly-applicable laws, from the re-open lister."""
    try:
        import reopen_candidates as rc
        return {row["symbol"]: row for row in rc.candidates(since="2026-09-10")}
    except Exception:
        return {}


def parse_plan(args: list[str]) -> dict[str, list[str]]:
    plan: dict[str, list[str]] = {}
    for item in args:
        if "=" not in item:
            raise ValueError(f"expected <lane>=<symbols>, got {item!r}")
        lane, symbols = item.split("=", 1)
        plan.setdefault(lane.strip(), []).extend(
            s.strip() for s in symbols.split(",") if s.strip())
    return plan


def assignability(symbols: list[str]) -> dict[str, str]:
    """Each symbol's lane_status assignment state.

    ONLY `base-only` may be dispatched; every other state is fail-closed so
    stale evidence cannot become duplicate matching work. The state is invisible
    in the ranking, and it MOVES: merging a lane that edited a handoff shifts
    that symbol's handoff commit and invalidates its reopen pin, so a target
    assignable before a merge is often shut immediately after one.

    That window cost three dispatched lanes on a single function. Each spun up,
    found the gate closed, reported "blocked by the assignment gate" and did no
    work -- roughly three hours of a strong model, spent on nothing, because the
    plan was made before the merge and executed after it.

    Returns {} if lane_status cannot be consulted; a degraded check still
    catches duplicates and unqueued symbols, and is better than refusing to run.
    """
    import sys as _sys
    _sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
    try:
        import lane_status as ls
    except ImportError:
        return {}
    try:
        base = "campaign/unchain"
        ctx = ls.AssignmentContext.build(base, symbols, jobs=4)
        return {s: ctx.classify(base, s).state for s in symbols}
    except Exception:
        return {}


def check(plan: dict[str, list[str]]) -> tuple[list[str], list[str]]:
    rows, closures = queued_rows(), closure_facts()
    problems, notes = [], []

    # Distinct lanes, not occurrences: a symbol listed twice inside one lane is
    # sloppy input, not a planning fault, and refusing on it would make the
    # message ("assigned to 2 lanes") untrue. A guard that says something false
    # gets ignored, which costs more than the case it was catching.
    owner: dict[str, set[str]] = collections.defaultdict(set)
    for lane, symbols in plan.items():
        for symbol in symbols:
            owner[symbol].add(lane)

    for symbol, lanes in sorted(owner.items()):
        if len(lanes) > 1:
            problems.append(
                f"{symbol} is assigned to {len(lanes)} lanes ({', '.join(sorted(lanes))}). "
                f"It will be worked twice and the second merge will conflict.")
        if symbol not in rows:
            problems.append(
                f"{symbol} is not in the NON_MATCHING queue: already matched, or misspelled. "
                f"A lane pointed at it re-derives a landed match.")

    # Two lanes editing one translation unit is the conflict that actually
    # costs an integration, and the symbol check above does not see it: three
    # overlay 8 functions with distinct names live in one overlay_008.c, so a
    # plan that splits them across lanes passes every check here and then
    # conflicts on every merge. The ranking carries each symbol's source file,
    # so this is free to check and nothing else checks it.
    files: dict[str, dict[str, list[str]]] = collections.defaultdict(
        lambda: collections.defaultdict(list))
    for lane, symbols in plan.items():
        for symbol in symbols:
            row = rows.get(symbol) or {}
            path = row.get("file") or row.get("rel_c_file")
            if path:
                files[path][lane].append(symbol)
    for path, lanes in sorted(files.items()):
        if len(lanes) > 1:
            spread = "; ".join(f"{lane} has {', '.join(sorted(syms))}"
                               for lane, syms in sorted(lanes.items()))
            problems.append(
                f"{path} is edited by {len(lanes)} lanes ({spread}). One owner "
                f"per translation unit: a per-TU flag or a shared static is one "
                f"edit that moves every function in it, and two lanes editing "
                f"one file conflict on every merge.")

    states = assignability(sorted(owner))
    if not states:
        notes.append("\nNOTE lane_status unavailable -- assignability unchecked; "
                     "a lane may still arrive at a closed gate")
    for symbol in sorted(owner):
        state = states.get(symbol)
        if state is not None and state != "base-only":
            problems.append(
                f"{symbol} is {state}, not base-only. A lane dispatched at it will "
                f"arrive at a closed assignment gate and do no work. If the state is "
                f"reopen-authorization-stale, run tools/authorize_reopen.py "
                f"--refresh-stale first; merging a lane invalidates its own pin.")

    for lane, symbols in sorted(plan.items()):
        notes.append(f"\n=== {lane} ===")
        for symbol in symbols:
            row = rows.get(symbol)
            if row is None:
                notes.append(f"  {symbol}: NOT QUEUED")
                continue
            raw = row.get("differing_words")
            masked = row.get("relocation_masked_differing_words")
            artifact = (raw - masked) if (raw is not None and masked is not None) else None
            line = (f"  {symbol}: {row['size_bytes']} B, masked={masked}, "
                    f"artifact={artifact}, delta={row.get('size_delta')}, "
                    f"class={row.get('category')}")
            closure = closures.get(symbol)
            if closure:
                line += (f"\n      closure predates the current laws "
                         f"({closure['closed']}); newly applicable: "
                         f"{', '.join(law.split()[0] for law in closure['laws'])}")
            line += "\n      " + prior_work(symbol)
            notes.append(line)
    return problems, notes


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("assignments", nargs="*")
    parser.add_argument("--plan", type=pathlib.Path,
                        help="JSON object mapping lane name to a list of symbols")
    parser.add_argument(
        "--cites", action="append", default=[], metavar="PATH",
        help="a document the brief will cite; refused if it does not exist")
    args = parser.parse_args(argv)

    if args.plan:
        plan = {k: list(v) for k, v in json.loads(args.plan.read_text()).items()}
    else:
        try:
            plan = parse_plan(args.assignments)
        except ValueError as error:
            parser.error(str(error))
    if not plan:
        parser.error("no assignments given")

    problems, notes = check(plan)
    problems = check_doc_paths(args.cites) + problems
    print("\n".join(notes).lstrip("\n"))
    if problems:
        print("\nPLAN REFUSED:", file=sys.stderr)
        for problem in problems:
            print(f"  {problem}", file=sys.stderr)
        return 1
    print(f"\nplan sound: {sum(len(v) for v in plan.values())} symbol(s) "
          f"across {len(plan)} lane(s), no overlap, all queued, all assignable")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
