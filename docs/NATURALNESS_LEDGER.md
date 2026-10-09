# Naturalness ledger: matched C a reviewer would question

Lane `r-natural`, 2026-10-07. Scope: every function outside `#ifdef NON_MATCHING`
in `src/main/*.c` and `src/overlays/**/*.c` (1,864 bodies found by the census).
No C was changed. This is a list to clean against before any completion claim.
Its companion is `docs/cleanup-queue.md`, which records forms already known to
be inert. This ledger also covers what that queue misses.

Severity: **A**, a reviewer would call it a fake match (semantically inert code
whose only job is to steer codegen). **B**, unnatural but defensible (carriers,
`goto`, `volatile`, pads beyond one, offsets cast through `u8 *`). **C**,
cosmetic (m2c names, `unkNN` fields, placeholder-typed `D_` externs, `+ 0`
indices, KSEG0 arithmetic written in C).

"Keep bytes" is a prediction, not a measurement. It is "unlikely" when the
comment above the function, `docs/cleanup-queue.md` or `docs/LANE_BRIEF.md`
says the construct is load-bearing. It is "likely" for type and name changes,
which do not reach codegen. Everything else is "unknown".

## Summary by severity

| Severity | Sites | Notes |
|---|---|---|
| A | 100 | in 75 functions; 35 of those functions have an entry in `docs/cleanup-queue.md`, 40 do not |
| B | 389 | 75 `goto`, 80 `volatile` (hardware registers excluded), 76 offset casts, 24 `if (1)` region openers, 12 permuter `new_var` functions, plus reviewed carriers and pads |
| C | 1495 | 916 placeholder-typed `D_` externs, 150 `D_` float externs, 174 pad locals, 147 KSEG0 sums, 51 `register` |

Site counts combine the mechanical census (part 3) with the read-through (part 2).
A sites are deduplicated by file and line. B and C may count a site twice where
both passes saw it. The census undercounts B and C: it does not see carrier
locals or role reuse, which only the read-through found.

## Summary by file (25 heaviest, by A then B)

| File | A | B | C |
|---|---|---|---|
| `src/main/anim.c` | 6 | 20 | 6 |
| `src/overlays/o001/overlay_001_tail.c` | 6 | 8 | 18 |
| `src/overlays/o058/func_overlay_058_F000138C_18B0574.c` | 6 | 7 | 17 |
| `src/overlays/o008/overlay_008.c` | 6 | 6 | 9 |
| `src/main/frontend_37D50.c` | 6 | 4 | 28 |
| `src/main/objects.c` | 5 | 23 | 75 |
| `src/main/track.c` | 4 | 26 | 91 |
| `src/main/textures_354C8.c` | 4 | 4 | 28 |
| `src/main/font.c` | 3 | 3 | 17 |
| `src/main/flash_58C10.c` | 3 | 0 | 8 |
| `src/main/runlink.c` | 3 | 0 | 17 |
| `src/main/menu.c` | 2 | 13 | 42 |
| `src/overlays/o079/func_overlay_079_F0001290_18CE230.c` | 2 | 2 | 0 |
| `src/main/diRcp.c` | 2 | 1 | 8 |
| `src/overlays/o058/overlay58DrawLargePointQuad.c` | 2 | 1 | 1 |
| `src/overlays/o058/overlay58DrawPointQuad.c` | 2 | 1 | 1 |
| `src/overlays/o066/func_overlay_066_F00004E0_18C6948.c` | 2 | 1 | 3 |
| `src/overlays/o089/overlay89UpdateStateAndParticles.c` | 2 | 1 | 0 |
| `src/main/shadows.c` | 1 | 25 | 23 |
| `src/main/particles.c` | 1 | 16 | 15 |
| `src/main/charControl.c` | 1 | 14 | 48 |
| `src/main/audio_manager_36D0.c` | 1 | 4 | 8 |
| `src/overlays/o101/overlay101BuildBorder.c` | 1 | 4 | 0 |
| `src/main/fx.c` | 1 | 3 | 23 |
| `src/main/menu_3B1A0.c` | 1 | 3 | 3 |
| 241 other files | 27 | 199 | 1006 |

## What a reviewer from another project would say first

1. **"These are fake matches."** `x * 0`, `x ^ 0`, `x OR 0`, `if (x) {}`,
   `if (c);`, `x = x;`, `entry++; entry--;`, `a && a` and `do { } while (0)`
   around one statement are the standard permuter fingerprints. decomp.me and
   the other N64 projects reject them on sight. ADR 0001 defines "matched" by
   bytes, so these functions are matched under it. They would not pass review
   in DKR, PD or the Zelda decomps. There are 100 such sites in 75 functions.
   40 of those functions have no `docs/cleanup-queue.md` entry, and the queue
   is the only place the tree admits to them.
2. **"`func_8003968C` is permuter output, not source."** It is a whole body on
   one line: `new_var` temporaries, five chained 64-bit all-ones masks around
   the literal `1`, and an empty `if` with a duplicated `!new_var` test. It is
   the worst single body the census found, and it is not in the queue.
   (Resolved 2026-10-08, lane c-3: the source is a four-iteration
   controller loop that IDO unrolls.)
3. **"The 2026-10-07 batch used the most stand-ins."** Eight of the 24 functions
   matched that day carry an A construct: `func_80038190` (five OR-zero probes),
   o066 `F00004E0` (two `width * 0`), `overlay68UpdateAnimation` (`index =
   (s16)index`), `func_80010654` (inert mask), `func_80049B14` (redundant store),
   `func_80011CDC` (single-statement region), `func_800349A4` (dead read) and
   `func_8004B1DC`. The comments above them are candid, but only `func_8004B1DC`
   is in the queue.
4. **"Where are the structs?"** Whole functions read fields as
   `*(T *)((u8 *)obj + 0xNN)` or through per-function `SG_`/`E920_` offset
   macros (`shadowGenerate`, `func_80016890`, `func_80007118`, `func_8000E920`).
   This is not fake. A type change does not reach codegen. But it reads as
   unfinished, and it is the cheapest category to fix.
5. **"Pads, `volatile` locals and role-swapped carriers everywhere."** One
   `s32 pad` is accepted practice. Six (`overlay100DrawMotion`), nine
   (`overlay99RenderSortedEntries`) or a `volatile s16 reservation` in eight
   o101 functions is not. Neither is a local named `speed` holding a bounce
   scale. Reviewers will read the carriers as allocator steering, and the
   comments say that is what they are.

Smaller points a careful reviewer would raise: `Powerf` is declared with one
argument in `shadows.c` and called with two in `charControl.c`.
`shadowGenerate` writes index 2 of a two-element array. `D_8007CA60` is read at
negative offsets, which suggests a missing table symbol. `vsprintf` and
`func_8005BA40` carry stale comments calling them non-matching.
`func_overlay_058_F000138C_18B0574` line 1349 tests the same global twice, which
may be an original typo. Check it against the target before calling it a probe.

## Part 1: severity A sites outside the reviewed functions

One entry per site, from the census (part 3), each confirmed by reading the line.
Sites in the 44 reviewed functions are listed in part 2 instead.

- `src/main/anim.c:578` `func_80050E9C` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: unknown - not measured; not in the cleanup queue.
- `src/main/anim.c:600` `func_80050E9C` **A**: multiply-by-zero stand-in for a constant.
  Natural: the literal `0`. Keep bytes: unknown - not measured; not in the cleanup queue.
