<!-- plateau-handoff:func_overlay_001_F0002B4C_184EF2C:start -->
### `func_overlay_001_F0002B4C_184EF2C` plateau handoff

- source: `src/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c`
- score: 0/451 words, promoted
- frame: 0xB0
- relocations: 68
- first mismatch: none
- summary: Matched. for (sum = 0, k = 0) keeps the zero with the induction init, so the func_80005820 delay slot is the argument copy. Reversed comma is 2 words. The loopunroll override was not load-bearing.

## 2026-10-02 (lane w6-o001b): matched, 6 to 0

Deleting `-Wo,-loopunroll,0` left the same 6 masked words at size delta 0, first mismatch +0x60C, so the six head spellings already called flat were not repeated. Nesting `sum = 0` in the rank test, wrapping the call in a do/while or if region, and folding the zero into the pointer expression were byte-identical to that baseline or worse (the region forms scored 9). The adopted spelling deletes the separate `sum = 0` and writes `for (sum = 0, k = 0; k < state->lap; k++)`. Promotion proof: 451 words, frame 0xB0, 68 relocations, identity static. Resident calls are renamed on this object's POSTPROCESS.

Summary before this remeasure: Listing rewrite in while(i--) shape with per-site identities: 433 to 186 masked, size -16 to 0, frame exact; temp ring offset from +0xE4 open.

Summary before this remeasure: Candidate is four instructions shorter and unresolved relocation identities prevent a linked trial; schedule allocation remains the next lever.

## 2026-10-02 (lane `o-ovl7`): listing rewrite, 433 -> 186, size -16 -> 0

The inherited candidate was an m2c shape with the wrong semantics in places
(a continue/goto anchor search, a pointer-cast mode word, three resident
calls through one cast). Rewritten from the target listing and the module's
relocation records, measured with `tools/fast_score.py` and
`tools/shape_product.py`:

  - Every loop is `i = n; while (i--)`: the target's `or v0,s7,zero` beside
    each `bnez s7; addiu s7,-1` is the post-decrement value. Identities: the
    mode is the overlay BSS word +4 (G_o1_83e4), the clear test before the
    gap loop is BSS +0, the order bytes and weights are resident tables, the
    spacing mode is a resident byte, the objects come from func_80005750 and
    func_80005820, the distance is sqrtf. First full rewrite: 388 at -28.
  - The mode dispatch as a switch with an explicit `case 0` (the target's
    leading `beql v0,zero`): 316 at -20. An if-chain measured 391, a switch
    without case 0 383.
  - The pair loop's equality test written `if (i != j) {...} else {zeros}`:
    316 -> 316 alone, but needed for the next step.
  - One state local for the outer pair state and the bottom loop's state
    (its home at +0x7C, re-read each inner iteration) and another for the
    inner and rank-loop states: 316 -> 195 and size delta 0.
  - The rank loops read `object = list[order[i]]` before the state (the
    target holds the object in a0): 195 -> 186.

Measured flat (no cell below 186): the flag store spellings around +0xC0,
the copy-loop spellings (eight forms, including a u8 mask, s8 cast, and
`for`), early-return versus wrapped-block structure, the sort's swap order
and flag carrier, and the bottom loop's object/state split, sum-init order
and loop form. register_census reads the residual as 11 windows with 51%
coherence, so it is not one ring phase: the next lever is the decision
records (the sort's a0/a1 and f0/f2 colours, the s0/s1/s2 cycle in the gap
loop), not another spelling lattice.

## 2026-10-02 (lane `p-ovl8`): per-region locals, 186 -> 6

The 11 register windows were not independent colour decisions: most were
webs merged across regions by reusing one local (L131, checklist 14).
Measured with `tools/shape_product.py`, each step a product cell:

  - The anchor, rank and leader regions read their object into `other`
    (not the pair loop's `object`, which lives across sqrtf in s5): 243
    cells over the object/state variable of each region, 186 -> 64. The
    early ring offset at +0xE4 closed with it; it was this merge, not the
    copy loop (copy-loop and flag-store spellings, 60 cells, were flat).
  - Sort reads `swap = w[i]` before the compare (f0/f2 and a0/a1 land),
    and the leader reads `otherState = other->state` once (s4, as the
    target): 24 cells, 64 -> 27.
  - The bottom loop's previous-rank state is its own local `prev` (a0;
    `otherState` is live across calls in s4) and its split index its own
    `k` (s0; `j` and `changed` merge with the pair and sort webs). Each new
    declared local moves the +0x54 spill cell down 4 bytes (21-position
    sweep, flat at 32), so `dz` and `dx` are written inline at the sqrtf
    call to keep the count: 27 -> 6. Inlining `delta` or `step` instead
    changes the size.

Open (6 words at +0x60C): uopt emits the bottom loop's `sum = 0` (and
`k = 0` when written) before the first func_80005820 call, so as1 puts it in
that jal's delay slot and hoists the argument copy above the D_1DC0 branch;
the target has both zeroes after the call. Placement of the two inits
(before/after the call, either order, dropped) and six head spellings
(intermediate object, `(*f()).state`, `if (x)`) are flat at 6.
<!-- plateau-handoff:func_overlay_001_F0002B4C_184EF2C:end -->
