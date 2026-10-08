<!-- plateau-handoff:func_overlay_101_F0002510_18DDD30:start -->
### `func_overlay_101_F0002510_18DDD30` plateau handoff

- source: `src/overlays/o101/func_overlay_101_F0002510_18DDD30.c`
- score: 90/293 words
- frame: 0xE8
- relocations: 6
- first mismatch: +0x70
- summary: bottom is a web only with no indirect load in its blocks; target clip block reached at 260/-4 (aligned 61); needs v0 denied to the post-GetBounds webs.

Summary before this remeasure: y origin local tested, y copied after the clip tests: delta 0. bottom never forms a web (no argument pin); alias spellings fold.

Summary before this remeasure: y origin local tested, y copied after the clip tests (target's v1/s4 split): delta 0. Left: clip-block schedule, left/top in s3/s1.

Summary before this remeasure: 257 words at size -8, frame 0xE8 exact, SDK GBI body with early returns. Left: the y split, left/top in s3/s1, rotated hoisted rect words.

Summary before this remeasure: 263 words at size -8 (was 291 at +8), frame 0xE8 exact, SDK GBI macro body. Left: the y split, left/top in s3/s1, rotated hoisted rect words.

Summary before this remeasure: 291 words, size +8, frame 0xF8 vs 0xE8. Bounds-array and width-local spellings did not beat it.

Summary before this remeasure: Rebuilt on the four decoded SYMBOL callees and this overlay's own display-list command idiom; structural residual 142 to 125 and size delta -52 to +8, where the +8 is one extra callee-saved register, while the positional count went 276 to 291 because the frame is 0xF8 against 0xE8.

#### 2026-09-12, lane p11-o101: the four callees are three different functions

The retained body named `func_overlay_101_F0000000_18DB820` for all four calls
and handed it four different argument lists. That came from the extracted
assembly, where all four jumps decode to overlay offset 0. They do not go
there. Every one of the four is a **SYMBOL** relocation record, so the shipped
word carries the `0xF0000000` addend rather than `offset >> 2`, and the callee
is named by the module's runtime relocation table instead. Decoded from it,
with the resident base the matched siblings already calibrate:

- `+0x68` is overlay 101 `+0x2118`, `overlay101GetBounds`, five arguments:
  the node and four `s32 *` outputs.
- `+0xDC` and `+0x460` are overlay 101 `+0x1F80`, `overlay101SetScissor`,
  five arguments: the display-list pointer and four edges.
- `+0x444` is the resident `func_80034920`, at `+0x344D0` past the resident
  base, and it takes **one** argument. The retained body passed four, which is
  where its two extra stack homes at 124 and 144 came from.

Both overlay callees are ROM-exact in this tree, so their prototypes are the
matched ones rather than a guess, and all three must be reached through the
generated surface names when this function is eventually promoted.

The display-list idiom is likewise this overlay's own, not an invention: the
ROM-exact `overlay101SetScissor` writes each command as a bare block holding a
fresh `Overlay101Gfx *command` snapshot of the cursor. The target's
`move aN,v0` before every `addiu v0,v0,8` is that shape. Writing the stores
through the cursor itself, as the retained body did, folds the increments and
emits negative displacements the target never has.

#### What that bought, measured with tools/align_symbol.py

  - retained body: size delta -52, positional masked 276, byte-exact 48,
    register naming 101, immediate only 13, really different 142.
  - this body: size delta +8, positional masked 291, byte-exact 46, register
    naming 128, immediate only 13, really different 125.

The positional count is worse and the shape is closer. The reason is the
frame: 0xF8 against the target's 0xE8, so every stack displacement differs and
a quarter of the naming rows are that one fact. The `+8` size delta is exactly
one extra callee-saved register's save and restore -- this candidate holds ten
callee-saved registers where the target holds nine.

#### The decision variable, and the axes already measured

**One long-lived value too few.** Because this candidate has a spare register,
uopt hoists five loop-invariant opcode constants (`0xFD100000`, `0xF5100000`,
`0xE6000000`, `0xF3000000`, `0x07000000`) into registers across the loop. The
target hoists exactly one, `0xF5100000`, into `ra`, and rematerialises the rest
inside the loop body. So the target's register file is fuller than this
candidate's by roughly four values, not emptier -- the hoisting is a symptom,
and the lever is to find the live values the target carries that this
reconstruction does not.

One such value is already identified and does not close it: the target keeps
**two copies of `rows`**, in `s6` and `s7`, with an explicit `move s7,s6`
between the stride multiply and the display-list read. `s7` serves only the
per-iteration `texture->width * rows` for the load count; `s6` serves the
stride, the mask and the per-row chunk count. Splitting the carrier in the
source reproduces the copy but costs a stack home: frame goes 0xF8 to 0x100 at
an unchanged 291, in all six placements tried (after the shift, after the
stride multiply, after the display-list read, after the mask, with the
declaration in two positions).

Closed by measurement on the configured TU:

- Clip-condition form. The target evaluates `texture->width`,
  `texture->height`, `x + width` and `y + height` in the block *before* the
  first clip branch and then takes four plain `bnez`. Computing the two edge
  sums into locals ahead of a `&&` chain reproduces that block exactly and is
  worth delta +28 to +8 and 298 to 291; a logical-or around an inverted test
  is identical to it; a bitwise-or chain collapses to one branch and is 299;
  a bitwise-and chain is 304. The retained `goto done` per test emits
  branch-likely where the target has plain branches.
- Explicitly hoisting the loop-invariant `rectRight`, `rectLeft` and
  `tileBottom` out of the loop is **byte-inert**: uopt already hoists them, and
  the object is identical with and without the source hoist.
- `register` on `gfx`, `source`, `rows`, `mask`, `stride`, `y` and
  `drawHeight` is inert, every one of them, at 291.
- `volatile` on `stride` costs 11 words (291 to 302). The target's one stack
  home at 168 with a single store and two loads is an ordinary spill, not a
  volatile: a volatile local reads five times.
- A `u8 intensity` carrier for the primary-colour command costs a stack home,
  frame 0xF8 to 0x100, at an unchanged count.

Next concrete step: identify the remaining live values the target carries
through the loop -- `s1` holds the assembled texture-rectangle `w0`, `s2` the
rectangle `w1`, `s3` the shifted source column, `s4` the doubled stride and
`s5` the tile bottom, all hoisted -- and find the source form that makes uopt
carry all of them at once, which is what starves the opcode hoisting and drops
the tenth callee-saved register.

#### 2026-09-24, lane w4-o101: three attempts, no better residual

insertion_pairs: size +8, frame +16, label missing-CSE, aligned residual 266. The pair with the shadow is the prologue through the scissor setup. Candidate-only words there are the extra callee-save frame word on the signature, two alus on the GetBounds call, and a load of the texture width for the 0x800 division. The size itself is the two extra callee-save restores at the epilogue.

Packing left/top/right/bottom into one array was byte-inert at 291. Hoisting texture width and height into locals for every use grew the frame to 0x110 and the size delta to -44 (293 masked). A width local around only the division scored 294 at delta +12. All three reverted. The body is the inherited one.

Stall: those three spellings of the missing-CSE pair's owning lines (the bounds addresses, and the width load) produced no better residual, no new identity beyond the array being inert, and eliminated hoisting the texture dimensions. Do not colour-sweep while the size is off. The open lever is still the tenth callee-saved register.
#### 2026-10-02, lane x-o101: SDK GBI macro body, 291 to 263, frame exact

The command stream is the SDK GBI macro set, read off the words: G_DL push
0x06 with D_230, G_SETPRIMCOLOR from the node's intensity and alpha, then
per row `gDPLoadTextureBlockS(gfx++, source, G_IM_FMT_RGBA, G_IM_SIZ_16b,
texture->width, rows, 0, clamp, clamp, nomask, nomask, nolod, nolod)` (the
LoadBlock dxt of 0 is the S variant's; the target's MIN() branch is its
texel clamp) and `gSPTextureRectangle(gfx++, ...)` with dsdx and dtdy
1 << 10 (old GBI: RDPHALF_1 0xB3, RDPHALF_2 0xB2). The source is included
from the tree's include/n_audio/gbi.h with _SHIFTL/_SHIFTR defined as mbi.h
does; without them every _SHIFTL compiles as an implicit call. The texture
rows are a u16 pointer stepped by the stride (the target's pre-loop
stride * 2).

Measured with tools/align_symbol.py, before and after:

  - before (lane p11 body): size +8, positional 291, byte-exact 46, naming
    128, immediate 13, really different 125, displacement tax 25.
  - after: size -8, positional 263, byte-exact 214, naming 52, immediate 8,
    really different 24, displacement tax 179.

The frame closed on a law this function makes easy to read: every declared
local takes a cell, top-down in declaration order, register-allocated or
not. With the bounds declared first they sat at 0xEC..0xE0; the target's
0xB8..0xAC with the stride spill at 0xA8 below them means eleven locals
declared ahead of `left` and `stride` right after `bottom`. One cell too
many remained (0xF0); dropping the nextY carrier (`drawY += chunkRows * 4`,
the rectangle taking `drawY + chunkRows * 4`) gives 0xE8. Merging rowOffset
into sourceY also gives 0xE8 but scores 266 against 264.

Axes measured flat on this body (each a full product): x/y statement order
(2 better than y first), edge sums as locals in either order (inline is +12
bytes), four spellings of edgeY, three spellings of the clip chain, the
0x800 / width quotient as a named local, while against guarded do-while,
and five positions of `source += stride` (end of loop body 263, others 264;
splitting gDPLoadTextureBlockS into its parts to put the increment before
the LoadBlock does not move it into the target's block).

What is left is allocation, all of it in register_census: one global mapping
explains 74% (seven windows). The target computes y into v1 and copies it to
a callee-saved s4 (one target-only move at +0x98), keeps left and top in
s3/s1 before the scissor call where this body loads left straight into a1,
holds the shift in ra and the mask in a3 (here s4 and ra), and its four
hoisted rectangle words sit one register round from these (s1..s4). The
size -8 is those one-sided moves: target-only at +0x98, +0xD4, +0x1BC and
+0x208 plus the dList reload at +0x14C; candidate-only moves at +0x160,
+0x1E0 and +0x22C. The next lever is the y split: find what makes uopt keep
the clip-test y and the post-call y as two webs.

#### 2026-10-02, lane x-o101 (second pass): early returns, 263 to 257

Two more shape facts on the same body, both at size -8 and frame 0xE8:

- The type, null and clip tests as early returns (`if (...) return;`)
  instead of nested ifs: 263 to 257 at the cell level; the aligned residual
  after shadow fell 118 to 73. Separate `if` statements per clip test are
  +16 bytes (286 at +8).
- The 0x800 / width quotient held in `rows` itself (`rows = 0x800 /
  texture->width; if (rows >= 8) ...; rows = 1 << shift;`) puts the
  quotient in s7 as the target does; positional score unchanged, aligned
  residual 73 to 70.

tools/align_symbol.py now: size -8, positional 257, byte-exact 228, naming
38, immediate 8, really different 24.

The y split is reachable from source and is the lead for the next lane.
Spelling the bottom test with the expression (`bottom < node->y +
element->y`, y still assigned from the same expression) makes uopt emit the
target's copy of y into a callee-saved register right after its definition:
259 at size -4. It then assigns node and the y copy s4/s5 the other way
round from the target (one priority fact), so the aligned rows regress;
not adopted.

Delta-0 cells exist and are traps. A `u8 intensity` local for the
primary colour plus reusing x as drawX (or y as drawY) reaches 190-199 at
size 0 with the frame and bounds homes exact, but the zero comes from x or
y being spilled to its frame home where the target keeps it in a register:
align_symbol reads 144 exact, 105 naming, 48 really different on the best
of them, against 228/38/24 here. Do not bank those.

#### 2026-10-04: declaring y first leaves the repeated sum at 259

The configured full-TU baseline is 291 words against 293, 257 differing,
frame 0xE8 on both sides, and an instruction deficit of 2. Declaring y
before x and repeating the y sum in the bottom test recompiled to 292
words, 259 differing, the same frame, and a deficit of 1. That is the
259-word result already measured for the repeated sum alone, so putting
y first did not change the saved-register assignment. The body is not
kept. Do not reopen this function to declare y earlier.

#### 2026-10-04: sourceY from the y sum grows the function

Configured full-TU baseline: size delta -8, 257 raw and masked words, first
mismatch +0x44. Writing both the bottom comparison and
`sourceY = top - y` as `node->y + element->y` measures size delta +4, 268
raw and masked words, first mismatch +0x34. That is 11 more differing words
and 12 more candidate bytes than the baseline, and worse than the
bottom-test sum alone (259 at -4). The extra sum is not folded into the
copy. The body is not kept. Do not repeat this pair.

#### 2026-10-04: computing y before x stays two instructions short

Configured full-TU baseline: 257 raw and masked words, target 1172 bytes,
size delta -8, first mismatch +0x44. Exchanging only the two assignment
statements, so y is computed before x, recompiled to 262 raw and masked
words at the same size delta and the same first mismatch. The missing copy
is not emitted. The restored body re-scores 257 at delta -8. The body is
not kept. Do not repeat this statement exchange.

#### 2026-10-04: copying y into sourceY is coalesced

Configured full-TU baseline: 257 raw and masked words, target 1172 bytes,
size delta -8, first mismatch +0x44. Assigning that y to the existing
sourceY local before the scissor call, and reading sourceY afterwards,
recompiled to the same 257 words at delta -8. The 292-word owned text is
byte-identical to the baseline. The body is not kept. Do not repeat this
copy. It is not the closed repeated-sum spelling.

#### 2026-10-04: a second y-sum recompute stays on the rejected cell

Configured full-TU baseline: 257 raw and masked words, target 1172 bytes,
size delta -8, first mismatch +0x44. Recomputing `node->y + element->y`
for both the height edge and the bottom test recompiled to 259 raw and
masked words, size delta -4, first mismatch +0x3C. That is the same cell
as the bottom-test sum alone. Aligned exact words are 128, naming 139,
immediate 7, and really different 24, against the recorded baseline of
228 exact, 38 naming, 8 immediate, and 24 really different. The restored
body re-scores 257 at delta -8. The body is not kept. Do not repeat this
pair of recomputes.

#### 2026-10-05: recomputing the sum only for edgeY raises the residual

Configured full-TU baseline remains 257 masked and 257 raw words, target 1172 bytes, size delta -8, first mismatch +0x44. Aligned exact 228, naming 38, immediate 8, really different 24, displacement tax 187.

Writing `edgeY` as `(node->y + element->y) + texture->height`, with the bottom test still reading `y`, scored 261 masked words at the same size delta -8. Aligned exact fell from 228 to 224 and naming rose from 38 to 42. The first mismatch moved to +0x3C. The body is not kept. The restored source re-scores 257 at delta -8. Do not repeat this edge-only sum. It does not emit the missing copy.

#### 2026-10-05: computing y before GetBounds does not emit the copy

Configured full-TU baseline: 257 masked and 257 raw words, target 1172 bytes, size delta -8, first mismatch +0x44. Aligned exact 228, naming 38, immediate 8, really different 24, displacement tax 187. Candidate 291 words, target 293.

Computing `y` before `overlay101GetBoundsReloc`, and leaving `x` after that call, scored 270 masked and 270 raw words at the same size delta -8. The candidate stayed 291 words. Aligned exact fell from 228 to 219 and really different rose from 24 to 40. The aligned residual rose from 70 to 81. The missing copy was not gained as a word. The body is not kept. The restored source re-scores 257 at delta -8. Do not repeat this placement.

#### 2026-10-05: computing x and y before GetBounds does not emit the copy

Configured full-TU baseline: 257 masked and 257 raw words, target 1172 bytes, size delta -8, first mismatch +0x44. Aligned exact 228, naming 38, immediate 8, really different 24, displacement tax 187. Candidate 291 words, target 293.

Computing both `x` and `y` before `overlay101GetBoundsReloc`, the order used by matched `overlay101DrawElement`, scored 267 masked and 267 raw words at the same size delta -8. The candidate stayed 291 words. Aligned exact fell from 228 to 218, naming fell from 38 to 37, and really different rose from 24 to 35. The aligned residual rose from 70 to 80. The missing copy was not gained as a word. The body is not kept. The restored source re-scores 257 at delta -8. Do not repeat this placement.

#### 2026-10-05: swapped y-sum addends stay on the rejected cell

Configured full-TU baseline: 257 masked and 257 raw words, target 1172 bytes, size delta -8, first masked mismatch +0x44. Aligned exact 228, naming 38, immediate 8, really different 24, displacement tax 187. Candidate 291 words, target 293.

Writing the bottom test as element->y + node->y, with y still assigned from node->y + element->y, scores 259 masked and 259 raw words at size delta -4. The candidate grows to 292 words. Aligned exact falls from 228 to 128, naming rises from 38 to 139, immediate falls from 8 to 7, and really different stays 24. The aligned residual rises from 70 to 170. Absolute size delta falls from 8 to 4, but the residual rises, so the body is not kept. These buckets are the same cell as the closed node->y + element->y recompute. The restored source re-scores 257 at delta -8, and the 1164-byte function text matches the proved baseline. Do not repeat this addend order. The repeated sum stays closed.

#### 2026-10-05: narrowing y to s16 grows the residual

Configured full-TU baseline: 257 masked and 257 raw words, target 1172 bytes, size delta -8, first masked mismatch +0x44. Aligned exact 228, naming 38, immediate 8, really different 24, displacement tax 187. Candidate 291 words, target 293. Owned text is 1164 bytes.

Declaring y as s16, with x left as s32 and the clip expression unchanged, scores 263 masked and 263 raw words at size delta +4. The candidate grows to 294 words and 1176 bytes. The first masked mismatch stays +0x44. Aligned exact falls from 228 to 127, naming rises from 38 to 136, immediate falls from 8 to 6, and really different rises from 24 to 33. The aligned residual rises from 70 to 175. Absolute size delta falls from 8 to 4, but the residual rises, so the body is not kept. The restored source re-scores 257 at delta -8, and the 1164-byte function text matches the proved baseline. Do not repeat an s16 y.

#### 2026-10-07: a y origin local for the clip tests reaches delta 0

Lane a-ovl3. Configured baseline 257 masked at size delta -8, aligned exact 228. The clip tests now read `originY = node->y + element->y` (declared where `edgeY` was, so the frame stays 0xE8), the bottom edge is written inline as `originY + texture->height`, and `y = originY` is assigned after the tests, before the scissor call. Result 90 masked at size delta 0, aligned exact 229, naming 36, immediate 4, really different 33. That is the target's split of y: the sum in v1 for the tests and a callee-saved copy across the call. Measured on the way: y computed before x (262 at -8), the sums inline in the tests with y assigned after (263 at -4), y assigned after the scissor call (270 at +8), y = originY before the tests (90 at 0, 226 exact), the bottom edge held in any existing later local (drawY, drawHeight, sourceY, chunkRows, rowOffset, y: all 257 at -8).

Left: the target computes the bottom edge (a1) before the tests where this body adds it in the last test, holds left and top in s3 and s1 for the scissor call, and keeps the shift amount in ra.
#### 2026-10-07 (second pass): the y copy only survives when the copy's source is redefined

Lane a-ovl3. Records (CDX_PROC=0, 66 decisions, 16 split) on the early-edge shape: `y = originY` is copy-propagated into the originY web (s3) whenever originY is not redefined afterwards, so y never becomes its own web and there is no `move s4, v1`. Fourteen cells (y copied before or after the tests, crossed with the bottom edge inline or held in drawY, drawHeight, sourceY, chunkRows, rowOffset or y) are all 90 at 0 (inline) or 257 at -8 (any carrier). A separate edgeY local beside originY gives the target's clip block exactly (edge sums in the first block, all four tests, scissor), except that y stays one web, and it costs a frame cell (264 at -8, frame 0xF0). Redefining the sum after the copy (`y = edgeY; edgeY += height`) does produce the copy (`move s5, v0`), but the bottom test then reads the copy where the target reads the sum: 257 at -4 with x first, 260 at -4 with y first. Putting the `+=` inside the last test, or testing the raw sum inline, gives 258 to 265.

Next: the target holds three values (the sum in v1, the edge in a1, the copy in s4), so the source needs one local more than this body and one fewer somewhere else. Find which declared local the target lacks; frame_census on the 264 cell gives the extra cell.
#### 2026-10-07, lane c-near: an edgeY local in originY's cell, sum tested inline

On the 90 body (1172 bytes, 90 masked, size delta 0). Reading the target's clip block again: x is one web (s2) used directly by the tests, while y is a sum in v1 used by the tests and a separate copy in s4 across the scissor call, and both edges are summed before the first test. The frame-neutral way to hold three y values is to replace the originY declaration with edgeY (same cell) and test the sum inline: `edgeY = node->y + element->y + texture->height` before the tests, `bottom < node->y + element->y` in the second test, `y = node->y + element->y` before or after the tests.

That does produce the copy (`move s4, v0`, the sum is a CSE temporary and y its own web) but costs 4 bytes: 261 to 268 masked at size delta -4 or +4 over 14 cells (three statement orders of x, edgeX and edgeY, both addend orders of edgeY, y before or after the tests). Aligned against the 90 body the naming rows rise in every window (+106 aligned). Not kept.

Next: frame_census the a-ovl3 264 cell (edgeY beside originY, frame 0xF0) and find which other declared local the target lacks, so that cell can be paid for elsewhere; the inline-sum route above is closed at -4.
#### 2026-10-07 (lane d-mid1): the 264 cell's extra frame cell is any declared local

On the 90 body (1172 bytes, 90 masked, size delta 0, aligned exact 229, naming 36, immediate 4, really different 15).

frame_census on a-ovl3's 264 cell (edgeY declared beside originY, `edgeY = originY + texture->height` with x's edge, tested as `edgeY < top`; reproduced at 264, size delta -8, frame 0xF0): both sides use 16 slots and the ladders are identical below +0x40; above it every home (the GetBounds outputs and the argument save) sits 8 bytes higher. The gap between the callee saves and the homes is the declared-local reservation, so the extra cell is the count of declared locals, not a particular home: folding any one local back restores 0xE8. Measured, one local removed from the 264 cell each:

- edgeX dropped (x's edge inline in the third test): 88 masked at size delta 0, aligned exact 230, naming 33, immediate 2, really different 20. The aligned residual is the 90 body's 55 again, and the clip block is not the target's (x's edge is summed in the test block, left and top are not held in s3/s1). Not kept.
- the edge held in drawX, drawWidth (x) or drawY (y) instead of a local: 257 at -8. stride inline at both uses: 289 at -8.

insertion_pairs on the 264 cell (trace identity-gated): the -8 is three target-only moves, the y copy into s4 at +0x98 (line of the tests), a move at +0xD4 (line 117, move_to_dest) and one at +0x1BC, against one candidate-only stack load at +0x158. So that cell is short of exactly the copies, and its frame cell is free to pay for with edgeX.

Next: on the 264 cell with edgeX folded (frame 0xE8), find the source of the +0xD4 and +0x1BC moves (insertion_pairs owners: line 117 and line 142 of that cell) before the y copy; the y copy needs the sum redefined after `y = originY` (a-ovl3), which so far moves the last test onto the copy.
#### 2026-10-07 (lane e-ovl2): statement order and the copy position are inert on the 90 body

On the 90 body (1172 bytes, 90 masked, size delta 0). An 8-cell product: x and originY assigned in either order, `y = originY` before or after the clip tests, crossed with the edgeX local or x's edge inline (edgeX still declared). Every edgeX-local cell is 90 and byte-identical (uopt orders the two sums itself; the target's ugen order, y's loads first, is not a statement-order fact); every inline cell is 269 at +12. Records on the 90 body (CDX_PROC=0, identity gate passed, 64 decisions): twelve webs are split (318 to 329, save 3.33 over 3 blocks, the hoisted loop constants), and six webs tie at save 3.67 over 3 blocks taking s1 to s5 in web-number order.

Cycle-21: on the 90 body, read whether the target's y is one web split by the allocator around the scissor call (the `or s4, v1` at +0x98 sits at the split point), against this body's originY copy that uopt propagates; the lane that closed overlay 1's F0001D78 found the word/pointer double spelling of one object was the artefact, so check that no other value here is held under two names before forcing.
#### 2026-10-07 (lane g-3): the copy in the clip block is reachable, the ring phase is not

On the 90 body (90 masked, size delta 0). The originY local is copy-propagated: the same body with y assigned the sum directly and originY unused is byte-identical (90). Measured (masked, size delta):

- edgeY held in the originY cell (`originY = y + texture->height` before the tests, x's edge in edgeX): 257 at -8. The clip block is then the target's (both edges summed before the first test, left and top loaded for the tests) but y stays one web and the -8 is the y copy plus the argument moves.
- A 16-cell product: the bottom test on y or on the repeated sum, the edge local from y or from the sum, the edge inline or in the originY cell, y assigned before or after the tests. Best valid cells: 90 (the tracked body), 101 at 0 (sum tested inline, y assigned after the tests: the copy appears, but in the last test block), 259 at -4 (y assigned the sum before the tests, bottom test and edge local on the repeated sum: the copy is emitted in the first block exactly where the target has it, aligned exact 128, naming 139, immediate 7, really different 24).
- On that 259 cell the records give element 0.857 (s4) over y 0.667 (s5); forcing y to s4 and element to s5 (accepted) scores 255 at -4, really different 13. The rest is a temp-ring phase (t6/t8 and t7/t9 exchanged in every window), the shadow of the one missing word, the left argument move (the target loads left into s3 and copies it to a1).
- Carrying the y sum in another local (drawY, sourceY, drawHeight, rowOffset) with the edge in the originY cell: 129 to 271; the best two (rowOffset, drawHeight) spill y to the frame.
- An allocator split of the y web on the 90 body or the edge-local body (`p1:w22=s`, accepted): 236 to 270 at +4 or +12, y spilled.

Cycle-21: on the 259 cell the open question is the target's left in s3: the left value loaded for the third test and copied into a1 for the scissor call. Read that web's p1cost list on the 259 cell (forbidden mask against the scissor call's a0-a3) and find which shape makes it a callee-saved web; that is the missing word, and with it the ring phase.

#### 2026-10-07 (lane h-3): bottom is the missing web

On g-3's 259 cell (y assigned the sum before the tests, the bottom test and the edge local on the repeated sum; reproduced at 259, size delta -4). Records with the instrumented compiler (proc 0, 66 decisions; the instrumented object scores the same 259). The clip block's webs: the y sum w26 (v0, totalsave 4 over nocs 2, blocks 6-7), x's edge w28 (v1), y's edge w33 (a0), left w48 (a1), top w53 (s1), the y copy w22 (s5), node w0 (s4), shift w66 (s3). left's web spans the scissor call (blocks 8, 9, 12, 19) and its cost list offers only a1 at 0, s3/s4/s5 at 0.1 and every t register at 1e20. right and bottom are not webs at all: they are loads held in t9 and t6 across the test blocks.

The target has bottom in v0, the y sum in v1, x's edge in a0, y's edge in a1 and left in s3: each of the four clip webs exactly one colour later than here, which is what a bottom web taking v0 first produces, and with a1 taken left has nothing cheaper than s3. Forces w26=c2, w28=c3, w33=c4, w48=c17 (all accepted) give the target's clip block colours and score 225 at +4: the y copy is then denied s5/s4 (shift takes s4) and spilled. Adding w0=c19 and w22=c18: w22 is never applied (forced=-2).

Measured negative: bottom carried in a register local (a new local, or reusing chunkRows, drawHeight or rowOffset; read in the test only, or in the test and the call, or assigned inside the test), 16 cells: 259 to 269 at -4. uopt propagates the copy and bottom stays a t-register load.

Cycle-21: the decision variable is whether bottom's load is a uopt web. Read why left and top become load webs here and bottom and right do not (their block spans and use counts in the records), and find the source in which bottom's load is a web coloured ahead of the y sum. No register-local spelling reaches it.

#### 2026-10-07, lane i-3: why bottom is not a web, read from the records

Measured by tools/bank.py: masked 90 (raw 90), size delta +0, candidate 293 words vs target 293. Aligned: byte-exact 229, register naming 36, immediate only 4, really different 33.

On the 90 body (90 masked, size delta 0, aligned exact 229, naming 36, immediate 4, really different 33). web_report, proc 0, identity gate passed, 64 decisions.

- left (web 43) and top (web 50) are symbol webs of the address-taken locals, not load expressions. left: occurrences bb8 (the third test), bb12 (the scissor call), bb19 (after the call), gross 4, chargeA 3, net 1, nocs 2, save 0.5, coloured a1. top: bb12, bb23, bb24 (its test block bb9 is not an occurrence), gross 4, chargeA 2, save 1.0, coloured s1.
- right (web 33) has one occurrence, bb12, gross 1, chargeA 1, net 0, verdict 2: never coloured. Its test-1 use is not part of the web.
- bottom (var -60) has no symbol web at all: it appears only inside expression webs (`bottom - drawY`, web 120). It is the one bound whose address reaches GetBounds through the stack (the fifth argument), the likely reason it is not a promotion candidate.

The shipped code has bottom in v0, loaded in the second test's block and stored as the scissor call's fifth argument: a web over bb7 to bb12. So the target's bottom is either a promoted symbol (its address not escaping through memory) or an expression web whose two loads were made one. Neither is reachable by spelling the clip tests (the closed products above). No source cycle was spent here.

Cycle-21: decide which. Compile the 90 body with the GetBounds call's fifth argument passed through a pointer local (`s32 *bottomOut = &bottom;`), or with bottom's uses after the call read from a copy, and read whether a bottom web appears in the p1 records (web_report) before scoring.

#### 2026-10-07, lane i-3 (resumed): a copy of bottom is propagated

Measured by tools/bank.py: masked 90 (raw 90), size delta +0, candidate 293 words vs target 293. Aligned: byte-exact 229, register naming 36, immediate only 4, really different 33.

On the 90 body. bottom copied into a local after GetBounds (a new local, or chunkRows reused) and the copy used by the second test and the scissor call: 166 and 163 at 0. uopt propagates the copy; the load stays a ring temporary (t8) carried to the call, and no bottom web appears. Not kept. Cycle-21 unchanged (decide whether the target's bottom is a promoted symbol or one load expression; the fifth argument's stack escape is the candidate cause).

#### 2026-10-08, lane j-3: bottom never forms a web; pointer, address, in-place and nesting forms all measured

Measured by tools/bank.py: masked 90 (raw 90), size delta +0, candidate 293 words vs target 293. Aligned: byte-exact 229, register naming 36, immediate only 4, really different 33.

On the 90 body (size delta 0, aligned exact 229, naming 36, immediate 4, really different 33). web_report, proc 0, identity gate passed.

Records: the three bounds that are register arguments of the scissor call (left a1, top a2, right a3) each get a symbol web from that call's argument pin (bbpin in block 12); bottom, the stack argument, has no pin and no web. Left's and top's occurrences each carry their own load charge (left: gross 4, chargeA 3, net 1), so outside a pin an address-taken local gets a web only where one block uses it twice. Right's test in block 6 and bottom's test in block 7 are not part of any web.

Measured, all with no bottom web:

- &bottom through a pointer local for the GetBounds argument: 100 at 0 (the pointer takes a frame cell). Reading bottom through the pointer at the test and the call, or everywhere: 100 at 0; uopt folds the pointer back to &bottom.
- `*&bottom` and `*(s32 *)&bottom` at the test and the call: 90, byte-identical.
- The four bounds as one array (`bounds[3]` for left down to `bounds[0]` for bottom): 90, byte-identical.
- `bottom -= drawY` (and/or `right -= drawX`) in place after the call: 235 to 246 at +4 or +8 (the store back).
- A 12-cell product of the clip shape (90 body, g-3's 259 cell, y-first 262 cell) by if/else or default-then-override for the x and y clamps: floor 90 (if/else both).
- The clip tests as one positive `&&` condition wrapping the rest of the body, on each clip shape: 259 to 276; the early-return form stays best.

Cycle-21: bottom's web cannot come from a spelling of bottom; every alias spelling folds to the same stack variable. Settle on a mini TU whether IDO ever gives an address-taken local with no register-argument pin a web (a local passed by address, then read in a test block and stored as a later call's stack argument). If it never does, the target's v0 value is not that local's own web, and the question becomes which value live over blocks 7 to 12 the target's source has that ours lacks (the decision variable is v0's occupant there; the record is the p1color row for c1 over blocks 7 to 12).

#### 2026-10-08, lane k-2: bottom's web needs no indirect load in its blocks; the clip shape priced at 51 under four forces

Measured by tools/bank.py: masked 90 (raw 90), size delta +0, candidate 293 words vs target 293. Aligned: byte-exact 229, register naming 36, immediate only 4, really different 33.

On the 90 body. j-3's reading (bottom has no web because the stack argument has no pin) is wrong. Mini TUs with the tree's IDO and the instrumented uopt (web_report on the saved log): an address-taken bound passed as the fifth, stack, argument does get a symbol web (t0 or v0) when no indirect load occurs between its first test and the call; one `texture->height` load inline in the last test, or the repeated sum `node->y + element->y` in the bottom test (even CSE'd), removes the web entirely, and with it the cached right in a3. The 90 body tests `originY + texture->height` inline, so bottom is a t-register load there. The target computes both edges in the first block and its tests read only locals.

Measured on the tracked TU (shape_product, masked at size delta; aligned exact, naming, immediate, different):

- Both edges before the tests in the 90 body's two locals (y one web): 257 at -8 (228/38/8/24). bottom becomes a web (t0) and right is copied into a3 as shipped, but y has no copy.
- The y sum in a later-role local (drawY, sourceY, drawHeight, rowOffset, chunkRows, drawWidth) with y copied from it at four positions and the y edge from the sum or from y, 48 cells: best valid 267 at -4 (223/49/4/21). Every such local is one web with its post-call role, so the sum spans the call and takes a saved register.
- The target's clip block exactly: originY the sum, `y = originY` beside it, edgeX and a new edgeY local before the tests, then a dead redefinition of originY after the tests (`originY = 0`, a cast, an or-zero or `originY = y`, all identical) so the copy is not propagated: 265 at -4, frame 0xF0. The copy is emitted in the first block where the target has it, the sum is a short caller-saved web, bottom is a web.
- The same with rowOffset folded into sourceY (`sourceY = (sourceY & (rows - 1)) << 5`, three spellings identical; the target computes the mask in sourceY's register): frame 0xE8, 260 at -4 (235/34/8/19, residual 61 against 73). Not banked: the size is the left argument move.
- GetBounds declared to return s32 (result unused): byte-identical; v0 is not added to any block's mask.

Records on that last cell (proc 0): originY save 2.0 (v0), edgeX 1.0 (v1), edgeY 1.0 (a0), left 1.0 (a1), bottom 0.5 (t0). Forcing originY to c2, edgeX to c3 and edgeY to c4 (all accepted) gives bottom v0 and left s3 unforced and scores 60 positional at size delta 0 (253 exact); adding node to c19 gives y s4 and 51 (262/19/2/7). Forcing bottom to c1 alone is never applied (v0 already taken).

Cycle-21: on the 260 cell, the three clip webs defined after GetBounds must be denied v0 (or bottom must outrank a save of 2.0, which two references cannot). Decision variable: v0 in the forbidden mask of webs 22, 31 and 36 in the block after the GetBounds call. Find what source delivers v0 into that block (item 38) without a use the listing lacks; then node over y for s5/s4.
<!-- plateau-handoff:func_overlay_101_F0002510_18DDD30:end -->