- `src/main/anim.c:2938` `func_800557F8` **A**: empty `if (1) { }` regions (two, 2938 and 2948).
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/main/anim.c:3000` `func_80055970` **A**: empty `if (1)`/`if (0)` region.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/main/anim.c:3067` `func_80055B24` **A**: empty `if (1)`/`if (0)` region.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/main/audio_manager_36D0.c:658` `func_80003760` **A**: `entry++; entry--;` pair plus a single-statement region.
  **Resolved (lane c-3, 2026-10-08):** the cancelling pair, the region and the `group` local are gone; the row is `D_800C9238[point->unk23 - 1]` and the bound `D_80078F04[point->unk23 - 1]` (the typed count array, not a cast of `D_80078F00`). Byte-identical: `group` was the web numbered ahead of `entry`; reading the field at each use lets `entry` take v0 as shipped.
- `src/main/audiomgr.c:474` `func_80002188` **A**: `if (0) { }`.
  **Resolved (lane c-3, 2026-10-08):** deleted; it was inert (byte-identical without it).
- `src/main/camera.c:1283` `func_80023598` **A**: empty `do { } while (0)`.
  Natural: delete. Keep bytes: no - measured (lane c-3, 2026-10-08): deleted, the function changes size. The function comment ties the region at the if/else join to keeping `dlist` in its home; the natural statement that supplies that block boundary is not found yet.
- `src/main/charControl.c:1057` `func_8001D2A0` **A**: self-assignment.
  Natural: delete the statement. Keep bytes: no - measured (lane c-3, 2026-10-08). `D_800CB300 = D_800CB300;` emits nothing, but the use/def in the flag-1 block stops uopt sharing the global's address across `camGetListPtr()`, where the target splits it (`sw` through `at`, then a fresh `lui`/`addiu`); deleted, the function changes size. A natural statement that defines `D_800CB300` in that block without code is still to find.
- `src/main/diRcp.c:377` `diRcpDmaOffsets` **A**: empty if body or `if (c);` probe.
  **Resolved (lane c-3, 2026-10-08):** the empty `if (dList) {}` is now the guard of the debug print, `if (dList != NULL) { stubbed_printf(...); }`. `stubbed_printf` is an empty macro in this TU, so the body compiles away; the test is the one real read of `dList`, which is why the target homes only `command` (a1) and not `dList`. Every sibling whose parameters appear only in a print homes them all. Byte-identical.
- `src/main/diRcp.c:417` `diRcpMoveWd` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: no - measured (lane c-3, 2026-10-08). The `command = index` store in the G_MW_FOG case and the `command && command` read together keep the six empty cases as separate jump-table blocks; with either removed the cases merge into the exit and the function shrinks by 12 words. Not reproduced: both removed; `if (command == (u8) G_MOVEWORD)` and `if (command != 0)` guards on the final print (with and without the store). The same pair is marked `fakematch` in JFG's diRcp.c.
- `src/main/flash_5885C.c:89` `osFlashReadArray` **A**: OR/XOR with zero, or an all-ones mask.
  **Resolved (lane c-3, 2026-10-08):** `(page_num ^ 0) * D_800D77D8` is `page_num * D_800D77D8` with the page-size global declared `u32` (as the u32 page arithmetic it feeds; `flash_58570.c` now declares it the same way). The XOR stood in for the operand type: with an `s32` global the plain product loads the operands in the other order (3 words); swapping the operands does not help. Byte-identical.
- `src/main/flash_58C10.c:70` `func_800580F0` **A**: `do { } while (0)` around one call (three sites, 70-78).
  Natural: the bare calls. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Left (lane c-2, 2026-10-08):** the three regions are three blocks, so `&D_800D7830` is rematerialized per call rather than held in s0 (without them it moves to s0 and the frame grows to 0x38). Measured: no wrappers; one region over all three calls; one region over the middle two; an `OSPfs *pfs` local for the address; an empty test on the connector result. None keeps the bytes.
- `src/main/font.c:937` `func_8004C690` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Left (lane c-2, 2026-10-08):** the OR-zero keeps `k` a non-basic induction variable, which is what leaves the copy rolled with the `sltiu` counter; every plain counter is rewritten to `!=` or unrolled. Not re-measured here; the shard lists the spellings already tried.
- `src/main/frontend_37680.c:228` `func_80036DD0` **A**: `& 0xFFFFFFFF` on a u32 address.
  Natural: the bare address. Keep bytes: no - measured (lane c-3, 2026-10-08): `(u8 *) compressedAddr` without the mask differs in 23 words. Not explored further.
- `src/main/frontend_37D50.c:165` `func_80037414` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-2, 2026-10-08):** not re-measured; the cleanup queue lists 16+ three-argument spellings already tried.
- `src/main/frontend_37D50.c:371` `func_80037AEC` **A**: all-ones 64-bit mask chain.
  Natural: the bare value. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Resolved (lane c-2, 2026-10-08):** rewritten in func_800378A4's style: nested `do`/`while` loops with named `height`/`value`/`angle` locals, no `if (1)`, no mask, no `vertex[-1]`. A redundant `s32 row = 0;` initialiser (a dead store uopt deletes) numbers `row` before `phase`, the same first-reference rule as `dst` in func_8004C690. Byte-identical.
- `src/main/gameVi.c:161` `func_800336A8` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** the two `!=` mode tests and the empty `if (1) {}` (inherited from JFG viChangeMode) are a `switch (D_800D2F98 & 3)` with `case 2: case 3:` allocating and `default:` placing the second buffer after the first. Deleting the `if (1)` alone moves every global-address register (93 lines); the inverted `==`/`||` test differs in 11. Byte-identical.
- `src/main/menu.c:818` `func_8003968C` **A**: one-line body: `new_var` carriers, 64-bit mask chains, duplicated empty test.
  **Resolved (lane c-3, 2026-10-08):** the one-line permuter body is a four-iteration controller loop storing `-1`, `20` and `15` to `menuPreviousButtons[i]`, `menuRepeatX[i]` and `menuRepeatY[i]` (the TU's existing array aliases). IDO unrolls it completely, which is why the target loads each constant afresh per controller. Byte-identical.
- `src/main/menu.c:1163` `func_8003A2C8` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: no - measured (lane c-3, 2026-10-08). The target holds `&D_8007C090` in a1 for both the load and the store; three plain spellings (`screenMode &= 3` with a `u8 *current` for load and store, an `s32 mode` local with the global named, and the pointer for the load with the name for the store) all fold the address into `lui`/`lbu` pairs (22-24 words differ). The address-taken `modeBits` behind `modeBitPtr` is what keeps the address; leave until that is understood.
- `src/main/menu_3B1A0.c:190` `func_8003A754` **A**: empty if body.
  Natural: delete. Keep bytes: no - measured (lane c-3, 2026-10-08): deleting the empty `record != base` test differs in 20 words. Not explored further.
- `src/main/models_5B300.c:497` `func_8005ABA8` **A**: `(instance && instance) && instance` with an empty body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** deleted; the guard was byte-inert in the current tree. The two `if (x) { do { ... } while (x); }` wrap loops are plain `while` loops as well (IDO inverts them to the same code). Byte-identical.
- `src/main/objects.c:2278` `func_80006B04` **A**: empty `do { } while (0)`.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-2, 2026-10-08):** not re-measured; the cleanup queue row lists eleven placements already tried.
- `src/main/objects.c:3174` `func_80008028` **A**: multiply-by-zero stand-in for a constant.
  Natural: the literal `0`. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Resolved (lane c-2, 2026-10-08):** `updateModels = object->unk40->unkD0[1] != 0.0f;` and an inner `for (modelIndex = 0; ...)` loop; the zero lands at the join without the product. Byte-identical.
- `src/main/objects.c:3190` `func_80008028` **A**: empty if body.
  Natural: delete. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Resolved (lane c-2, 2026-10-08):** deleted; the outer loop is `objectIndex = D_800C949C; for (; objectIndex < D_800C9498; objectIndex++)` (folding the read into the initialiser loads the bound first, 8 words). Byte-identical.
- `src/main/objects.c:5339` `func_8000BB84` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Left (lane c-2, 2026-10-08):** the block boundary keeps the time scale out of the one-block body, where it interferes with every caller-saved float web and is split to memory (shard). Measured: deleted (+3 words); a separate `factor` local for the reflection (+3 words).
- `src/main/particles.c:993` `partObjFreeTriggers` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** the byte-offset cursor, the `if (count > 0) do { } while (++i != count)` shape and the empty `if (offset) {}` are one indexed loop, `for (i = 0; i < count; i++) { if (object->triggers[i].flags & 0x8000) ... }`. Deleting only the empty test swaps the index and offset registers (8 lines). Byte-identical.
- `src/main/pi.c:101` `piRomLoadCompressed` **A**: empty `if (1)`/`if (0)` region.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** the empty `if (1)` is gone; the copy-target assignment is the `else` arm of the `out == NULL` test, which gives it the same block of its own (the donors JFG and DKR both carry the `if (1) {}` as a fakematch). Deleted outright, the target is computed in v0 and copied to a1 (4 words). The inverted `if (out != NULL) { ... }` body differs in 7. Byte-identical.
- `src/main/runlink.c:500` `func_800320F0` **A**: empty if body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-2, 2026-10-08):** the extra reference ranks the `overlayCount` web into s1. Measured: deleted (38 words differ); a `count = overlayCount` loop-bound local, declared first or last, with the `if (1)` blocks (23) and without (87).
- `src/main/runlink.c:504` `func_800320F0` **A**: three empty `if (1) { }` on one line.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-2, 2026-10-08):** deleting them alone moves 38 words; with the `overlayCount` block also removed, 70. See the row above.
- `src/main/runlink.c:773` `runlinkUnloadOverlay` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-2, 2026-10-08):** the patch operation is read from the record at each use, as JFG's src/runLink.c does (`relocEntry->patchOperation` twice); the `patchOperation` local and the xor are gone. A local with a plain compare, or assigned inside the test, differs by 2 words. Byte-identical.
- `src/main/shadows.c:1338` `func_800180B4` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unknown - not measured; not in the cleanup queue.
- `src/main/spranim.c:271` `effectboxControl` **A**: `do { } while (0)` around one magic-offset store.
  Natural: `st->owner = arg0;` on a typed struct. Keep bytes: unknown - not measured; not in the cleanup queue.
- `src/main/textures_354C8.c:877` `func_80035E88` **A**: empty if body.
  Natural: delete. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Left (lane c-2, 2026-10-08):** the empty `if (tex) {}` is in JFG's published donor (src/textures.c func_800570D8_57CD8), so it is inherited, not added here.
- `src/main/track.c:4983` `func_800148E0` **A**: empty `if (1)`/`if (0)` region.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/main/vehicle_sounds.c:395` `func_8005830C` **A**: empty if body.
  Natural: delete. Keep bytes: no - measured (lane c-3, 2026-10-08): deleting the empty `if (cameras) {}` differs in 6 words. Not explored further.
