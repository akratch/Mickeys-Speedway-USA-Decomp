#!/usr/bin/env python3
"""Run a candidate and a reference source on synthetic inputs and compare what they do.

    tools/behaviour_check.py <candidate.c> --driver DRIVER.c --reference SRC.c
    tools/behaviour_check.py <candidate.c> --driver DRIVER.c --digest   # print the candidate's trace only
    tools/behaviour_check.py <candidate.c> --driver DRIVER.c --coverage func_800517E0

A score says how close a candidate is to the target's bytes; it says nothing
about whether the candidate still means what the source means. Permuter and
rename-scan cells change C freely, and a cell that writes into a live local
(the row counter of overlay64GenerateTexture, twice on 2026-10-15) can score
better than the honest source while computing something else. This tool is
the semantic gate for such cells.

Both translation units are compiled for the HOST with the system C compiler,
`-DNON_MATCHING` (so the candidate C is what runs, never a GLOBAL_ASM pragma),
through LLVM IR, each linked with the same DRIVER. The driver owns `main`: it
builds synthetic inputs, calls the function, and prints what the function did.
The candidate passes when its output equals the reference's. On a difference
the first differing scenario is re-run verbosely on both sides and the lines
that differ are shown.

Host fidelity, applied to every build:
  - the o32 type widths (tools/behaviour_drivers/host/PR/ultratypes.h comes
    first on the include path: the tracked header's `long` s32 is 64 bits on
    the host);
  - `-fwrapv -ffp-contract=off -fno-strict-aliasing` (wrapping arithmetic, no
    fused multiply-add, IDO's aliasing);
  - `#pragma weak A = B` lines are dropped (clang rejects the alias followed
    by its typed declaration; each alias becomes an ordinary external, so it
    is stubbed under its own name), and K&R definitions with narrow
    parameters are rewritten to take the promoted type and narrow on entry,
    which is what K&R means and what IDO does.

Driver modes, chosen by markers in the driver text:
  - `#include BEHAVIOUR_TU`: the driver includes the translation unit (it is
    passed as -DBEHAVIOUR_TU), so it sees the TU's own types and statics;
    otherwise the driver and the TU are separate objects, as before.
  - `#define BEHAVIOUR_AUTO_STUBS` (or --auto-stubs): every external the unit
    references and nothing defines -- not the driver, not the runtime, not the
    host C library -- is generated. A function becomes a recording stand-in
    that prints its name, every argument after the prototype's conversions,
    and a deterministic return value (integers from a per-name sequence that
    bh_reset() restarts, floats from a small finite set, pointers NULL). A
    call through an unprototyped declaration (TrapDanglingJump's) is rewritten
    in the IR to a variadic recorder, so each call site's own argument count
    and promoted types are recorded. A global becomes a definition (an
    incomplete array gets 1024 elements), seeded deterministically from its
    name: integer bytes random, floats small finite values, pointers NULL.
    Every global, generated or defined in the unit, is registered with the
    runtime, so bh_reset() restores it and a pointer to it is printed by name.
  - `#define BEHAVIOUR_BIG_ENDIAN` (or --big-endian, include mode only): every
    16/32/64-bit integer and float load and store in the unit is byte-swapped
    in the IR, and multi-byte initializers are stored swapped, so memory is
    big-endian as on the N64. A byte load of halfword data then reads the
    byte the target reads; on a little-endian host `*((u8 *) p + 5)` and
    `*(u16 *) (p + 4) & 0xFF` would disagree, and a sweep cell trading one
    for the other would be misjudged. Pointer words stay native.

The runtime (tools/behaviour_drivers/behaviour_rt.[ch]) gives drivers an
arena with fixed, deterministic addresses, bh_reset(), and digests that print
pointers as region+offset, never as host addresses.

Limits, stated so nobody reads more into a pass than it carries: pointers are
64 bits on the host, so a struct with pointer members has the host's layout
(both sides share it; a driver must not alias two views of one object whose
layouts diverge); behaviour is compared only on the inputs the driver builds;
undefined behaviour the reference shares is not caught; a stand-in records
its arguments, not what the real callee would read through them (memory is
digested where the driver asks, normally after each scenario). A driver is
synthetic input only. Nothing ROM-derived goes in one.

Exit status: 0 same behaviour, 1 different, 2 a compile or run failure. A
candidate that crashes or hangs on an input the reference completes is a
difference (1), not a failure: the reference failing, or the candidate not
building, is 2.
Drivers live in tools/behaviour_drivers/.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DRIVERS = ROOT / "tools" / "behaviour_drivers"
HOST_INCLUDE = DRIVERS / "host"
RUNTIME_C = DRIVERS / "behaviour_rt.c"
CACHE_DIR = ROOT / "build" / "behaviour-cache"
TOOL_VERSION = "2"

COMPAT_FLAGS = [
    "-w", "-O0", "-g0", "-fwrapv", "-ffp-contract=off", "-fno-strict-aliasing",
    "-Wno-int-conversion", "-Wno-incompatible-pointer-types", "-Wno-implicit-function-declaration",
    "-Wno-implicit-int", "-Wno-incompatible-function-pointer-types", "-Wno-return-type",
    "-DNON_MATCHING", "-D_LANGUAGE_C", "-DBEHAVIOUR_HOST",
]
INCOMPLETE_ARRAY_ELEMENTS = 1024
MIN_ARRAY_ELEMENTS = 256

MARK_INCLUDE = re.compile(r"^\s*#\s*include\s+BEHAVIOUR_TU\b", re.M)
MARK_AUTO = re.compile(r"^\s*#\s*define\s+BEHAVIOUR_AUTO_STUBS\b", re.M)
MARK_BE = re.compile(r"^\s*#\s*define\s+BEHAVIOUR_BIG_ENDIAN\b", re.M)


class BuildError(Exception):
    """A unit that cannot be prepared, compiled, linked or run."""


# --------------------------------------------------------------------------
# Source preparation (host compatibility; line numbers are preserved).

NARROW = {"u8", "s8", "u16", "s16", "char", "short", "unsigned char", "signed char",
          "unsigned short", "signed short", "short int", "unsigned short int"}
FLOATS = {"f32", "float"}
KNR_HEAD = re.compile(r"^(\w[\w\s\*]*?\b)(\w+)\s*\(\s*(\w+(?:\s*,\s*\w+)*)\s*\)\s*$")
KNR_DECL = re.compile(r"^(.*?)(\w+)\s*;$")


def prepare_source(text: str) -> str:
    """Make a game TU compile for the host without changing what it means."""
    src = text.split("\n")
    out: list[str] = []
    knr: dict[str, list[str]] = {}
    i = 0
    while i < len(src):
        line = src[i]
        if re.match(r"^\s*#\s*pragma\s+weak\b", line):
            out.append("")
            i += 1
            continue
        m = KNR_HEAD.match(line)
        if m and m.group(2) not in ("if", "while", "for", "switch", "return", "sizeof"):
            names = [n.strip() for n in m.group(3).split(",")]
            j = i + 1
            decls: dict[str, str] = {}
            ok = True
            while j < len(src) and not src[j].strip().startswith("{"):
                d = KNR_DECL.match(src[j].strip())
                if not d or not d.group(1).strip():
                    ok = False
                    break
                decls[d.group(2)] = d.group(1).strip()
                j += 1
            if ok and decls and j < len(src) and set(decls) <= set(names):
                params, inits = [], []
                for n in names:
                    t = decls.get(n, "int")
                    if t in NARROW:
                        params.append(f"int __bh_knr_{n}")
                        inits.append(f"{t} {n} = ({t}) __bh_knr_{n};")
                    elif t in FLOATS:
                        params.append(f"double __bh_knr_{n}")
                        inits.append(f"{t} {n} = ({t}) __bh_knr_{n};")
                    else:
                        params.append(f"{t} {n}")
                knr[m.group(2)] = names
                out.append(f"{m.group(1)}{m.group(2)}({', '.join(params)})")
                out.extend([""] * (j - i - 1))
                out.append(src[j].replace("{", "{ " + " ".join(inits), 1) if inits else src[j])
                i = j + 1
                continue
        out.append(line)
        i += 1
    joined = "\n".join(out)
    # A prototype with narrow parameters for a K&R-defined function conflicts
    # with the definition on clang; an empty parameter list passes the same
    # values (the definition narrows on entry either way).
    narrow = r"\b(?:u8|s8|u16|s16|char|short|f32|float)\b"
    for name in knr:
        decl = re.compile(r"^(\s*(?:extern\s+|static\s+)?\w+[\s\*]+(?:\w+[\s\*]+)*" + re.escape(name)
                          + r"\s*)\(([^()]*)\)(\s*;.*)$", re.M)
        joined = decl.sub(lambda mm: mm.group(1) + "()" + mm.group(3) if re.search(narrow, mm.group(2))
                          else mm.group(0), joined)
    return joined


# --------------------------------------------------------------------------
# LLVM IR helpers.

def split_top(s: str, sep: str = ",") -> list[str]:
    """Split S at SEP outside brackets and quotes."""
    parts, depth, cur, q = [], 0, [], False
    for ch in s:
        if q:
            cur.append(ch)
            if ch == '"':
                q = False
            continue
        if ch == '"':
            q = True
        elif ch in "([{<":
            depth += 1
        elif ch in ")]}>":
            depth -= 1
        if ch == sep and depth == 0:
            parts.append("".join(cur).strip())
            cur = []
        else:
            cur.append(ch)
    tail = "".join(cur).strip()
    if tail:
        parts.append(tail)
    return parts


def match_paren(s: str, i: int) -> int:
    """Index of the bracket closing the one at s[i]."""
    pairs = {"(": ")", "[": "]", "{": "}", "<": ">"}
    stack, q = [], False
    for k in range(i, len(s)):
        ch = s[k]
        if q:
            if ch == '"':
                q = False
            continue
        if ch == '"':
            q = True
        elif ch in pairs:
            stack.append(pairs[ch])
        elif stack and ch == stack[-1]:
            stack.pop()
            if not stack:
                return k
    raise BuildError(f"unbalanced IR text: {s[i:i + 60]!r}")


def parse_type(s: str, i: int = 0):
    """Parse one IR type at s[i:]; return (node, end)."""
    while i < len(s) and s[i] == " ":
        i += 1
    if s.startswith("<{", i):
        j = match_paren(s, i)
        inner = s[i + 2:j - 1]
        return ("struct", True, [parse_type(t)[0] for t in split_top(inner)]), j + 1
    ch = s[i]
    if ch == "{":
        j = match_paren(s, i)
        inner = s[i + 1:j].strip()
        return ("struct", False, [parse_type(t)[0] for t in split_top(inner)] if inner else []), j + 1
    if ch in "[<":
        j = match_paren(s, i)
        m = re.match(r"\s*(\d+)\s+x\s+(.*)$", s[i + 1:j], re.S)
        if not m:
            raise BuildError(f"cannot parse IR type {s[i:j + 1]!r}")
        elem = parse_type(m.group(2).strip())[0]
        return ("arr" if ch == "[" else "vec", int(m.group(1)), elem), j + 1
    m = re.compile(r'%(?:"[^"]+"|[-\w.$]+)|i\d+|float|double|half|ptr(?: addrspace\(\d+\))?|void|x86_fp80|fp128').match(s, i)
    if not m:
        raise BuildError(f"cannot parse IR type at {s[i:i + 40]!r}")
    tok = m.group(0)
    if tok.startswith("%"):
        return ("named", tok), m.end()
    if tok[0] == "i":
        return ("int", int(tok[1:])), m.end()
    if tok.startswith("ptr"):
        return ("ptr",), m.end()
    return (tok,), m.end()


def type_text(node) -> str:
    k = node[0]
    if k == "int":
        return f"i{node[1]}"
    if k == "arr":
        return f"[{node[1]} x {type_text(node[2])}]"
    if k == "vec":
        return f"<{node[1]} x {type_text(node[2])}>"
    if k == "struct":
        body = ", ".join(type_text(t) for t in node[2])
        return f"<{{ {body} }}>" if node[1] else ("{ " + body + " }" if body else "{}")
    if k == "named":
        return node[1]
    return k


class Layout:
    """Host (arm64/x86-64 LP64) sizes and seed plans for IR types."""

    def __init__(self, named: dict[str, str]):
        self.named = named
        self.cache: dict[str, tuple[int, int, list]] = {}

    def resolve(self, name: str):
        body = self.named.get(name)
        if body is None or body.strip() == "opaque":
            return None
        return parse_type(body.strip())[0]

    def of(self, node) -> tuple[int, int, list]:
        """(size, align, plan items) of NODE."""
        k = node[0]
        if k == "int":
            n = max(1, (node[1] + 7) // 8)
            return n, (16 if n > 8 else min(n, 8)), [("r", n)]
        if k == "float":
            return 4, 4, [("f",)]
        if k == "double":
            return 8, 8, [("d",)]
        if k == "half":
            return 2, 2, [("r", 2)]
        if k == "ptr":
            return 8, 8, [("z", 8)]
        if k in ("x86_fp80", "fp128"):
            return 16, 16, [("r", 16)]
        if k == "arr":
            size, align, plan = self.of(node[2])
            n = node[1]
            if n == 0 or size == 0:
                return 0, align, []
            if len(plan) == 1 and plan[0][0] in "rz":
                return size * n, align, [(plan[0][0], size * n)]
            return size * n, align, [("*", n, plan)]
        if k == "vec":
            size, align, _ = self.of(node[2])
            total = size * node[1]
            return total, min(16, total), [("r", total)]
        if k == "named":
            if node[1] in self.cache:
                return self.cache[node[1]]
            res = self.resolve(node[1])
            if res is None:
                raise BuildError(f"opaque type {node[1]} has no layout")
            out = self.of(res)
            self.cache[node[1]] = out
            return out
        if k == "struct":
            off, align, plan = 0, 1, []
            for f in node[2]:
                size, falign, fplan = self.of(f)
                if not node[1]:
                    pad = (-off) % falign
                    if pad:
                        plan.append(("r", pad))
                    off += pad
                    align = max(align, falign)
                plan.extend(fplan)
                off += size
            pad = (-off) % align if not node[1] else 0
            if pad:
                plan.append(("r", pad))
            return off + pad, (1 if node[1] else align), plan
        raise BuildError(f"no layout for IR type {type_text(node)}")


def plan_text(items) -> str:
    merged: list = []
    for it in items:
        if merged and it[0] in "rz" and merged[-1][0] == it[0]:
            merged[-1] = (it[0], merged[-1][1] + it[1])
        else:
            merged.append(it)
    out = []
    for it in merged:
        if it[0] in "rz":
            out.append(f"{it[0]}{it[1]}")
        elif it[0] == "*":
            out.append(f"*{it[1]}({plan_text(it[2])})")
        else:
            out.append(it[0])
    return "".join(out)


def c_string(name: str, text: str) -> str:
    data = text.encode() + b"\0"
    esc = "".join(chr(b) if 32 <= b < 127 and b not in (34, 92) else f"\\{b:02X}" for b in data)
    return f'{name} = private unnamed_addr constant [{len(data)} x i8] c"{esc}", align 1'


GLOBAL_RE = re.compile(r'^(@(?:"[^"]+"|[-\w.$]+)) = (.*?)\b(global|constant) (.*)$')
DECLARE_RE = re.compile(r'^declare (.*?)(@(?:"[^"]+"|[-\w.$]+))\(')
DEFINE_RE = re.compile(r'^define (?:[^@]*?)(@(?:"[^"]+"|[-\w.$]+))\(')
SKIP_GLOBAL = re.compile(r'^@(?:\.|llvm\.|__llvm_|__prof|__covrec|__const\.|"\.)')


def bare(sym: str) -> str:
    return sym[1:].strip('"')


class Unit:
    """A textual LLVM module and what it declares and defines."""

    def __init__(self, text: str):
        self.lines = text.split("\n")
        self.named: dict[str, str] = {}
        for line in self.lines:
            m = re.match(r'^(%(?:"[^"]+"|[-\w.$]+)) = type (.*)$', line)
            if m:
                self.named[m.group(1)] = m.group(2)
        self.layout = Layout(self.named)

    def defined_functions(self) -> set[str]:
        return {bare(m.group(1)) for line in self.lines if (m := DEFINE_RE.match(line))}

    def declarations(self) -> dict[str, int]:
        return {bare(m.group(2)): i for i, line in enumerate(self.lines) if (m := DECLARE_RE.match(line))}

    def globals(self) -> list[tuple[int, str, str, str]]:
        """(line index, @name, linkage words, rest) for every global variable."""
        out = []
        for i, line in enumerate(self.lines):
            m = GLOBAL_RE.match(line)
            if m and not line.startswith(("declare", "define")):
                out.append((i, m.group(1), m.group(2), m.group(4)))
        return out

    def text(self) -> str:
        return "\n".join(self.lines)


def split_type_rest(rest: str) -> tuple[str, str]:
    """Split 'TYPE [INIT][, align N...]' into the type text and the remainder."""
    node, end = parse_type(rest)
    return rest[:end].strip(), rest[end:]


# --------------------------------------------------------------------------
# Auto stubs.

INT_KIND = {8: "8", 16: "16", 32: "32", 64: "64"}


def _param_info(param: str) -> tuple[str, str, str]:
    """(ir type, attrs text, record kind) of one declared parameter."""
    node, end = parse_type(param)
    ty = type_text(node)
    attrs = param[end:].strip()
    if re.search(r"\b(byval|sret|inalloca|preallocated)\b", attrs):
        raise BuildError(f"cannot stub a by-value aggregate parameter ({param})")
    if node[0] == "int":
        sign = "s" if "signext" in attrs else ("u" if "zeroext" in attrs else "i")
        if node[1] == 64:
            sign = "i"
        if node[1] not in INT_KIND:
            raise BuildError(f"cannot stub parameter type {ty}")
        return ty, attrs, f"{sign}{node[1]}"
    if node[0] in ("float", "double", "ptr"):
        return ty, attrs, node[0]
    raise BuildError(f"cannot stub parameter type {ty}")


def stub_function(name: str, decl: str, k: int) -> tuple[str, list[str]]:
    """IR definition for a declared function; returns (text, extra globals)."""
    m = DECLARE_RE.match(decl)
    lp = decl.index("(", m.end() - 1)
    rp = match_paren(decl, lp)
    pre = m.group(1).strip()
    params_text = decl[lp + 1:rp].strip()
    tail = decl[rp + 1:]
    tail = re.sub(r"#\d+", "", tail).strip()
    toks = pre.split()
    if not toks:
        raise BuildError(f"cannot parse declaration {decl!r}")
    ret = toks[-1]
    ret_attrs = " ".join(toks[:-1])
    params = split_top(params_text) if params_text else []
    if "..." in params:
        raise BuildError(f"{name} is variadic and was not rewritten")
    sname = f"@.bhn.{k}"
    extra = [c_string(sname, name)]
    body = [f"  call void @bh_call_begin(ptr {sname})"]
    formal = []
    for n, p in enumerate(params):
        ty, attrs, kind = _param_info(p)
        formal.append(f"{ty} {attrs} %a{n}".replace("  ", " "))
        if kind in ("float", "double"):
            body.append(f"  call void @bh_arg_f{'32' if kind == 'float' else '64'}({ty} %a{n})")
        elif kind == "ptr":
            body.append(f"  call void @bh_arg_p(ptr %a{n})")
        else:
            bits = int(kind[1:])
            tname = f"@.bht.{kind}"
            ext = "sext" if kind[0] == "s" else "zext"
            if bits == 64:
                body.append(f"  call void @bh_arg_i(ptr {tname}, i64 %a{n})")
            else:
                body.append(f"  %x{n} = {ext} {ty} %a{n} to i64")
                body.append(f"  call void @bh_arg_i(ptr {tname}, i64 %x{n})")
    if ret == "void":
        body.append("  call void @bh_call_end()")
        body.append("  ret void")
    elif ret == "ptr":
        body.append("  call void @bh_call_end()")
        body.append("  ret ptr null")
    elif ret in ("float", "double"):
        fn = "@bh_ret_f32" if ret == "float" else "@bh_ret_f64"
        body.append(f"  %r = call {ret} {fn}(ptr {sname})")
        body.append("  call void @bh_call_end()")
        body.append(f"  ret {ret} %r")
    elif re.fullmatch(r"i\d+", ret):
        body.append(f"  %r = call i64 @bh_ret_int(ptr {sname})")
        if ret == "i64":
            body.append("  call void @bh_call_end()")
            body.append("  ret i64 %r")
        else:
            body.append(f"  %rt = trunc i64 %r to {ret}")
            body.append("  call void @bh_call_end()")
            body.append(f"  ret {ret} %rt")
    else:
        raise BuildError(f"cannot stub return type {ret} of {name}")
    head = f"define {ret_attrs} {ret} @{name}({', '.join(formal)}) {tail}".replace("  ", " ")
    return head + " {\nentry:\n" + "\n".join(body) + "\n}", extra


CALL_RE = re.compile(r'^(\s*)(?:(%[-\w.$"]+) = )?((?:tail |musttail |notail )?call )(.*?)(@(?:"[^"]+"|[-\w.$]+))\(')
ARG_KIND = {"i32": "i", "i64": "l", "double": "d", "ptr": "p"}


def rewrite_vcalls(unit: Unit, names: set[str]) -> int:
    """Send every call to an unprototyped NAMES function to bh_vcall."""
    count = 0
    extra: list[str] = []
    sigs: dict[str, str] = {}
    vnames: dict[str, str] = {}
    out = []
    for line in unit.lines:
        m = CALL_RE.match(line)
        if not m or bare(m.group(5)) not in names:
            out.append(line)
            continue
        name = bare(m.group(5))
        lp = m.end() - 1
        rp = match_paren(line, lp)
        args = split_top(line[lp + 1:rp]) if line[lp + 1:rp].strip() else []
        pre = re.sub(r"\(.*\)\s*$", "", m.group(4).strip()).strip()
        ret = pre.split()[-1] if pre else "void"
        sig_args = []
        conv: list[str] = []
        fixed_args = []
        for n, a in enumerate(args):
            node, end = parse_type(a)
            ty = type_text(node)
            val = a[end:].strip()
            if ty in ARG_KIND:
                sig_args.append(ARG_KIND[ty])
                fixed_args.append(a)
            elif ty == "float":
                tmp = f"%bhvf.{count}.{n}"
                val = re.sub(r"^(?:noundef|signext|zeroext|nonnull|inreg|\s)+", "", val)
                conv.append(f"{m.group(1)}{tmp} = fpext float {val} to double")
                sig_args.append("d")
                fixed_args.append(f"double {tmp}")
            elif ty in ("i8", "i16"):
                tmp = f"%bhvi.{count}.{n}"
                ext = "sext" if "signext" in val else "zext"
                val = re.sub(r"^(?:noundef|signext|zeroext|\s)+", "", val)
                conv.append(f"{m.group(1)}{tmp} = {ext} {ty} {val} to i32")
                sig_args.append("i")
                fixed_args.append(f"i32 {tmp}")
            else:
                raise BuildError(f"cannot record a {ty} argument to {name}")
        rk = {"void": "v", "ptr": "p", "float": "f", "double": "d"}.get(ret, "i" if re.fullmatch(r"i\d+", ret) else None)
        if rk is None:
            raise BuildError(f"cannot record a call returning {ret} from {name}")
        sig = f"{rk}:{''.join(sig_args)}"
        if sig not in sigs:
            sigs[sig] = f"@.bhvs.{len(sigs)}"
            extra.append(c_string(sigs[sig], sig))
        if name not in vnames:
            vnames[name] = f"@.bhvn.{len(vnames)}"
            extra.append(c_string(vnames[name], name))
        ind = m.group(1)
        out.extend(conv)
        argtext = ", ".join([f"ptr {vnames[name]}", f"ptr {sigs[sig]}"] + fixed_args)
        res = m.group(2)
        tmp = f"%bhv.{count}"
        if rk in ("f", "d"):
            out.append(f"{ind}{tmp} = call double (ptr, ptr, ...) @bh_vcall_fp({argtext})")
            if res:
                out.append(f"{ind}{res} = " + (f"fptrunc double {tmp} to float" if rk == "f" else f"fadd double {tmp}, 0.0"))
        else:
            out.append(f"{ind}{tmp} = call i64 (ptr, ptr, ...) @bh_vcall({argtext})")
            if res:
                if rk == "p":
                    out.append(f"{ind}{res} = inttoptr i64 {tmp} to ptr")
                elif ret == "i64":
                    out.append(f"{ind}{res} = add i64 {tmp}, 0")
                else:
                    out.append(f"{ind}{res} = trunc i64 {tmp} to {ret}")
        count += 1
    unit.lines = out + extra
    return count


_HOST = None


def host_symbol(name: str) -> bool:
    """True when the host C library (already loaded into this process) defines NAME."""
    global _HOST
    try:
        if _HOST is None:
            _HOST = ctypes.CDLL(None)
        getattr(_HOST, name)
        return True
    except (AttributeError, OSError):
        return False


def nm_defined(obj: Path) -> set[str]:
    r = subprocess.run(["nm", "-g", "-U" if sys.platform == "darwin" else "--defined-only", str(obj)],
                       capture_output=True, text=True)
    out = set()
    for line in r.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 3:
            sym = parts[-1]
            if sys.platform == "darwin" and sym.startswith("_"):
                sym = sym[1:]
            out.add(sym)
    return out


def generate_stubs(unit: Unit, provided: set[str], big_endian: bool) -> dict[str, list[str]]:
    """Define every external NAME nothing provides; register every global. Returns a report."""
    defined = unit.defined_functions()
    decls = unit.declarations()
    report: dict[str, list[str]] = {"functions": [], "variadic": [], "globals": []}
    missing = {}
    for name, idx in decls.items():
        if name.startswith(("llvm.", "bh_")) or name in defined or name in provided:
            continue
        if host_symbol(name):
            continue
        missing[name] = idx
    variadic = set()
    for name, idx in missing.items():
        line = unit.lines[idx]
        lp = line.index("(", DECLARE_RE.match(line).end() - 1)
        params = split_top(line[lp + 1:match_paren(line, lp)])
        if "..." in params:
            variadic.add(name)
    report["variadic"] = sorted(variadic)
    rewrite_vcalls(unit, variadic)
    decls = unit.declarations()
    extra: list[str] = []
    drop: set[int] = set()
    for k, name in enumerate(sorted(missing)):
        if name in variadic:
            continue
        text, strings = stub_function(name, unit.lines[decls[name]], k)
        drop.add(decls[name])
        extra.append(text)
        extra.extend(strings)
        report["functions"].append(name)
    for kind in ("u8", "s8", "i8", "u16", "s16", "i16", "u32", "s32", "i32", "i64"):
        extra.append(c_string(f"@.bht.{kind}", kind))
    ctor = ["define internal void @bh.ctor() {", "entry:"]
    if big_endian:
        ctor.append("  call void @bh_set_big_endian(i32 1)")
    regs = 0
    for idx, sym, linkage, rest in unit.globals():
        name = bare(sym)
        if SKIP_GLOBAL.match(sym) or '__DATA,__bhkeep' in rest or "thread_local" in linkage:
            continue
        if name in provided or name.startswith("bh_"):
            continue
        ty, after = split_type_rest(rest)
        if "external" in linkage.split():
            node = parse_type(ty)[0]
            if node[0] == "arr" and node[1] == 0:
                node = ("arr", INCOMPLETE_ARRAY_ELEMENTS, node[2])
            try:
                size, _align, plan = unit.layout.of(node)
                if node[0] == "arr" and node[1] < MIN_ARRAY_ELEMENTS:
                    # Zero slack past the declared end: an index the driver's
                    # edge values push out of range lands in this global's own
                    # deterministic bytes, not in whatever the linker put next.
                    esize = unit.layout.of(node[2])[0]
                    plan = plan + [("z", (MIN_ARRAY_ELEMENTS - node[1]) * esize)]
                    node = ("arr", MIN_ARRAY_ELEMENTS, node[2])
                    size = MIN_ARRAY_ELEMENTS * esize
            except BuildError:
                node, plan = ("arr", 4096, ("int", 8)), [("r", 4096)]
                size = 4096
            if size == 0:
                continue
            tytext = type_text(node)
            align = re.search(r", align (\d+)", after)
            unit.lines[idx] = (f"{sym} = global {tytext} zeroinitializer"
                               + (f", align {align.group(1)}" if align else ""))
            pname = f"@.bhp.{regs}"
            extra.append(c_string(f"@.bhg.{regs}", name))
            extra.append(c_string(pname, plan_text(plan)))
            ctor.append(f"  call void @bh_region_auto(ptr @.bhg.{regs}, ptr {sym}, "
                        f"i64 ptrtoint (ptr getelementptr ({tytext}, ptr null, i32 1) to i64), ptr {pname})")
            report["globals"].append(name)
        else:
            extra.append(c_string(f"@.bhg.{regs}", name))
            ctor.append(f"  call void @bh_region(ptr @.bhg.{regs}, ptr {sym}, "
                        f"i64 ptrtoint (ptr getelementptr ({ty}, ptr null, i32 1) to i64))")
        regs += 1
    ctor += ["  ret void", "}"]
    unit.lines = [line for i, line in enumerate(unit.lines) if i not in drop]
    extra.append("\n".join(ctor))
    for decl in ("declare void @bh_call_begin(ptr)", "declare void @bh_call_end()",
                 "declare void @bh_arg_i(ptr, i64)", "declare void @bh_arg_f32(float)",
                 "declare void @bh_arg_f64(double)", "declare void @bh_arg_p(ptr)",
                 "declare i64 @bh_ret_int(ptr)", "declare float @bh_ret_f32(ptr)",
                 "declare double @bh_ret_f64(ptr)", "declare void @bh_set_big_endian(i32)",
                 "declare void @bh_region(ptr, ptr, i64)", "declare void @bh_region_auto(ptr, ptr, i64, ptr)",
                 "declare i64 @bh_vcall(ptr, ptr, ...)", "declare double @bh_vcall_fp(ptr, ptr, ...)"):
        fname = decl.split("@")[1].split("(")[0]
        if not any(line.startswith("declare") and f"@{fname}(" in line for line in unit.lines):
            extra.append(decl)
    add_ctor(unit, "@bh.ctor")
    unit.lines.extend(extra)
    return report


def add_ctor(unit: Unit, fn: str) -> None:
    entry = f"{{ i32, ptr, ptr }} {{ i32 65535, ptr {fn}, ptr null }}"
    for i, line in enumerate(unit.lines):
        m = re.match(r"^@llvm\.global_ctors = appending global \[(\d+) x \{ i32, ptr, ptr \}\] \[(.*)\]$", line)
        if m:
            unit.lines[i] = (f"@llvm.global_ctors = appending global [{int(m.group(1)) + 1} x {{ i32, ptr, ptr }}] "
                             f"[{m.group(2)}, {entry}]")
            return
        if line.startswith("@llvm.global_ctors"):
            raise BuildError("cannot extend this module's llvm.global_ctors")
    unit.lines.append(f"@llvm.global_ctors = appending global [1 x {{ i32, ptr, ptr }}] [{entry}]")


# --------------------------------------------------------------------------
# Big-endian memory.

SWAP_TYPES = {"i16": "i16", "i32": "i32", "i64": "i64", "float": "i32", "double": "i64"}
LOAD_RE = re.compile(r"^(\s*)(%[-\w.$\"]+) = load (volatile )?([^,]+), (ptr .*)$")
STORE_RE = re.compile(r"^(\s*)store (volatile )?(\S+) (.*)$")


def swap_int(value: int, bits: int) -> int:
    return int.from_bytes((value & ((1 << bits) - 1)).to_bytes(bits // 8, "little"), "big")


def float_bits(text: str, double: bool) -> int:
    import struct
    if text.startswith("0x"):
        if text[2] in "KLMHR":
            raise BuildError(f"unsupported float literal {text}")
        raw = int(text, 16)
        d = struct.unpack("<d", raw.to_bytes(8, "little"))[0]
    else:
        d = float(text)
    if double:
        return int.from_bytes(struct.pack("<d", d), "little")
    return int.from_bytes(struct.pack("<f", d), "little")


def retype(node, layout: Layout):
    """NODE with every float/double replaced by the integer of its width."""
    k = node[0]
    if k == "float":
        return ("int", 32)
    if k == "double":
        return ("int", 64)
    if k in ("arr", "vec"):
        return (k, node[1], retype(node[2], layout))
    if k == "struct":
        return ("struct", node[1], [retype(f, layout) for f in node[2]])
    if k == "named":
        res = layout.resolve(node[1])
        if res is None:
            raise BuildError(f"opaque {node[1]} in an initializer")
        return retype(res, layout) if has_wide_scalar(res, layout) else node
    return node


def swap_constant(node, text: str, layout: Layout):
    """Return (new type node, new value text) of a constant with its wide scalars byte-swapped.

    Floats become integers holding the swapped bit pattern (with opaque
    pointers nothing that uses the global depends on its value type)."""
    text = text.strip()
    k = node[0]
    if text in ("zeroinitializer", "undef", "poison"):
        return retype(node, layout), text
    if k == "ptr":
        return node, text
    if k == "int":
        if node[1] in (16, 32, 64) and text not in ("true", "false"):
            return node, str(swap_int(int(text), node[1]))
        return node, text
    if k in ("float", "double"):
        bits = 32 if k == "float" else 64
        return ("int", bits), str(swap_int(float_bits(text, k == "double"), bits))
    if k == "named":
        res = layout.resolve(node[1])
        if res is None:
            raise BuildError(f"opaque {node[1]} in an initializer")
        if not has_wide_scalar(res, layout):
            return node, text
        return swap_constant(res, text, layout)
    if k == "arr":
        if text.startswith('c"'):
            return node, text
        if not text.startswith("["):
            raise BuildError(f"cannot swap initializer {text[:40]!r}")
        items = split_top(text[1:-1]) if text[1:-1].strip() else []
        vals = []
        for it in items:
            en, end = parse_type(it)
            nt, nv = swap_constant(en, it[end:], layout)
            vals.append(f"{type_text(nt)} {nv}")
        return ("arr", node[1], retype(node[2], layout)), "[" + ", ".join(vals) + "]"
    if k == "struct":
        packed = text.startswith("<{")
        inner = text[2:-2] if packed else text[1:-1]
        items = split_top(inner) if inner.strip() else []
        fields, vals = [], []
        for it in items:
            en, end = parse_type(it)
            nt, nv = swap_constant(en, it[end:], layout)
            fields.append(nt)
            vals.append(f"{type_text(nt)} {nv}")
        body = ", ".join(vals)
        return ("struct", packed, fields), ("<{ " + body + " }>" if packed else "{ " + body + " }")
    raise BuildError(f"cannot swap a {type_text(node)} initializer")


def has_wide_scalar(node, layout: Layout, seen=None) -> bool:
    k = node[0]
    if k == "int":
        return node[1] in (16, 32, 64)
    if k in ("float", "double"):
        return True
    if k in ("arr", "vec"):
        return has_wide_scalar(node[2], layout, seen)
    if k == "struct":
        return any(has_wide_scalar(f, layout, seen) for f in node[2])
    if k == "named":
        seen = seen or set()
        if node[1] in seen:
            return False
        seen.add(node[1])
        res = layout.resolve(node[1])
        return res is not None and has_wide_scalar(res, layout, seen)
    return False


def big_endian_transform(unit: Unit) -> dict[str, int]:
    """Byte-swap every wide scalar load, store and initializer in UNIT."""
    out = []
    counts = {"loads": 0, "stores": 0, "initializers": 0}
    n = 0
    for line in unit.lines:
        s = line.lstrip()
        if s.startswith(("load atomic", "store atomic")) or " = load atomic " in line or \
                re.match(r"^\s*(?:%\S+ = )?(atomicrmw|cmpxchg)\b", line):
            raise BuildError("big-endian mode: atomic memory operations are not supported")
        if "@llvm.va_start" in line and not line.startswith("declare"):
            raise BuildError("big-endian mode: a variadic function body in the unit is not supported")
        if "@__profc_" in line:
            out.append(line)  # coverage counters stay native for llvm-profdata
            continue
        m = LOAD_RE.match(line)
        if m:
            ty = m.group(4).strip()
            if ty in SWAP_TYPES:
                it = SWAP_TYPES[ty]
                ind, res, vol, rest = m.group(1), m.group(2), m.group(3) or "", m.group(5)
                tag = f"%bhl.{n}"
                n += 1
                out.append(f"{ind}{tag} = load {vol}{it}, {rest}")
                if ty == it:
                    out.append(f"{ind}{res} = call {it} @llvm.bswap.{it}({it} {tag})")
                else:
                    out.append(f"{ind}{tag}.s = call {it} @llvm.bswap.{it}({it} {tag})")
                    out.append(f"{ind}{res} = bitcast {it} {tag}.s to {ty}")
                counts["loads"] += 1
                continue
            if not re.fullmatch(r"i8|i1|ptr", ty):
                raise BuildError(f"big-endian mode: unsupported load of {ty}")
        m = STORE_RE.match(line)
        if m:
            ty = m.group(3)
            if ty in SWAP_TYPES:
                ind, vol, rest = m.group(1), m.group(2) or "", m.group(4)
                cut = rest.find(", ptr ")
                if cut < 0:
                    raise BuildError(f"cannot parse store {line.strip()!r}")
                value, ptr = rest[:cut], rest[cut + 2:]
                it = SWAP_TYPES[ty]
                tag = f"%bhs.{n}"
                n += 1
                if ty != it:
                    out.append(f"{ind}{tag}.b = bitcast {ty} {value} to {it}")
                    value = f"{tag}.b"
                out.append(f"{ind}{tag} = call {it} @llvm.bswap.{it}({it} {value})")
                out.append(f"{ind}store {vol}{it} {tag}, {ptr}")
                counts["stores"] += 1
                continue
            if not re.fullmatch(r"i8|i1|ptr", ty):
                raise BuildError(f"big-endian mode: unsupported store of {ty}")
        g = GLOBAL_RE.match(line)
        if g and not line.startswith(("declare", "define")) and "external" not in g.group(2).split() \
                and not re.match(r'^@(?:__llvm_|__prof|__covrec|llvm\.)', g.group(1)):
            rest = g.group(4)
            node, end = parse_type(rest)
            if has_wide_scalar(node, unit.layout):
                tail = rest[end:]
                cut = re.search(r", (?:align \d+|section |comdat|!)", tail)
                init, after = (tail[:cut.start()], tail[cut.start():]) if cut else (tail, "")
                if init.strip() != "zeroinitializer":
                    nt, nv = swap_constant(node, init, unit.layout)
                    line = f"{g.group(1)} = {g.group(2)}{g.group(3)} {type_text(nt)} {nv}{after}"
                    counts["initializers"] += 1
        out.append(line)
    for it in ("i16", "i32", "i64"):
        decl = f"declare {it} @llvm.bswap.{it}({it})"
        if not any(line.startswith(f"declare {it} @llvm.bswap.{it}(") for line in out):
            out.append(decl)
    unit.lines = out
    return counts


# --------------------------------------------------------------------------
# Build and run.

def _run(cmd: list[str], what: str, cwd: Path | None = None) -> None:
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd)
    if r.returncode:
        msg = (r.stderr.strip() or r.stdout.strip()).splitlines()
        err = [m for m in msg if "error" in m.lower()] or msg or ["?"]
        raise BuildError(f"{what} failed: {err[0]}")


def driver_modes(driver_text: str, auto_stubs: bool | None, big_endian: bool | None) -> tuple[bool, bool, bool]:
    include = bool(MARK_INCLUDE.search(driver_text))
    auto = bool(MARK_AUTO.search(driver_text)) if auto_stubs is None else auto_stubs
    be = bool(MARK_BE.search(driver_text)) if big_endian is None else big_endian
    if be and not include:
        raise BuildError("big-endian mode needs a driver that does #include BEHAVIOUR_TU")
    return include, auto, be


def build(source: Path, driver: Path, tmp: Path, cc: str = "cc", auto_stubs: bool | None = None,
          big_endian: bool | None = None, coverage: bool = False, report: dict | None = None,
          include_dirs: list[Path] | None = None) -> Path:
    """Build SOURCE with DRIVER into an executable under TMP; return its path."""
    driver_text = driver.read_text()
    include, auto, be = driver_modes(driver_text, auto_stubs, big_endian)
    tu = tmp / "tu.c"
    tu.write_text(prepare_source(source.read_text()))
    inc = ["-I", str(HOST_INCLUDE), "-I", str(DRIVERS), "-I", str(source.resolve().parent)]
    for d in include_dirs or []:
        inc += ["-I", str(d)]
    inc += ["-I", str(ROOT / "include"), "-I", str(ROOT)]
    cov = ["-fprofile-instr-generate", "-fcoverage-mapping"] if coverage else []
    unit_src = driver if include else tu
    defs = [f'-DBEHAVIOUR_TU="{tu}"'] if include else []
    ll = tmp / "unit.ll"
    _run([cc, *COMPAT_FLAGS, *cov, *inc, *defs, "-S", "-emit-llvm", str(unit_src), "-o", str(ll)],
         "compile")
    rt = tmp / "rt.o"
    _run([cc, "-w", "-O1", "-c", str(RUNTIME_C), "-o", str(rt)], "runtime compile")
    objs = [rt]
    provided = nm_defined(rt)
    if not include:
        dobj = tmp / "driver.o"
        _run([cc, *COMPAT_FLAGS, *inc, "-c", str(driver), "-o", str(dobj)], "driver compile")
        objs.append(dobj)
        provided |= nm_defined(dobj)
    unit = Unit(ll.read_text())
    rep = report if report is not None else {}
    if auto:
        rep.update(generate_stubs(unit, provided, be))
    if be:
        rep["big_endian"] = big_endian_transform(unit)
    ll.write_text(unit.text() + "\n")
    uobj = tmp / "unit.o"
    _run([cc, "-w", "-O0", *cov, "-c", str(ll), "-o", str(uobj)], "unit compile")
    exe = tmp / "run"
    _run([cc, *cov, "-o", str(exe), str(uobj), *map(str, objs), "-lm"], "link")
    return exe


def _cache_key(source: Path, driver: Path, cc: str, opts: tuple) -> str:
    h = hashlib.sha256()
    for part in (TOOL_VERSION, source.read_bytes(), driver.read_bytes(), RUNTIME_C.read_bytes(),
                 (HOST_INCLUDE / "PR" / "ultratypes.h").read_bytes(), (DRIVERS / "behaviour_rt.h").read_bytes(),
                 Path(__file__).read_bytes(), cc.encode(), repr(opts).encode()):
        h.update(part if isinstance(part, bytes) else part.encode())
        h.update(b"\0")
    try:
        h.update(subprocess.run([cc, "--version"], capture_output=True, text=True).stdout.encode())
    except OSError:
        pass
    return h.hexdigest()[:32]


def run_tu(source: Path, driver: Path, cc: str = "cc", timeout: int = 120, auto_stubs: bool | None = None,
           big_endian: bool | None = None, env: dict | None = None, cache: bool = True,
           keep: Path | None = None, include_dirs: list[Path] | None = None) -> tuple[bool, str]:
    """Build SOURCE with DRIVER for the host, run it, return (ok, stdout or error)."""
    source, driver = Path(source), Path(driver)
    key = None
    if cache and not env:
        key = _cache_key(source, driver, cc, (auto_stubs, big_endian, [str(d) for d in include_dirs or []]))
        hit = CACHE_DIR / f"{key}.out"
        if hit.exists():
            return True, hit.read_text()
    tmpdir = tempfile.mkdtemp(prefix="behaviour_")
    tmp = Path(tmpdir)
    try:
        try:
            exe = build(source, driver, tmp, cc, auto_stubs, big_endian, include_dirs=include_dirs)
        except BuildError as e:
            return False, str(e)
        run_env = dict(os.environ)
        run_env.pop("BH_ONLY", None)
        run_env.pop("BH_VERBOSE", None)
        run_env.update(env or {})
        try:
            r = subprocess.run([str(exe)], capture_output=True, text=True, timeout=timeout, env=run_env)
        except subprocess.TimeoutExpired:
            return False, "run timed out"
        if r.returncode:
            detail = r.stderr.strip().splitlines()
            return False, f"run failed with status {r.returncode}" + (f": {detail[-1]}" if detail else "")
        if key:
            try:
                CACHE_DIR.mkdir(parents=True, exist_ok=True)
                part = CACHE_DIR / f"{key}.tmp{os.getpid()}"
                part.write_text(r.stdout)
                part.replace(CACHE_DIR / f"{key}.out")
            except OSError:
                pass
        return True, r.stdout
    finally:
        if keep:
            keep.mkdir(parents=True, exist_ok=True)
            for f in tmp.iterdir():
                shutil.copy2(f, keep / f.name)
        shutil.rmtree(tmpdir, ignore_errors=True)


def first_difference(a: str, b: str) -> tuple[int, str, str, str]:
    """(line index, scenario label, reference line, candidate line) of the first difference."""
    la, lb = a.split("\n"), b.split("\n")
    label = ""
    for i in range(max(len(la), len(lb))):
        x = la[i] if i < len(la) else "<end of output>"
        y = lb[i] if i < len(lb) else "<end of output>"
        if x.startswith("== ") and x == y:
            label = x[3:]
        if x != y:
            return i, label, x, y
    return -1, label, "", ""


def explain(candidate: Path, reference: Path, driver: Path, label: str, cc: str, auto_stubs, big_endian,
            limit: int = 12) -> list[str]:
    """Re-run one scenario verbosely on both sides; list the lines that differ."""
    env = {"BH_VERBOSE": "1", "BH_ONLY": label}
    inc = [reference.resolve().parent]
    ok_r, out_r = run_tu(reference, driver, cc, auto_stubs=auto_stubs, big_endian=big_endian, env=env,
                         include_dirs=inc)
    ok_c, out_c = run_tu(candidate, driver, cc, auto_stubs=auto_stubs, big_endian=big_endian, env=env,
                         include_dirs=inc)
    if not (ok_r and ok_c):
        return []
    ra = [x for x in out_r.split("\n") if x and not x.startswith("== ")]
    ca = [x for x in out_c.split("\n") if x and not x.startswith("== ")]
    import difflib
    lines = []
    for op in difflib.unified_diff(ra, ca, "reference", "candidate", n=0, lineterm=""):
        if op.startswith(("---", "+++", "@@")):
            continue
        lines.append(op)
        if len(lines) >= limit:
            lines.append("...")
            break
    return lines


def same_behaviour(candidate: Path, reference: Path, driver: Path, cc: str = "cc", auto_stubs: bool | None = None,
                   big_endian: bool | None = None, detail: bool = False) -> tuple[int, str]:
    inc = [Path(reference).resolve().parent]
    ok_r, out_r = run_tu(reference, driver, cc, auto_stubs=auto_stubs, big_endian=big_endian, include_dirs=inc)
    if not ok_r:
        return 2, f"reference: {out_r}"
    ok_c, out_c = run_tu(candidate, driver, cc, auto_stubs=auto_stubs, big_endian=big_endian, include_dirs=inc)
    if not ok_c:
        if out_c.startswith("run "):
            # Crashing or hanging where the reference ran is a behaviour.
            return 1, f"different behaviour: the candidate {out_c[4:]} (the reference ran to completion)"
        return 2, f"candidate: {out_c}"
    if out_c == out_r:
        return 0, "same behaviour"
    if not detail:
        return 1, "different behaviour"
    idx, label, x, y = first_difference(out_r, out_c)
    msg = [f"different behaviour (first at output line {idx + 1}, scenario {label!r})",
           f"  reference: {x}", f"  candidate: {y}"]
    lines = explain(Path(candidate), Path(reference), Path(driver), label, cc, auto_stubs, big_endian)
    if lines:
        msg.append(f"scenario {label!r}, verbose, lines that differ:")
        msg.extend("  " + ln for ln in lines)
    return 1, "\n".join(msg)


def coverage_report(source: Path, driver: Path, symbol: str, cc: str, auto_stubs, big_endian) -> tuple[int, str]:
    """Run SOURCE under DRIVER with source coverage; report SYMBOL's uncovered regions."""
    tmpdir = tempfile.mkdtemp(prefix="behaviour_cov_")
    tmp = Path(tmpdir)
    try:
        try:
            exe = build(Path(source), Path(driver), tmp, cc, auto_stubs, big_endian, coverage=True)
        except BuildError as e:
            return 2, str(e)
        env = dict(os.environ, LLVM_PROFILE_FILE=str(tmp / "run.profraw"))
        env.pop("BH_ONLY", None)
        r = subprocess.run([str(exe)], capture_output=True, text=True, env=env, timeout=600)
        if r.returncode:
            return 2, f"run failed with status {r.returncode}"
        tool = (lambda t: ["xcrun", t]) if sys.platform == "darwin" else (lambda t: [t])
        _run([*tool("llvm-profdata"), "merge", "-o", str(tmp / "run.profdata"), str(tmp / "run.profraw")],
             "llvm-profdata")
        rep = subprocess.run([*tool("llvm-cov"), "report", str(exe), "-instr-profile", str(tmp / "run.profdata"),
                              "-show-functions", str(tmp / "tu.c")], capture_output=True, text=True)
        rows = [ln for ln in rep.stdout.splitlines() if ln.startswith(symbol + " ") or ln.startswith("Name")]
        show = subprocess.run([*tool("llvm-cov"), "show", str(exe), "-instr-profile", str(tmp / "run.profdata"),
                               f"-name={symbol}", "-show-line-counts-or-regions", "-show-branches=count",
                               str(tmp / "tu.c")], capture_output=True, text=True)
        uncovered, branches = [], []
        for ln in show.stdout.splitlines():
            m = re.match(r"^\s*(\d+)\|\s*0\|(.*)$", ln)
            if m:
                uncovered.append(f"  line {m.group(1)}: {m.group(2).strip()}")
            b = re.search(r"Branch \((\d+):(\d+)\): \[True: ([^,]+), False: ([^\]]+)\]", ln)
            if b and (b.group(3).strip() == "0" or b.group(4).strip() == "0"):
                side = "true" if b.group(3).strip() == "0" else "false"
                branches.append(f"  branch at {b.group(1)}:{b.group(2)} never {side}")
        out = rows + ([f"{len(uncovered)} lines never executed:"] + uncovered[:40] if uncovered
                      else ["every line executed"])
        out += [f"{len(branches)} branch outcomes never taken:"] + branches[:40] if branches else \
            ["every branch outcome taken"]
        return 0, "\n".join(out)
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("candidate", type=Path)
    ap.add_argument("--driver", type=Path, required=True)
    ap.add_argument("--reference", type=Path, help="reference TU (required unless --digest/--coverage)")
    ap.add_argument("--digest", action="store_true", help="print the candidate's output and exit")
    ap.add_argument("--coverage", metavar="SYMBOL", help="report which lines of SYMBOL the driver never runs")
    ap.add_argument("--auto-stubs", action="store_true", default=None,
                    help="generate stand-ins for every undefined external (default: the driver's marker)")
    ap.add_argument("--big-endian", action="store_true", default=None,
                    help="byte-swap unit memory (default: the driver's marker)")
    ap.add_argument("--quiet", action="store_true", help="no explanation on a difference")
    ap.add_argument("--no-cache", action="store_true", help="do not reuse cached outputs")
    ap.add_argument("--keep", type=Path, help="copy the build directory (IR, objects) here")
    ap.add_argument("--cc", default=os.environ.get("CC", "cc"))
    a = ap.parse_args(argv)
    if a.coverage:
        status, msg = coverage_report(a.candidate, a.driver, a.coverage, a.cc, a.auto_stubs, a.big_endian)
        print(msg)
        return status
    if a.digest or a.keep:
        ok, out = run_tu(a.candidate, a.driver, a.cc, auto_stubs=a.auto_stubs, big_endian=a.big_endian,
                         cache=not a.no_cache and not a.keep, keep=a.keep)
        if a.digest:
            print(out.rstrip() if ok else out)
            return 0 if ok else 2
        if not ok:
            print(out)
            return 2
    if a.reference is None:
        ap.error("--reference is required unless --digest or --coverage")
    global CACHE_DIR
    saved = CACHE_DIR
    if a.no_cache:
        CACHE_DIR = Path(tempfile.mkdtemp(prefix="behaviour_nocache_"))
    try:
        status, msg = same_behaviour(a.candidate, a.reference, a.driver, a.cc, a.auto_stubs, a.big_endian,
                                     detail=not a.quiet)
    finally:
        if a.no_cache:
            shutil.rmtree(CACHE_DIR, ignore_errors=True)
            CACHE_DIR = saved
    print(msg)
    return status


if __name__ == "__main__":
    sys.exit(main())
