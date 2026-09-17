# The overlay system (docs/modules.md section 5)

Split out of `docs/modules.md` on 2026-08-24; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 5.x identity so existing references resolve.

## 5. The overlay system

### 5.1 What runs it

The resident segment carries a complete Rare/DKR-lineage runtime linker at ROM
`0x323E0`–`0x33FA0`, plus its trampoline at `0x33FA0`. Fourteen of its functions are
decompiled and byte-matched; four more are named from Mickey's call graph. The
mechanism, entirely from Mickey's own disassembly:

1. A call to a function that lives in a not-yet-loaded overlay is assembled as
   a `jal` to **`TrapDanglingJump`** (`0x800333A0`).
2. `TrapDanglingJump` saves every argument register (`a0`–`a3`, `f12`–`f15`,
   `v0`, `v1`) and computes `ra - 8`, the address of the `jal` that reached it.
3. It searches **`mainRelocTable`** (8-byte entries) for the entry whose call
   site is that address, which yields an index into **`overlayRomTable`**.
4. That 4-byte entry splits into a 12-bit overlay number and a 20-bit offset
   (`RomTableEntry`).
5. **`runlinkDownloadCode`** (`0x80031C78`) loads the overlay and relocates it,
   calling `ProcessRelocationEntry` per record and finishing with
   `osInvalICache`, genuine self-modifying code.
6. The trampoline recomputes `overlayTable[n].vramBase + offset`, restores the
   arguments and `jr`s to the real function. The caller never knows.

Step 0 is `runlinkInit` (`0x800328CC`), which runs once at boot, out of
`func_80026E4C`, immediately before the first `TrapDanglingJump`: it allocates
and DMA-copies the three tables out of ROM and synthesizes `overlayTable[0]`
for the resident module itself. §5.3.

`runlinkGetAddressInfo` (`0x800331E4`) is the inverse, and is what the debug
monitor uses to print "Module %d at %08x". Its fourth parameter is an optional
symbol-name out-pointer filled by **`GetSymbolName`** (`0x800317E0`), which in
this retail build is four instructions that spill their argument to the stack,
never read it back, and return the constant string `"unknown"`. The ROM-side
symbol table the mechanism is built around is simply absent from the shipped
image. Its C now reproduces all four instructions and its `D_80082410`
relocation exactly under the resident `-O2 -mips2 -32` flags.

`runlinkResumeAll` (`0x80032FE0`, `0x60` bytes) scans the sixteen pending-load
slots and resumes every entry not marked `0xFFB`. Its name and role follow
JFG's ordered `runlinkResumeAll` peer (tier B); the adapted C compiles to all
24 Mickey instruction words with the exact data/call relocation surface under
the resident `-O2 -mips2 -32` flags, and its linked ROM range is byte-identical.

`runlinkFlushModules` (`0x80032820`, `0xAC` bytes) unloads resident overlays in
descending order, then frees and clears all sixteen pending-load slots. Its
name, position, and control flow follow JFG's `runlinkFlushModules` peer (tier
B); the adapted C compiles to all 43 Mickey instruction words with the exact
global and call relocation surface under the resident `-O2 -mips2 -32` flags,
and its linked ROM range is byte-identical.

`runlinkTick` (`0x80033090`, `0xBC` bytes) walks the packed link-slot table
backwards when the resident enable flag is set. It ages both counters and calls
`runlinkFreeCode` when the upper ten-bit counter reaches zero. Its name, role, and
control flow follow JFG's 0xC0-byte `runlinkTick` peer (tier B); Mickey's mips2
build omits JFG's load-delay nops. The C compiles to all 47 instruction words
with the exact three-global/one-call relocation surface, and its linked ROM
range is byte-identical under `-O2 -mips2 -32`.

`runlinkSuspendCode` (`0x80032B14`, `0xE4` bytes) claims a free pending-load
slot, records the overlay's resident address, unloads it, and reserves its old
address range for a later resume. Its name, position, and control flow follow
JFG's 0xE8-byte peer (tier B); Mickey uses allocation tag `0x83` and its mips2
build omits JFG's load-delay no-op. The adapted C compiles to all 57
instruction words with the exact relocation surface, and the linked ROM range
is byte-identical under `-O2 -mips2 -32`.

`runlinkUnloadOverlay` (`0x80032618`, `0x208` bytes) frees a resident overlay
or cancels its pending allocation, clears the packed link-slot state, and
rewrites resident references to the dangling-jump trap. Its name, role, and
body follow JFG's published peer (tier B), adapted to Mickey's relocation and
section layouts. The C compiles to all 130 instruction words with the exact
relocation surface, and its linked ROM range is byte-identical under
`-O2 -mips2 -32`.

`ProcessRelocationEntry` (`0x80031A30`, `0x248` bytes) remains
`NON_MATCHING`. A fresh 119-combination flag sweep and an explicit
`-O2 -g0 -mips2 -32` schedule probe both retain the stock 147-instruction
candidate against the target's 146, with 126 positional word differences from
`+0x0` and a `0x48` frame against `0x40`; `-O2 -g3` regresses to 128 words.
The candidate continues to promote `patchLocation` to `s1`, adding its
save/restore pair, while the target caller-saves that value around
`ResolveRelocAddress` and reserves `s0` only for `relocEntry`. The remaining
untried family is the documented pool-position levers 8-13, not another flag
or branch-shape permutation.

`runlinkFreeCode` (`0x80032338..0x80032618`, ROM `0x32F38..0x33218`)
is matched C (tier A): all 184 instruction words (736 bytes), the `0x58`
frame, and all 32 static relocation offsets, types, and identities are exact
under `-O2 -mips2 -32`. The linked owned range is byte-identical to Mickey's
ROM, with no padding credited. It cancels a pending allocation or frees the
loaded overlay, clears its link slot, and patches references in every other
loaded module. PROVENANCE: adapted from Jet Force Gemini's published
`src/runLink.c:runlinkFreeCode` at `efd5abb`; Mickey's packed records and
resident section anchors decide the layout. The donor's cached loaded base,
explicit patch address, and counted pending scan close the former size/frame
residual. Unsigned N64 base-address sums close the remaining four operand
register differences; pointer-addition commutation alone was flat.

`tier-B runlinkResumeCode`: 6 stack operands remain, first `+0x0`; 250 instructions, opcode schedule, registers, and relocations are exact.
Workbench frame-layout; stack-home levers 26/32, frame-local variants, flag lattice, and bounded permutation did not alter the frame.
The target reserves `0x50` with pendingLoad at `sp+0x44`; the candidate reserves `0x48` with the home at `sp+0x40`.

`runlinkInit` is exact C. Its authenticated owner is resident
`0x800328CC..0x80032B14` (ROM `0x334CC..0x33714`), with no padding before
`runlinkSuspendCode`; exact-C `mainInitGame+0x12C` is its sole direct caller,
the ABI is `void (void)`, and it has no export or runtime-overlay relocation.
All 146 instruction words, the `0x38` frame and all 64 static relocations are
exact, and the linked ROM range is byte-identical.

The plateau was one mechanism, and it was a relocation-*identity* question
rather than a code-generation one. The target materializes `0x80078D60` twice
and `0x80085A40` twice, while uopt shares one address materialization per
*symbol* -- it shares `func_80000450` between the `vramBase` store and the
`textSize` difference inside this same target. Two names each, not two uses of
one name: the original link placed the code-segment end and the data-segment
start at `0x80078D60`, and the data-segment end and the bss-segment start at
`0x80085A40`. Spelling all four through one name per address costs exactly the
four instructions the earlier candidates were short.

This build supplies two real names at `0x80085A40` -- `main_RODATA_END` and
`main_BSS_START`, from the generated linker script -- and only one at
`0x80078D60`, because splat's own text/data boundary is `0x80076110`. The
data-segment start is therefore spelled off the following word. Both spellings
resolve to `0x80078D60` and link to the same two instruction words; only the
symbol/addend split inside the unlinked object differs, exactly as the
pending-list base does (`D_800D2DC8 + 0x78` against the target's
`D_800D2E40`).

The `#pragma weak` aliases the previous candidate carried could not have been
promoted at all. IDO turns `#pragma weak name = extern-symbol` into a weak
*undefined* symbol rather than an alias, so those four references would have
linked to zero. They were also invisible to `tools/score_symbol.py`, whose mask
erases the `HI16`/`LO16` value field without comparing relocation identities,
which is how that body could read one raw and zero relocation-masked differing
words while carrying eight unresolvable references. `function_preflight.py`
reported them, as eight unresolved candidate identities.

JFG applicability was concrete but limited: its published `runlinkInitialise`
has the same table setup, pending-list, resident-header and anchor
rematerialization schedule, and Mickey's body was adapted from it, but its C is
not equivalent and the four-name mechanism above is Mickey's own.

| Function | ROM | Bytes | Flags | Donor and verdict |
|---|---:|---:|---|---|
| `runlinkDownloadCode` | `0x32878` | `0x478` | `-O2 -mips2 -32` | JFG `src/runLink.c`; 286/286 instruction words and all relocations exact, linked ROM byte-identical |

`func_800320F0` (`runlinkEnsureJumpIsValid`, `0x800320F0`, `0x194` bytes) is
Evidence A exact C after a bounded permuter pass resolved the prior register
allocation mismatch. Under `-O2 -mips2 -32`, all 101 instruction words, the
`-0x20` frame, all 21 relocations, and the linked ROM bytes match. JFG's
0x1A8-byte peer remains provenance for the role and skeleton; Mickey's target
boundary and exact output are established independently by its own object and
ROM bytes. The permuter-forced inert blocks are tracked in
`docs/cleanup-queue.md` as a readability follow-up, not a matching deficit.

`runlinkGetAddressInfo` (`0x800331E4`) is an exact 108-word match under
`-O2 -mips2 -32`; its three following nop words are alignment padding before
the separately split trap TU. The body donor is JFG's public runlink assembly.

### 5.2 The tables

Named from stride and use in Mickey's disassembly (`symbol_addrs.us.txt`), all
six in BSS, all six written by `runlinkInit` and by nothing else:

| Symbol | VRAM | Element | Stride | Filled from |
|---|---|---|---|---|
| `overlayTable` | `0x800D2D90` | `OverlayHeader` | `0x20` | ROM `0x184B680`, at `+0x20` |
| `mainRelocTable` | `0x800D2D94` | `RelocTableEntry` | `0x8` | ROM `0x1848B74` |
| `overlayRomTable` | `0x800D2D98` | `RomTableEntry` | `0x4` | ROM `0x1849730` |
| `overlayCount` | `0x800D2D9C` | count = 108 | — | `(0xD60 >> 5) + 1` |
| `mainRelocTableCount` | `0x800D2DA0` | count = 375 | — | the word at ROM `0x1848B70` |
| `linkSlotTable` | `0x800D2E48` | `LinkSlot` | `0x2` | allocated and zeroed |

`overlayCount` is 108 and there are 107 headers in ROM: entry 0 is synthesized
for the resident module, so overlay *n* is ROM header *n - 1*.

`overlayCount` bounds **both** the overlay table and the link-slot table, so
there is exactly one link slot per overlay. That is the best available
evidence for what `LinkSlot`'s two fields mean, and still not enough to promote
them out of inference.

`RelocTableEntry` (`include/game/runlink.h`) is **not JFG's layout**: Mickey
puts the ROM-table index first and packs the call site as a 24-bit offset from
`0x80000450` in the high bits of the second word. Derived from Mickey's ROM;
only the type's name is borrowed.

### 5.3 Where the tables are in ROM

Four blocks, stored flat and uncompressed, back to back:

| ROM | Size | Contents |
|---|---|---|
| `0x1848B70` | `0x4` | `u32 mainRelocTableCount` = **375** |
| `0x1848B74` | `0xBB8` | `RelocTableEntry[375]`, then four bytes of pad |
| `0x1849730` | `0x1F50` | `RomTableEntry[2004]` |
| `0x184B680` | `0xD60` | `OverlayHeader[107]` |
| `0x184C3E0` | `0xA5C00` | **107 overlays**, each image `[.text][.data][relocTable1][relocTable2]` |

`runlinkInit` (`0x800328CC`) computes each block's size as the difference of two
ROM address literals, allocates, and DMA-copies it through `func_8002E3E0`,
which is `osInvalDCache` plus a loop of `osPiStartDma` and `osRecvMesg` in
`0x400`-byte chunks. **The one thing that hid this layout is a single word**:
the relocation block opens with its own entry count, and the initializer sets
`mainRelocTable = copy + 4`, so decoding 8-byte entries from `0x1848B70` reads
every entry one word out of phase and swaps its two fields.

The three table blocks remain `bin` segments in `mickey.us.yaml`.
`gmake overlay-tables` (`tools/overlay_tables.py`) decodes them and the module
images into a 107-row map, re-asserting the five checks below against the ROM
on every run.

**What makes the layout a measurement rather than a reading.**

- For all 107 modules, with zero mismatches,
  `next.romAddress - this.romAddress == textSize + dataSize + relocTableSize +
  relocTableSize2`. The last module ends at `0x18F1FE0`, byte-exact with the
  `rom_fill` boundary the ROM map already had. No rodata term fits that sum,
  which is what makes `OverlayHeader[0x10]` `bssSize` (`include/game/runlink.h`).
- 370 of the 375 relocation entries decode to a call site whose ROM word is
  literally `0C00CCE8`, i.e. `jal TrapDanglingJump`. That is the same 370 as the
  `jal TrapDanglingJump` sites in `asm/`, and it settles §5.1's mechanism from
  the shipped bytes: an unloaded call site *is* a real `jal` in the ROM image.
  The five that are not are two `HI16`/`LO16` `SYMBOL` pairs patching a
  `lui`/`addiu` of `0x800D2DC4` and one `R_MIPS_32` patching a data word, which
  is what their flags bytes say they are and what a `jal` test correctly
  rejects.
- `clone/clone.c`'s two `__FILE__` strings (§3.1, located from resident rodata
  and independently of any of this) land at offsets `0x1500`/`0x1510` inside
  **overlay 43's `.data`** under this arithmetic, with nothing fitted.
- The maximum `romTableIndex` any relocation entry uses is 1473, inside
  `RomTableEntry[2004]`; the maximum real overlay number in `overlayRomTable` is
  107, and its reserved selectors `0xFFF` and `0xFFD` appear 136 and 97 times,
  confirming `RomTableEntry`'s 12/20 bitfield split.
- Every one of the 107 headers has `vramBase == 0` and a strictly monotonic
  `romAddress`: the modules ship unrelocated and are placed at load time.

**Nothing in this path decompresses.** `runlinkInit` and `runlinkDownloadCode`
both copy through `osPiStartDma`, and neither call graph reaches `gzip_asm`
(`0x4EA60`), the resident decompressor the asset loader uses. The block bytes
decode as the documented structs directly.

