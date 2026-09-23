#!/usr/bin/env python3
"""Synthesize an overlay object's relocation surface from the shipped tables.

Background
----------
An overlay module ships *unrelocated*: `runlinkDownloadCode` patches every
site named by the module's own `reloc1`/`reloc2` tables after the DMA.  What
the ROM image therefore stores at a relocation site is not the final address
but the record's **stored addend** -- the value the runtime adds its base to.
`docs/overlays.md` sections 5.1-5.4 establish that model; this tool is its
mechanical consequence.

A C translation unit compiled for an overlay cannot express those addends: IDO
emits an ordinary `R_MIPS_26` / `R_MIPS_HI16` + `R_MIPS_LO16` reference to a
symbol, and the symbol has no address in this build because it lives in a
different module (or in a section the runtime places).  The project's answer
is a placeholder extern (`overlay1Chain0Reloc`, `D_0210`, ...) whose *value*
is then supplied as a linker-script assignment in
`overlay_undefined_syms.us.txt`, so that the linked instruction word carries
exactly the stored addend the retail image carries.

That value is not a judgement call.  For a candidate whose instruction
schedule already agrees with the target, every placeholder's value is
**readable from the ROM at the site the relocation names**:

    R_MIPS_26   value = SYNTHETIC_VMA | (stored_imm26 << 2)
    HI16/LO16   value = (stored_hi16 << 16) + sign_extend16(stored_lo16)

This tool reads a compiled object, maps each of its undefined-symbol
relocations back to a module text offset, reads the shipped word from the
baserom, and prints the linker-script assignments (and, with --makefile, the
equivalent objcopy spec).  It also cross-checks each site against the decoded
module relocation table, which is what distinguishes a genuine relocation
site from a literal the compiler must not relocate.

A candidate whose schedule differs anywhere before a site moves that site, and
then no offset in the table names it.  The table still records *which*
relocations the function performs and in *what order* -- the runtime walks
reloc1/reloc2 in offset order, and reordering instructions cannot reorder the
references -- so `align_sites` matches the object's sites onto the module's
records over the same text range by relocation type, in offset order.  The
placement is accepted only when it is the *only* order-preserving one
(`unique_order_preserving_embedding`); a differing multiset, a type the retail
order cannot supply, or two ways to lay the sequence down is a refusal, because
choosing between them would be inventing an addend.  An object whose sites all
corroborate where they stand is not realigned at all.  Every alignment is
reported (`/* ALIGNED obj: aligned/total site(s), N shifted */`) and the
promotion trial records the counts per row, so a shifted site is never read as
an exact one: the words it moved still count as differences, and the linked-ROM
byte comparison remains the only proof that a candidate is right.

Nothing ROM-derived is written: the baserom and the atlas are read at run
time and only *addresses and symbol values already required by the link* are
emitted.

Usage:
    tools/reloc_surface.py OBJECT [--overlay N] [--rom PATH] [--makefile]
    tools/reloc_surface.py --audit           # replay the whole tracked surface
    tools/reloc_surface.py compare SYMBOL --candidate-object OBJECT
"""

from __future__ import annotations

import argparse
import collections
import json
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import overlay_tables as ot  # noqa: E402
import reloc_identity as ri  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
DEFAULT_ROM = REPO / "baseroms" / "mickey.us.z64"
SYNTHETIC_VMA = 0xF0000000
VERSION = "us"
LINK_SYMS = REPO / f"overlay_undefined_syms.{VERSION}.txt"

SHT_SYMTAB = 2
SHT_REL = 9
SHN_UNDEF = 0

R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6
R_MIPS_PC16 = 10
TYPE_NAMES = {R_MIPS_32: "R_MIPS_32", R_MIPS_26: "R_MIPS_26",
              R_MIPS_HI16: "R_MIPS_HI16", R_MIPS_LO16: "R_MIPS_LO16",
              R_MIPS_PC16: "R_MIPS_PC16"}

# func_overlay_018_F0000000_18745B8 -- the generated identity carries both the
# synthetic VMA and the ROM address, so a module offset needs no extra table.
GEN_NAME_RE = re.compile(r"^func_overlay_(\d{3})_F([0-9A-F]{7})_[0-9A-F]+$")

# func_80029FE4, D_80003634 -- splat's auto-name for a *resident* address.  An
# overlay that calls one of these spells the call with the resident function's
# own global name, which is the one name the relocation surface must not
# assign: the name is shared with the resident segment, so a value line for it
# would move the symbol for every resident caller too (§5.6).  The alias below
# is the per-module placeholder that carries the addend instead.
RESIDENT_NAME_RE = re.compile(r"^(?:func|D)_8[0-9A-F]{7}$")
RESIDENT_ALIAS_FMT = "%s_o%03dReloc"
RESIDENT_ALIAS_RE = re.compile(r"^(?:func|D)_8[0-9A-F]{7}_o\d{3}Reloc$")


# --------------------------------------------------------------------- ELF

class Elf:
    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        d = self.data
        if d[:6] != b"\x7fELF\x01\x02":
            raise SystemExit(f"{path}: expected a big-endian ELF32 object")
        shoff = struct.unpack_from(">I", d, 0x20)[0]
        shentsize = struct.unpack_from(">H", d, 0x2E)[0]
        shnum = struct.unpack_from(">H", d, 0x30)[0]
        shstrndx = struct.unpack_from(">H", d, 0x32)[0]
        self.sh = [struct.unpack_from(">10I", d, shoff + i * shentsize)
                   for i in range(shnum)]
        names_off, names_size = self.sh[shstrndx][4], self.sh[shstrndx][5]
        blob = d[names_off:names_off + names_size]
        self.names = [blob[s[0]:blob.find(b"\0", s[0])].decode() for s in self.sh]

    def section(self, name):
        for i, n in enumerate(self.names):
            if n == name:
                return i, self.sh[i]
        return None, None

    def symbols(self):
        """[(name, value, size, info, shndx)] from the first .symtab."""
        for i, s in enumerate(self.sh):
            if s[1] == SHT_SYMTAB:
                strtab = self.sh[s[6]]
                strs = self.data[strtab[4]:strtab[4] + strtab[5]]
                out = []
                for off in range(s[4], s[4] + s[5], s[9] or 16):
                    nm, val, size, info, _o, shndx = struct.unpack_from(
                        ">IIIBBH", self.data, off)
                    name = strs[nm:strs.find(b"\0", nm)].decode()
                    out.append((name, val, size, info, shndx))
                return out
        return []

    def relocations(self, target=r"\.text"):
        """[(section_name, offset, type, symbol_index)] for REL sections."""
        out = []
        for i, s in enumerate(self.sh):
            if s[1] != SHT_REL:
                continue
            tgt = self.names[s[7]] if s[7] < len(self.names) else ""
            if not re.fullmatch(target, tgt):
                continue
            for off in range(s[4], s[4] + s[5], 8):
                r_off, r_info = struct.unpack_from(">II", self.data, off)
                out.append((tgt, r_off, r_info & 0xFF, r_info >> 8))
        return out

    def section_bytes(self, name):
        i, s = self.section(name)
        return b"" if i is None else self.data[s[4]:s[4] + s[5]]


# ------------------------------------------------------------------ mapping

def tu_base(obj_path: Path, overlay: int, atlas_rows):
    """Module text offset of the translation unit this object came from.

    `config/overlays.us.json` records one contiguous `text_ownership` row per
    source file, so an object's base needs no name convention, no linked ELF,
    and no heuristic -- which matters because the link is exactly what is
    missing when a promotion fails to resolve.  The row's size is the object's
    trimmed `.text` extent, so a size disagreement is a real signal and is
    reported rather than silently mapped.
    """
    stem = obj_path.name
    for suffix in (".c.o", ".o"):
        if stem.endswith(suffix):
            stem = stem[: -len(suffix)]
            break
    for row in atlas_rows:
        if Path(row["source"]).name == stem and row.get("type") == "c":
            return int(row["offset"], 16), int(row["size"], 16)
    return None, None


# ------------------------------------------------------------------- solver

def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


def stored_field(rom, site, rtype):
    word = struct.unpack_from(">I", rom, site)[0]
    if rtype == R_MIPS_26:
        return word & 0x03FFFFFF
    if rtype in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_PC16):
        return word & 0xFFFF
    return word


def _pc16_branch_instruction(word):
    """Recognize only MIPS conditional branches with a signed PC16 field."""
    opcode = word >> 26
    rt = (word >> 16) & 31
    if opcode in (4, 5, 20, 21):  # BEQ/BNE and their likely forms.
        return True
    if opcode in (6, 7, 22, 23):
        return rt == 0
    if opcode == 1:  # REGIMM branch and branch-and-link families.
        return rt in (0, 1, 2, 3, 16, 17, 18, 19)
    return opcode == 17 and ((word >> 21) & 31) == 8 and rt in (0, 1, 2, 3)


def _pairs(sites):
    """Attach each R_MIPS_HI16 to its matching R_MIPS_LO16.

    IDO emits the pair in order against the same symbol, which is the MIPS REL
    convention `ld` itself relies on.  Pairing matters because a HI16's value
    is only meaningful together with the sign of its LO16.
    """
    pending = collections.defaultdict(list)
    out = []
    for s in sites:
        if s["type"] == R_MIPS_HI16:
            pending[s["symbol"]].append(s)
        elif s["type"] == R_MIPS_LO16:
            his = pending.pop(s["symbol"], [])
            if his:
                for h in his:
                    out.append((h, s))
            else:
                out.append((None, s))
        else:
            out.append((s, None))
    for group in pending.values():
        for h in group:
            out.append((h, None))
    return out


def module_text_defs(objects, overlay, atlas_rows):
    """Names defined in this module's own .text, across all its objects.

    A reference to one of those is the only kind that needs no assignment: an
    intra-module `jal` is a JUMP record, whose stored immediate is the target's
    module offset shifted right two, which is exactly what the assembler emits
    for that symbol at the synthetic VMA.  Every other referenced symbol -- a
    resident function, another module's function, or this module's own data,
    which the runtime places separately -- must carry the stored addend
    instead of whatever address this build happens to give it.
    """
    out = set()
    for ov, obj in objects:
        if ov != overlay:
            continue
        try:
            elf = Elf(obj)
        except SystemExit:
            continue
        text_idx, _ = elf.section(".text")
        if text_idx is None:
            continue
        for name, _v, _s, _i, shndx in elf.symbols():
            if name and shndx == text_idx:
                out.add(name)
    return out


GENERATED_NAME_TEXT_RE = re.compile(
    r"func_overlay_(\d{3})_F([0-9A-Fa-f]{7})_([0-9A-Fa-f]+)")


def misspelled_generated_names(atlas, texts):
    """Generated overlay names whose ROM suffix disagrees with the atlas.

    ``texts`` is an iterable of ``(label, text)``.  A generated name encodes
    ``overlay``, module offset and ROM address; the address must equal the
    overlay's atlas ROM start plus the offset.  A name that does not is not an
    identity at all, and ``promotion-proof`` refuses a function whose C
    spells one ("encoded ROM address conflicts with atlas ownership") even
    when a POSTPROCESS rename hides it from the link.  Returns sorted
    ``(label, name, expected_name)`` rows.
    """
    starts = {}
    for module in atlas.get("modules", []):
        rom_row = module.get("rom")
        if isinstance(rom_row, dict) and "start" in rom_row:
            starts[module.get("overlay")] = int(rom_row["start"], 16)
    out = set()
    for label, text in texts:
        for match in GENERATED_NAME_TEXT_RE.finditer(text):
            overlay, offset = int(match.group(1)), int(match.group(2), 16)
            if overlay not in starts:
                continue
            expected = starts[overlay] + offset
            if int(match.group(3), 16) != expected:
                out.add((label, match.group(0), "func_overlay_%03d_F%07X_%X"
                         % (overlay, offset, expected)))
    return sorted(out)


def tree_generated_name_texts(root=None):
    """(label, text) for every tracked file that may spell a generated name."""
    root = REPO if root is None else Path(root)
    paths = []
    for base, pattern in (("src", "*.c"), ("src", "*.h"), ("include", "*.h"),
                          ("mk", "*.mk"), ("config/normalizations", "*")):
        paths += sorted(path for path in (root / base).rglob(pattern) if path.is_file())
    paths.append(root / "Makefile")
    return [(path.relative_to(root).as_posix(),
             path.read_text(errors="replace")) for path in paths if path.is_file()]


def foreign_text_values(values, local):
    """Valued names that some module defines in its own ``.text``.

    ``values`` maps name -> (value, asking object name); ``local`` maps
    overlay -> names defined in that module's ``.text``.  Returns
    name -> (asking object name, defining overlay).  An asker's own module
    never values its own ``.text`` names (``synthesize`` skips them), so any
    hit here is a cross-module reference.
    """
    out = {}
    for name, (_value, asker) in values.items():
        owners = sorted(ov for ov, names in local.items() if name in names)
        if owners:
            out[name] = (asker, owners[0])
    return out


# ------------------------------------------------------- site alignment
#
# A site is normally corroborated at its own module offset: the object's
# relocation lands on the byte the module's table names, and the addend is
# read there.  A candidate whose instruction schedule differs anywhere before
# a site moves that site, and then no offset in the table names it -- which is
# what `schedule-divergence-at-site` reports.  The table is still authoritative
# for *which* relocations the function performs and in *what order*: the
# runtime walks reloc1/reloc2 in offset order, and a compiler reordering
# instructions cannot reorder the references themselves.  So a shifted site can
# still be identified by its position in that order.

RELOCATABLE_MODES = frozenset((R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16))


def unique_order_preserving_embedding(candidate, retail):
    """Index in `retail` for each entry of `candidate`, or None.

    `candidate` and `retail` are relocation-type sequences in offset order.
    The embedding must preserve order and match type exactly; it is accepted
    only when it is the *only* such embedding, which is decided by computing
    the leftmost and the rightmost one and requiring them to agree.  Anything
    else -- a candidate type the retail order cannot supply, a differing
    multiset, or two ways to lay the sequence down -- is a refusal, because
    picking one of several would be inventing an addend.
    """
    n, m = len(candidate), len(retail)
    if n > m:
        return None
    left, j = [], 0
    for t in candidate:
        while j < m and retail[j] != t:
            j += 1
        if j == m:
            return None
        left.append(j)
        j += 1
    right, j = [], m - 1
    for t in reversed(candidate):
        while j >= 0 and retail[j] != t:
            j -= 1
        if j < 0:
            return None
        right.append(j)
        j -= 1
    right.reverse()
    return left if left == right else None


def align_sites(sites, records, base, end):
    """Give every mapped site the table offset whose addend it must carry.

    Returns a summary dict.  A site already corroborated at its own offset
    keeps that offset, and when *every* mapped site is corroborated the
    alignment does not run at all -- so an object whose schedule agrees with
    the shipped module is valued exactly as before.  Otherwise the object's
    sites and the module's records over the same text range are matched by
    `unique_order_preserving_embedding`, and each site records the offset it
    was aligned to and whether that offset moved.  A shifted site is never
    silently treated as exact: `shifted` counts it here and the trial row
    reports it, while the ROM byte comparison remains the only proof that the
    candidate is right.
    """
    mapped = sorted([s for s in sites if s["module_off"] is not None],
                    key=lambda s: (s["module_off"], s["type"]))
    summary = {"sites": len(mapped), "aligned": 0, "shifted": 0,
               "attempted": False, "refused": None}
    if not mapped or all(s["in_table"] for s in mapped):
        summary["aligned"] = sum(1 for s in mapped if s["in_table"])
        return summary
    summary["attempted"] = True
    pool = sorted([r for r in records
                   if base <= r["target_offset"] < end
                   and r["mode"] in RELOCATABLE_MODES],
                  key=lambda r: (r["target_offset"], r["mode"]))
    embedding = unique_order_preserving_embedding(
        [s["type"] for s in mapped], [r["mode"] for r in pool])
    if embedding is None:
        summary["refused"] = ("no unique order-preserving alignment of %d "
                              "site(s) onto %d shipped record(s)"
                              % (len(mapped), len(pool)))
        return summary
    for site, index in zip(mapped, embedding):
        record = pool[index]
        site["table_off"] = record["target_offset"]
        site["in_table"] = True
        site["op"] = record["op_name"]
        site["shifted"] = record["target_offset"] != site["module_off"]
        summary["aligned"] += 1
        summary["shifted"] += 1 if site["shifted"] else 0
    return summary

