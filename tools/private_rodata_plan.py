#!/usr/bin/env python3
"""Plan the metadata-only POSTPROCESS that retires a TU's private .rodata copy.

When a function whose jump tables and float literals already live in an
extracted rodata pool (``asm/data/*.rodata.s``, whose table entries name the
case labels) is compiled from C, IDO emits its own private copy of those
tables and literals in the TU's ``.rodata``. The established resident fix
(anim.c's func_800563B4 face table, 2026-10-02) is metadata only:

  * export every case label the shipped tables name as a global symbol at
    its ``.text`` offset, so the pool's ``.word .L800xxxxx`` entries resolve;
  * rebind each ``.text`` -> ``.rodata`` HI16/LO16 relocation to a symbol S
    whose address plus the compiler's in-place addend A is the shipped object
    T (S = T - A), so no instruction word or addend is edited;
  * drop ``.rel.rodata`` and trim the private copy to the retained prefix,
    guarded by the SHA-256 of the discarded bytes.

This tool derives all three from a *pre-POSTPROCESS* object and the local
pool listing, and refuses unless every discarded word is proved equal to the
shipped word it stands for: a literal byte for byte, a table entry as
``text_base + in-place .text offset`` equal to the shipped label's address.
S is a shipped symbol the object already references when one sits at T - A
(``rebind_elf_relocations.py`` can only bind to a symbol in the object's own
table); otherwise a local absolute anchor is emitted for it.

It prints a report (``#`` lines) and the Makefile POSTPROCESS fragment. Only
symbol names, ``.text`` offsets, relocation sites and one digest are printed;
the listing and object are read at run time and nothing ROM-derived is
written.

Get a pre-POSTPROCESS object from the tree's own recipe (a command-line
variable overrides the target-specific one), then move it out of build/ so
the un-normalized object is never linked:

    gmake build/src/main/anim.c.o POSTPROCESS=:
    mv build/src/main/anim.c.o /tmp/anim.raw.o
    tools/private_rodata_plan.py /tmp/anim.raw.o asm/data/84BAC.rodata.s \\
        --text-base 0x80050000 --keep 0x4 \\
        --range 0x4:0x238=D_80083FB4 --range 0x238:0x250=jtbl_800841F4 \\
        --anchor-prefix anim

``--range START:END=SYMBOL`` says private ``.rodata`` [START, END) stands
for the shipped bytes beginning at SYMBOL. Uncovered words past ``--keep``
must be zero (alignment padding).
"""

import argparse
import hashlib
import re
import struct
import sys
from pathlib import Path

SHT_SYMTAB = 2
SHT_REL = 9
STT_SECTION = 3
SHN_UNDEF = 0
R_MIPS_32 = 2
R_MIPS_HI16 = 5
R_MIPS_LO16 = 6


class PlanError(ValueError):
    """The object and the listing do not prove the requested retirement."""


def fail(message):
    raise PlanError(message)


def hx(value):
    return f"0x{value:X}"


class Elf:
    def __init__(self, data):
        if data[:6] != b"\x7fELF\x01\x02":
            fail("expected a big-endian ELF32 object")
        self.data = data
        shoff = struct.unpack_from(">I", data, 0x20)[0]
        shentsize = struct.unpack_from(">H", data, 0x2E)[0]
        shnum = struct.unpack_from(">H", data, 0x30)[0]
        shstrndx = struct.unpack_from(">H", data, 0x32)[0]
        self.sh = [struct.unpack_from(">10I", data, shoff + i * shentsize)
                   for i in range(shnum)]
        start, size = self.sh[shstrndx][4], self.sh[shstrndx][5]
        blob = data[start:start + size]
        self.names = [blob[s[0]:blob.find(b"\0", s[0])].decode() for s in self.sh]

    def index(self, name):
        found = [i for i, n in enumerate(self.names) if n == name]
        if len(found) != 1:
            fail(f"expected one section {name!r}, found {len(found)}")
        return found[0]

    def payload(self, index):
        s = self.sh[index]
        return self.data[s[4]:s[4] + s[5]]

    def symbols(self):
        tables = [s for s in self.sh if s[1] == SHT_SYMTAB]
        if len(tables) != 1:
            fail("expected one symbol table")
        s = tables[0]
        strtab = self.sh[s[6]]
        strs = self.data[strtab[4]:strtab[4] + strtab[5]]
        out = []
        for off in range(s[4], s[4] + s[5], s[9] or 16):
            nm, value, size, info, _other, shndx = struct.unpack_from(
                ">IIIBBH", self.data, off)
            out.append((strs[nm:strs.find(b"\0", nm)].decode(), value, size,
                        info, shndx))
        return out

    def rel(self, target_index):
        """[(site, type, symbol_index)] in REL table order."""
        rows = []
        for s in self.sh:
            if s[1] == SHT_REL and s[7] == target_index:
                for off in range(s[4], s[4] + s[5], 8):
                    site, info = struct.unpack_from(">II", self.data, off)
                    rows.append((site, info & 0xFF, info >> 8))
        return rows


