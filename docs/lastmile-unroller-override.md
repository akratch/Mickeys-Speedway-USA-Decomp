# Last-mile lane s2-a (2026-09-16): a per-file flag falsified by its own target, and an empty check that reorders webs

Three targets, 4,604 bytes, all at delta zero on arrival: `func_8000590C`
(2,876 bytes, 26 masked), `func_80006534` (820, 17) and
`overlay19BuildSpatialMasks` (908, 17). All twenty budgeted cycles were spent (a cycle is one batched hypothesis measured on a private
direct-`cc` harness that reproduced every score of record before use;
every adopted number was re-taken with `tools/score_symbol.py` on the
tree). No target reached zero. Two improved and are banked at delta zero,
both still guarded: `func_8000590C` 26 to **2** (commit `0e68ee21`) and
`overlay19BuildSpatialMasks` 17 to **9** (`5dd1c4ec`); `func_80006534`
is unmoved at 17 with its mechanism narrowed. Evidence (every cell's
source and object, decoded ladders, block sets, side-by-side listings) is
outside the tree under Git's common dir, `lane-evidence/s2-a/`; nothing
ROM-derived is tracked.

## Where the dispatch, a shard, the brief or the Makefile was wrong

- **The Makefile's `-Wo,-loopunroll,0` on `src/main/objects.c` was a
  claim resting on a NON_MATCHING candidate.** Its comment credited the
  "rolled byte-copy loop in func_80005548"; that function is
  `GLOBAL_ASM`-backed and the flag came in with commit `7adbef15`
  ("Candidate func_80005548"). Compiled without it, exactly six functions
  in the TU change, all six of them `GLOBAL_ASM`-backed candidates; the
  other 59, every matched function among them, are byte-identical and
  `gmake verify` prints the expected SHA1. And the target of this lane
  refutes it outright: the ROM's copy loop is IDO's unroller output. The
  override is removed. **The five other candidates in the TU
  (`func_80004590`, `func_80004FE0`, `func_80005548`, `func_80009414`,
  `func_8000A39C`) will score worse until their hand-unrolled loops are
  rewritten as the plain loops the unroller expects** -- their sizes grow
  by 12 to 680 bytes under the unroller -- and their ranking rows are
  stale until the coordinator regenerates.
- The dispatch's named step for `func_8000590C` ("a nocs difference from
  a different block shape around `object->unk68 = ...`") was not it: the
  three webs' `nocs` never moved. The tie was broken by an empty
  overflow check after the special-list append, which changes the
  constants' bookkeeping at zero width.
- s1-c's shard said the copy loop's `sll a0,zero,2` "is only reachable
  by strength reduction" and that the all-subscripted form is 35 because
  "the SR temps renumber the fixup-chain ties". Both were true of the
  hand-unrolled shape and beside the point: the preheader is the
  unroller's, and the seven fixup-chain rows were `offset` shrinking to
  a small type-3 web that wins a tie on number.
- The o19 shard's "reading the span through an expression instead of a
  carrier is 210 at +24 because the volatile offset is re-read per
  occurrence" was measured with the volatile member. With the union's
  non-volatile member the expression is CSE'd to one load and is the
  17-to-9 edit; and the volatile qualifier never produced the loop-head
  reload (the carrier form with the non-volatile member is
  byte-identical to the base). The memory-class struct does that.
- The dispatch's "empty if" reading transfers only where the check
  emits nothing: on `func_80006534` every empty-`if` use of the fields
  costs 4 to 48 bytes.
- Region openers (`if (1) { }`) at six points in `func_8000590C`'s tail
  are byte-identical to the base, as s1-c found; the dispatch's
  suggestion to re-read them was worth one cycle and no more.
- The coordinator's resume note placed this lane at the zsh
  word-splitting trap; it was at 26 to 2 with two staged files. Nothing
  was lost.

## Mechanism 1: a hoisted constant def sits a fixed count above its use; the second def's placement is the web's shape

`func_8000590C`'s tail was a three-way 3/7 tie (`arg1 & 1` and the two
counter addresses) coloured in web-number order. The records say more
than the ladder: the ROM's second materialisation of `&D_800C9498` sits
at the join, ours on the else-arm edge, so the ROM's failure-path range
has both predecessors outside it. And both sides' hoisted first def
sits exactly 0xB4 above the tail test whether a fixup diamond above it
is deleted, one between is deleted, or one is duplicated (three
diagnostic compiles): the hoist is a fixed dominator-count rule,
register permitting, on both sides. So the difference was bookkeeping,
not placement. The DKR sibling's `spawnObject` carries an overflow check
after its list append; an empty `if (D_800C94A8 > 0x100) { }` after the
special-list increment emits nothing and reorders the constants' webs:
26 to 5 in one cell. On the regular list alone it is 29; on both, the
object is byte-identical to the else-arm one.

