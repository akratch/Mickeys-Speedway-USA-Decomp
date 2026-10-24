#!/usr/bin/env python3
"""Generate Mickey's canonical overlay manifest and splat segment block.

The retail ROM is the only authority for module boundaries.  This tool reads
the same tables as :mod:`overlay_tables`, decodes every resident and per-module
relocation, and derives two tracked artifacts from that one model:

* ``config/overlays.us.json`` -- reviewable module, export, dependency, and
  campaign-priority metadata.  It contains addresses and counts, never ROM
  bytes or disassembly.
* the generated overlay block in ``mickey.us.yaml`` -- one independent code
  segment per non-empty module, with unowned initialized ranges and relocation
  tails retained as exact binary ranges.

The shipped ``vramBase`` is zero for every module.  Splat is given one common
synthetic VMA whose high nibble does not alter a MIPS J-type target field.  A
shared ``exclusive_ram_id`` tells Splat that every module occupies the same
mutually-exclusive RAM class, while ROM ranges and segment-qualified names
keep their symbols distinct.  Runtime addresses remain ``(overlay, section,
offset)`` throughout the atlas; the synthetic VMA is never reported as if it
were a loaded address.

Usage:
    python3 tools/overlay_atlas.py                 # concise summary
    python3 tools/overlay_atlas.py --write         # refresh manifest and YAML block
    python3 tools/overlay_atlas.py --check         # fail if either artifact is stale
    python3 tools/overlay_atlas.py --delta HEAD^    # ref versus worktree
    python3 tools/overlay_atlas.py --delta A B --format json
    python3 tools/overlay_atlas.py --overlay 61    # one detailed module row
    python3 tools/overlay_atlas.py --relocations 61  # one module's records
"""

import argparse
import collections
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

import overlay_tables
import executable_accounting


REPO = Path(__file__).resolve().parent.parent
DEFAULT_ROM = REPO / "baseroms" / "mickey.us.z64"
NON_MATCHING_RE = re.compile(r"^\s*#\s*ifdef\s+NON_MATCHING\b", re.MULTILINE)
GLOBAL_ASM_RE = re.compile(r"^\s*#\s*pragma\s+GLOBAL_ASM\b", re.MULTILINE)


def is_nonmatching_text(text):
    """Whether source text contains any compiler-unmatched function."""
    return bool(NON_MATCHING_RE.search(text) or GLOBAL_ASM_RE.search(text))


def is_nonmatching_source(overlay, source_name):
    """Whether overlays/oNNN/<source_name>.c contains unmatched code.

    Both a guarded candidate and a bare ``GLOBAL_ASM`` fallback make the
    broad translation-unit owner conservative. Reviewed exact functions in
    such a TU are credited separately through ``MIXED_TU_EXACT_C_RANGES``
    (empty while every overlay TU is complete).
    The signal is mechanically derived from source every run.
    """
    path = REPO / "src" / "overlays" / f"o{overlay:03d}" / f"{source_name}.c"
    if not path.is_file():
        return False
    with open(path, encoding="utf-8", errors="replace") as fh:
        return is_nonmatching_text(fh.read())
DEFAULT_MANIFEST = REPO / "config" / "overlays.us.json"
DEFAULT_YAML = REPO / "mickey.us.yaml"
MANIFEST_REPO_PATH = "config/overlays.us.json"

YAML_BEGIN = "  # BEGIN GENERATED OVERLAY SEGMENTS -- tools/overlay_atlas.py"
YAML_END = "  # END GENERATED OVERLAY SEGMENTS -- tools/overlay_atlas.py"
SYNTHETIC_VMA = 0xF0000000
OVERLAY_RAM_CLASS = "overlay_modules"

# Physical loader capacity only, never reconstructed C ownership. Splat's
# automatic .bss inputs reserve nothing when every C object has empty BSS.
# The explicit unowned segment materializes module 1's authenticated capacity;
# remove or split it when actual source-owned storage replaces that reserve.
UNOWNED_BSS_RESERVE_OVERLAYS = frozenset({1})

# The ladder exercises successively harder format cases before the generated
# split is treated as settled.  These are campaign choices, not inferred names.
PILOT_ROLES = {
    107: "text-only whole-module donor match",
    103: "text plus primary relocation table",
    76: "text, initialized data, BSS, and both relocation tables",
    61: "DKR-backed semantic crosswalk (ghost/controller-pak UI)",
}

# Evidence already established by relocation-aware object comparison.  Names
# are intentionally absent unless the match is unique and strong enough under
# docs/modules.md section 1.2.  The full donor scan is a separate report; this
# small map only contributes a stable campaign-priority signal to the atlas.
EXACT_DONOR_OVERLAYS = {
    107: {
        "project": "jfg",
        "object": "overlays/o156/overlay_156.c.o",
        "symbol": "osRamTest4_6105",
        "scope": "whole_text",
        "size": 0x30,
        "rom_occurrences": 1,
    },
}

# Reviewed text ownership. Offsets are relative to the module text start; all
# modules not listed here remain one generated-assembly subsegment. This map is
# deliberately explicit so the manifest, splat YAML, and progress metric share
# one collision-free `(overlay, offset)` source of truth.
DATA_RODATA_OWNERSHIP = {
    48: [(0x0, 0x280, "overlay_048")],
    89: [(0x0, 0x10, "overlay_089")],
    16: [(0x0, 0x10, "overlay_016")],
    23: [(0x0, 0x10, "overlay_023")],
    97: [(0x0, 0x10, "overlay_097")],
    95: [(0x0, 0x10, "overlay_095")],
    72: [(0x0, 0x10, "overlay_072")],
    22: [(0x0, 0x60, "overlay_022")],
    45: [(0x0, 0x80, "overlay_045")],
    47: [(0, 1376, 'overlay_047')],
    # Offsets are relative to data_rodata. The source must already own a C text
    # row: its compiled .data is linked before any remaining raw initialized
    # tail, so no duplicate C subsegment is emitted at the section boundary.
    15: [(0x0, 0x50, "overlay_015")],
    42: [(0x0, 0x10, "overlay_042")],
    79: [(0x0, 0x60, "overlay_079")],
    # The whole .data section; data_rodata +0x2D0..+0x2E0 is the original
    # TU's .rodata (its own relocation base) and stays raw.
    54: [(0x0, 0x2D0, "overlay_054")],
    # The whole data_rodata range: the compiled .data is 0x328 bytes padded to
    # the section's 16-byte alignment, which covers the zero tail.
    52: [(0x0, 0x330, "overlay_052")],
}

# Reviewed initialized subranges emitted by a C owner's non-.data section.
# Offsets are relative to the module's combined initialized data range.  Unlike
# DATA_RODATA_OWNERSHIP, these ranges may be interior to the raw range; the
# YAML projection therefore emits the raw fragments on either side and a
# typed subsegment for the owning TU.  The four-byte subalignment below is
# required for IDO's literal-pool and jump-table placement.  A sixth tuple
# field marks a pool whose candidate object is externalized back onto the
# retained raw slice; that trial leaves the raw bytes in place rather than
# carving them a second time.
FIXED_DATA_RODATA_OWNERSHIP = {
    # The fifth field identifies the guarded C definition that emits this
    # range. Consolidated TUs also contain candidates whose promotion does not
    # emit the owner's compiler data, so trial projection must be function-
    # scoped rather than merely source-file-scoped.
    1: [(0x274, 0x294, "overlay_001_tail", ".rodata", "overlay1DispatchMode", True)],
    8: [(0x27C, 0x2AC, "overlay_008", ".rodata", "func_overlay_008_F0004CF0_1862A48")],
    9: [(0x390, 0x3E0, "overlay_009", ".rodata", "func_overlay_009_F0000CE4_186735C", True)],
    14: [(0x158, 0x190, "overlay_014", ".rodata", "overlay14LoadRelocatedValue", True)],
    # The record/targeting update's five float literals, rodata-relative +0x0.
    29: [
        (
            0x0,
            0x14,
            "overlay_029",
            ".rodata",
            "func_overlay_029_F00005C4_187D874",
            True,
        )
    ],
    58: [
        # The module TU's .rodata: the state update's seven-entry switch table
        # and 0.02f literal, then the mode update's thirteen-entry table,
        # rodata-relative +0x104 through +0x158.
        (
            0x3D4,
            0x428,
            "overlay_058",
            ".rodata",
            "func_overlay_058_F00005FC_18AF7E4",
            True,
        )
    ],
    57: [
        # The module TU's .rodata: the initializer's mode switch table, then
        # the dispatch's, rodata-relative +0x6C through +0x104.
        (
            0x5AC,
            0x644,
            "overlay_057",
            ".rodata",
            "func_overlay_057_F0000000_18A3BF8",
            True,
        )
    ],
    59: [(0x76C, 0x78C, "overlay_059", ".rodata", "overlay59Advance", True)],
    # The 9-entry menu switch table, rodata-relative +0x188.
    61: [
        (
            0x278,
            0x29C,
            "overlay_061",
            ".rodata",
            "func_overlay_061_F0000B84_18BFF4C",
            True,
        )
    ],
    # The updater's five-entry mode-switch table and float literals,
    # rodata-relative +0xC (data_rodata +0xDC), which the shipped %hi/%lo
    # pairs encode.
    73: [
        (
            0xDC,
            0x12C,
            "overlay_073",
            ".rodata",
            "func_overlay_073_F0000190_18CAC50",
            True,
        )
    ],
    86: [
        (
            0x80,
            0xA0,
            "overlay_086",
            ".rodata",
            "func_overlay_086_F0000474_18D22AC",
            True,
        )
    ],
    # The state machine's float literals and seven-entry switch table,
    # rodata-relative +0x8, after overlay90Initialize's two data words.
    90: [
        (
            0x8,
            0x40,
            "overlay_090",
            ".rodata",
            "func_overlay_090_F00000FC_18D4BF4",
            True,
        )
    ],
    # The 25-entry selector switch table, rodata-relative +0xEAC; it ends the
    # module's initialized data.
    101: [
        (
            0x5CFC,
            0x5D60,
            "overlay_101",
            ".rodata",
            "func_overlay_101_F000C6E8_18E7F08",
            True,
        )
    ],
}

