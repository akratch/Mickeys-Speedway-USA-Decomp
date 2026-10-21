#!/usr/bin/env python3
"""Turn a forward-substitution KEEP counterfactual into a mechanical source search.

    tools/keep_search.py <symbol> --candidate F --def LINE[,LINE] [--want kept]
                         [--also-oracle 'p1:w127=s,...' --proc N] [--known EXPR,...]
                         [--classes a,b] [--jobs 3] [--top N] [--json PATH] [--scratch DIR]

WHY. `DKWB_SUBST_KEEP=<lines>` prices what the code looks like when uopt keeps a
definition instead of forwarding its right-hand side into every read (a kept
definition keeps its stack home; func_80054B3C's four late stack words are
exactly that counterfactual). The force says what the target did; it does not
say which source spelling makes stock uopt decide it. This tool does the
second half: for each chosen definition it prints the whole reaching/read
picture the trace gives, generates one source variant per KEEP-predicate class
that could flip a read's verdict, measures every variant with the stock
compiler and the substitution trace, and ranks by the kept state first and
bytes second. Pairs of single-definition winners are combined automatically.

THE PICTURE (section 1 of the output, per definition): the variable's trace
id and name, the RHS shape (operands, whether it loads, operator kind), and
every read of the variable the definition can reach: LOCAL (same block) or
GLOBAL (another block), the verdict, uopt's reason, this definition's entry
in the candidate list, whether a call sits between the definition and the
read, and the operator the read feeds (a multiply operand, a compare, a
copy ...). The STORE row says whether the definition is KEPT today.

THE CLASSES (one per predicate that can refuse a substitution; each cell is
a same-line edit so every other line keeps its number):

  addr_taken     `addr = &x;` after the definition: not_vreg_local (the
                 oracle; marked diag, it is not an adoptable spelling)
  operand_redef  `op = op;` right after the definition, for each RHS
                 operand: operand_altered at the later reads (inert copy)
  operand_cast   `op = (T) op;` likewise (lever_sweep's noop_redef)
  dead_carrier   `c = op; x = RHS[op -> c]; c = 0;` through a dead local c
                 of op's type: the operand the RHS names is killed before
                 the first read while the real operand is untouched (the
                 shard's carrier form; both carrier stores are dead)
  zero_after     `op = 0;` after op's last read: a kill between the
                 definition and the later reads (def_not_available)
  rhs_ilod       an operand that is itself a field read is spelled as that
                 field read inside the RHS (rhs_has_ilod / has_ilod)
  cast_def       `x = (T) (RHS);`
  hoist          the definition moved before the enclosing if/loop statement
                 (block placement: not_entry_value / cross-block reads)
  sink           the definition moved to just before its first read's
                 statement when that is another block
  known_region   `if (EXPR) { x = RHS; }` for an expression known true there
                 (--known, or an int local set to one literal before the
                 line and never reassigned before it): two reaching paths
  multi_def      `if (EXPR) { x = RHS; } else { x = RHS; }`: two reaching
                 definitions with the same value
  dup_in_arms    the definition copied into the tail of both arms of the
                 if/else statement right before it, original dropped

Semantics: every class but addr_taken (diag), known_region/multi_def/hoist
of a loading RHS (check) preserves the value; the column says which.

MEASUREMENT. Stock compile through the tree's own recipe (`fast_score`), size
delta, masked words and the aligner's buckets; an instrumented compile with
`DKWB_SUBST_TRACE` (identity gate against the stock bytes) read for the STORE
row of the variable at the definition's (new) line: `kept` is the goal
state. `--also-oracle` adds `CDX_FORCE` targets: the base is compiled with the
force AND `DKWB_SUBST_KEEP` of the chosen lines (so the forced webs exist),
and each cell's unforced records are checked with lever_sweep's oracle reader
(expression + line overlap, never web numbers).

RANK: kept state (all chosen definitions kept, oracle reproduced when asked),
then aligned residual, then size delta, then masked words. `--want forwarded`
inverts the goal for a definition that is kept today and should not be.
"""
from __future__ import annotations

import argparse
import collections
import dataclasses
import hashlib
import itertools
import json
import os
import tempfile
import pathlib
import re
import subprocess
import sys
import threading
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import fast_score  # noqa: E402
import lever_sweep as ls  # noqa: E402
import subst_report as sr  # noqa: E402

ROOT = fast_score.ROOT
CALL_RE = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
FIELD_READ_RE = re.compile(r"^\s*(?:\(\s*[\w\s\*]+\)\s*)?(\*?[A-Za-z_]\w*(?:\s*(?:->|\.)\s*[A-Za-z_]\w*)+|\*\s*[A-Za-z_]\w*)\s*$")
ZERO = {"float": "0.0f", "int": "0", "pointer": "0"}
CLASSES = ("addr_taken", "operand_redef", "operand_cast", "dead_carrier", "zero_after", "rhs_ilod",
           "cast_def", "hoist", "sink", "known_region", "multi_def", "dup_in_arms")


# ---------------------------------------------------------------- the picture

def def_text(line: str) -> tuple[str, str] | None:
    """(name, rhs) of a plain `name = rhs;` definition on one source line."""
    m = re.match(r"^\s*([A-Za-z_]\w*)\s*=(?!=)\s*(.*?)\s*;\s*(?:/[/*].*)?$", ls.strip_comments(line))
    return (m.group(1), m.group(2)) if m else None


