#!/usr/bin/env python3
"""Per-handler comparison of a switch-dispatched function against its target.

    tools/handler_diff.py <symbol> <candidate.c>                 # summary per handler
    tools/handler_diff.py <symbol> <candidate.c> --case 0x79 8   # side by side
    tools/handler_diff.py <symbol> <candidate.c> --roles         # saved-register role table

For a function that is one big `switch` (func_800517E0's 127-entry command
interpreter is the case it was written for), whole-function alignment smears
one handler's length error over everything after it. This tool splits both
sides at the dispatch table instead: the target's table is read from the
extracted rodata listing (the `jtbl_` symbol the function's `jr` loads), the
candidate's from the run of `.rodata` relocations against `.text` of the same
length. Each handler is then aligned on its own, register-erased, and reported
as length delta, shape differences (different instruction or operand kind) and
naming differences (same instruction, different register).

`--roles` prints, per handler, which callee-saved registers (s0, s3-s7, fp) are
written and from what, target beside candidate. A saved register written in a
handler with no call after it marks a variable shared with another case where
it does cross a call, so the table names which candidate local should carry
each value. On func_800517E0 the target keeps one value class per register.

The candidate is compiled with tools/fast_score.py (the tree's own recipe).
Everything printed is read at run time from the extracted listing and the
candidate object; nothing is stored. Display only: the measurement is
fast_score's masked count and aligned residual.
"""

from __future__ import annotations

import argparse
import difflib
import glob
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

STORES = ("sb", "sh", "sw", "swc1", "sdc1", "swl", "swr", "sd")
SAVED = ("$s0", "$s3", "$s4", "$s5", "$s6", "$s7", "$fp")
REGS = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3",
        "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
        "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
LISTING_ROW = re.compile(r"\s+/\* [0-9A-Fa-f]+ ([0-9A-Fa-f]{8}) [0-9A-Fa-f]{8} \*/\s+(\S+)\s*(.*)")


def parse_listing(text: str) -> tuple[int, list[tuple[int, str]]]:
    """(function vram, [(offset, 'mnemonic operands')]) from a splat listing."""
    rows, base = [], None
    for line in text.splitlines():
        m = LISTING_ROW.match(line)
        if not m or m.group(2).startswith("."):
            continue
        vram = int(m.group(1), 16)
        base = vram if base is None else base
        rows.append((vram - base, (m.group(2) + " " + m.group(3).replace(", ", ",")).strip()))
    return base or 0, rows


def parse_objdump(text: str, symbol: str) -> list[tuple[int, str]]:
    """[(offset, 'mnemonic operands')] for `symbol` from `objdump -d -r`."""
    rows, inside, base = [], False, 0
    for line in text.splitlines():
        h = re.match(r"^([0-9a-f]+) <(\S+)>:", line)
        if h:
            if inside:
                break
            inside = h.group(2) == symbol
            base = int(h.group(1), 16)
            continue
        if not inside or re.match(r"\s+[0-9a-f]+:\s+R_", line):
            continue
        m = re.match(r"\s+([0-9a-f]+):\s+(\S+)\s*(.*)", line)
        if m:
            ops = re.sub(r"\$(\d+)\b", lambda r: "$" + REGS[int(r.group(1))], m.group(3))
            rows.append((int(m.group(1), 16) - base, (m.group(2) + " " + re.sub(r"\s+", "", ops)).strip()))
    return rows


