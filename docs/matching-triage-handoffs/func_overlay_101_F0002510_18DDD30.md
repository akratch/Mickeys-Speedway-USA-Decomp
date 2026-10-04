<!-- plateau-handoff:func_overlay_101_F0002510_18DDD30:start -->
### `func_overlay_101_F0002510_18DDD30` plateau handoff

- source: `src/overlays/o101/func_overlay_101_F0002510_18DDD30.c`
- score: 257 differing words
- frame: 0xE8
- relocations: 6
- first mismatch: +0x44
- summary: 257 words at size -8, frame 0xE8 exact, SDK GBI body with early returns. Left: the y split, left/top in s3/s1, rotated hoisted rect words.

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

<!-- plateau-handoff:func_overlay_101_F0002510_18DDD30:end -->
