# Last-mile lane s2-b (2026-09-16/17): a deleted definition still shapes the records

Three overlay targets, 2,848 bytes, all at delta zero on arrival, each
driven down by an earlier lane and stalled with a named next step. One
matched and promoted (`overlay13UpdateRecord`, 644 bytes, 70 to 0); one
improved at delta zero (`overlay15InitStarsAndPalette`, 60 to 41); one
left in the tree unchanged with its clean form priced and its ladder read
(`overlay58FinalizePackedStatus`, 78). Seventeen of the twenty budgeted
cycles were spent (a cycle is one batched hypothesis measured with a
private direct-`cc` harness that reproduced `score_symbol.py` on every TU
before use, including the `-Wab,-r4300_mul` overlay 15 carries; every
adopted number was re-taken with `score_symbol.py` on the tree). Three
were left unspent rather than spent on cells the records did not
motivate. Evidence (every cell's source and object, listings, ladders,
freelist traces, gate logs) is under Git's common dir,
`lane-evidence/s2-b/`; nothing ROM-derived is tracked.

| symbol | bytes | before | after | cycles | state |
|---|---|---|---|---|---|
| `overlay13UpdateRecord` | 644 | 70 | **0**, promoted `d9893755` | 7 | no `state` local, `s32 timer` + `goto`, `s16 y`, x/y/z stores, two dead definitions |
| `overlay15InitStarsAndPalette` | 988 | 60 | 41, banked | 6 | `count \|= 0` after the multiply, natural xMax/yRange order, direct count store |
| `overlay58FinalizePackedStatus` | 1216 | 78 | 78 (clean form 186 to 140) | 4 | ladder read; one caller-saved permutation named |

## The mechanism: a definition uopt deletes has already been counted

Every closure this lane opened turned on the same fact. uopt runs its
forwarding, web numbering and code placement on the source's
definitions before it removes the dead ones, so a definition that emits
nothing still decides what the emitted code looks like. Three uses of
it, in the order they were found:

