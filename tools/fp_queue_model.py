#!/usr/bin/env python3
"""IDO's ugen floating-point temporary queue, as a checkable model.

Within one procedure, IDO ugen hands single-precision expression temporaries
out of a first-in, first-out queue of caller-saved even FP registers:

  * a result takes the register at the head of the queue, *before* any of its
    operands are given back;
  * an operand is given back -- appended to the tail -- at its last use, and
    when one instruction releases two operands they go back in either order
    (the model tries both);
  * a store is a use, so a stored value goes back after its store, and a
    value read by several instructions (a reloaded formal, say) goes back at
    the last of them;
  * a formal moved in with mtc1 is uopt's: its register is out of the queue
    for the whole procedure, as is any register uopt colours a local into.

The queue registers are $f4 $f6 $f8 $f10 $f16 $f18, initially in that order
less the ones mtc1 formals hold.  A temporary in any other register is a
uopt-coloured local and takes no part.  uopt colours locals into $f0 and $f2
before any queue register, so a routine that writes neither has no coloured
local and every non-formal FP value in it is a queue temporary.

Measured against IDO 5.3 and 7.1 compiles of generated C (``--validate``),
this reproduces every compile's FP register fields, given the compile's own
instructions and some evaluation order its data flow allows; a compile that
gave a local a queue register (that register then sits out) also wrote $f0
or $f2 every time; and applying the hand edit below to every compile that
writes neither -- delete each row's translation load and add, store the row
sum -- leaves a graph that fits no order, every time.

Because register numbers are fixed before the assembler schedules, a routine
the compiler produced must fit the model for *some* evaluation order of its
own values.  A routine with no coloured local that fits no order, from any
queue start order, did not come from this allocator with these instructions;
if it fits exactly once extra values are inserted, those values were compiled
and later deleted.

Modes (exit status: 0 the verdict holds, 1 it does not, 2 cannot run):

    tools/fp_queue_model.py --rom func_8002B040
        Read the routine's words from baseroms/mickey.us.z64 at run time, and
        show (a) no evaluation order fits, from any of the queue start orders
        tried, and (b) it fits once each stored row sum is followed by one
        extra loaded value and one extra sum (the deleted translation load and
        add).  Exit 0 when (a) and (b) both hold.

    tools/fp_queue_model.py --reference
        The Banjo-Kazooie ultralib guMtxXFMF object (located through
        tools/reference-builds.lock under $REFS_ROOT, digest checked) fits
        with no extra values; when the baserom is present, also say where
        the object occurs whole in it.  Exit 0 when it fits.

    tools/fp_queue_model.py --validate [--quick] [--jobs N]
        Generate C forms into a temporary directory, compile them with the
        project's IDO 5.3 and, when present, an IDO 7.1 recomp, and confirm
        the three measured properties above.  Exit 0 when all three hold.
        -O1 is out of scope: without uopt, ugen reuses cached memory
        contents past a value's last use.

IDO 7.1 is looked for in --ido71 DIR, then $IDO71_DIR, then the pinned
reference farm's copy (<refs>/perfect_dark/build/recomp/7.1).  The decompals
release binaries in that farm carry cc/cfe/uopt/ugen/as1 only, so -O3 (which
runs ujoin, uld, usplit and umerge) cannot run from them; a complete recomp
set such as dinosaur-planet's (<dp64>/tools/ido_static_recomp/7.1) can.  A
compiler set that cannot run a flag set is skipped and reported, not failed.

Output carries counts and register numbers only: no instruction text and no
machine words.  Nothing is written into the tree.
"""

from __future__ import annotations

import argparse
import itertools
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
QUEUE_REGS = (4, 6, 8, 10, 16, 18)
DEFAULT_REFS = os.environ.get(
    "REFS_ROOT", os.path.join(os.path.expanduser("~"), "Desktop", "dev", "decomp-refs"))