- `src/overlays/o001/overlay_001_tail.c:1568` `overlay1StartTimerCallbacks` **A**: empty if body.
  **Left** (lane c-1, 2026-10-08): deleting the empty `if (entry->modeMask) {}` after `callback()` changes about 12 words; the re-read of `modeMask` after the call is load-bearing.
- `src/overlays/o001/overlay_001_tail.c:1625` `overlay1FindDirectionalObject` **A**: empty then-arm `if (a == b) { } else { ... }`.
  **Resolved** (lane c-1, 2026-10-08): `if (other == object) { continue; }` with the rest of the body unnested, byte-identical. The inverted `if (other != object) { ... }` is not (the loop re-lays out).
- `src/overlays/o001/overlay_001_tail.c:2320` `overlay1ConsumeNearbyPending` **A**: empty if body.
  **Left** (lane c-1, 2026-10-08): deleting the empty `if (other) {}` changes about 12 words; the extra reference to `other` before `other->state` is load-bearing.
- `src/overlays/o001/overlay_001_tail.c:2903` `overlay1UpdateValueCache` **A**: multiply-by-zero stand-in for a constant.
  Natural: the literal `0`. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/overlays/o003/overlay3FindClosestObject.c:39` `overlay3FindClosestObject` **A**: empty if body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Partly resolved (lane c-4, 2026-10-09):** the `if (&pad)` probe was byte-inert and is deleted (the `pad` declaration stays: without it the frame moves, 9 lines). **Left:** the empty `!cursor` test and the `if (1)` around the loop body; deleting either swaps the index and cursor registers (s3/s4; 10 and 7 lines). Measured without them: an indexed `objects[index]` loop (12), a pointer-walk `for (index = start, cursor = &objects[start]; ...; index++, cursor++)` (14), the same with the cursor set first (19), `while` with both increments at the tail (14), `*cursor++` (14). The fakes add references that rank the index web ahead of the cursor; a natural second use of `index` is still to find.
- `src/overlays/o007/overlay_007_tail.c:268` `overlay7FillValues` **A**: `((!value) & 0xFFFFU) && (!value)` with an empty body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** deleted, the counter takes v0 and its post-decrement copy v1, the target has them the other way round (4 lines). The two reads of the parameter before it is overwritten are what number its web first. Measured: a separate `s16 *entry` local (6), counter assigned before the pointer (4), a `for (i = 9; i >= 0; i--)` with `*value-- = 0xF0` (6), an indexed store below the end symbol (unrolled), `void` return with and without the parameter (4, 5), declaration initialisers with `!= 0` (6). The function overwrites its only parameter, so the signature itself is suspect; a caller-side reading of the argument would settle it.
- `src/overlays/o008/overlay_008.c:416` `func_overlay_008_F0001000_185ED58` **A**: empty if body.
  **Left** (lane c-1, 2026-10-08, measured): deleting the empty `if (unused) {}` changes about 100 words; the parameter reference removes its home store (cf. overlay99BuildHeightGrid). Prior note: Natural: delete. Keep bytes: unknown - not measured; not in the cleanup queue.
- `src/overlays/o008/overlay_008.c:1519` `overlay8ScaleOutputs` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/overlays/o012/func_overlay_012_F00000C4_186D344.c:41` `func_overlay_012_F00000C4_186D344` **A**: empty if body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** the empty `!i` test keeps `i` live after the clear loop, which stops IDO turning the `< 64` exit into `!= 64` (2 lines: `li`/`bne` against `slti`/`bnez`). The sibling overlay12Initialize resolved with `for (i = 0, entry = gOverlay12Entries; i < 64; i++, entry++)` because its counter is reused after the loop; here the same loop (2), `<= 63` (2), a plain `do`/`while` (3, 4), an indexed loop (8) and deleting the test (7) all differ. A natural later use of `i` is still to find.
- `src/overlays/o012/overlay_012.c:26` `overlay12Initialize` **A**: empty if body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** the `do`/`while` with `entry[-1]` and the empty `!remaining` test are one loop, `for (i = 0, entry = gOverlay12Entries; i < 64; i++, entry++) { entry->active = 0; }`; the counter (renamed `i`) is still the carrier of the trailing `gOverlay12Value1598` zero, which a literal store reorders (4 lines). Byte-identical.
- `src/overlays/o022/overlay22RemoveObject.c:60` `func_overlay_022_F0000D30_1878E38` **A**: empty if body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** deleting the empty `if (i)` moves 55 lines. Measured: the compaction written as a loop over `i` from `found` (`for` and `while`, 41 each, 19 lines longer) and `break` in place of `i = count` (56). The test keeps `i` live across the compaction so its save ranks below the cursor; a natural use of `i` there is still to find.
- `src/overlays/o027/overlay_027.c:391` `overlay27Activate` **A**: `state == 0 && state == 0` with an empty body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** the `savedObject` alias and the duplicated empty `state == 0` test are gone; the two state tests are `if (... == 4) { ... = 3; } else if (... == 2) { ... timer = 0; }` with one `return 1`. Two early returns without the probe differ in 6 lines, a `state` local in 6. Byte-identical.
- `src/overlays/o040/overlay40AddEntry.c:39` `overlay40AddEntry` **A**: all-ones 32-bit mask chain on a zero carrier.
  Natural: `entry->state = 0`, no carrier. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** without the mask chain, and with no carrier at all (literal zero, three `30` stores), the a0/v1 swap remains (4 lines); a `zero` local used for the state and subscript also 4; a `for` loop is unrolled (o040 unrolls). The crew note lists seven earlier lifetime variants.
  **Resolved (lane c-5, 2026-10-09):** the walk is `remaining = 8; while (remaining--) { ... entry++; }` with literal `0` and `30` stores; the carrier, the mask chain and `new_var2` are gone. The counted `while` keeps the counter in a0 and the 30 in v1 as shipped, which the `do`/`while (remaining--)` from 7 only reached through the extra carrier references. Byte-identical.
- `src/overlays/o043/overlay43AllocateResources.c:25` `func_overlay_043_F0001184_188B154` **A**: `if (&pad);` reads a pad address.
  Natural: delete; the pad alone. Keep bytes: unknown - not measured; not in the cleanup queue.
  **Resolved (lane c-4, 2026-10-09):** deleted; byte-inert. The `s32 pad[2]` declaration stays (one `s32 pad` moves the frame, 6 lines).
- `src/overlays/o057/overlay57UpdateInterface.c:188` `func_overlay_057_F0000954_18A454C` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** the empty `field08` test and the redundant `& 0xFFFF` on the same field work together: removing either alone moves 86-88 lines, removing both leaves only the config base address in v0 where the target has v1 (5 lines). An `O57Config0954 *config` pointer for the three tests (assigned in place or as an initialiser) differs in 30. Seek a natural statement after the switch that forbids v0 in that block (law 38: an earlier int-returning call).
- `src/overlays/o058/overlay58DrawLargePointQuad.c:90` `overlay58DrawLargePointQuad` **A**: empty if body or `if (c);` probe.
  **Resolved** (lane c-1, 2026-10-08): both probes and the dead `vertices += 3` replaced by the four-vertex colour loop `for (i = 0; i < 4; i++) { vertices->r = ... vertices++; }`, which IDO unrolls into the shipped stores; the loop's entry and exit supply the two block boundaries. Byte-identical.
- `src/overlays/o058/overlay58DrawLargePointQuad.c:108` `overlay58DrawLargePointQuad` **A**: empty if body or `if (c);` probe.
  **Resolved** (lane c-1, 2026-10-08): both probes and the dead `vertices += 3` replaced by the four-vertex colour loop `for (i = 0; i < 4; i++) { vertices->r = ... vertices++; }`, which IDO unrolls into the shipped stores; the loop's entry and exit supply the two block boundaries. Byte-identical.
