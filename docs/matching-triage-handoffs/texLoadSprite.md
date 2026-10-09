<!-- plateau-handoff:texLoadSprite:start -->
### `texLoadSprite` plateau handoff

- source: `src/main/textures_354C8.c`
- score: 0/269 words, promoted
- frame: 0x68
- relocations: 44
- first mismatch: none
- summary: Matched. cacheFull is cleared beside cacheNum between the two cache scans; as1 can then lift it into the bounds test's delay slot.

Summary before this remeasure: 7 to 2: sizeof-typed (unsigned) vertex, texture and command sizes fix the a3/t0 offsets. Left: as1 fills the bounds-test delay slot with the D_800D2FF8 high half

Summary before this remeasure: 105 to 7: direct cache and ROM-table reads, triangleOffset reloaded, += offset chains. Left: vertex/command offsets a2/a3 vs a3/t0 and the cacheFull delay slot

Summary before this remeasure: Table entry pointer (106->105); open: target hoists the D_800D2FF8 lui and the id shift into block 2, here the shift lands in block 1

Summary before this remeasure: Size and frame closed (269 words, 0x68). 109 masked; recover vs triangleOffset spill is the remaining 3-word insertion. Do not colour yet.

Lane p2b-tex tried three live-across-func_8002B314 spellings of the pure align16 result. A new scalar spilled and reloaded it at +0x40 and scored 136 masked. Storing it through the existing arenaCount local in its old position spilled at +0x44 and scored 136. Moving that declaration to just after cacheNum stored and reloaded the value at +0x58, and s2 stayed a sum rather than a copy of the align16 result, but the other word spills and the byte homes moved and the masked count rose to 135. None beat the 109-word baseline, which is retained. Size delta stayed 0, frame 0x68, and both sides kept 44 relocations.

Fresh configured evidence on 2026-09-05 proves the resident owner at
`0x800355A0..0x800359D4` / ROM `0x361A0..0x365D4`: 1,076 executable bytes,
269 target instructions, no padding, and 44 static target relocations. The
function has 78 authenticated overlay callers, including one each in overlays
12, 23, 31, 60, and 62, two in overlay 13, and 71 in overlay 101.

The retained candidate replaces the prior 15-instruction cache scaffold with
the complete sprite path: flat-cache lookup and reuse, vacant-slot selection,
section `0x15` loading, maximum frame/tile arena sizing, one tagged allocation,
texture acquisition and rollback, metadata transfer, per-frame display-list
construction, draw-flag propagation, and cache insertion. Its control flow is
adapted from Diddy Kong Racing's public `tex_load_sprite` source and checked
against Jet Force Gemini's public assembly-only `texLoadSprite`; Mickey's
fields, globals, allocation geometry, calls, and object remain authoritative.

Configured full-TU C emits 268 instructions with frame `0x70`, 177 positional
word differences, workbench normalized distance 27, and function SHA-1 prefix
`495cbdbee7b5`. The target has 269 instructions and frame `0x68`; after the
frame word, the first substantive mismatch is the zero/cache-table scheduling
pair at `+0x48/+0x5C`. All 44 candidate relocation identities resolve, but only
20 records currently align in offset, type, and identity because the arena
layout web shifts the remainder.

Ten coherent forms covered direct offsets, the historical DKR additive
allocator, target-shaped arithmetic staging, local reuse, narrow and promoted
arena counts, implicit vertex common subexpressions, pointer aliases, and
defined zero-carrier variants. The best admissible form uses an equivalent
`(arenaCount * 2) * 16` association, a typed allocation-base alias, and reuses
the dead texture offset as a defined zero carrier. A 20-minute four-worker
permuter batch improved raw permuter score 1,870 to 996, but its two additional
gains read uninitialized `size` and `vertexOffset`; those forms were rejected.

Workbench classifies the remaining mechanism as `stack-home`: the candidate
pool lane contains one extra declared-local web, giving eight extra non-save
frame bytes. Its next lever is `drop-a-declared-local`: repeat one arena
expression at both uses so IDO creates a call-crossing common subexpression in
the compiler-temporary region. Reopen only with a natural, defined spelling of
that lever. Do not repeat the ten source families, the completed generic batch,
or the rejected uninitialized-local cues, and do not treat the full semantic
body or resolved relocation identities as an exact match.

Lane w30-tex (2026-09-19) closed size and frame. Identity-gate: instrumented IDO
`.text` is byte-identical to stock. Procindex maps this function to CDX_PROC=9,
p1-only (47 p1dec, 0 p2). Dropping the `newBase` copy matches frame 0x68.
Scale expressions `((i * 4) * 8)` and `((i * 4) * 10)` reconstruct the target's
shared `*4` temporary. Deleting the `triangleOffset` local and recovering
`displayListOffset - ((i * 2) * 16)` after alloc is size-exact (269 words).
Reusing `i` as the arena count plus a second-loop `&D_800D2FFC[i << 1]` node
carrier (overlay22 L145) reaches 109 masked. An unused `arenaCount`
declaration immediately before the offset locals holds frame 0x68; removing it
drops the frame to 0x60.

