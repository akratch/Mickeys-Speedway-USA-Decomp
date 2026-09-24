<!-- plateau-handoff:overlay10Initialize:start -->
### `overlay10Initialize` plateau handoff

- source: `src/overlays/o010/overlay10Initialize.c`
- score: 37/172 words
- frame: 0x68
- relocations: 41
- first mismatch: +0x0
- summary: hypothesis=hoist width and height into a1 and a2 without extra s32 homes; spellings=param-pin scored 173, register copy inert at 37, OR-zero deleted reloads and grew 8 bytes; stall=none lowered the masked count at delta 0 without growing the frame

Summary before this remeasure: loopunroll,0 closes size. leftover offset OR-zero closed slti vs li. Copies cost 16 frame bytes and buy the first-loop shape.

Configured compile carries -Wo,-loopunroll,0 (NON_MATCHING-only). Identity-gate: instrumented .text matches stock, CDX_PROC=0.

Closed this lane: size 688, delta 0 (was +156). leftover offset OR-zero in the entry loop closed the extra/missing pair (li 4096 vs slti). D_140, D_400, D_10 are the splat-named loop ends.

Blocker: frame 0x68 vs 0x58. `widthValue`/`heightValue` copies are load-bearing for increment-first viewport stores and hoisted width/height. Each copy costs 8 frame bytes and saves about 4 code bytes. Zero copies: frame exact, size +8, the loop reloads width/height every iteration. `register`, nested blocks, an s32[2], and a pre-incremented cursor all failed to hoist without homes.

First mismatch +0x0 is the frame addiu. 29 naming rows: a0/s0/v1 cycle on angle/offset/entries; a1/a2 vs v1/a0 on width/height. Splitting the inner-loop index from `offset` shifted s-regs and scored 44.

Next: a spelling that hoists width/height into a1/a2 without extra s32 homes, then colour. Do not run a colour landscape until size and frame are both 0.
<!-- plateau-handoff:overlay10Initialize:end -->
