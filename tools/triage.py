#!/usr/bin/env python3
"""Answer "what should the next wave work on", in one command.

    tools/triage.py [--target-pct 65] [--top N] [--json] [--no-cache]

Built because the same scoping pass was hand-assembled four times in one day,
each time from the ranking plus ad-hoc `align_symbol`/`frame_census` runs. It
reports the three things that have actually decided where to send a lane:

**The gap.** How many bytes stand between the tree and a stated whole-program
percentage, and the cheapest set of functions that covers it — cheapest by
*words per byte*, because a 2,100-byte function at 149 words is a better buy
than a 200-byte function at 40.

**Clusters.** Functions of identical size in the same overlay are, in this
codebase, the same routine specialised N ways. That has been measured three
times: a diagnosis on one sibling transferred to the others by line range with
no per-function tuning, twice landing them at the same score or better. The
leverage is real and it is large -- at the time of writing, one lead per
cluster is 2,457 words against 7,088 for working every sibling separately. So
a cluster's cost is roughly its lead's word count, not its total.

**Bands.** Where the queue's words sit. Functions under about 20 words close at
a high rate; functions over 400 reduce but rarely close. Work that moves a
function *into* the low band is worth counting differently from work inside it.

**Delta groups.** Every section above is split three ways by the ranking's
`size_delta`: `delta-0`, `small-delta` (|delta| <= 12, one to three
instructions) and `big-delta`. They are different work. Colour landscapes,
L160 and the web laws act at delta 0 and cannot emit or delete an instruction;
a size-mismatch function needs Track B's insertion-pair method
(docs/NEXT_CAMPAIGN.md), and `dispatch_check.py` refuses to give one to a lane
not marked `track=B`. A route whose bytes are mostly small-delta is a Track B
plan, whatever its word count says.

**Milestones.** The default target is 65%. The report also prints the gap to
the next 5% milestone above the current figure, because the stated target and
the next milestone stop being the same number the moment a milestone is
crossed, and the previous default (60%) had been read as "gap 0" for a week.

This tool reads the ranking only -- no compiles -- so it is cheap to run before
every wave. For the cause split of a specific function use `align_symbol.py`,
and for its stack slots `frame_census.py`.
"""
from __future__ import annotations

import argparse
import collections
import json
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

ROOT = pathlib.Path(__file__).resolve().parents[1]
RANKING = ROOT / "config" / "nonmatching-ranking.us.json"
UNASSIGNABLE = ROOT / "config" / "unassignable-symbols.us.json"
# Whole-program text, the denominator README's headline percentage uses.
WHOLE_PROGRAM = 944344

# |size_delta| at or below this is one to three instructions: Track B's bridge.
SMALL_DELTA = 12
DELTA_GROUPS = ("delta-0", "small-delta", "big-delta")
MILESTONE_STEP = 5.0

BANDS = ((0, 20, "closes often"), (21, 60, "one or two decisions"),
         (61, 150, "a region or two"), (151, 400, "several regions"),
         (401, None, "reduces, rarely closes"))


def assignability(names: list[str], base: str = "campaign/unchain", *,
                  use_cache: bool = True) -> dict[str, str]:
    """Map each symbol to its lane_status assignment state.

    **Only `base-only` may be dispatched.** Every other state is fail-closed by
    design so stale evidence cannot become duplicate matching work, and the
    states are not visible in the ranking at all -- a function can sit in the
    queue at nine masked words and be unassignable because a reopen
    authorization's pins drifted.

    This is not a nicety. Measured once: 338 queued functions, of which only 68
    were assignable, and a wave was dispatched at nine targets of which three
    were. A lane that is handed a non-assignable target correctly refuses it and
    the slot is wasted. Returns {} if lane_status cannot be consulted, since a
    degraded triage is better than none; the reason goes to stderr.

    The base-derived part of each verdict is served from
    `lane_status.AssignmentCache` under ``build/cache/lane-assignment/``,
    keyed by content (base commit, source/shard/authorization blobs, classifier
    code), never by age. A cold run over the queue took about three minutes on
    2026-09-23; a warm one on an unchanged base takes seconds. Lane ownership
    is recomputed every run. ``use_cache=False`` (``--no-cache``) bypasses it.
    """
    try:
        import lane_status as ls
    except ImportError as error:
        print(f"triage: lane_status unavailable: {error}", file=sys.stderr)
        return {}
    try:
        cache = (ls.AssignmentCache(base, ROOT / ls.ASSIGNMENT_CACHE_DIR)
                 if use_cache else None)
        ctx = ls.AssignmentContext.build(base, names, jobs=8, cache=cache)
        states = {n: ctx.classify(base, n).state for n in names}
        ctx.save()
        if cache is not None:
            print(f"triage: assignment cache {cache.hits} hit(s), "
                  f"{cache.misses} miss(es)", file=sys.stderr)
        return states
    except Exception as error:  # noqa: BLE001 -- degraded triage, said aloud
        print(f"triage: lane_status failed, assignability unchecked: "
              f"{type(error).__name__}: {error}", file=sys.stderr)
        return {}


