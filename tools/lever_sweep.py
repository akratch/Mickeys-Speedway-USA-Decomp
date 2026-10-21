#!/usr/bin/env python3
"""Search the source-lever catalogue for the edit that reproduces a forced allocator state.

    tools/lever_sweep.py <symbol> --proc N [--oracle 'p1:w387=s[,p2:w131=c21...]']
                         [--bias 'w387=-4.5,...'] [--root WORKTREE] [--candidate FILE] [--web W ...] [--block B ...]
                         [--lines LO-HI] [--levers a,b] [--jobs J]
                         [--max-size-delta 16] [--max-cells 600] [--json PATH] [--no-fail]

EXIT STATUS. Default: 0 when some cell is exact (masked 0, size delta 0), 1
when none is -- scripts rely on that. A completed sweep that only ranks (a
`--max-size-delta`/`--bias` search, a no-oracle survey) has no exact cell by
design and reads as a failure; `--no-fail` exits 0 for it. Either way the run
prints a `no exact cell` line saying the sweep completed. Errors that stop the
sweep (identity gate, bad oracle) still exit nonzero under `--no-fail`.

NO ORACLE. With neither --oracle nor --bias the sweep still runs: cells are
ranked by exactness, aligned residual, size delta and masked words, the
`dbase` column gives each cell's size move against the base, and the oracle
column reads `-`. Use it to rank the catalogue's effect on a function whose
target state is not priced yet.

WHY. In nearly every late close of the 2026-10-07/08 waves the lane had
already PRICED the target with a force (`CDX_FORCE=p1:wN=cK` or `=s`, scored
directly) and then hand-wrote 50-500 cells of source transforms until one
reproduced the forced records. Those transforms are a small catalogue. This
tool generates every applicable transform at every applicable statement
position, compiles each cell with the stock and the instrumented compiler,
scores it, and checks whether the records now show the oracle's state
WITHOUT the force.

THE CATALOGUE (each lever names the brief item or wave law it comes from):

  dead_read      `x = READ;` -- an assigned dead read into an existing local
                 that is dead at that point (item 44, item 21: the read's
                 address expression enters uopt's table first; a bare dead
                 read is inert, so none is generated). READ is a field,
                 subscript or deref read found within --window statements.
  dead_masked    `x = READ & 0xFFFF;` -- a dead local assigned through a
                 masked RHS: the load goes to a scratch temp, the copy is
                 deleted, the web still denies a register (o008 F00034A0).
  keep_alive     `x |= 0;` / `x ^= 0;` right after `x = <expression>;`: kills
                 uopt's substitution of the symbol (o017).
  noop_redef     `x = (T) x;` with T the declared type: deleted, but kills
                 forward substitution into a later use (item 48).
  narrow_type    a narrower or wider integer type on one local (items 16, 32;
                 semantics: check).
  subscript      `[E]` -> `[(E) & 0xFFFF]` (item 43, semantics: check) and
                 `&X[E]` -> a byte-scaled `(u8 *) X + (E) * sizeof(X[0])`
                 (the scaled-subscript draw, item 43).
  boundary       `do { S } while (0);` around one statement, and an empty
                 `if (v) {}` at a position (item 18; wave law n-f2: the empty
                 if seeds a split piece in its block).
  global_reread  after `G = x;` (G not a local), the next read of x becomes a
                 read of G by name (wave law n-f2; semantics: check).
  zero_def       `k = 0;` for a dead integer local right after a loop: a def
                 between two loops keeps uopt's `<` exit test (law n-f1).
  split_local    one local becomes two names from a dominating redefinition
                 on (items 14, 39).
  merge_locals   two same-typed locals with disjoint ranges become one name,
                 declaration kept or dropped (item 39, item 40; check).
  merge_pairs    the merge_locals family's second member: every same-typed pair
                 merged over the whole body, no liveness test, declaration
                 dropped when possible (tools/merge_locals.py shares the
                 renaming and classifies frame-only merges; check).
  reorder        two adjacent independent statements swapped (statement order
                 reaches as1 and ugen emission order; semantics: check when
                 both touch memory).
  loop_move      the statement before a loop moved into its body head, or the
                 body's first statement hoisted out (semantics: check).
  const_iv       `K = C; for (v = K; ...)` for `for (v = C; ...)`: a constant
                 symbol folded into the IV init (w26 on func_80009414).

SOURCE-LEVER MODE (`--source-levers`, 2026-10-21). No --proc, no oracle, stock
compiles only; the default levers are the five that moved func_800517E0 that
day, each with a per-cell `sem` of safe | check:<reason> | diag:

  literal_type   integer literal spellings N <-> NU and the bit-identical
                 M <-> ~(~M) <-> -N forms, ONLY at value-safe sites: ==, !=,
                 &, |, ^, &=, |=, ^=, case labels and shift counts, and never
                 against a floating operand (relational, arithmetic, division,
                 call arguments and plain assignments are refused).
  carrier_rename a case-scoped rename of a local to another declared,
                 type-compatible local, generated only when def-before-use
                 holds on the RENAMED program (class Dataflow): the new name is
                 written before read inside the region, and on every exit of
                 the region -- the loop back edge and nested switches included
                 -- neither the new nor the old name is read before being
                 written. A width change (s32 -> u16) is `check`.
  reorder        two adjacent statements with disjoint read/write sets, no
                 call and no volatile; two memory accesses with a store are
                 `check:<reason>`.
  mask_shift     `x = (E) & M; f(x)` <-> `x = E; f(x & M)` and `x = (E) << k`
                 <-> `f(x << k)` when x has exactly one use; a 0xFF/0xFFFF mask
                 introduced or dropped when the callee's prototype narrows to
                 that width (K&R or implicit callee: `check`).
  read_order     permutations of a run of adjacent independent stream reads
                 (loads into distinct locals, no store or call between).

--case LABEL restricts to a switch region, --pairs N adds same-handler
combinations, --diag 'decl:NAME=TYPE' (or @FILE) pins a NON-adoptable base and
measures every cell on both bases; the ranking uses the diag column whenever
the adoptable base is off size. --greedy adopts the best SAFE improving cell,
re-enumerates on the new base and repeats --rounds times; --emit writes the
frozen adoptable TU (shape_freeze resolves any SHAPE_ axes) plus OUT.c.diff.

"Dead" is decided conservatively from the statement tree: the next mention of
the local after the position must be a plain `x = ...` (not reading x) at a
level that every later path passes, no break/continue/goto/label sits between,
and every loop enclosing the position must re-kill x before reading it.
Levers marked `check` can change behaviour: read any winner against the whole
enclosing scope before adopting it (brief trap 8).

HOW EACH CELL IS MEASURED. The compile command is the build's own
(`force_lattice.compile_command`, the same asm-processor recipe web_report
uses, so per-file flags are honoured and procedure ordinals agree with every
shard), with only the source and output paths swapped. Per cell:

1. stock compile; the symbol's size is read first and a cell whose size moves
   more than --max-size-delta bytes is dropped before any scoring;
2. a cell whose function bytes equal the base's is `inert` (the base's
   numbers are copied; no instrumented compile is spent on it);
3. otherwise masked + aligned residual (align_symbol.AlignedScorer, target
   read once) and an instrumented compile with CDX_PROC=N, CDX_LOG=1,
   CDX_DETAIL_WEB=all, CDX_WEBREPORT=1 and NO force. The instrumented .text
   must equal the stock .text (identity gate) or the cell's oracle column
   reads `gate`.

THE ORACLE CHECK. The base is compiled once with the oracle's forces; every
force must be accepted (force_lattice.force_acceptance) or the run stops. Each
forced web is then identified by its expression (`webexpr`) and the source
lines of the blocks it spans. Web numbers are NOT stable across source edits
(a dead statement renumbers webs, law m-3), so in a cell the web is found by
the same expression in either phase with the best line overlap (Jaccard >= 0.3).
A colour force is satisfied when that web is coloured cK; a split force when
no same-expression piece over those lines receives a colour (a piece that is
never formed counts). The column reads
`yes`, `k/n` for partial, or `no`; `=forced` marks a cell whose function bytes
equal the forced base's exactly.

BIAS ORACLE. `--bias 'web=delta,...'` compiles the base with CDX_BIAS (the
shared instrumented uopt adds delta to a p1 web's save for globalcolor's
selection only; see force_lattice.py). The pairs of p1 webs whose decision
order the bias REVERSED against the unbiased base are the state to price; a
cell satisfies one when its own unbiased records decide the two webs (found
again by expression + line overlap) in the biased order. It can be combined
with --oracle; use bias alone when the order, not a colour, is what the
target needs.

AMBIGUITY. The forced web is re-derived in every cell from its expression text
and source-line overlap, never by web number. If the best-overlapping rows tie
and disagree about the outcome the cell reads `ambiguous` (and the candidates
are printed) instead of yes or no.

DEAD-READ TYPES. `x = READ;` is generated only when READ's declared type is
inferred (locals, params, struct fields from the TU and include/*.h) and is
assignment-compatible with x; an unknown type never pairs.

ROOT. The worktree is the git toplevel of the current directory (or --root),
not the script's own path, so a copy of this tool runs against the lane
standing in it.

POSITIONS. Every statement position of the function body (after each
compound's declarations), optionally narrowed to the source lines of the
blocks a web spans (--web, from the forced base's `webblocks`), of given
blocks (--block, from `bbline`), or a line range (--lines). Inserted text
goes on the same physical line as the statement it follows, so every other
line keeps its number (as1's tie-break reads source lines, brief trap 7);
--own-line puts it on a line of its own instead.

Nothing is written into the tree: cells, objects and logs live under
--scratch (default /private/tmp/claude-501/lever-sweep-<pid>), and only
the best --keep cells' sources survive the run.
"""
from __future__ import annotations

import argparse
import collections
import dataclasses
import difflib
import hashlib
import itertools
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import threading
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import merge_locals as merge_locals_tool  # noqa: E402


def resolve_root(explicit: str | None = None, cwd: str | None = None) -> pathlib.Path:
    """The worktree to work in: --root, else the git toplevel of the current directory.

    Never the script's own location: a copy of this tool run from a scratch
    directory (or from another lane's worktree) must still compile, and refuse
    scratch dirs, relative to the tree the lane is standing in.
    """
    if explicit:
        root = pathlib.Path(explicit).expanduser().resolve()
        if not (root / "Makefile").is_file():
            raise SystemExit(f"--root {root}: not a repository root (no Makefile)")
        return root
    r = subprocess.run(["git", "rev-parse", "--show-toplevel"], cwd=cwd, capture_output=True,
                       text=True)
    if r.returncode == 0 and r.stdout.strip():
        return pathlib.Path(r.stdout.strip()).resolve()
    return pathlib.Path(__file__).resolve().parents[1]


def use_root(root: pathlib.Path) -> None:
    """Make `root` the tree every sibling tool module resolves against."""
    global ROOT
    ROOT = root
    tools = str(root / "tools")
    if (root / "tools").is_dir() and tools not in sys.path[:1]:
        sys.path.insert(0, tools)


ROOT = resolve_root()

KEYWORDS = {
    "if", "else", "for", "while", "do", "switch", "case", "default", "break",
    "continue", "return", "goto", "sizeof", "struct", "union", "enum",
    "typedef", "static", "extern", "const", "volatile", "register", "auto",
    "signed", "unsigned", "void", "char", "short", "int", "long", "float",
    "double", "NULL",
}
INT_TYPES = {"s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "int", "char",
             "short", "long", "unsigned", "signed", "unsigned int", "unsigned char",
             "unsigned short", "signed char"}
FLOAT_TYPES = {"f32", "f64", "float", "double"}
NARROW = {
    "s32": ("s16", "u16", "u8", "s8", "u32"), "u32": ("s32", "u16", "s16", "u8"),
    "int": ("s16", "u16", "u8"), "s16": ("s32", "u16", "u8"),
    "u16": ("s32", "s16", "u8"), "u8": ("s32", "u16", "s16"), "s8": ("s32", "s16"),
}
IDENT_RE = re.compile(r"[A-Za-z_]\w*")
DECL_RE = re.compile(
    r"^\s*(?:(?:const|volatile|static|register|unsigned|signed|struct|union|enum)\s+)*"
    r"[A-Za-z_]\w*[\s*]+[A-Za-z_]\w*\s*(?:\[[^\]]*\]\s*)*(?:=|;|,)", re.S)
ASSIGN_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=(?!=)(.*);\s*$", re.S)
STORE_RE = re.compile(r"^\s*(.+?)\s*=(?!=)\s*([A-Za-z_]\w*)\s*;\s*$", re.S)
READ_RE = re.compile(
    r"(?<![\w.>])(?:\*\s*[A-Za-z_]\w*|[A-Za-z_]\w*"
    r"(?:\s*(?:->|\.)\s*[A-Za-z_]\w*|\s*\[[^\[\]]*\])+)")
FORCE_RE = re.compile(r"^p([12]):w(\d+)=(c\d+|s)$")


# ---------------------------------------------------------------- lexing

def skip_ws(t: str, i: int) -> int:
    """Skip whitespace and comments."""
    n = len(t)
    while i < n:
        if t[i].isspace():
            i += 1
        elif t.startswith("/*", i):
            j = t.find("*/", i + 2)
            i = n if j < 0 else j + 2
        elif t.startswith("//", i):
            j = t.find("\n", i)
            i = n if j < 0 else j + 1
        else:
            break
    return i


def skip_literal(t: str, i: int) -> int:
    """i at a quote: return the index after the closing quote."""
    q = t[i]
    i += 1
    while i < len(t) and t[i] != q:
        i += 2 if t[i] == "\\" else 1
    return i + 1


def match_close(t: str, i: int) -> int:
    """i at an opening bracket: return the index after its partner."""
    depth = 0
    n = len(t)
    while i < n:
        c = t[i]
        if c in "\"'":
            i = skip_literal(t, i)
            continue
        if t.startswith("/*", i) or t.startswith("//", i):
            i = skip_ws(t, i)
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise ValueError("unbalanced brackets")


def scan_to(t: str, i: int, stop: str) -> int:
    """Index after the first `stop` character at bracket depth 0."""
    n = len(t)
    while i < n:
        c = t[i]
        if c in "\"'":
            i = skip_literal(t, i)
            continue
        if t.startswith("/*", i) or t.startswith("//", i):
            i = skip_ws(t, i)
            continue
        if c in "([{":
            i = match_close(t, i)
            continue
        if c == stop:
            return i + 1
        i += 1
    raise ValueError(f"no {stop!r} before the end")


def word_at(t: str, i: int) -> str:
    m = IDENT_RE.match(t, i)
    return m.group(0) if m else ""


def strip_comments(s: str) -> str:
    s = re.sub(r"/\*.*?\*/", " ", s, flags=re.S)
    return re.sub(r"//[^\n]*", " ", s)


def idents(s: str) -> list[str]:
    """Variable-like identifiers: not member names, not keywords (memoised by text)."""
    got = _IDENTS.get(s)
    if got is None:
        got = _IDENTS[s] = _idents(s)
        if len(_IDENTS) > 200000:
            _IDENTS.clear()
    return got


_IDENTS: dict = {}


def _idents(s: str) -> list[str]:
    s = strip_comments(s)
    s = re.sub(r'"(?:\\.|[^"\\])*"', '""', s)
    out = []
    for m in IDENT_RE.finditer(s):
        before = s[:m.start()].rstrip()
        if before.endswith(".") or before.endswith("->"):
            continue
        if m.group(0) in KEYWORDS or (m.start() and (s[m.start() - 1].isdigit())):
            continue
        out.append(m.group(0))
    return out


# ---------------------------------------------------------------- statement tree

@dataclasses.dataclass(eq=False)
class Node:
    kind: str                 # compound simple decl if loop do switch label pp jump
    start: int
    end: int
    children: list = dataclasses.field(default_factory=list)
    header: tuple | None = None  # (start, end) of the parenthesised header
    loop: str = ""            # for / while / do
    parent: "Node | None" = None
    ndecls: int = 0           # compound: leading declaration statements

    def text(self, t: str) -> str:
        return t[self.start:self.end]


def parse_stmt(t: str, i: int) -> Node:
    i = skip_ws(t, i)
    if t[i] == "#":
        j = i
        while True:
            k = t.find("\n", j)
            if k < 0:
                return Node("pp", i, len(t))
            if t[k - 1] != "\\":
                return Node("pp", i, k)
            j = k + 1
    if t[i] == "{":
        return parse_compound(t, i)
    if t[i] == ";":
        return Node("simple", i, i + 1)
    w = word_at(t, i)
    if w == "if":
        h0 = skip_ws(t, i + 2)
        h1 = match_close(t, h0)
        then = parse_stmt(t, h1)
        node = Node("if", i, then.end, [then], (h0, h1))
        j = skip_ws(t, then.end)
        if word_at(t, j) == "else":
            other = parse_stmt(t, j + 4)
            node.children.append(other)
            node.end = other.end
        return node
    if w in ("for", "while", "switch"):
        h0 = skip_ws(t, i + len(w))
        h1 = match_close(t, h0)
        body = parse_stmt(t, h1)
        return Node("switch" if w == "switch" else "loop", i, body.end, [body], (h0, h1),
                    loop=w)
    if w == "do":
        body = parse_stmt(t, i + 2)
        j = skip_ws(t, body.end)
        if word_at(t, j) != "while":
            raise ValueError("do without while")
        h0 = skip_ws(t, j + 5)
        h1 = match_close(t, h0)
        end = scan_to(t, h1, ";")
        return Node("loop", i, end, [body], (h0, h1), loop="do")
    if w in ("case", "default"):
        return Node("label", i, scan_to(t, i, ":"))
    if w and w not in KEYWORDS:
        j = skip_ws(t, i + len(w))
        if j < len(t) and t[j] == ":" and t[j + 1:j + 2] != ":":
            return Node("label", i, j + 1)
    end = scan_to(t, i, ";")
    kind = "jump" if w in ("break", "continue", "goto", "return") else "simple"
    return Node(kind, i, end)