- `src/overlays/o058/overlay58DrawPointQuad.c:90` `overlay58DrawPointQuad` **A**: empty if body or `if (c);` probe.
  **Resolved** (lane c-1, 2026-10-08): both probes and the dead `vertices += 3` replaced by the four-vertex colour loop `for (i = 0; i < 4; i++) { vertices->r = ... vertices++; }`, which IDO unrolls into the shipped stores; the loop's entry and exit supply the two block boundaries. Byte-identical.
- `src/overlays/o058/overlay58DrawPointQuad.c:108` `overlay58DrawPointQuad` **A**: empty if body or `if (c);` probe.
  **Resolved** (lane c-1, 2026-10-08): both probes and the dead `vertices += 3` replaced by the four-vertex colour loop `for (i = 0; i < 4; i++) { vertices->r = ... vertices++; }`, which IDO unrolls into the shipped stores; the loop's entry and exit supply the two block boundaries. Byte-identical.
- `src/overlays/o059/overlay59DrawFrame.c:32` `overlay59DrawFrame` **A**: empty if with a duplicated condition.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** deleted together with the two `register` keywords; byte-inert (so is placing `x += 4` before or after the entry). The packed call line is split. Reading `entry->owner` in the test without the `owner` local differs (15 lines). Byte-identical.
- `src/overlays/o059/overlay59PrepareEntry.c:77` `overlay59PrepareEntry` **A**: `& 0xFFFFFFFF` on a u32 value.
  Natural: the bare value. Keep bytes: no - measured (lane c-3, 2026-10-08): `handle = (u32) func_80034448((s32) value);` without the masked copy differs in 9 words. Not explored further.
  **Left (lane c-5, 2026-10-09):** the masked copy defines `handle` from the loaded `value`, which joins the load, the argument and the call result into one web coloured v0 (load into v0, `move a0,v0` per call). Every unmasked spelling propagates the load into the argument register a0 and gives the same 9-word object: `(u32) value`, `(s32) value`, a `(s32) (u32)` round trip, a separate `handle = (u32) value;`, and a `while ((value = descriptor[0]) != 0)` loop. The permuter's `new_var` zero on the second failure path was inert and is deleted.
- `src/overlays/o063/overlay63Initialize.c:89` `overlay63Initialize` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: no - measured (lane c-3, 2026-10-08): `while (index != -1)` and `while (-1 != index)` both differ in one word, the loop's `bnel` with its operands swapped (target compares the held `-1` register first). The XOR keeps `index` as the second operand.
- `src/overlays/o068/overlay68CheckKind.c:69` `overlay68CheckKind` **A**: OR/XOR with zero, or an all-ones mask.
  Natural: the bare operand. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** the same one-word `bnel` operand swap lane c-3 measured in o063: `currentKind != -1`, `-1 != currentKind` and `!= (s8)-1` each differ in 1 line; testing `mapping->kind` directly (10) and a `while (1)` with a `break` on the sentinel (10, both spellings) are worse.
- `src/overlays/o073/overlay73Initialize.c:101` `func_overlay_073_F0000000_18CAAC0` **A**: multiply-by-zero stand-in for a constant.
  Natural: the literal `0`. Keep bytes: unknown - not measured; not in the cleanup queue.
- `src/overlays/o079/func_overlay_079_F0001290_18CE230.c:111` `func_overlay_079_F0001290_18CE230` **A**: empty if body.
  **Resolved** (lane c-1, 2026-10-08): the empty `if (object->position.y) {}` was byte-inert and is deleted. The `else if (1)` arm opener was inert too and is now a plain `else`.
- `src/overlays/o079/func_overlay_079_F0001290_18CE230.c:124` `func_overlay_079_F0001290_18CE230` **A**: empty `if (1)`/`if (0)` region.
  **Left** (lane c-1, 2026-10-08): the block boundary between the `field3C` store and the emit call keeps the post-call linked-state load (`spawned = node->next->state`) in v0; deleted, it takes v1 at both sites (4 words). Storing through `node->next->state->field40` directly costs 26 words. Seek a natural block boundary at that point.
- `src/overlays/o082/overlay_082_tail.c:131` `overlay82Update` **A**: empty if body or `if (c);` probe.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Resolved (lane c-4, 2026-10-09):** the `(s32 *)state` cursor walk with `targetValues[-1]` and the empty `currentValues` test are an indexed loop, `for (index = 0; index < 6; index++) { state->values[index] += (((s32)targetRow[index] << 16) - state->values[index]) >> shift; }`, and the `targetValues` local is gone. Byte-identical. Not changed: the never-read `currentValues++` in the display loop (removing it moves 1 line) and its packed line (splitting the two stores onto two lines reorders one `sh`, so line placement reaches the scheduler here).
- `src/overlays/o089/overlay89UpdateStateAndParticles.c:152` `overlay89UpdateStateAndParticles` **A**: empty `do { } while (0)`, twice.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
- `src/overlays/o092/func_overlay_092_F0000308_18D6228.c:130` `func_overlay_092_F0000308_18D6228` **A**: self-assignment.
  Natural: delete the statement. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** deleting the self-copy moves 139 lines, as does a plain `f32 x` (with and without `x += (pathX - x) * wave`). The union keeps `x` address-taken in memory and the copy forces a store and reload there; not explored further.
- `src/overlays/o099/overlay99BuildHeightGrid.c:55` `overlay99BuildHeightGrid` **A**: empty if body.
  Natural: delete. Keep bytes: unlikely - queued in cleanup-queue.md as load-bearing.
  **Left (lane c-4, 2026-10-09):** `(void) unused;` emits the home store like a deletion (one word longer, 47 lines); an unnamed parameter is rejected by cfe; an `f32` second parameter is still homed (47).
- `src/overlays/o101/overlay101BuildBorder.c:72` `overlay101BuildBorder` **A**: empty `if (1)`/`if (0)` region.
  **Left** (lane c-1, 2026-10-08, measured): deleting the empty `if (1) {}` and declaring `trailingColor` `volatile` (as `leadingColor` already is) keeps size and registers but schedules two loads late (4 words); the join between the two `trailingColor` reads is load-bearing. Prior note: Natural: delete. Keep bytes: unknown - not measured; not in the cleanup queue.

## Part 2: read-through of 44 functions

The 24 functions matched on 2026-10-07 (from `git log --grep=Match --since=2026-10-07`
plus the same day's "matched" commits; `overlay17CreateChain` is still
`NON_MATCHING` and is excluded) and the 20 largest matched functions. Each was read
in full together with the comment block above it. The verdict line carries the
function's A/B/C counts.

### Matched on 2026-10-07

#### `overlay68UpdateAnimation`: A 1, B 2, C 2

One A-grade self-assign probe, not in the queue.

- `src/overlays/o068/overlay68UpdateAnimation.c:182` **A**: no-op self-assign `index = (s16)index` blocks forward substitution.
  Natural: delete it, and the argument reads `index < 1` directly. Keep bytes: unlikely - comment measures 2 words lost without it.
  **Left (lane c-4, 2026-10-09):** not re-measured; this is the measured case of law 48 (LANE_BRIEF), where the redefinition is what stops the forward substitution into the call argument.
- `src/overlays/o068/overlay68UpdateAnimation.c:111` **B**: two unreferenced locals unused1/unused2 for the 0x78 frame.
  Natural: one pad or a real local. Keep bytes: unlikely - frame size and home map.
- `src/overlays/o068/overlay68UpdateAnimation.c:152` **B**: assignment inside the if test `(index = state->keyframeIndex) >= count`.
  Natural: assign before the test. Keep bytes: unlikely - one web spans the loop.

#### `func_overlay_056_F00001A0_18A2F18`: A 0, B 4, C 4

Defensible, pad-heavy, no fake constructs.

- `src/overlays/o056/overlay_056.c:191` **B**: three unreferenced locals pad2/pad3/unusedF.
  Natural: fewer pads or a natural extra local. Keep bytes: unlikely - frame layout.
- `src/overlays/o056/overlay_056.c:164` **B**: segment addresses typed as extern `D_80000004[]`/`D_80000030[]`.
  Natural: segment-address constants or named symbols. Keep bytes: unlikely - relocation identity.
- `src/overlays/o056/overlay_056.c:177` **B**: matrix address built as `(u32)m + 0x80000000`.
  Natural: shared GBI macro with the JFG conversion. Keep bytes: likely - same addend either way.

#### `func_overlay_001_F0001D78_184E158`: A 0, B 4, C 2

Defensible, alias and pad issues only.

- `src/overlays/o001/func_overlay_001_F0001D78_184E158.c:145` **B**: two named pads pad5C/pad50 between live locals.
  Natural: real locals. Keep bytes: unlikely - comment ties them to homes.
