#!/usr/bin/env python3
"""Splice a permuter winner's function back into its TU and score it there.

    tools/permute_transplant.py <symbol> <permuter-output-dir | source.c> <tu.c> -o out.c [--no-score]

A permuter `output-<score>-<n>/source.c` is the whole pruned scratch TU, re-printed
by pycparser. The number that decides adoption is not the permuter's score but
the tree's: the winning function compiled inside the real translation unit with
the real recipe. This tool takes the function's definition out of the permuter
output, puts it in place of the symbol's definition in `<tu.c>` (the one inside
`#ifdef NON_MATCHING` when the TU carries more than one), writes `out.c`, and
prints `tools/fast_score.py <symbol> out.c --aligned` beside the permuter's own
`score.txt`:

    permuter score: 3120 (build/permuter/func_x/scratch/output-3120-1/score.txt)
    out.c: 7232 840 834 6 4 size-mismatch   (bytes raw masked artifact delta category)
      aligned: ...

Only the function is transplanted. When the scratch's `base.c` sits next to the
output directory, top-level lines the permuter changed outside the function
(a randomized external or function type) are listed, because they are NOT carried
over and the tree score is measured without them.

`<tu.c>` is never modified; copy-and-edit discipline is the caller's (pass a
scratch copy, or the tracked path with `-o` pointing elsewhere).
"""

from __future__ import annotations

import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


class TransplantError(RuntimeError):
    pass


def _skip_noncode(text: str, i: int) -> int:
    """Index past a comment or string/char literal starting at `i`, else `i`."""
    if text.startswith("//", i):
        end = text.find("\n", i)
        return len(text) if end < 0 else end
    if text.startswith("/*", i):
        end = text.find("*/", i + 2)
        return len(text) if end < 0 else end + 2
    if text[i] in "\"'":
        quote, j = text[i], i + 1
        while j < len(text) and text[j] != quote:
            j += 2 if text[j] == "\\" else 1
        return j + 1
    return i


def code_mask(text: str) -> list[bool]:
    """True for each character that is code (not inside a comment or literal)."""
    mask = [True] * len(text)
    i = 0
    while i < len(text):
        j = _skip_noncode(text, i)
        if j != i:
            for k in range(i, min(j, len(text))):
                mask[k] = False
            i = j
        else:
            i += 1
    return mask


def _match_brace(text: str, mask: list[bool], open_at: int) -> int:
    depth = 0
    for i in range(open_at, len(text)):
        if not mask[i]:
            continue
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i + 1
    raise TransplantError("unbalanced braces after the definition")


def definitions(text: str, symbol: str) -> list[tuple[int, int]]:
    """(start, end) of each top-level definition of `symbol` (prototypes and calls skipped)."""
    mask = code_mask(text)
    out = []
    head = re.compile(rf"^(?:[A-Za-z_][^;{{}}()\n]*?\b)?{re.escape(symbol)}\s*\(", re.M)
    for m in head.finditer(text):
        if not mask[m.start()]:
            continue
        # balance the parameter list
        depth, i = 0, m.end() - 1
        while i < len(text):
            if mask[i] and text[i] == "(":
                depth += 1
            elif mask[i] and text[i] == ")":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        # a definition reaches `{` before any `;` that is not a K&R parameter declaration
        j = i + 1
        while j < len(text) and (not mask[j] or text[j] not in "{;"):
            j += 1
        if j >= len(text):
            continue
        if text[j] == ";":
            between = text[i + 1:j + 1]
            if not between.strip() or between.strip() == ";":
                continue  # a prototype
            k = j  # K&R: `name(a) u8 a; {`, accept parameter declarations up to the brace
            while k < len(text) and (not mask[k] or text[k] != "{"):
                if mask[k] and text[k] in "()":
                    break
                k += 1
            if k >= len(text) or text[k] != "{":
                continue
            j = k
        out.append((m.start(), _match_brace(text, mask, j)))
    return out


