#!/usr/bin/env python3
"""Run the insertion-pair reader over every small size-mismatch function.

    tools/small_delta_census.py [--max-delta 12] [--out docs/small-delta-census.md]
                                [--json PATH] [--symbols a,b,...]

`gmake small-delta-census` runs it. For every ranking row with
0 < |size_delta| <= 12 it runs `tools/insertion_pairs.py`'s analysis -- one
configured compile and one traced compile per translation unit, the trace
identity-gated per function -- and writes a tracked summary: one row per
function, sorted by aligned residual after shadow ascending, plus totals by
label.

The summary is counts and labels only. It carries no instruction text, no
words and no offsets: a pair's location is a lane's question, answered by
running the reader on that symbol, not a fact to publish.

WHY THE SORT KEY

The ranking sorts size-mismatch rows by positional masked words, which on
these functions is mostly shadow (L155): a function four bytes off with 160
masked words may have 90 real ones. The aligned residual after shadow is the
count a lane would actually have to change, so it is the order to take them in.
"""
from __future__ import annotations

import argparse
import collections
import json
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import insertion_pairs as ip  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]
RANKING = ROOT / "config" / "nonmatching-ranking.us.json"
DEFAULT_OUT = ROOT / "docs" / "small-delta-census.md"


def small_delta_rows(ranking: dict, max_delta: int) -> list[dict]:
    return [r for r in ranking["functions"]
            if r.get("size_delta") and abs(int(r["size_delta"])) <= max_delta]


def dominant_class(classes: dict[str, int]) -> str:
    if not classes:
        return "-"
    return sorted(classes.items(),
                  key=lambda kv: (-kv[1], ip.CLASSES.index(kv[0])))[0][0]


def summarise(result: dict) -> dict:
    return {
        "symbol": result["symbol"],
        "file": result["file"],
        "bytes": result["size_bytes"],
        "delta": result["size_delta"],
        "frame_delta": result["frame_delta"],
        "pairs": len(result["pairs"]),
        "one_sided": result["one_sided_words"],
        "positional": result["positional"],
        "shadow": result["shadow"],
        "aligned_after_shadow": result["aligned_after_shadow"],
        "naming_in_pairs": result["naming_in_pairs"],
        "class": dominant_class(result["classes"]),
        "label": result["label"],
        "owned": result["owned"],
        "owning_lines": result["owning_lines"],
        "trace": result["trace"],
        "outside_agrees": result["outside_agrees"],
        "basis": result["basis"],
    }


def run(rows: list[dict]) -> tuple[list[dict], list[dict]]:
    """Analyse every row, compiling each translation unit once per side."""
    import nm_ranking as nr
    import permute_batch as pb

    queue = {item.func: item for item in pb.discover_queue()}
    by_tu: dict[str, list] = collections.defaultdict(list)
    errors: list[dict] = []
    for row in rows:
        item = queue.get(row["name"])
        if item is None:
            errors.append({"symbol": row["name"], "file": row["file"],
                           "error": "not in the NON_MATCHING queue"})
            continue
        by_tu[item.rel_c_file].append(item)

    results: list[dict] = []
    with ip._isolated_workdir(nr) as work:
        for tu in sorted(by_tu):
            items = by_tu[tu]
            print(f"  {tu}: {len(items)} function(s)", file=sys.stderr)
            commands = nr.configured_compile_commands(items)
            obj, error = nr.compile_configured_tu(tu, commands[tu])
            if obj is None:
                errors.extend({"symbol": i.func, "file": tu, "error": error}
                              for i in items)
                continue
            traced_obj, text, note = ip.compile_traced(items[0], work)
            procs = ip.parse_trace(text) if text else None
            for item in items:
                try:
                    result = ip.analyse(item, obj, procs, note, traced_obj)
                except SystemExit as exc:
                    errors.append({"symbol": item.func, "file": tu,
                                   "error": str(exc)})
                    continue
                results.append(summarise(result))
    results.sort(key=lambda r: (r["aligned_after_shadow"], r["symbol"]))
    return results, errors


