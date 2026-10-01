<!-- plateau-handoff:overlay1ChoosePath:start -->
### `overlay1ChoosePath` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 0/446 words, promoted
- frame: 0x90
- relocations: 63
- first mismatch: none
- summary: Matched. A separate variable for the selection maximum, FindType47ByAngle's result in object, and for-break count-down loops; promoted as overlay1ChoosePath.

Formerly `func_overlay_001_F0003750_184FB30`; the C definition took the
friendly name when it was promoted.

Summary before this remeasure: 139 to 63 at delta 0 by dropping inherited carriers; the rest is a0/a1 naming of scores-pointer vs table/best webs and chosenState in v1.

Summary before this remeasure: Delta +4 closed and frame ladder exact, 341 to 139 at delta 0; the rest is naming led by the transition weight's f0 split.

Summary before this remeasure: Direct selector loads remove the redundant path web and improve 360 to 341 masked differences without changing the 447/446 size. Linked trial has zero measured diffs but remains 12 bytes long.
- summary: Direct selector loads remove the redundant path web and improve 360 to 341 masked differences without changing the 447/446 size. Linked trial has zero measured diffs but remains 12 bytes long.

#### tu2-o1tail: measurement only, plus the tool that cannot see this function

Re-measured: 341 masked words, 447 instructions against the target's 446
(+1), 0x90 frame exact, prefix exact to row 11, 64 relocation sites naming a
different symbol on each side. No source attempts were made here; the lane's
budget went to the near-miss rows.

Tooling: `tools/wb_compare.sh` cannot measure this function at all. The
preflight resolves the symbol through the Makefile's POSTPROCESS
`--redefine-sym` map to `overlay1ChoosePath`, then fails with "does not select
one C definition for overlay1ChoosePath" because the C body is still named
`func_overlay_001_F0003750_184FB30`. The same fault blocks
`func_overlay_001_F000438C_185076C` (mapped to `overlay1UpdateObjectPhysics`).
Either rename the C bodies to the friendly names the POSTPROCESS already
assigns, or teach the preflight that a redefine-sym destination may have no C
definition under that name. Until then the measurement route is to assemble
`asm/nonmatchings/.../<splat name>.s` by hand and run `decomp-workbench
diagnose` against the NON_MATCHING object directly.
#### B3-o001 (Track B, 2026-09-23): 341 at delta +4 to 139 at delta 0

Cycle 0 read one open pair from +0x264, labelled missing-CSE, owned by the
`if (i-- != 0)` guard and the object loop; `frame_census.py` showed the same
0x90 frame with a different home ladder. Measured steps, in order:

- declaration order to the target ladder (`i` first at 0x8C, `scores` at
  0x60, the temporaries at 0x48/0x44): 341 to 321, ladder equal but for two
  spill slots.
- the four `D_E8`, `D_EC`, `D_F0`, `D_F4` externs are rodata literals: the
  relocation records name the overlay-local base 0x8230 (the same base the
  `overlay1FindType47ByAngle` handoff authenticated), and base plus addend
  reads -1.2f, 400.5f, 0.1f, 0.1f. Literals stop uopt hoisting three address
  webs out of the object loop: 321 to 285 (delta -4).
- the object loop guard as `loopValue = i; i--; if (loopValue != 0)` (the
  target tests the old count directly, ours materialised an `sltu`) and the
  cursor decremented at the loop bottom rather than in `*cursor--`. The frame
  then shrank by 8; one unreferenced `s32` at the end of the declarations
  restores 0x90 (L99 corrected): 285 to 291 but structurally closer.
- the step selection as an if/else on `otherState->selector` with `value`
  assigned from the same expression after it: the target's `move` of the
  loaded selector into a separate web. 291 to 186 at size delta 0. The
  reader's missing-CSE label was, as the protocol predicts, a copy the target
  keeps and the candidate had folded.
- `i`/`selected` and `chosenState` declarations swapped to put the spills at
  0x88 and 0x3C: 186 to 182.
- the FindChoice sentinel passed as the literal and `value` set to it just
  before the selection loop reproduces the target's rematerialised constant
  (lui/ori after each call) but reads 220 at delta -4: the lost word is an
  as1 branch-likely conversion at the 0.1f test that the target gets because
  its state pointer is in a0 (so the SubmitChoice argument's `lui a0` cannot
  be hoisted). Forcing that web to c3 scored 139 at delta 0.
- that force's source form: FindChoice's result assigned to `object` instead
  of `found` (L115, carrier reuse). 220 to 139 at delta 0 with no force.
  Removing the `chosenState` carrier instead (L160) was 233 at delta +4.
- `weight` declared at 0x40 so its spill slot is the target's: frame ladder
  exact, score unchanged at 139.

