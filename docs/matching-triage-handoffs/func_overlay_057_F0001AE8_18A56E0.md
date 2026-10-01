<!-- plateau-handoff:func_overlay_057_F0001AE8_18A56E0:start -->
### `func_overlay_057_F0001AE8_18A56E0` plateau handoff

- source: `src/overlays/o057/func_overlay_057_F0001AE8_18A56E0.c`
- score: 0/883 words, promoted
- frame: 0x38
- relocations: 359
- first mismatch: none
- summary: Matched. Indexed descriptor walks, fog easing loads by name and stores through a pointer, (s32) on the first addend, count declared first.

## 2026-10-01 (lane `d-o057`): ROM-exact closure, 614 -> 0 at size delta -132 -> 0

No allocator force was used in the adopted source; forces were used only as
pricing instruments. Every step rewrites inherited shape. Measured with
`tools/fast_score.py` (configured flags) unless stated.

  - The -132 was the fog easing tail, not the dispatch. The target reloads
    each value in all four unrolled copies, loads through an address register
    (t0/t1) and stores with a direct symbol store; the candidate forwarded the
    stored values between copies. `volatile` reaches delta 0 only by emitting
    two loads per statement (594, wrong shape). Reading by name and storing
    through `s32 *near = &D_134` (an ISTR the loads cannot forward across)
    reproduces the copy shape exactly: 617 at -4, and with the counter priced
    into a2 by force, 226 at delta 0. Dropping the dead `value` local fixed the
    frame (0x40 -> 0x38).
  - The remaining -4 and every naming row before it was the s0 contest. The
    priced force `p1:w56=c5` (the shared loop counter into a2) was worth
    594 -> 413. The source edit that removes the need for it: every
    descriptor walk becomes an indexed loop (`for (count = 0; count < 9;
    count++) f(range[count], 0)`, keeping the shipped `<` against `!=` per
    walk), so uopt creates the walking pointers and the counter stops
    outranking the +0xE0 base. 617 -> 24 at delta 0, no force.
  - `addu` operand order: `*near = D_134 + X` emits X first; `(s32)D_134 + X`
    emits the loaded value first as shipped (20 words, measured on a mini TU
    first, 13 spellings, only the cast and an unsigned cast move it).
  - The counter's spill cell: declaring `count` before the other locals puts
    it at 0x34 (4 words). 7 declaration shapes by 2 cast cells measured; one
    exact.
  - Cleanups then measured flat at 0: the `cursor` local and the `limit`
    carrier removed, the four range-end externs dropped.

Promotion: the dispatch is a 21-entry compiler jump table already in the
retained data at data_rodata +0x5F0 (rodata +0xB0). Overlay 46's form: an
absolute anchor `gOverlay57DispatchJumpTableReloc=0xB0`, the two text
relocations (+0x30, +0x38) rebound by
`config/normalizations/func_overlay_057_F0001AE8_18A56E0.rebind.spec`, the
private table externalized by digest, `.rel.rodata` dropped, `.text` trimmed
to 0xDCC; atlas row in `FIXED_DATA_RODATA_OWNERSHIP` (externalized). The old
trim-only rule for this object was removed. `gmake verify` printed
507341c0a40ca3e9a7cee969b396ee53facfb548; `promotion-proof` PASS (883 words,
359/359 relocations).
<!-- plateau-handoff:func_overlay_057_F0001AE8_18A56E0:end -->
