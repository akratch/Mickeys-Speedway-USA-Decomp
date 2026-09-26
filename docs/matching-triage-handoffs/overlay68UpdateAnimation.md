<!-- plateau-handoff:overlay68UpdateAnimation:start -->
### `overlay68UpdateAnimation` plateau handoff

- source: `src/overlays/o068/overlay68UpdateAnimation.c`
- score: 213/356 words
- frame: 0x78
- relocations: 15
- first mismatch: +0x1C
- summary: Pointer walk matches duration-loop shape at delta 0, 213/356, first +0x1C; state stays t1 not t2. Subscript walk grew 16 bytes, reverted. No colour sweep.

Summary before this remeasure: L160 named keyframes base puts object in t2 unforced at size 0 frame 0x78. Remaining blocker is duration-loop structure.

Pointer walk of the same trip count. The cursor is the addressed keyframe.
Each trip subtracts its duration, stores the advanced index, and steps the
cursor. The break arm clamps the index, clears elapsed and active, and
reloads the cursor from the animation. The fraction test is the cursor
duration's truth value, so the exit uses the duration already loaded.

A walk that still subscripted the samples afterward grew 16 bytes (the
scaled index was recomputed after the loop) and was reverted. The kept
spelling reads the samples through the cursor. It is size 0, frame 0x78,
15 relocations, 213/356 positional, first mismatch +0x1C. The loop shape
matches. State remains in t1, not t2, and the positional residual is that
naming shift. Counted-for and pre-decrement forms were not applied: outside
this TU they emit a shift-add index, not the multiply the loop needs. No
colour sweep.

Prior L160 named-base form put state in t2 at 214/356, first +0x34. The
break-arm reload and keyframeIndex store remain load-bearing for size 0.
Flag lattice and permuter stay closed.
<!-- plateau-handoff:overlay68UpdateAnimation:end -->
