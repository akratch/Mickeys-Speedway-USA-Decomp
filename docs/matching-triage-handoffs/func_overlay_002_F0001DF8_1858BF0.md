<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:start -->
### `func_overlay_002_F0001DF8_1858BF0` plateau handoff

- source: `src/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c`
- score: 370/460 words
- frame: 0x880
- relocations: 5
- first mismatch: +0x38
- summary: First-loop lastCandidate copy only: 370 masked at size -4, exact 170. Leftover OR-zero/empty-if/comma emit 0 or 2 words, never 1.

Identity gate: instrumented IDO .text is byte-identical to stock tools/ido/cc on this TU.

Best source: lastCandidate = candidate in the first scan only, not the group-collect loop. Aligned exact 170, naming 223, immediate 5, structural 73, size -4, frame 0x880, slots 21 vs 20, first mismatch +0x38.

The extra word is not a declared walking cursor (frame grew to 0x888) and not a nested route-group test (size +8). (u32) on func_8000BCB0 is required: (s32) drops 30 words.

Measured size levers on the -4 shape, all 0 extra words unless noted:

- leftover OR-zero on count, closestIndex, distance, end, and lastCandidate as a pointer: copy-propagated
- index OR-zero inside the first loop: -16 (induction)
- overlay22 empty if (index) / if (lastCandidate) / if (1): inert
- overlay40 comma-assign of lastCandidate: byte-identical to a separate assignment
- L144 address-form end reload: inert
- volatile end reload: +2 words, exact 178
- explicit if (start < end) around the second loop: +2 words
- L160 dropping the first-loop candidate carrier: -28
- post-loop lastCandidate = candidate: -20 (copy-prop merges the names)

The one-word gap does not have a leftover that emits exactly one instruction. Next: a first-loop spelling that replaces the 3-word compare/copy cluster at +0x1D4 with the target's 2-word load/branch at +0x1AC without a second spill.
<!-- plateau-handoff:func_overlay_002_F0001DF8_1858BF0:end -->
