#!/usr/bin/env python3
"""Per-web allocator report: why each web of one function got its colour.

    tools/web_report.py <symbol> [--proc N] [--web W ...] [--block B]
                        [--trace allocator.log] [--keep DIR] [--json]

WHAT IT PRINTS, PER WEB

1. Identity: the web's value as an expression over the function's own locals
   (`(col & 7)`, `index`, `*(grid + 0)`), whether it is a symbol (a local,
   parameter or register pseudo) or an expression temporary, its priority
   (`save`), `nocs`, `totalsave`, and the p1/p2 decision: colour, the order it
   was offered a colour in, and the `forced` state of both records.
2. References: one row per occurrence (a block the web is used or defined in),
   with the block's uses, defs, loop weight and term, the source lines the
   block spans, and which of those lines name the web's variables. The p1dec
   record prints only the total; this is the breakdown that says which
   reference to add or remove to move it. `gross - chargeA - chargeB` is
   checked against `totalsave`.
3. Splits: for a web the pass split, every growth test of every piece -- block,
   `new`, `left_before`, `left_after`, `numintf`, verdict -- re-checked against
   the L161 rule (accept iff new < left_before and 2*left_after >= numintf +
   new), with the first refused block called out.
4. The forbidden seed: per block of the web, the register mask folded into its
   forbidden set (`forbidden0` before any neighbour is coloured), and for each
   bit the precoloured value that put it there -- a parameter, a call
   argument that is a web, a call result held over its range -- or, when no
   web owns it, that it is a register bound outside any web (a constant or
   address call argument). Bits in the decision's `forbidden0` beyond the seed
   are neighbours' colours.

HOW

One compile of the symbol's TU with the instrumented toolchain
(`~/Desktop/dev/ido-instrumented`, command taken from the build, never typed),
`CDX_LOG=1 CDX_DETAIL_WEB=all CDX_WEBREPORT=1`. `CDX_WEBREPORT` is the
switch for the records this tool reads (`bbline`, `bbpin`, `rangepin`,
`saveocc`, `savedetail`, `webexpr`, `forbidseed`); with it unset the compiler's
object, stderr and every other record are byte-identical to the profile
without it. A stock compile is made beside it and the two `.text` sections are
compared: the report refuses to print over a failed identity gate. Names come
from a third, `-g3` stock compile's `.mdebug` locals (cfe assigns frame
offsets before optimisation, so they are the offsets uopt's records carry).

Without `--proc` the procedure is found by itself: every procedure is logged
and the one whose blocks carry the symbol's definition line is taken.

`--source FILE` reports a candidate C file in place of the tracked TU (as the
other tools' `--candidate` do); `DKWB_CUT_*` in the environment reach the
instrumented compile, and the identity gate then compares it with an
instrumented compile that has the same cuts and no `CDX_*` logging.

`--trace` reads a saved log instead of compiling (no gate, names or source
lines unless the symbol resolves in the queue); this is how the tests run.
"""
from __future__ import annotations

import argparse
import collections
import json
import os
import pathlib
import re
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

FIELD_RE = re.compile(r"(\w+)=(\S+)")
WEBREPORT_RECORDS = ("bbline", "bbpin", "rangepin", "saveocc", "savedetail",
                     "webexpr", "forbidseed")
COLOURS = {1: "v0", 2: "v1", 3: "a0", 4: "a1", 5: "a2", 6: "a3", 7: "t0",
           8: "t1", 9: "t2", 10: "t3", 11: "t4", 12: "t5", 14: "s0", 15: "s1",
           16: "s2", 17: "s3", 18: "s4", 19: "s5", 20: "s6", 21: "s7",
           22: "s8", 23: "ra"}
GPR = ("zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2",
       "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6",
       "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra")
INFIX = {"add": "+", "sub": "-", "mpy": "*", "div": "/", "and": "&",
         "ior": "|", "xor": "^", "shl": "<<", "shr": ">>", "rem": "%",
         "mod": "%", "equ": "==", "neq": "!=", "les": "<", "leq": "<=",
         "grt": ">", "geq": ">="}