# Permanent interior initialized-storage owners. These rows participate in
# every canonical atlas/YAML projection, unlike function-scoped trial rows.
# The exact data-only source allowlist keeps physical ownership independent
# from executable text ownership and matching credit.
CANONICAL_FIXED_DATA_RODATA_OWNERSHIP = {
    11: [(0x30, 0x1B0, "overlay11BackgroundTriangles", ".data"),
         (0x1B0, 0x1E0, "overlay11MenuControls", ".data"),
         (0x1F0, 0x210, "overlay11BaseHandles", ".data")],
    58: [
        (0xC8, 0x108, "overlay58Assets", ".data"),
        (0x180, 0x1A0, "overlay58BadgeSprites", ".data"),
        (0x248, 0x278, "overlay58LayoutTables", ".data"),
        (0x2D0, 0x310, "overlay58TextLabels", ".rodata"),
    ],
    46: [(0x0, 0x60, "overlay46InitialState", ".data"),
         (0x60, 0x180, "overlay46EntryTables", ".data"),
         (0x184, 0x194, "overlay46InitialTransform", ".data"),
         (0x230, 0x330, "overlay46TrailTriangles", ".data")],
    50: [(0x6C, 0x21C, "overlay50GlyphLists", ".data"),
         (0x220, 0x2B0, "overlay50LapTemplate", ".data"),
         (0x2C0, 0x320, "overlay50HudSprites", ".data"),
         (0x328, 0x338, "overlay50ItemState", ".data"),
         (0x33C, 0x36C, "overlay50ControlTail", ".data")],
    48: [(0x280, 0x700, "overlay_048", ".rodata")],
    59: [(0x0, 0x5A0, "overlay59Frames", ".data")],
    57: [
        (0x1FC, 0x27C, "overlay57PanelSprites", ".data"),
        (0x32C, 0x36C, "overlay57DetailSprites", ".data"),
        (0x3C8, 0x4B8, "overlay57MenuTables", ".data"),
        (0x4D0, 0x4F0, "overlay57AnimationState", ".data"),
        (0x540, 0x580, "overlay57MenuLabels", ".rodata"),
    ],
    7: [(0x4C, 0x92C, "overlay7DispatchTables", ".data"),
        (0x930, 0x950, "overlay_007", ".rodata")],
    101: [(0x1D0, 0x230, "overlay101State", ".data"),
          (0x230, 0x340, "overlay101ClockData", ".data"),
          (0x340, 0x4E50, "overlay101EventStreams", ".data"),
          (0x4E50, 0x5A70, "overlay101Labels", ".data"),
          (0x5A70, 0x5AF0, "overlay101LabelTail", ".data"),
          (0x5AF0, 0x5C30, "overlay101WindowLabels", ".data")],
    75: [(0x0, 0x10, "overlay_075", ".rodata")],
    44: [(0x0, 0x50, "overlay_044", ".rodata")],
    8: [
        (0x02D0, 0x0310, "overlay8SurfaceNameTable", ".data"),
        (0x0310, 0x0360, "overlay8SelectorTables", ".data"),
        (0x0360, 0x03E0, "overlay8SelectorFlags", ".data"),
        (0x03E0, 0x0520, "overlay8EffectTables", ".data"),
        (0x0520, 0x05E0, "overlay8MotionScales", ".rodata"),
        (0x05E0, 0x20C0, "overlay8TuningRecords", ".data"),
        (0x20C0, 0x2110, "overlay8TuningTables", ".data"),
        (0x2110, 0x2280, "overlay8ParameterTables", ".data"),
        (0x2280, 0x2330, "overlay8SurfaceNames", ".data"),
        (0x2378, 0x2418, "overlay8MotionConstants", ".data"),
    ],
    9: [(0x2D0, 0x300, "overlay9ParameterState", ".data"),
        (0x300, 0x390, "overlay9CameraTables", ".data")],
    24: [(0x0, 0x10, "overlay_024", ".rodata")],
    25: [
        (0x0, 0x20, "overlay_025", ".data"),
        (0x20, 0x40, "overlay_025", ".rodata"),
    ],
    26: [(0x0, 0x20, "overlay_026", ".rodata")],
    27: [
        (0x0, 0x1B0, "overlay_027", ".data"),
        (0x1B0, 0x1C0, "overlay_027", ".rodata"),
    ],
    28: [(0x0, 0x130, "overlay_028", ".data")],
    31: [(0x0, 0x40, "overlay_031", ".rodata")],
    34: [(0x0, 0x20, "overlay_034", ".data")],
    36: [
        (0x0, 0x400, "overlay_036", ".data"),
        (0x400, 0x440, "overlay_036", ".rodata"),
    ],
    37: [
        (0x0, 0xE0, "overlay_037", ".data"),
        (0xE0, 0xF0, "overlay_037", ".rodata"),
    ],
    38: [
        (0x0, 0x90, "overlay_038", ".data"),
        (0x90, 0xA0, "overlay_038", ".rodata"),
    ],
    40: [(0x0, 0x30, "overlay_040", ".data")],
    64: [(0x0, 0x10, "overlay_064", ".data")],
    77: [
        (0x0, 0x10, "overlay_077", ".data"),
        (0x10, 0x30, "overlay_077", ".rodata"),
    ],
    47: [(1376, 1440, 'overlay_047', '.rodata')],
    80: [(0x0, 0x30, "overlay_080", ".rodata")],
    74: [(0x0, 0x10, "overlay_074", ".rodata")],
    76: [(0x0, 0x10, "overlay_076", ".data")],
    81: [(0x0, 0x10, "overlay_081", ".data"),
         (0x10, 0x20, "overlay_081", ".rodata")],
    91: [(0x0, 0x20, "overlay_091", ".rodata")],
    97: [(0x10, 0x20, "overlay_097", ".rodata")],
    82: [(0x0, 0xA0, "overlay_082", ".rodata")],
}
DATA_ONLY_FIXED_DATA_SOURCES = {
    (11, "overlay11BackgroundTriangles", ".data"),
    (11, "overlay11MenuControls", ".data"),
    (11, "overlay11BaseHandles", ".data"),
    (9, "overlay9ParameterState", ".data"),
    (9, "overlay9CameraTables", ".data"),
    (58, "overlay58Assets", ".data"),
    (58, "overlay58BadgeSprites", ".data"),
    (58, "overlay58LayoutTables", ".data"),
    (58, "overlay58TextLabels", ".rodata"),
    (46, "overlay46InitialState", ".data"),
    (46, "overlay46EntryTables", ".data"),
    (46, "overlay46InitialTransform", ".data"),
    (46, "overlay46TrailTriangles", ".data"),
    (50, "overlay50GlyphLists", ".data"),
    (50, "overlay50LapTemplate", ".data"),
    (50, "overlay50HudSprites", ".data"),
    (50, "overlay50ItemState", ".data"),
    (50, "overlay50ControlTail", ".data"),
    (59, "overlay59Frames", ".data"),
    (57, "overlay57PanelSprites", ".data"),
    (57, "overlay57DetailSprites", ".data"),
    (57, "overlay57AnimationState", ".data"),
    (57, "overlay57MenuTables", ".data"),
    (57, "overlay57MenuLabels", ".rodata"),
    (7, "overlay7DispatchTables", ".data"),
    (101, "overlay101Labels", ".data"),
    (101, "overlay101WindowLabels", ".data"),
    (101, "overlay101State", ".data"),
    (101, "overlay101LabelTail", ".data"),
    (101, "overlay101ClockData", ".data"),
    (101, "overlay101EventStreams", ".data"),
    (8, "overlay8SelectorFlags", ".data"),
    (8, "overlay8MotionConstants", ".data"),
    (8, "overlay8EffectTables", ".data"),
    (8, "overlay8SurfaceNameTable", ".data"),
    (8, "overlay8SelectorTables", ".data"),
    (8, "overlay8MotionScales", ".rodata"),
    (8, "overlay8TuningRecords", ".data"),
    (8, "overlay8TuningTables", ".data"),
    (8, "overlay8ParameterTables", ".data"),
    (8, "overlay8SurfaceNames", ".data"),
}

# Reviewed gaps that require explicit padding rather than compiler alignment.
EXPLICIT_TEXT_PADDING = {}


