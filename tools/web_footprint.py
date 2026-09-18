#!/usr/bin/env python3
"""Map every coloured web of one function to the words its colour controls.

    tools/web_footprint.py <symbol> --out DIR [--trace <allocator.log>]
                           [--proc N] [--every-colour] [--cross-kind]
                           [--webs 7,44,101] [--limit N] [--window 0x80]
                           [--hold p1:w75=c16 ...]
                           [--list-procs] [--timeout S] [--minutes M]

WHY THIS EXISTS

`tools/force_lattice.py` searches combinations of forces a human nominated.
On the overlay 58 function a lane nominated five, measured all 31 subsets,
found every pair exactly additive, and reported a floor of 185 masked words
against a base of 227. Read as a floor that is wrong twice over:

  1. Five is 5 of the **139** webs that procedure actually colours (431 p1
     decisions, 139 p1color records). The other 134 were never asked.
  2. 81 of the 227 positional words sit in two windows bracketed by one-word
     insertions, where every following word mismatches by position and aligns
     perfectly. No colour can move them. The aligned residual of those two
     windows is 22 rows, not 81.

So the lattice's floor is the floor of the five questions asked, and the
question that could not be asked was "which webs live where". Nomination was
the bottleneck, and nomination needs a map from a residual window back to the
webs whose colour controls it.

HOW IT MAPS

Forcing one web to a different legal colour and diffing the resulting object
against the unforced one shows exactly the words that web's colour controls.
One compile per web, and the map is a lookup thereafter: point at a window of
residual and read off its candidate forces.

The diff is POSITIONAL, which is the right view here even though it is the
wrong view for ranking a window. Both objects are compared against the same
target, so an insertion's positional shadow is identical in both and cancels
in the difference. A force that changes the function's size shifts that shadow
instead of cancelling it, so those cells are reported and excluded rather than
mapped.

HOLDING A SET OF FORCES: THE SECOND-ORDER LANDSCAPE

Every landscape above probes each web against the UNFORCED baseline. Once a
packing of forces is known to be right -- on overlay 58, five forces whose
every member has a named variable and a measured reason -- the question that
remains is what the OTHER webs do once those five are held. `--hold` takes a
list of forces (repeatable), compiles the baseline with them applied, reads
the probe plan from that baseline's own trace (the held colours change which
colours the other webs are offered, so an unforced trace would plan against
the wrong tables), skips the held webs, and adds the held set to every probe
cell. The scores, footprints, winners and packing are then relative to the
held baseline, and a winner is a force to ADD to the held set. A `--trace`
given alongside `--hold` is refused for that reason.

CHOOSING THE PROBE COLOUR

The legal colours for a web are the keys of its own `p1cost` table, not the
bits of the `available0/1` mask in its `p1dec` record. Measured: all 139
coloured webs of this procedure list their own final colour in their cost
table, while the availability mask calls three of the five colours the lane
successfully forced illegal. The mask is the state at the moment that web was
decided; a force overrides the decision, so the mask under-reports and using
it would have skipped good probes.

`p1cost` also carries `kind`, caller or callee. Probing a callee-save web with
a caller-save colour changes what the prologue saves, which changes the
function's size, which shifts the insertion shadow this tool differences
against -- the cell then has to be thrown away. So the probe stays inside the
web's own kind where it can (median 7 same-kind alternatives, minimum 0), and
among those takes the cheapest, breaking ties by the colour nearest the one
the web holds, because an adjacent register is the smallest perturbation that
still shows where the web lives.
"""
from __future__ import annotations

import argparse
import collections
import itertools
import json
import pathlib
import re
import sys
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import force_lattice as fl  # noqa: E402

FIELD_RE = re.compile(r"(\w+)=(\S+)")


def source_fingerprint(symbol: str) -> str | None:
    """The ranking's source hash for this function, or None if unavailable.

    A landscape is measured against ONE function body. Change the source and it
    is void -- the standing rule says so, and it still cost this campaign five
    consecutive dispatches on the tree's biggest function, each told not to
    re-run a landscape that had been measured against a body two revisions old.
    Stamping the hash into the report turns that from a judgement call into a
    line of output.
    """
    try:
        document = json.loads((fl.ROOT / "config" /
                               "nonmatching-ranking.us.json").read_text())
    except (OSError, ValueError):
        return None
    for row in document.get("functions", []):
        if row.get("name") == symbol:
            return row.get("source_context_sha256")
    return None