def rhs_shape(rhs: str) -> dict:
    """Operands, whether the RHS loads memory, and the kind of its top operator."""
    body = ls.strip_comments(rhs)
    tight = re.sub(r"\s+", "", body)
    ilod = bool(re.search(r"->|[\w)\]]\.[A-Za-z_]|\[|(?<![\w)\]])\*[A-Za-z_(]", tight))
    ops = [c for c in re.findall(r"[-+*/%<>=!&|^]+", re.sub(r"\(\s*[\w\s\*]+\)", "", body))]
    kind = ("call" if CALL_RE.search(body) else "op" if ops else "var" if ls.idents(body) else "const")
    return {"vars": sorted(set(ls.idents(body))), "ilod": ilod, "kind": kind, "ops": ops,
            "literal": bool(re.search(r"\d", body)) and not ls.idents(body)}


def parent_op(line: str, name: str) -> str:
    """The operator the first read of `name` on the line feeds ('mul', 'sub', 'cmp', 'copy', 'arg', ...)."""
    s = ls.strip_comments(line)
    for m in re.finditer(rf"\b{re.escape(name)}\b", s):
        before = s[:m.start()].rstrip()
        after = s[m.end():].lstrip()
        if before.endswith((".", "->")):
            continue
        if re.match(r"^\s*=(?!=)", after) and not before.endswith(("+", "-", "*", "/")):
            return "def"  # the variable is assigned here, not read
        left = before[-2:] if before[-2:] in ("->", "<=", ">=", "==", "!=") else before[-1:]
        right = after[:2] if after[:2] in ("<=", ">=", "==", "!=") else after[:1]
        # look through grouping parentheses on either side
        while left == "(":
            before = before[:-1].rstrip()
            left = before[-1:]
        while right == ")":
            after = after[1:].lstrip()
            right = after[:1]
        for side in (left, right):
            if side == "*":
                return "mul"
            if side == "/":
                return "div"
            if side == "-":
                return "sub"
            if side == "+":
                return "add"
            if side in ("<", ">", "<=", ">=", "==", "!="):
                return "cmp"
        if left == "=" and right == ";":
            return "copy"
        if left in ("(", ",") or right in (",", ")"):
            return "arg"
        return (left + "|" + right).strip() or "?"
    return "n/a"


def calls_between(source: list[str], lo: int, hi: int, name: str) -> list[str]:
    """Calls on lines lo+1..hi-1, plus any on line hi textually before the read of `name`."""
    found = []
    for n in range(lo + 1, hi):
        found += [m.group(1) for m in CALL_RE.finditer(ls.strip_comments(source[n - 1]))
                  if m.group(1) not in ls.KEYWORDS]
    if 0 < hi <= len(source) and hi != lo:
        s = ls.strip_comments(source[hi - 1])
        m = re.search(rf"\b{re.escape(name)}\b", s)
        head = s[:m.start()] if m else ""
        found += [c.group(1) for c in CALL_RE.finditer(head)
                  if c.group(1) not in ls.KEYWORDS and c.end() < len(head)
                  and head[c.end():].count(")") > head[c.end():].count("(")]
    return found


def var_of_def(rows: list[dict], line: int) -> str | None:
    """The trace variable id of the definition on `line` (its own rows, else a candidate mention)."""
    for r in rows:
        if r.get("def_line") == line:
            return r.get("var")
    for r in rows:
        if any(l == line for l, _ in sr.parse_cands(r.get("cands", ""))):
            return r.get("var")
    return None


def picture(rows: list[dict], source: list[str], line: int) -> dict:
    """Everything the trace and the source say about the definition on `line`."""
    var = var_of_def(rows, line)
    dt = def_text(source[line - 1]) if 0 < line <= len(source) else None
    name, rhs = dt if dt else ("?", "")
    pic = {"line": line, "var": var, "name": name, "rhs": rhs, "shape": rhs_shape(rhs),
           "reads": [], "store": None, "kept": None}
    if var is None:
        return pic
    seen = set()
    for r in rows:
        if r.get("var") != var:
            continue
        if r["kind"] == "STORE":
            if r["def_line"] == line:
                pic["store"] = {"decision": r["decision"], "reason": r["reason"]}
                pic["kept"] = r["decision"] == "kept"
            continue
        cands = sr.parse_cands(r.get("cands", ""))
        mine = r.get("def_line") == line or any(l == line for l, _ in cands)
        if not mine:
            continue
        key = (r["kind"], r["use_line"])
        if key in seen:
            continue
        seen.add(key)
        verdict = r["reason"] if r["kind"] == "LOCAL" else next(
            (v for l, v in cands if l == line), r["reason"])
        pic["reads"].append({
            "use_line": r["use_line"], "scope": ("earlier" if r["use_line"] < line else "same-block" if r["kind"] == "LOCAL" else "other-block"),
            "decision": r["decision"], "reason": r["reason"], "this_def": verdict,
            "calls_between": calls_between(source, line, r["use_line"], name),
            "parent_op": parent_op(source[r["use_line"] - 1], name) if 0 < r["use_line"] <= len(source) else "?",
            "text": source[r["use_line"] - 1].strip()[:60] if 0 < r["use_line"] <= len(source) else "",
            "vreg": r.get("vreg"),
        })
    pic["reads"].sort(key=lambda d: d["use_line"])
    return pic


