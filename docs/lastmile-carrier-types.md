# Last-mile lane lm-c (2026-09-16): declared types and carriers

Eight small near-misses, 4,048 bytes, all at delta zero on arrival. One
matched and promoted; seven left with a measured negative each. 15 of the
24 budgeted cycles were spent (a cycle is one batched hypothesis measured
against `tools/score_symbol.py`'s comparator); the rest were left unspent
because every remaining target carries a closure the reads below did not
overturn, and a fifteenth cycle on any of them would have been a sampled
lattice against an argued mechanism.

The one lesson that paid: read the target's instruction stream for what the
original **declared**, not just for what it computes. The overlay 41 match
was four carrier facts and no colour force, and the decisive one was a
variable's *type* -- a `u8` that turns a copy into a conversion node uopt
does not propagate. Two other targets (`func_80046BCC`, `func_80006534`)
have the same class of tell in the target stream; both resisted it in four
cycles each, for reasons recorded below.

Instruments used that are not cycles: `residual_map.py`, both sides'
disassembly in private scratch, `lineage_census.py` on `func_80006534`
(procedure 23) and `func_80046BCC` (procedure 11). The direct scorer was a
private wrapper over `nm_ranking.process_item` with the configured per-TU
command; its base score reproduced `score_symbol.py` on every TU before use.

## func_overlay_041_F0001298_18885D0 -- matched, promoted, 460 bytes

14 -> 8 -> 4 -> 1 -> 0 at delta zero, 4 cycles, 54 cells. `u8 limit` makes
`divisor = limit` a widening conversion, which uopt keeps as the target's
`move a0,v1` instead of coalescing (with the start read moved after the
clamp, where the target reads it): 8. Comparing through the stored field
(`state->value39 < 0xFF`) lets uopt forward the store into the reload and
mask it, the target's `sb` of the unmasked sum beside an `andi`: 4. No
`value`, `start` or `delta` local at all -- the sum is a ring temporary in
the target and any declared carrier takes a pool colour that pushes `divisor`
off `a0`: 1. The redundant `& 0xFFFF` the p9-mid pass fitted as a ring-phase
device was the one surplus draw on the new shape (L146): 0. Verified:
`gmake verify` prints the expected SHA1 from the C, `promotion-proof` PASS
(identity=static, 4/4 relocations), `check-overlay-syms` up to date,
`tools/gates.sh --promotion` green. Commit `41cdc3a0`.

## func_80010900 -- 3, unchanged (1 cycle)

The requirement is one web through two definitions (the sum and the sqrt
result). Hypothesis: a self-reading def at the *join* after the `if` block,
the whale's `i &= 0` mechanism, gives both defs a common reachable use.
Nine cells: `x = x`, `x += 0.0f`, `(f32)x`, a copy into either dead frame
carrier, the loop-condition comma, the loop top: all byte-identical at 3.
The else-arm placements change the shape (delta -8). uopt folds every plain
join-point read here before web building; the whale's form worked because
its read sat between two competing stores of the same variable. Next: a def
that reads itself *and* cannot be folded because its own def follows the
read in the same statement -- no float spelling of that exists without an
instruction, so the answer is more likely on the target side: the sum is
stored into the frame as `scratch.length` in the original (19 with a plain
store, measured before), and a form where that store is the only def of the
length symbol before the call is the untested cell.

## levelFreeAll -- 3, unchanged (0 cycles)

Read only. The closure (mask, table, scale, sum ring order inside one
expression; both postorder classes measured; ~2,000 cells) is mechanically
argued from the compiler listing and the free-list trace, and nothing in
the carrier-type lens reaches a ring draw order. One untested axis worth a
single cell: the index as a cfe *bit-field extraction* (`u16 index : 14` on
the entry word) rather than an `&` -- a different lowering that may
materialise the field before the base load. It is cheap to refute because
it probably turns the `lh` into `lhu`.

## func_8005ABA8 -- 2, unchanged (0 cycles)

Read only. The residual is an as1 branch-likely decision proved to need a
zero-node target block; no source produces one. Nothing here reopens it.

## func_8005A7A0 -- 10, unchanged (1 cycle)

The target's stream has a shape no pass had stated: the `firstAnimation & 3`
value is coloured `s0`, **spilled between the two calls** (`sw s0,52(sp)`
after the first `jal`, before the second's arguments) and **reloaded into
`t0`** for the doubling, after which `s0` is reassigned as the loop byte
offset. So the target's alignment symbol is split by uopt at the second
call, the reload piece is what feeds the loop-setup block, and that is why
the two webs do not interfere there -- the 0x38 frame's single carrier word
is that spill. Ours colours the value `v1` and spills it *before* the first
call. The frame arithmetic (`round8(52 + 4N)`, target N <= 1) says the
original declares at most one local; one cycle of locals deletion (drop
`lastAnimation`: 27; drop the load-size local: +16 or +60; drop
`firstAnimation`: +8) shows each deleted local is recomputed rather than
CSE'd. Next: compare the `decision=split` records' spill placement between
p7-big's forced object (65 words, delta +4) and the target with
`lineage_census.py --web`; the force split the right web at the wrong point,
and the source lever is whatever moves the split to the second call.

## overlay15DrawScreenStars -- 9, unchanged (1 cycle)

Entry block only: the fade `lui/lwc1` sits above the synthesised zero-trip
guard where the target has it in the preheader; the other five words are
as1 reshuffling that block. Hypothesis: `const` is what let the p6-small
pointer view hoist, so a `const`-qualified read of the global itself
(`*(const f32 *)&g`) at the use site should hoist without a pointer symbol.
Five cells: the entry-block const form is byte-identical at 9; every in-loop
read, const or plain, stays in the loop (+4, 73 to 84). The hoist is a
property of the pointer *variable*, not the qualifier. Next: the pointer
view with a discarded second dereference (`*p;`) late in the loop body, to
raise the fade web's `nocs` above 6 before uopt deletes the read; the float
bank has no L109 probe but a pointer read is integer-side until the deref.

## func_80046BCC -- 16, unchanged (4 cycles)

Target: the loaded byte lives in caller-saved `v0` and never crosses the
call; the working char is `s0`; the arms read `addiu s2,s0,32; andi
s0,s2,0xff; move s2,s0`, and the copy `move s0,s2` is *not* propagated into
the test that follows it, so the working char is a separate variable whose
copy from `var_s2` is not a plain copy. The census confirms ours has one
symbol web (web 0, `s0`, 17 interferences) spanning the loop. Measured:
- three-statement arm (`var_s2 = w + 0x20; w = var_s2 & 0xFF; var_s2 = w`)
  is byte-identical at 16; on `var_s2` alone 38.
- a declared working copy in any width, with the copy inside the arms and
  the tests mixed or on the copy: 16 cells, all +4 to +24 bytes. The surplus
  word is `move s3,v0` at +0x58: both arms' `w = var_s2` hoist into the
  predecessor and common with the `andi`, which is the 2026-09-09 "web 32".
- anti-commoning spellings of the two copies (`var_s2` vs `var_v0 & 0xFF`,
  a `u8` cast, an extra mask): still +4 to +12.
- the pure two-variable form (no working copy, arms on `var_s2`): 42 at
  delta zero, and `u8 var_s2` -4.
Next: the target's `s0` is most likely uopt's own **split** of `var_s2`, not
a source variable -- a split piece is exactly a copy that is not propagated.
Ours colours `var_s2` whole (`s2`, totalsave 101). The lever is the split
gate `totalsave <= bestcost` on that web, which the type/copy axes never
touch; read its record with `lineage_census.py --web` and price what makes
its cost reach 101.

## func_80006534 -- 17, unchanged (4 cycles)

Target: `flags` in `v1`, `reference` (`>>22`) computed before `frameOffset`
(`>>16`) and coloured `a0`/`a1`. Ours: the three carriers take `a0/a1/a2`
and none reaches `v1`, which is free in the loop body. The census names the
occupant: web 77, a u32 single-member expression temp living only in
block 17, coloured `v1`, whose instruction is not in the output. It is
created after `reference` and before `count`, i.e. where the `if (1) { }`
region opener sits: the boundary makes a copy of `flags` for the store on
its far side, the copy is deleted, and its colour is spent. Measured:
- shift order and shift source (field with cast, field `u32`, `flags`):
  32 cells, 17 or worse; `reference` first is 19 in every typing.
- the opener's position: 17 only after `reference` or after the store; every
  other position keeps the copy (+4, 134 to 137); removed, 61.
- loop 1 with a declared cursor, loop 2 indexed: +4 to +44.
Next: the region exists only to rank `sprite` above loop 1's induction
pointer for `s1`; the original does that without a region. Raise `sprite`'s
save or lower the induction pointer's in loop 1 -- a second surviving
reference to `sprite` inside loop 2 at zero instructions, or a loop-1 shape
whose cursor is a declared pointer with a depth-0 definition -- and then
the copy temp, and with it `v1`, is free for `flags`.
