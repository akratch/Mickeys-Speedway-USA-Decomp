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
    such a TU are credited separately through ``MIXED_TU_EXACT_C_RANGES``.
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
    22: [(0x0, 0x60, "overlay22RemoveObject")],
    45: [(0x0, 0x80, "func_overlay_045_F0000764_188CBBC")],
    47: [(0x0, 0x5A0, "overlay47ReleaseResources")],
    # Offsets are relative to data_rodata. The source must already own a C text
    # row: its compiled .data is linked before any remaining raw initialized
    # tail, so no duplicate C subsegment is emitted at the section boundary.
    15: [(0x0, 0x50, "overlay_015")],
    42: [(0x0, 0x10, "overlay_042")],
    77: [(0x0, 0x30, "overlay_077")],
    79: [(0x0, 0x60, "func_overlay_079_F0000134_18CD0D4")],
    # The whole .data section; data_rodata +0x2D0..+0x2E0 is the original
    # TU's .rodata (its own relocation base) and stays raw.
    54: [(0x0, 0x2D0, "overlay54Initialize")],
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
    7: [(0x934, 0x950, "overlay_007_tail", ".rodata", "overlay7DispatchModes")],
    8: [(0x27C, 0x2AC, "overlay_008", ".rodata", "func_overlay_008_F0004CF0_1862A48")],
    9: [(0x390, 0x3E0, "overlay_009", ".rodata", "func_overlay_009_F0000CE4_186735C", True)],
    14: [
        (0x158, 0x174, "overlay14LoadRelocatedValue", ".rodata", "overlay14LoadRelocatedValue", True),
        (0x174, 0x190, "func_overlay_014_F0001830_1871108", ".rodata", "func_overlay_014_F0001830_1871108", True),
    ],
    25: [(0x20, 0x40, "overlay_025", ".rodata", "overlay25UpdateEffect")],
    41: [
        (0x0, 0x3C, "overlay41SampleCurve", ".rodata", "func_overlay_041_F00002AC_18875E4"),
        (0x3C, 0x54, "overlay41UpdateCurveObject", ".rodata", "func_overlay_041_F0000854_1887B8C"),
    ],
    46: [(0x364, 0x378, "overlay46UpdateSequence", ".rodata", "func_overlay_046_F0000120_188E518")],
    58: [
        (
            0x3F4,
            0x428,
            "func_overlay_058_F000138C_18B0574",
            ".rodata",
            "func_overlay_058_F000138C_18B0574",
            True,
        )
    ],
    59: [(0x76C, 0x78C, "overlay59Advance", ".rodata", "overlay59Advance", True)],
    86: [
        (
            0x80,
            0xA0,
            "func_overlay_086_F0000474_18D22AC",
            ".rodata",
            "func_overlay_086_F0000474_18D22AC",
            True,
        )
    ],
}

# When a C owner's initialized input follows its text, IDO's measured .text
# alignment can emit the intervening zero padding without a separate asm row.
# Keep the row in the atlas so padding is never counted as executable C credit.
COMPILER_TEXT_ALIGNMENT_PADDING = {
    22: "overlay_022_padding",
    45: "overlay_045_padding",
    15: "overlay_015_padding",
}

# Overlay 47's final text address is already 8-byte aligned, but the ROM has
# an explicit eight-byte gap before initialized data. Emit a bounded asm row
# for that gap rather than letting the old row absorb the data block.
EXPLICIT_TEXT_PADDING = {
    47: "overlay_047_padding",
    # Overlay 54's text tail pads to 0x1EF0; overlay54Initialize's .data
    # follows it, so the pad needs its own bounded row.
    54: "overlay_054_padding",
}