def synthesize(obj_path: Path, overlay: int, rom: bytes, atlas_rows, records,
               local_text=frozenset()):
    """Return (sites, {symbol: required link value}, conflicts, alignment).

    The required value is always `shipped_target - addend_already_in_the_object`.
    Subtracting the object's own addend is what lets one base symbol serve many
    struct-field references: IDO puts the field offset in the instruction, and
    only the base belongs in the linker script.
    """
    elf = Elf(obj_path)
    syms = elf.symbols()
    mods = ot.build_modules(ot.read_headers(rom))
    module = mods[overlay - 1]
    ranges = ot.module_section_ranges(module)
    text_start, text_size = ranges["text"][0], module["text_size"]
    base, extent = tu_base(obj_path, overlay, atlas_rows)
    obj_text = elf.section_bytes(".text")
    text_idx, _ = elf.section(".text")

    table = {}
    for r in records:
        table.setdefault(r["target_offset"], []).append(r)

    sites = []
    for _sec, off, rtype, symidx in elf.relocations():
        if symidx >= len(syms):
            continue
        name, _v, _s, _i, shndx = syms[symidx]
        if not name or name in local_text:
            continue
        if shndx != SHN_UNDEF and shndx == text_idx:
            # defined in this object's own .text: an intra-module call, already
            # correct at the synthetic VMA.
            continue
        mod = None if base is None else base + off
        entry = {"symbol": name, "obj_off": off, "type": rtype,
                 "module_off": mod, "table_off": None, "shifted": False,
                 "stored": None, "obj": None,
                 "in_table": None, "op": None, "note": ""}
        if mod is None or mod >= text_size or off + 4 > len(obj_text):
            entry["note"] = "unmapped"
        else:
            entry["obj"] = stored_field(obj_text, off, rtype)
            rec = [r for r in table.get(mod, []) if r["mode"] == rtype]
            entry["in_table"] = bool(rec)
            entry["op"] = rec[0]["op_name"] if rec else None
            if rec:
                entry["table_off"] = mod
        sites.append(entry)

    # Give a site the table offset it belongs to before any addend is read.
    # Nothing changes for an object every one of whose sites is corroborated
    # where it stands; a schedule that moved a site is matched by order.
    alignment = align_sites(sites, records, base or 0,
                            (base or 0) + max(extent or 0, len(obj_text)))
    for entry in sites:
        if entry["module_off"] is None or entry["note"] == "unmapped":
            continue
        # An uncorroborated, unaligned site still reads its addend where it
        # stands: the corroboration filter below decides whether that reading
        # is used, exactly as before the aligner existed.
        site_off = entry["table_off"]
        if site_off is None:
            site_off = entry["module_off"]
        entry["stored"] = stored_field(rom, text_start + site_off,
                                       entry["type"])
    # A site the module's own relocation table does not name is not a
    # relocation site in the shipped image: reading an addend there reads an
    # instruction word.  When any site for a symbol is corroborated by the
    # table, ignore the ones that are not -- that is the schedule-divergence
    # filter, and it is the ROM's own statement, not a heuristic.
    corroborated = {s["symbol"] for s in sites if s["in_table"]}

    wanted = collections.defaultdict(set)
    unmapped = set()
    for hi, lo in _pairs(sites):
        anchor = hi or lo
        name = anchor["symbol"]
        if name in corroborated and not all(
                s["in_table"] for s in (hi, lo) if s is not None):
            continue
        if any(s is not None and s["stored"] is None for s in (hi, lo)):
            unmapped.add(name)
            continue
        if hi is not None and lo is not None and hi["type"] == R_MIPS_HI16:
            target = (hi["stored"] << 16) + sext16(lo["stored"])
            have = (hi["obj"] << 16) + sext16(lo["obj"])
        elif anchor["type"] == R_MIPS_26:
            target = SYNTHETIC_VMA | (anchor["stored"] << 2)
            have = anchor["obj"] << 2
        elif anchor["type"] == R_MIPS_32:
            target, have = anchor["stored"], anchor["obj"]
        elif anchor["type"] == R_MIPS_HI16:
            target, have = anchor["stored"] << 16, anchor["obj"] << 16
        else:
            target, have = sext16(anchor["stored"]), sext16(anchor["obj"])
        wanted[name].add((target - have) & 0xFFFFFFFF)

    # A symbol every one of whose sites was dropped by the corroboration
    # filter is not "fine": it means the module's table corroborates the symbol
    # somewhere, but not at any site this object still spells the same way. No
    # addend is readable, and saying nothing would leave the caller with an
    # undefined reference and no reason for it.
    seen = {s["symbol"] for s in sites}
    filtered = sorted(seen - set(wanted) - unmapped)

    values, conflicts = {}, []
    for name in filtered:
        conflicts.append((name, "no corroborated site survived: the schedule "
                                "diverges at every site the table names"))
    for name in sorted(wanted):
        if name in unmapped:
            conflicts.append((name, "unmapped site"))
        elif len(wanted[name]) == 1:
            values[name] = next(iter(wanted[name]))
        else:
            conflicts.append((name, f"{len(wanted[name])} distinct values: "
                                    f"{sorted(hex(v) for v in wanted[name])}"))
    for name in sorted(unmapped - set(wanted)):
        conflicts.append((name, "unmapped site"))
    return sites, values, conflicts, alignment


# ---------------------------------------------------------------- generate
#
# The whole of `overlay_undefined_syms.us.txt` is derivable.  It has exactly
# two kinds of line and both are mechanical:
#
#   value line   `gOverlay14Reset1C = 0x1C;`
#       the stored addend a placeholder extern must carry, synthesized by
#       `synthesize()` above from the baserom at the site the module's own
#       relocation table names.
#
#   alias line   `func_overlay_001_F0000CA8_184D088 = overlay1InterpolatePath;`
#       the generated splat identity for a module offset, pointed at whatever
#       friendly name the adopted C defines there.  `text_ownership` maps the
#       object to its module offset and the object's symbol table supplies the
#       name, so the pair is a pure function of the atlas plus the objects --
#       the same data the `--redefine-sym func_...=<friendly>` half of every
#       POSTPROCESS rule spells by hand.
#
# Generating both removes the duplicate assignments the hand-maintained file
# accumulated, and removes the alias-block coupling that made promoting one
# function in a shared TU strand another function's alias line.

STT_NOTYPE = 0
STT_OBJECT = 1
STT_FUNC = 2
STT_SECTION = 3
STB_GLOBAL = 1
SHN_ABS = 0xFFF1

GENERATED_HEADER = """\
/*
 * Generated by tools/reloc_surface.py generate -- do not edit by hand.
 * Regenerate with `gmake overlay-syms`; `gmake check-overlay-syms` fails on
 * drift.  See docs/reloc-surface.md.
 *
 * Raw overlay relocation addends used by adopted C, plus the generated-identity
 * aliases the splat assembly still references. The runtime linker supplies the
 * actual resident/module/section base; resolving these aliases to their stored
 * addends keeps the linked overlay text byte-identical.
 */"""


def generated_identity(overlay, module_off, rom_start):
    return "func_overlay_%03d_F%07X_%X" % (overlay, module_off,
                                           rom_start + module_off)


def linked_overlay_objects():
    """Every overlay object the link actually consumes, in ld-script order.

    The splat linker script names every input object explicitly, which is the
    only authoritative list: an earlier draft filtered build artifacts by
    whether the Makefile mentioned the object's name, and silently dropped the
    21 overlay objects that reach the link through a pattern rule.
    """
    ld = (REPO / f"mickey.{VERSION}.ld").read_text()
    seen = set()
    out = []
    missing = []
    for m in re.finditer(r"build/src/overlays/o(\d{3})/(\S+?\.o)\(", ld):
        rel = f"build/src/overlays/o{m.group(1)}/{m.group(2)}"
        if rel in seen:
            continue
        seen.add(rel)
        path = REPO / rel
        if path.is_file():
            out.append((int(m.group(1)), path))
        else:
            missing.append(rel)
    if missing:
        preview = "\n".join("  " + rel for rel in missing[:20])
        if len(missing) > 20:
            preview += "\n  ... and %d more" % (len(missing) - 20)
        raise SystemExit(
            "overlay relocation surface requires the complete linker object "
            "set; %d object(s) are missing:\n%s\n"
            "Build the overlay objects before generating or checking the "
            "surface." % (len(missing), preview))
    return out


def object_aliases(obj_path, overlay, atlas_rows, rom_start):
    """[(generated_identity, friendly_name)] for one object's defined text.

    Only global function symbols matter: a static is not linkable and a name
    that already *is* its generated identity needs no alias (and a self
    assignment would be circular).
    """
    elf = Elf(obj_path)
    text_idx, _ = elf.section(".text")
    if text_idx is None:
        return []
    base, _extent = tu_base(obj_path, overlay, atlas_rows)
    if base is None:
        return []
    out = []
    for name, value, _size, info, shndx in elf.symbols():
        if shndx != text_idx or not name:
            continue
        if (info >> 4) != STB_GLOBAL or (info & 0xF) != STT_FUNC:
            continue
        ident = generated_identity(overlay, base + value, rom_start)
        if name != ident:
            out.append((ident, name))
    return out


def resident_defined_names():
    """Every symbol the *resident* side of the link defines.

    An overlay's call to a resident target does not have to be spelled with a
    splat auto-name.  `overlay5InitializeAudio` calls `alHeapDBAlloc`,
    `osCreateMesgQueue` and `n_alCSPSetMessageQ` -- ordinary libultra globals --
    and a value line for one of those breaks the resident link exactly as
    `func_80034448` does.  What the two shapes have in common is not the name,
    it is that the resident side owns it, so that is what this measures: the
    global symbols defined by every non-overlay object the linker script names,
    plus the names the auto-generated symbol scripts assign.

    Deliberately excludes the overlay objects.  A call to another *module's*
    function is a different case, already handled by the generated-identity
    alias block (S5.3), and must keep its own name.
    """
    ld = (REPO / f"mickey.{VERSION}.ld").read_text()
    out = set()
    for m in re.finditer(r"(build/(?!src/overlays/)\S+?\.o)\(", ld):
        path = REPO / m.group(1)
        if not path.is_file():
            continue
        try:
            elf = Elf(path)
        except SystemExit:
            continue
        for name, _v, _s, info, shndx in elf.symbols():
            if name and shndx != SHN_UNDEF and (info >> 4) != 0:
                out.add(name)
    for fname in (f"undefined_funcs_auto.{VERSION}.txt",
                  f"undefined_syms_auto.{VERSION}.txt",
                  f"libultra_undefined_syms.{VERSION}.txt"):
        path = REPO / fname
        if path.is_file():
            for line in path.read_text().splitlines():
                m = re.match(r"^\s*([A-Za-z_]\w*)\s*=", line)
                if m:
                    out.add(m.group(1))
    return out


def resident_call_aliases(obj_path, overlay, atlas_rows, records,
                          resident_names=frozenset()):
    """(renames, refusals) for this object's calls to *resident* functions.

    An overlay module ships unrelocated, and §5.2's census says a `SYMBOL`
    `R_MIPS_26` record stores immediate zero whether its target is another
    module or the resident segment: the shipped word is `jal 0` and the runtime
    patches it.  So a resident call needs exactly the same addend every other
    cross-module call needs, `0xF0000000`, and nothing about it is special --
    except its *name*.

    Adopted C spells the call with splat's resident auto-name (`func_80029FE4`),
    and that name is global.  Assigning it in `overlay_undefined_syms.us.txt`
    does not give the overlay an addend, it moves the resident function for
    every resident caller as well: `func_80034448` is called from `models.c`,
    `level.c`, `menu.c` and four asm objects, and a value line for it turns all
    of them into `relocation truncated to fit: R_MIPS_26`.  That is the whole of
    the `resident-symbol-missing` and `relocation-truncated` classes.

    The fix is the one the hand-written POSTPROCESS rules already use (overlay
    49 rebinds `func_800254FC` to `overlay65UpdateReloc`): give the *site* a
    per-module placeholder name and value that.  This derives the same rebind
    mechanically -- one alias per (resident name, module) -- so the resident
    name keeps its real address and the overlay keeps its stored addend.

    Only `R_MIPS_26` sites are aliased.  A resident *data* reference stores the
    real resident address in its HI16/LO16 pair, so valuing it under its own
    name is already correct and already what the surface does (`D_80000040 =
    0x80000040`); renaming those would churn the file for no change in bytes.
    """
    elf = Elf(obj_path)
    syms = elf.symbols()
    base, _extent = tu_base(obj_path, overlay, atlas_rows)
    table = {}
    for r in records:
        table.setdefault(r["target_offset"], []).append(r)

    by_symbol = collections.defaultdict(list)
    for _sec, off, rtype, symidx in elf.relocations():
        if symidx >= len(syms):
            continue
        name, _v, _s, _i, shndx = syms[symidx]
        if shndx != SHN_UNDEF:
            continue
        if not (RESIDENT_NAME_RE.match(name) or name in resident_names):
            continue
        by_symbol[name].append((off, rtype))

    renames, refusals, notes = {}, [], []
    for name in sorted(by_symbol):
        sites = by_symbol[name]
        calls = [(off, t) for off, t in sites if t == R_MIPS_26]
        if not calls:
            continue
        other = [(off, t) for off, t in sites if t != R_MIPS_26]
        if other:
            refusals.append((name, "resident symbol reached by both a call and "
                                   "a data reference; one placeholder cannot "
                                   "carry both addends"))
            continue
        if base is None:
            refusals.append((name, "resident call in an object with no "
                                   "text_ownership row: no module offset"))
            continue
        # Corroboration is a *note* here, not a refusal.  `synthesize()`
        # already applies the module table's own filter when it picks the
        # addend, and under the alias a value read from an uncorroborated site
        # can only produce a differing word inside the promoted function --
        # which is the measurement the trial exists to take.  Refusing instead
        # costs the candidate its linked-ROM oracle for no gain:
        # `overlay34SortAndDraw` goes from 168 in-range words back to a bare
        # build failure.  Only the ambiguous cases above are refused.
        if not any([r for r in table.get(base + off, [])
                    if r["mode"] == R_MIPS_26] for off, _t in calls):
            notes.append((name,
                          "no resident call site (module offset %s) is named "
                          "by the module relocation table; the addend is read "
                          "from an uncorroborated site and the differing words "
                          "are reported in range"
                          % ", ".join("%#x" % (base + o) for o, _t in calls)))
        renames[name] = RESIDENT_ALIAS_FMT % (name, overlay)
    return renames, refusals, notes


def rebind_resident_calls(objects, rom, rows, mods, rom_table,
                          objcopy=None, records_cache=None,
                          refused_names=None, apply=True):
    """Plan resident-call placeholder renames and optionally apply them.

    Idempotent: the alias no longer matches `RESIDENT_NAME_RE`, so a second
    pass finds nothing to rename and `generate()` values the alias from the
    same corroborated site. Read-only callers use ``apply=False`` so checking a
    generated file can never modify the objects it is validating.
    """
    objcopy = objcopy or (REPO / "tools" / "binutils" / "mips64-elf-objcopy")
    refusals, notes, planned = [], [], []
    refused_names = set() if refused_names is None else refused_names
    resident_names = resident_defined_names()
    for ov, obj in objects:
        recs = (records_cache or {}).get(ov)
        if recs is None:
            recs = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
            if records_cache is not None:
                records_cache[ov] = recs
        try:
            renames, refused, noted = resident_call_aliases(
                obj, ov, rows.get(ov, []), recs, resident_names)
        except SystemExit:
            continue
        refusals += [(obj.name, n, why) for n, why in refused]
        notes += [(obj.name, n, why) for n, why in noted]
        refused_names.update(n for n, _why in refused)
        if not renames:
            continue
        planned += [(obj.name, old, renames[old]) for old in sorted(renames)]
        if not apply:
            continue
        import subprocess
        cmd = [str(objcopy)]
        for old in sorted(renames):
            cmd += ["--redefine-sym", "%s=%s" % (old, renames[old])]
        cmd.append(str(obj))
        subprocess.run(cmd, check=True)
    return refusals, notes, planned


def generate(rom, atlas, objects=None, quiet=False, rebind_resident=True,
             mutate_objects=True):
    """Return the whole linker-script block as text, plus a diagnostics dict."""
    rows = {m["overlay"]: m["text_ownership"] for m in atlas["modules"]}
    rom_starts = {m["overlay"]: int(m["rom"]["start"], 16) for m in atlas["modules"]}
    rom_table = ot.read_rom_table(rom)
    mods = ot.build_modules(ot.read_headers(rom))

    if objects is None:
        objects = linked_overlay_objects()

    values = {}          # name -> (value, first object that asked for it)
    conflicts = []       # (object, name, why)
    value_order = []     # (overlay, object name, [names])
    alias_pairs = {}     # identity -> (name, object)
    alias_order = []
    local = {}
    records = {}
    alignment = {}      # object name -> site-alignment summary
    # A resident call has to stop spelling itself with the resident function's
    # global name *before* anything is valued; see `rebind_resident_calls`.
    # In the matching tree this renames nothing (no overlay object carries an
    # R_MIPS_26 against a resident auto-name), so the generated block is
    # unchanged; it fires only for a promoted candidate.
    refused_resident = set()
    resident_notes = []
    pending_rebinds = []
    if rebind_resident:
        refused, resident_notes, planned = rebind_resident_calls(
            objects, rom, rows, mods, rom_table, records_cache=records,
            refused_names=refused_resident, apply=mutate_objects)
        conflicts += refused
        if not mutate_objects:
            pending_rebinds = planned
    for ov, obj in objects:
        if ov not in local:
            local[ov] = module_text_defs(objects, ov, rows.get(ov, []))
        if ov not in records:
            records[ov] = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
        recs = records[ov]
        _sites, vals, unresolved, aligned = synthesize(
            obj, ov, rom, rows.get(ov, []), recs, local[ov])
        conflicts += [(obj.name, n, why) for n, why in unresolved]
        if aligned["attempted"]:
            alignment[obj.name] = aligned
        # A refused resident call must not fall back to a value line under its
        # *global* name.  `synthesize()` still reads an addend for it from the
        # corroborated sites, but assigning `func_8002A8C0` in the linker
        # script moves the resident function for `shadows.c`, `camera.c` and
        # every other resident caller -- 30-odd `relocation truncated to fit`
        # errors, from a line meant to help one overlay.  Refused means
        # refused: no value, and the reason is already reported.
        vals = {k: v for k, v in vals.items() if k not in refused_resident}
        mine = []
        for name in sorted(vals):
            if name in values:
                if values[name][0] != vals[name]:
                    conflicts.append((obj.name, name,
                                      "conflicts with %s: %#x vs %#x"
                                      % (values[name][1], values[name][0],
                                         vals[name])))
                continue
            values[name] = (vals[name], obj.name)
            mine.append(name)
        if mine:
            value_order.append((ov, obj, mine))
        mine_aliases = []
        for ident, name in object_aliases(obj, ov, rows.get(ov, []),
                                          rom_starts[ov]):
            if ident in alias_pairs:
                if alias_pairs[ident][0] != name:
                    conflicts.append((obj.name, ident,
                                      "alias conflict: %s vs %s"
                                      % (alias_pairs[ident][0], name)))
                continue
            alias_pairs[ident] = (name, obj.name)
            mine_aliases.append((ident, name))
        if mine_aliases:
            alias_order.append((ov, obj, mine_aliases))

    # A value line is a linker-script assignment, and an assignment overrides
    # the definition for *every* module. Valuing a name another module
    # defines in its own .text therefore moves that module's function to the
    # stored addend: its linked symbol becomes *ABS*, and every canonical
    # ownership proof over that module refuses it. Overlay 49 and overlay 11
    # called overlay65Initialize and overlay66Select by their own names;
    # both sit at module offset 0, so the addend 0xF0000000 happened to equal
    # their real address and the ROM still matched. A cross-module call must
    # spell a *Reloc placeholder (an objcopy --redefine-sym in the caller's
    # POSTPROCESS rule), exactly as a resident call does. Refused means
    # refused: no value line, and the reason is reported.
    for ov in {ov for ov, _obj in objects}:
        if ov not in local:
            local[ov] = module_text_defs(objects, ov, rows.get(ov, []))
    foreign = foreign_text_values(values, local)
    if foreign:
        for name, (asker, owner) in sorted(foreign.items()):
            conflicts.append((asker, name,
                              "names overlay %d's own .text definition; a value "
                              "line would override it for every module -- call it "
                              "through a *Reloc placeholder" % owner))
        values = {k: v for k, v in values.items() if k not in foreign}
        value_order = [(ov, obj, [n for n in names if n not in foreign])
                       for ov, obj, names in value_order]
        value_order = [row for row in value_order if row[2]]

    # A generated identity that is aliased to a friendly name is *defined* by
    # the alias, so a value line for the same name would be a shadowed
    # assignment -- exactly the duplicate the hand-maintained file carried, and
    # ld silently took the last one.  The alias is the definition that must
    # win, so drop the shadowed value lines outright.
    shadowed = set(values) & set(alias_pairs)
    if shadowed:
        values = {k: v for k, v in values.items() if k not in shadowed}
        value_order = [(ov, obj, [n for n in names if n not in shadowed])
                       for ov, obj, names in value_order]
        value_order = [row for row in value_order if row[2]]

    lines = [GENERATED_HEADER, ""]
    lines.append("/* Stored relocation addends, per translation unit. */")
    for ov, obj, names in value_order:
        lines.append("")
        lines.append("/* overlay %d: %s */" % (ov, obj.name))
        for name in names:
            lines.append("%s = %#010x;" % (name, values[name][0]))
    lines.append("")
    lines.append("/* Generated assembly still uses address-qualified labels "
                 "for C boundaries. */")
    for ov, obj, pairs in alias_order:
        lines.append("")
        lines.append("/* overlay %d: %s */" % (ov, obj.name))
        for ident, name in pairs:
            lines.append("%s = %s;" % (ident, name))
    lines.append("")
    text = "\n".join(lines)

    diag = {"values": len(values), "aliases": len(alias_pairs),
            "objects": len(objects), "conflicts": conflicts,
            "alignment": alignment,
            "resident_notes": resident_notes, "shadowed": sorted(shadowed)}
    diag["pending_rebinds"] = pending_rebinds
    return text, diag


