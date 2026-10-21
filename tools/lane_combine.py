#!/usr/bin/env python3
"""Combine several lanes' candidates of one TU into one SHAPE_ product.

    tools/lane_combine.py <base.c> --lane NAME=cand.c [--lane NAME=cand.c ...]
                          [--rebase newbase.c] [--function SYM] [--per-hunk]
                          [--strict] [--json PATH] -o product.c

Several lanes work on the same function from the same base, and each comes back
with a whole-TU candidate. What the next round needs is the product of their
edits, measured whole: hunks that regress alone can be exact together. This tool
diffs each candidate against the base line by line and writes every hunk into
one TU as a preprocessor alternative,

    #if SHAPE_<NAME> == 1
    <the lane's lines>
    #else
    <the base lines>
    #endif

so `tools/shape_product.py <sym> product.c` measures every on/off combination
of the lanes (`--per-hunk` makes each hunk its own axis, `SHAPE_<NAME>_<k>`, to
find which of a lane's hunks pay).

**Overlaps.** Two lanes that edit the same base lines (or insert at the same
point) cannot be switched independently there. Their hunks are joined into one
region spanning both, each lane's text for the region is the base region with
that lane's hunks applied, and the region becomes one chain,
`#if SHAPE_A == 1 ... #elif SHAPE_B == 1 ... #else <base> #endif`, so the
earlier `--lane` wins where both are on. Every overlap is reported with its base
line ranges and function; `--strict` refuses instead.

**Rebase.** `--rebase newbase.c` builds the product over a newer base: each lane
hunk (computed against `<base.c>`) is carried onto the new base where the base
lines it touched are unchanged there, a three-way apply with zero context. A hunk
whose base lines the new base also changed is reported as a rebase conflict and
left out (`--strict` refuses).

`--function SYM` keeps only the hunks inside that function's body and reports
the rest. Each hunk is reported with its base line range and enclosing function
(or `file scope`). Exit status: 0 written, 1 refused (`--strict`), 2 bad input.
"""

from __future__ import annotations

import argparse
import difflib
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import permute_transplant  # noqa: E402  (code_mask: comments and literals)

NAME_RE = re.compile(r"^[A-Za-z0-9_]+$")
KEYWORDS = {"if", "for", "while", "switch", "return", "sizeof", "do", "else", "case"}


class CombineError(RuntimeError):
    pass


@dataclass
class Hunk:
    lane: str
    start: int  # base line index, inclusive (0-based)
    end: int  # base line index, exclusive; start == end is an insertion
    lines: list[str]
    function: str = "file scope"
    index: int = 0  # ordinal within its lane
    conflict: str | None = None

    def axis(self, per_hunk: bool) -> str:
        return f"SHAPE_{self.lane}_{self.index}" if per_hunk else f"SHAPE_{self.lane}"

    def label(self) -> str:
        span = (f"{self.start + 1}-{self.end}" if self.end > self.start
                else f"insert before {self.start + 1}")
        return f"{self.lane}#{self.index} base {span} (+{len(self.lines)}/-{self.end - self.start}) in {self.function}"


@dataclass
class Region:
    start: int
    end: int
    hunks: list[Hunk] = field(default_factory=list)


def split_lines(text: str) -> list[str]:
    return text.split("\n")


HEAD_RE = re.compile(r"\b([A-Za-z_]\w*)\s*\([^()]*\)\s*(?:[^;{}()=]*;\s*)*$")


COND_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b")


