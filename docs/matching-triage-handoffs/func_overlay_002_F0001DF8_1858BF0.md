<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:start -->
### `func_overlay_002_F0001DF8_1858BF0` plateau handoff

- source: `src/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c`
- score: 0/460 words, promoted
- frame: 0x880
- relocations: 5
- first mismatch: none
- summary: Matched. One candidate local for both scans and the renumbering loop (fourteen locals, frame 0x880), a single continue in the second scan, closestIndex and the two route locals reused, an early return on a nonzero group, and the renumbering start in the for-init.

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
#### 2026-10-02, lane z-ovl1: 198 to 0, promoted

The split carrier (lastCandidate plus a trailing candidate) was the frame
cell: fifteen locals where the target has fourteen. Replacing the loop
temporary with any existing local gave frame 0x880 at once. What then
decided the registers, read from the allocator records (save is totalsave
over nocs, nocs is 1 + floor((blocks + 2) / 4)):

- index took a1 at 188/12 = 15.67 against the renumbering cursor's
  31/2 = 15.5, where the target has index in t0 and both cursors in a1.
- the unroller's bound took a2 at 13/3 = 4.33 against the indices base at
  51/12 = 4.25, where the target has the base in a2 and the bound in t3.
- forcing those two (index c7, base c5, bound c10; all accepted) left only
  ring phase after the group copy, so the structure was already right.

One `continue` for the three rejecting tests of the second scan adds
blocks to every web spanning that loop: index falls to 188/15 = 12.53 and
the bound to 13/4 = 3.25 below the base at 51/14 = 3.64, and both orders
land with no force. Three separate `continue` statements stop the
unroller (size -172).

The rest, each measured:

- closestIndex reused for the closest object's slot (t3 in both roles),
  closestRoute for `previous->route` in the renumbering loop (a3 in both),
  candidateRoute and header loaded before the two increments.
- `if (route->group != 0) return;` ahead of the chained
  `route->group = input->group = closestRoute->group`: the copy block then
  reloads route after the store, the word the chained form had been
  missing.
- `route->order = input->order = K`, input stored first.
- `closestIndex == count - 1` for the last-slot test (operand order).
- `for (index = count - 1; ...; index--)`: with the start as its own
  statement the copy is emitted before the cursor setup (2 words,
  schedule-only); in the for-init it follows it (checklist item 22).

Broken closure: the earlier section's "a natural single-candidate rewrite
gives candidate a callee-saved register, so the split carrier is
load-bearing" was true of the shape it held fixed (no continue, separate
position and tail locals). With the reuse above the single candidate is
exact.
<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:end -->