def render_picture(pic: dict) -> str:
    sh = pic["shape"]
    out = [f"def L{pic['line']} {pic['name']} = {pic['rhs']}   var {pic['var'] or '?'}  "
           f"[rhs kind={sh['kind']} vars={','.join(sh['vars']) or '-'} ilod={int(sh['ilod'])}]  "
           f"STORE {pic['store']['decision'] + ' ' + pic['store']['reason'] if pic['store'] else 'no row'}"
           f"  -> {'KEPT' if pic['kept'] else 'forwarded' if pic['kept'] is not None else 'unknown'}"]
    for r in pic["reads"]:
        calls = ",".join(r["calls_between"]) or "-"
        out.append(f"   read L{r['use_line']:<5d} {r['scope']:<11s} {r['decision']:<11s} "
                   f"{r['this_def']:<22s} call={calls:<10s} op={r['parent_op']:<5s} {r['text']}")
    if not pic["reads"]:
        out.append("   (no read of this definition in the trace)")
    return "\n".join(out)


# ---------------------------------------------------------------- source edits

@dataclasses.dataclass
class Cell:
    cls: str
    def_line: int
    edit: str
    spans: list            # [(start, end, replacement)] on the base text, no newlines added
    def_lines: set         # where the definition now sits
    semantics: str = "safe"
    index: int = -1

    def key(self):
        return tuple(sorted((s, e, r) for s, e, r in self.spans))


def apply_spans(text: str, spans: list) -> str:
    for start, end, repl in sorted(spans, key=lambda s: -s[0]):
        text = text[:start] + repl + text[end:]
    return text


def disjoint(a: Cell, b: Cell) -> bool:
    for s0, e0, _ in a.spans:
        for s1, e1, _ in b.spans:
            if s0 < e1 and s1 < e0 and not (s0 == e0 == s1 == e1 and False):
                return False
            if s0 == s1 and e0 == e1:
                return False
    return True


def compose(cells: list[Cell]) -> Cell:
    spans = [s for c in cells for s in c.spans]
    return Cell("+".join(c.cls for c in cells), min(c.def_line for c in cells),
                " | ".join(c.edit for c in cells), spans,
                set().union(*(c.def_lines for c in cells)),
                "check" if any(c.semantics == "check" for c in cells) else
                "diag" if any(c.semantics == "diag" for c in cells) else "safe")


def def_node(fn: ls.Function, line: int, name: str) -> tuple[ls.Node, ls.Node, int] | None:
    """(statement, compound, index) of `name = ...;` on `line`."""
    for node in ls.walk(fn.body):
        if node.kind != "compound":
            continue
        for i, s in enumerate(node.children):
            if s.kind == "simple" and fn.line_of(s.start) == line:
                m = ls.ASSIGN_RE.match(ls.strip_comments(s.text(fn.text)))
                if m and m.group(1) == name:
                    return s, node, i
    return None


def type_of(fn: ls.Function, name: str) -> tuple[str, str] | None:
    info = fn.locals.get(name) or fn.params.get(name)
    if not info:
        return None
    return info["type"] + (" " + "*" * info["ptr"] if info.get("ptr") else ""), ls.category(info)


def assigned_between(fn: ls.Function, lo: int, hi: int, names: set) -> set:
    """Names of `names` assigned by a statement whose start offset lies in [lo, hi)."""
    hit = set()
    for s in ls.simple_statements(fn):
        if lo <= s.start < hi:
            m = ls.ASSIGN_RE.match(ls.strip_comments(s.text(fn.text)))
            if m and m.group(1) in names:
                hit.add(m.group(1))
            m2 = re.match(r"^\s*([A-Za-z_]\w*)\s*(?:\+\+|--|[-+*/]=)", s.text(fn.text))
            if m2 and m2.group(1) in names:
                hit.add(m2.group(1))
    return hit


def known_true(fn: ls.Function, line: int) -> list[str]:
    """`v == LIT` for int locals set to one literal before `line` and never otherwise assigned before it."""
    out = []
    for name, info in fn.locals.items():
        if ls.category(info) != "int":
            continue
        lits = set()
        ok = True
        for s in ls.simple_statements(fn):
            if fn.line_of(s.start) >= line:
                continue
            m = ls.ASSIGN_RE.match(ls.strip_comments(s.text(fn.text)))
            if m and m.group(1) == name:
                v = m.group(2).strip()
                if re.fullmatch(r"-?\d+", v):
                    lits.add(v)
                else:
                    ok = False
            elif name in ls.idents(s.text(fn.text)) and re.match(rf"\s*{name}\s*(\+\+|--|[-+*/|&^]=)", s.text(fn.text)):
                ok = False
        if ok and len(lits) == 1:
            out.append(f"{name} == {next(iter(lits))}")
    return out


def pointer_local(fn: ls.Function, base_type: str) -> str | None:
    for name, info in fn.locals.items():
        if info["type"] == base_type and info.get("ptr") == 1 and not info.get("array"):
            return name
    return None


def field_def_of(fn: ls.Function, op: str, before_line: int) -> str | None:
    """The field-read RHS of op's last `op = p->f;` before `before_line` if op is not reassigned after it."""
    last = None
    for s in ls.simple_statements(fn):
        if fn.line_of(s.start) >= before_line:
            break
        m = ls.ASSIGN_RE.match(ls.strip_comments(s.text(fn.text)))
        if m and m.group(1) == op:
            last = (s, m.group(2).strip())
    if not last:
        return None
    s, rhs = last
    if not FIELD_READ_RE.match(rhs):
        return None
    base = ls.idents(rhs)
    end = next((t.start for t in ls.simple_statements(fn) if fn.line_of(t.start) >= before_line), len(fn.text))
    if assigned_between(fn, s.end, end, set(base) | {op}):
        return None
    return rhs