def code_only(text: str) -> str:
    """`text` with comments, literals and preprocessor lines blanked (newlines kept).

    Only the first arm of each `#if` chain is kept: alternative arms (SHAPE_
    axes, `#else` fallbacks) each carry their own braces, and counting them all
    would unbalance every function after the first axis.
    """
    mask = permute_transplant.code_mask(text)
    chars = [c if mask[i] or c == "\n" else " " for i, c in enumerate(text)]
    out = []
    stack: list[bool] = []  # per open conditional: is the current arm a later one?
    for line in "".join(chars).split("\n"):
        m = COND_RE.match(line)
        if m:
            kind = m.group(1)
            if kind.startswith("if"):
                stack.append(False)
            elif kind in ("elif", "else") and stack:
                stack[-1] = True
            elif kind == "endif" and stack:
                stack.pop()
            out.append(" " * len(line))
        elif line.lstrip().startswith("#") or any(stack):
            out.append(" " * len(line))
        else:
            out.append(line)
    return "\n".join(out)


def function_spans(text: str) -> list[tuple[str, int, int]]:
    """(name, first line, last line) of each top-level function definition, 0-based.

    A top-level `{` opens a function body when the code since the previous
    top-level `}` ends in `name(params)`, optionally followed by K&R parameter
    declarations. The span runs from the line naming the function to its `}`.
    """
    code = code_only(text)
    spans = []
    depth, last_close, current = 0, 0, None
    line = 0
    for i, ch in enumerate(code):
        if ch == "\n":
            line += 1
        elif ch == "{":
            if depth == 0:
                m = HEAD_RE.search(code, last_close, i)
                current = None
                if m and m.group(1) not in KEYWORDS:
                    current = (m.group(1), code.count("\n", 0, m.start()))
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                if current:
                    spans.append((current[0], current[1], line))
                current, last_close = None, i + 1
    return spans


def enclosing(spans: list[tuple[str, int, int]], start: int, end: int) -> str:
    last = max(end - 1, start)
    for fname, first, final in spans:
        if first <= start and last <= final:
            return fname
    return "file scope"


def directive_balanced(lines: list[str]) -> bool:
    """True when `lines` hold only whole `#if ... #endif` chains."""
    depth = 0
    for line in lines:
        m = COND_RE.match(line)
        if not m:
            continue
        kind = m.group(1)
        if kind.startswith("if"):
            depth += 1
        elif kind in ("elif", "else") and depth == 0:
            return False
        elif kind == "endif":
            depth -= 1
            if depth < 0:
                return False
    return depth == 0


def cond_blocks(lines: list[str]) -> list[tuple[int, int, list[int]]]:
    """(first, last, directive lines) of every `#if ... #endif` chain, 0-based, last inclusive."""
    stack: list[list[int]] = []
    out = []
    for i, line in enumerate(lines):
        m = COND_RE.match(line)
        if not m:
            continue
        kind = m.group(1)
        if kind.startswith("if"):
            stack.append([i])
        elif stack:
            stack[-1].append(i)
            if kind == "endif":
                rows = stack.pop()
                out.append((rows[0], i, rows))
    return out


def _widen(s: int, e: int, blocks) -> tuple[int, int] | None:
    """Grow [s, e) over the innermost chain it cuts through, or None if none does."""
    cut = [b for b in blocks if any(s <= d < e for d in b[2]) and not (s <= b[0] and b[1] < e)]
    if not cut:
        return None
    first, last, _ = min(cut, key=lambda b: b[1] - b[0])
    return min(s, first), max(e, last + 1)


