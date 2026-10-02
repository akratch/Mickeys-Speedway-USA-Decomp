<!-- plateau-handoff:func_overlay_058_F00005FC_18AF7E4:start -->
### `func_overlay_058_F00005FC_18AF7E4` plateau handoff

- source: `src/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c`
- score: 509/829 words
- frame: 0xA0
- relocations: 269
- first mismatch: +0x0
- summary: Vertices read as geometry->vertices at each use (536 to 511), decl hill climb 509, size delta -12 to +4; ninth saved web (s8) remains, frame 0xA0 against 0x88.

Summary before this remeasure: Pair 5 extra-ILOD (-12), lines 514, 525, 548; pair 1 s8 line 207. Split D_1A0 cursor; marker=-1 inside bit test. Stall: neg1, volatile scale, per-draw reloads.

Summary before this remeasure: The +4 size delta is a cancellation, not a findable instruction: the aligner puts a 2-word insertion at candidate +0x0 where the candidate saves s8 and the target does not, which is +8 on its own, and the body is -4 against it; the candidate frame is 0xA0 against the target's 0x88, 24 bytes and not the 16 previously recorded, so the ninth callee-saved web is the whole lever. Removing the named geometry local is byte-identical, so the stack park is a uopt temporary and no rename reaches it. Block-scoped Overlay58Vec3f *v = geometry->vertices in each of the five drawing blocks moves the aligned split from 314/301/230 to 418/265/162 and the positional residual from 722 to 551, but takes the size to +28 and the frame to 0xB0; it is recorded as a fork, adoptable only after the callee-saved surplus is solved.
#### 2026-10-02, lane g-ovl5: 536 at -12 to 509 at +4

- `geometry->vertices[...]` read at each use (the cached vertex pointer
  removed): 536 to 511, size delta -12 to +4. Declaration hill climb: 509.
- Register reading after the change: start, end and stage are s1, s0, s2;
  status, increment and the end offset are stack homes, as in the target. The
  only structural surplus is one extra saved web: `&D_2C0` is hoisted (s5 or
  s6 depending on form) next to `12` and `geometry`, so geometry lands in s8
  where the target has it in s7 and shares s6 between `&D_2B0` and `12`.
  The target does not hoist `&D_2C0` (a fresh lui at each of six references).
- A local `progress = D_2C0 + increment; D_2C0 = progress; if (progress > ...)`
  drops the reload after the store (513 at -4) and moves `-1` to s5 and
  `&D_2B0` to s6 as in the target, but `&D_2C0` still takes a register.
  Other spellings measured: `&&` for the bitwise and (533 at -8), increment
  inline (500 at +8), increment after the call (501 at +8), `status` volatile
  (857 at +188). Decision variable: what keeps the `&D_2C0` web below `12` in
  colouring order.
<!-- plateau-handoff:func_overlay_058_F00005FC_18AF7E4:end -->
