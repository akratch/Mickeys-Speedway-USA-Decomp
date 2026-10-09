# Overlay 51 reconstruction identity audit

**Status (2026-10-02): matched and promoted.** The function is now exact C in
`src/overlays/o051/overlay_051.c`, written as the cut-down copy of overlay 50's
matched `func_overlay_050_F0000334_1896CA4` and built with `-Wab,-r4300_mul`;
the TU has no `GLOBAL_ASM` left. The data identities below were merged to one
name per object: `gOverlay51Mode`/`gOverlay51Index` are `gOverlay51Item`,
`gOverlay51InitialValue` is `gOverlay51HudHeight`, `gOverlay51Resource1C` and
`gOverlay51ResourceBC` are the glyph rows `gOverlay51TimeGlyphs` and
`gOverlay51ClockGlyphs`, and `gOverlay51InlineResource` is
`gOverlay51Resource0`. The rest of this page is the pre-match receipt, kept as
written.

This is a read-only identity receipt, not a candidate, match or reopen grant.
The owned fallback is `func_overlay_051_F00000D0_18999D0` in
`src/overlays/o051/overlay_051.c`, module 51 text +0xD0 through +0x858
(1,928 bytes). Assignment still fails closed as `stale-ledger/source-identity`:
there is no current configured C definition for this bare fallback.

The historical candidate is retained at
`1ae8ab558ab67871c210e2a007bfd9164b4d504d`; its removal is
`d1eec213d9de4aedf1842ec94f4e46001febdeb5`. That candidate conflates
unresolved calls under one generated declaration and conflates integer and
floating data. Its historical score is not a current configured baseline.

## Independently bound calls

The authenticated module runtime records contain 23 call sites and 64 data
records (32 HI/LO pairs), 87 records in total. Decoding runtime identities,
including stored addends, resolves the calls to 18 distinct callees. Names were
bound through the independently linked canonical ELF using section-qualified
overlay identities, not the shared synthetic overlay address alone. Seventeen
callees have canonical C definitions, covering 22 call sites. The remaining
callee has guarded candidate C; its identity does not prove that candidate ABI.

| Callee | Call sites | Canonical source |
| --- | ---: | --- |
| `func_80028F54` | 1 | `src/main/main.c` |
| `camStandardOrtho` | 1 | `src/main/camera.c` |
| `overlay56SplitTime` | 1 | `src/overlays/o056/overlay_056.c` |
| `levelGetLevel` | 1 | `src/main/level.c` |
| `func_800290A0` | 1 | `src/main/main.c` |
| `func_8002F618` | 4 | `src/main/rcpFast3d.c` |
| `texDPInit` | 2 | `src/main/textures_354C8.c` |
| `func_80039E34` (guarded) | 1 | `src/main/menu.c` |
| `freeFrontEndItem` | 2 | `src/main/menu.c` |
| `loadFrontEndItem` | 1 | `src/main/menu.c` |
| `overlay59Interpolate` | 1 | `src/overlays/o059/overlay59Interpolate.c` |
| `overlay59BuildList` | 1 | `src/overlays/o059/overlay59BuildList.c` |
| `mainGetMode` | 1 | `src/main/main.c` |
| `func_800016EC` | 1 | `src/main/audio_manager_1050.c` |
| `func_8003A590` | 1 | `src/main/menu.c` |
| `func_80037414` | 1 | `src/main/frontend_37D50.c` |
| `mainChangeLevel` | 1 | `src/main/main.c` |
| `amTuneSetFade` | 1 | `src/main/audio_manager_1050.c` |

Existing exact declarations provide useful ABI constraints:
`func_8002F618` takes four trailing unsigned-byte arguments,
`func_800016EC` takes an unsigned byte, and `func_80037414` has two floating
parameters after its first integer parameter. The historical overlay-50 or
overlay-51 candidate declarations are not authority for these widths.
`overlay56SplitTime` provides three integer output pointers;
`overlay59Interpolate` and `overlay59BuildList` have distinct independently
defined interfaces. These witnesses permit bounded reconstruction, but do not
establish the target body or all remaining data types.

## Independently bound data

There are 14 distinct data identities across the 32 pairs. Matching named
relocations in the canonical overlay-51 object to runtime records at exact
sibling sites binds five identities covering 13 target pairs:

| Named sibling witness | Target pairs | Constraint |
| --- | ---: | --- |
| `gOverlay51Mode` / `gOverlay51Index` | 7 | Two names for one runtime identity, not distinct fields |
| `gOverlay51InitialValue` | 3 | Exact initializer supplies a floating-value witness |
| `gOverlay51Objects` | 1 | Exact sibling uses an object pointer table |
| `gOverlay51Resource1C` | 1 | Exact initializer supplies named local identity |
| `gOverlay51ResourceBC` | 1 | Exact initializer supplies named local identity |

Nineteen pairs across nine data identities remain unbound by this audit.
Reserved runtime namespaces remain distinct; no resident address or shared
synthetic overlay address was substituted for identity. Pointer binding alone
does not establish every pointee layout or field type.

The next reconstruction packet should bind those remaining data groups and
recover declarations from exact callee definitions before compiling a full-TU
candidate. A bare-fallback assignment path still needs explicit coordinator
authorization and normal source-identity safeguards. No game-source edit,
candidate compile, reopen change or new matching credit occurred here.

Evidence used the lane-owned canonical ELF and object from its verified ROM
build, `overlay_tables.read_module_relocations`, and runtime identity attachment
from `tools/reloc_surface.py`. Private decoded records and mappings remain in
ignored build artifacts. Documentation and clean-room gates validate this
report-only receipt; they are not a replacement for future matching proofs.
