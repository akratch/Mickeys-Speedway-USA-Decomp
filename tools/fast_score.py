#!/usr/bin/env python3
"""Compile one candidate translation unit directly and score one symbol in it.

    tools/fast_score.py <symbol> <candidate.c> [--diff] [--keep-object PATH]

This is the fast loop every lane rebuilds by hand: a direct IDO compile of a
whole candidate TU is byte-identical in `.text` to the asm-processor build when
the candidate has no `GLOBAL_ASM` pragma, and it takes about a second against
roughly a minute for a tree build. The one hazard is the per-file flags
(`-Wab,-r4300_mul`, `-Wo,-loopunroll,0`, `-O1`, `-g3` ...): a hand-typed
command that drops one measures a different compiler, silently. On 2026-10-01
one lane read 197 words at a four-byte size mismatch against the true 13 for
exactly that reason.

So the compiler line is never typed here. It is read from the tree's own
`NON_MATCHING=1` recipe for the symbol's tracked TU (`gmake -n`), with only the
source path and the output path replaced. The candidate is compiled in the
repository root so relative includes resolve as the build's do, and the object
is scored with `tools/score_symbol.py --object`, so the number agrees with the
ranking by construction.

`--aligned` adds the aligner's four buckets (exact, register naming, immediate
only, really different) plus the one-sided word spans, which stay meaningful
when the candidate is an instruction long or short and the positional count is
noise. `--diff` prints the words that still differ, target word beside candidate
word, read from the extracted target listing and the candidate object. It is a
reading aid for choosing the next edit; the masked count is the measurement.

The candidate must be a complete TU for the same source file the symbol lives
in (copy the tracked file, edit the copy). Relocation-bearing words are masked
the same way the ranking masks them, so an address spelled as a literal and
one spelled as a symbol compare equal.
"""

from __future__ import annotations

import argparse
import glob
import json
import os
import re
import shlex
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def source_grep_command(symbol: str) -> list[str]:
    """`git grep` arguments listing the tracked files that name `symbol` as a whole word.

    A `\\b` boundary inside `-E` is a GNU extension that macOS's regex library
    does not provide, so the pattern matched nothing there. `-w -F` asks git
    itself for the word match and needs no regex at all.
    """
    return ["git", "grep", "-l", "-w", "-F", symbol, "--", "src"]


def tracked_source_for(symbol: str) -> str:
    """Return the tracked TU path (repo-relative) carrying `symbol`'s candidate."""
    ranking = json.loads((ROOT / "config/nonmatching-ranking.us.json").read_text())
    for row in ranking["functions"]:
        if row["name"] == symbol:
            return row["file"]
    hits = subprocess.run(source_grep_command(symbol), cwd=ROOT, capture_output=True, text=True).stdout.split()
    hits = [h for h in hits if h.endswith(".c")]
    if len(hits) != 1:
        raise SystemExit(f"cannot identify one tracked TU for {symbol}: {hits}")
    return hits[0]


def configured_cc_args(source: str) -> list[str]:
    """Expand the tree's NON_MATCHING recipe for `source` and return the cc arguments.

    The asm-processor wrapper line is `build.py <cc> -- <as args> -- <cc args>`;
    the cc arguments are everything after the second `--`.
    """
    proc = subprocess.run(
        ["gmake", "--no-print-directory", "-n", "NON_MATCHING=1", "-W", source,
         f"build_non_matching/{source}.o"],
        cwd=ROOT, capture_output=True, text=True, timeout=300,
    )
    if proc.returncode:
        raise SystemExit(proc.stderr.strip() or "cannot expand the configured recipe")
    text = proc.stdout.replace("\\\n", " ")
    for line in text.splitlines():
        if "tools/asm-processor/build.py" not in line or source not in line:
            continue
        return split_recipe(line)
    raise SystemExit(f"no asm-processor recipe found for {source}")


def split_recipe(line: str) -> list[str]:
    """Return the compiler arguments of one asm-processor recipe line."""
    words = shlex.split(line)
    seps = [i for i, w in enumerate(words) if w == "--"]
    if len(seps) < 2:
        raise SystemExit("recipe lacks the two '--' separators asm-processor uses")
    # build.py takes its own options (`--force` on overlay TUs) before the
    # compiler path, so the compiler is the first word after it that is not one.
    after = words[words.index("tools/asm-processor/build.py") + 1:seps[0]]
    cc = next((w for w in after if not w.startswith("-")), None)
    if cc is None:
        raise SystemExit("recipe names no compiler before the first '--'")
    return [cc] + words[seps[1] + 1:]


def rewrite_io(args: list[str], candidate: Path, source: str, obj: Path) -> list[str]:
    out = []
    i = 0
    while i < len(args):
        a = args[i]
        if a == "-o":
            out += ["-o", str(obj)]
            i += 2
            continue
        if a == source:
            out.append(str(candidate))
        else:
            out.append(a)
        i += 1
    return out


