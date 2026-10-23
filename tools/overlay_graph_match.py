#!/usr/bin/env python3
"""Score structural leads between Mickey's overlays and JFG's modules.

Module sizes and dependency edges come from Mickey's atlas and JFG's
published symbols/map. Mickey function boundaries come from overlay-qualified
compiled ELF symbols, including statics. Named resident-callee fingerprints
come from each game's relocation tables; no instruction is decoded.

The scores rank investigation leads. They do not prove semantic identity or
adopt a name. The checked-in report includes anchor and cross-version
calibration so readers can assess the method's limits.

    gmake overlay-graph          # regenerate from complete local builds
    gmake check-overlay-graph    # fail on drift or missing inputs
    tools/overlay_graph_match.py --self-test
    tools/overlay_graph_match.py --overlay 61
"""

import argparse
import bisect
import collections
import json
import math
import os
import re
import struct
import sys
from collections import defaultdict

import overlay_tables

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ATLAS = os.path.join(REPO, "config", "overlays.us.json")
REPORT = os.path.join(REPO, "config", "overlay-graph.us.json")
MICKEY_ROM = os.path.join(REPO, "baseroms", "mickey.us.z64")
MICKEY_SYMS = os.path.join(REPO, "symbol_addrs.us.txt")

JFG_ROOT = os.path.expanduser(
    os.environ.get("JFG_ROOT", "~/Desktop/dev/decomp-refs/jfg")
)
JFG_SYMS_US = os.path.join(JFG_ROOT, "jfg_us_syms_full.txt")
JFG_SYMS_KIOSK = os.path.join(JFG_ROOT, "jfg_kiosk_syms_full.txt")
JFG_REFS = os.path.join(JFG_ROOT, "overly_refs.txt")
JFG_MAP = os.path.join(JFG_ROOT, "build", "jfg.us.map")
JFG_ROM = os.path.join(JFG_ROOT, "build", "jfg.us.z64")

# The three byte-identical cross-game anchors (docs/acceleration-survey.md
# section 6). Only two land inside a JFG *overlay module* -- alSeqFileNew is
# resident code on BOTH sides (JFG's Main Code section; Mickey's own overlay
# 5 copy is a per-overlay DKR donor object, not resident, per
# docs/modules.md sec. "5.3"/649-669 -- so even though Mickey placed the
# function in overlay 5, JFG never gave it a Module N section to anchor
# against). Reported but not scored as a module pair either way.
KNOWN_ANCHORS = {
    # Mickey overlay -> JFG module, by the shared byte-identical symbol.
    49: {"jfg_module": 2, "symbol": "refractOutput"},
    107: {"jfg_module": 156, "symbol": "osRamTest4_6105"},
}
UNANCHORABLE = {
    "alSeqFileNew": (
        "resident in JFG's Main Code section (no Module N of its own); "
        "lives in Mickey overlay 5, but that does not create a JFG module "
        "to pair it against"
    )
}

# ---------------------------------------------------------------------------
# Resident-callee fingerprint
#
# The task brief for this pass assumed a resident-bound `jal` is left
# untouched by the runtime linker (its target never moves, so nothing
# needs patching) and that reading it therefore requires decoding ROM
# instruction words. An empirical check overturned that for Mickey: every
# `mode==R_MIPS_26, op==SYMBOL` relocation record in overlay 1 that
# resolves through `overlayRomTable` (`tools/overlay_atlas.py`'s own
# `read_module_relocations`) landed on a real target, and 1161 of
# `overlayRomTable`'s 2004 entries carry overlay number 0 -- the resident
# segment. Mickey's overlay format reserves overlay 0 for "resident" in
# exactly the same table Rare's JFG runtime linker uses (JFG's own
# `tools/overlay_reloc.py` documents the identical convention for its own
# ROM). So EVERY external call -- resident or overlay-to-overlay -- is a
# placeholder `jal` patched at load time on both sides, and the resident-
# callee fingerprint is recoverable from relocation-table METADATA alone,
# the same records `tools/overlay_atlas.py` already reads for the atlas.
# No ROM instruction word is decoded anywhere below; only relocation
# records (already-read-elsewhere content) and the resulting symbol
# names/addresses/counts leave this section for config/overlay-graph.us.json.
# ---------------------------------------------------------------------------

# The resident segment's base VRAM address, as `overlayRomTable` encodes it
# (`tools/overlay_tables.py`'s `RESIDENT_VRAM_BASE`; corroborated here by
# `entrypoint = 0x80000400 (size 0x50)` in symbol_addrs.us.txt -- the boot
# stub ends exactly at 0x80000450, which is also JFG's own `base_addr` for
# the identical mechanism, per its tools/overlay_reloc.py).
MICKEY_RESIDENT_BASE = overlay_tables.RESIDENT_VRAM_BASE  # 0x80000450

RELOC_MODE_R_MIPS_26 = 4  # runlink.h RELOC_TYPE_26 -- the patch rewrites a j/jal target
RELOC_OP_SYMBOL = 0       # runlink.h RELOC_OP_SYMBOL -- resolved through overlayRomTable

FUNC_SYM_RE = re.compile(r"^(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*type:func")
STUB_NAME_RE = re.compile(r"^func_[0-9A-Fa-f]{8}$")