TEXT_SUBSEGMENTS = {
    3: [
        (0x000, "c", "overlay_003"),
        (0x7C4, "asm", "overlay_003_padding"),
    ],
    4: [
        (0x000, "c", "overlay_004"),
        (0xCAC, "asm", "overlay_004_padding"),
    ],
    1: [
        (0x0000, "c", "overlay_001"),
        (0x07B0, "c", "overlay_001_build"),
        (0x0BD4, "c", "overlay_001_head"),
        (0x1D78, "c", "func_overlay_001_F0001D78_184E158"),
        (0x2744, "c", "overlay_001_middle"),
        (0x2B4C, "c", "func_overlay_001_F0002B4C_184EF2C"),
        (0x3258, "c", "func_overlay_001_F0003258_184F638"),
        (0x3578, "c", "overlay_001_tail"),
        (0x7BDC, "c", "overlay_001_create"),
        (0x7D6C, "c", "overlay_001_end"),
        (0x80BC, "c", "overlay_001_scaled"),
        (0x8114, "asm", "overlay_001_padding"),
    ],
    2: [
        (0x000, "c", "overlay_002"),
        (0x2548, "asm", "overlay_002_tail"),
    ],
    5: [
        (0x000, "c", "overlay_005"),
        (0x764, "asm", "overlay_005_padding"),
    ],
    6: [
        (0x000, "c", "overlay_006"),
        (0x01C, "asm", "overlay_006_padding"),
    ],
    7: [
        (0x000, "c", "overlay_007"),
        (0xFB8, "asm", "overlay_007_padding"),
    ],
    8: [
        (0x000, "c", "overlay_008"),
        (0x5128, "asm", "overlay_008_padding"),
    ],
    9: [
        (0x000, "c", "overlay_009"),
        (0x151C, "asm", "overlay_009_padding"),
    ],
    10: [
        (0x000, "c", "overlay_010"),
    ],
    12: [
        (0x000, "c", "overlay_012"),
        (0x129C, "asm", "overlay_012_padding"),
    ],
    13: [
        (0x000, "c", "overlay_013"),
        (0xB2C, "asm", "overlay_013_padding"),
    ],
    14: [
        (0x000, "c", "overlay_014"),
    ],
    15: [
        (0x000, "c", "overlay_015"),
        (0xC6C, "asm", "overlay_015_padding"),
    ],
    16: [
        (0x000, "c", "overlay_016"),
        (0x424, "asm", "overlay_016_padding"),
    ],
    18: [
        (0x000, "c", "overlay_018"),
    ],
    19: [
        (0x000, "c", "overlay_019"),
        (0x12E4, "asm", "overlay_019_padding"),
    ],
    20: [
        (0x000, "c", "overlay_020"),
        (0x1484, "asm", "overlay_020_padding"),
    ],
    22: [
        (0x000, "c", "overlay_022"),
        (0xE9C, "asm", "overlay_022_padding"),
    ],
    31: [
        (0x000, "c", "overlay_031"),
        (0xF44, "asm", "overlay_031_padding"),
    ],
    33: [
        (0x000, "c", "overlay_033"),
        (0x728, "asm", "overlay_033_padding"),
    ],
    36: [
        (0x000, "c", "overlay_036"),
        (0x1748, "asm", "overlay_036_padding"),
    ],
    38: [
        (0x000, "c", "overlay_038"),
        (0x7E8, "asm", "overlay_038_padding"),
    ],
    41: [
        (0x000, "c", "overlay_041"),
    ],
    21: [
        (0x000, "c", "overlay_021"),
        (0x2D4, "asm", "overlay_021_padding"),
    ],
    23: [
        (0x000, "c", "overlay_023"),
        (0x568, "asm", "overlay_023_padding"),
    ],
    24: [
        (0x000, "c", "overlay_024"),
        (0x414, "asm", "overlay_024_padding"),
    ],
    25: [
        (0x000, "c", "overlay_025"),
        (0x608, "asm", "overlay_025_padding"),
    ],
    34: [
        (0x000, "c", "overlay_034"),
    ],
    27: [
        (0x000, "c", "overlay_027"),
    ],
    37: [
        (0x000, "c", "overlay_037"),
        (0x558, "asm", "overlay_037_padding"),
    ],
    39: [
        (0x000, "c", "overlay_039"),
        (0x168, "asm", "overlay_039_padding"),
    ],
    40: [
        (0x000, "c", "overlay_040"),
        (0x824, "asm", "overlay_040_padding"),
    ],
    42: [
        (0x000, "c", "overlay_042"),
    ],
    43: [
        (0x000, "c", "overlay_043"),
        (0x1424, "asm", "overlay_043_padding"),
    ],
    44: [
        (0x000, "c", "overlay_044"),
        (0xAF4, "asm", "overlay_044_padding"),
    ],
    45: [
        (0x000, "c", "overlay_045"),
        (0x1C1C, "asm", "overlay_045_padding"),
    ],
    47: [(0, 'c', 'overlay_047'), (11752, 'asm', 'overlay_047_padding')],
    49: [
        (0x000, "c", "overlay_049"),
        (0x374, "asm", "overlay_049_padding"),
    ],
    56: [
        (0x000, "c", "overlay_056"),
        (0xAF4, "asm", "overlay_056_padding"),
    ],
    63: [
        (0x000, "c", "overlay_063"),
        (0x928, "asm", "overlay_063_padding"),
    ],
    64: [
        (0x000, "c", "overlay_064"),
    ],
    65: [
        (0x000, "c", "overlay65Initialize"),
        (0x080, "c", "overlay_065"),
        (0x1BB4, "asm", "overlay_065_padding"),
    ],
    61: [
        (0x000, "c", "overlay_061"),
        (0x1A84, "asm", "overlay_061_padding"),
    ],
    66: [
        (0x000, "c", "overlay_066"),
    ],
    67: [
        (0x000, "c", "overlay_067"),
        (0x14C, "asm", "overlay_067_padding"),
    ],
    68: [
        (0x000, "c", "overlay_068"),
        (0x15AC, "asm", "overlay_068_padding"),
    ],
    69: [
        (0x000, "c", "overlay_069"),
        (0x70C, "asm", "overlay_069_padding"),
    ],
    70: [
        (0x000, "c", "overlay_070"),
        (0x728, "asm", "overlay_070_padding"),
    ],
    71: [
        (0x000, "c", "overlay_071"),
        (0xB48, "asm", "overlay_071_tail"),
    ],
    72: [
        (0x000, "c", "overlay_072"),
        (0x168, "asm", "overlay_072_padding"),
    ],
    73: [
        (0x000, "c", "overlay_073"),
        (0xEA8, "asm", "overlay_073_padding"),
    ],
    74: [
        (0x000, "c", "overlay_074"),
        (0x248, "asm", "overlay_074_padding"),
    ],
    75: [
        (0x000, "c", "overlay_075"),
        (0x6F8, "asm", "overlay_075_padding"),
    ],
    76: [
        (0x000, "c", "overlay_076"),
        (0x114, "asm", "overlay_076_padding"),
    ],
    77: [
        (0x000, "c", "overlay_077"),
    ],
    78: [
        (0x000, "c", "overlay_078"),
        (0x0A8, "asm", "overlay_078_padding"),
    ],
    79: [
        (0x000, "c", "overlay_079"),
    ],
    80: [
        (0x000, "c", "overlay_080"),
        (0x3EC, "asm", "overlay_080_padding"),
    ],
    81: [
        (0x000, "c", "overlay_081"),
        (0x34C, "asm", "overlay_081_padding"),
    ],
    82: [
        (0x000, "c", "overlay_082"),
        (0x4CC, "asm", "overlay_082_padding"),
    ],
    83: [
        (0x000, "c", "overlay_083"),
    ],
    84: [
        (0x000, "c", "overlay_084"),
        (0x13BC, "asm", "overlay_084_padding"),
    ],
    85: [
        (0x000, "c", "overlay_085"),
        (0x29C, "asm", "overlay_085_padding"),
    ],
    86: [
        (0x000, "c", "overlay_086"),
        (0xECC, "asm", "overlay_086_padding"),
    ],
    88: [
        (0x000, "c", "overlay_088"),
    ],
    89: [
        (0x000, "c", "overlay_089"),
        (0x7C4, "asm", "overlay_089_padding"),
    ],
    90: [
        (0x000, "c", "overlay_090"),
        (0xB1C, "asm", "overlay_090_padding"),
    ],
    92: [
        (0x000, "c", "overlay_092"),
    ],
    93: [
        (0x000, "c", "overlay_093"),
        (0x0EC, "asm", "overlay_093_padding"),
    ],
    94: [
        (0x000, "c", "overlay_094"),
        (0x568, "asm", "overlay_094_padding"),
    ],
    95: [
        (0x000, "c", "overlay_095"),
        (0x1D8, "asm", "overlay_095_padding"),
    ],
    96: [
        (0x000, "c", "overlay_096"),
        (0x6D4, "asm", "overlay_096_padding"),
    ],
    97: [
        (0x000, "c", "overlay_097"),
        (0xA54, "asm", "overlay_097_padding"),
    ],
    98: [
        (0x000, "c", "overlay_098"),
        (0xA04, "asm", "overlay_098_padding"),
    ],
    99: [
        (0x000, "c", "overlay_099"),
        (0x135C, "asm", "overlay_099_padding"),
    ],
    100: [
        (0x000, "c", "overlay_100"),
        (0x94C, "asm", "overlay_100_padding"),
    ],
    17: [
        (0x000, "c", "overlay_017"),
    ],
    55: [
        (0x000, "c", "overlay_055"),
        (0xC68, "asm", "overlay_055_padding"),
    ],
    50: [
        (0x000, "c", "overlay_050"),
        (0x1E68, "asm", "overlay_050_padding"),
    ],
    51: [
        (0x000, "c", "overlay_051"),
        (0x8AC, "asm", "overlay_051_padding"),
    ],
    52: [
        (0x000, "c", "overlay_052"),
        (0x2128, "asm", "overlay_052_padding"),
    ],
    53: [
        (0x000, "c", "overlay_053"),
        (0xCA8, "asm", "overlay_053_padding"),
    ],
    54: [
        (0x000, "c", "overlay_054"),
        (0x1EE4, "asm", "overlay_054_padding"),
    ],
    57: [
        (0x000, "c", "overlay_057"),
    ],
    58: [
        (0x000, "c", "overlay_058"),
        (0x5A14, "asm", "overlay_058_padding_b"),
    ],
    62: [
        (0x000, "c", "overlay_062"),
    ],
    87: [
        (0x000, "c", "overlay_087"),
        (0x964, "asm", "overlay_087_padding"),
    ],
    91: [
        (0x000, "c", "overlay_091"),
        (0x574, "asm", "overlay_091_padding"),
    ],
    11: [
        (0x000, "c", "overlay_011"),
        (0x2CB4, "asm", "overlay_011_padding"),
    ],
    29: [
        (0x000, "c", "overlay_029"),
        (0x16CC, "asm", "overlay_029_padding"),
    ],
    26: [
        (0x000, "c", "overlay_026"),
    ],
    30: [
        (0x000, "c", "overlay_030"),
        (0x438, "asm", "overlay_030_padding"),
    ],
    28: [
        (0x000, "c", "overlay_028"),
        (0x7EC, "asm", "overlay_028_padding"),
    ],
    35: [
        (0x000, "c", "overlay_035"),
        (0x13E8, "asm", "overlay_035_padding"),
    ],
    59: [
        (0x000, "c", "overlay_059"),
        (0xA1C, "asm", "overlay_059_padding"),
    ],
    60: [
        (0x000, "c", "overlay_060"),
        (0x355C, "asm", "overlay_060_padding"),
    ],
    46: [
        (0x000, "c", "overlay_046"),
        (0x195C, "asm", "overlay_046_padding"),
    ],
    48: [
        (0x000, "c", "overlay_048"),
        (0x46C, "asm", "overlay_048_padding"),
    ],
    101: [
        (0x000, "c", "overlay_101"),
        (0xCED8, "asm", "overlay_101_padding"),
    ],
    102: [
        (0x000, "c", "overlay_102"),
        (0x06C, "asm", "overlay_102_padding"),
    ],
    103: [
        (0x000, "c", "overlay_103"),
        (0x06C, "asm", "overlay_103_padding"),
    ],
    104: [(0x000, "c", "overlay_104")],
    105: [(0x000, "c", "overlay_105")],
    106: [
        (0x000, "c", "overlay_106"),
        (0x008, "asm", "overlay_106_padding"),
    ],
    107: [
        (0x000, "c", "overlay_107"),
        (0x028, "asm", "overlay_107_padding"),
    ],
}