- `src/overlays/o001/func_overlay_001_F0001D78_184E158.c:116` **B**: PREV macro casts D_1D64 while D_1D64 is also used raw.
  Natural: one typed global, one spelling. Keep bytes: unknown - comment says spelling mattered.
- `src/overlays/o001/func_overlay_001_F0001D78_184E158.c:163` **B**: the same flag test in two consecutive ifs.
  Natural: one block. Keep bytes: unknown - a store sits between them.

#### `func_800349A4`: A 1, B 3, C 1

One A-grade dead read, otherwise defensible.

- `src/main/textures_354C8.c:341` **A**: dead read `lowerWord = settings->lower[...].words.w0`, deleted by the optimiser.
  Natural: remove it. Keep bytes: unlikely - comment says 5 words regress.
  **Left (lane c-2, 2026-10-08):** not re-measured; L44 is the measured law for this assigned dead read.
- `src/main/textures_354C8.c:226` **B**: `numTextures` reused to save and restore D_8007BD90 around a call.
  Natural: a dedicated saved local. Keep bytes: unlikely - comment ties it to a spill.
- `src/main/textures_354C8.c:312` **B**: byte-offset table index `(u8 *)table + i * sizeof(...)`.
  Natural: `&table[i]`. Keep bytes: unknown - operand order is table first.

#### `func_80010654`: A 1, B 4, C 2

One A-grade inert mask plus loop-shape contortion.

- `src/main/track.c:2859` **A**: redundant `& 0xFFFF` on a u16 index.
  Natural: `node->planes[edge]`. Keep bytes: unlikely - comment says it spends a ring draw.
- `src/main/track.c:2816` **B**: two unreferenced pads pad94/pad8C.
  Natural: one pad or a real local. Keep bytes: unlikely - frame layout.
- `src/main/track.c:2851` **B**: split loop: `i = 0;` then `for (; i < n;)` with `i++` at the tail.
  Natural: `for (i = 0; i < n; i++)`. Keep bytes: unlikely - comment ties it to scheduling.

#### `func_80049B14`: A 1, B 2, C 3

One A-grade redundant store.

- `src/main/fx.c:1601` **A**: redundant `carry = 0` in an else arm (already 0).
  Natural: delete the else arm. Keep bytes: unlikely - comment says it is for block layout.
- `src/main/fx.c:1539` **B**: `switch (mode = record->state)` with mode never read after.
  Natural: `switch (record->state)`. Keep bytes: unknown - mode also feeds the outer test.

#### `func_80011CDC`: A 1, B 2, C 3

One A-grade single-statement region.

- `src/main/track.c:3564` **A**: `do { record = ... } while (0)` around one statement.
  Natural: plain assignment or `&table[i]`. Keep bytes: unlikely - comment cites checklist item 18.
- `src/main/track.c:3593` **B**: normalX and differenceX reused for unrelated values.
  Natural: distinct locals. Keep bytes: unknown - frame and colour effects.

#### `func_8001DD70`: A 0, B 5, C 2

Defensible, carrier-heavy but no fakes.

- `src/main/charControl.c:1526` **B**: float literals held as extern D_80081850..60 rodata cells.
  Natural: literals (blocked by GLOBAL_ASM pool order). Keep bytes: unlikely - pool placement per comment.
- `src/main/charControl.c:1585` **B**: `d` assigned inside a condition, then reloaded as a literal carrier.
  Natural: separate locals. Keep bytes: unlikely - comment ties it to a float register.
- `src/main/charControl.c:1553` **B**: `(&player->unk320)[i]` indexes from a scalar field address.
  Natural: an array field. Keep bytes: likely - same address math.

#### `overlay100DrawMotion`: A 0, B 1, C 3

Natural apart from the pad count.

- `src/overlays/o100/overlay100DrawMotion.c:36` **B**: six unreferenced locals unused0..unused5 for the 0xC0 frame.
  Natural: fewer pads or real locals. Keep bytes: unlikely - frame and spill homes.

#### `overlay36UpdateInteractiveEntity`: A 0, B 6, C 1

Defensible, one borderline type-pun CSE dodge.

- `src/overlays/o036/overlay36UpdateInteractiveEntity.c:233` **B**: type-pun reload `*(s32 *)&found[0]` to defeat CSE (borderline A).
  Natural: `found[0]->state64`. Keep bytes: unlikely - comment says it is load-bearing.
- `src/overlays/o036/overlay36UpdateInteractiveEntity.c:118` **B**: `found[9]`, an oversized array kept for the 0x80 frame.
  Natural: real buffer size plus a pad. Keep bytes: unlikely - frame size.
- `src/overlays/o036/overlay36UpdateInteractiveEntity.c:193` **B**: `(u8 *)entity + 0x28` passed as the angles pointer.
  Natural: `&entity->angles`. Keep bytes: likely - same address.

#### `func_overlay_066_F00004E0_18C6948`: A 2, B 4, C 2

Fake match by strict standards (two times-zero).

- `src/overlays/o066/func_overlay_066_F00004E0_18C6948.c:75` **A**: `y1 = 960 + width * 0`, an opaque zero against constant propagation.
  Natural: `y1 = 960`. Keep bytes: unlikely - comment says 13 words short without it.
- `src/overlays/o066/func_overlay_066_F00004E0_18C6948.c:77` **A**: `s = width * 0`, an opaque zero stand-in.
  **Left** (lane c-1, 2026-10-08, measured): the donor's own spelling `s = x0 << 5` (fxScreenEffect takes `s` from the x coordinate before the rescale) lets uopt fold the zero and re-lays out the function (over 200 words). Prior note: Natural: `s = 0` or a real macro parameter. Keep bytes: unlikely - same mechanism as line 75.
- `src/overlays/o066/func_overlay_066_F00004E0_18C6948.c:87` **B**: inline DMA packet with a magic command word and `+ 0x80000000`.
  Natural: the f3ddkr DMA display-list macro. Keep bytes: unknown - word store order.

#### `func_8000E920`: A 0, B 1, C 3

Natural control flow;  only untyped object offsets.

- `src/main/track.c:1880` **B**: E920_U8/S8/S16/S32/PTR byte-offset macros for every object field.
  Natural: typed object struct. Keep bytes: likely - field types do not change codegen.
- `src/main/track.c:1894` **C**: unused `s32 pad` sizing the frame.
  Natural: one pad is the accepted idiom. Keep bytes: unlikely - comment says it sizes the frame.

#### `func_800103D4`: A 0, B 3, C 0

Clean apart from the parameter macro and reuse.

- `src/main/track.c:2699` **B**: `#define object ((TrackAlphaObject *) objectArg)` stands in for a parameter.
  Natural: typed `TrackAlphaObject *object` parameter. Keep bytes: unknown - not measured.
- `src/main/track.c:2753` **B**: `kind` reused to hold the distance limit (carrier).
  Natural: separate limit local. Keep bytes: unlikely - comment says the register sharing is needed.

#### `func_80038190`: A 2, B 3, C 2

Five OR-zero probes, not in the cleanup queue.

- `src/main/frontend_37D50.c:550` **A**: FE38190_PROBE macro ORs the cursor with zero (used at 611 and 615).
  Natural: plain FRONTEND38190_EMIT. Keep bytes: unlikely - comment says L109 probes rank webs.
  **Left (lane c-2, 2026-10-08):** replacing both cursor probes with the plain packet macro differs by 8 words; they rank the two block-scoped cursors below `var_t2` (shard, L109).
- `src/main/frontend_37D50.c:619` **A**: `var_a2 = var_a2 OR 0` twice and `var_a3 OR 0` at the loop end.
  Natural: delete all three statements. Keep bytes: unlikely - comment says they set the t2 colour.
  **Left (lane c-2, 2026-10-08):** deleting the three loop-end probes differs by 38 words; they lift `var_a2`/`var_a3` above `var_t2` (save 20). A natural second use of both indices was not found within budget.
- `src/main/frontend_37D50.c:599` **B**: K0 arithmetic `(u32)ptr + 0x7FFFFB00u` and split subtractions.
  Natural: OS_K0_TO_PHYSICAL(ptr) - 0x500. Keep bytes: unknown - comment says the split form is needed.
- `src/main/frontend_37D50.c:591` **B**: viewport written as s16 at byte offsets on a u8 pointer.
  Natural: Vp element with typed vscale/vtrans. Keep bytes: likely - types only.

#### `func_80037C74`: A 0, B 2, C 1

Natural;  one carrier block and a struct cast.

- `src/main/frontend_37D50.c:502` **B**: `(FrontendVertex **) &D_8007BE88` reinterprets a struct global as an array.
  Natural: typed buffer member. Keep bytes: unknown - may change addressing.