KINDS = {1: "address", 2: "constant", 3: "symbol", 4: "expression temporary",
         5: "address", 6: "symbol", 8: "constant"}


def colour_name(colour: int) -> str:
    return COLOURS.get(colour, f"c{colour}")


def bits(mask: int) -> list[int]:
    """Colours set in a forbidden0-style word: colour c is bit 31 - c."""
    return [c for c in range(1, 32) if (mask >> (31 - c)) & 1]


def opcode_names() -> tuple[str, ...]:
    try:
        from decomp_workbench.ucode import OPCODE_NAMES
        return tuple(OPCODE_NAMES)
    except Exception:  # noqa: BLE001 - names are a convenience
        return ()


# ---------------------------------------------------------------- records

def parse_records(text: str) -> list[tuple[str, dict]]:
    rows = []
    for raw in text.splitlines():
        if not raw.startswith("[CDX] "):
            continue
        parts = raw.split(None, 2)
        if len(parts) < 2:
            continue
        rows.append((parts[1], dict(FIELD_RE.findall(raw))))
    return rows


def procs_with_lines(rows) -> dict[int, set[int]]:
    found: dict[int, set[int]] = collections.defaultdict(set)
    for event, f in rows:
        if event == "bbline" and f.get("lines", "-") != "-":
            found[int(f["proc"])].update(int(x) for x in f["lines"].split(","))
    return found


def build(rows, proc: int) -> dict:
    """Join one procedure's records into decisions with their evidence."""
    p = str(proc)
    blocks, pins, rangepins = {}, collections.defaultdict(list), collections.defaultdict(list)
    decisions: list[dict] = []
    colours: dict[tuple, list] = collections.defaultdict(list)
    saves: dict[str, list] = collections.defaultdict(list)    # lr -> [(index, detail, occs)]
    forbid: dict[str, list] = collections.defaultdict(list)   # lr -> [(index, row)]
    growth: list[tuple[int, str, dict]] = []
    pending_occ: dict[str, list] = collections.defaultdict(list)
    for index, (event, f) in enumerate(rows):
        if f.get("proc") != p:
            continue
        if event == "bbline":
            blocks[int(f["bb"])] = {
                "weight": int(f["weight"]), "entry": int(f["entry"]),
                "lines": [] if f["lines"] == "-" else [int(x) for x in f["lines"].split(",")],
                "mask1": int(f["mask1"].split(",")[0], 16),
                "mask2": int(f["mask2"].split(",")[0], 16)}
        elif event == "bbpin":
            pins[int(f["bb"])].append(f)
        elif event == "rangepin":
            rangepins[int(f["bb"])].append(f)
        elif event == "saveocc":
            pending_occ[f["lr"]].append(f)
        elif event == "savedetail":
            saves[f["lr"]].append((index, f, pending_occ.pop(f["lr"], [])))
        elif event == "forbidseed":
            forbid[f["lr"]].append((index, f))
        elif event in ("p1dec", "p2dec"):
            decisions.append({"index": index, "phase": event[:2], "dec": f,
                              "web": int(f["web"])})
        elif event in ("webexpr", "webblocks", "webdetail") and decisions:
            last = decisions[-1]
            if f.get("web") == str(last["web"]) and f.get("phase") == last["phase"] \
                    and f.get("role", "target") == "target":
                last[event] = f
        elif event in ("p1color", "p2color"):
            colours[(event[:2], int(f["web"]))].append((index, f))
        elif event in ("seed", "seedcand", "grow", "growv", "livbb"):
            growth.append((index, event, f))
    for order, d in enumerate(decisions, 1):
        d["order"] = order
        lr = d.get("webexpr", {}).get("lr")
        d["lr"] = lr
        d["save"] = None
        for idx, detail, occs in saves.get(lr, []):
            if idx < d["index"]:
                d["save"] = (detail, occs)
        following = decisions[order]["index"] if order < len(decisions) else len(rows)
        later = [c for c in colours.get((d["phase"], d["web"]), [])
                 if d["index"] < c[0] < following]
        d["color"] = later[0][1] if later else None
        previous = max([e["index"] for e in decisions
                        if e.get("webexpr", {}).get("lr") == lr and e["index"] < d["index"]],
                       default=-1)
        d["forbid"] = [r for i, r in forbid.get(lr, []) if previous < i < d["index"]]
    # growth belongs to the split decision that precedes it
    for d in decisions:
        d["growth"] = []
    split_decs = [d for d in decisions if d["dec"].get("decision") == "split"]
    for index, event, f in growth:
        owner = None
        for d in split_decs:
            if d["index"] < index:
                owner = d
        if owner is not None:
            owner["growth"].append((event, f))
    pieces = {d["lr"]: d for d in decisions}
    return {"proc": proc, "blocks": blocks, "pins": pins, "rangepins": rangepins,
            "decisions": decisions, "pieces": pieces}