The per-module relocation tables are `RelocationEntry[]` as
`include/game/runlink.h` documents, `relocTableSize`/`relocTableSize2` bytes
each. Across all 107 modules, table 1 is 6943 records that are overwhelmingly
`SYMBOL` (`symbolIndex` indexes `overlayRomTable`) and table 2 is 11599 records
that are overwhelmingly `LOCAL` (`symbolIndex` is a byte offset from the
module's own base). **That division of labour is inferred from the flag census,
not proven**; `runlinkDownloadCode` is what would prove it.

### 5.4 The canonical overlay work surface

That split is now complete. `config/overlays.us.json`, generated by
`tools/overlay_atlas.py`, is the canonical projection of the shipped tables.
It records every module's ROM and section ranges, BSS size, entry points,
exports, resident callers, imports, relocation census, graph edges, and a
transparent campaign priority. `gmake overlay-atlas` regenerates both the JSON
and the marked yaml block in memory and fails on drift; only
`gmake overlay-atlas-write` updates them.

The measured totals are:

| Surface | Total |
|---|---:|
| headers / non-empty modules | 107 / 106 (overlay 32 is empty) |
| text | 469,264 bytes |
| initialized `data_rodata` | 61,312 bytes |
| BSS | 77,680 bytes |
| module relocation records | 18,542 |
| cross-overlay relocations / directed edges | 608 / 97 |

The header has one initialized-data size, not separate `.data` and `.rodata`
sizes. The atlas therefore calls that range `data_rodata`; inventing a split
would turn a missing fact into false precision. Each non-empty module is a
Splat `code` segment with text emitted as assembly and the initialized and
relocation tails preserved as binary until their internal ownership is known.

All shipped `vramBase` values are zero, so the build uses `0xF0000000` as a
**synthetic link VMA**, never as a claim about a runtime load address. Every
module shares one `exclusive_ram_id`: Splat uses that ID to hide symbols from
other mutually exclusive segments. Giving every overlay a distinct ID leaks
one overlay's local labels into another. `subalign: 1` is equally deliberate;
the relocation tails are only 8-byte aligned, and the linker's default
16-byte input alignment inserts bytes that do not exist in the ROM.

The generated identity is `(overlay, section, byte offset)`, reflected in
labels such as `func_overlay_061_...`; a bare synthetic VMA is not unique
because all modules overlap there. This representation links all 106
non-empty modules and reproduces the full US ROM byte-for-byte while leaving
the progress denominator unchanged.

`config/overlay-donors.us.json` is the companion evidence ledger. It checks
every overlay against pinned DKR v77, DKR v80, and JFG object builds, recording
strong, ambiguous, empty, and negative results. The first reusable findings
are DKR's unique `alSeqFileNew` at overlay 5 offset zero; JFG's named
`refractOutput` in overlay 49; and JFG's whole-text `osRamTest4_6105` match for
overlay 107. Placeholder-only matches remain placeholders. Overlay 61 has a
semantic DKR crosswalk to ghost/Controller Pak code, but no exact-byte match,
so it is explicitly a workflow lead rather than adopted identification.

Four pilot shapes are retained in the atlas: overlay 107 (whole-module donor),
103 (text plus one relocation table), 76 (text, initialized data, BSS, and both
relocation tables), and 61 (DKR semantic crosswalk). Together they exercise
every structural case without pretending that generated assembly is C.

### 5.5 Epoch 3 reviewed maps and matched ownership

The first overlay matching tranche owns exactly 2,000 text bytes. Ownership is
stored as explicit half-open offset ranges in the atlas, rather than inferred
from synthetic ELF symbols:

| Overlay | Matched ranges | Bytes | Evidence / disposition |
|---:|---|---:|---|
| 5 | `0x000`–`0x2E4` | 740 | DKR exact `alSeqFileNew`; DKR `bnkf.c` source/flag crosswalk for the remaining audio-bank patcher |
| 6 | `0x000`–`0x01C` | 28 | complete dependency-free neighborhood; three function boundaries, four padding bytes excluded |
| 14 | `0xB40`–`0xB5C` | 28 | exact JFG placeholder body, retained under a neutral Mickey name |
| 49 | `0x354`–`0x374` | 32 | exact named JFG `refractOutput`; the three alignment nops are still assembly |
| 72 | `0x000`–`0x0B4` | 180 | independently matched initializer after negative exact scans |
| 76 | `0x000`–`0x114` | 276 | complete three-function structural pilot; 12 padding bytes excluded |
| 78 | `0x000`–`0x0A8` | 168 | complete two-function leaf; eight padding bytes excluded |
| 93 | `0x000`–`0x01C` | 28 | initializer only; update remains assembly |
| 102–105 | function text | 472 | four one-function leaves; eight combined padding bytes excluded; exact donor scans negative |
| 106 | `0x000`–`0x008` | 8 | tier-A `osRamTest3_6105`; complete text matches JFG o144, with eight padding bytes excluded |
| 107 | `0x000`–`0x028` | 40 | exact named JFG `osRamTest4_6105` donor; eight padding bytes excluded |
| **total** | | **2,000** | matched C only; no generated-assembly or padding credit |

Overlay 5 is compiled at `-O3 -mips2 -32`. DKR established both the shared
audio-bank source family and that optimisation level, but Mickey retains calls
that DKR's whole `bnkf.c` translation unit inlines. The six measured Mickey
source boundaries preserve those calls. The other adopted overlay translation
units use `-O2 -mips2 -32`. Where a source boundary is not 16-byte aligned,
`tools/trim_elf_section.py` removes only IDO's zero object padding and refuses
to discard a nonzero byte.

The four required pilot reviews are:

- **Overlay 107:** one function at `+0x000`, export table index 2001, no header
  entry point, resident caller, relocation, initialized data, BSS, or import.
  Its `0x28` function text is `osRamTest4_6105`; the final eight bytes are
  alignment padding.
- **Overlay 103:** one function at `+0x000`, export index 1356 and resident call
  site ROM `0x27138`; no header entry point, initialized data, BSS, or overlay
  import. Its five primary-table relocations are one resident `R_MIPS_26` call
  and two `HI16`/`LO16` pairs for reserved symbol `0xFFD` addend `0x14A4`.
  Its `0x6C` function text is matched C; the final four bytes are padding.
- **Overlay 76:** functions at `+0x000`, `+0x038`, and `+0x0D0`, exported at
  table indices 1240, 1286, and 1332 and called from resident ROM `0xB9E4`,
  `0xBE4C`, and `0x1C5E0`. Two primary-table `R_MIPS_26` records are the
  resident sound and random calls; twelve local `HI16`/`LO16` records prove the
  16-byte initialized range and 32-byte status BSS. It has no overlay imports,
  and its `0x114` function text is matched C; the final 12 bytes are padding.
- **Overlay 61:** thirteen reviewed boundaries at `+0x000`, `+0x1C0`,
  `+0x1DC`, `+0x3C0`, `+0x7C4`, `+0x968`, `+0xB84`, `+0x1578`, `+0x1648`,
  `+0x17B8`, `+0x18A0`, `+0x19B0`, and `+0x1A6C`. Its three exports are
  `+0x968`, `+0xB84`, and `+0x1578` (indices 1894–1896); it has no resident
  inbound call. Its 549 relocations comprise 160 primary and 389 secondary
  records: 160 `SYMBOL`, 373 `LOCAL`, 16 `JUMP`, with 19 `R_MIPS_32`, 158
  `R_MIPS_26`, and 186 each `HI16`/`LO16`. Fifty-four imports target overlay
  45 and two target overlay 68. The `0x2A0` initialized range and `0x5E0` BSS
  remain raw. Exact C now owns the contiguous `+0x000..+0xB84` prefix and
  `+0x1578..+0x1A84`; the
  intervening ranges remain assembly, followed by twelve bytes of padding.
  DKR's `save_data.c`,
  `racer.c`, and `menu.c` are a semantic navigation crosswalk only: the ghost
  and Controller Pak strings do not by themselves prove function names.

The closed dependency neighborhood is overlay 6. Its header init is `+0x000`,
its additional exports are `+0x008` and `+0x010`, and resident call sites at
ROM `0x3344`/`0x3A48` reach the latter two. It has no relocation table, data,
BSS, or imports, so all directly required symbols are local and the complete
`0x20` text can be owned without pulling in another module.

### 5.6 Rejected and deferred overlay candidates

The tranche deliberately records near misses so a later pass does not repeat
them:

- JFG's placeholder-only hits in overlays 5, 14, and 16 prove byte reuse but
  do not supply names. Overlay 14's exact body was adopted under a neutral
  name; overlay 5's placeholder region after `+0x2E4` and all of overlay 16
  remain assembly. A source reconstruction of the overlay 16 hit did not
  reproduce the object.
- Overlay 93's initially rejected donor-shaped update was revisited from
  Mickey's instructions. `-Wab,-r4300_mul` supplies the required R4300 hazard
  schedule, and the complete `0x0F0` text is now matched C.
- Overlay 39's earlier prefix register-allocation mismatch was closed with an
  otherwise-unused volatile reservation; the complete module is now matched
  C except padding. Overlay 95's main body is likewise exact and closes that
  module. Overlay 85's first 192 bytes are exact with
  `-Wab,-r4300_mul`; its 476-byte tail remains deferred on register colouring.
- Negative DKR v77/v80 and JFG scans for overlays 72, 76, 78, 93, and 102–106
  are retained as useful evidence: their matching C was derived from Mickey's
  own instructions and relocations, not attributed to a donor.

### 5.7 Epoch 4 tranche-B ownership and hub maps

The completed tranche adds 4,100 matched overlay bytes, raising explicit
overlay C ownership from 2,000 to 6,100 bytes:

| Overlay | New matched ranges | Bytes | Disposition |
|---:|---|---:|---|
| 1 | `0x0E4`–`0x154`, `0x69A0`–`0x6A14`, `0x6B28`–`0x6B6C` | 296 | wrapped offset plus two state initializers |
| 5 | `0x6C0`–`0x764` | 164 | object/player constructor tail; padding remains assembly |
| 21 | `0x000`–`0x10C` | 268 | plane/geometry registration routine |
| 23 | `0x208`–`0x468` | 608 | initializer and update; prefix/tail remain assembly |
| 24 | `0x000`–`0x01C` | 28 | initializer |
| 25 | `0x588`–`0x608` | 128 | vector copy and state-flag selection |
| 27 | `0x000`–`0x064` | 100 | initializer |
| 39 | `0x0C8`–`0x168` | 160 | reset and readback exports; prefix remains assembly |
| 56 | `0x0B8`–`0x10C`, `0xAB4`–`0xAF4` | 148 | integer time split and packed-colour unpack |
| 67 | `0x000`–`0x14C` | 332 | module complete except padding |
| 69 | `0x000`–`0x04C` | 76 | initializer |
| 72 | `0x0B4`–`0x168` | 180 | update tail; module complete except padding |
| 74 | `0x000`–`0x0B8` | 184 | initializer; update remains assembly |
| 77 | `0x3B8`–`0x430` | 120 | selection and callback tail exports |
| 81 | `0x000`–`0x0CC`, `0x0CC`–`0x220`, `0x220`–`0x34C` | 844 | full module except four-byte padding; 556 bytes are beyond the checkpoint |
| 88 | `0x000`–`0x04C` | 76 | initializer |
| 92 | `0x000`–`0x068` | 104 | initializer |
| 93 | `0x01C`–`0x0EC` | 208 | update; module complete except padding |
| 95 | `0x000`–`0x00C` | 12 | empty callback boundary |
| 97 | `0x1A8`–`0x1E8` | 64 | radius-squared initializer |
| **tranche total** | | **4,100** | matched C only; no padding/generated-assembly credit |

The overlay 81 row shows its complete `0x34C` C surface for clarity; its
epoch contribution is 844 bytes, of which the original checkpoint already
reported 288. Overlays 67, 81, and 93 have no cross-overlay imports and retain
only alignment padding. Together with complete dependency-free overlays
102–107, the exit state contains nine dependency-free leaves. Overlay 72 is
also complete except padding but has one recorded import from overlay 8, so it
is not included in that count.

All new code uses measured `-O2 -mips2 -32`. Overlay 39 additionally needs
`-Wo,-loopunroll,0`; overlays 21, 23, 74, 81, 93, and 97 use
`-Wab,-r4300_mul` where their instruction schedules require it. No new
initialized-data or BSS slice was claimed: partial-module data remains raw,
and the absolute symbols used by C are relocation addends rather than invented
ownership boundaries.

Fresh pinned DKR v77/v80 and JFG object scans reproduce the donor ledger and
are negative for new exact attribution. DKR source was also searched first for
each routine's constants, structure effects, and control shape. The known JFG
overlay 5 placeholder at `+0x2E4` remains assembly: its shared instructions do
not reproduce Mickey's stack frame from plausible C.

#### Overlay 61's overlay 45/68 API surface

Relocation decoding accounts for all 56 cross-overlay records, with no
unresolved target:

| Target | Overlay 61 call sites | Count | Proved neutral contract |
|---|---|---:|---|
| overlay 45 `+0x00C` | `+0x994` through `+0xB14`, thirteen calls | 13 | accepts a data pointer plus numeric descriptor fields, allocates/initializes a linked resource descriptor and returns its handle (or zero) |
| overlay 45 `+0x270` | `+0x1590` through `+0x1620`, twelve-byte stride | 13 | finds a supplied descriptor in overlay 45's linked list, unlinks it through field `+0x30`, and releases storage at `+0x2C` |
| overlay 45 `+0x1BE0` | 28 sites from `+0xBB8` through `+0x1358` | 28 | null-guarded byte setter for descriptor field `+0x1D` |
| overlay 68 `+0x000` | `+0x1708`, `+0x1724` | 2 | returns constant `0x2EF0`; the paired calls clamp a `CHAR`-tagged payload-copy length to that bound |

Epoch 5's complete inbound census groups all 298 overlay 45 relocations and all
six overlay 68 relocations by target and caller. Counts are relocation records,
not inferred source calls:

| Overlay 45 target | Inbound total | Caller overlays (count) |
|---:|---:|---|
| `+0x000` | 1 | 10 (1) |
| `+0x00C` | 71 | 11 (35), 47 (5), 48 (1), 50 (1), 52 (1), 54 (1), 57 (12), 61 (13), 62 (1), 63 (1) |
| `+0x270` | 50 | 11 (10), 47 (5), 48 (2), 50 (2), 52 (2), 54 (2), 57 (12), 61 (13), 62 (1), 63 (1) |
| `+0x314` | 73 | 47 (8), 57 (65) |
| `+0x640` | 1 | 47 (1) |
| `+0x1BE0` | 84 | 11 (14), 48 (2), 50 (3), 52 (3), 54 (3), 57 (26), 61 (28), 62 (3), 63 (2) |
| `+0x1BF4` | 18 | 11 (14), 47 (4) |
| **total** | **298** | |

| Overlay 68 target | Inbound total | Caller overlays (count) |
|---:|---:|---|
| `+0x000` | 2 | 61 (2) |
| `+0x4E4` | 1 | 1 (1) |
| `+0x146C` | 3 | 57 (1), 58 (1), 60 (1) |
| **total** | **6** | |

The converted overlay 61 edges have the following ABI and field-level map.
No stack arguments occur on these four calls:

| Target | Arguments / return | Nullability and observed effects |
|---|---|---|
| overlay 45 `+0x00C` | `a0` text/data pointer, `a1` signed width, `a2` signed height, `a3` flags; nullable descriptor in `v0` | Allocation failure returns zero. Success initializes allocation/list links `+0x2C/+0x30`, flags `+0x08`, dimensions `+0x18/+0x1A`, count `+0x1C`, mode `+0x1D`, element/string pointers `+0x24/+0x28`, and per-element records; overlay 61 stores each return in handles `+0x58..+0x88`. |
| overlay 45 `+0x270` | nullable descriptor in `a0`; no used return | A null argument is a no-op. Otherwise it searches the global list, rewrites the head or predecessor `+0x30` link, and frees the target allocation at `+0x2C`. |
| overlay 45 `+0x1BE0` | nullable descriptor in `a0`, mode in `a1`; no used return | A null argument is a no-op; otherwise the low byte of `a1` is stored at descriptor `+0x1D`. |
| overlay 68 `+0x000` | no consumed argument; `0x2EF0` in `v0` | Pure constant accessor. Overlay 61 compares its pending copy length with the return and clamps before the resident copy path. |

Overlay 45 itself has ten exports, one resident caller, no imports, 298
cross-overlay inbound relocations, `0x80` initialized bytes, and `0x10` BSS.
Overlay 68 has eighteen exports, fifteen resident call sites, no imports, six
cross-overlay inbound relocations, `0x40` initialized bytes, and no BSS. Those
whole-module facts and the field effects above are enough to type overlay 61's
edges; neither hub's bulk text is claimed as C or given a stronger semantic
name.

#### Overlay 61 export `+0x968`

The `0x21C`-byte export is a module resource/configuration initializer. After
two resident calls, it invokes overlay 45 `+0x00C` thirteen times with local
data records at `+0xD0`, `+0xDC`, `+0xE8`, `+0xF4`, `+0x100`, `+0x10C`,
`+0x114`, `+0x120`, `+0x12C`, `+0x130`, `+0x134`, `+0x13C`, and `+0x144`.
The returned handles are retained in module globals `+0x58` through `+0x88`.
It then copies two resident configuration words (`+0x10` and `+0x14`) into
module globals `+0x08` and `+0x30`, initializes state words `+0xA4`–`+0xB0`
to `2, 0, 2, 7`, and finishes with a resident call using argument zero.

That narrative proves resource allocation/registration and state setup. It
does not prove DKR's ghost or Controller Pak identities, so those remain a
semantic navigation crosswalk and no borrowed name is adopted.

#### Overlay 61 export `+0xB84`

The `0x9F4`-byte export is a nine-state controller/menu state machine. It
begins by calling local `+0x000` to derive four directional/confirm/cancel-like
outputs, clears two emission markers, and uses overlay 45 `+0x1BE0` 28 times
to reset or enable thirteen descriptor handles. The initialized-data jump
table at `+0x278` maps state word `+0xA4` to cases 0 through 8 at `+0xCB0`,
`+0x00000D98`, `+0x00000DE8`, `+0x00001080`, `+0x00001168`, `+0x00001254`, `+0x00001334`, `+0x00001414`,
and `+0x1428`.

- State 0 selects one of handles `+0x6C/+0x70/+0x74` from selection word
  `+0xA8`, enables `+0x80/+0x84`, and on confirm/cancel copies the saved
  `+0xAC/+0xB0` state before emitting resident actions `0xC/0xD`.
- State 1 enables `+0x68`; either decision advances to state 2 with action
  `0xC`. State 2 performs the resident query at `+0x2D0D8`. Success installs
  `(state, selection, back, next) = (0, result, 2, 7)`. Failure resets the
  three counters through exact helper `+0x1C0`, scans sixteen records at
  `+0xC0`, probes them through local `+0x18A0`, and registers modes 4/6 via
  local `+0x1DC`. Optional object sizing through `+0x1A6C` is rounded to the
  next `0x100` before mode 5 is registered; a resident `+0x2D408` result is
  retained at global `+0x94` and registered as mode 7. The case ends in
  state 3.
- State 3 handles cancel to state 7/action `0xD`, confirm through the selected
  record's `+0xD0` field/action `0xC`, and wraps counter `+0x9C` against count
  `+0x98` on signed vertical input with action `0xF`.
- States 4, 5, and 6 enable descriptor sets `+0x5C/+0x78/+0x7C`,
  `+0x60/+0x78/+0x7C`, and `+0x64/+0x78/+0x7C`. Their confirm paths use local
  helpers `+0x1648`, `+0x19B0` then `+0x17B8`, and resident `+0x2D51C`;
  success installs tuples `(0,result,4,2)`, `(0,result,5,2)`, or
  `(0,result,6,2)`, while failure returns to state 1. Cancel returns to the
  appropriate prior state.
- State 7 calls resident `+0x288E0(0)` and advances to state 8. State 8 enters
  the common tail directly. The tail optionally calls local `+0x7C4`, invokes
  resident `+0x36354` twice for configuration, scales/truncates floats by
  `0x47800000` into config `+0x08` and global `+0x38`, and emits either marked
  parameter block through resident `+0x2F1C8`.

Its direct dependency list is complete: overlay 45 `+0x1BE0` (28 calls);
resident `+0xB44` (13), `+0x2D0D8` (1), `+0x2D408` (1), `+0x2D51C` (1),
`+0x288E0` (1), `+0x36354` (2), and `+0x2F1C8` (2); local `+0x000`,
`+0x1C0`, `+0x1DC` (5), `+0x7C4`, `+0x1648`, `+0x17B8`, `+0x18A0`,
`+0x19B0`, and `+0x1A6C`; plus five reserved-symbol HI/LO pairs to
overlay 4093 `+0x1494` and three to overlay 4095 `+0x4D700`. The trace proves
the state and descriptor effects above, not a ghost or Controller Pak name.

#### Rejected/deferred candidates

Near matches for overlay 24's update/render routines, overlay 60's quad draw,
overlay 83's mesh draw, and initializers in overlays 82, 84, and 91 were
removed from source and atlas ownership. They had plausible control flow or
exact footprint but unresolved register allocation/scheduling differences.
Overlay 1 `+0x154` likewise remains assembly after a two-register allocation
swap. These ranges receive no progress credit and are the explicit next
compiler-level boundaries rather than parked non-matching C.

---

### 5.8 Epoch 5 execution ledger

Epoch 5's exact-only pass currently contributes **6,376 overlay text bytes**.
This raises overlay C ownership from 6,100 to 12,476 bytes. The mandatory
1,344-byte semantic spine is complete, and overlay 61 `+0x1C0` contributes an
additional 28-byte helper proved by the `+0xB84` trace. No padding, data, BSS,
generated assembly, exact-size-only body, or semantic near match is included.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 37 | `+0x000..+0x088`, `+0x4F4..+0x558` | 236 | negative |
| 39 | `+0x000..+0x0C8` | 200 | negative; module closed with its existing tail |
| 40 | `+0x2E4..+0x690` | 940 | timed interpolation is semantically related to DKR weather shifting; the tint rectangle matched through SDK display-list macro scoping, with no exact donor; `+0x0E8` remains assembly after a two-instruction startup scheduling mismatch |
| 42 | `+0x000..+0x0F4`, `+0x6A4..+0x700` | 336 | semantic display-list lifecycle / framebuffer swap; the unresolved renderer resembles `screenimage_draw` |
| 45 | `+0x00C..+0x314`, `+0x1BE0..+0x1BF4` | 796 | negative for allocator, release, and setter shapes |
| 56 | `+0x000..+0x0B8`, `+0x10C..+0x1A0` | 332 | generic setter/viewport idioms; unresolved minimap has a strong DKR HUD semantic lead |
| 61 | `+0x1C0..+0x1DC`, `+0x968..+0xB84` | 568 | negative; nearby DKR ghost/Controller Pak code is navigation only |
| 68 | `+0x000..+0x008` | 8 | negative |
| 75 | `+0x000..+0x214`, `+0x6D4..+0x6F8` | 568 | initializer exact-negative; leaf has only generic reference-array assignment similarity |
| 77 | `+0x000..+0x130` | 304 | DKR scenery/object radius clamp and divide is a strong semantic source lead |
| 82 | `+0x498..+0x4CC` | 52 | negative except generic setter reuse |
| 85 | `+0x000..+0x0C0` | 192 | negative |
| 95 | `+0x00C..+0x1D8` | 460 | DKR HUD audio has a related timed handle/volume ramp, not an exact donor; module closed |
| 96 | `+0x57C..+0x5C8` | 76 | negative |
| 97 | `+0x130..+0x1A8`, `+0x1E8..+0x3F4`, `+0x420..+0x508`, `+0x748..+0xA54` | 1,656 | negative for adopted bodies; DKR `obj_init_scenery` is a semantic lead for the unresolved scale initializer |
| **total** | | **6,376** | no newly adopted exact object donor |

The closure cohort supplied 2,864 bytes, the semantic spine and its extra
overlay 61 helper supplied 1,372, and the measured fallback ladder supplied
2,140: overlay 40 (592), overlay 56 (332), overlay 42 (336), overlay 37
(236), overlay 96 (76), and overlay 75 (568). These substitutions replace
blocked primary boundaries explicitly; they do not imply that the primary
modules closed.

Only overlays 39 and 95 reached full cohort closure. The other seven closure
targets were bounded and left as assembly where exactness failed: overlay 74
has one 400-byte, six-word register-allocation mismatch with its instruction
schedule aligned; overlay 85 has a 476-byte
timer/trigger register-colouring mismatch; overlay 23 retains indivisible
520- and 256-byte bodies; overlay 77 retains one 648-byte projectile body;
overlay 24 retains 616- and 400-byte bodies; overlay 82 retains its 64-byte
initializer and 1,112-byte main routine; and overlay 97 retains 304-, 44-, and
576-byte bodies. The 44-byte overlay 97 state helper is the narrowest miss:
10 of 11 words match, but IDO saves `a1` rather than the target's `a2` in the
call delay slot, so it receives no credit.

Compiler settings remain narrow. All adopted game C uses `-O2 -mips2 -32`;
overlay 39's prefix additionally uses `-Wo,-loopunroll,0`, while overlay 77's
initializer, overlay 75's initializer, overlay 85's configuration routine,
and the affected overlay 97 math bodies use
`-Wab,-r4300_mul`. Every absolute symbol in partial modules is a measured raw
relocation addend. Nonexact experiment sources, Make rules, and placeholder
aliases are removed after each bounded pass.

The hard 8,192-byte and six-module closure exits are therefore **not yet
satisfied**: 1,816 more exact bytes and four more cohort closures are required.
The semantic-spine, second-overlay-61-function, `+0xB84` trace, inbound
histogram, and ABI-map exits are satisfied. This is an execution checkpoint,
not an Epoch 5 completion claim.

### 5.9 Epoch 6 exact-leaf recovery ledger

Epoch 6 adds **1,040 exact non-padding overlay text bytes**, raising overlay C
ownership from 12,476 to **13,516 bytes** and whole-program resolved code from
55,128 to **56,168 / 949,944 bytes (5.91%)**. Every range below was compiled
with the overlay default `-O2 -mips2 -32` except the measured overlay 97
direction initializer noted below, linked with its measured addends, and
compared in the complete US ROM. No generated alignment padding is credited.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 8 | `+0x0000..+0x0008`, `+0x49A4..+0x49E8` | 76 | generic no-op, buffer, and scalar access patterns only; no donor |
| 9 | `+0x10A4..+0x10B4` | 16 | generic no-op only; no donor |
| 14 | `+0x31C..+0x328`, `+0xACC..+0xAF8` | 56 | generic flag access/reset patterns only; no donor |
| 15 | `+0x000..+0x00C`, `+0x6A4..+0x6B0`, `+0xB7C..+0xB94` | 48 | generic resource/scalar accessors only; no donor |
| 34 | `+0x0C8..+0x0D4` | 12 | generic scalar setter only; no donor |
| 41 | `+0x172C..+0x1740` | 20 | generic no-op only; no donor |
| 45 | `+0x1BF4..+0x1C1C` | 40 | generic null-guarded byte setters only; no donor |
| 61 | `+0x1A6C..+0x1A84` | 24 | negative for the record-size calculation |
| 66 | `+0x034..+0x040` | 12 | generic accessor only; no donor |
| 79 | `+0x1280..+0x1290`, `+0x147C..+0x149C` | 48 | generic object-state assignments only; no donor |
| 84 | `+0x00000DBC..+0x00000DD0`, `+0x00001004..+0x00001060`, `+0x00001294..+0x000012B4`, `+0x00001350..+0x000013BC` | 252 | generic state accessors/setters only; no donor |
| 86 | `+0x444..+0x474` | 48 | generic object initializer only; no donor |
| 94 | `+0x55C..+0x568` | 12 | generic scalar setter only; no donor |
| 97 | `+0x000..+0x130`, `+0x3F4..+0x420` | 348 | DKR `obj_init_scenery` supports only the direction routine's scale-prefix semantics; no exact donor or name evidence |
| 101 | `+0x1BB4..+0x1BD0` | 28 | generic three-global reset only; no donor |
| **total** | | **1,040** | pinned DKR v77/v80 ledger remains exact-donor negative for every adopted module |

Overlay 79's unresolved `+0x1290..+0x147C` owner
(`func_overlay_079_F0001290_18CE230`) remains `NON_MATCHING` at 492 bytes / 123
words. Retained configured-flag isolated C is 111/123 words with frame `0x48`
and 12 register-only sites from `+0xC8` at the historical baseline. Explicit
`u8` width preservation with `(value & 0xFF) | 4` consumes one backend temp
without emitting an instruction; the retained C is now 119/123 words with four
register-only sites, and its integer temp lane is exact. Opaque-pointer and
zero-offset typed state-slot follow-ups were byte-flat, leaving only the
linked-state v1/v0 pool web. All 119 flag identities remain nonexact. Its 15 runtime-
backed records include eleven calls, a local counter pair, and the reserved
loader flags pair; an identity-canonical comparison proves all 15 offsets, types,
identities, and addends. The assembled target retains only 13 static records
because the runtime table supplies the already-zero flags instructions. The
committed filter/rebind metadata is diagnostic and is not part of production
`POSTPROCESS`. Exact linked range/module/full-ROM evidence proves the assembly
fallback only; no linked C candidate survives. Pinned DKR v77/v80 and JFG
scans are negative.

Overlay 41's `+0x000..+0x124` owner
(`func_overlay_041_F0000000_1887338`) is exact C: 292 bytes / 73 words with the
retail `0x30` frame. Its object carries stored-zero proxies at `+0x14/+0x28`
and `+0xD0`; the runtime table resolves them to `D_800D6B58` and resident
`func_8000D16C`. The owned range, linked module, and full ROM are byte-identical
with only trailing section-alignment trimming. Pinned DKR v77/v80 and JFG scans
remain negative; JFG's `animseqUpdateTextureScrollers` is a role-only
comparison, not a donor.

Overlay 41's `+0x2AC..+0x7FC` curve sampler is exact C: 1,360 bytes / 340
words with the retail `0x58` frame. Keeping each coefficient's leading source
term last and spelling the derivative scale as `coefficient2 * 3.0f` restores
IDO's late floating-point evaluation and temporary schedule. The compiler's
64-byte duplicate jump table and literal pool are authenticated by digest,
then externalized while all 20 text relocations are rebound to the retained
initialized-data base without changing instruction words or addends. The
unchanged runtime table plus the exact linked range prove all 20 effective
identities; the owned range, complete module, and full US ROM are exact.

Overlay 41's `+0x1740..+0x195C` item spawner is exact C: 540 bytes / 135
words with the retail `0xA0` frame. Four compiler-private relocations are
rebound, without changing instruction fields, to the retained jump-table and
scalar pool beginning at initialized-data `+0x58`; the discarded 32-byte
duplicate is guarded by SHA-256. All 11 runtime relocation offsets/types are
exact. Ten identities are explicit in object/link metadata, while the shipped
runtime record plus exact linked bytes prove the final call as Overlay 12
`+0x1B4`. The owned range, linked module, and full US ROM are exact. This
replaces the retired wrapper that edited two LO16 fields after compilation.

Two old compiler blockers were closed rather than merely bypassed. Overlay 97
`+0x000` required preserving explicit unsigned-byte angle masks plus
`-Wab,-r4300_mul`; `+0x3F4` required the original three-argument ABI shape,
whose otherwise-unused third argument produces the target `a2` home store in
the call delay slot. The latter was Epoch 5's one-word-near function.

The remaining overlay 82 initializer experiment was removed. IDO consistently
folds the target's explicit `li`/`sll`/`addu` address calculation into one
`addiu`; volatile variants spill and are farther away. It stays assembly and
receives no credit. A stale generated object initially exposed why linked
validation is mandatory: its untrimmed alignment tail shifted the next
subsegment, while rebuilding with the committed exact-size trim restored the
full ROM. The final image is byte-identical at SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

The fresh 107-overlay scan covers both pinned local DKR revisions and JFG.
DKR v77/v80 each remain `1 strong / 1 ambiguous / 104 none / 1 empty`; none of
the Epoch 6 targets is the strong or ambiguous entry. Source-level similarities
above are deliberately classified as semantic or generic and do not promote
DKR names.

### 5.10 Epoch 7 leaf and wrapper ledger

Epoch 7 adds **508 exact non-padding bytes**, raising overlay C from 13,516 to
**14,024 bytes** and whole resolved code from 56,168 to **56,676 bytes**.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 1 | `+0x00005BA4..+0x00005BC0`, `+0x00006788..+0x000067C0`, `+0x00007FCC..+0x00008008` | 144 | generic state initialization, byte copy, and predicates only |
| 14 | `+0xB34..+0xB40` | 12 | generic scalar getter only |
| 15 | `+0x00C..+0x04C` | 64 | generic resource release only |
| 20 | `+0xE0C..+0xE28` | 28 | generic nested-state mark only |
| 33 | `+0x17C..+0x19C`, `+0x708..+0x728` | 64 | generic zero-argument wrappers only |
| 36 | `+0x1470..+0x14B0` | 64 | generic mode wrappers only |
| 46 | `+0x112C..+0x1150` | 36 | generic submit wrapper only |
| 65 | `+0xBC0..+0xBF0` | 48 | generic double-release wrapper only |
| 101 | `+0xCEA8..+0xCED8` | 48 | byte-string loop; no donor |
| **total** | | **508** | no exact DKR donor or adopted DKR name |

Every row uses the overlay default `-O2 -mips2 -32`. The relocation-bearing
wrappers were verified in the complete linked image, not merely by comparing
unresolved objects. The adjacent overlay 101 `+0xCED8..+0xCEE0` and overlay 33
`+0x728..+0x730` zero tails are padding and receive no credit.

### 5.11 Epoch 8 accessor and resource-wrapper ledger

Epoch 8 adds **436 exact non-padding bytes**, raising overlay C from 14,024 to
**14,460 bytes** and whole resolved code from 56,676 to **57,112 bytes**.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 15 | `+0x6B0..+0x6E8` | 56 | generic resource release only |
| 63 | `+0x74C..+0x77C` | 48 | generic double-release wrapper only |
| 84 | `+0x000..+0x048`, `+0xC74..+0xC9C`, `+0xFC4..+0x1004`, `+0x12B4..+0x1350` | 332 | generic object-state access and initialization only |
| **total** | | **436** | no exact DKR donor or adopted DKR name |

Overlay 84's two longer queries required sequential early returns to preserve
the target branch-likely duplication. A 72-byte overlay 65 reset reached the
correct loop under `-Wo,-loopunroll,0` but retained a different relocation and
register schedule; it was removed. No nonexact Epoch 8 source or alias remains.

### 5.12 Epoch 9 overlay 68 lifecycle ledger

Epoch 9 adds **524 exact non-padding bytes**, raising overlay C from 14,460 to
**14,984 bytes** and whole resolved code from 57,112 to
**57,636 / 949,944 bytes (6.07%)**.

| Overlay | Newly matched ranges | Bytes | DKR-first result |
|---:|---|---:|---|
| 68 | `+0x008..+0x148`, `+0x484..+0x51C`, `+0x1438..+0x146C` | 524 | lifecycle/allocation semantics are generic; no exact donor |
| **total** | | **524** | pinned DKR v77/v80 row remains `none` |

The first range contains two 108-byte allocators followed by two 52-byte
release wrappers. The middle range clears a nested flag, closes an active
entry while advancing its generation, and arms a timer. The last range is a
third release wrapper. All calls and zero-addend globals were checked after
linking, and the complete image retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

Epochs 7–9 contribute **1,468 bytes** together. The original Epoch 5 byte exit
is now satisfied at **8,884 / 8,192**, 692 bytes over target. Its separate
module-closure exit remains **2 / 6**, so four cohort closures still block the
campaign's final completion claim.

### 5.13 Epoch 10 execution checkpoint

Epoch 10's current checkpoint adds **16,708 exact non-padding bytes**, raising
overlay C from 14,984 to **31,692 bytes** and whole-program resolved code from
57,636 to **74,344 / 949,944 bytes (7.83%)**. This remains an active-campaign
checkpoint rather than the 10.00% exit.

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| `+0x108..+0x164` | find the requested occurrence in a linked entry chain | 92 |
| `+0x164..+0x258`, `+0x344..+0x438`, `+0x948..+0xA3C`, `+0xB28..+0xC1C`, `+0x11E0..+0x12D4` | timed two-coordinate interpolators | 1,220 |
| `+0x258..+0x344`, `+0x438..+0x524` | allocate and schedule two-coordinate interpolation | 472 |
| `+0x524..+0x5E0` | activate/advance slot state | 188 |
| `+0x668..+0x708`, `+0x7D8..+0x878` | timed byte interpolators | 320 |
| `+0x708..+0x7D8`, `+0x878..+0x948` | allocate and schedule byte interpolation | 416 |
| `+0xD08..+0xD80` | timed scalar interpolation | 120 |
| `+0xE54..+0xEF4` | timed delta interpolation | 160 |
| `+0xFF4..+0x110C` | timed unsigned-byte interpolation | 280 |
| `+0x13C0..+0x1558` | four-channel color interpolation | 408 |
| `+0x1868..+0x1970` | timed global coordinate interpolation | 264 |
| `+0x2CE4..+0x2DC0` | construct four intensity/color words | 220 |
| **total** | **20 functions** | **4,160** |

The next scheduler/lifecycle wave adds another 4,864 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| overlay 101 `+0x5E0..+0x668` | promote a pending slot into the active state | 136 |
| overlay 101 `+0x00000A3C..+0x00000B28`, `+0x00000C1C..+0x00000D08`, `+0x00000D80..+0x00000E54`, `+0x00000EF4..+0x00000FF4`, `+0x0000110C..+0x000011E0`, `+0x000012D4..+0x000013C0`, `+0x00001558..+0x00001678` | allocate linked pair, scalar, byte, scaled, and color transitions | 1,676 |
| overlay 101 `+0x00001678..+0x00001728`, `+0x00001728..+0x00001868`, `+0x00001970..+0x00001A38` | update and schedule frame/global-pair transitions | 696 |
| overlay 101 `+0x000036E4..+0x00003814`, `+0x00003814..+0x00003998`, `+0x00003998..+0x00003A58` | draw and update presentation chains and slots | 884 |
| overlay 101 `+0xCBDC..+0xCD50`, `+0xCD50..+0xCEA8` | update presentation state and release completed work | 716 |
| overlay 68 `+0x21C..+0x2E0`, `+0x2E0..+0x484`, `+0x8E0..+0x96C` | attach an object, update its trail, and initialize object state | 756 |
| **subtotal** | **19 functions** | **4,864** |

The closure and volume lanes add 1,536 more bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| overlay 85 `+0x0C0..+0x29C` | update the configured countdown/timer and emit the triggered effect | 476 |
| overlay 1 `+0x000..+0x050`, `+0x080..+0x0E4` | ring-pointer and entry-index helpers | 180 |
| overlay 1 `+0x000002D4..+0x00000378`, `+0x000010C0..+0x000010C8`, `+0x00001D58..+0x00001D78`, `+0x00003E48..+0x00003FD8`, `+0x00005ECC..+0x00005ED4`, `+0x00008008..+0x00008114` | linked-record filters, generic no-op, call wrappers, sampled/relative-angle and distance helpers, and zero-return leaf | 880 |
| **subtotal** | **18 functions** | **1,536** |

The next overlay 101 control-flow wave adds 560 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| `+0x000..+0x0B4` | initialize configuration, selection, and resource state | 180 |
| `+0x1A38..+0x1BB4` | dispatch 40 active entries across thirteen exact update routines | 380 |
| **subtotal** | **2 functions** | **560** |

Overlay 16 contributes a further 340 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| `+0x08C..+0x1A8` | allocate and populate four 64-step RGB gradient bands | 284 |
| `+0x1A8..+0x1E0` | release the gradient buffer and clear its owner | 56 |
| **subtotal** | **2 functions** | **340** |

The compact-function follow-up contributes 3,524 bytes:

| Newly matched ranges | Role | Bytes |
|---|---|---:|
| overlays 45 and 66 | state reset, pair accessor, and selection update | 124 |
| overlays 8, 14, and 79 | indexed selection, resource release, and timer update | 208 |
| overlays 2, 13, 83, and 99 | mode/color wrappers, two-stage submit, and entry cleanup | 236 |
| overlays 17, 55, 58, 62, and 87 | exact resource-lifecycle wrappers | 296 |
| overlays 50–55 | six instruction-identical index-to-pointer patch loops | 480 |
| overlays 54 and 55 | copy linked records with table-selected offsets and return the selected offset pair | 800 |
| overlay 91 | object/state initializer | 76 |
| overlays 11, 46, and 51 | handle cleanup and state lifecycle wrappers | 252 |
| overlay 11 grouped-release family | six exact fixed-span release/finalize loops | 600 |
| overlay 29 | select parallel table entries and rotate the four active values in both directions | 452 |
| overlays 3, 28, 35, 36, 48, 59, and 100 | queue/vector/state helpers, fixed-list lifecycle, buffer reset, and compact record selectors | 1,724 |
| **subtotal** | **56 functions** | **5,248** |
| **Epoch 10 total** | **117 functions** | **16,708** |

The repeated functions were not credited by resemblance. Each TU was compiled
at the overlay default `-O2 -mips2 -32`, trimmed to its proven boundary, and
compared word-for-word. The complete linked ROM exposed one integration error:
local calls in the stored image retain a zero J target for the runtime overlay
relocator, so assigning their eventual local offsets changed ten bytes. Giving
the C call aliases the correct stored zero target restored the exact US SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

The refreshed pinned scan remains `1 strong / 1 ambiguous / 104 none / 1
empty` for both DKR v77 and v80 and `5 / 5 / 96 / 1` for JFG. Overlay 101 is
`none` in all three ledgers; linked-list traversal, fixed-pool allocation, and
timed interpolation are semantic patterns only, so no donor name was adopted.
Overlay 85 is now fully owned by exact C except its final four-byte alignment
pad. That supplies **1 / 4** of Epoch 10's closure exit and moves the cumulative
cohort ledger to **3 / 6**. The hard Epoch 10 byte exit has **20,651 bytes
remaining**, and three more cohort modules are still required.

Later linked-exact waves raise the same active Epoch 10 checkpoint to **22,788
new non-padding bytes**, **37,772 / 469,264 overlay C bytes (8.05%)**, and
**80,424 / 949,944 whole-program resolved bytes (8.47%)**. The newest ranges
are overlay 13 `+0x124..+0x188` (100 bytes), which releases three optional
resources and clears the active word, and overlay 59 `+0x168..+0x1D4` (108
bytes), which resets the fixed entry table. Both were compared word-for-word
after relocation resolution. The hard Epoch 10 byte exit now has **14,571
bytes remaining**; the module-closure exit remains **1 / 4**.

A further lifecycle pass adds **496 exact non-padding bytes**: overlay 20's
release-handle and release-entry functions (116 bytes), overlay 44's four-slot
release loop (112), overlay 18's initializer (88), overlay 33's global release
wrapper (56), and overlay 86's current-entry processor (124). The linked
checkpoint reaches **24,284 new Epoch 10 bytes**, **39,268 / 469,264 overlay C
bytes (8.37%)**, and **81,920 / 949,944 whole-program resolved bytes (8.62%)**.

Overlay 27's `+0xB68..+0xBC0` activation transition adds **88 exact bytes**.
All 22 instruction words were independently compared before atlas adoption;
the linked ROM remains byte-identical. The live checkpoint is therefore
**24,372 new Epoch 10 bytes**, **39,356 / 469,264 overlay C bytes (8.39%)**,
and **82,008 / 949,944 whole-program resolved bytes (8.63%)**. The hard 10.00%
exit has **12,987 bytes remaining**.

Overlay 65's `+0x000..+0x080` fixed-pool initializer adds another **128 exact
bytes**. Its complete record-clear loop, both allocation calls, the local reset
call, and all linked data addends compare word-for-word; pinned DKR v77/v80 and
JFG scans remain donor-negative for the module. The live checkpoint is now
**24,500 new Epoch 10 bytes**, **39,484 / 469,264 overlay C bytes (8.41%)**,
and **82,136 / 949,944 whole-program resolved bytes (8.65%)**. The hard 10.00%
exit has **12,859 bytes remaining**.

Overlay 20's `+0x000..+0x07C` recursive three-child release adds **124 exact
bytes**. Its null path, branch-likely child traversal, recursive local calls,
field clears, and final release call all compile exactly on the first recovered
source shape. The checkpoint advances to **24,624 new Epoch 10 bytes**,
**39,608 / 469,264 overlay C bytes (8.44%)**, and **82,260 / 949,944
whole-program resolved bytes (8.66%)**. The hard 10.00% exit has **12,735
bytes remaining**.

The next compact cluster adds **1,012 exact bytes**. Overlay 13's
`+0x508..+0x580` active-record walker contributes 120 bytes. Overlay 84 adds
the adjacent `+0xA54..+0xAFC` input-gated resource update (168) and
`+0xAFC..+0xB7C` current-resource reset (128). Overlay 11 adds its
`+0xA18..+0xAF4` four-handle creator (220), `+0x1058..+0x1130` enable
transition (216), and `+0x1130..+0x11D0` disable transition (160). Every range
was linked at its real overlay offset and the complete ROM compared
byte-for-byte. DKR v77/v80 source searches found only generic fixed-pool,
resource, and handle-array idioms, with no exact donor or naming evidence.
The checkpoint is now **25,636 new Epoch 10 bytes**, **40,620 / 469,264
overlay C bytes (8.66%)**, and **83,272 / 949,944 whole-program resolved bytes
(8.77%)**. The hard 10.00% exit has **11,723 bytes remaining**.

The following compact pass adds another **504 exact bytes**. Overlay 11's
`+0x2BF4..+0x2CB4` six-way release dispatcher contributes 192 bytes; its IDO
jump table is redirected to Mickey's runtime-relocated table with the same
post-link ELF-word patching already established for overlay 101. Overlay 7's
`+0x298..+0x324` entry-appending helper adds 140 bytes, and overlay 19's
`+0x000..+0x0AC` selected-or-all item dispatcher adds 172. All three ranges
match after relocation resolution, and the rebuilt ROM retains SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. DKR v77/v80 and JFG scans are
donor-negative for the three clusters. The checkpoint is now **26,140 new
Epoch 10 bytes**, **41,124 / 469,264 overlay C bytes (8.76%)**, and **83,776 /
949,944 whole-program resolved bytes (8.82%)**. The hard 10.00% exit has
**11,219 bytes remaining**.

### 5.14 Epoch 10 final exit

Epoch 10 closes at **37,360 new exact non-padding overlay bytes**. Overlay C
ownership is **52,344 / 469,264 (11.15%)**, and total resolved text is
**94,996 / 949,944 (10.00%)**, one exact byte beyond the integer 94,995-byte
hard floor.

The final 488-byte step is independently linked and compared at each real
overlay offset:

| Overlay range | Role | Bytes |
|---|---|---:|
| 1 `+0x1CA4..+0x1D58` | release the record array, secondary allocation, and final handle | 180 |
| 1 `+0x7B64..+0x7BDC` | select the best matching fixed record | 120 |
| 61 `+0x19B0..+0x1A6C` | choose the first unused controller-pak filename extension | 188 |
| **final step** | | **488** |

Overlay 61's controller-pak selector (`+0x19B0..+0x1A6C`, 188 bytes) requires
its measured per-object `-Wab,-r4300_mul` schedule and stays exact C, as does
overlay 1's release loop (`+0x1CA4..+0x1D58`, 180 bytes), natural exact after
relocation resolution. Overlay 1's fixed-record selector (`+0x7B64..+0x7BDC`)
— 120 bytes. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match
via a twelve-word register-coloring reassignment, IDO's interchangeable
`a1`/`a3` choice restored to retail's); source kept as decomp-permuter input.
The table row above is therefore a historical checkpoint, not current C
credit; the generated scoreboard is authoritative. Retained configured C is
18/30 words with all four runtime records, while linked range/module/ROM
identity proves the assembly fallback only.

At this historical checkpoint overlays 74, 77, 85, and 97 were counted exact
under the then-active instruction-field normalization. Current canonical
`overlay74Update` remains guarded `NON_MATCHING`, contributes zero exact C
bytes, and its exact linked/module/ROM evidence proves assembly fallback only.
That retired accounting, together with the earlier overlay 39 and 95 closures,
had satisfied the Epoch 5 cohort exit at **6 / 6**. The regenerated donor ledger
covers 107 overlays against pinned DKR v77/v80 and JFG and remains negative
for the final three clusters. A bounded `gmake -j2` rebuild is byte-identical
to the US baserom with SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.

### 5.15 Epoch 11 execution checkpoint

Epoch 11 opens with three exact bodies. Overlay 21's remaining plane-side
priority routine at `+0x10C..+0x2D4` contributes **456 bytes / 114 words** of
exact C. A bounded annotated-target permutation found a redundant comparison
temporary that naturally gives IDO the shipped caller-saved allocation; the
measured `-Wab,-r4300_mul` flag reproduces its FP schedule. Pinned DKR v77/v80
and JFG scans are exact-negative for the body. The following `+0x2D4..+0x2E0`
is 12 bytes of alignment padding and receives no C credit. The non-idiomatic
temporary is tracked in `docs/cleanup-queue.md` as a readability follow-up.

Overlay 30's initializer at `+0x000..+0x2B4` contributes **692 bytes**. Its
natural source reproduces all 173 words and all 61 resident/local relocation
sites; the following 388-byte transposition tail remains assembly. Overlay
41's transition updater at `+0x1B00..+0x1C84` contributes **388 bytes**. A
natural five-record loop reproduces all 97 words and all nine address pairs
without calls, floating point, or code-word correction. Pinned DKR v77/v80
and JFG scans remain exact-negative for both bodies. Overlay 31's palette
builder at `+0x4F8..+0x6B0` adds **440 bytes**. Its bounded source-shape search
settles the compiler's 0x70-byte stack layout and reproduces all 110 words,
three calls, and three address pairs without post-compilation correction; the
same pinned donor scans are exact-negative.

Overlay 31's buffer initializer at `+0x6B0..+0xA84` adds **980 bytes / 245
words**. Keeping `assetBuffer` after the three integer locals preserves IDO's
0x48-byte frame without changing the DKR-derived control flow. The compiled C
reproduces all 16 calls and 19 HI16/LO16 pairs (54 records total), the linked
overlay range, and the whole-ROM hash. The owned range has no padding.

Overlay 37's object updater at `+0x088..+0x19C` — 276 bytes / 69 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
sixteen-word parameter-home/resource-register web normalization and an
operand commute on a finite sine-result multiply); source kept as
decomp-permuter input. Pinned donor scans are exact-negative for this body.

