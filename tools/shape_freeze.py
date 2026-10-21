#!/usr/bin/env python3
"""Resolve a candidate's SHAPE_ axes into plain C.

    tools/shape_freeze.py <cand.c> [--cell 'a=1,b=0'] [-o out.c] [--keep-macro NAME ...]
                          [--no-expand] [--default-zero]

A shape_product candidate carries its alternatives as preprocessor axes
(`#if SHAPE_x == 1 ... #else ... #endif`) and pins the chosen ones with
`#define SHAPE_x N`. Before a lane's candidate can be checked (def-before-use on
renamed locals, a diff against the tracked body) or promoted, those axes have
to be resolved, and `unifdef` was run by hand for it. This does that resolution
itself, so the frozen file is what the compiler saw:

- Every `#if`/`#elif` whose condition names a `SHAPE_` axis is evaluated with
  the pinned values (`#define SHAPE_x N` lines in the file, overridden by
  `--cell 'x=N,...'`), the live arm is kept and the dead arms and the
  directives themselves are dropped. The evaluator accepts integers, axis
  names, `== != < <= > >= && || ! + - *` and parentheses; anything else in a
  SHAPE condition (a `defined(...)`, a non-axis macro) is an error, never a
  guess. An axis with no value is an error unless `--default-zero`.
- Conditionals that name no axis (`#ifdef NON_MATCHING`) pass through
  untouched, with their nesting.
- `#define SHAPE_x N` lines are dropped.
- Carrier macros -- object-like `#define NAME IDENT` whose body is one
  identifier, the lanes' `#define T45 timer` -- are expanded at their uses and
  dropped, so the frozen body names the real local. `#define X X` guards (the
  TU's own callee placeholders) are never touched; `--keep-macro NAME` protects
  any other; `--no-expand` keeps every macro.

The frozen output compiles to the same object as the axis-bearing candidate
under the same `-DSHAPE_` cell: that is the test to run before trusting a
frozen file, and `tools/lever_sweep.py --emit` and `tools/shape_product.py
--emit-best` both go through `freeze_text`.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

AXIS_RE = re.compile(r"\bSHAPE_([A-Za-z0-9_]+)\b")
DEFINE_RE = re.compile(r"^\s*#\s*define\s+SHAPE_([A-Za-z0-9_]+)\s+(-?\d+)\s*$")
COND_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$")
CARRIER_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*$")
TOKEN_RE = re.compile(r"\s*(?:(\d+)|(SHAPE_[A-Za-z0-9_]+)|(==|!=|<=|>=|&&|\|\||[-+*<>!()]))")


class FreezeError(Exception):
    pass


def evaluate(cond: str, values: dict[str, int], default_zero: bool = False) -> bool:
    """Evaluate a SHAPE condition; only the documented token set is accepted."""
    pos, py = 0, []
    cond = cond.strip()
    while pos < len(cond):
        m = TOKEN_RE.match(cond, pos)
        if not m or m.end() == pos:
            raise FreezeError(f"cannot evaluate SHAPE condition {cond!r} at {cond[pos:pos + 12]!r}")
        pos = m.end()
        num, axis, op = m.groups()
        if num is not None:
            py.append(num)
        elif axis is not None:
            name = axis[len("SHAPE_"):]
            if name not in values:
                if not default_zero:
                    raise FreezeError(f"axis SHAPE_{name} has no value (pin it with --cell or #define)")
                values[name] = 0
            py.append(str(values[name]))
        else:
            py.append({"&&": " and ", "||": " or ", "!": " not "}.get(op, op))
    expr = " ".join(py)
    try:
        return bool(eval(expr, {"__builtins__": {}}, {}))  # tokens are whitelisted above
    except Exception as e:  # pragma: no cover - the tokeniser keeps this unreachable in practice
        raise FreezeError(f"cannot evaluate SHAPE condition {cond!r}: {e}")


def pinned_values(text: str) -> dict[str, int]:
    out = {}
    for line in text.splitlines():
        m = DEFINE_RE.match(line)
        if m:
            out[m.group(1)] = int(m.group(2))
    return out


def resolve_axes(text: str, cell: dict[str, int], default_zero: bool = False) -> str:
    """Keep the live arms of every SHAPE conditional; pass other conditionals through."""
    values = dict(pinned_values(text))
    values.update(cell)
    out = []
    # frame: [is_shape, parent_active, taken, active]
    stack: list[list] = []

    def active() -> bool:
        return all(f[3] for f in stack)

    for line in text.splitlines(keepends=True):
        if DEFINE_RE.match(line):
            continue
        m = COND_RE.match(line)
        if not m:
            if active():
                out.append(line)
            continue
        kind, rest = m.group(1), m.group(2)
        is_axis = bool(AXIS_RE.search(rest))
        if kind in ("if", "ifdef", "ifndef"):
            parent = active()
            if is_axis and kind == "if":
                taken = parent and evaluate(rest, values, default_zero)
                stack.append([True, parent, taken, taken])
            elif is_axis:
                raise FreezeError(f"#{kind} on a SHAPE axis is not supported: {line.strip()}")
            else:
                stack.append([False, parent, False, parent])
                if parent:
                    out.append(line)
            continue
        if not stack:
            raise FreezeError(f"#{kind} without #if: {line.strip()}")
        frame = stack[-1]
        if kind == "elif":
            if frame[0]:
                if not is_axis:
                    raise FreezeError(f"#elif without an axis inside a SHAPE conditional: {line.strip()}")
                live = frame[1] and not frame[2] and evaluate(rest, values, default_zero)
                frame[3] = live
                frame[2] = frame[2] or live
            else:
                if is_axis:
                    raise FreezeError(f"#elif on a SHAPE axis inside a plain conditional: {line.strip()}")
                if frame[1]:
                    out.append(line)
            continue
        if kind == "else":
            if frame[0]:
                frame[3] = frame[1] and not frame[2]
                frame[2] = True
            elif frame[1]:
                out.append(line)
            continue
        # endif
        stack.pop()
        if not frame[0] and frame[1]:
            out.append(line)
    if stack:
        raise FreezeError("unterminated conditional")
    return "".join(out)


def carrier_macros(text: str, keep: set[str]) -> dict[str, str]:
    out = {}
    for line in text.splitlines():
        m = CARRIER_RE.match(line)
        if m and m.group(1) != m.group(2) and m.group(1) not in keep:
            out[m.group(1)] = m.group(2)
    return out


def expand_carriers(text: str, keep: set[str] = frozenset()) -> str:
    macros = carrier_macros(text, set(keep))
    if not macros:
        return text
    lines = [l for l in text.splitlines(keepends=True)
             if not (CARRIER_RE.match(l) and CARRIER_RE.match(l).group(1) in macros)]
    text = "".join(lines)
    for name, body in macros.items():
        text = re.sub(rf"(?<![\w.>]){re.escape(name)}\b", body, text)
    return text


def freeze_text(text: str, cell: dict[str, int], keep: set[str] = frozenset(), expand: bool = True,
                default_zero: bool = False) -> str:
    frozen = resolve_axes(text, cell, default_zero)
    if expand:
        frozen = expand_carriers(frozen, keep)
    left = sorted(set(AXIS_RE.findall(frozen)))
    if left:
        raise FreezeError(f"axes survive the freeze (used outside a conditional?): {', '.join(left)}")
    return frozen


def parse_cell(spec: str | None) -> dict[str, int]:
    out = {}
    for item in (spec or "").split(","):
        item = item.strip()
        if not item:
            continue
        name, _, value = item.partition("=")
        name = name.strip()
        if name.startswith("SHAPE_"):
            name = name[len("SHAPE_"):]
        try:
            out[name] = int(value)
        except ValueError:
            raise SystemExit(f"--cell {item!r}: expected name=integer")
    return out


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("candidate")
    ap.add_argument("--cell", help="axis values 'a=1,b=0' (override the file's #define pins)")
    ap.add_argument("-o", "--output", help="write here (default: stdout)")
    ap.add_argument("--keep-macro", action="append", default=[], metavar="NAME",
                    help="do not expand this carrier macro")
    ap.add_argument("--no-expand", action="store_true", help="keep every #define NAME IDENT")
    ap.add_argument("--default-zero", action="store_true", help="an unpinned axis reads as 0")
    ns = ap.parse_args(argv)
    text = pathlib.Path(ns.candidate).read_text()
    try:
        frozen = freeze_text(text, parse_cell(ns.cell), set(ns.keep_macro), not ns.no_expand,
                             ns.default_zero)
    except FreezeError as e:
        raise SystemExit(f"shape_freeze: {e}")
    if ns.output:
        pathlib.Path(ns.output).write_text(frozen)
        print(f"froze {ns.candidate} -> {ns.output}", file=sys.stderr)
    else:
        sys.stdout.write(frozen)
    return 0


if __name__ == "__main__":
    sys.exit(main())