def freshness(report: dict) -> str | None:
    """Say plainly when a saved landscape no longer describes the tree."""
    stamped = report.get("source_context_sha256")
    if not stamped:
        return ("this landscape carries no source fingerprint, so whether it "
                "still describes the tree cannot be checked -- re-run it")
    current = source_fingerprint(report.get("symbol", ""))
    if current is None:
        return None
    if current != stamped:
        return (f"STALE: measured against source {stamped}, tree is now "
                f"{current}. A source change voids a landscape; re-run it "
                f"before nominating from it.")
    return None


def parse_trace(text: str, proc: int) -> dict[int, dict]:
    """Collect every web's detail, colour, cost table and availability mask."""
    webs: dict[int, dict] = collections.defaultdict(
        lambda: {"costs": {}, "kinds": {}, "detail": {}, "color": None,
                 "reg": None, "available": 0, "forbidden": 0})
    for raw in text.splitlines():
        if not raw.startswith("[CDX] "):
            continue
        parts = raw.split(None, 2)
        if len(parts) < 3:
            continue
        event = parts[1]
        if event not in ("webdetail", "p1color", "p1cost", "p1dec"):
            continue
        fields = dict(FIELD_RE.findall(parts[2]))
        if fields.get("proc") != str(proc) or "web" not in fields:
            continue
        entry = webs[int(fields["web"])]
        if event == "webdetail":
            entry["detail"] = fields
        elif event == "p1color":
            entry["color"] = int(fields["color"])
            entry["reg"] = fields.get("reg")
        elif event == "p1cost":
            entry["costs"][int(fields["color"])] = float(fields["cost"])
            entry["kinds"][int(fields["color"])] = fields.get("kind")
        elif event == "p1dec":
            entry["available"] = (int(fields.get("available0", "0"), 0)
                                  | int(fields.get("available1", "0"), 0) << 32)
            entry["forbidden"] = (int(fields.get("forbidden0", "0"), 0)
                                  | int(fields.get("forbidden1", "0"), 0) << 32)
    return dict(webs)


def probe_colour(entry: dict) -> int | None:
    """The smallest legal perturbation that still relocates the web.

    Legality is cost-table membership; see the header for why the availability
    mask is not used. Same-kind colours are preferred so the probe does not
    rewrite the prologue, but a web with no same-kind alternative still gets
    probed -- a cell whose size moves is reported and excluded, which is a
    better outcome than a web that never appears on the map at all.
    """
    taken = entry.get("color")
    if taken is None:
        return None
    costs, kinds = entry["costs"], entry["kinds"]
    legal = [c for c in costs if c != taken]
    if not legal:
        return None
    mine = kinds.get(taken)
    return min(legal, key=lambda c: (kinds.get(c) != mine, costs[c],
                                     abs(c - taken), c))


def colourable(webs: dict[int, dict]) -> list[int]:
    return sorted(w for w, e in webs.items() if e.get("color") is not None)


def every_colour(entry: dict, same_kind: bool = True) -> list[int]:
    """Every legal colour other than the one the web holds.

    One probe per web locates a web; it does not price it, because the probe
    colour is chosen to perturb least, not to help. At the measured rate --
    roughly 0.6s per compile on this function -- the whole cross product of
    webs and their legal colours is minutes, so the landscape can be read
    exhaustively instead of sampled. Ordered nearest-first so a truncated run
    still covers each web's smallest perturbations.
    """
    taken = entry.get("color")
    if taken is None:
        return []
    kinds, mine = entry["kinds"], entry["kinds"].get(entry["color"])
    legal = [c for c in entry["costs"] if c != taken
             and (not same_kind or kinds.get(c) == mine)]
    return sorted(legal, key=lambda c: (abs(c - taken), c))


def held_webs(hold: list[str]) -> set[int]:
    """The webs a held force pins; probing one of them would contradict the hold."""
    return {w for force in hold for w in fl.webs_of(force)}


