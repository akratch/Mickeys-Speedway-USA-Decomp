#!/usr/bin/env python3
"""List the inherited shape artefacts in one NON_MATCHING candidate.

    tools/shape_lint.py <symbol> [--object PATH]

The brief's shape checklist is what matched sixty functions on 2026-10-01:
nearly every one was an m2c-derived candidate whose closure described the
inherited shape, not the function. Reading a candidate for those artefacts by
hand takes a lane its first twenty minutes and is easy to do incompletely.
This prints them, each with the checklist item it belongs to and the axis a
`tools/shape_product.py` candidate should carry for it.

It reads the candidate's `#ifdef NON_MATCHING` body and, when a compiled
object is available (the tree's `build_non_matching/` object, or `--object`),
its relocation records, which is how a "global" that is really a float
literal in the TU's own pool is told apart from a real global: its records
are LOCAL against `.rodata` rather than against a named data symbol.

Every finding is a hypothesis to price, not a verdict: the same artefact was
load-bearing on `overlay14CreateValue` and inert on `func_8000B3CC`. The
report says what to measure, in the order the checklist found decisive.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import fast_score  # noqa: E402

CHECKS = [
    # (item, pattern, message, suggested axis)
    (1, re.compile(r"^\s*(\w+)\s*=\s*\(\s*(\w+(?:->\w+|\[\w+\])*)\s*[-+*/]?=\s*"),
     "assignment-in-expression carrier `{0} = ({1} ...)`: read the field or global back at each use",
     "SHAPE_carrier_{0}"),
    (1, re.compile(r"^\s*(\w+)\s*=\s*(\w+)\s*;\s*$"),
     "copy `{0} = {1}` of a global or parameter into a local: consider reading `{1}` at each use",
     "SHAPE_copy_{0}"),
    (3, re.compile(r"\bvolatile\b"),
     "`volatile`: usually an inherited device; the natural spelling or the address form (L144) replaces it",
     "SHAPE_volatile"),
    (4, re.compile(r"<<\s*\d+\s*\)\s*(>=|<)\s*0|\(\s*u8\s*\*\s*\)\s*&\s*\w+\s*\|=|&\s*0x[0-9A-Fa-f]+\s*\)\s*(!=|==)\s*0"),
     "flag word tested by shift-and-sign or masked: a bitfield struct spends the shipped draws",
     "SHAPE_bitfield"),
    (5, re.compile(r"^\s*goto\s+\w+;"),
     "`goto` loop: write the natural `while`/`for`/`do` (m2c emits gotos the author never wrote)",
     "SHAPE_loop"),
    (5, re.compile(r"\bdo\s*\{"),
     "`do { } while`: check whether a `while (n--)` / `for (i = 0; i < count; i++)` reading the count global is the author's form",
     "SHAPE_loop"),
    (6, re.compile(r"->w0\s*=|->w1\s*=|words\.w0"),
     "display-list words written by hand: one packet macro per command taking `(*commands)++`, opcode word first",
     "SHAPE_dlmacro"),
    (7, re.compile(r"\bunion\b"),
     "`union` standing in for two locals: two plain locals in frame order land the homes",
     "SHAPE_union"),
    (7, re.compile(r"\bu8\s+\w*(pad|unused|stack|work|scratch)\w*\s*\[\s*0x[0-9A-Fa-f]+\s*\]"),
     "padded buffer local: replace with plain locals plus unused s32 pads counted by frame_census",
     "SHAPE_pads"),
    (8, re.compile(r"^\s*(\w+)\s*=\s*[-\w.]+;\s*\n(?:\s*\n)*\s*if\s*\(.*\)\s*\{?\s*\n\s*\1\s*="),
     "default-then-override `{0} = K; if (c) {0} = f;`: the target may be `if (c) ... else ...`",
     "SHAPE_ifelse_{0}"),
    (0, re.compile(r"\|\s*0\b|\(\s*u32\s*\)\s*\w+\s*\|\s*0"),
     "or-with-zero probe: an inherited allocator device; price its removal",
     "SHAPE_or0"),
    (0, re.compile(r"\bif\s*\(\s*1\s*\)|do\s*\{\s*\}\s*while\s*\(\s*0\s*\)"),
     "`if (1)` / `do {} while (0)` region marker: inherited; price its removal with the natural shape",
     "SHAPE_region"),
    (0, re.compile(r"^#line\s+\d+", re.M),
     "`#line` directive: an inherited as1 tie device; a natural one-statement-per-line layout usually supersedes it",
     "SHAPE_lines"),
]

FLOAT_EXTERN_RE = re.compile(r"^\s*extern\s+f32\s+([A-Za-z_]\w*)\s*;", re.M)
ALIAS_EXTERN_RE = re.compile(r"^\s*extern\s+[\w\s\*]+?\b([A-Za-z_]\w*)\s*\[\s*\]\s*;", re.M)


def candidate_body(text: str, symbol: str) -> str:
    """Return the NON_MATCHING body that defines `symbol`, or the whole file."""
    m = re.search(rf"^[\w\s\*]*\b{re.escape(symbol)}\s*\(", text, re.M)
    if not m:
        return text
    start = text.rfind("#ifdef NON_MATCHING", 0, m.start())
    end = text.find("#else", m.end())
    if start < 0 or end < 0:
        return text[m.start():]
    return text[start:end]


def object_relocs(obj: Path) -> list[tuple[str, str]]:
    out = subprocess.run(
        [str(ROOT / "tools/binutils/mips64-elf-objdump"), "-r", str(obj)],
        capture_output=True, text=True,
    ).stdout
    rows = []
    for line in out.splitlines():
        m = re.match(r"^[0-9a-f]{8}\s+(R_MIPS_\w+)\s+(\S+)", line)
        if m:
            rows.append((m.group(1), m.group(2)))
    return rows


def lint(symbol: str, source: str, text: str, relocs: list[tuple[str, str]] | None) -> list[dict]:
    body = candidate_body(text, symbol)
    findings: list[dict] = []
    seen: set[tuple[int, str]] = set()
    for item, pat, msg, axis in CHECKS:
        pat = re.compile(pat.pattern, pat.flags | re.M)
        for m in pat.finditer(body):
            groups = [g for g in m.groups() if g] if m.groups() else []
            message = msg.format(*groups) if groups else msg
            key = (item, message)
            if key in seen:
                continue
            seen.add(key)
            line = body[: m.start()].count("\n") + 1
            findings.append({"item": item, "line": line, "finding": message,
                             "axis": axis.format(*groups) if groups else axis})
    # Item 2: float "globals" whose records are local to .rodata are literals.
    float_names = FLOAT_EXTERN_RE.findall(text)
    if float_names:
        referenced = {n for n in float_names if re.search(rf"\b{n}\b", body)}
        if relocs is not None:
            named = {sym for _, sym in relocs}
            pool = [n for n in referenced if n not in named]
            if pool:
                findings.append({"item": 2, "line": 0,
                                 "finding": f"f32 externs with no relocation record in the object (pool literals, not globals): {', '.join(sorted(pool))}",
                                 "axis": "SHAPE_literals"})
        elif referenced:
            findings.append({"item": 2, "line": 0,
                             "finding": f"f32 externs {', '.join(sorted(referenced))}: check whether their records are LOCAL against .rodata (then they are literals)",
                             "axis": "SHAPE_literals"})
    # Item 3: several array externs sharing one stored addend in the alias file.
    alias_file = ROOT / "overlay_undefined_syms.us.txt"
    if alias_file.is_file():
        values: dict[str, list[str]] = {}
        names = set(ALIAS_EXTERN_RE.findall(text))
        for line in alias_file.read_text().splitlines():
            m = re.match(r"^(\w+)\s*=\s*(0x[0-9a-fA-F]+);", line)
            if m and m.group(1) in names:
                values.setdefault(m.group(2).lower(), []).append(m.group(1))
        for value, syms in values.items():
            if len(syms) > 1:
                findings.append({"item": 3, "line": 0,
                                 "finding": f"array externs sharing stored addend {value}: {', '.join(syms)} (alias names for one object; use one symbol)",
                                 "axis": "SHAPE_alias"})
    findings.sort(key=lambda f: (f["item"] or 99, f["line"]))
    return findings


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("--object", help="compiled NON_MATCHING object to read relocations from")
    ap.add_argument("--json", action="store_true")
    ns = ap.parse_args(argv)
    source = fast_score.tracked_source_for(ns.symbol)
    text = (ROOT / source).read_text()
    obj = Path(ns.object) if ns.object else ROOT / f"build_non_matching/{source}.o"
    relocs = object_relocs(obj) if obj.is_file() else None
    findings = lint(ns.symbol, source, text, relocs)
    if ns.json:
        print(json.dumps({"symbol": ns.symbol, "source": source, "findings": findings}, indent=1))
        return 0
    print(f"{ns.symbol} ({source}): {len(findings)} artefact(s); relocations {'read' if relocs is not None else 'not available (no object)'}")
    for f in findings:
        item = f"[{f['item']}]" if f["item"] else "[-]"
        where = f"line {f['line']}" if f["line"] else "file"
        print(f"  {item:4s} {where:9s} {f['finding']}\n        axis: {f['axis']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
