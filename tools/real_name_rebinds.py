#!/usr/bin/env python3
"""Rename planning for real overlay names (docs/reloc-surface.md section 5.7).

`reloc_surface.plan_resident_rebinds()` plans, per object, the
`--redefine-sym <name>=<name>_oNNNReloc` rules that
`mk/overlay_resident_rebinds.generated.mk` declares.  Resident calls were the
first case (section 5.6).  This module holds the second: overlay C that names
a reference by its *real* overlay target -- another module's function or
data, or this module's own export -- instead of a placeholder.

Such a name cannot link bare.  Every overlay is linked at `0xF0000000`, so
`jal overlay45CreateDescriptor` from overlay 11 would store overlay 45's
offset, while the shipped `SYMBOL` record stores only the addend the runtime
adds the export's address to.  So the site goes through a per-module
placeholder valued at the stored addend, and the real name stays in the C,
where the runlink packer reads it (docs/overlays.md section 5.3.1).

Kept apart from reloc_surface.py only because that file sits at the
repository's tracked-file size limit; the ELF reader and the atlas lookup are
passed in, so the planner has no other dependency.
"""

from __future__ import annotations

import collections
import re

SHN_UNDEF = 0
R_MIPS_26 = 4
STT_SECTION = 3
STT_FUNC = 2
STB_GLOBAL = 1
ALIAS_FMT = "%s_o%03dReloc"
# A short `D_<hex>` is a per-module placeholder: the same spelling in two
# overlays is two different things (CLAUDE.md, "A generically-named D_ symbol").
GENERIC_DATA_NAME_RE = re.compile(r"^D_[0-9A-Fa-f]{1,5}$")


GENERATED_FUNC_RE = re.compile(r"^(func_overlay_\d{3}_F[0-9A-F]{7})_[0-9A-F]+$")


def alias_name(name, overlay):
    """The per-module placeholder for ``name``.

    A generated function name drops its ROM-address suffix (the overlay and
    offset already identify it): with it the alias is 43 characters of the
    base64url alphabet, which the clean-room detectors read as an encoded run.
    """
    m = GENERATED_FUNC_RE.match(name)
    return ALIAS_FMT % (m.group(1) if m else name, overlay)


def text_aliases(elf, overlay, base, rom_start):
    """(generated identity, friendly name) for each global text function.

    Statics are not linkable, and a function already using its generated
    identity needs no equality alias. The canonical module and ROM offsets
    distinguish functions even though overlay VMAs share a synthetic base.
    """
    text_index, _section = elf.section(".text")
    if text_index is None or base is None:
        return []
    aliases = []
    for name, value, _size, info, section in elf.symbols():
        if section != text_index or not name or info >> 4 != STB_GLOBAL or info & 0xF != STT_FUNC:
            continue
        offset = base + value
        identity = "func_overlay_%03d_F%07X_%X" % (overlay, offset, rom_start + offset)
        if name != identity:
            aliases.append((identity, name))
    return aliases


def defined_names(objects, elf_factory, aliases=None):
    """{name: {(overlay, kind)}} for the globals overlay objects define.

    ``kind`` is ``"text"`` for a definition in the object's ``.text`` and
    ``"data"`` for one in any other real section.  Absolute and COMMON symbols
    are not definitions an identity can be read from (an ``--add-symbol``
    literal marker is absolute; a COMMON block is placed by the link), so they
    are left out.  Read from the normal build objects, the same ones the
    surface is valued from. ``aliases``, when supplied, returns the generated
    linker identities of each object's global text functions. They are real
    definitions too: callers may keep an address-qualified name after the
    callee's C adopts a friendly name. Count these identities in the rename
    plan before its value pass can shadow a linker equality alias.
    """
    out = collections.defaultdict(set)
    for ov, obj in objects:
        try:
            elf = elf_factory(obj)
        except (SystemExit, OSError):
            continue
        text_idx, _ = elf.section(".text")
        for name, _v, _s, info, shndx in elf.symbols():
            if not name or (info >> 4) == 0 or (info & 0xF) == STT_SECTION:
                continue
            if shndx == SHN_UNDEF or shndx >= 0xFF00:
                continue
            out[name].add((ov, "text" if shndx == text_idx else "data"))
        if aliases is not None:
            for identity, _friendly in aliases(ov, obj):
                out[identity].add((ov, "text"))
    return dict(out)