def parse_compound(t: str, i: int) -> Node:
    assert t[i] == "{"
    close = match_close(t, i) - 1
    node = Node("compound", i, close + 1)
    j = i + 1
    leading = True
    while True:
        j = skip_ws(t, j)
        if j >= close:
            break
        child = parse_stmt(t, j)
        if child.kind == "simple" and leading and DECL_RE.match(t[child.start:child.end]) \
                and word_at(t, child.start) not in ("return",):
            child.kind = "decl"
            node.ndecls = len(node.children) + 1
        elif child.kind != "pp":
            leading = False
        node.children.append(child)
        j = child.end
    return node


def link(node: Node, parent: Node | None = None) -> None:
    node.parent = parent
    for c in node.children:
        link(c, node)


def walk(node: Node):
    yield node
    for c in node.children:
        yield from walk(c)


@dataclasses.dataclass
class Function:
    text: str                 # the whole TU
    symbol: str
    body: Node                # the function's outer compound
    sig: tuple                # (start, end) of the parameter list parens
    locals: dict              # name -> {"type": str, "ptr": int, "array": bool, "decl": Node}
    params: dict

    def line_of(self, offset: int) -> int:
        return self.text.count("\n", 0, offset) + 1


def find_function(text: str, symbol: str) -> Function:
    """Parse the C definition of `symbol` (the NON_MATCHING arm when guarded)."""
    for m in re.finditer(rf"^[\w \t\*]*\b{re.escape(symbol)}\s*\(", text, re.M):
        p0 = m.end() - 1
        try:
            p1 = match_close(text, p0)
        except ValueError:
            continue
        j = skip_ws(text, p1)
        if j < len(text) and text[j] == "{":
            body = parse_compound(text, j)
            link(body)
            fn = Function(text, symbol, body, (p0, p1), {}, {})
            fn.locals = collect_locals(text, body)
            fn.params = parse_params(text[p0 + 1:p1 - 1])
            return fn
    raise SystemExit(f"no C definition of {symbol} in the candidate")


def split_top(s: str, sep: str = ",") -> list[str]:
    out, depth, cur = [], 0, []
    for c in s:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == sep and depth == 0:
            out.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    out.append("".join(cur))
    return out


def parse_declarator(base: str, d: str) -> tuple[str, dict] | None:
    """One declarator of a declaration: name, pointer depth, array."""
    d = d.split("=", 1)[0].strip()
    stars = len(d) - len(d.lstrip("* "))
    stars = d[:stars].count("*")
    m = IDENT_RE.match(d.lstrip("* ").strip())
    if not m:
        return None
    return m.group(0), {"type": base.strip(), "ptr": stars, "array": "[" in d}


def collect_locals(text: str, body: Node) -> dict:
    out = {}
    for node in walk(body):
        if node.kind != "decl":
            continue
        decl = strip_comments(node.text(text)).strip().rstrip(";")
        parts = split_top(decl)
        m = re.match(r"^((?:(?:const|volatile|static|register|unsigned|signed|struct|union|enum)\s+)*"
                     r"[A-Za-z_]\w*)", parts[0].strip())
        if not m:
            continue
        base = m.group(1)
        first = parts[0].strip()[len(base):]
        for d in [first] + parts[1:]:
            got = parse_declarator(base, d)
            if got:
                name, info = got
                info["decl"] = node
                out[name] = info
    return out