def cmd_generate(argv):
    p = argparse.ArgumentParser(
        prog="reloc_surface.py generate",
        description="Synthesize the whole overlay_undefined_syms block.")
    p.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    p.add_argument("--out", type=Path, default=LINK_SYMS)
    p.add_argument("--write", action="store_true",
                   help="rewrite --out in place (the default is to print)")
    p.add_argument("--check", action="store_true",
                   help="exit 1 if --out is not what the tree generates")
    p.add_argument("--quiet", action="store_true")
    p.add_argument("--compare", action="store_true",
                   help="report which tracked symbols the generator does not "
                        "reproduce, and which it adds")
    args = p.parse_args(argv)

    import json
    rom = args.rom.read_bytes()
    atlas = json.loads((REPO / "config" / "overlays.us.json").read_text())
    text, diag = generate(rom, atlas, rebind_resident=not args.compare,
                          mutate_objects=args.write)

    if diag.get("pending_rebinds"):
        print("overlay objects require resident-call symbol rebinding; "
              "read-only generation will not modify them:", file=sys.stderr)
        for obj, old, new in diag["pending_rebinds"][:20]:
            print("  %s: %s -> %s" % (obj, old, new), file=sys.stderr)
        if len(diag["pending_rebinds"]) > 20:
            print("  ... and %d more" % (len(diag["pending_rebinds"]) - 20),
                  file=sys.stderr)
        print("Declare the rebind in that object's POSTPROCESS rule, rebuild, "
              "and rerun the check.", file=sys.stderr)
        return 1

    if args.compare:
        old = args.out.read_text().splitlines()
        def names(lines):
            out = {}
            for line in lines:
                m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*(.+?)\s*;", line)
                if m:
                    out[m.group(1)] = m.group(2)
            return out
        a, b = names(old), names(text.splitlines())
        missing = sorted(set(a) - set(b))
        added = sorted(set(b) - set(a))
        changed = sorted(k for k in set(a) & set(b) if a[k] != b[k])
        print("tracked=%d generated=%d missing=%d added=%d changed=%d"
              % (len(a), len(b), len(missing), len(added), len(changed)))
        for n in changed:
            print("  CHANGED", n, a[n], "->", b[n])
        for n in missing:
            print("  MISSING", n, "=", a[n])
        for n in added[:60]:
            print("  ADDED  ", n, "=", b[n])
        return 0

    if args.check:
        misspelled = misspelled_generated_names(atlas, tree_generated_name_texts())
        for label, name, expected in misspelled:
            print("%s: %s does not match the atlas; the overlay/offset it "
                  "encodes is %s" % (label, name, expected), file=sys.stderr)
        if misspelled:
            return 1
        current = args.out.read_text() if args.out.is_file() else ""
        if current == text:
            if not args.quiet:
                print("%s: up to date (%d values, %d aliases, %d objects)"
                      % (args.out.name, diag["values"], diag["aliases"],
                         diag["objects"]))
            return 0
        print("%s: out of date; run `gmake overlay-syms`" % args.out.name,
              file=sys.stderr)
        import difflib
        for line in list(difflib.unified_diff(
                current.splitlines(), text.splitlines(),
                "tracked", "generated", lineterm=""))[:60]:
            print(line, file=sys.stderr)
        return 1

    if args.write:
        args.out.write_text(text)
        if not args.quiet:
            print("%s: %d values, %d aliases from %d objects"
                  % (args.out.name, diag["values"], diag["aliases"],
                     diag["objects"]))
    else:
        sys.stdout.write(text)
    for obj, name, why in diag["conflicts"]:
        print("/* UNRESOLVED %s %s: %s */" % (obj, name, why), file=sys.stderr)
    # A note is not a refusal: the symbol *is* valued and the link resolves it.
    # Printed under its own marker so a caller matching UNRESOLVED does not
    # class a measurable candidate as a failure.
    for obj, name, why in diag.get("resident_notes", []):
        print("/* NOTE %s %s: %s */" % (obj, name, why), file=sys.stderr)
    # An object whose sites did not all corroborate where they stand is
    # reported whether or not the alignment succeeded, so a shifted site is
    # never mistaken for an exact one and a refusal carries its reason.
    for obj, summary in sorted(diag.get("alignment", {}).items()):
        print("/* ALIGNED %s: %d/%d site(s), %d shifted%s */"
              % (obj, summary["aligned"], summary["sites"],
                 summary["shifted"],
                 "" if not summary["refused"] else
                 "; refused: " + summary["refused"]), file=sys.stderr)
    return 0


# ------------------------------------------------- permuter target annotation
#
# decomp-permuter scores a candidate object against a *target object* it
# assembles from splat's .s.  For an overlay function that comparison is
# invalid, and the reason is the same one section 1 gives: the shipped image
# stores addends, not addresses, so splat's .s assembles with no relocations
# at all.  Every cross-module call reads back as `jal <whatever symbol sits at
# module offset 0>` (the SYMBOL records all store immediate zero) and every
# address materialization as a bare `lui`/`addiu` pair carrying the stored
# addend.  The candidate object, compiled from C, carries an honest
# `R_MIPS_26` / `%hi`+`%lo` reference to a placeholder extern instead.
#
# The scorer's own symbol-difference rule cannot bridge that: it ignores a
# field mismatch only when the candidate's field looks like a symbol *and*
# the target line carries a relocation (scorer.py `field_matches_any_symbol(nf)
# and old_line.has_symbol`), and the target line carries none.  So a candidate
# that differs from the shipped ROM by two words scored 700 -- one insertion
# plus one deletion penalty for every relocation site in the function.
#
# The fix is not to relax the scorer but to give the target the relocations
# the shipped module says are there.  The module's own reloc1/reloc2 tables
# name every site and its type, and for a SYMBOL record they name the callee
# (overlay + offset) outright; that is a *stable identity*, independent of any
# name this tree happens to have chosen.  So:
#
#   * annotate every owned runtime site, pairing by runtime table order and
#     retaining standalone LO records, even without any candidate relocations;
#   * for each relocation the candidate's base object carries inside the
#     function, map its object offset to a module offset and look the site up
#     in the module's table.  A site the table does not name is not a
#     relocation site in the shipped image and is left alone (section 2);
#   * derive a corroborated candidate name for the *base symbol* --
#     `__ovcall_o<overlay>_<offset>` for a SYMBOL call, `__ovjump_<offset>`
#     for an intra-module JUMP, `__ovval_<value>` for a HI/LO site whose
#     value the ROM spells at the site -- subtracting whatever addend the
#     object already carries, exactly as `synthesize()` does;
#   * rewrite the target .s line to reference that name symbolically, and
#     rename the candidate's placeholder to the same name with
#     `objcopy --redefine-sym`.
#
# Both sides then render identically at every corroborated site, so the score
# reflects only real codegen difference -- and a candidate that calls the
# *wrong* placeholder still scores a penalty, because the canonical name comes
# from the ROM's record, not from the candidate's own symbol table.
#
# Nothing here writes ROM-derived content anywhere: the rewritten .s lives in
# the permuter's gitignored scratch, exactly like the label rename
# permute_batch.py already performs.

# splat's disassembly line: `/* <rom> <vma> <word> */  mnemonic operands`.
ASM_LINE_RE = re.compile(
    r"^(?P<pre>\s*/\* (?P<rom>[0-9A-Fa-f]+) (?P<vma>[0-9A-Fa-f]{8}) (?P<word>[0-9A-Fa-f]{8}) \*/\s*)"
    r"(?P<mnem>\S+)(?P<sp>\s*)(?P<ops>.*?)\s*$")
_MEM_OPERAND_RE = re.compile(r"^(?P<disp>.*)\((?P<reg>\$\w+)\)$")


class AnnotationError(Exception):
    """The target .s could not be annotated; the caller falls back."""


def _function_text_symbol(elf, names):
    """(offset, size) of the first of `names` defined in the object's .text."""
    text_idx, _ = elf.section(".text")
    if text_idx is None:
        raise AnnotationError("object has no .text")
    by_name = {}
    for name, value, size, _info, shndx in elf.symbols():
        if shndx == text_idx and name:
            by_name.setdefault(name, (value, size))
    for name in names:
        if name in by_name:
            return by_name[name]
    raise AnnotationError("none of %s is defined in the object's .text"
                          % ", ".join(names))


def _annotation_identity(record):
    op = record.get("op_name")
    if op not in {"SYMBOL", "LOCAL", "JUMP", "DATA"}:
        raise AnnotationError("unsupported runtime operation")
    index = record.get("symbol_index")
    if type(index) is not int or index < 0:
        raise AnnotationError("malformed runtime symbol index")
    target = ()
    if op == "SYMBOL":
        target = (record.get("target_overlay"), record.get("target_symbol_offset"))
        if any(type(value) is not int or value < 0 for value in target):
            raise AnnotationError("malformed runtime SYMBOL identity")
    return (op, index, *target)


def _annotation_register(value):
    names = ("zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 "
             "s0 s1 s2 s3 s4 s5 s6 s7 t8 t9 k0 k1 gp sp fp ra").split()
    value = value.strip().removeprefix("$")
    if value == "s8":
        return 30
    if value in names:
        return names.index(value)
    if value.startswith("f"):
        value = value[1:]
    if value.isdecimal() and 0 <= int(value) < 32:
        return int(value)
    raise AnnotationError("unsupported relocation register operand")


def _annotation_operand(match, rtype):
    """Validate opcode/register shape before replacing only its address field."""
    word = int(match.group("word"), 16)
    opcode, rt, rs = word >> 26, (word >> 16) & 31, (word >> 21) & 31
    mnemonic = match.group("mnem")
    fields = [field.strip() for field in match.group("ops").split(",")]
    if rtype == R_MIPS_26:
        if mnemonic not in {"j", "jal"} or opcode != {"j": 2, "jal": 3}[mnemonic] or len(fields) != 1:
            raise AnnotationError("unsupported R_MIPS_26 operand/opcode")
    elif rtype == R_MIPS_HI16:
        if mnemonic != "lui" or opcode != 15 or rs or len(fields) != 2 or _annotation_register(fields[0]) != rt:
            raise AnnotationError("unsupported HI16 operand/opcode")
    elif rtype == R_MIPS_LO16:
        memory = {"lb": 32, "lh": 33, "lwl": 34, "lw": 35, "lbu": 36,
                  "lhu": 37, "lwr": 38, "sb": 40, "sh": 41, "swl": 42,
                  "sw": 43, "swr": 46, "ll": 48, "lwc1": 49, "ldc1": 53,
                  "sc": 56, "swc1": 57, "sdc1": 61}
        if mnemonic in {"addi", "addiu"}:
            valid = (opcode == {"addi": 8, "addiu": 9}[mnemonic] and len(fields) == 3
                     and _annotation_register(fields[0]) == rt and _annotation_register(fields[1]) == rs)
        else:
            operand = _MEM_OPERAND_RE.match(fields[-1])
            valid = (mnemonic in memory and opcode == memory[mnemonic] and len(fields) == 2
                     and operand is not None and _annotation_register(fields[0]) == rt
                     and _annotation_register(operand.group("reg")) == rs)
        if not valid:
            raise AnnotationError("unsupported LO16 operand/opcode")
    else:
        raise AnnotationError("unsupported owned runtime relocation type %s" % rtype)


def _annotation_target_sites(records, line_of, lines, overlay, rom, text_start):
    """Decode the runtime table's own HI/LO order, including standalone LOs."""
    result, pairs = {}, set()
    selected = [record for record in records if record["target_offset"] in line_of]
    selected.sort(key=lambda record: (record["table"], record["index"]))
    cursor = 0
    while cursor < len(selected):
        record = selected[cursor]
        site, kind = record["target_offset"], record["mode"]
        identity = _annotation_identity(record)
        _annotation_operand(ASM_LINE_RE.match(lines[line_of[site]]), kind)
        members = [(site, {R_MIPS_HI16: "hi", R_MIPS_LO16: "lo", R_MIPS_26: "26"}[kind])]
        if kind == R_MIPS_HI16:
            if cursor + 1 >= len(selected):
                raise AnnotationError("incomplete target: runtime HI16 has no owned LO16")
            low = selected[cursor + 1]
            if low["mode"] != R_MIPS_LO16 or _annotation_identity(low) != identity:
                raise AnnotationError("ambiguous target runtime HI16/LO16 identity")
            lo_site = low["target_offset"]
            _annotation_operand(ASM_LINE_RE.match(lines[line_of[lo_site]]), R_MIPS_LO16)
            value = ((stored_field(rom, text_start + site, kind) << 16)
                     + sext16(stored_field(rom, text_start + lo_site, R_MIPS_LO16))) & 0xFFFFFFFF
            members.append((lo_site, "lo"))
            pairs.add((site, lo_site))
            cursor += 1
        elif kind == R_MIPS_LO16:
            value = sext16(stored_field(rom, text_start + site, kind)) & 0xFFFFFFFF
        else:
            value = stored_field(rom, text_start + site, kind) << 2
        if kind == R_MIPS_26 and identity[0] == "SYMBOL" and value == 0:
            name = "__ovcall_o%d_%X" % (record["target_overlay"], record["target_symbol_offset"])
        elif kind == R_MIPS_26 and identity[0] == "JUMP":
            name = "__ovjump_%06X" % value
        elif kind in {R_MIPS_HI16, R_MIPS_LO16} and identity[0] in {"SYMBOL", "LOCAL", "DATA"}:
            name = "__ovtarget_o%d_%s_v%08X" % (overlay, "_".join(str(x) for x in identity), value)
        else:
            raise AnnotationError("unsupported target runtime operation/type or call addend")
        for offset, form in members:
            result[offset] = (form, name, 0, value, identity)
        cursor += 1
    return result, pairs


