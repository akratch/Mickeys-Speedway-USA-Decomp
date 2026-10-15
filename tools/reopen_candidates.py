#!/usr/bin/env python3
"""List still-queued functions whose closure predates a compiler law.

    tools/reopen_candidates.py [--since YYYY-MM-DD] [--top N] [--json]

A plateau handoff records that a search came back flat. It is evidence about
**the lever set it was measured against**, not about the function. When a new
law lands, every closure written before it becomes a re-open candidate on its
face -- and that is computable rather than arguable.

It stopped being hypothetical on 2026-09-10: `func_overlay_041_F0000854`
carried a handoff recording its C-body search as closed, no reopen
authorization existed for it, and it matched exactly, in one lane, on the first
attempt after three new laws landed. The winning edit was never in the space
the closure searched. Its own evidence was never wrong -- only its scope was.
Reaching it took a hunch; this makes it a query.

A row here is a prompt to re-read a closure, never permission to skip it. The
useful output of working one is often "this closure was correct and remains
correct under the new laws", which is worth recording as explicitly as a match.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
HANDOFFS = ROOT / "docs" / "matching-triage-handoffs"
RANKING = ROOT / "config" / "nonmatching-ranking.us.json"

# Constructs each law reaches. A closure whose text mentions one was searching
# in territory that law now describes, so it is worth re-reading.
# Laws whose subject is a loop. A closure's prose mentions "loop" for all sorts
# of reasons, so for these the function's own body has to contain one before
# the tag means anything. Without this check the pool tagged a function with no
# loop and no induction variable at all, and a lane spent probes proving the
# law could not apply -- as it put the criticism, the tag was "a date
# comparison, not an applicability test".
LOOP_LAWS = {"L90 induction exit test", "L93 loop shape"}

LAW_CONSTRUCTS = {
    "L90 induction exit test": re.compile(
        r"\bloop\b|\bcounter\b|induction|exit test|\bfor\b|\bwhile\b", re.I),
    "L92 operand weight": re.compile(
        r"commut|operand order|address (?:sum|arith)|\bindex(?:ed)?\b|scaled", re.I),
    "L93 loop shape": re.compile(
        r"do\s*\{|do/while|top-tested|bottom-tested|loop shape|\blatch\b", re.I),
    "L94 co-region reads": re.compile(
        r"\bextern\b|\bimport\b|global address|two reads|%hi|materiali", re.I),
    # 2026-09-10 laws. L106 is the widest of these on purpose: a closure that
    # measured declaration ORDER was measuring the declaration list, which
    # decides stack homes, and never the position of a defining STATEMENT,
    # which decides web numbers. One such closure recorded 4,190 flat forms and
    # the function matched on a single hoisted definition.
    #
    # But an L106 tag is the one on this list that most needs verifying before a
    # lane is spent on it, and the check is one compile. L106 only has an axis
    # where the procedure emits p2 allocator records, and four large procedures
    # measured on 2026-09-10 emitted 428, 473, 1218 and 1655 p1 decisions and
    # ZERO p2 -- one TU emits p2 only in its small procedures. Two lanes were
    # sent after an L106 lever on functions that turned out to have no p2 sweep
    # at all. Run the phase census first (see L108); this tag is a prompt to
    # ask, never an answer.
    "L100 save ratio": re.compile(
        r"totalsave|globalcolor|caller-saved|\bnocs\b|save ratio", re.I),
    "L101 call-result span": re.compile(
        r"return register|call result|p1cost|candidate list|"
        r"declined (?:silently|rather)|forbidden", re.I),
    "L102 rematerialised value": re.compile(
        r"remateriali|forward-?substitut|manufactur|name it to", re.I),
    "L103 float pool spelling": re.compile(
        r"\.rodata|float (?:constant|pool|literal)|constant pool", re.I),
    "L104 surviving copy": re.compile(
        r"dead copy|copy[- ]propagat|folded away|redundant copy", re.I),
    "L105 call block": re.compile(
        r"basic block|join block|early[- ]exit|block membership", re.I),
    "L106 definition order": re.compile(
        r"declaration order|declared? (?:above|below|before|after)|web number|"
        r"colour order|color order|first[- ]definition", re.I),
    "L107 region reassociation": re.compile(
        r"reassociat|region boundary|\bif \(1\)|do\s*\{\s*\}\s*while", re.I),
    # 2026-09-11/12 laws. L140 is deliberately the widest on this list, and
    # for a different reason from L106's breadth: it is not a new lever but a
    # retraction of evidence. CDX_FORCE is silently ignored unless CDX_PROC is
    # also set (Trap 20), recording forced=-2 and returning a byte-identical
    # object indistinguishable from a legitimate decline -- so ANY closure
    # resting on a force sweep that did not record acceptance is unproven
    # rather than negative. One was re-run and a decision it called
    # unreachable scored 6 at delta 0. Checking is cheap: re-run one cell with
    # CDX_PROC and read the forced value.
    "L140 unverified force sweep": re.compile(
        r"CDX_FORCE|force sweep|forcing web|forced (?:it|the|each|every)|"
        r"never beats|force cells|force grid", re.I),
    "L142 call arity offer set": re.compile(
        r"p1cost|argument register|\bariti?y\b|spanned call|forbidden0?\b|"
        r"available0?\b|not offered|never offered", re.I),
    "L144 reload without volatile": re.compile(
        r"volatile|reload(?:s|ed|ing)?\b|memory[- ]class|storage class", re.I),
    "L139 totalsave against bestcost": re.compile(
        r"totalsave|bestcost|\bsplit the web|web split", re.I),
    "L131 range per IR name": re.compile(
        r"live range|common ?subexpression|\bCSE\b|expression temp|"
        r"same (?:address )?expression", re.I),
}


UNASSIGNABLE = ROOT / "config" / "unassignable-symbols.us.json"


def barred() -> dict[str, dict]:
    """Symbols with a proof that no legal source reaches the target.

    They must never be surfaced here. A reopen candidate is ranked by how
    closed-yet-cheap it looks, which is exactly the shape a proven-unmatchable
    function has -- large, nearly exact, and therefore top of any list. One sat
    first in this tool's output while carrying a proof that it cannot match.
    """
    if not UNASSIGNABLE.exists():
        return {}
    return json.loads(UNASSIGNABLE.read_text(encoding="utf-8"))["symbols"]


def queued() -> dict[str, dict]:
    document = json.loads(RANKING.read_text(encoding="utf-8"))
    bars = barred()
    return {row["name"]: row for row in document["functions"]
            if row["name"] not in bars}


def closure_date(path: pathlib.Path) -> str:
    """The date the handoff's evidence last actually changed."""
    return subprocess.run(
        ["git", "log", "-1", "--format=%ad", "--date=short", "--", str(path)],
        cwd=ROOT, capture_output=True, text=True).stdout.strip()


