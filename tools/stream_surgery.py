#!/usr/bin/env python3
"""Edit one function's ugen listing and score what as1 makes of it.

    tools/stream_surgery.py <symbol> --edit FILE.diff      # unified diff of the .s
    tools/stream_surgery.py <symbol> --script EDIT.py      # def edit(lines) -> lines
    tools/stream_surgery.py <symbol> --enumerate 40-43,60-62 [--edit/--script]
    tools/stream_surgery.py <symbol> --dump                # numbered function lines

The question it answers: "if ugen had emitted THIS stream, would as1 give the
target?" That separates a ugen/uopt decision from an as1 one before any source
search, and gives a known destination to hunt a spelling for.

Pipeline, all from the tree's own recipe (the compiler line is never typed):

1. `cc -S` the symbol's tracked TU with the configured per-file flags. The
   driver writes `<base>.s` beside the input. This is ugen's allocation, before
   as1 schedules.
2. Take the function's lines (`.ent` through `.end`), apply the edit.
3. Reassemble with `as0` then `as1` using the flags the C compile gave `as1`
   (read from `cc -show`), NOT the driver's `.s` path. The driver adds
   `-pic0 -noglobal` for a `.s` input, and `-noglobal` alone changes as1's
   global scheduling and register renaming (+20 bytes on the o020 grid); a
   round trip through `cc -c x.s` therefore measures a different assembler.
4. Score the reassembled object's symbol like fast_score does (masked,
   size delta, aligned buckets) when the symbol is on the NON_MATCHING queue.

A CONTROL line always prints first: the untouched listing reassembled and
compared with the configured compile's object (function bytes and relocation
records). If it says `differs`, no edit result below is evidence of anything.

`--script` defines `edit(lines)`: `lines` is the function's listing as a list
of strings without newlines (see `--dump` for the numbering); return the new
list. `--enumerate` takes 1-based inclusive line ranges of the (edited)
function listing; each range is a movable chunk, the text between chunks stays
put, and every ordering of the chunks over the chunk slots is assembled and
scored (the identity ordering is the baseline). Use it for join-order and
schedule questions. Chunks of different length are fine; the slots keep their
positions and the chunks are laid into them in the permuted order.

The whole reassembled TU is scored through the symbol, so every other
function in the object rides along unchanged; the function's bytes are not
spliced into a second object because the score reads the symbol's own span and
relocation records.

`--instrumented` (implied by any `CDX_*`, `DKWB_CUT_*` or `DKWB_SUBST_*` variable in the
environment) produces the listing and the configured object with the
instrumented compiler, so a biased or cut-forced listing round-trips and the
control compares against the same instrumented object, not stock.

`--keep DIR` retains the `.s` and `.o` of the baseline and (single-edit mode)
the edited cell. Scratch lives in a temp directory otherwise.
"""

from __future__ import annotations

import argparse
import importlib.util
import itertools
import math
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import fast_score  # noqa: E402

ROOT = fast_score.ROOT
IDO = ROOT / "tools/ido"
INSTRUMENTED_CC = fast_score.INSTRUMENTED_CC
INSTRUMENTED_PREFIXES = fast_score.INSTRUMENTED_PREFIXES
instrumented_wanted = fast_score.instrumented_wanted
use_compiler = fast_score.use_compiler


# ------------------------------------------------------------ pure helpers

def function_span(lines: list[str], symbol: str) -> tuple[int, int]:
    """Half-open [start, end) line indexes of `.ent symbol` .. `.end symbol`."""
    ent = re.compile(rf"^\s*\.ent\s+{re.escape(symbol)}(\s|$)")
    end = re.compile(rf"^\s*\.end\s+{re.escape(symbol)}(\s|$)")
    start = None
    for i, line in enumerate(lines):
        if start is None and ent.match(line):
            start = i
        elif start is not None and end.match(line):
            return start, i + 1
    raise SystemExit(f"no `.ent {symbol}` .. `.end {symbol}` in the listing")