def parse_params(s: str) -> dict:
    out = {}
    for p in split_top(s):
        p = p.strip()
        if not p or p == "void":
            continue
        m = re.search(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$", p)
        if m:
            out[m.group(1)] = {"type": p[:m.start()].replace("*", "").strip(),
                               "ptr": p.count("*"), "array": bool(m.group(2))}
    return out


def category(info: dict) -> str:
    if info.get("array"):
        return "array"
    if info.get("ptr"):
        return "pointer"
    t = info.get("type", "")
    if t in FLOAT_TYPES:
        return "float"
    if t in INT_TYPES:
        return "int"
    return "other"


# ---------------------------------------------------------------- expression types

_STRUCTS: dict[str, dict] = {}
_STRUCTS_ROOT: list = [None]
STRUCT_RE = re.compile(r"(?:typedef\s+)?(struct|union)\s*(\w*)\s*\{")


def parse_structs(text: str, into: dict | None = None) -> dict:
    """tag/typedef name -> {field: info} for flat struct and union bodies.

    Nested aggregates are skipped, so a field inside one is simply unknown (an
    unknown type never pairs with anything, see `compatible`).
    """
    out = {} if into is None else into
    text = strip_comments(text)
    for m in STRUCT_RE.finditer(text):
        try:
            close = match_close(text, m.end() - 1)
        except ValueError:
            continue
        body = text[m.end():close - 1]
        flat, depth = [], 0
        for ch in body:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
            elif depth == 0:
                flat.append(ch)
        fields = {}
        for decl in "".join(flat).split(";"):
            decl = decl.strip()
            mm = re.match(r"^((?:(?:const|volatile|unsigned|signed|struct|union|enum)\s+)*"
                          r"[A-Za-z_]\w*)", decl)
            if not mm:
                continue
            base = mm.group(1)
            for d in split_top(decl[len(base):]):
                got = parse_declarator(base, d.split(":")[0])
                if got:
                    fields[got[0]] = got[1]
        names = []
        if m.group(2):
            names.append(m.group(2))
        tail = re.match(r"\s*(\w+)\s*;", text[close:])
        if tail:
            names.append(tail.group(1))
        for n in names:
            out.setdefault(n, fields)
    return out


def structs_for(fn: "Function") -> dict:
    """Struct tables from the TU and the repository's headers (cached per root)."""
    if _STRUCTS_ROOT[0] != ROOT:
        _STRUCTS.clear()
        _STRUCTS_ROOT[0] = ROOT
        for h in sorted((ROOT / "include").rglob("*.h")) if (ROOT / "include").is_dir() else []:
            try:
                parse_structs(h.read_text(errors="replace"), _STRUCTS)
            except Exception:
                continue
    table = dict(_STRUCTS)
    parse_structs(fn.text, table)
    return table


def expr_type(fn: "Function", expr: str, structs: dict) -> dict | None:
    """Declared type {type, ptr, array} of a read expression, or None when unknown."""
    expr = expr.strip()
    deref = 0
    while expr.startswith("*"):
        deref += 1
        expr = expr[1:].strip()
    m = IDENT_RE.match(expr)
    if not m:
        return None
    info = fn.locals.get(m.group(0)) or fn.params.get(m.group(0))
    if info is None:
        return None
    cur = {"type": info["type"], "ptr": info["ptr"], "array": bool(info.get("array"))}
    rest = expr[m.end():]
    ops = re.findall(r"->\s*\w+|\.\s*\w+|\[[^\[\]]*\]", rest)
    if "".join(o.replace(" ", "") for o in ops) != re.sub(r"\s+", "", rest):
        return None
    for op in ops:
        if op.startswith("["):
            if cur["array"]:
                cur["array"] = False
            elif cur["ptr"] > 0:
                cur["ptr"] -= 1
            else:
                return None
        else:
            arrow = op.startswith("->")
            if cur["array"] or cur["ptr"] != (1 if arrow else 0):
                return None
            fields = structs.get(re.sub(r"^(?:struct|union|const|volatile)\s+", "", cur["type"]))
            f = fields.get(op.lstrip("->.").strip()) if fields else None
            if not f:
                return None
            cur = {"type": f["type"], "ptr": f["ptr"], "array": bool(f.get("array"))}
    for _ in range(deref):
        if cur["ptr"] > 0:
            cur["ptr"] -= 1
        elif cur["array"]:
            cur["array"] = False
        else:
            return None
    return cur


def bare_type(t: str) -> str:
    return re.sub(r"^(?:(?:struct|union|enum|const|volatile)\s+)+", "", t.strip())


def compatible(local: dict, read: dict | None) -> bool:
    """May `local = <read>;` compile as a plain assignment? Unknown types never pair."""
    if read is None or read["array"] or local.get("array"):
        return False
    local = dict(local, type=bare_type(local["type"]))
    read = dict(read, type=bare_type(read["type"]))
    lc, rc = category(local), category(read)
    if lc != rc:
        return False
    if lc == "int":
        return True
    if lc == "float":
        return local["type"] == read["type"]
    if lc == "pointer":
        return local["ptr"] == read["ptr"] and (local["type"] == read["type"]
                                               or "void" in (local["type"], read["type"]))
    return local["type"] == read["type"] and local["ptr"] == read["ptr"]


# ---------------------------------------------------------------- flow helpers

def simple_statements(fn: Function) -> list[Node]:
    return [n for n in walk(fn.body) if n.kind in ("simple", "jump")]


def ancestors(node: Node) -> list[Node]:
    out = []
    while node.parent is not None:
        node = node.parent
        out.append(node)
    return out


def mentions(fn: Function, node: Node, name: str) -> bool:
    return name in idents(node.text(fn.text))


def is_kill(fn: Function, node: Node, name: str) -> bool:
    """`name = <expr not reading name>;`, or a `for` whose init is exactly that."""
    if node.kind == "loop" and node.loop == "for":
        h0, h1 = node.header
        init = fn.text[h0 + 1:h1 - 1].split(";", 1)[0]
        m = re.match(r"^\s*([A-Za-z_]\w*)\s*=(?!=)(.*)$", strip_comments(init), re.S)
        return bool(m) and m.group(1) == name and name not in idents(m.group(2)) \
            and "," not in m.group(2)
    if node.kind != "simple":
        return False
    m = ASSIGN_RE.match(strip_comments(node.text(fn.text)))
    return bool(m) and m.group(1) == name and name not in idents(m.group(2))


def escapes(fn: Function, stmt: Node, in_loop: bool = False, in_switch: bool = False) -> bool:
    """Can control leave `stmt` other than by falling through or returning?"""
    t = fn.text
    if stmt.kind == "label":
        return True
    if stmt.kind == "jump":
        w = word_at(t, stmt.start)
        return (w == "goto" or (w == "continue" and not in_loop)
                or (w == "break" and not (in_loop or in_switch)))
    loop = in_loop or stmt.kind == "loop"
    switch = in_switch or stmt.kind == "switch"
    return any(escapes(fn, c, loop, switch) for c in stmt.children)


def dead_at(fn: Function, compound: Node, index: int, name: str) -> bool:
    """Is a value written to `name` at position `index` of `compound` never read?

    Conservative: walks forward through the enclosing compounds; the first
    mention must be a plain `name = <expr not reading name>;` statement of a
    compound every later path passes. A label, goto, or a break/continue that
    leaves the statement on the way answers False, and so does any enclosing
    loop whose header names the local or whose body reads it before the
    position without killing it first (the back edge).
    """
    t = fn.text
    off = insertion_offset(fn, compound, index)
    cur, pos = compound, index
    while True:
        for stmt in cur.children[pos:]:
            if escapes(fn, stmt):
                return False
            if not mentions(fn, stmt, name):
                continue
            return is_kill(fn, stmt, name)
        child, parent = cur, cur.parent
        while parent is not None and parent.kind != "compound":
            if parent.kind == "loop":
                h = parent.header
                if name in idents(t[h[0]:h[1]]):
                    return False
                body = parent.children[0]
                for stmt in (body.children if body.kind == "compound" else [body]):
                    if stmt.start >= off:
                        break
                    if stmt.end > off:
                        if name in idents(t[stmt.start:off]):
                            return False
                        break
                    if mentions(fn, stmt, name):
                        if not is_kill(fn, stmt, name):
                            return False
                        break
            child, parent = parent, parent.parent
        if parent is None:
            return True
        cur, pos = parent, parent.children.index(child) + 1


def positions(fn: Function) -> list[tuple[Node, int]]:
    """Every (compound, index) where a statement can be inserted."""
    out = []
    for node in walk(fn.body):
        if node.kind != "compound":
            continue
        for i in range(node.ndecls, len(node.children) + 1):
            out.append((node, i))
    return out


def insertion_offset(fn: Function, compound: Node, index: int) -> int:
    if index == 0:
        return compound.start + 1
    return compound.children[index - 1].end


def position_line(fn: Function, compound: Node, index: int) -> int:
    return fn.line_of(insertion_offset(fn, compound, index))


def insert_at(fn: Function, compound: Node, index: int, stmt: str, own_line: bool) -> str:
    t = fn.text
    off = insertion_offset(fn, compound, index)
    if own_line:
        line_start = t.rfind("\n", 0, off) + 1
        indent = re.match(r"[ \t]*", t[line_start:]).group(0)
        if index < len(compound.children):
            indent = re.match(r"[ \t]*", t[t.rfind("\n", 0, compound.children[index].start) + 1:]).group(0)
        return t[:off] + "\n" + indent + stmt + t[off:]
    return t[:off] + " " + stmt + t[off:]


def window_reads(fn: Function, stmts: list[Node], center: int, width: int) -> list[str]:
    """Distinct field/subscript/deref reads in the statements around a position."""
    seen: list[str] = []
    for s in stmts[max(0, center - width):center + width]:
        body = strip_comments(s.text(fn.text))
        for m in READ_RE.finditer(body):
            if m.group(0).startswith("*"):
                prev = body[:m.start()].rstrip()[-1:]
                if prev and (prev.isalnum() or prev in ")]_"):
                    continue  # a multiplication, not a dereference
            r = re.sub(r"\s+", " ", m.group(0)).strip()
            if r not in seen:
                seen.append(r)
    return seen


# ---------------------------------------------------------------- levers

@dataclasses.dataclass
class Cell:
    lever: str
    line: int
    edit: str
    text: str
    semantics: str = "safe"      # safe | check:<reason> | diag
    edits: tuple = ()            # (start, end, replacement) over the base text, when known
    region: str = ""             # the innermost switch region the edit lives in, when known
    group: str = ""              # the outermost one (pairs are formed within it)


def apply_edits(text: str, edits) -> str:
    """Apply (start, end, replacement) edits given against `text`, last first."""
    for s, e, r in sorted(edits, key=lambda x: (x[0], x[1]), reverse=True):
        text = text[:s] + r + text[e:]
    return text


def edits_overlap(a, b) -> bool:
    for s1, e1, _ in a:
        for s2, e2, _ in b:
            if s1 < e2 and s2 < e1 or s1 == s2:
                return True
    return False


def edit_cell(fn: "Function", lever: str, edit: str, edits, sem: str = "safe",
              region: str = "", line: int | None = None) -> Cell:
    edits = tuple(edits)
    first = min(s for s, _, _ in edits)
    inner, outer = regions_at(fn, first)
    return Cell(lever, line if line is not None else fn.line_of(first), edit,
                apply_edits(fn.text, edits), sem, edits, region or inner, outer)


def regions_at(fn: "Function", offset: int) -> tuple[str, str]:
    """(innermost, outermost) switch region names containing a text offset."""
    inner = outer = None
    for r in regions_cached(fn):
        if r.start <= offset < r.end:
            if inner is None or r.start >= inner.start:
                inner = r
            if outer is None or r.start < outer.start:
                outer = r
    return (inner.name if inner else ""), (outer.name if outer else "")


def stmt_index(stmts: list[Node], offset: int) -> int:
    """Index of the first simple statement at or after `offset`."""
    for i, s in enumerate(stmts):
        if s.start >= offset:
            return i
    return len(stmts)


def lever_dead_read(fn, ctx, masked=False):
    out = []
    for comp, idx in ctx["positions"]:
        line = position_line(fn, comp, idx)
        center = stmt_index(ctx["stmts"], insertion_offset(fn, comp, idx))
        reads = window_reads(fn, ctx["stmts"], center, ctx["window"])[: ctx["max_reads"]]
        for name, info in fn.locals.items():
            cat = category(info)
            if cat not in ("int", "pointer", "float") or (masked and cat != "int"):
                continue
            if name not in ctx["mentioned"]:
                continue
            if not dead_at(fn, comp, idx, name):
                continue
            for r in reads:
                if re.match(rf"^\*?\s*{re.escape(name)}\b", r):
                    continue
                if not compatible(info, expr_type(fn, r, ctx["structs"])):
                    continue
                rhs = f"{r} & 0xFFFF" if masked else r
                stmt = f"{name} = {rhs};"
                out.append(Cell("dead_masked" if masked else "dead_read", line, stmt,
                                insert_at(fn, comp, idx, stmt, ctx["own_line"])))
    return out


def lever_dead_masked(fn, ctx):
    return lever_dead_read(fn, ctx, masked=True)


def lever_keep_alive(fn, ctx):
    out = []
    for s in ctx["stmts"]:
        if s.kind != "simple" or not ctx["in_lines"](fn.line_of(s.start)):
            continue
        m = ASSIGN_RE.match(strip_comments(s.text(fn.text)))
        if not m or m.group(1) not in fn.locals or category(fn.locals[m.group(1)]) != "int":
            continue
        rhs = m.group(2).strip()
        if re.fullmatch(r"[A-Za-z_]\w*|-?(0x[0-9A-Fa-f]+|\d+)[uUlL]*", rhs):
            continue
        comp = s.parent
        if comp is None or comp.kind != "compound":
            continue
        idx = comp.children.index(s) + 1
        for op in ("|=", "^="):
            stmt = f"{m.group(1)} {op} 0;"
            out.append(Cell("keep_alive", fn.line_of(s.start), stmt,
                            insert_at(fn, comp, idx, stmt, ctx["own_line"])))
    return out


def lever_noop_redef(fn, ctx):
    out = []
    t = fn.text
    for name, info in fn.locals.items():
        cat = category(info)
        if cat not in ("int", "pointer"):
            continue
        occ = [s.start for s in ctx["stmts"] if name in idents(s.text(t))]
        if not occ:
            continue
        first, last = min(occ), max(occ)
        ty = info["type"] + (" " + "*" * info["ptr"] if info["ptr"] else "")
        for comp, idx in ctx["positions"]:
            off = insertion_offset(fn, comp, idx)
            if not first < off <= last:
                continue
            stmt = f"{name} = ({ty}) {name};"
            out.append(Cell("noop_redef", position_line(fn, comp, idx), stmt,
                            insert_at(fn, comp, idx, stmt, ctx["own_line"])))
    return out


def lever_narrow_type(fn, ctx):
    out = []
    t = fn.text
    for name, info in fn.locals.items():
        if category(info) != "int" or name not in ctx["mentioned"]:
            continue
        decl = info["decl"]
        dtext = decl.text(t)
        if "," in strip_comments(dtext):
            continue  # multi-declarator lines are left alone
        for new in NARROW.get(info["type"], ()):
            ntext = re.sub(rf"^(\s*){re.escape(info['type'])}\b", rf"\g<1>{new}", dtext, count=1)
            if ntext == dtext:
                continue
            out.append(Cell("narrow_type", fn.line_of(decl.start), f"{new} {name}",
                            t[:decl.start] + ntext + t[decl.end:], "check"))
    return out


SUBSCRIPT_RE = re.compile(r"\[([^\[\]]+)\]")
ADDR_SUB_RE = re.compile(r"&\s*([A-Za-z_]\w*(?:\s*(?:->|\.)\s*[A-Za-z_]\w*)*)\s*\[([^\[\]]+)\]")


def lever_subscript(fn, ctx):
    out = []
    t = fn.text
    for s in ctx["stmts"]:
        line = fn.line_of(s.start)
        if not ctx["in_lines"](line):
            continue
        body = s.text(t)
        for m in SUBSCRIPT_RE.finditer(body):
            e = m.group(1).strip()
            if re.fullmatch(r"-?(0x[0-9A-Fa-f]+|\d+)[uU]?", e) or "0xFFFF" in e:
                continue
            nb = body[:m.start()] + f"[({e}) & 0xFFFF]" + body[m.end():]
            out.append(Cell("subscript", line, f"[{e}] -> [({e}) & 0xFFFF]",
                            t[:s.start] + nb + t[s.end:], "check"))
        for m in ADDR_SUB_RE.finditer(body):
            x, e = m.group(1).strip(), m.group(2).strip()
            rep = f"(void *) ((u8 *) ({x}) + ({e}) * sizeof(({x})[0]))"
            nb = body[:m.start()] + rep + body[m.end():]
            out.append(Cell("subscript", line, f"&{x}[{e}] -> byte-scaled",
                            t[:s.start] + nb + t[s.end:]))
    return out


def lever_boundary(fn, ctx):
    out = []
    t = fn.text
    for s in ctx["stmts"]:
        if s.kind != "simple" or not ctx["in_lines"](fn.line_of(s.start)):
            continue
        if s.parent is None or s.parent.kind != "compound":
            continue
        body = s.text(t)
        out.append(Cell("boundary", fn.line_of(s.start), "do { S } while (0)",
                        t[:s.start] + "do { " + body + " } while (0);" + t[s.end:]))
    for comp, idx in ctx["positions"]:
        center = stmt_index(ctx["stmts"], insertion_offset(fn, comp, idx))
        near = set()
        for s in ctx["stmts"][max(0, center - ctx["window"]):center + ctx["window"]]:
            near.update(idents(s.text(t)))
        for name in sorted(near):
            info = fn.locals.get(name) or fn.params.get(name)
            if not info or category(info) not in ("int", "pointer"):
                continue
            stmt = f"if ({name}) {{}}"
            out.append(Cell("boundary", position_line(fn, comp, idx), stmt,
                            insert_at(fn, comp, idx, stmt, ctx["own_line"])))
    return out


def lever_global_reread(fn, ctx):
    out = []
    t = fn.text
    for s in ctx["stmts"]:
        if s.kind != "simple" or not ctx["in_lines"](fn.line_of(s.start)):
            continue
        m = STORE_RE.match(strip_comments(s.text(t)))
        if not m:
            continue
        lhs, x = m.group(1).strip(), m.group(2)
        root = IDENT_RE.match(lhs.lstrip("*( "))
        if x not in fn.locals or (root and lhs.strip() in fn.locals):
            continue
        if root and root.group(0) == x:
            continue
        comp = s.parent
        if comp is None or comp.kind != "compound":
            continue
        for later in comp.children[comp.children.index(s) + 1:]:
            if not mentions(fn, later, x):
                continue
            is_return = later.kind == "jump" and word_at(t, later.start) == "return"
            if (later.kind != "simple" and not is_return) or \
                    (not is_return and writes_reads(fn, later)[0] == x):
                break  # x is redefined (or the read is inside control flow): nothing to re-read
            body = later.text(t)
            lm = ASSIGN_RE.match(strip_comments(body))
            region = (body.index("=") + 1) if lm else 0
            hit = re.compile(rf"(?<![\w.>]){re.escape(x)}\b").search(body, region)
            if hit is None:
                break
            nb = body[:hit.start()] + lhs + body[hit.end():]
            out.append(Cell("global_reread", fn.line_of(later.start), f"{x} -> {lhs}",
                            t[:later.start] + nb + t[later.end:], "check"))
            break
    return out


def lever_zero_def(fn, ctx):
    out = []
    for node in walk(fn.body):
        if node.kind != "loop" or node.parent is None or node.parent.kind != "compound":
            continue
        comp = node.parent
        idx = comp.children.index(node) + 1
        line = position_line(fn, comp, idx)
        if not ctx["in_lines"](line) and not ctx["in_lines"](fn.line_of(node.start)):
            continue
        for name, info in fn.locals.items():
            if category(info) != "int" or not dead_at(fn, comp, idx, name):
                continue
            stmt = f"{name} = 0;"
            out.append(Cell("zero_def", line, stmt, insert_at(fn, comp, idx, stmt, ctx["own_line"])))
    return out


def rename_from(t: str, start: int, end: int, old: str, new: str) -> str:
    return merge_locals_tool.rename_word(t, start, end, old, new)


def add_decl(fn: Function, text: str, name: str, new: str) -> str:
    info = fn.locals[name]
    decl = info["decl"]
    ty = info["type"] + (" " + "*" * info["ptr"] if info["ptr"] else "")
    return text[:decl.end] + f" {ty} {new};" + text[decl.end:]


def lever_split_local(fn, ctx):
    out = []
    t = fn.text
    top = fn.body
    for name, info in fn.locals.items():
        if category(info) not in ("int", "pointer", "float") or info["decl"].parent is not top:
            continue
        seen_before = False
        for stmt in top.children[top.ndecls:]:
            if is_kill(fn, stmt, name) and seen_before:
                new = f"{name}_b"
                if new in fn.locals:
                    continue
                text = rename_from(t, stmt.start, top.end, name, new)
                text = add_decl(fn, text, name, new)
                out.append(Cell("split_local", fn.line_of(stmt.start), f"{name} -> {new} from here",
                                text))
            if mentions(fn, stmt, name):
                seen_before = True
    return out


def lever_merge_locals(fn, ctx):
    out = []
    t = fn.text
    stmts = ctx["stmts"]
    occ = {}
    for name in fn.locals:
        hits = [s for s in stmts if name in idents(s.text(t))]
        if hits:
            occ[name] = hits
    for x, xs in occ.items():
        for y, ys in occ.items():
            ix, iy = fn.locals[x], fn.locals[y]
            if x == y or ix["type"] != iy["type"] or ix["ptr"] != iy["ptr"] \
                    or category(ix) not in ("int", "pointer", "float"):
                continue
            if max(s.start for s in xs) >= min(s.start for s in ys):
                continue
            first_y = min(ys, key=lambda s: s.start)
            if not is_kill(fn, first_y, y):
                continue
            loops_x = {id(a) for a in ancestors(max(xs, key=lambda s: s.start)) if a.kind == "loop"}
            loops_y = {id(a) for a in ancestors(first_y) if a.kind == "loop"}
            if loops_x & loops_y:
                continue
            line = fn.line_of(first_y.start)
            decl = iy["decl"]
            scope = (first_y.start, fn.body.end)
            kept = merge_locals_tool.merge_text(t, scope, (decl.start, decl.end), x, y, drop_decl=False)
            out.append(Cell("merge_locals", line, f"{y} -> {x} (decl kept)", kept, "check"))
            dropped = merge_locals_tool.merge_text(t, scope, (decl.start, decl.end), x, y)
            if dropped is not None:
                out.append(Cell("merge_locals", line, f"{y} -> {x} (decl dropped)", dropped, "check"))
    return out


def lever_merge_pairs(fn, ctx):
    """Every ordered pair of same-typed locals merged over the whole body.

    The merge_locals family's second member (tools/merge_locals.py is the
    stand-alone classifier and shares this renaming): no liveness test, so it
    reaches the frame-cell reuse the author's shared names would have made.
    Declaration dropped when possible; semantics: check.
    """
    out = []
    t = fn.text
    mentioned = ctx["mentioned"]
    scope = (fn.body.start + 1, fn.body.end)
    for keep, drop in merge_locals_tool.candidate_pairs(fn, sys.modules[__name__]):
        if keep not in mentioned or drop not in mentioned:
            continue
        decl = fn.locals[drop]["decl"]
        text = merge_locals_tool.merge_text(t, scope, (decl.start, decl.end), keep, drop)
        if text is None:
            text = merge_locals_tool.merge_text(t, scope, (decl.start, decl.end), keep, drop, drop_decl=False)
        if text is not None:
            out.append(Cell("merge_pairs", fn.line_of(decl.start), f"{drop} -> {keep} (whole body)", text, "check"))
    return out


def writes_reads(fn: Function, s: Node) -> tuple[str | None, set, bool, bool]:
    """(written root identifier, identifiers read, has a call, stores to memory)."""
    body = strip_comments(s.text(fn.text))
    m = re.match(r"^\s*(.+?)\s*([-+*/&|^%]|<<|>>)?=(?!=)(.*);\s*$", body, re.S)
    call = bool(re.search(r"[A-Za-z_]\w*\s*\(", body.replace("sizeof", "")))
    if not m:
        return None, set(idents(body)), call, False
    lhs = m.group(1)
    mem = bool(re.search(r"->|\[|\*|\.", lhs))
    names = idents(lhs)
    root = names[0] if names else None
    reads = set(idents(m.group(3))) | (set(names[1:]) if mem else set())
    if m.group(2) and root:
        reads.add(root)
    return root, reads, call, mem


def lever_reorder(fn, ctx):
    out = []
    t = fn.text
    for comp in walk(fn.body):
        if comp.kind != "compound":
            continue
        kids = comp.children
        for i in range(comp.ndecls, len(kids) - 1):
            a, b = kids[i], kids[i + 1]
            if a.kind != "simple" or b.kind != "simple" or not ctx["in_lines"](fn.line_of(a.start)):
                continue
            sem = swap_semantics(fn, a, b)
            if sem is None:
                continue
            out.append(edit_cell(fn, "reorder", "swap with next",
                                 [(a.start, a.end, b.text(t)), (b.start, b.end, a.text(t))], sem,
                                 region_name_at(fn, a), fn.line_of(a.start)))
    return out


def swap_semantics(fn: Function, a: Node, b: Node) -> str | None:
    """May two adjacent simple statements swap? None when a dependence forbids it.

    Disjoint read/write sets of locals swap `safe`; a call or a volatile access
    on either side refuses; two memory accesses of which one is a store are
    `check:<reason>` (the roots may alias).
    """
    t = fn.text
    wa, ra, ca, ma = writes_reads(fn, a)
    wb, rb, cb, mb = writes_reads(fn, b)
    if ca or cb or wa is None or wb is None:
        return None
    if "volatile" in a.text(t) or "volatile" in b.text(t):
        return None
    if not ma and (wa in rb or (not mb and wa == wb)):
        return None
    if not mb and (wb in ra or (not ma and wa == wb)):
        return None
    loads_a = bool(re.search(r"->|\[|\*", strip_comments(a.text(t)).split("=", 1)[-1]))
    loads_b = bool(re.search(r"->|\[|\*", strip_comments(b.text(t)).split("=", 1)[-1]))
    if ma and mb:
        return "check:two stores may alias"
    if (ma and loads_b) or (mb and loads_a):
        return "check:store may alias the other's load"
    return "safe"


def lever_loop_move(fn, ctx):
    out = []
    t = fn.text
    for loop in walk(fn.body):
        if loop.kind != "loop" or loop.parent is None or loop.parent.kind != "compound":
            continue
        body = loop.children[0]
        if body.kind != "compound" or not ctx["in_lines"](fn.line_of(loop.start)):
            continue
        comp = loop.parent
        i = comp.children.index(loop)
        line = fn.line_of(loop.start)
        prev = comp.children[i - 1] if i > comp.ndecls else None
        wprev = writes_reads(fn, prev)[0] if prev is not None and prev.kind == "simple" else None
        if wprev and wprev not in idents(t[loop.header[0]:loop.header[1]]):
            ins = body.children[body.ndecls - 1].end if body.ndecls else body.start + 1
            text = t[:prev.start] + t[prev.end:ins] + " " + prev.text(t) + t[ins:]
            out.append(Cell("loop_move", line, "previous statement into the loop head", text, "check"))
        if len(body.children) > body.ndecls and body.children[body.ndecls].kind == "simple":
            first = body.children[body.ndecls]
            m = ASSIGN_RE.match(strip_comments(first.text(t)))
            if m:
                written = set()
                for s in walk(body):
                    if s.kind == "simple" and s is not first:
                        w, _, _, _ = writes_reads(fn, s)
                        if w:
                            written.add(w)
                h = loop.header
                written.update(idents(t[h[0]:h[1]]))
                if not (set(idents(m.group(2))) & written) and m.group(1) not in written:
                    text = t[:loop.start] + first.text(t) + " " + t[loop.start:first.start] \
                        + t[first.end:]
                    out.append(Cell("loop_move", line, "first body statement hoisted", text, "check"))
    return out


def lever_const_iv(fn, ctx):
    out = []
    t = fn.text
    for loop in walk(fn.body):
        if loop.kind != "loop" or loop.loop != "for" or loop.parent is None \
                or loop.parent.kind != "compound":
            continue
        h0, h1 = loop.header
        head = t[h0 + 1:h1 - 1]
        m = re.match(r"\s*([A-Za-z_]\w*)\s*=\s*(-?(?:0x[0-9A-Fa-f]+|\d+))\s*;", head)
        if not m or not ctx["in_lines"](fn.line_of(loop.start)):
            continue
        comp = loop.parent
        idx = comp.children.index(loop)
        for name, info in fn.locals.items():
            if name == m.group(1) or category(info) != "int":
                continue
            if mentions(fn, loop, name) or not dead_at(fn, comp, idx + 1, name):
                continue
            newhead = head[:m.start(2)] + name + head[m.end(2):]
            text = t[:loop.start] + f"{name} = {m.group(2)}; " + t[loop.start:h0 + 1] + newhead + t[h1 - 1:]
            out.append(Cell("const_iv", fn.line_of(loop.start), f"{name} = {m.group(2)}; init through {name}",
                            text))
    return out


# ---------------------------------------------------------------- switch regions

@dataclasses.dataclass
class Region:
    """The statements of one case group of a switch, labels excluded."""
    switch: Node
    labels: list
    stmts: list
    name: str

    @property
    def start(self) -> int:
        return self.labels[0].start

    @property
    def end(self) -> int:
        return (self.stmts[-1] if self.stmts else self.labels[-1]).end


def label_name(fn: Function, label: Node) -> str:
    return re.sub(r"\s+", " ", strip_comments(label.text(fn.text)).strip().rstrip(":")).strip()


def switch_regions(fn: Function, switch: Node) -> list[Region]:
    body = switch.children[0]
    if body.kind != "compound":
        return []
    out: list[Region] = []
    labels: list[Node] = []
    for child in body.children:
        if child.kind == "label":
            if out and out[-1].stmts == [] and not labels:
                labels = out.pop().labels
            labels.append(child)
            continue
        if labels:
            out.append(Region(switch, labels, [], "/".join(label_name(fn, l) for l in labels)))
            labels = []
        if out:
            out[-1].stmts.append(child)
    if labels:
        out.append(Region(switch, labels, [], "/".join(label_name(fn, l) for l in labels)))
    return out


def all_regions(fn: Function) -> list[Region]:
    out = []
    for node in walk(fn.body):
        if node.kind == "switch":
            out.extend(switch_regions(fn, node))
    return out


def region_of(fn: Function, node: Node) -> Region | None:
    """The innermost switch region a statement lies in."""
    best = None
    for r in regions_cached(fn):
        if r.start <= node.start and node.end <= r.end:
            if best is None or r.start >= best.start:
                best = r
    return best


def regions_cached(fn: Function) -> list[Region]:
    cache = fn.__dict__.setdefault("_regions", None)
    if cache is None:
        cache = fn.__dict__["_regions"] = all_regions(fn)
    return cache


def region_name_at(fn: Function, node: Node) -> str:
    r = region_of(fn, node)
    return r.name if r else ""


def ends_with_jump(fn: Function, node: Node) -> bool:
    if node.kind == "jump":
        return True
    if node.kind == "compound":
        return bool(node.children) and ends_with_jump(fn, node.children[-1])
    if node.kind == "if" and len(node.children) == 2:
        return all(ends_with_jump(fn, c) for c in node.children)
    return False


def region_exit_statuses(fn: Function, region: Region) -> set[str]:
    """How control can leave the region: break, continue, flow (fall-through); return is silent."""
    out = set()
    if not region.stmts:
        return {"flow"}
    if not ends_with_jump(fn, region.stmts[-1]):
        out.add("flow")
    for top in region.stmts:
        for node in walk(top):
            if node.kind != "jump":
                continue
            w = word_at(fn.text, node.start)
            if w == "goto":
                out.add("goto")
            elif w in ("break", "continue"):
                inner = [a.kind for a in ancestors(node) if a.start >= top.start]
                if w == "break" and ("loop" in inner or "switch" in inner):
                    continue  # leaves an inner construct, stays in the region
                if w == "continue" and "loop" in inner:
                    continue
                out.add(w)
    return out


# ---------------------------------------------------------------- def-before-use

READ = "READ"


class Dataflow:
    """Is a local read before it is written, along the statement tree?

    `seq(stmts, name)` answers for a statement list: READ when some path through
    it reads `name` unwritten, else the set of exit statuses ({"flow", "break",
    "continue"}) of the paths that leave it still unwritten (a path that writes
    first, or returns, is resolved and drops out). `after(node, name, status)`
    follows control from `node` leaving with `status` -- the rest of the
    compound, the enclosing if/switch/loop (a loop goes round: its header and
    body again, then its exit) -- and answers whether `name` can be read
    unwritten anywhere afterwards. Loops are followed once (the back edge),
    `goto` and labels answer READ.

    `rename=(start, end, old, new)` makes the analysis read the text with `old`
    spelled `new` inside that span, so a case-scoped rename is checked on the
    renamed program without re-parsing it.
    """

    def __init__(self, fn: Function, rename: tuple | None = None):
        self.fn = fn
        self.rename = rename
        self.cache: dict = {}

    def names_in(self, node: Node, name: str) -> str | None:
        """The name to look for inside `node` (`None`: nothing to look for)."""
        if not self.rename:
            return name
        s, e, old, new = self.rename
        if node.start >= s and node.end <= e:
            if name == new:
                return old
            if name == old:
                return None
        return name

    def access(self, stmt: Node, name: str) -> str:
        look = self.names_in(stmt, name)
        # outside the renamed span the answer is the same for every rename:
        # share it across instances through the function
        shared = look == name and not (self.rename and stmt.start >= self.rename[0]
                                       and stmt.end <= self.rename[1])
        cache = self.fn.__dict__.setdefault("_access_cache", {}) if shared else self.cache
        key = ("a", id(stmt), name)
        if key in cache:
            return cache[key]
        fn = self.fn
        if look is None:
            r = "none"
        else:
            text = strip_comments(stmt.text(fn.text))
            if look not in idents(text):
                r = "none"
            elif stmt.kind == "decl":
                m = re.search(rf"(?<![\w.>]){re.escape(look)}\s*(\[[^\]]*\]\s*)?=(?!=)([^,;]*)", text)
                r = "write" if m and look not in idents(m.group(2)) else \
                    ("none" if not re.search(r"=(?!=)", text) else "read")
            elif is_kill(fn, stmt, look):
                r = "write"
            else:
                r = "read"
        cache[key] = r
        return r

    def header(self, node: Node, name: str, skip_init: bool = False) -> tuple[str, bool]:
        """Access of a parenthesised header: ('read'|'write'|'none', write is conditional)."""
        look = self.names_in(node, name)
        if look is None:
            return "none", False
        t = self.fn.text
        h0, h1 = node.header
        text = strip_comments(t[h0 + 1:h1 - 1])
        if node.kind == "loop" and node.loop == "for":
            parts = text.split(";")
            if skip_init:
                parts = parts[1:]
            elif parts and look in idents(parts[0]):
                m = re.match(r"^\s*([A-Za-z_]\w*)\s*=(?!=)(.*)$", parts[0], re.S)
                if m and m.group(1) == look and look not in idents(m.group(2)):
                    return "write", False  # `for (x = C; ...)`: later reads follow the write
                return "read", False
            return ("read" if any(look in idents(p) for p in parts) else "none"), False
        if look not in idents(text):
            return "none", False
        m = re.search(rf"(?<![\w.>]){re.escape(look)}\s*=(?!=)", text)
        if m and look not in idents(text[:m.start()]) and look not in idents(text[m.end():]) \
                and "||" not in text and "?" not in text:
            return "write", True
        return "read", False

    def node(self, node: Node, name: str):
        key = ("n", id(node), name)
        if key in self.cache:
            return self.cache[key]
        r = self._node(node, name)
        self.cache[key] = r
        return r

    def _node(self, node: Node, name: str):
        t = self.fn.text
        k = node.kind
        if k in ("simple", "decl"):
            a = self.access(node, name)
            return READ if a == "read" else (set() if a == "write" else {"flow"})
        if k == "jump":
            w = word_at(t, node.start)
            if w == "goto":
                return READ
            if self.access(node, name) == "read":
                return READ
            return {w} if w in ("break", "continue") else set()
        if k == "label":
            return {"flow"} if word_at(t, node.start) in ("case", "default") else READ
        if k == "pp":
            return {"flow"}
        if k == "compound":
            return self.seq(node.children, name)
        if k == "if":
            ha, cond = self.header(node, name)
            if ha == "read":
                return READ
            then = set() if ha == "write" else self.node(node.children[0], name)
            if then == READ:
                return READ
            other = self.node(node.children[1], name) if len(node.children) > 1 else {"flow"}
            if other == READ:
                return READ
            return then | other
        if k == "switch":
            ha, _ = self.header(node, name)
            if ha == "read":
                return READ
            body = node.children[0]
            if body.kind != "compound":
                r = self.node(body, name)
                return READ if r == READ else {"flow"} | {s for s in r if s != "break"}
            regions = switch_regions(self.fn, node)
            out = {"flow"}  # a value no case claims
            kids = body.children
            for reg in regions:
                i = kids.index(reg.labels[0])
                r = self.seq(kids[i:], name)
                if r == READ:
                    return READ
                out |= {"flow" if s == "break" else s for s in r}
            return out
        if k == "loop":
            return self.loop(node, name, again=False)
        return READ

    def loop(self, node: Node, name: str, again: bool):
        """A loop entered (`again=False`: its init runs) or re-entered from its back edge.

        Returns the statuses of the paths that leave the loop still unwritten.
        A `for`/`while` header runs before the body (a zero-trip exit is always
        possible); a `do` body runs first and its condition after it.
        """
        body = node.children[0]
        if node.loop == "do":
            r = self.node(body, name)
            if r == READ:
                return READ
            ha, _ = self.header(node, name)
            if ha == "read" and r:
                return READ
            return {"flow"} if r else set()
        ha, conditional = self.header(node, name, skip_init=again)
        if ha == "read":
            return READ
        if ha == "write" and not conditional:
            return set()
        if ha == "write":
            return {"flow"}  # the body runs written; the exit path is not
        r = self.node(body, name)
        if r == READ:
            return READ
        return {"flow"}

    def seq(self, items: list, name: str):
        states = {"flow"}
        for it in items:
            if "flow" not in states:
                break
            r = self.node(it, name)
            if r == READ:
                return READ
            states = (states - {"flow"}) | r
        return states

    def after(self, node: Node, name: str, status: str = "flow", seen: set | None = None) -> bool:
        """May `name` be read unwritten after `node` finishes with `status`?"""
        if status == "goto":
            return True
        seen = set() if seen is None else seen
        key = (id(node), status)
        if key in seen:
            return False
        seen.add(key)
        parent = node.parent
        if parent is None:
            return False
        if parent.kind == "compound":
            if status != "flow":
                return self.after(parent, name, status, seen)
            idx = parent.children.index(node)
            r = self.seq(parent.children[idx + 1:], name)
            if r == READ:
                return True
            return any(self.after(parent, name, s, seen) for s in r)
        if parent.kind == "if":
            return self.after(parent, name, status, seen)
        if parent.kind == "switch":
            return self.after(parent, name, "flow" if status == "break" else status, seen)
        if parent.kind == "loop":
            if status == "break":
                return self.after(parent, name, "flow", seen)
            # the back edge: the header and the body once more, then the exit
            if self.loop(parent, name, again=True) == READ:
                return True
            return self.after(parent, name, "flow", seen)
        return True


def address_taken(fn: Function, name: str) -> bool:
    return bool(re.search(rf"&\s*{re.escape(name)}\b", strip_comments(fn.body.text(fn.text))))


def region_mentions(fn: Function, region: Region, name: str) -> bool:
    return any(mentions(fn, s, name) for s in region.stmts)


def rename_verdict(fn: Function, region: Region, old: str, new: str) -> str | None:
    """Is `old` -> `new` inside `region` behaviour-preserving? None = yes, else the reason.

    Checked on the renamed program: `new` is written before read inside the
    region (no value of `old` flows in), and after the region -- every exit it
    has, the loop back edge included -- neither `new` (its clobbered value) nor
    `old` (no longer written here) is read before being written again.
    """
    if not region.stmts or not region_mentions(fn, region, old):
        return "old not mentioned"
    if region_mentions(fn, region, new):
        return "new mentioned in the region"
    if address_taken(fn, old) or address_taken(fn, new):
        return "address taken"
    exits = region_exit_statuses(fn, region)
    if "goto" in exits:
        return "goto"
    df = Dataflow(fn, (region.start, region.end, old, new))
    if df.seq(region.stmts, new) == READ:
        return f"{old} read before written in the region"
    last = region.stmts[-1]
    for status in exits or {"return"}:
        if status == "return":
            continue
        if df.after(last, new, status):
            return f"{new} live after the region ({status})"
        if df.after(last, old, status):
            return f"{old} live after the region ({status})"
    return None


# ---------------------------------------------------------------- declared types

TYPE_BITS = {"s8": (8, True), "u8": (8, False), "s16": (16, True), "u16": (16, False),
             "s32": (32, True), "u32": (32, False), "int": (32, True), "unsigned": (32, False),
             "unsigned int": (32, False), "char": (8, True), "unsigned char": (8, False),
             "short": (16, True), "unsigned short": (16, False), "long": (32, True)}

GLOBAL_DECL_RE = re.compile(
    r"^[ \t]*(?:extern\s+)?(?:static\s+)?(?:(?:const|volatile)\s+)*((?:struct|union|enum)\s+)?"
    r"([A-Za-z_]\w*)\s+(\**)\s*([A-Za-z_]\w*)\s*((?:\[[^\]]*\]\s*)*)(?:;|=)", re.M)
PROTO_RE = re.compile(
    r"^[ \t]*(?:extern\s+)?(?:static\s+)?([A-Za-z_]\w*(?:\s+\*+|\s+)?)\s*([A-Za-z_]\w*)\s*\(([^()]*)\)\s*;", re.M)

_DECLS: dict = {}
_DECLS_ROOT: list = [None]


def repo_declarations() -> tuple[dict, dict]:
    """(global name -> type info, function name -> parameter list or None for K&R) from include/."""
    if _DECLS_ROOT[0] != ROOT:
        _DECLS.clear()
        _DECLS_ROOT[0] = ROOT
        globs, protos = {}, {}
        for h in sorted((ROOT / "include").rglob("*.h")) if (ROOT / "include").is_dir() else []:
            try:
                scan_declarations(h.read_text(errors="replace"), globs, protos)
            except Exception:
                continue
        _DECLS.update(globals=globs, protos=protos)
    return _DECLS["globals"], _DECLS["protos"]


def scan_declarations(text: str, globs: dict, protos: dict) -> None:
    text = strip_comments(text)
    for m in GLOBAL_DECL_RE.finditer(text):
        base, stars, name, arr = m.group(2), m.group(3), m.group(4), m.group(5)
        if base in KEYWORDS and base not in ("int", "char", "short", "long", "float", "double",
                                             "unsigned", "signed"):
            continue
        globs.setdefault(name, {"type": base, "ptr": len(stars), "array": bool(arr.strip())})
    for m in PROTO_RE.finditer(text):
        name, params = m.group(2), m.group(3).strip()
        if name in KEYWORDS:
            continue
        if params in ("", "void") and params == "":
            protos.setdefault(name, None)
        elif params == "void":
            protos.setdefault(name, [])
        else:
            protos.setdefault(name, [p.strip() for p in split_top(params)])


def declarations_for(fn: Function) -> tuple[dict, dict]:
    """Global and prototype tables: the repository's headers, then the TU itself."""
    globs, protos = repo_declarations()
    globs, protos = dict(globs), dict(protos)
    tu_g, tu_p = {}, {}
    scan_declarations(fn.text, tu_g, tu_p)
    globs.update(tu_g)
    protos.update(tu_p)
    return globs, protos


def param_type(params: list | None, index: int) -> str | None:
    """The bare declared type of parameter `index` (None: unknown or K&R)."""
    if params is None or index >= len(params):
        return None
    p = params[index]
    if "*" in p or "[" in p:
        return "pointer"
    words = [w for w in re.split(r"\s+", p.strip()) if w not in ("const", "volatile", "register")]
    if len(words) >= 2 and re.match(r"[A-Za-z_]\w*$", words[-1]) and words[-1] not in INT_TYPES \
            and words[-1] not in FLOAT_TYPES and words[-1] != "void":
        words = words[:-1]  # a parameter name
    return " ".join(words)


def operand_is_float(fn: Function, text: str, globs: dict, structs: dict) -> bool | None:
    """Is the operand expression floating? None when its type is unknown."""
    s = text.strip()
    while s.startswith("(") and s.endswith(")"):
        try:
            if match_close(s, 0) == len(s):
                s = s[1:-1].strip()
                continue
        except ValueError:
            pass
        break
    m = re.match(r"^\(\s*(f32|f64|float|double)\s*\)", s)
    if m:
        return True
    m = re.match(r"^\(\s*(?:u8|s8|u16|s16|u32|s32|int|unsigned|char|short|long)\s*\)", s)
    if m:
        return False
    m = re.match(r"^\*\s*\(\s*\(\s*([A-Za-z_]\w*)\s*\*\s*\)", s)  # *((T *) addr): a T
    if m:
        return m.group(1) in FLOAT_TYPES
    if re.match(r"^-?(?:\d+\.\d*|\.\d+|\d+[eE][-+]?\d+|\d+\.?\d*[fF])$", s):
        return True
    if re.match(r"^-?(?:0x[0-9A-Fa-f]+|\d+)[uUlL]*$", s):
        return False
    if re.search(r"[|&^~%]|<<|>>", re.sub(r"->|&&|\|\|", "", s)):
        return False  # a bitwise or remainder operator only applies to integers
    if re.search(r"[-+*/<>=!]", re.sub(r"->", "", s)):
        return None  # a compound expression: not classified
    ty = expr_type(fn, s, structs)
    if ty is None:
        m = IDENT_RE.match(s)
        if m and m.group(0) in globs and re.sub(r"\s+", "", s) == m.group(0):
            ty = globs[m.group(0)]
    if ty is None:
        return None
    if ty.get("ptr") or ty.get("array"):
        return False
    return bare_type(ty["type"]) in FLOAT_TYPES


INT_LITERAL_RE = re.compile(r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)([uU]?[lL]{0,2}|[lL]{1,2}[uU])(?![\w.])")
SAFE_LEFT = ("==", "!=", "&=", "|=", "^=", "<<=", ">>=", "<<", ">>", "&", "|", "^")
CHECK_LEFT = {"<=": "relational", ">=": "relational", "<": "relational", ">": "relational",
              "+": "arithmetic", "-": "arithmetic", "*": "arithmetic", "/": "arithmetic",
              "%": "arithmetic", "+=": "arithmetic", "-=": "arithmetic", "*=": "arithmetic",
              "/=": "arithmetic", "%=": "arithmetic", "=": "assignment", "(": "call argument",
              ",": "call argument", "[": "subscript", "?": "conditional", ":": "conditional",
              "&&": "logical", "||": "logical", "!": "logical", "return": "return"}


def other_operand(text: str, op_end: int, side: str) -> str:
    """The operand text next to an operator: left of `op_end` or right of it."""
    if side == "left":
        s = text[:op_end].rstrip()
        if s.endswith(")"):
            depth, i = 0, len(s) - 1
            while i >= 0:
                if s[i] == ")":
                    depth += 1
                elif s[i] == "(":
                    depth -= 1
                    if depth == 0:
                        break
                i -= 1
            j = i
            while j > 0 and (s[j - 1].isalnum() or s[j - 1] in "_. \t"):
                j -= 1
            k = len(s[:j].rstrip())
            if k and s[k - 1] in "*~!" and not (k > 1 and (s[k - 2].isalnum() or s[k - 2] in ")]_")):
                j = k - 1  # a unary operator on the parenthesised operand (a dereference)
            return s[j:]
        m = re.search(r"(?:\(\s*\w+\s*\*?\s*\)\s*)?[\w.\->\[\]]+$", s)
        return m.group(0) if m else ""
    s = text[op_end:].lstrip()
    m = re.match(r"(?:\(\s*\w+\s*\*?\s*\)\s*)?(?:\(|[\w.\->\[\]]+)", s)
    if not m:
        return ""
    if m.group(0).endswith("("):
        try:
            return s[:match_close(s, m.end() - 1)]
        except ValueError:
            return ""
    return m.group(0)


def literal_site(text: str, start: int, end: int) -> tuple[str, str, str, int]:
    """Classify a literal's site: (verdict, reason, other operand text, operator end).

    verdict: 'safe' (==, !=, &, |, ^, their assignments, case labels, shift
    counts), 'check' with the reason (relational, arithmetic, call argument,
    assignment, ...). `start:end` spans the spelling unit (unary ~/- included).
    """
    left = text[:start].rstrip()
    right = text[end:].lstrip()
    if re.search(r"\bcase$", left):
        return "safe", "case label", "", 0
    for op in sorted(SAFE_LEFT, key=len, reverse=True):
        if left.endswith(op):
            if op in ("&", "|") and left.endswith(op * 2):
                return "check", "logical", "", 0
            if op in ("<<", ">>", "<<=", ">>="):
                return "safe", "shift count", "", 0
            return "safe", f"left of {op}", other_operand(text, len(left) - len(op), "left"), 0
    for op in sorted(CHECK_LEFT, key=len, reverse=True):
        if left.endswith(op) and not (op in ("<", ">") and left.endswith(op * 2)):
            # a literal that is the LEFT operand of a safe operator on its right
            for rop in ("==", "!=", "&", "|", "^"):
                if right.startswith(rop) and not right.startswith(rop * 2) and not right.startswith(rop + "="):
                    return "safe", f"right of {rop}", other_operand(text, end + text[end:].index(rop) + len(rop), "right"), 0
            if right.startswith("<<") or right.startswith(">>"):
                return "check", "shifted operand", "", 0
            return "check", CHECK_LEFT[op], "", 0
    for rop in ("==", "!=", "&", "|", "^"):
        if right.startswith(rop) and not right.startswith(rop * 2) and not right.startswith(rop + "="):
            return "safe", f"right of {rop}", other_operand(text, end + text[end:].index(rop) + len(rop), "right"), 0
    return "check", "unclassified site", "", 0


def literal_spellings(unit: str) -> list[str]:
    """Bit-identical 32-bit spellings of one literal unit (`0xFF`, `~0xE00000`, `-1`, `1U`)."""
    m = re.match(r"^\s*([~-]?)\s*(0[xX][0-9A-Fa-f]+|\d+)([uUlL]*)\s*$", unit)
    if not m:
        return []
    sign, lit, suffix = m.groups()
    v = int(lit, 0)
    if v >= 1 << 32:
        return []
    if sign == "~":
        v = ~v & 0xFFFFFFFF
    elif sign == "-":
        v = -v & 0xFFFFFFFF
    is_hex = lit.lower().startswith("0x")
    out = set()
    plain = f"0x{v:X}" if is_hex or v >= 1 << 31 else str(v)
    out.add(plain)
    out.add(plain + "U")
    comp = ~v & 0xFFFFFFFF
    if bin(comp).count("1") <= bin(v).count("1") or v >= 1 << 31:
        out.add(f"~0x{comp:X}")
        out.add(f"~0x{comp:X}U")
    if v >= 1 << 31:
        out.add(f"-0x{(-v) & 0xFFFFFFFF:X}" if is_hex else f"-{(-v) & 0xFFFFFFFF}")
    norm = re.sub(r"\s+", "", unit).upper().replace("0X", "0x").rstrip("L")
    return sorted(s for s in out if s.upper().replace("0X", "0x") != norm)


def literal_units(text: str):
    """(start, end, unit) of every integer literal with its unary ~/- prefix, strings skipped."""
    text = re.sub(r'"(?:\\.|[^"\\])*"', lambda m: " " * len(m.group(0)), text)
    text = re.sub(r"'(?:\\.|[^'\\])+'", lambda m: " " * len(m.group(0)), text)
    for m in INT_LITERAL_RE.finditer(text):
        s, e = m.start(), m.end()
        if re.match(r"[eE.]", text[e:e + 1] or " ") or text[e:e + 1] in ("f", "F"):
            continue
        left = text[:s].rstrip()
        if left.endswith("~") or left.endswith("-"):
            before = left[:-1].rstrip()
            if left.endswith("-") and (before[-1:].isalnum() or before[-1:] in ")]_"):
                yield s, e, text[s:e]
                continue
            if left.endswith("-") and before.endswith("-"):
                yield s, e, text[s:e]
                continue
            yield len(left) - 1, e, text[len(left) - 1:e]
            continue
        yield s, e, text[s:e]


def lever_literal_type(fn, ctx):
    """Integer literal spellings (N <-> NU, M <-> ~M, -N) at value-safe sites only."""
    out = []
    t = fn.text
    globs = ctx.get("globals") or {}
    for s in ctx["stmts"] + ctx.get("headers", []):
        line = fn.line_of(s.start)
        if not ctx["in_lines"](line):
            continue
        body = t[s.start:s.end]
        clean = strip_comments(body)
        for u0, u1, unit in literal_units(clean):
            verdict, reason, other, _ = literal_site(clean, u0, u1)
            if verdict != "safe":
                continue
            sem = "safe"
            if other:
                isf = operand_is_float(fn, other, globs, ctx["structs"])
                if isf:
                    continue
                if isf is None:
                    sem = "check:operand type unknown"
            for sp in literal_spellings(unit):
                out.append(edit_cell(fn, "literal_type", f"{unit.strip()} -> {sp} ({reason})",
                                     [(s.start + u0, s.start + u1, sp)], sem,
                                     region_name_at(fn, s), line))
    return out


def lever_carrier_rename(fn, ctx):
    """Case-scoped rename of a local to another declared local proven dead across the region."""
    out = []
    t = fn.text
    for region in regions_cached(fn):
        if not region.stmts or not ctx["region_selected"](region):
            continue
        names = set()
        for s in region.stmts:
            names.update(idents(s.text(t)))
        olds = [n for n in names if n in fn.locals]
        for old in sorted(olds):
            io = fn.locals[old]
            if category(io) not in ("int", "pointer", "float"):
                continue
            enclosing = {id(a) for a in ancestors(region.switch)} | {id(region.switch)}
            for new, info in fn.locals.items():
                if new == old or new in names or category(info) != category(io) \
                        or info["ptr"] != io["ptr"] or info.get("array") or io.get("array"):
                    continue
                if id(info["decl"].parent) not in enclosing:
                    continue  # declared in a block that does not enclose the region
                if category(io) != "int" and bare_type(info["type"]) != bare_type(io["type"]):
                    continue
                why = rename_verdict(fn, region, old, new)
                if why:
                    continue
                sem = "safe" if bare_type(info["type"]) == bare_type(io["type"]) \
                    else f"check:carrier width {io['type']} -> {info['type']}"
                edits = []
                for s in region.stmts:
                    body = s.text(t)
                    nb = merge_locals_tool.rename_str(body, old, new)
                    if nb != body:
                        edits.append((s.start, s.end, nb))
                if edits:
                    out.append(edit_cell(fn, "carrier_rename", f"{old} -> {new} in {region.name}",
                                         edits, sem, region.name, fn.line_of(region.start)))
    return out


MASK_RHS_RE = re.compile(r"^\s*(.*?)\s*&\s*(0[xX][0-9A-Fa-f]+|\d+)[uU]?\s*$", re.S)
SHIFT_RHS_RE = re.compile(r"^\s*(.*?)\s*<<\s*(\d+)\s*$", re.S)


def unparen(s: str) -> str:
    s = s.strip()
    while s.startswith("(") and s.endswith(")"):
        try:
            if match_close(s, 0) != len(s):
                break
        except ValueError:
            break
        s = s[1:-1].strip()
    return s


def carrier_bits(info: dict) -> tuple[int, bool] | None:
    if info.get("ptr") or info.get("array"):
        return None
    return TYPE_BITS.get(bare_type(info["type"]))


def mask_move_sem(info: dict, mask: int) -> str:
    bits = carrier_bits(info)
    if bits is None:
        return "check:carrier type unknown"
    n, signed = bits
    if n >= 32:
        return "safe"
    if mask < (1 << (n - 1 if signed else n)):
        return "safe"
    return f"check:mask exceeds the {info['type']} carrier"


def call_arguments(text: str):
    """(callee, [(arg_start, arg_end)]) for a statement that is one call."""
    m = re.match(r"^\s*(?:[\w.\->\[\]]+\s*=\s*)?([A-Za-z_]\w*)\s*\(", text)
    if not m or m.group(1) in KEYWORDS:
        return None
    p0 = m.end() - 1
    try:
        p1 = match_close(text, p0)
    except ValueError:
        return None
    if text[p1:].strip() != ";":
        return None
    args, depth, cur = [], 0, p0 + 1
    i = p0 + 1
    while i < p1 - 1:
        c = text[i]
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        elif c == "," and depth == 0:
            args.append((cur, i))
            cur = i + 1
        i += 1
    if p1 - 1 > p0 + 1:
        args.append((cur, p1 - 1))
    spans = []
    for a0, a1 in args:
        s = text[a0:a1]
        lead = len(s) - len(s.lstrip())
        trail = len(s) - len(s.rstrip())
        spans.append((a0 + lead, a1 - trail))
    return m.group(1), spans


def single_use_call(fn: Function, comp: Node, idx: int, name: str):
    """The one later statement in `comp` reading `name`, when it is a call and nothing redefines first."""
    use = None
    for later in comp.children[idx + 1:]:
        if later.kind in ("label", "jump"):
            break  # the next case, or the end of this one's flow
        if not mentions(fn, later, name):
            continue
        if later.kind != "simple" or use is not None:
            return None
        use = later
    if use is None:
        return None
    body = strip_comments(use.text(fn.text))
    if len(re.findall(rf"(?<![\w.>]){re.escape(name)}\b", body)) != 1:
        return None
    return use


def lever_mask_shift(fn, ctx):
    """`x = (E) & M; f(x)` <-> `x = E; f(x & M)`, likewise `<< k`; introduced or dropped when the callee's parameter type makes them value-identical."""
    out = []
    t = fn.text
    _, protos = declarations_for(fn)
    for comp in walk(fn.body):
        if comp.kind != "compound":
            continue
        for i, s in enumerate(comp.children):
            if s.kind != "simple" or not ctx["in_lines"](fn.line_of(s.start)):
                continue
            m = ASSIGN_RE.match(strip_comments(s.text(t)))
            if not m or m.group(1) not in fn.locals:
                continue
            x, rhs = m.group(1), m.group(2).strip()
            if re.search(r"[A-Za-z_]\w*\s*\(", rhs.replace("sizeof", "")):
                continue  # a call in the carrier's definition: leave it
            use = single_use_call(fn, comp, i, x)
            if use is None:
                continue
            ubody = use.text(t)
            ca = call_arguments(strip_comments(ubody))
            if not ca:
                continue
            callee, spans = ca
            arg_index = next((k for k, (a0, a1) in enumerate(spans)
                              if re.fullmatch(rf"\(?\s*{re.escape(x)}\s*\)?(?:\s*(?:&|<<)\s*\S+)?", strip_comments(ubody)[a0:a1].strip())), None)
            if arg_index is None:
                continue
            a0, a1 = spans[arg_index]
            arg = strip_comments(ubody)[a0:a1].strip()
            if strip_comments(ubody)[a0:a1] != ubody[a0:a1]:
                continue  # a comment inside the argument: leave it
            info = fn.locals[x]
            ptype = param_type(protos.get(callee, None) if callee in protos else None, arg_index)
            region = region_name_at(fn, s)
            line = fn.line_of(s.start)
            m2 = re.match(rf"\s*{re.escape(x)}\s*=", s.text(t))
            if not m2:
                continue
            rhs_span = (s.start + m2.end(), s.end - 1)
            mm = MASK_RHS_RE.match(rhs)
            sm = SHIFT_RHS_RE.match(rhs)
            am = re.fullmatch(rf"{re.escape(x)}\s*&\s*(0[xX][0-9A-Fa-f]+|\d+)[uU]?", arg)
            asft = re.fullmatch(rf"{re.escape(x)}\s*<<\s*(\d+)", arg)
            if mm and arg == x:
                inner, mask = unparen(mm.group(1)), mm.group(2)
                sem = mask_move_sem(info, int(mask, 0))
                out.append(edit_cell(fn, "mask_shift", f"{x} & {mask}: assignment -> call ({region})",
                                     [(rhs_span[0], rhs_span[1], f" {inner}"),
                                      (use.start + a0, use.start + a1, f"{x} & {mask}")], sem, region, line))
                if ptype in ("u8", "u16", "unsigned char", "unsigned short") and \
                        int(mask, 0) == (1 << TYPE_BITS[ptype][0]) - 1:
                    out.append(edit_cell(fn, "mask_shift", f"{x} & {mask} dropped: {callee} takes {ptype} ({region})",
                                         [(rhs_span[0], rhs_span[1], f" {inner}")], "safe", region, line))
            elif am and not mm:
                mask = am.group(1)
                sem = mask_move_sem(info, int(mask, 0))
                out.append(edit_cell(fn, "mask_shift", f"{x} & {mask}: call -> assignment ({region})",
                                     [(rhs_span[0], rhs_span[1], f" ({rhs}) & {mask}"),
                                      (use.start + a0, use.start + a1, x)], sem, region, line))
            elif sm and arg == x:
                inner, k = unparen(sm.group(1)), sm.group(2)
                bits = carrier_bits(info)
                sem = "safe" if bits and bits[0] >= 32 else f"check:shift in a {info['type']} carrier"
                out.append(edit_cell(fn, "mask_shift", f"{x} << {k}: assignment -> call ({region})",
                                     [(rhs_span[0], rhs_span[1], f" {inner}"),
                                      (use.start + a0, use.start + a1, f"{x} << {k}")], sem, region, line))
            elif asft and not sm:
                k = asft.group(1)
                bits = carrier_bits(info)
                sem = "safe" if bits and bits[0] >= 32 else f"check:shift in a {info['type']} carrier"
                out.append(edit_cell(fn, "mask_shift", f"{x} << {k}: call -> assignment ({region})",
                                     [(rhs_span[0], rhs_span[1], f" ({rhs}) << {k}"),
                                      (use.start + a0, use.start + a1, x)], sem, region, line))
            if arg == x and not mm and not sm and re.search(r"->|\[|\*\s*\(", rhs):
                # introduce a byte/halfword mask: value-identical when the callee narrows to it
                for mask, width in (("0xFF", 8), ("0xFFFF", 16)):
                    if ptype in TYPE_BITS and TYPE_BITS[ptype][0] == width and not TYPE_BITS[ptype][1]:
                        sem = "safe"
                    elif ptype is None:
                        sem = f"check:{callee} has no prototype"
                    else:
                        continue
                    out.append(edit_cell(fn, "mask_shift", f"{x} & {mask} introduced in the assignment ({region})",
                                         [(rhs_span[0], rhs_span[1], f" {rhs} & {mask}")], sem, region, line))
                    out.append(edit_cell(fn, "mask_shift", f"{x} & {mask} introduced at the call ({region})",
                                         [(use.start + a0, use.start + a1, f"{x} & {mask}")], sem, region, line))
    return out


def is_stream_read(fn: Function, s: Node) -> str | None:
    """The local a statement `x = <load>;` assigns, when the RHS loads memory without a call or store."""
    if s.kind != "simple":
        return None
    body = strip_comments(s.text(fn.text))
    m = ASSIGN_RE.match(body)
    if not m or m.group(1) not in fn.locals:
        return None
    rhs = m.group(2)
    if re.search(r"[A-Za-z_]\w*\s*\(", rhs.replace("sizeof", "")):
        return None  # a call (a cast is `(type *)`, never `ident(`)
    if "volatile" in rhs or "=" in re.sub(r"==|!=|<=|>=", "", rhs):
        return None
    if not re.search(r"->|\[|\*\s*\(", rhs):
        return None
    return m.group(1)


def lever_read_order(fn, ctx):
    """Permutations of a run of adjacent independent stream reads (loads, no store between)."""
    out = []
    t = fn.text
    for comp in walk(fn.body):
        if comp.kind != "compound":
            continue
        kids = comp.children
        i = comp.ndecls
        while i < len(kids):
            run = []
            written: set = set()
            j = i
            while j < len(kids):
                x = is_stream_read(fn, kids[j])
                if x is None or x in written:
                    break
                reads = set(idents(strip_comments(kids[j].text(t)).split("=", 1)[1]))
                if reads & written or any(x in set(idents(strip_comments(r.text(t)).split("=", 1)[1])) for r in run):
                    break
                written.add(x)
                run.append(kids[j])
                j += 1
            if len(run) >= 2 and ctx["in_lines"](fn.line_of(run[0].start)):
                n = len(run)
                orders = []
                if n <= 4:
                    orders = [p for p in itertools.permutations(range(n)) if list(p) != list(range(n))]
                else:
                    for a in range(n):
                        for b in range(a + 1, n):
                            p = list(range(n))
                            p[a], p[b] = p[b], p[a]
                            orders.append(tuple(p))
                        if a:
                            orders.append(tuple([a] + [k for k in range(n) if k != a]))
                        if a != n - 1:
                            orders.append(tuple([k for k in range(n) if k != a] + [a]))
                texts = [s.text(t) for s in run]
                seen = set()
                for p in orders:
                    if p in seen:
                        continue
                    seen.add(p)
                    edits = [(run[k].start, run[k].end, texts[p[k]]) for k in range(n) if p[k] != k]
                    out.append(edit_cell(fn, "read_order", "reads in order " + ",".join(str(k) for k in p),
                                         edits, "safe", region_name_at(fn, run[0]), fn.line_of(run[0].start)))
            i = j if j > i else i + 1
    return out


SOURCE_LEVERS = ("literal_type", "carrier_rename", "reorder", "mask_shift", "read_order")

LEVERS = {
    "dead_read": lever_dead_read,
    "dead_masked": lever_dead_masked,
    "keep_alive": lever_keep_alive,
    "noop_redef": lever_noop_redef,
    "narrow_type": lever_narrow_type,
    "subscript": lever_subscript,
    "boundary": lever_boundary,
    "global_reread": lever_global_reread,
    "zero_def": lever_zero_def,
    "split_local": lever_split_local,
    "merge_locals": lever_merge_locals,
    "merge_pairs": lever_merge_pairs,
    "reorder": lever_reorder,
    "loop_move": lever_loop_move,
    "const_iv": lever_const_iv,
    "literal_type": lever_literal_type,
    "carrier_rename": lever_carrier_rename,
    "mask_shift": lever_mask_shift,
    "read_order": lever_read_order,
}


def header_nodes(fn: Function) -> list[Node]:
    """Unlinked pseudo-statements for every if/loop/switch header and case label."""
    out = []
    for node in walk(fn.body):
        if node.header:
            out.append(Node("hdr", node.header[0], node.header[1], parent=node.parent))
        elif node.kind == "label":
            out.append(node)
    return out


def parse_case_filter(specs: list[str], fn: Function) -> set[int]:
    """Source lines of the switch regions whose labels name one of `specs` (`0x22`, `default`)."""
    want = {re.sub(r"\s+", "", s).lower().replace("case", "") for s in specs}
    lines: set[int] = set()
    found = set()
    for r in regions_cached(fn):
        labels = {re.sub(r"\s+", "", label_name(fn, l)).lower().replace("case", "") for l in r.labels}
        hit = labels & want
        if hit:
            found |= hit
            lines.update(range(fn.line_of(r.start), fn.line_of(r.end) + 1))
    missing = want - found
    if missing:
        raise SystemExit(f"--case {sorted(missing)}: no switch region with that label in {fn.symbol}")
    return lines


def generate(fn: Function, levers: list[str], lines: set[int] | None, own_line: bool = False,
             window: int = 3, max_reads: int = 16) -> list[Cell]:
    """Every candidate cell of the chosen levers, deduplicated by text."""
    in_lines = (lambda line: True) if not lines else (lambda line: line in lines)
    pos = [(c, i) for c, i in positions(fn) if in_lines(position_line(fn, c, i))]
    stmts = simple_statements(fn)
    mentioned = set()
    for s in stmts:
        mentioned.update(idents(s.text(fn.text)))
    def region_selected(region: Region) -> bool:
        if not lines:
            return True
        return bool(lines & set(range(fn.line_of(region.start), fn.line_of(region.end) + 1)))

    ctx = {"positions": pos, "stmts": stmts, "in_lines": in_lines, "own_line": own_line,
           "window": window, "max_reads": max_reads, "mentioned": mentioned,
           "structs": structs_for(fn), "region_selected": region_selected,
           "headers": header_nodes(fn) if any(l == "literal_type" for l in levers) else [],
           "globals": declarations_for(fn)[0] if any(l == "literal_type" for l in levers) else {}}
    seen = {hashlib.sha1(fn.text.encode()).hexdigest()}
    cells = []
    for name in levers:
        for cell in LEVERS[name](fn, ctx):
            key = hashlib.sha1(cell.text.encode()).hexdigest()
            if key in seen:
                continue
            seen.add(key)
            cells.append(cell)
    return cells


def interleave(cells: list[Cell], limit: int) -> list[Cell]:
    """Round-robin across levers so a cap leaves every lever represented."""
    if len(cells) <= limit:
        return cells
    by = collections.OrderedDict()
    for c in cells:
        by.setdefault(c.lever, []).append(c)
    out = []
    while len(out) < limit:
        for k in list(by):
            if by[k]:
                out.append(by[k].pop(0))
                if len(out) == limit:
                    break
            else:
                del by[k]
        if not by:
            break
    return out


# ---------------------------------------------------------------- oracle

def parse_oracle(spec: str) -> list[tuple[str, int, str]]:
    out = []
    for f in [s.strip() for s in spec.split(",") if s.strip()]:
        m = FORCE_RE.match(f)
        if not m:
            raise SystemExit(f"oracle force {f!r} is not p1:wN=cK or p1:wN=s")
        out.append((f"p{m.group(1)}", int(m.group(2)), m.group(3)))
    return out


def decisions(text: str, proc: int) -> tuple[list[dict], dict[int, list[int]]]:
    """Decision rows of one procedure: web, phase, expr, kind, lines, decision, colour.

    Parsed with web_report's own record reader (`parse_records`, `build`), so a
    decision's colour is the colour row that follows it, as web_report reads it.
    """
    import web_report as wr
    model = wr.build(wr.parse_records(text), proc)
    blocks = model["blocks"]
    out = []
    for d in model["decisions"]:
        expr = d.get("webexpr", {})
        wb = d.get("webblocks")
        bbs = [int(b) for b in wb["bbs"].split(",") if b] if wb else []
        lines = set()
        for b in bbs:
            lines.update(blocks.get(b, {}).get("lines", []))
        out.append({"web": d["web"], "phase": d["phase"], "expr": expr.get("expr"),
                    "kind": expr.get("kind"), "bbs": bbs, "lines": sorted(lines),
                    "decision": d["dec"].get("decision"), "forced": d["dec"].get("forced"),
                    "colour": int(d["color"]["color"]) if d["color"] else None})
    return out, {b: info["lines"] for b, info in blocks.items()}


def oracle_targets(forced: list[dict], oracle) -> list[dict]:
    """Every decision row a force touched in the forced base, with the outcome it had.

    A split force leaves several rows under one web number (the split, then each
    piece's decision); all of them are part of the state to reproduce.
    """
    targets = []
    for phase, web, want in oracle:
        rows = [d for d in forced if d["phase"] == phase and d["web"] == web
                and d["forced"] == "-1"] or \
               [d for d in forced if d["phase"] == phase and d["web"] == web]
        if not rows:
            raise SystemExit(f"oracle {phase}:w{web}: no decision for that web in the forced base")
        targets.append({"spec": f"{phase}:w{web}={want}", "want": want, "rows": [
            {"expr": d["expr"], "kind": d["kind"], "lines": set(d["lines"]),
             "decision": d["decision"], "colour": d["colour"]} for d in rows]})
    return targets


def parse_bias(spec: str) -> list[tuple[int, float]]:
    out = []
    for f in [s.strip() for s in spec.split(",") if s.strip()]:
        m = re.match(r"^w?(\d+)=(-?\d+(?:\.\d+)?)$", f)
        if not m:
            raise SystemExit(f"bias {f!r} is not web=delta (e.g. 387=-4.5)")
        out.append((int(m.group(1)), float(m.group(2))))
    return out


def bias_env(bias: list[tuple[int, float]]) -> str:
    return ",".join(f"{w}={d:g}" for w, d in bias)


def p1_order(decs: list[dict]) -> list[dict]:
    """Phase-1 webs in decision order: the first decision row of each web."""
    seen, out = set(), []
    for d in decs:
        if d["phase"] == "p1" and d["web"] not in seen:
            seen.add(d["web"])
            out.append(d)
    return out


def bias_targets(unbiased: list[dict], biased: list[dict], bias) -> list[dict]:
    """Pairs (biased web, other web) whose decision order the bias reversed.

    Read off two logs of the same source: the bias is the only difference, so
    the reversed pairs are exactly the order the oracle prices. A cell
    satisfies one when, WITHOUT the bias, its records decide the two webs in
    the biased order. Each side is kept as an expression + lines signature so
    it can be found again in a renumbered cell.
    """
    ub = {d["web"]: i for i, d in enumerate(p1_order(unbiased))}
    bi_rows = p1_order(biased)
    bb = {d["web"]: i for i, d in enumerate(bi_rows)}
    by_web = {d["web"]: d for d in bi_rows}
    sig = lambda d: {"expr": d["expr"], "kind": d["kind"], "lines": set(d["lines"])}
    targets = []
    for w, delta in bias:
        if w not in bb:
            raise SystemExit(f"bias w{w}: no p1 decision for that web in the biased base")
        for o in bb:
            if o == w or w not in ub or o not in ub:
                continue
            if (ub[w] < ub[o]) != (bb[w] < bb[o]):
                first, second = (w, o) if bb[w] < bb[o] else (o, w)
                targets.append({"spec": f"w{first} before w{second}", "want": "order",
                                "first": sig(by_web[first]), "second": sig(by_web[second])})
    if not targets:
        raise SystemExit("bias reverses no p1 decision order against the unbiased base; "
                         "nothing for the oracle to price")
    return targets


def jaccard(a: set, b: set) -> float:
    return len(a & b) / len(a | b) if a | b else 0.0


def find_rows(cands: list[dict], row: dict) -> list[dict]:
    """Rows of a candidate's records that can be the forced row, best overlap first.

    Re-derived per candidate, never by web number: same expression and kind
    over at least 0.3 of the same source lines; failing that (the edit changed
    the expression's text) the same kind over at least 0.6 of the lines.
    """
    lines = set(row["lines"])
    same = [d for d in cands if d["expr"] == row["expr"] and d["kind"] == row["kind"]
            and jaccard(set(d["lines"]), lines) >= 0.3]
    if not same:
        same = [d for d in cands if d["kind"] == row["kind"]
                and jaccard(set(d["lines"]), lines) >= 0.6]
    return sorted(same, key=lambda d: -jaccard(set(d["lines"]), lines))


def tied(rows: list[dict], row: dict) -> list[dict]:
    """The leading rows that tie on overlap, one per distinct (phase, web)."""
    if not rows:
        return []
    lines = set(row["lines"])
    top = jaccard(set(rows[0]["lines"]), lines)
    out, seen = [], set()
    for d in rows:
        if top - jaccard(set(d["lines"]), lines) > 0.05:
            break
        key = (d["phase"], d["web"])
        if key not in seen:
            seen.add(key)
            out.append(d)
    return out


def web_name(d: dict) -> str:
    return f"{d['phase']}:w{d['web']}"


def order_status(cands: list[dict], tgt: dict) -> dict:
    order = {}
    for i, d in enumerate(p1_order(cands)):
        order[d["web"]] = i
    ends = []
    for side in ("first", "second"):
        ties = [d for d in tied(find_rows([d for d in cands if d["phase"] == "p1"], tgt[side]),
                                tgt[side])]
        ends.append(ties)
    if not ends[0] or not ends[1]:
        return {"spec": tgt["spec"], "ok": False, "ambiguous": False,
                "rows": [{"match": None}]}
    verdicts = {order[a["web"]] < order[b["web"]] for a in ends[0] for b in ends[1]
                if a["web"] != b["web"]}
    amb = len(verdicts) > 1
    return {"spec": tgt["spec"], "ok": verdicts == {True}, "ambiguous": amb,
            "rows": [{"match": [web_name(d) for d in ends[0]]},
                     {"match": [web_name(d) for d in ends[1]]}],
            "candidates": [web_name(d) for d in ends[0] + ends[1]] if amb else []}


def oracle_status(cands: list[dict], targets: list[dict]) -> tuple[int, list[dict]]:
    """How many oracle states the (unforced, unbiased) decision rows reproduce.

    Each forced row is re-derived among the candidate's rows (`find_rows`). A
    colour force's row must carry that colour. A split force reads as memory:
    every same-expression row over those lines must be uncoloured, and a row
    with no counterpart at all (the piece is never formed) counts as
    reproduced. A bias target is an order (`order_status`). When the leading
    rows tie on overlap and disagree about the outcome the target is
    `ambiguous`: it is neither counted as a hit nor as a miss, and the
    candidates are listed in the detail.
    """
    hits, detail = 0, []
    for tgt in targets:
        if tgt["want"] == "order":
            st = order_status(cands, tgt)
            hits += st["ok"] and not st["ambiguous"]
            detail.append(st)
            continue
        ok_all, amb, matches, cand_names = True, False, [], []
        for row in tgt["rows"]:
            same = find_rows(cands, row)
            if tgt["want"] == "s":
                # memory: no live piece of that value over those lines holds a register
                ok = all(d["colour"] is None for d in same)
                ok_all &= ok
                matches.append({"match": [web_name(d) for d in same],
                                "colour": [d["colour"] for d in same], "ok": ok})
                continue
            if not same:
                ok_all = False
                matches.append({"match": None})
                continue
            want = int(tgt["want"][1:])
            ties = tied(same, row)
            outcomes = {d["colour"] == want for d in ties}
            if len(outcomes) > 1:
                amb = True
                cand_names += [f"{web_name(d)}(c{d['colour']})" for d in ties]
            best = ties[0]
            score = jaccard(set(best["lines"]), row["lines"])
            ok = best["colour"] == want
            ok_all &= ok
            matches.append({"match": web_name(best), "jaccard": round(score, 2),
                            "decision": best["decision"], "colour": best["colour"], "ok": ok})
        hits += ok_all and not amb
        detail.append({"spec": tgt["spec"], "ok": ok_all and not amb, "ambiguous": amb,
                       "rows": matches, "candidates": cand_names})
    return hits, detail


def oracle_label(hits: int, n: int, ambiguous: int = 0) -> str:
    if n == 0:
        return "-"  # no oracle: nothing was asked, so "yes" would be a lie
    if ambiguous:
        return "ambiguous"
    return "yes" if hits == n else ("no" if hits == 0 else f"{hits}/{n}")


# ---------------------------------------------------------------- compiling

class Runner:
    def __init__(self, symbol: str, proc: int, scratch: pathlib.Path, source: str):
        import force_lattice as fl
        import align_symbol
        self.fl = fl
        self.symbol, self.proc, self.scratch, self.source = symbol, proc, scratch, source
        self.command = fl.compile_command(symbol)
        if self.command[-1] != source:
            raise SystemExit(f"configured command does not end in {source}")
        self.scorer = align_symbol.AlignedScorer(symbol, scratch)
        self.env = {k: v for k, v in os.environ.items() if not k.startswith(("CDX_", "DKWB_"))}

    def _cmd(self, src: pathlib.Path, obj: pathlib.Path, instrumented: bool) -> list[str]:
        cmd = list(self.command)
        if instrumented:
            cmd = self.fl.replace_compiler(cmd, self.fl.INSTRUMENTED / "cc")
        cmd[cmd.index("-o") + 1] = str(obj)
        cmd[-1] = str(src)
        return cmd

    def compile(self, src: pathlib.Path, obj: pathlib.Path, *, instrumented: bool = False,
                force: str | None = None, bias: str | None = None,
                log: pathlib.Path | None = None) -> str | None:
        env = dict(self.env)
        if instrumented:
            env.update(CDX_LOG="1", CDX_PROC=str(self.proc), CDX_DETAIL_WEB="all",
                       CDX_WEBREPORT="1", CDX_OUT=str(log))
            if force:
                env["CDX_FORCE"] = force
            if bias:
                env["CDX_BIAS"] = bias
        r = subprocess.run(self._cmd(src, obj, instrumented), env=env, cwd=ROOT,
                           capture_output=True, text=True, timeout=900)
        if r.returncode or not obj.is_file():
            return (r.stderr.strip().splitlines() or ["compile failed"])[-1]
        return None

    def func_bytes(self, obj: pathlib.Path) -> bytes | None:
        import nm_ranking as nr
        span = nr.func_symbol_span(obj, self.symbol)
        return nr.text_bytes(obj, *span) if span else None

    def size(self, obj: pathlib.Path) -> int | None:
        import nm_ranking as nr
        span = nr.func_symbol_span(obj, self.symbol)
        return span[1] if span else None


def measure_cell(run: Runner, index: int, cell: Cell, base: dict, targets, max_delta: int,
                 lock: threading.Lock, counter: list) -> dict:
    d = run.scratch / f"cell{index:04d}"
    d.mkdir(exist_ok=True)
    src = d / ("candidate" + pathlib.Path(run.source).suffix)
    src.write_text(cell.text)
    row = {"index": index, "lever": cell.lever, "line": cell.line, "edit": cell.edit,
           "semantics": cell.semantics}
    try:
        obj = d / "stock.o"
        err = run.compile(src, obj)
        if err:
            row["error"] = err
            return row
        size = run.size(obj)
        if size is None:
            row["error"] = "symbol not in object"
            return row
        row["delta"] = size - run.scorer.target_size
        row["dbase"] = size - base["size"]
        if abs(size - base["size"]) > max_delta:
            row["skipped"] = f"size moved {size - base['size']:+d}"
            return row
        fb = run.func_bytes(obj)
        if fb == base["bytes"]:
            row.update({k: base[k] for k in ("masked", "residual", "buckets", "oracle", "oracle_detail")})
            row["inert"] = True
            row["equals_forced"] = fb == base["forced_bytes"]
            return row
        score = run.scorer.score(obj)
        row["masked"], row["residual"] = score["masked"], score["residual"]
        row["buckets"] = [score["aligned_exact"], score["aligned_register_naming"],
                          score["aligned_immediate_only"], score["aligned_really_different"]]
        row["equals_forced"] = fb == base["forced_bytes"]
        if not targets:
            row["oracle"] = "-"
            return row
        log = d / "allocator.log"
        iobj = d / "instrumented.o"
        err = run.compile(src, iobj, instrumented=True, log=log)
        if err:
            row["oracle"] = "err"
        elif run.func_bytes(iobj) != fb:
            row["oracle"] = "gate"
        else:
            decs, _ = decisions(log.read_text(errors="replace"), run.proc)
            hits, detail = oracle_status(decs, targets)
            row["oracle"] = oracle_label(hits, len(targets), sum(d["ambiguous"] for d in detail))
            row["oracle_detail"] = detail
        return row
    finally:
        for f in d.glob("*"):
            if f.name != src.name:
                f.unlink(missing_ok=True)
        with lock:
            counter[0] += 1
            if counter[0] % 25 == 0:
                print(f"  {counter[0]} cells measured", flush=True)


def rank_key(r: dict):
    if "error" in r or "skipped" in r or "residual" not in r:
        return (1, 0, 0, 0, 0)
    exact = r["masked"] == 0 and r.get("delta") == 0
    return (0, not exact, r["residual"], r.get("oracle") != "yes", abs(r.get("delta", 0)), r["masked"])


def summary_line(symbol, proc, specs, generated, results, scored, errors, yes, ambiguous,
                 exact, best) -> str:
    """One line for a shard note."""
    pct = 100.0 * len(errors) / len(results) if results else 0.0
    tail = (f"best {best['lever']} L{best['line']} `{best['edit'][:40]}` masked {best['masked']} "
            f"delta {best['delta']:+d} resid {best['residual']}") if best else "no scored cell"
    return (f"SUMMARY lever_sweep {symbol} proc {proc} [{';'.join(specs)}]: {generated} cells, "
            f"{len(results)} measured, {len(errors)} compile errors ({pct:.0f}%), "
            f"oracle yes {len(yes)}, ambiguous {len(ambiguous)}, exact {len(exact)}; {tail}")


def parse_lines(spec: str | None) -> set[int]:
    if not spec:
        return set()
    lo, _, hi = spec.partition("-")
    return set(range(int(lo), int(hi or lo) + 1))


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--proc", type=int, help="procedure ordinal (as web_report prints); "
                                             "required unless --source-levers")
    ap.add_argument("--source-levers", action="store_true",
                    help="source-lever mode: stock compile only, both bases measured, no oracle; "
                         "default levers " + ",".join(SOURCE_LEVERS))
    ap.add_argument("--diag", metavar="SPEC",
                    help="pinned NON-adoptable measurement base: 'decl:NAME=TYPE[,...]' or @FILE "
                         "(a unified diff, or specs one per line); every cell is measured on both")
    ap.add_argument("--case", action="append", default=[], metavar="LABEL",
                    help="restrict to the switch region(s) labelled LABEL (e.g. 0x22); repeatable")
    ap.add_argument("--pairs", type=int, default=0, metavar="N",
                    help="also measure up to N pairs of same-region non-overlapping cells")
    ap.add_argument("--onehot", action="store_true", help="rank single cells (the default)")
    ap.add_argument("--greedy", action="store_true",
                    help="adopt the best safe improving cell, re-enumerate on the new base, repeat")
    ap.add_argument("--rounds", type=int, default=4, help="greedy rounds (default 4)")
    ap.add_argument("--emit", metavar="OUT.c",
                    help="write the frozen adoptable TU (SHAPE axes resolved) plus OUT.c.diff")
    ap.add_argument("--oracle", default="", help="CDX_FORCE spec the source must reproduce")
    ap.add_argument("--bias", default="",
                    help="CDX_BIAS oracle 'web=delta,...': the decision order it imposes is the "
                         "state to reproduce without it (may be combined with --oracle)")
    ap.add_argument("--root", help="repository worktree (default: git toplevel of the cwd)")
    ap.add_argument("--candidate", help="base TU (default: the tracked source)")
    ap.add_argument("--web", type=int, action="append", default=[],
                    help="restrict positions to the lines of the blocks this web spans (forced base)")
    ap.add_argument("--block", type=int, action="append", default=[],
                    help="restrict positions to this block's lines")
    ap.add_argument("--lines", help="restrict positions to a source line range LO-HI")
    ap.add_argument("--levers", default=",".join(LEVERS),
                    help="comma list (default all): " + ", ".join(LEVERS))
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--max-size-delta", type=int, default=16,
                    help="drop a cell whose size moves more than this many bytes from the base")
    ap.add_argument("--max-cells", type=int, default=600)
    ap.add_argument("--no-fail", action="store_true",
                    help="exit 0 when the sweep completes without an exact cell (default exits 1)")
    ap.add_argument("--window", type=int, default=3,
                    help="statements either side a dead read's source is taken from")
    ap.add_argument("--max-reads", type=int, default=16)
    ap.add_argument("--own-line", action="store_true", help="insert statements on their own line")
    ap.add_argument("--top", type=int, default=25)
    ap.add_argument("--keep", type=int, default=5, help="keep this many best cells' sources")
    ap.add_argument("--scratch", help="work directory (default /private/tmp/claude-501/lever-sweep-<pid>)")
    ap.add_argument("--json", metavar="PATH")
    ap.add_argument("--dry-run", action="store_true", help="generate and count cells only")
    ns = ap.parse_args(argv)

    use_root(resolve_root(ns.root))
    source_mode = ns.source_levers or ns.diag or ns.greedy or ns.emit or ns.case or ns.pairs
    levers_given = ns.levers != ",".join(LEVERS)
    if source_mode and not levers_given:
        levers = list(SOURCE_LEVERS)
    else:
        levers = [x.strip() for x in ns.levers.split(",") if x.strip()]
    unknown = [x for x in levers if x not in LEVERS]
    if unknown:
        raise SystemExit(f"unknown lever(s) {unknown}; known: {', '.join(LEVERS)}")
    if source_mode:
        if ns.oracle or ns.bias:
            raise SystemExit("--source-levers measures stock compiles only; drop --oracle/--bias")
        return source_main(ns, levers)
    if ns.proc is None:
        raise SystemExit("--proc is required (the oracle mode reads the instrumented records)")
    oracle = parse_oracle(ns.oracle)
    bias = parse_bias(ns.bias)
    # With neither, the sweep has no state to reproduce: cells are ranked by
    # masked words, aligned residual and size delta alone (oracle column "-").
    import fast_score
    source = fast_score.tracked_source_for(ns.symbol)
    base_path = pathlib.Path(ns.candidate).resolve() if ns.candidate else ROOT / source
    base_text = base_path.read_text()
    scratch = pathlib.Path(ns.scratch or f"/private/tmp/claude-501/lever-sweep-{os.getpid()}").resolve()
    if ROOT.resolve() in scratch.parents or scratch == ROOT.resolve():
        raise SystemExit("--scratch must be outside the repository")
    scratch.mkdir(parents=True, exist_ok=True)
    fn = find_function(base_text, ns.symbol)

    run = Runner(ns.symbol, ns.proc, scratch, source)
    bdir = scratch / "base"
    bdir.mkdir(exist_ok=True)
    bsrc = bdir / ("candidate" + pathlib.Path(source).suffix)
    bsrc.write_text(base_text)
    force_env = ",".join(f"{p}:w{w}={c}" for p, w, c in oracle) or None
    for label, kw in (("stock", {}), ("unforced", {"instrumented": True}),
                      ("forced", {"instrumented": True, "force": force_env,
                                  "bias": bias_env(bias) if bias else None})):
        err = run.compile(bsrc, bdir / f"{label}.o", log=bdir / f"{label}.log", **kw)
        if err:
            raise SystemExit(f"base {label} compile failed: {err}")
    base_bytes = run.func_bytes(bdir / "stock.o")
    if run.func_bytes(bdir / "unforced.o") != base_bytes:
        raise SystemExit("IDENTITY GATE FAILED: instrumented .text differs from stock on the base")
    forced_log = (bdir / "forced.log").read_text(errors="replace")
    specs = tuple(f"{p}:w{w}={c}" for p, w, c in oracle)
    refused = run.fl.force_acceptance(forced_log, ns.proc, specs) if specs else None
    if refused:
        raise SystemExit(f"oracle not accepted on the base: {refused}")
    forced_decs, blines = decisions(forced_log, ns.proc)
    targets = oracle_targets(forced_decs, oracle) if oracle else []
    if bias:
        unbiased_decs, _ = decisions((bdir / "unforced.log").read_text(errors="replace"), ns.proc)
        # a bias-only run compares against the unbiased base; with forces too, the
        # forced base is still biased, and the unforced log is the reference order
        targets += bias_targets(unbiased_decs, forced_decs, bias)
        specs = specs + tuple(f"bias {w}={d:g}" for w, d in bias)
    base_score = run.scorer.score(bdir / "stock.o")
    forced_score = run.scorer.score(bdir / "forced.o")
    base_decs, _ = decisions((bdir / "unforced.log").read_text(errors="replace"), ns.proc)
    bhits, bdetail = oracle_status(base_decs, targets)
    base = {"size": run.size(bdir / "stock.o"), "bytes": base_bytes,
            "forced_bytes": run.func_bytes(bdir / "forced.o"),
            "masked": base_score["masked"], "residual": base_score["residual"],
            "buckets": [base_score["aligned_exact"], base_score["aligned_register_naming"],
                        base_score["aligned_immediate_only"], base_score["aligned_really_different"]],
            "oracle": oracle_label(bhits, len(targets), sum(d["ambiguous"] for d in bdetail)),
            "oracle_detail": bdetail}
    print(f"{ns.symbol} proc {ns.proc}: base {base['masked']} masked, residual {base['residual']} "
          f"at {base['size'] - run.scorer.target_size:+d}; oracle {','.join(specs) or 'none'} accepted, "
          f"forced {forced_score['masked']} masked, residual {forced_score['residual']} at "
          f"{forced_score['delta']:+d}; base oracle state: {base['oracle']}")
    for t in targets:
        if t["want"] == "order":
            print(f"  oracle {t['spec']} (decision order, p1)")
            continue
        for r in t["rows"]:
            span = f"{min(r['lines'])}-{max(r['lines'])}" if r["lines"] else "-"
            print(f"  oracle {t['spec']}: {r['decision']} row, expr {r['expr']} kind {r['kind']}, "
                  f"{len(r['lines'])} lines in {span}, forced colour {r['colour']}")

    lines = parse_lines(ns.lines)
    for w in ns.web:
        rows = [d for d in forced_decs if d["web"] == w]
        if not rows:
            raise SystemExit(f"--web {w}: no decision for that web in proc {ns.proc}")
        for r in rows:
            lines.update(r["lines"])
    for b in ns.block:
        if b not in blines:
            raise SystemExit(f"--block {b}: no such block in proc {ns.proc}")
        lines.update(blines[b])
    cells = generate(fn, levers, lines or None, ns.own_line, ns.window, ns.max_reads)
    counts = collections.Counter(c.lever for c in cells)
    kept = interleave(cells, ns.max_cells)
    print(f"cells: {len(cells)} generated ({', '.join(f'{k} {v}' for k, v in counts.items())}); "
          f"{len(kept)} measured" + (f" (--max-cells {ns.max_cells})" if len(kept) < len(cells) else "")
          + (f"; positions restricted to {len(lines)} lines" if lines else ""), flush=True)
    if ns.dry_run:
        return 0

    lock, counter = threading.Lock(), [0]
    with ThreadPoolExecutor(max_workers=ns.jobs) as pool:
        results = list(pool.map(lambda ic: measure_cell(run, ic[0], ic[1], base, targets,
                                                        ns.max_size_delta, lock, counter),
                                enumerate(kept)))
    results.sort(key=rank_key)
    scored = [r for r in results if "residual" in r]
    errors = [r for r in results if "error" in r]
    skipped = [r for r in results if "skipped" in r]
    inert = [r for r in scored if r.get("inert")]
    yes = [r for r in scored if r.get("oracle") == "yes"]
    exact = [r for r in scored if r["masked"] == 0 and r.get("delta") == 0]
    print(f"measured {len(results)}: scored {len(scored)} ({len(inert)} inert), "
          f"size-skipped {len(skipped)}, compile errors {len(errors)}; oracle reproduced "
          f"{len(yes)}; exact {len(exact)}")
    print(f"{'cell':>5} {'lever':<14} {'line':>5} {'masked':>6} {'delta':>6} {'resid':>5} "
          f"{'naming':>6} {'imm':>4} {'diff':>4} {'dbase':>5} {'oracle':>6} {'sem':>5}  edit")
    for r in [r for r in scored if not r.get("inert")][: ns.top]:
        b = r["buckets"]
        tag = r.get("oracle", "-") + ("=F" if r.get("equals_forced") and targets else "")
        print(f"{r['index']:>5} {r['lever']:<14} {r['line']:>5} {r['masked']:>6} {r['delta']:>+6} "
              f"{r['residual']:>5} {b[1]:>6} {b[2]:>4} {b[3]:>4} {r.get('dbase', 0):>+5} {tag:>6} {r['semantics']:>5}  "
              f"{r['edit'][:70]}")
    keepdir = scratch / "best"
    keepdir.mkdir(exist_ok=True)
    for r in [r for r in scored if not r.get("inert")][: ns.keep]:
        src = scratch / f"cell{r['index']:04d}" / ("candidate" + pathlib.Path(source).suffix)
        if src.exists():
            shutil.copy(src, keepdir / f"cell{r['index']:04d}{src.suffix}")
    for r in results:
        d = scratch / f"cell{r['index']:04d}"
        shutil.rmtree(d, ignore_errors=True)
    print(f"best cells' sources: {keepdir}")
    ambiguous = [r for r in scored if r.get("oracle") == "ambiguous" and not r.get("inert")]
    for r in ambiguous[: ns.top]:
        for d in r.get("oracle_detail", []):
            if d.get("ambiguous"):
                print(f"  cell {r['index']} oracle: ambiguous for {d['spec']}: "
                      f"{', '.join(d.get('candidates', []))}")
    best = next((r for r in scored if not r.get("inert")), None)
    print(summary_line(ns.symbol, ns.proc, specs, len(cells), results, scored, errors, yes,
                       ambiguous, exact, best))
    if ns.json:
        pathlib.Path(ns.json).write_text(json.dumps({
            "symbol": ns.symbol, "proc": ns.proc, "oracle": list(specs),
            "base": {k: v for k, v in base.items() if k not in ("bytes", "forced_bytes")},
            "forced": {"masked": forced_score["masked"], "residual": forced_score["residual"],
                       "delta": forced_score["delta"]},
            "targets": [{**t, "rows": [{**r, "lines": sorted(r["lines"])} for r in t["rows"]]}
                        if "rows" in t else {"spec": t["spec"], "want": t["want"]}
                        for t in targets],
            "cells": results}, indent=1, default=str))
    return exit_status(exact, ns.no_fail)


