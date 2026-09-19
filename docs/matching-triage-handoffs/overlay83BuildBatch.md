<!-- plateau-handoff:overlay83BuildBatch:start -->
### `overlay83BuildBatch` plateau handoff

- source: `src/overlays/o083/overlay83BuildBatch.c`
- score: 98/168 words
- frame: 0x80
- relocations: 5
- first mismatch: +0x0
- summary: o34 comma inert. dummy-after-linkedInit 91/0x88 home +0x58. batch-as-remaining frame 0x78 at 107. No size-0 net-zero slide.
- baseline: Exact 672-byte size and 168-word body. Inherited candidate was 97 masked at frame 0x90. This pass is 98 masked at frame 0x80; aligned byte-exact 126, naming 12, immediate 9, structural 26.
- identity gate: instrumented IDO `.text` is byte-identical to stock; proc=0. The 0x80 shape emits 13 p1 decisions.
- save set: candidate and target both save s0-s6, ra, f20, and f22 at the same slots. Extra frame is unused local-block homes, not extra callee-saves.
- retained gain: dissolving the inner scale, convertedScale, and world carriers (L134/L145) cut frame 0x90 to 0x88. Dissolving the u8 count local then cut 0x88 to 0x80 at score 98, size exact, save set unchanged. Linked initializer remains in its only conditional lifetime.
- last 8-byte home: `remaining` occupies it. A pointer-bound loop without that local hits frame 0x78 but grows size by 28 bytes and adds s7. `register` on remaining is byte-inert.
- scaleFactor: required to keep f22. Deleting it hits frame 0x78 with the wrong save set and one extra word.
- L99: an unused pointer declared first grows 0x80 to 0x88 at unchanged score, so unused pointer homes still charge on this TU.
- negatives on this shape: dropping allocated or walking allocated instead of output (score 105, frame unchanged); dropping batch (size +12); u8 remaining (size +16); L154 index shape (frame 0x88, size +36, extra fp and s7).
- colour landscape: 79 same-kind probes over 13 webs. Floor 89 at delta 0 from p1:w61=c11 (rival w61=c10, identical radius). No zero-scoring force. Colour cannot close the last 8-byte home.
- relocation proof: five records each side; three offsets/types align; all five candidate identities remain unresolved.
- donor evidence: JFG `lightShowInit` is assembly-only structural (0x78 frame, three calls, s0-s6 plus ra). No C body.
- reopen condition: find a spelling that keeps `remaining` live without a declared cell, or pay for that cell by dissolving a different 4-byte home without growing size. Do not repeat the original ten scope/setup-order forms, the negatives listed above, or another colour landscape on this source hash.

#### 2026-09-19, lane w36-o083b: overlay34 recipe on this body

Re-measure matches the inherited 98/168 at size 0, frame 0x80, first +0x0. Identity-gated instrumented IDO vs stock PASS, proc 0, 13 p1 decisions. -g3 homes: six function-scope slots at FP-4 through FP-24 (no spill traffic) and linkedInit at FP-48 / SP+0x50. Target linkedInit is SP+0x58 with only two function-scope slots at the top and a 16-byte gap after the saves. Same save set.

overlay34 comma-assign in the condition (`if (remaining = source->count - 1, source->count != 0)`, records/allocated comma, scaleFactor comma, flags comma, while-comma increments): byte-identical to baseline or worse. `remaining >= 0` is 94 at size +4. leftover remaining OR-zero and overlay22 empty-if are inert at 98.

L99 unused pointers declared first and L112 dummy arrays at function scope (pads 0-5, dummy 0-5, overlay34 declaration order) keep score 98 and grow the frame 0x80 to 0x88/0x98/0xa8. They do not slide linkedInit. Five unused pointers plus dummy[1] is 0x98 at 98.

`f32 dummy[2]` declared AFTER linkedInit in the flags block (N5) is 91 at size 0, frame 0x88, linkedInit +0x58. Aligned 133 exact, 23 naming, 3 immediate, 4 structural. The 9 immediate linkedInit displacements close; the +0x90 target-only delay-slot word remains. Comma-assign, OR-zero, empty-if, and while-comma on that shape are inert at 91. dummy[3] is 98 at 0x90. dummy[0] does not compile. Five pads on N5 grow to 0xa0 at 98.

Reusing batch as remaining and folding allocated into output (L1) is 107 at size 0, FRAME 0x78, linkedInit still +0x50, first mismatch +0x48 (prologue matches). Four function-scope locals. L97 `if (1) {}` and `do {} while (0)` are inert on it. Adding dummy[2] after linkedInit on that 4-local pun (N10) is 102 at frame 0x80, linkedInit +0x58: the dummy converts the 8-byte shrink into an 8-byte push of linkedInit, so frame returns to 0x80.

Dissolving two function-scope homes shrinks the frame from the top and leaves linkedInit at +0x50. Dummy-after-linkedInit grows the frame from below linkedInit and slides it to +0x58. Doing both nets frame 0x80. A net-zero conversion (exactly two function-scope 4-byte homes plus 8 bytes after linkedInit) needs the walking output/input cursors undeclared. Indexed `allocated[remaining]` is size +36 to +48 and frame 0x88. `parent->batch->records[remaining]` is size +408. Walking output plus indexed input is size +8. Dropping batch via `parent->batch` re-reads is size +12 at frame 0x78.

Do not repeat comma-assign of remaining/allocated/scale/flags on the 6-local body, unused pointers declared first, function-scope dummy arrays, indexed stride-0x258 forms, pointer-bound without remaining, parent->batch re-reads, L97 on the 4-local pun, or another colour landscape on the 0x80 source hash. Packed force-0 is still not a match.

Reopen: a size-0 spelling with two function-scope 4-byte homes and 8 bytes of block storage after linkedInit, without punning remaining into a live pointer and without extra parent->batch loads. That is walking output/input as generated temps, which indexing has not delivered.
<!-- plateau-handoff:overlay83BuildBatch:end -->
