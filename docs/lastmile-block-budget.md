# Last-mile lane nx-b (2026-09-16): folded draws and uopt's block budget

Lane `nx-b` was dispatched four near-miss functions, two resident and fresh
(`packInit`, `func_80020E4C`), one resident with a named hypothesis
(`func_80006534`) and one overlay (`overlay34CreateRecord`), 2,232 bytes,
all at size delta zero, with a budget of 20 measured cycles capped at six
per target. Nineteen cycles were spent. Two targets matched and are
promoted (912 bytes); one improved 6 to 2; one is unmoved at 17 with its
mechanism corrected. Evidence (every source/object pair, the aligned maps,
ugen listings, allocator traces and the synthetic block probes) is outside
the tree under Git's common dir, `lane-evidence/nx-b/`; nothing ROM-derived
is tracked.

Two mechanisms recur across the four targets and are stated first.

## Mechanism 1: a folded draw is a source spelling, and there are three of them

L149 says count draws, not registers, because as1 can delete an instruction
after ugen has drawn its ring temp. This lane closed both fresh targets on
that law and found the generators the other way round -- a target ring one
position ahead of ours is one folded draw, and the source form that spends
it is one of:

- **a redundant mask on a narrow memory read.** `packInit`'s pak pattern is
  a memory `u8`; `(pakPattern & 0xFF) & controllerBit` draws a temp for an
  `andi` that as1 folds into the `lbu`. That one cell is the whole 29-word
  residual (matched).
- **a doubly-scaled address.** `func_80020E4C` indexes its model cache with
  `(modelIndex << 1) << 2`; ugen draws a temp per shift and as1 folds the
  pair into one `sll`. Neither an or-with-zero, a `u32` cast, an offset
  local, `* 8`, `sizeof` nor typed-pointer arithmetic reproduces it, and a
  cfe-scaled subscript `[modelIndex << 1]` evaluates the index before the
  base and rotates the ring the other way. Worth 34 to 6 there.
