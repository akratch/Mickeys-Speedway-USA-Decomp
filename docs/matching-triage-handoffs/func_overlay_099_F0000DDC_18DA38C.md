<!-- plateau-handoff:func_overlay_099_F0000DDC_18DA38C:start -->
### `func_overlay_099_F0000DDC_18DA38C` plateau handoff

- source: `src/overlays/o099/func_overlay_099_F0000DDC_18DA38C.c`
- score: 0/352 words, promoted
- frame: 0xF0
- relocations: 13
- first mismatch: none
- summary: Matched. Rewritten from the listing: packet macros on (*displayList)++, MIN/MAX tile corners, vertex rows indexed from the grid, vertex address in a local, both second inductions in the for-headers, one unused local.

Summary before this remeasure: 331 differing words at size delta 0, frame 0xF0, 13 relocations, first mismatch +0x4; 352 candidate instructions, setup and loop structure unresolved.

#### 2026-10-02, lane w2-capbuf: matched and promoted (331 to 0)

The inherited m2c shape (a `command` carrier per packet, precomputed
offsets, a 0x38-byte volatile pad array standing in for the frame) was
discarded and the function written from the listing. The relocation
records name the callees: resident camStandardPersp and `func_80034920`
(texDPInit), and same-module `overlay99RenderSegments` through a SYMBOL
record, so all three are `*Reloc` placeholders. Measured steps, masked
words at size delta:

- Natural rewrite: one macro per packet on `(*displayList)++`, the tile
  corners as `MAX(x - 1, 0)`, `MAX(y - 1, 0)`, `MIN(x + stepX, 319)`,
  `MIN(y + stepY, 239)`, the two vertex loads as
  `&buffers[index][i * width + j]` and `[(i + 1) * width + j]` written
  inside the macro argument: 124 at +48. The macro names its address
  twice, so the buffer table was read twice per command.
- The vertex address held in a local (`vtx`), plus one unused local for
  the frame's last cell: 7 at 0, frame 0xF0 exact. uopt builds both row
  offsets and their +20 steps from the subscripts.
- A 36-cell product over the two loop headers: ten exact cells. What
  decides it is the second induction of each loop sitting in the
  for-header with the first (`i++, y += stepY`; `j += 2, x += stepX`),
  which orders the two latch loads and the `x` copy against the row-offset
  step. `x += stepX` as the last body statement leaves 7.

The inherited `-Wo,-r4300_mul` per-file flag was inert (the exact cell
scores 0 with and without it) and is removed.

Promotion: the function's object ends at its return (0x580), so the
module's last word at +0x135C became an `overlay_099_padding` row, as
overlay 46's did; `config/nonexecutable-ranges.us.json` carries it as
`overlay-padding` instead of `overlay-trailing-alignment` and the excluded
total is unchanged. `gmake verify` OK, `check-overlay-syms` up to date,
promotion-proof PASS (352 words, relocations 13/13).
<!-- plateau-handoff:func_overlay_099_F0000DDC_18DA38C:end -->
