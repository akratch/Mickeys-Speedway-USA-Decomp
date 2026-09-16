# Last-mile lane lm-b: `src/overlays/o001/overlay_001_tail.c`

Single-TU lane, 2026-09-16. Three targets, one translation unit, ten measured
cycles of the eighteen budgeted. One match, promoted and verified; two
precise negatives with the decision variable named.

| symbol | before | after | cycles | state |
|---|---|---|---|---|
| `overlay1UpdateAimedTransient` | 14 masked / 996 B | **0, promoted** | 4 | `gmake verify` prints the expected SHA1 from the C |
| `overlay1UpdateRangeFlags` | 2 / 480 B | 2 | 1 | closed at the ugen ring, as recorded |
| `overlay1BendPathPoint` | 21 / 428 B | 21 retained; a 52-word basin found that reproduces the byte spill | 5 | see below |

One correction to the dispatch: `score_symbol.py`'s `artifact` column is
`raw - masked`, not a subset of `masked`. AimedTransient's 24/14/10 meant the
14 masked words were already artifact-free; the "real residual is closer to 4
rows" reading was wrong, and the 14 were all one mechanism.

## What closed `overlay1UpdateAimedTransient`

The residual was the prologue: the target dereferences the shared-world
pointer above ten register saves, the candidate below them. Five lanes had
closed this as "as1 orders every stack store before a register-indirect
load; ugen would have to emit the load first", and asked for a counterexample.

**The counterexample is a directive, not an emission order.** Read off
`cc -S`, ugen stamps a load through a uopt-created address-constant web with
`.noalias <reg>,$sp`. That is what tells as1 the load cannot alias the stack,
and with it the load carries no ordering edge from the saves and schedules
freely. The candidate's `u32 worldAddress` carrier kept the web but laundered
its provenance through an integer, so ugen emitted no directive, every save got
an edge to the load, and the tie the p6-mid closure priced was real -- for that
source shape only.

Measured both ways on the ugen listing (the `.s` round trip through `cc -c` is
not identity-clean for this TU, so the reading is the *relative* effect):

- overlay 14's `func_overlay_014_F0001540_1870E18` has a byte-exact prologue
  of the same shape in the tree. Deleting its `.noalias $16,$sp` drops its
  load below all ten saves -- the candidate's shape.
- Adding `.noalias $16,$sp` to this function's listing lifts `addiu s0` and the
  load into the target's slot.

The tree holds nine functions with this prologue shape (o014, o034, o046,
o063, `func_800517E0`, `func_8000D1B8`, `func_8000E5EC`, `gzip_inflate_dynamic`
and this one); any lane on one of them should start from the rule below.

**When uopt builds the web.** Mini-TU probes (twenty-odd cells, direct `cc -S`):

- two reads of a global in one basic block with a *store* between them build a
  block-local web (`.noalias` before, `.alias` after the next call); it never
  reaches another block;
- two reads separated by a call, in any block arrangement, build nothing;
- two reads in a procedure that **also contains a loop** build a hoisted,
  callee-saved web at the procedure entry that serves *every* read of that
  symbol. Bisecting o014 confirmed it: with its loop removed the web is gone;
  with its loop body emptied to one call the web is back. Read count alone
  (three reads, no loop) does nothing.

The target serves two reads (head, and the store site after two calls) from
the web and four later reloads from plain symbol loads, so the two groups must
be spelled through different symbols at the same address. This TU already
declares the alias (`D_1DA0_array`, line 1688); the adopted form reads `D_1DA0`
at the two web sites and `D_1DA0_array[0]` at the four reloads. The two
assignments produce identical text (measured), so which group carries which
name is a reconstruction choice. All six through one symbol folds everything
(50 words); the carrier form with the alias on the reloads is unchanged (14),
which is the measured proof that the carrier, not the reload spelling, was the
defect.

That took 14 to 4 at delta 0; the last four were the frame word and one home,
because dropping the carrier shrank the frame by eight. An unused local
declared before `savedState` restores both (the tu2 record had removed such an
`owner` pointer while the carrier was still present). Pointer, `f32` and `s32`
all work in that position; after `savedState` none does (L99).

**Promotion.** The first link missed the ROM by exactly four words, all `jal`
sites: the overlay 36 spawn and three same-module callees
(`overlay1ReadSelection`, `overlay1SolveAngleCandidates`,
`overlay1InitTimedState`). The module records them as SYMBOL relocations, so
they go through `*Reloc` placeholders that `overlay-syms` values at
`0xF0000000` -- trap 9 in the brief, live on a 0-masked score. Three resident
callees were added to this TU's POSTPROCESS rule; no bare `func_80`/`D_80`
name survives in the object's relocation surface, and the body has no
`switch`. Commit `0c2d8566`.

## `overlay1UpdateRangeFlags`: 2, closed at the ugen ring