def permuter_annotation(target_s_text, base_o: Path, func_names, overlay: int,
                        rom: bytes, records=None):
    """Annotate a permuter target .s with the module's own relocation sites.

    Returns ``(annotated_text, renames, notes)``:

      annotated_text  the .s with every owned runtime site rewritten to a
                      symbolic operand (unchanged if there are no records);
      renames         {candidate symbol: canonical name} to hand
                      `objcopy --redefine-sym`;
      notes           human-readable diagnostics (site/rename counts, and any
                      symbol whose sites disagree about its identity).

    Raises AnnotationError when the inputs cannot be mapped at all.
    """
    lines = target_s_text.split("\n")
    starts = [i for i, line in enumerate(lines) if re.fullmatch(
        r"\s*glabel\s+(?:" + "|".join(map(re.escape, func_names)) + r")\s*", line)]
    ends = [i for i, line in enumerate(lines) if re.fullmatch(
        r"\s*endlabel\s+(?:" + "|".join(map(re.escape, func_names)) + r")\s*", line)]
    if len(starts) != 1 or len(ends) != 1 or starts[0] >= ends[0]:
        raise AnnotationError("target lacks one unique symbol-owned glabel/endlabel range")
    line_of, addresses = {}, set()
    for i, raw in enumerate(lines):
        m = ASM_LINE_RE.match(raw)
        if m:
            offset = int(m.group("vma"), 16) - SYNTHETIC_VMA
            if offset < 0 or offset % 4 or offset in addresses:
                raise AnnotationError("duplicate or invalid addressed target offset")
            addresses.add(offset)
            if starts[0] < i < ends[0]:
                line_of[offset] = i
    if not line_of:
        raise AnnotationError("no addressed instruction lines in the target .s")
    fn_start = min(line_of)
    fn_end = max(line_of) + 4
    if sorted(line_of) != list(range(fn_start, fn_end, 4)):
        raise AnnotationError("target does not have one contiguous owned instruction range")

    mods = ot.build_modules(ot.read_headers(rom))
    if not 1 <= overlay <= len(mods):
        raise AnnotationError(f"overlay {overlay} is out of range")
    module = mods[overlay - 1]
    if fn_end > module["text_size"]:
        raise AnnotationError("addressed target exceeds module text ownership")
    if records is None:
        records = ot.read_module_relocations(rom, module, ot.read_rom_table(rom))
    table, seen = {}, set()
    for r in records:
        offset = r.get("target_offset")
        if (type(offset) is not int or offset < 0 or offset % 4
                or offset + 4 > module["text_size"] + module["data_size"]):
            raise AnnotationError("malformed or out-of-module runtime relocation site")
        if offset in line_of:
            if offset in seen:
                raise AnnotationError("duplicate owned runtime relocation site")
            if type(r.get("table")) is not int or type(r.get("index")) is not int:
                raise AnnotationError("runtime record lacks authoritative table/index order")
            seen.add(offset)
            _annotation_identity(r)
        table.setdefault((r["target_offset"], r["mode"]), r)
    text_start = module["rom_start"]
    for offset, index in line_of.items():
        match = ASM_LINE_RE.match(lines[index])
        if (int(match.group("rom"), 16) != text_start + offset
                or text_start + offset + 4 > len(rom)
                or int(match.group("word"), 16) != struct.unpack_from(">I", rom, text_start + offset)[0]):
            raise AnnotationError("addressed target ROM/word evidence disagrees with owned ROM")
    targets, target_pairs = _annotation_target_sites(records, line_of, lines, overlay, rom, text_start)

    elf = Elf(base_o)
    syms = elf.symbols()
    obj_text = elf.section_bytes(".text")
    fn_off, fn_size = _function_text_symbol(elf, func_names)

    sites = []
    for _sec, off, rtype, symidx in elf.relocations():
        if not (fn_off <= off < fn_off + fn_size) or symidx >= len(syms):
            continue
        name = syms[symidx][0]
        if not name:
            continue
        sites.append({"symbol": name, "obj_off": off, "type": rtype,
                      "module_off": fn_start + (off - fn_off)})

    # Pass 1: name each corroborated site by its stored link base. Equal
    # stored bases are not runtime identities: pass 2 qualifies collisions
    # with the authoritative operation/index tuple before any rename. An `R_MIPS_26`
    # SYMBOL site stores immediate zero (docs/reloc-surface.md section 1), so
    # its value carries no identity at all and the record's own ROM-table
    # entry -- the callee's overlay and offset -- names it instead.
    proposals, annotations = [], {offset: row[:3] for offset, row in targets.items()}
    for hi, lo in _pairs(sites):
        anchor = hi or lo
        members = [s for s in (hi, lo) if s is not None]
        recs = [table.get((s["module_off"], s["type"])) for s in members]
        if any(r is None for r in recs) or any(s["module_off"] not in targets for s in members):
            continue  # not a relocation site in the shipped image
        rec = recs[0]
        if hi is not None and lo is not None:
            if (hi["module_off"], lo["module_off"]) not in target_pairs:
                continue  # target pairing is runtime-owned, never guessed from candidate order
            have = ((stored_field(obj_text, hi["obj_off"], R_MIPS_HI16) << 16)
                    + sext16(stored_field(obj_text, lo["obj_off"], R_MIPS_LO16)))
            full = ((stored_field(rom, text_start + hi["module_off"],
                                  R_MIPS_HI16) << 16)
                    + sext16(stored_field(rom, text_start + lo["module_off"],
                                          R_MIPS_LO16)))
            value = (full - have) & 0xFFFFFFFF
            base = "__ovval_%08X" % value
            where = [(hi["module_off"], "hi"), (lo["module_off"], "lo")]
        elif anchor["type"] == R_MIPS_26:
            # `jal sym` carries no addend the assembler can spell back, so a
            # site whose object word is not a bare zero is left alone.
            if stored_field(obj_text, anchor["obj_off"], R_MIPS_26):
                continue
            have = 0
            if rec["op_name"] == "SYMBOL":
                base = "__ovcall_o%d_%X" % (rec["target_overlay"],
                                            rec["target_symbol_offset"])
            elif rec["op_name"] == "JUMP":
                base = "__ovjump_%06X" % (
                    stored_field(rom, text_start + anchor["module_off"],
                                 R_MIPS_26) << 2)
            else:
                continue
            value = targets[anchor["module_off"]][3]
            where = [(anchor["module_off"], "26")]
        else:
            continue  # a lone HI16/LO16 or an R_MIPS_32: nothing to pair with
        proposals.append((anchor["symbol"], base, have, where, _annotation_identity(rec), value))

    # Pass 2: a symbol whose sites do not agree on one name has no canonical
    # candidate identity; its target sites retain runtime-only annotations.
    # Renaming it either way
    # would make the target disagree with the candidate at the sites that
    # wanted the other name -- worse than leaving it alone, and silently so.
    proposed = collections.defaultdict(set)
    for symbol, base, _have, _where, identity, _value in proposals:
        proposed[symbol].add((base, identity))
    conflicts = ["%s: candidate identity ambiguous (%s); not renamed; target remains annotated"
                 % (symbol, ", ".join(str(name) for name in sorted(names)))
                 for symbol, names in sorted(proposed.items()) if len(names) > 1]
    renames = {symbol: next(iter(names))[0]
               for symbol, names in proposed.items() if len(names) == 1}
    # One coincident pair cannot bind a symbol whose other sites have moved
    # onto different runtime operations. Such a rename changes the whole
    # candidate symbol, not just its corroborated pair.
    for symbol in list(renames):
        identity = next(iter(proposed[symbol]))[1]
        owned_sites = [site for site in sites if site["symbol"] == symbol]
        if any((site["module_off"], site["type"]) not in table
               or site["module_off"] not in targets
               or targets[site["module_off"]][4] != identity for site in owned_sites):
            del renames[symbol]
            conflicts.append(symbol + ": incomplete candidate runtime correspondence; not renamed")
    destination_identities = collections.defaultdict(set)
    for symbol, destination in renames.items():
        destination_identities[destination].add(next(iter(proposed[symbol]))[1])
    for symbol, destination in list(renames.items()):
        if len(destination_identities[destination]) > 1:
            identity = next(iter(proposed[symbol]))[1]
            # No dots: the vendor MIPS scorer treats dotted operands as
            # generic symbols. Keep distinct runtime identities score-visible.
            renames[symbol] = destination + "_o%d_" % overlay + "_".join(map(str, identity))
    stored_bases = {}
    for symbol, _base, have, where, _identity, value in proposals:
        if symbol not in renames:
            continue
        base = renames[symbol]
        if base in stored_bases and stored_bases[base] != value:
            raise AnnotationError("ambiguous corroborated stored base")
        stored_bases[base] = value
        for module_off, kind in where:
            annotations[module_off] = (kind, base, have)

    # A shared-HI standalone LO does not establish a new candidate identity.
    # It may reuse an already corroborated one only when runtime identity and
    # the actual stored low field both agree with that existing base/addend.
    for site in sites:
        offset, symbol = site["module_off"], site["symbol"]
        if site["type"] != R_MIPS_LO16 or offset not in targets or symbol not in renames:
            continue
        _original_base, identity = next(iter(proposed[symbol]))
        base = renames[symbol]
        if not base.startswith("__ovval_") or identity != targets[offset][4] or targets[offset][0] != "lo":
            continue
        if annotations[offset][1] == base:
            continue  # retain a paired LO's complete (not merely signed-low) addend
        have = sext16(stored_field(obj_text, site["obj_off"], R_MIPS_LO16))
        value = stored_bases[base]
        if (value + have) & 0xFFFF == targets[offset][3] & 0xFFFF:
            annotations[offset] = ("lo", base, have)

    for module_off, (kind, base, have) in annotations.items():
        index = line_of.get(module_off)
        if index is None:
            continue
        m = ASM_LINE_RE.match(lines[index])
        spelled = base if not have else ("%s+0x%X" % (base, have) if have > 0 else "%s-0x%X" % (base, -have))
        if kind == "26":
            operands = spelled
        else:
            prefix = "%hi" if kind == "hi" else "%lo"
            fields = [f.strip() for f in m.group("ops").split(",")]
            mem = _MEM_OPERAND_RE.match(fields[-1])
            if mem:
                fields[-1] = "%s(%s)(%s)" % (prefix, spelled, mem.group("reg"))
            else:
                fields[-1] = "%s(%s)" % (prefix, spelled)
            operands = ", ".join(fields)
        lines[index] = m.group("pre") + m.group("mnem") + m.group("sp") + operands

    notes = ["%d relocation sites annotated, %d placeholder symbols renamed"
             % (len(annotations), len(renames))]
    notes.append("target coverage: %d/%d owned runtime records; candidate renames require independent corroboration"
                 % (len(annotations), len(seen)))
    values = {}
    for offset, (_kind, base, have) in annotations.items():
        value = (stored_bases[base] if base in stored_bases
                 else (targets[offset][3] - have) & 0xFFFFFFFF)
        if base in values and values[base] != value:
            raise AnnotationError("ambiguous stored-value assignment for annotated target")
        values[base] = value
    notes.append("target-proof: " + json.dumps({"values": values,
        "records": sorted([[offset - fn_start, {"26": R_MIPS_26, "hi": R_MIPS_HI16,
                                                "lo": R_MIPS_LO16}[annotations[offset][0]]]
                           for offset in annotations]),
        "rom_start": text_start + fn_start, "size": fn_end - fn_start}, sort_keys=True))
    notes += conflicts
    return "\n".join(lines), renames, notes


# -------------------------------------------------------------------- audit

def tracked_values(path=None):
    out = {}
    path = path or LINK_SYMS
    if not path.is_file():
        return out
    for line in path.read_text().splitlines():
        m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;", line)
        if m:
            out[m.group(1)] = int(m.group(2), 0)
    return out


def overlay_of(path: Path):
    m = re.search(r"/o(\d{3})/", str(path))
    return int(m.group(1)) if m else None


# ------------------------------------------------------- function comparison

class SurfaceComparisonError(Exception):
    """A function's target relocation ownership is not uniquely provable."""


SurfaceRecord = ri.RelocationRecord


def compare_record_sets(target, candidate):
    """Compact exactness census for two relocation-record collections."""
    return ri.compare_records(target, candidate)


def _source_from_object(path: Path):
    """Best-effort atlas source key from build*/src/<source>.c.o."""
    parts = path.as_posix().split("/")
    indexes = [i for i, part in enumerate(parts) if part == "src"]
    if not indexes:
        return None
    rel = "/".join(parts[indexes[-1] + 1:])
    for suffix in (".c.o", ".o", ".c"):
        if rel.endswith(suffix):
            rel = rel[:-len(suffix)]
            break
    return rel or None


def _canonical_source_key(source):
    """Normalize a source/object spelling to the linker object's source key."""
    if source is None:
        return None
    key = Path(source).as_posix()
    if key.startswith("src/"):
        key = key[4:]
    for suffix in (".c.o", ".o", ".c"):
        if key.endswith(suffix):
            key = key[:-len(suffix)]
            break
    return key or None


def resolve_resident_target_object(candidate_object, source=None):
    """Return the unique canonical object carrying a resident target body.

    Resident target relocations are static linker relocations, not entries in
    the game's resident runtime patch table.  The ordinary ``build`` object
    retains the fallback assembly's exact relocation surface, while a
    ``build_non_matching`` or scratch object carries the candidate C surface.
    Require those two objects to name the same canonical translation unit;
    never guess from a basename.
    """
    inferred = _canonical_source_key(_source_from_object(Path(candidate_object)))
    asserted = _canonical_source_key(source)
    if inferred and asserted and inferred != asserted:
        raise SurfaceComparisonError(
            "resident source is ambiguous: candidate path implies %s but "
            "--source asserts %s" % (inferred, asserted))
    key = asserted or inferred
    if not key:
        raise SurfaceComparisonError(
            "resident candidate has no canonical build*/src source; supply "
            "--source")
    if key.startswith("overlays/"):
        raise SurfaceComparisonError(
            "%s is an overlay source, not a resident translation unit" % key)
    target = REPO / "build" / "src" / (key + ".c.o")
    if not target.is_file():
        raise SurfaceComparisonError(
            "missing canonical resident target object %s" % target)
    return target


def resolve_overlay_ownership(obj_path, atlas, overlay_hint=None, source=None):
    """Return the unique ``(module, ownership row)`` or ``None`` for resident.

    A hint is an assertion, never a way to choose between duplicate rows.
    Scratch objects may supply ``source`` explicitly when their path no longer
    contains the original ``build*/src`` context.
    """
    source = source or _source_from_object(obj_path)
    path_says_overlay = bool(source and source.startswith("overlays/"))
    rows = []
    if source:
        for module in atlas.get("modules", []):
            for row in module.get("text_ownership", []):
                if row.get("type") == "c" and row.get("source") == source:
                    rows.append((module, row))
    if not rows and overlay_hint is not None:
        stem = Path(source or obj_path.name).name
        for module in atlas.get("modules", []):
            if module.get("overlay") != overlay_hint:
                continue
            for row in module.get("text_ownership", []):
                if (row.get("type") == "c"
                        and Path(row.get("source", "")).name == stem):
                    rows.append((module, row))
    if len(rows) > 1:
        raise SurfaceComparisonError(
            "%s has %d overlay text owners; supply a unique canonical source"
            % (source or obj_path, len(rows)))
    if not rows:
        if overlay_hint is not None or path_says_overlay:
            raise SurfaceComparisonError(
                "%s has no overlay text_ownership row" % (source or obj_path))
        return None
    module, row = rows[0]
    if overlay_hint is not None and module["overlay"] != overlay_hint:
        raise SurfaceComparisonError(
            "overlay hint %d disagrees with atlas owner %d"
            % (overlay_hint, module["overlay"]))
    return module, row


def _unique_symbol(elf, name, *, require_text=False):
    found = []
    for symbol, value, size, _info, shndx in elf.symbols():
        if symbol != name or shndx == SHN_UNDEF or size == 0:
            continue
        section = elf.names[shndx] if shndx < len(elf.names) else ""
        if require_text and section != ".text":
            continue
        found.append((value, size, section))
    if len(found) != 1:
        raise SurfaceComparisonError(
            "%s: expected one nonzero definition of %s, found %d"
            % (elf.path, name, len(found)))
    return found[0]


RESERVED_SELECTORS = frozenset((0xFFD, 0xFFE, 0xFFF))


def _resident_versus_reserved(first, second):
    """True when one identity is resident and the other a reserved selector."""
    kinds = {first[0], second[0]}
    return 0 in kinds and bool(kinds & RESERVED_SELECTORS) and len(kinds) == 2


def _identity_add(identity, addend):
    return ri.effective_identity(identity, addend)


def _numeric_assignments(path):
    if not path.is_file():
        return {}
    try:
        return ri.parse_numeric_assignments(path.read_text())
    except ri.RelocationIdentityError as error:
        raise SurfaceComparisonError(str(error)) from error


def _stable_symbol_identities(path, candidate_elf, overlay, tu_base_offset,
                              target_elf, redefine_aliases=None):
    """Map names to unambiguous ``(overlay, offset)`` runtime identities."""
    proposed = collections.defaultdict(set)

    def propose(name, identity):
        if name:
            proposed[name].add(identity)

    # Generated overlay identities and their friendly aliases are stable even
    # though every module shares the same synthetic VMA.
    equality_aliases = []
    if path.is_file():
        equality_aliases = ri.parse_linker_aliases(path.read_text())
        for generated, _friendly in equality_aliases:
            gm = GEN_NAME_RE.match(generated)
            if gm:
                propose(generated, (int(gm.group(1)), int(gm.group(2), 16)))

    # Resident auto-names and every resident address exported by the linked
    # build have a unique address space. Overlay-valued ELF symbols are not
    # admitted here: F0000000 is deliberately shared by 107 modules.
    for elf in (candidate_elf, target_elf):
        for name, value, _size, info, shndx in elf.symbols():
            # A section symbol names one object's input section, never a
            # program-wide identity: every TU has its own `.rodata`, and an
            # externalized one survives in the linked ELF as `*ABS*` zero.
            if not name or shndx == SHN_UNDEF or info & 0xF == STT_SECTION:
                continue
            if 0x80000000 <= value < 0x90000000:
                propose(name, (0, value - ot.RESIDENT_VRAM_BASE))
            elif overlay is None and shndx == SHN_ABS:
                # Resident assembly sometimes names a physical/RSP address
                # through an ELF absolute assignment. It is not a resident
                # pointer, so keep it in a distinct identity namespace rather
                # than adding the resident VRAM base to the link value.
                propose(name, (ri.ABSOLUTE_IDENTITY, value))
    for elf in (candidate_elf, target_elf):
        for name, _value, _size, _info, _shndx in elf.symbols():
            m = RESIDENT_NAME_RE.match(name)
            if m:
                propose(name, (0, int(name[-8:], 16) - ot.RESIDENT_VRAM_BASE))

    # Definitions in the candidate TU are uniquely placed by its atlas owner.
    # This covers intra-overlay JUMPs without consulting the shared linked VMA.
    if overlay is not None:
        text_idx, _ = candidate_elf.section(".text")
        for name, value, _size, _info, shndx in candidate_elf.symbols():
            if name and shndx == text_idx:
                propose(name, (overlay, tu_base_offset + value))

    redefine_pairs = [
        (source, destination)
        for destination, source in (redefine_aliases or {}).items()
    ]
    try:
        resolution = ri.resolve_identities(
            proposed,
            equality_aliases=equality_aliases,
            redefine_aliases=redefine_pairs,
        )
    except ri.RelocationIdentityError as error:
        raise SurfaceComparisonError(str(error)) from error
    return resolution.resolved, set(resolution.ambiguous)


def _atlas_hex(row, field, description):
    value = row.get(field)
    if not isinstance(value, str) or not re.fullmatch(r"0x[0-9A-Fa-f]+", value):
        raise SurfaceComparisonError(
            "%s has invalid %s %r" % (description, field, value))
    return int(value, 16)


def _callee_build_dependencies(root, source_path, source_text):
    """Conservative canonical dependency set for an identity-only read proof.

    This does not build or create a compiler receipt. Literal header lookup
    follows the canonical C rule (including asm-processor's source directory);
    unsupported includes fail closed. Build policy and compiler/metadata tools
    are timestamp prerequisites even where make's object rule omits them.
    """
    import permute_batch as batch
    import subprocess
    import time
    root = Path(root)
    if root.resolve() != batch.ROOT.resolve():
        return None
    try:
        deadline = time.monotonic() + 5
        recipe = batch.build_recipe_for(source_path, deadline=deadline)
        if not recipe.from_dry_run or not recipe.compiler_args:
            return None
        # asm-processor appends the original source directory to the real
        # compiler's -I list because its temporary C lives elsewhere.
        includes = batch.source_dependencies(
            source_path, recipe.compiler_args + ("-I", str(source_path.parent)), deadline)
    except (OSError, RuntimeError, ValueError, subprocess.SubprocessError):
        return None
    if any(name.startswith("missing:") for name in includes):
        return None
    dependencies = set(path for path in (root / "include").rglob("*")
                       if path.is_file())
    dependencies.update(path for path in (root / "mk").rglob("*.mk") if path.is_file())
    # Rebind/filter specs are build inputs too; their suffix is not uniformly
    # JSON and editing them need not alter the expanded command string.
    dependencies.update(path for path in (root / "config/normalizations").rglob("*")
                        if path.is_file())
    dependencies.update(path for path in (root / "tools/ido").rglob("*")
                        if path.is_file())
    dependencies.update(path for path in (root / "tools/asm-processor").glob("*.py")
                        if path.is_file())
    for relative in (
            "Makefile", "build/.splat-stamp", "tools/binutils/mips64-elf-as",
            "tools/binutils/mips64-elf-objcopy", "tools/normalize_elf_instructions.py",
            "tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
            "tools/externalize_elf_section.py", "tools/rebind_elf_relocations.py",
            "tools/set_elf_flags.py", "tools/render_overlay_aliases.py"):
        path = root / relative
        if path.is_file():
            dependencies.add(path)
    dependencies.update(root / name for name in includes)
    return dependencies, (recipe, includes)


