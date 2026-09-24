<!-- plateau-handoff:overlay25UpdateEffect:start -->
### `overlay25UpdateEffect` plateau handoff

- source: `src/overlays/o025/overlay_025.c`
- score: 74/259 words
- frame: 0xA0
- relocations: 25
- first mismatch: +0x3C
- summary: hypothesis=source-authentic save-ratio so web 122 follows web 129, or 12 bytes of else-arm home; spellings=post-loop use 106 at +24, second post-loop use 103 at +8, hit struct kept at 74 and delta 0; stall=the save-ratio uses spilled and s4 is unproved, while objects homes at +0x4C

Summary before this remeasure: Hybrid L99 homes 82 to 76. Objects 0x58 vs 0x4C. Colour floor 63 via p1:w122=c18; L100 leftovers do not rank other onto s4 unforced.

#### 2026-09-17, lane w10-o025: frame closed at 0xA0; colour floor is 69

Arrival was 1,036 B / 259 words, frame 0xC8 vs 0xA0, 127 masked, first +0x0.
`align_symbol.py` read size delta 0, 138 exact / 95 naming / 8 immediate / 20
structural, candidate-only +0x54 +0x26C, target-only +0x48 +0x27C.
`frame_census.py`: 15 slots each; hitSomething traffic at candidate 0x88 vs
target 0x98; objects array 2& at target 0x4C vs 1& at candidate 0x8C.

Levers that moved the residual, all at size 1,036 / 259 words:

- Deleting decompiler register temps (`velocityX`/`velocityZ`, `extraSteps`,
  walking `cursor`, `queryRadius`, `ownerState`) and writing `2.0f *` for the
  doubled transform value: 127 to 86, frame 0xC8 to 0xB0.
- L112: `objects[6]` solves the target frame 0xA0 from that 0xB0 body
  (`0xB0 - 0xA0 = 16 = 4 pointers`). Score 84.
- L99: one unused pointer declared first homes `hitSomething` at 0x98 without
  growing the frame (it occupies the 4-byte top pad). Score 82. A second unused
  pointer grows the frame to 0xA8; five more grow it to 0xB8.

Aligned at the 82-word shape: 177 exact / 67 naming / 8 immediate / 7
structural, displacement tax 0, no extra/missing words. Candidate homes:
+0x98 hitSomething (matches), +0x94 radius, +0x88..+0x90 position, +0x70
objects[6]. Target: +0x98 hitSomething, +0x80 radius, +0x74..+0x7C position,
+0x4C objects (40-byte gap).

Identity gate: instrumented IDO scores 82 at delta 0 (same as stock).
Procedure 1 of 3, 31 p1 decisions, 25 coloured webs. Exhaustive
`--every-colour` landscape: 154 probes, out directory untracked. One force
beats 82 at delta 0: `p1:w122=c18` (s3 to s4, same 12.5 callee cost) scores
69. L159 packing is that singleton, predicted 69. Web 132 is inert at 82
across c25-c29.

Eliminated on this body:

- `objects[10]` at the 82-shape declaration list: frame 0xB0, 86 masked
- inlining `other` as `objects[index]` (L160 on web 122): size delta -4, 235
  masked, loses the s0 save
- reusing `hitSomething` as the movement countdown: size delta +4, 258 masked
- inlining `otherState` through `other->state->entity`: size delta -4, 246
  masked
- inlining `delta` as two `other->y - object->y` compares: size delta +4, 145
  masked

Next: a source-authentic way for web 122 to take s4 while keeping `other`
(the 69-word force), and/or 0x24 of unused-as-memory homes between position
and objects so the array starts at 0x4C without growing the 0xA0 frame.
Colour will not move those stack offsets.

#### 2026-09-19, lane w27-o025: hybrid L99 homes 82 to 76; L100 does not unforce s4

Re-measured the 82-shape: 1,036 B, size delta 0, masked 82, frame 0xA0,
25 relocations, first +0x3C. Aligned 177 exact / 67 naming / 8 immediate /
7 structural. Identity gate PASS (stock and instrumented `.text` identical;
`CDX_OUT` not `CDX_LOG`; proc 1 of 3, 31 p1 decisions). Unforced records
read `forced=-2`. Force `p1:w122=c18` accepted (`forced=-1` / colour 18)
scores 69 at delta 0, blast 0x280-6 0x300-2 0x380-5. Web 122 (`other`,
type-3 pointer, save 6.4 nocs 5 tot 32) takes s3; sibling web 129 (type-4
pointer, same blocks, save 6.2 nocs 5 tot 31) takes s4. Same 12.5 callee
cost; s3 is free when 122 is coloured so the force is a ranking swap.

