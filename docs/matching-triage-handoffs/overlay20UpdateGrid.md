<!-- plateau-handoff:overlay20UpdateGrid:start -->
### `overlay20UpdateGrid` plateau handoff

- source: `src/overlays/o020/overlay20UpdateGrid.c`
- score: 173 differing words
- frame: 0x170
- relocations: 6
- first mismatch: +0x0
- summary: 173 words, delta 0, frame 0x170. missing-CSE overlapBase spill at entryCount test; dx/dy/amplitude inlines held size, minX/minY and pointer restore regressed.

Summary before this remeasure: Remeasured 2026-09-23: 185 masked at size delta +4 (216 of 215 words), frame 0x188 against 0x140; frame, registers and loop topology remain.

The opening one-sided word was an ALU on the entryCount test, the addiu that
materialises the overlap array and then spills it. Dropping that pointer and
writing the array index directly removes the spill. Inlining dx, dy and
amplitude kept size delta 0 (173 masked, frame 0x170, first mismatch +0x0, 6
relocs, callee ext_o0_6ec00). register, a scan-scoped pointer, or-zero, and
dropping minX/minY did not close the remaining 0x30 of unaccessed frame under
the array at sp+0xA8 (target sp+0x6C). Repeating the colour expression and
restoring the pointer both regressed size.
<!-- plateau-handoff:overlay20UpdateGrid:end -->