def unassignable() -> dict[str, dict]:
    """Symbols with a proof that no legal source reaches the target.

    Kept out of every route, cluster and band below. They are not plateaus
    waiting on a better idea, and a ratio-sorted list surfaces them first
    precisely because they are large and nearly closed -- which is what makes
    a lane spend itself re-deriving the proof.
    """
    if not UNASSIGNABLE.exists():
        return {}
    return json.loads(UNASSIGNABLE.read_text(encoding="utf-8"))["symbols"]


def load() -> list[dict]:
    rows = json.loads(RANKING.read_text(encoding="utf-8"))["functions"]
    barred = unassignable()
    return [r for r in rows if r["name"] not in barred]


def resolved_bytes() -> int:
    """What README's Progress block currently reports as resolved."""
    text = (ROOT / "README.md").read_text(encoding="utf-8", errors="replace")
    for line in text.splitlines():
        if "**Whole program**" in line:
            cells = [c.strip() for c in line.split("|")]
            for cell in cells:
                digits = cell.replace(",", "")
                if digits.isdigit():
                    return int(digits)
    raise SystemExit("could not read the resolved byte count from README.md")


def delta_group(delta: int | None) -> str:
    """`delta-0`, `small-delta` (|delta| <= SMALL_DELTA) or `big-delta`."""
    size = abs(delta or 0)
    if size == 0:
        return "delta-0"
    return "small-delta" if size <= SMALL_DELTA else "big-delta"


def group_totals(rows: list[dict]) -> dict[str, dict]:
    """Functions, bytes and masked words per delta group, every group present."""
    out = {g: {"functions": 0, "bytes": 0, "words": 0} for g in DELTA_GROUPS}
    for row in rows:
        entry = out[delta_group(row.get("size_delta"))]
        entry["functions"] += 1
        entry["bytes"] += row["size_bytes"]
        entry["words"] += row["relocation_masked_differing_words"]
    return out


