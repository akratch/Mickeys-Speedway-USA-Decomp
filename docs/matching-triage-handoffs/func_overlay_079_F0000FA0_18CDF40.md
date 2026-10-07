<!-- plateau-handoff:func_overlay_079_F0000FA0_18CDF40:start -->
### `func_overlay_079_F0000FA0_18CDF40` plateau handoff

- source: `src/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c`
- score: 0/184 words, promoted
- frame: 0x98
- relocations: 13
- first mismatch: none
- summary: Matched. Rewritten in the shape of the resident sibling func_800115E4 (track.c) with literals at their uses and declaration order fixing the homes.

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

#### 2026-10-05: a truthy plane-flag test folds away

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184.

Writing the plane flag test as a truthy mask, the form matched overlay 26 uses, scored the same 143 masked and 144 raw words at size delta 0. Aligned exact, naming, immediate, really different, and displacement tax stay 65, 80, 5, 38, and 20. The 736-byte function text is byte-identical to the proved baseline. The compare is folded. The body is not kept. The restored source is the proved baseline. Do not repeat this truthy flag test on the kept body. The cross association and the divide order stay closed.

#### 2026-10-05: one shared length raises the residual

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184.

Deleting both block-scope length declarations, so both square roots use the function-scope length, scored 149 masked and 150 raw words at size delta 0. Aligned exact fell from 65 to 58, naming fell from 80 to 79, immediate rose from 5 to 12, and really different rose from 38 to 39. Displacement tax fell from 20 to 19. The aligned residual rose from 123 to 130. The first masked mismatch moved to entry. The frame stayed equal and the candidate stayed 184 words. The body is not kept. The restored source re-scores 143 masked words at delta 0. Do not repeat this shared length. The truthy flag test, the cross association, and the divide order stay closed.

#### 2026-10-05: dropping register from ny does not change the object

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first masked mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184. Owned text sha1 988af62cf5ea6522197832dc831728393b02f632.

Writing ny as an ordinary float, without the register keyword, scores the same 143 masked and 144 raw words at size delta 0. Aligned exact, naming, immediate, really different, and displacement tax stay 65, 80, 5, 38, and 20. The 736-byte function text is byte-identical to the proved baseline. The keyword does not move the colour. The body is not kept. The restored source re-scores the same sha1. Do not repeat this register deletion. The shared length, the truthy flag test, the cross association, and the divide order stay closed.

#### 2026-10-05: declaring projectedX ahead of crossZ leaves the buckets

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184. Aligned residual 123. Owned text sha1 988af62cf5ea6522197832dc831728393b02f632.

Declaring projectedX immediately ahead of crossZ scored the same 143 masked and 144 raw words at size delta 0. Aligned exact, naming, immediate, really different, and displacement tax stayed 65, 80, 5, 38, and 20. The first mismatch stayed +0x3C. The 736-byte function text is not byte-identical: sha1 bfb72659061b0fba66c6810dbc13b1ea10e482fc. The body is not kept. The restored source re-scores the baseline sha1. Do not repeat this declaration swap. The unused length deletion and the lengthSquared reuse stay closed.

#### 2026-10-05: reusing value for crossZ shortens the function

Configured full-TU baseline: 143 masked and 144 raw words, target 736 bytes, size delta 0, first mismatch +0x3C. Aligned exact 65, naming 80, immediate 5, really different 38, displacement tax 20. Candidate 184 words, target 184. Aligned residual 123. Owned text sha1 988af62cf5ea6522197832dc831728393b02f632.

Carrying crossZ in value, and deleting the crossZ declaration, scored 156 masked and 157 raw words at size delta -4. The candidate shrank to 183 words and 732 bytes. Aligned exact stayed 65, naming fell from 80 to 78, immediate rose from 5 to 7, and really different fell from 38 to 36. Displacement tax rose from 20 to 35. The aligned residual fell from 123 to 121. The absolute size delta grew from 0 to 4. The first mismatch stayed +0x3C. Owned text sha1 7c04419e3f5baef1f2c928c8bb5b020d2c664fef. The body is not kept. The restored source re-scores the baseline sha1. Do not repeat this value carrier. The declaration swap, the unused length deletion, and the lengthSquared reuse stay closed.

#### 2026-10-06: loading nx and nz before ny does not move f16

The kept body scores 736 bytes, 144 raw and 143 masked words, size delta 0, first mismatch +0x3C. The y component loads into f12. Loading x, then z, then y scores the same 143 masked words at size delta 0. The first mismatch stays +0x3C. The object is not the baseline object. The body is not kept. Do not repeat this load order.

#### 2026-10-06: adding distance times zero to ny is folded

The unmodified body scores 736 bytes, 144 raw and 143 masked words, size delta 0, first mismatch +0x3C. The ROM loads the plane y component into f16. This body loads it into f12.

Adding distance times zero to that y load scores the same 143 masked and 144 raw words at size delta 0. The mismatch list is unchanged. The product is folded. Not kept. The 143-word body stays. Do not repeat this distance tie. The nx and nz load order stays closed.

### 2026-10-07, lane a-ovl2: matched from the resident sibling

`tools/sibling_scan.py` ranked `func_800115E4` (src/main/track.c) at 0.66. It is the same collision response with one more branch (the `ny <= -0.866f` floor case) and a plane offset computed before the branch. Every earlier pass held the m2c carrier shape fixed; this one discarded it.

- Copying the sibling with the middle branch removed, the offset computed at the top of the slide branch, and the three literals written at their uses (`0.707f`, `0.1f`, `0.01f` twice; IDO emits two separate pool words for the two `0.01f`, which are the shipped addends +0x38 and +0x3C) scored 149 masked at size delta +16. The memory-class set (plane constant, delta, u, v, w) already agreed with the target.
- Declaration order nx, ny, nz, dx, d, dy, dz, len, delta, u, v, w, value, angle put every home on the target's cell: 147 masked, still +16.
- One 192-cell product over which local holds the plane offset, the quotient `delta / angle`, the first branch's `radius - plane->distance`, and how `0.01f - value` reaches delta: floor 4 masked at delta 0. The four extra words were caller-save spills of the first branch's dy and of the quotient; reusing `len` for the first-branch offset and `value` for both slide-branch roles gives the target's saved colours.
- Moving `state = object->state` ahead of `d = plane->constant` closed the entry schedule: 0 masked, 4 raw (the literal-pool addresses).

Promotion: the object's .rodata is externalized onto the module pool at anchor 0x30, and the resident callees use the existing `_o079Reloc` surface names. `gmake verify` passed, `check-overlay-syms` up to date, `promotion-proof` PASS (184 words, frame 0x98, relocations 13/13).

<!-- plateau-handoff:func_overlay_079_F0000FA0_18CDF40:end -->