def exit_status(exact: list, no_fail: bool) -> int:
    """0 with an exact cell; otherwise 1, or 0 under --no-fail, with a line saying why."""
    if exact:
        return 0
    print("no exact cell: the sweep completed" + (" (exit 0 by --no-fail)" if no_fail
                                                 else " (exit 1; --no-fail makes it 0)"))
    return 0 if no_fail else 1


# ---------------------------------------------------------------- source mode

class DiagError(Exception):
    pass


def parse_diag(spec: str | None) -> list[str]:
    """`decl:NAME=TYPE[,decl:...]` or `@FILE` (a unified diff, or one spec per line)."""
    if not spec:
        return []
    if spec.startswith("@"):
        text = pathlib.Path(spec[1:]).read_text()
        if re.match(r"^(---|\+\+\+|diff |@@)", text.lstrip()):
            return ["patch:" + spec[1:]]
        return [l.strip() for l in text.splitlines() if l.strip() and not l.startswith("#")]
    return [s.strip() for s in spec.split(",") if s.strip()]


def apply_diag(text: str, symbol: str, diags: list[str]) -> str:
    """The pinned NON-adoptable measurement base derived from `text`."""
    for d in diags:
        kind, _, rest = d.partition(":")
        if kind == "decl":
            name, _, ty = rest.partition("=")
            fn = find_function(text, symbol)
            info = fn.locals.get(name.strip())
            if info is None:
                raise DiagError(f"diag decl: no local {name!r} in {symbol}")
            decl = info["decl"]
            dtext = decl.text(text)
            if "," in strip_comments(dtext):
                raise DiagError(f"diag decl: {name} shares a declaration line")
            ntext = re.sub(rf"^(\s*){re.escape(info['type'])}\b", rf"\g<1>{ty.strip()}", dtext, count=1)
            if ntext == dtext:
                raise DiagError(f"diag decl: {name} is already {ty}")
            text = text[:decl.start] + ntext + text[decl.end:]
        elif kind == "patch":
            with tempfile.TemporaryDirectory(prefix="lever-diag-") as tmp:
                src = pathlib.Path(tmp) / "src.c"
                src.write_text(text)
                r = subprocess.run(["patch", "-s", "-f", "-o", str(src) + ".out", str(src), rest],
                                   capture_output=True, text=True)
                if r.returncode:
                    raise DiagError("diag patch did not apply: " + (r.stderr.strip() or r.stdout.strip()))
                text = pathlib.Path(str(src) + ".out").read_text()
        else:
            raise DiagError(f"diag spec {d!r}: expected decl:NAME=TYPE or @FILE")
    return text


