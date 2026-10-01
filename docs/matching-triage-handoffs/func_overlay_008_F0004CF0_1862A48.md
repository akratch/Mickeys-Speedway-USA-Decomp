<!-- plateau-handoff:func_overlay_008_F0004CF0_1862A48:start -->
### `func_overlay_008_F0004CF0_1862A48` plateau handoff

- source: `src/overlays/o008/overlay_008.c`
- score: 0/270 words, promoted
- frame: -0x90
- relocations: 15
- first mismatch: none
- summary: Matched. The surface normal is read through a pointer taken at entry and passed to the surface query; no volatile qualifier.

#### 2026-10-01, lane d-o008: ROM-exact closure

1 to 0 at size delta 0, promoted; `gmake verify` and `promotion-proof`
(270 words, frame 0x90, 29 of 29 relocations, linked ROM) pass.

- **What the qualifier was standing in for.** Without it uopt forwards the
  first product's sum into the first call's argument, so the difference is
  emitted first and the body is one word short (164 masked, delta -4). The
  sum stays a register web at its own statement only when something between
  it and the call may change one of its operands. A 512-cell product of
  `volatile` over the four normal members and five float locals bottoms at
  the inherited 1; every cell without a qualified z is 31 or worse.
- **Reading the members through a pointer does it.** `normalPtr = &normal`
  at entry, passed to the surface query, and every normal read through it:
  0 masked. The indirect loads are not forwarded past the store of the
  difference to its home, so the sum is evaluated first and z is read once,
  as shipped. Six placements of the pointer assignment (entry, before the
  scene-item query, just before the products; reads through it in the first
  product only, both products, or every use) all score 0; assigning it
  inside the surface-query call costs 8 to 12 bytes. The pointer takes the
  slot the dead `motionTarget` float held, so the frame is unchanged.
- **Measured and not the lever:** a memory-resident operand kill (the
  difference written into axisA) gives the right order at delta 0 but 55
  words, because axisA's home then holds both values; the same through
  axisB stays forwarded (-4); a one-member struct, a one-element array and an
  address-form store for the difference, block splitters (label, empty
  block, `do {} while (0)`, empty `if`) and an inner block around the sum
  are all forwarded or grow the body.
- **Promotion.** The function's seven literals are a third object pool
  (object 0x1C on, retail 0x290 on), rebound by
  `config/normalizations/overlay8P4CF0.rebind.spec` to an absolute anchor
  `gOverlay8P4CF0Constants = 0x274`, with the TU's `.rodata` digest updated
  in `mk/overlays.mk`.

## 2026-10-01 lane a-o008: 39 to 1, and the decision variable

align_symbol now reads 269 byte-exact, 0 naming, 0 immediate, 1 really
different at size delta 0.

What moved it:

- **The 39 were not a free-list phase.** Reading the target's draws in order
  gives z, first product, x, second product, sum, third product, fourth
  product, difference: one FIFO pass over a five-register ring that includes
  the fifth register the sum lands in. The sum is then negated straight into
  the first argument register. Nothing is rotated; the candidate simply held
  a different value in the coloured register.
- **axisA shared a symbol with the second block's motion value.** That made
  it one long web with loop references and it took the high colour. Giving
  the second block the `horizontalA` name leaves axisA a short web that takes
  the low colour, as shipped, and the second block's value then sits where the
  target has it. 39 to 31 together with direct `normal.x` reads and the
  negation written at the call.
- **The qualifier was on the wrong member.** With `volatile` on `normal.x` the
  x reload lands in a fresh ring register and shifts every later draw (28
  naming rows). On `normal.z` the reload reuses the register the first z load
  held and falls exactly in the first multiply-hazard slot. 31 to 1.

The remaining word and what was measured against it:

- The target loads z once and keeps a `nop` in that slot. The qualifier is
  load-bearing: with no qualifier every form tried computes the difference
  first and forwards the sum into the argument register, one instruction
  short. Measured: 5 statement forms crossed with 3 second-block carriers and
  5 accumulation forms, all at delta -4; 9 region placements, which grow by 8
  to 20 bytes; 3 pointer-store forms and 7 single-read casts, of which the
  four that hold delta 0 score 71 or more.