def replace_var(expr: str, old: str, new: str, count: int = 0) -> str:
    """Replace reads of identifier `old` (not members) in `expr`; count=0 means all."""
    n = [0]

    def sub(m):
        before = expr[:m.start()].rstrip()
        if before.endswith((".", "->")):
            return m.group(0)
        if count and n[0] >= count:
            return m.group(0)
        n[0] += 1
        return new
    return re.sub(rf"\b{re.escape(old)}\b", sub, expr)


def stmt_bounds(fn: ls.Function, s: ls.Node) -> tuple[int, int]:
    """[start, end) of the statement including the whitespace before it on its line."""
    start = s.start
    while start > 0 and fn.text[start - 1] in " \t":
        start -= 1
    return start, s.end


def hoist_point(fn: ls.Function, comp: ls.Node) -> int | None:
    """Offset just before the statement that encloses `comp` in its parent compound."""
    anc = comp.parent
    while anc is not None and anc.kind != "compound":
        parent_comp = anc.parent
        if parent_comp is not None and parent_comp.kind == "compound":
            return ls.insertion_offset(fn, parent_comp, parent_comp.children.index(anc))
        anc = parent_comp
    return None


def generate(fn: ls.Function, pic: dict, classes: list[str], known: list[str], carriers: int = 2) -> list[Cell]:
    """Every cell of the asked classes for one definition."""
    line, name, rhs = pic["line"], pic["name"], pic["rhs"]
    found = def_node(fn, line, name)
    if not found or name == "?":
        return []
    stmt, comp, idx = found
    t = fn.text
    ty = type_of(fn, name)
    if not ty:
        return []
    tname, tcat = ty
    after = stmt.end  # insertion point right after the definition, same line
    cells: list[Cell] = []
    ops = [v for v in pic["shape"]["vars"] if v != name and (v in fn.locals or v in fn.params)]
    read_lines = [r["use_line"] for r in pic["reads"] if r["use_line"] > line]

    def add(cls, edit, spans, sem="safe", def_lines=None):
        cells.append(Cell(cls, line, edit, spans, def_lines or {line}, sem))

    if "addr_taken" in classes:
        p = pointer_local(fn, tname)
        if p:
            add("addr_taken", f"{p} = &{name};", [(after, after, f" {p} = &{name};")], "diag")
    for op in ops:
        oty = type_of(fn, op)
        if not oty:
            continue
        otname, ocat = oty
        if "operand_redef" in classes:
            add("operand_redef", f"{op} = {op};", [(after, after, f" {op} = {op};")])
        if "operand_cast" in classes:
            add("operand_cast", f"{op} = ({otname}) {op};", [(after, after, f" {op} = ({otname}) {op};")])
        if "dead_carrier" in classes and ocat in ZERO:
            used = 0
            for c, info in fn.locals.items():
                if carriers and used >= carriers:
                    break
                if c in (name, op) or ls.category(info) != ocat or info["type"] != otname or info.get("array"):
                    continue
                if not ls.dead_at(fn, comp, idx, c):
                    continue
                s0, s1 = stmt_bounds(fn, stmt)
                indent = t[s0:stmt.start]
                used += 1
                early = hoist_point(fn, comp)
                for count, tag in ((0, "all"), (1, "first")):
                    new_rhs = replace_var(rhs, op, c, count)
                    if new_rhs == rhs:
                        continue
                    body = f"{indent}{c} = {op}; {name} = {new_rhs}; {c} = {ZERO[ocat]};"
                    add("dead_carrier", f"{c} = {op}; {name} = {new_rhs}; {c} = 0 [{tag}]", [(s0, s1, body)])
                    # the shard's form: the snapshot taken in the block BEFORE the enclosing
                    # statement, so its own load is not forwarded into the definition
                    if early is not None and not assigned_between(fn, early, stmt.start, {op, c}) \
                            and c not in ls.idents(t[early:stmt.start]) and ls.dead_at(fn, comp, idx, c):
                        body2 = f"{indent}{name} = {new_rhs}; {c} = {ZERO[ocat]};"
                        add("dead_carrier", f"{c} = {op} before L{fn.line_of(early)}; {name} = {new_rhs}; {c} = 0 [{tag},early]",
                            [(early, early, f" {c} = {op};"), (s0, s1, body2)])
                    if count == 1 and rhs.count(op) < 2:
                        break
        if "zero_after" in classes and op in fn.locals and ocat in ZERO:
            last = None
            for s in ls.simple_statements(fn):
                if s.start >= stmt.start and op in ls.idents(s.text(t)):
                    last = s
            if last is not None:
                lline = fn.line_of(last.start)
                comp2 = last.parent
                while comp2 is not None and comp2.kind != "compound":
                    comp2 = comp2.parent
                if comp2 is not None and ls.dead_at(fn, comp2, comp2.children.index(last) + 1, op) \
                        and any(r > lline for r in read_lines):
                    add("zero_after", f"{op} = 0 after L{lline}", [(last.end, last.end, f" {op} = {ZERO[ocat]};")])
        if "rhs_ilod" in classes:
            fr = field_def_of(fn, op, line)
            if fr:
                new_rhs = replace_var(rhs, op, f"({fr})" if "->" not in fr or " " in fr else fr)
                m = re.search(r"=(?!=)", t[stmt.start:stmt.end])
                r0 = stmt.start + m.end()
                r1 = t.rfind(";", 0, stmt.end)
                add("rhs_ilod", f"{name} = {new_rhs};", [(r0, r1, " " + new_rhs)], "check")
    if "cast_def" in classes:
        m = re.search(r"=(?!=)", t[stmt.start:stmt.end])
        r0 = stmt.start + m.end()
        r1 = t.rfind(";", 0, stmt.end)
        add("cast_def", f"{name} = ({tname}) ({rhs});", [(r0, r1, f" ({tname}) ({rhs})")])
    s0, s1 = stmt_bounds(fn, stmt)
    body = t[stmt.start:stmt.end]
    if "hoist" in classes:
        anc = comp.parent
        while anc is not None and anc.kind != "compound":
            parent_comp = anc.parent
            if parent_comp is not None and parent_comp.kind == "compound":
                j = parent_comp.children.index(anc)
                off = ls.insertion_offset(fn, parent_comp, j)
                if not assigned_between(fn, off, stmt.start, set(ops) | {name}) \
                        and name not in ls.idents(t[off:stmt.start]) and not CALL_RE.search(rhs):
                    sem = "check" if pic["shape"]["ilod"] else "safe"
                    add("hoist", f"{body} before L{fn.line_of(off)}",
                        [(s0, s1, ""), (off, off, " " + body)], sem, {fn.line_of(off)})
                break
            anc = parent_comp
    if "sink" in classes and read_lines:
        first = min(read_lines)
        tgt = next((s for s in ls.simple_statements(fn) if fn.line_of(s.start) == first and s.start > stmt.end), None)
        if tgt is not None and tgt.parent is not comp \
                and not assigned_between(fn, stmt.end, tgt.start, set(ops) | {name}) \
                and name not in ls.idents(t[stmt.end:tgt.start]):
            p0, _ = stmt_bounds(fn, tgt)
            add("sink", f"{body} before L{first}", [(s0, s1, ""), (tgt.start, tgt.start, body + " ")],
                "check" if pic["shape"]["ilod"] else "safe", {first})
    exprs = list(known) + [k for k in known_true(fn, line) if k not in known]
    for ex in exprs:
        if "known_region" in classes:
            add("known_region", f"if ({ex}) {{ {body} }}", [(stmt.start, stmt.end, f"if ({ex}) {{ {body} }}")], "check")
        if "multi_def" in classes:
            add("multi_def", f"if ({ex}) {{ {body} }} else {{ {body} }}",
                [(stmt.start, stmt.end, f"if ({ex}) {{ {body} }} else {{ {body} }}")], "check")
    if "dup_in_arms" in classes and idx > 0:
        prev = comp.children[idx - 1]
        if prev.kind == "if" and len(prev.children) == 2 and all(c.kind == "compound" for c in prev.children):
            arms = prev.children
            if not any(assigned_between(fn, a.start, a.end, set(ops) | {name}) for a in arms) \
                    and not any(ls.escapes(fn, a) for a in arms) and "return" not in prev.text(t):
                spans = [(s0, s1, "")]
                lines = set()
                for a in arms:
                    close = a.end - 1
                    spans.append((close, close, body + " "))
                    lines.add(fn.line_of(close))
                add("dup_in_arms", f"{body} in both arms of the if before it", spans, "safe", lines)
    out, seen = [], set()
    for c in cells:
        if c.key() in seen:
            continue
        seen.add(c.key())
        out.append(c)
    return out


