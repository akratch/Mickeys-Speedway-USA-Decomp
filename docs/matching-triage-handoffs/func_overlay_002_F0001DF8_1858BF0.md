<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:start -->
### `func_overlay_002_F0001DF8_1858BF0` plateau handoff

- source: `src/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c`
- score: 198/460 words
- frame: 0x888
- relocations: 5
- first mismatch: +0x0
- summary: Home-order locals, no-closest return, chained order stores, 1U tail start: 353 to 198 at delta 0; frame 0x888 vs 0x880 (one temp cell).

Summary before this remeasure: extra-ILOD pair at the previousDistance call line. Delta 0 via lastCandidate carrier. Stall: order share -12, register inert, header hoist flat, post-loop +16.

The open extra-ILOD pair was the tail. Its first target-only word was the stack store on the previousDistance call, because closest was spilled across the second scan and not saved again at the calls. Carrying the scanned object in lastCandidate (lastCandidate = objects[index], then candidate = lastCandidate) closes that gap: size delta 0, 353 masked, exact 174, frame 0x880, first mismatch +0x20.

Kept from the earlier -4 shape, still true there: (u32) on func_8000BCB0 is required, a declared cursor grows the frame to 0x888, and OR-zero, empty if, and a comma copy of lastCandidate do not emit the missing word.

Stall after the size closed. These spellings did not improve the residual:

- sharing the two order+1 stores removes the tail recompute and moves size to -12
- register on closest is byte-inert
- loading closest->header before joyGetButtons does not improve the residual
- a post-loop lastCandidate = candidate assignment moves size to +16 and 457 masked

The remaining 353 words are mostly register naming. No colour sweep was run; the lane was stock-only.
#### 2026-10-02, lane x-o051: 353 to 198 at delta 0

Products over the inherited lastCandidate shape (fast_score/shape_product):

- Declarations in the target's home order (lastCandidate 0x6C, closest
  0x68, previous 0x64, next, closestRoute, candidateRoute, route 0x54,
  header, closestIndex 0x4C, position, bestDistance 0x44, distance,
  previousDistance 0x3C, count 0x38, then the loop-local candidate):
  353 to 345.
- An explicit `return` after `input->group = route->group = 1` (the target
  returns straight from that arm): first mismatch moves past the
  prologue, but the frame grows to 0x888.
- L151 decides the joypad block: with `position = count - 1` uopt hoists
  `count - 1` above the position-0 test and shares it with the tail; the
  target folds it into the address (`-2` off `&indices[count]`) and
  recomputes it at the tail. `count - 1U` in the tail does exactly that.
  `count - 1U` in the branch, or `position = count; position--`, is inert.
- Chained order stores: `input->order = route->order = 1/2` and
  `route->order = input->order = candidateRoute->order + 1` (input stored
  first at the tail, as shipped).
- A natural single-`candidate` rewrite (no lastCandidate) gives candidate
  a callee-saved register (s0/s1 pair, 433 to 448 at -8 to -24 bytes):
  the target keeps it a memory local cached in v1, so the split carrier is
  load-bearing, not an artefact.

Open, in order:

- Frame 0x888 against 0x880: every home sits 8 high because the bottom
  temp area has one more cell (the loop-1 cursor temp is at 0x30 against
  0x2C). It appears with the explicit return; without it the frame is
  0x880 but the shape is 345+.
- The group copy: the target loads closestRoute->group once and reloads
  route from 0x54 before the second store (a block boundary there);
  `route->group = input->group = closestRoute->group` loads once but drops
  the reload (-4 bytes).
- The position loop loads both route and header before the two increments
  and reloads candidateRoute->order at the bottom; preloading them into
  closestRoute/header locals measured 208 against 198.
<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:end -->
