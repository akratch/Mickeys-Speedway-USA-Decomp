<!-- plateau-handoff:overlay25UpdateEffect:start -->
### `overlay25UpdateEffect` plateau handoff

- source: `src/overlays/o025/overlay_025.c`
- score: 82/259 words
- frame: 0xA0
- relocations: 25
- first mismatch: +0x3C
- summary: Frame and size closed. Homes still 0x14/0x24 high (objects 0x70 vs 0x4C). Colour floor 69 via p1:w122=c18 (s3->s4); L160 delete-other regresses.

#### 2026-09-17, lane w10-o025: frame closed at 0xA0; colour floor is 69

Arrival was 1,036 B / 259 words, frame 0xC8 vs 0xA0, 127 masked, first +0x0.
`align_symbol.py` read size delta 0, 138 exact / 95 naming / 8 immediate / 20
structural, candidate-only +0x54 +0x26C, target-only +0x48 +0x27C.
`frame_census.py`: 15 slots each; hitSomething traffic at candidate 0x88 vs
target 0x98; objects array 2& at target 0x4C vs 1& at candidate 0x8C.

Levers that moved the residual, all at size 1,036 / 259 words:

- Deleting decompiler register temps (`velocityX`/`velocityZ`, `extraSteps`,
  walking `cursor`, `queryRadius`, `ownerState`) and writing `2.0f *` for the
  doubled transform value: 127 to 86, frame 0xC8 to 0xB0.
- L112: `objects[6]` solves the target frame 0xA0 from that 0xB0 body
  (`0xB0 - 0xA0 = 16 = 4 pointers`). Score 84.
- L99: one unused pointer declared first homes `hitSomething` at 0x98 without
  growing the frame (it occupies the 4-byte top pad). Score 82. A second unused
  pointer grows the frame to 0xA8; five more grow it to 0xB8.

Aligned at the 82-word shape: 177 exact / 67 naming / 8 immediate / 7
structural, displacement tax 0, no extra/missing words. Candidate homes:
+0x98 hitSomething (matches), +0x94 radius, +0x88..+0x90 position, +0x70
objects[6]. Target: +0x98 hitSomething, +0x80 radius, +0x74..+0x7C position,
+0x4C objects (40-byte gap).

Identity gate: instrumented IDO scores 82 at delta 0 (same as stock).
Procedure 1 of 3, 31 p1 decisions, 25 coloured webs. Exhaustive
`--every-colour` landscape: 154 probes, out directory untracked. One force
beats 82 at delta 0: `p1:w122=c18` (s3 to s4, same 12.5 callee cost) scores
69. L159 packing is that singleton, predicted 69. Web 132 is inert at 82
across c25-c29.

Eliminated on this body:

- `objects[10]` at the 82-shape declaration list: frame 0xB0, 86 masked
- inlining `other` as `objects[index]` (L160 on web 122): size delta -4, 235
  masked, loses the s0 save
- reusing `hitSomething` as the movement countdown: size delta +4, 258 masked
- inlining `otherState` through `other->state->entity`: size delta -4, 246
  masked
- inlining `delta` as two `other->y - object->y` compares: size delta +4, 145
  masked

Next: a source-authentic way for web 122 to take s4 while keeping `other`
(the 69-word force), and/or 0x24 of unused-as-memory homes between position
and objects so the array starts at 0x4C without growing the 0xA0 frame.
Colour will not move those stack offsets.
<!-- plateau-handoff:overlay25UpdateEffect:end -->
