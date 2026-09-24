<!-- plateau-handoff:ProcessRelocationEntry:start -->
### `ProcessRelocationEntry` plateau handoff

- source: `src/main/runlink.c`
- score: 126 differing words
- frame: 0x48
- relocations: 25
- first mismatch: +0x0
- summary: Pair 4 (+0x210, split-not-copy) is the move at line 423. hiAddr size 0/137, inlined hi 127, u32 carrier 129, text-base hoist 127. Stall: a1 forbidden, s1 stays.

Summary before this remeasure: Remeasured 2026-09-23: 126 masked at size delta +4, frame 0x48 against 0x40, relocations 25 of 25; a1/a3 after the call stay structural.

First structured handoff. The plateau commit of 2026-08-25 recorded prose
only, in the source: -O2 -mips2 stays 0x4 long with 126 differing words from
+0x0; splitting the pointer lifetime removes s1 and yields the a3 caller
spill but grows the frame; type, pool-order, volatile-home, result-reuse and
register-hint variants miss a1/a3 after the call. This remeasure (preflight,
resolved through the guarded func_80031A30 fallback) reproduces that count.

Lane w4-rl, 2026-09-24, stock -O2 -mips2. Cycle 0 on the retained body:
insertion_pairs reads four pairs, shadow 6, aligned residual after shadow
120. The function label is the open pair from +0x210 to the end (shadow 4),
a candidate-only move owned on line basis by line 423
`PatchInstruction(patchLocation, resolvedAddr, mode)` via move_to_dest.
The other split-not-copy pair is the move at line 403. Pair 1 is a hoist of
stack stores (candidate +0xC prologue, target +0x20). Pair 2 is the target
stack store at the ResolveRelocAddress call (line 381) against an extra alu
on the overlayNumber line (384). Identity-gated instrumented uopt text
matches stock. Proc 3. resolvedAddr (sym 33, nocs 6, save 1.5) colours a3 at
cost 2.30; a1 is absent from its cost list (forbidden0 0x5c020000).
patchLocation (sym 16, nocs 7) is offered only callee-saves and takes s1 at
7.15, which is the frame 0x48 against 0x40. nextPatchLocation (sym 62)
decision=split. The line 423 word is `move a1, a3`.

Spellings not in the August list, all stock:

- HI16 address copied into hiAddr before the two PatchInstruction calls.
  Size delta 0, 137 masked, frame still 0x48. The early addiu of the HI16
  constant disappeared and beql became beq; the default-path move remained.
  hiAddr (save 2.0) took a1 first and resolvedAddr stayed a3, still forbidden
  from a1. Not a better aligned residual.
- Inlining the hi immediate on that shape: back to size +4, 127 masked.
- patch address kept as a u32 across the call and recast to the pointer
  after it: 129 masked, size +4, frame still 0x48. Does not demote s1.
- D_800D2DAC loaded into a local before the op test, so the join might
  start at the next-record load: 127 masked, size +4. The target-only const
  moved from +0xBC to +0x104. Not bnel, not the line 423 deletion.

Stall: three later attempts after the size-0 identity neither beat 126
masked / aligned 120 nor put resolvedAddr in a1 nor removed s1. The retained
body is the original. Do not repeat hiAddr, inlined-hi, u32 patchSaved, or
the text-base hoist.
<!-- plateau-handoff:ProcessRelocationEntry:end -->