def normalise(ins: str) -> str:
    """Erase immediates, relocations and branch targets; canonicalise aliases."""
    ins = re.sub(r"\s*<[^>]*>.*$", "", ins)
    parts = ins.split(None, 1)
    m, o = parts[0], (parts[1] if len(parts) > 1 else "")
    alias = {"beqz": ("beq", 1), "bnez": ("bne", 1), "beqzl": ("beql", 1), "bnezl": ("bnel", 1)}
    if m in alias:
        m = alias[m][0]
        o = o.replace(",", ",$zero,", 1)
    elif m == "b":
        m, o = "beq", "$zero,$zero," + o
    elif m == "nop":
        m, o = "sll", "$zero,$zero,0"
    elif m == "move":
        m, o = "or", o + ",$zero"
    elif m == "not":
        m, o = "nor", o + ",$zero"
    if m.startswith("b") or m in ("j", "jal"):
        o = "" if m in ("j", "jal") else ",".join(o.split(",")[:-1])
    o = re.sub(r"\(0x[0-9A-Fa-f]+ >> 16\)|%(?:hi|lo)\([^)]*\)", "N", o)
    o = re.sub(r"-?0x[0-9A-Fa-f]+|-?\b\d+\b", "N", o)
    return (m + " " + o).strip()


def regions(starts: list[int]) -> list[tuple[int, int | None]]:
    """Per table entry, (start, end) of its handler; end is the next distinct start."""
    uniq = sorted(set(starts))
    nxt = {s: (uniq[i + 1] if i + 1 < len(uniq) else None) for i, s in enumerate(uniq)}
    return [(s, nxt[s]) for s in starts]


def rows_in(rows, span):
    lo, hi = span
    return [r for r in rows if r[0] >= lo and (hi is None or r[0] < hi)]


def compare(trows, crows):
    """(opcodes, normalised target, normalised candidate, shape, naming) for one handler."""
    tn = [normalise(r[1]) for r in trows]
    cn = [normalise(r[1]) for r in crows]
    erase = lambda s: re.sub(r"\$\w+", "R", s)  # noqa: E731
    sm = difflib.SequenceMatcher(None, [erase(x) for x in tn], [erase(x) for x in cn], autojunk=False)
    shape = naming = 0
    ops = sm.get_opcodes()
    for op, a1, a2, b1, b2 in ops:
        if op == "equal":
            naming += sum(tn[a1 + k] != cn[b1 + k] for k in range(a2 - a1))
        else:
            shape += max(a2 - a1, b2 - b1)
    return ops, tn, cn, shape, naming


def saved_defs(rows) -> list[str]:
    out = []
    for _, ins in rows:
        parts = ins.split(None, 1)
        if len(parts) < 2 or parts[0] in STORES or parts[0].startswith("b") or parts[0] in ("j", "jal", "jr", "jalr"):
            continue
        ops = [o.strip() for o in parts[1].split(",")]
        if ops[0] in SAVED:
            rest = re.sub(r"-?0x[0-9A-Fa-f]+|-?\b\d+\b", lambda n: str(int(n.group(0), 0)), ",".join(ops[1:]))
            out.append(f"{ops[0][1:]}={parts[0]} {rest}")
    return out


def target_table(listing_text: str) -> list[int]:
    """Handler start vrams of the largest jump table the listing loads."""
    best: list[int] = []
    for name in sorted(set(re.findall(r"%lo\((jtbl_\w+)\)", listing_text))):
        for path in glob.glob(str(ROOT / "asm/data/*.s")):
            text = Path(path).read_text()
            i = text.find(f"dlabel {name}\n")
            if i < 0:
                continue
            end = text.find("enddlabel", i)
            table = [int(x, 16) for x in re.findall(r"\.word \.L([0-9A-Fa-f]+)", text[i:end])]
            if len(table) > len(best):
                best = table
    return best