L100 leftovers do not rank 122 onto s4 unforced:

- `updateRate` OR-with-zero as a comma in the while or as a statement after
  the inner decls does not move web 2's save (stays 2.615385 / tot 34). Size
  +12, frame 0xA8, 260 masked. L109 verification failed on this param.
- Last-declared `zero` OR-with-zero (overlay31 form, statement after inner
  decls) creates web 121 at save 4.2 tot 21, coloured s5 after 122/129. Frame
  0xB0, 260 masked. Dropping the `zero = 0` init, or OR-assigning that zero
  into hitSomething after the loop, still leaves 122 on s3.
- Extra `objects[index]` in a comma is CSE'd (82 identical). `objects[0] =
  objects[0]` after the query is 76 identical on the hybrid (web numbers
  shift, s3/s4 pairing unchanged).
- Naming `state->owner` grows the frame to 0xB0, size +4, 253 masked, and
  cascades `other` onto s2.
- Address-taken OR-with-zero on `other` or `otherState` is L144, not L100:
  size delta 0, a new gap home at 0x4C or 0x44, and a callee-save cascade
  that moves 122 onto s2 (wrong direction), 245-253 masked.

L99 unused-as-memory at function scope grows the frame (one extra f32:
0xA8 / 86; nine pads: 0xC8 / 86) and does not eat the 0x38-0x70 pad.
The 0x14 gap between hitSomething and radius is not a pile of unused
decls: it appears when radius/position live in the activeDuration arm
and hitSomething stays function-scope.

Hybrid packing (this body): function-scope unused pointer plus
hitSomething (keeps 0x98); radius and position in the activeDuration
arm (land at 0x80 / 0x74); `objects[6]` in the else arm (lands at 0x58).
Official remeasure: 76 masked, size delta 0, frame 0xA0, 25 relocations,
first +0x3C. Aligned 183 exact / 68 naming / 2 immediate / 6 structural.
`frame_census.py`: only remaining home mismatch is objects 0x58 vs 0x4C.
Force `p1:w122=c18` on this body scores 63 at delta 0 (same +13).

Else-arm extensions that would move objects to 0x4C all grow the frame:
`objects[7]`/`[8]` 0xA8 / 86; `[9]`/`[10]`, three unused pointers or
floats before or after `objects[6]` 0xB0 / 86. Dropping the function-scope
unused pointer moves hitSomething to 0x9C (84 masked). Slack below 0x58
down to the save area at 0x34 is 8 bytes; 12 are required. IDO will not
spend that pad on more else-arm memory without raising 0xA0.

Next: a source-authentic save-ratio so web 122 is coloured after web 129
(tot 32 vs 31, one non-loop occurrence would invert it if nocs also
rises) without the L144 address-taken cascade, and/or 12 bytes of else-arm
home that occupy the 0x34-0x58 pad without growing the frame. Packed
force 63 is not a match.

#### 2026-09-24, lane p2b-o025: else-arm 12-byte home, 76 to 74

Baseline remeasured with CDX and DKWB unset: 1036 bytes, masked 76, size
delta 0, frame 0xA0, 25 candidate relocations, first mismatch +0x3C.
Aligned 183 exact, 68 naming, 2 immediate, 6 structural. Objects at
+0x58 versus target +0x4C.

Spellings, at most three:

1. Hoisted `other` to the else scope and, after the loop, stored
   `state->lifetime = (other != NULL) ? 0 : 0`. Masked 106, size delta
   +24, frame 0xA0, new slot +0x54. Not deleted. Reverted.
2. Same hoist, lifetime store `(s16)(((s32) other & -1) - (s32) other)`.
   Masked 103, size delta +8, frame 0xA0, same +0x54 spill. Reverted.
   A post-loop use that survives keeps `other` live and spills it, so it
   cannot invert the save ratio at delta 0.
3. One 12-byte else-arm struct (`other`, `delta`, `otherState`) declared
   before `objects[6]`, assignments and calls unchanged. Kept. Masked 74,
   size delta 0, frame 0xA0, 25 candidate relocations (target object has
   13), first mismatch +0x3C. `frame_census.py` ladders match, including
   objects at +0x4C. Aligned 185 exact, 68 naming, 0 immediate, 6
   structural. The immediate bucket closed. Naming did not move.

Stall: web 122 on s4 is unproved. The kept edit does not colour it; the
save-ratio spellings spill before they can. Packed force 63 was not
re-run and is not a match.
<!-- plateau-handoff:overlay25UpdateEffect:end -->