def _canonical_overlay_call_boundary(atlas, source_overlay, generated_name,
                                     target_elf, root=None,
                                     elf_loader=None, rom=None):
    """Authenticate generated calls from canonical ownership, never name shape.

    Generated overlay names are not identities merely because their linked
    value lies under the shared synthetic VMA. The legacy same-overlay
    owner-start route retains its physically trimmed owner boundary behavior.
    Cross-overlay and interior C members additionally require an
    ordinary definition, exact-C ownership, fresh object, bounded function
    symbols, linked ROM equality and unchanged non-relocation instruction bits.
    Contradictory evidence is an error; missing proof supplies no identity.
    """
    match = GEN_NAME_RE.fullmatch(generated_name)
    if not match:
        return None
    target_overlay = int(match.group(1))
    target_offset = int(match.group(2), 16)

    modules = [
        module for module in atlas.get("modules", [])
        if module.get("overlay") == target_overlay
    ]
    if len(modules) > 1:
        raise SurfaceComparisonError(
            "atlas has ambiguous duplicate rows for overlay %d" % target_overlay)
    if not modules:
        return None
    module = modules[0]
    if module.get("identity") != "overlay:%d" % target_overlay:
        raise SurfaceComparisonError(
            "overlay %d atlas identity conflicts with its module row"
            % target_overlay)
    if _atlas_hex(module, "synthetic_vma", "overlay module") != SYNTHETIC_VMA:
        raise SurfaceComparisonError(
            "overlay %d atlas synthetic VMA conflicts with canonical policy"
            % target_overlay)
    text = module.get("sections", {}).get("text")
    rom_row = module.get("rom")
    if not isinstance(text, dict) or not isinstance(rom_row, dict):
        return None
    text_start = _atlas_hex(text, "start", "overlay text section")
    text_end = _atlas_hex(text, "end", "overlay text section")
    text_size = _atlas_hex(text, "size", "overlay text section")
    rom_start = _atlas_hex(rom_row, "start", "overlay ROM row")
    if text_start != rom_start or text_end - text_start != text_size:
        raise SurfaceComparisonError(
            "overlay %d atlas text ownership is internally inconsistent"
            % target_overlay)
    if int(generated_name.rsplit("_", 1)[1], 16) != rom_start + target_offset:
        raise SurfaceComparisonError(
            "%s encoded ROM address conflicts with atlas ownership" % generated_name)

    covering = []
    for row in module.get("text_ownership", []):
        if not isinstance(row, dict):
            raise SurfaceComparisonError(
                "overlay %d has a malformed text ownership row" % target_overlay)
        start = _atlas_hex(row, "offset", "text ownership row")
        end = _atlas_hex(row, "end_offset", "text ownership row")
        size = _atlas_hex(row, "size", "text ownership row")
        if end - start != size or not 0 <= start < end <= text_size:
            raise SurfaceComparisonError(
                "overlay %d text ownership boundary is inconsistent"
                % target_overlay)
        if start <= target_offset < end:
            covering.append((row, start, end, size))
    if len(covering) > 1:
        raise SurfaceComparisonError(
            "%s has ambiguous overlapping atlas owners" % generated_name)
    if not covering:
        return None
    row, row_start, _row_end, row_size = covering[0]
    # Containment in a section or broad TU does not prove a function start.
    if row.get("type") != "c":
        return None
    strict_c = target_overlay != source_overlay or row_start != target_offset
    if strict_c and rom is None:
        return None

    source = row.get("source")
    if not isinstance(source, str) or not source:
        return None
    source_rel = Path(source)
    if source_rel.is_absolute() or ".." in source_rel.parts:
        raise SurfaceComparisonError(
            "%s has an unsafe atlas source path" % generated_name)
    if strict_c and source_rel.parts[:2] != ("overlays", "o%03d" % target_overlay):
        raise SurfaceComparisonError("generated callee source belongs to another overlay")
    root = REPO if root is None else Path(root)
    source_path = root / "src" / (source + ".c")
    object_path = root / "build" / "src" / (source + ".c.o")
    if not source_path.is_file() or not object_path.is_file():
        return None
    # A stale object cannot authenticate the tracked source/owner relation.
    if object_path.stat().st_mtime_ns < source_path.stat().st_mtime_ns:
        return None
    target_path = getattr(target_elf, "path", None)
    if (isinstance(target_path, Path) and target_path.is_file()
            and target_path.stat().st_mtime_ns < object_path.stat().st_mtime_ns):
        return None
    if strict_c:
        # Detect ordinary concurrent replacement while this proof is read.
        # This is dependency freshness, not a reusable compiler-context receipt.
        if not isinstance(target_path, Path) or not target_path.is_file():
            return None
        import proof_provenance as provenance
        source_text = source_path.read_text().replace("\\\n", "")
        dependency_proof = _callee_build_dependencies(root, source_path, source_text)
        if dependency_proof is None:
            return None
        dependencies, _context = dependency_proof
        proof_paths = [source_path, object_path, target_path, *dependencies]
        def file_state(path):
            stat = path.stat()
            return stat.st_dev, stat.st_ino, stat.st_size, stat.st_mtime_ns, stat.st_ctime_ns
        before_files = {path: file_state(path) for path in proof_paths}
        object_time = before_files[object_path][3]
        if (before_files[source_path][3] > object_time
                or before_files[target_path][3] < object_time
                or any(before_files[path][3] > object_time for path in dependencies)):
            return None
        # target_elf was parsed by the caller before this helper. Pin its
        # actual bytes, not merely whichever file now occupies its pathname.
        if getattr(target_elf, "data", None) != target_path.read_bytes():
            return None
        object_bytes_before = object_path.read_bytes()
        if source_path.read_text().replace("\\\n", "") != source_text:
            return None
        # This route proves an actual ordinary C definition, not a label in
        # fallback assembly. Unknown conditional compilation is not evidence.
        facts = provenance.source_facts(source_text, generated_name)
        if len(facts.definitions) != 1 or facts.pragmas:
            return None
        depth = 0
        for line in provenance._mask_c(source_text).splitlines()[:facts.definitions[0].line - 1]:
            if re.match(r"^\s*#\s*(if|ifdef|ifndef)\b", line):
                depth += 1
            elif re.match(r"^\s*#\s*endif\b", line):
                depth -= 1
            if depth < 0:
                return None
        if depth:
            return None

    loader = Elf if elf_loader is None else elf_loader
    canonical = loader(object_path)
    if strict_c and getattr(canonical, "data", None) != object_bytes_before:
        return None
    text_index, _text_header = canonical.section(".text")
    if text_index is None or len(canonical.section_bytes(".text")) != row_size:
        return None
    canonical_functions = [
        (name, value, size)
        for name, value, size, info, shndx in canonical.symbols()
        if shndx == text_index and (info & 0xF) == STT_FUNC
    ]
    if strict_c:
        named = [row for row in canonical.symbols()
                 if row[0] == generated_name and row[4] != SHN_UNDEF]
        if len(named) > 1:
            raise SurfaceComparisonError("generated callee has ambiguous canonical definitions")
        if named and (named[0][4] != text_index or named[0][3] & 0xF != STT_FUNC):
            return None
    definitions = [
        row for row in canonical_functions if row[0] == generated_name
    ]
    if len(definitions) > 1:
        raise SurfaceComparisonError(
            "%s canonical owner has ambiguous function symbols" % generated_name)
    if not definitions:
        return None
    _name, object_value, object_symbol_size = definitions[0]
    expected_object_value = target_offset - row_start
    if object_value != expected_object_value or object_symbol_size <= 0:
        raise SurfaceComparisonError(
            "%s canonical object symbol conflicts with atlas boundary"
            % generated_name)
    if strict_c:
        object_end = object_value + object_symbol_size
        if object_end > row_size:
            raise SurfaceComparisonError("generated callee escapes canonical object ownership")
        for name, value, size in canonical_functions:
            if name == generated_name or (value, size) == (object_value, object_symbol_size):
                continue
            if size > 0 and value < object_end and object_value < value + size:
                raise SurfaceComparisonError("generated callee has overlapping function boundaries")
        exact_ranges = []
        for exact in module.get("mixed_tu_exact_c_ranges", []):
            if not isinstance(exact, dict):
                raise SurfaceComparisonError("malformed mixed-TU exact ownership")
            a = _atlas_hex(exact, "offset", "mixed-TU exact ownership")
            b = _atlas_hex(exact, "end_offset", "mixed-TU exact ownership")
            n = _atlas_hex(exact, "size", "mixed-TU exact ownership")
            if b - a != n or not row_start <= a < b <= text_size:
                # Other TUs have independent ownership, checked when used.
                if exact.get("source") == source:
                    raise SurfaceComparisonError("inconsistent mixed-TU exact ownership")
            if a <= target_offset < b:
                exact_ranges.append(exact)
        if exact_ranges:
            if len(exact_ranges) != 1:
                raise SurfaceComparisonError("ambiguous mixed-TU exact ownership")
            exact = exact_ranges[0]
            if (exact.get("source") != source
                    or _atlas_hex(exact, "offset", "exact owner") != target_offset
                    or _atlas_hex(exact, "size", "exact owner") != object_symbol_size):
                raise SurfaceComparisonError("generated callee conflicts with exact ownership")
        elif row.get("matched") is not True or row.get("nonmatching") is not False:
            return None

    linked = []
    if strict_c:
        named = [row for row in target_elf.symbols()
                 if row[0] == generated_name and row[4] != SHN_UNDEF]
        if len(named) > 1:
            raise SurfaceComparisonError("generated callee has ambiguous linked definitions")
        if named and (named[0][4] == SHN_ABS or named[0][3] & 0xF != STT_FUNC):
            return None
    for name, value, size, info, shndx in target_elf.symbols():
        if (name == generated_name and shndx != SHN_UNDEF
                and (info & 0xF) == STT_FUNC):
            section = target_elf.names[shndx] if shndx < len(target_elf.names) else ""
            linked.append((value, size, section))
    if len(linked) > 1:
        raise SurfaceComparisonError(
            "%s has ambiguous linked function symbols" % generated_name)
    if not linked:
        return None
    linked_value, linked_size, linked_section = linked[0]
    if (linked_value != SYNTHETIC_VMA + target_offset
            or linked_section != ".overlay_%03d" % target_overlay
            or linked_size != object_symbol_size):
        raise SurfaceComparisonError(
            "%s linked symbol conflicts with canonical overlay ownership"
            % generated_name)
    if strict_c:
        section_index, section_header = target_elf.section(linked_section)
        if section_index is None or not isinstance(section_header, tuple):
            return None
        if section_header[3] != SYNTHETIC_VMA:
            raise SurfaceComparisonError("generated callee linked section has wrong base")
        end = target_offset + object_symbol_size
        linked_bytes = target_elf.section_bytes(linked_section)[target_offset:end]
        retail_bytes = rom[rom_start + target_offset:rom_start + end]
        if (len(linked_bytes) != object_symbol_size
                or len(retail_bytes) != object_symbol_size
                or linked_bytes != retail_bytes):
            return None
        # Relocation values differ before linking; instruction bits may not.
        masks = {}
        for _section, offset, rtype, _symbol in canonical.relocations():
            if not object_value <= offset < object_end:
                continue
            if offset % 4 or offset + 4 > object_end or offset in masks:
                raise SurfaceComparisonError("invalid generated callee relocation geometry")
            mask = {R_MIPS_26: 0xFC000000, R_MIPS_HI16: 0xFFFF0000,
                    R_MIPS_LO16: 0xFFFF0000, R_MIPS_PC16: 0xFFFF0000,
                    R_MIPS_32: 0}.get(rtype)
            if mask is None:
                return None
            masks[offset] = mask
        if object_value % 4 or object_symbol_size % 4:
            return None
        object_text = canonical.section_bytes(".text")
        for relative in range(0, object_symbol_size, 4):
            offset = object_value + relative
            raw_word = struct.unpack_from(">I", object_text, offset)[0]
            linked_word = struct.unpack_from(">I", linked_bytes, relative)[0]
            if (raw_word ^ linked_word) & masks.get(offset, 0xFFFFFFFF):
                return None
        try:
            if any(file_state(path) != state for path, state in before_files.items()):
                raise SurfaceComparisonError("generated callee proof inputs changed during inspection")
            if _callee_build_dependencies(root, source_path, source_text) != dependency_proof:
                raise SurfaceComparisonError("generated callee proof dependency set changed")
        except OSError as error:
            raise SurfaceComparisonError("generated callee proof input disappeared") from error
    return target_overlay, target_offset


def _stable_overlay_call_identities(path, candidate_elf, source_overlay,
                                    target_elf, atlas, start, size,
                                    redefine_aliases=None, root=None,
                                    elf_loader=None, module=None, rom=None,
                                    runtime_module=None, target_records=None,
                                    own_range=None):
    """Resolve uniquely boundary- or sibling-authenticated ``R_MIPS_26`` names.

    ``own_range`` is the module-offset extent of the function under proof;
    it is never its own sibling witness (see
    ``_matched_overlay_relocation_witnesses``).
    """
    symbols = candidate_elf.symbols()
    proposed = collections.defaultdict(set)
    reverse = redefine_aliases or {}
    valid = set()
    for _section, offset, rtype, symbol_index in candidate_elf.relocations():
        if rtype != R_MIPS_26 or not start <= offset < start + size:
            continue
        current = symbols[symbol_index][0] if symbol_index < len(symbols) else ""
        original = reverse.get(current, current)
        if original:
            valid.add(original)
        identity = _canonical_overlay_call_boundary(
            atlas, source_overlay, original, target_elf,
            root=root, elf_loader=elf_loader, rom=rom)
        if identity is not None:
            proposed[original].add(identity)

    if module is not None and rom is not None and runtime_module is not None:
        witnessed = _matched_overlay_relocation_witnesses(
            module, target_elf, rom, runtime_module, valid,
            root=root, elf_loader=elf_loader, exclude_range=own_range)
        for name, identities in witnessed.items():
            proposed[name].update(identities)

    runtime_ambiguous = set()
    if module is not None and target_records is not None:
        if source_overlay != module.get("overlay"):
            raise SurfaceComparisonError(
                "overlay call owner conflicts with its atlas module")
        correlated, runtime_ambiguous = (
            _runtime_correlated_overlay_r26_identities(
                candidate_elf, module, start, size, target_records,
                redefine_aliases))
        for name, identity in correlated.items():
            proposed[name].add(identity)

    equality_aliases = []
    if path.is_file():
        equality_aliases = ri.parse_linker_aliases(path.read_text())
    redefine_pairs = [
        (source, destination)
        for destination, source in reverse.items()
    ]
    try:
        resolution = ri.resolve_identities(
            proposed,
            equality_aliases=equality_aliases,
            redefine_aliases=redefine_pairs,
        )
    except ri.RelocationIdentityError as error:
        raise SurfaceComparisonError(str(error)) from error
    ambiguous = set(resolution.ambiguous) | runtime_ambiguous
    resolved = {
        name: identity for name, identity in resolution.resolved.items()
        if name not in ambiguous
    }
    return resolved, ambiguous


def _overlay_module_extent(module, field, description):
    value = module.get(field)
    if not isinstance(value, str) or not re.fullmatch(r"0x[0-9A-Fa-f]+", value):
        raise SurfaceComparisonError(
            "overlay %s has invalid %s %r" %
            (module.get("overlay"), description, value))
    return int(value, 16)