def section_symbol(symbols, shndx):
    found = [i for i, row in enumerate(symbols)
             if row[4] == shndx and row[3] & 0xF == STT_SECTION]
    if len(found) != 1:
        fail(f"section {shndx} has {len(found)} SECTION symbols")
    return found[0]


LINE_RE = re.compile(
    r"/\*\s*([0-9A-F]+)\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s*\*/\s*\.(\w+)\s+(\S+)")
LABEL_PREFIXES = ("dlabel ", "glabel ", "jlabel ")


def parse_listing(text):
    """({address: (word, label-or-None)}, {symbol: address}, {address: [symbols]})"""
    words, symbols, by_address = {}, {}, {}
    pending = []
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith(LABEL_PREFIXES):
            pending.append(stripped.split()[1])
            continue
        match = LINE_RE.search(line)
        if not match:
            continue
        address = int(match.group(2), 16)
        word = int(match.group(3), 16)
        operand = match.group(5)
        is_label = match.group(4) == "word" and operand.startswith(".L")
        words[address] = (word, operand if is_label else None)
        for name in pending:
            symbols[name] = address
            by_address.setdefault(address, []).append(name)
        pending = []
    return words, symbols, by_address


def hi_lo_addends(text, relocs, symbol_index):
    """[(site, type, addend)] for every HI16/LO16 against ``symbol_index``.

    Pairing follows the REL table's own order, which is the linker's rule:
    each HI16 takes the next LO16 against the same symbol, and a LO16 with no
    fresh HI16 reuses the last paired one.
    """
    rows = [(site, kind) for site, kind, sym in relocs if sym == symbol_index]
    others = sorted(site for site, kind in rows
                    if kind not in (R_MIPS_HI16, R_MIPS_LO16))
    if others:
        fail(f"unsupported .rodata relocation types at {[hx(s) for s in others]}")
    out = []
    pending_hi = []
    paired_hi_imm = None
    for site, kind in rows:
        imm = struct.unpack_from(">I", text, site)[0] & 0xFFFF
        if kind == R_MIPS_HI16:
            pending_hi.append((site, imm))
            continue
        lo = imm - 0x10000 if imm & 0x8000 else imm
        for hi_site, hi_imm in pending_hi:
            out.append((hi_site, R_MIPS_HI16, ((hi_imm << 16) + lo) & 0xFFFFFFFF))
            paired_hi_imm = hi_imm
        pending_hi = []
        if paired_hi_imm is None:
            fail(f"LO16 at {hx(site)} has no preceding HI16")
        out.append((site, R_MIPS_LO16, ((paired_hi_imm << 16) + lo) & 0xFFFFFFFF))
    if pending_hi:
        fail(f"HI16 without LO16 at {[hx(s) for s, _ in pending_hi]}")
    return sorted(out)


def parse_range(text):
    span, _, name = text.partition("=")
    start, _, end = span.partition(":")
    if not name or not start or not end:
        raise argparse.ArgumentTypeError(
            f"invalid range {text!r}; expected START:END=SYMBOL")
    return int(start, 0), int(end, 0), name


