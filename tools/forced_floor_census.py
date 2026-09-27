#!/usr/bin/env python3
"""Report colour-floor claims without turning prose into assignment authority.

Legacy prose is advisory (needs-review), regardless of document order, score or
exhaustion wording. A decisive record is one committed single-line comment:

    <!-- colour-exhaustion-v1 {JSON object} -->

The object supplies symbol, source, source_sha256, source_context_sha256,
overlay (integer or null), target_size_bytes, search_scope (every-colour),
result (exhausted or zero-floor), and floor. The configured full-TU fingerprint
is the existing nm_ranking source context: source, includes, command, compiler,
target and tools. It is recomputed, never inferred from an unchanged score.
The exact receipt line's Git blame supplies its evidence commit; uncommitted,
ambiguous, contradictory or stale records remain needs-review. A record does
not grant a reopen. Colour exhaustion never excludes structural work.

No compiler is invoked. Raw captures and machine words are never persisted.
"""
from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
HANDOFFS = ROOT / "docs" / "matching-triage-handoffs"
SOURCES = ROOT / "src"
RANKING = ROOT / "config" / "nonmatching-ranking.us.json"
DOC = ROOT / "docs" / "forced-floor-census.md"

# A floor sentence must be about colour forcing: "floor" alone also describes
# frame sizes, declaration counts and a whole family's structural minimum.
COLOUR_CONTEXT = re.compile(
    r"colou?r|lattice|landscape|force|forcing|probes?\b|winners?|every-colour|"
    r"\bwebs?\b", re.I)
NUMBER = r"~?(\d[\d,]*)(?:/\d[\d,]*)?"
FLOOR_PATTERNS = (
    # "colour floor 28", "lattice floor is 14", "single-force floor is 90/355",
    # "measured colour floor is therefore 71", "landscape floor of 39"
    re.compile(
        r"\bfloor\s+(?:is\s+|of\s+|at\s+|remains\s+|=\s*)?"
        r"(?:therefore\s+|exactly\s+|still\s+)?" + NUMBER, re.I),
    # "floors at 2", "floor at exactly 9"
    re.compile(r"\bfloors?\s+at\s+(?:exactly\s+)?" + NUMBER, re.I),
    # "no accepted force beat 14"
    re.compile(r"\bno accepted force beat\s+" + NUMBER, re.I),
    # "zero winners of 28" -- a score; "0 winners of 172 probes" is a count
    re.compile(r"\b(?:zero|0|no) winners of\s+" + NUMBER + r"(?![\d,/]|\s*probes)", re.I),
)
PROVED = re.compile(
    r"no (?:zero|0)-scor(?:ing|e) force"
    r"|winners? list is empty"
    # "zero winners", "0 winners", "no winners list" -- but not "delta-0
    # winners", which is a list of winners at delta 0.
    r"|(?<![\w-])(?:zero|0|no) winners\b"
    r"|no accepted force (?:beat|reached)"
    r"|colou?r cannot close",
    re.I)
SECTION_HEADER = re.compile(r"^#{2,6}\s")
SRC_BLOCK = re.compile(r"/\*\s*PLATEAU-HANDOFF(?::(?P<keyed>[A-Za-z_]\w*):start)?"
                       r"(?P<body>.*?)\*/", re.DOTALL)
SRC_SYMBOL = re.compile(r"^\s*\*?\s*symbol:\s*([A-Za-z_]\w*)\s*$", re.M)


@dataclasses.dataclass
class Claim:
    floor: int
    proved: bool
    position: int          # display location only; never establishes chronology
    sentence: str = ""     # the claim, for --why; never written to the doc
    proof: str = ""        # the exhaustion phrase that proved it, if any


@dataclasses.dataclass
class Row:
    symbol: str
    handoff: str           # repository-relative path
    floor: int
    proved: bool
    base: int | None       # current masked words, None when not queued
    size_delta: int | None
    size_bytes: int | None
    commit: str | None
    status: str = ""
    sentence: str = ""
    proof: str = ""
    reason: str = "unbound prose claim"
    bound: bool = False
    claims: list[int] = dataclasses.field(default_factory=list)
    search_scope: str | None = None


def sentences(section: str) -> list[str]:
    """One claim unit per bullet line, else per sentence of a paragraph."""
    units: list[str] = []
    paragraph: list[str] = []

    def flush() -> None:
        if paragraph:
            text = " ".join(paragraph)
            units.extend(u for u in re.split(r"(?<=[.;])\s+(?=[A-Z`(*])", text) if u)
            paragraph.clear()

    for raw in section.splitlines():
        line = raw.strip().lstrip("*").strip()
        if not line:
            flush()
            continue
        if line.startswith("- "):
            flush()
            units.append(line[2:])
            continue
        paragraph.append(line)
    flush()
    return units