Overlay 40's duplicate-removing table scan at `+0x084..+0x0E8` contributes
**100 bytes**. The source reproduces all 25 words and both relocated global
address pairs naturally. Its second formal is overwritten from the meaningful
identifier before use, an ABI-neutral source shape that selects the shipped
saved-identifier register web; the function remains a frame-free, call-free
leaf and continues scanning after a hit by design.

Overlay 40's add-entry scan at `+0x000..+0x084` — 132 bytes / 33 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
four-word loop-counter register reassignment between equal `v1`/`a0` copies);
source kept as decomp-permuter input.

Overlay 36's final-effect callback at `+0x1688..+0x1748` — 192 bytes / 48
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
two-word private stack-tail shrink, eight bytes); source kept as
decomp-permuter input. The following `+0x1748..+0x1750` is padding and
receives no C credit.

Overlay 46's release/synchronization leaf at `+0x614..+0x69C` — 136 bytes / 34
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an
eight-word pointer-register reassignment, two nullable resource pointers moved
from `v0` to `a0` at their call delay slots); source kept as decomp-permuter
input.

Overlay 14's packed-rectangle builder at `+0x12D8..+0x13F4` — 284 bytes / 71
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
two-word private stack-offset shift, the 24-entry rectangle array moved from
IDO's `sp+0x28` placement to the shipped `sp+0x24`); source kept as
decomp-permuter input.