def body_has_loop(path: str, symbol: str) -> bool | None:
    """True/False if the symbol's body could be read; None if it could not.

    Deliberately returns None rather than False on a parse failure: an
    unreadable body is not evidence that a law does not apply, and dropping a
    candidate on that basis would hide work rather than route it.
    """
    source = ROOT / path
    try:
        text = source.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    opening = re.search(
        r"^[A-Za-z_][^\n;]*\b%s\s*\([^;]*\)\s*\{" % re.escape(symbol),
        text, re.M)
    if not opening:
        return None
    index, depth = opening.end(), 1
    while index < len(text) and depth:
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
        index += 1
    return bool(re.search(r"\b(for|while|do)\b", text[opening.end():index]))


def candidates(since: str) -> list[dict]:
    open_functions = queued()
    rows = []
    for path in sorted(HANDOFFS.glob("*.md")):
        if ".archive-" in path.stem:      # folded into its shard's text below
            continue
        symbol = path.stem
        row = open_functions.get(symbol)
        if row is None:
            continue                      # matched already; nothing to re-open
        date = closure_date(path)
        if not date or date >= since:
            continue                      # written with the laws already in hand
        text = path.read_text(encoding="utf-8", errors="replace")
        for archive in sorted(HANDOFFS.glob(f"{symbol}.archive-*.md")):
            text += "\n" + archive.read_text(encoding="utf-8", errors="replace")
        laws = [name for name, rx in LAW_CONSTRUCTS.items() if rx.search(text)]
        if any(name in LOOP_LAWS for name in laws):
            # A row without a file, like a row whose body will not parse, is
            # "cannot tell" and keeps its tag. Dropping a candidate on missing
            # evidence would hide work rather than route it.
            path = row.get("file")
            has_loop = body_has_loop(path, symbol) if path else None
            if has_loop is False:
                laws = [name for name in laws if name not in LOOP_LAWS]
        if not laws:
            continue
        masked = row.get("relocation_masked_differing_words")
        rows.append({
            "symbol": symbol,
            "size_bytes": row["size_bytes"],
            "masked_differing_words": masked,
            "size_delta": row.get("size_delta"),
            "closed": date,
            "laws": laws,
            "bytes_per_word": row["size_bytes"] / masked if masked else None,
        })
    rows.sort(key=lambda r: -(r["bytes_per_word"] or 0))
    return rows


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--since", default="2026-09-10",
                        help="closures strictly before this date are candidates "
                             "(default: the date L90-L94 landed)")
    parser.add_argument("--top", type=int, default=20)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    rows = candidates(args.since)
    if args.json:
        print(json.dumps({"since": args.since, "candidates": rows},
                         indent=2, sort_keys=True))
        return 0
    if not rows:
        print(f"no still-queued closures predate {args.since}")
        return 0
    print(f"{len(rows)} candidate(s), {sum(r['size_bytes'] for r in rows)} bytes, "
          f"closures before {args.since}\n")
    print("%-42s %7s %7s %6s %-11s %s" % (
        "symbol", "bytes", "masked", "delta", "closed", "laws now applicable"))
    for row in rows[:args.top]:
        print("%-42s %7d %7s %6s %-11s %s" % (
            row["symbol"][:42], row["size_bytes"],
            row["masked_differing_words"], row["size_delta"], row["closed"],
            ", ".join(law.split()[0] for law in row["laws"])))
    if len(rows) > args.top:
        print(f"\n... and {len(rows) - args.top} more (--top to widen)")
    print("\nA row is a prompt to re-read a closure, not permission to skip it.\nA law tag means the closure's evidence predates that law and touches what it\ndescribes -- not that the law applies. Verify applicability before spending a lane.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
