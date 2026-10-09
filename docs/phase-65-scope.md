# Phase: reach 65% resolved

Measured on this tree by `tools/triage.py --target-pct 65` and
`gmake check-scoreboard`. These figures replace the 2026-09-23 arithmetic
in `docs/NEXT_CAMPAIGN.md`.

```
resolved 590,948 / 944,340 = 62.58%
target   65% = 613,821 bytes
GAP      22,873 bytes
```

`gmake check-scoreboard` reports that `README.md` matches the tree:
whole program 590,948 of 944,340 (62.58%). A plateau, a `NON_MATCHING`
body, padding, and the refused overlay 9 credit do not move this number.
Only an ADR 0001 match does.

## Closed, and not reopened

- `func_80058250` (one-symbol high-half size lever, still +16 bytes).
- The twelve functions whose size is off by 4–12 bytes:
  `func_8002B040` (+4), `ProcessRelocationEntry` (+4), `func_800084C4` (+4),
  `func_80011980` (+4), `func_overlay_043_F0000324_188A2F4` (−4),
  `func_overlay_047_F0000B30_1891948` (+4),
  `func_overlay_101_F0002510_18DDD30` (+8), `func_8000DFBC` (+8),
  `func_80028564` (+8), `func_8005AF14` (+12),
  `func_overlay_022_F00002B0_18783B8` (−12),
  `func_overlay_058_F00005FC_18AF7E4` (−12).
- Colour sweeps, including every symbol the forced-floor census already
  withholds, and odd-FP flag sweeps.
- The overlay 1 tail (`overlay_001_tail.c`), including
  `overlay1UpdateRangeFlags`, `overlay1TransitionState`, and
  `overlay1BendPathPoint`.
- `func_8005ABA8`, `overlay57UpdateModeState`, `refractOutputAssembler`.
- Bare overlay `GLOBAL_ASM` with no candidate (overlay 51 middle, overlays
  69 and 88 sorted-draw, leading overlay 14, the overlay 1 function that
  shares another function's `#else`).
- `main/trackasm`, `main/shadows_fp`, `main/weather_snow_asm`, and the
  four-byte nop.
- Already-recorded stalls that this phase does not retry:
  `levelInit`, `overlay14CreateValue`, `overlay20RemoveEntry`,
  `func_80023598`, `func_overlay_071_F0000870_18CA390`, `func_8004EC60`,
  `func_80018F08`. No new proof step is added for
  `func_overlay_009_F0000744_1866DBC` (proof refusal: expected one tracked
  exact atlas range for func_overlay_009_F0000744_1866DBC, found 0 (none)).

## Attack

Seventeen size-exact functions, one per translation unit, 22,952 bytes.
That is the gap of 22,873 plus the function that crosses it. Each lane
gets one new source hypothesis and at most three spellings, then a match
or an ADR 0018 plateau.

| Bytes | Masked | Symbol | Translation unit | Hypothesis |
| ---: | ---: | --- | --- | --- |
| 3528 | 198 | `func_overlay_079_F0000134_18CD0D4` | `src/overlays/o079/func_overlay_079_F0000134_18CD0D4.c` | Break the `-1.0f` CSE so `forward.z` draws on its own |
| 2112 | 130 | `func_overlay_035_F0000B40_1882820` | `src/overlays/o035/func_overlay_035_F0000B40_1882820.c` | Edge-loop induction order |
| 1836 | 186 | `func_80007118` | `src/main/objects.c` | Payload address reads without a second live copy |
| 1476 | 157 | `func_80022FD4` | `src/main/camera.c` | Unfold the GP-draw schedule at `+0x9C` |
| 1472 | 48 | `func_overlay_027_F0000064_187BA3C` | `src/overlays/o027/overlay_027.c` | Real-ABI call input with no spill |
| 1424 | 214 | `overlay68UpdateAnimation` | `src/overlays/o068/overlay68UpdateAnimation.c` | Duration-loop structure |
| 1376 | 51 | `render_epc_lock_up_display` | `src/main/diCpu.c` | Leading ring draw of the dead tick |
| 1344 | 125 | `func_overlay_009_F0000000_1866678` | `src/overlays/o009/overlay_009.c` | Local-stack order of the GPR web |
| 1268 | 54 | `func_overlay_046_F0000120_188E518` | `src/overlays/o046/overlay46UpdateSequence.c` | Zero-size web partition |
| 1216 | 75 | `overlay58FinalizePackedStatus` | `src/overlays/o058/overlay58FinalizePackedStatus.c` | Fifth home for the mode-0 index |
| 1208 | 72 | `func_80040B88` | `src/main/particles.c` | One source-safe draw before `MOVE_END` |
| 1088 | 202 | `texDPTextureX` | `src/main/textures_354C8.c` | Keep `D_800D302C` address in a register |
| 1012 | 39 | `overlay2QueryNode` | `src/overlays/o002/overlay2QueryNode.c` | Forbid `v1` on the save below 1.6 |
| 768 | 56 | `func_80030610` | `src/main/sched.c` | Symbol compare whose `lui` fills the branch delay |
| 740 | 158 | `func_80047304` | `src/main/fx.c` | Hoisted bound, no prior source attempt |
| 732 | 37 | `func_overlay_008_F0002640_1860398` | `src/overlays/o008/overlay_008.c` | Preheader emission-count move |
| 736 | 63 | `func_80019AB8` | `src/main/lights.c` | Re-derive from the newer Jet Force Gemini body; Mickey's ROM decides |

23,336 bytes in total. `overlay1ActivateObject` was dropped: lane status is
already-integrated/exhausted, not base-only.
