# Last-mile lane w1-b (2026-09-16): the block budget counts local loads, and the o058 pair is block structure

Lane `w1-b` was dispatched four overlay functions at size delta zero:
`overlay34CreateRecord` (2 masked), the `overlay58DrawPointQuad` /
`overlay58DrawLargePointQuad` pair (21 each) and `overlay14CreateValue`
(13), 1,716 bytes, twenty measured cycles capped at six per target. Twenty
cycles were spent: o034 six, the o058 pair eight (six on the Large sibling,
two on the transfer and the last two words, charged to the Point sibling),
o014 six. Two targets matched and are promoted (832 bytes); one went 13 to
2; one is unmoved at 2 with its mechanism now calibrated exactly. Evidence
(sources, objects, block-set readings, ladders, ugen listings, the as1 trace)
is outside the tree under Git's common dir, `lane-evidence/w1-b/`; nothing
ROM-derived is tracked.

Every score below is `tools/score_symbol.py` or nm_ranking's comparator on
the configured TU; nothing was forced into any candidate. `gmake verify`
printed the expected SHA1 from the C, with `GLOBAL_ASM` gone, for both
promoted functions, and `tools/gates.sh --promotion` was green before the
checkpoint commit.

## The instrument: what uopt's block budget actually counts

Lane nx-b found that uopt closes a straight-line basic block after "twenty
loads of local variables" and priced it on a synthetic copy. This lane
calibrated it on the real `overlay34CreateRecord` TU with a sliding
sentinel -- `byte03 = 2`, K stores, `byte12 = 2`, and the literal's block set
read off the instrumented uopt -- so each construct's cost is a measured
integer:

- The unit is a LOAD OF A LOCAL VARIABLE, parameters included. A store of a
  constant costs one (its base pointer), a store of a local two,
  `p->f = q->g` two, `p->f = g` one, a definition costs one per local it
  reads regardless of its ILOD chain, a self-defining `x = x | 0` costs one,
  `register` is inert.
- Global loads cost NOTHING: `g = 7` and `g++` are zero units. ILODs,
  constants, arithmetic nodes and statements themselves cost nothing.
- The block closes when the running count reaches twenty; the counter
  restarts at the block head (after a branch), not at a call: probes and
  stores placed in the call block before the resource test move nothing.
- The count is taken after propagation: `height = width - 1` folds its own
  def away and reads as two fewer.

Two consequences drove the rest of the lane. On o034, the ROM's store order
holds nineteen units before `short16`, so `short16` (two) closes the block and
`byte12` opens the next one; both in one block needs at most seventeen
before `short16`, and dropping two zero stores achieves it while dropping
one does not. On o058, the appends hold only four local loads, so the block
boundaries the ROM's colouring requires cannot come from the budget and must
be zero-cost openers.

## The o058 pair: matched and promoted, and yes, the mechanism transferred

Both at 0/104 words, delta 0, frame 0x18, 11 relocations, `verify` exact
from the C. Aligner before: 83 / 21 / 0 / 0 on each; after: 104 / 0 / 0 / 0.
Every measurement was taken on the Large sibling and the final form was
transferred to the Point sibling unchanged except for the names and the
8/18 offsets: it scored 0 on the first compile. Commit `2209b01c`.

The residual was four colours (`dl` v0, `0x80000000` t1, `y` t0, `xPlus`
a1) plus the argument `lui` pair. Read as a ladder, the ROM's colours say
what its block structure was:

1. Both packet appends and `vertices = cursor` in ONE block. The shared
   `dl++` expression web then has one occurrence set (save 6.0, not 3.0)
   and is decided before the packet macros' two block-scoped `_g` phantoms
   (3.0 each), which is what gives it v0; the phantoms take a1/a2 in that
   block and, with `vertices` (v1) also live there, the display-list
   address falls to a3 and `0x80000000` to t1.
2. A zero-cost boundary after `vertices = cursor` and another after the
   last colour store, so `0xFF` is a single-block web (15, v0 in a block the
   `dl` web does not reach) and the four coordinate expressions are
   single-block webs coloured v0/v1/a1/a2 in first-occurrence order --
   exactly the ROM's `xMinus v0, zMinus v1, xPlus a1, zPlus a2`.
3. The last two words are the two argument `lui`s: as1's trace ties them
   at equal `aftercycles` and breaks the tie on source line (79 against 80),
   and physical-line variants of the call are inert because each argument
   carries its own line. Reading the render-state field into a local
   inside a region before the call gives the load the earlier line: 0.

The packet spellings are the JFG engine's own `gSPVertexJFG` and
`gSPPolygon` through `gDma1p` (PROVENANCE in the source): the vertex macro's
`v` argument appears twice, which is why the ROM loads the cursor twice,
and each macro is one physical line, which is what the "one-line append"
trick had been emulating. The dead `vertices += 3` survives without any
probe.

What was measured closed on the way (cells in `lane-evidence/w1-b/o058/`):
the natural probe-free body 47 (the budget splits the colour stores); the
macro idiom in the base's probe frame 79-84 (two phantoms plus a 6.0 `dl`
web in the colour block); every region form without `vertices` in the
append block 81-87; a def/use split of the carrier across a region 66 (the
value crosses the boundary through the symbol, adding moves); `gfx = dl;
dl++` byte-identical to `gfx = dl++` (uopt propagates the load temp, not
the global, so the phantom survives every carrier spelling); the coordinate
locals declared 15-17 against inline expressions 2-4.

