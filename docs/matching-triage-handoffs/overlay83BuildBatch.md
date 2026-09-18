<!-- plateau-handoff:overlay83BuildBatch:start -->
### `overlay83BuildBatch` plateau handoff

- source: `src/overlays/o083/overlay83BuildBatch.c`
- score: 98/168 words
- frame: 0x80
- relocations: 5
- first mismatch: +0x0
- summary: L134 inner spills plus u8 count cut frame 0x90 to 0x80. remaining is the last 8-byte home; colour floor 89 at delta 0.
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
<!-- plateau-handoff:overlay83BuildBatch:end -->