Overlay 46's state initializer at `+0x000..+0x120` contributes **288 bytes**.
All 72 words are natural after the eleven fixed overlay-local addends and the
genuine `D_184` relocation resolve. The configure helper's third formal is
`f32`; passing `0.0f` selects the shipped zero materialization. The 64-byte
frame, eight calls, descriptor construction, nullable result update, address
pairs, branches, and effects are exact without code-word correction.

Overlay 91's render wrapper at `+0x4BC..+0x574` contributes **184 bytes**.
The first source basin reproduces all 46 words naturally, including six
resident relocation calls, temporary buffer and width outputs, the centered
render band, alpha initialization, and the flag clear/restore around the final
object draw. The trailing `+0x574..+0x580` is padding and receives no C
credit.

Overlay 46's particle initializer at `+0x69C..+0x874` — 472 bytes / 118 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
two-word swap of adjacent, unconditional initialization order between the
independent resource-table-base and loop-counter registers); source kept as
decomp-permuter input. Walks nineteen 60-byte records and their four-byte
configurations, assigning randomized angles/offsets, fixed scale, copied
target coordinates, table-selected resources, and final state/timer values.

Overlay 94's controller initializer at `+0x000..+0x110` — 272 bytes / 68
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
four-word private frame-tail restoration, eight bytes, plus a one-word
or-zero/addiu-zero substitution); source kept as decomp-permuter input.

Overlay 80's contact initializer at `+0x000..+0x11C` — 284 bytes / 71 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
one-word operand commute on the finite sources of the final multiply); source
kept as decomp-permuter input. The measured `-Wab,-r4300_mul` assembler mode
naturally inserts the shipped FP hazard no-op and places both call relocations
at their exact offsets.

Overlay 80's contact updater at `+0x11C..+0x3EC` — 720 bytes / 180 words.
Exact C: a bounded annotated-target permutation found three redundant pointer
and float aliases that naturally give IDO the shipped `0x80` frame and register
allocation. Under `-Wab,-r4300_mul`, the final source reproduces all 180 words,
all 20 relocation records, the linked module, and the full ROM. The aliases are
tracked in `docs/cleanup-queue.md` for a readability follow-up. Together the two
contact bodies cover all **1,004 executable bytes** in overlay 80; the final
`+0x3EC..+0x3F0` remains four bytes of explicit padding and receives no C
credit.

Overlay 10's sole initializer at `+0x000..+0x2B0` — 688 bytes / 172 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
thirty-seven word assertions covering five register/frame webs: private
frame/output homes, a width/height permutation, two base/end completion
swaps, the entry-loop register cycle, and two outer-loop initializations);
source kept as decomp-permuter input. Closes the module with no padding; 135
of 172 words were natural, including all eleven call relocations and fifteen
resident-data address pairs. Previously credited as **3 / 8** Epoch 11
closures.

Overlay 23's `+0x468..+0x568` render helper — 256 bytes / 64 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
thirty-three word assertions covering three register webs: state-pointer
permutation, the call-free packet/display-list schedule, and final-call
argument registers); source kept as decomp-permuter input. The final
`+0x568..+0x570` remains eight bytes of explicit padding.

Overlay 31's pool allocator at `+0xE7C..+0xF44` — 200 bytes / 50 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via fourteen
word assertions covering the unused eight-byte private-frame tail and a
call-free record-pointer/outer-count register web); source kept as
decomp-permuter input. The following `+0xF44..+0xF50` remains 12 bytes of
explicit padding.

Overlay 50's `+0x1C54..+0x1E68` formatter contributes **532 exact C bytes**,
preserving the separate eight-byte padding tail. Overlay 19's `+0xC1C..+0xD78`
adjacency search contributes **348 exact C bytes** and one exact local
`R_MIPS_26` call to the `+0xD78` classifier; its configured object is 87/87
words. Both splits survive atlas regeneration, the donor scan, full link, and
direct ROM comparison.

Overlay 96's `+0x5C8..+0x6D4` object renderer contributes **268 exact C
bytes**. All 67 configured instruction words and the call plus local-data
HI/LO relocation tuples are exact; the compiler alignment word is trimmed at
the `0x10C` symbol boundary.

Overlay 44's `+0x000..+0x224` animation-state constructor contributes **548
exact C bytes** and crosses Epoch 11's 8,000-byte Milestone A gate. All 137
configured instruction words are exact, with nine runtime relocation sites
(two data HI/LO pairs and five calls) reconciled against the retained runtime
census.

Overlay 8 gained seven exact C islands. The motion starter at
`+0x0E88..+0x0F1C` contributes **148 bytes / 37 words**; the activation path
at `+0x0F1C..+0x1000` contributes **228 bytes / 57 words**; the color
applicator at `+0x3278..+0x3368` contributes **240 bytes / 60 words** — all
naturally exact with their call relocations reconciled against retained atlas
splits.

The `+0x2EC0..+0x3018` child updater adds **344 bytes / 86 words**, naturally
exact: 28 FP-touching instructions, ordered branches, a signed post-decrement
loop, six LOCAL data relocations (all resolving to module-local `+0x73B0`),
and a terminal resident-call identity.

Overlay 8's channel updater beginning at `+0x3018` — 608 bytes / 152 words.
The stock C emits all 152 instruction words exactly. Ten compiler-private
literal relocations are rebound, without changing their instruction fields, to
one absolute symbol naming the retained pool at overlay-local `+0x1BC`; the
discarded duplicate payload is guarded by its SHA-256 digest. All 16 runtime
relocation offset/type/identity tuples, the linked range, and the full US ROM
are exact. This replaces the retired wrapper that reached the same bytes by
editing five LO16 addends after compilation.

Overlay 8 `+0x3368..+0x34A0` (`overlay8ScaleOutputs`) owns 312 bytes / 78
words with a `0x8` frame. Retained post-correction configured mixed-TU C is
76/78 raw words because two LO16 fields are unresolved; applying its four
exact runtime tuples gives 78/78. Table-2 LOCAL base `+0x73B0` plus stored
addends `+0x1D0/+0x1D4` resolves to module-local `+0x7580/+0x7584`. Loading
the lower threshold through the upper local preserves the retail `$f2`/`$f0`
web. Retained linked function, mixed-TU text, overlay text, and complete module
are exact. The surviving full-ROM artifact predates promotion, so a fresh
compile→link→ROM chain remains required.

The motion-output body at decimal overlay offsets `+18,920..+19,696` adds
**776 bytes / 194 words**. The measured `-Wab,-r4300_mul` object naturally
reproduces its complete text, including its R4300 scheduler no-op; the
private `0.04f` literal is externalized to decimal overlay-local offset
`+652` so the retained data payload remains the sole byte owner. The direct
ROM slice and cumulative linked ROM are exact.

Overlay 24's updater at `+0x001C..+0x0284` — 616 bytes / 154 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud ELF normalization externalizing a duplicate compiler literal
section already owned by the retained data payload); source kept as
decomp-permuter input. Overlay 24's renderer at `+0x0284..+0x0414` adds **400
bytes / 100 words**, naturally exact, beside the already-matched 28-byte
initializer. The three-word tail at `+0x0414..+0x0420` remains separate
assembly padding. This had completed overlay 24's closure under the retired
scheme.

Overlay 43's child submitter at `+0x1264..+0x1378` — 276 bytes / 69 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud four-word `s2`/`s1` owner-web normalization); source kept as
decomp-permuter input. Natural source was otherwise exact in size, frame, and
opcode/CFG sequence, with both local call relocations exact.

Overlay 62's decimal overlay range `+212..+1,388`, between the existing
initializer and release routine — 1,176 bytes / 294 words. NON_MATCHING: the
configured C baseline preserves the target's 294-instruction shape and 0x88-byte
frame. It has five raw sites at `+0x44/+0x50/+0x54/+0x64/+0x130`; runtime
normalization removes the exact local LO16 at `+0x130`, leaving four
register-allocation sites. All 71 runtime tuples are exact. The assembled
fallback target's 29 ELF records are incomplete, and the former ordinary
zero-word comparison proved the fallback rather than candidate C. One fresh
configured baseline, the missing flag lattice, and one bounded macro-faithful
annotated permutation remain; the assembly fallback stays canonical.

Overlay 84's current-record activation body at decimal overlay offsets
`+4,192..+4,596` — 404 bytes / 101 words. NON_MATCHING: retired 2026-08-24 per
ADR 0002 (was made to match via nine whole-word assertions selecting the
retail private-frame and equal selected-value homes); source kept as
decomp-permuter input. The configured source otherwise retained the exact
root-object address pair, three distinct call identities, selector
branch-likely behavior, and FP/copy schedule. The direct ROM slice at file
offset 26,023,232 has SHA256
`87ac5da55e8b23ea2a9f42a737c478716c2b762b1f792a3423dc6b255198ed24`
in both images, and the cumulative full ROM is exact.

Overlay 68's kind classifier at decimal overlay offsets `+5,228..+5,548` —
320 bytes / 80 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made
to match via eighteen guarded words normalizing private frame, stack-slot,
and register-allocation webs); source kept as decomp-permuter input. Direct
ROM slices from file offset `0x18C85CC` share SHA256
`8a7b237e640d76a01166f3d9b862b59a4ccf03fa576eb6be6b3be9306e90a8c7`,
and the cumulative full ROM is exact.

Overlay 1's type-five keyed search at decimal overlay offsets `+888..+1,044`
is 156 bytes / 39 words and remains `NON_MATCHING`. Its earlier apparent match
used seventeen prohibited post-compile operand-field rewrites. Retained
isolated C differs at 17 register-operand words from `+0x1C`, with frame
`0x30`; no configured full-TU or linked C proof survives. Runtime records prove
the call at `+0x14` resolves to resident `func_8000572C` and the sole direct
inbound call comes from `overlay1TransitionState`. Direct ROM slices at file
offset `0x184C758` share SHA256
`daeb9395211c01871e6c40bafdf49a8187ac111a96855d1ed62d05ca5e80271d`,
but that and the exact retained full ROM prove only the assembly fallback.

| Overlay | Range | Function | Bytes | Exactness | Donor |
|---:|---|---|---:|---|---|
| 1 | `+0x0050..+0x0080` | `overlay1GetEntry` | 48 | canonical object and linked ROM exact | pinned DKR/JFG scans negative |
| 14 | `+0x0328..+0x0498` | `overlay14ApplyValues` | 368 | canonical object and linked ROM exact | Mickey-only |
| 14 | `+0x0D68..+0x0F64` | `overlay14AdvanceCommand` | 508 | canonical object and linked ROM exact | Mickey-only |
| 14 | `+0x0F64..+0x1028` | `overlay14StepCommand` | 196 | canonical object and linked ROM exact | Mickey-only |
| 14 | `+0x13F4..+0x1540` | `func_overlay_014_F00013F4_1870CCC` | 332 | canonical object and linked ROM exact | Mickey-only |
| 43 | `+0x0194..+0x0280` | `func_overlay_043_F0000194_188A164` | 236 | canonical object and linked ROM exact | Mickey-only |
| 70 | `+0x0000..+0x00D8` | `func_overlay_070_F0000000_18C91C8` | 216 | canonical object and linked ROM exact | Mickey-only |

Overlay 1's backward usable-record search at decimal offsets `+1,044..+1,204`
— 160 bytes / 40 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was
made to match via twelve guarded words selecting the retail preheader and
temporary-register webs); source kept as decomp-permuter input. Direct ROM
slices at file offset `0x184C7F4` share SHA256
`837c7fcf2d1ee011313f664b42f663b8bca7658473c801b697e24a81031461ff`.

Overlay 82's newly matched updater at `+0x0040..+0x0498` contributes **1,112
bytes / 278 words** with an exact normalized natural object, 26 retained
relocation sites, and separate four-byte tail padding; two call proxies and
three input-data proxies preserve the runtime relocation placeholders. This
made overlay 82 fully C-owned across its 1,228 executable bytes. Direct ROM
slices from file offset `0x18CF1C0` share SHA256
`38fbefda01c642125d3f1badef620054a90760e461f494741c370f660c65aad2`,
and the cumulative ROM is exact, credited as the sixth Epoch closure.

Overlay 45's layout configurator at decimal offsets `+788..+1,600` adds **812
bytes / 203 words**. Its natural object reproduces the complete 0x80-frame
code body, five calls, fourteen data-address relocations, twelve FP
instructions, stream loop, likely branches, and glyph update order with no
word correction; three call proxies and five data proxies retain the shipped
runtime relocation identities. Direct ROM slices from file offset `0x188C76C`
share SHA256
`0eb13f9257a0d760179622fb1929659d758b435121ba7c3f3722dc9a015766b2`,
and the cumulative ROM is exact.

Overlay 45 `+0x0764..+0x1158`
(`func_overlay_045_F0000764_188CBBC`) contributes **2,548 exact C bytes /
637 words** with the retail `0x88` frame. The compiler's instruction fields
are untouched: six existing HI16/LO16 relocations are rebound to one ABS
`+0x24` carrier for the three constants already retained at module
`+0x1C94/+0x1C98/+0x1C9C`, and the duplicate 16-byte literal pool is removed
only after its complete SHA-256 agrees. All 24 shipped runtime relocation
offsets, types, and effective identities are exact: 14 identities resolve
statically, and the remaining 10 are proved by the unchanged runtime table
plus the instruction-exact linked range. The linked owner, complete overlay,
and full US ROM are byte-identical.

Overlay 23's `+0x000..+0x208` attachment spawner adds **520 naturally exact C
bytes / 130 words** with its `0xA0` frame and two call relocations exact.
With the existing C islands and separate eight-byte tail padding, this had
made all executable text C-owned, credited as the seventh Epoch 11 closure.

Overlay 19's `+0xA30..+0xC1C` adjacency builder — 492 bytes / 123 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via 41
guarded register-only word selections); source kept as decomp-permuter
input. The natural object was otherwise topology-, frame-, and
relocation-exact.

Overlay 19's `+0xD78..+0xF58` edge classifier owns 480 bytes /
120 words with no padding, frame, calls, or static/runtime relocations.
It remains guarded `NON_MATCHING`. Historical exactness used prohibited
instruction-field normalization and was retired under ADR 0002. The retained
configured full-TU and isolated C candidates are byte-identical at 110/120
words: six `$v1`/target-`$t3` carrier sites and four final two-load ordering
sites differ, first at `+0x138`. The sole direct caller is the exact local
`R_MIPS_26` call from `overlay19FindAdjacent+0xD8`. Exact ordinary-object,
linked-function, complete-module, and ROM evidence proves the assembly
fallback only; no linked C candidate survives. Run the one unrecorded
119-flag lattice, then park if canonical flags remain best.

Overlay 42's `+0x0F4..+0x6A4` captured-buffer renderer adds **1,456 naturally
exact C bytes / 364 words**. Its historical display-list macro spelling
emits the exact `0xD0` frame, seven calls, nine address pairs, every
register and schedule choice, and the complete command stream without word
normalization. With the four existing exact functions, this had made
overlay 42's full `+0x000..+0x700` executable text C-owned, credited as the
eighth Epoch 11 closure.

Overlay 19's `+0xF58..+0x12E4` spatial-mask builder — 908 bytes / 227 words,
with twelve following padding bytes retained separately in assembly.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
SHA-anchored normalization selecting two register webs and four independent
schedule pairs); source kept as decomp-permuter input. Its minimized source
naturally matched 164/227 words and the exact frame, CFG, memory effects, and
zero-relocation contract.

Overlay 58's rank-set refresh — 608 bytes / 152 words. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a SHA-anchored decoded-field
normalization selecting two interchangeable private temporary webs); source
kept as decomp-permuter input. The natural object was 141/152 with exact
topology, opcodes, memory effects, and local/call relocation roles; this was
volume credit and did not close overlay 58.

The next volume tranche added overlay 57's draw body (**832 bytes**), overlay
58's segment-strip renderer (**804 bytes**), overlay 101's clock renderer
(**952 bytes**), and two overlay 57 mode bodies (**1,132** and **868 bytes**),
each naturally exact with no compiler-only alignment bytes counted.

The byte-identical shared renderers in overlays 69 and 88 add **1,436 bytes
each**. Their objects and six call relocations were validated independently.
Overlay 69's following four-byte padding remains assembly-owned, while
overlay 88 ends directly at its relocation payload. Both complete linked
overlay spans are exact.

Overlay 58's two adjacent point-quad renderers add **416 bytes each**. They
share one reviewed source and normalization shape but keep independent
symbol and payload bindings. Both configured objects, direct slices, and the
full ROM are exact.

Overlay 58's results renderer, `func_overlay_058_F000138C_18B0574`, is
Tier A exact C at `+0x138C..+0x4C04`: **14,456 bytes** / 3,614 words, frame
`0x138`, and all 1,253 runtime relocation offsets, types and identities
(2026-09-16, lane wv-y; the largest function matched in the tree). The
compiler's private thirteen-entry jump-table pool duplicates the shipped
table at data_rodata `+0x3F4`, so the object's two table references are
rebound to `gOverlay58ModeJumpTableReloc` and the pool is externalized by
digest, contributing no bytes; the atlas records the rodata row as
externalized ownership. Untouched configured IDO, the linked owned range
and the full ROM pass the normal promotion proofs. No instruction rewriting
is used. The inert forms its match rests on are listed in
`docs/cleanup-queue.md`; the route from 187 words is `docs/WHALE.md`.

Overlay 40's state updater at `+0x0E8..+0x1A0` — 184 bytes / 46 words,
creating a 416-byte contiguous prefix. NON_MATCHING: the bounded natural
direct-shift source is exact-sized and frameless at 44/46 positional words,
first `+0xC`. Its body and register allocation are exact; only the adjacent
loop-count initialization and object-table LO16 completion are reversed.
Three of four runtime tuples align exactly by offset, type, and identity, with
the fourth displaced by that schedule swap. All 119 ownership-aware cached
flag rows are nonexact. ORT 1451 and sole resident inbound
`func_80051364+0x78` authenticate the exact no-padding owner. JFG's nearest
skeleton is an assembly-only contextual result, not a donor body. Exact linked
range/module/ROM evidence therefore still proves the assembly fallback only.