def plan_probes(webs: dict[int, dict], wanted: list[int], *,
                every: bool = False, cross_kind: bool = False,
                hold: list[str] = ()) -> list[tuple[int, int | None]]:
    """The (web, colour) cells to compile, in order.

    A held web is not planned at all: its colour is the premise of the
    landscape, and a probe on it would measure a different premise. It is
    reported as such rather than dropped silently, so a hold that names a
    web the trace does not colour is visible.
    """
    pinned = held_webs(list(hold))
    plan: list[tuple[int, int | None]] = []
    for web in wanted:
        entry = webs.get(web)
        if web in pinned:
            continue
        if entry is None:
            plan.append((web, None))
        elif every:
            colours = every_colour(entry, same_kind=not cross_kind)
            plan.extend((web, c) for c in colours)
            if not colours:
                plan.append((web, None))
        else:
            plan.append((web, probe_colour(entry)))
    return plan


def cell_forces(hold: list[str], web: int, colour: int) -> tuple[str, ...]:
    """The held set first, then the probe, as one cell's force list."""
    return tuple(hold) + (f"p1:w{web}=c{colour}",)


class Reader:
    """Read candidate objects against one assembled target, assembled once.

    `nm_ranking.word_streams` reassembles the target for every object it is
    handed, which is right for a one-shot score and wrong for a sweep: the
    target does not change between webs, and on this function reassembling it
    once per web is most of the run. The target side is read once here and the
    candidate side per object.
    """

    def __init__(self, symbol: str, base_object: pathlib.Path):
        import nm_ranking as nr
        import permute_batch as pb

        self.nr = nr
        self.symbol = symbol
        items = [i for i in pb.discover_queue() if i.func == symbol]
        if not items:
            raise SystemExit(f"web_footprint: {symbol} is not in the "
                             "NON_MATCHING queue")
        self.item = items[0]
        streams, error = nr.word_streams(self.item, base_object)
        if streams is None:
            raise SystemExit(f"web_footprint: {error}")
        self.target_words = streams.target_words
        self.target_reloc = streams.target_reloc

    def positions(self, obj: pathlib.Path) -> list[int] | None:
        nr = self.nr
        span = nr.func_symbol_span(obj, self.symbol)
        if span is None:
            return None
        start, size = span
        words = nr.words_of(nr.text_bytes(obj, start, size))
        reloc = nr.relocations(obj, start, size)
        return nr.masked_mismatch_positions(
            words, self.target_words, reloc, self.target_reloc)


def histogram(positions: list[int], width: int) -> dict[int, int]:
    counts: dict[int, int] = {}
    for index in positions:
        key = (index * 4) // width * width
        counts[key] = counts.get(key, 0) + 1
    return counts


def windows_of(row: dict) -> dict[int, int]:
    """A row's footprint with integer window keys.

    JSON has no integer keys, so a footprint written to `footprints.json` and
    read back has string ones. Every consumer here formats windows as hex and
    compares them as numbers, so a reloaded report used to raise `Unknown
    format code 'x' for object of type 'str'` -- and the reload is the whole
    point of writing the file.
    """
    return {int(k): v for k, v in (row.get("footprint") or {}).items()}


def footprint(base: dict[int, int], cell: dict[int, int]) -> dict[int, int]:
    keys = set(base) | set(cell)
    moved = {k: cell.get(k, 0) - base.get(k, 0) for k in keys}
    return {k: v for k, v in sorted(moved.items()) if v}


def winners(rows: list[dict], base_score: int) -> list[dict]:
    """Probes that beat the unforced score without changing the size.

    This is the deliverable of an exhaustive run. A probe chosen to LOCATE a
    web perturbs it least and usually scores worse; a probe that scores better
    is a force worth nominating into a lattice, and the exhaustive sweep finds
    them without anyone guessing which web to ask about.
    """
    return sorted((r for r in rows if r["status"] == "ok"
                   and r["score"] is not None and r["score"] < base_score),
                  key=lambda r: (r["score"], r["web"]))


