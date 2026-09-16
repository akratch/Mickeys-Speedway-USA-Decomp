# Last-mile lane s1-a (2026-09-16): the loop keyword, the block cursor, and a sibling that was not one

Four overlay targets, 2,876 bytes, all at delta zero on arrival. Two
matched and promoted (`overlay27UpdateCoordinates`, 260 bytes, and
`overlay13DrawRecord`, 756 bytes); two improved at delta zero
(`overlay13UpdateRecord` 96 to 70 with its semantics reconstructed,
`overlay58FinalizePackedStatus` 88 to 78). Fourteen of the twenty budgeted
cycles were spent, one of them void (a generator edit moved a base and the
base-reproduction cell caught it). A cycle is one batched hypothesis
measured against `tools/score_symbol.py`'s comparator; a private direct-`cc`
harness reproduced the score of record on every TU before use. Evidence
(variant sources, objects, listings, lineage and draw censuses, gate logs)
is under Git's common dir, `lane-evidence/s1-a/`; nothing ROM-derived is
tracked.

| symbol | bytes | before | after | cycles | state |
|---|---|---|---|---|---|
| `overlay27UpdateCoordinates` | 260 | 19 | **0**, promoted `9c0a85f0` | 1 | `while (n--)` numbers the temp ahead of the indices |
| `overlay13DrawRecord` | 756 | 97 | **0**, promoted `42321d51` | 2 | block-scoped cursor, gDma1p's OR order, _SHIFTL masks |
| `overlay13UpdateRecord` | 644 | 96 | 70, `ced935cd` | 6 | semantics reconstructed off the listing; void |
| `overlay58FinalizePackedStatus` | 1216 | 88 | 78 | 5 (1 void) | three orderings; probe-free shape found |

## The sibling bet did not transfer, and the measurement says why

The dispatch's lead hypothesis was that the two o013 functions are a
sibling pair -- 97 and 96 masked rows -- to be worked as one problem. The
per-window register substitutions were diffed before either was touched:
`overlay13DrawRecord` reads `t1->t3 x6, a2->a3 x5, t0->t2 x4` with a
five-cycle in the temp ring; `overlay13UpdateRecord` reads `t2->t3 x4,
v1->t1 x4, t0->a0 x3` with an `a0<->t0` two-cycle. No mapping is shared,
the aligner shapes differ (71 naming / 22 structural against 34 / 45 with
six insertions each way), and the bodies are unrelated code -- a
display-list builder and a physics update. The row-count coincidence was
exactly that. No cycle was spent on transfer; the two were worked as two
problems and closed on different mechanisms, and the o013 pair's shared
budget of eight was spent 2 + 6.

So "sibling" is not a reliable multiplier on row count alone. The o058
quads transferred because they were the same code; the test that predicts
it is the residual-vs-residual substitution diff, which costs nothing.

## Mechanism 1: the loop keyword decides a temp's web number

`overlay27UpdateCoordinates` is a leaf, so p2 only: colours are assigned in
ascending web number, lowest free colour (L141, L154), and web numbers
follow first-definition order in the body (L106). The lineage census on
procedure 3 numbered the declared symbols 0/7/16/19, the loop's three
scaled-index expression webs 23/33/43, the post-decrement temp of
`remaining--` 54 and the address constants 61 to 67 -- and read against the
listing that order gives our object exactly: countdown a1, indices a2/a3/t0,
temp t1. The ROM has the temp on a1 ahead of the indices (a3/t0/t1) and the
countdown on a2, so the temp must be numbered before the index expressions.

A `do { } while (remaining--)` creates that temp after the body. Written
`remaining = 10; while (remaining--) { }` the temp is created in the
condition, before the body; uopt propagates the constant through the
peeled first test, so the emitted shape is the identical do-while with the
preheader 9 and the numbering is the ROM's. 19 to 0 on the first compile,
delta zero, and `gmake verify` prints the expected SHA1 from the C.

Two controls are worth keeping: `while (remaining-- != 0)` is a *different
object* (16 at -12 bytes) -- cfe builds an `sltu` boolean for an explicit
comparison of a post-decrement and copies-then-tests for a bare truth
test -- and every `for`, pre-decrement and pointer-bound form changes size.
The same fact closed the o013 update function's count tests below.

