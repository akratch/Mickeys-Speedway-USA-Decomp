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
    python3 tools/check_shard_metrics.py --write

Exit 1 on any disagreement, 0 otherwise. Reads the ranking JSON and the shard
headers only: no build, no compile, nothing ROM-derived. `gmake check-docs`
runs it, so drift fails the docs gate rather than waiting for a lane to trip
over it.

**The ranking is the fresher measurement.** Its rows carry the source-context
hash `nm_ranking.py --check-doc` holds to the tree, so a row is always a
measurement of today's candidate; a header is a measurement of the candidate
on the day its plateau was recorded. The ranking's `relocation_masked_*`
fields are the same positional count `function_preflight.py` reports and
`finalize_plateau.py` records (checked on three remeasures, 2026-09-23), so
where they disagree the header is what moved. A bare `N differing words` is
also accepted against the ranking's unmasked count, as offsets already were:
some plateaus were recorded before masking.

`--write` regenerates the drifted fields from the ranking: it rewrites the
`score` / `first-mismatch` lines of the symbol's source PLATEAU-HANDOFF marker,
projects the shard from it with `plateau_handoff_audit` (the shard's evidence
prose is kept), and appends one line to that prose saying what the header read
before. Frame, relocations and summary are not the ranking's to change and
are left alone. `matched-but-open` findings are not rewritten: a matched
function's shard needs a human decision, not a number. It refuses a dirty
source or shard. Every rewritten shard moves its ledger commit, so run
`tools/authorize_reopen.py --refresh-stale` after the commit lands.
"""

from __future__ import annotations

import argparse
import datetime
import json
import pathlib
import re
import subprocess
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


def check(root: pathlib.Path = REPO) -> "list[dict]":
    ranking = json.loads(
        (root / RANKING.relative_to(REPO)).read_text(encoding="utf-8"))
    queued = {row["name"]: row for row in ranking["functions"]}
    findings: list[dict] = []

    for path in sorted((root / SHARD_DIR.relative_to(REPO)).glob("*.md")):
        symbol = path.stem
        head = header_of(path)
        score_m = SCORE_RE.search(head)
        first_m = FIRST_RE.search(head)
        if not score_m:
            continue
        score = score_m.group(1).strip()
        first = first_m.group(1).strip() if first_m else "unknown"
        rel = path.relative_to(root).as_posix()

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
        raw = row.get("differing_words")
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
            if claimed == raw:
                claimed = None
        else:
            claimed = None
        if claimed is not None and claimed != masked:
            findings.append({
                "kind": "score",
                "shard": rel,
                "symbol": symbol,
                "claim": f"score: {score}",
                "actual": f"{masked} masked differing words",
                "value": f"{masked} differing words",
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
                "value": f"+0x{actual_off:X}",
            })

    return findings


MARKER_FIELD = {"score": "score", "first-mismatch": "first-mismatch"}


class WriteRefused(RuntimeError):
    """--write would overwrite something it cannot account for."""


def _dirty(root: pathlib.Path, relative: str) -> bool:
    return bool(subprocess.run(
        ["git", "status", "--porcelain=v1", "--", relative], cwd=root,
        text=True, stdout=subprocess.PIPE, check=True,
    ).stdout)


def _rewrite_marker(text: str, line: int, fields: dict[str, str]) -> str:
    """Replace ``fields`` inside the marker comment that starts at ``line``."""
    lines = text.splitlines(keepends=True)
    seen: set[str] = set()
    for index in range(line - 1, len(lines)):
        row = lines[index]
        for key, value in fields.items():
            if row.startswith(f" * {key}: "):
                lines[index] = f" * {key}: {value}\n"
                seen.add(key)
        if row.rstrip("\n") == " */":
            break
    missing = set(fields) - seen
    if missing:
        raise WriteRefused(
            f"marker at line {line} has no {', '.join(sorted(missing))} field")
    return "".join(lines)


def write(root: pathlib.Path, findings: "list[dict]", *,
          today: "str | None" = None) -> "list[str]":
    """Regenerate drifted header fields from the ranking. Returns paths."""
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
    import plateau_handoff_audit as pha

    today = today or datetime.date.today().isoformat()
    wanted: dict[str, dict[str, tuple[str, str]]] = {}
    for finding in findings:
        if finding["kind"] not in MARKER_FIELD:
            continue
        old = finding["claim"].split(": ", 1)[1]
        wanted.setdefault(finding["symbol"], {})[finding["kind"]] = (
            old, finding["value"])
    if not wanted:
        return []
    audit = pha.audit_tree(root)
    markers = {marker.symbol: marker for marker in audit.markers}
    edits: dict[str, str] = {}
    for symbol, fields in sorted(wanted.items()):
        marker = markers.get(symbol)
        if marker is None:
            raise WriteRefused(f"{symbol}: no structured source marker to rewrite")
        shard = SHARD_DIR.relative_to(REPO).as_posix() + f"/{symbol}.md"
        for relative in (marker.source, shard):
            if _dirty(root, relative):
                raise WriteRefused(f"{symbol}: {relative} has local changes")
        text = edits.get(marker.source) or (root / marker.source).read_text(
            encoding="utf-8")
        edits[marker.source] = _rewrite_marker(text, marker.line, {
            MARKER_FIELD[kind]: new for kind, (_old, new) in fields.items()
        })
    for relative, text in edits.items():
        (root / relative).write_text(text, encoding="utf-8", newline="\n")
    audit = pha.audit_tree(root)
    written = pha.write_actionable(root, audit)
    changed = set(edits)
    for symbol, fields in sorted(wanted.items()):
        shard = root / SHARD_DIR.relative_to(REPO) / f"{symbol}.md"
        before = "; ".join(
            f"{'first mismatch' if kind == 'first-mismatch' else kind} {old}"
            for kind, (old, _new) in sorted(fields.items()))
        note = (
            f"Header regenerated from the ranking on {today} "
            f"(check_shard_metrics --write); it read {before}.\n"
        )
        end = f"<!-- plateau-handoff:{symbol}:end -->"
        text = shard.read_text(encoding="utf-8")
        if end not in text:
            raise WriteRefused(f"{symbol}: shard has no end marker")
        head, tail = text.split(end, 1)
        if not head.endswith("\n\n"):
            head = head.rstrip("\n") + "\n\n"
        shard.write_text(head + note + end + tail, encoding="utf-8", newline="\n")
        changed.add(shard.relative_to(root).as_posix())
    return sorted(changed | set(written))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--json", action="store_true", help="machine-readable output")
    parser.add_argument(
        "--write", action="store_true",
        help="regenerate drifted score/first-mismatch fields from the ranking",
    )
    args = parser.parse_args()

    findings = check()
    if args.write:
        try:
            paths = write(REPO, findings)
        except WriteRefused as error:
            print(f"check_shard_metrics: {error}", file=sys.stderr)
            return 2
        for path in paths:
            print(f"wrote {path}")
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
            print("Re-run tools/finalize_plateau.py for the symbol, or record the match;")
            print("tools/check_shard_metrics.py --write regenerates score/first-mismatch drift from the ranking.")
        else:
            print("shard metrics OK -- every header agrees with the ranking")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