def lane_hunks(base: list[str], cand: list[str], lane: str) -> list[Hunk]:
    """The lane's hunks against `base`, each grown to whole preprocessor chains.

    A raw line hunk can delete an `#if` and keep its `#endif` (a lane that
    resolved a SHAPE_ axis does exactly that); wrapped as it stands it would
    break the nesting of the product. Each hunk is widened over the chains it
    cuts on the base side, and merged with its lane's next hunk while its own
    lines are unbalanced, until both sides are whole.
    """
    matcher = difflib.SequenceMatcher(None, base, cand, autojunk=False)
    raw = [Hunk(lane, i1, i2, cand[j1:j2])
           for tag, i1, i2, j1, j2 in matcher.get_opcodes() if tag != "equal"]
    blocks = cond_blocks(base)
    out: list[Hunk] = []
    k = 0
    while k < len(raw):
        members = [raw[k]]
        s, e = raw[k].start, raw[k].end
        k += 1
        for _ in range(10000):
            changed = False
            while out and out[-1].end > s:
                prev = out.pop()
                members = prev.members + members  # type: ignore[attr-defined]
                s, e = min(s, prev.start), max(e, prev.end)
                changed = True
            while k < len(raw) and raw[k].start < e:
                members.append(raw[k])
                e = max(e, raw[k].end)
                k += 1
                changed = True
            if not directive_balanced(base[s:e]):
                grown = _widen(s, e, blocks)
                if grown is not None and grown != (s, e):
                    s, e = grown
                    changed = True
            elif not directive_balanced(apply_within(base, s, e, members)) and k < len(raw):
                members.append(raw[k])
                e = max(e, raw[k].end)
                k += 1
                changed = True
            if not changed:
                break
        hunk = Hunk(lane, s, e, apply_within(base, s, e, members))
        hunk.members = members  # type: ignore[attr-defined]
        out.append(hunk)
    for index, h in enumerate(out):
        h.index = index
        if not (directive_balanced(base[h.start:h.end]) and directive_balanced(h.lines)):
            h.conflict = "cannot be made preprocessor-balanced"
    return out


def overlaps(a: Hunk, b: Hunk) -> bool:
    """Same base lines touched, or the same insertion point."""
    if a.start == a.end and b.start == b.end:
        return a.start == b.start
    if a.start == a.end:
        return b.start < a.start < b.end
    if b.start == b.end:
        return a.start < b.start < a.end
    return a.start < b.end and b.start < a.end


def regions(hunks: list[Hunk]) -> list[Region]:
    """Group hunks into disjoint regions; a region holding two lanes is an overlap."""
    ordered = sorted(hunks, key=lambda h: (h.start, h.end))
    out: list[Region] = []
    for h in ordered:
        if out and any(overlaps(h, other) for other in out[-1].hunks):
            region = out[-1]
            region.start, region.end = min(region.start, h.start), max(region.end, h.end)
            region.hunks.append(h)
        else:
            out.append(Region(h.start, h.end, [h]))
    # A merged region can grow into the next one; merge until stable.
    merged: list[Region] = []
    for r in out:
        if merged and any(overlaps(x, y) for x in merged[-1].hunks for y in r.hunks):
            merged[-1].start = min(merged[-1].start, r.start)
            merged[-1].end = max(merged[-1].end, r.end)
            merged[-1].hunks += r.hunks
        else:
            merged.append(r)
    return merged


def apply_within(base: list[str], start: int, end: int, hunks: list[Hunk]) -> list[str]:
    """base[start:end] with the given (non-overlapping, one-lane) hunks applied."""
    out, pos = [], start
    for h in sorted(hunks, key=lambda h: (h.start, h.end)):
        out += base[pos:h.start]
        out += h.lines
        pos = h.end
    return out + base[pos:end]


def base_map(old: list[str], new: list[str]) -> dict[int, int]:
    """old line index -> new line index for lines the two texts share."""
    mapping = {}
    matcher = difflib.SequenceMatcher(None, old, new, autojunk=False)
    for tag, i1, i2, j1, _j2 in matcher.get_opcodes():
        if tag == "equal":
            for k in range(i2 - i1):
                mapping[i1 + k] = j1 + k
    return mapping


def already_applied(h: Hunk, mapping: dict[int, int], new: list[str], old_len: int) -> bool:
    """The new base already reads the lane's lines between the hunk's unchanged neighbours."""
    before = mapping.get(h.start - 1) if h.start > 0 else -1
    after = mapping.get(h.end) if h.end < old_len else len(new)
    if before is None or after is None:
        return False
    return new[before + 1:after] == h.lines


