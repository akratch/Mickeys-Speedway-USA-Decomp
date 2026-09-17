# Last-mile lane s2-c (2026-09-16): a symbol web is kept by a self-referencing use, and a leaf join is a two-armed assignment

Four targets, 4,496 bytes, all at delta zero on arrival: `overlay2QueryNode`
(39 masked, 27 real), `func_80034448` (54), `func_80040B88` (72) and
`func_overlay_002_F0000C90_1857A88` (98). Twenty measured cycles were spent
(a cycle is one hypothesis taken to a batched measurement on a private
direct-`cc` harness that reproduced `score_symbol.py` on all four TUs before
use; every adopted number was re-taken with `tools/score_symbol.py` on the
tree). One target matched and is promoted (`func_80034448`, 856 bytes,
commit `5febf751`); the other three are unmoved, two with their mechanism
read off the allocator records. Evidence (every cell's source and object,
the decoded ladders, the freelist traces, side-by-side listings and the
scorer's own target words) is outside the tree under Git's common dir,
`lane-evidence/s2-c/`; nothing ROM-derived is tracked.

| symbol | bytes | before | after | cycles | state |
|---|---|---|---|---|---|
| `overlay2QueryNode` | 1012 | 39 | 39 | 6 | leaf join read; 41-shape priced to 32 by one accepted force |
| `func_80034448` | 856 | 54 | **0**, promoted `5febf751` | 10 | four edits, each priced alone |
| `func_80040B88` | 1208 | 72 | 72 | 2 | promotion draw at the result test not reached by spelling |
| `func_overlay_002_F0000C90_1857A88` | 1420 | 98 | 98 | 2 | frame claim confirmed; dead locals cost nothing on this TU |

## Mechanism 1: a single-def local becomes an expression temp unless a use sits inside a self-referencing assignment

`func_80034448`'s open question (s1-b) was why nine spellings of a declared
aligned-address local never produced a symbol web: the records showed the
same type-4 expression web each time, carried past `piRomLoadSection` in a
temp, with the local's home idle. Thirty-eight cells here draw the boundary:

- A single plain def is converted to a temp whatever else is done to it:
  block scope, `register`, a pointer type, a region opener (`if (1) { }`,
  `do { } while (0)`) before the def or between the calls, the def inside
  the call's argument list, DKR's `u32` with `(s32)` casts, and a second
  def *after* the call (`alignedAddress += 0x20`) all leave the temp in
  place at frame 0x58. A copy def elsewhere (`alignedAddress = (s32)texture`
  in the other branch) is propagated away first and does not count.
- The conversion stops when a use of the local sits inside a **direct
  assignment to one of the def's own operands**: `loadSize = alignedAddress
  + 0x20` or `assetSize = alignedAddress + 0x20` at the post-call use, or
  `loadSize = alignedAddress % 16` between two defs. That is the base's own
  `loadSize = f(loadSize)` stated generally. A comma expression that kills
  the operand around the call (`assetSize = (func(..alignedAddress..),
  uncompressedSize)`) does *not* count: the rule is the assignment
  statement, not the kill. Reading an operand through its address
  (`*(s32 *)&assetSize`) also keeps the symbol, at the price of changing
  that operand's reload into the argument register.
- With the symbol web kept there is no temp region, the local's own home
  is the spill slot, and the frame is `0x20 + 4 x declared` exactly. With
  the temp, the region is 8 bytes for one used slot in every form measured.

## Mechanism 2: the ring is decided by the local's declared type

The same function's 70 naming rows with the structure exact (q4) are one
ring position from +0x58 on. The `s32` aligned local is one draw ahead of
the target; DKR's `u32` local with `(s32)` casts on the modulo is not
(n6: three naming rows on the temp shape; s4/t1: 26 rows, all immediates,
on the same shape). The two-operand kill (`loadSize`) under the `s32`
spelling also left a phantom pair (webs 107/109, save 3, one block) on the
post-call block holding `t0` ahead of `loadSize` (save 2.33); under the
`u32` spelling the same kill leaves none and `loadSize` takes the target's
`t0`. `v8`, `v10` and `v12` are byte-identical at 0.

The match, in order of what each edit is worth alone: the `u32` type
(ring), the redundant `else` kept (the target's branch-over at +0x1AC is
its shape; dropping it is one word short, so s1-b's "byte-flat" is not),
a twelfth declared scalar between `assetIndex` and `assetOffset` (every
home from `assetOffset` down onto the target's), and the self-referencing
post-call assignment (the frame). 54 to 0, delta 0, frame 0x50, 214 of 214
words, no force; `gmake verify` from the C; `tools/gates.sh --promotion`
green.

## Mechanism 3: a zero at a loop join that survives is a two-armed assignment

`overlay2QueryNode`'s leaf join has the target zeroing `a0` before the
`D_60` branch and returning `a0` or `a1` from the arms. uopt folds a
single-def join zero into the return and deletes the counter copies (every
reset form 152 to 155 at minus four, refuting the shard's "+5"), and it does
not know an induction variable's exit value (`return remaining + 1` emits
the add). The zero survives only with two reaching defs at the return:
`if (D_60 != 0) count = leafResult; else count = 0; return count;`. That
removes the insertion pair and both immediate rows (branch offsets) and
leaves three colours, which one accepted force on the dead post-decrement
temp (`w69=c3`) closes in cascade: 41 to 32 at delta zero. The temp is
offered `v0` first in ours and does not take it in the target, and the
tail-call child expression and the `D_50` address show the same denial in
block 23. One `v0` holder our source does not declare is the reading; it is
not a plain function-wide local, because a web live across a call is not
offered `v0`. Not adopted: 41 is worse positionally than 39.

## Where a shard, the brief or the dispatch was wrong

- `overlay2QueryNode`'s shard: an explicit `count = 0` after the loop is
  not +5, it is 152 at minus four (every reset spelling); `return 0` and the
  reset are the same collapse.
- `func_80034448`'s shard (s1-b): "eleven declared homes plus one 4-byte
  temp" was the right count of slots and the wrong split; the temp region
  is never 4 bytes on this compiler, so the target's 0x50 with the aligned
  value at 0x20 is twelve declared homes and no temp, and "dropping the
  redundant else is byte-flat" is one word short (the target has the
  branch-over).
- The corrected L99 ("every declared local takes a slot, a dead `s32` costs
  8") holds on `textures_35024.c` and `particles.c` and does **not** hold
  on `src/overlays/o002/func_overlay_002_F0000C90_1857A88.c`, where one to
  three unused function-scope or block-scope `s32` declarations are
  byte-identical at frame 0x60. p5-ovl's shard was right for its TU. The
  rule is per TU and needs its condition named before it is cited again.
- The dispatch's sibling heuristic: the two o002 residuals share no
  register substitution (`s1->a0, t4->t3, t7->t6, v0->a0` against `v1->a1,
  a1->a0, t0->s1` and a `t2..t9` rotation) and predicted nothing, as the
  brief's o013 finding said it would not.
- The dispatch's per-target cap of six was exceeded on `func_80034448`
  (ten) because every batch closed a bucket (57 structural at 0x50, 70 with
  0 immediate and 0 structural, 26 with 0 naming, 0); the brief's "spend
  where it closes" wins and it matched. The other three targets paid for it
  (6, 2, 2).
- L149 as a first probe: on `func_80040B88` the promotion draw at the
  `result != 0xFF` test is real (ours draws one temp the target does not,
  the shard's MOVE_END on t7 read from the other side), but eight
  spellings of the comparison (casts to s32/u8/u16/u32, an unsigned or
  u8-typed constant, the negated equality) are byte-flat; an s16 cast is
  287. The generator is not the comparison's spelling.
- A private side-by-side tool reads the splat `.s` for the target text;
  the scorer's own target words (`nm_ranking.word_streams`) were dumped and
  disassembled before any register reading was trusted, and they agree.

## Ledger

Twenty measured cycles: `overlay2QueryNode` 6 (five spelling batches, one
force pricing), `func_80034448` 10, `func_80040B88` 2, o002 C90 2.
Matched and promoted: `func_80034448` (856 bytes, `5febf751`), verified
with `tools/score_symbol.py` at 0 masked and delta 0 before promotion, with
`gmake verify` printing the expected SHA1 from the C after it, and with
`tools/gates.sh --staged` and `--promotion` green; the ranking was
regenerated in the measuring form (`--refresh-stale`, one retired row) then
`--write-doc`, and `gmake scoreboard` rewrote README's block. Nothing
forced into the tree; the two forces reported on `overlay2QueryNode` are
priced diagnostics on retained objects.