# A consolidated C object can contain both untouched, byte-exact C functions
# and GLOBAL_ASM fallbacks. `is_nonmatching_source()` intentionally remains a
# conservative object-level signal, so a reviewed exact function inside such a
# *mixed* TU is credited separately: one row per function,
#
#     overlay: [(start, end, "ownFunctionName"[, "range-specific evidence"]), ...]
#
# sorted by offset, non-overlapping, each inside exactly one mixed C ownership
# row, labelled with the function's own C definition (promotion-proof
# authenticates a function only against such a range). A three-field row
# defaults its evidence to the pre-consolidation metadata-only proof.
#
# Every overlay TU is complete now (no `#pragma GLOBAL_ASM` and no NON_MATCHING
# block in src/overlays; no text_ownership row is nonmatching), so there is no
# mixed TU to credit. The 182 rows this table
# carried until 2026-10-21 each sat inside a fully matched TU, which
# `mixed_tu_exact_c_rows()` already skipped; removing them left
# config/overlays.us.json and the mickey.us.yaml block byte-identical. The
# code path stays for a future mixed TU.
MIXED_TU_EXACT_C_RANGES = {}


def hx(value):
    """Stable hexadecimal rendering for reviewable address/size fields."""
    return f"0x{value:X}"


def counter_dict(counter, names=None):
    """A deterministic JSON object for an integer-keyed Counter."""
    out = {}
    for key in sorted(counter):
        label = names.get(key, str(key)) if names else str(key)
        out[label] = counter[key]
    return out


def range_row(start, end):
    return {"start": hx(start), "end": hx(end), "size": hx(end - start)}


class AtlasDeltaError(ValueError):
    """An atlas state cannot be compared without guessing at ownership."""


def _atlas_int(value, field, state):
    if isinstance(value, bool):
        raise AtlasDeltaError(f"{state}: {field} is not an integer")
    try:
        if isinstance(value, str):
            parsed = int(value, 0)
        elif isinstance(value, int):
            parsed = value
        else:
            raise TypeError
    except (TypeError, ValueError) as exc:
        raise AtlasDeltaError(f"{state}: invalid {field} {value!r}") from exc
    if parsed < 0:
        raise AtlasDeltaError(f"{state}: negative {field} {value!r}")
    return parsed


def _exact_c_row(overlay, row, state, mixed=False):
    try:
        start = _atlas_int(row["offset"], "offset", state)
        end = _atlas_int(row["end_offset"], "end_offset", state)
        size = _atlas_int(row["size"], "size", state)
    except KeyError as exc:
        raise AtlasDeltaError(
            f"{state}: overlay {overlay} exact-C row lacks {exc.args[0]}"
        ) from exc
    if start >= end or size != end - start:
        raise AtlasDeltaError(
            f"{state}: overlay {overlay} exact-C range {hx(start)}..{hx(end)} "
            f"has inconsistent size {hx(size)}"
        )
    source = row.get("source")
    if not isinstance(source, str) or not source:
        raise AtlasDeltaError(
            f"{state}: overlay {overlay} exact-C range at {hx(start)} "
            "has no source"
        )
    label = row.get("label") if mixed else source.rsplit("/", 1)[-1]
    if not isinstance(label, str) or not label:
        raise AtlasDeltaError(
            f"{state}: overlay {overlay} exact-C range at {hx(start)} "
            "has no label"
        )
    return {
        "key": f"overlay:{overlay}:text:{hx(start)}",
        "overlay": overlay,
        "section": "text",
        "offset": start,
        "end_offset": end,
        "size": size,
        "label": label,
        "source": source,
        "kind": "mixed_tu_range" if mixed else "ownership_row",
    }


def exact_c_index(atlas, state="atlas"):
    """Return exact C ranges keyed by canonical overlay/text offset.

    Release deltas must not infer identity from a synthetic VMA, source name,
    or row order. Duplicate modules, duplicate starts, overlapping exact-C
    ranges, inconsistent extents, and a total that disagrees with the atlas
    all fail closed.
    """
    modules = atlas.get("modules")
    if not isinstance(modules, list):
        raise AtlasDeltaError(f"{state}: modules is not a list")
    index = {}
    seen_overlays = set()
    by_overlay = collections.defaultdict(list)
    for module in modules:
        if not isinstance(module, dict) or "overlay" not in module:
            raise AtlasDeltaError(f"{state}: malformed module row")
        overlay = _atlas_int(module["overlay"], "overlay", state)
        if not 1 <= overlay <= overlay_tables.HEADER_COUNT:
            raise AtlasDeltaError(f"{state}: overlay {overlay} is out of range")
        if overlay in seen_overlays:
            raise AtlasDeltaError(f"{state}: duplicate overlay {overlay} module")
        seen_overlays.add(overlay)
        ownership = module.get("text_ownership", [])
        mixed_ranges = module.get("mixed_tu_exact_c_ranges", [])
        if not isinstance(ownership, list) or not isinstance(mixed_ranges, list):
            raise AtlasDeltaError(
                f"{state}: overlay {overlay} ownership rows are not lists"
            )
        candidates = []
        parsed_ownership = []
        for row in ownership:
            if not isinstance(row, dict):
                raise AtlasDeltaError(
                    f"{state}: overlay {overlay} has a malformed ownership row"
                )
            if row.get("type") != "c":
                continue
            if row.get("matched") is not True or not isinstance(
                row.get("nonmatching"), bool
            ):
                raise AtlasDeltaError(
                    f"{state}: overlay {overlay} C ownership row at "
                    f"{row.get('offset')!r} has ambiguous matching status"
                )
            parsed = _exact_c_row(overlay, row, state)
            parsed_ownership.append((parsed, row["nonmatching"]))
            if not row["nonmatching"]:
                candidates.append(parsed)
        for row in mixed_ranges:
            if not isinstance(row, dict):
                raise AtlasDeltaError(
                    f"{state}: overlay {overlay} has a malformed mixed-TU row"
                )
            candidate = _exact_c_row(overlay, row, state, mixed=True)
            containers = [
                owner
                for owner, nonmatching in parsed_ownership
                if nonmatching
                and owner["offset"] <= candidate["offset"]
                and candidate["end_offset"] <= owner["end_offset"]
            ]
            if len(containers) != 1:
                raise AtlasDeltaError(
                    f"{state}: overlay {overlay} mixed-TU exact range "
                    f"text+{hx(candidate['offset'])} is not inside exactly "
                    "one nonmatching C owner"
                )
            candidates.append(candidate)
        for candidate in candidates:
            key = (candidate["overlay"], candidate["offset"])
            if key in index:
                raise AtlasDeltaError(
                    f"{state}: ambiguous exact-C identity overlay {overlay} "
                    f"text+{hx(candidate['offset'])}"
                )
            index[key] = candidate
            by_overlay[overlay].append(candidate)

    for overlay, rows in by_overlay.items():
        previous = None
        for row in sorted(rows, key=lambda item: item["offset"]):
            if previous is not None and row["offset"] < previous["end_offset"]:
                raise AtlasDeltaError(
                    f"{state}: overlapping exact-C identities in overlay {overlay}: "
                    f"text+{hx(previous['offset'])}..{hx(previous['end_offset'])} "
                    f"and text+{hx(row['offset'])}..{hx(row['end_offset'])}"
                )
            previous = row

    extracted_total = sum(row["size"] for row in index.values())
    totals = atlas.get("totals")
    if not isinstance(totals, dict) or "matched_overlay_c_bytes" not in totals:
        raise AtlasDeltaError(f"{state}: atlas lacks an exact-C byte total")
    declared_total = _atlas_int(
        totals["matched_overlay_c_bytes"],
        "totals.matched_overlay_c_bytes",
        state,
    )
    if declared_total != extracted_total:
        raise AtlasDeltaError(
            f"{state}: exact-C rows total {extracted_total} bytes, "
            f"atlas declares {declared_total}"
        )
    return index


def _reviewed_mixed_repartition(atlas, owner, mixed, state):
    """Recognize an exact whole owner deliberately narrowed to proven subranges."""
    if owner["kind"] != "ownership_row" or mixed["kind"] != "mixed_tu_range":
        return False
    if owner["overlay"] != mixed["overlay"] or owner["source"] != mixed["source"]:
        return False
    if not (
        owner["offset"] <= mixed["offset"]
        and mixed["end_offset"] <= owner["end_offset"]
    ):
        return False
    modules = atlas.get("modules", [])
    module = next(
        (
            item
            for item in modules
            if isinstance(item, dict)
            and _atlas_int(item.get("overlay"), "overlay", state) == owner["overlay"]
        ),
        None,
    )
    if module is None:
        return False
    for row in module.get("text_ownership", []):
        if not isinstance(row, dict) or row.get("type") != "c":
            continue
        parsed = _exact_c_row(owner["overlay"], row, state)
        if (
            row.get("matched") is True
            and row.get("nonmatching") is True
            and parsed["offset"] == owner["offset"]
            and parsed["end_offset"] == owner["end_offset"]
            and parsed["source"] == owner["source"]
        ):
            return True
    return False


def _uncovered_exact_segments(row, blockers):
    """Split one exact row into the physical byte ranges absent from blockers."""
    cursor = row["offset"]
    segments = []
    for blocker in sorted(blockers, key=lambda item: item["offset"]):
        if blocker["end_offset"] <= cursor:
            continue
        if blocker["offset"] >= row["end_offset"]:
            break
        if blocker["offset"] > cursor:
            segments.append((cursor, min(blocker["offset"], row["end_offset"])))
        cursor = max(cursor, blocker["end_offset"])
        if cursor >= row["end_offset"]:
            break
    if cursor < row["end_offset"]:
        segments.append((cursor, row["end_offset"]))
    result = []
    for start, end in segments:
        segment = dict(row)
        segment.update(
            key=f"overlay:{row['overlay']}:text:{hx(start)}",
            offset=start,
            end_offset=end,
            size=end - start,
        )
        result.append(segment)
    return result


