# Last-mile lane w1-a (2026-09-16): a web's membership is the lever

Four resident targets, 2,856 bytes, all at delta zero on arrival. Two
matched and promoted (`func_8003F154`, 1,188 bytes, and `func_8005A7A0`,
424 bytes; 59.80% to 59.97%); two left with a measured negative each and
the mechanism named. 13 of the 20 budgeted cycles were spent (a cycle is
one batched hypothesis measured against `tools/score_symbol.py`'s
comparator; a private harness over `nm_ranking.process_item` with the
configured per-TU command reproduced the score of record on every TU
before use). Evidence is outside the tree under Git's common dir,
`lane-evidence/w1-a/`; nothing ROM-derived is tracked.

Both matches came from the same question, asked of the allocator records
rather than of the score: **which references belong to the web that took
the wrong register, and what puts a reference in or out of it.** On the
first target the answer was a literal's type; on the second, a second
symbol. Neither is a colour, a spelling of the failing statement, or a
statement order, and every lattice on both pages had held it fixed.

## Where the dispatch, a shard, or the brief was wrong

- The dispatch said the largest target alone would clear 60%. It is
  1,188 bytes against 1,922 needed; both matches together (1,612) leave
  the tree 310 bytes short at 59.97%.
- The dispatch's and the shard's named step for `func_8003F154` -- a
  second `0.0f` reference confined to the comparison's block that emits
  nothing -- was one route, not the requirement. The comparison's piece
  had nothing to save only because the web was split, and it was split
  only because the velocity head's three `0.0f` stores stretched it across
  six calls. Removing those references from the web by literal type is a
  zero-instruction edit no lattice had varied.