def rebase_hunk(h: Hunk, mapping: dict[int, int], old_len: int, new_len: int) -> Hunk | None:
    """Carry a hunk onto the new base, or None when the new base changed its lines."""
    if h.end > h.start:
        js = [mapping.get(k) for k in range(h.start, h.end)]
        if None in js or js != list(range(js[0], js[0] + len(js))):
            return None
        return Hunk(h.lane, js[0], js[0] + len(js), h.lines, h.function, h.index)
    before = mapping.get(h.start - 1) if h.start > 0 else -1
    after = mapping.get(h.start) if h.start < old_len else new_len
    if before is None or after is None or after != before + 1:
        return None
    return Hunk(h.lane, after, after, h.lines, h.function, h.index)


def render(base: list[str], regs: list[Region], per_hunk: bool, lane_order: list[str]) -> list[str]:
    out, pos = [], 0
    for r in regs:
        out += base[pos:r.start]
        lanes = sorted({h.lane for h in r.hunks}, key=lane_order.index)
        if len(lanes) == 1 and len(r.hunks) == 1:
            h = r.hunks[0]
            out.append(f"#if {h.axis(per_hunk)} == 1")
            out += h.lines
            out.append("#else")
            out += base[r.start:r.end]
            out.append("#endif")
        else:
            first = True
            for lane in lanes:
                mine = [h for h in r.hunks if h.lane == lane]
                cond = " || ".join(f"{h.axis(per_hunk)} == 1" for h in mine) if per_hunk else \
                    f"{mine[0].axis(False)} == 1"
                out.append(f"#{'if' if first else 'elif'} {cond}")
                out += apply_within(base, r.start, r.end, mine)
                first = False
            out.append("#else")
            out += base[r.start:r.end]
            out.append("#endif")
        pos = r.end
    return out + base[pos:]


def combine(base_text: str, lanes: list[tuple[str, str]], *, rebase_text: str | None = None,
            function: str | None = None, per_hunk: bool = False) -> dict:
    base = split_lines(base_text)
    spans = function_spans(base_text)
    lane_order = [name for name, _ in lanes]
    hunks: list[Hunk] = []
    ignored: list[Hunk] = []
    conflicts: list[Hunk] = []
    for name, text in lanes:
        for h in lane_hunks(base, split_lines(text), name):
            h.function = enclosing(spans, h.start, h.end)
            if function is not None and h.function != function:
                ignored.append(h)
            elif h.conflict:
                conflicts.append(h)
            else:
                hunks.append(h)
    applied: list[Hunk] = []
    target = base
    if rebase_text is not None:
        new = split_lines(rebase_text)
        mapping = base_map(base, new)
        carried = []
        for h in hunks:
            moved = rebase_hunk(h, mapping, len(base), len(new))
            if moved is not None:
                carried.append(moved)
            elif already_applied(h, mapping, new, len(base)):
                applied.append(h)
            else:
                h.conflict = "the new base changed these lines"
                conflicts.append(h)
        hunks, target = carried, new
    regs = regions(hunks)
    overlap_regions = [r for r in regs if len({h.lane for h in r.hunks}) > 1]
    for r in overlap_regions:
        versions = [target[r.start:r.end]] + [
            apply_within(target, r.start, r.end, [h for h in r.hunks if h.lane == lane])
            for lane in {h.lane for h in r.hunks}]
        if not all(directive_balanced(v) for v in versions):
            raise CombineError(f"overlap at base lines {r.start + 1}-{r.end} cuts a preprocessor chain; "
                               "combine those lanes by hand")
    product = render(target, regs, per_hunk, lane_order)
    axes = sorted({h.axis(per_hunk) for h in hunks})
    return {"product": "\n".join(product), "hunks": hunks, "ignored": ignored, "conflicts": conflicts,
            "applied": applied,
            "overlaps": overlap_regions, "axes": axes, "rebased": rebase_text is not None}