PERMUTATION_LEVERS = ("reorder", "read_order")


def compose_edits(fn: Function, a: Cell, b: Cell):
    """The edits of two cells applied together, or None when they collide.

    A permutation cell (reorder, read_order) replaces whole statements with
    other statements' base text, so an edit INSIDE one of those statements is
    folded into the moved text rather than refused: the 0x22 lever was exactly
    a swap plus a mask written into the swapped statement.
    """
    def fold(perm: Cell, other: Cell):
        edits = list(perm.edits)
        rest = []
        for bs, be, br in other.edits:
            for i, (s, e, r) in enumerate(edits):
                # the slot `s:e` now holds the base text of some statement; find it
                src = next(((ss, se) for ss, se, _ in perm.edits if fn.text[ss:se] == r), None)
                if src and src[0] <= bs and be <= src[1]:
                    rel = bs - src[0]
                    edits[i] = (s, e, r[:rel] + br + r[rel + (be - bs):])
                    break
            else:
                rest.append((bs, be, br))
        if edits_overlap(edits, rest):
            return None
        return tuple(edits) + tuple(rest)

    if a.lever in PERMUTATION_LEVERS and b.lever not in PERMUTATION_LEVERS:
        return fold(a, b)
    if b.lever in PERMUTATION_LEVERS and a.lever not in PERMUTATION_LEVERS:
        return fold(b, a)
    if edits_overlap(a.edits, b.edits):
        return None
    return a.edits + b.edits