def nonmatching_regions(text: str) -> list[tuple[int, int]]:
    """Character spans of `#ifdef NON_MATCHING` / `#if NON_MATCHING` true arms."""
    spans, stack = [], []
    pos = 0
    for line in text.splitlines(keepends=True):
        s = line.strip()
        if re.match(r"#\s*if(def)?\b", s):
            stack.append((pos, bool(re.match(r"#\s*(ifdef\s+NON_?MATCHING|if\s+(defined\s*\(\s*)?NON_?MATCHING)\b", s))))
        elif re.match(r"#\s*(else|elif)\b", s) and stack:
            start, flag = stack[-1]
            if flag:
                spans.append((start, pos))
            stack[-1] = (pos, False)
        elif re.match(r"#\s*endif\b", s) and stack:
            start, flag = stack.pop()
            if flag:
                spans.append((start, pos))
        pos += len(line)
    return spans


def choose_definition(text: str, symbol: str) -> tuple[int, int]:
    defs = definitions(text, symbol)
    if not defs:
        raise TransplantError(f"no definition of {symbol} found")
    if len(defs) == 1:
        return defs[0]
    regions = nonmatching_regions(text)
    inside = [d for d in defs if any(a <= d[0] < b for a, b in regions)]
    if len(inside) == 1:
        return inside[0]
    raise TransplantError(f"{len(defs)} definitions of {symbol}; cannot tell which one is the candidate")


def transplant(symbol: str, winner: str, tu: str) -> str:
    w_start, w_end = choose_definition(winner, symbol)
    t_start, t_end = choose_definition(tu, symbol)
    return tu[:t_start] + winner[w_start:w_end] + tu[t_end:]


def outside_changes(symbol: str, base: str, winner: str) -> list[str]:
    """Top-level lines of the winner that differ from the scratch base outside `symbol`."""
    def rest(text: str) -> list[str]:
        start, end = choose_definition(text, symbol)
        return [line.strip() for line in (text[:start] + text[end:]).splitlines()
                if line.strip() and not line.strip().startswith("#pragma _permuter")]
    out = []
    for line in difflib.unified_diff(rest(base), rest(winner), lineterm="", n=0):
        if line.startswith(("---", "+++", "@@")):
            continue
        out.append(line)
    return out


def resolve_inputs(path: Path) -> tuple[Path, Path | None, Path | None]:
    """(source.c, score.txt or None, scratch base.c or None) for a dir or a file."""
    source = path / "source.c" if path.is_dir() else path
    if not source.is_file():
        raise TransplantError(f"no permuter source at {source}")
    score = source.parent / "score.txt"
    base = source.parent.parent / "base.c"
    return source, (score if score.is_file() else None), (base if base.is_file() else None)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("winner", type=Path, help="permuter output-<score>-<n>/ directory or its source.c")
    ap.add_argument("tu", type=Path, help="the TU to splice into (not modified)")
    ap.add_argument("-o", "--output", type=Path, required=True)
    ap.add_argument("--no-score", action="store_true", help="write out.c only; skip fast_score")
    ns = ap.parse_args(argv)
    try:
        source, score, base = resolve_inputs(ns.winner)
        winner = source.read_text()
        out = transplant(ns.symbol, winner, ns.tu.read_text())
        notes = outside_changes(ns.symbol, base.read_text(), winner) if base else []
    except (TransplantError, OSError) as error:
        print(f"permute_transplant: {error}", file=sys.stderr)
        return 2
    if ns.output.resolve() == ns.tu.resolve():
        print("permute_transplant: -o must not be the input TU", file=sys.stderr)
        return 2
    ns.output.write_text(out)
    print(f"wrote {ns.output}")
    if score:
        print(f"permuter score: {score.read_text().strip()} ({score})")
    else:
        print("permuter score: unknown (no score.txt beside the source)")
    if notes:
        print(f"note: the permuter output also changes {len(notes)} line(s) outside {ns.symbol}; "
              "NOT transplanted, and not in the tree score below:")
        for line in notes[:20]:
            print(f"  {line}")
    if ns.no_score:
        return 0
    proc = subprocess.run([sys.executable, str(ROOT / "tools/fast_score.py"), ns.symbol, str(ns.output),
                           "--aligned"], cwd=ROOT, capture_output=True, text=True)
    sys.stdout.write(proc.stdout)
    sys.stderr.write(proc.stderr)
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
