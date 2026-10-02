<!-- plateau-handoff:overlay25UpdateEffect:start -->
### `overlay25UpdateEffect` plateau handoff

- source: `src/overlays/o025/overlay_025.c`
- score: 0/259 words, promoted
- frame: 0xA0
- relocations: 25
- first mismatch: none
- summary: Matched. Listing rewrite; dead `y` carries the height difference; early continue on the disabled test; hit result held in dead `steps` across hitSomething = 1.

Summary before this remeasure: 16/259 words at size delta 0, frame 0xA0, first mismatch +0x2A0.

Summary before this remeasure: Listing rewrite 74 to 19 at delta 0. Open: hit-loop index (tot 32) outranks its cursor (tot 31) for s3; delta f0 for f2; one as1 slot.

Summary before this remeasure: hypothesis=source-authentic save-ratio so web 122 follows web 129, or 12 bytes of else-arm home; spellings=post-loop use 106 at +24, second post-loop use 103 at +8, hit struct kept at 74 and delta 0; stall=the save-ratio uses spilled and s4 is unproved, while objects homes at +0x4C

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

Earlier next, superseded by the 2026-09-19 hybrid and the 2026-09-24 attempt below: a source-authentic way for web 122 to take s4 while keeping `other`
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

Tried 2026-09-24, recorded in the section below, not an open assignment:
a source-authentic save-ratio so web 122 is coloured after web 129
(tot 32 vs 31, one non-loop occurrence would invert it if nocs also
rises) without the L144 address-taken cascade, and 12 bytes of else-arm
home in the 0x34-0x58 pad without growing the frame. The two save-ratio
spellings spilled (106 at +24, 103 at +8) and were reverted. The 12-byte
else-arm struct was kept: 74 masked words, delta 0, objects at +0x4C.
Packed force 63 is not a match and was not re-run.

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
#### 2026-10-02, lane x-ovla: rewrite pass, 74 to 19 at delta 0

The inherited body was the m2c shape (a `remaining--` do-while with an
explicit guard, a 12-byte hit struct, a separate `current` carrier). Rewritten
from the listing and measured as products (shape_product, masked words):

- Natural body: `steps = updateRate - 1; while (steps--)` over the state
  fields with no carriers, the hit loop as `while (count--)` reading
  `objects[count]`, every local at function scope: 78 at delta 0, first
  mismatch moved from +0x3C to +0xCC (the movement arm is exact).
  Reusing the parameter (`updateRate--; while (updateRate--)`) is 232 at +8.
- `radius = 4` (or `4.0`) instead of `4.0f`, with radius declared before
  position: 27. With `4.0f` the radius shares one constant web with the else
  arm's two `4.0f` and is coloured f2; the target gives it a ring temporary,
  and every float temporary after it was one ring step out.
- Lifetime decremented before duration in the else arm: 21 (the t6/t8 draw
  order at +0x190).
- objects[6] declared after count, other, delta and otherState: 19, every
  frame slot equal (frame_census). Each declared scalar takes a 4-byte cell
  here, so the target's 16 bytes between position and the array are those
  four locals. Larger arrays or unused pads grow the frame to 0xA8.
- Hit-loop spellings (`delta` read through `objects[count]`, pointer
  arithmetic, `while (count != 0) { count--;`, `objects[--count]`,
  `count-- > 0`): 19 or a size change.

Remaining 19 (aligned 240 exact, 17 naming, 2 structural): the hit loop's
index and the cursor uopt strength-reduces from `objects[count]` take s3/s4
the wrong way round. Decision records (proc 1, 31 p1 decisions): index web
totalsave 32, nocs 5, save 6.4 against the cursor's 31, nocs 5, save 6.2;
the target needs the cursor ranked first. `delta` is coloured f0 (c24) where
the target has f2 (c25), and as1 orders `hitSomething = 1` against the two
argument moves differently around the `enabled` test.
#### 2026-10-02, lane x-ovla: the 19 priced by force

