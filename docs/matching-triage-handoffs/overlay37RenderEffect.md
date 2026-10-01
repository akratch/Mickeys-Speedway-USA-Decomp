<!-- plateau-handoff:overlay37RenderEffect:start -->
### `overlay37RenderEffect` plateau handoff

- source: `src/overlays/o037/overlay37Render.c`
- score: 0/214 words, promoted
- frame: 0x98
- relocations: 12
- first mismatch: none
- summary: Matched. Packet macros for the five commands, a one-argument closing call, and the locals declared in frame order.

#### 2026-10-01, lane b-o101: matched by rewriting the inherited shape

Promoted at 214 of 214 words, delta 0, frame 0x98, 12 of 12 relocations.
Three inherited parts of the candidate were replaced; none of the retained
43 words was a colouring question.

First, each display-list command is written as its packet macro taking the
post-incremented cursor, so each site has its own block-scoped pointer. On
the old declarations that alone scored 56 with the frame 0x10 too large,
because the five macro pointers take frame cells of their own.

Second, the locals are declared in the order the frame shows: frame, state
and record above resource, camera and the transform, then the three deltas,
the two blend scalars and one unused word above red, green and blue, with
nothing declared below them. The five macro pointers fill the cells under
the colours, which is what the two pad arrays had been standing in for.
That layout scored 15 with every register agreeing and the object two words
long.

Third, the two extra words were the closing call. The candidate passed the
vertex address as a second argument because the target leaves that address
in the second argument register. The resident callee pops a model matrix and
takes the cursor only; the register simply still holds the vertex command's
operand. With the argument present uopt either rebuilt the address before
the call, two words long, or with an integer cast carried it in a different
register, 2 words at delta 0. With the argument removed the function is exact.

The nested-assignment carrier in the middle blend arm is not needed: two
plain statements score the same. The integer literals there are kept because
the constant pool is keyed on spelling.

Closures this breaks: the green-reload-before-cursor-load stall and the
transform-home displacement were both consequences of the shared command
local and the pad arrays, and the single-force colour reading for the
0xFA000000 constant was true only of that shape.

Gates: gmake verify, check-overlay-syms and promotion-proof pass.


Summary before this remeasure: Size 0 at 61 via r4300_mul, 0x04000000 with 0x80, L112 pad[4]. Colour floor 61, 58 with p1:w184=c7. Extra pair +0x26C vs +0x284; transform homes 0x60 vs 0x6C.

Identity-gate PASS on stock vs instrumented IDO 5.3 (`CDX_PROC=0`, 28 p1 decisions). Configured `-mips2` without `-Wab,-r4300_mul` was 213 words, delta -4; the missing word is the else-arm mul-hazard nop. Adding `-Wab,-r4300_mul` on this single-function TU closes size to 214 words at frame 0x88 (masked 68). Spelling `0x04000000` with `0x80` instead of `0x04000080` closes one extra pair. An unused `f32 pad[4]` (L112) grows the frame to the target `0x98` and drops the unforced score to 61.

Aligned at that body: 155 byte-exact, 34 naming, 18 immediate, 8 really-different; one candidate-only word at +0x26C and one target-only word at +0x284. Transform homes remain at +0x60 versus the target +0x6C, which is the 18-immediate bucket. The extra pair is the first prim-color block reloading green before the command pointer.

`--every-colour` (132 probes, 25 webs) has one delta-0 winner: `p1:w184=c7` scores 58. Web 184 is the `0xFA000000` constant (type-2, currently a2; target keeps it in t0). L159 packing is that single force. Colour cannot move the extra pair or the transform homes.

Tried and rejected on this body: countdown index into `gOverlay37Records` (110, frame 0xA0), generated subscript without the record pointer (204, delta +16), postfix `(*commands)++` (75), L97 `if (1)` around the blend (96), L144 volatile colors (177, delta -12), L103 else-arm `(f32)239` recasts (180, delta +8), pad[3] (83, frame 0x90), transform-first declaration (65). Deleting `distanceDelta` is load-bearing (180, delta +4).

Next: a source form that emits the command-pointer load before the green reload, and that places the transform at +0x6C rather than +0x60. Do not re-run the colour landscape until that extra pair is gone.
<!-- plateau-handoff:overlay37RenderEffect:end -->