def _canonical_overlay_data_identity(module, candidate_elf, name,
                                     numeric_value, target_elf, root=None,
                                     elf_loader=None):
    """Authenticate one LOCAL/data base without consulting target tuples.

    The linker deliberately contains two symbols with the placeholder's name:
    an ABS assignment carrying the object's section-relative addend, and the
    canonical data definition at its overlay-specific linked address.  The
    latter is a stable identity only after a unique fresh canonical object
    proves the same definition and section-relative value.
    """
    overlay = module.get("overlay")
    if not isinstance(overlay, int):
        raise SurfaceComparisonError("overlay module has no numeric identity")
    if module.get("identity") != "overlay:%d" % overlay:
        raise SurfaceComparisonError(
            "overlay %d atlas identity conflicts with its module row" % overlay)
    if _atlas_hex(module, "synthetic_vma", "overlay module") != SYNTHETIC_VMA:
        raise SurfaceComparisonError(
            "overlay %d atlas synthetic VMA conflicts with canonical policy"
            % overlay)

    absolute = [
        (value, size, info)
        for symbol, value, size, info, shndx in target_elf.symbols()
        if symbol == name and shndx == SHN_ABS
    ]
    if len(absolute) > 1:
        raise SurfaceComparisonError(
            "candidate data symbol %s has ambiguous linked assignments" % name)
    if not absolute:
        return None
    if absolute[0][0] != numeric_value:
        raise SurfaceComparisonError(
            "candidate data symbol %s has a conflicting linked assignment" % name)

    linked = []
    for symbol, value, size, info, shndx in target_elf.symbols():
        if symbol != name or shndx in (SHN_UNDEF, SHN_ABS):
            continue
        section = target_elf.names[shndx] if shndx < len(target_elf.names) else ""
        if section in (".overlay_%03d" % overlay,
                       ".overlay_%03d_bss" % overlay):
            linked.append((value, size, info, section))
    if len(linked) > 1:
        raise SurfaceComparisonError(
            "candidate data symbol %s has ambiguous overlay definitions" % name)
    if not linked:
        # A friendly candidate name may intentionally have no linked data
        # alias.  A sole canonical object that owns the *entire* output BSS is
        # still sufficient provenance: the ABS assignment is then an exact
        # byte offset in that object, independent of any target relocation.
        root = REPO if root is None else Path(root)
        loader = Elf if elf_loader is None else elf_loader
        output_index, output_header = target_elf.section(
            ".overlay_%03d_bss" % overlay)
        if output_index is None or not isinstance(output_header, tuple):
            return None
        output_address, output_size = output_header[3], output_header[5]
        rom_size = _atlas_hex(module.get("rom", {}), "size", "overlay ROM row")
        bss_size = _overlay_module_extent(module, "bss_size", "BSS size")
        if (output_address != SYNTHETIC_VMA + rom_size
                or output_size > bss_size or numeric_value >= output_size):
            return None
        whole_owners = []
        for row in module.get("text_ownership", []):
            if not isinstance(row, dict) or row.get("type") != "c":
                continue
            source = row.get("source")
            if not isinstance(source, str):
                continue
            source_rel = Path(source)
            if source_rel.is_absolute() or ".." in source_rel.parts:
                raise SurfaceComparisonError(
                    "overlay %d has an unsafe canonical source path" % overlay)
            source_path = root / "src" / (source + ".c")
            object_path = root / "build" / "src" / (source + ".c.o")
            if not source_path.is_file() or not object_path.is_file():
                continue
            if object_path.stat().st_mtime_ns < source_path.stat().st_mtime_ns:
                continue
            target_path = getattr(target_elf, "path", None)
            if (isinstance(target_path, Path) and target_path.is_file()
                    and target_path.stat().st_mtime_ns
                    < object_path.stat().st_mtime_ns):
                continue
            obj = loader(object_path)
            _index, header = obj.section(".bss")
            if isinstance(header, tuple) and header[5] == output_size:
                whole_owners.append((source, obj))
        if len(whole_owners) > 1:
            raise SurfaceComparisonError(
                "candidate data symbol %s has ambiguous whole-BSS ownership"
                % name)
        if not whole_owners:
            return None
        candidate_source = _source_from_object(Path(candidate_elf.path))
        candidate_path = root / "src" / ((candidate_source or "") + ".c")
        if (not candidate_source or not candidate_path.is_file()
                or not Path(candidate_elf.path).is_file()
                or Path(candidate_elf.path).stat().st_mtime_ns
                < candidate_path.stat().st_mtime_ns):
            return None
        text_size = _atlas_hex(
            module.get("sections", {}).get("text", {}), "size",
            "overlay text section")
        data_size = _atlas_hex(
            module.get("sections", {}).get("data_rodata", {}), "size",
            "overlay data/rodata section")
        return overlay, text_size + data_size + numeric_value
    linked_value, linked_size, linked_info, linked_section = linked[0]
    linked_type = linked_info & 0xF
    if linked_type not in (STT_NOTYPE, STT_OBJECT):
        return None
    if linked_value < SYNTHETIC_VMA:
        raise SurfaceComparisonError(
            "candidate data symbol %s is not at the synthetic VMA" % name)
    linked_offset = linked_value - SYNTHETIC_VMA

    root = REPO if root is None else Path(root)
    loader = Elf if elf_loader is None else elf_loader
    canonical = []
    sources = []
    for row in module.get("text_ownership", []):
        if isinstance(row, dict) and row.get("type") == "c":
            source = row.get("source")
            if isinstance(source, str) and source not in sources:
                sources.append(source)
    for row in module.get("data_rodata_ownership") or []:
        if isinstance(row, dict) and row.get("type") == "c":
            source = row.get("source")
            if isinstance(source, str) and source not in sources:
                sources.append(source)

    for source in sources:
        source_rel = Path(source)
        if source_rel.is_absolute() or ".." in source_rel.parts:
            raise SurfaceComparisonError(
                "overlay %d has an unsafe canonical source path" % overlay)
        source_path = root / "src" / (source + ".c")
        object_path = root / "build" / "src" / (source + ".c.o")
        if not source_path.is_file() or not object_path.is_file():
            continue
        if object_path.stat().st_mtime_ns < source_path.stat().st_mtime_ns:
            continue
        target_path = getattr(target_elf, "path", None)
        if (isinstance(target_path, Path) and target_path.is_file()
                and target_path.stat().st_mtime_ns < object_path.stat().st_mtime_ns):
            continue
        obj = loader(object_path)
        for symbol, value, size, info, shndx in obj.symbols():
            if symbol != name or shndx in (SHN_UNDEF, SHN_ABS):
                continue
            section = obj.names[shndx] if shndx < len(obj.names) else ""
            if section not in (".data", ".rodata", ".bss"):
                continue
            if (info & 0xF) not in (STT_NOTYPE, STT_OBJECT):
                continue
            canonical.append((source, obj, value, size, section))
    if len(canonical) > 1:
        raise SurfaceComparisonError(
            "candidate data symbol %s has ambiguous canonical definitions" % name)
    if not canonical:
        return None
    source, obj, object_value, object_size, object_section = canonical[0]
    if object_value != numeric_value or object_size != linked_size:
        raise SurfaceComparisonError(
            "candidate data symbol %s conflicts with its canonical object"
            % name)

    rom_size = _atlas_hex(module.get("rom", {}), "size", "overlay ROM row")
    text_size = _atlas_hex(
        module.get("sections", {}).get("text", {}), "size",
        "overlay text section")
    data_size = _atlas_hex(
        module.get("sections", {}).get("data_rodata", {}), "size",
        "overlay data/rodata section")
    bss_size = _overlay_module_extent(module, "bss_size", "BSS size")
    if object_section == ".bss":
        if linked_section != ".overlay_%03d_bss" % overlay:
            raise SurfaceComparisonError(
                "candidate data symbol %s has conflicting BSS ownership" % name)
        if not rom_size <= linked_offset < rom_size + bss_size:
            raise SurfaceComparisonError(
                "candidate data symbol %s escapes overlay BSS ownership" % name)
        # The host link lays BSS after the shipped relocation blobs.  The
        # runtime image discards those blobs before BSS, so translate the
        # linked offset back to the runtime text+data layout.
        runtime_offset = text_size + data_size + (linked_offset - rom_size)
    else:
        if linked_section != ".overlay_%03d" % overlay:
            raise SurfaceComparisonError(
                "candidate data symbol %s has conflicting data ownership" % name)
        if not text_size <= linked_offset < text_size + data_size:
            raise SurfaceComparisonError(
                "candidate data symbol %s escapes overlay data ownership" % name)
        owners = []
        for row in module.get("data_rodata_ownership") or []:
            if not isinstance(row, dict) or row.get("type") != "c":
                continue
            if row.get("source") != source:
                continue
            section = row.get("section")
            if section is not None and section != object_section:
                continue
            start = _atlas_hex(row, "offset", "data ownership row")
            end = _atlas_hex(row, "end_offset", "data ownership row")
            size = _atlas_hex(row, "size", "data ownership row")
            if end - start != size or not 0 <= start <= end <= data_size:
                raise SurfaceComparisonError(
                    "overlay %d data ownership boundary is inconsistent" % overlay)
            owners.append((start, end))
        if len(owners) > 1:
            raise SurfaceComparisonError(
                "candidate data symbol %s has ambiguous data ownership" % name)
        if owners:
            expected = text_size + owners[0][0] + object_value
            if linked_offset != expected:
                raise SurfaceComparisonError(
                    "candidate data symbol %s conflicts with canonical data ownership"
                    % name)
        runtime_offset = linked_offset

    # The candidate itself must still be a current object for its asserted TU.
    candidate_source = _source_from_object(Path(candidate_elf.path))
    candidate_path = root / "src" / ((candidate_source or "") + ".c")
    if (not candidate_source or not candidate_path.is_file()
            or not Path(candidate_elf.path).is_file()
            or Path(candidate_elf.path).stat().st_mtime_ns
            < candidate_path.stat().st_mtime_ns):
        return None
    return overlay, runtime_offset


def _runtime_correlated_overlay_hilo_identities(candidate_elf, module, start,
                                                size, target_records,
                                                redefine_aliases=None):
    """Bind undefined HI16/LO16 proxies to uniquely owned runtime tuples.

    This is intentionally narrower than matching a synthetic address.  The
    caller has already authenticated the candidate TU and target function to
    one atlas overlay owner, and ``target_records`` came from that overlay's
    shipped relocation table.  A proxy is admitted only when every complete
    pair that names it aligns by function-relative offset/type and derives the
    same stable ``(overlay selector, offset)`` base after removing the
    candidate object's REL addend.  Missing, duplicate, unpaired, or
    conflicting evidence leaves the name unresolved/ambiguous.

    Reserved runtime selectors (0xFFD..0xFFF) remain distinct identities.
    Nothing here compares or normalizes the overlays' shared synthetic VMA.
    """
    overlay = module.get("overlay")
    if not isinstance(overlay, int):
        raise SurfaceComparisonError("overlay module has no numeric identity")
    if module.get("identity") != "overlay:%d" % overlay:
        raise SurfaceComparisonError(
            "overlay %d atlas identity conflicts with its module row" % overlay)
    if _atlas_hex(module, "synthetic_vma", "overlay module") != SYNTHETIC_VMA:
        raise SurfaceComparisonError(
            "overlay %d atlas synthetic VMA conflicts with canonical policy"
            % overlay)

    by_shape = collections.defaultdict(list)
    for record in target_records or ():
        by_shape[(record.offset, record.rtype)].append(record)

    symbols = candidate_elf.symbols()
    reverse = redefine_aliases or {}
    sites = []
    referenced_sections = collections.defaultdict(set)
    referenced_types = collections.defaultdict(set)
    for _section, offset, rtype, symbol_index in candidate_elf.relocations():
        if not start <= offset < start + size:
            continue
        if symbol_index >= len(symbols):
            continue
        current, _value, _symbol_size, _info, shndx = symbols[symbol_index]
        if not current:
            continue
        name = reverse.get(current, current)
        referenced_sections[name].add(shndx)
        referenced_types[name].add(rtype)
        if rtype not in (R_MIPS_HI16, R_MIPS_LO16):
            continue
        sites.append({
            "offset": offset,
            "relative_offset": offset - start,
            "type": rtype,
            "symbol": name,
        })

    object_text = candidate_elf.section_bytes(".text")
    proposed = collections.defaultdict(set)
    ambiguous = set()
    disqualified = {
        name for name, types in referenced_types.items()
        if not types <= {R_MIPS_HI16, R_MIPS_LO16}
    }
    for high, low in _pairs(sites):
        anchor = high or low
        if anchor is None or anchor["type"] not in (R_MIPS_HI16, R_MIPS_LO16):
            continue
        name = anchor["symbol"]
        # This route binds source-level proxy externs only.  A definition in
        # any candidate section needs independent canonical owner evidence.
        if referenced_sections[name] != {SHN_UNDEF}:
            disqualified.add(name)
            continue
        if high is None or low is None:
            disqualified.add(name)
            continue
        high_targets = by_shape[(high["relative_offset"], R_MIPS_HI16)]
        low_targets = by_shape[(low["relative_offset"], R_MIPS_LO16)]
        if len(high_targets) != 1 or len(low_targets) != 1:
            if high_targets or low_targets:
                ambiguous.add(name)
            else:
                disqualified.add(name)
            continue
        high_target, low_target = high_targets[0], low_targets[0]
        identity = high_target.identity
        if (identity is None or identity != low_target.identity
                or high_target.link_addend is None
                or high_target.link_addend != low_target.link_addend):
            ambiguous.add(name)
            continue
        if (not isinstance(identity, tuple) or len(identity) != 2
                or not all(isinstance(value, int) for value in identity)
                or not 0 <= identity[0] <= 0xFFF or identity[1] < 0):
            ambiguous.add(name)
            continue
        if identity[1] - high_target.link_addend < 0:
            ambiguous.add(name)
            continue
        if high["offset"] + 4 > len(object_text) \
                or low["offset"] + 4 > len(object_text):
            ambiguous.add(name)
            continue
        addend = (
            stored_field(object_text, high["offset"], R_MIPS_HI16) << 16
        ) + sext16(stored_field(
            object_text, low["offset"], R_MIPS_LO16))
        base_offset = identity[1] - addend
        if base_offset < 0:
            ambiguous.add(name)
            continue
        proposed[name].add((identity[0], base_offset))

    for name, identities in proposed.items():
        if len(identities) > 1:
            ambiguous.add(name)
    resolved = {
        name: next(iter(identities))
        for name, identities in proposed.items()
        if (len(identities) == 1 and name not in ambiguous
                and name not in disqualified)
    }
    return resolved, ambiguous


def _runtime_correlated_overlay_r26_identities(candidate_elf, module, start,
                                               size, target_records,
                                               redefine_aliases=None):
    """Bind a repeated undefined same-overlay call proxy fail closed.

    One aligned call site would merely copy its target tuple and is therefore
    circular. This route requires at least two distinct candidate
    ``R_MIPS_26`` uses of the same undefined proxy. Every use must align with
    exactly one shipped runtime tuple and independently derive the same
    same-overlay base after removing the candidate REL addend. Missing,
    duplicate, mixed-type, defined-symbol, or conflicting evidence is never
    resolved.
    """
    overlay = module.get("overlay")
    if not isinstance(overlay, int):
        raise SurfaceComparisonError("overlay module has no numeric identity")
    if module.get("identity") != "overlay:%d" % overlay:
        raise SurfaceComparisonError(
            "overlay %d atlas identity conflicts with its module row" % overlay)
    if _atlas_hex(module, "synthetic_vma", "overlay module") != SYNTHETIC_VMA:
        raise SurfaceComparisonError(
            "overlay %d atlas synthetic VMA conflicts with canonical policy"
            % overlay)

    by_shape = collections.defaultdict(list)
    for record in target_records or ():
        by_shape[(record.offset, record.rtype)].append(record)

    symbols = candidate_elf.symbols()
    reverse = redefine_aliases or {}
    sites = collections.defaultdict(list)
    referenced_sections = collections.defaultdict(set)
    referenced_types = collections.defaultdict(set)
    for _section, offset, rtype, symbol_index in candidate_elf.relocations():
        if not start <= offset < start + size or symbol_index >= len(symbols):
            continue
        current, _value, _symbol_size, _info, shndx = symbols[symbol_index]
        if not current:
            continue
        name = reverse.get(current, current)
        referenced_sections[name].add(shndx)
        referenced_types[name].add(rtype)
        if rtype == R_MIPS_26:
            sites[name].append((offset, offset - start))

    object_text = candidate_elf.section_bytes(".text")
    resolved = {}
    ambiguous = set()
    for name, name_sites in sites.items():
        # The independent witness is reuse of one source-level undefined call
        # proxy. A definition or any mixed relocation class needs canonical
        # symbol/section evidence instead.
        if (referenced_sections[name] != {SHN_UNDEF}
                or referenced_types[name] != {R_MIPS_26}):
            continue
        unique_sites = sorted(set(name_sites))
        if len(unique_sites) < 2 or len(unique_sites) != len(name_sites):
            continue

        proposed = set()
        incomplete = False
        conflicting = False
        outside_owned_overlay = False
        for absolute_offset, relative_offset in unique_sites:
            targets = by_shape[(relative_offset, R_MIPS_26)]
            if len(targets) != 1:
                incomplete = True
                if targets:
                    conflicting = True
                continue
            target = targets[0]
            identity = target.identity
            if (not isinstance(identity, tuple) or len(identity) != 2
                    or not all(isinstance(value, int) for value in identity)
                    or not 0 <= identity[0] <= 0xFFF
                    or identity[1] < 0 or target.link_addend != 0
                    or absolute_offset + 4 > len(object_text)):
                conflicting = True
                continue
            if identity[0] != overlay:
                # A consistent resident, reserved-selector, or other-overlay
                # target is valid runtime evidence, but outside this narrowly
                # owned same-overlay proof. Keep the proxy unresolved rather
                # than poisoning unrelated admissible names in the function.
                outside_owned_overlay = True
                continue
            addend = stored_field(
                object_text, absolute_offset, R_MIPS_26) << 2
            base_offset = identity[1] - addend
            if base_offset < 0:
                conflicting = True
                continue
            proposed.add((overlay, base_offset))

        if outside_owned_overlay:
            continue
        if conflicting or len(proposed) > 1:
            ambiguous.add(name)
        elif not incomplete and len(proposed) == 1:
            resolved[name] = next(iter(proposed))
    return resolved, ambiguous


def _overlay_identity_extent(module):
    """Bytes an overlay identity offset can legitimately address, or None.

    The module's ROM row plus its BSS. Returns None when the atlas row cannot
    supply both, so the bound is skipped rather than guessed at.
    """
    try:
        rom_size = _atlas_hex(module.get("rom", {}), "size", "overlay ROM row")
        bss_size = _overlay_module_extent(module, "bss_size", "BSS size")
    except (SurfaceComparisonError, KeyError, TypeError, ValueError):
        return None
    if not isinstance(rom_size, int) or not isinstance(bss_size, int):
        return None
    return rom_size + bss_size


def _stable_overlay_data_identities(path, candidate_elf, module, target_elf,
                                    start, size, redefine_aliases=None,
                                    root=None, elf_loader=None, rom=None,
                                    runtime_module=None, target_records=None,
                                    own_range=None):
    """Resolve candidate-side same-overlay LOCAL/data identities fail closed.

    ``own_range`` excludes the function under proof from the sibling
    witnesses, as in ``_stable_overlay_call_identities``.
    """
    symbols = candidate_elf.symbols()
    sites = []
    names = set()
    reverse = redefine_aliases or {}
    for _section, offset, rtype, symbol_index in candidate_elf.relocations():
        if not start <= offset < start + size or rtype == R_MIPS_26:
            continue
        current = symbols[symbol_index][0] if symbol_index < len(symbols) else ""
        original = reverse.get(current, current)
        sites.append({"offset": offset, "type": rtype, "symbol": original})
        names.add(original)

    # A malformed or unpaired HI16/LO16 surface cannot authenticate that name.
    valid = set(names)
    for high, low in _pairs(sites):
        anchor = high or low
        if anchor is None:
            continue
        if anchor["type"] in (R_MIPS_HI16, R_MIPS_LO16) \
                and (high is None or low is None):
            valid.discard(anchor["symbol"])

    numeric = _numeric_assignments(path)
    proposed = collections.defaultdict(set)
    for name in sorted(valid):
        if name not in numeric:
            continue
        identity = _canonical_overlay_data_identity(
            module, candidate_elf, name, numeric[name], target_elf,
            root=root, elf_loader=elf_loader)
        if identity is not None:
            proposed[name].add(identity)

    if rom is not None and runtime_module is not None:
        witnessed = _matched_overlay_relocation_witnesses(
            module, target_elf, rom, runtime_module, valid,
            root=root, elf_loader=elf_loader, exclude_range=own_range)
        for name, identities in witnessed.items():
            proposed[name].update(identities)

    runtime_ambiguous = set()
    if target_records is not None:
        correlated, runtime_ambiguous = (
            _runtime_correlated_overlay_hilo_identities(
                candidate_elf, module, start, size, target_records,
                redefine_aliases))
        for name, identity in correlated.items():
            proposed[name].add(identity)

    # An offset outside the overlay's own extent is not an overlay identity,
    # whatever arithmetic produced it -- so drop it here, once, rather than at
    # each of the three sources above.
    #
    # splat names every unresolved absolute after its own value
    # (`D_80000078 = 0x80000078`), and those are literal operands: display-list
    # command words, hardware addresses. Paired against a matched sibling and
    # had the sibling's addend subtracted, such a name yields the absolute
    # itself as a proposed "offset" -- 0x80005A98 against overlay 58, whose
    # entire extent is 0x9568. It then collided with the same name's resident
    # identity, and `promotion-proof` refused four functions whose bytes were
    # exact; two had been promoted weeks earlier, so the refusal reproduced on
    # already-shipped work and was not a property of the new ones.
    #
    # Dropping the out-of-range proposal is fail-closed. The name keeps
    # whatever the resident pass gives it, and a genuine overlay datum lies
    # inside the extent by construction, so it is untouched.
    extent = _overlay_identity_extent(module)
    if extent is not None:
        for name in list(proposed):
            kept = {
                identity for identity in proposed[name]
                if not (isinstance(identity, tuple) and len(identity) == 2
                        and identity[0] == module.get("overlay"))
                or 0 <= identity[1] < extent
            }
            if kept:
                proposed[name] = kept
            else:
                del proposed[name]

    equality_aliases = ri.parse_linker_aliases(path.read_text()) \
        if path.is_file() else []
    redefine_pairs = [
        (source, destination)
        for destination, source in reverse.items()
    ]
    try:
        resolution = ri.resolve_identities(
            proposed, equality_aliases=equality_aliases,
            redefine_aliases=redefine_pairs)
    except ri.RelocationIdentityError as error:
        raise SurfaceComparisonError(str(error)) from error
    ambiguous = set(resolution.ambiguous) | runtime_ambiguous
    resolved = {
        name: identity for name, identity in resolution.resolved.items()
        if name not in ambiguous
    }
    return resolved, ambiguous


