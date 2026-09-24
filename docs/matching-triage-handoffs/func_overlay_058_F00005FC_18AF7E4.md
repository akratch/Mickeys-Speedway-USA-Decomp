<!-- plateau-handoff:func_overlay_058_F00005FC_18AF7E4:start -->
### `func_overlay_058_F00005FC_18AF7E4` plateau handoff

- source: `src/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c`
- score: 536 differing words
- frame: 0xA0
- relocations: 269
- first mismatch: +0x0
- summary: Pair 5 extra-ILOD (-12), lines 514, 525, 548; pair 1 s8 line 207. Split D_1A0 cursor; marker=-1 inside bit test. Stall: neg1, volatile scale, per-draw reloads.

Summary before this remeasure: The +4 size delta is a cancellation, not a findable instruction: the aligner puts a 2-word insertion at candidate +0x0 where the candidate saves s8 and the target does not, which is +8 on its own, and the body is -4 against it; the candidate frame is 0xA0 against the target's 0x88, 24 bytes and not the 16 previously recorded, so the ninth callee-saved web is the whole lever. Removing the named geometry local is byte-identical, so the stack park is a uopt temporary and no rename reaches it. Block-scoped Overlay58Vec3f *v = geometry->vertices in each of the five drawing blocks moves the aligned split from 314/301/230 to 418/265/162 and the positional residual from 722 to 551, but takes the size to +28 and the frame to 0xB0; it is recorded as a fork, adoptable only after the callee-saved surplus is solved.
<!-- plateau-handoff:func_overlay_058_F00005FC_18AF7E4:end -->
