<!-- plateau-handoff:overlay68UpdateAnimation:start -->
### `overlay68UpdateAnimation` plateau handoff

- source: `src/overlays/o068/overlay68UpdateAnimation.c`
- score: 214/356 words
- frame: 0x78
- relocations: 15
- first mismatch: +0x34
- summary: L160 named keyframes base puts object in t2 unforced at size 0 frame 0x78. Remaining blocker is duration-loop structure.

L160: named `keyframes` base plus local `index`, generated subscripts, no
walking `current++`. Object (web 0) takes t2 unforced, size 0, frame 0x78,
15 relocations, 214/356 positional, first +0x34. Identity-gated `CDX_PROC=0`.
Forcing w0=c9 on the pre-base form scored 200 at delta 0 (accepted).

Break-arm `keyframes` reload and `keyframeIndex` store are load-bearing for
size 0. Dropping `duration`/`value` restored frame 0x78 after the generated
cursor added a temp slot. L109 last-declared OR-zero and L100
updateRate OR-zero did not recover the duration-loop structural deficit.
Flag lattice and permuter remain closed from the prior plateau.
<!-- plateau-handoff:overlay68UpdateAnimation:end -->