def section_claims(section: str, start: int) -> list[Claim]:
    """Advisory claims; exhaustion wording must belong to the same claim unit."""
    clean = section.replace("**", "").replace("`", "")
    out = []
    for offset, unit in enumerate(sentences(clean)):
        if not COLOUR_CONTEXT.search(unit):
            continue
        proof = PROVED.search(unit)
        found: list[tuple[int, int]] = []
        for pattern in FLOOR_PATTERNS:
            for match in pattern.finditer(unit):
                found.append((match.start(), int(match.group(1).replace(",", ""))))
        for position, value in sorted(found):
            out.append(Claim(value, proof is not None,
                             start + offset * 1000 + position, unit,
                             proof.group(0) if proof else ""))
    return out


def text_claims(text: str) -> list[Claim]:
    """Claims in a handoff, sectioned by markdown headers."""
    sections: list[list[str]] = [[]]
    for line in text.splitlines():
        if SECTION_HEADER.match(line):
            sections.append([])
        sections[-1].append(line)
    claims: list[Claim] = []
    for index, lines in enumerate(sections):
        claims.extend(section_claims("\n".join(lines), index * 10 ** 7))
    return claims


def shard_texts() -> dict[str, tuple[str, str]]:
    out = {}
    for path in sorted(HANDOFFS.glob("*.md")):
        out[path.stem] = (path.relative_to(ROOT).as_posix(),
                          path.read_text(encoding="utf-8", errors="replace"))
    return out


def source_texts() -> dict[str, list[tuple[str, str]]]:
    out: dict[str, list[tuple[str, str]]] = {}
    for path in sorted(SOURCES.rglob("*.c")):
        text = path.read_text(encoding="utf-8", errors="replace")
        if "PLATEAU-HANDOFF" not in text:
            continue
        for match in SRC_BLOCK.finditer(text):
            body = match.group("body")
            symbol = match.group("keyed")
            if symbol is None:
                named = SRC_SYMBOL.search(body)
                symbol = named.group(1) if named else None
            if symbol:
                out.setdefault(symbol, []).append(
                    (path.relative_to(ROOT).as_posix(), body))
    return out


RECEIPT = re.compile(r"^\s*<!-- colour-exhaustion-v1 (.*?) -->\s*$", re.M)


def receipt_commit(path: str, line: str) -> str | None:
    """Pin the exact committed claim, not the shard's unrelated last edit."""
    import nm_ranking as nm
    try:
        matches = [(commit, content) for commit, content in
                   nm.blamed_source_lines("HEAD", path) if content.strip() == line.strip()]
    except nm.RankingDocumentError:
        return None
    if len(matches) != 1:
        return None
    return matches[0][0]


def current_contexts(records: list[dict]) -> dict[tuple[str, str], str]:
    import nm_ranking as nm
    import permute_batch as pb
    keys = {(r.get("source"), r.get("symbol")) for r in records}
    items = [item for item in pb.discover_queue()
             if (item.rel_c_file, item.func) in keys]
    return nm.current_source_contexts(items) if items else {}


def authenticate(record: dict, entry: dict | None, contexts: dict,
                 evidence_commit: str | None) -> str | None:
    """Return a refusal reason, or None for a current, committed receipt."""
    required = {"symbol", "source", "source_sha256", "source_context_sha256",
                "overlay", "target_size_bytes", "search_scope", "result", "floor"}
    if set(record) != required:
        return "invalid receipt fields"
    if any(not isinstance(record[k], str) for k in
           ("symbol", "source", "source_sha256", "source_context_sha256", "search_scope", "result")):
        return "invalid receipt field types"
    if not evidence_commit:
        return "receipt is not uniquely committed"
    if not entry:
        return "symbol is not queued"
    if (record["symbol"] != entry["name"] or record["source"] != entry.get("file")
            or record["overlay"] != entry.get("overlay")
            or type(record["target_size_bytes"]) is not int
            or record["target_size_bytes"] != entry["size_bytes"]):
        return "ownership mismatch"
    if record["search_scope"] != "every-colour":
        return "unsupported search scope"
    if (type(record["floor"]) is not int or record["floor"] < 0
            or record["result"] not in {"exhausted", "zero-floor"}
            or (record["result"] == "zero-floor") != (record["floor"] == 0)):
        return "invalid result"
    source = pathlib.PurePosixPath(record["source"])
    if source.is_absolute() or ".." in source.parts or not str(source).startswith("src/"):
        return "invalid source path"
    try:
        digest = hashlib.sha256((ROOT / source).read_bytes()).hexdigest()
    except OSError:
        return "source unavailable"
    if record["source_sha256"] != digest:
        return "source changed"
    context = contexts.get((record["source"], record["symbol"]))
    if not context or record["source_context_sha256"] != context:
        return "configured source/compiler context changed or unavailable"
    if entry.get("source_context_sha256") != context:
        return "ranking context is stale"
    if entry.get("size_delta") != 0 or entry["relocation_masked_differing_words"] < record["floor"]:
        return "ranking contradicts the receipt"
    return None


