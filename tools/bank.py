#!/usr/bin/env python3
"""Bank an improvement, or record a match, in one correct and repeatable step.

    .venv/bin/python tools/bank.py <symbol> [--note FILE] [--summary TEXT]
                                   [--match] [--commit] [--trailer T]

Every matching lane used to repeat the same sequence by hand: measure with
score_symbol, regenerate the ranking (measuring form, then --write-doc), write
the shard header and a dated section, update the in-source PLATEAU marker, run
check_shard_metrics, run the gates, commit with a trailer. Most
`finalize_plateau.py` refusals were the ranking regeneration that `check-docs`
demands sitting uncommitted. This does the sequence, in the order that works:

  1. Re-measure the symbol: the ranking's positional masked count, frame and
     relocation count, and align_symbol's four aligned buckets (byte-exact,
     register naming, immediate only, really different), which are the honest
     numbers at a nonzero size delta.
  2. Write the shard header and the in-source marker FROM THE MEASUREMENT.
     Score, frame, relocations and first mismatch are never taken from the
     caller. The caller supplies only `--summary` and the `--note` prose.
  3. Append (or replace, by heading) the dated section from `--note`.
     The note's first line must be a `#### YYYY-MM-DD, ...` heading; the shard
     grammar is validated with finalize_plateau's own checks (no `|`, no CR,
     one marker pair).
  4. Regenerate the ranking: `nm_ranking.py` (measuring form), then
     `--write-doc`. Done after the source edit so the row hashes final source.
  5. Run check_shard_metrics.py and plateau_handoff_audit.py --check.
  6. With --commit: stage exactly the files this wrote, run
     `tools/gates.sh --staged` (true exit status, unpiped) and commit only on 0,
     ending with `--trailer` lines or $MICKEY_COMMIT_TRAILER.

`--match` records a match instead: the function must have left the NON_MATCHING
queue (guard removed) and the ranking, its source marker is removed, and the
shard gets the "Matched." header form. The source file comes from the old
shard/marker, or `--source`.

A PLATEAU-HANDOFF block that is mid-file (not an EOF suffix, which
finalize_plateau.update_source refuses) is replaced in place. Re-running with
the same inputs changes nothing.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import finalize_plateau as fp  # noqa: E402

RANKING_REL = "config/nonmatching-ranking.us.json"
RANKING_DOC_REL = "docs/nm-ranking.md"
HEADING_RE = re.compile(r"^#### \d{4}-\d{2}-\d{2}[,:].*\S$")
ADDIU_SP_HI = 0x27BD


class BankError(RuntimeError):
    """A clear refusal; nothing is committed."""


# --------------------------------------------------------------------------
# Measurement (lazy imports: they need the venv's permuter, tests do not)
# --------------------------------------------------------------------------

def queue_source(symbol: str) -> str | None:
    """Repository-relative source of a queued symbol, or None if not queued."""
    import permute_batch as pb
    for item in pb.discover_queue():
        if item.func == symbol:
            return item.rel_c_file
    return None


def frame_of(words: list[int]) -> str:
    """Frame size from the prologue's `addiu sp, sp, -N`, else `frameless`."""
    for word in words[:8]:
        if word >> 16 == ADDIU_SP_HI:
            imm = word & 0xFFFF
            if imm & 0x8000:
                return f"0x{0x10000 - imm:X}"
    return "frameless"