# JFG's own overlay-table layout (permitted public source: its own
# tools/extract_overlays.py and tools/overlay_reloc.py document these exact
# constants for the US ROM/build, which is byte-identical to JFG's retail
# ROM since it's a matching decomp). Reused here only as plain arithmetic
# offsets -- nothing about JFG's instruction bytes is read, since JFG's own
# runtime linker already relocates resident-bound `jal`s through this same
# reloc-table mechanism, so the resident-callee fingerprint is recoverable
# from JFG's *metadata* alone.
JFG_OVERLAY_TABLE_ROM = 0x1ED2780
JFG_OVERLAY_DATA_ROM = 0x1ED3B20
JFG_OVERLAY_ROM_TABLE_ROM = 0x1ED0270
JFG_NUM_OVERLAYS = 157
JFG_NUM_ORT_ENTRIES = 2371
JFG_MAIN_BASE = 0x80000450
JFG_RELOC_PATCH_JAL = 4     # overlay_reloc.py RELOC_PATCH_JAL / R_MIPS_26
JFG_RELOC_TYPE_LOCAL = 1    # never targets main code, by definition

AUTOEXIT_RE = re.compile(r"^_AutoExit\d+$")
SECTION_RE = re.compile(
    r"^=== Section 0x([0-9A-Fa-f]+): (Main Code|Data \(FFD\)|BSS|Module (\d+)) "
    r"\((\d+) symbols\) ==="
)
SYM_RE = re.compile(r"^\s*\+0x([0-9A-Fa-f]+)\s+(\S+)")
MAIN_SYM_RE = re.compile(r"^\s*([0-9A-Fa-f]{8})\s+(\S+)")

MAP_OBJ_RE = re.compile(
    r"^\s*\.(text|data|rodata|bss)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+"
    r"build/(?:src/overlays/o(\d+)/overlay_\d+\.c\.o"
    r"|asm/data/overlays/o(\d+)/overlay_\d+\.data\.s\.o)"
)

REF_OUT_RE = re.compile(r"^Overlay\s+(\d+)\s+->\s+\[([^\]]*)\]\s+\((\d+) references?\)")
REF_IN_RE = re.compile(r"^Overlay\s+(\d+)\s+<-\s+\[([^\]]*)\]\s+\((\d+) callers?\)")


def load_json(path):
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def hx(s):
    """Parse a '0x...' string from the atlas into an int."""
    return int(s, 16)


def load_rom(path):
    if not os.path.exists(path):
        return None
    with open(path, "rb") as fh:
        return fh.read()


def load_mickey_resident_names():
    """{VRAM address -> name} for every resident `type:func` entry in
    symbol_addrs.us.txt, excluding func_XXXXXXXX stand-ins (those carry no
    information beyond the address itself, so they are opaque tokens, not
    names, for cross-game bridging purposes)."""
    names = {}
    if not os.path.exists(MICKEY_SYMS):
        return names
    with open(MICKEY_SYMS, encoding="utf-8") as fh:
        for line in fh:
            m = FUNC_SYM_RE.match(line.strip())
            if not m:
                continue
            name, addr = m.group(1), int(m.group(2), 16)
            if STUB_NAME_RE.match(name):
                continue
            names[addr] = name
    return names


def mickey_resident_calls(rom):
    """{overlay: {"calls": Counter(target_vram -> count),
                  "call_sites": [(text_offset, target_vram), ...]}}.

    CORRECTION over this lane's task brief: Mickey's runtime linker does
    NOT leave resident-bound `jal`s untouched. An empirical check (every
    mode-R_MIPS_26/op-SYMBOL relocation record in overlay 1 that resolves
    through `overlayRomTable`) found 148 call sites whose resolved target
    is overlay 0 -- and `overlayRomTable` itself carries 1161 overlay-0
    entries out of 2004 total (`tools/overlay_tables.py`'s own decode).
    Mickey's per-module relocation tables use exactly JFG's mechanism
    (`overlayRomTable` includes the resident segment as "overlay 0", same
    as JFG's own tools/overlay_reloc.py documents for its own ROM): EVERY
    external call, resident or overlay-to-overlay, is a placeholder `jal`
    patched at load time, not a directly-encoded address. So this is
    metadata, exactly like the JFG side below -- no `jal` word is ever
    decoded from Mickey's ROM text, only relocation records that were
    already being read for the atlas (`tools/overlay_atlas.py` uses the
    same `read_module_relocations`).
    """
    headers = overlay_tables.read_headers(rom)
    mods = overlay_tables.build_modules(headers)
    rom_table = overlay_tables.read_rom_table(rom)

    out = {}
    for module in mods:
        ovl = module["overlay"]
        records = overlay_tables.read_module_relocations(rom, module, rom_table)
        calls = collections.Counter()
        call_sites = []
        for r in records:
            if r["mode"] != RELOC_MODE_R_MIPS_26 or r["op"] != RELOC_OP_SYMBOL:
                continue
            if r.get("target_overlay") != 0:
                continue
            target = MICKEY_RESIDENT_BASE + r["target_symbol_offset"]
            calls[target] += 1
            call_sites.append((r["target_offset"], target))
        out[ovl] = {"calls": calls, "call_sites": call_sites}
    return out


def tokenize_calls(calls, name_map):
    """Split a Counter(address -> count) into named (bridgeable across
    games) and opaque (address-only, same-game-only) token multisets."""
    named = collections.Counter()
    opaque = collections.Counter()
    for addr, n in calls.items():
        name = name_map.get(addr)
        if name:
            named[name] += n
        else:
            opaque[f"unnamed:{addr:#010x}"] += n
    return named, opaque


# Populated by compute_name_weights() before any scoring pass runs (a module-
# level cache rather than a threaded parameter, since every scorer --
# score_pair, run_anchor_check, propose_names -- needs the same weights and
# none of them owns building the corpus). Empty means "no weighting yet",
# which multiset_jaccard treats as weight 1.0 per name, i.e. unweighted.
_NAME_WEIGHTS = {}