def pack(rows: list[dict], base_score: int) -> tuple[list[dict], int]:
    """The best set of forces whose radii do not overlap, and its predicted score.

    Forces with disjoint radii are additive (L156), so the best COMBINATION is
    a maximum-weight packing over radii -- not the top of the winners list.
    Greedy by single score gets it wrong, measurably: on overlay 58
    `w225=c20` scores 217 alone and `w225=c14` scores 220, yet the five-force
    set containing c14 measures 185 while the one containing c20 measures 192.
    `w225=c20` shares its radius exactly with `w379=c20` -- rival webs for one
    slot -- so taking it abandons the separate region that c14 reaches.

    Two constraints, both load-bearing:

      * radii must not overlap, or the members contend and additivity fails;
      * at most ONE colour per web, because a web has one colour. Without this
        the packing happily proposes `w225=c14` and `w225=c20` together and
        predicts a score no compile can produce.

    Exact by enumeration. The winner list is small -- eleven on the largest
    function in the tree -- because most forces do not beat the base at all.
    """
    winners_ = winners(rows, base_score)
    for r in winners_:
        r["_gain"] = base_score - r["score"]
        r["_radius"] = frozenset(windows_of(r))
    best: list[dict] = []
    best_gain = 0
    for size in range(1, len(winners_) + 1):
        for combo in itertools.combinations(winners_, size):
            if len({r["web"] for r in combo}) != len(combo):
                continue
            covered: set[int] = set()
            for r in combo:
                if covered & r["_radius"]:
                    break
                covered |= r["_radius"]
            else:
                gain = sum(r["_gain"] for r in combo)
                if gain > best_gain:
                    best_gain, best = gain, list(combo)
    return best, base_score - best_gain


def rivals(rows: list[dict], base_score: int) -> list[list[dict]]:
    """Groups of winning forces that share one radius: one question, many handles.

    Three webs on overlay 58 carry byte-identical radii around `+0x1700`. They
    are not three findings; they are one, reachable three ways, and a lattice
    that nominates two of them pays for both and gets neither.
    """
    groups: dict[frozenset, list[dict]] = {}
    for r in winners(rows, base_score):
        groups.setdefault(frozenset(windows_of(r)), []).append(r)
    return [sorted(g, key=lambda r: r["score"])
            for g in groups.values() if len(g) > 1]


def render(rows: list[dict], width: int, base_score: int | None = None,
           hold: list[str] | None = None) -> str:
    out = [f"web footprints (windows of {width:#x} bytes, signed words)", ""]
    hold = list(hold or [])
    if hold:
        out.append("  HELD in every cell (second-order landscape; scores, "
                   "footprints and the packing are relative to this set, and a "
                   "winner is a force to ADD to it):")
        for force in hold:
            out.append(f"    --force {force}")
        out.append("")
    if base_score is not None:
        beat = winners(rows, base_score)
        baseline = "held" if hold else "unforced"
        out.append(f"  probes beating the {baseline} {base_score} at delta 0: "
                   f"{len(beat)}")
        for r in beat:
            out.append(f"    p1:w{r['web']}=c{r['probe']:<4} {r['reg'] or '?':<4} "
                       f"{r['score']:>5}  ({base_score - r['score']:+d})")
        out.append("")
        for group in rivals(rows, base_score):
            out.append("  RIVALS -- one question, "
                       f"{len(group)} handles, identical radius: "
                       + " ".join(f"w{r['web']}=c{r['probe']}({r['score']})"
                                  for r in group))
        chosen, predicted = pack(rows, base_score)
        if chosen:
            out.append("")
            out.append(f"  best disjoint set, one colour per web -> predicted "
                       f"{predicted}:")
            for r in sorted(chosen, key=lambda r: r["score"]):
                out.append(f"    --force p1:w{r['web']}=c{r['probe']}"
                           f"   ({base_score - r['score']:+d})")
            out.append("  Measure it; the prediction assumes additivity, which "
                       "disjoint radii imply but do not guarantee.")
        out.append("")
    mapped = [r for r in rows if windows_of(r)]
    silent = [r for r in rows if r.get("status") == "ok" and not windows_of(r)]
    skipped = [r for r in rows if r.get("status") not in ("ok",)]
    out.append(f"  {len(rows)} webs probed: {len(mapped)} located, "
               f"{len(silent)} moved nothing, {len(skipped)} not probed")
    out.append("")
    out.append("  web   reg  ->probe   score  windows moved")
    for r in sorted(mapped, key=lambda r: (min(windows_of(r)), r["web"])):
        spans = " ".join(f"{k:#07x}{v:+d}"
                         for k, v in sorted(windows_of(r).items()))
        out.append(f"  {r['web']:<5} {r['reg'] or '?':<4} ->c{r['probe']:<6} "
                   f"{r['score']:>5}  {spans}")
    if silent:
        out.append("")
        out.append("  moved no word (colour is free here): "
                   + " ".join(str(r["web"]) for r in silent))
    if skipped:
        out.append("")
        out.append("  not probed:")
        for r in skipped:
            out.append(f"    web {r['web']:<5} {r['status']}")

    # A probe relocates its own web and can cascade into its neighbours, so a
    # footprint is a superset of the web's own instructions -- one v0 web here
    # reads across half the function. Ranking each window's candidates by how
    # much they move THAT window puts the web whose own range covers it first,
    # which is the order a nomination should be tried in.
    index: dict[int, list[tuple[int, int]]] = collections.defaultdict(list)
    labelled = len({r["web"] for r in mapped}) != len(mapped)
    for r in mapped:
        for window, moved in windows_of(r).items():
            index[window].append(
                (abs(moved), f"{r['web']}=c{r['probe']}" if labelled
                 else str(r["web"])))
    out += ["", "  window -> webs whose colour controls it, strongest first "
                "(this is the nomination table)"]
    for window in sorted(index):
        ranked = sorted(index[window], key=lambda p: (-p[0], p[1]))
        out.append(f"   {window:#07x}  "
                   + " ".join(f"{web}({size})" for size, web in ranked))
    if labelled:
        out.append("  (a web probed at several colours is named web=colour)")
    return "\n".join(out)


