<!-- plateau-handoff:overlay65UpdateParticles:start -->
### `overlay65UpdateParticles` plateau handoff

- source: `src/overlays/o065/overlay65UpdateParticles.c`
- score: 458/720 words
- frame: 0xF8
- relocations: 64
- first mismatch: +0x50
- summary: Size closed 0 at frame 0xF8. Colour 267 probes floor 395 on one c8 radius. Cursor still one-behind. overlay65Initialize identities untouched.

#### 2026-09-19, lane w29-o065b: size closed; colour floor 395

Live V0 was 710/720 words, size delta -40, masked 689, first +0xC, frame 0xF8.
Identity-gate: instrumented IDO `.text` matches stock. CDX_PROC 0, 59 p1dec, 39 p1color, 0 p2.

The missing 10 words were the L160 transformed cursor. Hand-unrolled `point[0..3]`
folds to stack-relative loads. The target keeps s4 as `&transformed` and a
word-index in v1 (`sll 2` plus `addu`, `v1 += 3`) one-ahead of the store.
Spelling vertex 0 through `point` and vertices 1-3 through
`(O65Vec3f *)((s32 *)point + groundIndex)` with `groundIndex = 3; groundIndex += 3`
emits that form. Reusing `groundIndex` (L115) keeps the 0xF8 frame; a dedicated
`s32 vertexIndex` grows it to 0x100.

The remaining +4 closed by loading `*arg1` while a1 is still live, before the
D_210 buffer swap. Empty-if and leftover OR-zero on groundIndex were inert on that shape.

Aligned at size 0: 321 exact, 316 naming, 7 immediate, 87 structural.
11 candidate-only and 11 target-only words. Dominant census is s0 to s1 x83
(particle), coherence 51 percent, 24 windows: not a ring phase.

`--every-colour` 267 probes over 39 webs. Best same-kind score 395 is one c8
radius with six handles (w203, w261, w283, w301, w372, w452). Forcing particle
web 12 (s0, save 123.9) onto s1 scores 439 and overlaps that radius. Colour
cannot emit the missing one-ahead `li v1, 3` in the vertex-0 slot (candidate
still hoists `li a0, 6` there) or move spawnCount from 0xE8 to 0xD4.

Next: keep the index out of a0 so `li v1, 3` occupies the vertex-0 nop, then
L99 so spawnCount lands at 0xD4, transformed at 0x9C, ground at 0x88. Do not
invent overlay65Initialize relocation identities from the target.
<!-- plateau-handoff:overlay65UpdateParticles:end -->