def compose_pairs(cells: list[Cell], fn: Function, limit: int) -> list[Cell]:
    """Cells of the same region whose edits do not overlap, applied together."""
    by: dict[str, list[Cell]] = collections.OrderedDict()
    for c in cells:
        if c.edits and c.group:
            by.setdefault(c.group, []).append(c)  # by OUTERMOST region: nested switches pair too
    out = []
    for region, group in by.items():
        for a, b in itertools.combinations(group, 2):
            if a.lever == b.lever == "carrier_rename":
                continue
            edits = compose_edits(fn, a, b)
            if edits is None:
                continue
            sem = "safe" if a.semantics == b.semantics == "safe" else \
                ";".join(sorted({s for s in (a.semantics, b.semantics) if s != "safe"}))
            cell = edit_cell(fn, "+".join(sorted((a.lever, b.lever))),
                             f"{a.lever}: {a.edit} + {b.lever}: {b.edit}",
                             edits, sem, region, min(a.line, b.line))
            out.append(cell)
    kept = interleave(out, limit) if len(out) > limit else out  # even across lever combinations
    for c in kept:
        c.lever = "pair"
    return kept


class SourceScorer:
    """Compile a TU text with the configured direct command and score one symbol."""

    def __init__(self, symbol: str, scratch: pathlib.Path, source: str):
        import fast_score
        import align_symbol
        self.symbol, self.scratch, self.source = symbol, scratch, source
        self.args = fast_score.configured_cc_args(source)
        self.rewrite = fast_score.rewrite_io
        self.scorer = align_symbol.AlignedScorer(symbol, scratch)
        self.env = {k: v for k, v in os.environ.items() if not k.startswith(("CDX_", "DKWB_"))}
        self.n = 0
        self.lock = threading.Lock()

    def __call__(self, text: str) -> dict:
        with self.lock:
            self.n += 1
            k = self.n
        d = self.scratch / f"m{k:05d}"
        d.mkdir(exist_ok=True)
        src = d / ("candidate" + pathlib.Path(self.source).suffix)
        obj = d / "stock.o"
        src.write_text(text)
        try:
            r = subprocess.run(self.rewrite(self.args, src, self.source, obj), env=self.env, cwd=ROOT,
                               capture_output=True, text=True, timeout=900)
            if r.returncode or not obj.is_file():
                return {"error": (r.stderr.strip().splitlines() or ["compile failed"])[-1]}
            score = self.scorer.score(obj)
            if score is None:
                return {"error": "symbol not in object"}
            return {"masked": score["masked"], "delta": score["delta"], "residual": score["residual"],
                    "buckets": [score["aligned_exact"], score["aligned_register_naming"],
                                score["aligned_immediate_only"], score["aligned_really_different"]]}
        finally:
            shutil.rmtree(d, ignore_errors=True)


