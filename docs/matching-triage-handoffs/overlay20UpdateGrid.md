<!-- plateau-handoff:overlay20UpdateGrid:start -->
### `overlay20UpdateGrid` plateau handoff

- source: `src/overlays/o020/overlay20UpdateGrid.c`
- score: 0/215 words, promoted
- frame: 0x140
- relocations: 6
- first mismatch: none
- summary: Matched. Rewritten from the listing with indexed loops, a 32-entry overlap array declared last, and one index shared by the entry scan and the vertex countdown.

Summary before this remeasure: 173 words, delta 0, frame 0x170. missing-CSE pair is the overlapBase spill at the entryCount test line; dx/dy/amplitude inlines held size, minX/minY and pointer restore regressed. Stall: size is closed, frame stays 0x170 against 0x140.

Summary before this remeasure: Remeasured 2026-09-23: 185 masked at size delta +4 (216 of 215 words), frame 0x188 against 0x140; frame, registers and loop topology remain.

Lane w2-ovld (wave X, 2026-10-02) discarded the inherited body (cursor pointers, `-DEXPLICIT_BOUNDS -DSCAN_TOP_LOAD` per-file macros, scoped locals) and wrote it from the listing. Measured steps, each on the previous, with `tools/shape_product.py`:

- Natural indexed source (`for` over `gOverlay20Entries[i]`, `while (remaining--)`, `overlaps[overlapCount++]`): 70 masked at delta 0. Structure identical; frame 0x180 against 0x140.
- Frame: every declared local takes a cell here, the five cells under the locals are fixed, and nothing is stored to the frame but the overlap array. So the array is the last declared local and scalars plus array length equal 53 cells. `overlay20ConfigureEntry` caps the list at 32 entries, which gives 21 scalars: 66.
- A separate index for the inner loop (sharing the scan index made it one web holding s1): 55. The saved registers then all agree.
- `maxX = grid->minX + grid->width` (the target adds the origin first): 49.
- The scan index's dead `i = 0` web took t2 out of the temp ring (decision records). Reusing the countdown variable as the scan index removes that web: 24.
- `vertex` fetched before the vertex count: the three draws it spends come first, and the ring lines up: 0.

The per-file `-D` macros were dropped. `-Wab,-r4300_mul` stays: without it the function is 20 bytes short.

The opening one-sided word was an ALU on the entryCount test, the addiu that
materialises the overlap array and then spills it. Dropping that pointer and
writing the array index directly removes the spill. Inlining dx, dy and
amplitude kept size delta 0 (173 masked, frame 0x170, first mismatch +0x0, 6
relocs, callee ext_o0_6ec00). register, a scan-scoped pointer, or-zero, and
dropping minX/minY did not close the remaining 0x30 of unaccessed frame under
the array at sp+0xA8 (target sp+0x6C). Repeating the colour expression and
restoring the pointer both regressed size.
<!-- plateau-handoff:overlay20UpdateGrid:end -->