def refuse_every_colour(size_delta: int | None, every_colour: bool) -> str | None:
    """L155: a colour landscape on a size mismatch is insertion shadow.

    `--every-colour` is the exhaustive single-force map. Each probe that
    inserts or deletes a word shifts every later mismatch, so the footprint
    names the insertion, not the web. Refuse before the first probe compile.
    A non-landscape run (one web, a hold, `--list-procs`) still measures.
    """
    if every_colour and size_delta:
        return (
            f"web_footprint: refusing --every-colour: live size_delta "
            f"{size_delta:+d} (L155: colour on a size mismatch is insertion "
            f"shadow). Close size/frame first."
        )
    return None


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Map each coloured web to the words its colour controls.")
    parser.add_argument("symbol")
    parser.add_argument("--trace", type=pathlib.Path, default=None,
                        help="an unforced allocator.log captured with "
                             "CDX_DETAIL_WEB=all; omit to capture one from "
                             "this run's own unforced baseline compile")
    parser.add_argument("--list-procs", action="store_true",
                        help="capture a baseline, print its procindex rows "
                             "and stop; use this to find --proc on a TU that "
                             "holds more than one function")
    parser.add_argument("--proc", type=int, default=0)
    parser.add_argument("--webs", default="",
                        help="comma-separated subset; default is every "
                             "coloured web")
    parser.add_argument("--limit", type=int, default=0,
                        help="probe at most this many webs")
    parser.add_argument("--window", type=lambda v: int(v, 0), default=fl.WINDOW)
    parser.add_argument("--out", type=pathlib.Path, required=True,
                        help="directory for per-web objects, logs and results")
    parser.add_argument("--timeout", type=float, default=600.0)
    parser.add_argument("--minutes", type=float, default=240.0)
    parser.add_argument("--every-colour", action="store_true",
                        help="probe every legal colour of each web, not one; "
                             "this is the exhaustive single-force landscape")
    parser.add_argument("--cross-kind", action="store_true",
                        help="with --every-colour, also probe colours of the "
                             "other save kind (these usually change size)")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--report", type=pathlib.Path, default=None,
                        help="re-render a saved footprints.json and stop; no "
                             "compile, and the packing is recomputed")
    parser.add_argument("--hold", action="append", default=[],
                        help="a force (p1:wN=cM) kept applied in the baseline "
                             "and in every probe; repeatable. This is the "
                             "second-order landscape: what every other web "
                             "does once the held set is right")
    args = parser.parse_args(argv)
    if args.hold:
        fl.validate_forces(args.hold)
        if args.trace is not None:
            raise SystemExit("web_footprint: --trace cannot be combined with "
                             "--hold; the held baseline's own trace is the "
                             "one whose cost tables the probes are planned "
                             "from")

    if args.report is not None:
        saved = json.loads(args.report.read_text())
        warning = freshness(saved)
        if warning:
            print(f"!! {warning}\n")
        print(render(saved["rows"], saved.get("window", args.window),
                     saved.get("base_score"), saved.get("hold")))
        return 1 if warning and warning.startswith("STALE") else 0

    command = fl.compile_command(args.symbol)
    args.out.mkdir(parents=True, exist_ok=True)
    deadline = time.time() + args.minutes * 60

    base_cell = fl.run_cell(args.symbol, args.proc, tuple(args.hold),
                            command=command, directory=args.out / "base",
                            deadline=deadline, timeout=args.timeout)
    if not base_cell.accepted:
        raise SystemExit(f"web_footprint: {'held' if args.hold else 'unforced'}"
                         f" baseline failed: {base_cell.note}")
    refusal = refuse_every_colour(base_cell.size_delta, args.every_colour)
    if refusal:
        raise SystemExit(refusal)
    captured = args.out / "base" / "allocator.log"
    trace_path = args.trace if args.trace is not None else captured
    trace_text = trace_path.read_text() if trace_path.is_file() else ""

    if args.list_procs:
        rows = [l.strip() for l in trace_text.splitlines()
                if l.startswith("[CDX] procindex")]
        print(f"baseline scored {base_cell.score}; procedure index rows in "
              f"{trace_path}:")
        for row in rows or ["  (none -- the capture carries no procindex row)"]:
            print(f"  {row}")
        print("\nA TU holding one function indexes it at proc=0. For a TU with "
              "several, map the ordinal to a name with "
              "tools/allocator_trace_receipt.py rather than guessing: the "
              "ordinal is the Ucode procedure order, not the source order.")
        return 0

    webs = parse_trace(trace_text, args.proc)
    if not webs:
        raise SystemExit(
            f"web_footprint: no proc={args.proc} records in {trace_path}; "
            "re-run with --list-procs to see which ordinals this TU emits, "
            "and check the capture carried CDX_DETAIL_WEB=all")
    wanted = ([int(w) for w in args.webs.split(",") if w.strip()]
              if args.webs else colourable(webs))
    plan = plan_probes(webs, wanted, every=args.every_colour,
                       cross_kind=args.cross_kind, hold=args.hold)
    if args.limit:
        plan = plan[:args.limit]

    reader = Reader(args.symbol, args.out / "base" / "candidate.o")
    base_hist = histogram(reader.positions(args.out / "base" / "candidate.o"),
                          args.window)
    print(f"base {base_cell.score} masked words over {len(base_hist)} "
          f"windows; {len(plan)} probes over {len({w for w, _ in plan})} webs"
          + (f"; holding {' '.join(args.hold)}" if args.hold else ""),
          flush=True)

    rows: list[dict] = []
    for n, (web, colour) in enumerate(plan, 1):
        entry = webs.get(web)
        row = {"web": web, "reg": entry and entry.get("reg"),
               "probe": colour, "score": None, "footprint": {}, "status": ""}
        if colour is None:
            row["status"] = ("no second colour in its cost table" if entry
                             else "no records for this web")
            rows.append(row)
            continue
        cell = fl.run_cell(args.symbol, args.proc,
                           cell_forces(args.hold, web, colour),
                           command=command,
                           directory=args.out / f"web-{web}-c{colour}",
                           deadline=deadline, timeout=args.timeout)
        if not cell.accepted:
            row["status"] = f"declined: {cell.note}"
        elif cell.size_delta:
            row["status"] = (f"size delta {cell.size_delta:+d}; the insertion "
                             "shadow shifts and cannot be differenced")
            row["score"] = cell.score
        else:
            positions = reader.positions(
                args.out / f"web-{web}-c{colour}" / "candidate.o")
            row["status"] = "ok"
            row["score"] = cell.score
            row["footprint"] = footprint(base_hist,
                                         histogram(positions, args.window))
        rows.append(row)
        print(f"  [{n}/{len(plan)}] web {web}=c{colour} "
              f"{row['status'] or 'ok':<40} score {row['score']}", flush=True)
        if time.time() >= deadline:
            print("  deadline reached; stopping", flush=True)
            break

    report = {"symbol": args.symbol, "proc": args.proc, "window": args.window,
              "base_score": base_cell.score, "hold": list(args.hold),
              "rows": rows,
              "source_context_sha256": source_fingerprint(args.symbol)}
    (args.out / "footprints.json").write_text(json.dumps(report, indent=2) + "\n")
    print()
    print(json.dumps(report, indent=2) if args.json
          else render(rows, args.window, base_cell.score, args.hold))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
