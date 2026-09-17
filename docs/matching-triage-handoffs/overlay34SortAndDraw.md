<!-- plateau-handoff:overlay34SortAndDraw:start -->
### `overlay34SortAndDraw` plateau handoff

- source: `src/overlays/o034/overlay34SortAndDraw.c`
- score: 97 differing words
- frame: 0x1A0
- relocations: 16
- first mismatch: +0x50
- summary: Size closed at 760 B / 0x1A0. Interleaved swap plus reused-i copy. Unforced 97. Forced packing 64. Colour landscape floor 64. Homes still 24 B high.

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