# ---------------------------------------------------------------- naming

class Namer:
    def __init__(self, locals_by_offset: dict[int, list[str]] | None = None,
                 params_by_offset: dict[int, str] | None = None):
        self.locals = locals_by_offset or {}
        self.params = params_by_offset or {}
        self.ops = opcode_names()

    def var(self, off: int, storage: int) -> str:
        if storage == 1 and off in self.locals:
            return "/".join(self.locals[off])
        if storage == 2 and off in self.params:
            return self.params[off]
        if storage == 3 and 0 <= off < len(GPR):
            return f"${GPR[off]}"
        if storage == 3 and 32 <= off < 64:
            return f"$f{off - 32}"
        return {1: f"auto[{off}]", 2: f"param[{off}]"}.get(storage, f"var[{off}:{storage}]")

    def variables(self, expr: str) -> list[str]:
        names = []
        for off, storage in re.findall(r"var:(-?\d+):(\d+):\d+", expr):
            name = self.var(int(off), int(storage))
            names.extend(n for n in name.split("/") if re.fullmatch(r"\w+", n))
        return sorted(set(names))

    def render(self, expr: str) -> str:
        text, pos = expr, 0

        def parse() -> str:
            nonlocal pos
            m = re.match(r"op(\d+)\(", text[pos:])
            if m:
                pos += m.end()
                args = [parse()]
                while text[pos] == ",":
                    pos += 1
                    args.append(parse())
                pos += 1  # ')'
                code = int(m.group(1))
                name = self.ops[code] if code < len(self.ops) else f"op{code}"
                if name in INFIX and len(args) == 2:
                    return f"({args[0]} {INFIX[name]} {args[1]})"
                if name == "ilod":
                    off = args[1][1:] if len(args) > 1 and args[1].startswith("@") else "0"
                    return f"*({args[0]} + {off})" if off != "0" else f"*{args[0]}"
                if name == "cvt":
                    return f"(cvt){args[0]}"
                if name == "ixa" and len(args) == 2:
                    return f"{args[0]}[{args[1]}]"
                return f"{name}({', '.join(args)})"
            m = re.match(r"(s?var):(-?\d+):(\d+):(\d+)", text[pos:])
            if m:
                pos += m.end()
                return self.var(int(m.group(2)), int(m.group(3)))
            m = re.match(r"@(-?\d+)", text[pos:])
            if m:
                pos += m.end()
                return m.group(0)
            m = re.match(r"const:(-?\d+)", text[pos:])
            if m:
                pos += m.end()
                return m.group(1)
            m = re.match(r"k[15]:0x([0-9a-f]+)", text[pos:])
            if m:
                pos += m.end()
                return f"&[0x{m.group(1)}]"
            m = re.match(r"[^,()]+", text[pos:])
            pos += m.end() if m else 1
            return m.group(0) if m else "?"
        try:
            return parse()
        except (IndexError, AttributeError):
            return expr