- **a rotated `while` on an index the cursor init reads.** The same
  function's exception scan written as `while (exceptions[i] != -1 &&
  excluded == 0)` keeps `i` as a variable that the strength-reduced
  cursor's preheader init multiplies (`mul idx,2` in ugen's listing), which
  as1 prints as the target's `sll zero,1` and then deletes the dead index
  init; the guarded `do` form propagates the constant and folds the init to
  a `move`. Same instruction count, one draw and one word apart.

## Mechanism 2: uopt closes a straight-line block after a fixed budget of local loads

`overlay34CreateRecord`'s closure asked for "the block table before any
further order sweep". The table was read off the instrumented uopt's per-web
block sets (`lineage_census.py --web`), and the rule behind it was measured
on a synthetic copy of the then-block driven through the same trace
(private `bbprobe.py`, one second per cell):

- uopt ends a straight-line basic block after **twenty loads of local
  variables** counted from the block's start. A store of zero or of a
  hoisted constant costs one (the base pointer's load), a store of a
  variable costs two, a definition statement costs one (its base load; the
  definition itself is free).
- Measured: a run of zero stores holds seventeen in the first block, of
  variable stores nine, of hoisted-constant stores seventeen, alternating
  twelve; an extra definition statement moves each boundary by one;
  chaining two zero stores into one statement moves the boundary earlier,
  not later; an explicit region opener restarts the count.
- On o034 the twenty are spent exactly at `byte12 = 2` in the retained
  order and one statement earlier in the ROM's store order, which is what
  p9-mid's byte12/short16 move was buying (the literal 2's `nocs`) and what
  it cost (the store pair).

This is the "invisible boundary" three lanes had inferred and none had
placed. It is a general instrument: any residual that reads as a `nocs`
ranking inside a long straight-line block should be priced against this
budget first.

## Target 1: `packInit` -- matched and promoted (commit `92274839`)

460 bytes, 115 of 115 words, frame `0x60`, unforced. `gmake verify` prints
the expected SHA1 from the C; `tools/gates.sh --promotion` all green.
Aligner before: 86 byte-exact / 29 naming / 0 immediate / 0 structural,
masked 29, first `+0xB0`; after: 115 / 0 / 0 / 0.

One cycle, six cells. Lane g1's reading ("the target's first predicate load
is one draw ahead") was right; the generator is Mechanism 1's first form.
Controls in the same batch: a `u8` or `u32` cast of the pattern is inert
(29), masking the bit instead adds a word (70, +4), assigning the unread
`osPfsIsPlug` result is inert (29), DKR's nested `ret = osMotorInit` form
with its empty `if` is 32.

## Target 2: `func_80020E4C` -- matched and promoted (commit `0ad3c978`)

452 bytes, 113 of 113 words, frame `0x40`, unforced. `gmake verify` prints
the expected SHA1 from the C; gates green. Aligner before: 90 / 14 / 1 / 7
with one candidate-only (`+0xC0`) and one target-only (`+0xAC`) word,
masked 24, first `+0xC`; after: 113 / 0 / 0 / 0.

Six cycles, 43 cells, no colour force:

1. The array subscript the p7-res closure had "refuted as a class" is the
   right base. Its positional 77 at -4 was insertion shadow (L155): aligned
   it is 77 / 25 / 5 / 5 with one target-only word, and the head is exact
   through `+0x80` because uopt strength-reduces the subscript into the
   target's stepping cursor and the parameter web then falls to s7. Every
   subscript spelling (pre-increment, `for`, `u32` index) is that same
   object.
2. The model-cache spellings: an `s32`-pair subscript `[modelIndex * 2]`
   makes uopt strength-reduce the model index (65, +4); or-with-zero on the
   shift is byte-identical; chained index inits and narrow index types add
   16.
3. `* 8`, `sizeof`, typed-pointer arithmetic, `u32` casts, an offset local,
   a flipped compare and a shared index on the `do` base are all the
   77-object or worse.
4. The `while` form (Mechanism 1, third generator): 77/-4 to 34 at delta 0,
   all naming -- a ten-register ring rotation from the model-loop head, the
   D_80079C08 value on a3 where the target has a2, and the compare order.
   A `modelId` local, typed pointer, offset local and `for` are 34-35.
5. The rotation is Mechanism 1's second generator: 34 to 6. Index-init
   placement and narrow index types do nothing (34, +16).
6. The last six are one ranking and one web number, read from the records:
   the scan index is a two-block web (save 10) coloured a2 ahead of the
   D_80079C08 value web (save 6.4, `nocs` 5); one index `i` shared with the
   texture loop merges it into that loop's s1 web and the dead scan init
   vanishes (1); the id read through the global expression instead of a
   `cache` pointer is numbered after the scanned value, which puts the
   value first in the `bnel` (5); together 0. A diagnostic
   `exceptionIndex = excluded` (a register zero, so no index web) prices
   the first mechanism at the same 6.

## Target 3: `overlay34CreateRecord` -- 6 to 2, not matched

500 bytes, 125 words, frame `0x30`. Aligner before 119 / 4 / 0 / 2 (one
a0/v1 cycle at `+0x88..+0xA4` and the byte12/short16 store pair), after
123 / 0 / 0 / 2, masked 2 (raw 4), first `+0xE8`, delta 0, unforced. Six
cycles, 37 cells.

The four naming rows were the ranking lm-a named (resource copy 3 over
`nocs` 1 before height 4 over 2) and Mechanism 2 places height's second
block: its last store is past the twenty-load boundary. Two self-defining
or-with-zero reads of `height` after byte12 add a def and a use each in
that second block at zero width (8 over 2, decided before the copy): 6 to
2. One such read ties and loses; discarded reads are dropped by uopt (6);
three change the shape (92, -4). A region opener after the dimensions with
the height definition at its first store also flips the pair but moves the
height load (16); other opener positions are -4.

Not closed: the byte12/short16 pair. The ROM emits short16 before byte12
with the literal 2 still at `nocs` 1, so its block held both, at least
twenty-two loads under this budget against our twenty. Refuted at delta 0
or worse: the natural order (11), both physical-line folds (2, 12), the
dimension definitions folded into their first stores (27-28, frame moved),
`register` on the base or the dimensions (inert), an unscaled carrier for
both dimensions (-4, the copy vanishes), height alone unscaled (34, its
shifted value becomes a pool web), every resource-pointer carrier (R1-R7:
-4 to 44), chained zero stores (moves the boundary earlier). The next
question is the budget's unit: a base that is not a local symbol load, or
dimension values that are uopt temps in a form that keeps the copy. Price
any candidate with the synthetic probe before scoring.

## Target 4: `func_80006534` -- 17, not matched, mechanism corrected

820 bytes, 205 words, frame `0x38`. Unchanged at 188 / 15 / 0 / 2. Six
cycles, 36 cells, none below 17.

The dispatch's named hypothesis was that the region opener creates the
copy temp that holds v1. It does not: the ghost web 77 is uopt's CSE temp
for the three `record->unk8` reads (ugen loads into it, copies to `flags`,
shifts from it; as1 renames the temp into `flags`'s register, so the
surviving colour is the copy destination's), and without the region the
load still lands in a0 (63, with the sprite/cursor swap back). Both webs
are born in block 17 and interference is a block-set intersection (L115),
so `flags` cannot share v1 with the temp in any statement order; writing
the capture after both shifts is byte-identical (17). The lever is the
save ranking (temp 40 over 1 against `flags` 50 over 2) or the temp's
existence. Reading the fields from `flags` removes the temp and puts the
load in v1, but uopt then rematerialises both shifts inside the else-arms
(136, +4) in every order and opener position -- a load-derived temp is
what keeps them at the top. Types (`s32 flags` 27, `u32` field
byte-identical, `register` inert), deleting `flags` for the stored field
(43 at delta 0 with the head exact and the load in a1; +4 with the
region), tests through the packed field (+4/+8, reloads) and moving the
scale/unk5/count statements above the load (+4) are all measured.

Next: a self-defining read of `flags` after the store in block 18, which
adds a def and a use at zero width -- the form that moved o034's height
web -- read from the records before believing the score; the temp has no
name, so only `flags`'s side can be raised.

## Where the dispatch and the brief disagreed

- The dispatch says to regenerate `config/nonmatching-ranking.us.json` and
  `docs/nm-ranking.md` fully before hand-back; the brief says not to
  refresh the ranking unless `check-docs` fails without it. `check-docs`
  failed on the first promotion (a retired row), so the ranking was
  regenerated in the measuring form and `--write-doc` after every
  scoreboard change, as both then require. The brief's wording should say
  that a promotion always needs it.
- The dispatch's reading of `func_80006534` (v1 held by "a copy temp that
  the `if (1) { }` boundary creates") is corrected above: the temp is the
  CSE of the packed read and exists without the region.
- The p7-res closure on `func_80020E4C` ("the cursor is not reachable by
  respelling this loop; all seven forms compile to the same object") was
  right about the object and wrong about its meaning: that object is the
  base the match was built on, and its 77 was insertion shadow.

## Ledger

Nineteen measured cycles of 20: `func_80020E4C` 6, `packInit` 1,
`func_80006534` 6, `overlay34CreateRecord` 6. Matched and promoted:
`func_80020E4C` (452 bytes), `packInit` (460) -- 912 of the lane's 2,232,
59.66% to 59.75%. Improved: o034 6 to 2. Unmoved: 6534 at 17. Every claimed
match was verified with `tools/score_symbol.py` at 0 masked and delta 0
before promotion and with `gmake verify` printing the expected SHA1 from
the C after it, and `tools/gates.sh --promotion` was green on both.
Nothing was forced.