def _matched_overlay_relocation_witnesses(module, target_elf, rom,
                                          runtime_module, names, root=None,
                                          elf_loader=None, exclude_range=None):
    """Prove relocation-name identities through exact canonical siblings.

    A numeric overlay placeholder is not an identity.  A different, already
    matched function in the same overlay can provide one only when its fresh
    function-sized object owns the complete atlas row, the linked row is
    byte-identical to the retail ROM, and one static relocation tuple aligns
    with one shipped runtime tuple.  The returned value is the relocation
    symbol's base identity before its object-side REL addend is applied.

    This is deliberately a name witness, not an offset normalizer.  A caller's
    candidate records retain their own offsets and are compared to the target
    surface unchanged.

    ``exclude_range`` is the ``(start, end)`` module-offset extent of the
    function under proof, and any row overlapping it is skipped.  Re-proving
    an already promoted function otherwise finds that function's own
    canonical object among the matched rows, and its tuple at a site is the
    target's tuple at the same site: a one-site copy that "witnesses" any
    name the candidate uses.  overlay96DrawObject's generated
    ``func_overlay_096_F0000000_*`` call proxy was bound to a resident
    function that way, contradicting the name's own overlay-96 identity.
    """
    wanted = set(names)
    if not wanted:
        return {}
    overlay = module.get("overlay")
    if not isinstance(overlay, int) or runtime_module.get("overlay") != overlay:
        raise SurfaceComparisonError(
            "overlay runtime module conflicts with atlas identity")
    text_size = _atlas_hex(
        module.get("sections", {}).get("text", {}), "size",
        "overlay text section")
    rom_row = module.get("rom", {})
    rom_start = _atlas_hex(rom_row, "start", "overlay ROM row")
    if runtime_module.get("rom_start") != rom_start:
        raise SurfaceComparisonError(
            "overlay runtime module conflicts with atlas ROM ownership")

    linked_section_name = ".overlay_%03d" % overlay
    linked_text = target_elf.section_bytes(linked_section_name)
    if len(linked_text) < text_size:
        raise SurfaceComparisonError(
            "overlay %d linked section is shorter than atlas text" % overlay)

    root = REPO if root is None else Path(root)
    loader = Elf if elf_loader is None else elf_loader
    target_path = getattr(target_elf, "path", None)
    proposed = collections.defaultdict(set)
    context = {"kind": "overlay", "overlay": overlay,
               "module": runtime_module}

    for row in module.get("text_ownership", []):
        if not isinstance(row, dict):
            raise SurfaceComparisonError(
                "overlay %d has a malformed text ownership row" % overlay)
        row_start = _atlas_hex(row, "offset", "text ownership row")
        row_end = _atlas_hex(row, "end_offset", "text ownership row")
        row_size = _atlas_hex(row, "size", "text ownership row")
        if (row_end - row_start != row_size
                or not 0 <= row_start < row_end <= text_size):
            raise SurfaceComparisonError(
                "overlay %d text ownership boundary is inconsistent" % overlay)
        if (exclude_range is not None
                and row_start < exclude_range[1]
                and exclude_range[0] < row_end):
            continue
        if (row.get("type") != "c" or row.get("matched") is not True
                or row.get("nonmatching") is not False):
            continue

        source = row.get("source")
        if not isinstance(source, str) or not source:
            continue
        source_rel = Path(source)
        if source_rel.is_absolute() or ".." in source_rel.parts:
            raise SurfaceComparisonError(
                "overlay %d has an unsafe canonical source path" % overlay)
        source_path = root / "src" / (source + ".c")
        object_path = root / "build" / "src" / (source + ".c.o")
        if not source_path.is_file() or not object_path.is_file():
            continue
        if object_path.stat().st_mtime_ns < source_path.stat().st_mtime_ns:
            continue
        if (isinstance(target_path, Path) and target_path.is_file()
                and target_path.stat().st_mtime_ns < object_path.stat().st_mtime_ns):
            continue

        canonical = loader(object_path)
        text_index, _header = canonical.section(".text")
        object_text = canonical.section_bytes(".text")
        if text_index is None or len(object_text) != row_size:
            continue
        functions = [
            (name, value, size)
            for name, value, size, info, shndx in canonical.symbols()
            if shndx == text_index and (info & 0xF) == STT_FUNC
        ]
        if (len(functions) != 1 or functions[0][1] != 0
                or functions[0][2] != row_size):
            continue
        function_name, _value, function_size = functions[0]
        linked = []
        for name, value, size, info, shndx in target_elf.symbols():
            if (name != function_name or shndx == SHN_UNDEF
                    or (info & 0xF) != STT_FUNC):
                continue
            section = (target_elf.names[shndx]
                       if shndx < len(target_elf.names) else "")
            linked.append((value, size, section))
        if len(linked) > 1:
            raise SurfaceComparisonError(
                "%s has ambiguous linked function symbols" % function_name)
        if not linked:
            continue
        linked_value, linked_size, linked_section = linked[0]
        if (linked_value != SYNTHETIC_VMA + row_start
                or linked_section != linked_section_name
                or linked_size != function_size):
            raise SurfaceComparisonError(
                "%s linked symbol conflicts with canonical overlay ownership"
                % function_name)
        linked_range = linked_text[row_start:row_end]
        retail_range = rom[rom_start + row_start:rom_start + row_end]
        if linked_range != retail_range:
            continue

        runtime_records = _target_runtime_records(
            rom, context, row_start, row_size)
        by_shape = {}
        for record in runtime_records:
            key = (record.offset, record.rtype)
            if key in by_shape:
                raise SurfaceComparisonError(
                    "matched relocation witness has an ambiguous runtime tuple "
                    "at +%#x/type%d" % key)
            by_shape[key] = record

        symbols = canonical.symbols()
        sites = []
        for _section, offset, rtype, symbol_index in canonical.relocations():
            if not 0 <= offset < row_size:
                continue
            name = symbols[symbol_index][0] if symbol_index < len(symbols) else ""
            if name in wanted:
                sites.append({"offset": offset, "type": rtype, "symbol": name})
        for high, low in _pairs(sites):
            anchor = high or low
            if anchor is None:
                continue
            name = anchor["symbol"]
            if anchor["type"] in (R_MIPS_HI16, R_MIPS_LO16):
                if high is None or low is None:
                    continue
                high_target = by_shape.get((high["offset"], R_MIPS_HI16))
                low_target = by_shape.get((low["offset"], R_MIPS_LO16))
                if (high_target is None or low_target is None
                        or high_target.identity is None
                        or high_target.identity != low_target.identity):
                    continue
                addend = (
                    stored_field(object_text, high["offset"], R_MIPS_HI16) << 16
                ) + sext16(stored_field(
                    object_text, low["offset"], R_MIPS_LO16))
                identity = high_target.identity
            elif anchor["type"] == R_MIPS_32:
                target = by_shape.get((anchor["offset"], R_MIPS_32))
                if target is None or target.identity is None:
                    continue
                addend = stored_field(object_text, anchor["offset"], R_MIPS_32)
                identity = target.identity
            elif anchor["type"] == R_MIPS_26:
                target = by_shape.get((anchor["offset"], R_MIPS_26))
                if target is None or target.identity is None:
                    continue
                addend = stored_field(
                    object_text, anchor["offset"], R_MIPS_26) << 2
                identity = target.identity
            else:
                continue
            proposed[name].add((identity[0], identity[1] - addend))
    return proposed


def _runtime_base(record, source_overlay, rom_table):
    op = record["op"]
    if op in (0, 3):
        index = record["symbol_index"]
        if index >= len(rom_table):
            raise SurfaceComparisonError(
                "runtime relocation uses out-of-range ROM-table index %d" % index)
        target = rom_table[index]
        return (target["overlay"], target["offset"])
    if op == 1:
        return (source_overlay, record["symbol_index"])
    if op == 2:
        return None
    raise SurfaceComparisonError("unknown runtime relocation operation %d" % op)


def _attach_runtime_identities(records, rom, source_overlay, rom_table,
                               site_rom):
    """Return SurfaceRecords; fail if a target HI16 pair crosses ownership."""
    out = []
    index = 0
    while index < len(records):
        record = records[index]
        rtype = record["mode"]
        base = _runtime_base(record, source_overlay, rom_table)
        if rtype == R_MIPS_HI16:
            if index + 1 >= len(records):
                raise SurfaceComparisonError("runtime HI16 has no owned LO16")
            low = records[index + 1]
            if (low["mode"] != R_MIPS_LO16
                    or low["op"] != record["op"]
                    or low["symbol_index"] != record["symbol_index"]):
                raise SurfaceComparisonError("runtime HI16/LO16 identity is ambiguous")
            addend = ((stored_field(rom, site_rom(record), R_MIPS_HI16) << 16)
                      + sext16(stored_field(rom, site_rom(low), R_MIPS_LO16)))
            identity = _identity_add(base, addend) if base is not None else None
            out.append(SurfaceRecord(
                record["relative_offset"], rtype, identity, addend))
            out.append(SurfaceRecord(
                low["relative_offset"], low["mode"], identity, addend))
            index += 2
            continue
        stored = stored_field(rom, site_rom(record), rtype)
        if rtype == R_MIPS_26 and record["op"] == 2:
            identity = (source_overlay, stored << 2)
        elif base is None:
            identity = None
        elif rtype == R_MIPS_LO16:
            identity = _identity_add(base, sext16(stored))
        elif rtype == R_MIPS_32 and record["op"] == 1:
            identity = _identity_add(base, stored)
        else:
            identity = base
        link_addend = 0
        if rtype == R_MIPS_LO16:
            link_addend = sext16(stored)
        elif rtype == R_MIPS_32 and record["op"] == 1:
            link_addend = stored
        out.append(SurfaceRecord(
            record["relative_offset"], rtype, identity, link_addend))
        index += 1
    return out


def _target_runtime_records(rom, context, start, size):
    rom_table = ot.read_rom_table(rom)
    if context["kind"] == "overlay":
        overlay = context["overlay"]
        module = context["module"]
        all_records = ot.read_module_relocations(rom, module, rom_table)
        selected = [dict(r, relative_offset=r["target_offset"] - start)
                    for r in all_records
                    if start <= r["target_offset"] < start + size]
        selected.sort(key=lambda r: (r["table"], r["index"]))
        return _attach_runtime_identities(
            selected, rom, overlay, rom_table,
            lambda r: module["rom_start"] + r["target_offset"])

    count, all_records = ot.read_reloc_table(rom)
    if count != len(all_records):
        raise SurfaceComparisonError("resident relocation table count disagrees")
    selected = []
    for r in all_records:
        if not start <= r["call_site_offset"] < start + size:
            continue
        selected.append(dict(
            r,
            mode=r["flags"] >> 4,
            op=r["flags"] & 0xF,
            symbol_index=r["rom_table_index"],
            relative_offset=r["call_site_offset"] - start,
        ))
    return _attach_runtime_identities(
        selected, rom, 0, rom_table,
        lambda r: (ot.RESIDENT_VRAM_BASE + r["call_site_offset"]
                   - ot.VRAM_ROM_DELTA))


def _candidate_surface_records(elf, start, size, target, identities,
                               numeric_values, ambiguous_identities, overlay,
                               overlay_call_identities=None,
                               ambiguous_overlay_calls=None):
    syms = elf.symbols()
    target_by_shape = {(r.offset, r.rtype): r for r in target}
    raw = []
    for _section, offset, rtype, symbol_index in elf.relocations():
        if not start <= offset < start + size:
            continue
        name = syms[symbol_index][0] if symbol_index < len(syms) else ""
        raw.append({"offset": offset, "relative_offset": offset - start,
                    "type": rtype, "symbol": name})

    obj_text = elf.section_bytes(".text")
    output = []
    pending_highs = collections.defaultdict(list)
    high_to_low = {}
    low_to_highs = collections.defaultdict(list)
    for index, row in enumerate(raw):
        if row["type"] == R_MIPS_HI16:
            pending_highs[row["symbol"]].append(index)
        elif row["type"] == R_MIPS_LO16:
            highs = pending_highs.pop(row["symbol"], [])
            for high_index in highs:
                high_to_low[high_index] = index
                low_to_highs[index].append(high_index)
    paired_identities = collections.defaultdict(list)
    for i, record in enumerate(raw):
        rtype = record["type"]
        name = record["symbol"]
        identity_is_ambiguous = (
            name in ambiguous_identities
            or (
                rtype == R_MIPS_26
                and name in (ambiguous_overlay_calls or set())
            )
        )
        target_record = target_by_shape.get((record["relative_offset"], rtype))
        identity = None
        if rtype == R_MIPS_HI16:
            low_i = high_to_low.get(i)
            if low_i is not None:
                low = raw[low_i]
                high_value = stored_field(obj_text, record["offset"], rtype)
                low_value = stored_field(obj_text, low["offset"], R_MIPS_LO16)
                addend = (high_value << 16) + sext16(low_value)
                if not identity_is_ambiguous and name in identities:
                    identity = _identity_add(identities[name], addend)
                elif (not identity_is_ambiguous
                      and overlay is None and name in numeric_values
                      and target_record is not None):
                    target_identity = target_record.identity
                    if target_identity is not None:
                        # Numeric overlay placeholders carry the link-time
                        # addend. Apply their delta from the target's shipped
                        # addend in the same stable runtime address space.
                        candidate_addend = numeric_values[name] + addend
                        identity = _identity_add(
                            target_identity,
                            candidate_addend - target_record.link_addend)
                output.append(SurfaceRecord(
                    record["relative_offset"], rtype, identity))
                paired_identities[low_i].append(identity)
                continue
            # A HI16's addend is undefined without its REL-paired LO16.
            output.append(SurfaceRecord(record["relative_offset"], rtype, None))
            continue
        if rtype == R_MIPS_LO16 and i in low_to_highs:
            identities_for_low = set(paired_identities.get(i, []))
            if len(identities_for_low) > 1:
                raise SurfaceComparisonError(
                    "candidate relocation symbol %s has conflicting HI16/LO16 addends"
                    % name)
            identity = (next(iter(identities_for_low))
                        if identities_for_low else None)
            output.append(SurfaceRecord(record["relative_offset"], rtype, identity))
            continue
        addend = stored_field(obj_text, record["offset"], rtype)
        if rtype == R_MIPS_LO16:
            addend = sext16(addend)
        elif rtype == R_MIPS_26:
            addend <<= 2
        elif rtype == R_MIPS_PC16:
            # REL stores the addend in instruction words, relative to P.
            # Branch semantics use P+4, so the effective destination is
            # S + sign_extend(field)*4 + 4. Ordinary local-label REL has -1.
            instruction = struct.unpack_from(">I", obj_text, record["offset"])[0]
            if not _pc16_branch_instruction(instruction):
                raise SurfaceComparisonError("PC16 relocation is not on a supported branch instruction")
            addend = (sext16(addend) << 2) + 4
        base_identity = None if identity_is_ambiguous else identities.get(name)
        if (not identity_is_ambiguous
                and rtype == R_MIPS_26
                and name in (overlay_call_identities or {})):
            call_identity = overlay_call_identities[name]
            if base_identity is not None and base_identity != call_identity:
                # Two independent passes disagree about a call name. The
                # recurring cause is a zero-field call proxy: a resident call
                # stores a zero jump field, which is also the field of a call
                # to the overlay's own offset-0 function, so splat names both
                # `func_overlay_NNN_F0000000_*` and the C keeps that name. The
                # name's shape then says (NNN, 0) while an exact sibling's
                # runtime tuple says resident. Neither is chosen: the site is
                # left unresolved, as for any other conflicting witnesses, and
                # only the linked-ROM route can account for it.
                base_identity = None
            else:
                base_identity = call_identity
        if base_identity is not None:
            identity = _identity_add(base_identity, addend)
        elif (not identity_is_ambiguous
              and overlay is None and rtype != R_MIPS_26 and name in numeric_values
              and target_record is not None and target_record.identity is not None):
            candidate_addend = numeric_values[name] + addend
            identity = _identity_add(
                target_record.identity,
                candidate_addend - target_record.link_addend)
        output.append(SurfaceRecord(record["relative_offset"], rtype, identity))
    return output


LINK_MAP = REPO / "build" / "mickey.us.map"


def linked_input_sections(map_path=LINK_MAP):
    """{(object, section): (address, size)} for every input section the link placed.

    Read from the linker map written by the same link as the ELF: the only
    record of where one object's anonymous `.rodata`/`.data`/`.bss` landed.
    """
    placed = {}
    pattern = re.compile(
        r"^ (\.[\w.]+)\s+0x([0-9A-Fa-f]+)\s+0x([0-9A-Fa-f]+)\s+(\S+\.o)\s*$")
    for line in Path(map_path).read_text(errors="replace").splitlines():
        match = pattern.match(line)
        if match:
            key = (match.group(4), match.group(1))
            value = (int(match.group(2), 16), int(match.group(3), 16))
            if placed.setdefault(key, value) != value:
                raise SurfaceComparisonError(
                    "link map places %s(%s) twice" % key[::-1])
    return placed


def _resident_section_identities(target_object, target_object_path, map_path=LINK_MAP):
    """Resident identities of one object's own section symbols, from the link map.

    A relocation against a section symbol (`.rodata` for a jump table or a
    literal pool) addresses that object's input section, whose linked address
    only the map records. The map entry must exist once and have the object's
    own section size; a section the link did not place is left unresolved.
    """
    placed = linked_input_sections(map_path)
    try:
        key_object = Path(target_object_path).resolve().relative_to(REPO.resolve()).as_posix()
    except ValueError:
        return {}
    identities = {}
    for name, _value, _size, info, shndx in target_object.symbols():
        if info & 0xF != STT_SECTION or not 0 < shndx < len(target_object.names):
            continue
        section = target_object.names[shndx]
        if name != section or (key_object, section) not in placed:
            continue
        address, size = placed[(key_object, section)]
        header = target_object.sh[shndx]
        if size != header[5] or not 0x80000000 <= address < 0x90000000:
            raise SurfaceComparisonError(
                "link map disagrees with %s(%s)" % (key_object, section))
        identities[name] = (0, address - ot.RESIDENT_VRAM_BASE)
    return identities


