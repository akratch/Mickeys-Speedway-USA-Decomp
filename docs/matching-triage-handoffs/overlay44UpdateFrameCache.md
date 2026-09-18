<!-- plateau-handoff:overlay44UpdateFrameCache:start -->
### `overlay44UpdateFrameCache` plateau handoff

- source: `src/overlays/o044/overlay44UpdateFrameCache.c`
- score: 18/187 words
- frame: 0x48
- relocations: 4
- first mismatch: +0x1F4
- summary: L99 packing closed the extra spill slot (30 to 18, homes exact). Remaining 18 is ugen ring naming at +0x1F4; colour stops at t4.

#### 2026-09-17, lane w8-o044: size closed; colour floor is 30

Arrival was ranking 748 B, delta -4, masked 133, first +0x0, candidate frame 0x40 versus target 0x48, one word short. Aligned buckets were 139 exact, 11 naming, 12 immediate, 29 structural, with displacement tax 81. Candidate-only words sat at +0x68 +0x208 +0x214 +0x290; target-only at +0x198 +0x1FC +0x200 +0x284 +0x288. Frame census: candidate 7 slots, target 6.

The missing word and the eight-byte frame were the same two source-shape facts:

- An unused pointer declared first (L99) grows the frame from 0x40 to 0x48. Declared last it is eliminated and the frame falls back to 0x40. An unused s32 array of length 2 is byte-inert on the 30-word score and does not fix the extra spill slot.
- `nextSlot = -1` belongs after the wrap of `nextFrame`, not hoisted into the 0x80-flag delay. That swap was the +0x68 versus +0x198 one-word pair.
- A block-local `src = (frame * source->frameSize) + source->data` before the cached-frame store is what puts the multiply in the delay-slot shape and closes the jal nop. Inlining `src` (L160) reopens delta -4 and scores 57.

Retained form: 187 words, delta 0, frame 0x48, masked 30, displacement tax 0, aligned 157 exact / 18 naming / 8 immediate / 4 structural. First naming +0x1F4, first immediate +0x218, first structural +0x2A0. Four candidate relocations (HI16/LO16 of the source table and two upload jals); both jals now sit at the target offsets.

Identity gate: instrumented IDO `.text` is byte-identical to the stock object, both score 30 at delta 0. Procedure 0, 20 p1 decisions. Exhaustive `--every-colour` landscape: 157 probes over 20 webs, out directory untracked. No same-kind force beats 30 at delta 0. Nine forces are inert at 30 (web 80 at c3-c6, web 121 at c1/c7, web 142 at c1/c8/c9). L159 packing therefore has nothing to add; L160 does not apply because no force scores 0.

Eliminated on this body, all at the 30-word shape unless noted:

- named `size` local before `src`: 33 masked, naming 21
- `source->frameSize * frame` versus `frame * source->frameSize`: byte-identical
- typed unused (`Overlay44FrameSource *`): byte-identical with `void *`
- named `data` pointer before `src`: 68 masked, delta +4

The remaining 30 is the first-upload register assignment plus stack homes. Candidate still has 7 slots against the target's 6: unique candidate homes at +0x28 +0x2C +0x40, unique target homes at +0x34 +0x38. The 8 immediate rows are the first-call spill offsets. The 4 structural rows are the second-call pair, which spills frameSlot then slot; the target spills slot then frameSlot and reuses two first-call homes. Colour does not move those offsets. The next lever is a source-authentic home packing that drops the extra spill slot, not another colour probe.

#### 2026-09-18, lane w21-o044: extra spill slot packed; 18 naming remain

Arrival reproduced 748 B, delta 0, masked 30, frame 0x48 both, 7 slots vs 6. Identity gate PASS, CDX_PROC=0, 20 p1. First-call spilled t4 t3 t1 v1 to the extra homes; second-call spilled frameSlot then slot onto a new slot. Colour landscape on that shape is void for packing.

L99 packing that closed the extra slot, 30 to 18, homes identical to the target (ladder +0x38 +0x34 +0x30 +0x24 +0x1C +0x18, 6 of 6):

- `source` declared last (t4 stays at +0x24).
- `frame` parked below the spilled cluster (`nextFrame`, `frameSlot`, `nextSlot`) so those three sit at +0x38 +0x34 +0x30.
- `frameSlot = slot` and `nextSlot = slot` moved before each upload so both values are live across the first call and the second call reuses those homes, spilling slot then frameSlot.

Unused pointer remains first (frame 0x48). Coalescing `limit`/`delta` onto the slot names dropped words (delta -4 or -8). L112 pads either matched the unused pointer or grew the frame. Function-scope `src` did not drop the extra slot.

Aligned after packing: 169 exact, 18 naming, 0 immediate, 0 structural. First +0x1F4. The 18 is t5/t9/t7/t8/t2/t3/t6, 7 incoherent windows, not one ring phase. p1 colours stop at t4; t5-t9 are ugen ring. Forces of w7/w88/w74 onto t5 (c12) accepted and scored 106/145/113 at delta -4. nonvolatile source, `handles[frameSlot]`, line-folds, L109 nested OR-zero, and function-scope `src` did not beat 18. Next is a ring-draw / emission-order lever at +0x1F4, not another home permutation or same-kind colour force.

<!-- plateau-handoff:overlay44UpdateFrameCache:end -->