def next_milestone(have: int) -> dict:
    """The first multiple of MILESTONE_STEP percent strictly above `have`."""
    pct = 100.0 * have / WHOLE_PROGRAM
    step = int(pct // MILESTONE_STEP) + 1
    milestone = step * MILESTONE_STEP
    target = int(WHOLE_PROGRAM * milestone / 100.0)
    return {"pct": milestone, "bytes": target, "gap_bytes": max(target - have, 0)}


def clusters(rows: list[dict]) -> list[dict]:
    """Identical size in the same overlay: the same routine, specialised."""
    grouped: dict[tuple, list[dict]] = collections.defaultdict(list)
    for row in rows:
        grouped[(row["size_bytes"], row["overlay"])].append(row)
    out = []
    for (size, overlay), members in grouped.items():
        if len(members) < 2:
            continue
        words = sorted(m["relocation_masked_differing_words"] for m in members)
        out.append({
            "size_bytes": size, "overlay": overlay, "count": len(members),
            "total_bytes": size * len(members), "words": words,
            "lead_words": words[0], "spread": words[-1] - words[0],
            "members": [m["name"] for m in members],
            "delta_groups": sorted({delta_group(m.get("size_delta"))
                                    for m in members},
                                   key=DELTA_GROUPS.index),
        })
    return sorted(out, key=lambda c: -c["total_bytes"])


def cheapest_route(rows: list[dict], gap: int) -> dict:
    """The fewest bytes-cheapest functions that cover the gap."""
    ordered = sorted(
        rows, key=lambda r: r["relocation_masked_differing_words"] / max(r["size_bytes"], 1))
    picked, acc, words = [], 0, 0
    for row in ordered:
        picked.append(row)
        acc += row["size_bytes"]
        words += row["relocation_masked_differing_words"]
        if acc >= gap:
            break
    return {"functions": len(picked), "bytes": acc, "words": words,
            "covers_gap": acc >= gap,
            "worst": picked[-1] if picked else None,
            "names": [r["name"] for r in picked],
            "by_group": group_totals(picked)}


def cluster_summary(cl: list[dict], gap: int) -> dict:
    total = sum(c["total_bytes"] for c in cl)
    lead = sum(c["lead_words"] for c in cl)
    every = sum(w for c in cl for w in c["words"])
    return {"groups": len(cl), "functions": sum(c["count"] for c in cl),
            "bytes": total,
            "pct_of_gap": (100.0 * total / gap) if gap else 0.0,
            "words_every_sibling": every, "words_one_lead": lead,
            "leverage": (every / lead) if lead else 0.0}


def report(target_pct: float, top: int, *, use_cache: bool = True) -> dict:
    rows = load()
    all_names = {r["name"] for r in json.loads(
        RANKING.read_text(encoding="utf-8"))["functions"]}
    excluded = [{"name": n, "reason": v["reason"]}
                for n, v in sorted(unassignable().items()) if n in all_names]
    states = assignability([r["name"] for r in rows], use_cache=use_cache)
    if states:
        blocked = [r for r in rows if states.get(r["name"], "base-only") != "base-only"]
        rows = [r for r in rows if states.get(r["name"], "base-only") == "base-only"]
        by_state: dict[str, dict] = {}
        for r in blocked:
            e = by_state.setdefault(states[r["name"]], {"functions": 0, "bytes": 0})
            e["functions"] += 1
            e["bytes"] += r["size_bytes"]
        blocked_summary = {"functions": len(blocked),
                           "bytes": sum(r["size_bytes"] for r in blocked),
                           "by_state": by_state}
    else:
        blocked_summary = None
    have = resolved_bytes()
    target = int(WHOLE_PROGRAM * target_pct / 100.0)
    gap = max(target - have, 0)
    queue_bytes = sum(r["size_bytes"] for r in rows)
    grouped = {g: [r for r in rows if delta_group(r.get("size_delta")) == g]
               for g in DELTA_GROUPS}
    cl = clusters(rows)
    band_rows = []
    for lo, hi, note in BANDS:
        sel = [r for r in rows
               if lo <= r["relocation_masked_differing_words"] <= (hi or 10 ** 9)]
        band_rows.append({"lo": lo, "hi": hi, "note": note, "functions": len(sel),
                          "bytes": sum(r["size_bytes"] for r in sel),
                          "words": sum(r["relocation_masked_differing_words"] for r in sel),
                          "by_group": group_totals(sel)})
    route_by_group = {}
    for g in DELTA_GROUPS:
        alone = cheapest_route(grouped[g], gap)
        route_by_group[g] = {k: alone[k] for k in
                             ("functions", "bytes", "words", "covers_gap")}
    return {
        "resolved_bytes": have, "whole_program": WHOLE_PROGRAM,
        "pct": 100.0 * have / WHOLE_PROGRAM, "target_pct": target_pct,
        "target_bytes": target, "gap_bytes": gap,
        "next_milestone": next_milestone(have),
        "queue": {"functions": len(rows), "bytes": queue_bytes,
                  "by_group": group_totals(rows)},
        "route": cheapest_route(rows, gap),
        "route_by_group": route_by_group,
        "clusters": dict(cluster_summary(cl, gap), top=cl[:top],
                         by_group={g: cluster_summary(clusters(grouped[g]), gap)
                                   for g in DELTA_GROUPS}),
        "bands": band_rows,
        "excluded": excluded,
        "blocked": blocked_summary,
    }


def _group_lines(by_group: dict[str, dict], indent: str,
                 words: bool = True) -> list[str]:
    out = []
    for g in DELTA_GROUPS:
        e = by_group[g]
        line = f"{indent}{g:<12} {e['functions']:4d} fns {e['bytes']:9,} B"
        if words:
            line += f" {e['words']:8,} w"
        out.append(line)
    return out


def render(r: dict) -> str:
    nm = r["next_milestone"]
    out = [
        f"resolved {r['resolved_bytes']:,} / {r['whole_program']:,} = {r['pct']:.2f}%",
        f"target   {r['target_pct']:.0f}% = {r['target_bytes']:,} bytes",
        f"GAP      {r['gap_bytes']:,} bytes  "
        f"({100.0 * r['gap_bytes'] / max(r['queue']['bytes'], 1):.0f}% of the "
        f"{r['queue']['bytes']:,} still queued)",
        f"NEXT 5%  {nm['pct']:.0f}% = {nm['bytes']:,} bytes, gap {nm['gap_bytes']:,}",
    ]
    for ex in r.get("excluded", []):
        out.append(f"EXCLUDED {ex['name']} -- {ex['reason']} Never assign it.")
    bl = r.get("blocked")
    if bl is None:
        out.append("NOTE lane_status unavailable -- figures below may include "
                   "targets no lane can accept")
    elif bl["functions"]:
        out.append(f"NOT ASSIGNABLE {bl['functions']} fns, {bl['bytes']:,} bytes "
                   f"-- excluded from everything below")
        for st, e in sorted(bl["by_state"].items(), key=lambda kv: -kv[1]["bytes"]):
            out.append(f"    {st:<32} {e['functions']:>4} fns  {e['bytes']:>9,} B")
        out.append("    repin stale authorizations with tools/authorize_reopen.py")
    out += ["", "queue by delta group (small = |size_delta| <= "
            f"{SMALL_DELTA}; only delta-0 is colour work):"]
    out += _group_lines(r["queue"]["by_group"], "  ")
    out += [
        "",
        "cheapest route (by words per byte):",
        f"  {r['route']['functions']} functions, {r['route']['bytes']:,} bytes, "
        f"{r['route']['words']:,} masked words to close",
    ]
    out += _group_lines(r["route"]["by_group"], "    ")
    worst = r["route"]["worst"]
    if worst:
        out.append(f"  worst in that set: {worst['relocation_masked_differing_words']}"
                   f" words on {worst['size_bytes']} bytes ({worst['name']}, "
                   f"{delta_group(worst.get('size_delta'))})")
    out.append("  each group alone:")
    for g in DELTA_GROUPS:
        e = r["route_by_group"][g]
        verdict = "covers the gap" if e["covers_gap"] else "cannot cover the gap"
        out.append(f"    {g:<12} {e['functions']:4d} fns {e['bytes']:9,} B "
                   f"{e['words']:8,} w  {verdict}")
    c = r["clusters"]
    out += ["",
            f"clusters: {c['groups']} groups, {c['functions']} functions, "
            f"{c['bytes']:,} bytes = {c['pct_of_gap']:.0f}% of the gap",
            f"  every sibling separately: {c['words_every_sibling']:,} words",
            f"  one lead per cluster    : {c['words_one_lead']:,} words "
            f"({c['leverage']:.1f}x leverage)"]
    for g in DELTA_GROUPS:
        e = c["by_group"][g]
        out.append(f"    {g:<12} {e['groups']:3d} groups {e['functions']:4d} fns "
                   f"{e['bytes']:9,} B  lead {e['words_one_lead']:6,} w "
                   f"({e['leverage']:.1f}x)")
    out.append("")
    for cl in c["top"]:
        tight = "  <- tight, expect transfer" if cl["spread"] <= 10 else ""
        out.append(f"  {cl['count']}x {cl['size_bytes']:5d}B "
                   f"ov={str(cl['overlay'] or 'main'):5s} "
                   f"{cl['total_bytes']:6,}B  words {cl['words']} "
                   f"[{'/'.join(cl['delta_groups'])}]{tight}")
    out += ["", "bands (delta-0 / small-delta / big-delta, fns and bytes):"]
    for b in r["bands"]:
        hi = b["hi"] if b["hi"] is not None else "+"
        split = "  ".join(f"{b['by_group'][g]['functions']}/{b['by_group'][g]['bytes']:,}"
                          for g in DELTA_GROUPS)
        out.append(f"  {b['lo']:4d}-{str(hi):<5s} {b['functions']:3d} fns "
                   f"{b['bytes']:9,}B {b['words']:8,}w   {b['note']:<24} [{split}]")
    return "\n".join(out)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description="Scope the next wave from the ranking.")
    ap.add_argument("--target-pct", type=float, default=65.0)
    ap.add_argument("--top", type=int, default=12)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--no-cache", action="store_true",
                    help="classify every symbol from git history, ignoring "
                         "and not writing the assignment cache")
    args = ap.parse_args(argv)
    r = report(args.target_pct, args.top, use_cache=not args.no_cache)
    print(json.dumps(r, indent=2) if args.json else render(r))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
