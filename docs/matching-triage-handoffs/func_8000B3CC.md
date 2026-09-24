<!-- plateau-handoff:func_8000B3CC:start -->
### `func_8000B3CC` plateau handoff

- source: `src/main/objects.c`
- score: 116 differing words
- frame: 0x98
- relocations: 20
- first mismatch: +0x148
- summary: Hypothesis +0x2C/+0x30/+0x34. Block scope 116 d0 unhomed; split 116 d0 folded; aggregate 197 d+16 (+0x2C 1ld +0x30 3ld). Stall: speed +0x4C, step missed +0x34.

Summary before this remeasure: Size exact after L144 address-form homes on speed and dot. Next: step +0x34, speed +0x30, factor +0x2C, then reflection schedule and s0/s2.

The retained C is now size-exact: 494 words (1976 bytes), delta 0, frame 0x98,
20 relocations, 116 raw/masked differences, first +0x148. Displacement tax
fell from 69 to 5. Aligned: 388 exact, 86 naming, 5 immediate, 20 structural.
Five candidate-only words at +0x3E4 +0x560 +0x5AC +0x5C4 +0x610 pair with five
target-only words at +0x3DC +0x540 +0x54C +0x5A0 +0x5BC. Still NON_MATCHING,
no matching credit.

L144 address form on the speed divide (`*(f32 *)&speed`) and on the
reflection negation (`-*(f32 *)&dot`) is what closed the +12 size deficit.
The dot home is now +0x58 with 0 loads and 1 store, matching the target.
The speed eq sequence is store plus two reloads, but at +0x4C not +0x30.
Step is still at +0x30 (target +0x34). Factor has no home (target +0x2C,
3 loads 1 store). Candidate has 25 slots, target 26.

Identity-gated instrumented IDO: stock and instrumented .text are
byte-identical. Procedure ordinal is 60 (32 p1 decisions, 16 float). Forcing
`p1:w204=s` on the dot-only L144 form scored 204 at delta -4 and restored the
speed spill; the source address form on the divide then reached delta 0 at 116
without a force.

Closed on this shape, all worse or flat: declaration reorder of step/speed
(122 masked, delta 0), factor address form (307 masked, delta +20), volatile
speed (226 masked, delta -4), hoisted speed assignment (232 masked, delta +8),
start[0] via config (481 masked, lost the config copy), split if/else-if zero
tests (224 masked, delta +12), speed[1] array (220 masked, delta -12),
reversed equality operands (byte-identical), first sqrtf stored only to
state->unk18 (byte-identical). Do not add volatile fields.

Next: put step at +0x34, speed at +0x30 and factor at +0x2C without undoing
the L144 homes; then the five reflection scheduling pairs (target has the mul
nop and three velocities live; we reload) and the s0/s2 naming. Proc 60.

Validation includes configured full-TU comparison, identity-gated IDO with
CDX_PROC, force lattice on proc 60, finalize_plateau.py, check-docs and
cleanroom. ROM verification continues to use the assembly fallback.

<!-- plateau-handoff:func_8000B3CC:end -->