def real_aliases(elf, overlay, base, records, overlay_defs, undo=None):
    """(renames, refusals) for one object's references to real overlay names.

    ``elf`` is the object (reloc_surface.Elf), ``base`` its module text offset
    from the atlas (None without a `text_ownership` row), ``records`` the
    module's shipped relocation records and ``overlay_defs`` the
    `defined_names()` census.

    * A name defined by another module is always renamed (a value line under
      the name itself would move the definition for its own module).
    * A name defined by this module is renamed only when its corroborated
      sites need the stored addend: a `SYMBOL` record, or the address (HI/LO)
      of a function, since the surface never values a name the module's own
      `.text` defines.  A `JUMP` call already stores the offset the synthetic
      VMA produces, and a `LOCAL` reference to data is valued under its own
      name (section 5.2), so both keep it.

    Refused, with the reason: a name more than one module defines (no single
    identity); a name whose corroborated sites in this object mix sites that
    need an addend with sites that link bare; a cross-module
    reference in an object with no `text_ownership` row.  A name this object
    itself defines never reaches here -- its references are not undefined --
    and an objcopy rename would rename the definition with them.
    """
    undo = undo or {}
    syms = elf.symbols()
    defined_here = {name for name, _v, _s, _i, shndx in syms
                    if name and shndx != SHN_UNDEF}
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
        name = undo.get(name, name)
        if name in overlay_defs and name not in defined_here:
            by_symbol[name].append((off, rtype))

    for name in [n for n in by_symbol if GENERIC_DATA_NAME_RE.match(n)]:
        if {ov for ov, _k in overlay_defs[name]} != {overlay}:
            del by_symbol[name]

    renames, refusals = {}, []
    for name in sorted(by_symbol):
        owners = {ov for ov, _kind in overlay_defs[name]}
        if len(owners) > 1:
            refusals.append((name, "defined by overlays %s; no single identity"
                             % ", ".join(str(o) for o in sorted(owners))))
            continue
        if owners != {overlay}:
            if base is None:
                refusals.append((name, "cross-module reference in an object "
                                       "with no text_ownership row: no module "
                                       "offset"))
                continue
            renames[name] = alias_name(name, overlay)
            continue
        if base is None:
            continue
        # What each site needs: "bare" (the name links as it stands) or
        # "addend" (a value line, so a placeholder).  A JUMP call and a LOCAL
        # data reference are bare; a SYMBOL site needs its addend, and so does
        # a text name's address (HI/LO), which the surface never values under
        # a name defined in the module's own .text.
        text = any(kind == "text" for _ov, kind in overlay_defs[name])
        needs = set()
        for off, rtype in by_symbol[name]:
            for r in table.get(base + off, []):
                if r["mode"] != rtype:
                    continue
                if r["op_name"] == "SYMBOL" or (text and rtype != R_MIPS_26):
                    needs.add(("addend", r["op_name"]))
                else:
                    needs.add(("bare", r["op_name"]))
        if not any(n == "addend" for n, _op in needs):
            continue
        if any(n == "bare" for n, _op in needs):
            refusals.append((name, "this module's own name reached by %s "
                                   "records needing an addend and %s records "
                                   "that link bare; one placeholder cannot "
                                   "carry both"
                             % ("/".join(sorted({op for n, op in needs if n == "addend"})),
                                "/".join(sorted({op for n, op in needs if n == "bare"})))))
            continue
        renames[name] = alias_name(name, overlay)
    return renames, refusals