O3_HELPERS = ("ujoin", "uld", "usplit", "umerge")
SP = 29

# ----------------------------------------------------------------------------
# Decoding: only what the data flow needs.

OP_COP1, OP_LWC1, OP_SWC1, OP_LDC1, OP_SDC1 = 0x11, 0x31, 0x39, 0x35, 0x3D
FMT_S, FMT_D, FMT_W = 16, 17, 20
COP1_MT = 4
# funct -> (name, number of FP source operands)
FP_FUNCT = {0: ("add", 2), 1: ("sub", 2), 2: ("mul", 2), 3: ("div", 2),
            4: ("sqrt", 1), 5: ("abs", 1), 6: ("mov", 1), 7: ("neg", 1),
            0x20: ("cvt.s", 1), 0x21: ("cvt.d", 1), 0x24: ("cvt.w", 1)}


@dataclass(frozen=True)
class Op:
    """One decoded FP event: kind, destination register, source registers."""
    kind: str            # "mtc1" | "load" | "store" | "arith" | "other-fp"
    dest: int | None
    srcs: tuple
    base: int | None = None   # GPR base of a load/store
    offset: int | None = None


def decode(word: int) -> Op | None:
    """The FP data-flow event in one instruction word, or None if it has none."""
    op = word >> 26
    rs, rt = (word >> 21) & 31, (word >> 16) & 31
    imm = word & 0xFFFF
    if op in (OP_LWC1, OP_LDC1):
        return Op("load", rt, (), rs, imm - 0x10000 if imm & 0x8000 else imm)
    if op in (OP_SWC1, OP_SDC1):
        return Op("store", None, (rt,), rs, imm - 0x10000 if imm & 0x8000 else imm)
    if op != OP_COP1:
        return None
    if rs == COP1_MT:
        return Op("mtc1", (word >> 11) & 31, ())
    if rs in (FMT_S, FMT_D, FMT_W):
        fd, fs, ft, fn = (word >> 6) & 31, (word >> 11) & 31, rt, word & 63
        if fn in FP_FUNCT:
            _name, nsrc = FP_FUNCT[fn]
            return Op("arith", fd, (fs, ft)[:nsrc])
        return Op("other-fp", None, ())
    return Op("other-fp", None, ())


# ----------------------------------------------------------------------------
# Data flow.

@dataclass
class Node:
    dest: int | None          # register written; None for a store
    srcs: tuple               # node indices read
    kind: str                 # "formal" (uopt-owned), "temp", "store", "extra"
    row: int = -1             # store-delimited row, for reporting


class OutOfScope(Exception):
    """The routine reads an FP register it never defined, or similar."""


def dataflow(words) -> list[Node]:
    """FP value graph of a routine, in instruction order.

    A value moved in with mtc1 is a formal: uopt coloured it, and its
    register is out of the queue for the whole procedure.  Every other FP
    definition -- a stack reload of a formal included -- is a temporary.
    """
    nodes: list[Node] = []
    current: dict[int, int] = {}
    row = 0
    for w in words:
        d = decode(w)
        if d is None:
            continue
        if d.kind == "other-fp":
            raise OutOfScope("FP operation outside the modelled set")
        srcs = []
        for r in d.srcs:
            if r not in current:
                raise OutOfScope("reads $f%d before defining it" % r)
            srcs.append(current[r])
        if d.kind == "store":
            nodes.append(Node(None, tuple(srcs), "store", row))
            row += 1
            continue
        kind = "formal" if d.kind == "mtc1" else "temp"
        nodes.append(Node(d.dest, tuple(srcs), kind, row))
        current[d.dest] = len(nodes) - 1
    return nodes