# ---------------------------------------------------------------- measuring

class Runner:
    def __init__(self, symbol: str, source: str, scratch: pathlib.Path, proc: int | None):
        import align_symbol
        self.symbol, self.source, self.scratch, self.proc = symbol, source, scratch, proc
        self.args = fast_score.configured_cc_args(source)
        self.scorer = align_symbol.AlignedScorer(symbol, scratch)
        self.env = {k: v for k, v in os.environ.items() if not k.startswith(("CDX_", "DKWB_"))}
        self.inst = fast_score.compiler_for(True)

    def compile(self, src: pathlib.Path, obj: pathlib.Path, *, trace: pathlib.Path | None = None,
                keep: str | None = None, force: str | None = None, log: pathlib.Path | None = None) -> str | None:
        instrumented = bool(trace or keep or force or (log is not None and self.proc is not None))
        args = fast_score.use_compiler(fast_score.rewrite_io(self.args, src, self.source, obj),
                                       self.inst if instrumented else None)
        env = dict(self.env)
        if trace:
            env.update(DKWB_SUBST_TRACE=str(trace), DKWB_SUBST_PROC=self.symbol)
        if keep:
            env["DKWB_SUBST_KEEP"] = keep
        if log is not None and self.proc is not None:
            env.update(CDX_LOG="1", CDX_PROC=str(self.proc), CDX_DETAIL_WEB="all",
                       CDX_WEBREPORT="1", CDX_OUT=str(log))
        if force:
            env["CDX_FORCE"] = force
        r = subprocess.run(args, env=env, cwd=ROOT, capture_output=True, text=True, timeout=900)
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


def kept_state(rows: list[dict], var: str | None, def_lines: set) -> tuple[bool | None, dict]:
    """(every STORE row of var on def_lines is kept, the rows) -- None when no row is found."""
    stores = {r["def_line"]: r for r in rows if r["kind"] == "STORE" and r.get("var") == var
              and r["def_line"] in def_lines}
    if not stores:
        return None, {}
    return all(r["decision"] == "kept" for r in stores.values()), {
        l: f"{r['decision']} {r['reason']}" for l, r in stores.items()}


