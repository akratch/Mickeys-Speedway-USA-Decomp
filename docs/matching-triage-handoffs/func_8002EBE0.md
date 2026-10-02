<!-- plateau-handoff:func_8002EBE0:start -->
### `func_8002EBE0` plateau handoff

- source: `src/main/rcpFast3d.c`
- score: 89/255 words
- frame: 0x88
- relocations: 2
- first mismatch: +0x138
- summary: Listing rewrite: delta 0, frame exact. Left: 39 naming rows and three copies from the post-decrement loop webs and the colour copy.

Summary before this remeasure: Exact-sized C keeps a 0x58 versus 0x88 frame after RGB aggregate and lifetime forms; next lever is an authentic early-live-web source shape.
#### 2026-10-02 (lane `w2-front`): rewritten from the listing, 218 -> 89 at delta 0, frame exact

The inherited m2c body was dropped and the function rewritten from the
target listing. The edits below are in the order they moved the count. Every
step was measured as a `tools/shape_product.py` product.

1. **Natural loops** (218 -> 221 at delta 0, structurally closer). These are
   `while (screens--)`, an eight-band `i = 0; do { i++; ... } while (i != 8)`
   (the target increments at the top), and `while (steps--)` for the gradient
   steps.
2. **The band loop increments at the top** (`do { i++; ...} while (i != 8)`):
   221 -> 169.
3. **No `bandEnd` variable** (154 -> 85). The target computes
   `bandStart + screenHeight` straight into s6 in each branch and then copies
   it to bandStart. That is uopt's CSE of the expression, written as
   `bandStart += screenHeight` at the bottom with the expression inline in each
   branch. A declared bandEnd always produced a temp-then-copy.
4. **`y += 2` and `y += steps * 2` with the end row inline** in
   gDPFillRectangle. A nextY local adds a web that takes ra and pushes cmd
   into s0.
5. **Frame.** The target's homes put `screens` (0x48) above `cmd` (0x40),
   with one cell between and 15 above. `s32 pad[15]; s32 screens; s32 pad2;
   RcpCommand *cmd;` reproduces the 0x88 frame and the whole slot ladder
   (frame_census: 15 slots each side, identical). This is L112's free
   parameter. The array is not claimed as the original's declaration.
6. **Flat fill uses GPACK inline, and `colour` is `u32`** (83 at +4 -> 89 at
   delta 0). With an s32 colour the flat branch keeps a colour copy that the
   target lacks.

What is left, per align_symbol on the banked body: byte-exact 207, naming
39, immediate 2, structural 9. There are two one-sided pairs: a candidate-only
word at +0x2B0 and +0x3A0, and a target-only word at +0x29C and +0x2A4.

- **Gradient-loop preheader.** The target copies `steps` (v1, the divisor)
  into v0 for the post-decrement temp before `beqz`. Ours gives the divisor web
  v0, so the copy coalesces away and the branch becomes `beqzl`. The decision
  variable is the colour of the pre-loop steps web: v1 in the target, v0 here.
  Eight exit-test spellings (`steps--`, `!= 0`, `> 0`, `for` forms) by four
  colour types were flat.
- **Colour copy in the gradient loop.** The target keeps `colour = rgb OR 1`
  as a copy into v1 at the end of each step. Ours drops it with a u32 colour
  and keeps it with an s32 colour, which then also copies in the flat branch.
- **Screens loop tail.** Ours loads `screens` into v1 and copies it to v0.
  The target loads straight into v0. This copy came and went with the colour
  type in every product, so it is the same allocator ordering.

The naming residual is two closed cycles (a0/t0: gradient counter against the
second packet pointer; v0/v1). register_census reads 95% coherent. Next
pass: dump the procedure-5 records (CDX_PROC=5 in this TU) and price the
steps-pre web onto v1 with a force. If one force removes the preheader copy
pair, the open question becomes which declaration stands in its way (L160).

Axes measured on this shape and flat: colour as a var, inline, the OR 1 split out or
as a compound OR-assign; r/g/b as s32, u8 or u32; colour as s32, u32, u16 or s16; the outer loop
as `while`, `!= 0` or `for`; `y = 0` before or after screenHeight (after is
2 better).
<!-- plateau-handoff:func_8002EBE0:end -->
