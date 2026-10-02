<!-- plateau-handoff:func_overlay_001_F0002B4C_184EF2C:start -->
### `func_overlay_001_F0002B4C_184EF2C` plateau handoff

- source: `src/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c`
- score: 186/451 words
- frame: 0xB0
- relocations: 63
- first mismatch: +0xE4
- summary: Listing rewrite in while(i--) shape with per-site identities: 433 to 186 masked, size -16 to 0, frame exact; temp ring offset from +0xE4 open.

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
<!-- plateau-handoff:func_overlay_001_F0002B4C_184EF2C:end -->