def _resident_target_records(candidate_object, source, target_elf,
                             target_symbol, target_value, target_size,
                             target_section, values_path,
                             runtime_records=()):
    """Read one resident function's authoritative static relocation surface.

    The resident runtime relocation table is a sparse table used by the game
    at startup; it is not the linker's complete static relocation table.  The
    canonical ordinary object still contains the fallback assembly and its
    ``.rel.text`` entries.  Authenticate that object's symbol range against
    the linked ELF bytes before trusting its tuples, then resolve every tuple
    in the resident address space.
    """
    target_object_path = resolve_resident_target_object(candidate_object, source)
    target_object = Elf(target_object_path)
    object_start, object_size, object_section = _unique_symbol(
        target_object, target_symbol, require_text=True)
    if object_size != target_size:
        raise SurfaceComparisonError(
            "resident target size disagrees between canonical object and "
            "linked ELF: %#x vs %#x" % (object_size, target_size))

    object_text = target_object.section_bytes(object_section)
    if object_start + object_size > len(object_text):
        raise SurfaceComparisonError(
            "resident target range escapes canonical object .text")

    symbols = target_object.symbols()
    shape = []
    pc16_targets = {}
    seen_shape = collections.Counter()
    for _section, offset, rtype, symbol_index in target_object.relocations():
        if not object_start <= offset < object_start + object_size:
            continue
        if offset + 4 > object_start + object_size:
            raise SurfaceComparisonError(
                "resident target relocation crosses the function boundary")
        if symbol_index >= len(symbols) or not symbols[symbol_index][0]:
            raise SurfaceComparisonError(
                "resident target relocation has no symbol identity")
        relative = offset - object_start
        if rtype == R_MIPS_PC16:
            _name, label_value, _size, _info, label_section = symbols[symbol_index]
            text_index, _ = target_object.section(object_section)
            if label_section != text_index:
                raise SurfaceComparisonError("resident PC16 target is not defined in the owned text section")
            instruction = struct.unpack_from(">I", object_text, offset)[0]
            if not _pc16_branch_instruction(instruction):
                raise SurfaceComparisonError("resident PC16 relocation is not on a supported branch instruction")
            destination = label_value + (sext16(instruction & 0xFFFF) << 2) + 4
            if destination % 4 or not object_start <= destination < object_start + object_size:
                raise SurfaceComparisonError("resident PC16 destination escapes the owned function or is unaligned")
            pc16_targets[relative] = target_value + destination - object_start
        seen_shape[(relative, rtype)] += 1
        shape.append(SurfaceRecord(relative, rtype))
    duplicates = [key for key, count in seen_shape.items() if count > 1]
    if duplicates:
        raise SurfaceComparisonError(
            "resident target has ambiguous duplicate relocation tuples: %s"
            % ", ".join("+%#x/type%d" % key for key in sorted(duplicates)))

    _linked_index, linked_header = target_elf.section(target_section)
    if linked_header is None:
        raise SurfaceComparisonError(
            "linked resident target section %s is missing" % target_section)
    linked_start = linked_header[3]
    linked_text = target_elf.section_bytes(target_section)
    linked_offset = target_value - linked_start
    if (linked_offset < 0
            or linked_offset + target_size > len(linked_text)):
        raise SurfaceComparisonError(
            "resident linked range %#x..%#x escapes section %s %#x..%#x"
            % (target_value, target_value + target_size, target_section,
               linked_start, linked_start + len(linked_text)))
    object_bytes = object_text[object_start:object_start + object_size]
    linked_bytes = linked_text[linked_offset:linked_offset + target_size]
    relocated_bytes = {
        byte
        for record in shape
        for byte in range(record.offset, record.offset + 4)
    }
    if any(object_bytes[index] != linked_bytes[index]
           for index in range(target_size) if index not in relocated_bytes):
        raise SurfaceComparisonError(
            "canonical resident object bytes disagree with linked target "
            "outside relocation words")

    identities, ambiguous = _stable_symbol_identities(
        values_path, target_object, None, 0, target_elf)
    identities.update(_resident_section_identities(target_object, target_object_path))
    records = _candidate_surface_records(
        target_object, object_start, object_size, shape, identities,
        _numeric_assignments(values_path), ambiguous, None)
    if len(records) != len(shape):
        raise SurfaceComparisonError(
            "resident target relocation pairing changed the tuple count")
    unresolved = sum(record.identity is None for record in records)
    if unresolved:
        raise SurfaceComparisonError(
            "resident target has %d relocation tuple(s) with unresolved "
            "runtime identity" % unresolved)

    # Authenticate the bytes hidden by the object/link comparison mask too.
    # These are the ordinary static-link values from the canonical object;
    # sparse runtime-table identities are merged only after this proof.
    for record in records:
        if record.identity[0] not in (0, ri.ABSOLUTE_IDENTITY):
            raise SurfaceComparisonError(
                "resident static relocation resolves outside the resident "
                "address space")
        address = (record.identity[1]
                   if record.identity[0] == ri.ABSOLUTE_IDENTITY
                   else ot.RESIDENT_VRAM_BASE + record.identity[1])
        actual = stored_field(linked_bytes, record.offset, record.rtype)
        if record.rtype == R_MIPS_26:
            expected = (address >> 2) & 0x03FFFFFF
        elif record.rtype == R_MIPS_HI16:
            expected = ((address + 0x8000) >> 16) & 0xFFFF
        elif record.rtype == R_MIPS_LO16:
            expected = address & 0xFFFF
        elif record.rtype == R_MIPS_32:
            expected = address & 0xFFFFFFFF
        elif record.rtype == R_MIPS_PC16:
            if address != pc16_targets[record.offset]:
                raise SurfaceComparisonError("resident PC16 symbol identity disagrees with the owned function layout")
            displacement = address - (target_value + record.offset + 4)
            if displacement % 4 or not -(1 << 17) <= displacement < (1 << 17):
                raise SurfaceComparisonError("resident PC16 displacement is unaligned or out of range")
            expected = (displacement >> 2) & 0xFFFF
            original_word = struct.unpack_from(">I", object_bytes, record.offset)[0]
            linked_word = struct.unpack_from(">I", linked_bytes, record.offset)[0]
            if (original_word ^ linked_word) & 0xFFFF0000:
                raise SurfaceComparisonError("resident PC16 opcode/register bits disagree with linked target")
        else:
            raise SurfaceComparisonError(
                "resident target uses unsupported static relocation type %d"
                % record.rtype)
        if actual != expected:
            raise SurfaceComparisonError(
                "canonical resident relocation at +%#x does not reproduce "
                "the linked target word" % record.offset)

    runtime_by_shape = {}
    for record in runtime_records:
        key = (record.offset, record.rtype)
        if key in runtime_by_shape:
            raise SurfaceComparisonError(
                "resident runtime table has an ambiguous duplicate tuple at "
                "+%#x/type%d" % key)
        runtime_by_shape[key] = record
    static_shapes = {(record.offset, record.rtype) for record in records}
    missing_runtime = sorted(set(runtime_by_shape) - static_shapes)
    if missing_runtime:
        raise SurfaceComparisonError(
            "resident runtime tuple has no canonical static relocation: %s"
            % ", ".join("+%#x/type%d" % key for key in missing_runtime))
    return [runtime_by_shape.get((record.offset, record.rtype), record)
            for record in records]


def function_surface_comparison(symbol, candidate_object, target_elf_path,
                                rom_path=DEFAULT_ROM, atlas_path=None,
                                values_path=LINK_SYMS, candidate_symbol=None,
                                target_symbol=None, overlay_hint=None, source=None,
                                candidate_redefine_aliases=None,
                                include_candidate_identities=False):
    candidate_symbol = candidate_symbol or symbol
    target_symbol = target_symbol or symbol
    atlas_path = atlas_path or (REPO / "config" / "overlays.us.json")
    atlas = json.loads(atlas_path.read_text())
    candidate_elf = Elf(candidate_object)
    target_elf = Elf(target_elf_path)
    owner = resolve_overlay_ownership(
        candidate_object, atlas, overlay_hint=overlay_hint, source=source)
    candidate_start, candidate_size, _ = _unique_symbol(
        candidate_elf, candidate_symbol, require_text=True)
    target_value, target_size, target_section = _unique_symbol(
        target_elf, target_symbol)

    if owner is not None:
        module_row, row = owner
        overlay = module_row["overlay"]
        generated = GEN_NAME_RE.match(target_symbol)
        if generated and int(generated.group(1)) != overlay:
            raise SurfaceComparisonError(
                "target symbol names overlay %s but owner is overlay %d"
                % (generated.group(1), overlay))
        if target_value < SYNTHETIC_VMA:
            raise SurfaceComparisonError("overlay target is not at synthetic VMA")
        target_start = target_value - SYNTHETIC_VMA
        if generated and int(generated.group(2), 16) != target_start:
            raise SurfaceComparisonError(
                "target symbol offset %#x disagrees with linked value %#x"
                % (int(generated.group(2), 16), target_start))
        row_start = int(row["offset"], 16)
        row_end = int(row["end_offset"], 16)
        if not (row_start <= target_start
                and target_start + target_size <= row_end):
            raise SurfaceComparisonError(
                "target range %#x..%#x escapes atlas owner %#x..%#x"
                % (target_start, target_start + target_size, row_start, row_end))
        if candidate_start + candidate_size > int(row["size"], 16):
            raise SurfaceComparisonError("candidate function escapes TU ownership")
        context = {"kind": "overlay", "overlay": overlay,
                   "module": ot.build_modules(ot.read_headers(
                       rom_path.read_bytes()))[overlay - 1]}
        tu_base_offset = row_start
    else:
        overlay = None
        if not (ot.RESIDENT_VRAM_BASE <= target_value < 0x90000000):
            raise SurfaceComparisonError("resident target address is out of range")
        target_start = target_value - ot.RESIDENT_VRAM_BASE
        context = {"kind": "resident"}
        tu_base_offset = 0

    rom = rom_path.read_bytes()
    if context["kind"] == "overlay":
        # Reuse the module objects decoded from the exact same ROM bytes.
        context["module"] = ot.build_modules(ot.read_headers(rom))[overlay - 1]
    if context["kind"] == "overlay":
        target_records = _target_runtime_records(
            rom, context, target_start, target_size)
    else:
        resident_runtime_records = _target_runtime_records(
            rom, context, target_start, target_size)
        target_records = _resident_target_records(
            candidate_object, source, target_elf, target_symbol,
            target_value, target_size, target_section, values_path,
            resident_runtime_records)
    identities, ambiguous_identities = _stable_symbol_identities(
        values_path, candidate_elf, overlay, tu_base_offset, target_elf,
        candidate_redefine_aliases)
    overlay_call_identities, ambiguous_overlay_calls = (
        _stable_overlay_call_identities(
            values_path, candidate_elf, overlay, target_elf, atlas,
            candidate_start, candidate_size, candidate_redefine_aliases,
            module=module_row if overlay is not None else None, rom=rom,
            runtime_module=context.get("module"),
            target_records=target_records,
            own_range=((target_start, target_start + target_size)
                       if overlay is not None else None))
    )
    if overlay is not None:
        overlay_data_identities, ambiguous_overlay_data = (
            _stable_overlay_data_identities(
                values_path, candidate_elf, module_row, target_elf,
                candidate_start, candidate_size, candidate_redefine_aliases,
                rom=rom, runtime_module=context["module"],
                target_records=target_records,
                own_range=(target_start, target_start + target_size))
        )
        for name, identity in overlay_data_identities.items():
            existing = identities.get(name)
            if (existing is not None and existing != identity
                    and _resident_versus_reserved(existing, identity)):
                # The symbol pass reads a resident *address* and calls it a
                # resident identity; the shipped table may instead reach the
                # same byte through a reserved runtime selector (0xFFD..0xFFF).
                # Reserved selectors stay distinct identities, so neither is
                # chosen: the name is ambiguous and only the linked-ROM route
                # can account for its sites.
                identities.pop(name, None)
                ambiguous_identities.add(name)
                continue
            if existing is not None and existing != identity:
                raise SurfaceComparisonError(
                    "candidate relocation symbol %s has conflicting runtime "
                    "identity: symbol pass %r, data pass %r"
                    % (name, existing, identity))
            identities[name] = identity
        ambiguous_identities.update(ambiguous_overlay_data)
    numeric_values = _numeric_assignments(values_path)
    candidate_records = _candidate_surface_records(
        candidate_elf, candidate_start, candidate_size, target_records,
        identities, numeric_values, ambiguous_identities, overlay,
        overlay_call_identities, ambiguous_overlay_calls)
    result = compare_record_sets(target_records, candidate_records)
    if include_candidate_identities:
        # Per-site static identities, for a caller that must prove individual
        # sites rather than the whole surface (the declared-metadata proof).
        result["candidate_identities"] = [
            {"offset": row.offset, "rtype": row.rtype,
             "identity": list(row.identity) if row.identity is not None else None}
            for row in candidate_records]
    result["candidate_redefine_alias_count"] = len(
        candidate_redefine_aliases or {}
    )
    result.update({
        "symbol": symbol,
        "candidate_symbol": candidate_symbol,
        "target_symbol": target_symbol,
        "context": "overlay:%d" % overlay if overlay is not None else "resident",
        "target_surface_source": (
            "overlay-runtime-table" if overlay is not None
            else "resident-canonical-static-object"),
        "target_offset": "0x%X" % target_start,
        "target_size": "0x%X" % target_size,
    })
    return result


def cmd_compare(argv):
    parser = argparse.ArgumentParser(
        prog="reloc_surface.py compare",
        description="Compare one candidate function's static relocations "
                    "with its shipped runtime relocation tuples.")
    parser.add_argument("symbol")
    parser.add_argument("--candidate-object", type=Path, required=True)
    parser.add_argument("--candidate-symbol")
    parser.add_argument("--target-symbol")
    parser.add_argument("--target-elf", type=Path,
                        default=REPO / "build" / "mickey.us.elf")
    parser.add_argument("--overlay", type=int,
                        help="assert the atlas-derived overlay number")
    parser.add_argument("--source",
                        help="canonical atlas source for a copied scratch object")
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--atlas", type=Path,
                        default=REPO / "config" / "overlays.us.json")
    parser.add_argument("--values", type=Path, default=LINK_SYMS)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--check", action="store_true",
                        help="exit 1 unless both surfaces are exactly equal")
    args = parser.parse_args(argv)
    try:
        result = function_surface_comparison(
            args.symbol, args.candidate_object, args.target_elf,
            rom_path=args.rom, atlas_path=args.atlas, values_path=args.values,
            candidate_symbol=args.candidate_symbol,
            target_symbol=args.target_symbol, overlay_hint=args.overlay,
            source=args.source)
    except (OSError, ValueError, SurfaceComparisonError) as error:
        parser.error(str(error))
    if args.json:
        print(json.dumps(result, sort_keys=True, separators=(",", ":")))
    else:
        print(
            "{symbol}: {context} target={target_runtime_record_count} "
            "candidate={candidate_record_count} "
            "offset/type={offset_type_alignment_count}/"
            "{target_runtime_record_count} "
            "identity={stable_identity_alignment_count}/"
            "{target_runtime_record_count} "
            "resolved={candidate_identity_resolved_count}".format(**result))
    if args.check and not (
            result["offset_type_exact"] and result["stable_identity_exact"]):
        return 1
    return 0


def main(argv):
    if argv and argv[0] == "generate":
        return cmd_generate(argv[1:])
    if argv and argv[0] == "compare":
        return cmd_compare(argv[1:])
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("object", nargs="*", type=Path)
    p.add_argument("--overlay", type=int)
    p.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    p.add_argument("--against", type=Path, default=LINK_SYMS,
                   help="file the audit replays against (default: the "
                        "generated overlay_undefined_syms block; point it at a "
                        "hand-maintained copy to replay that instead)")
    p.add_argument("--audit", action="store_true",
                   help="replay every built overlay object and score the "
                        "synthesized values against overlay_undefined_syms.us.txt")
    p.add_argument("--sites", action="store_true", help="print every site")
    p.add_argument("--apply", action="store_true",
                   help="rename every synthesized undefined symbol in the "
                        "object to a unique alias and emit its value")
    p.add_argument("--ld-out", type=Path,
                   help="append the synthesized assignments to this linker "
                        "script fragment")
    p.add_argument("--objcopy", type=Path,
                   default=REPO / "tools" / "binutils" / "mips64-elf-objcopy")
    args = p.parse_args(argv)

    rom = args.rom.read_bytes()
    import json
    atlas = json.loads((REPO / "config" / "overlays.us.json").read_text())
    rows = {m["overlay"]: m["text_ownership"] for m in atlas["modules"]}
    rom_table = ot.read_rom_table(rom)
    mods = ot.build_modules(ot.read_headers(rom))

    linked = linked_overlay_objects()
    local_defs = {}
    objects = list(args.object)
    if args.audit:
        objects = [obj for _ov, obj in linked]
    if not objects:
        p.error("no objects")

    stale = []
    known = tracked_values(args.against)
    agree = disagree = unknown = 0
    bad = []
    all_conflicts = []
    for obj in objects:
        ov = args.overlay or overlay_of(obj)
        if ov is None:
            print(f"skip {obj}: no overlay", file=sys.stderr)
            continue
        if ov not in local_defs:
            local_defs[ov] = module_text_defs(linked, ov, rows.get(ov, []))
        recs = ot.read_module_relocations(rom, mods[ov - 1], rom_table)
        sites, values, conflicts, aligned = synthesize(
            obj, ov, rom, rows.get(ov, []), recs, local_defs[ov])
        all_conflicts += [(obj.name, n, why) for n, why in conflicts]
        if aligned["attempted"]:
            print("/* ALIGNED %s: %d/%d site(s), %d shifted%s */"
                  % (obj.name, aligned["aligned"], aligned["sites"],
                     aligned["shifted"],
                     "" if not aligned["refused"] else
                     "; refused: " + aligned["refused"]), file=sys.stderr)
        if args.audit:
            for name, v in values.items():
                if name in known:
                    if known[name] == v:
                        agree += 1
                    else:
                        disagree += 1
                        bad.append((obj.name, name, hex(known[name]), hex(v)))
                else:
                    unknown += 1
            continue
        if args.sites:
            for s in sites:
                mo = "?" if s["module_off"] is None else f'{s["module_off"]:#x}'
                st = "?" if s["stored"] is None else f'{s["stored"]:#x}'
                print(f'  {s["obj_off"]:#06x} -> {mo:>8} '
                      f'{TYPE_NAMES.get(s["type"], s["type"]):12} '
                      f'stored={st:>10} table={s["op"] or "-":6} {s["symbol"]}')
        alias = {n: f"rs_{ov}_{obj.name.replace('.', '_')}_{n}" for n in values}
        if args.apply and values:
            import subprocess
            cmd = [str(args.objcopy)]
            for n in sorted(values):
                cmd += ["--redefine-sym", f"{n}={alias[n]}"]
            cmd.append(str(obj))
            subprocess.run(cmd, check=True)
        lines = [f"{alias[n] if args.apply else n} = {values[n]:#010x};"
                 for n in sorted(values)]
        if args.ld_out:
            with open(args.ld_out, "a") as fh:
                fh.write(f"/* {obj} */\n" + "\n".join(lines) + "\n")
        print(f"/* {obj.name}: {len(values)} synthesized symbol values */")
        for line in lines:
            print(line)
        for name, why in conflicts:
            print(f"/* UNRESOLVED {name}: {why} */")

    if args.audit:
        total = agree + disagree
        print(f"tracked-value replay: {agree}/{total} agree "
              f"({100.0 * agree / total if total else 0:.1f}%), "
              f"{disagree} disagree, {unknown} not tracked, "
              f"{len(all_conflicts)} unresolved, "
              f"{len(stale)} stale object(s) skipped")
        for row in bad[:40]:
            print("  MISMATCH", *row)
        for row in all_conflicts[:40]:
            print("  UNRESOLVED", *row)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
