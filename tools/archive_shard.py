#!/usr/bin/env python3
"""Archive old dated sections of a plateau handoff shard.

A shard under docs/matching-triage-handoffs/ grows with every dated section and
the clean-room oversize rule (262,144 bytes) eventually stops the next note.
This tool moves the oldest dated sections, in order and byte-for-byte, into
`<symbol>.archive-<n>.md` (next free number) and leaves one pointer line in the
live shard. The live shard keeps its start/end markers, metric header, summary
and every section it does not move, so the shard grammar and audits still hold.

A dated section is a heading of level 3-6 whose text begins with YYYY-MM-DD. It
runs to the next heading of the same or a shallower level, or to the end
marker. Dated sections nested in another dated section move with it.

    tools/archive_shard.py func_800517E0 --keep-sections 20
    tools/archive_shard.py func_800517E0 --archive-before 2026-10-09 [--dry-run]

The archive file carries no markers: readers skip `*.archive-*.md` by name.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

import finalize_plateau as plateau

ROOT = Path(__file__).resolve().parent.parent
SIZE_LIMIT = 262144
HEADING_RE = re.compile(r"^(#{1,6}) (.*)$")
DATED_RE = re.compile(r"^(#{3,6}) (\d{4}-\d{2}-\d{2})\b")
POINTER_PREFIX = "Archived older dated sections:"


class ArchiveError(Exception):
    pass


def dated_sections(lines: list[str], end_marker_index: int) -> list[tuple[int, int, str]]:
    """Outermost dated sections as (first line, one past last line, date)."""
    sections: list[tuple[int, int, str]] = []
    index = 0
    while index < end_marker_index:
        match = DATED_RE.match(lines[index])
        if not match:
            index += 1
            continue
        level = len(match.group(1))
        stop = index + 1
        while stop < end_marker_index:
            other = HEADING_RE.match(lines[stop])
            if other and len(other.group(1)) <= level:
                break
            stop += 1
        sections.append((index, stop, match.group(2)))
        index = stop
    return sections


def next_archive_number(directory: Path, symbol: str) -> int:
    taken = [
        int(m.group("number"))
        for p in directory.glob(f"{symbol}.archive-*.md")
        if (m := plateau.ARCHIVE_NAME_RE.fullmatch(p.name)) and m.group("symbol") == symbol
    ]
    return max(taken, default=0) + 1


def plan(text: str, symbol: str, keep: int | None, before: str | None,
         archive_rel: str, number: int) -> tuple[str, str, int]:
    """Return (new live shard text, archive text, sections moved)."""
    plateau.parse_shard(text, symbol)  # must be a valid shard to begin with
    lines = text.splitlines(keepends=True)
    end_marker = f"<!-- plateau-handoff:{symbol}:end -->"
    end_index = next(i for i, line in enumerate(lines) if line.strip() == end_marker)
    sections = dated_sections(lines, end_index)
    if keep is not None:
        moved = sections[: max(len(sections) - keep, 0)]
    else:
        moved = [s for s in sections if s[2] < before]
    if not moved:
        raise ArchiveError("nothing to archive under that selection")
    drop = set()
    for first, stop, _date in moved:
        drop.update(range(first, stop))
    first_date, last_date = moved[0][2], moved[-1][2]
    pointer = (
        f"{POINTER_PREFIX} `{archive_rel}` ({len(moved)} sections, "
        f"{first_date} to {last_date}).\n"
    )
    out: list[str] = []
    placed = False
    for i, line in enumerate(lines):
        if i in drop:
            if not placed:
                out.extend([pointer, "\n"])
                placed = True
            continue
        out.append(line)
    archive = [
        f"# `{symbol}` plateau handoff archive {number}\n\n",
        f"Dated sections moved unchanged out of `docs/matching-triage-handoffs/{symbol}.md` "
        f"by `tools/archive_shard.py`. Evidence only: this file is not a shard.\n\n",
    ]
    for first, stop, _date in moved:
        archive.extend(lines[first:stop])
        if not archive[-1].endswith("\n\n") and archive[-1].strip():
            archive.append("\n")
    new_text = "".join(out)
    plateau.parse_shard(new_text, symbol)  # the live shard must stay valid
    return new_text, "".join(archive).rstrip("\n") + "\n", len(moved)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("symbol")
    choice = parser.add_mutually_exclusive_group(required=True)
    choice.add_argument("--keep-sections", type=int, metavar="N",
                        help="keep the N most recent dated sections in the live shard")
    choice.add_argument("--archive-before", metavar="YYYY-MM-DD",
                        help="archive dated sections whose date is earlier than this")
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--root", type=Path, default=ROOT, help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    if args.keep_sections is not None and args.keep_sections < 0:
        parser.error("--keep-sections must be >= 0")
    if args.archive_before and not re.fullmatch(r"\d{4}-\d{2}-\d{2}", args.archive_before):
        parser.error("--archive-before wants YYYY-MM-DD")
    try:
        shard_rel = plateau.handoff_shard_path(args.symbol)
        directory = args.root / plateau.HANDOFF_SHARD_DIR
        shard = args.root / shard_rel
        number = next_archive_number(directory, args.symbol)
        archive_rel = f"{plateau.HANDOFF_SHARD_DIR}/{args.symbol}.archive-{number}.md"
        text = shard.read_text(encoding="utf-8")
        new_text, archive, count = plan(
            text, args.symbol, args.keep_sections, args.archive_before,
            archive_rel, number)
        if len(archive.encode()) >= SIZE_LIMIT:
            raise ArchiveError("archive itself would reach the oversize limit; keep more sections")
    except (ArchiveError, plateau.PlateauError, OSError) as error:
        print(f"archive_shard: {error}", file=sys.stderr)
        return 1
    print(f"{count} section(s): {len(text.encode()):,} -> {len(new_text.encode()):,} B live, "
          f"{len(archive.encode()):,} B archive ({archive_rel})")
    if not args.dry_run:
        (args.root / archive_rel).write_text(archive, encoding="utf-8")
        shard.write_text(new_text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