Overlay 40's fade-record owner at `+0x690..+0x824` remains `NON_MATCHING` at
404 bytes / 101 words; `+0x824..+0x830` is separate padding. The compiled
output-origin C is the bounded plateau at 98/101 words, frame `0x8`, with only
`+0xC/+0x10/+0x24` differing as one `v0`/`v1` globalcolor outcome; its
temporary-register lane is exact. One allocator trace isolated that pool swap,
and all 119 flag combinations were nonexact with canonical `-O2 -mips2` tied
for best. Its ten runtime BSS records resolve to `D_800D6C4C`, `D_800D6C52`,
`D_800D6C50`, `D_800D6C4E`, and `D_800D6C54`; the target/fallback object
retains none statically. Resident `func_8000D978 +0x130` is the sole proved
direct caller. Exact linked range/module/full-ROM evidence proves assembly
fallback only; no linked C artifact survives. Pinned DKR v77/v80 and JFG scans
are negative.

Overlay 30's `+0x2B4..+0x438` byte-plane transposer — 388 bytes / 97 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
guarded schedule and decoded field ledger preserving the opcode census,
counted loop, memory behavior, and zero-relocation surface); source kept as
decomp-permuter input. The separate `+0x438..+0x440` padding remains
assembly-owned; with the existing initializer this had made all overlay 30
text exact C, credited as the ninth Epoch 11 overlay closure.

Overlay 1's `+0x7B0..+0xBD4` point-record builder adds **1,060 naturally
exact C bytes / 265 words**. The configured four-way-unrolled R4300-scheduled
object matches SHA256
`f1c43d4bed886287e2cdade26036351d04985a00eb7ae3f0d2456f431aaa1d85`.
Its two calls retain distinct runtime relocation identities at `+0x1C` and
`+0x2C`; neighboring assembly resumes exactly at `+0xBD4`.

Overlay 84's `+0xC9C..+0xDBC` current-resource loader adds **288 exact C
bytes / 72 words**. A bounded decomp-permuter pass escaped the former
state-pointer spill-home plateau; untouched IDO output now supplies the exact
frame, CFG, opcode/call/FP schedule, memory effects, and five relocation
sites, and the linked overlay is byte-identical to the ROM.

Overlay 59's `+0x070..+0x168` entry preparer — 248 bytes / 62 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
nine-word decoded ledger selecting retail's equivalent descriptor-value/
call-argument web); source kept as decomp-permuter input. Retained configured
evidence is 53/62 relocation-normalized words with the exact boundary, frame,
CFG and memory effects, but no linked candidate-C proof survives. The six
runtime records are a local table HI/LO pair, local `JUMP` calls to
`overlay59Release` at `+0x48/+0xD0`, and resident `SYMBOL` calls to
`func_80034448` at `+0x64/+0x94`; the synthetic target object collapses those
four zero-field calls and is not identity authority.

Overlay 48's `+0x144..+0x40C` state updater — 712 bytes / 178 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
schedule permutation moving one argument materialization to the retail slot
and five branch-displacement updates); source kept as decomp-permuter input.
Natural source otherwise supplied the exact boundary, `0x38` frame, and all
22 runtime relocation records. The independent `+0x060..+0x144` body remains
assembly-owned, so this was volume credit rather than a closure;
`+0x46C..+0x470` remains explicit padding.

Overlay 101's transformed-object renderer — 664 bytes / 166 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two
command-schedule permutations and decoded private temporary webs selecting
retail's equivalent allocation); source kept as decomp-permuter input.
Natural source otherwise supplied the exact `0x90` frame, ABI, seven-call
layout, CFG, memory/stack effects, and FP topology.

Overlay 59's `+0x36C..+0x784` six-state advancer — 1,048 bytes / 262 words.
NON_MATCHING: bounded configured C is exact-sized at 240/262 raw and 243/262
relocation-aware positional words, first `+0x4`, with the exact `0x58` frame,
CFG, loops, branch-likely forms, and identical integer, FP-pool, and temporary
register lanes. All 119 compiler configurations are nonexact; canonical
`-O2 -mips2` ties for best. A trace-off-identical IDO 5.3 `as1 -R` capture
shows the remaining executable gap is one prologue ready-chain choice: stock
schedules the factor save/load chain first, while retail schedules the
constant-register chain first. Ten source-faithful assignment-order, explicit
constant, copy-carrier, scope, declaration, and physical-line forms were flat
or worse. The target and C each carry 15 relocations; 13 offsets/types and 12
identities align, with the factor pair displaced by the prologue schedule. The
retained switch table at `+0x76C` is already owned and rebound. ORT 1435 and
resident calls from `func_80038E1C+0x2C4/+0x320` authenticate the one-word
`steps` ABI and exact owner; `+0x784` starts the next function with no padding.
The owner is ROM `0x18B8ABC..0x18B8ED4`. No credible permitted donor was found
(best skeleton similarity 0.0433). Exact linked function, overlay, and ROM
evidence remains fallback-only and contributes **0 exact C bytes**.

Overlay 68's `+0x51C..+0x650` secondary-entry promoter adds **308 exact C
bytes / 77 words**. A bounded decomp-permuter pass escaped the former private
frame and stack-home plateau. Untouched IDO output supplies the exact `0x30`
frame, call-survival and null-exit forms, copy loop, memory effects, and all
nine relocation sites; the complete linked US ROM is byte-identical.

Overlay 68's following interpolation body — 656 bytes / 164 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
fail-loud symbol metadata, schedule, and decoded private register ledgers
proving and selecting the full retail ownership unit); source kept as
decomp-permuter input. Natural source otherwise supplied the complete
semantic body and three zero alignment words.

Overlay 68's secondary-resource rebuild — 488 bytes / 122 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud private stack/register ledger selecting retail's equivalent
homes); source kept as decomp-permuter input. Natural source otherwise
supplied the exact frame, opcode schedule, map walk, probe selection, nine
calls, and all 19 relocation offsets.

Overlay 66's RGB5551 smoothing-and-draw body at `+0x040..+0x4E0` — 1,184
bytes / 296 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to
match via a fail-loud instruction-schedule and private register ledger
selecting retail's equivalent allocation and three proved local symbol
addends); source kept as decomp-permuter input. Natural source otherwise
supplied the exact frame, boundary, opcode inventory, CFG, unrolled
four-pixel filter, and all 28 runtime relocation records. The independent
`+0x4E0..+0x810` helper remains assembly-owned.

Overlay 68's keyframe-animation updater at `+0x96C..+0xEFC` — 1,424 bytes /
356 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match
via a fail-loud decoded-field ledger selecting retail's equivalent private
allocation, plus removal of four duplicate compiler records resolving to the
resident BSS flag); source kept as decomp-permuter input. Natural source
otherwise supplied the exact frame, boundary, eleven calls, keyframe loop,
interpolation/direction CFG, and runtime relocation sites.
`+0xEFC..+0x1250` remains separately assembly-owned.

Overlay 69's anchor updater at `+0x04C..+0x170` adds **292 exact C bytes / 73
words** and removes its final executable assembly gap. Natural source
supplies the exact `0x50` frame, branch-likely gate, store order, four call
sites, and all instruction words; the shipped overlay relocation table
proves the four distinct resident call identities. The independent
`+0x70C..+0x710` alignment word remains assembly-owned, credited as closure
eleven.

Overlay 18's state reconfiguration body at `+0x24C..+0x4F4` — 680 bytes / 170
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud ledger removing one redundant compiler copy and selecting three
independent schedules plus private stack and GPR homes); source kept as
decomp-permuter input. Natural source otherwise supplied the exact `0x68`
frame, state transitions, equality-only rate loops, old-velocity integration
order, five semantic calls, and all 64 relocation records. The independent
`+0x000..+0x1F4` and `+0x4F4..+0x650` bodies remain assembly-owned, so this
was volume credit rather than a closure.

Overlay 98's unique-height collector at `+0x000..+0x144` — 324 bytes / 81
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud schedule and decoded-field ledger selecting retail's equivalent
three-base allocation and address materializations); source kept as
decomp-permuter input. Natural source otherwise supplied the complete nested
traversal, flag gate, duplicate scan, signed/unsigned field behavior,
fifteen-entry cap, and exact instruction count. The successor begins exactly
at `+0x144` and remains assembly-owned.

Overlay 89's effect updater at `+0x000..+0x138` — 312 bytes / 78 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a
fail-loud ledger selecting the retail private stack/register web and a
four-instruction create-call schedule); source kept as decomp-permuter
input. Natural source otherwise supplied the exact `0x58` frame, opcode and
branch topology, six calls, FP operations, and eight runtime relocation
sites. The unrelated tail at `+0x270` remains assembly-owned, so this was
volume credit rather than a closure.

Overlay 18's buffer initializer at `+0x4F4..+0x650` — 348 bytes / 87 words.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two
relocation-aware schedule permutations plus a fail-loud local-addend/
private-spill/GPR ledger selecting the retail layout); source kept as
decomp-permuter input. Natural source otherwise supplied the exact `0x20`
frame, straight-line CFG, opcode inventory, two allocation calls, memory
effects, and all 50 runtime relocation sites and types. Only
`+0x000..+0x1F4` remains assembly-owned, so this was volume credit rather
than a closure.

Overlay 18's startup loader at `+0x000..+0x1F4` — 500 bytes / 125 words, and
removes the module's final executable assembly gap. The module's own
relocation record at `+0x1CC` resolves through resident overlay zero to
`osSetTime`; restoring its real one-`OSTime` ABI makes IDO naturally emit the
retail `a0`/`a1` zero pair. Ordinary `-O2 -mips2 -32` codegen then reproduces
all 125 words, and the configured object retains all 60 runtime relocation
sites and identities. Together with the three exact successors, all **1,616
executable bytes** of overlay 18 are C-owned, credited as closure thirteen.

Overlay 88's anchor updater at `+0x04C..+0x1A4` adds **344 exact C bytes / 86
words** and removes its final executable assembly gap. Natural source
supplies the exact `0x58` frame, branch-likely delay-slot load, FP and store
schedule, and seven call sites; the shipped overlay relocation table proves
the seven distinct resident roles. Overlay 88 has no executable padding,
credited as closure twelve.

Overlay 68's sorted-entry renderer — 852 bytes / 213 words, and removed its
final executable assembly gap. NON_MATCHING: retired 2026-08-24 per ADR 0002
(was made to match via a relocation-aware permutation moving the prepare
call to its retail site, a peeled adjacent-swap schedule choice, and a
fail-loud decoded-field ledger selecting private frame/register/spill
allocation); source kept as decomp-permuter input. Natural source otherwise
reproduced the exact boundary, CFG, three calls, gathered-entry and
descriptor effects, and stack-array offsets. The only remaining assembly
interval is the explicit four-byte tail padding, credited as closure
fourteen.

Overlay 89's state/particle updater — 544 bytes / 136 words. NON_MATCHING:
retired 2026-08-24 per ADR 0002 (was made to match via a single fail-loud
bijection selecting retail's argument spill, private frame, saved-register
allocation, and instruction schedule); source kept as decomp-permuter input.
Natural source otherwise reproduced the exact instruction count, FP/integer
inventories, constants, structure accesses, and fourteen runtime relocation
roles.

Overlay 89's initializer, `overlay89InitializeEffect`, is Tier A exact C at
`+0x270..+0x5A4`: 820 executable bytes, 205 words, frame `0x58`, and all five
runtime relocation offsets, types and identities. The scale's struct-member
access preserves conversion-before-load evaluation without a float carrier.
The maintain call uses the retained state pointer; an independently reviewed
inert region keeps that pointer caller-saved and is listed in
`docs/cleanup-queue.md`. Untouched configured IDO, the linked owned range,
and the full ROM pass the normal promotion proofs. No instruction rewriting
is used. The twelve-byte module tail remains assembly-owned padding and earns
no executable credit.

Overlay 48's state initializer at `+0x060..+0x144` owns 228 bytes / 57 words
with frame `0x18`; `+0x144` starts the next function, so it owns no padding.
It remains NON_MATCHING. Fresh configured C emits 53 words and preserves the
frame, but matches only 10/57 positional words from first mismatch `+0x0`.
The target's 25 runtime records are fully decoded: local state/seed/script
roles plus two resident calls. The candidate emits 24 records; its shifted
schedule aligns only 12 offset/type sites, resolves eight candidate identities,
and aligns one identity. ORT 1423 exports the function, and resident
`func_80038BC4+0x188` is its sole authenticated caller.

A bounded 14-build pass tested twelve natural source families spanning direct
arrays, pointer-controlled loops, `for`/`do` forms, hand unrolling, aggregate
and scalar BSS ownership, local/global declarations, and volatile qualifiers.
A scalar declaration model reproduced the module's exact `0x60` BSS extent,
but worsened text. Every loop form either peeled the first record or emitted
extra address producers; every hand-unrolled form erased the target's retained
index/seed-base setup. The prior full flag lattice was therefore not repeated.
The exact-donor scan is negative, and the nearest JFG result (`levelFreeAll`,
0.0769 skeleton similarity) is unrelated and offers no actionable source or
JFG-maintainer lead. Preserve the retained baseline and assembly fallback;
resume only with source-authentic declaration/TU evidence that keeps the
unrolled index setup while materializing the record base from its first entry.

Overlay 16's gradient applicator owns `+0x1E0..+0x424`, 580 bytes / 145
words. Its policy-clean configured C remains NON_MATCHING but has the exact
`0x20` frame, 85/145 raw and runtime-normalized words, first difference
`+0x3C`, and all six LOCAL relocation tuples exact by offset, type, identity,
and addend. Those pairs address the gradient buffer at addend zero, phase at
`+8`, and mode at `+4`. ORT 1313 exports the function; resident relocation
155 at exact-C `func_8000D978+0xFC` is its sole authenticated inbound. The
2026-08-29 bounded pass exhausted V0, all 119 canonical flags, one
fidelity-clean allocator trace, and three natural forms. Reordering the six
channel definitions was the sole strict gain, reducing 64 register-only
differences to 60; phase chaining was flat and delayed gradient loads
regressed. No exact untouched IDO object was found, so the assembly fallback
remains canonical. The following `+0x424..+0x430` twelve-byte padding is
separate ownership. The earlier post-compile match remains retired under ADR
0002 and is not promotion evidence.

Overlay 49's initializer and updater at `+0x000..+0x354` — 852 bytes / 213
words, and removed its final executable assembly. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a guarded ledger selecting
private allocation/scheduling webs plus removal of one compiler-alignment
copy, and three guarded field/schedule operations on the post-decrement
input loop); source kept as decomp-permuter input. The 500-byte initializer
preserved thirty call/address relocation sites; the 352-byte updater
preserved twenty-three relocations and the shipped post-decrement input loop
including index zero. The two retail body SHA256 values are
`75dbf550918557a9ade8802c46458d379d324e23d1eca9b477ed9c902e564a96`
and
`122d09b0ff67c971b223ce8cee57ea647fa1e8646077794a30be6095569f5bee`.
The exact `refractOutput` tail stays C and only twelve bytes of explicit
padding remain assembly-owned, credited as closure eighteen.

Overlay 37's renderer at `+0x19C..+0x4F4` — 856 bytes / 214 words, and
removed that module's final executable assembly. NON_MATCHING: retired
2026-08-24 per ADR 0002 (was made to match via a fail-loud bijection
selecting the retail frame, private GPR/FPR allocation, branch distances,
and legal schedule); source kept as decomp-permuter input. Natural IDO owned
852 bytes followed by one proved zero alignment word, extended into the
function. All twelve call/local-data relocation sites remain exact. The
configured raw body SHA256 is
`812e309eb30b88554676a6b7a11b8cd823a7426d7666506654bef7db8b6119e5`;
the retail ROM slice SHA256 is
`4b26c018a45c6d4ccd8285ec8570fa279d399e0253b008485be69f74456ce975`.
Only eight bytes of explicit padding remain, credited as closure nineteen.

Overlay 31's configuration allocator at `+0xA84..+0xDC4` — 832 bytes / 208
words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via
eight fail-loud decoded-field assertions selecting four private
representation webs: commutative operand order, one spill home, one
comparison destination, and one producer/use/store temporary); source kept
as decomp-permuter input. Natural output was otherwise already exact in
opcode inventory, CFG, delay slots, `0x50` frame, and its sole overlay-local
call relocation, agreeing at 200 of 208 words. The retail body SHA256 is
`43fe93dec2619d929e2a047471d108014dc9916045bcbbcfab2ea9a323779782`.

