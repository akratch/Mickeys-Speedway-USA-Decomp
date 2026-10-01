<!-- plateau-handoff:overlay57UpdateSelection:start -->
### `overlay57UpdateSelection` plateau handoff

- source: `src/overlays/o057/overlay57UpdateSelection.c`
- score: 0/283 words, promoted
- frame: 0x48
- relocations: 91
- first mismatch: none
- summary: Matched. No volatile state pointer or sentinel local, id walks as while (*list != -1) with the list read at each use, early-return tail, two trailing frame cells.

## 2026-10-01 (lane `d-o057`): ROM-exact closure, 162 -> 0

The previous closure ("the read count on the primary-state address is the
decision variable; sharing one read collapses the chain") was a statement about
the inherited shape: the `volatile s32 *primaryState`, the `sentinel` local and
the `(entry = *list) != sentinel` walks. Rewritten from the target listing as
plain global reads, measured with `tools/fast_score.py`:

  - Plain reads of the primary state, `while (*list != -1)` walks reading
    `*list` at each use, an else-if chain for the tail: 287 masked at +28.
    Every head instruction then agrees; the +28 is two hoists the target does
    not make (the 0.007f literal into f20 and the +0x17C table base into s4,
    each one callee-saved register more). Records: both webs sit at
    totalsave 18 against a callee-save cost of 17.75.
  - The same body with the tail written as three early returns (as the
    inherited source had it) instead of the else-if chain: 14 masked at
    delta 0, all frame immediates. The block structure moves the callee-save
    cost past those two webs' totalsave.
  - Two trailing declared s32 cells: frame 0x40 -> 0x48, 0 masked.
  - The published index reads the current-selection word back after the
    store, as the target does.

Promotion: single-function TU, existing trim rule (0x46C) unchanged.
`gmake verify` printed 507341c0a40ca3e9a7cee969b396ee53facfb548;
`promotion-proof` PASS (283 words, 91/91 relocations).
<!-- plateau-handoff:overlay57UpdateSelection:end -->