What is left (139, first mismatch +0x58) is register naming. The first
window is the transition path: the target computes the weight product into a
ring temp, stores it to 0x40 and reloads into f0 after both InterpolatePath
calls, where ours colours the product f12 and spills it. Using `difference`
as that path's carrier scores 134 but moves the spill to 0x30, off the target
ladder, so it was not adopted. The FP ring downstream (ft0..ft3 rotated one
position) follows from that first decision. Next: read the p1 records for the
weight web (proc 2) and ask what makes its pre-call segment a split rather
than a colour.

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0xC.
#### 2026-10-01, lane d-o001: 139 to 63 at delta 0 by dropping inherited carriers

Every step is masked words at size delta 0 with the 0x90 ladder exact,
measured with `tools/fast_score.py` in products of natural alternatives:

- the score decay's absolute value as a ternary instead of the `weight`
  if/else carrier: 139 to 129.
- the object loop indexed (`object = objects[i]; otherState = object->state`)
  instead of the hand-walked `cursor`: with `found` for FindChoice's result
  and loop 3's zero spelled `0`, 129 to 122. `otherState = objects[i]->state`
  without `object` is 237 (the load lands in a ring temp, the target keeps it
  in v1). The cursor also hid the one-register shift of every object-loop web.
- MeasureChoice's first argument inline rather than through `weight`:
  122 to 78. The transition block is then byte-exact.
- the transition step compared and subtracted as `D_1D94 * 8` with no
  `value` carrier: 78 to 72.
- the selection loop as `for (i = 0; i < 8; i++)`: 72 to 63.

Measured and not adopted: every `while (i--)` or `i = 8; while (i--)`
replacement of a `loopValue` guard (125 to 176 in a 32-cell product);
reading D_1D68 at each use in the redirect block (133); one symbol for
D_1D68Read (+16 bytes); `object` reuse for FindChoice's result now costs 4
bytes; a `best` local for the selection maximum (66 with object reuse, 151
without); spelling every zero the same way (85 whether all `0` or all
`0.0f`; loop 3 must differ from the object loop).

What is left (63): register naming in a0/a1. The scores-pointer webs of
loop 3, loop 5 and the selection loop take a0 in the target and a1 here,
and the table and best-score webs the reverse; chosenState lands in v1 where
the target has a0, which also costs as1's branch-likely at the 0.1f test
(the +0x4FC/+0x578 pair). Next: read the p1 records (proc 2) for the loop-3
pointer web and check whether it is offered a0 at all (L142: a web live
across GetChoiceObjects is denied a0).

#### 2026-10-01, lane d-o001 (continued): matched and promoted, 63 to 0

The p1 records named the a0/a1 residual. Web 155, the one variable
`value`, served both the object-loop index and the selection maximum and so
was a single web, coloured a0 (save 132.75, nocs 4); the target has the
index in a0 and the maximum in a1. Webs 151 (`step`) and 98 carry a
forbidden0 mask that denies them a0, so no force reached the target layout
from that source. Measured (masked words, delta 0):

- the selection maximum in its own variable (`step`, or a new `best`): 63
  to 4, but only together with FindType47ByAngle's result going to `object`
  (with `found` it is 110 at -4 bytes; `object` alone was 152 at +4).
- the last 4 words were the `i = 7` store before loops 5a and 5b scheduled
  ahead of the hoisted D_1DA0 load and the -1000000 constant. Neither moving
  statements nor a local copy of D_1DA0 changed it; writing those loops as
  `for (i = 7; ; ) { ...; loopValue = i; i--; if (loopValue == 0) break; }`
  did (4 to 2 for loop 5a, 2 to 0 with loop 5b). The score initialiser takes
  the same form (neutral, adopted for consistency); the redirect loop and
  the object loop stay `do`/`while` (the redirect loop in `for` form is 50).
- callee identities from the module relocation table: +0x30 resident
  0x8002A8BC, +0x260 resident func_80005750, +0x588 overlay 36 +0x1470
  (overlay36CallModeZero, placeholder overlay1ModeAction6), and
  FindType47ByAngle takes one `f32`. All four changes are neutral at 0.

Promotion: the C definition is renamed to the friendly name overlay1ChoosePath
(function_preflight resolves the symbol through the POSTPROCESS rename and
found no C definition under the generated name); atlas range 0x3750..0x3E48.
The TU's .rodata is now ChoosePath's four literals followed by the
DispatchMode jump table, so the POSTPROCESS rule binds the literal pairs to
`gOverlay1ChoosePathLiterals = 0xE8`, moves `gOverlay1ModeTable` to
0x164 - 0x10 = 0x154, and asserts the new pool digest. `gmake verify`,
`check-overlay-syms` and `promotion-proof` (446 words, frame 0x90,
relocations 63/63) pass.

<!-- plateau-handoff:overlay1ChoosePath:end -->