This checkpoint had contributed **48,908 / 47,496 (102.97%)** Epoch 11 bytes
and recorded nineteen overlay closures against the eight-closure target under
the retired object-editing scheme; under ADR 0002 the functions listed above
as NON_MATCHING are reclassified pending decomp-permuter matches, and the
closure/byte totals above are historical record rather than current status.
The linked binary from that checkpoint remained byte-identical to the US
baserom at SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`.

### 5.16 Epoch 12 execution checkpoint

Epoch 12 opens with four consecutive presentation builders in overlay 101.

`overlay 101 +0x99C4..+0x9D04`, `+0x9D04..+0xA044`, `+0xA044..+0xA384` — 832 bytes / 208 words each. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant-instruction deletion at +0x324); source kept as decomp-permuter input. Each preserves 24 HI/LO pairs and four calls (52 relocations).

`overlay 101 +0xA384..+0xA6BC` — 824 bytes / 206 words. Naturally exact at the `0x338` size; no correction needed.

Overlay 33's present-and-swap helper at `+0x066C..+0x0708` — 156 bytes / 39 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via private-register web reassignment); source kept as decomp-permuter input. Declaring the double-buffer index volatile naturally reproduces the target's `0x18` frame, size, opcode schedule, two calls, and all 18 relocation sites. The larger renderer and initializer remain assembly-owned, so this is not a module closure.

#### ADR 0001/0002 reclassification of the following Epoch 12 tranche

A later canonical-source audit found that the historical rolling checkpoints
from `overlay 58 +0x5554` through `overlay 1 +0x7D6C` credited **25,548 bytes**
whose C bodies remain guarded by `NON_MATCHING`. Those candidates remain useful
matching evidence, but assembly is canonical and they receive no matched-C,
resolved-text, or module-closure credit. The tranche now contains **14,120 bytes**
of genuine exact promotions; its old cumulative numerators, percentages, and
closure-gate values are withdrawn. Current cumulative progress is reported
only by the generated scoreboard.

The audited tranche spans **39,300 executable bytes**. Padding and exact
ranges credited before this tranche are excluded.

#### Canonical exact C ranges

| Overlay | Exact ranges (bytes) | Subtotal |
|---:|---|---:|
| 1 | `+0x6424..+0x64F8` (212); `+0x73A0..+0x7580` (480); `+0x7BDC..+0x7D6C` (400) | 1,092 |
| 2 | `+0x0000..+0x01BC` (444); `+0x049C..+0x06E0` (580); `+0x123C..+0x1364` (296) | 1,320 |
| 3 | `+0x027C..+0x03B0` (308); `+0x0588..+0x06D8` (336); `+0x06D8..+0x0760` (136) | 780 |
| 4 | `+0x0000..+0x0138` (312); `+0x05D0..+0x0710` (320); `+0x0734..+0x08F4` (448); `+0x08F4..+0x0CAC` (952) | 2,032 |
| 7 | `+0x0000..+0x00A8` (168); `+0x0F08..+0x0FB8` (176) | 344 |
| 14 | `+0x1B7C..+0x1C40` (196) | 196 |
| 20 | `+0x00A8..+0x0204` (348); `+0x07C4..+0x09DC` (536); `+0x0E28..+0x0F78` (336) | 1,220 |
| 40 | `+0x0534..+0x0690` (348) | 348 |
| 61 | `+0x0000..+0x01C0` (448); `+0x01DC..+0x03C0` (484); `+0x03C0..+0x07C4` (1,028); `+0x07C4..+0x0968` (420); `+0x1578..+0x19B0` (1,080) | 3,460 |
| 83 | `+0x02A0..+0x0514` (628) | 628 |
| 87 | `+0x0000..+0x0128` (296) | 296 |
| 91 | `+0x004C..+0x04BC` (1,136) | 1,136 |
| 96 | `+0x0000..+0x0070` (112); `+0x00F8..+0x04BC` (964); `+0x04BC..+0x057C` (192) | 1,268 |
| **Total** | | **14,120** |

Overlay 4's three omitted mixed-TU atlas exceptions total **1,720 bytes**
(`+0x05D0..+0x0710`, `+0x0734..+0x08F4`, and
`+0x08F4..+0x0CAC`). They are already included in the 14,120-byte subtotal;
the atlas correction changes attribution, not this tranche total.

#### Guarded and uncredited ranges

| Overlay | Guarded ranges (bytes) | Subtotal |
|---:|---|---:|
| 1 | `+0x6B6C..+0x6CE8` (380); `+0x7130..+0x72A4` (372); `+0x72A4..+0x7344` (160); `+0x7580..+0x7730` (432) | 1,344 |
| 2 | `+0x02C4..+0x0400` (316); `+0x06E0..+0x0B70` (1,168); `+0x0B70..+0x0C90` (288) | 1,772 |
| 3 | `+0x00B8..+0x027C` (452); `+0x03B0..+0x0588` (472) | 924 |
| 4 | `+0x0138..+0x04D0` (920) | 920 |
| 5 | `+0x031C..+0x06C0` (932) | 932 |
| 7 | `+0x00A8..+0x0228` (384); `+0x0894..+0x0AA0` (524); `+0x0AA0..+0x0CCC` (556); `+0x0CCC..+0x0DBC` (240); `+0x0DBC..+0x0EDC` (288) | 1,992 |
| 17 | `+0x0000..+0x0318` (792); `+0x0318..+0x0628` (784); `+0x0668..+0x08B4` (588) | 2,164 |
| 20 | `+0x0204..+0x038C` (392); `+0x0A68..+0x0DC4` (860); `+0x1018..+0x10EC` (212) | 1,464 |
| 25 | `+0x0000..+0x017C` (380); `+0x017C..+0x0588` (1,036) | 1,416 |
| 29 | `+0x14C8..+0x16CC` (516) | 516 |
| 33 | `+0x0000..+0x0144` (324) | 324 |
| 34 | `+0x0000..+0x00C8` (200) | 200 |
| 40 | `+0x01A0..+0x02E4` (324); `+0x0690..+0x0824` (404) | 728 |
| 44 | `+0x0294..+0x0580` (748) | 748 |
| 57 | `+0x3A4C..+0x3FD4` (1,416) | 1,416 |
| 58 | `+0x5554..+0x5A14` (1,216) | 1,216 |
| 75 | `+0x0214..+0x06D4` (1,216) | 1,216 |
| 83 | `+0x053C..+0x07DC` (672); `+0x0850..+0x0984` (308) | 980 |
| 94 | `+0x0110..+0x055C` (1,100) | 1,100 |
| 96 | `+0x0070..+0x00F8` (136) | 136 |
| 99 | `+0x0638..+0x0800` (456); `+0x0800..+0x0BA4` (932); `+0x0BA4..+0x0DDC` (568) | 1,956 |
| 100 | `+0x0000..+0x0214` (532); `+0x038C..+0x050C` (384) | 916 |
| 101 | `+0xA6BC..+0xAB4C` (1,168) | 1,168 |
| **Total** | | **25,548** |

The exact and guarded tables supersede the mixed historical rows and every
rolling Epoch 12 numerator, percentage, and closure claim formerly attached to
them. The generated scoreboard is the authority for current cumulative
progress.

Overlay 1 `+0x78DC..+0x7B64` — 648 bytes / 162 words, bounded path tracing, endpoint append, and branch-record cloning. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an integer-temporary web reassignment); source kept as decomp-permuter input. Natural source otherwise emits the complete opcode schedule, 88-byte frame, stack layout, eight-call CFG, FP conversions, delay slots, and all observable effects; the configured object retains all eight call records and seven local HI/LO pairs.

Overlay 34 `+0x2C8..+0x378` — 176 bytes / 44 words, active-record removal, list compaction, resident resource release, and count update. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via count/pointer web reassignment plus removal of two literal pointer-address relocation records absent from retail); source kept as decomp-permuter input. Natural source otherwise owns the exact boundary, frame, CFG, call ABI, and effects; the configured object retains the loader call and both active-count HI/LO pairs.

Overlay 34's record constructor at `+0x0D4..+0x2C8` — 500 bytes / 125 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a compiler-representation/schedule web reassignment); source kept as decomp-permuter input. Natural source otherwise supplies the exact functional boundary, CFG, field initialization, allocation/load path, direction setup, and two-call ABI, preserving all 12 relocation sites: five local-data HI/LO pairs and two independently decoded resident calls.

Overlay 34 `+0x378..+0x540` — 456 bytes / 114 words, storage reset and active-record updates. Both functions are exact C and together retain all 22 shipped runtime relocations.

`overlay 34 +0x040C..+0x0540` (`overlay34UpdateRecords`) is **308 exact C bytes / 77 words**. Placing the existing countdown initializer at the start of the live-record branch makes untouched configured IDO emit the retail `0x30` frame and exact instruction stream; it removes the previous invariant saved-register carrier and its three-word/frame excess without synthetic state or compiler-output instruction edits. Runtime metadata authenticates LOCAL sites `+0x28/+0x2C` for active count, `+0x40/+0x4C` for the pointer array, `+0x44/+0x48` for the float parameter, and the local remove-record call at `+0xF0`. The call is rebound only at relocation-symbol metadata to the overlay's zero-field proxy, preserving the shipped effective `overlay34RemoveRecord +0x2C8` identity. The linked owner, complete Overlay 34 image, and full ROM are byte-identical. ORT 1304 and resident relocation-table row 146 prove the sole inbound from `func_8000BD50+0x4C` and its one-word `a0` ABI.

JFG's public `src/overlays/o10/overlay_10.c:sparkUpdate` remains assembly-only but is the closest measured structural counterpart: maintainers can use Mickey's exact body as a starting scaffold at that insertion point. JFG's extra lifetime/fade behavior and different field layout remain project-specific reconstruction work; no JFG C body was adopted here.

Overlay 1 `+0x5CD4..+0x5ECC` — 504 bytes / 126 words, directional object selection. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via stack-home plus a two-instruction temporary-web reassignment); source kept as decomp-permuter input. Target-local R4300 multiply hazards naturally recover the exact frame, opcode stream, CFG, FP schedule, and all 11 relocation sites.

Overlay 1 `+0x7D6C..+0x7FCC` — 608 bytes / 152 words, path-point resolution.
NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two
scheduling permutations plus a four-field register reassignment); source kept
as decomp-permuter input. The current retained candidate has the exact
120-byte frame, boundary, CFG and call topology, but ten raw/eight
addend-normalized word differences and only 19/22 exact relocation tuples:
the `D_218` LO16 and `D_1D88` HI16 exchange `+0x090/+0x094`, while the
`D_1BA4` LO16 is at candidate `+0x14C` instead of target `+0x148`. Runtime
evidence also corrects the clear target to local `D_220` and the sole caller
to a seven-argument ABI.

Overlay 1 `+0x5BF4..+0x5CD4` — 224 bytes / 56 words, timer and mode callback dispatch. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a loop-tail reschedule plus register/branch-lowering reassignment); source kept as decomp-permuter input. Natural source otherwise recovers the exact boundary, frame, direct setup call, two indirect callbacks, loop semantics, and all 13 runtime relocation records.

Overlay 34 `+0x608..+0x900` — final 760 bytes / 190 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/register/frame web reassignment); source kept as decomp-permuter input. Typed source owns the depth census, parallel bubble sort, color interpolation, and ten-argument render dispatch; canonical integration rejected a zero-local proxy false positive and retained the shipped semantic local addends for all five HI16/LO16 pairs while using pre-loader carriers only for the six calls, and all 16 runtime records remain exact. Every executable interval now has C source, but four owners remain guarded `NON_MATCHING`; Overlay 34 is not exact-closed and receives no module-closure credit.
`overlay 1 +0x7730..+0x78DC` — 428 bytes / 107 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via prologue-instruction reordering plus a commutative-operand swap); source kept as decomp-permuter input. All six runtime calls were confirmed exact.

`overlay 1 +0x6A14..+0x6B28` — 276 bytes / 69 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via instruction-scheduling reorder plus a register-allocation rewrite); source kept as decomp-permuter input. One runtime relocation was confirmed exact.

`overlay 1 +0x5ED4..+0x61F0` — 796 bytes / 199 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via allocator-web reassignment plus a pre-loader addend rewrite); source kept as decomp-permuter input. 61 runtime text records were confirmed authoritative.

`overlay 1 +0x6270..+0x63CC` (`overlay1ChooseModeObject`) is now **348 exact C bytes / 87 words**. The ordinary configured IDO build reaches the target by declaring the existing locals in reverse stack-home order; no synthetic local, compiler-output edit, or pre-loader addend rewrite is used. The `0x50` frame and all 13 runtime text records are exact by offset, type, and identity: calls resolve to resident `func_80005750` once and `mathRnd` twice, while the five local HI16/LO16 pairs retain their overlay-local identities. The function has no owned padding, is referenced by the overlay-local `R_MIPS_32` table entry at `+0x8218`, and is exact in the linked overlay and full ROM. The donor scan found no useful DKR, JFG, Conker, or same-overlay sibling candidate.

`overlay 1 +0x64F8..+0x6724` — 556 bytes / 139 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via loop-control carrier relocation and a shared-tail reorder); source kept as decomp-permuter input. Four runtime call records were confirmed exact.

`overlay 1 +0x6D4C..+0x7130` — 996 bytes / 249 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule reordering plus a two-slot stack-home rewrite); source kept as decomp-permuter input. All 43 runtime text records were confirmed exact.

`overlay 1 +0x67C0..+0x69A0` — 480 bytes / 120 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via register-allocation reassignment across six sites); source kept as decomp-permuter input. Four runtime call records were confirmed exact.

`overlay 1 +0x04B4..+0x0758` — 676 bytes / 169 words: `overlay1ActivateObject` at `+0x04B4..+0x0614` and `overlay1FindClosestSample` at `+0x0614..+0x0758`. Both C bodies are guarded `NON_MATCHING` over `GLOBAL_ASM`, so they contribute **0 exact C bytes**; canonical linked-range and ROM equality comes from the assembly fallbacks. The activation candidate has the exact 88-instruction shape and relocation roles but a 50-word allocation mismatch. No object-, linked-, or ROM-exact C promotion exists for `overlay1FindClosestSample`.

`overlay 1 +0x01AC..+0x02D4` — 296 bytes / 74 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a schedule/register-home rewrite that also relocated a scale-relocation instruction); source kept as decomp-permuter input. All six runtime text records were confirmed exact.

`overlay 1 +0x0BD4..+0x10C0` — 1,260 bytes / 315 words across four functions (motion scaling, path interpolation, motion-point resolution, sampled curve length). NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via schedule/allocation/stack-home rewrites and local literal-addend edits); source kept as decomp-permuter input. All 44 runtime text records were confirmed authoritative.

`overlay1MeasureCurves` (`overlay 1 +0x0F84..+0x10C0`) plateau: exact 79-word size,
frame `0x70`, and five call-relocation sites; 27 masked words differ from `+0xC`.
The full flag lattice and 40-minute permuter produced no valid exact source.

`overlay 1 +0x19B8..+0x1CA4` — 748 bytes / 187 words, covering mode-state initialization and object-mapping construction. The first function is naturally exact apart from a trimmed alignment word. NON_MATCHING (second function): retired 2026-08-24 per ADR 0002 (was made to match via a representation-web rewrite selecting the retail schedule); source kept as decomp-permuter input. All 28 runtime text records were confirmed exact.

| Overlay | Range | Function | Bytes | Exactness | Donor |
|---:|---|---|---:|---|---|
| 1 | `+0x296C..+0x2AA4` | `overlay1AdvanceObjectGauges` | 312 | canonical object and linked ROM exact | Mickey-only |

`overlay 1 +0x2AA4..+0x2B4C` and `+0x3578..+0x3750` — 640 bytes / 160 words across three gauge and variable-record functions. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via representation-web rewrites, with two identity copies removed under whole-function edits); source kept as decomp-permuter input. All remaining runtime records were confirmed exact.

`overlay 1 +0x61F0..+0x6270` — 128 bytes / 32 words, cached-mode handling. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant-branch/identity-copy removal plus a register/schedule rewrite); source kept as decomp-permuter input. The 13 runtime records comprise three calls, five HI16, and five LO16 sites.

`overlay 60 +0x3488..+0x355C` — 212 bytes / 53 words, choice-slot reassignment. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a stack/address/cursor representation rewrite); source kept as decomp-permuter input. The 8 runtime records are four HI16 and four LO16 sites targeting resident offset `0x4D618`.

`overlay 15 +0xB94..+0xC6C` — 216 bytes / 54 words, rain rendering. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant address-producer removal plus a count-home/addend rewrite); source kept as decomp-permuter input. The 17 runtime records comprise two resident calls plus seven HI16 and eight LO16 local sites.

`overlay 36 +0x09B8..+0x0A60` (`overlay36` position-effect callback) contributes **168 exact bytes / 42 words**. Natural codegen owns every instruction; only eight bytes of tail alignment are trimmed. The exact 3 runtime records are one resident call and one HI16/LO16 pair for the Overlay 36 variant byte.

`overlay 15 +0x0428..+0x0500` — 216 bytes / 54 words, star motion. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant address-anchor removal plus an addend/schedule rewrite); source kept as decomp-permuter input. All 21 runtime roles (one resident call, twenty local data/BSS records) were confirmed.

`overlay 53 +0x0000..+0x011C` (`overlay53Initialize`) contributes **284 exact C bytes / 71 words**. The unguarded C is instruction-word exact; symbol renaming and trailing alignment trimming are metadata-only. The configured object retains all 30 runtime relocation roles, and the canonical promotion records exact linked ownership and a byte-exact full US ROM.

`overlay 53 +0x016C..+0x0240` (offset-entry copying) contributes **212 exact bytes / 53 words**. IDO emits it naturally with no normalization. Its exact 5 runtime records are one resident call and two local HI16/LO16 table pairs.

`overlay 1 +0x438C..+0x5BA4` — 6,168 bytes / 1,542 words, the central object-physics/state updater. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a large decoded schedule/register/frame rewrite over most of the function); source kept as decomp-permuter input. All 184 runtime relocations were confirmed present.

`overlay 15 +0x09E0..+0x0B7C` — 412 bytes / 103 words, moving-star camera updater. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via redundant HI-anchor removal plus a decoded addend/schedule rewrite); source kept as decomp-permuter input. All 39 runtime records (two calls, 33 BSS records, four data records) were confirmed.

`overlay 1 +0x3FD8..+0x438C` — 948 bytes / 237 words, a two-path five-phase transition-state machine. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a representation-pool plus schedule/register-web rewrite); source kept as decomp-permuter input. All 13 runtime relocations were confirmed exact; three intra-overlay loader calls keep raw zero fields in the static link while runtime resolves their separate identities.

`overlay 15 +0x0500..+0x06A4` — 420 bytes / 105 words, the starfield renderer. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a local-addend and branch-displacement schedule rewrite); source kept as decomp-permuter input. All 10 runtime records (two resident calls, six initialized-data sites, two constant sites) were confirmed.

`overlay 1 +0x3750..+0x3E48` — 1,784 bytes / 446 words, the path-selection and interpolation state machine. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a redundant-zero removal plus a relocation-aware schedule/register rewrite); source kept as decomp-permuter input. The static linker surface has 31 records; the runtime evidence covers all 63 relocation roles.

`overlay 36 +0x0818..+0x0914` — 252 bytes / 63 words, the nearby-height filter. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a redundant-zero removal plus a schedule/register rewrite); source kept as decomp-permuter input. All three runtime relocations were confirmed exact.

`overlay 1 +0x10C8..+0x19B8` — 2,288 bytes / 572 words, packed-record loading and group/metric construction. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a representation-pool plus relocation-aware schedule/register/frame rewrite); source kept as decomp-permuter input. Runtime evidence covers all 114 relocation roles; the static object's raw link surface is 32 records.

`overlay 13 +0x0000..+0x0124` and `+0x0188..+0x0284` (module initialization and record allocation) contribute **544 exact bytes / 136 words**. Natural typed source reproduces every instruction with no normalization. The configured objects retain exact 17-record and 7-record relocation surfaces.

`overlay 14 +0x1040..+0x1164` (`overlay14DispatchCommand`) contributes **292 exact C bytes / 73 words**. The unguarded C is instruction-word exact; only metadata-only symbol renaming and section trimming remain. Its 15 runtime relocation roles and seven-call static surface are exact, and the canonical promotion records an exact linked range and full US ROM.

`overlay 14 +0x1540..+0x1830` (`func_overlay_014_F0001540_1870E18`) — **752 exact C bytes / 188 words**, frame `0x80`, 26 of 26 relocation identities (2026-09-16, lane lm-a). The last twelve words were a `v0`/`v1` exchange between the entries-base web and the index reload: the base web had `v0` forbidden by the unused results of the two dispatch calls before the loop head, each a `v0` web occupying its block, so the calls whose results the body never reads are declared through the void `overlay14DrawPrimitive` surface the sibling TU already uses for the same shapes. The outer while and generated entry stride are the previous lanes' work. `gmake verify` and `promotion-proof` pass from the C; the route is `docs/lastmile-region-boundary.md`.

`overlay 3 +0x03B0..+0x0588` (`overlay3SelectScoredObject`) — **472 exact C bytes / 118 words**, frame `0x80`, 5 of 5 relocation identities (2026-09-16, lane lm-a). The loop subscripts the helper's array by index so strength reduction owns the walking cursor (L160); uopt then keeps the call result for the cached path, copies it into `a1` before the guard and hands both `count` reloads `v0`, the ROM's shape. One unused pointer declaration holds the `0x80` frame (an unused `f32` is eliminated); it is listed in `docs/cleanup-queue.md`. `gmake verify` and `promotion-proof` pass from the C; the route is `docs/lastmile-region-boundary.md`.

`overlay 55 +0x0000..+0x013C` (`overlay55Initialize`) contributes **316 exact C bytes / 79 words**. The unguarded C is instruction-word exact, with only trailing alignment trimming. Its 33 runtime roles and 21-record static surface are exact, and the canonical promotion records an exact linked range and full US ROM.

`overlay 84 +0x0DD0..+0x0F18` (`overlay84AdvanceCurrent`) — 328 bytes / 82 words — is Evidence A exact C after bounded permutation resolved the two-word spill residual. All words, the `-0x30` frame, five runtime relocation roles, linked overlay range, and full ROM match.

`overlay 36 +0x150C..+0x1688` (`overlay36UpdatePeers`) contributes **380 exact C bytes / 95 words**. The unguarded C is instruction-word exact; symbol renaming and trailing trimming are metadata-only. All six call identities, the linked owned range, and the full US ROM are recorded exact.

`overlay 86 +0x007C..+0x0158` (`overlay86ScaledVectorPosition`) contributes **220 exact C bytes / 55 words**. Its instruction stream is exact; filtering the loader-owned vector-table pair, renaming the call symbol, and trimming alignment are metadata-only. All three runtime roles, the linked range, and the full US ROM are recorded exact.

`overlay 36 +0x1378..+0x1470` (`overlay36SpawnAndUpdate`) — 248 exact bytes / 62 words. Recovering the second physical argument as `s32` yields natural 62/62 codegen with the exact three-call relocation surface; no instruction normalization is used.

`overlay 86 +0x02E4..+0x0444` (`overlay86SelectPosition`) contributes **352 exact C bytes / 88 words**. Node-first owner comparison and the five-local declaration order reproduce the retail schedule; only a metadata-only symbol rename remains. All three runtime roles, the linked range, and the full US ROM are recorded exact.

`overlay 86 +0x0158..+0x02E4` (`overlay86BuildTransform`) — 396 bytes / 99 words. Natural local-declaration order gives the exact frame, CFG, all five runtime relocation roles, linked owned range, and full-US-ROM hash without instruction normalization or compiler forcing.

`overlay 86 +0x0474..+0x0ECC` (`func_overlay_086_F0000474_18D22AC`) — **2,648 exact C bytes / 662 words**, frame `0xA8`, 38 of 38 relocation identities (2026-09-16, lane lm-a). The last seven words were two generated pointer loads that the ROM holds in `v0`: a value loaded in the same basic block as a call is denied `v0` by that call's unused result web, so the call before each load (`ext_o0_5a914`, whose result no site uses) is declared `void` and each load is written as the expression itself rather than through the declared carrier; an `if (1) { }` boundary after the call is the byte-identical alternative. The compiler's private pool (two float constants and the five-entry state-switch table) duplicates the shipped pool at data_rodata `+0x80`, so the six references are rebound to `gOverlay86StatePoolReloc` and the pool is externalized by digest, overlay 58's metadata-only form, with an externalized rodata ownership row in the atlas; the three same-module callees are `SYMBOL` records and are named through `*Reloc` placeholders. `gmake verify` and `promotion-proof` pass from the C; the route is `docs/lastmile-region-boundary.md`.

`overlay 36 +0x1214..+0x1378` (`overlay36SpawnOffsetA9`) contributes **356 exact C bytes / 89 words**. Natural codegen is instruction-word exact; only trailing alignment is trimmed. All four call relocations, the linked range, and the full US ROM are recorded exact.

`overlay 36 +0x0F20..+0x1084` (`overlay36SpawnDirectional`) contributes **356 exact C bytes / 89 words**. Natural codegen is instruction-word exact; only trailing alignment is trimmed. All four call relocations, the linked range, and the full US ROM are recorded exact.

`overlay 36 +0x1084..+0x1214` (`overlay36SpawnConditional`) contributes **400 naturally exact bytes / 100 words**. The compiler emits the exact owner with all four runtime relocation roles; no normalization or section trim is used.

`overlay 36 +0x0D8C..+0x0F20` (`overlay36SpawnLinked7F`) contributes **404 exact C bytes / 101 words**. Natural codegen is instruction-word exact; only trailing alignment is trimmed. Its seven-record relocation surface, linked range, and full US ROM are recorded exact. Together with the four adjacent exact owners, this validly closes the 1,764-byte `+0x0D8C..+0x1470` exact-C island.

`overlay 36 +0x0694..+0x07B0` (`overlay36SpawnTransient`) contributes **284 exact C bytes / 71 words**. The promoted source is instruction-word exact; only trailing alignment is trimmed. Its three-record relocation surface, linked range, and full US ROM are recorded exact.

`overlay 46 +0x0FD0..+0x112C` (`overlay46UpdateTransition`) contributes **348 naturally exact bytes / 87 words**. The exact owner needs no instruction normalization; only an independent compiler alignment word is trimmed. All 28 runtime relocation roles were confirmed.

`overlay 14 +0x1184..+0x12D8` (`overlay14UpdateTransition`) contributes **340 exact C bytes / 85 words**. The C instruction stream is exact; relocation filtering/rebinding, symbol renaming, and alignment trimming are metadata-only. All 28 runtime roles and the linked range are exact, and promotion commit `954f2267` explicitly records a successful full canonical verify.

`overlay 57 +0x4C18..+0x4D90` (`overlay57UpdateModeTrigger`) contributes **376 naturally exact bytes / 94 words**, frame `0x28`, and the object owns all 38 runtime relocation roles: 31 local HI16/LO16 records, five external calls, and two overlay-local jumps. Retired 2026-08-24 per ADR 0002, when the only known match needed a setup/index/register rewrite plus three asserted compiler-elided local-base records; reopened under its recorded authorization and closed by source and declared metadata alone. The setup array is a six-iteration constant loop: uopt unrolls it by four, peels the two remainder iterations — which fold to the literal stores of 0 and 0x30 — and leaves the unrolled body's induction variable register-resident at 2, a shape no straight-line reconstruction of the six stores reaches. The TU defines overlay 57's local BSS block, because IDO shares one `lui $at` across the two constant-index setup stores only for a locally-defined symbol and that shared materialisation is in the shipped bytes. Declaring the block `static` makes all 31 local accesses section-relative records whose addend is already the module-relative offset the shipped word carries, so the compiled immediates need no link-time adjustment; overlay 57's own runtime relocation table owns those records, and the declared `filter_elf_relocations.py` step drops them exactly as overlay 7's tail does. One offset-0 pad stays global as the proof's named `.bss` ownership witness. The block's offsets are also the corroboration for the layout: IDO 4-aligns `.bss` scalars but 8-aligns arrays, so the interior pads are scalars, and `reloc_surface.py` independently values `gOverlay57ModeFlag` at `0x144`, `gOverlay57Timer` at `0x118`, `gOverlay57Object` at `0x180` and `gOverlay57ObjectId` at `0x183` — object byte 3, which is how both `lbu a0,387(a0)` sites read it.

`overlay 36 +0x01D0..+0x0694` (`overlay36UpdateInteractiveEntity`) — 1,220 bytes / 305 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via two countdown-CFG corrections plus twelve schedule permutations and a decoded frame/register rewrite); source kept as decomp-permuter input. The object retains all 24 runtime relocation roles.

`overlay 41 +0x1C84..+0x1DE0` (`overlay41DrawItem`) contributes **348 naturally exact bytes / 87 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only the independent trailing alignment word is trimmed. The object retains all 14 runtime relocation roles.

`overlay 99 +0x0064..+0x021C` (`overlay99InitializeEntries`) contributes **440 naturally exact bytes / 110 words**. Declaring the address-taken spawn descriptor after the entry cursor moves its compiler-selected home from `sp+0x58` to the retail `sp+0x54`; the ordinary `-O2 -mips2 -32` object is then instruction-exact. The linked overlay retains all 7 runtime relocation roles, including both local-storage pairs and the exact `+0x90` addend, and the full ROM is byte-exact.

`overlay 99 +0x0638..+0x0800` (`overlay99BuildHeightGrid`) owns 456 bytes / 114 words with a `0x28` frame, and is exact canonical C at `-O2 -mips2 -32 -Wo,-loopunroll,0`: all 114 words, the `0x28` frame, and 29 of 29 relocation records at the target offsets, with the linked overlay text, the complete module and the full ROM byte-identical. Two source facts closed it. The extra word was IDO homing the unreferenced second parameter to its incoming argument slot; an empty test on that parameter, placed after the grid null check, removes the store, and only that position works -- earlier the store returns, later it also exchanges the point/value pool colours. The remaining three words were the clamp's negation: writing the magnitude back through `value` reuses its register, while spelling the store as `-value` gives the negation its own caller-saved temporary, which is what the target does. The earlier reading that the split carrier "regresses and swaps the v0/v1 pool" was measuring the two levers separately; they only work together. The empty test is recorded in `docs/cleanup-queue.md`.

`overlay 99 +0x0800..+0x0BA4` (`overlay99RenderSortedEntries`) owns
932 guarded bytes / 233 words. The surviving configured C predates the latest
field-store change and is 216/233 raw or 218/233 runtime-normalized, with the
exact `0x148` frame and ten runtime-backed relocation sites. The current
220/233 normalized claim is unretained historical evidence. The sole inbound
identity is overlay-local call `+0xD78`; exact linked function/module/ROM
evidence proves the assembly fallback only.

`overlay 99 +0x0BA4..+0x0DDC` (`overlay99RenderSegments`) contributes **568
exact C bytes / 142 words** with frame `0xA8` and no padding. Declaring the two
address-taken size outputs immediately after the coordinate deltas restores
their retail stack homes; spelling the display-list advance as direct
post-increment restores its four-register web, and explicit signed flag loads
select the retail halfword operations. All 15 runtime relocation records agree
by offset, type, effective identity, and addend: seven calls, three local-data
pairs, and the texture pair. ORT 1925 exports the owner, with sole local inbound
at module `+0x1324`. The linked range, complete overlay, and full ROM are
byte-identical.

`overlay 11 +0x0AF4..+0x0C88` (`overlay11InitializeFour`) contributes **404 naturally exact bytes / 101 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only independent section alignment is trimmed, and the link resolves three proved local addends. The object retains all 22 runtime relocation roles.

`overlay 11 +0x0000..+0x0150` (`overlay11Initialize`) contributes **336 naturally exact bytes / 84 words**. Typed source's compiler-emitted six-entry switch table exactly matches the existing runtime-relocated table at module `+0x2ED8`; the duplicate private section is discarded after rebinding a local `+8` text pair. The object retains all 31 runtime text relocation roles, and the retained table preserves its six `R_MIPS_32` roles.

`overlay 11 +0x11D0..+0x1398` (`overlay11UpdateSelection`) contributes **456 naturally exact bytes / 114 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only independent section alignment is trimmed. The object retains all 58 runtime relocation roles: ten calls and 24 HI16/LO16 pairs.

`overlay 19 +0x00AC..+0x01E0` (`overlay19BuildOutput`) contributes **308 naturally exact bytes / 77 words**. Ordinary `-O2 -mips2 -32` codegen is exact; only independent trailing section alignment is trimmed. The object retains all 10 `R_MIPS_26` runtime roles: three local calls and seven resident calls across five semantic imports.

`overlay 19 +0x0D78..+0x0F58` (`overlay19ClassifyEdge`) — 480 bytes / 120 words. NON_MATCHING: bounded configured full-TU C is exact-sized and frameless at 110/120 raw/normalized words, first `+0x138`, with no relocations or owned padding. All 119 flags and three trace-supported natural forms are nonexact. Its sole inbound is the local jump from `overlay19FindAdjacent+0xD8`; exact linked bytes come from the assembly fallback and contribute **0 exact C bytes**.

`overlay 22 +0x0A7C..+0x0D30` (`func_overlay_022_F0000A7C_1878B84`)
contributes **692 exact C bytes / 173 words** with the retail `0x88` frame.
Rotating the two independent negative cross-product terms, while retaining an
explicit dereference for the remaining product, reproduces IDO's floating-point
operand order without changing semantics. The configured instruction stream is
exact. Its LOCAL HI16/LO16 pair at function `+0x2C/+0x30` addresses the module
constant base, and its SYMBOL call at `+0x110` binds resident `sqrtf`; the
metadata-only call rebind restores the shipped placeholder identity without
altering an instruction. All three runtime records, the linked owned range,
complete overlay, and full US ROM are byte-identical.

`overlay 22 +0x0D30..+0x0E9C` (`func_overlay_022_F0000D30_1878E38`) — 364 bytes / 91 words. NON_MATCHING: identity-correct configured full-TU C has the exact `0x28` frame and 48/91 raw/runtime-normalized words, first `+0x10`; all 43 residual sites are register-only. The runtime contract is exact at all 12 offsets, types, identities, and addends: a LOCAL pair to count `+0xED0`, three LOCAL node-array pairs to `+0xEA0`, and SYMBOL calls to resident `partUpdateTriggers`, `func_80002FE0` twice, and `func_80006EA0`. The 119-row flag lattice is nonexact, one fidelity-clean allocator trace records 11 high-confidence integer webs, and three natural lifetime/loop forms produce no gain. ORT 1465 has seven resident inbounds from `func_80053868` (`+0x5F8/+0xCF0/+0xCFC`), `func_800557F8+0x158`, `func_80055D08+0x124`, `func_80055E50+0xF0`, and `func_800573C8+0x350`; Overlay 22 table-2 records 10 and 17 add local inbounds at module `+0x274/+0x5C4`. The function consumes object and flags in `a0/a1`; one local call also leaves an ignored count pointer in `a2`. The following `+0xE9C..+0xEA0` word is separately owned padding. Exact linked function/module/ROM evidence proves the assembly fallback only, so this owner contributes **0 exact C bytes**.

`overlay 99 +0x02A0..+0x0638` (`overlay99ApplySegment`) — 920 bytes / 230 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a bijective private-representation rewrite, plus removal of a 16-byte compiler-private `.rodata` duplicate already covered by the retained runtime table); source kept as decomp-permuter input. The object covers 13 static and 27 runtime relocation roles.

`overlay 11 +0x1398..+0x184C` (`overlay11UpdateMenu`) contributes **1,204 exact C bytes / 301 words**, at the exact `0x48` frame with no padding. All 102 runtime relocation offsets, types, identities, and addends are exact; symbol renaming and the section trim are metadata-only. The linked range and the full US ROM are recorded exact.

`overlay 63 +0x077C..+0x0928` (`overlay63UpdateSequence`) — 428 bytes / 107 words; the separate `+0x0928..+0x0930` eight-byte zero padding remains assembly with no C credit. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via restoring one folded identity copy and rotating a `v0`/`v1` register pair); source kept as decomp-permuter input. The object retains all 39 runtime relocation roles.

`overlay 11 +0x184C..+0x1A7C` (`overlay11UpdateTwoOptionMenu`) contributes **560 exact C bytes / 140 words**. The C instruction stream is exact; relocation filtering/rebinding and symbol renaming are metadata-only. All 46 runtime roles, the linked range, and the full US ROM are recorded exact.

`overlay 63 +0x0000..+0x01D4` (`overlay63Initialize`) — 468 bytes / 117 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via restoring a folded chain-address materialization and rotating a bounded register web); source kept as decomp-permuter input. The object retains all 46 runtime relocation roles.

`overlay 100 +0x0580..+0x094C` (`overlay100DrawMotion`) — 972 bytes / 243 words; its final four-byte zero word remains separately owned padding. NON_MATCHING: a complete 119-flag sweep and ten distinct source/allocation candidates leave 161 positional differences from `+0x0`; the retained fidelity-clean declaration order improves the natural frame from `0x88` to `0xB0`, versus target `0xC0`. The candidate and runtime surfaces each contain seven records (five calls and one HI16/LO16 pair), but only four offset/type sites align and candidate runtime identities remain unresolved. The earlier apparent match required a prohibited private representation rewrite. Every non-padding executable interval has C source, but `overlay100DrawMotion` remains guarded `NON_MATCHING`; Overlay 100 is not exact-closed.

`overlay 100 +0x0000..+0x0214` (`overlay100InitializeMotion`) contributes **532 exact C bytes / 133 words**. The configured object has the exact `0x78` frame and no padding. Its 13 static records agree with all runtime offsets and types. The runtime identities are the reserved `overlay:4093:+0x518` pairs at `+0x4/+0x8` and `+0x1C0/+0x1C4`; resident calls to `+0x2AE30` at `+0x68`, `+0x29598` at `+0x110/+0x120/+0x130`, and `+0x29B94` at `+0x150`; and local pairs to `+0x980` at `+0xAC/+0x108` and `+0x990` at `+0x1CC/+0x1D8`, all with zero addends. Eight data identities resolve directly from the ordinary object; linked-ROM equality proves the five friendly call proxies bind to the required resident identities. ORT 1456 exports the function, and its sole inbound is resident `func_800517E0+0xB48` at `vram:0x80052328`. The linked owned range and rebuilt ROM range are byte-identical.

`overlay 100 +0x038C..+0x050C` (`overlay100UpdateMotion`) contributes **384 exact C bytes / 96 words**. Replacing the expanded count/remaining loop with its natural `if (count--)` and `while (count--)` form restores IDO's otherwise-dead post-decrement carrier and closes the prior one-word structural gap. The configured object has the exact `0x18` frame and no padding. Its three runtime records agree by offset, type, identity, and addend: the call at `+0x3C` targets local `overlay100RemoveEntry` (`+0x278`), and the HI16/LO16 pair at `+0xD4/+0xD8` addresses module `+0x984` with addend 4. ORT 1930 exports the function and its sole inbound is local `overlay100ApplyValue+0x44`. The linked owned range, complete overlay, and full ROM are byte-identical.

`overlay 11 +0x1A7C..+0x1E4C` (`overlay11UpdateFiveOptionMenu`) contributes **976 exact bytes / 244 words**. The configured object retains 78 static relocation records, and its separately retained 20-byte switch table is also exact.

`overlay 63 +0x01D4..+0x074C` (`overlay63UpdateEffects`) — 1,400 bytes / 350 words. The C body is guarded `NON_MATCHING` over `GLOBAL_ASM`; its exact size does not establish exactness, and the current plateau records 139 differing positional words with the first mismatch at `+0x16C`. It contributes **0 exact C bytes**. The fallback supplies the exact linked range and ROM; the 71-record configured surface is not evidence of an exact C object.

`overlay 19 +0x01E0..+0x0A30` (`overlay19BuildPlanes`) — 2,128 bytes / 532 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete private-representation rewrite); source kept as decomp-permuter input. The retained relocation surface covers four calls. Every non-padding executable interval has C source, but `overlay19BuildPlanes`, `overlay19BuildAdjacency`, `overlay19ClassifyEdge`, and `overlay19BuildSpatialMasks` remain guarded `NON_MATCHING`; Overlay 19 is not exact-closed.

`overlay 98 +0x0144..+0x0234` (`overlay98CollectAccepted`) contributes **240 exact C bytes / 60 words**. A fidelity-gated IDO stack-home trace selected the natural declaration/lifetime form: removing two transient entry aliases and retaining the address-taken result as the third automatic reproduces the `0x50` frame. All six relocation sites and effective identities, the linked owner and complete overlay, and the full US ROM are exact. `overlay98CheckObject` at `+0x0848..+0x0A04` remains guarded `NON_MATCHING` and contributes 0 exact C bytes; its final 12 zero bytes remain separately owned assembly padding.

`overlay 71 +0x0278..+0x07A8` (`func_overlay_071_F0000278_18C9D98`) contributes **1,328 exact C bytes / 332 words**. Reordering only the existing semantic locals restores the retail point-array, cursor, state, index, and floating-point spill homes in the unchanged `0xE0` frame. All six runtime relocation offsets, types, and effective identities agree, including the local resource and global-flag pairs plus the resident calls. The two resident call names are rebound to their generated overlay-scoped proxies as reviewed metadata only; no instruction field changes. The linked owner, complete overlay, and full US ROM are byte-identical.

`overlay 65 +0x0080..+0x0BC0` (`overlay65UpdateParticles`) — 2,880 bytes / 720 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete private-representation rewrite across the function); source kept as decomp-permuter input. The retained surface covers 36 static relocation records and all 64 runtime roles. Every non-padding executable interval has C source, but `overlay65UpdateParticles` and `func_overlay_065_F0000C38_18C4EA0` remain guarded `NON_MATCHING`; Overlay 65 is not exact-closed.

`overlay 98 +0x0234..+0x0848` (`overlay98RenderReflections`) — 1,556 bytes / 389 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a private-representation rewrite that converted 36 runtime semantic relocations to a 16-record static proxy surface); source kept as decomp-permuter input. Every non-padding executable interval has C source, but `overlay98CollectUniqueY` and `overlay98RenderReflections` remain guarded `NON_MATCHING`; Overlay 98 is not exact-closed.

`overlay 13`'s three remaining owners—`overlay13UpdateRecord` at `+0x0284..+0x0508`, `overlay13DrawRecord` at `+0x0580..+0x0874`, and `overlay13DrawActive` at `+0x0874..+0x0B0C`—total **2,064 guarded NON_MATCHING bytes / 516 words**. All three C bodies remain behind `#ifdef NON_MATCHING` with `GLOBAL_ASM` fallbacks, so they contribute **0 exact C bytes** and do not close the module. Their fallback/static relocation surfaces do not establish object-, linked-, or ROM-exact C.