def render(results: list[dict], errors: list[dict], rows: list[dict],
           max_delta: int) -> str:
    total_bytes = sum(int(r["size_bytes"]) for r in rows)
    owned = [r for r in results if r["owned"]]
    by_label: dict[str, list[dict]] = collections.defaultdict(list)
    for r in results:
        by_label[r["label"]].append(r)
    positional = sum(r["positional"] for r in results)
    shadow = sum(r["shadow"] for r in results)
    basis = collections.Counter()
    for r in results:
        basis.update(r["basis"])
    words = sum(basis.values())
    naming_in = sum(r["naming_in_pairs"] for r in results)
    out = [
        "# Small-delta census",
        "",
        "Generated by `gmake small-delta-census` (`tools/small_delta_census.py`",
        "over `tools/insertion_pairs.py`). Do not edit by hand; regenerate.",
        "",
        f"Scope: every row of `config/nonmatching-ranking.us.json` with "
        f"0 < |size_delta| <= {max_delta}: **{len(rows)} functions, "
        f"{total_bytes:,} bytes**. Measured {len(results)}; "
        f"{len(errors)} could not be read (listed at the end).",
        "",
        "What the columns mean is in the reader's docstring and in",
        "`docs/LANE_BRIEF.md` (Instruments). In short: a **pair** is one run of",
        "index misalignment between the candidate and the target; **shadow** is",
        "the positional masked count that misalignment adds on top of the words",
        "that really differ (L155); **aligned** is positional less shadow, the",
        "number of words a lane actually has to change. **In-pair naming** is",
        "the part of that which is register naming inside a pair -- the rows a",
        "free list rotated by the one-sided word would produce, so a fix to that",
        "word may take them with it; it is an upper bound on that, not a",
        "measurement of it. **Class** is the most",
        "common class of the one-sided words; **label** is the label of the pair",
        "carrying the most shadow; **owned** means every one-sided word has a",
        "source line and a ugen construct (or is an as1 nop).",
        "",
        "## Headline",
        "",
        f"- positional masked words {positional:,}, of which shadow "
        f"{shadow:,} and aligned {positional - shadow:,}; "
        f"{naming_in:,} of the aligned are register-naming rows inside a "
        "pair, the part a rotated free list would explain",
        f"- fully owned {len(owned)} of {len(results)}; "
        f"not fully owned {len(results) - len(owned)}",
        f"- {words:,} one-sided words, owned on basis: "
        + ", ".join(f"{k} {basis[k]:,}" for k in
                    ("line", "nearest", "prologue", "as1", "neighbour",
                     "unowned") if basis.get(k)),
        "  (`line` and `prologue` are direct; `nearest` is within three",
        "  lines; `neighbour` is a target-only word placed only by its",
        "  neighbour's line, the weakest basis)",
        "",
        "| label | functions | bytes | aligned after shadow | owned |",
        "|---|---:|---:|---:|---:|",
    ]
    for label in ip.LABELS:
        group = by_label.get(label)
        if not group:
            continue
        out.append(f"| {label} | {len(group)} | "
                   f"{sum(r['bytes'] for r in group):,} | "
                   f"{sum(r['aligned_after_shadow'] for r in group):,} | "
                   f"{sum(1 for r in group if r['owned'])} |")
    out += [
        "",
        "## Functions, by aligned residual after shadow",
        "",
        "| symbol | TU | bytes | delta | frame | pairs | shadow | aligned | "
        "in-pair naming | class | label | owned | lines |",
        "|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|---|---:|",
    ]
    for r in results:
        tu = r["file"].removeprefix("src/")
        out.append(
            f"| `{r['symbol']}` | `{tu}` | {r['bytes']:,} | {r['delta']:+d} | "
            f"{r['frame_delta']:+d} | {r['pairs']} | {r['shadow']} | "
            f"{r['aligned_after_shadow']} | {r['naming_in_pairs']} | "
            f"{r['class']} | {r['label']} | "
            f"{'yes' if r['owned'] else 'no'} | {r['owning_lines']} |")
    if errors:
        out += ["", "## Not measured", "",
                "| symbol | TU | reason |", "|---|---|---|"]
        for e in sorted(errors, key=lambda e: e["symbol"]):
            reason = " ".join(str(e["error"]).split())
            reason = reason.replace(str(ROOT) + "/", "")
            out.append(f"| `{e['symbol']}` | `{e['file']}` | {reason} |")
    gated = [r for r in results if r["trace"] != "identity gate passed"]
    if gated:
        out += ["", "Trace not used (identity gate or compile): "
                + ", ".join(f"`{r['symbol']}`" for r in gated) + "."]
    disagree = [r for r in results if not r["outside_agrees"]]
    if disagree:
        out += ["", "Outside-pair positional and aligned counts DISAGREE on: "
                + ", ".join(f"`{r['symbol']}`" for r in disagree) + "."]
    return "\n".join(out) + "\n"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--max-delta", type=int, default=12)
    parser.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT)
    parser.add_argument("--json", type=pathlib.Path, default=None,
                        help="also write the per-function rows here "
                             "(untracked scratch)")
    parser.add_argument("--symbols", default=None,
                        help="comma list: measure only these (no doc written)")
    args = parser.parse_args(argv)
    forced = ip.forced_environment()
    if forced:
        print("error: " + ", ".join(forced) + " is set; the census recompiles "
              "every TU unforced. Unset it.", file=sys.stderr)
        return 2
    ranking = json.loads(RANKING.read_text())
    rows = small_delta_rows(ranking, args.max_delta)
    if args.symbols:
        wanted = set(args.symbols.split(","))
        rows = [r for r in rows if r["name"] in wanted]
    results, errors = run(rows)
    if args.json:
        args.json.write_text(json.dumps({"results": results, "errors": errors},
                                        indent=2) + "\n")
    text = render(results, errors, rows, args.max_delta)
    if args.symbols:
        print(text)
    else:
        args.out.write_text(text)
        print(f"wrote {args.out.name}: "
              f"{len(results)} measured, {len(errors)} not", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