def mdebug_frame_names(obj: pathlib.Path, symbol: str):
    """(locals by frame offset, params by offset) for one procedure's .mdebug."""
    data = obj.read_bytes()
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from(">HHH", data, 0x2E)
    secs = [struct.unpack_from(">IIIIIIIIII", data, shoff + i * shentsize)
            for i in range(shnum)]
    names = secs[shstrndx][4]

    def secname(sec):
        start = names + sec[0]
        return data[start:data.index(b"\0", start)].decode()
    md = next((s for s in secs if secname(s) == ".mdebug"), None)
    if md is None:
        return {}, {}
    h = struct.unpack_from(">hh" + "I" * 23, data, md[4])
    isym_off, ss_off, ifd_max, fd_off = h[10], h[16], h[19], h[20]
    locals_: dict[int, list[str]] = collections.defaultdict(list)
    params: dict[int, str] = {}
    for f in range(ifd_max):
        fd = struct.unpack_from(">IIIIII", data, fd_off + f * 72)
        iss_base, isym_base, csym = fd[2], fd[4], fd[5]
        inside = False
        for k in range(csym):
            iss, value, word = struct.unpack_from(">IiI", data, isym_off + (isym_base + k) * 12)
            st = word >> 26
            start = ss_off + iss_base + iss
            name = data[start:data.index(b"\0", start)].decode(errors="replace")
            if st in (6, 14):  # stProc, stStaticProc
                inside = name == symbol
            elif st == 8 and name == symbol:  # stEnd of the procedure
                inside = False
            elif inside and st == 4:
                locals_[value].append(name)
            elif inside and st == 3:
                params[value] = name
    return dict(locals_), params


