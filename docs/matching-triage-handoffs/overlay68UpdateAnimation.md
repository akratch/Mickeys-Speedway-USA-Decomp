<!-- plateau-handoff:overlay68UpdateAnimation:start -->
### `overlay68UpdateAnimation` plateau handoff

- source: `src/overlays/o068/overlay68UpdateAnimation.c`
- score: 106/356 words
- frame: 0x78
- relocations: 15
- first mismatch: +0xCC
- summary: Opacity store follows the elapsed add: 108 to 106 at size 0. Left: the ring from +0xCC and the neighbour pointers' colours.

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

<!-- plateau-handoff:overlay68UpdateAnimation:end -->
