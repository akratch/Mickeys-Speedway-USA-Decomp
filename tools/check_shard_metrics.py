#!/usr/bin/env python3
"""Reconcile plateau-handoff shard headers against the measured ranking.

`plateau_handoff_audit.py` compares each shard to the PLATEAU-HANDOFF marker
in its source file. Both are written together by `finalize_plateau.py`, so
they agree with each other by construction -- and can both disagree with what
the function actually measures today. A later lane that improves a number
without re-running `finalize_plateau` leaves the pair self-consistent and
wrong, and the audit reports `current`.

Two failure modes follow, and both have cost real work:

**A matched function whose shard still claims a residual.** When a function
matches it leaves `config/nonmatching-ranking.us.json` entirely, and its
shard becomes unbacked -- a state the audit counts but does not check. Its
header goes on advertising an open score. A lane handed that shard plans an
attempt on a function that is already byte-exact. Sixteen shards were in this
state on 2026-09-16, including one matched earlier the same day.

**A queued function whose header disagrees with the ranking.** The `score`
and `first mismatch` fields drift as later work moves the numbers. A lane
that trusts the header measures something else on its first cycle and spends
the difference working out which number is real.

    python3 tools/check_shard_metrics.py [--json]

Exit 1 on any disagreement, 0 otherwise. Reads the ranking JSON and the shard
headers only: no build, no compile, nothing ROM-derived.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import sys

REPO = pathlib.Path(__file__).resolve().parent.parent
RANKING = REPO / "config" / "nonmatching-ranking.us.json"
SHARD_DIR = REPO / "docs" / "matching-triage-handoffs"

# The header block finalize_plateau writes; only the first lines are scanned,
# so a later section quoting an old score is history and not a claim.
HEADER_LINES = 16

SCORE_RE = re.compile(r"^- score: (.+)$", re.M)
FIRST_RE = re.compile(r"^- first mismatch: (.+)$", re.M)
# "12/345 words" -- differing over total. finalize_plateau also emits the
# bare "N differing words" form, which carries no total to check against.
PAIR_RE = re.compile(r"^(\d+)\s*/\s*(\d+)\b")
BARE_RE = re.compile(r"^(\d+)\s+differing words\b")


def header_of(path: pathlib.Path) -> str:
    with path.open(encoding="utf-8") as handle:
        return "".join(line for _, line in zip(range(HEADER_LINES), handle))


def parse_offset(text: str) -> "int | None":
    text = text.strip()
    if text in {"none", "unknown"}:
        return None
    try:
        return int(text, 16) if text.startswith(("+0x", "0x")) else int(text, 16)
    except ValueError:
        return None


def claims_a_residual(score: str, first: str) -> bool:
    """True when the header advertises work still to do.

    A matched shard reads N/N words, or `none` for its first mismatch, or
    zero differing words. Anything else is a claim that the function is open.
    """
    if first.strip() == "none":
        return False
    pair = PAIR_RE.match(score.strip())
    if pair:
        return pair.group(1) != pair.group(2)
    bare = BARE_RE.match(score.strip())
    if bare:
        return bare.group(1) != "0"
    return False  # an unrecognised spelling is not evidence of a claim


def check() -> "list[dict]":
    ranking = json.loads(RANKING.read_text(encoding="utf-8"))
    queued = {row["name"]: row for row in ranking["functions"]}
    findings: list[dict] = []

    for path in sorted(SHARD_DIR.glob("*.md")):
        symbol = path.stem
        head = header_of(path)
        score_m = SCORE_RE.search(head)
        first_m = FIRST_RE.search(head)
        if not score_m:
            continue
        score = score_m.group(1).strip()
        first = first_m.group(1).strip() if first_m else "unknown"
        rel = path.relative_to(REPO).as_posix()

        row = queued.get(symbol)
        if row is None:
            if claims_a_residual(score, first):
                findings.append({
                    "kind": "matched-but-open",
                    "shard": rel,
                    "symbol": symbol,
                    "claim": f"score: {score}",
                    "actual": "absent from the ranking (matched, or never queued)",
                })
            continue

        masked = row["relocation_masked_differing_words"]
        pair = PAIR_RE.match(score)
        bare = BARE_RE.match(score)
        # `N/M words` is written both ways in this corpus: some shards put the
        # DIFFERING count first, some the MATCHED count. Both are in use and
        # neither is wrong, so a header is only drifted when it agrees with
        # the ranking under NEITHER reading. Assuming one convention reported
        # 31 false positives out of 43 on this tree.
        if pair:
            differing, total = int(pair.group(1)), int(pair.group(2))
            agrees = differing == masked or total - differing == masked
            claimed = None if agrees else differing
        elif bare:
            claimed = int(bare.group(1))
        else:
            claimed = None
        if claimed is not None and claimed != masked:
            findings.append({
                "kind": "score",
                "shard": rel,
                "symbol": symbol,
                "claim": f"score: {score}",
                "actual": f"{masked} masked differing words",
            })

        # A header may legitimately quote either the masked or the raw first
        # mismatch; the ranking carries both. Only a value matching neither is
        # drift. Comparing against the masked offset alone reported 6 false
        # positives out of 33.
        claimed_off = parse_offset(first)
        offsets = {
            row.get("relocation_masked_first_mismatch_offset"),
            row.get("first_mismatch_offset"),
        } - {None}
        actual_off = row.get("relocation_masked_first_mismatch_offset")
        if claimed_off is not None and offsets and claimed_off not in offsets:
            findings.append({
                "kind": "first-mismatch",
                "shard": rel,
                "symbol": symbol,
                "claim": f"first mismatch: {first}",
                "actual": f"+0x{actual_off:X}",
            })

    return findings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--json", action="store_true", help="machine-readable output")
    args = parser.parse_args()

    findings = check()
    if args.json:
        print(json.dumps(findings, indent=2))
    else:
        order = {"matched-but-open": 0, "score": 1, "first-mismatch": 2}
        for f in sorted(findings, key=lambda f: (order[f["kind"]], f["shard"])):
            print(f"{f['kind']}: {f['shard']}")
            print(f"    claims  {f['claim']}")
            print(f"    ranking {f['actual']}")
        counts = {k: sum(1 for f in findings if f["kind"] == k) for k in order}
        if findings:
            print(
                f"\nshard metrics FAILED -- matched-but-open={counts['matched-but-open']} "
                f"score={counts['score']} first-mismatch={counts['first-mismatch']}"
            )
            print("Re-run tools/finalize_plateau.py for the symbol, or record the match.")
        else:
            print("shard metrics OK -- every header agrees with the ranking")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
