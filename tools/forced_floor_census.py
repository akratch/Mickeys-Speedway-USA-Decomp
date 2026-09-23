#!/usr/bin/env python3
"""Census of forced colour floors recorded in plateau handoffs.

    tools/forced_floor_census.py [--json] [--all]
    tools/forced_floor_census.py --write docs/forced-floor-census.md
    tools/forced_floor_census.py --check docs/forced-floor-census.md
    tools/forced_floor_census.py --why SYMBOL
    gmake forced-floor-census

Lanes price a function's colour axis with `force_lattice.py` and the
`--every-colour` landscape, and write the result into the plateau handoff:
"the lattice floor is 14", "Colour floor 28 (202 probes, 0 winners)",
"No zero-scoring force", "Colour cannot close it". That sentence is the most
expensive fact in the shard -- an exhaustive landscape is minutes of compiles
per web -- and until now nothing read it back. Triage routed the same function
to the next colour lane, which re-derived the floor.

This tool reads every handoff shard under `docs/matching-triage-handoffs/` and
every `PLATEAU-HANDOFF` comment block under `src/`, and records per symbol:

* **floor** -- the last colour/lattice/force floor stated, in document order
  (the shard header's summary counts as the earliest statement);
* **base** -- the symbol's current masked score from
  `config/nonmatching-ranking.us.json`;
* **proved** -- whether the section that states that floor also states the
  search was exhausted: no zero-scoring force, an empty winner list, zero
  winners, no accepted force beating the floor, or "colour cannot close";
* **handoff commit** -- the last commit that touched the handoff.

and classifies it:

    colour-exhausted  proved, floor > 0, size delta 0, base >= floor.
                      Colour alone cannot close it; triage excludes it and
                      reports it, and only a structural lane should take it.
    zero-floor        a 0-scoring force exists: the L160 route, not exhausted.
    superseded        base < floor: the source moved past the recorded floor,
                      which was therefore measured on an older source.
    size-mismatch     floor recorded, but the size delta is nonzero today:
                      Track B, not a colour question.
    unproved          a floor is stated but no exhaustion claim accompanies it.
    not-queued        no longer in the ranking (matched, promoted or retired).

It reads prose, so it is a parser of claims, not a re-measurement: a row says
what a lane wrote down, and the handoff commit says where to check it. It
never reads assembly, build products or the baserom, and the tracked summary
it writes carries symbols, numbers and commits only.
"""
from __future__ import annotations

import argparse
import dataclasses
import json
import pathlib
import re
import subprocess
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
    position: int          # document order: later statements supersede earlier
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
    """Every colour-floor claim in one section, proved if the section says so."""
    clean = section.replace("**", "").replace("`", "")
    proof = PROVED.search(" ".join(clean.split()))
    out = []
    for offset, unit in enumerate(sentences(clean)):
        if not COLOUR_CONTEXT.search(unit):
            continue
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


def latest(claims: list[Claim]) -> Claim | None:
    return max(claims, key=lambda c: c.position) if claims else None


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


# Ten hex digits: unambiguous here, and never the 4/8/16-digit shape the
# clean-room sweep reads as a bare machine word (a column of 8-digit short
# hashes is indistinguishable from a word dump to that detector, rightly).
ABBREV = "--abbrev=10"


def last_commits(paths: set[str]) -> dict[str, str]:
    """Last commit touching each path, from one log walk per directory."""
    found: dict[str, str] = {}
    shard_dir = HANDOFFS.relative_to(ROOT).as_posix()
    wanted_shards = {p for p in paths if p.startswith(shard_dir + "/")}
    if wanted_shards:
        log = subprocess.run(
            ["git", "log", ABBREV, "--format=@%h", "--name-only", "--", shard_dir],
            cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        ).stdout
        current = None
        for line in log.splitlines():
            if line.startswith("@"):
                current = line[1:]
            elif line and current and line in wanted_shards and line not in found:
                found[line] = current
    for path in sorted(paths - wanted_shards):
        value = subprocess.run(
            ["git", "log", "-1", ABBREV, "--format=%h", "--", path],
            cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        ).stdout.strip()
        if value:
            found[path] = value
    return found


def classify(row: Row) -> str:
    if row.base is None:
        return "not-queued"
    if row.floor == 0:
        return "zero-floor"
    if row.size_delta:
        return "size-mismatch"
    if row.base < row.floor:
        return "superseded"
    if not row.proved:
        return "unproved"
    return "colour-exhausted"


def census(*, ranking: dict[str, dict] | None = None,
           shards: dict[str, tuple[str, str]] | None = None,
           sources: dict[str, list[tuple[str, str]]] | None = None,
           commits: bool = True) -> list[Row]:
    """Every symbol with a recorded colour floor, classified."""
    if ranking is None:
        ranking = {r["name"]: r for r in json.loads(
            RANKING.read_text(encoding="utf-8"))["functions"]}
    shards = shard_texts() if shards is None else shards
    sources = source_texts() if sources is None else sources
    rows: list[Row] = []
    for symbol in sorted(set(shards) | set(sources)):
        # The shard is the full record; a source block is its short summary.
        # Prefer the shard's claim, fall back to the source block's.
        best: tuple[Claim, str] | None = None
        if symbol in shards:
            path, text = shards[symbol]
            claim = latest(text_claims(text))
            if claim is not None:
                best = (claim, path)
        if best is None:
            for path, body in sources.get(symbol, ()):
                claim = latest(text_claims(body))
                if claim is not None:
                    best = (claim, path)
        if best is None:
            continue
        claim, path = best
        entry = ranking.get(symbol)
        row = Row(
            symbol=symbol, handoff=path, floor=claim.floor, proved=claim.proved,
            base=entry["relocation_masked_differing_words"] if entry else None,
            size_delta=entry.get("size_delta") if entry else None,
            size_bytes=entry["size_bytes"] if entry else None,
            commit=None, sentence=claim.sentence, proof=claim.proof,
        )
        row.status = classify(row)
        rows.append(row)
    if commits:
        found = last_commits({r.handoff for r in rows})
        for row in rows:
            row.commit = found.get(row.handoff)
    return rows


def colour_exhausted(rows: list[Row] | None = None) -> dict[str, Row]:
    """Symbols triage must not route to a colour lane."""
    rows = census(commits=False) if rows is None else rows
    return {r.symbol: r for r in rows if r.status == "colour-exhausted"}


STATUS_ORDER = ("colour-exhausted", "zero-floor", "unproved", "superseded",
                "size-mismatch", "not-queued")


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
        "a proved forced floor above zero at size delta 0. `tools/triage.py` excludes",
        "them from routes, clusters and bands and reports them; send them to a",
        "structural lane, not a colour sweep.",
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
        "| symbol | bytes | delta | base | floor | proved | status | handoff commit |",
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
