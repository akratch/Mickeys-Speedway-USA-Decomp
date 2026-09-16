<!-- plateau-handoff:overlay1ResolvePathPoint:start -->
### `overlay1ResolvePathPoint` plateau handoff

- source: `src/overlays/o001/overlay_001_end.c`
- score: 152/152 words
- frame: 0x78
- relocations: 22
- first mismatch: none
- summary: MATCHED. This header claimed an open residual until 2026-09-16, when tools/check_shard_metrics.py reconciled every shard against the ranking; the function had already left the queue. The claim it carried was: One word: the else-branch base reads the raw call-result web (v0) where the target reads the saved copy (s4); the doubled product is closed


#### 2026-09-09, lane win-b: 8 -> 3 words, three named mechanisms

Measured against the full-TU object with a direct `tools/ido/cc` compile
(byte-identical in `.text` to the asm-processor NON_MATCHING object at this
TU's real flags -- note `overlay_001_end.c` does **not** carry
`-Wab,-r4300_mul`, unlike `overlay_001_tail.c`). Three independent residual
groups, closed in order:

1. **The m2c `register u32 groupAddress` carrier was the two relocation-order
   words at +0x090/+0x094.** Replacing `groupAddress = (u32)&D_1D88;
   *(s32 *)groupAddress = 0x3F;` and `(*(s32 *)groupAddress)--;` with plain
   `D_1D88 = 0x3F;` and `D_1D88--;` makes the twice-spelled global materialise
   its own address into the pool colour and interleaves the `%hi` of D_1D88
   between the `lui $at` and the `sw` exactly as the target does. Two words.
   Physical line grouping is **inert** here: all 32 groupings of the six head
   statements produce a byte-identical object, so the fx.c `%lo`-ordering
   lever does not apply to this block.
2. **The declaration census moved `result` when the carrier was dropped.**
   Losing one 4-byte local put `result` at 0x64 instead of 0x60 while the frame
   stayed 0x78, costing the `addiu $s3, $sp, ...` and the `lh ..., ...($sp)`.
   All 720 orders of the six locals were scored: the ones that put `result`
   fifth are exact, and `index, record, point, scanIndex, result, product` is
   the adopted order.
3. **The induction pointer's first textual encounter decides which `record` web
   the loop-entry pointer draws from.** With `point = &record->x[2];` before
   the `do`/`while` and `point++` inside, uopt feeds the loop-entry pointer
   from the raw call-result web (`$v0`) and as1 schedules it two rows early.
   Writing the loop as `do { point = &record->x[scanIndex]; ... scanIndex++; }
   while (scanIndex < record->count);` -- semantically identical, and strength
   reduction rebuilds the same `addiu $s0, $s0, 2` induction step -- moves the
   first encounter inside the loop, puts the loop-entry pointer on the saved
   web (`$s4`), and restores the +0x148/+0x14C/+0x150 ordering. Four words
   became one.

Residual, both `allocation-mismatch`:

- **+0x124**, the `else`-branch `point`: target `addiu $s0, $s4, 0x4`, we emit
  `addiu $s0, $v0, 0x4`. uopt copy-propagates the raw call-result web into the
  `$36` block because nothing redefines `$2` on the path from the call to it,
  while the in-loop copy is forced onto `$s4` by the call inside the loop.
  Exhausted: six spellings of the base (`&record->x[2]`, `record->x + 2`,
  `&record->x[index]`, `(s16 *)((u8 *)record + 4)`, `&record->x[0] + 2`,
  `&record[0].x[2]`) crossed with six spellings of the pre-`if` assignment, an
  `index = record->count` carrier, an empty `else`, an `else` that re-uses
  `scanIndex`, a `record->count < 2` inversion, and a second
  `Overlay1PoolRecord *` alias -- all 720 declaration orders each. Floor 3.
- **+0x1C8/+0x1CC**, the doubled product: target `addu $v1, $v0, $v0;
  sltiu $at, $v1, 0x11`, we emit `addu $v0, $v0, $v0`. The `cc -S` listing
  shows why the space is only two-valued: `product = product + product;`
  assigns back to the same ugen expression register and emits `addu $2,$2,$2`,
  while every form that makes the doubled value a distinct value
  (`if ((u32)(product + product) >= ...)`, `product * 2`, `product << 1`, a
  second declared local, `u32 product`) demotes it to a **ring** temp
  (`addu $9,$2,$2`) and shifts every later ring temp by one, costing 11 more
  words. `$v1` is ugen's second statement-result register, taken only when
  `$2` still holds a live statement value; no source form reached it.

Next lever for both sites is a uopt web-split observation (CDX/`p1dec`), not
another source spelling: the question is what keeps the raw call-result web
from reaching the `$36` block.

#### 2026-09-10, lane p1-perm: 3 -> 1 word, the doubled product is closed