#### `func_8001E5C4`: A 0, B 1, C 3

Natural;  one reused float carrier.

- `src/main/charControl.c:1742` **B**: `speed` reused to hold the bounce scale.
  Natural: separate f32 scale local. Keep bytes: unlikely - comment says this closed the match.

#### `func_8001FC50`: A 0, B 2, C 2

Natural allocator;  pointer-in-int reuse.

- `src/main/models.c:518` **B**: `s32 i` holds a pointer address, then the loop index.
  Natural: pointer local for the state block. Keep bytes: unlikely - comment cites the reuse.
- `src/main/models.c:534` **B**: `(&instance->pointsA)[i]` indexes adjacent members as an array.
  Natural: `points[2]` array member. Keep bytes: likely - same layout.

#### `shadowGenerate`: A 0, B 6, C 2

No fakes;  heavy untyped offsets and reuse.

- `src/main/shadows.c:213` **B**: SG_* byte-offset macros for every object/surface/model field.
  Natural: typed structs. Keep bytes: likely - types only.
- `src/main/shadows.c:279` **B**: writes index 2 of a 2-element u16 array via `(u8 *)D_8007945C + 4`.
  Natural: declare it as one ShadowGenerateAngle. Keep bytes: likely - type change only.
- `src/main/shadows.c:356` **B**: `j` reused as the trap result carrier.
  Natural: `if (TrapDanglingJump(...))` as in the sibling arm. Keep bytes: unlikely - comment says j is needed.
- `src/main/shadows.c:274` **B**: Powerf declared with one argument here, called with two in charControl.c.
  Natural: one shared correct prototype. Keep bytes: unknown - arity changes the a1 setup.

#### `func_80016890`: A 0, B 4, C 2

No fakes;  unused frame locals and offsets.

- `src/main/shadows.c:562` **B**: three unused locals kept for the frame (copy of arg2p, distance, radius).
  Natural: one pad, or real uses. Keep bytes: unlikely - comment says they give the frame.
- `src/main/shadows.c:710` **B**: `*(u8 **)((u8 *)(s32) D_800CB284 + 8)` pointer-int-pointer casts.
  Natural: typed struct global. Keep bytes: likely - types only.
- `src/main/shadows.c:583` **B**: `point0` reused for the distance clamp, a ratio and an extent.
  Natural: separate locals. Keep bytes: unlikely - comment says one web needed.

#### `overlay83DrawStrip`: A 0, B 3, C 0

No fakes;  alias pointer and K0 magic.

- `src/overlays/o083/overlay83DrawStrip.c:15` **B**: `extern u8 D_80000000[]` symbol standing for the K0 base.
  Natural: named vertex base with a K0 macro. Keep bytes: unknown - relocation identity.
- `src/overlays/o083/overlay83DrawStrip.c:71` **B**: vertex address as `(u8 *)strip + idx*size + 0x800000F0`.
  Natural: `&strip[idx].vertices` through a K0 macro. Keep bytes: unlikely - comment says the byte form is needed.

#### `func_overlay_079_F0000FA0_18CDF40`: A 0, B 1, C 1

Natural.

- `src/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c:87` **B**: `delta = len` copy carrier, `len` reused for four roles.
  Natural: separate locals. Keep bytes: unlikely - comment cites stack homes.

#### `func_8004B1DC`: A 2, B 3, C 1

Duplicated empty test and inert mask (in the queue).

- `src/main/font.c:318` **A**: empty duplicated test `if (D_8007D540 && D_8007D540) { }`.
  Natural: delete it. Keep bytes: unlikely - queue says 25 registers differ without it.
  **Left (lane c-2, 2026-10-08):** not re-measured; cleanup queue: removing it restores 25 register differences.
- `src/main/font.c:364` **A**: redundant `& 0xFFu` on a u8 width.
  Natural: `x += font->characterWidth`. Keep bytes: unlikely - comment says temp selection.
  **Left (lane c-2, 2026-10-08):** not re-measured; queued with the test above (L43 family).
- `src/main/font.c:208` **B**: `D_800D64F2` alias extern for `D_800D64E8[0].y2`.
  Natural: one symbol. Keep bytes: unknown - alias may set the hi/lo split.

#### `func_80028FCC`: A 0, B 0, C 2

Natural (three real calls in one short-circuit).


#### `func_8001953C`: A 0, B 0, C 1

Clean (two typed-return casts).


### The twenty largest (bytes from `symbol_addrs.us.txt`, overlays by body length)

#### `vsprintf`: A 0, B 3, C 5

Natural glibc/JFG port;  pads and the sign-byte test only.

- `src/main/diprint.c:485` **B**: sign test by a byte read of a float `*((s8 *) &spD0) < 0`.
  Natural: `spD0 < 0.0f`, as the 'f' case does. Keep bytes: unlikely - byte load against float compare.
- `src/main/diprint.c:187` **B**: several unused pad locals across cases (i, unused2, v1, a0).
  Natural: delete, or one counted pad. Keep bytes: unknown - frame census decides.
- `src/main/diprint.c:133` **C**: stale comment calls the matched function a relocation-layout mismatch.
  Natural: update or remove the comment. Keep bytes: likely - comment only.

#### `func_8005BA40`: A 0, B 0, C 6

Clean PD n_sndplayer adaptation;  cosmetic only.

- `src/main/gsSnd.c:254` **C**: stale NON_MATCHING plateau comment on a matched function.
  Natural: update the comment. Keep bytes: likely - comment only.
- `src/main/gsSnd.c:307` **C**: unreachable `state == 1` check right after the 4/5-only guard.
  Natural: keep as PD-inherited, with a note. Keep bytes: unknown - dead branch may still emit.

#### `func_8003D4FC`: A 0, B 5, C 3

Natural JFG-style renderer;  untyped offsets and rotated loops.

- `src/main/particles.c:520` **B**: address arithmetic `(s32)vertexStart + 0x80000000` (four sites).
  Natural: K0 conversion macro. Keep bytes: likely - same add.
- `src/main/particles.c:544` **B**: hand strength-reduced `(n << 3) + (n << 1) + 8` (two sites).
  Natural: `n * 10 + 8`. Keep bytes: unknown - IDO may choose other shifts.
- `src/main/particles.c:631` **B**: `points[-1]` read after `points += 2` (two sites).
  Natural: read `points[1]` before advancing. Keep bytes: unknown - changes the schedule.

#### `func_8000590C`: A 1, B 7, C 2

Matched but contorted;  one inert guard and allocator carriers.

- `src/main/objects.c:1987` **A**: empty `if (D_800C94A8 > 0x100) { }` after the append (not in the queue).
  Natural: real overflow handling or none. Keep bytes: unlikely - comment says it breaks a 3-way tie.
  **Resolved (lane c-2, 2026-10-08):** the guard now carries its debug-only report, `#ifdef _DEBUG` around `osSyncPrintf("ObjList Overflow %d!!!\n", D_800C94A8)`, the overflow print DKR's spawn_object compiles out at the same place. A live `stubbed_printf` with the string emits the literal into `.rodata`, so the report is preprocessed out instead. Byte-identical.
- `src/main/objects.c:1950` **B**: dead `u8 *aligned` reused as the unk48 nested integer (queued).
  Natural: dedicated s32 temp. Keep bytes: unlikely - a new local moves the 0x90 frame.
- `src/main/objects.c:1938` **B**: redundant `(u8 *)(u32)` second spelling of one store address.
  Natural: single spelling. Keep bytes: unlikely - comment says it splits a web.
- `src/main/objects.c:1838` **B**: `resultSize` carries four roles.
  Natural: separate locals. Keep bytes: unknown - frame slots are tight.

#### `func_80010B4C`: A 1, B 3, C 2

Readable body;  one inert OR-zero plus triple-add indexing.

- `src/main/track.c:3113` **A**: inert `bit = bit OR 0` in the retry loop (queued).
  Natural: delete. Keep bytes: unlikely - queued as load-bearing.
- `src/main/track.c:3106` **B**: `index + index + index` triple-add (eight sites).
  Natural: `index * 3`. Keep bytes: unknown - multiply-by-3 lowering differs.
- `src/main/track.c:3144` **B**: pointer passed as `(s32) rel` to an s32 parameter.
  Natural: fix the func_800115E4 prototype. Keep bytes: likely - same argument register.

#### `func_8004C8D8`: A 0, B 0, C 2

Natural display-list builder with only cosmetic issues.

- `src/main/font.c:957` **C**: unused second parameter named `unused`.
  Natural: drop it or name its real role. Keep bytes: likely - arity is set by the callers.
