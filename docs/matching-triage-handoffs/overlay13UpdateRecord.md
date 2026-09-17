<!-- plateau-handoff:overlay13UpdateRecord:start -->
### `overlay13UpdateRecord` plateau handoff

- source: `src/overlays/o013/overlay13ProcessRecord.c`
- score: matched (0/161 words), promoted 2026-09-17 by lane s2-b
- frame: 0x20
- relocations: 5
- first mismatch: none
- summary: No state local (a phantom v0 web), s32 timer with a goto exit, s16 y (two folded draws), x/y/z store order, and two dead definitions before the guard that number the velocities ahead of gravity and order the hoisted loads: 70 to 0 at delta 0.

- geometry: Target and configured C remain exact at `0x284`/644 bytes/161 words with `0x20` frame; the owned Overlay 13 range is `+0x284..+0x508`, ROM `0x186ED9C..0x186F020`, followed by `overlay13ProcessActive` with no padding.
- ABI/flags: The configured candidate is `s16 *overlay13UpdateRecord(Overlay13Record *, s32)` under overlay game-code `-O2 -mips2 -32` and the canonical symbol-redefine/trim postprocess.
- relocation proof: Target and candidate each contain five records with all five offsets/types aligned. Four stable/effective identities align; the `R_MIPS_26` call at `+0x20` remains unresolved because no canonical `overlay13Prepare` definition/alias or exact sibling witness exists.
- diagnosis: Workbench reproduces 96 raw/masked positional differences from `+0x20`, with 63 opcode differences, 31 register differences, and 92 alignment gaps.
- caller/donor result: One Overlay 13 runtime call at `+0x550` and the export authenticate the entry. Donors remain weak: Conker `func_151A6350` leads at 0.0588 Jaccard and supplies no source or relocation witness.
- maintenance result: Current proxy tooling yielded no genuine lever, so no C-body, flag, or permutation hypothesis was attempted under the authorization.
- next action: Preserve the fallback pending unique same-overlay call-proxy evidence or another explicitly authorized mechanism. Previously exhausted loop, result-type, pointer-lifetime, flag, and permutation families remain closed.

#### 2026-09-13, lane `j1`: call authentication and semantic counterexample

The current baseline is 161 words, frame 0x20, masked 96, first masked +0x2C:
86 aligned exact, 34 naming, two immediate and 45 structural rows including
six candidate-only and six target-only words. The old 65-word header was
stale and is corrected. Procedure 0 has 66 draws and 244 emissions; full-TU
stock/traced text is identical. Static tables have five candidate versus three
extracted target records. Runtime target ownership has five records.

The call at owned +0x20 is no longer an unknown same-overlay proxy. Mickey's
runtime table binds it to resident +0x32BF0 (canonical address 0x80033040), whose symbol
is SetLinkSlot. The canonical C defines a three-argument signature with
narrow unsigned tag/count fields. Replacing overlay13Prepare's four-argument
proxy declaration/call with that exact signature and symbol changes no owned
instruction, draw count, per-line count, emission total or draw order. This
source-authentic call correction is retained; no shared header is changed.
Fresh function_preflight.py is complete: all five candidate and runtime
records align in offset, type, stable identity and effective identity, with
static proof and no unresolved records. The former relocation blocker is closed.

Reading the target control flow also reveals a semantic deficit hidden by the
historical score. The fall loop's entry and backedge test the old tick value
before decrementing; the retained diagnostic decrements before each test.
With state one and one tick, for example, the target executes the fall body
once while the retained C skips it. The target also snapshots the remaining
ticks into the result during each fall iteration, which the retained C omits.
The retained numeric floor is therefore explicitly NON_EQUIVALENT, not a
semantically proved near-match. Its source comment now states that limitation.

Separate private corrections preserve each attempt. Correcting the entry
snapshot alone scores 141 and adds one word; correcting the backedge too
scores 147 at the same extent, with 87 exact, 34 naming, four immediate and
42 structural rows including eleven gaps. Adding the loop-result snapshot
scores 151 with two extra words, 87 exact, 35 naming, three immediate and 43
structural rows including twelve gaps. None changes total draws or draw order:
the guard corrections redistribute emission records and the result update
adds two. A further natural while-loop control retains 66 draws but grows by
four words and regresses; it also makes the gravity/target reads loop-local,
so it is not an adopted equivalence proof.

