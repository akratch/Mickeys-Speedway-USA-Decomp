<!-- plateau-handoff:func_overlay_044_F0000580_188BDE0:start -->
### `func_overlay_044_F0000580_188BDE0` plateau handoff

- source: `src/overlays/o044/func_overlay_044_F0000580_188BDE0.c`
- score: 13 differing words
- frame: 0x100
- relocations: 7
- first mismatch: +0x188
- summary: Rewrite from listing, 304 to 13 at delta 0. Stride temp loses a0 to xh/dsdx on web number; forced colours score 5 (schedule only).

Summary before this remeasure: Line 151 macro split was inert. arg0 address-read: 338 to 304, delta 0, unreloaded home store plus a2 copy. Stall: s32 reorder misses 0x64/0x58/0x50.

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
#### 2026-10-02, lane w2-capbuf: rewrite from the listing, 304 to 13 at size delta 0

The inherited m2c shape (precomputed command-word locals, `*(T **)&arg0`)
was discarded. The function is a strip loop like JFG `screen.c`
`screenDraw` (semantic relative only): two gDPLoadMultiBlockS loads
(tile 1 at TMEM 0x100 from `handles[protectedSlot1]`, tile 0 at 0 from
`handles[protectedSlot0]`), one gSPTextureRectangle with the RDPHALF pair,
then texDPInit (`func_80034920`) and white prim/env colours. Measured
steps, masked words at size delta:

- Natural rewrite, packet macros on `(*dl)++`: 328 at -20.
- `alpha` an s32 masked with 0xFF (a u8 local emits `lbu 7`), a `stride`
  local: 267 at -4. The incoming-a0 home store and the a2 copy appear
  with no address-read trick.
- Frame-source fields read through the global at each use instead of a
  `source` pointer local: 255 at 0, frame 0x100 exact (13 declared
  locals, width first at 0xFC, yPrev seventh at 0xE4).
- `yPrev = y` written straight after `y`: yPrev's range then starts in the
  block where y is defined, 18 blocks, nocs 6, save 3.5, below the seven
  loop-constant webs (4.0), so 0xF2000000 takes fp and yPrev spills to
  0xE4 as shipped. Writing it after the scale multiply (one block later,
  nocs 5, save 4.2) took s2 from the last constant.
- Frame-source reads before the handle reads (24-order product): 43.
- An unsigned stride (`(u32)width * 2`, or `width * sizeof(u16)`): 15. The
  shipped preheader carries a conversion copy of `width * 2` that the
  signed spelling never makes.
- `y = yPrev = state->valueA << 16;`: 13.

Residual (13 masked, 8 naming, 3 structural by the aligner): the stride
copy is a type-4 temp (web 117, bb 9, save 1.0) that ties xh (web 97)
and dsdx (web 105), both symbol webs spanning bb 8-9 at save 1.0, and
loses on web number, so it takes a3 where the ship has a0, with xh a2 and
dsdx a3. Forcing p1:w117=c3,p1:w97=c5,p1:w105=c6 (all accepted) scores 5,
schedule-only: the `scale *= 65536` multiply issues three slots later
than shipped. Flat, all at 13: return structure (4 forms), declaration
order (8), xh/dsdx/stride types, xh operand order, stride spelling (7
unsigned forms), stride position, pre-loop statement order (15 legal
permutations) and line folding (8).
<!-- plateau-handoff:func_overlay_044_F0000580_188BDE0:end -->