The remaining 3 candidate-only words are that recover (add, shift, sub).
The remaining 3 target-only words are the triangleOffset and displayListOffset
stack spills at +0x164. A named triangleOffset local takes s2 (copy from v0)
and is 4 bytes short (172 masked). volatile, address-taken, and s32[1]
homes either stay short or grow the frame. Empty if (i), leftover
spriteId OR-zero and flags OR-zero, comma-assign, L109 zero OR-zero, walking
cursors, and first-loop node were inert or worse on this shape. Do not run a
colour landscape while those insertions remain (L155). Tried 2026-09-24,
not an open assignment: store the align16 result and reload it, leaving
textureOffset in s2. A new scalar scored 136 with the home at +0x40.
The existing arenaCount local scored 136 at +0x44. Lifting that local
stored and reloaded the value at +0x58 and scored 135. Every spelling
raised the masked count above 109, so the 109-word body was kept.
Stall: the +0x58 reload moves the other spills.

## 2026-10-01 (lane d-res1): natural source, 109 to 106

The retained candidate carried three allocator cues (a dead `if (newSprite)`,
the arena offset reused as the integer zero, and a hand-built triangle
pointer). Rewritten as plain source with no cues, and measured as a product:

- Declaration order of the six word locals (cacheNum, triangleOffset,
  displayListOffset, vertexOffset, textureOffset, commandOffset) sets the
  frame homes top-down; with that order all four spills land at the target's
  homes (+0x58, +0x54, +0x50, +0x48) and the small scalars pack at +0x34..0x37.
- With the carrier read (`D_800D3018 = newSprite + triangleOffset`) all four
  spills exist but the temp ring is one draw off from +0x48: 131 masked, all
  register naming, no structural row. Without the carrier (the triangle
  pointer rebuilt from displayListOffset) the count is 106; that is the
  retained form, `triangleOffset` stays declared to hold its home.
- Zero reuse, the dead if, init order of `i` and `cacheFull`, `size` carrier,
  `align16` inline, texture carrier, `numTextures`/`i` assignment form and
  loop-1 spelling (for/while/pointer) were all flat (a 24-cell product).
- Cause of the head residual: the first mismatch is where the target schedules
  `or t1` (cacheFull = 0) in the first block's delay slot and the hoisted
  `lui t3 D_800D2FF8` plus `sll t4` in the second block; here the shift lands
  in the first block's delay slot and the `lui` is not hoisted. `cc -S` shows
  the shift in the join block, so as1 moved it; re-assembling the `-S` output
  does not reproduce the move. The remaining 131/106 words are one ring phase.
## 2026-10-02 (lane e-res3): 106 to 105, table entry pointer

Product over the ROM-table read: `size = D_800D2FF8[spriteId]` with the second
read as `[spriteId + 1]` (106), byte-offset forms (106), a block-scope
`entry = &D_800D2FF8[spriteId]` read as `entry[0]` / `entry[1]` (105), asset
read before the size (106), repeated reads at the call (106). Store-order
probes over the five D_800D30xx stores and the six metadata copies are worse
(110-118). Open decision variable unchanged: the target schedules `or t1`
(cacheFull) and `sll t4` (spriteId * 4) into the second block, here the shift
lands in the first block.

## 2026-10-02 (lane x-res): 105 to 7 at delta 0

Three edits that were each flat or worse alone on the inherited shape are
7 together (a 24-cell product, then 36 and 72 cells over the offset chain):

- the free-slot scan reads `D_800D2FFC[i << 1]` directly (no `node`
  pointer local);
- the ROM table is read as `D_800D2FF8[spriteId]` and
  `D_800D2FF8[spriteId + 1]` (no `entry` pointer local);
- the triangle pointer is `newSprite + triangleOffset`: the target reloads
  that home at 0x58 after the allocation. d-res1 measured this carrier at
  131 only on the node/entry-pointer shape.

