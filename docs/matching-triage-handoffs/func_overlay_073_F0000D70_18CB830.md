<!-- plateau-handoff:func_overlay_073_F0000D70_18CB830:start -->
### `func_overlay_073_F0000D70_18CB830` plateau handoff

- source: `src/overlays/o073/overlay73Draw.c`
- score: 0/78 words, promoted
- frame: 0x30
- relocations: 5
- first mismatch: none
- summary: Matched. Sibling copy of the overlay 71 renderer: packet macros, vertices[vertexBank * 6] over ten-byte vertices, one unused pad local.

Identity-gated instrumented IDO `.text` matches stock. `CDX_PROC=0` (12 p1 decisions, 8 coloured webs). `--every-colour` (59 probes, size 0): only `p1:w43=c4` (v1 to a1) beats 41, at 39; packing predicts 39. w9 (vertexBank, v0) cannot be forced onto a ring temp.

Proved: writing `command->w0 = 0x05710080` before `command->w1 = (u32)D_80000000` matches that pair's birth order. Same-line FA and G_VTX stores are inert. Unused `stackShape` was already DCE.

Eliminated on this shape:

- L145 delete vertices/index/physicalVertices: size +4, frame 0x28 (need the pre-call vertices spill)
- `state->vertexBank * 0x3C`: size -12 (the expanded shift spelling is load-bearing)
- nested shift expression without vertexBank: byte-identical to the named local, so that local is not the LBU-into-v0 cause
- `*3 *2 *5 *2`: 45 masked, more structural
- L97 around 0x0571 pair, its increment, or physicalVertices: flat at 42
- L97 on the first command increment: size +4
- L97 plus inlined 0x80000000 at both G_VTX uses: size +8

Next lever is a source form that puts the vertices spill at +0x2C and the vertexBank load in a ring temp without collapsing the shift chain. Colour cannot close it.
#### 2026-10-02, lane x-sib2: matched as a sibling copy

Matched and promoted; `gmake verify` and `promotion-proof` pass.  42 masked
words to 0 at size delta 0 in three measured edits on the shape of the matched
overlay 71 renderer `func_overlay_071_F0000870_18CA390` (same three resident
callees, same command sequence):

- Every command written as its packet macro (o071's `O71_*` set, renamed), no
  shared `command` cursor, no `volatile` vertices, no hand-expanded G_VTX
  word: 69 masked at size -12, because a `* 0x3C` byte offset expands as 15*4.
- The vertex bank as a subscript into an array of twelve ten-byte vertices,
  `&state->vertices[state->vertexBank * 6]`: the index times six, then the
  element size ten, is the shipped 3*2-then-5*2 chain that the closure above
  read as "the expanded shift spelling is load-bearing": 10 masked, delta 0.
- Declaring `vertices`, an unused `s32 pad`, then `state`: frame 0x28 to 0x30
  and the vertices spill to +0x2C: 0.

The closure's "next lever" (a source form for the +0x2C spill and the
vertexBank load in a ring temp) was the inherited shape: the LBU register
followed from the macro form with no lever of its own.
<!-- plateau-handoff:func_overlay_073_F0000D70_18CB830:end -->