Stop early under ADR 0018 after the authenticated ABI correction and measured
countdown/result controls, alongside the prior loop/lifetime negatives.
Do not repeat the unresolved-proxy diagnosis: it is resolved by runtime
identity. The next action is to reconstruct the target's complete countdown
and return-value semantics before solving the coupled floating-point and
result/constant allocation schedule. Retain the guarded numeric floor and
the corrected private candidates as distinct evidence; no variant is exact.
No colour sweep, flag lattice or permutation was repeated. Commands: assignment
gate, runtime-table identity decoding, canonical callee/source inspection,
function_preflight.py (complete five-of-five identities),
configured stock/traced compilation, draw_census.py comparisons,
residual_map.py --object --against, finalize_plateau.py and tools/gates.sh.
ROM verification covers the retail assembly fallback only.
#### 2026-09-16, lane s1-a: 96 to 70 at delta 0, the semantics reconstructed off the listing

Baseline reproduced at 96 masked, delta zero, first +0x2C, aligned 86
exact, 34 naming, 2 immediate, 45 structural with six insertions each
way. Six cycles, 60 cells. Retained candidate: 70 masked, delta zero,
first +0x4C, aligned 121 exact, 18 naming, 2 immediate, 25 structural
with five candidate-only and four target-only words. Not matched; the
retail fallback stays.

The j1 reading ("the target snapshots the remaining ticks into the result
during each fall iteration") is corrected: every `or v0,s0,zero` is the
dead copy of a bare `ticks--` truth test, the same artefact that closed
`overlay27UpdateCoordinates` the same day, and the function's one caller
declares it void and discards v0. Spelled `x-- != 0` the compiler builds
an `sltu` boolean instead; spelled bare it copies and tests the old value
with the decrement in the delay slot, as the ROM does. What the listing
says the source is, each measured in isolation:

- Both loops are guarded do-while loops on the old count (`if (ticks--)`
  then `do { } while (ticks--)`), the fall loop leaving through a `goto`
  past the count-exit's `state = record->state` reload (the ROM reloads
  only on that exit), the fade loop through a `break` to a reload common
  to both exits. The `for(;;)` and `while` forms with an inner exit test
  are 116 to 141.
- `state = record->state = 2` (sb then andi) in the fall loop.
- `gravity`, `velocityX` and `velocityY` are locals defined in that order
  before the loop (the FP colours f14/f16/f18 follow first-definition
  order, L106; as expression webs gravity colours first and is wrong),
  and the x update is written before the z update (as1 then schedules the
  z chain first): 9 words.
- The fade loop reads `record->timer` through the field with a forwarded
  reload (`record->timer -= 2; timer = record->timer;`): the ROM stores the
  unmasked subtraction from a ring temp and masks once into the promoted
  register. Every masked-local spelling stores the masked value or masks
  twice; the pure field form is one word short (88 at -4).
- `record->vertexIndex = 1 - record->vertexIndex` with no `index` local:
  the ROM's index is a ring temp (t1) and the chain skips a draw (t3),
  which is the deleted copy of the forwarded reload (L149/L150); the
  declared local is a coloured v1 and pushes the constant-1 web off a0.
- The four vertices are filled through a walking pointer (`v[1] = y;
  v[0] = ..; v[2] = ..; v += 5;`), which uopt folds to its final value: the
  `+48` stays with the pointer's definition before the u32-to-float
  branch and the `+30` lands after it, exactly the ROM's two adds, with
  no probe and no region. Writing the final offsets directly forwards the
  definition across the branch and folds the two adds (91 at -4); a
  discarded read of the pointer or an `if (1) { }` region also holds the
  split (79) but is artificial.

What remains, 70 words in three places: the preheader's four invariant
loads are ordered gravity, velocityX, z, velocityY in the ROM against
velocityX, velocityY, gravity, z here (defining gravity first is 73, so it
is not first-definition order alone); the fade loop's timer-zero exit
stores the count before its `beq` and carries a second `lbu` of
`record->state` in the delay slot where ours moves the store into the slot
(a second explicit reload is merged back by uopt, 70 either way); and the
vertex block schedules the 0.1875f constant first here where the ROM
schedules the index load first, with the `beql`/`lw ra` exit fill
following from that. Next: the vertex block's order is the question with
the most words; the ROM's `mtc1 at,f4` sits ten instructions later than
ours, which reads as the constant having no consumer in its block, i.e.
the u32-to-float conversion's `bgez` being a block boundary the ROM's
uopt respects and ours does not. Try the conversion spelled on an
explicit `u32` local, or the radius computed before the pointer, before
any colour work.
#### 2026-09-16, lane s2-b: a phantom state web, a narrowed y, 70 to 4

Baseline reproduced at 70 masked, delta zero, frame 0x20, first +0x4C,
aligner 121 exact, 18 naming, 2 immediate, 25 structural. Retained at 4
(4 raw): 157 exact, 4 naming, 0 immediate, 0 structural, no insertions.
Four cycles, 333 cells, a private direct-cc harness reproducing
score_symbol on the base; the instrumented uopt's ladder (procedure 0,
19 webs, all coloured, p1 only) and the freelist trace were read on the
27 shape before the decisive cells.

