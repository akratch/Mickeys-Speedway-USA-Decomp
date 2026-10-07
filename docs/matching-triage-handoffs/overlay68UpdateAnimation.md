<!-- plateau-handoff:overlay68UpdateAnimation:start -->
### `overlay68UpdateAnimation` plateau handoff

- source: `src/overlays/o068/overlay68UpdateAnimation.c`
- score: 2/356 words
- frame: 0x78
- relocations: 15
- first mismatch: +0x244
- summary: atStart assigned in the first call, red result through index: 6 to 2. Left: home store emitted at the definition, after the argument store.

Summary before this remeasure: Plain-if neighbours, copy order afterAfter/after/before, unmasked opacity before elapsed: 106 to 6. Left: atStart spills to a temp, not its home.

Summary before this remeasure: Opacity store follows the elapsed add: 108 to 106 at size 0. Left: the ring from +0xCC and the neighbour pointers' colours.

#### 2026-10-05: narrowing the elapsed add does not rotate the ring

`(s16)(state->elapsed + updateRate)`, `elapsed += (s16)updateRate`, and `(s16)((s32)elapsed + updateRate)` are each 108 masked words at 1,424 bytes, delta 0. The body is unchanged. Do not repeat those three casts.

Summary before this remeasure: Declaration-order hill climb 183 to 180; state web t2 against t1 (t-pool shift) remains.

Summary before this remeasure: Opacity no-op mask (ring phase) and declaration order: 213 to 183. State web t2 against t1 is a global t-pool shift.

Summary before this remeasure: Pointer walk matches duration-loop shape at delta 0, 213/356, first +0x1C; state stays t1 not t2. Subscript walk grew 16 bytes, reverted. No colour sweep.

Summary before this remeasure: L160 named keyframes base puts object in t2 unforced at size 0 frame 0x78. Remaining blocker is duration-loop structure.

Pointer walk of the same trip count. The cursor is the addressed keyframe.
Each trip subtracts its duration, stores the advanced index, and steps the
cursor. The break arm clamps the index, clears elapsed and active, and
reloads the cursor from the animation. The fraction test is the cursor
duration's truth value, so the exit uses the duration already loaded.

A walk that still subscripted the samples afterward grew 16 bytes (the
scaled index was recomputed after the loop) and was reverted. The kept
spelling reads the samples through the cursor. It is size 0, frame 0x78,
15 relocations, 213/356 positional, first mismatch +0x1C. The loop shape
matches. State remains in t1, not t2, and the positional residual is that
naming shift. Counted-for and pre-decrement forms were not applied: outside
this TU they emit a shift-add index, not the multiply the loop needs. No
colour sweep.

Prior L160 named-base form put state in t2 at 214/356, first +0x34. The
break-arm reload and keyframeIndex store remain load-bearing for size 0.
Flag lattice and permuter stay closed.

### 2026-10-01, lane d-ovl2: opacity mask and declaration order, 213 to 183

- `state->opacity = animationOpacity & 0xFF`: a no-op the peephole deletes
  that spends one ring temp (L127). 213 to 190 at size delta 0, the single
  largest cell of a 24-cell product over four no-op sites (the active flag,
  the duration reads, the opacity store). The other three sites were worse
  or flat, and 19 single & 0xFF wraps on the colour and duration reads were
  all worse at the adopted base.
- Declaration order, best of 300 random permutations: 190 to 183 (the next
  best orders read 184 and 185). Adopted: direction, animationOpacity, angle,
  opacity, index, current, tangentZ, atStart, animation, before, after,
  afterAfter, tangentX, state.