def measure_text(score, text: str, symbol: str, diags: list[str]) -> dict:
    """Both columns of one text: `a` (adoptable) and, with diags, `d` (the diag base)."""
    row = {"a": score(text)}
    if diags:
        try:
            row["d"] = score(apply_diag(text, symbol, diags))
        except DiagError as e:
            row["d"] = {"error": str(e)}
    return row


def rank_column(base: dict, diags: list[str]) -> str:
    """Rank on the diag column when the adoptable base is off size (and a diag base exists)."""
    if diags and "error" not in base.get("d", {"error": 1}) and base["a"].get("delta", 0) != 0:
        return "d"
    return "a"


def improves(row: dict, base: dict, col: str) -> bool:
    """Lower residual at no worse size on the ranking column -- or, whatever the
    column, an adoptable result that is closer to size with no worse residual
    (the 0x22 swap took the adoptable base from +4 to size 0 at the same
    residual while the diag column read it as worse)."""
    def better(c, size_only=False):
        r, b = row.get(c, {}), base.get(c, {})
        if "error" in r or "error" in b or "residual" not in r or "residual" not in b:
            return False
        if size_only:
            return abs(r["delta"]) < abs(b["delta"]) and r["residual"] <= b["residual"]
        return r["residual"] < b["residual"] and abs(r["delta"]) <= abs(b["delta"])
    return better(col) or better("a", size_only=True)