def compare_exact_c_atlases(
    base_atlas, target_atlas, base_name="base", target_name="target"
):
    """Compute an auditable exact-C ownership transition between atlases."""
    base = exact_c_index(base_atlas, base_name)
    target = exact_c_index(target_atlas, target_name)
    base_rows = list(base.values())
    target_rows = list(target.values())
    for before in base_rows:
        for after in target_rows:
            if before["overlay"] != after["overlay"]:
                continue
            if (
                before["end_offset"] <= after["offset"]
                or after["end_offset"] <= before["offset"]
            ):
                continue
            if (
                before["offset"] == after["offset"]
                and before["end_offset"] == after["end_offset"]
            ):
                continue
            if _reviewed_mixed_repartition(
                target_atlas, before, after, target_name
            ) or _reviewed_mixed_repartition(base_atlas, after, before, base_name):
                continue
            raise AtlasDeltaError(
                f"ambiguous identity overlay {before['overlay']} "
                f"text+{hx(before['offset'])} has extent "
                f"{hx(before['end_offset'])} in {base_name} and overlapping "
                f"text+{hx(after['offset'])}..{hx(after['end_offset'])} "
                f"in {target_name}"
            )

    promotions = []
    for row in target_rows:
        blockers = [item for item in base_rows if item["overlay"] == row["overlay"]]
        promotions.extend(_uncovered_exact_segments(row, blockers))
    retractions = []
    for row in base_rows:
        blockers = [item for item in target_rows if item["overlay"] == row["overlay"]]
        retractions.extend(_uncovered_exact_segments(row, blockers))
    promotions.sort(key=lambda row: (row["overlay"], row["offset"]))
    retractions.sort(key=lambda row: (row["overlay"], row["offset"]))
    promoted_bytes = sum(row["size"] for row in promotions)
    retracted_bytes = sum(row["size"] for row in retractions)
    return {
        "schema_version": 1,
        "promotions": promotions,
        "retractions": retractions,
        "totals": {
            "base_exact_c_bytes": sum(row["size"] for row in base.values()),
            "target_exact_c_bytes": sum(row["size"] for row in target.values()),
            "promotion_count": len(promotions),
            "promotion_bytes": promoted_bytes,
            "retraction_count": len(retractions),
            "retraction_bytes": retracted_bytes,
            "net_exact_c_bytes": promoted_bytes - retracted_bytes,
        },
    }


def _decode_atlas(payload, state):
    try:
        atlas = json.loads(payload)
    except (TypeError, json.JSONDecodeError) as exc:
        raise AtlasDeltaError(f"{state}: invalid atlas JSON: {exc}") from exc
    if not isinstance(atlas, dict):
        raise AtlasDeltaError(f"{state}: atlas root is not an object")
    return atlas


def load_atlas_state(spec, repo=REPO):
    """Load an atlas file/tree path or a Git tree-ish without checking it out."""
    path = Path(spec)
    if path.exists():
        atlas_path = path / MANIFEST_REPO_PATH if path.is_dir() else path
        if not atlas_path.is_file():
            raise AtlasDeltaError(f"{spec}: tree has no {MANIFEST_REPO_PATH}")
        payload = atlas_path.read_text(encoding="utf-8")
        digest = hashlib.sha256(payload.encode()).hexdigest()
        return _decode_atlas(payload, spec), {
            "input": spec,
            "kind": "tree" if path.is_dir() else "manifest",
            "resolved": f"sha256:{digest}",
        }

    try:
        tree = subprocess.run(
            ["git", "rev-parse", "--verify", f"{spec}^{{tree}}"],
            cwd=repo,
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        ).stdout.strip()
        payload = subprocess.run(
            ["git", "show", f"{spec}:{MANIFEST_REPO_PATH}"],
            cwd=repo,
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        ).stdout
    except subprocess.CalledProcessError as exc:
        detail = exc.stderr.strip() or "not a path or Git tree-ish"
        raise AtlasDeltaError(f"{spec}: {detail}") from exc
    return _decode_atlas(payload, spec), {
        "input": spec,
        "kind": "git",
        "resolved": tree,
    }


def load_worktree_atlas(path=DEFAULT_MANIFEST):
    payload = path.read_text(encoding="utf-8")
    digest = hashlib.sha256(payload.encode()).hexdigest()
    return _decode_atlas(payload, "WORKTREE"), {
        "input": "WORKTREE",
        "kind": "worktree",
        "resolved": f"sha256:{digest}",
    }


def render_exact_c_delta(delta):
    lines = [
        f"base:   {delta['base']['input']} ({delta['base']['resolved']})",
        f"target: {delta['target']['input']} ({delta['target']['resolved']})",
    ]
    for heading, marker, rows, byte_key in (
        ("promotions", "+", delta["promotions"], "promotion_bytes"),
        ("retractions", "-", delta["retractions"], "retraction_bytes"),
    ):
        lines.append(
            f"{heading}: {len(rows)} ranges, {delta['totals'][byte_key]} bytes"
        )
        for row in rows:
            lines.append(
                f"  {marker} overlay {row['overlay']:03d} "
                f"text+{hx(row['offset'])}..{hx(row['end_offset'])} "
                f"{row['size']} bytes {row['label']}"
            )
    net = delta["totals"]["net_exact_c_bytes"]
    lines.append(f"net exact C: {net:+d} bytes")
    return "\n".join(lines)


def data_rodata_ownership_rows(overlay, data_size, text_ownership):
    """Reviewed initialized ranges emitted by existing C objects."""
    rows = []
    previous_end = 0
    previous_text_index = -1
    fixed_source_sections = set()
    text_sources = [
        part["source"].rsplit("/", 1)[1]
        for part in text_ownership
        if part["type"] == "c"
    ]
    for start, end, source_name in DATA_RODATA_OWNERSHIP.get(overlay, []):
        if start != previous_end or start >= end or end > data_size:
            raise ValueError(
                f"invalid overlay {overlay} data/rodata ownership range"
            )
        try:
            text_index = text_sources.index(source_name)
        except ValueError as exc:
            raise ValueError(
                f"overlay {overlay} data/rodata owner {source_name} "
                "does not own a C text row"
            ) from exc
        if text_index <= previous_text_index:
            raise ValueError(
                f"overlay {overlay} data/rodata owners are not in text order"
            )
        rows.append(
            {
                "offset": hx(start),
                "end_offset": hx(end),
                "size": hx(end - start),
                "type": "c",
                "source": f"overlays/o{overlay:03d}/{source_name}",
            }
        )
        previous_end = end
        previous_text_index = text_index
    for fixed in FIXED_DATA_RODATA_OWNERSHIP.get(overlay, []):
        if len(fixed) not in (4, 5, 6):
            raise ValueError(
                f"invalid overlay {overlay} fixed data/rodata owner tuple"
            )
        start, end, source_name, section = fixed[:4]
        trial_function = fixed[4] if len(fixed) >= 5 else None
        externalized = fixed[5] if len(fixed) == 6 else False
        if not isinstance(externalized, bool):
            raise ValueError(
                f"invalid overlay {overlay} externalized ownership flag"
            )
        if (
            start < 0
            or start >= end
            or end > data_size
            or start % 4
            or end % 4
            or section not in (".data", ".rodata")
        ):
            raise ValueError(
                f"invalid overlay {overlay} fixed data/rodata ownership range"
            )
        if source_name not in text_sources:
            raise ValueError(
                f"overlay {overlay} fixed data/rodata owner {source_name} "
                "does not own a C text row"
            )
        source_section = (source_name, section)
        if source_section in fixed_source_sections:
            raise ValueError(
                f"overlay {overlay} repeats fixed data/rodata owner "
                f"{source_name} section {section}"
            )
        fixed_source_sections.add(source_section)
        row = {
            "offset": hx(start),
            "end_offset": hx(end),
            "size": hx(end - start),
            "type": "c",
            "section": section,
            "source": f"overlays/o{overlay:03d}/{source_name}",
        }
        if trial_function is not None:
            row["trial_function"] = trial_function
        if externalized:
            row["externalized"] = True
        rows.append(row)
    for start, end, source_name, section in CANONICAL_FIXED_DATA_RODATA_OWNERSHIP.get(
        overlay, []
    ):
        if (
            start < 0
            or start >= end
            or end > data_size
            or start % 4
            or end % 4
            or section not in (".data", ".rodata")
        ):
            raise ValueError(
                f"invalid overlay {overlay} canonical fixed data/rodata owner"
            )
        source_section = (source_name, section)
        if source_section in fixed_source_sections:
            raise ValueError(
                f"overlay {overlay} repeats fixed data/rodata owner "
                f"{source_name} section {section}"
            )
        fixed_source_sections.add(source_section)
        if source_name not in text_sources and (
            overlay, source_name, section
        ) not in DATA_ONLY_FIXED_DATA_SOURCES:
            raise ValueError(
                f"overlay {overlay} canonical data-only owner {source_name} "
                f"section {section} is not explicitly allowed"
            )
        rows.append(
            {
                "offset": hx(start),
                "end_offset": hx(end),
                "size": hx(end - start),
                "type": "c",
                "section": section,
                "source": f"overlays/o{overlay:03d}/{source_name}",
                "canonical": True,
            }
        )
    return rows


def data_rodata_ownership_totals(module_rows):
    """Separate canonical projected C storage from diagnostic declarations.

    Leading implicit .data rows and explicitly canonical fixed rows are the
    normal projection's physical owners. A trial-only declaration or an
    externalized duplicate earns no credit. Canonical fixed rows may retain a
    trial_function tag as provenance without losing their projected ownership.
    Diagnostic bytes sum declarations; they are not additional physical data.
    """
    owned = diagnostic = 0
    for module in module_rows:
        leading_end = 0
        for part in module.get("data_rodata_ownership", []):
            size = int(part["size"], 16)
            normal = part.get("type") == "c" and not part.get("externalized")
            leading = (normal and "section" not in part
                       and "trial_function" not in part
                       and int(part["offset"], 16) == leading_end)
            fixed = (normal and part.get("canonical") is True
                     and part.get("section") in (".data", ".rodata"))
            if leading:
                leading_end = int(part["end_offset"], 16)
            if leading or fixed:
                owned += size
            else:
                diagnostic += size
    return {"owned_data_rodata_bytes": owned,
            "diagnostic_data_rodata_bytes": diagnostic}