- Plain `if (index < count - 2) afterAfter = current + 2` with
  `before->red << 8` directly: 319 and 4 bytes short; the ternary through
  `angle` is load-bearing (it reproduces the target's early red load).
- Remaining: the state web is t2 in the target and t1 here, current t1 against
  t0, index t0 against v1; a global one-position shift of the t-pool colours
  (93 percent coherent mapping, 162 naming rows).
#### 2026-10-02, lane g-ovl5: 183 to 180

- Declaration-order hill climb over 14 declarations, 183 to 180.
- Keyframe neighbour selection as separate ifs, a ternary chain, or with the
  after-after default taken from `after`: 308 to 309 at -8; the comma form kept.
- Spending an extra t-pool draw before the state load (an OR-zero on
  updateRate, a ternary, if/else, an `&= 0xFFFF`, a state null test): all
  inert or worse; the one-position shift is not reached from the head.
#### 2026-10-02, lane x-sort: 180 to 108

The closure "state t2 against t1, a global t-pool shift" was a claim about the
shape. Records on the inherited body: current (save 6.57) outranks state
(6.09) and takes t0, so state lands in t1, while the loop's keyframe-index
web (save 2.75) is a separate, late web. Measured cells (masked, delta 0):

- The loop stores state->keyframeIndex = index + 1 and re-reads it into
  `index`, so one web carries the index through the loop and the neighbour
  selection: state t2, current t1, index t0, animation a2 and the stride
  constant a3 all match. 143 (frame then 0x70).
- Declarations in the target's home order (atStart 0x6C, tangentX 0x60,
  tangentZ 0x5C, current 0x58, before 0x54, after 0x4C, afterAfter 0x48,
  state 0x40) plus two unused cells at the bottom: frame 0x78, 110.
- The exit test written as (index = state->keyframeIndex) >= count, so the
  count load precedes the reload: 108.

Aligned buckets 178/150/4/25 -> 252/85/3/17 (exact, naming, immediate,
different). Left: from +0xD4 the ring temporaries run one draw apart (the
updateRate reload takes t6 where the target takes t9), and the neighbour
pointers' colours differ (target atStart v0, afterAfter v1, after t3, before
t4). The ternary through `angle` is still load-bearing: plain ifs lose the
branch-likely copy (-4) at this shape too. Next: draw census over the first
0xD4 bytes against the target's registers, then the neighbour webs' records.

#### 2026-10-03, lane codex-o068-phase: the opacity mask is not free to delete

Removing only the redundant byte mask drops the early draw count and makes a long prefix exact, then scores 124 masked words at size delta 0 because the neighbour allocation moves. Separating the red sample scores 151 at frame 0x80. Moving the start predicate scores 272 at size delta +4. Sharing that predicate with the preceding-neighbour branch scores 208 at size delta 0. None is kept. Do not delete the mask to chase the early ring.

## 2026-10-06: the opacity store follows the elapsed add

Configured baseline: 1424 bytes, 108 masked and 108 raw, size delta 0, first mismatch +0xD4.

The opacity store and the elapsed add are swapped. The mask stays on the store. Nothing between the two statements reads opacity.

Result: 106 masked and 106 raw words, size delta 0, first mismatch +0xCC. The body is kept. It is not a match. The three elapsed-add casts and the mask deletion stay closed.

## 2026-10-06: folding the opacity store into the elapsed add

The 106-word body loads elapsed before it stores opacity. The target stores the byte first.

Spelling: assign elapsed from a comma whose left side stores the masked opacity and whose right side is the elapsed sum. No new local and no new block.

Result: 108 masked and 108 raw words, size delta 0, first mismatch +0xD4. The store moves back ahead of the load and the two-word gain disappears. Not kept. The 106-word body stays.

## 2026-10-06: computing atStart before the neighbour copies

The 106-word body scores 1424 bytes, 106 masked words, size delta 0, first mismatch +0xCC. The target computes index < 1 into v0 before the keyframe-count tests. This pass moves that assignment ahead of the neighbour pointer copies and leaves the index > 0 test in place.

Result: 268 masked and 268 raw words, size delta +4. Not kept. The 106-word body stays. Do not repeat this early atStart.

#### 2026-10-06: one source line for the elapsed add and the opacity store is inert

The unmodified body scores 1424 bytes, 106 raw and 106 masked words, size delta 0, first mismatch +0xCC. The ROM stores the opacity byte and then loads elapsed. This body loads elapsed and then stores the byte.