def measure(symbol: str) -> dict:
    """Compile the configured TU once and return every measured field."""
    import align_symbol as al
    import nm_ranking as nr
    import permute_batch as pb
    import score_symbol as ss

    queue = {item.func: item for item in pb.discover_queue()}
    item = queue.get(symbol)
    if item is None:
        raise BankError(f"{symbol} is not in the NON_MATCHING queue")
    with ss._isolated_workdir():
        commands = nr.configured_compile_commands([item])
        candidate, error = nr.compile_configured_tu(
            item.rel_c_file, commands[item.rel_c_file])
        if candidate is None:
            raise BankError(f"{symbol}: compile failed: {error}")
        streams, error = nr.word_streams(item, candidate)
        if streams is None:
            raise BankError(f"{symbol}: {error}")
        result, error = nr.process_item(item, candidate)
        if result is None:
            raise BankError(f"{symbol}: {error}")
        row = ss._row(result)
        row.update(al.align(streams))
    row["candidate_words"] = len(streams.base_words)
    row["target_words"] = len(streams.target_words)
    row["frame"] = frame_of(streams.base_words)
    row["relocations"] = len(streams.base_reloc)
    row["source"] = item.rel_c_file
    return row


def header_fields(row: dict) -> fp.Metrics:
    masked = row["relocation_masked_differing_words"]
    first = row["relocation_masked_first_mismatch_offset"]
    if masked is None:
        raise BankError("measurement has no relocation-masked count")
    score = f"{masked}/{row['target_words']} words"
    first_text = "none" if (masked == 0 or first is None) else f"+0x{first:X}"
    return fp.Metrics(score, row["frame"], int(row["relocations"]), first_text)


def measured_line(row: dict) -> str:
    return (
        f"Measured by tools/bank.py: masked {row['relocation_masked_differing_words']}"
        f" (raw {row['differing_words']}), size delta {row['size_delta']:+d},"
        f" candidate {row['candidate_words']} words vs target {row['target_words']}."
        f" Aligned: byte-exact {row['aligned_exact']},"
        f" register naming {row['aligned_register_naming']},"
        f" immediate only {row['aligned_immediate_only']},"
        f" really different {row['aligned_really_different']}."
    )


# --------------------------------------------------------------------------
# External steps (separate functions so tests can replace them)
# --------------------------------------------------------------------------

def regenerate_ranking(root: Path) -> None:
    for extra in (["--no-table"], ["--write-doc"]):
        cmd = [sys.executable, "tools/nm_ranking.py", *extra]
        if subprocess.run(cmd, cwd=root).returncode != 0:
            raise BankError("nm_ranking.py " + (" ".join(extra))
                            + " failed")


def run_checks(root: Path) -> None:
    for cmd in (["tools/check_shard_metrics.py"],
                ["tools/plateau_handoff_audit.py", "--check"]):
        if subprocess.run([sys.executable, *cmd], cwd=root).returncode != 0:
            raise BankError(f"{cmd[0]} failed")


def run_gates(root: Path) -> int:
    return subprocess.run(["tools/gates.sh", "--staged"], cwd=root).returncode