def mixed_tu_exact_c_rows(overlay, ownership):
    """Reviewed exact-C subranges inside a source-level NON_MATCHING row.

    The broad ownership row remains the real splat/build subsegment. These
    rows affect matching status only; they never emit a second copy of the C
    object into the YAML or linker layout.
    """
    rows = []
    previous_end = -1
    for exact_range in MIXED_TU_EXACT_C_RANGES.get(overlay, []):
        start, end, label = exact_range[:3]
        evidence = (
            exact_range[3]
            if len(exact_range) == 4
            else "pre-consolidation metadata-only object; linked bytes exact"
        )
        if start < previous_end or start >= end:
            raise ValueError(
                f"invalid overlay {overlay} mixed-TU exact range "
                f"{hx(start)}..{hx(end)} ({label}): MIXED_TU_EXACT_C_RANGES "
                "rows must be non-empty, sorted by offset and non-overlapping")
        containers = [
            part
            for part in ownership
            if part["type"] == "c"
            and part["nonmatching"]
            and int(part["offset"], 16) <= start
            and end <= int(part["end_offset"], 16)
        ]
        if not containers:
            # The owning TU has since become fully matched: the range is
            # credited by its plain C row and the mixed entry is obsolete.
            plain = [
                part for part in ownership
                if part["type"] == "c" and not part["nonmatching"]
                and int(part["offset"], 16) <= start
                and end <= int(part["end_offset"], 16)
            ]
            if plain:
                previous_end = end
                continue
        if len(containers) != 1:
            raise ValueError(
                f"overlay {overlay} exact range {hx(start)}..{hx(end)} "
                "is not inside exactly one mixed C ownership row"
            )
        rows.append(
            {
                "offset": hx(start),
                "end_offset": hx(end),
                "size": hx(end - start),
                "label": label,
                "source": containers[0]["source"],
                "evidence": evidence,
            }
        )
        previous_end = end
    return rows


def priority_score(module, main_inbound, cross_inbound, export_count, record_count):
    """Rank small, evidenced modules ahead of large or relocation-heavy ones.

    The score is deliberately transparent rather than statistically dressed
    up.  Evidence contributes to the numerator; every byte that must be
    understood and every relocation record contributes to the denominator.
    An exact whole-module donor match receives a finite boost, so it wins among
    small modules without making donor presence an absolute authority.
    """
    evidence = (
        20
        + main_inbound * 100
        + cross_inbound * 20
        + export_count * 10
        + (1000 if module["overlay"] in EXACT_DONOR_OVERLAYS else 0)
    )
    work = (
        module["text_size"]
        + module["data_size"]
        + module["bss_size"]
        + record_count * 8
        + 0x40
    )
    return evidence * 1_000_000 // work


def build_atlas(rom):
    count, resident_relocs = overlay_tables.read_reloc_table(rom)
    headers = overlay_tables.read_headers(rom)
    modules = overlay_tables.build_modules(headers)
    rom_table = overlay_tables.read_rom_table(rom)

    if not overlay_tables.verify(rom, count, resident_relocs, modules):
        raise ValueError("the underlying overlay-table verification failed")

    exports = collections.defaultdict(list)
    for row in rom_table:
        if 1 <= row["overlay"] <= overlay_tables.HEADER_COUNT:
            exports[row["overlay"]].append(row)

    resident_inbound = collections.defaultdict(list)
    resident_mode_census = collections.Counter()
    resident_op_census = collections.Counter()
    for row in resident_relocs:
        if row["rom_table_index"] >= len(rom_table):
            raise ValueError(
                f"resident relocation {row['index']} uses out-of-range "
                f"ROM-table index {row['rom_table_index']}"
            )
        target = rom_table[row["rom_table_index"]]
        row = dict(row)
        row["target_overlay"] = target["overlay"]
        row["target_symbol_offset"] = target["offset"]
        mode, op = row["flags"] >> 4, row["flags"] & 0xF
        resident_mode_census[mode] += 1
        resident_op_census[op] += 1
        if 1 <= target["overlay"] <= overlay_tables.HEADER_COUNT:
            resident_inbound[target["overlay"]].append(row)

    records_by_overlay = {}
    table_census = collections.Counter()
    mode_census = collections.Counter()
    op_census = collections.Counter()
    reserved_symbol_refs = collections.Counter()
    import_edges = collections.Counter()
    cross_inbound = collections.Counter()

    for module in modules:
        source = module["overlay"]
        records = overlay_tables.read_module_relocations(rom, module, rom_table)
        records_by_overlay[source] = records
        for record in records:
            table_census[record["table"]] += 1
            mode_census[record["mode"]] += 1
            op_census[record["op"]] += 1
            if record["op"] != 0:
                continue
            target = record["target_overlay"]
            if target in (0xFFD, 0xFFE, 0xFFF):
                reserved_symbol_refs[target] += 1
            if 1 <= target <= overlay_tables.HEADER_COUNT and target != source:
                import_edges[(source, target)] += 1
                cross_inbound[target] += 1

    module_rows = []
    for module in modules:
        overlay = module["overlay"]
        ranges = overlay_tables.module_section_ranges(module)
        records = records_by_overlay[overlay]
        table_counts = collections.Counter(r["table"] for r in records)
        module_modes = collections.Counter(r["mode"] for r in records)
        module_ops = collections.Counter(r["op"] for r in records)
        module_reserved = collections.Counter(
            r["target_overlay"]
            for r in records
            if r["op"] == 0 and r["target_overlay"] in (0xFFD, 0xFFE, 0xFFF)
        )
        imports = [
            {"overlay": target, "relocations": amount}
            for (source, target), amount in sorted(import_edges.items())
            if source == overlay
        ]
        export_rows = [
            {"rom_table_index": row["index"], "offset": hx(row["offset"])}
            for row in exports[overlay]
        ]
        inbound_rows = [
            {
                "relocation_index": row["index"],
                "call_site_rom": hx(row["call_site_rom"]),
                "symbol_offset": hx(row["target_symbol_offset"]),
                "flags": hx(row["flags"]),
            }
            for row in resident_inbound[overlay]
        ]
        ownership = []
        text_size = module["text_size"]
        text_parts = (
            TEXT_SUBSEGMENTS.get(overlay, [(0, "asm", f"overlay_{overlay:03d}")])
            if text_size
            else []
        )
        if text_parts and text_parts[0][0] != 0:
            raise ValueError(f"overlay {overlay} text ownership does not start at zero")
        for i, (offset, kind, source_name) in enumerate(text_parts):
            end = text_parts[i + 1][0] if i + 1 < len(text_parts) else text_size
            if kind not in ("asm", "c") or offset >= end or end > text_size:
                raise ValueError(f"invalid overlay {overlay} text ownership range")
            nonmatching = kind == "c" and is_nonmatching_source(overlay, source_name)
            ownership.append(
                {
                    "offset": hx(offset),
                    "end_offset": hx(end),
                    "size": hx(end - offset),
                    "type": kind,
                    "source": f"overlays/o{overlay:03d}/{source_name}",
                    # "matched" is C ownership of the range, not DKR-style
                    # matching: a NON_MATCHING C file still owns its range
                    # (it is not raw "asm" ownership) but is not counted as
                    # matched by tools/progress.py's scoreboard, which reads
                    # "nonmatching" below. See docs/acceleration-survey.md
                    # sec.13.
                    "matched": kind == "c",
                    "nonmatching": nonmatching,
                }
            )
        data_ownership = data_rodata_ownership_rows(
            overlay, module["data_size"], ownership
        )
        mixed_exact_c = mixed_tu_exact_c_rows(overlay, ownership)
        row = {
            "overlay": overlay,
            "identity": f"overlay:{overlay}",
            "rom": range_row(module["rom_start"], module["rom_end"]),
            "stored_vram_base": hx(module["vram_base"]),
            "synthetic_vma": hx(SYNTHETIC_VMA),
            "sections": {
                name: range_row(*section_range)
                for name, section_range in ranges.items()
            },
            "text_ownership": ownership,
            "bss_size": hx(module["bss_size"]),
            "entrypoints": {
                "init_offset": (
                    None if module["init_function"] < 0 else hx(module["init_function"])
                ),
                "resume_offset": (
                    None
                    if module["resume_function"] < 0
                    else hx(module["resume_function"])
                ),
            },
            "exports": export_rows,
            "resident_inbound": inbound_rows,
            "imports": imports,
            "cross_overlay_inbound_relocations": cross_inbound[overlay],
            "relocations": {
                "records": len(records),
                "tables": counter_dict(table_counts),
                "operations": counter_dict(module_ops, overlay_tables.RELOC_OP_NAMES),
                "patch_types": counter_dict(
                    module_modes, overlay_tables.RELOC_TYPE_NAMES
                ),
                "reserved_symbol_references": {
                    hx(k): module_reserved[k] for k in sorted(module_reserved)
                },
            },
            "campaign": {
                "priority_score": priority_score(
                    module,
                    len(resident_inbound[overlay]),
                    cross_inbound[overlay],
                    len(export_rows),
                    len(records),
                ),
                "pilot_role": PILOT_ROLES.get(overlay),
                "exact_donor_match": EXACT_DONOR_OVERLAYS.get(overlay),
            },
        }
        if mixed_exact_c:
            row["mixed_tu_exact_c_ranges"] = mixed_exact_c
        if data_ownership:
            row["data_rodata_ownership"] = data_ownership
        module_rows.append(row)

    ranked = sorted(
        (row for row in module_rows if row["rom"]["size"] != "0x0"),
        key=lambda row: (-row["campaign"]["priority_score"], row["overlay"]),
    )
    rank_by_overlay = {row["overlay"]: i + 1 for i, row in enumerate(ranked)}
    for row in module_rows:
        row["campaign"]["priority_rank"] = rank_by_overlay.get(row["overlay"])

    all_records = sum(len(records) for records in records_by_overlay.values())
    expected_records = sum(
        (m["reloc1_size"] + m["reloc2_size"]) // overlay_tables.RELOC_RECORD_SIZE
        for m in modules
    )
    if all_records != expected_records:
        raise ValueError(
            f"decoded {all_records} module relocations, expected {expected_records}"
        )
    if sum(import_edges.values()) != sum(cross_inbound.values()):
        raise ValueError("cross-overlay edge and inbound totals disagree")

    return {
        "schema_version": 1,
        "generated_by": "tools/overlay_atlas.py",
        "source": {
            "rom": "baseroms/mickey.us.z64",
            "sha1": hashlib.sha1(rom).hexdigest(),
            "header_table_rom": hx(overlay_tables.HEADER_TABLE_BASE),
            "module_region": range_row(
                overlay_tables.MODULES_BASE, overlay_tables.MODULES_END
            ),
        },
        "address_model": {
            "canonical_identity": "(overlay, section, byte offset)",
            "stored_vram_base": hx(0),
            "synthetic_vma": hx(SYNTHETIC_VMA),
            "namespace": (
                "shared exclusive_ram_id; ROM ranges and segment-qualified "
                "names distinguish modules"
            ),
            "initialized_section": "data_rodata (header does not split the two)",
        },
        "totals": {
            "modules": len(modules),
            "nonempty_modules": sum(m["rom_end"] > m["rom_start"] for m in modules),
            "text_bytes": sum(m["text_size"] for m in modules),
            "matched_overlay_c_bytes": sum(
                int(part["size"], 16)
                for row in module_rows
                for part in row["text_ownership"]
                if part["matched"] and not part["nonmatching"]
            )
            + sum(
                int(part["size"], 16)
                for row in module_rows
                for part in row.get("mixed_tu_exact_c_ranges", [])
            ),
            "nonmatching_overlay_c_bytes": sum(
                int(part["size"], 16)
                for row in module_rows
                for part in row["text_ownership"]
                if part["matched"] and part["nonmatching"]
            )
            - sum(
                int(part["size"], 16)
                for row in module_rows
                for part in row.get("mixed_tu_exact_c_ranges", [])
            ),
            "data_rodata_bytes": sum(m["data_size"] for m in modules),
            **data_rodata_ownership_totals(module_rows),
            "bss_bytes": sum(m["bss_size"] for m in modules),
            "module_rom_bytes": sum(m["rom_end"] - m["rom_start"] for m in modules),
            "resident_relocations": len(resident_relocs),
            "module_relocations": all_records,
            "rom_table_entries": len(rom_table),
            "cross_overlay_relocations": sum(import_edges.values()),
            "cross_overlay_edges": len(import_edges),
        },
        "relocation_census": {
            "module_tables": counter_dict(table_census),
            "module_operations": counter_dict(op_census, overlay_tables.RELOC_OP_NAMES),
            "module_patch_types": counter_dict(
                mode_census, overlay_tables.RELOC_TYPE_NAMES
            ),
            "reserved_symbol_references": {
                hx(k): reserved_symbol_refs[k] for k in sorted(reserved_symbol_refs)
            },
            "resident_operations": counter_dict(
                resident_op_census, overlay_tables.RELOC_OP_NAMES
            ),
            "resident_patch_types": counter_dict(
                resident_mode_census, overlay_tables.RELOC_TYPE_NAMES
            ),
        },
        "dependency_graph": [
            {"source": source, "target": target, "relocations": amount}
            for (source, target), amount in sorted(import_edges.items())
        ],
        "modules": module_rows,
    }, records_by_overlay