`overlay 11 +0x2714..+0x2948` (`overlay11UpdateModeSix`) contributes **564 exact C bytes / 141 words**. The C instruction stream is exact; relocation filtering/rebinding, symbol renaming, and alignment trimming are metadata-only. All 47 relocation sites, the linked range, and the full US ROM are recorded exact.

`overlay 15 +0x06E8..+0x09E0` (`overlay15InitStars`) — 760 bytes / 190 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via an address-lowering rewrite recovering three explicit global-address carriers); source kept as decomp-permuter input. The 15 runtime roles reduce to an exact 13-record split surface.

`overlay 15 +0x004C..+0x0428` (`overlay15InitStarsAndPalette`) — 988 bytes / 247 words, supplying C-source coverage for this interval but not exact closure. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a relocation-anchored carrier rewrite); source kept as decomp-permuter input. All 14 shipped runtime roles reduce to an exact six-record split surface. Six Overlay 15 owners totaling 3,012 bytes remain guarded `NON_MATCHING`, so the module receives no closure credit.

`overlay 15 +0x0428..+0x0500` (`overlay15MoveStars`) — 216 bytes / 54 words. NON_MATCHING: configured full-TU C emits 58 instructions with the exact `0x40` frame, first mismatch `+0x30`, and 33 ownership-aware differing words. The target's 21 runtime records cover the resident `starfieldFastMove` call and 20 LOCAL star-state/bounds records; C emits 25 because nine distinct scalar bound symbols require four extra HI16 producers. Pair structs/arrays, file-static scalars, and pointer-derived adjacent fields did not reduce the instruction count and worsened schedule or tuple alignment. The prior 119-row flag lattice is nonexact. ORT 1663 exports the owner, but there is no direct resident/cross-overlay relocation inbound and `+0x0500` begins the next function with no padding. JFG's same-sized `starfieldMove` supports the role only, not source identity. Preserve the assembly fallback; the next source lever must share one direct-load high half across each adjacent bound pair without materializing a general-register base.

`overlay 61 +0x1648..+0x17B8` (`func_overlay_061_F0001648_18C0A10`) contributes **368 exact C bytes / 92 words**. A disclosed one-iteration grouping preserves IDO's exact 0x38-byte frame and instruction allocation. The object retains all nine calls plus the local path-address pair, and the linked overlay and whole ROM are byte-identical.

`overlay 11 +0x1E4C..+0x22E8` (`func_overlay_011_F0001E4C_186A694`) — 1,180 bytes / 295 words. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete carrier-schedule rewrite); source kept as decomp-permuter input. The retained split text surface is 41 records; the five-entry runtime switch table and its five relocations remain in their original data/rodata owner.

`overlay 11 +0x22E8..+0x2714` — 1,068 bytes / 267 words, eliminating the former `+0x1E4C..+0x2714` middle deficit. NON_MATCHING: retired 2026-08-24 per ADR 0002 (was made to match via a complete carrier-schedule rewrite plus a link-only addend rebind); source kept as decomp-permuter input. The pre-link object's split text surface was 39 records.

`overlay 14 +0x0498..+0x0578` (`overlay14ResetMode`) — 224 bytes / 56
words. NON_MATCHING: retained configured evidence has the exact boundary and
`0x30` frame, with 11 raw sites and seven after normalizing four output LO16
addends. Those stores are instruction sites `+0x88/+0x90/+0x98/+0xA4` for
local data `+0xF8/+0xD8/+0xDC/+0xE0`; they are not instruction offsets. The
shipped runtime surface has 18 records. The extracted target object's ten
records omit four loader-owned pairs and collapse the `+0x54` call, whose
actual `SYMBOL` identity is `overlay14ReleaseOwner` at `+0x1B54`; the `+0xB0`
call is a local `JUMP` to `overlay14MoveCommandCursor` at `+0x578`. The first
callee identity is repaired; one configured V0 and full flag lattice precede
at most three `s3`-lifetime probes. No C credit is claimed.

| Overlay | C-owned non-text | Translation unit | Proof |
|---:|---|---|---|
| 15 | data/rodata `+0x0..+0x50`; BSS `+0x0..+0xA0` | `overlay_015.c` | configured object and linked ROM exact |
| 101 | BSS `+0x0..+0xFD0` (shared builder owner) | `func_overlay_101_F000571C_18E0F3C.c` (consumer `func_overlay_101_F00078F4_18E3114.c`) | linked BSS placement exact; both C bodies remain structure-mismatch |

| Overlay | Range | Function | Bytes | Exactness | Donor |
|---:|---|---|---:|---|---|
| 35 | `+0x000..+0x1E0` | `func_overlay_035_F0000000_1881CE0` | 480 | canonical object and linked ROM exact | Mickey-only |

`overlay 20 +0x07C4..+0x09DC` (`overlay20BuildTileCommands`) — **536 exact C bytes / 134 words**. Mickey's `0x90` frame and `sp+0x5C` array base prove the seven-entry chunk array; declaring it after the six preceding scalar/cursor locals reproduces both homes without padding or synthetic control flow. The configured object is instruction-word-identical, its sole helper call relocation is exact by type, offset, and identity, and the linked overlay and full ROM are byte-identical.

