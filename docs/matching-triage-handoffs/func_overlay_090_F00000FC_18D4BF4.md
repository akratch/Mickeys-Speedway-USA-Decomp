<!-- plateau-handoff:func_overlay_090_F00000FC_18D4BF4:start -->
### `func_overlay_090_F00000FC_18D4BF4` plateau handoff

- source: `src/overlays/o090/overlay_090.c`
- score: 0/648 words, promoted
- frame: 0xC8
- relocations: 58
- first mismatch: none
- summary: Matched. do-while state machine with the rate reset before its test and no default arm, -Wab,-r4300_mul, sound pitch reusing animationValue declared after delta.

#### 2026-10-02, lane w2-ovlc: 327 at -8 to 0, promoted

- The goto loop was the inherited shape. With a separate back-edge block
  (goto, `while (1)` with break, `for (;;)`, a `&&` loop test) uopt's PRE
  places the `state->active` load and `transitioned = 0` in that block, so
  the head never reloads. A do-while whose last statement is
  `updateRate = 0` has a critical back edge and reloads at the head, but
  uopt then hoists the reset into every case and the `default:` block (499
  at +16). Deleting `default: break;` sends the jump table's out-of-range
  branch straight to the loop test, the reset stays there and as1 puts it in
  the back branch's delay slot: 327 at -8 to 130 at -20.
- `-Wab,-r4300_mul`: the target's nop between the paired `owner->x`
  products is the VR4300 multiply workaround (one word).
- The target spills the sound pitch through 0x90(sp) across mathRnd and the
  two sound calls. Reusing `animationValue` (whose merged web interferes with
  every callee-saved float in the loop) reproduces the spill: 130 to 4 at
  delta 0. Its home is a declaration-order slot; declared after `delta`,
  with `transitioned` moved above `remaining` to keep delta at 0x94, it lands
  at 0x90: 4 to 0.
- Promotion: the TU is now all C (overlay90Initialize was already exact).
  The pool (six floats, the switch table, 0.003f) is the retained data at
  data_rodata +0x8; sixteen HI/LO records rebind to gOverlay90StatePoolReloc
  (0x8) and the duplicate is digest-checked and dropped (overlay 86's form).
  gmake verify OK, check-overlay-syms up to date, promotion-proof PASS
  (648 words, frame 0xC8, 58/58 relocations).

Summary before this remeasure: Reconstruct state-machine CFG and local lifetimes to add nine instructions while reducing non-save frame use by 24 bytes.

2026-10-02 (lane q-ovl10), priced on the fast direct compile:

- The module data words after overlay90Initialize's two are this function's
  float literals and its switch table; writing them as literals at each use
  (0.0167f in case 2) and declaring func_8005ABA8 with a return value (its
  v0 then no longer colours the flag webs, which take v1 as in the target)
  took 575 at -36 to 445 at 0 on a do-while shape.
- The goto shape is the target's (its tail `bne` carries the rate reset in
  the delay slot and the cases branch to it directly); with the remaining
  edits (case 1 `while (remaining--)`, case 5 rereads, case 2 displayIndex,
  unguarded `*owner->attachment`, the s16 sound pointer taken before the
  1024.0f scale, `value20 + -100.0f` after the radial call, locals in frame
  order for 0xC8) it measures 327 at -8.
- Open: uopt moves the switch's `state->active` load into the loop's
  predecessors (the target reloads it after the label; label position, a
  `transitioned` reset at the goto, and reading through owner did not
  remove it), and soundPitch takes f20 where the target splits it through
  0x90(sp).

Earlier maintenance evidence (base `f6d1bdbe8338886e3cce45818144bf68cbec5720`):

- Ownership is overlay 90 `.text` offset `0xFC..0xB1C`, exactly 2,592 executable bytes; the separate `0xB1C..0xB20` owner is padding and receives no credit.
- The configured guarded candidate uses IDO 5.3, `-O2 -mips2 -32`, and ABI `void (Overlay90Owner *, s32)`. Its exported entry at overlay offset `0xFC` has one resident `R_MIPS_26` inbound caller at VMA `0x8000B110`.
- Fresh V0 is 639 words (2,556 bytes), nine words short of the 648-word target. The candidate frame is `0xE0`; the target frame is `0xC8`. Save slots agree at 76 bytes, while non-save storage is 148 versus 124 bytes.
- Candidate and target each have 58 relocation records. Only 10 offset/type sites and one stable/effective identity align; 27 candidate static identities remain unresolved, so relocation identity is not exact.
- Workbench verdict is `structure-mismatch`: 575 positional/raw differences, 105 alignment gaps, and first mismatch `+0x0`. Donor scan found no credible sibling (best retained donor score `0.052`).
- Historical work already exhausted four 119-variant flag lattices, the `-O2 -mips2 -Wo,-loopunroll,0` form, and five coherent source rounds covering local/cache lifetimes, vector spelling, sound-pitch storage, loop inlining, and animation-value scope. No bounded permutation is justified without a natural strict gain.
- Next lever: reconstruct the state-machine CFG and the remaining local lifetimes together so IDO emits nine additional instructions while eliminating 24 bytes of non-save frame storage; isolated cache ablations previously reduced only eight bytes and worsened structure.
<!-- plateau-handoff:func_overlay_090_F00000FC_18D4BF4:end -->