def render_manifest(atlas):
    # One-space indentation keeps all structure human-reviewable while staying
    # under the clean-room gate's 256 KiB tracked-text ceiling. Two spaces add
    # over 53 KiB of pure whitespace to this exhaustive 107-module artifact.
    return json.dumps(atlas, indent=1) + "\n"


TRIAL_SOURCE_ENV = "PROMOTION_TRIAL_SOURCE"
TRIAL_FUNCTION_ENV = "PROMOTION_TRIAL_FUNCTION"


def trial_sources(cli=None):
    """Translation units whose fixed data/rodata ownership a trial may carve.

    A carve is only correct while the owning TU actually emits those bytes,
    which happens only when that TU's NON_MATCHING candidate is promoted. A
    carve applied to any *other* trial leaves the range unclaimed: splat drops
    the covered bytes from the module's raw `bin`, the module shrinks, and
    every module behind it slides -- which is what made all 174 `text-differs`
    rows report the whole remainder of the overlay region as out-of-range.

    So the projection carves nothing unless the trial names the TU it is
    promoting, by `--trial-source` or by PROMOTION_TRIAL_SOURCE (the env form
    exists because the build's own `overlay_atlas.py --check` has to agree with
    the yaml the trial wrote). Function-scoped trials additionally pass
    `--trial-function` / PROMOTION_TRIAL_FUNCTION; a fixed row with a producer
    function is carved only for that definition. Values are source stems or
    bare basenames.
    """
    raw = list(cli or [])
    if not raw:
        raw = re.split(r"[,\s]+", os.environ.get(TRIAL_SOURCE_ENV, ""))
    return frozenset(
        part.rsplit("/", 1)[-1] for part in raw if part
    )


def trial_functions(cli=None):
    """C definitions whose compiler-owned data a trial may carve."""
    raw = list(cli or [])
    if not raw:
        raw = re.split(r"[,\s]+", os.environ.get(TRIAL_FUNCTION_ENV, ""))
    return frozenset(part for part in raw if part)


def compiler_text_padding(module, reviewed_padding):
    """Return the final aligned C TU's reviewed trailing alignment owner.

    The atlas keeps the padding row for accounting, while IDO's full .text
    emits its bytes. A name or zero-filled asm row alone is not evidence:
    only the separately reviewed nonexecutable identity may be absorbed.
    A split module can use the same rule when its final TU starts at a
    16-byte boundary. An unaligned TU cannot supply module-end alignment.
    """
    parts = module["text_ownership"]
    if len(parts) < 2 or parts[-2]["type"] != "c" or parts[-1]["type"] != "asm":
        return None
    code, padding = parts[-2:]
    text_size = int(module["sections"]["text"]["size"], 16)
    code_start = int(code["offset"], 16)
    code_end = int(code["end_offset"], 16)
    pad_start = int(padding["offset"], 16)
    pad_end = int(padding["end_offset"], 16)
    pad_size = int(padding["size"], 16)
    identity = (module["overlay"], padding["source"], pad_start, pad_size)
    if (
        identity not in reviewed_padding
        or code_start % 16 != 0
        or int(code["size"], 16) != code_end - code_start
        or code_end != pad_start
        or pad_end != text_size
        or pad_end - pad_start != pad_size
        or pad_size not in (4, 8, 12)
        or ((code_end + 15) & ~15) != text_size
    ):
        return None
    return padding["source"]


def render_yaml_block(
    atlas,
    trial_ownership=False,
    trial_sources=frozenset(),
    trial_functions=frozenset(),
    reviewed_padding=frozenset(),
):
    lines = [YAML_BEGIN]
    lines += [
        "  #",
        "  # Generated from the shipped headers. Do not edit module arithmetic",
        "  # here; run `gmake overlay-atlas-write` after changing the parser.",
        "  # The common synthetic VMA preserves J-type target bits. Splat's",
        "  # shared exclusive_ram_id marks every module as the same mutually",
        "  # exclusive RAM class; ROM ranges and segment-qualified names keep",
        "  # offsets distinct. Initialized bytes remain raw data_rodata except",
        "  # where the manifest records explicit compiled-C ownership evidence.",
    ]
    for row in atlas["modules"]:
        if row["rom"]["size"] == "0x0":
            lines += [
                "",
                f"  # overlay_{row['overlay']:03d} is a real zero-length header row;",
                "  # it is represented in config/overlays.us.json and has no ROM segment.",
            ]
            continue
        ov = row["overlay"]
        name = f"overlay_{ov:03d}"
        def fixed_data_matches_trial(part):
            if "section" not in part:
                return False
            if part.get("externalized"):
                # The candidate POSTPROCESS removes its duplicate section and
                # anchors references to this retained raw range.  Keeping the
                # bin row avoids dropping the bytes before the link.
                return False
            if trial_functions:
                return part.get("trial_function") in trial_functions
            return part["source"].rsplit("/", 1)[-1] in trial_sources

        owned_data = row.get("data_rodata_ownership", [])
        if any(
            part.get("canonical") is True and part.get("externalized")
            for part in owned_data
        ):
            raise ValueError(
                f"overlay {ov} canonical fixed data/rodata ownership "
                "cannot be externalized"
            )
        fixed_data = [
            part
            for part in owned_data
            if part.get("canonical") is True
            or (trial_ownership and fixed_data_matches_trial(part))
        ]
        carved = bool(fixed_data)
        lines += [
            "",
            f"  - name: {name}",
            "    type: code",
            f"    start: {row['rom']['start']}",
            f"    vram: {hx(SYNTHETIC_VMA)}",
            f"    bss_size: {row['bss_size']}",
            "    align: 0x8",
            f"    subalign: {'0x4' if carved else '0x1'}",
            f"    dir: overlays/o{ov:03d}",
            f"    exclusive_ram_id: {OVERLAY_RAM_CLASS}",
            "    symbol_name_format: $SEG_$VRAM_$ROM",
            "    subsegments:",
        ]
        text_start = int(row["sections"]["text"]["start"], 16)
        compiler_padding = compiler_text_padding(row, reviewed_padding)
        explicit_padding = EXPLICIT_TEXT_PADDING.get(ov)
        for part in row["text_ownership"]:
            source_name = part["source"].rsplit("/", 1)[1]
            if part["source"] == compiler_padding:
                # Keep the reviewed asm owner in the atlas for accounting;
                # IDO already supplies this sole TU's trailing alignment.
                continue
            if source_name == explicit_padding:
                if part["type"] != "asm" or not owned_data:
                    raise ValueError(
                        f"overlay {ov} explicit padding has invalid ownership"
                    )
                pad_start = text_start + int(part["offset"], 16)
                pad_end = text_start + int(part["end_offset"], 16)
                lines.append(
                    f"      - [{hx(pad_start)}, asm, {source_name}]"
                )
                lines.append(f"      - [{hx(pad_end)}]")
                continue
            lines.append(
                f"      - [{hx(text_start + int(part['offset'], 16))}, "
                f"{part['type']}, {source_name}]"
            )
        data_row = row["sections"]["data_rodata"]
        data_size = int(data_row["size"], 16)
        leading_data = [part for part in owned_data if "section" not in part]
        owned_end = (
            int(leading_data[-1]["end_offset"], 16) if leading_data else 0
        )
        if fixed_data:
            # Every raw slice needs its own asset name. splat writes one file
            # per `bin` row, so two rows sharing `{name}_data_rodata` write the
            # same path and the later, shorter slice wins: overlay 1 lost
            # 0x274 of its 0x2C0 data bytes that way, shrinking the module and
            # sliding every module behind it. Only the first slice keeps the
            # canonical name, so a projection with no carve is byte-identical
            # to the tracked yaml.
            emitted = 0

            def raw_slice(offset):
                nonlocal emitted
                suffix = "" if emitted == 0 else f"_{offset:x}"
                emitted += 1
                lines.append(
                    f"      - [{hx(int(data_row['start'], 16) + offset)}, "
                    f"bin, {name}_data_rodata{suffix}]"
                )

            ordered_fixed = sorted(
                fixed_data, key=lambda item: int(item["offset"], 16)
            )
            readonly_tail_start = data_size
            for part in reversed(ordered_fixed):
                if (part["section"] != ".rodata"
                        or int(part["end_offset"], 16) != readonly_tail_start):
                    break
                readonly_tail_start = int(part["offset"], 16)
            cursor = owned_end
            for part in ordered_fixed:
                fixed_start = int(part["offset"], 16)
                fixed_end = int(part["end_offset"], 16)
                if fixed_start < cursor:
                    raise ValueError(
                        f"overlay {ov} fixed data/rodata ownership overlaps "
                        "a leading C-owned range or another fixed range"
                    )
                if cursor < fixed_start:
                    raw_slice(cursor)
                start = hx(int(data_row["start"], 16) + fixed_start)
                source = part["source"].rsplit("/", 1)[1]
                if (part["section"] == ".rodata"
                        and fixed_start < readonly_tail_start):
                    # Splat normally groups every .rodata input after .data.
                    # An interior read-only contributor must keep its physical
                    # order before subsequent initialized slices. Change only
                    # ordering: the selected ELF section remains .rodata.
                    lines.append(
                        f"      - {{start: {start}, type: .rodata, name: {source}, "
                        "linker_section_order: .data}"
                    )
                else:
                    lines.append(
                        f"      - [{start}, {part['section']}, {source}]"
                    )
                cursor = fixed_end
            if cursor < data_size:
                raw_slice(cursor)
        elif owned_end < data_size:
            lines.append(
                f"      - [{hx(int(data_row['start'], 16) + owned_end)}, "
                f"bin, {name}_data_rodata]"
            )
        if ov in UNOWNED_BSS_RESERVE_OVERLAYS:
            bss_size = int(row["bss_size"], 16)
            if bss_size <= 0:
                raise ValueError(f"overlay {ov} unowned BSS reserve has no capacity")
            body_size = int(row["sections"]["text"]["size"], 16) + data_size
            lines.append(
                "      - { type: bss, vram: "
                f"{hx(SYNTHETIC_VMA + body_size)}, name: {name}_bss_reserve }}"
            )
        for section, suffix in (("reloc1", "_reloc1"), ("reloc2", "_reloc2")):
            section_row = row["sections"][section]
            if section_row["size"] == "0x0":
                continue
            if section == "data_rodata" and row.get("data_rodata_ownership"):
                continue
            # Runtime relocation records follow the loadable module image;
            # they are not the module's initialized .data section. Keeping
            # them in the code segment would order them before compiler
            # .rodata and move BSS past the serialized tables.
            lines += [
                "",
                f"  - name: {name}{suffix}",
                "    type: bin",
                f"    start: {section_row['start']}",
                "    align: 0x8",
                "    subalign: 0x1",
                f"    dir: overlays/o{ov:03d}",
            ]
    lines += ["", YAML_END]
    return "\n".join(lines)