def split_function(text: str, symbol: str) -> tuple[list[str], list[str], list[str]]:
    """(head lines, function lines, tail lines) of a ugen listing."""
    lines = text.split("\n")
    a, b = function_span(lines, symbol)
    return lines[:a], lines[a:b], lines[b:]


def join_listing(head: list[str], func: list[str], tail: list[str]) -> str:
    return "\n".join(head + func + tail)


def parse_ranges(spec: str, nlines: int) -> list[tuple[int, int]]:
    """'3-5,9,12-13' -> [(3, 5), (9, 9), (12, 13)], 1-based inclusive, sorted, disjoint."""
    out = []
    for part in [p.strip() for p in spec.split(",") if p.strip()]:
        lo, _, hi = part.partition("-")
        a, b = int(lo), int(hi or lo)
        if not 1 <= a <= b <= nlines:
            raise SystemExit(f"range {part} is outside the function listing (1-{nlines})")
        out.append((a, b))
    out.sort()
    for (a0, b0), (a1, b1) in zip(out, out[1:]):
        if a1 <= b0:
            raise SystemExit(f"ranges {a0}-{b0} and {a1}-{b1} overlap")
    if len(out) < 2:
        raise SystemExit("--enumerate needs at least two ranges to permute")
    return out


def permute_chunks(func: list[str], ranges: list[tuple[int, int]], order: tuple[int, ...]) -> list[str]:
    """Lay chunk `order[k]` into slot k; the text between slots is unchanged."""
    chunks = [func[a - 1:b] for a, b in ranges]
    out: list[str] = []
    cursor = 0
    for slot, (a, b) in enumerate(ranges):
        out += func[cursor:a - 1]
        out += chunks[order[slot]]
        cursor = b
    out += func[cursor:]
    return out


def orders(n: int, limit: int):
    if math.factorial(n) > limit:
        raise SystemExit(f"{n} chunks is {math.factorial(n)} orderings, over --max-cells {limit}")
    return itertools.permutations(range(n))


def tool_flags(show: str, tool: str) -> list[str]:
    """Flags `cc -show` printed for /usr/lib/<tool>: every token up to the first path."""
    for line in show.splitlines():
        words = line.split()
        if words and words[0].endswith("/" + tool):
            flags = []
            for w in words[1:]:
                if w.startswith("/"):
                    break
                flags.append(w)
            return flags
    raise SystemExit(f"`cc -show` printed no {tool} line; cannot read the C-compile flags")