- So the decision variable is **what keeps the sum from being forwarded past
  the difference without making either z read a qualified load**. In the
  target the sum is evaluated at its own statement and only the negation
  waits for the call. A store between the two that may alias `normal` would
  do it; a memory-resident `horizontalB` through `volatile` or an address
  cast did not.
2026-09-09 lane `lane/tu2-o008` maintenance evidence (base `304d363e`, score unchanged at 43
relocation-masked words / 48 raw, 270 of 270 instructions, exact `0x90` frame):

- The residual is now localized. Every integer lane is identical (pool 49/49, temp
  14/14, shared 11/11) and the only stack-home difference in the whole function is
  the 12-byte surface-normal aggregate, which the target homes four bytes below the
  candidate's placement; the neighbouring 12-byte point aggregate and all four
  compiler temp homes agree exactly. The rest is one FP colour exchange carried by
  two webs of sixteen sites each, plus the known literal-pool offset difference.
- Declaration order is eliminated as the lever for that home. Fifteen orders were
  measured: each 4-byte scalar moved individually between the two aggregates, the
  address-taken pair moved together and separately, the aggregate pointer moved
  between them, and both aggregates reordered against each other. Every order
  scored at or above the 43-word baseline (68, 65, 80 basins) and not one moved the
  aggregate's home or the frame. This is a size-class/region effect, not a position
  in the declaration list.
- Widening the surface-normal aggregate from 12 to 16 bytes does place it at the
  target displacement, which confirms the region reading, but it grows the frame by
  eight bytes and the score to 87. Rejected; the target's aggregate is 12 bytes.
- The declared-but-unused `register f32` local reserves no home: removing it and
  relocating it produce the same object. It is decorative and misleading.
- A 50-minute three-worker decomp-permuter run, roughly twelve times the previously
  recorded 241-second batch, produced no improvement over its base. That run also
  measured the scratch as unfaithful for this translation unit: the runner reports
  the digest-guarded post-compile pass is not replicated, and the scratch base score
  is 406 against a measured 43/48-word residual. Permuter scores do not transfer
  here, and a permuter result on this TU bounds the tool, not the function.
- Resume with allocator evidence rather than source search: the open question is why
  the aggregate region is packed four bytes differently, and no source form reached
  through declaration order, storage class or aggregate width has moved it.
2026-09-09 lane `lane/w3-o008`: 43 -> 39 masked words, and every stack home is now
exact.

- The four-byte surface-normal displacement is closed. The frame is a byte-granular
  declaration census: IDO homes each auto at `frame_top - k` in source declaration
  order (function scope first, then inner blocks in source order), an `s16` takes
  two bytes rather than four, and the whole auto region rounds to eight. The target
  carries four more bytes above the aggregate and four fewer below it, so two edits
  compose -- the normal aggregate takes a fourth float, and the blend tail's loop
  counter is spelled from `updateRate` instead of held in its own block local.
  Neither moves the frame alone; the eight-byte rounding hides a single four-byte
  step, which is why the previous lane read "removing a local does not shed the
  frame" as evidence against the census. It is evidence for it.
- Contrary to the previous handoff, the dead `register f32 motionTarget` is NOT
  decorative: it reserves an auto slot, and removing it moves every home below it
  and costs 22 words. The five `register` qualifiers, the doubled `surfaceHeight`
  assignment and the block-local `factor` are the genuinely inert ones, and are
  now removed.
- Widening the aggregate is byte-identical to inserting a four-byte local above it,
  so the two readings of the source cannot be distinguished from the object. The
  widened-struct form is retained because a surface query filling a plane is the
  more plausible original.
- The remaining 39 words are one extra FP pool web, and the in-source plateau
  comment carries the full exhaustion list: carrier relocation, operand order,
  volatile placement over four members, 37 physical line joins, and three
  statement-group permutations, all flat or worse.
#### Carrier-identity and region axes, all flat (2026-09-10, lane/c6-band-a)

Base `bfa8365f`, 39 masked / 44 raw, 270/270 instructions, exact `0x90` frame,
every integer lane identical, every stack home exact. The axis tested here is
the one that closed two other functions this week and is **not** in the
exhaustion list above: which *existing* local carries which value, and where
the block boundaries sit. 34 forms, none better than 39.

