<!-- plateau-handoff:func_overlay_058_F00005FC_18AF7E4:start -->
### `func_overlay_058_F00005FC_18AF7E4` plateau handoff

- source: `src/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c`
- score: 109/829 words
- frame: 0x88
- relocations: 278
- first mismatch: +0x41C
- summary: Plain -O2; D_2C0 compares via entry pointer; frame-order locals, z offset temp. Open: case-3 table web joins loop web (s8 only), s1 cam.

Summary before this remeasure: Plain -O2 (no -g3); &D_2C0 compares via entry pointer drop s8; frame order. Open: s0/s1 address webs, quad offsets.

#### 2026-10-02, lane q-ovl9: 509 at +4 to 137 at +4, frame 0x88

- The per-file `OPT_FLAGS := -O2 -g3` override is removed (checklist item
  11). The target fills the first `jal` delay slot with the parameter home
  store and the `beqz` before the switch jump with its `sll`; `-g3` emits
  neither. The overlay 58 whale matched at plain `-O2`. Alone the flag
  change reads 509 to 808 positional (aligned byte-exact 438 to 367), so it
  was only adoptable together with the edits below.
- `D_54 = 0` in both arms of the case 4 mode test: the target's `b` carries
  that store in its delay slot (2 structural words).
- The ninth saved web was the address of `D_2C0`. Decision records
  (identity-gated instrumented IDO, proc 0): its web had totalsave 60 and
  save 5.0, ahead of 12 (4.17), -1 (4.22) and geometry (3.92); the s8 cost
  is 46. With four direct references (totalsave 40, save 3.33) it is offered
  s8 and split instead. Reading the two `== 1.0f` / `< 1.0f` comparisons
  through `f32 *progressPtr = &D_2C0` taken at entry does that, and the
  sound-call argument then reloads after the join as in the target. Volatile
  `D_2C0` reproduces the loads but keeps the address web (and adds an
  addiu per access). Measured with a progress local for the clamp
  (`progress = D_2C0 + increment; D_2C0 = progress;`); `D_2C0 += increment`
  brings the extra reference back (811).
- Menu bits and the screen-mode nibble are one ROM-table symbol (addends 0
  and 0x13), now one struct; the nibble is a word load as in the target.
- Case 3 row addressed as `(&table[0][0] + player * 4)[active + 1]`: 333
  to 162 (register ring). Unused `verts`, `buttons`, `mode` removed and
  status declared third so the homes land at 0x7C and 0x58: 162 to 137.
- Open, with numbers: the case 3 table base (target s0 across
  func_800291B4) and the `mainChangeCameras` mode address (target s1)
  are rematerialised here; reading the mode global at each use, array or
  struct declarations for it, and struct-row / pointer-row / plain forms for
  the table are inert. The large-point-quad offsets convert before the
  vertex loads in the target: two f32 temporaries do it (size delta 0, 133)
  but take two frame slots (0x90); dropping `selection` frees one,
  dropping `command` costs 140 words. The `D_2BC == 0` load is scheduled
  later than the target's; swapping the `&` operands is untested.

- Same day, second bank: 137 at +4 to 109 at size delta 0. The z offset
  of the large point quad as an f32 temporary converts both offsets before
  the vertex loads (removes the hazard nop); `selection` read at each use
  (CSE'd to one load) frees the slot it needs. Two temporaries (x and z)
  give 133 at delta 0 but frame 0x90. `(*progressPtr < 1.0f) & (D_2BC ==
  0)` is byte-identical; `!D_2BC & ...` is 248.
- Records on the 109 shape: the case 3 `&SelectionTable` address web spans
  from case 3 into the drawing loop (26 blocks, totalsave 7) and is offered
  only s8 at cost 46, so it splits; the target keeps a case-3-only web in
  s0. The loop's table address is the same LDA, which is why every
  spelling of the case 3 access alone is inert.

Summary before this remeasure: Vertices read as geometry->vertices at each use (536 to 511), decl hill climb 509, size delta -12 to +4; ninth saved web (s8) remains, frame 0xA0 against 0x88.

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