def classify(row: Row) -> str:
    if row.base is None:
        return "not-queued"
    if not row.bound:
        return "needs-review"
    return "zero-floor" if row.floor == 0 else "colour-exhausted"


def census(*, ranking: dict[str, dict] | None = None,
           shards: dict[str, tuple[str, str]] | None = None,
           sources: dict[str, list[tuple[str, str]]] | None = None,
           commits: bool = True) -> list[Row]:
    """Prose remains advisory; only current committed receipts affect colour routing."""
    if ranking is None:
        ranking = {r["name"]: r for r in json.loads(RANKING.read_text())["functions"]}
    shards = shard_texts() if shards is None else shards
    sources = source_texts() if sources is None else sources
    documents = {symbol: list(sources.get(symbol, ())) for symbol in set(shards) | set(sources)}
    for symbol, document in shards.items():
        documents[symbol].insert(0, document)
    parsed = {}
    all_records = []
    for symbol, docs in documents.items():
        records = []
        for path, text in docs:
            for match in RECEIPT.finditer(text):
                try:
                    record = json.loads(match.group(1))
                    if not isinstance(record, dict):
                        raise ValueError("receipt must be an object")
                except (ValueError, TypeError):
                    record = {}
                records.append((record, path, match.group(0)))
                all_records.append(record)
        parsed[symbol] = records
    context_error = None
    try:
        contexts = current_contexts(all_records) if all_records else {}
    except Exception as error:
        contexts = {}
        context_error = f"context unavailable: {type(error).__name__}: {error}"
    rows = []
    for symbol, docs in sorted(documents.items()):
        claims = [(claim, path) for path, text in docs for claim in text_claims(text)]
        records = parsed[symbol]
        if not claims and not records:
            continue
        entry = ranking.get(symbol)
        # Display a deterministic representative only. All observed floors are
        # retained, so prepending or appending history cannot silently supersede it.
        claim, path = min(claims, key=lambda cp: (cp[0].floor, cp[1])) if claims else (Claim(0, False, 0), docs[0][0])
        row = Row(symbol, path, claim.floor, claim.proved,
                  entry["relocation_masked_differing_words"] if entry else None,
                  entry.get("size_delta") if entry else None,
                  entry["size_bytes"] if entry else None, None,
                  sentence=claim.sentence, proof=claim.proof,
                  claims=sorted({c.floor for c, _ in claims}))
        if len(row.claims) > 1:
            row.reason = "conflicting unbound prose floors"
        valid = []
        refusals = []
        for record, record_path, line in records:
            commit = receipt_commit(record_path, line)
            reason = context_error or authenticate(record, entry, contexts, commit)
            if record.get("symbol") != symbol:
                reason = "receipt symbol differs from handoff owner"
            if reason:
                refusals.append(reason)
            else:
                valid.append((record, record_path, commit))
        if valid and not refusals:
            results = {(r["floor"], r["result"], r["search_scope"]) for r, _, _ in valid}
            if len(results) == 1:
                record, row.handoff, row.commit = valid[0]
                row.floor = record["floor"]
                row.proved = True
                row.bound = True
                row.search_scope = record["search_scope"]
                row.reason = "current committed source/context-bound receipt"
            else:
                row.reason = "conflicting current receipts"
        elif refusals:
            row.reason = "; ".join(sorted(set(refusals)))
        row.status = classify(row)
        rows.append(row)
    # Legacy path history is navigation only; it never becomes an evidence pin.
    return rows


def colour_exhausted(rows: list[Row] | None = None) -> dict[str, Row]:
    """Symbols triage must not route to a colour lane."""
    rows = census(commits=False) if rows is None else rows
    return {r.symbol: r for r in rows if r.status == "colour-exhausted"}


STATUS_ORDER = ("colour-exhausted", "zero-floor", "needs-review", "not-queued")


