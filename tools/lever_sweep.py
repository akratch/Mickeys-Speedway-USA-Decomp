#!/usr/bin/env python3
"""Search the source-lever catalogue for the edit that reproduces a forced allocator state.

    tools/lever_sweep.py <symbol> --proc N --oracle 'p1:w387=s[,p2:w131=c21...]'
    tools/lever_sweep.py <symbol> --proc N --bias '429=40,442=50,...'
                         [--candidate FILE] [--web W ...] [--block B ...]
                         [--lines LO-HI] [--levers a,b] [--jobs J]
                         [--max-size-delta 16] [--max-cells 600] [--json PATH]

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
  reorder        two adjacent independent statements swapped (statement order
                 reaches as1 and ugen emission order; semantics: check when
                 both touch memory).
  loop_move      the statement before a loop moved into its body head, or the
                 body's first statement hoisted out (semantics: check).
  const_iv       `K = C; for (v = K; ...)` for `for (v = C; ...)`: a constant
                 symbol folded into the IV init (w26 on func_80009414).

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

A BIAS ORACLE. `--bias 'web=delta,...'` prices a decision ORDER instead of
colours: the forced base is compiled with `CDX_BIAS` (globalcolor's selection
key only, brief Instruments), and the state to reproduce is the colour (or
the split) each biased p1 web then took. It combines with `--oracle`.

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
import hashlib
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

ROOT = pathlib.Path(__file__).resolve().parents[1]

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
    """Variable-like identifiers: not member names, not keywords."""
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
    semantics: str = "safe"


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
    seg = re.sub(rf"(?<![\w.>]){re.escape(old)}\b", new, t[start:end])
    return t[:start] + seg + t[end:]


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
            out.append(Cell("merge_locals", line, f"{y} -> {x} (decl kept)",
                            rename_from(t, first_y.start, fn.body.end, y, x), "check"))
            decl = iy["decl"]
            if "," not in strip_comments(decl.text(t)):
                cut = decl.end - decl.start
                dropped = t[:decl.start] + t[decl.end:]
                out.append(Cell("merge_locals", line, f"{y} -> {x} (decl dropped)",
                                rename_from(dropped, first_y.start - cut, fn.body.end - cut, y, x),
                                "check"))
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
            wa, ra, ca, ma = writes_reads(fn, a)
            wb, rb, cb, mb = writes_reads(fn, b)
            if ca or cb or wa is None or wb is None:
                continue
            if not ma and (wa in rb or (not mb and wa == wb)):
                continue
            if not mb and (wb in ra or (not ma and wa == wb)):
                continue
            loads_a = bool(re.search(r"->|\[|\*", strip_comments(a.text(t)).split("=", 1)[-1]))
            loads_b = bool(re.search(r"->|\[|\*", strip_comments(b.text(t)).split("=", 1)[-1]))
            sem = "check" if (ma and (mb or loads_b)) or (mb and loads_a) else "safe"
            text = t[:a.start] + b.text(t) + t[a.end:b.start] + a.text(t) + t[b.end:]
            out.append(Cell("reorder", fn.line_of(a.start), "swap with next", text, sem))
    return out


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
    "reorder": lever_reorder,
    "loop_move": lever_loop_move,
    "const_iv": lever_const_iv,
}


def generate(fn: Function, levers: list[str], lines: set[int] | None, own_line: bool = False,
             window: int = 3, max_reads: int = 16) -> list[Cell]:
    """Every candidate cell of the chosen levers, deduplicated by text."""
    in_lines = (lambda line: True) if not lines else (lambda line: line in lines)
    pos = [(c, i) for c, i in positions(fn) if in_lines(position_line(fn, c, i))]
    stmts = simple_statements(fn)
    mentioned = set()
    for s in stmts:
        mentioned.update(idents(s.text(fn.text)))
    ctx = {"positions": pos, "stmts": stmts, "in_lines": in_lines, "own_line": own_line,
           "window": window, "max_reads": max_reads, "mentioned": mentioned}
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


BIAS_RE = re.compile(r"^(\d+)=(-?\d+(?:\.\d+)?)$")


def parse_bias(spec: str | None) -> list[tuple[int, str]]:
    """`web=delta,...` (the CDX_BIAS grammar) as (web, delta) pairs."""
    out = []
    for f in [s.strip() for s in (spec or "").split(",") if s.strip()]:
        m = BIAS_RE.match(f)
        if not m:
            raise SystemExit(f"bias {f!r} is not web=delta")
        out.append((int(m.group(1)), m.group(2)))
    return out


def bias_oracle(forced: list[dict], bias) -> list[tuple[str, int, str]]:
    """The state each biased p1 web reached in the biased base, as oracle forces.

    A web with a coloured row wants that colour; one with only split or memory
    rows wants memory (`s`). The result feeds `oracle_targets` unchanged.
    """
    out = []
    for web, _ in bias:
        rows = [d for d in forced if d["phase"] == "p1" and d["web"] == web]
        if not rows:
            raise SystemExit(f"bias p1:w{web}: no decision for that web in the biased base")
        coloured = [d for d in rows if d["colour"] is not None]
        out.append(("p1", web, f"c{coloured[-1]['colour']}" if coloured else "s"))
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


def jaccard(a: set, b: set) -> float:
    return len(a & b) / len(a | b) if a | b else 0.0


def oracle_status(cands: list[dict], targets: list[dict]) -> tuple[int, list[dict]]:
    """How many oracle forces the (unforced) decision rows reproduce, with the matches.

    Each forced row is found among the candidate's rows by expression and kind
    in either phase, taking the best overlap of block source lines (Jaccard,
    at least 0.3). A colour force's row must carry that colour. A split force
    reads as memory: every same-expression row over those lines must be
    uncoloured, and a row with no counterpart at all (the piece is never
    formed) counts as reproduced.
    """
    hits, detail = 0, []
    for tgt in targets:
        ok_all, matches = True, []
        for row in tgt["rows"]:
            same = [d for d in cands if d["expr"] == row["expr"] and d["kind"] == row["kind"]
                    and jaccard(set(d["lines"]), row["lines"]) >= 0.3]
            best = max(same, key=lambda d: jaccard(set(d["lines"]), row["lines"]), default=None)
            if tgt["want"] == "s":
                # memory: no live piece of that value over those lines holds a register
                ok = all(d["colour"] is None for d in same)
                ok_all &= ok
                matches.append({"match": [f"{d['phase']}:w{d['web']}" for d in same],
                                "colour": [d["colour"] for d in same], "ok": ok})
                continue
            if best is None:
                ok_all = False
                matches.append({"match": None})
                continue
            score = jaccard(set(best["lines"]), row["lines"])
            ok = best["colour"] == int(tgt["want"][1:])
            ok_all &= ok
            matches.append({"match": f"{best['phase']}:w{best['web']}", "jaccard": round(score, 2),
                            "decision": best["decision"], "colour": best["colour"], "ok": ok})
        hits += ok_all
        detail.append({"spec": tgt["spec"], "ok": ok_all, "rows": matches})
    return hits, detail


def oracle_label(hits: int, n: int) -> str:
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
            row["oracle"] = oracle_label(hits, len(targets))
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


def parse_lines(spec: str | None) -> set[int]:
    if not spec:
        return set()
    lo, _, hi = spec.partition("-")
    return set(range(int(lo), int(hi or lo) + 1))


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--proc", type=int, required=True, help="procedure ordinal (as web_report prints)")
    ap.add_argument("--oracle", default="", help="CDX_FORCE spec the source must reproduce")
    ap.add_argument("--bias", help="CDX_BIAS spec (web=delta,...): reproduce the order it imposes")
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

    oracle = parse_oracle(ns.oracle)
    bias = parse_bias(ns.bias)
    if not oracle and not bias:
        raise SystemExit("give --oracle, --bias or both")
    levers = [x.strip() for x in ns.levers.split(",") if x.strip()]
    unknown = [x for x in levers if x not in LEVERS]
    if unknown:
        raise SystemExit(f"unknown lever(s) {unknown}; known: {', '.join(LEVERS)}")
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
    for label, kw in (("stock", {}), ("unforced", {"instrumented": True}),
                      ("forced", {"instrumented": True,
                                  "force": ",".join(f"{p}:w{w}={c}" for p, w, c in oracle) or None,
                                  "bias": ",".join(f"{w}={d}" for w, d in bias) or None})):
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
    if bias:
        oracle = oracle + [o for o in bias_oracle(forced_decs, bias) if o[:2] not in
                           {(p, w) for p, w, _ in oracle}]
        specs = specs + tuple(f"bias:{w}={d}" for w, d in bias)
    targets = oracle_targets(forced_decs, oracle)
    base_score = run.scorer.score(bdir / "stock.o")
    forced_score = run.scorer.score(bdir / "forced.o")
    base_decs, _ = decisions((bdir / "unforced.log").read_text(errors="replace"), ns.proc)
    bhits, bdetail = oracle_status(base_decs, targets)
    base = {"size": run.size(bdir / "stock.o"), "bytes": base_bytes,
            "forced_bytes": run.func_bytes(bdir / "forced.o"),
            "masked": base_score["masked"], "residual": base_score["residual"],
            "buckets": [base_score["aligned_exact"], base_score["aligned_register_naming"],
                        base_score["aligned_immediate_only"], base_score["aligned_really_different"]],
            "oracle": oracle_label(bhits, len(targets)), "oracle_detail": bdetail}
    print(f"{ns.symbol} proc {ns.proc}: base {base['masked']} masked, residual {base['residual']} "
          f"at {base['size'] - run.scorer.target_size:+d}; oracle {','.join(specs)} accepted, "
          f"forced {forced_score['masked']} masked, residual {forced_score['residual']} at "
          f"{forced_score['delta']:+d}; base oracle state: {base['oracle']}")
    for t in targets:
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
          f"{'naming':>6} {'imm':>4} {'diff':>4} {'oracle':>6} {'sem':>5}  edit")
    for r in [r for r in scored if not r.get("inert")][: ns.top]:
        b = r["buckets"]
        tag = r.get("oracle", "-") + ("=F" if r.get("equals_forced") else "")
        print(f"{r['index']:>5} {r['lever']:<14} {r['line']:>5} {r['masked']:>6} {r['delta']:>+6} "
              f"{r['residual']:>5} {b[1]:>6} {b[2]:>4} {b[3]:>4} {tag:>6} {r['semantics']:>5}  "
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
    if ns.json:
        pathlib.Path(ns.json).write_text(json.dumps({
            "symbol": ns.symbol, "proc": ns.proc, "oracle": list(specs),
            "base": {k: v for k, v in base.items() if k not in ("bytes", "forced_bytes")},
            "forced": {"masked": forced_score["masked"], "residual": forced_score["residual"],
                       "delta": forced_score["delta"]},
            "targets": [{**t, "rows": [{**r, "lines": sorted(r["lines"])} for r in t["rows"]]}
                        for t in targets],
            "cells": results}, indent=1, default=str))
    return 0 if exact else 1


if __name__ == "__main__":
    sys.exit(main())