def compute_name_weights(feature_dicts):
    """Inverse-document-frequency weight per resident-callee name, pooled
    across every overlay/module feature dict given (Mickey's 107 and JFG's
    156 together, in practice). A name nearly every module calls (mmFree,
    osInvalDCache, mathRnd -- Sinf/Cosf/sqrtf top the JFG corpus at 50-62
    modules) is barely evidence that two SPECIFIC modules correspond; a name
    only 1-2 modules call is much stronger. Weight is 1/document_frequency,
    uncapped, so a name unique to one document on each side gets full
    weight and the common ones are downweighted proportionally."""
    doc_freq = collections.Counter()
    for feats in feature_dicts:
        for f in feats.values():
            for name in f.get("resident_named", {}):
                doc_freq[name] += 1
    return {name: 1.0 / count for name, count in doc_freq.items()}


def multiset_jaccard(a, b):
    """Weighted Jaccard over two Counters, weighted by _NAME_WEIGHTS (an
    unset/unknown name defaults to weight 1.0). None (not 0.0) when both are
    empty -- "no calls observed" is a missing signal, not evidence of a
    mismatch, and must not be scored as a hard miss."""
    keys = set(a) | set(b)
    if not keys:
        return None
    inter = sum(
        min(a.get(k, 0), b.get(k, 0)) * _NAME_WEIGHTS.get(k, 1.0) for k in keys
    )
    union = sum(
        max(a.get(k, 0), b.get(k, 0)) * _NAME_WEIGHTS.get(k, 1.0) for k in keys
    )
    return inter / union if union else None


def attribute_named_calls_to_functions(call_sites, functions, name_map):
    """{function_index: Counter(resident_name -> count)}, function_index
    indexing the *input* `functions` list (name, offset, size), for named
    resident calls only -- the per-function evidence a Tier B proposal
    needs. `call_sites` is [(site_offset, target_address), ...]."""
    order = sorted(range(len(functions)), key=lambda i: functions[i][1])
    starts = [functions[i][1] for i in order]
    result = collections.defaultdict(collections.Counter)
    for site_off, target in call_sites:
        name = name_map.get(target)
        if not name:
            continue
        pos = bisect.bisect_right(starts, site_off) - 1
        if pos < 0:
            continue
        idx = order[pos]
        _fname, foff, fsize = functions[idx]
        if foff <= site_off < foff + fsize:
            result[idx][name] += 1
    return result


# ---------------------------------------------------------------------------
# JFG side
# ---------------------------------------------------------------------------


def parse_jfg_syms(path):
    """Return {module_num: [(offset, name), ...]} plus main-code symbol set."""
    modules = defaultdict(list)
    main_code = []
    if not os.path.exists(path):
        return modules, main_code
    current = None
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            m = SECTION_RE.match(line.rstrip("\n"))
            if m:
                kind = m.group(2)
                if kind == "Main Code":
                    current = "main"
                elif kind.startswith("Module"):
                    num = int(m.group(3))
                    # Two sections in this file (0xD66 "Module 3430" and
                    # 0xFFC "Module 4092") are reserved-selector artifacts,
                    # not real overlay modules -- JFG only ships 157
                    # (jfg_us_syms_full.txt's own highest real "Module N" is
                    # 157, same as JFG_NUM_OVERLAYS below). Treat anything
                    # past that the same as Data (FFD) / BSS: not module code.
                    current = num if 1 <= num <= JFG_NUM_OVERLAYS else None
                else:
                    current = None  # Data (FFD) / BSS -- not module code
                continue
            if current is None:
                continue
            if current == "main":
                m2 = MAIN_SYM_RE.match(line)
                if m2:
                    main_code.append((int(m2.group(1), 16), m2.group(2)))
                continue
            m2 = SYM_RE.match(line)
            if m2:
                off = int(m2.group(1), 16)
                name = m2.group(2)
                if AUTOEXIT_RE.match(name):
                    continue
                modules[current].append((off, name))
    for k in modules:
        modules[k].sort()
    return modules, main_code


def parse_jfg_map(path):
    """Return {module_num: {"text": n, "data": n, "bss": n}}."""
    sizes = defaultdict(lambda: defaultdict(int))
    if not os.path.exists(path):
        return sizes
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = MAP_OBJ_RE.match(line)
            if not m:
                continue
            sec, _addr, size, n1, n2 = m.groups()
            n = int(n1 or n2)
            sizes[n][sec] += int(size, 16)
    return sizes


def parse_jfg_refs(path):
    """Return (out_edges, in_edges): {module: {neighbour: count}}."""
    out_edges = defaultdict(dict)
    in_edges = defaultdict(dict)
    if not os.path.exists(path):
        return out_edges, in_edges
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            m = REF_OUT_RE.match(line)
            if m:
                src = int(m.group(1))
                targets = [int(t) for t in m.group(2).split(",") if t.strip()]
                for t in targets:
                    out_edges[src][t] = out_edges[src].get(t, 0) + 1
                continue
            m = REF_IN_RE.match(line)
            if m:
                dst = int(m.group(1))
                sources = [int(t) for t in m.group(2).split(",") if t.strip()]
                for s in sources:
                    in_edges[dst][s] = in_edges[dst].get(s, 0) + 1
    return out_edges, in_edges


def jfg_function_sizes(offsets, text_size):
    """Given sorted (offset, name) pairs and the module's text size, return
    a list of (name, offset, size) using next-offset deltas and the text
    bound for the last one. Offsets/sizes only -- names are JFG's own."""
    out = []
    for i, (off, name) in enumerate(offsets):
        if i + 1 < len(offsets):
            end = offsets[i + 1][0]
        else:
            end = text_size if text_size else off
        size = max(end - off, 0)
        out.append((name, off, size))
    return out