That cell is 9; writing the display-list and texture offsets as `+=` chains
(`displayListOffset = triangleOffset; displayListOffset += ...;`, likewise
for textureOffset, DKR's allocSize idiom) gives the target's addu operand
order for those two sums: 7. The same chain on vertexOffset and
commandOffset, either operand order, a `<< 2` or `sizeof` spelling, and a
single running accumulator for all offsets (+4 bytes) are flat or worse.

Left, 7 words: vertexOffset and commandOffset take a2 and a3 where the
target has a3 and t0, and their addu operands are swapped (forcing the two
single-block webs 109 to c6 and 114 to c7 on proc 9, accepted, scores 4;
nothing visible holds a2 there in the target, so the decision variable is
what denies vertexOffset a2), plus the target's `move t1, zero` (cacheFull)
in the bounds-test delay slot where this build puts the hoisted
D_800D2FF8 high half. Seven placements of the cacheFull clear (for-init in
either order, before the loop, before the bounds test, at declaration)
are 7, 8 or 56.

## 2026-10-02 (lane x-res, second pass): 7 to 2

Records first (proc 9, identity-gated): the vertexOffset and commandOffset
register parts are single-block webs 109 and 114 (block 22, save 3.0, nocs
1, tied with webs 92 i*2 and 94 displayListOffset and coloured in web order
to the lowest free caller register). Every web live across block 22 was
listed with its colour (s0 spriteAsset, s1 the i*4 expression web, s2
textureOffset, s4/s5, v1/a1 and the four split offset homes); none holds
a2, and nothing in the target's block 22 references a2, so the target's
skip of a2 is not an occupant. The `if (x) {}` idiom at four positions and
six variables, a named `i * 4` local, `+=` accumulation forms (6 cells) and
a third parameter were flat or worse.

What moved it was item 23: the sizes written with sizeof are unsigned terms
and new IR names. `(i * 4) * sizeof(Gfx)` for the display lists, `i *
sizeof(TextureFrameHeader *)` for the texture pointers and `(i * 4) *
sizeof(SpriteVertex)` for the vertices give 2 (a 2x2x3x3 product, 7 for the
signed literals); vertexOffset and commandOffset then take a3 and t0 and
their addu operands come out in the target's order, both at once. The
triangle and frame-pointer terms (`sizeof(SpriteTriangle)`, `sizeof(Gfx *)`)
are byte-identical either way and are written with sizeof for consistency.

Left, 2 words, schedule only: the target's as1 puts `move t1, zero`
(cacheFull) in the bounds test's delay slot and keeps the D_800D2FF8 high
half in the loop-1 preheader; this build's as1 hoists that high half (from
the D_800D2FF8 read after the loops) into the slot. ugen's output for the
preheader is the same instruction list in both readings (la, i = 0,
cacheFull = 0, count load). Measured flat: four cacheFull placements on
this shape, the for-init and the ROM-table statements split or folded
across lines (9 cells), and the empty-if idiom in both cache loops and
before the cacheNum test (15 cells). Next instrument: `cc -Wa,-R` (as1's
scheduler trace) on the two readings to name the priority that picks the
slot instruction.

as1 trace (`cc -Wa,-R`, scratch only): the 2 words are as1's global
code motion, not ugen. In this build's second scheduling pass the bounds-test
block is [D_800D3004 load, slt, the D_800D3008 address pair from the next
block (line of the return), the D_800D2FF8 high half (line of the table
read), bnez]; the high half has aftercycles 0 and is scheduled last, into
the delay slot. The cacheFull clear stays in the loop-1 preheader. In the
target the clear is the instruction hoisted into that block and the high
half stays in the preheader. Physical line numbers reach this choice: a
`#line` lowering the table read below the loop's line moves the high half
earlier still (5 words); raising either line is inert (2). So the decision
variable is which preheader instruction as1 hoists into the bounds block,
not a uopt or ugen decision. Not measured: moving the ROM-table read's
statement relative to the cache loops in source (it must stay after them
semantically unless the read is hoisted as a pointer).

## 2026-10-02 (lane x-res, third pass): 2 to 0, promoted

The as1 trace named the decision. as1 fills the bounds test's delay slot by
trial: it lifts one instruction at a time from the blocks below (the
D_800D3008 address pair, `i = 0`, `cacheFull = 0`, the second scan's
`cacheNum = -1`, then the D_800D2FF8 high half), reschedules both blocks and
keeps the move only if the donor block does not get worse. The first loop's
preheader holds the count load and the branch on it (latency 3), so it
needs two other instructions between them; with `cacheFull = 0` in the first
loop's init the preheader has exactly two (`i = 0` and the clear), each
trial leaves a stall and is undone, and the far high half wins the slot.
The target took the clear, so the clear cannot have been one of the
preheader's two fillers: it comes from the second scan's block.

`cacheFull = 0; cacheNum = -1;` before the second loop is 0 masked at delta
0 (6-cell product; the clear after `cacheNum = -1`, or in the second loop's
init in either order, stays 2; those cells were not traced, the likely
reason being that as1 tries the block's instructions in order and
`cacheNum = -1` is that block's own delay-slot filler).

Closure broken: this lane's own and d-res1's reading that the head is "the
target hoists the D_800D2FF8 high half and the id shift" was the wrong
instruction; seven earlier placements of the clear all kept it in or above
the first loop.

Gates: gmake verify OK on the promoted tree, scoreboard and ranking
regenerated.

<!-- plateau-handoff:texLoadSprite:end -->