The named step (the vertex block's schedule from an explicit u32 local or
the radius before the pointer) was not it: the radius position and a
u32 spelling are inert or worse. What moved the function, each measured
alone and together:

- The fade exit. A `timer` declared s32 tests the once-masked forwarded
  reload without the second andi, and the break path leaving through a
  goto past the count-exit reload, like the fall loop, keeps the reloads
  apart. The two are coupled: s32 alone is -8, the goto alone +4, both at
  delta zero. With the x, y, z vertex store order (the y store between
  the x and z stores, as1 then hoists it into the x load's shadow): 70
  to 27, all naming.
- The constant 1 (a1 for a0) and the ActiveCount address (a0 for v0),
  seven rows, were one cause read off the ladder: web 0, a type-3 symbol
  web at a u8 home with references in exactly the five blocks where
  `state` is assigned, coloured v0 and never emitted -- a phantom; the
  value the code carries is the `record->state` expression web on v1.
  Its reference in the break block denied v0 to the address web. With no
  `state` local at all (every test reads `record->state`), the phantom
  is gone, the address takes v0 and the constant a0, and the reloads the
  ROM has on each loop exit are uopt's PRE of the load after the loop's
  possibly-aliasing stores, in the delay slots the ROM has them in.
- The vertex temps sat one ring position behind (t7 for t9, and so on
  through the eight stores). The GP ring in this TU is a plain ten-register
  FIFO (t6 t7 t8 t9 t0 t1 t2 t3 t4 t5, no interleaved frees), so a draw's
  register is its index mod 10; the trace put our first vertex mfc1 at
  draw 12 and the target's t9 is draw 14. `s16 y` spends exactly two more
  folded draws (mfc1, then the narrowing pair uopt drops because every use
  is an sh). 27 to 4 with the state change; u16 y is 23, s32 y 20.

What remains, four naming rows at +0x4C: the preheader loads. The ROM
emits gravity, velocityX, z (the rotated reload), velocityY and colours
velocityX f14, velocityY f16, gravity f18; ours emits velocityX,
velocityY, gravity, z with the same colours. Every cell that produces the
ROM's emission order (gravity a local, the velocities read in the loop
and hoisted: 6) rotates the colours with it, because the three webs tie
at save 5.5 over two blocks and the tie goes to web number, which
follows definition order. Measured flat or worse on the 4 shape: all six
definition orders crossed with local or hoisted forms of each of the
three and the six loop-body orders (96 cells, floor 4); gravity defined
inside the body (+4: a global load is not hoisted past the loop's
pointer stores, which also says the ROM defines it in the preheader);
any of the three defined before the ticks guard (the loads move into
the earlier block, 7 to 13). Next: a form that lowers gravity's save
below the velocities' (a third block in its web, or one more reference
on each velocity) without moving its load out of the preheader block;
the tie-break is the decision variable and the ladder on this shape is
saved with the evidence.

#### 2026-09-17, lane s2-b: matched, 4 to 0 -- a dead definition orders the hoisted loads

The four preheader rows closed in three cycles (25 cells) on the o015
mechanism transplanted: a definition uopt deletes still leaves its mark
on the records. A dead `velocityX = 0; velocityY = 0;` before the ticks
guard (or at the function top) numbers the two velocities' symbol webs
ahead of gravity's, so with gravity's definition first in the preheader
the emission order is gravity, velocityX, velocityY, z and the colours
are the ROM's: 4 to 2. The last two rows were velocityY's load before
z's PRE-inserted load. With velocityY read inside the loop, uopt hoists
it after z's load unless the expression was seen earlier, and the hoisted
section is emitted in the expressions' creation order; so a dead
`velocityX = record->z;` followed by a dead `velocityY =
record->velocityY;` before the guard creates z's expression, then
velocityY's, then gravity's symbol at its preheader definition: emitted
gravity, velocityX, z, velocityY; numbered velocityX, velocityY, gravity.
0 masked at delta 0, verified; the same two dead loads at the function
top or with velocityY defined inside the body are also 0. Controls: the
dead loads in the other order are 5; z's dead occurrence alone is 4;
velocityY = 0 in place of the dead load is 4.

The shard's earlier next step (an explicit u32 local for the timer
conversion, or the radius before the pointer) was measured inert or
worse on the way; the vertex block's schedule was the s16 y and the
store order, not the constant's block.

<!-- plateau-handoff:overlay13UpdateRecord:end -->
