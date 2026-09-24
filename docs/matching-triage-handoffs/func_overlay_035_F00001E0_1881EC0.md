<!-- plateau-handoff:func_overlay_035_F00001E0_1881EC0:start -->
### `func_overlay_035_F00001E0_1881EC0` plateau handoff

- source: `src/overlays/o035/func_overlay_035_F00001E0_1881EC0.c`
- score: 356/356 words
- frame: 0x40
- relocations: 63
- first mismatch: none
- summary: ROM-exact and promoted: 356 words, frame 0x40, 63 of 63 relocation identities, unforced. DKR-style global model accesses, post-decrement copy loops, walkers read back from the stored colorData fields, and the model table walked through its global.

Lane B3-rl35 (Track B, 2026-09-23) on base `6d05abd3`. Start: delta -8, masked 262, aligned residual after shadow 159; the reader labelled both pairs missing-CSE, owned by the second-loop preheader and the colour-copy loops.

- Every model access spelled through `D_o35_current_model` (header fix-ups and both segment loops written as DKR `for` loops): the loop-condition reload is reused by the next body and the second loop's preheader starts from constants. Delta -8 to -20 (the tail pair now isolated), 262 to 194 masked.
- Copy loops `while (remaining--)`: delta 0; the target's `move` of the count is the post-decrement's old-value copy (the reader's "missing-CSE" here was that copy).
- Colour and flag walkers read back from `colorData->colors` / `->flags` after storing them (uopt forwards the stores): 145 masked, all naming.
- Heap test through `D_o35_current_model` instead of `D_o35_model_heap`: 145 to 21. The heap value becomes a ring temp.
- Segment-count loop without the local `model`, and the count in a local of its own (it had shared `i`'s web with the table scan): 21 to 14.
- `cursor += dataOffset * 4` and `temp_s4 + 8 + j * 3`: operand order, 14 to 12. `temp = model; temp += 0x9F000 - size`: 12 to 10.
- Table scan and both reads through `D_o35_model_table` (no `modelTable` local): 10 to 0. The address web's loop uses outrank `0x9F000`, so the address takes `s3`.
- Removing the unused pad costs 2 words (mdl's home at sp+0x38); `register` removal and dropping the dead declarations are inert.
<!-- plateau-handoff:func_overlay_035_F00001E0_1881EC0:end -->