def read_text_section(obj: pathlib.Path) -> bytes:
    data = obj.read_bytes()
    shoff, = struct.unpack_from(">I", data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from(">HHH", data, 0x2E)
    secs = [struct.unpack_from(">IIIIIIIIII", data, shoff + i * shentsize)
            for i in range(shnum)]
    names = secs[shstrndx][4]
    for s in secs:
        start = names + s[0]
        if data[start:data.index(b"\0", start)] == b".text":
            return data[s[4]:s[4] + s[5]]
    return b""


# ---------------------------------------------------------------- report

def block_lines(info: dict | None) -> str:
    if not info:
        return "?"
    lines = sorted(set(info["lines"]))
    if not lines:
        return f"(no line of its own; in effect {info['entry']})"
    if lines == list(range(lines[0], lines[-1] + 1)) and len(lines) > 2:
        return f"{lines[0]}-{lines[-1]}"
    return ",".join(map(str, lines))


def naming_lines(info: dict | None, names: list[str], source: list[str]) -> list[int]:
    if not info or not names or not source:
        return []
    pattern = re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\b")
    return [n for n in sorted(set(info["lines"]))
            if 0 < n <= len(source) and pattern.search(source[n - 1])]


def grow_rule(row: dict) -> bool:
    new, before = int(row["new"]), int(row["left_before"])
    after, numintf = int(row["left_after"]), int(row["numintf"])
    if row.get("strict", "1") == "0":
        return after > 0
    return new < before and 2 * after >= numintf + new


def seed_sources(model: dict, bb: int, colour: int, namer: Namer) -> str:
    found = []
    for pin in model["pins"].get(bb, []):
        if int(pin["color"]) == colour:
            found.append(f"web {pin['web']} {namer.render(pin['expr'])}")
    for pin in model["rangepins"].get(bb, []):
        if colour in bits(int(pin["mask0"], 16)):
            found.append(f"{namer.render(pin['expr'])} held over its range")
    return "; ".join(found) or "no web (a constant or address bound to the register)"


def report_web(model: dict, d: dict, namer: Namer, source: list[str],
               block_filter: int | None) -> list[str]:
    f, out = d["dec"], []
    expr = d.get("webexpr", {})
    kind = KINDS.get(int(expr.get("kind", -1)), "unknown")
    rendered = namer.render(expr.get("expr", "?"))
    out.append(f"web {d['web']} {d['phase']}  {rendered}  [{kind}]   decision {d['order']} of "
               f"{len(model['decisions'])}")
    colour = d["color"]
    chosen = (f"{colour['reg']} (c{colour['color']})" if colour else "none (memory)")
    out.append(f"  priority save {float(f['save']):.3f}  nocs {f['nocs']}  totalsave "
               f"{float(f['totalsave']):.3f}  bestcost {float(f['bestcost']):.3f}  numintf "
               f"{f['numintf']}  regsleft {f['regsleft']}")
    out.append(f"  {f['decision']} -> {chosen}   forced: dec={f['forced']} color="
               f"{colour['forced'] if colour else '-'}  (-2 never forced, -1 accepted)")
    if d["save"] is None:
        out.append("  references: no savedetail record (compiler without CDX_WEBREPORT?)")
    else:
        detail, occs = d["save"]
        out.append(f"  references: gross {float(detail['gross']):g} - chargeA "
                   f"{float(detail['chargeA']):g} - chargeB {float(detail['chargeB']):g} = net "
                   f"{float(detail['net']):g}  (/ nocs {detail['nocs']} = save "
                   f"{float(detail['save']):g})")
        names = namer.variables(expr.get("expr", ""))
        for occ in occs:
            bb = int(occ["bb"])
            if block_filter is not None and bb != block_filter:
                continue
            info = model["blocks"].get(bb)
            flags = [k for k in ("nl", "o22", "o23") if occ.get(k) == "1"]
            naming = naming_lines(info, names, source)
            out.append(f"    bb{bb:<3} x{float(occ['weight']):<6g} uses {occ['uses']} defs "
                       f"{occ['defs']} -> {float(occ['term']):g}   lines {block_lines(info)}"
                       + (f"   naming {'/'.join(names)}: {','.join(map(str, naming))}" if naming else "")
                       + (f"   [{' '.join(flags)}]" if flags else ""))
    seed = 0
    rows = [r for r in d["forbid"] if block_filter is None or int(r["bb"]) == block_filter]
    contributions = collections.defaultdict(list)
    for r in d["forbid"]:
        own = int(r["own"])
        mask = int(r["mask0"], 16) & ~((1 << (31 - own)) if 0 < own < 32 else 0)
        seed |= mask
        for c in bits(mask):
            contributions[c].append(int(r["bb"]))
    decided = int(f["forbidden0"], 16)
    out.append(f"  forbidden seed 0x{seed:08x}  at decision 0x{decided:08x}"
               f"  (neighbours add {' '.join(colour_name(c) for c in bits(decided & ~seed)) or 'nothing'})")
    for c, bbs in sorted(contributions.items()):
        shown = [b for b in bbs if block_filter is None or b == block_filter]
        if shown:
            out.append(f"    {colour_name(c)} from " + ", ".join(
                f"bb{b} <- {seed_sources(model, b, c, namer)}" for b in shown))
    if block_filter is not None and not rows:
        out.append(f"    (web is not live in bb{block_filter})")
    if d["growth"]:
        out.append("  split growth (accept iff new < left_before and 2*left_after >= numintf+new):")
        piece, first_refused, pending = None, None, None
        for event, g in d["growth"]:
            if event == "seed":
                piece = g["lr"]
                target = model["pieces"].get(piece)
                out.append(f"    piece {piece} seeded at bb{g['bb']}" + (
                    f" -> web {target['web']} {target['dec']['decision']}"
                    + (f" {target['color']['reg']}" if target and target['color'] else "")
                    if target else ""))
            elif event == "grow":
                pending = g
            elif event == "growv" and pending is not None:
                g0, pending = pending, None
                verdict = g["accepted"] == "1"
                rule = grow_rule(g0)
                if not verdict and first_refused is None:
                    first_refused = (g0["lr"], g0["bb"])
                if block_filter is None or int(g0["bb"]) == block_filter:
                    out.append(f"      bb{g0['bb']:<3} new {g0['new']} left {g0['left_before']}->"
                               f"{g0['left_after']} numintf {g0['numintf']}  "
                               f"{'accepted' if verdict else 'REFUSED'}"
                               + ("" if rule == verdict else "  (rule disagrees: read the record)"))
        if first_refused:
            out.append(f"    first refused block: bb{first_refused[1]} (piece {first_refused[0]})")
    return out


def render(model: dict, namer: Namer, source: list[str], webs: set[int] | None,
           block_filter: int | None, header: str) -> str:
    out = [header]
    if block_filter is not None:
        info = model["blocks"].get(block_filter)
        out.append(f"bb{block_filter}: weight {info['weight'] if info else '?'}, lines "
                   f"{block_lines(info)}, pinned registers "
                   + ((" ".join(colour_name(c) for c in bits(info["mask1"] | info["mask2"]))
                       or "none") if info else "?"))
    for d in model["decisions"]:
        if webs and d["web"] not in webs:
            continue
        out.append("")
        out.extend(report_web(model, d, namer, source, block_filter))
    return "\n".join(out) + "\n"


# ---------------------------------------------------------------- compiling

def clean_env(environ) -> dict:
    """The environment with every CDX_*/DKWB_* variable removed."""
    return {k: v for k, v in environ.items() if not k.startswith(("CDX_", "DKWB_"))}


def cut_env(environ) -> dict:
    """The `DKWB_CUT_*` variables, which pick a cut-forced compile and must reach it."""
    return {k: v for k, v in environ.items() if k.startswith("DKWB_CUT_") and v}


def with_source(command: list[str], candidate: pathlib.Path | None) -> list[str]:
    """Replace the command's source argument (its last word) with `candidate`."""
    actual = list(command)
    if candidate is not None:
        actual[-1] = str(candidate)
    return actual


def compile_tu(symbol: str, work: pathlib.Path, proc: str,
               candidate: pathlib.Path | None = None):
    """Instrumented, stock and `-g3` names compiles of the TU (or of `candidate`).

    `DKWB_CUT_*` reach the instrumented compile. A stock compiler ignores them,
    so with a cut set the identity gate compares the instrumented object with a
    second instrumented compile (same cuts, no CDX_* logging) in the `stock`
    slot: the gate then proves the logging is inert, not that the cut is.
    """
    import force_lattice as fl
    command = with_source(fl.compile_command(symbol), candidate)
    env = clean_env(os.environ)
    cuts = cut_env(os.environ)
    runs = {}
    for label in ("instrumented", "stock", "names"):
        actual = (fl.replace_compiler(command, fl.INSTRUMENTED / "cc")
                  if label == "instrumented" or (label == "stock" and cuts) else list(command))
        out = work / f"{label}.o"
        actual[actual.index("-o") + 1] = str(out)
        run_env = dict(env)
        if label == "instrumented" or (label == "stock" and cuts):
            run_env.update(cuts)
        if label == "instrumented":
            run_env.update(CDX_LOG="1", CDX_PROC=proc, CDX_DETAIL_WEB="all",
                           CDX_WEBREPORT="1", CDX_OUT=str(work / "allocator.log"))
        if label == "names" and "-g3" not in actual:
            actual.insert(actual.index("-o"), "-g3")
        result = subprocess.run(actual, env=run_env, capture_output=True, text=True,
                                cwd=fl.ROOT, timeout=900)
        if result.returncode and label != "names":
            raise SystemExit(f"web_report: {label} compile failed (exit "
                             f"{result.returncode}):\n{result.stderr[-2000:]}")
        runs[label] = out if out.is_file() else None
    return runs


def source_for(symbol: str, candidate: pathlib.Path | None = None
               ) -> tuple[list[str], tuple[int, int] | None]:
    """The TU's lines (or `candidate`'s) and the symbol's definition span (signature to closing brace)."""
    try:
        import permute_batch as pb
        import force_lattice as fl
        items = [i for i in pb.discover_queue() if i.func == symbol]
        if not items:
            return [], None
        lines = (candidate or fl.ROOT / items[0].rel_c_file).read_text().splitlines()
    except Exception:  # noqa: BLE001
        return [], None
    return lines, definition_span(lines, symbol)


def definition_span(lines: list[str], symbol: str) -> tuple[int, int] | None:
    """Signature line to closing brace of `symbol`'s definition, 1-based.

    A prototype is skipped even when its parameter list runs over several
    lines: the line that closes the list ends in `;` (anim.c declares
    func_80054B3C that way above func_80053868, and taking it for the
    definition handed the caller func_80053868's procedure)."""
    pattern = re.compile(r"^\S.*\b" + re.escape(symbol) + r"\s*\(")
    for number, line in enumerate(lines, 1):
        if not pattern.search(line) \
                or line.lstrip().startswith(("#", "//", "*", "extern ")):
            continue
        depth, close = 0, number
        for n in range(number, len(lines) + 1):
            depth += lines[n - 1].count("(") - lines[n - 1].count(")")
            close = n
            if depth <= 0:
                break
        if lines[close - 1].rstrip().endswith(";"):
            continue
        end = next((n for n in range(close, len(lines) + 1)
                    if lines[n - 1].startswith("}")), len(lines))
        return (number, end)
    return None


def choose_proc(rows, span: tuple[int, int] | None) -> int | None:
    """The procedure whose first source line falls inside the symbol's body."""
    found = procs_with_lines(rows)
    if span:
        hits = [p for p, lines in found.items() if span[0] <= min(lines) <= span[1]]
        if len(hits) == 1:
            return hits[0]
    if len(found) == 1:
        return next(iter(found))
    return None


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("symbol")
    parser.add_argument("--proc", type=int, help="procedure ordinal (found if omitted)")
    parser.add_argument("--web", type=int, action="append", help="only this web (repeatable)")
    parser.add_argument("--block", type=int, help="only this block's rows")
    parser.add_argument("--source", type=pathlib.Path,
                        help="report this candidate C file in place of the tracked TU")
    parser.add_argument("--trace", help="read a saved CDX_WEBREPORT log instead of compiling")
    parser.add_argument("--keep", help="keep the compiles and the log in this directory")
    parser.add_argument("--json", action="store_true", help="print the joined model as JSON")
    args = parser.parse_args(argv)

    candidate = args.source.resolve() if args.source else None
    if candidate is not None and not candidate.is_file():
        raise SystemExit(f"web_report: no such --source: {candidate}")
    source, span = source_for(args.symbol, candidate)
    namer = Namer()
    gate = "not run (--trace)"
    if args.trace:
        text = pathlib.Path(args.trace).read_text()
    else:
        work = pathlib.Path(args.keep) if args.keep else pathlib.Path(tempfile.mkdtemp())
        work.mkdir(parents=True, exist_ok=True)
        runs = compile_tu(args.symbol, work, str(args.proc) if args.proc is not None else "all",
                           candidate)
        text = (work / "allocator.log").read_text()
        if read_text_section(runs["instrumented"]) != read_text_section(runs["stock"]):
            print("web_report: IDENTITY GATE FAILED -- the instrumented .text differs from "
                  "the stock compile; no reading is trustworthy.", file=sys.stderr)
            return 2
        gate = ".text identical to stock"
        if runs["names"]:
            namer = Namer(*mdebug_frame_names(runs["names"], args.symbol))
    rows = parse_records(text)
    if not any(event in WEBREPORT_RECORDS for event, _ in rows):
        print("web_report: the log has no CDX_WEBREPORT records; the instrumented uopt "
              "predates them (see docs/LANE_BRIEF.md, Instruments).", file=sys.stderr)
        return 2
    proc = args.proc if args.proc is not None else choose_proc(rows, span)
    if proc is None:
        print("web_report: cannot tell which procedure is the symbol; pass --proc. "
              f"Procedures with lines: {sorted(procs_with_lines(rows))}", file=sys.stderr)
        return 2
    model = build(rows, proc)
    if not model["decisions"]:
        print(f"web_report: no decisions for proc {proc}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps({k: v for k, v in model.items() if k != "pieces"},
                         default=str, indent=1))
        return 0
    header = (f"{args.symbol}  proc {proc}  {len(model['decisions'])} decisions  "
              f"identity gate: {gate}")
    sys.stdout.write(render(model, namer, source, set(args.web or []), args.block, header))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