- `src/main/font.c:1046` **C**: Gfx state copied word by word through w0/w1 (two pairs).
  Natural: `*displayList++ = state[0];` struct copy. Keep bytes: unknown - struct copy can schedule differently.

#### `func_800563B4`: A 0, B 1, C 3

Clean collision test, fine to call matched.

- `src/main/anim.c:3381` **B**: vectors are f32[3] locals cast to (AnimVec3f *) at five call sites.
  Natural: AnimVec3f locals passed by address. Keep bytes: likely - comment says memory homes are needed.
- `src/main/anim.c:3540` **C**: untyped extern f32 D_8008420C, possibly a pool literal.
  Natural: write the literal, or give it a real name. Keep bytes: unknown - relocation kind not checked.

#### `func_8003FB98`: A 0, B 2, C 4

Natural DKR-derived body, no fakes.

- `src/main/particles.c:1442` **B**: u8 colour table read as `*(u32 *)` and then shifted/masked three times.
  Natural: three byte reads colorTable[0..2]. Keep bytes: likely - the target probably does a word load.
- `src/main/particles.c:1493` **B**: impossible test `(particle->flags & 1) == 2` (always false).
  Natural: probably an original bug, keep it with a comment. Keep bytes: likely - the dead branch must exist in the target.
- `src/main/particles.c:1402` **C**: real data read through fields named pad04 and pad1C.
  Natural: rename the descriptor fields. Keep bytes: likely - names only.

#### `levelInit`: A 0, B 2, C 4

Natural JFG-shaped init, no fakes.

- `src/main/level.c:405` **B**: `resourceId &= 0x3FFF;` repeated in all four arms.
  Natural: mask once at the call. Keep bytes: unlikely - comment says per-arm form is required.
- `src/main/level.c:460` **B**: field resourceB8 holds an id and then a pointer via `(s32)` cast.
  Natural: union or separate field. Keep bytes: likely - layout is the same.

#### `func_8000B3CC`: A 0, B 3, C 4

Defensible match with no fakes;  literal respelling is the oddest part.

- `src/main/objects.c:5202` **B**: `volume`/`savedY` reused as move deltas before their named roles.
  Natural: separate dx/dy locals. Keep bytes: unlikely - frame and colour depend on homes.
- `src/main/objects.c:5297` **B**: `0.100000001f` respelled so it is not merged with 0.1f.
  Natural: 0.1f at both sites. Keep bytes: unlikely - comment says the pool needs two entries.
- `src/main/objects.c:5269` **C**: `(negativeDot + negativeDot)` spelled as a doubled add.
  Natural: `-2.0f * dot`. Keep bytes: unlikely - comment says it is a uopt spill pattern.

#### `func_80028564`: A 0, B 2, C 1

Natural JFG-derived flow, no fakes.

- `src/main/main.c:1223` **B**: s32 global D_8007A188 cast to `(u8 *)` and indexed.
  Natural: declare D_8007A188 as u8 *. Keep bytes: likely - type only.
- `src/main/main.c:1278` **B**: magic byte offset `levelGetLevel()[0x83]`.
  Natural: level header struct field. Keep bytes: likely - same load.

#### `func_80034E54`: A 2, B 3, C 2

Two inert empty-if steering blocks, would be called fake.

- `src/main/textures_354C8.c:410` **A**: empty `if (frameIndex) {}` after the truncating assignment.
  Natural: drop it. Keep bytes: unlikely - comment says truncation must stay in the join block.
  **Left (lane c-2, 2026-10-08):** not re-measured within budget.
- `src/main/textures_354C8.c:481` **A**: empty `if (tableFlags) {}` between the w0 and w1 copies.
  Natural: drop it. Keep bytes: unlikely - comment says it splits a tie.
  **Left (lane c-2, 2026-10-08):** not re-measured within budget; no DKR or JFG donor carries this settings copy.
- `src/main/textures_354C8.c:499` **B**: frameIndex scaled in place, one local in two roles.
  Natural: separate textureBase local. Keep bytes: unlikely - comment says the last 5 words need it.

#### `func_80007118`: A 0, B 3, C 1

No fakes but untyped;  byte-arena casts throughout.

- `src/main/objects.c:2503` **B**: whole body uses byte-arena casts `*(T *)(object + 0xNN)`.
  Natural: typed object/payload structs. Keep bytes: likely - same loads once typed.
- `src/main/objects.c:2758` **B**: byte-stride loops `payload + 0x134 + j`, `j += 0xC`.
  Natural: array subscripts over typed members. Keep bytes: unknown - stride form may decide the loop.

#### `func_overlay_058_F000138C_18B0574`: A 6, B 6, C 4

Five codegen probes, one duplicated test, hidden resets.

- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:691` **A**: `i &= 0` self-reading reset after the case 12 colour call.
  Natural: plain `i = 0` before the title loop. Keep bytes: unlikely - comment says the delay-slot reset dies (46).
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:718` **A**: empty `if (columnStep != 0);` probe in the case 12 title loop.
  **Left** (lane c-1, 2026-10-08, measured): deleting it changes about 19 words; it is the stride web's +10 that keeps `columnStep` above `i` (case 13 comment).
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:764` **A**: empty `if (columnCount != 0);` read of a dead local.
  **Resolved** (lane c-1, 2026-10-08): deleted; byte-identical on the current source shape (each alone and both together).
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:849` **A**: empty `if (columnCount != 0);` probe in the case 13 portrait loop.
  **Resolved** (lane c-1, 2026-10-08): deleted; byte-identical on the current source shape (each alone and both together).
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:1421` **A**: `i += (i * 0) + 1;` times-zero unit increment.
  **Resolved** (lane c-1, 2026-10-08): plain `i++`, byte-identical.
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:1349` **A**: duplicated condition, the same `== 3` test on both sides of an OR.
  **Left** (lane c-1, 2026-10-08, measured): a single `== 3` test changes the object (the externalized `.rodata` digest no longer matches). Prior note: Natural: single test, or the intended second global. Keep bytes: unknown - may be an original typo, read the target.
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:370` **B**: `(0xFF - (i = 0))` index reset hidden in a colour arg (six sites).
  Natural: `i = 0;` statement then plain 0xFF. Keep bytes: unlikely - comments say the reset lands in a delay slot.
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:304` **B**: unused `cursor` and `portraitX` declarations kept for frame cells.
  Natural: remove, or a used local of the same frame. Keep bytes: unlikely - comments say frame cell.
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:424` **B**: dual inductions: letter1/textY subscripts while `i` tests (five loops).
  Natural: one index per loop. Keep bytes: unlikely - comments cite one-web-per-name.
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:701` **B**: locals reused against their names (opponent as column X, rowY as index).
  Natural: role-named locals. Keep bytes: unlikely - comments say roles chosen for colours.
- `src/overlays/o058/func_overlay_058_F000138C_18B0574.c:1286` **B**: byte-arena slot `(u8 *)saves + idx * 32` via a dead local.
  Natural: `&saves[index]`. Keep bytes: unlikely - comment says add operand order.

#### `amPlayAudioMap`: A 0, B 2, C 4

Natural DKR-derived body, only a pad and donor leftovers.

- `src/main/audio_manager_36D0.c:254` **B**: dead `if (cameraCount != 1)` inside the `cameraCount == 1` arm.
  Natural: delete (unreachable). Keep bytes: unknown - check DKR's donor shape first.
- `src/main/audio_manager_36D0.c:270` **C**: `*(u32 *)&pitch` float-as-int argument (also 343).
  Natural: typed setter parameter. Keep bytes: likely - DKR/libultra idiom.

#### `func_80041530`: A 0, B 4, C 2

Real logic, raw DL words and pointer arithmetic.

- `src/main/particles.c:2151` **B**: `((void **)((u8 *)&D_8007CA60 + n*8))[-4]` reads below a pointer global.
  Natural: typed table extern indexed. Keep bytes: unknown - a missing table symbol below it.
- `src/main/particles.c:2209` **B**: hand-built DL words with `addressBase = 0x80000000`.
  Natural: f3ddkr packet macros. Keep bytes: unknown - comment says hoisted temps.
- `src/main/particles.c:2174` **B**: `((f32 *)entry->points)[i * 3 + k]` flat view of a struct array.
  Natural: `entry->points[i].x`. Keep bytes: unlikely - comment says i*12 regresses 87.

#### `func_overlay_060_F0000334_18BA10C`: A 0, B 8, C 3

No fake-match constructs;  carriers and arena casts.

- `src/overlays/o060/overlay60Prefix.c:250` **B**: unused local `object` kept only as a frame home.
  Natural: `s32 pad`, with the stale comment fixed. Keep bytes: likely - a rename only.
- `src/overlays/o060/overlay60Prefix.c:372` **B**: redundant mask on a 1-bit field `(field9 ^ 1) & 1` (four sites).
  Natural: `bits.field9 ^= 1`. Keep bytes: unknown - the bitfield store masks anyway.