def source_rank_key(row: dict, base: dict, col: str):
    r = row.get(col, {})
    if "error" in r or "residual" not in r:
        return (2, 0, 0, 0, 0)
    return (0 if improves(row, base, col) else 1, r["residual"], abs(r["delta"]), r["masked"],
            row.get("semantics") != "safe")


def source_sweep(text: str, symbol: str, levers: list[str], score, *, lines: set[int] | None = None,
                 cases: list[str] | None = None, diags: list[str] | None = None, jobs: int = 3,
                 max_cells: int = 600, pairs: int = 0, greedy: int = 0, log=print) -> dict:
    """Enumerate, measure (both bases) and rank; with `greedy` rounds, adopt and repeat.

    `score(text) -> {masked, delta, residual, buckets} | {error}` is the
    measurement; the tool injects the configured compile, tests inject a stub.
    Returns {"rounds": [...], "final": text, "base": row, "column": "a"|"d"}.
    """
    diags = diags or []
    rounds = []
    current = text
    base = measure_text(score, current, symbol, diags)
    if "error" in base["a"]:
        raise SystemExit(f"base does not compile: {base['a']['error']}")
    if diags and "error" in base["d"]:
        raise SystemExit(f"diag base: {base['d']['error']}")
    col = rank_column(base, diags)
    log(f"{symbol}: base adoptable masked {base['a']['masked']} delta {base['a']['delta']:+d} "
        f"residual {base['a']['residual']}" + (f"; diag masked {base['d']['masked']} delta "
                                               f"{base['d']['delta']:+d} residual {base['d']['residual']}"
                                               if diags else "") + f"; ranking on the {'diag' if col == 'd' else 'adoptable'} column")
    for rnd in range(max(1, greedy + 1) if greedy else 1):
        fn = find_function(current, symbol)
        sel = set(lines or set())
        if cases:
            sel |= parse_case_filter(cases, fn)
        cells = generate(fn, levers, sel or None)
        if pairs:
            cells += compose_pairs(cells, fn, pairs)
        counts = collections.Counter(c.lever for c in cells)
        kept = interleave(cells, max_cells)
        log(f"round {rnd}: {len(cells)} cells ({', '.join(f'{k} {v}' for k, v in counts.items())})"
            + (f"; {len(kept)} measured" if len(kept) < len(cells) else ""))
        counter = [0]
        lock = threading.Lock()

        def one(ic):
            i, c = ic
            row = measure_text(score, c.text, symbol, diags)
            row.update(index=i, lever=c.lever, line=c.line, edit=c.edit, semantics=c.semantics,
                       region=c.region, text=c.text)
            with lock:
                counter[0] += 1
                if counter[0] % 50 == 0:
                    log(f"  {counter[0]} cells measured")
            return row

        with ThreadPoolExecutor(max_workers=jobs) as pool:
            results = list(pool.map(one, enumerate(kept)))
        results.sort(key=lambda r: source_rank_key(r, base, col))
        improving = [r for r in results if improves(r, base, col)]
        rounds.append({"round": rnd, "base": {k: v for k, v in base.items()}, "column": col,
                       "generated": len(cells), "measured": len(results),
                       "errors": sum("error" in r["a"] for r in results),
                       "improving": len(improving), "cells": results})
        if not greedy:
            break
        pick = next((r for r in improving if r["semantics"] == "safe"), None)
        if pick is None:
            log(f"round {rnd}: no safe improving cell; stopping")
            break
        log(f"round {rnd}: adopt {pick['lever']} L{pick['line']} `{pick['edit']}` -> "
            + render_cols(pick, diags))
        current = pick["text"]
        base = {k: pick[k] for k in ("a", "d") if k in pick}
        rounds[-1]["adopted"] = {k: pick[k] for k in ("lever", "line", "edit", "semantics", "a", "d") if k in pick}
        col = rank_column(base, diags)
    return {"rounds": rounds, "final": current, "base": base, "column": col}


def render_cols(row: dict, diags: list[str]) -> str:
    def one(c):
        r = row.get(c, {})
        if "error" in r:
            return f"{'ERR':>6} {'':>6} {'':>5}"
        if "residual" not in r:
            return f"{'-':>6} {'':>6} {'':>5}"
        return f"{r['masked']:>6} {r['delta']:>+6} {r['residual']:>5}"
    return one("a") + ("  " + one("d") if diags else "")


def print_source_table(result: dict, diags: list[str], top: int) -> None:
    head = f"{'cell':>5} {'lever':<14} {'line':>5} {'region':<12} {'masked':>6} {'delta':>6} {'resid':>5}"
    if diags:
        head += f"  {'d.mask':>6} {'d.dlt':>6} {'d.res':>5}"
    print(head + "  sem  edit")
    last = result["rounds"][-1]
    base = last["base"]
    for r in last["cells"][:top]:
        mark = "*" if improves(r, base, last["column"]) else " "
        print(f"{r['index']:>5} {r['lever']:<14} {r['line']:>5} {r['region'][:12]:<12} "
              + render_cols(r, diags) + f" {mark}{r['semantics'][:24]}  {r['edit'][:60]}")


def function_diff(base_text: str, new_text: str, symbol: str) -> str:
    """A unified diff of the function's own lines (whole-TU line numbers kept)."""
    a = find_function(base_text, symbol)
    b = find_function(new_text, symbol)
    a_lines = base_text[a.body.start:a.body.end].splitlines(keepends=True)
    b_lines = new_text[b.body.start:b.body.end].splitlines(keepends=True)
    diff = difflib.unified_diff(a_lines, b_lines, fromfile=f"{symbol} (base)", tofile=f"{symbol} (emitted)",
                                n=2)
    out = []
    la, lb = base_text.count("\n", 0, a.body.start), new_text.count("\n", 0, b.body.start)
    for line in diff:
        m = re.match(r"^@@ -(\d+)(,\d+)? \+(\d+)(,\d+)? @@", line)
        if m:
            line = f"@@ -{int(m.group(1)) + la}{m.group(2) or ''} +{int(m.group(3)) + lb}{m.group(4) or ''} @@\n"
        out.append(line)
    return "".join(out)


def emit_frozen(text: str, out: pathlib.Path, base_text: str, symbol: str) -> None:
    """Write the adoptable TU (SHAPE axes resolved, carrier macros expanded) and its function diff."""
    if re.search(r"\bSHAPE_\w+", text):
        import shape_freeze
        text = shape_freeze.freeze_text(text, {})
    out.write_text(text)
    pathlib.Path(str(out) + ".diff").write_text(function_diff(base_text, text, symbol))


def source_main(ns, levers: list[str]) -> int:
    import fast_score
    source = fast_score.tracked_source_for(ns.symbol)
    base_path = pathlib.Path(ns.candidate).resolve() if ns.candidate else ROOT / source
    base_text = base_path.read_text()
    scratch = pathlib.Path(ns.scratch or f"/private/tmp/claude-501/lever-sweep-{os.getpid()}").resolve()
    if ROOT.resolve() in scratch.parents or scratch == ROOT.resolve():
        raise SystemExit("--scratch must be outside the repository")
    scratch.mkdir(parents=True, exist_ok=True)
    diags = parse_diag(ns.diag)
    fn = find_function(base_text, ns.symbol)
    lines = parse_lines(ns.lines)
    if ns.case:
        lines |= parse_case_filter(ns.case, fn)
    if ns.dry_run:
        cells = generate(fn, levers, lines or None)
        if ns.pairs:
            cells += compose_pairs(cells, fn, ns.pairs)
        counts = collections.Counter(c.lever for c in cells)
        print(f"cells: {len(cells)} generated ({', '.join(f'{k} {v}' for k, v in counts.items())})")
        for c in cells[: ns.top]:
            print(f"  {c.lever:<14} L{c.line:<5} {c.region[:12]:<12} {c.semantics[:28]:<28} {c.edit[:70]}")
        return 0
    score = SourceScorer(ns.symbol, scratch, source)
    result = source_sweep(base_text, ns.symbol, levers, score, lines=lines or None, cases=None,
                          diags=diags, jobs=ns.jobs, max_cells=ns.max_cells, pairs=ns.pairs,
                          greedy=ns.rounds if ns.greedy else 0)
    print_source_table(result, diags, ns.top)
    last = result["rounds"][-1]
    improving = [r for r in last["cells"] if improves(r, last["base"], last["column"])]
    safe = [r for r in improving if r["semantics"] == "safe"]
    adopted = [r["adopted"] for r in result["rounds"] if "adopted" in r]
    print(f"SUMMARY lever_sweep --source-levers {ns.symbol}: {len(result['rounds'])} round(s), "
          f"{sum(r['measured'] for r in result['rounds'])} cells measured, {len(adopted)} adopted"
          f"{' (' + '; '.join(a['edit'] for a in adopted) + ')' if adopted else ''}; last round "
          f"{len(improving)} improving ({len(safe)} safe) on the {'diag' if last['column'] == 'd' else 'adoptable'} column")
    if ns.emit:
        final = result["final"]
        if not adopted and last["cells"] and improves(last["cells"][0], last["base"], last["column"]):
            final = last["cells"][0]["text"]  # one-hot: the best improving cell
        emit_frozen(final, pathlib.Path(ns.emit), base_text, ns.symbol)
        print(f"emitted {ns.emit} (+ .diff)")
    if ns.json:
        pathlib.Path(ns.json).write_text(json.dumps({
            "symbol": ns.symbol, "mode": "source-levers", "levers": levers, "diag": diags,
            "rounds": [{**r, "cells": [{k: v for k, v in c.items() if k != "text"} for c in r["cells"]]}
                       for r in result["rounds"]]}, indent=1, default=str))
    shutil.rmtree(scratch, ignore_errors=True)
    return 0 if adopted or improving else 1


if __name__ == "__main__":
    sys.exit(main())