def insert_extras(nodes: list[Node], rows=None) -> list[Node]:
    """Insert, before each chosen row's store, one extra loaded value and one
    extra sum of it and the stored value, and point the store at the sum --
    the shape of a compiled translation term (load, add) that was deleted.
    Extra values take whatever register the queue gives them."""
    out: list[Node] = []
    remap: dict[int, int] = {}
    for i, n in enumerate(nodes):
        srcs = tuple(remap[s] for s in n.srcs)
        if n.kind == "store" and (rows is None or n.row in rows) and len(srcs) == 1:
            out.append(Node(-1, (), "extra", n.row))
            t = len(out) - 1
            out.append(Node(-1, (t, srcs[0]), "extra", n.row))
            srcs = (len(out) - 1,)
        out.append(Node(n.dest, srcs, n.kind, n.row))
        remap[i] = len(out) - 1
    return out


# ----------------------------------------------------------------------------
# The queue model.

@dataclass
class Fit:
    order: list               # emission order (node indices)
    extra_regs: dict = field(default_factory=dict)   # node index -> register


def initial_queue(nodes, owned=()):
    """Queue registers less those formals (or uopt-owned locals) hold."""
    held = {n.dest for n in nodes if n.kind == "formal"} | set(owned)
    return tuple(r for r in QUEUE_REGS if r not in held)


def fits(nodes: list[Node], start=None, owned=(), limit_states=2_000_000) -> Fit | None:
    """An evaluation order that reproduces every temporary's register, or None.

    `start` overrides the initial queue order.  `owned` names queue registers
    uopt gave to locals: values written there are not queue temporaries and
    their register never enters the queue.
    """
    owned = set(owned)
    queue0 = tuple(start) if start is not None else initial_queue(nodes, owned)
    n_nodes = len(nodes)
    uses = [0] * n_nodes
    for n in nodes:
        for s in n.srcs:
            uses[s] += 1
    # A temporary outside the queue registers was coloured by uopt (a local
    # in $f0/$f2, say): it neither takes from nor returns to the queue.
    allocates = [n.kind == "extra" or (n.kind == "temp" and n.dest in QUEUE_REGS
                                       and n.dest not in owned) for n in nodes]
    returns = allocates
    remaining = list(uses)
    regs: dict = {}
    order: list = []
    seen: set = set()
    full = (1 << n_nodes) - 1

    def rec(queue, done):
        if done == full:
            return True
        key = (queue, done, tuple(sorted(regs.items())))
        if key in seen or len(seen) > limit_states:
            return False
        seen.add(key)
        for i, n in enumerate(nodes):
            if done >> i & 1 or not all(done >> s & 1 for s in n.srcs):
                continue
            q = list(queue)
            if allocates[i]:
                if not q:
                    continue
                head = q.pop(0)
                if n.kind == "extra":
                    regs[i] = head
                elif head != n.dest:
                    continue
            for s in n.srcs:
                remaining[s] -= 1
            freed = [regs.get(s, nodes[s].dest) for s in dict.fromkeys(n.srcs)
                     if remaining[s] == 0 and returns[s]]
            if allocates[i] and uses[i] == 0:      # a dead result goes straight back
                freed.append(regs.get(i, n.dest))
            ok = False
            for perm in sorted(set(itertools.permutations(freed))):
                order.append(i)
                if rec(tuple(q) + perm, done | (1 << i)):
                    ok = True
                    break
                order.pop()
            for s in n.srcs:
                remaining[s] += 1
            if ok:
                return True
            regs.pop(i, None)
        return False

    if rec(queue0, 0):
        return Fit(list(order), dict(regs))
    return None


def start_orders(nodes, extras=(0, 2)):
    """Every initial queue order over the queue registers formals leave free,
    with each optional extra register ($f0/$f2) inserted at every position."""
    base = initial_queue(nodes)
    for k in range(len(extras) + 1):
        for chosen in itertools.combinations(extras, k):
            for perm in itertools.permutations(base + chosen):
                yield perm