- lm-c's and nx-c's readings of `func_8005A7A0` ("the target splits the
  alignment web at the second call", "the ROM's first piece spans call 1
  and stores at the remainder's entry") read one symbol's split where the
  ROM has two symbols. The follow-on frame arithmetic (`round8(52 + 4N)`
  with N as declared locals; "six fewer memory-class locals") was an
  artefact of the same reading: the same six declarations give 0x38 once
  the carrier is right.
- nx-b's and the dispatch's named step for `func_80006534` -- a
  self-defining read of `flags` after the store, the form that moved
  `overlay34CreateRecord` -- is byte-inert here in every spelling and
  position (nine cells at 17). L135 holds on this web; o034's mechanism
  does not transfer.
- The brief says to `gmake extract` on a resident match; the dispatch's
  recipe omitted it. The brief was followed (both promotions extracted
  before rebuilding), and the tree links and verifies from the C.
- The brief says not to refresh the ranking unless `check-docs` fails
  without it. A promotion retires a row, so it always fails; both
  promotions regenerated in the measuring form and then `--write-doc`,
  as nx-b recorded. The brief's wording should say so.

## Mechanism 1: a constant's web is its spelling, and a split piece saves references minus one

`func_8003F154`'s residual was the comparison zero after the sqrt call:
the ROM materialises it into `scale`'s pool colour, ours drew a ring temp,
and the tail's ring phase followed. Seven lanes had priced it as "a web
with one occurrence has nothing to save" and looked for a second
occurrence.

The instrumented uopt's `webblocks`, `seedcand` and `grow` rows say what
the web is. The `0.0f` constant is one web wherever it is spelled `0.0f`:
members in the disableTransform arm (block 39), the comparison (42) and
the velocity head (2), created in that order; the parent spans blocks 2
to 42 (nocs 11, totalsave 5, cost 9) and is split. The block-39 seed is
carved first (three references, totalsave 2, coloured c25); the block-42
seed next, one reference, never a candidate; the block-2 seed then grows
forward through the head and the three `mathRnd` call blocks to a
negative save, which is why the head materialises three separate ring
zeros on both sides.

So the comparison's piece is single because the head's stores keep the
web reaching back to the entry. Spelling the three velocity-head stores
as the integer literal `0` -- a different IR constant (L151), the lever
lm-a had already used on the emission-direction fill -- makes the `0.0f`
web two blocks: `nocs 2, totalsave 2, cost 0, decision=color, c25`. The
integer-zero constant becomes its own web (members in blocks 19 and 2),
split, its block-19 piece coloured c24 and its block-2 remainder growing
negative as before. c25 is free in the comparison's block only when
`scale` does not live in the sqrt-argument block, which the inlined
x-first sum buys; the direct Z accumulate is then the tail's ring phase,
exactly as nx-c's diagnostic had predicted.

One batch, eight cells, all delta 0: inlined sum 17; with the head `0`,
`0.0` or `(f32) 0` 3 each; the first two with the direct Z accumulate 0;
the named sum with the head `0` 18 (lm-a's cycle-4 number, explained:
`scale` in block 41 forbids c25 and the whole web takes another colour);
the inlined sum with the direct tail and a `0.0f` head 18. Promoted as
commit `dff3b727`: 297 of 297 words, frame 0x58, sixteen relocations,
`gmake verify` from the C, `tools/gates.sh --promotion` green.

The general form, for the index: **a split piece's save is its references
minus one per block, and which references a constant web holds is decided
by the literal's spelling at every other site, not at the failing one.**
Before looking for a second occurrence at the site, count the occurrences
elsewhere that are stretching the web.

## Mechanism 2: a spill at a call boundary can be a second symbol's definition

`func_8005A7A0`'s ten words were the frame (0x50 against 0x38), the colour
of `firstAnimation & 3` (ours v1, the ROM s0) and a spill placed around
the wrong call. Read off the ROM: the value is held in s0 through the
second `piRomLoadSection` call, stored to the frame's one home right
before `func_8002B314`, and read back only by the doubling in the loop
setup, into a temporary. Three lanes had read that as one web split by
uopt and asked what makes `split()` store at the boundary.

Nothing does. The store is the definition of a second symbol assigned
from the alignment between the calls, and the reload is that symbol's
use. `alignment`'s own web then ends at that copy, no longer shares the
loop-setup block with the loop's byte offset, and takes s0 at cost 0 --
the ROM's colour, on the first cell that tried it, with no force. Which
symbol carries the value decides the last five words, and the records say
why each does what it does:

- `loadSize` (5 at delta 0): web 36, nocs 3, totalsave 6, cost 4,
  coloured a3 whole; reloads into a3 and leaves three unused homes above
  the used one (frame 0x40).
- `inputOffset` (24): the callee-saved s1 absorbs it and no spill is
  emitted. `lastAnimation` (6): coloured a2. A fresh local (55, +4): the
  home block returns.
- `firstAnimation` (27): its web already spans the second call's argument
  block, so a0-a3 are denied (L142) and it takes c7 = t0 at cost 4 -- the
  ROM's reload register, and the frame is 0x38 -- but written as the plain
  copy `firstAnimation = alignment`, the two chains of `alignment` stop
  being renamed into separate symbols (one web 32 over blocks 3 to 19 at
  save 18.5, decided after `inputOffset` at 20.67, so s0 and s1
  exchange).
- The self-defining spelling `firstAnimation = firstAnimation & 3` keeps
  the chains apart: 2, and the two rows are the carrier's home one slot
  below the ROM's.
- Homes descend from the frame top in declaration order (L99). Declaring
  `firstAnimation` first: 0. Any local declared ahead of it: 2. The order
  of the rest is inert (five orders, all 0).

Four batches, 27 cells. Promoted as commit `148347d9`: 106 of 106 words,
frame 0x38, seven slots, ten relocation identities, `gmake verify` from
the C, gates green.

Two facts from this are reusable. **A caller-saved reload into a
temporary at a call boundary is a symbol whose two references straddle
the call and whose cheap callee-saved colours are all forbidden where it
is read**, not a split of the value it copies. And **a plain copy from a
symbol with two disjoint chains can stop uopt renaming those chains
apart, where a self-defining spelling of the same value keeps them
separate** -- the records show it as one symbol web against two, and the
score shows it as a callee-saved permutation.

## `func_80006534` -- 17, unchanged, the sinking gate located

Five cycles, 39 cells, none below 17. The ROM holds the packed word in v1
and computes both fields from it at the top; ours has a CSE temp for the
three `record->unk8` reads which takes v1 first. Reading the fields from
`flags` removes the temp and puts the load in v1, but uopt then forwards
`(flags >> k) & 0x3F` into arms 2 and 3 as ring-temp expressions (+4),
which nx-b had measured and which every cell here confirms:

- Two-definition spellings (`reference = flags >> 22; reference &= 0x3F`)
  in six orders and placements are byte-identical to the one-expression
  form (136, +4): uopt normalises the pair before the pass that sinks.
- A region around the chain, around the captures, or around the count and
  the chain does not block it (143, +4, with the s1/s2 exchange back).
  Narrow field types add masks (+44).
- Self-defining reads of `flags` after the store, before the store, before
  the chain, in or/and/xor/shift-by-zero spellings: all 17; two of them
  23.
- The mixed forms locate the gate. `frameOffset` from the temp and
  `reference` from `flags` is byte-identical to the base: with the temp
  present, a `flags` read is folded into it. `frameOffset` from `flags`
  and `reference` from the temp sinks (134, +4). So the forwarding fires
  exactly when the operand is a symbol defined by a load with no CSE temp
  behind it, and `register`, `s32` with casts, `u32` fields, a later
  placement of the captures, and a mask that clips `flags` after the
  store are all inert on that (136-144, +4 to +12).
- No `flags` local at all, every read through `record->unk8` and the
  store typed `u32` through a cast: a second load is emitted (+4), so the
  reload nx-b saw is not a type-based alias decision on the store.

Next: the decision variable is uopt's expression-forwarding gate on a
load-defined symbol. It is not reachable by any spelling of the captures,
the tests, or their placement, and the temp form's colour ranking is
closed by the earlier 155-force landscape. The next lane should read the
ROM's shape as "no temp, forwarding refused" and look for what makes the
forwarding refuse: a form in which `reference` and `frameOffset` are
used, at zero width, on the path uopt considers them dead on (arm 1 or
after the chain), or an instrumented reading of the pass itself.

## `func_80046BCC` -- 16, unchanged, the second symbol and its value number

Three cycles, 17 cells, none below 16. The records say ours has one symbol
web for the loaded byte and the working char (web 0, blocks 0 to 33,
totalsave 232, s0); the ROM keeps the loaded byte caller-saved and dead
before the call, so its source has two symbols. Every second symbol
measured costs a word, and the object says why: uopt value-numbers the
arm-head copy `w = var_s2` with `var_s2 = c & 0xFF` and gives the masked
value three destinations, a temp with a copy into `var_s2` (+4). The base
avoids that only because `w` and `c` are one symbol, so the mask's
operand is the copy's target.

- Moving the loop-bottom load into the loop condition or into its own
  region: 16, inert.
- A separate loaded-byte symbol, `s32` or `u8`: 91, +4, the temp above.
- `u8` working char with the three-statement fold and no explicit masks,
  two- and three-variable: 79 to 94, +4 to +16 -- IDO emits the
  truncations as instructions.
- `u8 var_s2` (the lazy-normalisation reading of the ROM's fold: sum
  stored unnormalised, mask on the read into the working char, plain copy
  back): 88 to 94, +4 to +20 -- the masks are emitted eagerly.

Next: the decision variable is the value number of the arm-head copy.
The ROM's copy reads `var_s2`'s register, so in its source the copy's
value is not the mask's value: either `var_s2` has a second reaching
definition at the arm heads, or the loaded byte is not available there.
A form where the loaded symbol is redefined between the mask and the arms
(at zero width) is the untested cell; a two-symbol form on the current
shape without it is closed by the 192-form lattice, lm-c's 16 cells and
the five here.

## Ledger

Thirteen measured cycles of 20: f154 1, 6534 5, a7a0 4, 46BCC 3, plus
instrument reads on every target (lineage census, frame census, ugen
listings) that were not counted as cycles. Matched and promoted: f154
(1,188 bytes, commit `dff3b727`) and a7a0 (424, commit `148347d9`);
59.80% to 59.97%. Unmoved: 6534 at 17, 46BCC at 16, each with a
mechanism named from the records rather than a closure argued from the
score. Every claimed match was verified with `tools/score_symbol.py` at 0
masked and delta 0 before promotion and with `gmake verify` printing the
expected SHA1 from the C after it; `tools/gates.sh --promotion` was green
on both. Nothing was forced.