Joining those two statements onto one source line scores the same 106 masked and 106 raw words at size delta 0. The mismatch list is unchanged. Not kept. The 106-word body stays. Do not repeat this joined line. The comma store and the early atStart stay closed.

#### 2026-10-06: atStart before the positive-index test is inert

The unmodified body scores 1424 bytes, 106 raw and 106 masked words, size delta 0, first mismatch +0xCC. The ROM stores the opacity byte and then loads elapsed. This body loads elapsed and then stores the byte.

Computing atStart after the neighbour copies and before the index-greater-than-zero test scores the same 106 masked and 106 raw words at size delta 0. The mismatch list is unchanged. Not kept. The 106-word body stays. Do not repeat this placement. The early atStart and the joined line stay closed.

#### 2026-10-06: an aliasing opacity store does not move the elapsed reload

The unmodified body scores 1424 bytes, 106 raw and 106 masked words, size delta 0, first mismatch +0xCC. The ROM stores the opacity byte and then loads elapsed. This body loads elapsed and then stores the byte.

Storing the masked opacity through a byte pointer at that field scores the same 106 masked and 106 raw words at size delta 0. The mismatch list is unchanged. The store is folded. Not kept.

Reading that byte back into the opacity local, on the direct field store, also scores 106 masked and 106 raw words at size delta 0. The mismatch list changes and the aligned split does not: 81 naming, 17 structural, 3 immediate. Not kept. The 106-word body stays. Do not repeat the byte pointer or this read-back. The joined line stays closed.

## 2026-10-07, lane a-ovl2: 106 to 6, the neighbour selection rewritten

The 2026-10-01 and 2026-10-02 closures said plain ifs lose the branch-likely copy and that the opacity mask is a load-bearing ring draw. Both were measured one at a time on the ternary shape; together they are the residual.

- Plain `if (index < count - 2) afterAfter = current + 2;` with `before->red << 8` read in the first call's argument list: 132 masked at size 0. The branch-likely copy of the red load comes back once the red read is the first instruction of the join block (a separate `angle = before->red` local, or `opacity` reused, is -4).
- On that shape, all eight orders of the three neighbour copies: afterAfter, after, before puts before in t4 and afterAfter in v1 as shipped.
- Opacity store placement (four positions) times the mask: no mask, before the elapsed add, 6 masked at size 0. With the mask any placement is 106 or worse.

Aligned now: 350 byte-exact, 0 naming, 6 immediate, 0 different. The six words are atStart's call-spanning spill: the target stores the v0 web to sp+0x6C (the third declared home) and reloads it for calls two to six; this body stores the same web to sp+0x34, a compiler temporary below the declared homes, so uopt is spilling an expression temporary for `index < 1` rather than the variable. Measured inert at 6: `index <= 0`, `!(index > 0)`, a conditional, `index == 0` (7), `state->keyframeIndex < 1`, `register`, `*&atStart`, u32. Reusing `opacity`, `direction` or `animationOpacity` as the carrier (each declared third) also spills to sp+0x34. `volatile` and `*(s32 *)&atStart` make every call reload the home but cost 4 bytes (309 masked). An s16 or u8 atStart, or an s16/u8 parameter, is +8. Reading `state->keyframeIndex` at every neighbour use instead of `index` is -4.

## 2026-10-07, lane a-ovl2 (second budget): atStart as a symbol web, measured

Instrumented records on the 6-word body: atStart's value is web 113, type 4 (an expression temporary), so its spill goes to the temporary area at sp+0x34; the declared atStart (home -0xC, sp+0x6C) has no web at all. One product over the forms that should make it a type-3 symbol web, checking `webdetail` per cell:

- Narrower types (s16, u8, s8, u16 declared): type-3 atStart, but the frame ladder shifts and the function grows 8 bytes (226 positional).
- if/else assignment (a phi): type-3, home sp+0x6C, but +4 bytes and branchy (315).
- `atStart = state->keyframeIndex < 1` (memory operand, a store between definition and use): type-3 web 111, spilled to sp+0x6C as shipped, size 0, 107 positional. It costs index one reference (totalsave 37 to 36, nocs 4 to 3), index's save rises to 12.0, ties web 80 and wins on web number, and the integer colours cascade.
- Reassigning index between the calls (`index = 0;` or `index = object->red;` after the red call): type-3 at sp+0x6C, but -4 bytes.
- Field reads at every neighbour use: -4 bytes.
- On the memory form, a `do { } while (0)` around the neighbour copies, the atStart assignment or the index load restores index nocs 4 (save 9.0, ranked after web 80): 96 positional. index then takes a1 because nothing denies it a1-a3; in the 6-word body it was denied a0-a3 because the propagated `index < 1` extended its range into the call block. The remaining atStart piece is denied v0-t3 (web 61, the CSE'd keyframeIndex load, holds v0) and takes t4, which removes t5 from the ring. Forcing index to t0 restores every colour except that piece; forcing web 111 to v0 colours the whole range and drops the split (-12 bytes).

Law: a value is a symbol web (and spills to its home) only when uopt cannot rebuild it, but every form that achieves that here moves the neighbour index's block count or reference count by one.

## 2026-10-07, lane a-ovl2 (third budget): the keyframeIndex re-read does not merge

On the memory form plus the copy wrapper (96 positional): reading `atStart = state->keyframeIndex < 1` before or after `index = state->keyframeIndex` leaves the second load as its own type-4 web (61, block 25, v0); uopt does not share the two loads. Computing atStart from a second local copied from the load (`opacity = state->keyframeIndex; index = opacity; atStart = opacity < 1`) is copy-propagated back to type 4 (-4 bytes). The last atStart piece is not denied v0 by web 61 alone: afterAfter's last piece (web 102, save 0.667, nocs 3) is decided before atStart's (save 0.333) and takes v0, where the target has afterAfter v1 and atStart v0. A force cannot address only the last piece (the pieces share the web number; forcing it colours the whole range and drops the split, -12 bytes).

Moving the memory-form assignment after the `count - 2` test gives a one-block range: atStart takes v1, 92 positional, size 0. What remains is the integer cascade from index: off the call block, index is no longer denied a0-a3 and takes a1 (target t0); forcing index to t0 restores those colours but some web then takes t5 and the ring shifts (162).

## 2026-10-07, lane c-near: 6 to 2, atStart assigned in the first call

The dispatch note said the target computes atStart and keeps index live into the first call block. Both facts are reached by one shape: the first call's last argument is `atStart = index < 1` (no earlier atStart statement), and the red result is carried through `index` (`index = func(...); object->red = index;`). The argument use keeps index's range in the call block, so index is denied a0-a3 and keeps t0; the redefinition kills `index < 1`, so uopt cannot rebuild atStart and it is the symbol web, spilled to its home at sp+0x6C. 1424 bytes, 2 masked, size delta 0, aligned 354 exact, 0 naming, 2 immediate, 0 different.

The two words are one store pair: the target emits the home store with the call's spill group (register order, v0 first) and then the argument store; ours emits the home store at the definition, ahead of the argument store, and as1 reverses both. A home store joins the spill group only when the web is defined in an earlier block than the call block, so the remaining requirement is an early definition plus a use of the old index in the call block.

Measured negatives (masked, size delta):

- do-while and if(1) wrappers around the atStart statement or the whole neighbour block, three spellings of the test, before or after the count tests: all 6 at 0 (54 cells).
- atStart assigned in the call argument without the index redefinition: 6. The red, green or blue result through index without the argument assignment: 313 to 319 at -4.
- atStart as a statement right before the call plus the index redefinition: 20 at 0 (home store scheduled into the branch-likely slot).
- index declared s16, u16 or short: 6 (u16 is +8).
- An early atStart with index redefined in the call block by an argument value (index = 1, or one of the four shifted samples, passed as that argument): 240 to 318 at -4. A definition does not extend the old range; it has to be a use.
- The assignment inside an earlier argument as a comma expression: 75 to 182, prologue moves.
- Line layout of the in-argument assignment (own line, one-line call, parentheses, index <= 0): all 2; index == 0 is 3.

<!-- plateau-handoff:overlay68UpdateAnimation:end -->
