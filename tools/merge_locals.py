#!/usr/bin/env python3
"""Merge same-typed locals pairwise: which merges are free, which move the code.

    tools/merge_locals.py <symbol> [--source FILE] [--jobs N] [--top N]
                          [--keep-decl] [--target-frame N] [--write OUT.c] [--json PATH]

A local's stack home is a frame cell. Two locals of one type that the
allocator keeps in registers (or never uses at the same time) can share a
name, which drops a declaration and with it a frame cell. A target frame that
is bigger or smaller than ours is therefore often a question of which locals
the original author reused, and the answer is invisible to every score that
does not look at the frame. On func_8001291C (frame 0x2A8 against 0x288)
five such merges were byte-identical outside the frame and together closed
the frame without moving a register; finding them by hand took 300 compiles.

For every ordered pair (KEEP, DROP) of locals with the same declared type
and pointer depth (arrays and structs excluded) in the symbol's
NON_MATCHING body this tool renames every use of DROP to KEEP, drops DROP's
declarator (`--keep-decl` keeps it), compiles the TU with the configured
recipe, and classifies the cell by comparing the function's instructions with
the unmerged base after masking every `N($sp)` offset and the frame
adjustment:

  inert         the function bytes equal the base's (the merge changed nothing)
  frame-only    the instructions are identical outside stack offsets and
                the frame adjustment: an allocation-neutral merge
  code-changing anything else; ranked by aligned residual (align_symbol)

Then every compatible frame-only merge is applied greedily, largest frame
reduction first, re-checking after each step that the result is still
frame-only against the ORIGINAL base (a merge that was neutral alone can stop
being neutral on a body another merge changed), and the frame delta and the
aligned score of the merged body are reported. `--write` saves that merged TU.

Caveats. The rename is lexical (a word not preceded by `.`, `>` or a word
character), so a macro that expands to the dropped name or a struct member
spelled like a local needs a manual look. A DROP declared with an initializer
is skipped. Merging live locals changes behaviour: this is a matching
instrument, and a winner must be read against the whole scope before it is
adopted. The pure text half lives here so `tools/lever_sweep.py` can use the
same renaming.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

# ------------------------------------------------------------ pure text half

TYPE_RE = re.compile(r"^\s*((?:(?:const|volatile|static|register|unsigned|signed|struct|union|enum)\s+)*"
                     r"[A-Za-z_]\w*)")
IDENT_RE = re.compile(r"[A-Za-z_]\w*")


def rename_str(s: str, old: str, new: str) -> str:
    """Rename the word `old` to `new`; member accesses (`.old`, `->old`) are left alone."""
    return re.sub(rf"(?<![\w.>]){re.escape(old)}\b", new, s)


def rename_word(t: str, start: int, end: int, old: str, new: str) -> str:
    """Rename `old` to `new` in t[start:end] only."""
    return t[:start] + rename_str(t[start:end], old, new) + t[end:]


def _split_top(s: str) -> list[str]:
    out, depth, cur = [], 0, []
    for c in s:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == "," and depth == 0:
            out.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    out.append("".join(cur))
    return out


def _declarator_name(d: str) -> str | None:
    m = IDENT_RE.search(d.split("=", 1)[0])
    return m.group(0) if m else None


def drop_declarator(decl: str, name: str) -> str | None:
    """`decl` (one declaration statement, with its `;`) without `name`'s declarator.

    Returns "" when `name` was the only declarator, the rewritten declaration
    otherwise, and None when `name` is absent or carries an initializer.
    """
    core = decl.strip()
    if not core.endswith(";"):
        return None
    core = core[:-1]
    parts = _split_top(core)
    m = TYPE_RE.match(parts[0])
    if not m:
        return None
    base = m.group(1)
    decls = [parts[0][m.end():]] + parts[1:]
    keep, found = [], False
    for d in decls:
        if _declarator_name(d) == name:
            if "=" in d:
                return None
            found = True
        else:
            keep.append(d.strip())
    if not found:
        return None
    if not keep:
        return ""
    lead = decl[:len(decl) - len(decl.lstrip())]
    return f"{lead}{base} {', '.join(keep)};"


def merge_text(text: str, scope: tuple[int, int], decl: tuple[int, int], keep: str, drop: str,
               drop_decl: bool = True) -> str | None:
    """Rename `drop` to `keep` over `scope`, optionally removing `drop`'s declarator.

    `decl` is the span of the declaration statement that declares `drop`; it is
    never renamed. It may lie inside `scope` or wholly before it. None when the
    declarator cannot be dropped (initializer, absent).
    """
    (s0, s1), (d0, d1) = scope, decl
    new_decl = text[d0:d1]
    if drop_decl:
        new_decl = drop_declarator(new_decl, drop)
        if new_decl is None:
            return None
    if s0 <= d0 and d1 <= s1:
        body = (rename_str(text[s0:d0], drop, keep) + new_decl
                + rename_str(text[d1:s1], drop, keep))
        return text[:s0] + body + text[s1:]
    if d1 <= s0:
        renamed = rename_word(text, s0, s1, drop, keep)
        return renamed[:d0] + new_decl + renamed[d1:]
    return None


# ------------------------------------------------------------ object half

SP_OFFSET = re.compile(r"-?(?:0x[0-9a-f]+|\d+)\((?:sp|\$sp)\)")
# `addiu sp,sp,-N` (the frame) and `addiu rX,sp,N` (the address of a local)
SP_ADJUST = re.compile(r"\b((?:addiu|daddiu|addi)\s+[$\w]+,(?:sp|\$sp),)-?(?:0x[0-9a-f]+|\d+)")
FRAME_RE = re.compile(r"\b(?:addiu|daddiu|addi)\s+(?:sp|\$sp),(?:sp|\$sp),(-?(?:0x[0-9a-f]+|\d+))")


def normalize_listing(rows: list[str]) -> list[str]:
    """Instruction text with every stack offset and the frame adjustment masked."""
    out = []
    for r in rows:
        r = SP_OFFSET.sub("N(sp)", r)
        r = SP_ADJUST.sub(r"\1N", r)
        out.append(r)
    return out


def frame_size(rows: list[str]) -> int:
    for r in rows:
        m = FRAME_RE.search(r)
        if m and m.group(1).startswith("-"):
            return -int(m.group(1), 0)
    return 0


def function_rows(objdump_text: str, symbol: str) -> list[str]:
    """Instruction rows (address and raw word stripped, relocations kept) of one symbol."""
    rows, inside = [], False
    for line in objdump_text.splitlines():
        if re.match(rf"^[0-9a-f]+ <{re.escape(symbol)}>:", line):
            inside = True
            continue
        if inside and re.match(r"^[0-9a-f]+ <", line):
            break
        if not inside:
            continue
        m = re.match(r"\s*[0-9a-f]+:\t[0-9a-f]{8} \t(.*)", line)
        if m:
            rows.append(re.sub(r"\s+", " ", m.group(1).replace("\t", " ").strip()))
            continue
        m = re.match(r"\s*[0-9a-f]+: R_MIPS_\w+\t(.*)", line)
        if m and rows:
            rows[-1] += " @" + m.group(1).strip()
    return rows


def classify(base_rows: list[str], rows: list[str]) -> str:
    if rows == base_rows:
        return "inert"
    if normalize_listing(rows) == normalize_listing(base_rows):
        return "frame-only"
    return "code-changing"


# ------------------------------------------------------------ driver

def disassemble(obj: Path, symbol: str) -> list[str]:
    import fast_score
    d = subprocess.run([str(fast_score.ROOT / "tools/binutils/mips64-elf-objdump"), "-d", "-r", "-z", str(obj)],
                       capture_output=True, text=True).stdout
    return function_rows(d, symbol)


def candidate_pairs(fn, sweep) -> list[tuple[str, str]]:
    """Ordered (keep, drop) pairs of same-typed scalar/pointer locals."""
    out = []
    for x, ix in fn.locals.items():
        for y, iy in fn.locals.items():
            if x == y or ix["type"] != iy["type"] or ix["ptr"] != iy["ptr"]:
                continue
            if sweep.category(ix) not in ("int", "pointer", "float") or sweep.category(iy) not in ("int", "pointer", "float"):
                continue
            out.append((x, y))
    return out


def merged_source(text: str, symbol: str, keep: str, drop: str, drop_decl: bool, sweep) -> str | None:
    fn = sweep.find_function(text, symbol)
    if keep not in fn.locals or drop not in fn.locals:
        return None
    decl = fn.locals[drop]["decl"]
    return merge_text(text, (fn.body.start + 1, fn.body.end), (decl.start, decl.end), keep, drop, drop_decl)


class Bench:
    """Compile candidate texts of one TU and read their function + score."""

    def __init__(self, symbol: str, source: str, base_args: list[str], work: Path, scorer):
        self.symbol, self.source, self.base_args, self.work, self.scorer = symbol, source, base_args, work, scorer
        self.n = 0

    def run(self, text: str, tag: str) -> dict:
        import fast_score
        cand = self.work / f"{tag}.c"
        obj = self.work / f"{tag}.o"
        cand.write_text(text)
        args = fast_score.rewrite_io(self.base_args, cand, self.source, obj)
        cc = subprocess.run(args, cwd=fast_score.ROOT, capture_output=True, text=True)
        res = {"tag": tag}
        if cc.returncode:
            res["error"] = (cc.stderr.strip().splitlines() or ["compile failed"])[-1][:100]
            return res
        res["rows"] = disassemble(obj, self.symbol)
        res["frame"] = frame_size(res["rows"])
        if self.scorer is not None:
            row = self.scorer.score(obj)
            if row:
                res.update(row)
        obj.unlink(missing_ok=True)
        cand.unlink(missing_ok=True)
        return res


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--source", help="candidate TU (default: the tracked source)")
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--top", type=int, default=12)
    ap.add_argument("--keep-decl", action="store_true", help="rename uses but keep DROP's declaration")
    ap.add_argument("--target-frame", type=lambda s: int(s, 0), metavar="N",
                    help="stop the greedy pass at this frame size (never go below it)")
    ap.add_argument("--write", metavar="OUT.c", help="write the greedy frame-only merged TU here")
    ap.add_argument("--json", metavar="PATH")
    ns = ap.parse_args(argv)

    import fast_score
    import lever_sweep as sweep
    import align_symbol

    source = fast_score.tracked_source_for(ns.symbol)
    text = Path(ns.source).read_text() if ns.source else (fast_score.ROOT / source).read_text()
    base_args = fast_score.configured_cc_args(source)
    drop_decl = not ns.keep_decl
    fn = sweep.find_function(text, ns.symbol)
    pairs = candidate_pairs(fn, sweep)

    with tempfile.TemporaryDirectory(prefix="merge-locals-") as tmp:
        work = Path(tmp)
        try:
            scorer = align_symbol.AlignedScorer(ns.symbol, work)
        except SystemExit as exc:
            raise SystemExit(f"{exc}\nmerge_locals ranks by aligned residual and needs a queued symbol") from None
        bench = Bench(ns.symbol, source, base_args, work, scorer)
        base = bench.run(text, "base")
        if "error" in base:
            raise SystemExit("base does not compile: " + base["error"])
        print(f"{ns.symbol}: base masked {base['masked']} delta {base['delta']:+d} "
              f"residual {base['residual']} frame {base['frame']:#x}; {len(pairs)} ordered pairs of "
              f"{len(fn.locals)} locals", flush=True)

        def cell(i_pair):
            i, (keep, drop) = i_pair
            new = merged_source(text, ns.symbol, keep, drop, drop_decl, sweep)
            if new is None:
                return {"keep": keep, "drop": drop, "class": "skipped"}
            r = bench.run(new, f"c{i}")
            r.update(keep=keep, drop=drop)
            r["class"] = "error" if "error" in r else classify(base["rows"], r["rows"])
            r.pop("rows", None)
            return r

        with ThreadPoolExecutor(max_workers=ns.jobs) as pool:
            cells = list(pool.map(cell, enumerate(pairs)))
        by = {k: [c for c in cells if c["class"] == k] for k in ("inert", "frame-only", "code-changing", "error", "skipped")}
        print("cells: " + ", ".join(f"{k} {len(v)}" for k, v in by.items()), flush=True)

        # greedy: largest frame reduction first, each step re-checked against the base
        order = sorted(by["frame-only"], key=lambda c: (c["frame"], c["keep"], c["drop"]))
        cur, applied = text, []
        cur_res = base
        for c in order:
            new = merged_source(cur, ns.symbol, c["keep"], c["drop"], drop_decl, sweep)
            if new is None:
                continue
            r = bench.run(new, f"g{len(applied)}_{c['keep']}_{c['drop']}")
            if "error" in r or classify(base["rows"], r["rows"]) not in ("frame-only", "inert"):
                continue
            if r["frame"] > cur_res["frame"]:
                continue
            if ns.target_frame is not None and r["frame"] < ns.target_frame:
                continue
            cur, cur_res = new, r
            applied.append((c["drop"], c["keep"], r["frame"]))
            if ns.target_frame is not None and r["frame"] <= ns.target_frame:
                break
        final = bench.run(cur, "final")
        print(f"frame-only merges applied greedily: {len(applied)}")
        for drop, keep, fr in applied:
            print(f"  {drop} -> {keep}   frame {fr:#x}")
        print(f"frame {base['frame']:#x} -> {final['frame']:#x} ({final['frame'] - base['frame']:+d} bytes); "
              f"masked {base['masked']} -> {final['masked']}, residual {base['residual']} -> {final['residual']}, "
              f"delta {final['delta']:+d}")
        if ns.write:
            Path(ns.write).write_text(cur)
            print(f"merged TU written to {ns.write}")

        changed = [c for c in by["code-changing"] if "residual" in c]
        changed.sort(key=lambda c: (c["residual"], abs(c["delta"]), c["masked"]))
        print(f"code-changing cells by aligned residual (base {base['residual']}):")
        print(f"{'resid':>5} {'masked':>6} {'delta':>6} {'frame':>6}  merge")
        for c in changed[: ns.top]:
            print(f"{c['residual']:>5} {c['masked']:>6} {c['delta']:>+6} {c['frame']:>#6x}  {c['drop']} -> {c['keep']}")
        if ns.json:
            Path(ns.json).write_text(json.dumps({"symbol": ns.symbol, "base": {k: v for k, v in base.items() if k != "rows"},
                                                 "cells": cells, "applied": applied}, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