def plan(object_bytes, listing_text, text_base, keep, ranges, anchor_prefix):
    elf = Elf(object_bytes)
    text_index = elf.index(".text")
    rodata_index = elf.index(".rodata")
    text = elf.payload(text_index)
    rodata = elf.payload(rodata_index)
    if keep % 4 or not 0 <= keep <= len(rodata):
        fail(f"--keep {hx(keep)} is not an aligned prefix of .rodata")
    symbols = elf.symbols()
    object_names = {row[0] for row in symbols if row[0]}
    text_symbol = section_symbol(symbols, text_index)
    rodata_symbol = section_symbol(symbols, rodata_index)
    rodata_symbol_name = symbols[rodata_symbol][0]
    words, shipped, by_address = parse_listing(listing_text)
    for start, end, name in ranges:
        if name not in shipped:
            fail(f"range symbol {name} is not in the listing")
        if start < keep or end > len(rodata) or start >= end:
            fail(f"range {hx(start)}:{hx(end)} lies outside the discarded bytes")

    def shipped_address(offset):
        for start, end, name in ranges:
            if start <= offset < end:
                return shipped[name] + (offset - start)
        return None

    table_rows = {}
    for site, kind, sym in elf.rel(rodata_index):
        if kind != R_MIPS_32 or sym != text_symbol:
            fail(f".rodata+{hx(site)}: expected R_MIPS_32 against .text")
        table_rows[site] = struct.unpack_from(">I", rodata, site)[0]

    labels = {}
    checked = {"literal": 0, "table": 0, "padding": 0}
    for offset in range(keep, len(rodata), 4):
        word = struct.unpack_from(">I", rodata, offset)[0]
        address = shipped_address(offset)
        if address is None:
            if word or offset in table_rows:
                fail(f".rodata+{hx(offset)} is nonzero but no range covers it")
            checked["padding"] += 1
            continue
        if address not in words:
            fail(f".rodata+{hx(offset)} -> {hx(address)} is not in the listing")
        shipped_word, label = words[address]
        if offset in table_rows:
            target = (text_base + table_rows[offset]) & 0xFFFFFFFF
            if label is None or shipped_word != target:
                fail(f".rodata+{hx(offset)}: private case {hx(target)} != "
                     f"shipped {hx(shipped_word)} ({label})")
            previous = labels.get(label)
            if previous is not None and previous != table_rows[offset]:
                fail(f"label {label} maps to two .text offsets")
            labels[label] = table_rows[offset]
            checked["table"] += 1
        else:
            if label is not None or shipped_word != word:
                fail(f".rodata+{hx(offset)}: private literal {hx(word)} != "
                     f"shipped {hx(shipped_word)}")
            checked["literal"] += 1

    rebinds, anchors, report, kept = [], {}, [], []
    for site, kind, addend in hi_lo_addends(text, elf.rel(text_index), rodata_symbol):
        if addend < keep:
            # The retained prefix stays this TU's own section-relative data.
            kept.append(site)
            continue
        target = shipped_address(addend)
        if target is None:
            fail(f".text+{hx(site)}: addend {hx(addend)} is in no range")
        base = (target - addend) & 0xFFFFFFFF
        target_name = by_address.get(target, [hx(target)])[0]
        usable = [name for name in by_address.get(base, []) if name in object_names]
        if usable:
            anchor = usable[0]
        else:
            anchor = f"{anchor_prefix}Anchor_{target_name}"
            if anchors.get(anchor, base) != base:
                fail(f"anchor {anchor} would need two values")
            anchors[anchor] = base
        rebinds.append((site, rodata_symbol_name, anchor))
        report.append((site, "HI16" if kind == R_MIPS_HI16 else "LO16", addend,
                       target_name, anchor, base))

    return {
        "labels": sorted(labels.items(), key=lambda item: (item[1], item[0])),
        "anchors": anchors,
        "rebinds": rebinds,
        "kept_sites": kept,
        "report": report,
        "checked": checked,
        "rodata_size": len(rodata),
        "digest": hashlib.sha256(rodata[keep:]).hexdigest(),
    }


def render(result, keep):
    """The POSTPROCESS fragment, in the Makefile's own continuation style."""
    lines = []
    adds = [f"--add-symbol {name}=.text:{hx(offset)},global"
            for name, offset in result["labels"]]
    adds += [f"--add-symbol {name}={hx(value)},local"
             for name, value in sorted(result["anchors"].items())]
    if adds:
        lines.append("\t$(OBJCOPY) " + " \\\n\t    ".join(adds) + " $@ && \\")
    if result["rebinds"]:
        specs = [f"{hx(site)}:{expected}:{anchor}"
                 for site, expected, anchor in result["rebinds"]]
        lines.append("\t$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \\\n\t    "
                     + " \\\n\t    ".join(specs) + " && \\")
    lines.append("\t$(OBJCOPY) --remove-section=.rel.rodata $@ && \\")
    lines.append(f"\t$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata {hx(keep)} \\\n"
                 f"\t    sha256:{result['digest']}")
    return "\n".join(lines)


def report_lines(result, keep):
    checked = result["checked"]
    yield (f"# .rodata {hx(result['rodata_size'])}: kept {hx(keep)}; proved "
           f"{checked['literal']} literal, {checked['table']} table, "
           f"{checked['padding']} padding words")
    yield (f"# {len(result['labels'])} case labels, {len(result['rebinds'])} rebinds, "
           f"{len(result['anchors'])} absolute anchors, "
           f"{len(result['kept_sites'])} sites left on the kept prefix")
    for site, kind, addend, target, anchor, base in result["report"]:
        yield (f"#   .text+{hx(site)} {kind} addend {hx(addend)} -> {target} "
               f"via {anchor} ({hx(base)})")


def main(argv=None):
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("object", type=Path, help="pre-POSTPROCESS object")
    parser.add_argument("listing", type=Path, help="extracted rodata pool listing")
    parser.add_argument("--text-base", type=lambda v: int(v, 0), required=True,
                        help="linked address of the object's .text")
    parser.add_argument("--keep", type=lambda v: int(v, 0), required=True,
                        help="retained .rodata prefix (the trim size)")
    parser.add_argument("--range", action="append", type=parse_range, required=True,
                        dest="ranges", metavar="START:END=SYMBOL")
    parser.add_argument("--anchor-prefix", default="private",
                        help="name prefix for local absolute anchors")
    args = parser.parse_args(argv)
    try:
        result = plan(args.object.read_bytes(), args.listing.read_text(),
                      args.text_base, args.keep, args.ranges, args.anchor_prefix)
    except PlanError as error:
        print(f"private_rodata_plan: {error}", file=sys.stderr)
        return 1
    for line in report_lines(result, args.keep):
        print(line)
    print(render(result, args.keep))
    return 0


if __name__ == "__main__":
    sys.exit(main())
