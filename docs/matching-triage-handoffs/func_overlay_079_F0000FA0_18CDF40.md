<!-- plateau-handoff:func_overlay_079_F0000FA0_18CDF40:start -->
### `func_overlay_079_F0000FA0_18CDF40` plateau handoff

- source: `src/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c`
- score: 41/184 words
- frame: 0x98
- relocations: 13
- first mismatch: +0x3C
- summary: Fresh reproof unchanged; no caller, Conker donor, or proxy evidence resolves the 13-to-5 relocation mismatch.

### 2026-10-02, lane w7-o079: sibling plane shape grows the frame

Per-file flags on this object are only `-Wab,-r4300_mul`. No `-Wo,-loopunroll,0`, `-Olimit`, or `-O2 -g3` is present, so nothing was removed. The multiply-hazard flag stayed.

`tools/shape_lint.py` reports three artefacts: the four `f32` externs, the `volatile` plane constant, and the masked flag test. `tools/overlay_tables.py --json` gives overlay 79 text `0x14E0` and data `0x60` at ROM `0x18CCFA0`. The same module's relocation decoder, filtered to this function (`0xFA0`..`0x1280`), shows 13 records, not 5. Five are `R_MIPS_26` `SYMBOL` calls: `sqrtf` at `+0x10A0`, `+0x11B8` and `+0x1204`, `Arctanf` at `+0x11C4`, `func_8002A8BC` at `+0x11D4`. The other eight are `LOCAL` `HI16`/`LO16` pairs, every one with base `0x1500` and lo addends `0x30`, `0x34`, `0x38`, `0x3C`. Those are `gOverlay79Constants` indices 12 through 15 (`0.707f`, `0.1f`, `0.01f`, `0.01f`), the tail of the pool defined in the neighbouring TU. The old 13-to-5 note counted calls only. The two `0.01f` words are distinct addresses, so one spelling would merge them.

Matched overlay 79 siblings (`overlay79FindNearby`, `func_overlay_079_F0000000_18CCFA0`) are straight field reads and inline literals, not this callback. The same `func_80010900` callback that is matched is `func_overlay_026_F0000B18_187AF10`: initialized normals, one reused length, a plain constant, and a truthy `flags & 0x10000000` test. Its else copies the hit point. This target's else is the arctan slide, so that part was not copied.

One `tools/shape_product.py --jobs 2` on that difference, 144 cells over cross spelling, divide order, equal-versus-not-equal, flag form (truthy, `!= 0`, bitfield), constant binding (four externs, the shared array, two literal spellings), and volatile versus plain. Floor 177 masked words at size delta +12. No cell at delta 0. Inside that shape the cross, divide, compare, flag, and volatile axes were flat; the array binding tied the floor. Not adopted. The kept body remains the carrier form.

Follow-up 1. Splitting `projectedX` so `crossY * nz` is its own statement before `crossZ` was byte-identical: 143 masked, size delta 0, first masked mismatch `+0x3C`. uopt folds the split. Eliminated.

Follow-up 2. Dropping `volatile` on the plane constant stays at size delta 0 but scores 144 masked, one word worse. Restored.

Kept measurement: 736 bytes, 143 masked of 184 words, 144 raw, size delta 0, frame `0x98`, first masked mismatch `+0x3C` (raw `+0x38`). Aligner: 65 byte-exact, 80 register naming, 5 immediate only, 38 really different, displacement tax 20. First naming `+0x3C`, first immediate `+0x60`, first structural `+0x9C`. Register census: no integer substitutions. Float swaps `f8`/`f6` and `f4`/`f10` dominate, but coherence is 73 percent across 16 windows, so it is not one ring phase. Frame census: both frames `0x98`. Target-only homes `0x84`, `0x74`, `0x70`, `0x6C`, `0x68`, `0x54`. Candidate-only homes `0x80`, `0x7C`, `0x78`, `0x60`, `0x50`. That is not a one-pad shift. The sibling local set moved the frame by 12 bytes instead of landing those homes.

Tool gaps: `overlay_tables.py --json` prints module headers only; per-site op and addend come from `read_module_relocations` in that script, which has no overlay filter. `shape_product.py` only treats `==` and `!=` as axis values. No full ROM verify, because the result is not exact. A cold `gmake -j2` failed at link until `gmake overlay-syms`; the retry linked.

### 2026-10-02, lane w2-ovld: the home ladder fits overlay 29's declaration order

Not adopted (the kept body is unchanged). Measured with `tools/fast_score.py` on scratch candidates:

- The target stores the first branch's cross Y and Z at sp+0x6C and sp+0x68 and the second branch's amount and projected X, Y at sp+0x74, sp+0x70, sp+0x6C, with the plane constant at sp+0x84. `overlay29ProjectPoint`'s declaration order (state, three normals, projected X/Y/Z, lengthSquared, cross X/Y/Z) with the plane constant inserted fifth puts lengthSquared, crossX, crossY, crossZ on exactly those four cells; the second branch then reuses lengthSquared for the amount and crossX/crossY for the projected point. The target's spill of the first branch's projected X at sp+0x54 is the first cell under sixteen declared locals.
- That candidate (overlay 26's first branch, non-volatile plane constant, a `dist` local for the dot and the quotient): 137 masked at size delta -4, frame 0x80, every relative home agreeing. The missing word pair is that spill: here the projected X takes a saved register, in the target f14 saved around the call.
- A third vector for the normalised projection and the second branch's deltas, sharing registers the way the target's colours suggest: 184 to 190 at +24 to +40. Refuted.

Open: which first-branch values the target keeps in caller-saved colours (ny in f16, cross X and the squared length in f12, projected X in f14).

#### 2026-10-04: projectedX union self-copy grows the function

Reopened for the overlay 92 float/unsigned-word union self-copy on
projectedX. The configured full-TU baseline of the carrier body is 736
bytes, 184 words on both sides, 143 masked and 144 raw, frame 0x98 on
both sides, first mismatch +0x38, 119 opcode mismatches, and 18
relocation sites that name different symbols. The verdict is
structure-mismatch. Diagnose names the constants first: the candidate
relocates four separate collision constants where the ROM table uses one
local pool. The shared-array binding of those four floats was already
measured at size +12 and was not retried.

The union replaced projectedX and copied its integer member onto itself
before the first square root. The compile grew the function to 185
instructions against 184, so the candidate escaped translation-unit
ownership. The frame stayed 0x98. Exact is false, opcode mismatches are
118, and raw words are 159. The verdict stays structure-mismatch. The
body is not kept. Do not reopen this function for another union spelling
of projectedX, or for an array or struct of the four collision constants.

#### 2026-10-04: ny as plane->normal.y grows the function

Configured full-TU baseline: 143 masked and 144 raw words, target 736
bytes, size delta 0, first masked mismatch +0x3C, frame 0x98. Replacing
the declared `ny` local with `plane->normal.y` at each use recompiled to
170 masked words at size delta +8. The first masked mismatch moved to
+0x34. The restored body re-scores 143 masked words at delta 0. The body
is not kept. Do not replace this declared normal with the field expression.

#### 2026-10-05: a region around the first sqrt grows the function

Configured full-TU baseline remains 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20.

Wrapping that first `sqrtf` in `do { } while (0)` scored 188 masked words at size delta +28, candidate 191 words. Aligned exact fell to 33 and really different rose to 68. First mismatch moved to entry. The region does not free f16 here. The body is not kept. The restored source re-scores 143 at delta 0. Do not repeat this region.

#### 2026-10-05: the unused outer length does not move the home

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184.

Deleting the function-scope `length`, which both block-scope lengths shadow, scored the same 143 masked and 144 raw words at size delta 0. The first masked mismatch stays +0x3C. Aligned exact stayed 65. Naming rose from 80 to 82 and really different fell from 38 to 36, so the aligned residual stays 123. The owned text is not the proved baseline. The body is not kept. Do not repeat this deletion. The field rewrite, the projectedX union, and the sqrt region stay closed.

#### 2026-10-05: reusing value for the root and the offset grows the function

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184.

The matched overlay 26 sibling carries the squared length, its square root, and the plane offset in one local. Writing those three assignments through `value`, and dropping the first branch's separate length, scored 159 masked and 160 raw words at size delta +8. The candidate grew to 186 words. Aligned exact rose from 65 to 68 and naming fell from 80 to 75, while really different rose from 38 to 40 and the displacement tax rose from 20 to 39. The aligned residual fell from 123 to 120, and the absolute size delta grew from 0 to 8. The first masked mismatch stays +0x3C. The body is not kept. The restored source re-scores 143 at delta 0. Do not repeat this reuse. The field rewrite, the projectedX union, the sqrt region, and the unused outer length stay closed.

#### 2026-10-05: dividing X then Y then Z is byte-identical

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184.

Dividing the normalised projection in the order X, then Y, then Z, matching overlay 26, scores the same 143 masked and 144 raw words at size delta 0. The owned text is byte-identical to the kept body. The three divides are independent, so their source order does not change the object. The body is not kept. The source is restored to Y, then X, then Z. Do not repeat this order. The value reuse stays closed.

#### 2026-10-05: the overlay 26 crossY association does not move the residual

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184.

Writing crossY as `-(axis->z * nx) + (nz * axis->x)`, the matched overlay 26 association, scores the same 143 masked and 144 raw words at size delta 0. Aligned exact, naming, immediate, really different, and displacement tax stay 65, 80, 5, 38, and 20. The first masked mismatch stays +0x3C. The owned text differs from the kept subtraction. The body is not kept. The restored source re-scores 143 at delta 0. Do not repeat this association. The divide order stays closed.

<!-- plateau-handoff:func_overlay_079_F0000FA0_18CDF40:end -->