def apply_script(path: Path, func: list[str]) -> list[str]:
    spec = importlib.util.spec_from_file_location("stream_surgery_edit", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    if not hasattr(mod, "edit"):
        raise SystemExit(f"{path} defines no edit(lines)")
    got = mod.edit(list(func))
    if not isinstance(got, list) or not all(isinstance(x, str) for x in got):
        raise SystemExit("edit(lines) must return a list of strings")
    return got


def apply_diff(diff: Path, text: str, workdir: Path) -> str:
    """Apply a unified diff to the whole listing with patch(1); fuzz allowed."""
    src, out = workdir / "diff-in.s", workdir / "diff-out.s"
    src.write_text(text)
    proc = subprocess.run(
        ["patch", "-s", "-F", "3", "--no-backup-if-mismatch", "-o", str(out), str(src)],
        input=diff.read_text(), capture_output=True, text=True)
    if proc.returncode:
        raise SystemExit("patch rejected the diff:\n" + (proc.stdout + proc.stderr)[-1500:])
    return out.read_text()


# ------------------------------------------------------------ compiler glue

def swap_to_S(args: list[str]) -> list[str]:
    """-c to -S, drop -o, make -I paths absolute.

    `cc -S` ignores -o and writes `<base>.s` into the CURRENT directory, so the
    compile runs in scratch; the recipe's relative include paths must therefore
    stop being relative to the repository root.
    """
    # abspath, not resolve(): the instrumented cc is a symlink to the stock driver,
    # which finds its pass binaries by the path it was invoked as.
    out, i = [os.path.abspath(ROOT / args[0])], 1
    while i < len(args):
        if args[i] == "-o":
            i += 2
            continue
        if args[i] == "-I" and i + 1 < len(args):
            out += ["-I", str((ROOT / args[i + 1]).resolve())]
            i += 2
            continue
        out.append("-S" if args[i] == "-c" else args[i])
        i += 1
    return out


class Recipe:
    """The configured compile for one symbol's TU, ugen listing and as flags."""

    def __init__(self, symbol: str, candidate: Path | None, workdir: Path,
                 instrumented: bool = False):
        self.symbol = symbol
        self.instrumented = instrumented_wanted(os.environ, instrumented)
        compiler = None
        if self.instrumented:
            compiler = INSTRUMENTED_CC
            if not compiler.is_file():
                raise SystemExit(f"instrumented compiler not found: {compiler}")
        self.workdir = workdir
        self.source = fast_score.tracked_source_for(symbol)
        self.args = use_compiler(fast_score.configured_cc_args(self.source), compiler)
        self.cand = workdir / "tu.c"
        self.cand.write_text((candidate or ROOT / self.source).read_text())
        self.base_obj = workdir / "base.o"
        args = fast_score.rewrite_io(self.args, self.cand, self.source, self.base_obj)
        shown = subprocess.run([args[0], "-show"] + args[1:], cwd=ROOT, capture_output=True, text=True)
        if shown.returncode:
            raise SystemExit(shown.stderr[-2000:])
        out = shown.stdout + shown.stderr
        self.as0_flags = tool_flags(out, "ugen")
        self.as1_flags = tool_flags(out, "as1")
        s_args = swap_to_S(fast_score.rewrite_io(self.args, self.cand, self.source, self.base_obj))
        proc = subprocess.run(s_args, cwd=workdir, capture_output=True, text=True)
        listing = workdir / "tu.s"
        if proc.returncode or not listing.is_file():
            raise SystemExit("cc -S failed:\n" + proc.stderr[-2000:])
        self.listing = listing.read_text()

    def assemble(self, text: str, tag: str, keep: Path | None = None) -> Path:
        """as0 then as1 on `text` with the C flags; returns the object path."""
        d = self.workdir / tag
        d.mkdir(exist_ok=True)
        s, b, t, o = d / "x.s", d / "x.b", d / "x.t", d / "x.o"
        s.write_text(text)
        for cmd in ([str(IDO / "as0")] + self.as0_flags + [str(s), "-o", str(b), "-t", str(t)],
                    [str(IDO / "as1")] + self.as1_flags + [str(b), "-o", str(o), "-t", str(t)]):
            proc = subprocess.run(cmd, cwd=d, capture_output=True, text=True)
            if proc.returncode:
                raise RuntimeError(f"{Path(cmd[0]).name} failed: " + (proc.stderr or proc.stdout)[-600:])
        if keep:
            keep.mkdir(parents=True, exist_ok=True)
            (keep / f"{tag}.s").write_text(text)
            (keep / f"{tag}.o").write_bytes(o.read_bytes())
        return o


def same_function(a: Path, b: Path, symbol: str) -> bool:
    """Function bytes and relocation records equal in the two objects."""
    import nm_ranking as nr
    sa, sb = nr.func_symbol_span(a, symbol), nr.func_symbol_span(b, symbol)
    if sa is None or sb is None or sa[1] != sb[1]:
        return False
    return (nr.text_bytes(a, *sa) == nr.text_bytes(b, *sb)
            and nr.relocations(a, *sa) == nr.relocations(b, *sb))


def make_scorer(symbol: str, workdir: Path):
    import align_symbol
    try:
        return align_symbol.AlignedScorer(symbol, workdir)
    except SystemExit:
        return None  # a matched function: only byte identity is meaningful


def row_for(scorer, obj: Path, base: Path, symbol: str) -> dict:
    row = {"same": same_function(obj, base, symbol)}
    if scorer is not None:
        got = scorer.score(obj)
        if got:
            row.update(got)
    return row


def render(row: dict) -> str:
    if "masked" not in row:
        return "identical to the configured object" if row["same"] else "differs from the configured object"
    import align_symbol
    return (f"masked {row['masked']} delta {row['delta']:+d}  "
            + align_symbol.render_buckets(row))


# ------------------------------------------------------------ main

def build_function(args, rec: Recipe) -> tuple[list[str], list[str], list[str]]:
    text = rec.listing
    if args.edit:
        text = apply_diff(Path(args.edit), text, rec.workdir)
    head, func, tail = split_function(text, rec.symbol)
    if args.script:
        func = apply_script(Path(args.script), func)
    return head, func, tail


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--source", help="candidate TU to compile (default: the tracked source)")
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--edit", help="unified diff against the full .s listing")
    g.add_argument("--script", help="python file defining edit(lines) -> lines")
    ap.add_argument("--enumerate", metavar="RANGES",
                    help="1-based inclusive function-line ranges to permute, e.g. 40-43,60-62")
    ap.add_argument("--instrumented", action="store_true",
                    help="use the instrumented compiler for the listing and the control object "
                         "(implied when any CDX_*, DKWB_CUT_* or DKWB_SUBST_* variable is set)")
    ap.add_argument("--dump", action="store_true", help="print the numbered function listing and exit")
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--max-cells", type=int, default=5040)
    ap.add_argument("--top", type=int, default=12)
    ap.add_argument("--keep", help="directory to keep .s/.o files in")
    ns = ap.parse_args(argv)
    keep = Path(ns.keep).resolve() if ns.keep else None

    with tempfile.TemporaryDirectory(prefix="stream-surgery-") as tmp:
        work = Path(tmp)
        rec = Recipe(ns.symbol, Path(ns.source) if ns.source else None, work, ns.instrumented)
        head, func, tail = build_function(ns, rec)
        if ns.dump:
            for i, line in enumerate(func, 1):
                print(f"{i:4d}  {line}")
            return 0
        scorer = make_scorer(ns.symbol, work)
        control = rec.assemble(rec.listing, "control", keep)
        if rec.instrumented:
            print("compiler   : instrumented (control compares against the instrumented object)")
        print("control (untouched listing, C flags): "
              + ("identical to the configured object" if same_function(control, rec.base_obj, ns.symbol)
                 else "DIFFERS from the configured object -- results below are not evidence"))
        base_row = row_for(scorer, rec.base_obj, rec.base_obj, ns.symbol)
        print("configured : " + render(base_row))

        if not ns.enumerate:
            obj = rec.assemble(join_listing(head, func, tail), "edit", keep)
            print("edited     : " + render(row_for(scorer, obj, rec.base_obj, ns.symbol)))
            return 0

        ranges = parse_ranges(ns.enumerate, len(func))
        cells = list(orders(len(ranges), ns.max_cells))

        def run(i_order):
            i, order = i_order
            try:
                obj = rec.assemble(join_listing(head, permute_chunks(func, ranges, order), tail), f"c{i}")
                row = row_for(scorer, obj, rec.base_obj, ns.symbol)
            except RuntimeError as exc:
                row = {"error": str(exc)}
            row["order"] = order
            (work / f"c{i}" / "x.o").unlink(missing_ok=True)
            return row

        with ThreadPoolExecutor(max_workers=ns.jobs) as pool:
            rows = list(pool.map(run, enumerate(cells)))
        ok = [r for r in rows if "error" not in r]
        if scorer is not None:
            ok.sort(key=lambda r: (r["residual"], abs(r["delta"]), r["masked"]))
            exact = [r for r in ok if r["masked"] == 0 and r["delta"] == 0]
        else:
            exact = [r for r in ok if r["same"]]
        print(f"{len(cells)} orderings of {len(ranges)} chunks; exact cells: {len(exact)}; "
              f"assembly errors: {len(rows) - len(ok)}")
        for r in (exact or ok)[: ns.top]:
            print(f"  order {list(r['order'])}: {render(r)}")
        return 0 if exact else 1


if __name__ == "__main__":
    sys.exit(main())
