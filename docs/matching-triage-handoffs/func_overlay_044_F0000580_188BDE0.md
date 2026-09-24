<!-- plateau-handoff:func_overlay_044_F0000580_188BDE0:start -->
### `func_overlay_044_F0000580_188BDE0` plateau handoff

- source: `src/overlays/o044/func_overlay_044_F0000580_188BDE0.c`
- score: 304 differing words
- frame: 0x100
- relocations: 7
- first mismatch: +0x8
- summary: Line 151 macro split was inert. arg0 address-read: 338 to 304, delta 0, unreloaded home store plus a2 copy. Stall: s32 reorder misses 0x64/0x58/0x50.

Summary before this remeasure: Frame 0x100 exact. Size -4 (348 vs 349). Incoming pointer web stays in a0; a2 copy and a0 home store are the missing word plus the 0x100 slot.
- assignment base: `cbaed235`
- owned range: overlay 44 `+0x580..+0xAF4`, 1,396 bytes / 349 words
- fresh baseline: candidate is 1,392 bytes / 348 true instructions with 338 of 349 positional words differing, size delta -4; first mismatch `+0x8`
- frame proof: candidate `0x100` versus target `0x100`. Saved s0-s8 and ra at `0x18` through `0x3C` match. High homes `+0xFC` (width) and `+0xE4` (y-prev) match. Extra command-word homes sit at `+0xA8`/`+0xAC`/`+0xB0` instead of `+0x40`/`+0x54`/`+0x64`. Target-only `+0x100` is the incoming a0 store (0ld 1st).
- size proof: one missing instruction. Target prologue copies a0 to a2 after mtc1 of the GPR-passed f32 and stores a0 at `0x100` in the beqz delay slot. Candidate keeps a0 and has no `0x100` store.
- new mechanism evidence: extra declared mips_to_c s32s were the 48-byte surplus (L134). Five used-but-colored working s32s declared between width and y-prev placed those homes at `0xFC`/`0xE4` without growing the frame (L99). Unused pointer pads grew the frame `0x100` to `0x118`. L144 `*(T **)&arg0` creates the `0x100` slot but with 3ld (size +8). L109 OR-with-zero on arg0 emits `or v1, a0` and cuts size -8 to -4.
- identity gate: instrumented IDO `.text` is byte-identical to stock; proc=0; 54 p1 decisions, 0 p2. Force `p1:w2=c5` accepted (forced=-1), 338 to 288 at size delta 0 versus this base. Web 2 (incoming pointer, save 1, nocs 1) costs 0.0 on a0 and 0.1 on a2, so globalcolor keeps a0. Do not run a colour landscape until size is 0 (L155).
- retained source: this candidate is best
- blocker: size delta -4 precedes colour/L159. Next is a source form that colours web 2 as a2 without L144 reloads, emitting the a2 copy and the unreloaded a0 home store, then pack the three mid command-word homes down to `0x64`/`0x54`/`0x40`.

Cycle 0 named pair 6, open to the end, shadow 30, a target-only alu on line 151 (the texture-rect shift). Splitting that macro onto later lines did not pin the shift and left the score at 338, delta -4. Replacing the or-with-zero with `state = *(Overlay44AnimationState **)&arg0` emits one unreloaded store at the incoming home and a copy into a2. score_symbol: 304 differing words, first mismatch +0x8, size delta 0, frame 0x100, 7 relocations. frame_census: the three command homes are still +0xAC/+0xA8/+0xA4 rather than +0x64/+0x58/+0x50. Reversing the eight block s32s, putting the float three first, and hoisting those three to function scope moves the high cluster (to +0xC0 or +0xE0) and does not land the target ladder. No colour sweep; size was off until the address-read.
<!-- plateau-handoff:func_overlay_044_F0000580_188BDE0:end -->
