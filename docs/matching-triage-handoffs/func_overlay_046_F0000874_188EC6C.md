<!-- plateau-handoff:func_overlay_046_F0000874_188EC6C:start -->
### `func_overlay_046_F0000874_188EC6C` plateau handoff

- source: `src/overlays/o046/func_overlay_046_F0000874_188EC6C.c`
- score: 28/450 words
- frame: 0xC0
- relocations: 96
- first mismatch: +0x1FC
- summary: Size 0, 28/450. L99 unused as GBI cmd; dying count++ subscript. Colour floor 28 (202 probes, 0 winners). Homes +0x4C/+0x58 vs +0x54.

Fresh evidence:

- Assignment base: `d8b737b08b810602e750e195d357f542c79f407c`.
- Owned range: overlay 46 `+0x874..+0xF7C`, 1,800 bytes / 450 words.
- Configured V0: 1,812 bytes / 453 instructions, 87 of 450 positional words exact, 363 relocation-masked and 364 raw differences, first mismatch `+0x0`.
- Frame: target `0xC0`, candidate `0xE0`. Save slots occupy the same 32 bytes; non-save storage is 160 bytes in the target versus 192 in the candidate.
- Relocations: the retained runtime surface has 96 records. The fallback/full-TU diagnostic also reports broad unresolved symbol identity drift, so relocation proof remains fail-closed.
- Donors: the pinned DKR v77/v80 and JFG scans still provide no close source analogue.

New-mechanism result:

- Overlay 26's strict gain and Overlay 22's later exact match establish FP term rotation as a real Mickey IDO lever after the prior plateau closed.
- Rotating the case-1 interpolations from `start + delta * progress` to `delta * progress + start` is byte-flat.
- Applying the same rotation only in case 4 is byte-flat.
- Rotating both cases together is byte-flat. All three forms retain candidate SHA-1 `5086008b44a1`, 453 instructions, the `0xE0` frame, and 363 masked differences.
- Functional C is restored. No flag sweep or generic permutation was repeated.
- Prior constant, particle-base, flag, lifetime, register-order, and step families remain closed. Reopen only with a new source-authentic mechanism that explains the 32-byte non-save-frame excess or three-instruction structural excess; do not repeat term rotation.

#### 2026-09-17: frame closed; draw-loop shape vs colouring

Lane `lane/w12-o046` on base `40789047`. The 32-byte non-save excess was extra named temps, not the 19-slot table length (L112 is observable: the init end-home sits 72 bytes above the table base).

Proved this session:

- Deleting decompiler interpolation, fade, angle, and display-list temps drops the frame from `0xE0` to `0xC0`.
- One function-scope `f32 step` keeps a second callee-saved float and shifts the save ladder. Three case-local steps restore the target save set.
- `finished` then `count` then `result` places result at the target home. An L99 unused pointer declared immediately above the table places the table at the target base and the init one-past-last at the target end-home.
- Indexed draw of that table scores 50 masked, size delta +8, 418 aligned byte-exact of 454, with the first 0x600 bytes at two immediate-only rows. An explicit walking pointer for the same loop scores 412 masked and rotates the saved-register assignment from the prologue.
- Inlining case-2 step overlays the two leftover 4-byte spill homes onto one slot (2 load / 2 store) but at the wrong offset and scores 86. The homes do not overlay while step stays a case-local.

Closed: spill-name excess, function-scope step, table length, unused-pointer placement of the table. Do not repeat term rotation or a function-scope step.

Open: the draw loop still indexes instead of walking, which is the +8 and the +0x650 window; the target shares one 4-byte temp for case-2 step and the late draw spill, while this candidate keeps two. Next lever is a strength-reduced index that dies in the draw loop (L113 / L154) without introducing a source-declared pointer web.

#### 2026-09-19, lane w27-o046b: size closed at 28; colour floor is 28

Identity-gated instrumented IDO `.text` matches stock. `CDX_PROC=0` (p1 only; 43 decisions on the +8 form, 45 on the size-0 form). Unforced `forced=-2`; `p1:w9=c2` accepted.

The +8 was the shared `gDisplayListHead` stores, not a still-live draw index. The indexed draw already strength-reduced to a walker. A new display-list cursor local closed size but grew the frame to 0xC8. Reusing the L99 `unused` pointer as `unused = gDisplayListHead++; unused->w0/w1` keeps frame 0xC0 and is 31 masked at delta 0.

L146: loop-keyword cells that were flat on the +8 shape had to be re-climbed. `particle = particlesByVariant[count++]` is 28 at delta 0; the same spelling was byte-flat at 50 on the old shape. Subscript-only (no `particle` assignment) is 51 at -12 and drops the extra +0x4C home. New integer or pointer locals grow the frame. Inlining case-2 step is 64. Leftover OR-with-zero is byte-flat.

`--every-colour` (202 probes, 31 webs) has zero winners of 28. Four probes tie at 28. No 0-scoring force, so L160's force-then-delete-carrier route does not apply.

Still two 4-byte homes (+0x4C draw spill, +0x58 case-2 step) against the target's one +0x54 slot (2ld 2st). The LoadParticleMaterial jal still takes a load in the delay slot instead of the material `%lo`. Next is a source form that overlays those homes at +0x54 without growing the frame or re-extending the `particle` web through case 2.
<!-- plateau-handoff:func_overlay_046_F0000874_188EC6C:end -->
