<!-- plateau-handoff:func_8000B3CC:start -->
### `func_8000B3CC` plateau handoff

- source: `src/main/objects.c`
- score: 0/494 words, promoted
- frame: 0x98
- relocations: 20
- first mismatch: none
- summary: Matched. Typed object parameter, divisor and doubled negative dot as uopt spills (no carriers or address forms), one pad local, trigger-block store order.

Summary before this remeasure: Velocity sums read fields at each use (116 to 100, delta 0). Left: reflection keeps fields in f0/f14/f2/f12, factor homed, zero in f18.

Summary before this remeasure: hypothesis=put step at +0x34, speed at +0x30 and factor at +0x2C without undoing the L144 homes; spellings=block scope 116 unhomed, split stores 116 folded, two-field aggregate 197 at +16; stall=speed stayed at +0x4C and step missed +0x34, body reverted

Summary before this remeasure: Size exact after L144 address-form homes on speed and dot. The home order asked for below was measured on 2026-09-24 and did not land.

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

Tried 2026-09-24, not an open assignment: put step at +0x34, speed at +0x30
and factor at +0x2C without undoing the L144 homes and without repeating the
declaration reorder. Block scope left factor unhomed at 116 words, delta 0.
Split stores folded and stayed 116, delta 0. A two-field aggregate scored
197 at delta +16; it gained a home at +0x2C, but speed stayed at +0x4C and
step missed +0x34. The body was reverted to the 116-word form. Stall: the
three homes were not reached together at delta 0. The reflection schedule
and the s0/s2 naming were not started. Proc 60.

Validation includes configured full-TU comparison, identity-gated IDO with
CDX_PROC, force lattice on proc 60, finalize_plateau.py, check-docs and
cleanroom. ROM verification continues to use the assembly fallback.

#### 2026-10-01, lane d-obj: 116 to 100 at delta 0

The three squared-length sums (after the first collision move, after
damping, and the bounce speed) now read the velocity fields at each use
instead of copying them into the `volume`, `savedY` and `moveZ` carriers;
the carriers stay for the collision deltas passed to func_80008128, which
regress to 182 when inlined. Measured as a 16-cell product: any one sum
inlined is 102, all three 100, none 116. The L144 address-form homes are
untouched. Buckets: byte-exact 405, naming 70, immediate 5, structural 10.

Read from the target, not yet reached: the reflection keeps the three
velocity fields in f0, f14 and f2 and state unk8 in f12 across the factor
store, negativeDot is a coloured f16, and the zero constant therefore lands
in f18; factor is a home read at each use (one store, three loads). Floated
and measured, all worse: the reflection through the three carriers (216 at
+8), the factor address form with or without them (180 to 296, +8 to +20),
a plain dot read (+12 to +16), and integer or double zero spellings (+16).
The three D_8008152C/30/34 externs are this function's float literals (0.1,
0.707, 0.1; the two 0.1 entries need distinct spellings) and sit directly
after the objects rodata, so a promotion grows the trim by 0xC.

#### 2026-10-02, lane g-objB: 100 to 0, promoted

Priced in order, every cell at delta 0 unless stated:

- The reflection with no address forms and no factor or speed carriers:
  the divisor read as `state->unk18` at the test and the first divide, the
  doubled negative dot written inline at each of the three uses, dot read
  plainly. 100 to 30. The target's 0x30 and 0x2C cells are uopt spills of
  those two expressions, not declared homes; the address-form homes were the
  inherited artefact. Any one of the three alone was worse (141 to 214, some
  at +16), which is why the earlier single-axis attempts read as closed.
- Unused declarations as frame cells: dropping factor and keeping
  negativeDot declared, 30 to 28. The declaration count, not the names,
  decides the frame (renames of step were byte-identical).
- The f0/f14 swap on unk1C/unk20 (18 words) was the colour order of three
  tied p1 webs (save 6 each, ascending web number). Forcing them
  (w274=c24, w266=c27, w270=c25 on proc 60) priced it at 30 to 12. The
  three loads share one ICHAIN bucket and their chain order follows the
  expression hash of the base: with `object` as the typed parameter instead
  of a local cast from `void *arg0`, unk1C is first. End-statement order,
  sum term order and the += spelling were all measured inert on it.
- Removing the local moved every home 4 bytes; a leading `s32 pad` restores
  them, and dropping the factor pad then puts the three spills on the
  target's 0x34/0x30/0x2C ladder without sharing: 12 to 3.
- `object->unk10 = state->unk14` before the two angle clears: 3 to 0.

Promotion: the three D_8008152C/30/34 externs are now literals (the second
0.1 spelled `0.100000001f` so the pool keeps two entries), the objects
.rodata trim grew 0x800 to 0x80C, the carve boundary moved 0x8212C to
0x82138 and the orphaned extract was deleted. The prototype and the one
caller now name the typed object. The color_gate witness recipe and source
pins were repinned.
<!-- plateau-handoff:func_8000B3CC:end -->