Measured with a direct `tools/ido/cc` full-TU compile, confirmed byte-identical
in `.text` to `build_non_matching/src/overlays/o001/overlay_001_end.c.o` and
cross-checked against `tools/wb_compare.sh --summary-json`, which now reports
`differing_words: 1`, `matched_words: 151`, `register_mismatches: 1`,
`opcode_mismatches: 0`, `instruction_delta: 0`. About 190 scored candidates a
second at eight workers.

**Scoring note that changed the reading of this function.** Six of the nine raw
differing rows are splat rendering an overlay-internal address as a bare
`lui`/`%lo` literal with no relocation where the C build emits HI16/LO16
against the extern. They link to the same bytes and the workbench normalises
them; a naive word diff counts them and reports nine. Any comparison on this
overlay must tolerate exactly that pairing -- one side relocated, the other a
literal -- and nothing else.

**The doubled product (+0x1C8/+0x1CC) is closed.** The target computes the
product into ugen's first statement register and the doubling into its second
(`mflo $v0; addu $v1, $v0, $v0; sltiu $at, $v1, 17`). Spelling the product
expression **twice** and letting uopt fold the two into one multiply reaches
that exactly:

    product = ((result[0] - point[0]) * (result[0] - point[32])) +
              ((result[0] - point[0]) * (result[0] - point[32]));

The earlier note that "no source form reached `$v1`" was measuring the wrong
axis. `product = product + product` writes back in place because the
destination is already `product`'s pool colour `$v0`; every form that makes the
doubled value a *fresh* value (`product * 2`, `product << 1`, an inline
`(product + product)` in the test, a seventh declared local at any of seven
positions and seven types) demotes it to a ugen ring temp. A repeated
*subexpression*, folded by CSE, is neither: it leaves the product live as a
statement value, so ugen takes its second statement register for the sum. This
is a new form of the "repeat the read" lever -- repeated for its **statement
register**, not for a copy or a ring slot.

A seventh declared local is separately excluded: the local block is already at
its 8-byte step, so any seventh scalar grows the frame from 0x78 to 0x80 and
costs 24 words in all 49 position x type cells.

**Remaining: one word at +0x124**, `addiu $s0, $s4, 4` against
`addiu $s0, $v0, 4`. This is the pre-branch materialisation of the else
branch's `point`. `cc -S` reads the cause directly: in the `$36` block ugen
emits `addu $16, $2, 4` while the loop's own setup two blocks earlier emits
`addu $16, $20, 4` from the same source expression. `record` has one definition
and uopt splits it at the call into a raw `$v0` web and a saved `$s4` copy
(`move $20, $2` in the branch-delay slot); in the target the raw web's last use
is the `lbu $v1, 192($v0)` count read and the else use belongs to the saved
web, while here the else use is still in the raw web. as1 then hoists the whole
`$36` block above the branch in both.

Newly eliminated this pass, each measured against the full-TU object and each
flat at one word:

- all 720 declaration orders, re-run because the product edit renumbers webs;
- 76 tested dead expressions (`if (E);`) over 19 expressions x four placements
  (before the `if`, first and last in the else arm, after the decrement).
  **Every one emits nothing and changes nothing** -- on this function the
  single-mention fold is total, which is the opposite of its behaviour on
  `levelFreeAll`, where the same construct reserved a pool colour;
- 48 else-branch cells: six base spellings x `index`/literal x four arm
  orderings;
- 13 structural forms of the if/else: empty else, `count < 2` inversion, the
  else body reached by `goto` and placed textually *after* the call (so text
  order is not what assigns the web), a shared `point` before the `if`, an
  alias local, and the same-expression else;
- six null-test spellings, `register` on `record`, seven pre-`if` base
  spellings, five orders of the three statements before the `if`;
- 54 base-decay cells (`&record->x[i]` against `record->x + i`) crossed over
  the loop, the if tail and the else arm, to make the two uses share an
  address CSE;
- 20 physical line groupings of the head statements crossed with the else arm.

Next lever: not a source spelling. The question is uopt's live-range split
rule -- what puts the `$36` use on the raw web here and on the saved web in the
target -- and it wants a `p1dec`/CDX web trace or a print in uopt's splitter,
not another lattice.

#### 2026-09-10, lane p1-perm (continued): the last word survives two more classes

After the doubled product closed, the remaining word at +0x124 was attacked
with the two levers that closed `func_80049000` and
`func_overlay_029_F00010C4_187E374` on the same day -- shortening the *other*
web's live range by reading a field back, and moving a statement into a
neighbouring statement's line group so a web stays live across the site.
Neither reaches it, and both classes are now measured:

- 45 cells of callee prototype (`Overlay1PoolRecord *`, `void *`, `s32`,
  `u32`, no prototype) x three orders of the three statements before the `if`
  x three else-arm shapes. Flat.