`overlay 20 +0x0204..+0x038C` (`overlay20UpdateObjectResource`) remains
`NON_MATCHING`: fresh configured full-TU C is exact-sized at 98 words with frame
`0x70` and 90/98 raw/normalized words, with eight register-only sites at
`+0xB0,+0xB4,+0xC0,+0xC8,+0x108,+0x10C,+0x110,+0x114`. All 119 compiler
configurations are nonexact. A stock-fidelity allocator/FIFO trace proves the
later `value0F`/`start` exchange is downstream of the scoped count carrier's
`v1` pool placement versus the target's temp `t6`; direct field access regresses
to 73/98. Runtime records prove calls to `trackGetTrack`,
`func_8000FEEC`, local `overlay20ConfigureResource`, and `sqrtf`; the fallback
object collapses those names to an assembly placeholder. A historical linked
C trial differed only inside the owned function, but no linked candidate
artifact survives. Current exact module/full-ROM evidence proves the assembly
fallback only. Pinned DKR v77/v80 and JFG scans are negative. Resume only with
a new legal pool-to-temp web-formation mechanism.

`overlay 34 +0x02C8..+0x0378` (`overlay34RemoveRecord`) — 176 bytes / 44 words. NON_MATCHING: bounded 2026-08-28 closeout retained the size-exact source after indexed-compaction, pointer-cursor, separated-count, and scoped-call probes. The best candidate matches 32/44 words with the first schedule/register residual at `+0x14`; it preserves one helper relocation and two repeated active-count HI16/LO16 pairs, while the target encodes its pointer load without relocation. Removing those source pointer relocations is prohibited, so no C credit is claimed.

`overlay 34 +0x00D4..+0x02C8` (`overlay34CreateRecord`) —
**500 exact C bytes / 125 words**, frame `0x30`. Rebinding `record = candidate`
after the resource call skips the fill-pointer `Ulod`s on uopt's block budget,
so `short16` and `byte12` share one block in ROM order. The configured object
is instruction-exact with all 12 relocation identities; the linked owned range
and the complete overlay section are byte-identical to the US ROM.

`overlay 14 +0x09F4..+0x0ACC` (`func_overlay_014_F00009F4_18702CC`) —
**216 exact C bytes / 54 words**. A bounded annotated-target permutation found
the inert block boundary that reproduces the retail allocator result. The
configured object is instruction-exact with the target's six relocation sites;
its additional state-anchor HI16/LO16 pair resolves to the shipped zero addend.
The linked owned range and the complete 10,944-byte overlay section are
byte-identical to the US ROM.

`overlay 84 +0x1060..+0x11F4` (`overlay84ActivateCurrent`) contributes **404
naturally exact bytes / 101 words**. Reusing the word-sized selector parameter
for the chosen byte preserves the retail register lifetime and `0x30` frame;
the configured object is instruction-exact and retains the expected global
HI16/LO16 pair plus three call relocations. The linked owned range and the full
5,752-byte overlay section are byte-identical to the US ROM.

`overlay 2 +0x0B70..+0x0C90` (`overlay2SplitRegion`) contributes **288 exact C
bytes / 72 words**. All 119 compiler configurations leave the configured
seven-word prologue schedule residual. A fidelity-clean IDO 5.3 `as1 -R`
trace identifies a physical-line tie between the function and loop headers;
placing those token-equivalent headers on one line produces the retail order.
The exact `0x30` frame and all nine runtime relocation tuples and identities
agree: local data at `+0x4/+0x24`, calls to `+0/+0x6E0/+0x49C` twice, and the
recursive `+0xB70` call. The linked owned range, complete overlay, and full ROM
are byte-identical.

`overlay 25 +0x000..+0x17C` (`overlay25InitializeEffect`) contributes **380
exact C bytes / 95 words**. Reading the retained owner through `state->owner`
reproduces the shipped `s0`/`s1` carrier allocation, while the measured
`-Wab,-r4300_mul` mode supplies the one FP multiply-hazard nop omitted by the
default schedule. The exact `0x40` frame and all nine runtime relocation
offsets, types, and identities agree: two calls each to resident
`func_8002A8C0` and `func_8002A8BC`, one call to `func_800299E8`, one reserved
data pair, and one Overlay 25 local-data pair. ORT 1225 and resident relocation
row 67 prove the sole inbound call; `+0x17C` starts the next function with no
padding. The linked owner, complete 1,904-byte overlay, and full ROM are
byte-identical. Pinned DKR v77/v80 and JFG donor rows for Overlay 25 remain
empty, so this is Mickey-derived source and does not support a JFG function or
module mapping.

`overlay 1 +0x0DF4..+0x0F84` (`overlay1ResolveMotionPoint`) contributes **400
exact C bytes / 100 words**. Assigning the measured distance through the
eventual scale local before its positive-value test preserves IDO's original
floating-point carrier web. The configured object has the exact `0x28` frame.
All 11 runtime records agree by offset and type; five identities resolve
directly from the ordinary object, and the unchanged runtime table plus the
exact linked ROM range proves the remaining three LOCAL pairs. The calls bind
to local `overlay1ActivateObject` (`+0x4B4`) and
`overlay1InterpolatePath` (`+0xCA8`), resident `sqrtf`, and resident
`func_8002A8C0`/`func_8002A8BC`. Overlay 92 calls the function at `+0x520`
and `+0x958`. The owned range, complete overlay, and full ROM are
byte-identical.

`overlay 1 +0x5ED4..+0x61F0` (`overlay1DispatchMode`) contributes **796 exact
C bytes / 199 words** (Tier A). The existing body compiles with the exact
`0x28` frame; the remaining discrepancy was link metadata, not allocation or
control flow. Its 61 raw and configured text relocation records agree with
the shipped runtime table by count, offset, and type. Static source identities,
the exact linked range, and the unchanged runtime table prove all 61
effective identities, including the retained table pair and generated
Overlay 8 call. The eleven external calls name canonical definitions with
their existing ABI, while four local calls retain their original pre-loader
addends through object-local aliases.

The eight-entry switch table remains in its initialized-data owner at
`+0x274`; its runtime LOCAL base lies at initialized-data `+0x110`, so the
stored addend is `+0x164`. Two existing compiler relocation records are
rebound to that anchor. All eight raw compiler table destinations agree
relative to the function entry before the duplicate compiler section is
externalized. Every retained instruction byte is untouched; only the TU's
12 trailing zero alignment bytes are trimmed. There is no function-owned
padding or new table credit. Both neighboring boundaries, the complete
overlay, and full ROM remain byte-identical.

### Overlay 26 effect-handler closure (2026-09-08)

`func_overlay_026_F0000D24_187B11C` in
`src/overlays/o026/overlay26HandleEffects.c` is now unconditional exact C.
The former plateau shard is retired; its prior evidence remains in Git history.

Tier A: Overlay 26 text `+0xD24..+0x1158`, ROM
`0x187B11C..0x187B550`, is 1,076 executable bytes with no owned padding.
The complete linked overlay and the extracted owned range equal the baserom.
The configured function retains all 269 compiler instruction words unchanged;
the existing trim removes only 12 zero section-alignment bytes. All 23 static
relocation offsets, types and runtime identities are exact. Declared per-object
symbol aliases preserve the stored call operands without moving resident symbols.

The configured baseline reproduced 11 raw and masked differences, frame
`0x50` versus target `0x48`, and identical register lanes. Workbench reported
`operand-mismatch`, with the `stack-home` lever overriding the generic
`constant-audit` routing for compiler-derived frame offsets. The earlier flag
lattice and single-alias/scope regressions were not rerun unchanged.

| Attempt | Natural source hypothesis | Words / raw differences | Frame |
|---|---|---|---|
| 1 | Register-qualified mode carrier | 269 / 11 | 0x50 |
| 2 | Byte mode carrier; only low five bits are observed | 270 / 267 | 0x50 |
| 3 | Explicit floating-point result for the nested square root | 269 / 11 | 0x50 |
| 4 | Replace the effect cursor with direct member accesses | 249 / 242 | 0x48 |
| 5 | Replace the object alias with the explicit square-root result | 269 / 38 | 0x48 |
| 6 | Remove the object alias and declare the mode carrier last | 269 / 19 | 0x48 |
| 7 | Remove the remaining redundant mode carrier | 269 / 0 | 0x48 |

Attempt 3 supplied changed evidence for the combined alias experiment; attempt
5 retained the register lanes and established displaced homes in the correct
frame. Attempt 6 corrected every named home, leaving two compiler temporary
homes displaced by one word. Attempt 7 corrected those remaining homes without
changing the frame. The final source uses the original parameters directly:
neither alias was modified or escaped, so widths, evaluation order, calls and
memory accesses remain unchanged. No diagnostic padding or inert operation is
retained. Exactness ended the packet; no stall or attempt-count cap was used.

Evidence is retained under ignored `build/wb/effects-attempts/`, together with
the baseline diagnosis, runtime relocation comparison, promotion proof and
linked owned-range byte comparison. Baseline prepared-input self-comparison
passed. The instrumented baseline also passed section/relocation/symbol
fidelity, but its missing frame-home records supplied no additional conclusion.

Validation: `gmake verify`, `tools/promotion_proof.py`, linked-range `cmp`,
`gmake check-overlay-syms`, `gmake cleanroom`, `gmake check-docs` and
`gmake check-scoreboard`. The full donor rescan cannot reproduce the pinned
JFG checkout: local revision `efd5abb1c796` differs from required
`c82affffe8f1`. No donor source was adopted. The existing atlas-digest refresh
updates ownership metadata only; the committed donor results are unchanged.

Next action: integrate the exact function commit and its compiler-learning
follow-up through the private campaign branch, then repeat integration gates.
`overlay 7 +0x0894..+0x0AA0` (`overlay7DispatchModes`) contributes **524 exact
C bytes / 131 words**. Extracting the unsigned flags sign bit through a left
shift followed by a right shift preserves the retail branch schedule and
restores IDO's target temporary allocation. The configured object has the
exact `0x20` frame and 23 text relocations; the unchanged runtime table and
linked ROM prove every effective identity, including two calls to resident
`mathRnd`, nine local calls to `overlay7CreateEntry`, four local calls to
`overlay7AppendEntry`, the two mode arrays, the resident flags word, and the
retained switch table.

The seven-entry switch table remains in initialized data at module `+0x18F4`
with stored LOCAL addend `+4`. Two existing compiler relocations bind to that
addend by symbol index only. The compiler-private 32-byte table section is
externalized behind an exact payload digest; no instruction field is edited
and the retained table receives no new code credit. ORT 1471 and five resident
calls plus Overlay 25 table-1 record 15 authenticate all six inbounds. The
owned range has no padding; it, the complete overlay, and the full ROM are
byte-identical.

### Overlay 16 gradient application closure

Tier A: `overlay16ApplyGradient` owns overlay 16 text `+0x1E0..+0x424`
(580 executable bytes), excluding the following twelve-byte padding. The
configured stock compiler emits 145 exact words with the target's `0x20`
frame and all six LOCAL relocation offsets, types, identities, and addends.
Storing then reading back the phase, separating the active cursor increment
from its condition, and defining input channels before gradient channels
restore the temporary and local register allocation. The linked owner and
full US ROM are exact. This completes the existing overlay-16 C translation
unit; it does not add credit for its previously matched functions or padding.

### Overlay 2 boundary classification closure

Tier A: `overlay2ClassifyBoundary` owns overlay 2 text `+0x2C4..+0x400`
(316 executable bytes). Its 79 configured words and six relocation offsets,
types, identities and addends are exact; the linked owner and full US ROM
match. Direct comparison stores and one shared return remove the boolean
carriers and restore the target branch layout. The existing overlay-specific
boundary globals preserve the LOCAL addends without generic-name collisions.

The inherited source-shape closure did not survive this combined change.
Direct comparison stores alone reduced 61 masked differences to 47; sharing
the return closed the residual and removed the surplus word. The aligned
buckets (exact, register naming, immediate, structural) changed from
32/28/4/18 to 79/0/0/0. A pointer-color force was explicitly declined, which
did not prove the source shape unavailable.

### Overlay 68 secondary-entry rebuild closure

Tier A: `overlay68RebuildSecondaryEntry` owns overlay 68 text
`+0x1250..+0x1438` (488 executable bytes). All 122 configured words, the
`0x40` frame, six used stack homes, and 19 relocation offsets/types are
exact. Seven identities resolve statically; the remaining twelve are proved
by the unchanged runtime table and exact linked ROM. The whole US ROM passes
`gmake verify`. ORT 1163 and its resident caller identify the owned function.

Replacing both manually rotated loops with counted `while` loops restores
the missing map-address materialization, probe-base copy, and call-argument
placement. Useful selected-entry and payload pointer intermediates restore
the frame; advancing the selected-entry pointer in two statements restores
the final operand draw order. The aligned buckets (exact, register naming,
immediate, structural) change from 65/28/9/21 to 122/0/0/0. Lexical scoping
alone remains ineffective, but the inherited structural plateau falls. No
unused padding local, alternate compiler flag, or instruction edit is used.

### Overlay 1 object-mapping closure

Tier A: `overlay1BuildObjectMappings` owns overlay 1 text
`+0x1A54..+0x1CA4` (592 executable bytes) inside the existing head TU.
All 148 configured words, the `0x70` frame and thirteen used stack homes
match. Sixteen relocation offsets, types and identities are proved with
static evidence, the unchanged runtime table and exact linked ROM. The whole
US ROM passes `gmake verify`; ORT 1167 and its resident caller establish the
inbound identity. The TU's configured `r4300_mul` flag is retained.

A counted clearing loop restores the partially unrolled initialization.
Indexed outer and inner loops let the compiler create the pointer induction
variables; reusing the data pointer and naming the inner data pointer restore
the allocation. Declaring the lookup as ten-byte rows supplies address scaling
instead of an integer multiply whose constant consumed a saved register. The
aligned buckets (exact, register naming, immediate, structural) change from
85/37/5/23 to 148/0/0/0. The old source-shape closure therefore falls.

The count is read at the target's later point, after state initialization.
The source uses overlay-specific data placeholders, and both runtime calls
into overlay 1 use `Reloc` placeholders so their stored zero addends cannot
be replaced by a callee's synthetic linked address. No neighboring function
or padding receives credit.

### Overlay 31 particle-asset initialization closure

Tier A: `func_overlay_031_F00002E8_187F808` owns overlay 31 text
`+0x2E8..+0x4F8` (528 executable bytes). The configured compiler emits
132 exact words with the `0x30` frame and all 24 relocation offsets, types,
identities and addends. The linked owner and full US ROM pass verification;
ORT 1672 and overlay 18's caller identify the function.

The runtime records prove that the supposed count scalar and state array
name the same storage. Replacing the invented separate scalar/readback with
one five-element array-clearing loop restores the partially unrolled
initialization and removes the surplus instruction. Earlier literal, cast,
and region variants did not repair the mistaken storage model. The aligned
buckets (exact, register naming, immediate, structural) change from
115/2/2/15 to 132/0/0/0. The inherited initialization closure therefore falls.

The existing provenance disclosure is retained. The palette-builder call
uses an overlay-specific `Reloc` placeholder for its zero stored addend;
the existing removable carrier section remains metadata only. No padding
or previously matched palette code receives credit.


### Overlay 63 sequence updater closure (2026-09-12)

`overlay63UpdateSequence` is ROM-exact over its 428-byte executable range,
overlay 63 text `+0x77C..+0x928`; its 39 relocation records have exact counts,
types, offsets and identities. The configured stock compiler produces all 107
words and the `0x20` frame, and `gmake verify` prints the expected US ROM hash.
The aligned buckets (exact/register/immediate/structural) moved from
83/7/8/9 to 107/0/0/0, with the missing instruction restored.

The inherited copy-spelling closure survives for its original types. Its
broader allocator closure does not: the runtime call identity resolves to
Mickey's `joyGetPressed`, whose canonical definition returns `u16`. The local
holds that result as `s32`, and the sequence is an array of button masks,
not an array of pointers. Those authoritative types restore the return-value
copy and its use across the control flow. The callee has an overlay-specific
relocation placeholder; no shared prototype or toolchain flags changed.


### Overlay 1 angle-candidate solver closure (2026-09-12)

`overlay1SolveAngleCandidates` is ROM-exact over its 556-byte executable range,
overlay 1 text `+0x64F8..+0x6724`. Its configured stock full-TU object and
linked bytes match all 139 words, the `0xA8` frame, and all four relocation
counts, types, offsets and identities. The three square-root calls and the
angle call resolve through the existing overlay-specific placeholders.
`gmake verify` prints the expected US ROM hash. The aligned buckets
(exact/register/immediate/structural) improve from 119/4/6/13 to 139/0/0/0;
the original candidate was two instructions short.

The inherited root-preheader closure falls. Defining the two-root count before
the discriminant region and its calls preserves the required entry guard.
Casting the initial value, changing its signedness, and moving only the
invariant denominator do not reach it; a narrow induction variable adds
unwanted narrowing instructions. Reusing the now-dead initial delta for the
signed radical separates that value from the quotient's floating argument
carrier without adding a local home. Finally, a switch with the two-solution
case, the one-solution case, then the default reproduces the return layout.
Other tested switch orders and nested-return forms leave the default value in
a separate block. No compiler flags changed. The unused historical loop-control
helper and the obsolete guarded handoff were removed.


### Overlay 26 draw-groups closure (2026-09-12)

`func_overlay_026_F0001158_187B550` is ROM-exact over overlay 26 text
`+0x1158..+0x1370`: 536 executable bytes, 134 words, the `0x58` frame and
all four relocation counts, types, offsets and identities. `gmake verify`
prints the expected US ROM hash. The aligned buckets
(exact/register/immediate/structural) move from 77/36/2/19 to 134/0/0/0;
the original clean candidate was one instruction short.

The inherited claim that the negation needs a second consumer falls: a typed
node-pointer table indexed by `4 - groupIndex` produces the required address
arithmetic directly, without the retired inert read. A natural four-group
countdown restores the setup ordering. Manually grouped command stores reach
eight differences; the configured SDK macros in the existing `n_audio/mbi.h`
then reproduce both the command-pointer allocation and store order. Replacing
only one command with its macro leaves a separate pointer colour; reconstructing
the complete command sequence is exact. The custom triangle-pointer command
remains explicit. Literal signedness, pointer-versus-integer null-argument
typing and a move-one statement climb did not close the manual-command shape.
Accepted direct colour forces correctly priced the constant-register residual,
but forced objects were diagnostic only. Stock output supplies the match.


### Overlay 29 draw-groups closure (2026-09-12)

`overlay29DrawGroups` is ROM-exact over overlay 29 text `+0x14C8..+0x16CC`:
516 executable bytes, 129 words, the `0x58` frame, and all three relocation
counts, types, offsets and identities. `gmake verify` prints the expected US
ROM hash. The aligned buckets (exact/register/immediate/structural) move from
86/28/3/13 to 129/0/0/0.

A typed node-pointer table indexed by `4 - groupIndex`, a natural countdown,
and the complete command sequence expressed through the existing SDK macros
reproduce the stock output. Flattening the group-one branch also restores its
control flow. The inherited manual-store allocator closure applies only to
that source shape: direct final-colour forcing prices the mask residual, but
forcing before the natural split changes the allocation being investigated.
Narrow mask types and declaration changes did not solve the manual-command
shape. The existing SDK macros restore the command-pointer webs and emission
order together. No forced compiler output or padding receives matching credit.


### Overlay 35 grid-mask builder closure (2026-09-12)

`func_overlay_035_F0000770_1882450` is ROM-exact over overlay 35 text
`+0x770..+0xB40`: 976 executable bytes, all 244 words, the `0x80` frame,
and an empty relocation table. `gmake verify` prints the expected US ROM hash.
The aligned buckets (exact/register/immediate/structural) move from
199/27/10/11 to 244/0/0/0.

The inherited frame warning survives: unconstrained declaration permutations
can improve a scalar score while growing the frame. Reconstructing useful
locals, their order, and indexed vertex access first reproduces every home.
Natural counted loops for the three axes then restore their narrowing and
branch schedules, invalidating the older order-sweep closure for the previous
loop shape. The remaining vertex-pointer/mask colour exchange was priced by
an accepted direct force. A natural counted vertex scan raises the mask's
block-reference denominator, ties its save ratio with the vertex base, and
lets the earlier vertex web win the required colour. Initializing the scan
index before loading the vertex base closes the last scheduling difference.
Only untouched stock compiler output supplies the match; compiler padding and
forced diagnostic objects receive no credit.

A same-input fidelity check of the final call-free TU passes text, data,
read-only data, relocations and symbols. Its correctly identified procedure
still records 29 first-phase and nine second-phase allocation decisions.
The inherited shortcut from no calls to second-phase-only allocation therefore
does not survive this control. Statement order remains a separate emission
and scheduling lever.


### Overlay 2 boundary chooser closure (2026-09-13)

Tier A: `overlay2ChooseBoundary` owns overlay 2 text `+0x6E0..+0xB70`,
1168 executable bytes / 292 words. Its untouched configured C output has
an exact 0x90 frame and 64/64 exact relocation identities. The linked owned
range and full ROM are byte-identical; the proof mode is
`static-plus-runtime-table-and-linked-rom`.

Direct countdown conditions, counter initialization at the coordinate-branch
join, and a byte-stride cursor definition on the loop-opener line close the
residual. The intermediate masked-zero result was not credited until canonical
linking and runtime-table identity checks passed. The
[function handoff](matching-triage-handoffs/overlay2ChooseBoundary.md) records
measured draw differences, controls, semantic limits and promotion evidence.
