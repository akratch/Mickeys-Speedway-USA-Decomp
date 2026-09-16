# Last-mile lane s1-c (2026-09-16): the carrier the source declares is the web the ROM does not have

Two targets, 3,764 bytes, both at delta zero on arrival: the calibration
target `func_overlay_060_F0002F54_18BCD2C` (888 bytes, 84 masked, no shard
history) and `func_8000590C` (2,876 bytes, 99 masked, a 46 KB shard). The
first matched and is promoted (commit `66c59a50`, 60.06% to 60.15%); the
second is banked at 26 (commits `e33974f7`, `616de6c0` and this one), with
the residual read off the allocator records and two of the shard's closures
refuted by arithmetic. Seventeen of the twenty budgeted cycles were spent
(a cycle is one hypothesis taken to a batched measurement on a private
direct-`cc` harness that reproduced both scores of record before use; every
adopted number was re-measured with `tools/score_symbol.py`). Evidence --
every candidate source and object, the ugen listings, freelist traces,
allocator records and the `.mdebug` home tables -- is outside the tree under
Git's common dir, `lane-evidence/s1-c/`; nothing ROM-derived is tracked.

One mechanism carried both targets, and it is L145/L160 stated from the
other side: **when the ROM keeps a value in a register no declared local can
take -- a ring temp, a ring temp across a branch, or a colour a phantom is
holding -- the source that produced it declared nothing there.** Six of the
eight adopted edits delete or split something the candidate had named.

## Mechanism 1: a folded draw is a mask the SDK's macros write

