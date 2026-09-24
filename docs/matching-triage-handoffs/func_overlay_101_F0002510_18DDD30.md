<!-- plateau-handoff:func_overlay_101_F0002510_18DDD30:start -->
### `func_overlay_101_F0002510_18DDD30` plateau handoff

- source: `src/overlays/o101/func_overlay_101_F0002510_18DDD30.c`
- score: 291 differing words
- frame: 0xF8
- relocations: 6
- first mismatch: +0x0
- summary: 291 words, size +8, frame 0xF8 vs 0xE8. Bounds-array and width-local spellings did not beat it.

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
<!-- plateau-handoff:func_overlay_101_F0002510_18DDD30:end -->