- **Carrier identity for the rotation pair.** `axisA` is uniquely correct: the
  eight cross-product forms that move it to `motionTarget`, `blendFactor` or
  `horizontalA` cost 13 words or a whole instruction. `axisB`'s carrier is
  inert (`motionTarget` and `blendFactor` are byte-identical to it), because
  `axisB` is never a coloured web at all -- it stays in `$f0`, the second
  call's return.
- **Carrier identity for the tail block.** Also uniquely `axisA`: `axisB`,
  `horizontalA`, `motionTarget` and `blendFactor` each cost 12 words,
  `surfaceHeight` 6, and `horizontalB` grows the frame. The blend-rate carrier
  is inert across `surfaceHeight`, `horizontalA` and `motionTarget`.
- **Inner-block scope.** Moving `targetA` to function scope costs 24 or 26
  words, moving `factor` there 24; `factor` before `targetA` is byte-identical.
  A fresh inner-block `f32` carrier for `normal.x` loses an instruction (269),
  the same failure every carrier-removal form has.
- **Splitting the negation** (`horizontalA = ...; horizontalA = -horizontalA;`),
  routing it through `factor`, and the `-(a) - (b)` rewrite: the first two are
  byte-identical, the third loses an instruction. uopt folds the split back, so
  the target's separate `add.s $f16` / `neg.s $f12,$f16` pair is not reachable
  by splitting the statement.
- **`volatile` placement, re-measured.** `volatile x` with the carrier is 39.
  Direct reads with `volatile x` are 57 (two loads, same instruction count).
  All four members volatile with direct reads scores **37**, but it is
  structurally further away, not closer: it emits five loads where the target
  emits three and fills both r4300 multiply-hazard `nop` slots the target
  keeps. Recorded so the next lane does not chase it -- a lower masked count is
  not automatically a closer candidate when the schedule moves.
- **Read-back and guard forms.** `surfaceHeight = surfaceHeight;`,
  `axisA = axisA;` before the carrier, `normal.y` through a local, and
  `point.y < surfaceHeight` for `surfaceHeight > point.y` are all
  byte-identical.

**The variable, restated with what is now excluded.** uopt gives the
`horizontalB = normal.x` copy its own FP *pool* colour (`f2`), and the target
leaves that value in ugen's FP ring (`f8`); as a consequence the target's
`axisA` reload takes the first pool colour `f2` where the candidate's takes
`f16`, and the target's `horizontalA` intermediate gets its own colour `f16`
where the candidate coalesces it into `f12`. That is 21 fp-pool sites against
the target's 19. Nothing in the *source* neighbourhood -- 15 declaration
orders, 16 operand orders, 15 volatile placements, 37 line joins, 3 statement
groups, 34 carrier/scope forms here -- changes which of those two webs uopt
colours first. Resume with a calibrated FP colouring receipt that attributes
the two webs to source, or with a matched sibling that reads a stack aggregate
member once and uses it twice from the FP ring.

#### 2026-09-11, lane p7-ovl2: the stated decision variable is refuted; this is
#### the ugen float free list, not globalcolor

Baseline reproduces: 1,080 bytes, 270 of 270 words, delta 0, masked 39, raw 44,
exact 0x90 frame. Aligner buckets, unchanged before and after because nothing
was adopted: 231 byte-exact, 39 register naming, 0 immediate-only, 0 really
different.

**The closure says to resume with a calibrated float colouring receipt. That
instrument has now been brought, and it says no.** This function is the
twenty-first to enter global colouring in its translation unit, so it is
procedure ordinal 20; the traced object scores the same 39 at delta 0 as the
stock configured object, so the trace is faithful. A 30-cell force sweep over
the five float webs that carry the rotation region, crossed with the split
verdict and the five lowest float colours, leaves the object at 39 in
seventeen cells and at 40 through 60 in the rest. Not one cell improves it.
The two webs the closure names are therefore not the decision.

**The corrected census says the same thing from the other end.** The float-bank
fix merged today changes this function's reading completely: all 39 sites are
float, and the dominant fact is a closed two-cycle over the two lowest scratch
registers covering 32 of the 39 sites, at 92 percent coherence in two windows
with the second opening at +0x1F4. Those two registers are ugen's scratch
rotation and are never a globalcolor colour in either direction, so 32 of the
39 words are a **ugen float free-list phase**, not an allocator decision. The
older census could not see the float bank at all, which is why the residual was
attributed to a pool web.

