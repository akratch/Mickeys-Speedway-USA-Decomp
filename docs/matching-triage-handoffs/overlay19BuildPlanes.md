<!-- plateau-handoff:overlay19BuildPlanes:start -->
### `overlay19BuildPlanes` plateau handoff

- source: `src/overlays/o019/overlay19BuildPlanes.c`
- score: 0/532 words, promoted
- frame: 0x140
- relocations: 4
- first mismatch: none
- summary: Matched. Written as the sibling of the matched o035 F0000B40 (DKR collision-plane builder) with no do-while(0) macros.

Summary before this remeasure: Exact frame is retained, but broad structure/register divergence remains after the full flag lattice and ten coherent source hypotheses.

Lane w2-ovld (wave X, 2026-10-02): `tools/sibling_scan.py` ranked the matched `func_overlay_035_F0000B40_1882820` at 0.79. The body was rewritten as that function's copy and measured in products (`tools/shape_product.py`):

- Straight o035 copy (its macros kept, record words copied as `s32`, 10.0f bisector scale, cross products relative to the second vertex in both passes, no early exit): 514 at delta +16. The `group` parameter was split at the copy loop and lived in memory outside the first pass's inner loop.
- Dropping every do-while(0) macro (span reads, normalisation, vertex reads): 70 at delta 0. The macros' extra blocks are what stopped the parameter's entry live range from growing through the first pass (the decision records show the split's growth refused at the outer span loop).
- Frame homes: `mag` declared after the normals and one unused `s32` before `x5..z5`: 33.
- Edge body under `if (neighbor != 0xFFFF)` instead of `continue`, and the neighbour scan as a do-while: 2. This moves the output pointer and the 0xFFFF constant to s4/s3 and the oppVertIndex and edge-offset webs to t3/t2 (block counts re-rank the tied saves, checklist 18).
- `oppVertIndex` declared after `opp`: its spill home moves to sp+0xB4, 0 at delta 0.

The header's record fields are `u16` (the target's `lhu`) and the output's third field is the `f32` plane array.

- assignment base: `b29cd11e60730f75b290e2b95ef0d362a766c6b0`
- owned range: overlay 19 `+0x01E0..+0x0A30`, 2,128 bytes / 532 words, followed immediately by separately owned `overlay19BuildAdjacency`
- baseline: the configured guarded C emitted 530 words with the exact `0x140` frame, 514 raw and relocation-masked differing words, and first mismatch `+0x48`
- retained result: direct use of the three ABI parameters plus a guarded advancing-pointer record copy emits 522 words with the exact `0x140` frame and lowers the raw/masked residual to 512 words; the assembly fallback remains canonical
- relocation proof: target and candidate each have four call records; one offset/type pair aligns, two candidate identities resolve, two remain unresolved, and zero stable identities are exact, so preflight correctly fails promotion closed
- diagnosis: workbench reports a broad `cfe-spelling` structure/register divergence, not a movable-block permutation; the retained result has 291 aligned structural, 277 aligned register, and ten aligned constant sites
- attempts: the complete 119-row flag lattice kept configured `-O2 -mips2 -32 -Wab,-r4300_mul` uniquely best on geometry; ten bounded attempts covered alias order, direct parameter webs, indexed versus advancing-pointer copy, cursor scoping, combined validated gains, target-shaped first and second traversal loops, edge-loop spelling, and a signed-16-bit plane counter
- rejected structural route: target-shaped first-traversal offsets lowered aligned structural sites from 291 to 260 but regressed the positional score to 515 words and enlarged the frame to `0x148`; second-traversal `do` loops were optimizer-flat and the narrow counter regressed
- donor evidence: the pinned DKR v77/v80 and JFG Overlay 19 ledger is `none`; the nearest skeleton is JFG `func_overlay_24_01800CE4_1F42814` at only 0.117 similarity, and a local coddog raw comparison yielded no credible exact-window hit while emitting configuration-bound diagnostics
- next action: reopen only with a new source-faithful lifetime/offset geometry that preserves the exact frame while retaining the first-traversal structural gain, together with authenticated identities for all four calls; do not repeat the flag lattice or these ten forms
<!-- plateau-handoff:overlay19BuildPlanes:end -->