def splice_yaml(text, block):
    start = text.find(YAML_BEGIN)
    end = text.find(YAML_END)
    if start < 0 or end < 0 or end < start:
        raise ValueError(
            f"{DEFAULT_YAML.name} lacks the generated overlay markers; "
            "add the BEGIN/END pair once before using --write"
        )
    end += len(YAML_END)
    return text[:start] + block + text[end:]


def write_if_changed(path, content):
    current = path.read_text() if path.exists() else None
    if current == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_suffix(path.suffix + ".tmp")
    temp.write_text(content)
    os.replace(temp, path)
    return True


def check_artifact(path, expected):
    if not path.exists():
        print(f"STALE missing {path.relative_to(REPO)}", file=sys.stderr)
        return False
    actual = path.read_text()
    if actual != expected:
        print(
            f"STALE {path.relative_to(REPO)}; run `gmake overlay-atlas-write`",
            file=sys.stderr,
        )
        return False
    print(f"OK    {path.relative_to(REPO)}")
    return True


def print_summary(atlas):
    totals = atlas["totals"]
    print(
        f"{totals['modules']} modules ({totals['nonempty_modules']} non-empty), "
        f"{totals['text_bytes']} text bytes, "
        f"{totals['data_rodata_bytes']} initialized bytes, "
        f"{totals['bss_bytes']} BSS bytes"
    )
    print(
        f"{totals['module_relocations']} module relocations, "
        f"{totals['cross_overlay_relocations']} cross-overlay references on "
        f"{totals['cross_overlay_edges']} directed edges, "
        f"{totals['resident_relocations']} resident relocations"
    )
    ranked = sorted(
        (r for r in atlas["modules"] if r["campaign"]["priority_rank"]),
        key=lambda r: r["campaign"]["priority_rank"],
    )[:12]
    print("top campaign candidates:")
    for row in ranked:
        print(
            f"  {row['campaign']['priority_rank']:>3}. overlay {row['overlay']:>3} "
            f"score={row['campaign']['priority_score']:<8} "
            f"text={row['sections']['text']['size']:<7} "
            f"resident={len(row['resident_inbound']):<2} "
            f"cross-in={row['cross_overlay_inbound_relocations']:<3} "
            f"exports={len(row['exports']):<2}"
        )


def main():
    parser = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--yaml", type=Path, default=DEFAULT_YAML)
    action = parser.add_mutually_exclusive_group()
    action.add_argument("--write", action="store_true")
    action.add_argument("--check", action="store_true")
    action.add_argument(
        "--trial-yaml",
        action="store_true",
        help="write only the temporary fixed-data projection used by promotion trials",
    )
    action.add_argument(
        "--trial-projection",
        action="store_true",
        help="write the temporary manifest and fixed-data YAML projection used by promotion trials",
    )
    action.add_argument(
        "--delta",
        nargs="+",
        metavar="STATE",
        help=(
            "compare exact C ownership in BASE [TARGET]; a state is an atlas "
            "JSON path, a checkout path, or a Git tree-ish, and TARGET "
            "defaults to the current worktree atlas"
        ),
    )
    action.add_argument("--overlay", type=int)
    action.add_argument("--relocations", type=int, metavar="OVERLAY")
    parser.add_argument(
        "--trial-source",
        action="append",
        metavar="STEM",
        help=(
            "translation unit being promoted by this trial; only its fixed "
            "data/rodata ownership is carved out of the raw bin (repeatable, "
            "defaults to $PROMOTION_TRIAL_SOURCE)"
        ),
    )
    parser.add_argument(
        "--trial-function",
        action="append",
        metavar="FUNCTION",
        help=(
            "C definition whose compiler-owned data this trial emits; only "
            "its fixed data/rodata ownership is carved (repeatable, defaults "
            "to $PROMOTION_TRIAL_FUNCTION)"
        ),
    )
    parser.add_argument(
        "--format",
        choices=("text", "json"),
        default="text",
        help="output format for --delta (default: text)",
    )
    args = parser.parse_args()

    if args.delta:
        if len(args.delta) not in (1, 2):
            parser.error("--delta requires BASE or BASE TARGET")
        try:
            base_atlas, base_state = load_atlas_state(args.delta[0])
            if len(args.delta) == 2:
                target_atlas, target_state = load_atlas_state(args.delta[1])
            else:
                target_atlas, target_state = load_worktree_atlas(args.manifest)
            delta = compare_exact_c_atlases(
                base_atlas,
                target_atlas,
                base_state["input"],
                target_state["input"],
            )
            delta["base"] = base_state
            delta["target"] = target_state
        except (AtlasDeltaError, OSError) as exc:
            parser.exit(2, f"error: {exc}\n")
        if args.format == "json":
            print(json.dumps(delta, indent=2, sort_keys=True))
        else:
            print(render_exact_c_delta(delta))
        return

    if args.format != "text":
        parser.error("--format is only valid with --delta")

    rom = args.rom.read_bytes()
    atlas, records_by_overlay = build_atlas(rom)
    manifest = render_manifest(atlas)
    reviewed_padding = {
        (part["overlay"], part["source"], int(part["offset"], 16), part["bytes"])
        for part in executable_accounting.reviewed_ranges(REPO, atlas, rom)
        if part["kind"] == "overlay-padding"
    }
    yaml_text = args.yaml.read_text()
    trial_ownership = args.trial_yaml or args.trial_projection or (
        os.environ.get("PROMOTION_TRIAL", "") not in ("", "0")
    )
    generated_yaml = splice_yaml(
        yaml_text,
        render_yaml_block(
            atlas,
            trial_ownership=trial_ownership,
            trial_sources=trial_sources(args.trial_source),
            trial_functions=trial_functions(args.trial_function),
            reviewed_padding=reviewed_padding,
        ),
    )

    if args.trial_yaml:
        if write_if_changed(args.yaml, generated_yaml):
            print(f"updated temporary {args.yaml.relative_to(REPO)}")
        else:
            print("temporary ownership projection current")
        return

    if args.trial_projection:
        changed = []
        if write_if_changed(args.manifest, manifest):
            changed.append(str(args.manifest.relative_to(REPO)))
        if write_if_changed(args.yaml, generated_yaml):
            changed.append(str(args.yaml.relative_to(REPO)))
        print("updated temporary " + ", ".join(changed)
              if changed else "temporary ownership projection current")
        return

    if args.write:
        changed = []
        if write_if_changed(args.manifest, manifest):
            changed.append(str(args.manifest.relative_to(REPO)))
        if write_if_changed(args.yaml, generated_yaml):
            changed.append(str(args.yaml.relative_to(REPO)))
        print("updated " + ", ".join(changed) if changed else "overlay artifacts current")
        return

    if args.check:
        ok = check_artifact(args.manifest, manifest)
        ok = check_artifact(args.yaml, generated_yaml) and ok
        if not ok:
            sys.exit(1)
        return

    if args.overlay is not None:
        if not 1 <= args.overlay <= overlay_tables.HEADER_COUNT:
            parser.error("--overlay must be in 1..107")
        print(json.dumps(atlas["modules"][args.overlay - 1], indent=2))
        return

    if args.relocations is not None:
        if not 1 <= args.relocations <= overlay_tables.HEADER_COUNT:
            parser.error("--relocations must be in 1..107")
        print(json.dumps(records_by_overlay[args.relocations], indent=2))
        return

    print_summary(atlas)


if __name__ == "__main__":
    main()