def goal_met(state: bool | None, want: str) -> bool:
    return state is not None and state == (want == "kept")


def read_tally(rows: list[dict], var: str | None, def_lines: set) -> str:
    c = collections.Counter()
    for r in rows:
        if r["kind"] in ("LOCAL", "GLOBAL") and r.get("var") == var:
            cands = sr.parse_cands(r.get("cands", ""))
            if r.get("def_line") in def_lines or any(l in def_lines for l, _ in cands):
                c[r["decision"]] += 1
    return ",".join(f"{k} {v}" for k, v in sorted(c.items())) or "-"


def measure(run: Runner, cell: Cell, base_text: str, pics: dict, want: str, targets: list,
            base: dict, lock: threading.Lock, counter: list) -> dict:
    d = run.scratch / f"cell{cell.index:04d}"
    d.mkdir(exist_ok=True)
    src = d / ("candidate" + pathlib.Path(run.source).suffix)
    src.write_text(apply_spans(base_text, cell.spans))
    row = {"index": cell.index, "class": cell.cls, "def_line": cell.def_line, "edit": cell.edit,
           "semantics": cell.semantics, "def_lines": sorted(cell.def_lines)}
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
        fb = run.func_bytes(obj)
        row["inert"] = fb == base["bytes"]
        row["sha"] = hashlib.sha1(fb or b"").hexdigest()[:12]
        score = run.scorer.score(obj)
        row["masked"], row["residual"] = score["masked"], score["residual"]
        row["buckets"] = [score["aligned_exact"], score["aligned_register_naming"],
                          score["aligned_immediate_only"], score["aligned_really_different"]]
        trace, iobj, log = d / "subst.trace", d / "instrumented.o", d / "allocator.log"
        err = run.compile(src, iobj, trace=trace, log=log if targets else None)
        if err:
            row["error"] = "instrumented: " + err
            return row
        if run.func_bytes(iobj) != fb:
            row["gate"] = True
        rows = sr.parse_trace(trace.read_text() if trace.is_file() else "", run.symbol)
        states = {}
        for line, pic in pics.items():
            lines = cell.def_lines if line == cell.def_line else {line}
            st, stores = kept_state(rows, pic["var"], lines)
            states[line] = {"kept": st, "stores": stores, "reads": read_tally(rows, pic["var"], lines)}
        row["states"] = states
        row["goal"] = all(goal_met(s["kept"], want) for s in states.values())
        row["own"] = goal_met(states.get(cell.def_line, {}).get("kept"), want) and not row["inert"]
        if targets:
            decs, _ = ls.decisions(log.read_text(errors="replace"), run.proc)
            hits, detail = ls.oracle_status(decs, targets)
            row["oracle"] = ls.oracle_label(hits, len(targets), sum(x["ambiguous"] for x in detail))
            row["oracle_detail"] = detail
        else:
            row["oracle"] = "-"
        return row
    finally:
        for f in d.glob("*"):
            if f.name != src.name:
                f.unlink(missing_ok=True)
        with lock:
            counter[0] += 1
            if counter[0] % 10 == 0:
                print(f"  {counter[0]} cells measured", flush=True)


def rank_key(r: dict):
    if "error" in r or "residual" not in r:
        return (2, 0, 0, 0, 0)
    oracle_ok = r.get("oracle") in ("-", "yes")
    exact = r["masked"] == 0 and r.get("delta") == 0
    return (0 if r.get("goal") else 1, not oracle_ok, not exact, r["residual"],
            abs(r.get("delta", 0)), r["masked"])


def own_key(r: dict):
    """Per-definition table: the definition's own goal first, then rank_key."""
    return (not r.get("own"),) + rank_key(r)


def winners(results: list[dict], cells: dict[int, Cell], def_line: int, want: str, limit: int) -> list[Cell]:
    """Cells of one definition whose own definition reached the goal, best bytes first.

    One cell per distinct object: the carriers of one operand all compile to the
    same bytes, and pairing each of them would measure the same combination again.
    """
    out, seen = [], set()
    for r in sorted(results, key=own_key):
        if "error" in r or r.get("inert") or r["def_line"] != def_line or (r.get("sha") and r["sha"] in seen):
            continue
        if r.get("sha"):
            seen.add(r["sha"])
        st = r.get("states", {}).get(def_line, {}).get("kept")
        # the oracle is not required here: against a several-web oracle a
        # single-definition cell can only ever reproduce its own web's state
        if goal_met(st, want):
            out.append(cells[r["index"]])
        if len(out) >= limit:
            break
    return out


def combinations(per_def: dict[int, list[Cell]]) -> list[Cell]:
    """Pairs of single-definition winners of different definitions with disjoint edits,
    plus the all-definition product of the best winners when more than two definitions were asked."""
    out, seen = [], set()
    lines = sorted(per_def)
    for a, b in itertools.combinations(lines, 2):
        for ca in per_def[a]:
            for cb in per_def[b]:
                if disjoint(ca, cb):
                    c = compose([ca, cb])
                    if c.key() not in seen:
                        seen.add(c.key())
                        out.append(c)
    if len(lines) > 2:
        for choice in itertools.product(*(per_def[l][:2] for l in lines)):
            if all(disjoint(x, y) for x, y in itertools.combinations(choice, 2)):
                c = compose(list(choice))
                if c.key() not in seen:
                    seen.add(c.key())
                    out.append(c)
    return out


