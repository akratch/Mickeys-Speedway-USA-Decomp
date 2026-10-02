<!-- plateau-handoff:overlay65UpdateParticles:start -->
### `overlay65UpdateParticles` plateau handoff

- source: `src/overlays/o065/overlay65UpdateParticles.c`
- score: 0/720 words, promoted
- frame: 0xF8
- relocations: 64
- first mismatch: none
- summary: Matched. Vertex block as a counted loop (depth 2), packet macros, locals cut to one per role, volatile and register dropped.

#### 2026-10-02, lane g-ovl5: matched, 458 to 0

Priced edits, each measured with fast_score / shape_product (masked words, size
delta 0 unless stated):
- Declaration order cut (spawnCount, transformed, ground positions): 458 to 454.
  This moved transformed to 0x9C and spawnCount to 0xD4 as in the target.
- The four vertex writes as `for (n = 4; n != 0; n--)` with a word index
  `idx += 3` over the transformed points: 454 to 375 at +12. The loop depth
  was the cause of the saved-register order: alpha, the transformed base and
  radius sit in s3/s4/s0 only when that block is depth 2 (uopt unrolls it
  four times with no exit test; a count-up loop or a pointer-increment loop
  does not give the same iv-indexed copies and the transformed copies fold to
  sp offsets).
- Packet macros (JFG gSPVertexJFG / gSPPolygon, PROVENANCE in source) for the
  batch flush and the tail: 368 to 251, size delta 12 to 0.
- Dropping `volatile` on spawnCount: 245 to 68. It was inert until the loop
  and macros were right; a plain local is spilled by uopt to the same 0xD4 home.
- Colour/store order of the spawn block (r, g, b, then active): 68 to 63.
- Locals: `point` replaced by `transformed`, the vertex loop counter and the
  tail count reusing groundCount (one fewer home each): frame 0x108 to 0xF8.
  48, then a declaration-order hill climb 48 to 34 (11 moves of 17 locals).
- Update statements back to `x += y * arg2` in source order: 34 to 17.
- Prologue statement order (commands, cursor, buffer swap, particle,
  spawnCount, remaining): 17 to 7.
- Tail count written inline `(6 - remaining)`: 7 to 0.

Promotion: atlas written (TU completed, no range entry), extract, overlay-syms
twice, `mk/overlays.mk` POSTPROCESS for the object (first callee redefined onto
the offset-zero carrier, the other 20 call sites rebound, trim 0xB40).
verify, check-overlay-syms and promotion-proof (720 words, relocations 64/64) pass.

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