**One stale measurement corrected.** The closure records that removing the
normal-x carrier drops an instruction, 269 words and 159 masked. On the current
exact-home baseline it does not: the carrier-free form is delta 0 at 270
instructions and 57 masked, and three operand orders of it plus a normal-z
carrier all land at 57 to 59 at delta 0. The carrier is still wrong, but it is
wrong by 18 words, not by an instruction.

**Ring probes negative, 63 cells.** Nine probe forms, float self-assignments on
the three normal members, on the surface height and on the point's y, discarded
float reads of two members, an integer mask, and an empty region marker, each
placed at seven points spanning the two trigonometric calls, the two rotation
products, the two angle calls and the first blend call. Every cell is 39 or far
worse. No zero-byte float ring draw was found in that neighbourhood.

**Reopen with the right instrument.** The question is which ugen float free-list
position the shipped code is at when it enters the rotation region, and what
consumes the one position this candidate has already spent. That is a
DKWB-FREELIST float allocation trace read against a reconstruction, or a matched
sibling in the same overlay whose rotation block has the same shape. It is not a
globalcolor receipt, and it is not another spelling of the rotation block: the
carrier, operand-order, volatile-placement, line-join, statement-group,
carrier-identity and scope lattices recorded above, plus the force sweep and the
ring probes here, have all been measured.

#### 2026-09-12, lane p9-mid: the float-probe family does not draw a ring temp at all

Baseline reproduces: 1,080 bytes, 270 of 270 words, delta 0, masked 39, raw 44,
frame 0x90. The frame census now prints two identical eighteen-slot ladders with
no slot either side uses alone, so the home question really is closed. Nothing
was adopted, so the buckets are unchanged: 231 byte-exact, 39 register naming,
0 immediate-only, 0 really different.

**The lever that moved this function's TU-mate does not transfer, and the
reason is measurable.** `func_overlay_008_F0002640_1860398` moved 62 to 37 today
on declaration order plus two statement moves, and
`func_overlay_041_F0001298_18885D0` moved 34 to 14 on a single redundant 16-bit
mask supplying one ugen ring draw. Neither reaches here. The declaration-order
lever cannot: the ladders are already identical. The ring-draw lever cannot
either, and the new fact is **why**: of 24 float probes measured here -- six
forms (multiply by one, add zero, self-assignment, a discarded comparison, a
double negation, and a scale of the point's y) at four points spanning the
point setup, the surface-height assignment, the guarded block's head and the
first trigonometric call -- **eighteen leave the object byte-identical**,
meaning they do not draw a float ring temp at all; they are deleted before ugen
sees them. The other six change the size. So the integer trick has no float
analogue in this neighbourhood: an or-with-zero on an integer survives to ugen
as a temp request, and none of the float no-ops does.

**Also measured and flat here:** all five orders of the three point-accumulate
statements (the two that respell the accumulate as an explicit add cost 4
bytes; zyx is 45, xzy 63, yxz 73), both orders of the point initialisers,
hoisting the surface-height assignment past the loop, flipping either bounds
comparison or both, flipping the surface-height comparison, splitting the CSE
of the negated angle across the two trigonometric calls, moving targetB into
the guarded block or after the factor declaration (both 65), and swapping the
two dead float declarations.

**The question is unchanged and now sharper.** The residual is a ugen float
free-list phase, and no source form yet found draws a float ring temp at zero
byte cost. The next instrument is a DKWB-FREELIST float trace read against a
reconstruction -- specifically to find which construct *does* pop the float ring
without emitting an instruction, since the obvious family provably does not.
#### 2026-09-12, lane p23-lastmile3: proc20 exhaustive colour floor

Fresh residual map: 231 byte-exact, 39 naming, 0 immediate, 0 structural.
The mapped procedure is proc20 with 39 decisions. Its every-colour footprint
sampled 203 probes, 151 accepted; no accepted force beat 39, so the winner
list is empty and the lattice floor is 39. The named source question is the
ugen float free-list phase, with the f4/f6 cycle changing at +0x1f4; global
colour is not the lever.

<!-- plateau-handoff:func_overlay_008_F0004CF0_1862A48:end -->
