<!-- plateau-handoff:func_overlay_101_F000AB4C_18E636C:start -->
### `func_overlay_101_F000AB4C_18E636C` plateau handoff

- source: `src/overlays/o101/overlay101TailAB4C.c`
- score: 499/638 words
- frame: 0x48
- relocations: 108
- first mismatch: +0x10
- summary: 499 masked at size delta 0 after L103 split of the three 0.0f pool entries on sprite nodes; per-row addiu-3 is childType=3 materialized late, not a folded L149 draw. 237 exact / 340 naming / 5 imm / 74 structural.

#### 2026-09-18, lane w13-o101: close size, reopen the addiu-3 hoist

Identity-gated instrumented IDO (proc=0, 55 decisions). Baseline was 579 masked at delta -8, 636 vs 638 words.

Falsified, each measured:

- L149 `u8 length` without `(length & 0xFF)` is byte-identical to `s32` plus the mask. draw_census stays 19 GP draws per text row (21 on the first).
- L154 `pool[count]` vs the explicit `count * 0x18` byte pointer is byte-identical.
- Hoisting `previousType` / `previous` / `childType=3` / `child` ahead of `mode` is 470 positional but 154 aligned-structural against 499 / 74. Reverted.
- `child` then `childType`, and `(s32)(u8)3`, are inert.
- `(f32)(s32)1` for node 1 scale regresses to delta -8.

Adopted: `0.00f` on sprite node 2 and `0.000f` on node 3, against node 1's `0.0f`. That splits the float-zero CSE (L103) and restores the two `mtc1 $zero` the target emits, closing size to 638/638. `(f32)(s32)0` on both later nodes only reached delta -4.

The leftover per-row addiu-3 is the same `childType = 3` constant, materialized late here and with `mode=2` / `kind=4` in the target. Integer census is 24 percent coherent across 33 windows. FP is one clean six-cycle at 98 percent, new mapping at +0x1B8.

Next: demand constant 3 in the post-call materialization window with 2 and 4, without moving the previousType copy.

#### 2026-09-11, coordinator: four-bucket split, no source attempt

Measured with tools/align_symbol.py on the integration branch and reproduced independently of the lane that first ran it, whose numbers were identical but whose worktree persisted nothing.

  - size delta: +108
  - positional masked: 636
  - byte-exact aligned rows: 150
  - register-naming rows: 271
  - immediate-only rows: 9
  - really-different rows: 278
  - displacement tax: 78

Frame 0x48 exact on both sides while the size is 108 bytes over, so the surplus is body rather than layout. Structural and naming are near parity, which is unusual in this overlay.

This procedure issues calls, so by the call test it is p1-only and L100's save ratio is the axis for colours the web is actually offered. CORRECTED 2026-09-12: the call test retires L106 and nothing else. Statement order still reaches as1's scheduling and ugen's emission order, neither of which the census touches, and it was decisive on every one of seven functions closed in one lane. Do not read this shard as saying statement order is the wrong axis.

<!-- plateau-handoff:func_overlay_101_F000AB4C_18E636C:end -->