def delete_translations(nodes):
    """The hand edit, applied to a compiled graph: wherever a store reads a sum
    of a once-read load and another value, drop the load and the sum and store
    the other value.  None when no store has that shape."""
    uses = [0] * len(nodes)
    for n in nodes:
        for s in n.srcs:
            uses[s] += 1
    drop, repoint = set(), {}
    for i, n in enumerate(nodes):
        if n.kind != "store" or len(n.srcs) != 1:
            continue
        p = n.srcs[0]
        pn = nodes[p]
        if pn.kind != "temp" or len(pn.srcs) != 2 or uses[p] != 1:
            continue
        for t, other in (pn.srcs, pn.srcs[::-1]):
            tn = nodes[t]
            if tn.kind == "temp" and not tn.srcs and uses[t] == 1 and nodes[other].srcs:
                drop |= {t, p}
                repoint[i] = other
                break
    if not drop:
        return None
    index, out = {}, []
    for i, n in enumerate(nodes):
        if i in drop:
            continue
        srcs = (repoint[i],) if i in repoint else n.srcs
        out.append(Node(n.dest, tuple(index[s] for s in srcs), n.kind, n.row))
        index[i] = len(out) - 1
    return out


def explain(nodes):
    """How a compiled routine fits: 'queue', 'queue, uopt-owned $fN', or None."""
    if fits(nodes):
        return "queue"
    for r in QUEUE_REGS:
        if any(n.dest == r for n in nodes) and fits(nodes, owned=(r,)):
            return "queue, uopt-owned $f%d" % r
    return None


UOPT_FIRST = (0, 2)


def writes_uopt_first(nodes):
    """Does the routine write $f0 or $f2?  uopt colours locals there before
    it uses any queue register (``--validate`` checks this on every compile),
    so a routine that writes neither has no uopt-coloured local, and every
    one of its non-formal FP values is a queue temporary."""
    return any(n.dest in UOPT_FIRST for n in nodes)


# ----------------------------------------------------------------------------
# Inputs.

def read_symbol(symbol, version="us"):
    """(rom offset, size) of a symbol from symbol_addrs.<version>.txt."""
    path = os.path.join(ROOT_DIR, "symbol_addrs.%s.txt" % version)
    pat = re.compile(r"^\s*%s\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)$" % re.escape(symbol))
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            m = pat.match(line)
            if not m:
                continue
            vram, comment = int(m.group(1), 16), m.group(2)
            size = re.search(r"size:(0x[0-9A-Fa-f]+|\d+)", comment)
            rom = re.search(r"\bROM (0x[0-9A-Fa-f]+)", comment)
            if not size:
                raise SystemExit("fp-queue: %s has no size: in %s" % (symbol, os.path.basename(path)))
            if rom:
                offset = int(rom.group(1), 16)
            elif 0x80000400 <= vram < 0x80085A40:     # resident main: ROM 0x1000
                offset = vram - 0x80000400 + 0x1000
            else:
                raise SystemExit("fp-queue: %s has no ROM offset" % symbol)
            return offset, int(size.group(1), 0)
    raise SystemExit("fp-queue: %s not in %s" % (symbol, os.path.basename(path)))