Forcing the hit-loop index (web 115) to s4 (c18) and its cursor (web 123) to
s3 (c17), both accepted (`forced=18`/`forced=17`), scores 6 at delta 0: the
priority swap is 13 of the 19 words. The other 6 are `delta` in f0 for f2
(three words) and the `enabled` test's slot (three words: the target hoists
both argument moves and puts `hitSomething = 1` in the delay slot). Flat at
19: the index as a separate `i` (`i = count; while (i--)`), the explicit
guarded do-while (21), `for` forms (size changes), sharing one counter with
the movement loop (62), `count-- != 0` (256, -8), and putting
`hitSomething = 1` on the call's line or the `&&` chain on one line.
#### 2026-10-02, lane x-ovla (resumed): 19 to 16, the ranking reached

- Lever (a), a dead local carrying a value: the hit loop's height difference
  written into `y` (the movement accumulator, dead in this arm) instead of
  `delta`: 16. `y` is one web across both arms and takes its f2 there, which
  is the target's delta register. `x` 24, `z` 27, `radius` 99 (+12).
  The declared `delta` keeps its cell; removing it would move objects.
- Index/cursor ranking. Records (proc 1): index web 115, type 3,
  totalsave 32, blocks 27..42 and 49 (17), nocs 5, save 6.4; cursor web 123,
  type 4, totalsave 31, blocks 28..42 and 49 (16), nocs 5, save 6.2. The index
  has exactly one block the cursor lacks (the guard). nocs is
  1 + floor((blocks + 2) / 4), so the swap needs the index at 18 blocks with
  the cursor at 17 (or any b = 2 mod 4 with the cursor one less).
  Every zero-instruction region tried adds blocks in pairs to both ranges:
  do { } while (0) around each of five loop statement groups (32 cells, 19 or
  55, at five wraps both reach nocs 8), around the query call, the
  `hitSomething = 0` store, the whole loop or the post-loop test (19 to 59;
  the index range still starts at the guard, so blocks before it never
  count), `if (1)` (same as do-while), and unreferenced labels (inert).
  An odd block count inside the loop is the open lever.
- Totalsave from source, all flat or worse: dead reads of `objects[count]`
  at the loop top, after the load, at the body end, `(void)` and compare
  forms (19, the reads are deleted before counting); an explicit cursor
  in the unused pad cell (41 to 76); the index as `updateRate` (+4) or
  `steps` (59); `count` declared in the else arm or the inner arm (18, the
  array home moves, no swap).
- The as1 slot (3 words: the target hoists both argument moves above the
  `enabled` test and puts `hitSomething = 1` in the delay slot) did not move
  with any of these.
#### 2026-10-02, lane x-ovla (resumed): 16 to 0, matched and promoted

Two edits closed the last 16 words; both were measured with shape_product.

- One extra block inside the loop is worth 13 words, as the records
  predicted. With `goto g; g:` placed before the call, before
  `otherState =`, or after `selfHitCount++`, the score is 3 (42 at the loop
  top). The natural form is an early `continue`:
  `if (otherState->enabled == 0) { continue; }` split out of the `&&` chain
  gives 3, and so does `if (!overlay25CanHitReloc(other, otherState))
  { continue; }`. A continue on `other == owner` alone is 16, and all four
  tests as one continue is 22.
- The last 3 were the as1 order around the `enabled` test. In ugen's output
  `li t8,1; sw` comes before the two argument moves, and no line or
  comma-expression placement changed that. Holding the hit check's result in
  `steps` (dead in this arm) across the store gives 0:
  `steps = overlay25CanHitReloc(other, otherState); hitSomething = 1;
  if (steps) {`. The ugen order becomes moves, jal, store, which is the
  shipped delay-slot order.

Promotion: the TU now has no GLOBAL_ASM. Its compiled .rodata is the
retained overlay rodata at +0x20 byte for byte: the updater's six literals,
then overlay25SetVectorFlags' 0.707f, which is now a literal in place of the
extern gOverlay25Threshold (valued at that rodata offset). POSTPROCESS asserts
that pool by digest and externalizes it onto a zero base, and the atlas
ownership row is marked externalized. The callee names are the
`overlay25*Reloc` surface. gmake verify OK (also after a fresh object
rebuild), check-overlay-syms up to date, promotion-proof PASS 25/25
relocations. POSTPROCESS audit class: metadata.
<!-- plateau-handoff:overlay25UpdateEffect:end -->