def jfg_header(d, ovl):
    """One JFG OverlayHeader (1-indexed), per tools/extract_overlays.py /
    tools/overlay_reloc.py in JFG's own published repo."""
    base = JFG_OVERLAY_TABLE_ROM + (ovl - 1) * 0x20
    vram_base, rom_offset, text_size, data_size, bss_size = struct.unpack_from(
        ">iiiii", d, base
    )
    reloc_size, reloc2_size = struct.unpack_from(">HH", d, base + 0x14)
    return {
        "rom_offset": rom_offset,
        "text_size": text_size,
        "data_size": data_size,
        "reloc_size": reloc_size,
        "reloc2_size": reloc2_size,
    }


def jfg_ort_entry(d, index):
    """overlayRomTable[index] -> (overlay, offset); overlay 0 == main code."""
    word = struct.unpack_from(">I", d, JFG_OVERLAY_ROM_TABLE_ROM + index * 4)[0]
    return word >> 20, word & 0xFFFFF


def jfg_resident_calls(rom):
    """{module: {"calls": Counter(target_vram -> count),
                 "call_sites": [(text_offset, target_vram), ...]}}.

    Unlike Mickey, JFG's runtime linker relocates EVERY external `jal`,
    including calls into its own resident Main Code (overlay_reloc.py: a
    reloc_type of EXTERNAL or JUMP resolves through overlayRomTable, and
    overlay 0 in that table means "main"). So the resident-callee
    fingerprint is already sitting in JFG's own per-module relocation
    tables as plain metadata -- no JFG instruction word needs decoding at
    all, only arithmetic on the same reloc-table layout JFG's own tools
    publish.
    """
    if rom is None:
        return {}
    out = {}
    for ovl in range(1, JFG_NUM_OVERLAYS + 1):
        h = jfg_header(rom, ovl)
        rom_addr = JFG_OVERLAY_DATA_ROM + h["rom_offset"]
        reloc_base = rom_addr + h["text_size"] + h["data_size"]
        calls = collections.Counter()
        call_sites = []
        for table_start, size in (
            (reloc_base, h["reloc_size"]),
            (reloc_base + h["reloc_size"], h["reloc2_size"]),
        ):
            for i in range(size // 8):
                entry = table_start + i * 8
                symbol_index, info = struct.unpack_from(">II", rom, entry)
                target_offset = (info >> 8) & 0xFFFFFF
                patch_type = (info >> 4) & 0xF
                reloc_type = info & 0xF
                if patch_type != JFG_RELOC_PATCH_JAL:
                    continue
                if reloc_type == JFG_RELOC_TYPE_LOCAL:
                    continue  # intra-overlay -- can't be a resident call
                if symbol_index >= JFG_NUM_ORT_ENTRIES:
                    continue
                t_ovl, t_off = jfg_ort_entry(rom, symbol_index)
                if t_ovl != 0:
                    continue
                target = JFG_MAIN_BASE + t_off
                calls[target] += 1
                call_sites.append((target_offset, target))
        out[ovl] = {"calls": calls, "call_sites": call_sites}
    return out


def build_jfg_features(syms_path, map_path, refs_path, jfg_calls=None):
    modules, main_code = parse_jfg_syms(syms_path)
    sizes = parse_jfg_map(map_path)
    out_edges, in_edges = parse_jfg_refs(refs_path)
    main_names = {name for _, name in main_code}
    main_name_by_addr = {addr: name for addr, name in main_code}
    jfg_calls = jfg_calls or {}

    features = {}
    functions = {}
    for mod, offsets in modules.items():
        sz = sizes.get(mod, {})
        text = sz.get("text", 0)
        data = sz.get("data", 0) + sz.get("rodata", 0)
        bss = sz.get("bss", 0)
        fn_sizes = jfg_function_sizes(offsets, text)
        functions[mod] = fn_sizes
        out_deg = len(out_edges.get(mod, {}))
        in_deg = len(in_edges.get(mod, {}))
        out_rel = sum(out_edges.get(mod, {}).values())
        in_rel = sum(in_edges.get(mod, {}).values())
        calls = jfg_calls.get(mod, {}).get("calls", {})
        named, opaque = tokenize_calls(calls, main_name_by_addr)
        features[mod] = {
            "text_size": text,
            "data_size": data,
            "bss_size": bss,
            "function_count": len(offsets),
            "out_degree": out_deg,
            "in_degree": in_deg,
            "out_relocations": out_rel,
            "in_relocations": in_rel,
            "neighbours_out": set(out_edges.get(mod, {})),
            "neighbours_in": set(in_edges.get(mod, {})),
            "resident_named": named,
            "resident_opaque": opaque,
        }
    return features, functions, main_names, main_name_by_addr


# ---------------------------------------------------------------------------
# Mickey side
# ---------------------------------------------------------------------------


def mickey_function_ranges(atlas, objects=None, elf_factory=None):
    """Read real function boundaries from each module's compiled objects.

    Atlas rows describe translation units, not functions. Their count changes
    during a fold even when every function remains identical. Symbol offsets
    are qualified by the owning overlay and TU base; local statics are included.
    """
    import reloc_surface as surface

    objects = surface.linked_overlay_objects() if objects is None else objects
    elf_factory = surface.Elf if elf_factory is None else elf_factory
    modules = {m["overlay"]: m for m in atlas["modules"]}
    functions = defaultdict(dict)
    for overlay, obj in objects:
        module = modules[overlay]
        elf = elf_factory(obj)
        text_index, _section = elf.section(".text")
        symbols = [row for row in elf.symbols()
                   if row[4] == text_index and row[3] & 0xF == surface.STT_FUNC and row[2]]
        if not symbols:
            continue  # A linked data-only object has no text ownership row.
        base, extent = surface.tu_base(obj, overlay, module["text_ownership"])
        if base is None or text_index is None:
            raise ValueError(f"overlay {overlay}: object has no text ownership: {obj}")
        for name, value, size, _info, _section in symbols:
            if value < 0 or value + size > extent:
                raise ValueError(f"overlay {overlay}: function escapes its TU: {name}")
            offset = base + value
            prior = functions[overlay].get(offset)
            if prior and prior[2] != size:
                raise ValueError(f"overlay {overlay}: conflicting function boundaries at {offset:#x}")
            if prior is None or (prior[0].startswith("func_") and not name.startswith("func_")):
                functions[overlay][offset] = (name, offset, size)
    return {overlay: sorted(rows.values(), key=lambda row: row[1])
            for overlay, rows in functions.items()}


def build_mickey_features(atlas, mickey_calls=None, mickey_names=None, function_ranges=None):
    modules = atlas["modules"]
    in_edges = defaultdict(set)
    for m in modules:
        for imp in m["imports"]:
            in_edges[imp["overlay"]].add(m["overlay"])
    mickey_calls = mickey_calls or {}
    mickey_names = mickey_names or {}
    function_ranges = (mickey_function_ranges(atlas)
                       if function_ranges is None else function_ranges)

    features = {}
    functions = {}
    for m in modules:
        ovl = m["overlay"]
        text = hx(m["sections"]["text"]["size"])
        data = hx(m["sections"]["data_rodata"]["size"])
        bss = hx(m["bss_size"]) if isinstance(m["bss_size"], str) else m["bss_size"]
        c_entries = [e for e in m["text_ownership"] if e["type"] == "c"]
        fn_list = function_ranges.get(ovl, [])
        if text and not fn_list:
            raise ValueError(f"overlay {ovl}: no compiled function symbols; build the overlay objects")
        matched_count = sum(
            any(hx(e["offset"]) <= offset and offset + size <= hx(e["offset"]) + hx(e["size"])
                and e.get("matched", False) for e in c_entries)
            for _name, offset, size in fn_list)
        out_targets = {imp["overlay"] for imp in m["imports"]}
        out_rel = sum(imp["relocations"] for imp in m["imports"])
        functions[ovl] = fn_list
        calls = mickey_calls.get(ovl, {}).get("calls", {})
        named, opaque = tokenize_calls(calls, mickey_names)
        features[ovl] = {
            "text_size": text,
            "data_size": data,
            "bss_size": bss,
            "function_count": len(fn_list),
            "matched_function_count": matched_count,
            "out_degree": len(out_targets),
            "in_degree": len(in_edges[ovl]),
            "out_relocations": out_rel,
            "in_relocations": m["cross_overlay_inbound_relocations"],
            "neighbours_out": out_targets,
            "neighbours_in": set(in_edges[ovl]),
            "resident_named": named,
            "resident_opaque": opaque,
        }
    return features, functions


# ---------------------------------------------------------------------------
# Scoring
# ---------------------------------------------------------------------------

VECTOR_KEYS = (
    "text_size",
    "data_size",
    "bss_size",
    "function_count",
    "out_degree",
    "in_degree",
    "out_relocations",
    "in_relocations",
)

# Cosine similarity over these log-scaled counts collapses toward 1.0 for
# any pair of small, mostly-empty modules (a handful of near-zero dims that
# happen to point the same way outweighs the one dim that actually differs),
# which is exactly the regime most overlays are in (median text size is a
# few hundred bytes). ratio_similarity avoids that: each feature is scored
# as min/max in [0, 1], so two modules of very different size score low on
# that feature regardless of how many other dims are zero on both sides.


def ratio_similarity(a, b):
    hi, lo = max(a, b), min(a, b)
    if hi == 0:
        return 1.0
    return lo / hi


def degree_shape_similarity(fa, fb):
    """Cheap 'graph shape' term: how similar in/out degree and relocation
    counts are, independent of absolute size. Neighbour *identity* cannot be
    compared directly -- overlay numbers on the two sides are unrelated --
    so this stays a coarse degree-profile match, not a subgraph isomorphism."""
    parts = []
    for a, b in (
        (fa["out_degree"], fb["out_degree"]),
        (fa["in_degree"], fb["in_degree"]),
    ):
        hi, lo = max(a, b), min(a, b)
        parts.append(1.0 if hi == 0 else lo / hi)
    return sum(parts) / len(parts)


def score_pair(mfeat, jfeat):
    size_terms = [
        ratio_similarity(mfeat["text_size"], jfeat["text_size"]),
        ratio_similarity(mfeat["data_size"], jfeat["data_size"]),
        ratio_similarity(mfeat["bss_size"], jfeat["bss_size"]),
    ]
    # text size is the most reliable single signal (it is present and
    # nonzero for nearly every module on both sides); weight it double.
    size_score = (2 * size_terms[0] + size_terms[1] + size_terms[2]) / 4
    fn_score = ratio_similarity(mfeat["function_count"], jfeat["function_count"])
    shape = degree_shape_similarity(mfeat, jfeat)

    named_jaccard = multiset_jaccard(
        mfeat.get("resident_named", {}), jfeat.get("resident_named", {})
    )
    if named_jaccard is not None:
        # Named resident callees are the strongest signal available -- they
        # are the only feature that cannot be faked by two modules simply
        # happening to be a similar size (docs/overlay-graph.md sec. 1's
        # finding that metadata-only features top out at 9.6%). Give it the
        # majority weight; keep the old shape terms as a lower-weight
        # tiebreaker rather than dropping them, since a module with zero
        # named calls on one side still needs *some* signal.
        return 0.55 * named_jaccard + 0.25 * size_score + 0.12 * fn_score + 0.08 * shape
    # Neither side named a resident call (nothing decoded, or every callee
    # is still an unnamed stub): fall back to the original metadata-only
    # blend rather than silently treating the missing signal as 0.
    return 0.5 * size_score + 0.3 * fn_score + 0.2 * shape


def rank_candidates(m_features, j_features, top_n=3):
    results = {}
    for ovl, mfeat in m_features.items():
        scored = []
        for mod, jfeat in j_features.items():
            s = score_pair(mfeat, jfeat)
            scored.append((s, mod))
        scored.sort(key=lambda t: -t[0])
        results[ovl] = scored[:top_n]
    return results


def confidence_flag(top, mfeat=None):
    """A pair is 'confident' when the best score clears an absolute floor
    AND has a clear margin over the runner-up -- both are needed since a
    small module pool makes small absolute scores common. An all-zero (truly
    empty) Mickey overlay is never confident: ratio_similarity(0, 0) == 1.0
    for every feature, so an empty module trivially scores 1.0 against any
    other near-empty module and that is a degenerate match, not a real one."""
    if mfeat is not None and mfeat.get("text_size", 0) == 0:
        return False
    if len(top) < 2:
        return len(top) == 1 and top[0][0] >= 0.75
    best, second = top[0][0], top[1][0]
    if best < 0.70:
        return False
    if second <= 0:
        return True
    return (best - second) / best >= 0.25


# ---------------------------------------------------------------------------
# Name proposals for confident matches
# ---------------------------------------------------------------------------


def propose_names(m_functions, j_functions, ovl, mod, m_call_sites=None, j_call_sites=None,
                   mickey_names=None, jfg_main_names=None):
    """Line up Mickey's matched (named-by-offset-only, e.g. func_ stand-ins
    are absent here since only matched 'c' entries carry names) functions
    against JFG's module functions.

    Two passes: first, a Tier B pass -- for each Mickey function, find the
    JFG function in the same module whose OWN named-resident-callee set
    overlaps best (nonzero) with this Mickey function's own named-resident-
    callee set. That is call-graph evidence at the level of one function,
    not the module-wide shape score that picked this module in the first
    place, so it earns its own tier. Whatever is left over falls back to
    the old rank-order-by-offset Tier D pairing, unchanged.
    """
    mfns = m_functions.get(ovl, [])
    jfns = j_functions.get(mod, [])
    proposals = []
    if not mfns or not jfns:
        return proposals
    m_sorted = sorted(mfns, key=lambda t: t[1])
    j_sorted = sorted(jfns, key=lambda t: t[1])

    m_used, j_used = set(), set()
    if m_call_sites and j_call_sites and mickey_names and jfg_main_names:
        m_fn_calls = attribute_named_calls_to_functions(m_call_sites, m_sorted, mickey_names)
        j_fn_calls = attribute_named_calls_to_functions(j_call_sites, j_sorted, jfg_main_names)
        pairs = []
        for mi, mcalls in m_fn_calls.items():
            for ji, jcalls in j_fn_calls.items():
                jac = multiset_jaccard(mcalls, jcalls)
                if jac:
                    pairs.append((jac, mi, ji))
        pairs.sort(key=lambda t: -t[0])
        for jac, mi, ji in pairs:
            if mi in m_used or ji in j_used:
                continue
            m_used.add(mi)
            j_used.add(ji)
            mname, moff, msize = m_sorted[mi]
            jname, joff, jsize = j_sorted[ji]
            shared = sorted(set(m_fn_calls[mi]) & set(j_fn_calls[ji]))
            proposals.append(
                {
                    "mickey_source": mname,
                    "mickey_offset": hex(moff),
                    "mickey_size": hex(msize),
                    "jfg_name": jname,
                    "jfg_offset": hex(joff),
                    "jfg_size": hex(jsize),
                    "resident_callee_jaccard": round(jac, 3),
                    "shared_resident_callees": shared,
                    "evidence_tier": "B",
                    "note": (
                        "both functions call the same named resident "
                        "function(s) -- call-graph evidence, not shape"
                    ),
                }
            )

    # Rank-order alignment by position, since neither list is byte-comparable
    # across a revision gap. Both are pre-existing lists (Mickey's atlas
    # source names, JFG's published names): only the pairing is new here.
    m_remaining = [i for i in range(len(m_sorted)) if i not in m_used]
    j_remaining = [i for i in range(len(j_sorted)) if i not in j_used]
    n = min(len(m_remaining), len(j_remaining))
    for k in range(n):
        mname, moff, msize = m_sorted[m_remaining[k]]
        jname, joff, jsize = j_sorted[j_remaining[k]]
        if msize == 0 or jsize == 0:
            continue
        hi, lo = max(msize, jsize), min(msize, jsize)
        size_ratio = lo / hi
        proposals.append(
            {
                "mickey_source": mname,
                "mickey_offset": hex(moff),
                "mickey_size": hex(msize),
                "jfg_name": jname,
                "jfg_offset": hex(joff),
                "jfg_size": hex(jsize),
                "size_ratio": round(size_ratio, 3),
                "evidence_tier": "D",
                "note": (
                    "rank-order position + size agreement only; not a byte "
                    "or call-graph claim"
                ),
            }
        )
    return proposals


# ---------------------------------------------------------------------------
# Self-test: JFG-us vs JFG-kiosk should recover ~identity
# ---------------------------------------------------------------------------


def run_self_test():
    us_feat, us_fns, _, _ = build_jfg_features(JFG_SYMS_US, JFG_MAP, JFG_REFS)
    kiosk_modules, _ = parse_jfg_syms(JFG_SYMS_KIOSK)
    if not us_feat or not kiosk_modules:
        return {
            "ran": False,
            "reason": "JFG reference files not found at JFG_ROOT="
            + JFG_ROOT,
        }
    # Kiosk doesn't ship its own map/refs in this checkout; approximate its
    # sizes from symbol-offset deltas within each module's own text run
    # (last symbol's own module has no explicit text bound, so its final
    # function's size is left at 0 and excluded from that module's vector --
    # this only affects the coarse feature vector, not the recovery count).
    kiosk_feat = {}
    kiosk_fns = {}
    for mod, offsets in kiosk_modules.items():
        approx_text = offsets[-1][0] if offsets else 0
        fn_sizes = jfg_function_sizes(offsets, approx_text)
        kiosk_fns[mod] = fn_sizes
        kiosk_feat[mod] = {
            "text_size": approx_text,
            "data_size": 0,
            "bss_size": 0,
            "function_count": len(offsets),
            "out_degree": 0,
            "in_degree": 0,
            "out_relocations": 0,
            "in_relocations": 0,
            "neighbours_out": set(),
            "neighbours_in": set(),
        }

    ranked = rank_candidates(us_feat, kiosk_feat, top_n=3)
    total = len(ranked)
    top1_correct = 0
    top3_correct = 0
    confident = 0
    confident_correct = 0
    for mod, top in ranked.items():
        cands = [c[1] for c in top]
        if cands and cands[0] == mod:
            top1_correct += 1
        if mod in cands:
            top3_correct += 1
        if confidence_flag(top, us_feat.get(mod)):
            confident += 1
            if top and top[0][1] == mod:
                confident_correct += 1

    return {
        "ran": True,
        "pairs_tested": total,
        "top1_accuracy": round(top1_correct / total, 3) if total else 0,
        "top3_accuracy": round(top3_correct / total, 3) if total else 0,
        "confident_pairs": confident,
        "confident_pairs_correct": confident_correct,
        "confident_precision": round(confident_correct / confident, 3)
        if confident
        else None,
        "resident_callee_feature_engaged": False,
        "note": (
            "JFG-us vs JFG-kiosk is the SAME game at two revisions, so this "
            "upper-bounds what a size/count/degree profile can recover; the "
            "Mickey<->JFG problem is strictly harder (different game, "
            "different revision, resident-vs-overlay boundary moved). The "
            "resident-callee fingerprint added in this pass could NOT "
            "engage here: this checkout ships only JFG-kiosk's symbol list, "
            "not its ROM or a build, and the fingerprint needs the ROM's "
            "own relocation tables. score_pair() falls back to the "
            "original size/count/degree blend whenever a side's named-"
            "callee set is empty, which is every kiosk module, so this "
            "number is unchanged from the metadata-only pass and is not "
            "evidence the new feature works -- see the anchor check for "
            "that instead."
        ),
    }


def run_anchor_check(m_features, j_features):
    out = {}
    for ovl, anchor in KNOWN_ANCHORS.items():
        mod = anchor["jfg_module"]
        mfeat = m_features.get(ovl)
        jfeat = j_features.get(mod)
        if mfeat is None or jfeat is None:
            out[ovl] = {"symbol": anchor["symbol"], "jfg_module": mod, "ran": False}
            continue
        ranked_full = rank_candidates(
            {ovl: mfeat}, j_features, top_n=len(j_features)
        )[ovl]
        rank = next((i for i, (_, m) in enumerate(ranked_full) if m == mod), None)
        named_jaccard = multiset_jaccard(
            mfeat.get("resident_named", {}), jfeat.get("resident_named", {})
        )
        out[ovl] = {
            "symbol": anchor["symbol"],
            "jfg_module": mod,
            "score": next((s for s, m in ranked_full if m == mod), None),
            "rank_among_all_157": rank + 1 if rank is not None else None,
            "top1_score": ranked_full[0][0],
            "top1_module": ranked_full[0][1],
            "mickey_text_size": mfeat["text_size"],
            "jfg_text_size": jfeat["text_size"],
            "mickey_resident_named_calls": dict(mfeat.get("resident_named", {})),
            "jfg_resident_named_calls": dict(jfeat.get("resident_named", {})),
            "resident_named_jaccard": (
                round(named_jaccard, 3) if named_jaccard is not None else None
            ),
        }
    out["unanchorable"] = UNANCHORABLE
    return out


# ---------------------------------------------------------------------------
# Report assembly
# ---------------------------------------------------------------------------


def build_report():
    atlas = load_json(ATLAS)

    mickey_rom = load_rom(MICKEY_ROM)
    mickey_names = load_mickey_resident_names()
    mickey_calls = mickey_resident_calls(mickey_rom) if mickey_rom is not None else {}
    m_features, m_functions = build_mickey_features(atlas, mickey_calls, mickey_names)

    jfg_rom = load_rom(JFG_ROM)
    jfg_calls = jfg_resident_calls(jfg_rom) if jfg_rom is not None else {}
    j_features, j_functions, main_names, jfg_main_names = build_jfg_features(
        JFG_SYMS_US, JFG_MAP, JFG_REFS, jfg_calls
    )

    jfg_available = bool(j_features)
    global _NAME_WEIGHTS
    _NAME_WEIGHTS = compute_name_weights([m_features, j_features])
    ranked = rank_candidates(m_features, j_features) if jfg_available else {}

    overlays_with_named_calls = sum(1 for f in m_features.values() if f["resident_named"])
    modules_with_named_calls = sum(1 for f in j_features.values() if f["resident_named"])

    overlays_out = []
    confident_count = 0
    for ovl in sorted(m_features):
        top = ranked.get(ovl, [])
        confident = confidence_flag(top, m_features[ovl]) if top else False
        if confident:
            confident_count += 1
        candidates = []
        for score, mod in top:
            entry = {
                "jfg_module": mod,
                "score": round(score, 4),
                "jfg_text_size": j_features[mod]["text_size"],
                "jfg_function_count": j_features[mod]["function_count"],
                "resident_named_jaccard": (
                    lambda j: round(j, 3) if j is not None else None
                )(multiset_jaccard(m_features[ovl]["resident_named"], j_features[mod]["resident_named"])),
            }
            if confident and (score, mod) == top[0]:
                entry["name_proposals"] = propose_names(
                    m_functions,
                    j_functions,
                    ovl,
                    mod,
                    mickey_calls.get(ovl, {}).get("call_sites"),
                    jfg_calls.get(mod, {}).get("call_sites"),
                    mickey_names,
                    jfg_main_names,
                )
            candidates.append(entry)
        overlays_out.append(
            {
                "overlay": ovl,
                "mickey_text_size": m_features[ovl]["text_size"],
                "mickey_data_size": m_features[ovl]["data_size"],
                "mickey_bss_size": m_features[ovl]["bss_size"],
                "mickey_function_count": m_features[ovl]["function_count"],
                "mickey_matched_function_count": m_features[ovl][
                    "matched_function_count"
                ],
                "out_degree": m_features[ovl]["out_degree"],
                "in_degree": m_features[ovl]["in_degree"],
                "mickey_resident_named_calls": dict(m_features[ovl]["resident_named"]),
                "mickey_resident_opaque_call_count": sum(
                    m_features[ovl]["resident_opaque"].values()
                ),
                "confident": confident,
                "candidates": candidates,
            }
        )

    anchors = run_anchor_check(m_features, j_features) if jfg_available else {}
    self_test = run_self_test()

    report = {
        "schema_version": 2,
        "generated_by": "tools/overlay_graph_match.py",
        "source": {
            "mickey_atlas": os.path.relpath(ATLAS, REPO),
            "mickey_rom_available": mickey_rom is not None,
            "jfg_root": "$JFG_ROOT",
            "jfg_root_available": jfg_available,
            "jfg_rom_available": jfg_rom is not None,
        },
        "method": (
            "Weighted Jaccard over each overlay/module's multiset of NAMED "
            "resident-callee symbols (resolved from each side's relocation "
            "tables -- runlink.h / JFG's "
            "tools/overlay_reloc.py; docs/overlay-graph.md sec 1), blended "
            "0.55/0.25/0.12/0.08 with the prior size-ratio, function-count-"
            "ratio, and degree-shape terms. Falls back to the prior "
            "0.5/0.3/0.2 metadata-only blend when neither side named a "
            "resident call for a given pair. No neighbour-identity subgraph "
            "matching -- overlay numbers are not comparable across the two "
            "games. Mickey function boundaries come from overlay-qualified "
            "compiled function symbols, including statics; atlas ownership "
            "rows describe translation units."
        ),
        "totals": {
            "mickey_overlays": len(m_features),
            "jfg_modules_available": len(j_features),
            "confident_matches": confident_count,
            "mickey_overlays_with_named_resident_calls": overlays_with_named_calls,
            "jfg_modules_with_named_resident_calls": modules_with_named_calls,
        },
        "calibration": {
            "known_anchors": anchors,
            "self_test_jfg_us_vs_kiosk": self_test,
        },
        "overlays": overlays_out,
    }
    return report


def summarize(report):
    t = report["totals"]
    print(
        f"{t['mickey_overlays']} Mickey overlays, "
        f"{t['jfg_modules_available']} JFG modules available, "
        f"{t['confident_matches']} leads pass the score threshold"
    )
    st = report["calibration"]["self_test_jfg_us_vs_kiosk"]
    if st.get("ran"):
        print(
            f"self-test (JFG us vs kiosk): top1={st['top1_accuracy']} "
            f"top3={st['top3_accuracy']} "
            f"confident_precision={st['confident_precision']}"
        )
    else:
        print(f"self-test skipped: {st.get('reason')}")
    for ovl, a in report["calibration"]["known_anchors"].items():
        if ovl == "unanchorable":
            continue
        print(f"anchor overlay {ovl} ({a['symbol']}) -> module {a['jfg_module']}: "
              f"rank {a.get('rank_among_all_157')} of 157, "
              f"top1 is module {a.get('top1_module')}")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--overlay", type=int)
    args = ap.parse_args()

    if args.self_test:
        st = run_self_test()
        print(json.dumps(st, indent=2))
        return 0

    if args.write or args.check:
        required = (MICKEY_ROM, MICKEY_SYMS, JFG_ROM, JFG_SYMS_US,
                    JFG_SYMS_KIOSK, JFG_REFS, JFG_MAP)
        missing = [path for path in required if not os.path.isfile(path)]
        if missing:
            print("overlay graph requires the local MSU/JFG reference inputs; missing: "
                  + ", ".join(missing), file=sys.stderr)
            return 1
    try:
        report = build_report()
    except (OSError, ValueError) as error:
        print(f"overlay graph: {error}", file=sys.stderr)
        return 1

    if args.overlay is not None:
        row = next(
            (o for o in report["overlays"] if o["overlay"] == args.overlay), None
        )
        print(json.dumps(row, indent=2))
        return 0

    if args.write:
        with open(REPORT, "w", encoding="utf-8") as fh:
            json.dump(report, fh, indent=2)
            fh.write("\n")
        print(f"wrote {os.path.relpath(REPORT, REPO)}")
        summarize(report)
        return 0

    if args.check:
        if not os.path.exists(REPORT):
            print("missing config/overlay-graph.us.json; run --write", file=sys.stderr)
            return 1
        on_disk = load_json(REPORT)
        # Round-trip the freshly built report through JSON before comparing:
        # build_report() uses int overlay numbers as dict keys internally
        # (e.g. calibration.known_anchors), but JSON object keys are always
        # strings, so on_disk already has them as strings. Comparing the raw
        # Python dict against on_disk would report every re-run as stale.
        fresh = json.loads(json.dumps(report))
        # Compare everything except nothing -- both are fully deterministic
        # given the same inputs.
        if on_disk != fresh:
            print("config/overlay-graph.us.json is stale; run --write", file=sys.stderr)
            return 1
        print("config/overlay-graph.us.json is up to date")
        return 0

    summarize(report)
    return 0


if __name__ == "__main__":
    sys.exit(main())