def git(root: Path, *args: str) -> str:
    result = subprocess.run(["git", *args], cwd=root, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        raise BankError(f"git {' '.join(args)}: {result.stderr.strip()}")
    return result.stdout


def ranking_names(root: Path) -> set[str]:
    data = json.loads((root / RANKING_REL).read_text(encoding="utf-8"))
    return {row["name"] for row in data.get("functions", [])}


# --------------------------------------------------------------------------
# Source marker, handled wherever it sits
# --------------------------------------------------------------------------

def owned_blocks(text: str, symbol: str) -> list[re.Match]:
    if fp.LEGACY_HANDOFF_RE.search(text):
        raise BankError("legacy inline PLATEAU-HANDOFF: migrate it manually")
    blocks = list(fp.KEYED_HANDOFF_RE.finditer(text))
    if text.count("PLATEAU-HANDOFF:") != 2 * len(blocks):
        raise BankError("malformed symbol-keyed PLATEAU-HANDOFF metadata")
    owned = [b for b in blocks if b.group("symbol") == symbol]
    if len(owned) > 1:
        raise BankError(f"duplicate PLATEAU-HANDOFF metadata for {symbol}")
    return owned


def marker_summary(text: str, symbol: str) -> str:
    owned = owned_blocks(text, symbol)
    if not owned:
        return ""
    for line in owned[0].group(0).splitlines():
        if line.startswith(" * summary: "):
            return line[len(" * summary: "):]
    return ""


def set_marker(text: str, symbol: str, metrics: fp.Metrics) -> str:
    """Replace the owned block in place (mid-file included) or append it."""
    handoff = fp.source_handoff(symbol, metrics)
    owned = owned_blocks(text, symbol)
    if owned:
        return text[:owned[0].start()] + handoff + text[owned[0].end():]
    if not text or text.endswith("\n\n"):
        sep = ""
    elif text.endswith("\n"):
        sep = "\n"
    else:
        sep = "\n\n"
    return text + sep + handoff


def drop_marker(text: str, symbol: str) -> str:
    owned = owned_blocks(text, symbol)
    if not owned:
        return text
    block = owned[0]
    start, end = block.start(), block.end()
    # Take the blank line that separated the block from what precedes it.
    if text[:start].endswith("\n\n"):
        start -= 1
    return text[:start] + text[end:]


# --------------------------------------------------------------------------
# Shard
# --------------------------------------------------------------------------

def read_note(path: Path) -> str:
    text = path.read_text(encoding="utf-8").replace("\r\n", "\n").strip("\n")
    if not text:
        raise BankError("--note is empty")
    first = text.split("\n", 1)[0]
    if not HEADING_RE.match(first):
        raise BankError(
            "the note's first line must be a heading like "
            "'#### 2026-10-07, lane NAME: what changed'")
    if any(ch in text for ch in ("|", "\r")):
        bad = [n for n, line in enumerate(text.splitlines(), 1) if "|" in line]
        raise BankError(
            "the note may not contain '|' (the shard grammar forbids the "
            f"ledger's column separator; line(s) {bad[:5]}); write prose or an "
            "indented list, not a table")
    if "plateau-handoff:" in text or "<!--" in text:
        raise BankError("the note may not contain a plateau-handoff marker or comment")
    return text


def apply_section(shard: str, symbol: str, section: str) -> str:
    """Insert the dated section before the end marker, replacing its twin.

    A section with the same heading line is replaced up to the next heading
    of level 2-4 or the end marker, so a re-run is a no-op.
    """
    end = f"<!-- plateau-handoff:{symbol}:end -->"
    heading = section.split("\n", 1)[0]
    head, sep, tail = shard.rpartition(end)
    if not sep:
        raise BankError(f"shard for {symbol} has no end marker")
    lines = head.split("\n")
    start = next((i for i, line in enumerate(lines) if line == heading), None)
    if start is not None:
        stop = len(lines)
        for i in range(start + 1, len(lines)):
            if re.match(r"^#{2,4} ", lines[i]):
                stop = i
                break
        before = "\n".join(lines[:start]).rstrip("\n")
        after = "\n".join(lines[stop:]).lstrip("\n")
        head = before + "\n\n" + section + "\n" + ("\n" + after if after else "")
    else:
        head = head.rstrip("\n") + "\n\n" + section + "\n"
    return head + sep + tail


def shard_fields(text: str, symbol: str) -> dict:
    match = fp.shard_pattern(symbol).fullmatch(text)
    if match is None:
        raise BankError(fp.shard_rejection_reason(text, symbol))
    frame = re.search(r"^- frame: (.+)$", text, re.M).group(1).strip()
    relocs = int(re.search(r"^- relocations: (\d+)$", text, re.M).group(1))
    return {"source": match.group("source"), "summary": match.group("summary") or "",
            "frame": frame, "relocations": relocs}


def build_shard(old: str, symbol: str, source: str, metrics: fp.Metrics,
                section: str | None, note_measure: str | None) -> str:
    block = fp.markdown_handoff(symbol, source, metrics)
    text = fp.update_handoff_shard(old, symbol, block)
    if section is not None:
        body = section
        if note_measure:
            heading, _, rest = section.partition("\n")
            body = heading + "\n\n" + note_measure + ("\n" + rest if rest else "")
        text = apply_section(text, symbol, body)
    fp.parse_shard(text, symbol)  # the reader's own grammar, last word
    return text


# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def repo_root() -> Path:
    return fp.repository_root()


def write_if_changed(pairs: list[tuple[Path, str, str]]) -> list[Path]:
    changed = [(p, new) for p, old, new in pairs if old != new]
    if not changed:
        return []
    import time
    stamp = time.time()
    for path, new in changed:
        path.write_text(new, encoding="utf-8", newline="\n")
        os.utime(path, (stamp, stamp))
    return [p for p, _ in changed]


def bank(args: argparse.Namespace, root: Path) -> list[str]:
    symbol = args.symbol
    if not fp.SYMBOL_RE.fullmatch(symbol):
        raise BankError(f"invalid exact symbol {symbol!r}")
    if args.trailer and not args.commit:
        raise BankError("--trailer requires --commit")
    trailers = fp.commit_trailers(args.trailer, dict(os.environ)) if args.commit else []
    section = read_note(Path(args.note)) if args.note else None
    if args.summary:
        fp.validate_one_line(args.summary, "summary")

    shard_rel = fp.handoff_shard_path(symbol)
    shard_path = root / shard_rel
    old_shard = shard_path.read_text(encoding="utf-8") if shard_path.is_file() else ""
    old = shard_fields(old_shard, symbol) if old_shard else None

    queued_source = queue_source(symbol)
    if args.match:
        if queued_source is not None:
            raise BankError(
                f"{symbol} is still in the NON_MATCHING queue ({queued_source}); "
                "remove its #ifdef NON_MATCHING guard and GLOBAL_ASM fallback first")
        source_rel = args.source or (old["source"] if old else None)
        if not source_rel:
            raise BankError("cannot find the source of a matched symbol with no "
                            "shard; pass --source")
        rules = root / fp.OVERLAY_RULES_PATH
        aliases = fp.fallback_aliases(
            rules.read_text(encoding="utf-8") if rules.is_file() else None
        ).get((source_rel, symbol), frozenset())
        src_path = root / source_rel
        src_text = src_path.read_text(encoding="utf-8")
        if not fp.defined_without_fallback(src_text, symbol, aliases):
            raise BankError(f"{source_rel} does not define {symbol} with its "
                            "GLOBAL_ASM fallback gone")
        row = None
        carried_summary = (args.summary or "").strip()
        previous = old["summary"] if old else marker_summary(src_text, symbol)
        if carried_summary:
            summary = carried_summary if carried_summary.startswith("Matched") \
                else "Matched. " + carried_summary
        elif previous.startswith("Matched"):
            summary = previous
        else:
            summary = "Matched."
        metrics = fp.Metrics(
            "0 differing words",
            old["frame"] if old else "unknown",
            old["relocations"] if old else 0,
            "none", summary)
        new_src = drop_marker(src_text, symbol)
        note_measure = None
    else:
        if queued_source is None:
            raise BankError(
                f"{symbol} is not in the NON_MATCHING queue; if it matched, use "
                "--match after removing the guard")
        if args.source and args.source != queued_source:
            raise BankError(f"--source {args.source} disagrees with the queue "
                            f"({queued_source})")
        source_rel = queued_source
        if old and old["source"] != source_rel:
            raise BankError(f"shard records {old['source']} but the queue has "
                            f"{source_rel}")
        src_path = root / source_rel
        src_text = src_path.read_text(encoding="utf-8")
        rules = root / fp.OVERLAY_RULES_PATH
        aliases = fp.fallback_aliases(
            rules.read_text(encoding="utf-8") if rules.is_file() else None
        ).get((source_rel, symbol), frozenset())
        fp.require_guarded_candidate(src_text, symbol, aliases)
        row = measure(symbol)
        measured = header_fields(row)
        summary = (args.summary or "").strip() or marker_summary(src_text, symbol) \
            or (old["summary"] if old else "")
        metrics = fp.Metrics(measured.score, measured.frame, measured.relocations,
                             measured.first_mismatch, " ".join(summary.split()))
        new_src = set_marker(src_text, symbol, metrics)
        note_measure = measured_line(row)

    new_shard = build_shard(old_shard, symbol, source_rel, metrics, section, note_measure)

    allowed = {source_rel, shard_rel, RANKING_REL, RANKING_DOC_REL, *(args.also or [])}
    if args.commit:
        dirt = set()
        for sub in (("diff", "--name-only"), ("diff", "--cached", "--name-only"),
                    ("ls-files", "--others", "--exclude-standard")):
            dirt.update(line for line in git(root, *sub).splitlines() if line)
        if args.note:
            # The note is input, not output; it may sit untracked in the tree.
            try:
                dirt.discard(Path(args.note).resolve().relative_to(root.resolve()).as_posix())
            except ValueError:
                pass
        unrelated = sorted(dirt - allowed)
        if unrelated:
            raise BankError("unrelated worktree/index dirt (use --also PATH to "
                            "include it): " + ", ".join(unrelated))

    write_if_changed([(src_path, src_text, new_src), (shard_path, old_shard, new_shard)])
    # Source first, then the ranking: its rows hash the final source.
    regenerate_ranking(root)
    if args.match and symbol in ranking_names(root):
        raise BankError(f"{symbol} is still listed in {RANKING_REL} after "
                        "regeneration; it is not matched")
    run_checks(root)

    report = [f"symbol: {symbol}", f"source: {source_rel}", f"shard: {shard_rel}",
              f"score: {metrics.score}", f"frame: {metrics.frame}",
              f"relocations: {metrics.relocations}",
              f"first-mismatch: {metrics.first_mismatch}"]
    if row is not None:
        report.append(
            "aligned: exact %d, naming %d, immediate %d, different %d (size delta %+d)"
            % (row["aligned_exact"], row["aligned_register_naming"],
               row["aligned_immediate_only"], row["aligned_really_different"],
               row["size_delta"]))
    if args.match:
        report.append("reminder: gmake scoreboard, then commit the README diff")

    commit = "not requested"
    if args.commit:
        paths = sorted(p for p in allowed if (root / p).exists())
        git(root, "add", "--", *paths)
        staged = set(git(root, "diff", "--cached", "--name-only").splitlines())
        if staged - allowed:
            raise BankError("refusing to commit unrelated staged paths: "
                            + ", ".join(sorted(staged - allowed)))
        if not staged:
            commit = "unchanged"
        else:
            status = run_gates(root)
            if status != 0:
                raise BankError(f"tools/gates.sh --staged exited {status}; "
                                "nothing committed (files remain staged)")
            subject = fp.validate_one_line(
                args.message or (f"Match {symbol}" if args.match else f"Bank {symbol}"),
                "commit message", 100)
            body = subject + ("\n\n" + "\n".join(trailers) if trailers else "")
            git(root, "commit", "-m", body)
            commit = git(root, "rev-parse", "HEAD").strip()
    report.append(f"commit: {commit}")
    return report


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("symbol")
    p.add_argument("--note", help="file holding the dated section to append")
    p.add_argument("--summary", help="one-line blocker or next lever")
    p.add_argument("--match", action="store_true", help="record a match")
    p.add_argument("--source", help="source file (--match with no shard)")
    p.add_argument("--also", action="append", default=None,
                   help="extra tracked path to stage with --commit (repeatable)")
    p.add_argument("--commit", action="store_true")
    p.add_argument("--message", help="commit subject")
    p.add_argument("--trailer", action="append", default=None,
                   help=f"commit trailer line (repeatable); default ${fp.TRAILER_ENV}")
    return p


def main(argv: list[str] | None = None, root: Path | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        for line in bank(args, root or repo_root()):
            print(line)
    except (BankError, fp.PlateauError, OSError) as error:
        print(f"bank: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