def rom_words(rom_path, offset, size):
    with open(rom_path, "rb") as fh:
        fh.seek(offset)
        blob = fh.read(size)
    if len(blob) != size:
        raise SystemExit("fp-queue: ROM too short")
    return list(struct.unpack(">%dI" % (size // 4), blob))


def object_function_words(path, name=None):
    """Words of one function (or the whole .text) of an ELF32 object."""
    import reference_build_digest as rbd
    with open(path, "rb") as fh:
        blob = fh.read()
    end, _is64, sections = rbd._sections(blob)
    ti = next(i for i, s in enumerate(sections) if s[0] == ".text")
    _n, _t, off, size, _l, _e = sections[ti]
    text = blob[off:off + size]
    lo, hi = 0, len(text)
    if name is not None:
        for sname, stype, soff, ssize, link, entsize in sections:
            if stype != 2 or not entsize:
                continue
            stroff = sections[link][2]
            for k in range(soff, soff + ssize, entsize):
                nameoff, value, sym_size, _info, _other, shndx = struct.unpack_from(
                    end + "IIIBBH", blob, k)
                stop = blob.find(b"\0", stroff + nameoff)
                if shndx == ti and blob[stroff + nameoff:stop] == name.encode():
                    lo, hi = value, value + (sym_size or len(text) - value)
    text = text[lo:hi]
    return list(struct.unpack(">%dI" % (len(text) // 4), text[:len(text) // 4 * 4]))


def locate_reference(refs_root, title, basename):
    """Path of an object under a lock title's object roots, and the title's
    digest verdict (None when the lock or checkout is missing)."""
    import hasm_audit
    import reference_build_digest as rbd
    import hashlib
    lock = os.path.join(ROOT_DIR, "tools", "reference-builds.lock")
    for t, roots, _commit, digest in hasm_audit.parse_lock(lock):
        if t != title:
            continue
        checkout = os.path.join(refs_root, t)
        paths = []
        for root in roots:
            for dirpath, dirnames, filenames in os.walk(os.path.join(checkout, root)):
                dirnames.sort()
                paths.extend(os.path.join(dirpath, f) for f in sorted(filenames) if f.endswith(".o"))
        found = [p for p in paths if os.path.basename(p) == basename]
        aggregate = hashlib.sha256()
        for p in sorted(paths, key=lambda q: os.path.relpath(q, checkout)):
            aggregate.update(("%s %s\n" % (os.path.relpath(p, checkout), rbd.mining_surface(p))).encode())
        ok = bool(paths) and aggregate.hexdigest() == digest
        return found, (ok and len(paths)), checkout
    return [], None, None


# ----------------------------------------------------------------------------
# Reporting helpers (register numbers and counts only).

def row_count(nodes):
    return sum(1 for n in nodes if n.kind == "store")


def summarize(nodes):
    temps = sum(1 for n in nodes if n.kind == "temp")
    formals = sum(1 for n in nodes if n.kind == "formal")
    return "%d FP temporaries, %d formals, %d stores" % (temps, formals, row_count(nodes))


def extra_witness(nodes, fit):
    """'row r: $fA $fB' for the extra values of a fit."""
    by_row: dict = {}
    for i in fit.order:
        if nodes[i].kind == "extra":
            by_row.setdefault(nodes[i].row, []).append("$f%d" % fit.extra_regs[i])
    return "; ".join("row %d: %s" % (r, " ".join(v)) for r, v in sorted(by_row.items()))


# ----------------------------------------------------------------------------
# Modes.

def mode_rom(args):
    rom = os.path.join(ROOT_DIR, "baseroms", "mickey.%s.z64" % args.version)
    if not os.path.exists(rom):
        print("fp-queue: no baserom at baseroms/mickey.%s.z64" % args.version)
        return 2
    offset, size = read_symbol(args.rom, args.version)
    words = rom_words(rom, offset, size)
    try:
        nodes = dataflow(words)
    except OutOfScope as exc:
        print("fp-queue: %s: out of the model's scope (%s)" % (args.rom, exc))
        return 1
    print("%s: %d words, %s" % (args.rom, len(words), summarize(nodes)))
    locals_possible = writes_uopt_first(nodes)
    print("  writes $f0/$f2: %s" % ("yes -- uopt-coloured locals possible, so a queue misfit "
                                     "is not decisive" if locals_possible else
                                     "no, so every non-formal FP value is a queue temporary"))

    plain = fits(nodes)
    tried = hits = 0
    for start in start_orders(nodes):
        tried += 1
        if fits(nodes, start=start):
            hits += 1
    print("  as compiled: default queue order %s; %d of %d queue start orders fit"
          % ("fits" if plain else "does not fit", hits, tried))

    extra = insert_extras(nodes)
    efit = fits(extra)
    print("  with one extra loaded value and one extra sum before every store: %s"
          % ("fits, extra values in %s" % extra_witness(extra, efit) if efit else "does not fit"))
    needed = None
    for k in range(row_count(nodes) + 1):
        for rows in itertools.combinations(range(row_count(nodes)), k):
            if fits(insert_extras(nodes, set(rows))):
                needed = rows
                break
        if needed is not None:
            break
    if needed is not None:
        print("  fewest rows that need the extra pair: %s"
              % (", ".join(map(str, needed)) or "none"))
    verdict = (not locals_possible and not plain and hits == 0 and efit is not None)
    print("verdict: %s" % ("no evaluation order of these instructions fits; the "
                           "deleted-translation shape fits" if verdict else
                           "the deleted-translation verdict does not hold"))
    return 0 if verdict else 1


def mode_reference(args):
    found, ok_count, checkout = locate_reference(args.refs_root, "banjo-kazooie", "mtxxfmf.o")
    ok = bool(ok_count)
    if checkout is None or not found:
        print("fp-queue: no banjo-kazooie mtxxfmf.o under the lock's object roots in $REFS_ROOT")
        return 2
    rc = 0
    if not ok:
        print("fp-queue: banjo-kazooie farm digest does not match tools/reference-builds.lock")
        if not args.no_digest:
            return 2
    else:
        print("banjo-kazooie farm digest matches tools/reference-builds.lock (%d objects)" % ok_count)
    rom = os.path.join(ROOT_DIR, "baseroms", "mickey.%s.z64" % args.version)
    image = open(rom, "rb").read() if os.path.exists(rom) else None
    for path in found:
        words = object_function_words(path, "guMtxXFMF")
        nodes = dataflow(words)
        fit = fits(nodes)
        print("%s: %d words, %s: %s" % (os.path.relpath(path, checkout), len(words),
                                         summarize(nodes), "fits" if fit else "does not fit"))
        if not fit:
            rc = 1
        if image is not None:
            # The object has no relocations, so its bytes compare directly.
            blob = struct.pack(">%dI" % len(words), *words)
            at, hits = image.find(blob), []
            while at >= 0:
                hits.append("0x%X" % at)
                at = image.find(blob, at + 4)
            print("  occurs %d time(s) whole in the baserom%s" % (
                len(hits), (", at ROM " + ", ".join(hits)) if hits else ""))
    return rc


# Generated C forms for --validate.  Every form computes the same three rows;
# the axes are the ones that decide temporary order and operand release.
TREES = [(p, s) for p in itertools.permutations(range(3)) for s in "LR"]
TRANSLATION = {"none": None, "zero-weight": "{s} + {t}*0.0f",
               "zero-weight-first": "{t}*0.0f + {s}", "self-difference": "{s} + ({t} - {t})",
               "translation": "{s} + {t}", "translation-first": "{t} + {s}"}


def _form(tree, order, trans, index, temps):
    names = ("x", "y", "z")

    def elem(r, c):
        return {"2d": "mf[%d][%d]" % (r, c), "flat": "m[%d]" % (4 * r + c)}[index]

    def prod(r, c):
        return ("%s*%s" % (elem(r, c), names[r])) if order == "mv" else ("%s*%s" % (names[r], elem(r, c)))

    perm, shape = tree
    body = ["    float *m = (float *)mf;"] if index == "flat" else []
    decl = {"products": "    float a, b, c;", "row": "    float t;", "rows": "    float t0, t1, t2;"}
    if temps in decl:
        body.insert(0, decl[temps])
    stores = []
    for c, out in enumerate(("ox", "oy", "oz")):
        p = [prod(r, c) for r in range(3)]
        if temps == "products":
            body += ["    a = %s;" % p[0], "    b = %s;" % p[1], "    c = %s;" % p[2]]
            p = ["a", "b", "c"]
        a, b, d = (p[k] for k in perm)
        s = ("%s + %s + %s" % (a, b, d)) if shape == "L" else ("%s + (%s + %s)" % (a, b, d))
        if TRANSLATION[trans]:
            s = TRANSLATION[trans].format(s=s, t=elem(3, c))
        if temps == "row":
            body += ["    t = %s;" % s, "    *%s = t;" % out]
        elif temps == "rows":
            body.append("    t%d = %s;" % (c, s))
            stores.append("    *%s = t%d;" % (out, c))
        else:
            body.append("    *%s = %s;" % (out, s))
    body += stores
    return ("void f(float mf[4][4], float x, float y, float z, float *ox, float *oy, float *oz)\n"
            "{\n%s\n}\n" % "\n".join(body))


def generated_forms(quick=False):
    trees = TREES[:3] if quick else TREES
    for tree in trees:
        for order in ("mv", "vm"):
            for trans in TRANSLATION:
                for index in ("2d", "flat"):
                    for temps in ("inline", "products", "row", "rows"):
                        yield _form(tree, order, trans, index, temps)


FLAGSETS = [("-O3", ["-O3", "-mips2", "-o32", "-Wab,-r4300_mul"]),
            ("-O2", ["-O2", "-mips2", "-o32", "-Wab,-r4300_mul"]),
            ("-O3 -mips1", ["-O3", "-mips1", "-o32"])]
# -O1 is outside the model: without uopt, ugen keeps memory contents cached in
# registers past their last use, and over half of the -O1 compiles of these
# forms fit no order.  The model is for optimized code, where formals arrive
# by mtc1 into uopt's colours, as they do in the routines it is used on.
O3_LABELS = ("-O3", "-O3 -mips1")
COMMON = ["-c", "-G", "0", "-nostdinc", "-Xcpluscomm", "-non_shared", "-woff", "516,649,838,712"]


def compilers(args):
    """[(label, cc path, flag labels it can run, skipped note)]."""
    out = []
    ido53 = os.path.join(ROOT_DIR, "tools", "ido", "cc")
    if os.access(ido53, os.X_OK):
        out.append(("IDO 5.3", ido53, [f for f, _ in FLAGSETS], None))
    candidates = [args.ido71, os.environ.get("IDO71_DIR"),
                  os.path.join(args.refs_root, "perfect_dark", "build", "recomp", "7.1")]
    for d in candidates:
        if d and os.access(os.path.join(d, "cc"), os.X_OK):
            o3 = all(os.path.exists(os.path.join(d, h)) for h in O3_HELPERS)
            runs = [f for f, _ in FLAGSETS if o3 or f not in O3_LABELS]
            out.append(("IDO 7.1", os.path.join(d, "cc"), runs,
                        None if o3 else "IDO 7.1 -O3 skipped: this recomp set has no %s"
                        % "/".join(h for h in O3_HELPERS if not os.path.exists(os.path.join(d, h)))))
            break
    else:
        out.append(("IDO 7.1", None, [], "IDO 7.1 skipped: none found (--ido71, $IDO71_DIR)"))
    return out


def compile_one(cc, flags, src, workdir):
    d = tempfile.mkdtemp(dir=workdir)
    c, o = os.path.join(d, "f.c"), os.path.join(d, "f.o")
    with open(c, "w") as fh:
        fh.write(src)
    r = subprocess.run([cc] + COMMON + flags + [c, "-o", o], cwd=d,
                       capture_output=True, text=True, timeout=120)
    try:
        return object_function_words(o, "f") if r.returncode == 0 and os.path.exists(o) else None
    finally:
        shutil.rmtree(d, ignore_errors=True)


def mode_validate(args):
    sets = compilers(args)
    if not any(cc for _l, cc, _f, _n in sets):
        print("fp-queue: no IDO compiler found (tools/ido/cc)")
        return 2
    forms = list(dict.fromkeys(generated_forms(args.quick)))
    jobs = []
    for label, cc, runs, note in sets:
        if note:
            print("  " + note)
        for flab, flags in FLAGSETS:
            if flab in runs:
                jobs += [(label + " " + flab, cc, flags, src) for src in forms]
    counts: dict = {}
    distinct: dict = {}
    with tempfile.TemporaryDirectory(prefix="fpq-") as work, \
            ThreadPoolExecutor(max(1, args.jobs)) as ex:
        for (label, _cc, _flags, _src), words in ex.map(
                lambda j: (j, compile_one(j[1], j[2], j[3], work)), jobs):
            c = counts.setdefault(label, {"compiles": 0})
            c["compiles"] += 1
            if words is None:
                c["did not compile"] = c.get("did not compile", 0) + 1
                continue
            key = tuple(words)
            if key not in distinct:
                distinct[key] = classify(words)
            how = distinct[key][0]
            c[how] = c.get(how, 0) + 1

    total = sum(c["compiles"] for c in counts.values())
    print("validate: %d forms, %d compiles, %d distinct objects" % (len(forms), total, len(distinct)))
    for label, c in counts.items():
        rest = ", ".join("%s %d" % (k, v) for k, v in sorted(c.items()) if k != "compiles")
        print("  %-19s %5d compiles: %s" % (label, c["compiles"], rest))
    owned = [d for d in distinct.values() if d[0].startswith("queue, uopt-owned")]
    premise_broken = sum(1 for d in owned if not d[1])
    print("  uopt-coloured locals: %d distinct objects needed a queue register given to uopt; "
          "%d of them also write $f0/$f2" % (len(owned), len(owned) - premise_broken))
    edited = [d[2] for d in distinct.values() if d[2] is not None]
    print("  deletion control: %d distinct objects that write neither $f0 nor $f2 carry a "
          "translation load and add per store; with those deleted, %d fit no order"
          % (len(edited), edited.count(True)))
    misfits = sum(c.get("no fit", 0) + c.get("out of scope", 0) + c.get("did not compile", 0)
                  for c in counts.values())
    ok = total and not misfits and not premise_broken and all(edited)
    print("verdict: %s" % ("every compile fits the queue model, uopt colours $f0/$f2 first, "
                           "and every deletion control is rejected" if ok else
                           "%d compiles misfit, %d break the $f0/$f2 premise, %d deletion "
                           "controls still fit" % (misfits, premise_broken, edited.count(False))))
    return 0 if ok else 1


def classify(words):
    """(how it fits, writes $f0/$f2, deletion-control result or None)."""
    try:
        nodes = dataflow(words)
    except OutOfScope:
        return ("out of scope", False, None)
    how = explain(nodes) or "no fit"
    low = writes_uopt_first(nodes)
    control = None
    if not low:
        edited = delete_translations(nodes)
        if edited is not None:
            control = fits(edited) is None
    return (how, low, control)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--rom", metavar="SYMBOL", help="check a routine read from the baserom")
    mode.add_argument("--reference", action="store_true",
                      help="check Banjo-Kazooie's ultralib guMtxXFMF object")
    mode.add_argument("--validate", action="store_true",
                      help="compile generated C and check every compile")
    ap.add_argument("--version", default="us")
    ap.add_argument("--refs-root", default=DEFAULT_REFS)
    ap.add_argument("--ido71", help="IDO 7.1 recomp directory (default $IDO71_DIR)")
    ap.add_argument("--quick", action="store_true", help="--validate on a small subset")
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--no-digest", action="store_true",
                    help="--reference: proceed when the farm digest does not match")
    args = ap.parse_args(argv)
    if args.rom:
        return mode_rom(args)
    if args.reference:
        return mode_reference(args)
    return mode_validate(args)


if __name__ == "__main__":
    sys.exit(main())