- `src/overlays/o060/overlay60Prefix.c:458` **B**: low byte of an s32 global via `((u8 *)&g)[3]` (two globals).
  Natural: type the global as u8. Keep bytes: unknown - byte load against word load.
- `src/overlays/o060/overlay60Prefix.c:688` **B**: byte-arena slot step `(u8 *)slots + i * 32`.
  Natural: `&slots[i]`. Keep bytes: unlikely - comment measures the add operand order.
- `src/overlays/o060/overlay60Prefix.c:346` **B**: locals reused for unrelated roles (spare, row, count, i).
  Natural: one local per role. Keep bytes: unlikely - comment says identity is load-bearing.

#### `overlay1UpdateObjectPhysics`: A 2, B 5, C 4

Two empty-if steers and a volatile reread.

- `src/overlays/o001/overlay_001_tail.c:1282` **A**: empty if body around a call `if (func_800299E8(0, 127) != 0) { }`.
  **Left** (lane c-1, 2026-10-08): the bare call changes about 4 words (the result test is load-bearing). Plausibly a stripped debug body; seek the body or a real use of the result.
- `src/overlays/o001/overlay_001_tail.c:1439` **A**: empty if body `if (predicate() != 0) { }`.
  **Left** (lane c-1, 2026-10-08): the bare call changes about 11 words (the result test is load-bearing).
- `src/overlays/o001/overlay_001_tail.c:1414` **B**: volatile reread `*(volatile f32 *)&state->speedLimit`.
  Natural: `state->speedLimit`. Keep bytes: unlikely - forces the reload after a store.
- `src/overlays/o001/overlay_001_tail.c:1225` **B**: magic float indices `tuning[x + 50]`, `tuning[16]`.
  Natural: typed tuning struct. Keep bytes: likely - same offsets.

#### `func_overlay_008_F0001294_185EFEC`: A 4, B 5, C 3

Three single-statement regions and a dead parameter store.

- `src/overlays/o008/overlay_008.c:703` **A**: `do { } while (0)` around one clamp statement.
  **Left** (lane c-1, 2026-10-08, measured): both wrappers removed (plain `if` clamp and plain `D_10 *= factor;`) changes about 7 words. Prior note: Natural: plain `if (factor < 0.1f) factor = 0.1f;`. Keep bytes: unlikely - comment says the region count steers.
- `src/overlays/o008/overlay_008.c:704` **A**: `do { D_10 *= factor; } while (0)` around one statement.
  Natural: `D_10 *= factor;`. Keep bytes: unlikely - same region-count comment.
- `src/overlays/o008/overlay_008.c:1008` **A**: `do { index = steeringInput >> 4; } while (0)`.
  **Left** (lane c-1, 2026-10-08, measured): plain `index = steeringInput >> 4;` changes about 75 words. Prior note: Natural: plain statement. Keep bytes: unlikely - comment says no shift temp.
- `src/overlays/o008/overlay_008.c:735` **A**: dead store to a parameter `update = 0.0f;`.
  **Left** (lane c-1, 2026-10-08, measured): deleting the dead store changes about 3 words (L48 family: it stops forward substitution of the conversion). Prior note: Natural: delete. Keep bytes: unlikely - comment says it moves index to v1.
- `src/overlays/o008/overlay_008.c:812` **B**: `goto block_74` into an else arm (m2c label name).
  Natural: restructure the shared store. Keep bytes: unknown - shared store block.
- `src/overlays/o008/overlay_008.c:730` **B**: comma-assign carriers in conditions (three sites).
  Natural: plain reads before the if. Keep bytes: unlikely - decision records cite them.

#### `func_overlay_052_F000063C_189ACAC`: A 0, B 4, C 3

Clean;  documented carriers and a duplicated case arm.

- `src/overlays/o052/overlay52TailB.c:588` **B**: case 4 duplicates the case 3 body verbatim.
  Natural: `case 3: case 4:`. Keep bytes: unknown - IDO may emit both copies.
- `src/overlays/o052/overlay52TailB.c:533` **B**: `iconX` carries a time difference, negated in place.
  Natural: `s32 diff = -racer->timeDifference`. Keep bytes: unlikely - comment says the negate ranks the web.

## Part 3: census by pattern

Mechanical grep over the matched bodies, with `NON_MATCHING`/`NON_EQUIVALENT`/`#if 0`
branches and comments stripped. Lines were checked by eye for the A rows only.
Patterns with no hits in matched code: `#line` directives, `asm` statements and
hex-spelled float literals.

| Pattern | Severity | Sites | Functions |
|---|---|---|---|
| empty if / if-semicolon | A | 21 | 17 |
| do/while(0) region wrapper | A | 20 | 15 |
| OR/XOR-with-zero or all-ones mask probe | A | 14 | 12 |
| multiply-by-zero stand-in | A | 8 | 7 |
| self-assignment | A | 3 | 3 |
| duplicated condition | A | 3 | 3 |
| all-ones 64-bit mask chain | A | 2 | 2 |
| increment/decrement pair | A | 1 | 1 |
| volatile for codegen | B | 80 | 64 |
| magic field offset through cast | B | 76 | 25 |
| goto control flow | B | 75 | 29 |
| `if (1)` region opener | B | 24 | 21 |
| word read through type pun | B | 12 | 7 |
| byte-arena pointer arithmetic | B | 12 | 8 |
| permuter `new_var` temporaries | B | 12 | 12 |
| float reached through pointer cast | B | 3 | 3 |
| comment names a fakematch/probe | B | 3 | 2 |
| D_ extern with placeholder type | C | 916 | file scope |
| unused pad local | C | 174 | 117 |
| D_ float extern (possible pool literal) | C | 150 | file scope |
| KSEG0 address arithmetic in C | C | 147 | 75 |
| register keyword | C | 51 | 34 |
| `+ 0` index or macro field zero | C | 36 | 12 |
| (void) cast of unused | C | 7 | 6 |
| do/while(0) macro wrapper | C | 3 | 2 |

The A rows above are before reclassification. Eight `do { } while (0)` wrappers
around multi-statement groups were moved to B, and one libultra-derived empty
`if` in `__scHandleRSP` was moved to C. Multi-line empty `if` bodies (49 in
all) came from a second pass and are in part 1.

### B and C hot spots

- **magic field offset through cast**: main/shadows.c (`func_80017BCC` 16), main/objects.c (`func_8000590C`, `func_80007118`), main/charControl.c.
- **goto control flow**: `overlay19ClassifyEdge` (22), `func_8001F520` (6), `func_8000FAE0` (5), `vsprintf` (5, glibc-derived, natural).
- **volatile for codegen**: `func_80039E34` (nine `volatile` reads into a `volatile` stack struct), `func_80055F64`, `volatile s16 reservation` pads in eight o101 functions, `volatile` parameters in o008/o009/o014/o041/o097.
- **unused pad local**: `overlay99RenderSortedEntries` (9), `func_80010178` (7), `overlay15InitStars` (5), `overlay100DrawMotion` (6, found by the read-through).
- **permuter `new_var` temporaries**: `func_8003968C`, `func_80012574`, `overlay40AddEntry`, `overlay80UpdateContact`, `overlay101DrawTransformed`, `func_8004D40C`, `func_80050BF4`, `func_80034434`.
  **Lane c-5 (2026-10-09), byte-identical throughout:** removed in `overlay40AddEntry` (a counted `while (remaining--)` from 8; see its A row), `func_8004D40C` (the 0x80 register copy was inert; literal tests), `func_80034434` (the `&value` alias and mask are a `u8` parameter, as its sibling `func_80034424` takes; IDO homes a narrowed argument and masks it), `overlay101DrawClock` (inert copy of the seconds angle; the 1,553-character expanded-macro line is split with the O101_* macros) and `overlay59PrepareEntry` (inert zero). In `overlay80UpdateContact` the `contactState` alias was inert; the two never-read aliases only reserved frame words and are one `s32 pad[2]`. Renamed where load-bearing: `overlay101DrawTransformed` (`primColorCommand`/`syncCommand`, the nested-assignment carriers), `overlay21ApplyPriorities` (`selected`; the direct test differs in 10 words, a `continue` grows the function), `overlay68PromoteSecondary` (`primaryCopy`; dropping the chained assignment moves the stack homes, 10 words, and passing `primary` differs in 14).
- **register keyword**: `func_8004D840` (gzip `huft_build`, natural in its donor), elsewhere m2c or permuter residue.
- **KSEG0 address arithmetic in C**: `+ 0x80000000` on display-list addresses. It is the JFG/DKR idiom, but it should be one macro, not 121 spelled sums.