def target_listing_for(symbol: str, source_text: str) -> str | None:
    """Return the GLOBAL_ASM path paired with `symbol`'s candidate, if any.

    A friendly-named candidate (`overlay101TailA6BC`) keeps its fallback under
    the generated name (`func_overlay_101_...s`), so the listing cannot be
    found by symbol name: it is the first `#pragma GLOBAL_ASM` after the
    candidate's definition, in the `#else` arm of the same guard.
    """
    m = re.search(rf"^[\w\s\*]*\b{re.escape(symbol)}\s*\([^;]*?\)\s*\{{", source_text, re.M)
    if not m:
        return None
    tail = source_text[m.end():]
    g = re.search(r'#pragma\s+GLOBAL_ASM\("([^"]+)"\)', tail)
    return g.group(1) if g else None


def target_words(symbol: str, source: str) -> list[tuple[int, str]]:
    stem = Path(source).with_suffix("").as_posix().removeprefix("src/")
    files: list[str] = []
    pragma = target_listing_for(symbol, (ROOT / source).read_text())
    if pragma and (ROOT / pragma).is_file():
        files = [str(ROOT / pragma)]
    for p in ([] if files else [f"asm/nonmatchings/{stem}/{symbol}.s", f"asm/nonmatchings/{stem}/*.s"]):
        files = sorted(glob.glob(str(ROOT / p)))
        files = [f for f in files if symbol in Path(f).read_text()[:400] or Path(f).stem == symbol]
        if files:
            break
    if not files:
        raise SystemExit(f"no extracted target listing for {symbol}; run gmake extract")
    rows = []
    for line in Path(files[0]).read_text().splitlines():
        m = re.match(r"\s*/\* \w+ \w+ ([0-9A-Fa-f]{8}) \*/\s*(.*)", line)
        if m:
            rows.append((int(m.group(1), 16), re.sub(r"\s+", " ", m.group(2).strip())))
    return rows


def candidate_words(symbol: str, obj: Path) -> list[tuple[int, str]]:
    d = subprocess.run(
        [str(ROOT / "tools/binutils/mips64-elf-objdump"), "-d", "-z", str(obj)],
        capture_output=True, text=True,
    ).stdout
    rows, inside = [], False
    for line in d.splitlines():
        if re.match(rf"^[0-9a-f]+ <{re.escape(symbol)}>:", line):
            inside = True
            continue
        if inside and re.match(r"^[0-9a-f]+ <", line):
            break
        m = re.match(r"\s*([0-9a-f]+):\t([0-9a-f]{8}) \t(.*)", line)
        if inside and m:
            rows.append((int(m.group(2), 16), m.group(3).replace("\t", " ")))
    return rows


RELOC_NOISE = ("%lo", "%hi", ">> 16", "& 0xFFFF")


def print_diff(symbol: str, source: str, obj: Path) -> None:
    tgt = target_words(symbol, source)
    cand = candidate_words(symbol, obj)
    for i in range(max(len(tgt), len(cand))):
        t = tgt[i] if i < len(tgt) else (None, "-")
        c = cand[i] if i < len(cand) else (None, "-")
        if t[0] != c[0]:
            if any(n in t[1] for n in RELOC_NOISE) or t[1].startswith("jal"):
                continue  # relocation payload; the masked count already excludes it
            print(f"  +{i * 4:#06x}  T: {t[1]:40s} C: {c[1]}")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("candidate")
    ap.add_argument("--diff", action="store_true", help="print differing words")
    ap.add_argument("--aligned", action="store_true",
                    help="also print align_symbol's four buckets for this candidate")
    ap.add_argument("--keep-object", help="write the object here instead of beside the candidate")
    ns = ap.parse_args(argv)

    candidate = Path(ns.candidate).resolve()
    if not candidate.is_file():
        raise SystemExit(f"no such candidate: {candidate}")
    source = tracked_source_for(ns.symbol)
    obj = Path(ns.keep_object).resolve() if ns.keep_object else candidate.with_suffix(".o")
    args = rewrite_io(configured_cc_args(source), candidate, source, obj)
    cc = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    if cc.returncode:
        sys.stderr.write(cc.stderr[-2000:])
        return 1
    score = subprocess.run(
        [sys.executable, str(ROOT / "tools/score_symbol.py"), "--object", str(obj), ns.symbol],
        cwd=ROOT, capture_output=True, text=True,
    )
    lines = [l for l in score.stdout.splitlines() if l.startswith(ns.symbol)]
    if not lines:
        sys.stderr.write(score.stdout + score.stderr)
        return 1
    print(f"{candidate.name}: {' '.join(lines[0].split()[1:])}   (bytes raw masked artifact delta category)")
    if ns.aligned:
        import tempfile
        import align_symbol
        with tempfile.TemporaryDirectory(prefix="fast-score-") as tmp:
            row = align_symbol.AlignedScorer(ns.symbol, Path(tmp)).score(obj)
        print("  aligned: " + (align_symbol.render_buckets(row) if row else "symbol not found"))
        if row:
            for span in row["insertions"]:
                print(f"    candidate-only +{span['candidate_offset']:#x}: {span['words']} word(s)")
            for span in row["deletions"]:
                print(f"    target-only    +{span['target_offset']:#x}: {span['words']} word(s)")
    if ns.diff:
        print_diff(ns.symbol, source, obj)
    return 0


if __name__ == "__main__":
    sys.exit(main())
