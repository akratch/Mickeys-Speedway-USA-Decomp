<!-- plateau-handoff:func_overlay_073_F0000D70_18CB830:start -->
### `func_overlay_073_F0000D70_18CB830` plateau handoff

- source: `src/overlays/o073/overlay73Draw.c`
- score: 42/78 words
- frame: 0x30
- relocations: 5
- first mismatch: +0x1C
- summary: w0-first closed 0x0571/D_80000000 birth order (structural 4 to 2). Residual: spill +0x28 vs +0x2C and LBU v0 vs t7. Colour floor 39. L145/L97/lineno exhausted.

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
<!-- plateau-handoff:func_overlay_073_F0000D70_18CB830:end -->
