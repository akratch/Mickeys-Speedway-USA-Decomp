<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:start -->
### `func_overlay_002_F0001DF8_1858BF0` plateau handoff

- source: `src/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c`
- score: 353/460 words
- frame: 0x880
- relocations: 5
- first mismatch: +0x20
- summary: extra-ILOD pair at the previousDistance call line. Delta 0 via lastCandidate carrier. Stall: order share -12, register inert, header hoist flat, post-loop +16.

The open extra-ILOD pair was the tail. Its first target-only word was the stack store on the previousDistance call, because closest was spilled across the second scan and not saved again at the calls. Carrying the scanned object in lastCandidate (lastCandidate = objects[index], then candidate = lastCandidate) closes that gap: size delta 0, 353 masked, exact 174, frame 0x880, first mismatch +0x20.

Kept from the earlier -4 shape, still true there: (u32) on func_8000BCB0 is required, a declared cursor grows the frame to 0x888, and OR-zero, empty if, and a comma copy of lastCandidate do not emit the missing word.

Stall after the size closed. These spellings did not improve the residual:

- sharing the two order+1 stores removes the tail recompute and moves size to -12
- register on closest is byte-inert
- loading closest->header before joyGetButtons does not improve the residual
- a post-loop lastCandidate = candidate assignment moves size to +16 and 457 masked

The remaining 353 words are mostly register naming. No colour sweep was run; the lane was stock-only.
<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:end -->
