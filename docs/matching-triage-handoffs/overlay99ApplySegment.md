<!-- plateau-handoff:overlay99ApplySegment:start -->
### `overlay99ApplySegment` plateau handoff

- source: `src/overlays/o099/overlay99ApplySegment.c`
- score: 190/230 words
- frame: 0xD0
- relocations: 27
- first mismatch: +0x50
- summary: Size 0 at frame 0xD0. L115 x-reuse, overlay22 empty if(var_s0), volatile sp88. 190 masked; stack homes still +0xB4 not +0xA0.

#### 2026-09-19, lane w30-o099: size and frame closed; 190 remains

Live V0 was 202 masked, size delta -4, first +0x0, frame 0xE8. Identity-gate:
instrumented IDO .text matches stock. CDX_PROC 0, 41 p1dec, 30 p1color, 0 p2.
Prologue saves match the target (ra, s0-s8, six sdc1). The 24-byte excess was
extra spilled float homes, not extra callee-saves.

L115: reuse the interpolation x carrier (`temp_f26`) for the loop x coordinate
(delete `temp_f26_2`) restores frame 0xD0. Reuse of z does not shrink the frame
and scores 191. Isolated reuse_x is size delta -8 at 0xD0, first +0x50.

The two missing words on that shape: overlay22 empty `if (var_s0)` after the
inner compact adds one word (OR-zero on s0 folds; empty if on s3 is inert).
Volatile on `sp88` (the +0x88 plane coefficient) adds the second. Together:
190 masked, size 0, frame 0xD0, first +0x50. Volatile on `spA0` with the same
empty-if is also 190; `sp8C` volatile is already load-bearing (dropping it
grows the frame to 0xE0).

Aligned at size 0: 72 exact, 82 naming, 6 immediate, 81 structural.
11 candidate-only and 11 target-only words. Displacement tax 21. Stack homes
are still +0xB4..+0xCC against the target's +0x7C..+0xA0. Integer census is
incoherent (68 percent, four windows, no cycle). Float census 61 percent,
eight windows, f4-f8 swap is not a ring phase.

Refuted or inert on the 190 shape: leftover arg0 OR-zero at entry or in the
loop (folds); hoist of 1.0f; L160 indexed grid access (grows 44, frame 0xE0);
comma-assign of the plane setup (191); extra volatile on sp90/sp84/sp7C;
L109 zero (closes size on the old 0xE8 shape at 177 but grows to 0xF0, and on
the 0xD0 shape grows to 0xE0); declaration reversal; unused pointer; L97
around the loops; `if (var_s0 != 0);` is byte-identical to the empty block.

Next: match the remaining 11-word insertion pair and the +0x7C..+0xA0 homes.
Colour is now legal (size 0) but 81 structural rows are not a colour residual;
do not start `--every-colour` until the candidate-only trunc/mfc1/sw at +0x104
and the target-only lwc1/mtc1 cluster at +0x1B8 are named. Keep overlay99
relocation identities untouched.
<!-- plateau-handoff:overlay99ApplySegment:end -->