def candidate_table(obj: Path, symbol: str, length: int, objdump: str) -> list[int]:
    with tempfile.TemporaryDirectory() as tmp:
        ro = Path(tmp) / "ro.bin"
        subprocess.run([objdump.replace("objdump", "objcopy"), "-O", "binary", "-j", ".rodata", str(obj), str(ro)],
                       check=True)
        data = ro.read_bytes()
    rel = subprocess.run([objdump, "-r", "-j", ".rodata", str(obj)], capture_output=True, text=True).stdout
    offs = [int(m.group(1), 16) for m in re.finditer(r"^([0-9a-f]+)\s+R_MIPS_32\s+\.text\s*$", rel, re.M)]
    syms = subprocess.run([objdump, "-t", str(obj)], capture_output=True, text=True).stdout
    fbase = next(int(l.split()[0], 16) for l in syms.splitlines() if l.endswith(" " + symbol))
    run: list[int] = []
    for o in offs:
        if run and o != run[-1] + 4:
            if len(run) == length:
                break
            run = []
        run.append(o)
    if len(run) != length:
        raise SystemExit(f"no {length}-entry jump table in the candidate's .rodata")
    return [int.from_bytes(data[o:o + 4], "big") - fbase for o in run]


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("symbol")
    ap.add_argument("candidate")
    ap.add_argument("--case", nargs="*", default=None, help="side-by-side for these case values")
    ap.add_argument("--roles", action="store_true", help="saved-register role table")
    ap.add_argument("--all", action="store_true", help="summary rows for exact handlers too")
    ns = ap.parse_args(argv)

    import fast_score
    source = fast_score.tracked_source_for(ns.symbol)
    listing = fast_score.target_listing_for(ns.symbol, (ROOT / source).read_text())
    if not listing or not (ROOT / listing).is_file():
        raise SystemExit(f"no extracted listing for {ns.symbol}; run gmake extract")
    ltext = (ROOT / listing).read_text()
    base, trows = parse_listing(ltext)
    table = target_table(ltext)
    if not table:
        raise SystemExit("no jump table found for the target")
    objdump = str(ROOT / "tools/binutils/mips64-elf-objdump")
    with tempfile.TemporaryDirectory(prefix="handler-diff-") as tmp:
        obj = Path(tmp) / "cand.o"
        r = subprocess.run([sys.executable, str(ROOT / "tools/fast_score.py"), ns.symbol, ns.candidate,
                            "--keep-object", str(obj)], cwd=ROOT, capture_output=True, text=True)
        if r.returncode:
            sys.stderr.write(r.stdout + r.stderr)
            return 1
        print(r.stdout.splitlines()[0])
        dis = subprocess.run([objdump, "-d", "-r", "--no-show-raw-insn", "-M", "reg-names=numeric,no-aliases", str(obj)],
                             capture_output=True, text=True).stdout
        crows = parse_objdump(dis, ns.symbol)
        cstarts = candidate_table(obj, ns.symbol, len(table), objdump)
    treg = regions([v - base for v in table])
    creg = regions(cstarts)
    want = None if ns.case is None else {int(c, 16) for c in ns.case}
    seen, total = set(), 0
    for i, tspan in enumerate(treg):
        if tspan[0] in seen:
            continue
        seen.add(tspan[0])
        cases = [j for j in range(len(treg)) if treg[j][0] == tspan[0]]
        label = ",".join(hex(c) for c in cases[:4]) + (".." if len(cases) > 4 else "")
        t, c = rows_in(trows, tspan), rows_in(crows, creg[i])
        ops, tn, cn, shape, naming = compare(t, c)
        total += shape + naming
        if ns.roles:
            td, cd = saved_defs(t), saved_defs(c)
            if td or cd:
                print(f"{label:<14} T: {' | '.join(td)}")
                if td != cd:
                    print(f"{'':<14} C: {' | '.join(cd)}")
        elif want is None:
            if ns.all or shape or naming or len(t) != len(c):
                print(f"{label:<24} t={len(t):3d} c={len(c):3d} dlen={len(c) - len(t):+d} "
                      f"shape={shape} naming={naming}")
        elif want & set(cases):
            print(f"==== case {label}")
            for op, a1, a2, b1, b2 in ops:
                for k in range(max(a2 - a1, b2 - b1)):
                    tl = f"{t[a1 + k][0]:04x} {tn[a1 + k]}" if a1 + k < a2 else ""
                    cl = f"{c[b1 + k][0]:04x} {cn[b1 + k]}" if b1 + k < b2 else ""
                    same = a1 + k < a2 and b1 + k < b2 and tn[a1 + k] == cn[b1 + k]
                    mark = " " if same else ("~" if op == "equal" else "!")
                    print(f"{mark} {tl:<44} {cl}")
    if want is None and not ns.roles:
        print(f"total per-handler shape+naming {total}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