1. **A self-redefinition of an operand blocks forwarding at zero cost.**
   `overlay15InitStarsAndPalette` defines `starIndex = count * 12` and
   uses it twice, once as the allocate argument and once after the call.
   uopt forwards the multiply into a type-4 expression web that survives
   the call in `s3`, and `starIndex`'s own symbol web begins at
   `starIndex = 1`; the ROM keeps the size in `s0`, which under
   block-set interference is only possible as the one symbol web. What
   stops the forwarding is a redefinition of the multiply's operand in
   the same block. A dead `count = 0` does it (the `cc -S` listing turns
   `mul $19,$18,12` into a `mul` into `starIndex`'s register), but it
   makes `count`'s copy a statement copy, which as1 schedules five
   words late. `count |= 0` (or `^= 0`, `&= -1`, `*= 1`, `<<= 0`) blocks
   the forwarding and is then deleted, so `count` keeps its parameter
   copy at +0x18 and the head is exact through +0x74: 60 to 54. `+= 0`
   and `-= 0` are not deleted (+12), and a kill in any later block is +4
   to +12. Then the store order re-climbed on the new head (L146; the
   s1-b measurement of "yRange after xMax: 79" was true on the old shape
   and false on this one): the natural xMax-before-yRange order syncs
   the FP ring the previous lane had read as a folded draw, and writing
   `gOverlay15StarCount = count` directly rather than through the
   retained `(s32 *)(s32)&` cast pointer forms the address in `s7` as
   the ROM does instead of putting its hi part on `a1`. 41 at delta
   zero, 226/5/1/17 from 205/15/1/27.

2. **A declared local nobody reads is still a coloured web.** The
   retained `overlay13UpdateRecord` carried `u8 state`, assigned at five
   sites from `record->state`. The ladder shows it as web 0: a type-3
   symbol web at a u8 home, references in exactly those five blocks,
   coloured `v0`, never emitted -- a phantom, nx-a's mechanism -- while
   the value the code carries is the `record->state` expression web on
   `v1`. Its reference in the fade loop's break block denied `v0` to the
   ActiveCount address, which pushed the constant 1 off `a0`: seven
   rows, one cause. With no `state` local at all (every test reads
   `record->state`) the phantom is gone, and the reloads the ROM has on
   each loop exit, in the delay slots it has them in, are uopt's PRE of
   the load after the loop's possibly-aliasing stores. Nothing is added;
   a declaration is removed.

3. **A dead definition numbers a web and orders the hoisted code.** The
   last four rows of `overlay13UpdateRecord` were the fall loop's
   preheader: the ROM emits gravity, velocityX, z (the rotated reload),
   velocityY and colours velocityX f14, velocityY f16, gravity f18. The
   three webs tie at save 5.5 over two blocks, the tie goes to web
   number, and web number follows the definitions' order in the
   preheader -- so every spelling that emitted gravity first coloured it
   first (96 cells, floor 4). A dead `velocityX = 0; velocityY = 0;`
   before the guard numbers the velocities ahead of gravity and is
   deleted: 4 to 2. Then the hoisted section: a `record->velocityY` read
   inside the loop is hoisted after z's PRE load unless its expression
   was created earlier, and the hoisted loads are emitted in the
   expressions' creation order. So the two dead definitions
   `velocityX = record->z; velocityY = record->velocityY;` before the
   guard create z's expression, then velocityY's, then gravity's symbol
   at its preheader definition: emitted gravity, velocityX, z,
   velocityY; numbered velocityX, velocityY, gravity. 0 at delta zero;
   the dead loads in the other order are 5.

The reading for the next lane: when a residual is "the compiler chose
the lowest free colour, the target's is not the lowest", ask what
earlier occurrence the target's source could have had. The o015 `count`
kill, the o013 velocity numbering and the o013 hoist order were all
found by adding a definition that the final object does not contain.

## `overlay13UpdateRecord` -- matched and promoted

Three further facts closed it before the mechanism above did:

- **The fade exit.** A `timer` declared `s32` tests the once-masked
  forwarded reload without a second `andi`; the break path leaving
  through a `goto` past the count-exit reload, like the fall loop, keeps
  the reloads apart. The two are coupled: `s32` alone is -8, the `goto`
  alone +4, both at delta zero. With the vertex stores in x, y, z order
  (the y store sits between the x and z stores and as1 hoists it into
  the x load's shadow): 70 to 27, all naming.
- **The vertex temps were one ring position behind** (t7 for t9 through
  eight stores). The GP ring in this TU is a plain ten-register FIFO
  (t6 t7 t8 t9 t0 t1 t2 t3 t4 t5, no interleaved frees), so a draw's
  register is its index mod 10, and the trace put our first vertex
  `mfc1` at draw 12 against the ROM's draw 14. `s16 y` -- the vertex
  fields are s16 -- spends exactly two more folded draws (an `mfc1`,
  then the narrowing pair uopt drops because every use is an `sh`).
  L149 held here in its original form; `u16 y` is 23, `s32 y` 20.

Promotion: this completes the TU, so no mixed-TU range; the atlas
manifest, the digest, `overlay_undefined_syms`, the donor ledger, the
ranking (measuring form, then `--write-doc`, because `check-docs` fails
on the retired row) and the README scoreboard are regenerated. `gmake
verify` prints the expected SHA1 from the C with the pragma gone,
`promotion-proof` passes (161 words, relocations 5/5, identity static),
`check-overlay-syms` is up to date, `tools/gates.sh --staged` and
`--promotion` are green.

## `overlay58FinalizePackedStatus` -- priced, not moved

The dispatch's named step was the p1 ladder on the clean probe-free
form. Four structural facts the ROM's listing states were taken first
(one 36-cell lattice): `current`'s load and mask before the
`desired < 3` branch with the `srav` in its delay slot; the first loop's
compare as a coloured boolean; `count + 1` as a distinct value with its
own spill home; and the two literal 5s as separate webs (`5u` on either
one). 186 to 140 at delta zero, frame 0x48 (the `count + 1` forms grow
the frame to 0x68 and are excluded). From +0x1E8 to the end every row
of that form is one permutation of the caller-saved colours, and the
ladder says why: the packed-status base (48 over 17 blocks, save 2.82)
is decided after the constant 4 (3.25) and before `desired` (2.71), the
ROM's order is player, 4, desired, base, 3 on t0-t4, and `player` here
is not the load but a copy (a1, 6 over 4) of a CSE'd expression web
shared with the three selected-player re-reads (a3, 5 over 4), where
the ROM loads straight into `player`'s web. Fifteen cells on the copy
axis all lose bytes (every `= player` spelling is propagated, -16; every
kill -4), so the ROM's `or a3,t0,zero` is the forwarded second load or
a split, not a source copy on this shape. Next: force base to t3 and
player's load to t0 on the 140 form and price the permutation, then the
form that takes two references off the base without touching the
switch. The tree keeps the 78 probe form; the shard carries the
numbers.

## Where the dispatch, a shard, or the brief was wrong

- **The dispatch's L149 lead on o015 was not a folded draw.** The FP
  ring "one draw behind from yRange's mtc1" was the statement order
  xMax-before-yRange on the corrected head; no operand or draw was
  folded. L149 did hold on o013, in the y narrowing.
- **The o013 shard's named step** (an explicit `u32` local for the
  timer conversion, or the radius before the pointer) was inert or
  worse; the vertex block's schedule was the `s16 y` and the store
  order. Its "a second explicit reload is merged back by uopt" was
  right about the object and wrong about the lever: the reload the ROM
  has is PRE's, and it appears when the `state` local is *removed*.
- **The o015 shard's "not a block question"** was right; its remaining
  reading of the size as "the definition whose right-hand side reads a
  killed operand" was the right decision variable, and the kill is a
  self-redefinition of the operand that uopt deletes.
- **The o058 shard's "the ROM's selected player is a distinct symbol
  holding the second load"**: no source copy produces it on the clean
  form (fifteen spellings, all lose bytes); the copy in the ROM is a
  split or a forwarded load, and the s1-a reading that computing
  `current` before the `desired < 3` return is worse (94) held on the
  probe form only -- on the clean form it is part of the 46-word gain.
- **The dispatch's o013 budget note** ("Cap each target at 7") was
  right to the cycle: o013 closed on its seventh.
- **The corrected L99 is confirmed twice more:** on o058 most
  declaration orders change the frame size, and a declared `next` local
  grows it by 0x20.
- **Brief tool note:** `residual_map.py`'s window substitutions on o015
  were correct; the FP residual was read off the listing as instructed.
  A harness that prints `first=` in decimal (mine) reads as an offset;
  the numbers in this file are hex.

## Ledger

Seventeen measured cycles of 20: o015 6 (245 cells), o013 7 (358),
o058 4 (58), plus instrument reads (ugen `-S` listings on o015, the
s1-b ladder for o015, lineage censuses on o013 at 27 and 4 and on o058
at 140, the freelist trace on o013, frame censuses on o058) not counted
as cycles. Matched and promoted: `overlay13UpdateRecord` (644 bytes).
Improved at delta zero: `overlay15InitStarsAndPalette` 60 to 41
(commit `4364725b`). Unmoved in the tree: `overlay58FinalizePackedStatus`
78, its clean form priced at 140 with the decision order read off the
ladder. Every claimed number is `score_symbol.py`'s on the tree; nothing
was forced; `gmake verify` printed the expected SHA1 on every committed
tree.
