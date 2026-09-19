<!-- plateau-handoff:overlay34SortAndDraw:start -->
### `overlay34SortAndDraw` plateau handoff

- source: `src/overlays/o034/overlay34SortAndDraw.c`
- score: 0 differing words
- frame: 0x1A0
- relocations: 16
- first mismatch: none
- summary: Matched. Comma-assign delay slot, L99 five unused pointers plus distances[59], scalars declared above the array.

#### 2026-09-19, lane w33-o034: 97 to 13 at size 0

Identity-gated instrumented IDO vs stock PASS, proc 0, 26 p1 decisions, frame 0x1A0, 190 vs 190 words, displacement tax 0.

Named levers on the inherited 97 body:

- L99 unused pointer/f32 first or before the array: 101, first +0x0
- leftover arg0 OR-zero at entry, in the draw loop, or inside record != NULL: inert at 97
- leftover i OR-zero: size +4, 118
- overlay22 empty-if on offset, j, i after sort, or offset inside the record body: inert at 97
- overlay22 empty if (1) in the draw loop: 107
- overlay40 comma-assign of the record load: 96
- overlay40 comma while (i = i - 1, i >= 0): size -8, 160
- overlay41 remat half-first / inline half: size -4
- overlay41 remat frame-first: inert at 97
- L160 delete i copy: size -4, 119

What moved:

- L160 indexed draw (delete the byte-offset carrier, use gOverlay34Pointers[j]): 81, s0/s1 cycle gone, +0x68 home matches, insertion pair unchanged
- split record->frame spellings (then i = record->frame, else i = record->frame - half): 86, insertion pair closed
- together: 25, 165 exact / 11 naming / 10 immediate / 4 structural, no insertions
- carry the frame copy in position instead of reused sort-i: 24, sort-i takes a1
- leftover position OR-zero (L100, not the folded frame OR 0 expression): 15, s3/s4 swap gone, 1 naming left (v0 to s3)
- do-while fill in place of for (j): 13, first-loop structural pair closed

Best unforced: 760 B, delta 0, frame 0x1A0, 13 masked / 15 raw, first +0x54, aligned 177 exact / 1 naming / 10 immediate / 2 structural. w6 record takes s0 naturally. w25 takes a1.

Remaining on that body:

- 10 immediate: distances still at +0x98 vs +0x80, colors at +0x198/+0x19C vs +0x184/+0x188, swap at +0x9C vs +0x84. +0x68 matches. L112 lengths 60-72 and L99 pads 1-6 either grow the frame or break +0x68.
- 2 structural at +0x1F8/+0x1FC: delay-slot of position < half. Copying position before the test closes both (0 structural) and costs a 6-site v0/v1 swap, 17 masked. Same cell as overlay40 comma-assign of the test.
- 1 naming at +0x23C: v0 to s3 in the else arm.

Do not repeat declaration shuffles, flags, or permuter. Do not drop position OR-zero or the do-while fill. Next: a form that puts the position copy in the branch delay slot without the v0/v1 swap, or a 24-byte top-of-frame save that keeps +0x68 and frame 0x1A0.

#### 2026-09-17, lane w9-o034: size closed; colour floor 64

Arrival was 760 B target vs 756 B candidate, delta -4, masked 128, first +0x14,
frame 0x1A0, 16 relocs. Identity-gated instrumented IDO vs configured stock
PASS, proc 0, 26 p1 decisions.

The missing word was named: target-only `move v1, s5` after `half = length >> 1`
and `position < half`. Target loads `record->frame` into v0 (caller-saved), so
it copies position into s3 in the branch delay slot and copies half into v1 as
a real instruction. Candidate kept position in a callee-saved, so the half copy
filled the delay slot and the function was one word short.

Two source edits closed size and retired the three swap-schedule extras:

- Interleave the distance swap with the pointer swap (load record, store
  distances[j], store pointers[j], store distances[j+1], store pointers[j+1]).
  That eliminated the three candidate-only `lw` / target-only `swc1` pairs.
- Reuse dead sort index `i` as `i = position` for the interpolate args, so the
  copy does not grow the frame. A fresh `copied` local closed size but grew
  the frame to 0x1A8.

Unforced result: 190 vs 190 words, delta 0, frame 0x1A0, masked 97, first +0x50,
aligned 102 exact / 63 naming / 11 immediate / 18 structural.

Colour packing on that body, scored on forced objects directly:

- `p1:w9=c15` 73 at delta 0 (fill-offset web to s1; record cascades to s0)
- `p1:w9=c15,p1:w164=c2` 65 at delta 0
- `p1:w9=c15,p1:w164=c2,p1:w25=c18` 64 at delta 0

`tools/web_footprint.py --every-colour --hold` of those three, 132 probes over
21 webs: no additional colour beats 64. L160 does not apply; no force scores 0.

Remaining on the unforced 97: array/colour homes still 24 bytes high (target
distances at sp+0x80, colours at +0x184/+0x188; candidate +0x98 and
+0x198/+0x19C); one colour-address web splits so `&color` rematerializes
inside the draw loop; target still has the extra `move v1, s5`. One unused
pointer declared first grew the frame to 0x1A8 and was reverted. Copying with
bitwise-or-zero regressed to delta +4. Do not repeat declaration shuffles, flags, or permuter.
Next: make `w9=c15` natural by raising the record web's save above 217 (L100
nocs) so s0 is taken first, then re-pack; or split `record->frame` spellings
so position can live in v0 without a long-lived extra local.

#### Whole-itable phase-local reproof (2026-08-31)

- assignment base: `d63836a7`; configured V0 reproduces a 760-byte target
  against a 756-byte / 189-word candidate, 129 raw and 128 relocation-masked
  differences, first mismatch `+0x14`, and the exact target frame `0x1A0`.
- IDO 5.3 `uopt.c` was regenerated from static-recompiler revision
  `9c242adc` at the pinned `b0058f15...` source digest, instrumented with the
  shipped global-color profile and preserved whole-itable patch, and built in
  a lane-local toolchain copy. With tracing enabled, `.text`, `.data`,
  `.rodata`, relocations, and symbols are identical to stock output.
- the baseline ladder contains 17 slots and separately exposes the large
  distance-array region, the sort swap home, and the later address-taken color
  and interpolation-scalar homes. This validates the phase-local hypothesis
  without inferring names from raw compiler records.
- the single authorized source form placed the distance/swap locals and the
  render-only colors/scalars in disjoint lexical blocks. It changes ten
  stack-home operands: the distance family moves only four bytes while the
  render homes cross the array region, rather than producing the measured
  20/24-byte reuse. Against retail it retains 189 words, frame `0x1A0`, all
  128 masked differences, and the seven relocation sites that remain four
  bytes early.
- the alias-splitting follow-up was not attempted because the scoped form
  produced no strict geometry gain. Preserve the retained source and reopen
  only when producer evidence attributes a specific home or identifies the
  missing instruction; do not repeat scopes, declaration shuffles, flags, or
  broad permutation.
<!-- plateau-handoff:overlay34SortAndDraw:end -->