## Mechanism 2: a per-file flag is a falsifiable claim, and the ROM's loop said which way

The remaining preheader rows (`sll a0,zero,2; addu v1,v0,a0` against two
moves) and the ROM's `beqz` landing *past* the `resultSize != size` test
on the no-remainder path are the unroller's signature: it knows the
trip count is at least four there. The plain `while (resultSize < size)`
loop compiled without the override reproduces the remainder loop, the
bypass, the 4x body and the un-folded init, and the six `size = size |
0` probes become inert (0 through 5 probes all measure the same),
because `size` has its loop weight naturally. Only the all-subscripted
shape keeps the head: with the copy loop plain but the zeroing loop's
explicit `offset` carrier kept, `offset` loses its loop references, its
save falls and `selected` and the constant 1 rotate at +0x4. The unk48
nested value then has to be the expression, not `offset`: as a two-block
type-3 web `offset` is numbered before every type-4 web and wins the
1.5 tie against the unk48 value on number, where the ROM decides the
value first. 8 to 2. `offset` stays declared and dead; the frame is
0x90 with it and every live local.

## Mechanism 3: p2 orders by web number, and the number follows the type

`overlay19BuildSpatialMasks` had `span` (web 14, created at event 5)
and `firstItem` (web 19, event 7) as type-3 webs coloured in ascending
number in p2. A declared pointer is always numbered first. Dead
`firstItem` definitions at the loop head are deleted before numbering
(three spellings, inert). Reading the four span fields through the
expression itself, with the union's non-volatile member so uopt CSEs it
to one load, makes the base a type-4 web -- numbered after every type-3
web, decided after `firstItem`, coloured a0 with `firstItem` in v1: 17
to 9. What is left is as1 order: the mask/selector zero-init pair and
loop 1's tail (the sign-extension order and the `slti` position). A
16-cell physical-line grouping lattice of the tail's four statements in
two orders floors at 9; loop 1 as a `for` still loses the counter
numbering on this shape (26, 26, 63).

## `func_80006534`: the single-web shape, and what does not reach it

The ROM holds the packed word in one v1 web from the load through the
tests with the extractions at the top. Ours: the CSE temp (web 77, 40
over 1) is coloured v1 in the records and as1 renames it into `flags`'s
a0 to delete the copy. Four cycles, 41 cells:

- no `flags` local, everything through the stored `sprite->unkC`
  (forwarded), no region: byte-identical to the base; the forwarded value
  is a second web (40 over 2) and the copy is back. The region is not
  needed in that form.
- every expression-of-load definition of `flags` (or 0, xor 0, and ~0,
  plus 0, shift 0, u32 cast) is normalised to a load-defined symbol and
  sinks the extractions into the arms (+4);
- tests reading `record->unk8` directly reload in every store placement,
  including all three sprite stores moved after the chain (+8 to +32),
  so the reload is not an alias decision;
- empty-`if` uses of the fields at six points cost 4 to 48 bytes and do
  not stop the sinking.

Next: instrument the sinking pass; no spelling reaches it.

## Ledger

Twenty measured cycles: `func_8000590C` 12 (26 -> 77-183 -> 26 -> 5 -> 4
-> 11 -> 664 (unroller on, hand loops) -> 8 -> 2 -> 2 -> 2 -> 2 -> 2; the
last two at the coordinator's request, nine address and add spellings flat),
`func_80006534` 4 (17, 17, 140+, 136+), o19 4 (17 -> 9 -> 9 -> 9). Banked:
`func_8000590C` at 2 masked (717/2/0/0 from 695/16/2/4), delta 0, frame
0x90, 99 relocations, `GLOBAL_ASM` canonical; `overlay19BuildSpatialMasks`
at 9 (220/4/0/2 from 212/12/0/2), delta 0, frame 0x80. `gmake verify`
printed the expected SHA1 on every commit and `tools/gates.sh --staged`
was green on each. Nothing forced into the tree; the ranking rows for the
three targets and the five other objects.c candidates read stale until
the coordinator regenerates.