def render_row(r: dict, pics: dict) -> str:
    if "error" in r:
        return f"{r['index']:>4} {r['class']:<16} L{r['def_line']:<5d} error: {r['error'][:60]}"
    st = " ".join(f"L{l}:{'K' if s['kept'] else 'f' if s['kept'] is not None else '?'}"
                  for l, s in sorted(r["states"].items()))
    b = r["buckets"]
    flags = ("inert " if r.get("inert") else "") + ("gate " if r.get("gate") else "")
    return (f"{r['index']:>4} {r['class']:<16} L{r['def_line']:<5d} {r['masked']:>6} {r['delta']:>+5} "
            f"{r['residual']:>5} {b[1]:>4} {b[2]:>4} {b[3]:>4} {'GOAL' if r.get('goal') else '-':>4} "
            f"{r.get('oracle', '-'):>6} {r['semantics']:>5} {st:<18} {flags}{r['edit'][:60]}")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--candidate", required=True, help="candidate TU (a scratch copy, never the tracked file)")
    ap.add_argument("--def", dest="defs", required=True, help="definition source line(s), comma separated")
    ap.add_argument("--want", choices=("kept", "forwarded"), default="kept")
    ap.add_argument("--also-oracle", default="", help="CDX_FORCE spec the unforced records must reproduce")
    ap.add_argument("--proc", type=int, help="procedure ordinal (needed with --also-oracle)")
    ap.add_argument("--known", default="", help="expressions known true at the definitions, comma separated")
    ap.add_argument("--classes", default=",".join(CLASSES), help="comma list (default all): " + ", ".join(CLASSES))
    ap.add_argument("--jobs", type=int, default=3)
    ap.add_argument("--top", type=int, default=30)
    ap.add_argument("--combine", type=int, default=3, help="winners per definition fed to the pair step (0: off)")
    ap.add_argument("--carriers", type=int, default=2, help="dead locals tried per operand by dead_carrier (0: all)")
    ap.add_argument("--picture-only", action="store_true", help="print the picture and the cell list, compile nothing")
    ap.add_argument("--scratch", help="work directory (default keep-search-<pid> under the system temp directory)")
    ap.add_argument("--json", metavar="PATH")
    ns = ap.parse_args(argv)

    ls.use_root(ROOT)
    classes = [c.strip() for c in ns.classes.split(",") if c.strip()]
    unknown = [c for c in classes if c not in CLASSES]
    if unknown:
        raise SystemExit(f"unknown class(es) {unknown}; known: {', '.join(CLASSES)}")
    def_lines = [int(x) for x in ns.defs.split(",") if x.strip()]
    known = [k.strip() for k in ns.known.split(",") if k.strip()]
    oracle = ls.parse_oracle(ns.also_oracle)
    if oracle and ns.proc is None:
        raise SystemExit("--also-oracle needs --proc N (the procedure ordinal web_report prints)")
    candidate = pathlib.Path(ns.candidate).resolve()
    if not candidate.is_file():
        raise SystemExit(f"no such candidate: {candidate}")
    source = fast_score.tracked_source_for(ns.symbol)
    if candidate == (ROOT / source).resolve():
        raise SystemExit("--candidate must be a scratch copy, not the tracked source")
    base_text = candidate.read_text()
    fn = ls.find_function(base_text, ns.symbol)
    scratch = pathlib.Path(ns.scratch or os.path.join(tempfile.gettempdir(), f"keep-search-{os.getpid()}")).resolve()
    if ROOT.resolve() in scratch.parents or scratch == ROOT.resolve():
        raise SystemExit("--scratch must be outside the repository")
    scratch.mkdir(parents=True, exist_ok=True)
    (scratch / "base").mkdir(exist_ok=True)
    bsrc = scratch / "base" / ("candidate" + pathlib.Path(source).suffix)
    bsrc.write_text(base_text)

    run = Runner(ns.symbol, source, scratch, ns.proc)
    err = run.compile(bsrc, scratch / "base" / "stock.o")
    if err:
        raise SystemExit(f"base stock compile failed: {err}")
    trace = scratch / "base" / "subst.trace"
    err = run.compile(bsrc, scratch / "base" / "unforced.o", trace=trace,
                      log=scratch / "base" / "unforced.log" if oracle else None)
    if err:
        raise SystemExit(f"base instrumented compile failed: {err}")
    base_bytes = run.func_bytes(scratch / "base" / "stock.o")
    if run.func_bytes(scratch / "base" / "unforced.o") != base_bytes:
        raise SystemExit("IDENTITY GATE FAILED: instrumented .text differs from stock on the base")
    rows = sr.parse_trace(trace.read_text() if trace.is_file() else "", ns.symbol)
    if not rows:
        raise SystemExit("the substitution trace is empty for this procedure (see subst_report.py)")
    src_lines = base_text.splitlines()
    pics = {l: picture(rows, src_lines, l) for l in def_lines}
    base_score = run.scorer.score(scratch / "base" / "stock.o")
    print(f"{ns.symbol}: base {base_score['masked']} masked, residual {base_score['residual']} at "
          f"{base_score['delta']:+d}; want {ns.want}")
    for l in def_lines:
        print(render_picture(pics[l]))
        if pics[l]["var"] is None:
            print(f"   WARNING: no trace row names a definition on L{l}; is it a plain `x = ...;` line?")

    targets = []
    if oracle:
        keep = ",".join(str(l) for l in def_lines)
        flog = scratch / "base" / "forced.log"
        err = run.compile(bsrc, scratch / "base" / "forced.o", keep=keep, log=flog,
                          force=",".join(f"{p}:w{w}={c}" for p, w, c in oracle))
        if err:
            raise SystemExit(f"forced base compile failed: {err}")
        import force_lattice as fl
        refused = fl.force_acceptance(flog.read_text(errors="replace"), ns.proc,
                                      tuple(f"{p}:w{w}={c}" for p, w, c in oracle))
        if refused:
            raise SystemExit(f"oracle not accepted on the KEEP-forced base: {refused}")
        forced_decs, _ = ls.decisions(flog.read_text(errors="replace"), ns.proc)
        targets = ls.oracle_targets(forced_decs, oracle)
        fscore = run.scorer.score(scratch / "base" / "forced.o")
        print(f"oracle {ns.also_oracle} with DKWB_SUBST_KEEP={keep}: forced {fscore['masked']} masked, "
              f"residual {fscore['residual']} at {fscore['delta']:+d}")

    cells: list[Cell] = []
    for l in def_lines:
        cells += generate(fn, pics[l], classes, known, ns.carriers)
    for i, c in enumerate(cells):
        c.index = i
    counts = collections.Counter(c.cls for c in cells)
    print(f"cells: {len(cells)} ({', '.join(f'{k} {v}' for k, v in counts.items())})")
    if ns.picture_only:
        for c in cells:
            print(f"  {c.index:>4} {c.cls:<16} L{c.def_line} {c.semantics:<5} {c.edit[:90]}")
        return 0

    base = {"bytes": base_bytes}
    lock, counter = threading.Lock(), [0]
    by_index = {c.index: c for c in cells}

    def run_all(batch: list[Cell]) -> list[dict]:
        with ThreadPoolExecutor(max_workers=max(1, min(ns.jobs, 3))) as pool:
            return list(pool.map(lambda c: measure(run, c, base_text, pics, ns.want, targets, base, lock, counter),
                                 batch))

    results = run_all(cells)
    combined: list[dict] = []
    if ns.combine and len(def_lines) > 1:
        per_def = {l: winners(results, by_index, l, ns.want, ns.combine) for l in def_lines}
        combos = combinations({l: w for l, w in per_def.items() if w})
        for c in combos:
            c.index = len(by_index)
            by_index[c.index] = c
        print(f"combining: " + ", ".join(f"L{l} {len(w)} winner(s)" for l, w in per_def.items())
              + f" -> {len(combos)} combination cell(s)")
        if combos:
            combined = run_all(combos)
    everything = sorted(results + combined, key=rank_key)
    header = (f"{'cell':>4} {'class':<16} {'def':<6} {'masked':>6} {'delta':>5} {'resid':>5} {'nam':>4} "
              f"{'imm':>4} {'diff':>4} {'goal':>4} {'oracle':>6} {'sem':>5} {'state':<18} edit")
    print()
    print("per-definition cells, best first:")
    print(header)
    for r in sorted(results, key=own_key)[: ns.top]:
        print(render_row(r, pics))
    if combined:
        print()
        print("combinations:")
        print(header)
        for r in sorted(combined, key=rank_key)[: ns.top]:
            print(render_row(r, pics))
    print()
    for l in def_lines:
        best_of: dict[str, dict] = {}
        for r in sorted(results, key=own_key):
            if r["def_line"] == l and r.get("own") and r["class"] not in best_of:
                best_of[r["class"]] = r
        print(f"L{l} {pics[l]['name']}: classes reaching {ns.want}: " + (", ".join(
            f"{c} ({r['masked']}/{r['delta']:+d}, {r['semantics']})" for c, r in best_of.items()) or "none"))
    goal = [r for r in everything if r.get("goal") and "error" not in r]
    exact = [r for r in goal if r["masked"] == 0 and r.get("delta") == 0]
    best = next((r for r in everything if "error" not in r and not r.get("inert")), None)
    print(f"SUMMARY keep_search {ns.symbol} defs {ns.defs} want {ns.want}: {len(cells)} cells + "
          f"{len(combined)} combinations, goal reached {len(goal)}, exact {len(exact)}; best "
          + (f"{best['class']} L{best['def_line']} masked {best['masked']} delta {best['delta']:+d} "
             f"resid {best['residual']} goal {'yes' if best.get('goal') else 'no'}" if best else "none"))
    keepdir = scratch / "best"
    keepdir.mkdir(exist_ok=True)
    for r in everything[:5]:
        s = scratch / f"cell{r['index']:04d}" / ("candidate" + pathlib.Path(source).suffix)
        if s.exists():
            (keepdir / s.name.replace("candidate", f"cell{r['index']:04d}")).write_text(s.read_text())
    print(f"best cells' sources: {keepdir}")
    if ns.json:
        pathlib.Path(ns.json).write_text(json.dumps({
            "symbol": ns.symbol, "candidate": str(candidate), "defs": def_lines, "want": ns.want,
            "oracle": ns.also_oracle, "base": {k: base_score[k] for k in ("masked", "residual", "delta")},
            "pictures": pics, "cells": results, "combinations": combined}, indent=1, default=str))
    return 0 if goal else 1


if __name__ == "__main__":
    sys.exit(main())