def summary(rows: list[Row]) -> list[dict]:
    out = []
    for status in STATUS_ORDER:
        sel = [r for r in rows if r.status == status]
        out.append({"status": status, "functions": len(sel),
                    "bytes": sum(r.size_bytes or 0 for r in sel)})
    return out


def render_doc(rows: list[Row]) -> str:
    queued = [r for r in rows if r.status != "not-queued"]
    exhausted = [r for r in rows if r.status == "colour-exhausted"]
    lines = [
        "# Forced-floor census",
        "",
        "Generated by `gmake forced-floor-census` (`tools/forced_floor_census.py`)",
        "from the plateau handoffs and `config/nonmatching-ranking.us.json`. Do not",
        "edit by hand; regenerate. The tool's docstring defines every status.",
        "",
        f"**{len(exhausted)} queued functions, "
        f"{sum(r.size_bytes or 0 for r in exhausted):,} bytes, are colour-exhausted**:",
        "Only committed source/context-bound receipts can exclude colour work.",
        "Legacy prose is advisory and needs review. Structural visibility and",
        "the independent lane_status assignment gate are unchanged.",
        "",
        "| status | functions | bytes |",
        "|---|---:|---:|",
    ]
    for entry in summary(rows):
        lines.append(f"| {entry['status']} | {entry['functions']} | {entry['bytes']:,} |")
    lines += [
        "",
        "Queued functions with a recorded floor:",
        "",
        "| symbol | bytes | delta | base | floor | proved | status | evidence commit |",
        "|---|---:|---:|---:|---:|:-:|---|---|",
    ]
    for row in sorted(queued, key=lambda r: (STATUS_ORDER.index(r.status),
                                             -(r.size_bytes or 0), r.symbol)):
        lines.append(
            f"| `{row.symbol}` | {row.size_bytes:,} | {row.size_delta:+d} | "
            f"{row.base} | {row.floor} | {'yes' if row.proved else 'no'} | "
            f"{row.status} | {row.commit or '--'} |")
    return "\n".join(lines) + "\n"


def render_text(rows: list[Row], show_all: bool) -> str:
    out = []
    for entry in summary(rows):
        out.append(f"{entry['status']:<17} {entry['functions']:>4} fns "
                   f"{entry['bytes']:>9,} B")
    out.append("")
    for row in rows:
        if row.status == "not-queued" and not show_all:
            continue
        out.append(f"{row.status:<17} {row.symbol:<40} floor {row.floor:>5} "
                   f"base {str(row.base):>5} delta {str(row.size_delta):>5} "
                   f"proved {'y' if row.proved else 'n'} {row.commit or '--'}")
    return "\n".join(out)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--all", action="store_true",
                    help="also list symbols no longer queued")
    ap.add_argument("--why", metavar="SYMBOL",
                    help="print the sentence and exhaustion phrase behind a row")
    ap.add_argument("--write", type=pathlib.Path, metavar="PATH",
                    help="write the tracked summary (docs/forced-floor-census.md)")
    ap.add_argument("--check", type=pathlib.Path, metavar="PATH",
                    help="exit 1 if PATH differs from what --write would produce")
    args = ap.parse_args(argv)
    rows = census()
    if args.why:
        for row in rows:
            if row.symbol == args.why:
                print(f"{row.status}: floor {row.floor}, base {row.base}, "
                      f"delta {row.size_delta}, {row.handoff} @ {row.commit}")
                print(f"  claim: {row.sentence}")
                print(f"  proof: {row.proof or '(none stated)'}")
                print(f"  binding: {row.reason}; prose floors: {row.claims}")
                return 0
        print(f"no recorded colour floor for {args.why}", file=sys.stderr)
        return 1
    if args.check:
        current = args.check.read_text(encoding="utf-8") if args.check.is_file() else ""
        if current != render_doc(rows):
            print(f"{args.check} is stale: run gmake forced-floor-census",
                  file=sys.stderr)
            return 1
        print(f"{args.check}: up to date")
        return 0
    if args.write:
        args.write.write_text(render_doc(rows), encoding="utf-8")
        exhausted = [r for r in rows if r.status == "colour-exhausted"]
        print(f"wrote {args.write}: {len(exhausted)} colour-exhausted, "
              f"{sum(r.size_bytes or 0 for r in exhausted):,} bytes")
        return 0
    if args.json:
        print(json.dumps({"summary": summary(rows),
                          "rows": [dataclasses.asdict(r) for r in rows]}, indent=2))
    else:
        print(render_text(rows, args.all))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