## Mechanism 2: a shared display-list cursor is a phantom web

`overlay13DrawRecord`'s 97 were three source-shape facts, none of them a
colour:

1. Every command in the ROM is a gbi-style block with its own cursor,
   `{ Gfx *_g = (Gfx *)pkt; _g->w0 = ..; _g->w1 = ..; }` (PROVENANCE at the
   point of use: the shape of DKR's `fast3d_cmd`/`gDma1p` macros in
   `include/f3ddkr.h` and `gbi.h`; the words are this ROM's own). The
   retained source appended through one declared `cmd` local. That local
   was a phantom web (nx-a's mechanism on the o058 quads): it held v1
   across the else arm and pushed the render pointer to v1 and the D_24
   address to t0 where the ROM has t1 and t2, rotating the temp ring by
   two; the ROM's unused v1, a0 and t0 were its shadow. With the
   block-scoped cursor those three colours and the ring agree, and as1's
   line tie (L59, L132) stores w1 before w0 wherever both words are ready
   together, which the ROM does at five sites. Every one-line or
   cursor-fold spelling that keeps one declared cursor stays at 92 to 104.
2. The vertex word is built in gDma1p's order -- the command byte ORed
   with the shifted packet byte, then the length ORed in, an `or` with `at`
   followed by `ori 0x30`; uopt does not fold the two constants. Ours had
   folded them by hand. 97 to 39 with the block cursor.
3. The prim-colour word carries `_SHIFTL`'s `& 0xFF` on each u8 byte. With
   the masks uopt evaluates r before g and draws the env command's `lui`
   immediately after the prim command's, which as1 then hoists to the block
   top; without them every spelling (casts, grouping, reversal, addition,
   `s32`) is 38 or 39. 39 to 0, delta zero, verify from the C, promotion
   proof 18 of 18 identities.

The general reading: a display-list function written against Rare's macros
has one cursor *per command*, and reconstructing it with a shared cursor
buys a phantom register for the whole arm.

## `overlay13UpdateRecord` -- 96 to 70, the semantics reconstructed

The shard called the retained candidate NON_EQUIVALENT and read the ROM's
`or v0,s0,zero` copies as "result snapshots". They are the dead copies of
bare `ticks--` truth tests (Mechanism 1's artefact), and the one caller
declares the function void and discards v0. Reconstructed off the listing
and each measured alone: both loops are guarded do-while loops on the old
count (the fall loop leaves through a `goto` past the count-exit's
`state = record->state` reload; the fade loop through a `break` to a
common reload); `state = record->state = 2`; `gravity`, `velocityX`,
`velocityY` as locals defined in that order (the FP colours f14/f16/f18
follow first-definition order, and as expression webs gravity colours
first and wrong) with the x update written before the z update (9 words);
the fade loop reads `record->timer` through the field with a forwarded
reload (the ROM stores the unmasked subtraction from a ring temp and masks
once); `record->vertexIndex = 1 - record->vertexIndex` with no index local
(the ROM's index is a ring temp and its chain skips a draw, the deleted
copy of the forwarded reload, L149/L150); and the four vertices filled
through a walking pointer that uopt folds to its final value, which is
what puts the `+48` before the u32-to-float branch and the `+30` after it
with no probe and no region.

Aligner 86/34/2/45 to 121/18/2/25 at delta zero. The 70 that remain sit in
three places, each with a measured negative: the preheader's four
invariant loads are ordered gravity, velocityX, z, velocityY in the ROM
(defining gravity first is 73, so first-definition order alone is not it);
the fade loop's timer-zero exit keeps its store above the `beq` and carries
a second `lbu` of `record->state` in the delay slot, where ours moves the
store into the slot and a second explicit reload is merged back by uopt;
and the vertex block schedules the 0.1875f constant first where the ROM
schedules the index load first and the `beql`/`lw ra` exit fill follows
from that. The ROM's `mtc1 at,f4` sits ten instructions later than ours,
which reads as the constant having no consumer in its block -- the
conversion's `bgez` being a block boundary the ROM's uopt respects and
ours does not. Next: the conversion on an explicit `u32` local, or the
radius computed before the pointer, before any colour work.

## `overlay58FinalizePackedStatus` -- 88 to 78, and the shape under the probes

Three orderings read off the listing, all semantically inert, additive on
the probe-bearing source: `shift` defined before `mask` in every arm
(shift v1, mask a0 by first-definition order); the extended test written
`== 0` with the 0x1C0/6 arm as its then arm (uopt hoists the else arm's two
constants above the branch, so the arm that appears first in the object is
the else arm, and the ROM has 0xE00/9 first with `bne`); and every mode
section defining `i`, then `count`, then its selected player, the ROM's
three-`or` prologue in all three sections. 88 to 86 to 81 to 78. Computing
`current` before the `desired < 3` return is 94; a boolean local for the
first loop's compare is inert.

The finding for the next lane: the six probes are pinning a colour
ordering on the wrong loop shape. With no probes and the selected player
re-read from `records[0].player` (no `volatile`, no copy of `player`), the
mode-0 loop is instruction for instruction the ROM's -- the forwarded
second load `or a3,t0,zero`, the `i != selected` compare first with the
address shift in its delay slot, a plain `beq`, the count exit -- and the
residual of that form is 129 naming rows of which 44 are one web, the
packed-status base address coloured t0 there and t3 in the ROM, with the
ROM's t0 and t1 held by `player` and the constant 4. The plain copy
`selectedPlayer0 = player` is propagated away and loses four words (272 at
-16), so the ROM's selected player is a distinct symbol holding the second
load, not a copy. The retained source's `volatile` reload inside that loop
is not in the ROM. Next: on the clean form, read the p1 ladder for the
base-address web against `player` and the constant-4 web, then retire the
probes one at a time against that form. Do not add a seventh.

## Where the dispatch, a shard, or the brief was wrong

- The dispatch named `docs/lastmile-block-budget-globals.md`; the file is
  `docs/lastmile-block-budget.md`. It said `overlay58FinalizePackedStatus`
  had no shard; it has a 5 KB one with three lanes' history, which is why
  the probe form was not re-swept.
- The dispatch's o013 sibling bet is refuted by the substitution diff
  (above). Row-count near-identity did not predict a shared mechanism.
- The o013 update shard's reading of the `v0` copies as result snapshots,
  and its NON_EQUIVALENT verdict on the countdown, are both corrected: the
  copies are `ticks--` test temps and the function is void. The shard's
  measured negatives on the countdown corrections (141 to 151 with size
  growth) were of `!= 0` spellings, which build a boolean; the bare forms
  are the ROM's.
- The o013 draw shard's word count was 192; the function is 189 words.
- The brief says not to refresh the ranking unless `check-docs` fails
  without it; both promotions retired a row, `check-docs` failed, and the
  ranking was regenerated in the measuring form then `--write-doc`, as
  nx-b and w1-a recorded. The brief's wording should say a promotion
  always needs it (this is the third lane to write that sentence).
- The brief's shard grammar forbids `|`; a C expression quoted in a shard
  trips it, so expressions are written as prose there. That cost one gate
  run.
- The dispatch's "test L149 first on every target" was right on none of
  the four as a folded draw, but the *count draws* discipline found the
  skipped draw in the o013 update's index chain, and the `_SHIFTL` masks
  that closed the draw function are the inverse of L149's redundant-mask
  generator: here the ROM *has* the masks and needs them for the
  evaluation order.

## Ledger

Fourteen measured cycles of 20: o027 1, o013 draw 2, o013 update 6, o058 5
(one void), plus instrument reads (lineage census on all three TUs, draw
census on o027, residual maps on kept objects) not counted as cycles.
Matched and promoted: o027 (260 bytes, `9c0a85f0`) and o013 draw (756,
`42321d51`), 1,016 of the lane's 2,876 bytes. Improved at delta zero: o013
update 96 to 70 (`ced935cd`), o058 88 to 78. Every claimed match was
verified with `tools/score_symbol.py` at 0 masked and delta 0 before
promotion and with `gmake verify` printing the expected SHA1 from the C
after it, `promotion-proof` passing (o027 10/10 identities, o013 18/18),
`check-overlay-syms` up to date, and `tools/gates.sh --staged` and
`--promotion` green on both. Nothing was forced.
