# Final campaign handoff — 2026-10-21

A two-hour coordinated campaign on the last two `NON_MATCHING` functions,
following [the 2026-10-20 handoff](FINAL_HANDOFF_2026-10-20.md). No function
was matched. The resolved totals are unchanged (see `gmake progress`):
resolved 934,928 / 943,640 whole-program text bytes (99.08%), 8,712 bytes in
the two `NON_MATCHING` functions, and 1,351 / 1,460 functions matched.

## What changed

`func_800517E0` (the command interpreter) gained two semantics-preserving
improvements to its canonical body, both still `NON_MATCHING`:

- **Role changes in 0x4C, 0x6A, 0x6D and 0x45.** These make 0x47, 0x6A and
  0x6B exact and 0x6D shape-exact.
- **Read order in 0x22 and 0x25.** The value is read first, then
  `opcode = cursor->command & 0xFF`. Both handlers become exact.
- **0x4C** reads the command byte last and makes its byte shifts in their
  carriers.
- **0x6F** gets mixed-type mask literals plus a merged 0x800000 constant web,
  and the constant 1's `U` moves from the loop test to 0x6F's `state` test.
  0x6F becomes exact.

Six handlers are now exact (0x22, 0x25,
0x47, 0x6A, 0x6B and 0x6F) and 0x6D is shape-exact. Aligned residual fell 141 -> 103 (naming
44 -> 27, really different 29 -> 9). The candidate is now one word long (7,236
bytes), so the positional masked count reads 841, against 448 before. Before, 0x60's surplus word was cancelled positionally by 0x6A's
missing word; with 0x6A exact, 0x60 is the only length defect left. Frame
0x1B0 (29 slots) and 245 relocations are exact.

`func_80054B3C` (the collision test) is unchanged at 17 stack-immediate words.
About 250 measured cells closed the remaining storage-origin classes: carrier
reuse, multi-path definitions, block scopes, declaration order and split
weights.

The detail, with every ruled-out cell family, is in the shards:
[interpreter](matching-triage-handoffs/func_800517E0.md) and
[collision](matching-triage-handoffs/func_80054B3C.md), in their 2026-10-21
sections.

## How the campaign ran

A Fable advisor read the residual, then four parallel Fable lanes took
disjoint territories, re-tasked as the base improved: the 0x79 structural
block, the argument-copy family, the naming-only handlers, and a fresh
collision attempt. Two further lanes followed, on 0x22/0x25 scheduling and on
the 0x6F constant webs. Each lane worked only on private copies of
`src/main/anim.c` and measured with `tools/fast_score.py`,
`tools/handler_diff.py` and `tools/shape_product.py`. The coordinator merged
the transferable edits, checked each one by def-before-use dataflow, and
landed them.

Two practices worth reusing:

- **Use a size-0 diagnostic base when one handler is a word long.** Here that
  meant declaring the 0x60 carrier `s8`. It is not adoptable on semantics, but
  it makes positional masked counts meaningful again everywhere else, at 116
  masked / residual 99 on the final tree.
- **Freeze lane candidates before checking them.** `unifdef` resolves the
  `SHAPE_` axes and a small substitution expands carrier macros. Only then
  does a def-before-use check on renamed locals mean anything.

## Where the interpreter stands

- **0x60** is the one length defect. A signed sub-word local living only in
  0x60 keeps three remote handlers (0x4B, 0x79, 0x7B) right, but emits a
  conversion pair. Every real-web carrier gives 0x60's shipped head and rotates
  the whole register ring.
- **0x79** needs a second counted use of its address web, and none of the
  natural forms emits nothing.
- Register-colour-only residues remain in 0x4C (two swapped shift/store pairs),
  0x4D, 0x6D (`value` f16 against f0), 0x7E, 0x40, 0x45 and the 0xB group.
  Carrier, order and mask-placement sweeps on these were flat. The naming
  lane's lead is a web the target keeps live across the 0x40-0x4D stretch.

On the size-0 diagnostic, 45 words differ: 20 in 0x79, 3 in 0x60, and the rest
in clusters of one to five.

## Next campaign

The tooling pass that followed this handoff, and the decision-level plan for the
remaining matching, are in [LANDING_CAMPAIGN_2026-10-21.md](LANDING_CAMPAIGN_2026-10-21.md).

## Restart

```sh
git status --short
.venv/bin/python tools/fast_score.py func_800517E0 src/main/anim.c --aligned
.venv/bin/python tools/handler_diff.py func_800517E0 src/main/anim.c
.venv/bin/python tools/fast_score.py func_80054B3C src/main/anim.c --aligned
gmake progress
```