`gmake promotion-proof` refuses both with `candidate relocation symbol
D_8000009x has conflicting runtime identity`. That is the resident
auto-name versus reserved-selector namespace conflict already recorded on
`func_overlay_050` and the whale, and the promoted precedents
`overlay101DrawClock` (D_800002D8) and `overlay96DrawObject` (D_80000000)
refuse identically today. The shipped words at those sites are the real
resident addresses the extracted asm also names, `verify` is exact, and
`check-overlay-syms` is up to date; the refusal is the proof tool's, not the
bytes'.

## `overlay14CreateValue`: 13 to 2, the home and the PRE

Baseline 13 masked (14 raw), aligner 83/5/4/4; retained 2 (3 raw), delta 0,
frame 0x28, aligner 94/0/0/2, first +0x158. Read off the objects: the
target's pointer is one symbol web (scan cursor, free-loop cursor, chosen
slot) coloured v1, spilled around all three calls to its OWN home at 0x18,
with the shift a ring draw. Ours had the pointer flowing through a volatile
home plus a `register` copy spilled to a temp at 0x24, and a PRE'd shift.

Six cycles, in order: taking `&slot` in a dead branch removes the pointer
from colouring entirely (61-94; L55's mechanism is wrong here); the
plain-`chosen` shape the previous entry described as "the target from +0x9C
on" is 58 at plus four with the first difference at +0x4, because the
cursor web no longer spans the index blocks; one `slot` symbol through
everything with `index = 0` after the pointer and one or-with-zero read of
it is 25 at delta 0 (the read lifts the web to 59/7 = 8.43 past `value`'s
8.33, so it is decided first and takes v1); declaring `slot` FOURTH moves
its reserved home to 0x18 and the frame to 0x28, 10 (a coloured pointer
spilled around calls spills to its own reserved home, and homes descend in
declaration order; unused f32/f64/array locals buy nothing, against L99's
wording); `slot = base + index` is a different IR name from the loop's
`&base[index]`, so the shift is no longer PRE'd into a two-block web and is
drawn from the ring, realigning every later draw, 2. The last two rows are
the count load the ROM schedules above the key store. Lane lm-o014
(2026-09-17) proved the hoist by listing replay: a noalias stamp between
the slot register and the count address is exact, as is swapping ugen's
load/store emission. The instrumented alias query is may-alias because the
spilled slot is isvar against the count's islda -- a one-base pointer that
spills still queries may-alias, so the three-base diagnosis was the wrong
variable. A post-call `la` of the chosen base into slot stamps the fact at
plus four and 13 masked. Lane w2-o014 (2026-09-17) priced the other named
lever: any declared count carrier takes a1 (first free colour at the tail)
and a comma inside `+` folds to the baseline. Next steps are in the shard.

## `overlay34CreateRecord`: unmoved at 2, mechanism exact

The byte12/short16 pair needs both in one block with the ROM's order, i.e.
at most seventeen local loads before `short16` against nineteen in any
statement-per-store form with two declared dimensions. The one form that
reaches seventeen -- store the dimension expressions once and re-read the
fields -- fails because uopt will not forward an s32 truncated into an s16
field (it reloads with `lh`, 97 at plus four, frame 0x28). Measured inert
on the budget: block-9 content, `register`, ILOD chains, arithmetic nodes.
Next, in the shard: a base for the second half that is not a local symbol
load, or a store family cfe emits with one base load for two fields.

## Where the dispatch, the shards or the brief were wrong

- The dispatch's o034 next step, "a base or dimension carried in a uopt
  temp, priced with the synthetic probe first", was right in direction; the
  synthetic probe's rule was incomplete. Global loads are free, ILOD chains
  are free, and the counter restarts at the branch, none of which nx-b's
  rule states, and "a store of a variable costs two" is really "one per
  local load" (a `p->f = g` store is one).
- The dispatch said the o058 colour axis was closed and the question was
  "interference/block structure"; that was right. It also said to expect a
  `.rodata` pool from the `0xFF` constants: there is none, they are `li`
  immediates, and no normalization was needed.
- The o014 shard's claim that the plain-`chosen` plus killed-index form
  "is the target's from +0x9C on" is false as measured here: 58 at plus
  four with the head moved. Its named decision ("slot 57/7 vs value 25/3")
  was right, and one zero-width read settles it.
- L99's "an unused f32 or pointer is not eliminated" did not hold on o014:
  neither enlarges the frame. What sizes the frame is the reserved home of
  a coloured pointer that gets spilled, and its position is declaration
  order.
- The brief says a `_oNNReloc` alias is needed for every resident name a
  promotion references; the two low resident payloads here are named as the
  extracted asm names them, link to the shipped words, and are what the
  proof tool cannot classify. The promoted precedents carry the same names.

## Ledger

Twenty measured cycles of twenty. Matched and promoted:
`overlay58DrawLargePointQuad` and `overlay58DrawPointQuad` (416 bytes each,
832 total). Improved: `overlay14CreateValue` 13 to 2 (retained,
NON_MATCHING). Unmoved: `overlay34CreateRecord` at 2. Nothing pushed.