def report_lines(result: dict, per_hunk: bool) -> list[str]:
    lines = []
    by_lane: dict[str, list[Hunk]] = {}
    for h in result["hunks"]:
        by_lane.setdefault(h.lane, []).append(h)
    for lane, hs in by_lane.items():
        lines.append(f"lane {lane}: {len(hs)} hunk(s)")
        for h in hs:
            lines.append(f"  {h.axis(per_hunk)}  {h.label()}")
    for h in result["ignored"]:
        lines.append(f"ignored (outside --function): {h.label()}")
    for h in result["applied"]:
        lines.append(f"already in the new base: {h.label()}")
    for h in result["conflicts"]:
        lines.append(f"LEFT OUT: {h.label()}: {h.conflict}")
    for r in result["overlaps"]:
        where = sorted({h.function for h in r.hunks})
        detail = "; ".join(f"{h.lane} {h.start + 1}-{max(h.end, h.start + 1)}" for h in r.hunks)
        lanes = []
        for h in r.hunks:
            if h.lane not in lanes:
                lanes.append(h.lane)
        lines.append(f"OVERLAP {'new ' if result['rebased'] else ''}base lines {r.start + 1}-{max(r.end, r.start + 1)} "
                     f"in {', '.join(where)}: {detail}; {lanes[0]} wins where several are on")
    lines.append(f"axes: {', '.join(result['axes']) or 'none'}")
    return lines


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("base", type=Path)
    ap.add_argument("--lane", action="append", required=True, metavar="NAME=cand.c")
    ap.add_argument("--rebase", type=Path, metavar="newbase.c")
    ap.add_argument("--function", metavar="SYM", help="keep only hunks inside this function")
    ap.add_argument("--per-hunk", action="store_true", help="one axis per hunk, SHAPE_<NAME>_<k>")
    ap.add_argument("--strict", action="store_true", help="refuse on any overlap or rebase conflict")
    ap.add_argument("--json", type=Path, metavar="PATH")
    ap.add_argument("-o", "--output", type=Path, required=True)
    ns = ap.parse_args(argv)
    lanes = []
    try:
        for spec in ns.lane:
            name, sep, path = spec.partition("=")
            if not sep or not NAME_RE.match(name):
                raise CombineError(f"--lane wants NAME=path with NAME in [A-Za-z0-9_]: {spec!r}")
            if name in [n for n, _ in lanes]:
                raise CombineError(f"lane name {name!r} given twice")
            lanes.append((name, Path(path).read_text()))
        result = combine(ns.base.read_text(), lanes,
                         rebase_text=ns.rebase.read_text() if ns.rebase else None,
                         function=ns.function, per_hunk=ns.per_hunk)
    except (CombineError, OSError) as error:
        print(f"lane_combine: {error}", file=sys.stderr)
        return 2
    for line in report_lines(result, ns.per_hunk):
        print(line)
    if ns.json:
        ns.json.write_text(json.dumps({
            "axes": result["axes"],
            "hunks": [{"lane": h.lane, "index": h.index, "axis": h.axis(ns.per_hunk), "start": h.start + 1,
                       "end": h.end, "added": len(h.lines), "function": h.function} for h in result["hunks"]],
            "conflicts": [{"lane": h.lane, "index": h.index, "start": h.start + 1, "end": h.end,
                           "function": h.function} for h in result["conflicts"]],
            "overlaps": [{"start": r.start + 1, "end": r.end,
                          "hunks": [f"{h.lane}#{h.index}" for h in r.hunks]} for r in result["overlaps"]],
        }, indent=1) + "\n")
    if ns.strict and (result["overlaps"] or result["conflicts"]):
        print("lane_combine: refused (--strict): overlaps or rebase conflicts above", file=sys.stderr)
        return 1
    if ns.output.resolve() == ns.base.resolve():
        print("lane_combine: -o must not be the base", file=sys.stderr)
        return 2
    ns.output.write_text(result["product"])
    print(f"wrote {ns.output} ({len(result['hunks'])} hunk(s), {len(result['axes'])} axis/axes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