`func_overlay_060`'s loop body was 75 naming rows the shard called "the
display-list/x web and its downstream temporary-allocation cascade". The
freelist trace (`DKWB_UGEN_SCHED=1`) read against the target's arm-1
registers says the target's ring skips `t2`, `t5` and `t8` at exactly the
three colour-byte `lbu`s -- one invisible draw before each load, freed
before the shift. That is L149's `packInit` generator: `_SHIFTL(v, s, 8)`
on a `u8` field draws a temp for an `andi` that as1 folds into the `lbu`.
A 32-cell lattice says which masks carry it: arm 1 needs red, green and
blue (alpha inert), arm 2 needs red only -- the mask under `<< 24` of a
shifted byte is the one uopt does not remove first. Worth 78 to 19 in one
cycle, with the arm-1 `andi` no longer hoisted (as1 could hoist ours because
its draw was fresh; the ROM's reuses `t3` and carries a WAR edge).

## Mechanism 2: three appends in v0 and one in v1 is not one symbol

The target loads three display-list appends into `v0` and the loop's last
into `v1`. One declared `command` symbol web cannot do that; the first
append through a one-line packet macro with a block-scoped pointer (the
SDK's `Gfx *_g` shape) does, 84 to 78, and the 3^4 lattice over the four
sites shows why the rest stay declared: every extra block-scoped pointer is
a declared local and moves `color`'s home.

## Mechanism 3: the frame is a linear readout of the declaration list, temps included

The last seven rows on o060 were `color` one slot high. A thirty-line
`.mdebug` reader (private, `lane-evidence/s1-c/mdebug_locals.py`) settles
the layout on this compiler: **every declared local, live or dead, takes a
4-byte slot descending from the frame top in declaration order; block-scoped
locals come after the function-scope ones; and the frame is
`0x70 + round8(declared + temp reserve)`**, where the reserve read 24 with a
declared `offset` and 28 once uopt strength-reduced `x - left`. So the
reduced form (which is what puts x ahead of the display-list address web,
19 to 7) needs six slots above `color` and one fewer below it at frame
0xC0; of 64 cells, three are exact, and the adopted one declares the
input-axis value (`s16 axis`, the value the ROM holds in `v0` across both
tests) and inlines `topFloat`. Two L99 wordings are contradicted by the
readout and should be corrected: an unused `s32` is not eliminated before
the frame is sized here (a dead one costs 8), and a live register local
takes a slot position although it never spills.

## Mechanism 4: a store-forwarded re-read is a ring temp across a branch, and can leave a phantom

`func_8000590C`'s nested fixups hold the relocated pointer in `t8`/`t4` in
the ROM across the `beql` into the nested store. No symbol reaches those
registers (L130), and the previous shard had measured "deleting the local"
at 199 to 289 and closed the direction. Aligned, the same deletion with the
stored field re-read in ONE spelling is 657 exact / 20 naming against the
baseline's 661 / 47: uopt forwards the store into the CSE temp and ugen
keeps the temp in a ring register because the body block has the test
block as its only predecessor. Paired with the one-width nested read and
the unmasked `(size & 3)` copy it is 99 to 32 at delta 0.

The forwarded re-read then leaves a PHANTOM: web 584, coloured `a0` in
the site's two blocks and never emitted, tied with the nested value at 3/2
and ahead of it on web number, so the value fell to `a1`. Giving the nested
STORE's address its own spelling (`(u8 *)(u32)object->unk4C + 0x1C`)
splits the phantom at zero width and the value takes `a0` (28 to 26); the
same spelling on the read costs a reload. This is nx-a's phantom finding
for an expression rather than a declared local, and the tell is the same:
an unused register in the emitted code with a coloured web in the records.

## Mechanism 5: a tie at equal save is decided by web number, and a symbol's number is its chain

Two of the remaining pieces were exact ties. The copy-loop `loadType` piece
tied the `size & 3` test expression at 11/2 and lost on web number because
sharing `loadType` with the resource loop made the copy-loop piece a late
split web; a single-chain symbol (`remainder`, declared last so the homes
above it hold) is numbered with the type-3 webs and wins (32 to 28). The
tail's `arg1 & 1` and the two counter addresses tie at exactly 3/7 and are
coloured `s0`/`s1`/`s3` in number order where the ROM has `s3`/`s1`/`s0`.
Priced by force: the value web to `s3` is accepted and leaves the constants
in the reversed order (18 rows to 10); the address to `s0` is declined.
Region openers at six points, every dead-local carrier, the counters'
`++`/`--` spellings and the negated arm order (uopt normalises it; the
constants are numbered by the final layout) are all inert or worse. That
tie is the named next step, with the shard carrying the receipts.

## Where the dispatch, the brief or a shard was wrong

- The dispatch names `docs/lastmile-block-budget-globals.md`; the tree has
  `docs/lastmile-block-budget.md`.
- The dispatch's four-cycle cap on the calibration target was exceeded (six)
  because each cycle closed a bucket; the brief's "spend where it closes"
  wins. It matched.
- p12's "merging the two nested reads is delta -4, so the target has two
  reads there too" is a misreading of the baseline's own gap pair: delta 0
  was (-1 at +0x6DC) + (+1 at +0x7A0), so the merge MUST read -4. The ROM
  loads once. p12's "deleting `relocated` is closed" was positional shadow
  of the same -4 (L155); aligned it was the best form measured.
- The shard's "moving `resultSize = 0` to an earlier anchor restores the
  `sll`" holds only for anchors that also break the zeroing loop; the anchor
  before `if (object == NULL)` is byte-identical to the baseline. The
  `sll a0,zero,2` is a strength-reduced init and nothing else (reproduced
  exactly by indexing the loops by `resultSize`, at 35 with a different
  residual because the IV temps renumber the fixup chain's ties).
- The o060 shard's "s1/s0 display-list/x web ... prior register-web forms
  eliminated" was three separate mechanisms (a phantom-free append, a folded
  mask, a save ratio) and a frame readout, none of them a colour.
- The brief says not to refresh the ranking unless `check-docs` fails
  without it. It failed on the o060 promotion (retired row) and was
  regenerated in the measuring form then `--write-doc`; it does not fail on
  the 590C bank, so the row still reads 99 against a tree that measures 26.
  `tools/check_shard_metrics.py` flags that (and 13 pre-existing drifts);
  the coordinator's regeneration clears it.
- Promotion trap, recorded in the o060 shard: the cross-overlay `jal` to
  overlay 82 is a SYMBOL record (the ROM stores the `0xF0000000` addend),
  so the C must call `overlay82IsActive_o060Reloc`; `reloc_surface.py`
  rewrites resident calls only. And the three resident-call rebinds it
  applied in place had to be declared in the TU's `POSTPROCESS` rule or a
  clean rebuild (which `tools/gates.sh` performs) links the bare names --
  `verify` failed by exactly two bytes at 0 masked, then by a link, before
  it printed the SHA1.

## Ledger

Seventeen measured cycles of twenty: o060 6 (84 -> 78 -> 19 -> 7 -> 7 ->
0 with two frame lattices), 590C 11 (99 -> 99 -> 99 -> 288/-4 -> 32 -> 28
-> 28 -> 28 -> priced -> priced -> 26 -> 26). Matched and promoted:
`func_overlay_060_F0002F54_18BCD2C`, 222 of 222 words, frame 0xC0, 12 of
12 relocation identities, `gmake verify` from the C, `tools/gates.sh
--promotion` green. Banked: `func_8000590C` at 26 masked (695/16/2/4 from
661/47/2/12), delta 0, frame 0x90, GLOBAL_ASM canonical, `gmake verify`
printing the expected SHA1 on every commit. Nothing forced into the tree;
the two forces reported are priced diagnostics on retained objects.
