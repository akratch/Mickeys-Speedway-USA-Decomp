<!-- plateau-handoff:overlay15InitStars:start -->
### `overlay15InitStars` plateau handoff

- source: `src/overlays/o015/overlay_015.c`
- score: 0/190 words, promoted
- frame: 0xb8
- relocations: 15
- first mismatch: none
- summary: Matched. Rain-field bounds as the static struct, count*12 byte-pointer colour base, for loop with index and both cursors in the init clause, five unused s32 pads.

#### 2026-10-01, lane d-o015: matched by a 243-cell product

Promoted at 190 of 190 words, delta 0, frame 0xb8, 15 of 15 relocations.
Before: 89 masked words (the previous lane's best scratch, a for-loop with
pads, was 9). Priced by tools/shape_product.py over placements of the five
setup statements (index, star cursor, colour cursor, count store, zero store)
in three slots: floor 5. Then `colors = (u32 *)((u8 *)stars + count * 12)`
instead of `stars + count` (the latter commutes the final addu): 4. Then the
loop init clause `for (i = 0, stars = ..., colors = ...; ...)` with no
separate statements: 0, and 18 of 108 cells in that sub-product were exact, so
the statement slots no longer matter. The bounds are `sOverlay15Rain.bounds`
(the static field struct) rather than the proxy symbol; the .bss records at
0x738/0x73c are filtered in mk/overlays.mk.

Gates: gmake verify, check-overlay-syms and promotion-proof pass.

#### 2026-09-13, lane l1: measured schedule controls

Fresh procedure-7 baseline is 190 words at delta zero, with 89 masked
differences and the first positional mismatch at plus 0x7C. This supersedes
the older header's first-offset value. Alignment is 103 exact, 56 naming, six
immediate and 23 paired structural rows, plus two candidate-only and two
target-only words. Both frames are 0xB8, but the slot census still differs.
Candidate static relocations number fifteen against thirteen target entries;
this count is not canonical relocation identity proof. The census records
74 draws and 250 emissions.

Two controls test address normalization and source-location barriers. First,
spell the colour-buffer boundary as the equivalent unsigned byte offset rather
than a star-element pointer addition. Stock text, every line's counts and the
draw sequence are identical. Second, place the four ordered setup stores on
one physical source line, preserving their order and subsequent line numbers.
The individual draws/emissions are attributed to that line, but total counts,
draw order and stock text remain identical. A location change by itself is
therefore not a scheduling fix in this setup block. Both controls are rejected.
Stop early under the existing closed-colour evidence: neither control reaches
the remaining spill and emission-order mechanism. Next action requires a
trace-backed change to a named lifetime or release order, not another source
line fold on this unchanged shape.

Named Ucode mapping and full stock/capture fidelity pass for the baseline and
each retained experiment. Sources, stock objects, scores, frame/relocation
censuses and aligned deltas remain under ignored build/l1/overlay15InitStars. Commands:
configured compilation, allocator_trace_receipt mapping, draw_census profile
and comparison, residual_map object comparison, finalize_plateau, and
tools/gates.sh verify cleanroom check-docs. No matching credit is claimed.

<!-- plateau-handoff:overlay15InitStars:end -->