Re-measured 2 masked, delta 0, both rows the recorded `t5 -> t4` pair. The
lineage census at ordinal 23 lists twelve coloured webs and none of them is
the contested pair, which is a ring temporary; the p23 landscape already said
so. The one axis not in the ~3,000 recorded cells was the callee's declared
return type (a trusted narrow return changes how many draws the right summand
costs): `s32`/`s16`/`u16` crossed with five right-summand spellings and both
angle widths, 30 cells, every size-preserving cell exactly 2. Inert.

Next hypothesis: none in C. Every reachable corner is two words on the same
five-draw, ascending-free-order constraint that five lanes and the trace have
derived independently. The lever, if one exists, is at ugen (a source form
whose fifth draw is neither on the sum nor at the chain head), and this TU's
flags cannot move without unmatching the other functions in it.

## `overlay1BendPathPoint`: 21 retained, and a new basin at 52

The second half of the function (+0x98 on) is byte-exact; all 21 rows are the
prologue and the index block. Two findings, both with the decision variable
named.

**The prologue cluster is uopt spilling the parameter web, not a scheduling
choice.** The target's `sb a2,0x3B(sp)` in the call's delay slot and
`lbu a2,0x3B(sp)` after it are a byte spill of the `u8` parameter to the low
byte of its own home word (`0x38`, big-endian), reloaded into the argument
register as one web. The plain form already reloads from `0x3B` but into `v1`
and with no store (104, delta -4): uopt treats the home as valid at entry. Any
identity self-definition before the call (`| 0`, `& -1`, `^ 0`, `+ 0`, `* 1`,
`<< 0`, `>> 0`, unary `+`, `(u8)(s32)` cast -- nine spellings) reproduces the
target's store to `0x3B` *and* the reload into `a2`, at delta 0, first
mismatch +0x10: **52 masked**, a basin no record had reached. Its residual is
three things: the `andi`/`sb` order around the `jal` (two structural rows),
the unchanged index-block 4-cycle, and a one-slot rotation of the whole
temporary ring (28 naming rows) because the identity op is computed into a
ring temp that as1 then deletes with the draw still spent (L149). The target
stores the argument register itself and draws nothing. `register` on the
parameter is inert (104), K&R-style definition is inert (104), and the
`volatile u8` hack's own store cannot reach `0x3B` because a local lives in the
local area. So the decision variable is: a source form under which uopt
spill-stores the *parameter web* across the call without a preceding
definition. The pair worth trying first is the identity def with its temp
consumed by something the target already has, or a wider parameter type in
the prototype with the byte conversion done by the callee (the K&R probe here
declared `u8`; declaring `int` and narrowing is untested).

**The index block is a colour-order fact with a named cause.** Census at
ordinal 35: `currentIndex` (w22, blocks 2-4 and 6), `previousIndex` (w25,
2-4), `count` (w99, 2-4), `nextIndex` (w44, 5-7). The target's order is
`currentIndex c2, previousIndex c3, count c4, previous c5`; ours is
`previousIndex c2, previous c3, currentIndex c4, count c5`. In the target
`nextIndex` reuses `a0` after `previousIndex` dies, and `currentIndex` is
copied from `index` in *both* arms of the `if` -- two definitions, which is
what puts it first. Merging `previousIndex`/`nextIndex` into one local is
measured worse on the aligned map (+5 naming rows), and every spelling that
writes `currentIndex = index` in both arms (copy first, copy last, before,
after, reversed condition, two-statement else) is hoisted by uopt into one
`move` at delta -4 (97-101). The pointer-sum spelling of `previous`/`next` is
inert. Next hypothesis: a pair of assignments uopt will not hoist -- different
IR, same word -- for example the else-arm copy reading a differently-typed
view of `index`, measured against the 52-basin base, not the hack base.

## What this lane learned about the TU

- The `D_1DA0` / `D_1DA0_array` pair is a real lever for every function in
  this TU that reads the world pointer more than once: it decides which reads
  share a callee-saved address web. Nothing else in the file moved when it was
  used (RangeFlags and BendPathPoint re-scored identical after the promotion).
- The TU's POSTPROCESS rule now names `func_8002A910`, `func_8002A8BC` and
  `func_8002A8C0`; the three same-module SYMBOL-record callees have `*Reloc`
  externs beside the promoted function. Any later promotion calling
  `overlay1ReadSelection`, `overlay1SolveAngleCandidates` or
  `overlay1InitTimedState` should reuse those declarations rather than the
  in-module names, and check `overlay_tables.py`'s record type for any other
  same-module call.
- `lineage_census.py` without `--proc` reads procedure 0 of this TU and its
  numbers look plausible; pass the ordinal (`uopt -v` order: RangeFlags 23,
  BendPathPoint 35).
- The direct-`cc` harness in this TU is 0.4 s per compile and agrees with
  `score_symbol.py` on all three targets; 100-cell lattices are minutes.