TEXT_SUBSEGMENTS = {
    3: [
        (0x000, "c", "overlay3ContainsValue"),
        (0x050, "c", "overlay3ResetObjects"),
        (0x0B8, "c", "overlay3RunCachedModeAction"),
        (0x27C, "c", "overlay3FindClosestObject"),
        (0x3B0, "c", "overlay3SelectScoredObject"),
        (0x588, "c", "overlay3SelectTarget"),
        (0x6D8, "c", "overlay3TouchObject"),
        (0x760, "c", "overlay3UpdateTimedEntries"),
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
        (0x0000, "c", "overlay2ValidateRegion"),
        (0x01BC, "c", "overlay2AppendLine"),
        (0x02C4, "c", "overlay2ClassifyBoundary"),
        (0x0400, "c", "overlay2IntersectBoundary"),
        (0x049C, "c", "overlay2ClipLines"),
        (0x06E0, "c", "overlay2ChooseBoundary"),
        (0x0B70, "c", "overlay2SplitRegion"),
        (0x0C90, "c", "func_overlay_002_F0000C90_1857A88"),
        (0x121C, "c", "overlay2Enable"),
        (0x123C, "c", "overlay2ContainsPoint"),
        (0x1364, "c", "func_overlay_002_F0001364_185815C"),
        (0x1658, "c", "overlay2AdjacentIndices"),
        (0x16A0, "c", "overlay2QueryNode"),
        (0x1A94, "c", "func_overlay_002_F0001A94_185888C"),
        (0x1DF8, "c", "func_overlay_002_F0001DF8_1858BF0"),
        (0x2528, "c", "overlay2CopyColor"),
        (0x2548, "asm", "overlay_002_tail"),
    ],
    5: [
        (0x000, "c", "alSeqFileNew"),
        (0x040, "c", "alBnkfNew"),
        (0x0D0, "c", "_bnkfPatchBank"),
        (0x188, "c", "_bnkfPatchInst"),
        (0x220, "c", "_bnkfPatchSound"),
        (0x278, "c", "_bnkfPatchWaveTable"),
        (0x2E4, "c", "overlay_005"),
        (0x764, "asm", "overlay_005_padding"),
    ],
    6: [
        (0x000, "c", "overlay_006"),
        (0x01C, "asm", "overlay_006_padding"),
    ],
    7: [
        (0x000, "c", "overlay_007"),
        (0x324, "c", "func_overlay_007_F0000324_185C1AC"),
        (0x894, "c", "overlay_007_tail"),
        (0xFB8, "asm", "overlay_007_padding"),
    ],
    8: [
        (0x0000, "c", "overlay_008"),
        (0x5128, "asm", "overlay_008_padding"),
    ],
    9: [
        (0x0000, "c", "overlay_009"),
    ],
    10: [
        (0x000, "c", "overlay10Initialize"),
    ],
    12: [
        (0x000, "c", "overlay_012"),
        (0x0C4, "c", "func_overlay_012_F00000C4_186D344"),
        (0x1B4, "c", "overlay_012_tail"),
        (0x2E4, "c", "func_overlay_012_F00002E4_186D564"),
        (0x3A8, "c", "func_overlay_012_F00003A8_186D628"),
        (0x910, "c", "func_overlay_012_F0000910_186DB90"),
    ],
    13: [
        (0x000, "c", "overlay13Initialize"),
        (0x124, "c", "overlay13Release"),
        (0x188, "c", "overlay13CreateRecord"),
        (0x284, "c", "overlay13ProcessRecord"),
        (0x508, "c", "overlay13ProcessActive"),
        (0x580, "c", "overlay13DrawRecord"),
        (0x874, "c", "overlay13DrawActive"),
        (0xB0C, "c", "overlay13Call"),
        (0xB2C, "asm", "overlay_013_padding"),
    ],
    14: [
        (0x000, "c", "func_overlay_014_F0000000_186F8D8"),
        (0x13C, "c", "func_overlay_014_F000013C_186FA14"),
        (0x31C, "c", "overlay14ReturnOne"),
        (0x328, "c", "overlay14ApplyValues"),
        (0x498, "c", "overlay14ResetMode"),
        (0x578, "c", "overlay14MoveCommandCursor"),
        (0x6FC, "c", "overlay14CreateValue"),
        (0x87C, "c", "overlay14LoadRelocatedValue"),
        (0x9F4, "c", "func_overlay_014_F00009F4_18702CC"),
        (0xACC, "c", "overlay14ResetFlags"),
        (0xAE0, "c", "overlay14GetFlagC4"),
        (0xAEC, "c", "overlay14GetFlagC8"),
        (0xAF8, "c", "overlay14ReleaseCurrent"),
        (0xB34, "c", "overlay14GetFlagCC"),
        (0xB40, "c", "overlay14Reset"),
        (0xB5C, "c", "overlay14PrepareInputState"),
        (0xD68, "c", "overlay14AdvanceCommand"),
        (0xF64, "c", "overlay14StepCommand"),
        (0x1028, "c", "overlay14ReturnOneCallbacks"),
        (0x1040, "c", "overlay14DispatchCommand"),
        (0x1164, "c", "overlay14CallUpdate"),
        (0x1184, "c", "overlay14UpdateTransition"),
        (0x12D8, "c", "overlay14BuildRects"),
        (0x13F4, "c", "func_overlay_014_F00013F4_1870CCC"),
        (0x1540, "c", "func_overlay_014_F0001540_1870E18"),
        (0x1830, "c", "func_overlay_014_F0001830_1871108"),
        (0x1B54, "c", "overlay14ReleaseOwner"),
        (0x1B7C, "c", "overlay14FinalizeActiveHandle"),
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
        (0x000, "c", "overlay18Load"),
        (0x1F4, "c", "overlay18Initialize"),
        (0x24C, "c", "overlay18Reconfigure"),
        (0x4F4, "c", "overlay18InitializeBuffers"),
    ],
    19: [
        (0x000, "c", "overlay19Dispatch"),
        (0x0AC, "c", "overlay19BuildOutput"),
        (0x1E0, "c", "overlay19BuildPlanes"),
        (0xA30, "c", "overlay19BuildAdjacency"),
        (0xC1C, "c", "overlay19FindAdjacent"),
        (0xD78, "c", "overlay19ClassifyEdge"),
        (0xF58, "c", "overlay19BuildSpatialMasks"),
        (0x12E4, "asm", "overlay_019_padding"),
    ],
    20: [
        (0x000, "c", "overlay20ReleaseTree"),
        (0x07C, "c", "overlay20ReleaseHandle"),
        (0x0A8, "c", "overlay20ConfigureResource"),
        (0x204, "c", "overlay20UpdateObjectResource"),
        (0x38C, "c", "func_overlay_020_F000038C_1876964"),
        (0x07C4, "c", "overlay20BuildTileCommands"),
        (0x09DC, "c", "overlay20DrawResource"),
        (0x0A68, "c", "overlay20UpdateGrid"),
        (0x0DC4, "c", "overlay20ReleaseEntry"),
        (0x0E0C, "c", "overlay20MarkNested"),
        (0x0E28, "c", "overlay20ConfigureEntry"),
        (0x0F78, "c", "overlay20CreateEntry"),
        (0x1018, "c", "overlay20RemoveEntry"),
        (0x10EC, "c", "overlay20AdvanceEntries"),
        (0x1148, "c", "func_overlay_020_F0001148_1877720"),
        (0x1484, "asm", "overlay_020_padding"),
    ],
    22: [
        (0x000, "c", "overlay22InitializeObject"),
        (0x2B0, "c", "func_overlay_022_F00002B0_18783B8"),
        (0xA7C, "c", "overlay22ResolvePlane"),
        (0xD30, "c", "overlay22RemoveObject"),
        (0xE9C, "asm", "overlay_022_padding"),
    ],
    31: [
        (0x000, "c", "overlay31BuildLookupTables"),
        (0x2E8, "c", "overlay31InitializeParticleAssets"),
        (0x4F8, "c", "overlay31BuildPalettes"),
        (0x6B0, "c", "overlay31InitializeBuffers"),
        (0xA84, "c", "overlay31CreateConfig"),
        (0xDC4, "c", "overlay31CreateRecords"),
        (0xE7C, "c", "overlay31CreatePool"),
        (0xF44, "asm", "overlay_031_padding"),
    ],
    33: [
        (0x000, "c", "overlay33InitializeBuffers"),
        (0x144, "c", "overlay33ReleaseGlobal"),
        (0x17C, "c", "overlay33CallA"),
        (0x19C, "c", "overlay33BuildDisplayList"),
        (0x66C, "c", "overlay33PresentAndSwap"),
        (0x708, "c", "overlay33CallB"),
        (0x728, "asm", "overlay_033_padding"),
    ],
    36: [
        (0x000, "c", "overlay36InitObject"),
        (0x0D4, "c", "overlay36QueueAction"),
        (0x140, "c", "overlay36FlushQueue"),
        (0x1D0, "c", "overlay36UpdateInteractiveEntity"),
        (0x694, "c", "overlay36SpawnTransient"),
        (0x7B0, "c", "overlay36InitVectorState"),
        (0x818, "c", "overlay36CheckNearbyHeight"),
        (0x914, "c", "overlay36SelectState"),
        (0x988, "c", "overlay36CallGlobal"),
        (0x9B8, "c", "overlay36SpawnAtPosition"),
        (0xA60, "c", "overlay36ChooseWeightedState"),
        (0xD08, "c", "overlay36PrepareAndTick"),
        (0xD8C, "c", "overlay36SpawnLinked7F"),
        (0xF20, "c", "overlay36SpawnDirectional"),
        (0x1084, "c", "overlay36SpawnConditional"),
        (0x1214, "c", "overlay36SpawnOffsetA9"),
        (0x1378, "c", "overlay36SpawnAndUpdate"),
        (0x1470, "c", "overlay36CallModes"),
        (0x14B0, "c", "overlay36TickState"),
        (0x150C, "c", "overlay36UpdatePeers"),
        (0x1688, "c", "overlay36SpawnFinalEffect"),
        (0x1748, "asm", "overlay_036_padding"),
    ],
    38: [
        (0x000, "c", "func_overlay_038_F0000000_1885D10"),
        (0x154, "c", "overlay38UpdateParticles"),
        (0x47C, "c", "func_overlay_038_F000047C_188618C"),
        (0x7E8, "asm", "overlay_038_padding"),
    ],
    41: [
        (0x0000, "c", "overlay41AdvanceStepRecords"),
        (0x0124, "c", "overlay41UpdateColorRecords"),
        (0x02AC, "c", "overlay41SampleCurve"),
        (0x07FC, "c", "overlay41InterpolateAngle"),
        (0x0854, "c", "overlay41UpdateCurveObject"),
        (0x124C, "c", "overlay41IsUnitScale"),
        (0x1298, "c", "overlay41UpdateProgress"),
        (0x1464, "c", "overlay41ProcessEntry"),
        (0x1650, "c", "overlay41AddSlot"),
        (0x172C, "c", "overlay41Ignore"),
        (0x1740, "c", "overlay41SpawnItems"),
        (0x195C, "c", "overlay41EnqueueTransition"),
        (0x1B00, "c", "overlay41TickTransitions"),
        (0x1C84, "c", "overlay41DrawItem"),
    ],
    21: [
        (0x000, "c", "overlay21RegisterPlane"),
        (0x10C, "c", "overlay21ApplyPriorities"),
        (0x2D4, "asm", "overlay_021_padding"),
    ],
    23: [
        (0x000, "c", "overlay23SpawnAttachments"),
        (0x208, "c", "overlay23Init"),
        (0x350, "c", "overlay23Update"),
        (0x468, "c", "overlay23RenderEffect"),
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
        (0x000, "c", "overlay34InitStorage"),
        (0x0C8, "c", "overlay34SetValue10"),
        (0x0D4, "c", "overlay34CreateRecord"),
        (0x2C8, "c", "overlay34RemoveRecord"),
        (0x378, "c", "overlay34ResetStorage"),
        (0x40C, "c", "overlay34UpdateRecords"),
        (0x540, "c", "overlay34InterpolateColor"),
        (0x608, "c", "overlay34SortAndDraw"),
    ],
    27: [
        (0x000, "c", "overlay_027"),
    ],
    37: [
        (0x000, "c", "overlay37Init"),
        (0x088, "c", "overlay37Update"),
        (0x19C, "c", "overlay37Render"),
        (0x4F4, "c", "overlay37RecordMinimum"),
        (0x544, "c", "overlay37RecordActive"),
        (0x558, "asm", "overlay_037_padding"),
    ],
    39: [
        (0x000, "c", "overlay_039"),
        (0x168, "asm", "overlay_039_padding"),
    ],
    40: [
        (0x000, "c", "overlay40AddEntry"),
        (0x084, "c", "overlay40RemoveEntry"),
        (0x0E8, "c", "overlay40UpdateEntries"),
        (0x1A0, "c", "overlay40BuildFrame"),
        (0x2E4, "c", "overlay40DrawEntries"),
        (0x448, "c", "overlay40SetValues"),
        (0x490, "c", "overlay40Interpolate"),
        (0x534, "c", "overlay40DrawTintRectangle"),
        (0x690, "c", "overlay40FadeRecords"),
        (0x824, "asm", "overlay_040_padding"),
    ],
    42: [
        (0x000, "c", "overlay_042"),
    ],
    43: [
        (0x0000, "c", "overlay43InitializeState"),
        (0x0194, "c", "overlay43FlushPending"),
        (0x0280, "c", "overlay43ReleaseResources"),
        (0x0324, "c", "func_overlay_043_F0000324_188A2F4"),
        (0x0BE4, "c", "func_overlay_043_F0000BE4_188ABB4"),
        (0x10A8, "c", "overlay43ComputeMotion"),
        (0x1184, "c", "overlay43AllocateResources"),
        (0x1264, "c", "overlay43SubmitChildren"),
        (0x1378, "c", "overlay43FilterImage"),
        (0x1424, "asm", "overlay_043_padding"),
    ],
    44: [
        (0x000, "c", "overlay44CreateAnimationState"),
        (0x224, "c", "overlay44ReleaseHandles"),
        (0x294, "c", "overlay44UpdateFrameCache"),
        (0x580, "c", "func_overlay_044_F0000580_188BDE0"),
        (0xAF4, "asm", "overlay_044_padding"),
    ],
    45: [
        (0x000, "c", "overlay_045"),
        (0x764, "c", "func_overlay_045_F0000764_188CBBC"),
        (0x1158, "c", "func_overlay_045_F0001158_188D5B0"),
        (0x1BE0, "c", "overlay_045_tail"),
        (0x1C1C, "asm", "overlay_045_padding"),
    ],
    47: [
        (0x0000, "c", "func_overlay_047_F0000000_1890E18"),
        (0x09D0, "c", "overlay47ReleaseResources"),
        (0x0B30, "c", "func_overlay_047_F0000B30_1891948"),
        (0x2D10, "c", "overlay47SpawnObject"),
        (0x2DE8, "asm", "overlay_047_padding"),
    ],
    49: [
        (0x000, "c", "overlay_049"),
        (0x374, "asm", "overlay_049_padding"),
    ],
    56: [
        (0x000, "c", "overlay_056"),
        (0xAF4, "asm", "overlay_056_padding"),
    ],
    63: [
        (0x000, "c", "overlay63Initialize"),
        (0x1D4, "c", "overlay63UpdateEffects"),
        (0x74C, "c", "overlay63Release"),
        (0x77C, "c", "overlay63UpdateSequence"),
        (0x928, "asm", "overlay_063_padding"),
    ],
    64: [
        (0x000, "c", "overlay64GenerateTexture"),
    ],
    65: [
        (0x000, "c", "overlay65Initialize"),
        (0x080, "c", "overlay65UpdateParticles"),
        (0xBC0, "c", "overlay65Release"),
        (0xBF0, "c", "overlay65ResetSlots"),
        (0xC38, "c", "func_overlay_065_F0000C38_18C4EA0"),
        (0x1A14, "c", "overlay65SpawnRecord"),
        (0x1BB4, "asm", "overlay_065_padding"),
    ],
    61: [
        (0x000, "c", "overlay61UpdateInput"),
        (0x1C0, "c", "overlay61ResetCounters"),
        (0x1DC, "c", "overlay61AddEntry"),
        (0x3C0, "c", "overlay61DrawEntry"),
        (0x7C4, "c", "overlay61DrawList"),
        (0x968, "c", "overlay61InitResources"),
        (0xB84, "c", "func_overlay_061_F0000B84_18BFF4C"),
        (0x1578, "c", "overlay61ReleaseResources"),
        (0x1648, "c", "func_overlay_061_F0001648_18C0A10"),
        (0x17B8, "c", "overlay61WriteCharacter"),
        (0x18A0, "c", "overlay61ReadCharacter"),
        (0x19B0, "c", "overlay61ChooseFileExtension"),
        (0x1A6C, "c", "overlay61RecordSize"),
        (0x1A84, "asm", "overlay_061_padding"),
    ],
    66: [
        (0x000, "c", "overlay66Select"),
        (0x034, "c", "overlay66GetCurrent"),
        (0x040, "c", "overlay66SmoothAndDraw"),
        (0x4E0, "c", "func_overlay_066_F00004E0_18C6948"),
    ],
    67: [
        (0x000, "c", "overlay_067"),
        (0x14C, "asm", "overlay_067_padding"),
    ],
    68: [
        (0x000, "c", "overlay68PayloadLimit"),
        (0x008, "c", "overlay68CreateEntries"),
        (0x0E0, "c", "overlay68ReleasePrimary"),
        (0x114, "c", "overlay68ReleaseSecondary"),
        (0x148, "c", "overlay68CreatePayload"),
        (0x21C, "c", "overlay68AttachObject"),
        (0x2E0, "c", "overlay68UpdateTrail"),
        (0x484, "c", "overlay68ClearNestedFlag"),
        (0x4B0, "c", "overlay68FinishEntry"),
        (0x4E4, "c", "overlay68StartTimer"),
        (0x51C, "c", "overlay68PromoteSecondary"),
        (0x650, "c", "overlay68Interpolate"),
        (0x8E0, "c", "overlay68InitializeObject"),
        (0x96C, "c", "overlay68UpdateAnimation"),
        (0xEFC, "c", "overlay68DrawSortedEntries"),
        (0x1250, "c", "overlay68RebuildSecondaryEntry"),
        (0x1438, "c", "overlay68ReleaseTertiary"),
        (0x146C, "c", "overlay68CheckKind"),
        (0x15AC, "asm", "overlay_068_padding"),
    ],
    69: [
        (0x000, "c", "overlay69Init"),
        (0x04C, "c", "overlay69UpdateAnchor"),
        (0x170, "c", "overlay69DrawSortedGeometry"),
        (0x70C, "asm", "overlay_069_padding"),
    ],
    70: [
        (0x000, "c", "func_overlay_070_F0000000_18C91C8"),
        (0x0D8, "c", "func_overlay_070_F00000D8_18C92A0"),
        (0x384, "c", "func_overlay_070_F0000384_18C954C"),
        (0x728, "asm", "overlay_070_padding"),
    ],
    71: [
        (0x000, "c", "func_overlay_071_F0000000_18C9B20"),
        (0x278, "c", "func_overlay_071_F0000278_18C9D98"),
        (0x7A8, "c", "overlay71UpdateCoordinates"),
        (0x870, "c", "func_overlay_071_F0000870_18CA390"),
        (0xB48, "asm", "overlay_071_tail"),
    ],
    72: [
        (0x000, "c", "overlay_072"),
        (0x168, "asm", "overlay_072_padding"),
    ],
    73: [
        (0x000, "c", "overlay73Initialize"),
        (0x190, "c", "func_overlay_073_F0000190_18CAC50"),
        (0xD70, "c", "overlay73Draw"),
        (0xEA8, "asm", "overlay_073_padding"),
    ],
    74: [
        (0x000, "c", "overlay74Init"),
        (0x0B8, "c", "overlay74Update"),
        (0x248, "asm", "overlay_074_padding"),
    ],
    75: [
        (0x000, "c", "overlay75Init"),
        (0x214, "c", "overlay75UpdateMovingObject"),
        (0x6D4, "c", "overlay75MarkSlot"),
        (0x6F8, "asm", "overlay_075_padding"),
    ],
    76: [
        (0x000, "c", "overlay_076"),
        (0x114, "asm", "overlay_076_padding"),
    ],
    77: [
        (0x000, "c", "overlay_077"),
        (0x3B8, "c", "overlay_077_tail"),
    ],
    78: [
        (0x000, "c", "overlay_078"),
        (0x0A8, "asm", "overlay_078_padding"),
    ],
    79: [
        (0x0000, "c", "func_overlay_079_F0000000_18CCFA0"),
        (0x0134, "c", "func_overlay_079_F0000134_18CD0D4"),
        (0x0EFC, "c", "overlay79FindNearby"),
        (0x0FA0, "c", "func_overlay_079_F0000FA0_18CDF40"),
        (0x1280, "c", "overlay79SetLink"),
        (0x1290, "c", "func_overlay_079_F0001290_18CE230"),
        (0x147C, "c", "overlay79InitState"),
        (0x149C, "c", "overlay79UpdateTimers"),
    ],
    80: [
        (0x000, "c", "overlay80InitializeContact"),
        (0x11C, "c", "overlay80UpdateContact"),
        (0x3EC, "asm", "overlay_080_padding"),
    ],
    81: [
        (0x000, "c", "overlay_081"),
        (0x34C, "asm", "overlay_081_padding"),
    ],
    82: [
        (0x000, "c", "overlay_082"),
        (0x040, "c", "overlay_082_tail"),
        (0x4CC, "asm", "overlay_082_padding"),
    ],
    83: [
        (0x000, "c", "overlay83BuildLine"),
        (0x1AC, "c", "overlay83DrawLines"),
        (0x2A0, "c", "overlay83Update"),
        (0x514, "c", "overlay83Submit"),
        (0x53C, "c", "overlay83BuildBatch"),
        (0x7DC, "c", "overlay83DrawEntries"),
        (0x850, "c", "overlay83DrawStrip"),
        (0x984, "c", "overlay83Dispatch"),
        (0xA18, "c", "overlay83SubmitAll"),
    ],
    84: [
        (0x0000, "c", "overlay84InitState"),
        (0x0048, "c", "overlay84InitializeAndUpdate"),
        (0x0314, "c", "func_overlay_084_F0000314_18D07F4"),
        (0x0A54, "c", "overlay84UpdateResource"),
        (0x0AFC, "c", "overlay84ResetCurrent"),
        (0x0B7C, "c", "overlay84RefreshCurrent"),
        (0x0C74, "c", "overlay84GetActive"),
        (0x0C9C, "c", "overlay84LoadCurrent"),
        (0x0DBC, "c", "overlay84CopyPair"),
        (0x0DD0, "c", "overlay84AdvanceCurrent"),
        (0x0F18, "c", "overlay84SelectCurrent"),
        (0x0FC4, "c", "overlay84GetCurrent"),
        (0x1004, "c", "overlay84SetBit"),
        (0x1034, "c", "overlay84GetValues"),
        (0x1060, "c", "overlay84ActivateCurrent"),
        (0x11F4, "c", "overlay84InitializeCurrent"),
        (0x1294, "c", "overlay84ClearActive"),
        (0x12B4, "c", "overlay84IsUnitScale"),
        (0x12FC, "c", "overlay84GetEnabledCurrent"),
        (0x1350, "c", "overlay84ClearMode"),
        (0x1370, "c", "overlay84SetAngle"),
        (0x1398, "c", "overlay84Mark"),
        (0x13BC, "asm", "overlay_084_padding"),
    ],
    85: [
        (0x000, "c", "overlay_085"),
        (0x29C, "asm", "overlay_085_padding"),
    ],
    86: [
        (0x000, "c", "overlay86ProcessCurrent"),
        (0x07C, "c", "overlay86ScaledVectorPosition"),
        (0x158, "c", "overlay86BuildTransform"),
        (0x2E4, "c", "overlay86SelectPosition"),
        (0x444, "c", "overlay86Init"),
        (0x474, "c", "func_overlay_086_F0000474_18D22AC"),
        (0xECC, "asm", "overlay_086_padding"),
    ],
    88: [
        (0x000, "c", "overlay88Init"),
        (0x04C, "c", "overlay88UpdateAnchor"),
        (0x1A4, "c", "overlay88DrawSortedGeometry"),
    ],
    89: [
        (0x000, "c", "overlay89UpdateEffect"),
        (0x138, "c", "overlay89Evaluate"),
        (0x1A8, "c", "overlay89Update"),
        (0x270, "c", "overlay89InitializeEffect"),
        (0x5A4, "c", "overlay89UpdateStateAndParticles"),
        (0x7C4, "asm", "overlay_089_padding"),
    ],
    90: [
        (0x000, "c", "overlay_090"),
        (0xB1C, "asm", "overlay_090_padding"),
    ],
    92: [
        (0x000, "c", "overlay92Init"),
        (0x068, "c", "overlay92FindNearestCourse"),
        (0x308, "c", "func_overlay_092_F0000308_18D6228"),
    ],
    93: [
        (0x000, "c", "overlay_093"),
        (0x0EC, "asm", "overlay_093_padding"),
    ],
    94: [
        (0x000, "c", "overlay94InitializeController"),
        (0x110, "c", "overlay94UpdateController"),
        (0x55C, "c", "overlay94SetValue"),
        (0x568, "asm", "overlay_094_padding"),
    ],
    95: [
        (0x000, "c", "overlay_095"),
        (0x1D8, "asm", "overlay_095_padding"),
    ],
    96: [
        (0x000, "c", "overlay96Register"),
        (0x070, "c", "overlay96Unregister"),
        (0x0F8, "c", "overlay96BuildVolume"),
        (0x4BC, "c", "overlay96FindVolume"),
        (0x57C, "c", "overlay96TestBit"),
        (0x5C8, "c", "overlay96DrawObject"),
        (0x6D4, "asm", "overlay_096_padding"),
    ],
    97: [
        (0x000, "c", "overlay97InitDirection"),
        (0x130, "c", "overlay97CopyAngles"),
        (0x14C, "c", "overlay97InitTransform"),
        (0x1A8, "c", "overlay97InitRadius"),
        (0x1E8, "c", "overlay97InitResource"),
        (0x36C, "c", "overlay97InitSelection"),
        (0x3F4, "c", "overlay97AssignState"),
        (0x420, "c", "overlay97CreateDescriptor"),
        (0x508, "c", "overlay97InitScale"),
        (0x748, "c", "overlay97InitBounds"),
        (0x944, "c", "overlay97InitPlane"),
        (0xA54, "asm", "overlay_097_padding"),
    ],
    98: [
        (0x000, "c", "overlay98CollectUniqueY"),
        (0x144, "c", "overlay98CollectAccepted"),
        (0x234, "c", "overlay98RenderReflections"),
        (0x848, "c", "overlay98CheckObject"),
        (0xA04, "asm", "overlay_098_padding"),
    ],
    99: [
        (0x000, "c", "overlay99GetEntries"),
        (0x00C, "c", "overlay99ReleaseEntries"),
        (0x064, "c", "overlay99InitializeEntries"),
        (0x21C, "c", "overlay99ProjectVector"),
        (0x2A0, "c", "overlay99ApplySegment"),
        (0x638, "c", "overlay99BuildHeightGrid"),
        (0x800, "c", "overlay99RenderSortedEntries"),
        (0xBA4, "c", "overlay99RenderSegments"),
        (0xDDC, "c", "func_overlay_099_F0000DDC_18DA38C"),
    ],
    100: [
        (0x000, "c", "overlay100InitializeMotion"),
        (0x214, "c", "overlay100ReleaseAll"),
        (0x278, "c", "overlay100RemoveEntry"),
        (0x318, "c", "overlay100ApplyValue"),
        (0x38C, "c", "overlay100UpdateMotion"),
        (0x50C, "c", "overlay100ApplyToValue"),
        (0x580, "c", "overlay100DrawMotion"),
        (0x94C, "asm", "overlay_100_padding"),
    ],
    17: [
        (0x000, "c", "overlay17CalculateEndpoints"),
        (0x318, "c", "overlay17CreateChain"),
        (0x628, "c", "overlay17ReleaseChain"),
        (0x668, "c", "overlay17AdvanceChain"),
        (0x8B4, "c", "overlay17DrawStrip"),
    ],
    55: [
        (0x000, "c", "overlay55Initialize"),
        (0x13C, "c", "overlay55PatchIndices"),
        (0x18C, "c", "overlay55CopyOffsetRecords"),
        (0x274, "c", "overlay55GetOffsets"),
        (0x31C, "c", "func_overlay_055_F000031C_18A1E34"),
        (0xC30, "c", "overlay55ReleaseAll"),
        (0xC68, "asm", "overlay_055_padding"),
    ],
    50: [
        (0x000, "c", "overlay50Initialize"),
        (0x2E4, "c", "overlay50PatchIndices"),
        (0x334, "c", "func_overlay_050_F0000334_1896CA4"),
        (0x1BD0, "c", "overlay50Cleanup"),
        (0x1C54, "c", "overlay50SubmitTimeGlyphs"),
        (0x1E68, "asm", "overlay_050_padding"),
    ],
    51: [
        (0x000, "c", "overlay_051"),
        (0x8AC, "asm", "overlay_051_padding"),
    ],
    52: [
        (0x000, "c", "overlay52Initialize"),
        (0x4F0, "c", "overlay52PatchIndices"),
        (0x540, "c", "overlay52CopyOffsetEntries"),
        (0x63C, "c", "overlay52TailB"),
        (0x2098, "c", "overlay52Cleanup"),
        (0x2128, "asm", "overlay_052_padding"),
    ],
    53: [
        (0x000, "c", "overlay53Initialize"),
        (0x11C, "c", "overlay53PatchIndices"),
        (0x16C, "c", "overlay53CopyOffsetEntries"),
        (0x240, "c", "func_overlay_053_F0000240_189DBE8"),
        (0xC30, "c", "overlay53ReleaseResources"),
        (0xCA8, "asm", "overlay_053_padding"),
    ],
    54: [
        (0x000, "c", "overlay54Initialize"),
        (0x3CC, "c", "overlay54PatchIndices"),
        (0x41C, "c", "overlay54CopyOffsetRecords"),
        (0x504, "c", "overlay54GetOffsets"),
        (0x5AC, "c", "overlay54TailA"),
        (0x1E94, "c", "overlay54ReleaseResources"),
        (0x1EE4, "asm", "overlay_054_padding"),
    ],
    57: [
        (0x000, "c", "func_overlay_057_F0000000_18A3BF8"),
        (0x954, "c", "overlay57UpdateInterface"),
        (0x1020, "c", "func_overlay_057_F0001020_18A4C18"),
        (0x1978, "c", "overlay57ReleaseAll"),
        (0x1AE8, "c", "func_overlay_057_F0001AE8_18A56E0"),
        (0x28B4, "c", "overlay57EaseAndLatch"),
        (0x2C28, "c", "overlay57SmoothAndCheckDistance"),
        (0x2F48, "c", "overlay57CheckDistance"),
        (0x3048, "c", "overlay57UpdateTransition"),
        (0x3238, "c", "overlay57ApplyValue"),
        (0x32A0, "c", "overlay57Draw32A0"),
        (0x35E0, "c", "overlay57UpdateSelection"),
        (0x3A4C, "c", "overlay57UpdateModeState"),
        (0x3FD4, "c", "overlay57BeginMode"),
        (0x4064, "c", "overlay57HandleModeInput"),
        (0x43C8, "c", "overlay57StartMode"),
        (0x4460, "c", "func_overlay_057_F0004460_18A8058"),
        (0x4C18, "c", "overlay57UpdateModeTrigger"),
        (0x4D90, "c", "overlay57InitializeMode"),
        (0x4E18, "c", "func_overlay_057_F0004E18_18A8A10"),
        (0x60F8, "c", "func_overlay_057_F00060F8_18A9CF0"),
        (0x67DC, "c", "overlay57SetNodeValue"),
        (0x6878, "c", "overlay57ApplyTable"),
        (0x69B0, "c", "overlay57UpdateNode"),
    ],
    58: [
        (0x000, "c", "func_overlay_058_F0000000_18AF1E8"),
        (0x5C0, "c", "overlay58ReleaseResources"),
        (0x5FC, "c", "func_overlay_058_F00005FC_18AF7E4"),
        (0x12F0, "c", "overlay58SetNodeValue"),
        (0x138C, "c", "func_overlay_058_F000138C_18B0574"),
        (0x4C04, "c", "overlay58DrawSegmentStrip"),
        (0x4F28, "c", "overlay58DrawPointQuad"),
        (0x50C8, "c", "overlay58DrawLargePointQuad"),
        (0x5268, "c", "overlay58RefreshRankSet"),
        (0x54C8, "c", "overlay58EnsureResource"),
        (0x5554, "c", "overlay58FinalizePackedStatus"),
        (0x5A14, "asm", "overlay_058_padding_b"),
    ],
    62: [
        (0x000, "c", "overlay62Initialize"),
        (0x0D4, "c", "overlay62Update"),
        (0x56C, "c", "overlay62ReleaseAll"),
    ],
    87: [
        (0x000, "c", "overlay87InitializeObject"),
        (0x128, "c", "func_overlay_087_F0000128_18D3090"),
        (0x890, "c", "overlay87ReleaseCurrent"),
        (0x8C0, "c", "overlay87HasNearby"),
        (0x964, "asm", "overlay_087_padding"),
    ],
    91: [
        (0x000, "c", "overlay_091"),
        (0x04C, "c", "overlay_091_mul"),
        (0x574, "asm", "overlay_091_padding"),
    ],
    11: [
        (0x000, "c", "overlay11Initialize"),
        (0x150, "c", "func_overlay_011_F0000150_1868998"),
        (0xA18, "c", "overlay11CreateHandles"),
        (0xAF4, "c", "overlay11InitializeFour"),
        (0xC88, "c", "overlay11InitializeSixB"),
        (0xD70, "c", "overlay11InitializeThreeA"),
        (0xDFC, "c", "overlay11InitializeSixA"),
        (0xEE4, "c", "overlay11InitializeSixC"),
        (0xFCC, "c", "overlay11InitializeThreeB"),
        (0x1058, "c", "overlay11EnableHandles"),
        (0x1130, "c", "overlay11DisableHandles"),
        (0x11D0, "c", "overlay11UpdateSelection"),
        (0x1398, "c", "overlay11UpdateMenu"),
        (0x184C, "c", "overlay11UpdateTwoOptionMenu"),
        (0x1A7C, "c", "overlay11UpdateFiveOptionMenu"),
        (0x1E4C, "c", "func_overlay_011_F0001E4C_186A694"),
        (0x22E8, "c", "func_overlay_011_F00022E8_186AB30"),
        (0x2714, "c", "overlay11UpdateModeSix"),
        (0x2948, "c", "overlay11ReleaseGroup4"),
        (0x29AC, "c", "overlay11ReleaseGroup3A"),
        (0x2A10, "c", "overlay11ReleaseGroup6A"),
        (0x2A74, "c", "overlay11ReleaseGroup6B"),
        (0x2AD8, "c", "overlay11ReleaseGroup6C"),
        (0x2B3C, "c", "overlay11ReleaseGroup3B"),
        (0x2BA0, "c", "overlay11ReleaseHandles"),
        (0x2BF4, "c", "overlay11ReleaseCurrentGroup"),
        (0x2CB4, "asm", "overlay_011_padding"),
    ],
    29: [
        (0x000, "c", "overlay29Select"),
        (0x084, "c", "overlay29RotateForward"),
        (0x124, "c", "overlay29RotateBackward"),
        (0x1C4, "c", "overlay29BuildChain"),
        (0x23C, "c", "overlay29UpdateRatio"),
        (0x304, "c", "overlay29Sample"),
        (0x42C, "c", "overlay29InitializeObject"),
        (0x5C4, "c", "func_overlay_029_F00005C4_187D874"),
        (0xEE0, "c", "overlay29ProjectPoint"),
        (0x10C4, "c", "overlay29HandleEffects"),
        (0x14C8, "c", "overlay29DrawGroups"),
        (0x16CC, "asm", "overlay_029_padding"),
    ],
    26: [
        (0x000, "c", "overlay26InitializeObject"),
        (0x1A0, "c", "func_overlay_026_F00001A0_187A598"),
        (0xB18, "c", "func_overlay_026_F0000B18_187AF10"),
        (0xD24, "c", "overlay26HandleEffects"),
        (0x1158, "c", "overlay26DrawGroups"),
    ],
    30: [
        (0x000, "c", "overlay30Initialize"),
        (0x2B4, "c", "overlay30TransposePixels"),
        (0x438, "asm", "overlay_030_padding"),
    ],
    28: [
        (0x000, "c", "overlay_028"),
        (0x7EC, "asm", "overlay_028_padding"),
    ],
    35: [
        (0x000, "c", "overlay35Initialize"),
        (0x1E0, "c", "func_overlay_035_F00001E0_1881EC0"),
        (0x770, "c", "overlay35BuildGridMasks"),
        (0xB40, "c", "func_overlay_035_F0000B40_1882820"),
        (0x1380, "c", "overlay35SelectHeight"),
        (0x13E8, "asm", "overlay_035_padding"),
    ],
    59: [
        (0x000, "c", "overlay59Release"),
        (0x070, "c", "overlay59PrepareEntry"),
        (0x168, "c", "overlay59ResetEntries"),
        (0x1D4, "c", "overlay59ReleaseAll"),
        (0x21C, "c", "overlay59AppendValue"),
        (0x2D0, "c", "overlay59Update"),
        (0x36C, "c", "overlay59Advance"),
        (0x784, "c", "overlay59Interpolate"),
        (0x84C, "c", "overlay59BuildList"),
        (0x8EC, "c", "overlay59DrawFrame"),
        (0xA1C, "asm", "overlay_059_padding"),
    ],
    60: [
        (0x0000, "c", "overlay60Initialize"),
        (0x0334, "c", "overlay60Prefix"),
        (0x2EC8, "c", "overlay60ReleaseResources"),
        (0x2F54, "c", "func_overlay_060_F0002F54_18BCD2C"),
        (0x32CC, "c", "overlay60DrawBorder"),
        (0x33D8, "c", "overlay60DrawLine"),
        (0x3488, "c", "overlay60ReassignChoiceSlots"),
        (0x355C, "asm", "overlay_060_padding"),
    ],
    46: [
        (0x000, "c", "overlay46InitializeState"),
        (0x120, "c", "overlay46UpdateSequence"),
        (0x614, "c", "overlay46ReleaseState"),
        (0x69C, "c", "overlay46InitializeParticles"),
        (0x874, "c", "func_overlay_046_F0000874_188EC6C"),
        (0xF7C, "c", "overlay46InitState"),
        (0xFD0, "c", "overlay46UpdateTransition"),
        (0x112C, "c", "overlay46Submit"),
        (0x1150, "c", "overlay46InitializeBuffers"),
        (0x1228, "c", "func_overlay_046_F0001228_188F620"),
    ],
    48: [
        (0x000, "c", "overlay48Initialize"),
        (0x060, "c", "overlay48InitializeState"),
        (0x144, "c", "overlay48UpdateState"),
        (0x40C, "c", "overlay48ReleaseAll"),
        (0x46C, "asm", "overlay_048_padding"),
    ],
    101: [
        (0x0000, "c", "overlay101Initialize"),
        (0x00B4, "c", "overlay101AllocateEntry"),
        (0x0108, "c", "overlay101FindEntry"),
        (0x0164, "c", "overlay101UpdateEntry"),
        (0x0258, "c", "overlay101SchedulePair"),
        (0x0344, "c", "overlay101UpdateEntry12"),
        (0x0438, "c", "overlay101SchedulePair12"),
        (0x0524, "c", "overlay101ActivateSlot"),
        (0x0590, "c", "overlay101AdvanceSlot"),
        (0x05E0, "c", "overlay101PromoteSlot"),
        (0x0668, "c", "overlay101UpdateByte17"),
        (0x0708, "c", "overlay101ScheduleByte17"),
        (0x07D8, "c", "overlay101UpdateByte16"),
        (0x0878, "c", "overlay101ScheduleByte16"),
        (0x0948, "c", "overlay101UpdateEntry8"),
        (0x0A3C, "c", "overlay101ScheduleLinkedPair"),
        (0x0B28, "c", "overlay101UpdateEntry8B"),
        (0x0C1C, "c", "overlay101ScheduleLinkedPair2"),
        (0x0D08, "c", "overlay101UpdateFloat12"),
        (0x0D80, "c", "overlay101ScheduleLinkedFloat"),
        (0x0E54, "c", "overlay101UpdateDelta16"),
        (0x0EF4, "c", "overlay101ScheduleLinkedScaled"),
        (0x0FF4, "c", "overlay101UpdateByte18"),
        (0x110C, "c", "overlay101ScheduleLinkedByte"),
        (0x11E0, "c", "overlay101UpdateEntry8C"),
        (0x12D4, "c", "overlay101ScheduleLinkedPair3"),
        (0x13C0, "c", "overlay101UpdateColor"),
        (0x1558, "c", "overlay101ScheduleLinkedColor"),
        (0x1678, "c", "overlay101UpdateFrames"),
        (0x1728, "c", "overlay101ScheduleFrames"),
        (0x1868, "c", "overlay101UpdateGlobalPair"),
        (0x1970, "c", "overlay101ScheduleGlobalPair"),
        (0x1A38, "c", "overlay101DispatchActive"),
        (0x1BB4, "c", "overlay101Reset"),
        (0x1BD0, "c", "overlay101DispatchEvents"),
        (0x1F80, "c", "overlay101SetScissor"),
        (0x2118, "c", "overlay101GetBounds"),
        (0x2250, "c", "overlay101DrawElement"),
        (0x2510, "c", "func_overlay_101_F0002510_18DDD30"),
        (0x29A4, "c", "overlay101DrawTransformed"),
        (0x2C3C, "c", "overlay101BuildFrame"),
        (0x2CE4, "c", "overlay101BuildIntensityColors"),
        (0x2DC0, "c", "overlay101BuildBorder"),
        (0x2EFC, "c", "overlay101DrawPanel"),
        (0x332C, "c", "overlay101DrawClock"),
        (0x36E4, "c", "overlay101DrawChain"),
        (0x3814, "c", "overlay101UpdateChains"),
        (0x3998, "c", "overlay101DrawSlots"),
        (0x3A58, "c", "func_overlay_101_F0003A58_18DF278"),
        (0x512C, "c", "func_overlay_101_F000512C_18E094C"),
        (0x571C, "c", "func_overlay_101_F000571C_18E0F3C"),
        (0x5E08, "c", "func_overlay_101_F0005E08_18E1628"),
        (0x63F8, "c", "func_overlay_101_F00063F8_18E1C18"),
        (0x69E8, "c", "func_overlay_101_F00069E8_18E2208"),
        (0x78F4, "c", "func_overlay_101_F00078F4_18E3114"),
        (0x8128, "c", "func_overlay_101_F0008128_18E3948"),
        (0x895C, "c", "func_overlay_101_F000895C_18E417C"),
        (0x9190, "c", "func_overlay_101_F0009190_18E49B0"),
        (0x99C4, "c", "overlay101BuildPresentationA"),
        (0x9D04, "c", "overlay101BuildPresentationB"),
        (0xA044, "c", "overlay101BuildPresentationC"),
        (0xA384, "c", "overlay101BuildPresentationD"),
        (0xA6BC, "c", "overlay101TailA6BC"),
        (0xAB4C, "c", "overlay101TailAB4C"),
        (0xB544, "c", "overlay101TailB544"),
        (0xBA34, "c", "overlay101TailBA34"),
        (0xC144, "c", "overlay101TailC144"),
        (0xC6E8, "c", "overlay101TailC6E8"),
        (0xCBDC, "c", "overlay101UpdatePresentation"),
        (0xCD50, "c", "overlay101Cleanup"),
        (0xCEA8, "c", "overlay101ByteLength"),
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

# Reviewed initialized-section ownership. These overlays reproduce their full
# data/rodata range from ordinary definitions in an existing C translation
# A consolidated C object can contain both untouched, byte-exact C functions
# and GLOBAL_ASM fallbacks. `is_nonmatching_source()` intentionally remains a
# conservative object-level signal, while these reviewed ranges retain the
# finer pre-consolidation evidence needed by the byte-weighted scoreboard.
# Three-field ranges were independently compiled, metadata-only objects before
# consolidation. Newly promoted ranges carry a fourth, range-specific evidence
# field after their canonical mixed-TU and linked-ROM proofs.
MIXED_TU_EXACT_C_RANGES = {
    1: [
        # One row per function: promotion-proof authenticates a function
        # only against a range whose label is its own C definition.
        (0x0000, 0x0028, "overlay1PreviousPointer"),
        (0x0028, 0x0050, "overlay1NextPointer"),
        (
            0x0050,
            0x0080,
            "overlay1GetEntry",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x0080, 0x00A0, "overlay1GetEntryIndex"),
        (0x00A0, 0x00C0, "overlay1PreviousIndex"),
        (0x00C0, 0x00E4, "overlay1NextIndex"),
        (0x00E4, 0x0154, "overlay1WrapOffset"),
        (0x0154, 0x01AC, "overlay1SignedOffset"),
        (
            0x01AC,
            0x02D4,
            "overlay1FindType47ByAngle",
            "canonical mixed-TU C, six runtime relocation identities, and linked ROM bytes exact",
        ),
        (0x02D4, 0x0330, "overlay1GetLinkedActive"),
        (0x0330, 0x0378, "overlay1GetRecord"),
        (
            0x0378,
            0x0414,
            "overlay1FindType5ByKey",
            "canonical mixed-TU object, one runtime relocation, and linked ROM bytes exact",
        ),
        (0x0758, 0x07B0, "overlay1TestDirection"),
        (
            0x0BD4,
            0x0CA8,
            "overlay1InitMotionScale",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x0CA8,
            0x0DF4,
            "overlay1InterpolatePath",
            "canonical mixed-TU object, all 13 runtime relocations, and linked bytes exact",
        ),
        (
            0x0DF4,
            0x0F84,
            "overlay1ResolveMotionPoint",
            "canonical mixed-TU object, 11 runtime relocations, and linked bytes exact",
        ),
        (
            0x0F84,
            0x10C0,
            "overlay1MeasureCurves",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x10C0, 0x10C8, "overlay1Noop"),
        (0x19B8, 0x1A54, "overlay1InitializeModeState"),
        (0x1A54, 0x1CA4, "overlay1BuildObjectMappings"),
        (0x1CA4, 0x1D58, "overlay1ReleaseRecords"),
        (0x1D58, 0x1D78, "overlay1CallReset"),
        (
            0x2744,
            0x280C,
            "overlay1FindNextAngle",
            "canonical mixed-TU object, all four runtime relocations, and linked ROM bytes exact",
        ),
        (
            0x280C,
            0x28D4,
            "overlay1FindPreviousAngle",
            "canonical mixed-TU object, all four runtime relocations, and linked ROM bytes exact",
        ),
        (0x28D4, 0x293C, "overlay1RefreshMode"),
        (0x293C, 0x296C, "overlay1CallGlobal"),
        (
            0x296C,
            0x2AA4,
            "overlay1AdvanceObjectGauges",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x2AA4,
            0x2B4C,
            "overlay1AdvanceGauge",
            "canonical mixed-TU object, three runtime relocations, and linked ROM bytes exact",
        ),
        (
            0x3578,
            0x36A0,
            "overlay1InitializeGaugeObjects",
            "canonical mixed-TU object, all three runtime relocations, and linked ROM bytes exact",
        ),
        (
            0x36A0,
            0x3750,
            "overlay1AssignRecordIndex",
            "canonical mixed-TU object, seven runtime relocations, and linked ROM bytes exact",
        ),
        (0x3E48, 0x3E74, "overlay1SubmitGlobals"),
        (0x3E74, 0x3EB8, "overlay1SubmitAll"),
        (0x3EB8, 0x3F38, "overlay1AngleBetweenSamples"),
        (0x3F38, 0x3F88, "overlay1RelativeAngleA"),
        (0x3F88, 0x3FD8, "overlay1RelativeAngleB"),
        (0x5BA4, 0x5BC0, "overlay1InitTimedState"),
        (0x5BC0, 0x5BF4, "overlay1ConsumeTimer"),
        (
            0x5BF4,
            0x5CD4,
            "overlay1StartTimerCallbacks",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x5CD4,
            0x5ECC,
            "overlay1FindDirectionalObject",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x5ECC, 0x5ED4, "overlay1ReturnZero"),
        (
            0x5ED4,
            0x61F0,
            "overlay1DispatchMode",
            "canonical mixed-TU object, 61 runtime relocations, and linked bytes exact",
        ),
        (
            0x6270,
            0x63CC,
            "overlay1ChooseModeObject",
            "canonical mixed-TU object, 13 runtime relocations, and linked bytes exact",
        ),
        (0x63CC, 0x6424, "overlay1UpdateCountdown"),
        (0x6424, 0x64F8, "overlay1ReadSelection"),
        (0x64F8, 0x6724, "overlay1SolveAngleCandidates"),
        (0x6724, 0x6788, "overlay1UpdateModeSound"),
        (0x6788, 0x67C0, "overlay1CopyBytes"),
        (0x69A0, 0x6A14, "overlay1InitMotion"),
        (0x6B28, 0x6B6C, "overlay1InitRange"),
        (
            0x6B6C,
            0x6CE8,
            "overlay1SearchNearby",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x6CE8, 0x6D4C, "overlay1SelectMaskedMode"),
        (
            0x6D4C,
            0x7130,
            "overlay1UpdateAimedTransient",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x7130,
            0x72A4,
            "overlay1UpdateTransient",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x72A4,
            0x7344,
            "overlay1AllocateRecord",
            "canonical mixed-TU object, 10 runtime relocations, and linked bytes exact",
        ),
        (
            0x7344,
            0x73A0,
            "overlay1CloneRecord",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x73A0,
            0x7580,
            "overlay1UpdateValueCache",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x7580,
            0x7730,
            "overlay1AppendPathPoint",
            "canonical mixed-TU object, all eight relocation records, and linked ROM bytes exact",
        ),
        (
            0x7B64,
            0x7BDC,
            "overlay1FindBestRecord",
            "canonical mixed-TU object, both runtime relocations, and linked ROM bytes exact",
        ),
        (0x7FCC, 0x8008, "overlay1ModeChecks"),
        (0x8008, 0x8048, "overlay1DistanceFromCurrent"),
        (0x8048, 0x80BC, "overlay1DistanceFromSelected"),
    ],
    2: [
        (0x02C4, 0x0400, "overlay2ClassifyBoundary"),
    ],
    4: [
        (0x000, 0x138, "overlay4InitializeObjectMotion"),
        (0x4D0, 0x52C, "overlay4AttachObject"),
        (0x52C, 0x5D0, "overlay4RemoveObject"),
        (
            0x5D0,
            0x710,
            "overlay4UpdateGroupSpacing",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x710, 0x734, "overlay4GroupCount"),
        (
            0x734,
            0x8F4,
            "overlay4FindCategory2Object",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x8F4,
            0xCAC,
            "overlay4FindSearchPosition",
            "canonical mixed-TU object and linked bytes exact",
        ),
    ],
    5: [
        (0x2E4, 0x31C, "overlay5InitSequence"),
        (0x6C0, 0x764, "overlay5CreatePlayer"),
    ],
    7: [
        (
            0x000,
            0x0A8,
            "overlay7ReleaseEntry",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x228, 0x298, "overlay7CreateEntry"),
        (0x298, 0x324, "overlay7AppendEntry"),
        (
            0x894,
            0xAA0,
            "overlay7DispatchModes",
            "canonical mixed-TU object, 23 text plus seven switch-table runtime relocations, and linked bytes exact",
        ),
        (
            0xCCC,
            0xDBC,
            "overlay7DispatchSelection",
            "canonical mixed-TU object, all 13 runtime relocations, and linked bytes exact",
        ),
        (
            0xDBC,
            0xEDC,
            "overlay7CommitSelection",
            "canonical mixed-TU object, 17 runtime relocations with two declared metadata filters, and linked bytes exact",
        ),
        (0xEDC, 0xF08, "overlay7FillValues"),
        (0xF08, 0xFB8, "overlay7InitPool"),
    ],
    8: [
        (0x000, 0x008, "overlay8Ignore"),
        (0x008, 0x058, "overlay8GetIndexed"),
        (
            0x894,
            0xE88,
            "func_overlay_008_F0000894_185E5EC",
            "canonical mixed-TU object and linked ROM bytes exact",
        ),
        # Labels are the names overlay_008.c defines; the friendly names
        # these ranges once carried are not C definitions.
        (0xE88, 0xF1C, "func_overlay_008_F0000E88_185EBE0"),
        (0xF1C, 0x1000, "func_overlay_008_F0000F1C_185EC74"),
        (0x1000, 0x1294, "func_overlay_008_F0001000_185ED58"),
        (
            0x291C,
            0x2EC0,
            "func_overlay_008_F000291C_1860674",
            "canonical mixed-TU object and linked ROM bytes exact",
        ),
        (0x2EC0, 0x3018, "func_overlay_008_F0002EC0_1860C18"),
        (
            0x3018,
            0x3278,
            "overlay8UpdateChannels",
            "canonical mixed-TU object, 16 runtime relocations, and linked bytes exact",
        ),
        (0x3278, 0x3368, "func_overlay_008_F0003278_1860FD0"),
        (
            0x3368,
            0x34A0,
            "overlay8ScaleOutputs",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x49A4, 0x49B4, "overlay8SetBuffer"),
        (0x49B4, 0x49DC, "overlay8WriteCommand"),
        (0x49DC, 0x49E8, "overlay8SetValue"),
        (0x49E8, 0x4CF0, "overlay8UpdateMotionOutput"),
    ],
    9: [
        (
            0x09BC,
            0x0CE4,
            "func_overlay_009_F00009BC_1867034",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x0CE4,
            0x0F6C,
            "func_overlay_009_F0000CE4_186735C",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x0F6C,
            0x10A4,
            "func_overlay_009_F0000F6C_18675E4",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x10A4, 0x10B4, "overlay9Ignore"),
        (
            0x10B4,
            0x151C,
            "func_overlay_009_F00010B4_186772C",
            "canonical mixed-TU object and linked bytes exact",
        ),
    ],
    45: [
        (
            0x764,
            0x1158,
            "func_overlay_045_F0000764_188CBBC",
            "canonical object, 24 runtime relocations, and linked bytes exact",
        ),
    ],
    12: [
        (0x000, 0x0C4, "overlay12Initialize"),
    ],
    15: [
        (0x000, 0x00C, "overlay15GetResource4"),
        (0x00C, 0x04C, "overlay15ReleaseResource"),
        (0x500, 0x6A4, "overlay15DrawScreenStars"),
        (0x6A4, 0x6B0, "overlay15GetResource10"),
        (0x6B0, 0x6E8, "overlay15ReleaseResource10"),
        (0xB7C, 0xB88, "overlay15SetValueC"),
        (0xB88, 0xB94, "overlay15ClearValue7C"),
    ],
    16: [
        (0x000, 0x08C, "overlay16BuildGradient"),
        (0x08C, 0x1A8, "overlay16InitializeBuffer"),
        (0x1A8, 0x1E0, "overlay16ReleaseBuffer"),
        (0x1E0, 0x424, "overlay16ApplyGradient"),
    ],
    22: [
        (
            0xD30,
            0xE9C,
            "func_overlay_022_F0000D30_1878E38",
            "canonical mixed-TU object, 12 runtime relocations, and linked ROM bytes exact",
        ),
    ],
    25: [
        (
            0x000,
            0x17C,
            "overlay25InitializeEffect",
            "canonical mixed-TU object, nine runtime relocations, and linked bytes exact",
        ),
        (0x588, 0x608, "overlay25SetVectorFlags"),
    ],
    27: [
        (0x000, 0x064, "overlay27Init"),
        (0xA1C, 0xB20, "overlay27UpdateCoordinates"),
        (0xB20, 0xB68, "overlay27CanUse"),
        (0xB68, 0xBC0, "overlay27Activate"),
    ],
    26: [
        (0x1158, 0x1370, "func_overlay_026_F0001158_187B550"),
    ],
    28: [
        (0x000, 0x070, "overlay28ResetBuffer"),
        (0x070, 0x1B8, "overlay28UpdateVertices"),
        (0x1B8, 0x318, "overlay28InitializeWork"),
        (0x318, 0x4D8, "overlay28UpdateWork"),
    ],
    49: [
        (0x354, 0x374, "refractOutput"),
    ],
    29: [
        (0x14C8, 0x16CC, "overlay29DrawGroups"),
    ],
    31: [
        (0x02E8, 0x04F8, "func_overlay_031_F00002E8_187F808"),
    ],
    34: [
        (
            0x0D4,
            0x2C8,
            "overlay34CreateRecord",
            "canonical mixed-TU object, 12 runtime relocations, and linked bytes exact",
        ),
    ],
    35: [
        (0x0770, 0x0B40, "func_overlay_035_F0000770_1882450"),
    ],
    51: [
        (
            0x000,
            0x080,
            "overlay51Initialize",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x080,
            0x0D0,
            "overlay51PatchIndices",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x858,
            0x8AC,
            "overlay51ReleaseState",
            "canonical mixed-TU object and linked bytes exact",
        ),
    ],
    56: [
        (
            0x000,
            0x05C,
            "overlay56OffsetCoordinates",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x05C,
            0x0B8,
            "overlay56CenterCoordinates",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x0B8,
            0x10C,
            "overlay56SplitTime",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x10C,
            0x118,
            "overlay56SetMode",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x118,
            0x168,
            "overlay56LoadResource",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0x168,
            0x1A0,
            "overlay56ReleaseResource",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (
            0xAB4,
            0xAF4,
            "overlay56UnpackColor",
            "canonical mixed-TU object and linked bytes exact",
        ),
    ],
    58: [
        (
            0x138C,
            0x4C04,
            "func_overlay_058_F000138C_18B0574",
            "canonical mixed-TU object and linked bytes exact",
        ),
        (0x4F28, 0x50C8, "overlay58DrawPointQuad"),
        (0x50C8, 0x5268, "overlay58DrawLargePointQuad"),
    ],
    60: [
        (0x2F54, 0x32CC, "func_overlay_060_F0002F54_18BCD2C"),
    ],
    63: [
        (0x77C, 0x928, "overlay63UpdateSequence"),
    ],
    68: [
        (0x1250, 0x1438, "overlay68RebuildSecondaryEntry"),
    ],
    89: [
        (0x270, 0x5A4, "overlay89InitializeEffect"),
        (0x5A4, 0x7C4, "overlay89UpdateStateAndParticles"),
    ],
    90: [
        (0x000, 0x0FC, "overlay90Initialize"),
    ],
}


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
    return rows


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
            raise ValueError(f"invalid overlay {overlay} mixed-TU exact range")
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
            "owned_data_rodata_bytes": sum(
                int(part["size"], 16)
                for row in module_rows
                for part in row.get("data_rodata_ownership", [])
            ),
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


def render_yaml_block(
    atlas,
    trial_ownership=False,
    trial_sources=frozenset(),
    trial_functions=frozenset(),
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

        carved = trial_ownership and any(
            fixed_data_matches_trial(part)
            for part in row.get("data_rodata_ownership", [])
        )
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
        owned_data = row.get("data_rodata_ownership", [])
        compiler_padding = COMPILER_TEXT_ALIGNMENT_PADDING.get(ov)
        explicit_padding = EXPLICIT_TEXT_PADDING.get(ov)
        for part in row["text_ownership"]:
            source_name = part["source"].rsplit("/", 1)[1]
            if source_name == compiler_padding:
                if not owned_data:
                    raise ValueError(
                        f"overlay {ov} compiler padding needs C-owned data"
                    )
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
        fixed_data = [
            part
            for part in owned_data
            if fixed_data_matches_trial(part)
        ] if carved else []
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

            cursor = owned_end
            for part in sorted(fixed_data, key=lambda item: int(item["offset"], 16)):
                fixed_start = int(part["offset"], 16)
                fixed_end = int(part["end_offset"], 16)
                if fixed_start < cursor:
                    raise ValueError(
                        f"overlay {ov} fixed data/rodata ownership overlaps "
                        "a leading C-owned range or another fixed range"
                    )
                if cursor < fixed_start:
                    raw_slice(cursor)
                lines.append(
                    f"      - [{hx(int(data_row['start'], 16) + fixed_start)}, "
                    f"{part['section']}, {part['source'].rsplit('/', 1)[1]}]"
                )
                cursor = fixed_end
            if cursor < data_size:
                raw_slice(cursor)
        elif owned_end < data_size:
            lines.append(
                f"      - [{hx(int(data_row['start'], 16) + owned_end)}, "
                f"bin, {name}_data_rodata]"
            )
        for section, suffix in (("reloc1", "_reloc1"), ("reloc2", "_reloc2")):
            section_row = row["sections"][section]
            if section_row["size"] == "0x0":
                continue
            if section == "data_rodata" and row.get("data_rodata_ownership"):
                continue
            lines.append(
                f"      - [{section_row['start']}, bin, {name}{suffix}]"
            )
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