- Reusing `point` as the raw call-result carrier so `record` is only ever the
  saved copy: 14 words, first mismatch +0xE0.
- A `record = &record[0];` self-assignment between the null test and the
  decrement: folded, flat.
- Further line joins: the null-test arm on one line, the three head statements
  on one line, and the whole else arm on the `} else { ... }` line. Flat.

The mechanism is now named precisely, from `cc -S`: `record` has one
definition, uopt splits it at the call into a raw `$v0` web and a saved `$s4`
copy (`move $20, $2` in the branch-delay slot), and in the `$36` block ugen
emits `addu $16, $2, 4` while the loop's own setup emits `addu $16, $20, 4`
from the same source expression. The target's raw web ends at the
`lbu $v1, 192($v0)` count read. Nothing in C moves that boundary: text order
does not (the `goto` form that places the else body after the call is flat),
liveness does not (every tested dead expression on this function folds and
emits nothing), and the operand is not a colour choice, so the globalcolor
lever does not apply either. The next step is a uopt web/split trace.


#### c2-o001: the one word re-confirmed at the current base, and it is the copy question

L79 already records this function as a selection that ties on aftercycles and
separates on `besttime`, which is a key above `lineno` and therefore has no
source lever. That ruling still holds at this base, and the residual now has a
name: the single word is the operand of the dead default cursor assignment,
where the target reads the callee-saved copy of the record pointer and the
candidate reads the call's own return register. The record's count is read
from the return register on both sides at the immediately preceding
instruction, so the target ends the return register's live range one
instruction earlier than the candidate does.

Seventeen further forms were measured this lane and every one is byte-flat at
one word: the cursor index spelled from the index variable, as pointer
arithmetic, and packed onto one line with its neighbours; the three statements
of that group in six orders; the guard written with the constant on the left,
as a truth test and against NULL; the error block with its two output stores
moved ahead of the call (10 words, worse); a self-assignment of the record
pointer; and the count hoisted into `product` or `scanIndex` before the cursor
assignment. One thing worth knowing before spending a variant here: an
unused extra local costs twelve words on this function, because the declared
locals size the local block, so no lever that adds a declaration is free.

This is the same copy-propagation decision `overlay1FindPreviousUsable` shows
at twelve words in the same overlay. If that one is ever answered, come back
here first.


#### c2-o001 diagnostic: 140 inert placements, all flat at one word

The ADR 0017 inert-read family that is worth 18 words on
`overlay1AdvanceGauge` and 7 on `overlay1ConsumeNearbyPending` was run here:
140 placements over seven slots and 20 expressions, covering the record
pointer, the cursor, both indices, the two output pointers, the input
coordinates and the two imports, several of them doubled. Every one is flat at
one word.

Read with `overlay1FindPreviousUsable`, where the same family is equally inert
over 220 placements, this separates the overlay's residual classes cleanly:
the family moves the pool/ring population functions a long way and the
copy-propagation ones not at all. That makes it a cheap classifier to run
before choosing a lever, and it says this word is not bought with web
population.
#### Closed 2026-09-10, lane o7-tight: the last word was the assignment's block, not its spelling

Matched; `gmake verify` prints the expected SHA1 with the function promoted.

The residual was `addiu $s0, $v0, 4` against the target's `addiu $s0, $s4, 4`,
and every prior pass read it as a colouring or copy-propagation decision about
`record`. It is neither. It is **which basic block the assignment lives in**.

`point = &record->x[2]` was written before the `if`, so it shares a block with
the call that defines `record`; uopt forwards the call's own return register
into every use in that block, and the base reads `$v0`. Deleting that
assignment *and* the two per-arm assignments, and writing one
`point = &record->x[scanIndex];` after the if/else, moves the only assignment
into the join block, where the raw return web has ended and the base is the
saved copy `$s4`. `scanIndex` already carries the right value on both paths
(`2` from the `else`, the loop's index from the `then`), so nothing else moves:
the object is byte-identical to the target at 152 of 152 words, and the other
five functions in the translation unit are unchanged.

Why the earlier sweeps could not see it: they all kept an assignment in the
pre-`if` position or in an arm, and varied its *spelling*. The whole 48-cell
else-branch lattice, the 720 declaration orders, the 76 dead expressions and
the 140 inert placements are measurements of a space that never contained the
answer, because the decision variable is the assignment's block membership and
every cell in those lattices held it fixed. This is the concrete instance of
trap 7 in the plateau brief -- a flat spelling lattice is not evidence that a
residual is unreachable.

- disposition: matched 2026-09-10; the mechanism is recorded at the point of
  use in `src/overlays/o001/overlay_001_end.c`.
<!-- plateau-handoff:overlay1ResolvePathPoint:end -->
