<!-- plateau-handoff:overlay100DrawMotion:start -->
### `overlay100DrawMotion` plateau handoff

- source: `src/overlays/o100/overlay100DrawMotion.c`
- score: 113 differing words
- frame: 0xC0
- relocations: 7
- first mismatch: +0x194
- summary: Void getters, GBI macros, if/else alpha clamp: 155 to 113. Left: preheader colour/alpha order, inner-loop FP colours.

Summary before this remeasure: L99 unused pointers declared first close frame 0xC0 with exact homes. Packed RGB hoist is plus two words. Command/color lifetime remains.
- assignment base: `b05cf692e3fc02d376d334564fed1d6c1e0a8953`
- owned range: overlay 100 `+0x580..+0x94C`, 972 bytes / 243 words; the following four-byte padding is separately owned
- identity gate: instrumented IDO `.text` byte-identical to stock; `CDX_PROC=0` (47 p1 decisions)
- live: 972 bytes, size delta 0, masked 155, frame `0xC0` matching the target, 15 of 15 stack slots identical including traffic at `+0xC0` / `+0x9C` / `+0x98` / `+0x58`
- aligned: 146 byte-exact, 48 naming, 0 immediate, 69 really different; displacement tax 38
- first mismatch: positional `+0x0`; first naming `+0x58`; first structural `+0x34`
- relocations: seven records (five calls and one HI16/LO16 pair)
- frame close: four unused pointers declared first (L99) grow `0xB0` to `0xC0` at unchanged instruction count. Unused f32s and unused pointers declared last are eliminated. Four projection scalars then the two volatile color homes then `commands` last place every home on the target ladder (161 to 157 to 155; immediate 8 to 0)
- packed RGB hoist: a pre-loop packed color (new local, `progress` reuse, block-scope `register s32`, or in-place `red`) removes the 11-word target-only block at `+0x194` but emits two extra words (980 bytes) and was not adopted
- other lifetime probes: remaining saved in `x` is 968 bytes; remaining plus a `start` cursor is size 0 at 187 masked; empty `if (row) {}` is plus two words. None beat 155
- blocker: command and color lifetime still rebuilds RGB inside the loop and reloads `colorA0` / `colorA1` / remaining / `commands-1` instead of keeping the target's live `a0`/`a1`/`a2`. Next lever is a size-0 spelling of that hoist, not another frame or FP term-order edit

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0x0.
#### 2026-10-02, lane g-ovl5: no change, 155

- Loop spellings (`while (count--)`, `for` count-down, count-up): count-down
  and while forms are 155 (canonicalised), count-up 225; `count-- != 0`
  against `count--`: inert. Giving the constant 3 its own local: inert.
- Volatile off on green and blue: 184 at +8 (as before).
- Declaration-order hill climb over the 16 declarations (comma lists split):
  floor 155.
- Target colours: s1 holds the divisor 3 and ra holds the sync opcode; this
  build swaps them. Colour order of those two constants is the open variable.

#### 2026-10-05: the null-check schedule does not move

Configured full-TU baseline remains 155 masked and 155 raw words, target 972 bytes, size delta 0, first mismatch +0x30. The target saves the callee FP register and then tests the motion pointer. The candidate tests first.

Wrapping the body in a nonzero test compiles to the same text as the early return. Not kept.

Copying the incoming command pointer into the first frame slot before that test, and reading the copy at every later use, scores 156 masked words at size delta 0. Not kept. The 155-word body stays. Do not repeat either spelling.
#### 2026-10-02, lane x-ovla: natural rewrite measured, not adopted (155 kept)

A body written from the listing (no volatile, no register, `while (row--)`,
`while (count--)`, the alpha step as `alpha = alphaStep * (3 - row)` inside
the loop so uopt strength-reduces it, the colour word and constants hoisted
by the compiler) keeps the 26-cell declaration list and lands frame 0xC0 and
the green/blue spills at 0x9C/0x98 without volatile, but scores 191 to 192 at
size -4 against the inherited 155 at delta 0. Findings to carry:
- With u8 parameters on the angle-preparation call each colour argument gets
  an `andi`; the target passes the raw `lbu` values (s32 parameters), though
  on the inherited body this alone is flat (155).
- The target's first argument to that call is the segment packet's own
  pointer (`or a0, a3, zero` before the two stores, never recomputed); every
  spelling tried either recomputes `commands - 1` or grows the frame
  (a named packet pointer in place of an unused pad: 207 at +8, frame 0xE0).
- `if (motion != NULL) { ... }` against an early return, and `* 4` against
  `<< 2`, are flat.

#### 2026-10-03, getter-arity packet: negative transfer after diagnostic cleanup

The shipped relocation identities resolve the two getter calls to matched
`func_8002468C(void)` and `camGetPtr(void)`. Their Matrix and Camera return
storage agrees with this caller's existing read views. Removing the original
one and four pure argument evaluations preserves runtime callee behavior;
this does **not** establish the retail caller's original source signature,
which could still have passed ignored carrier arguments.

Stock configured contrasts, in order (differing words, candidate words,
size delta, frame, first mismatch):

- Unchanged inherited baseline: 155, 243, 0, `0xC0`, `+0x30`.
- Both getter declarations/calls changed to no arguments: 185, 240,
  -12 bytes, `0xC0`, `+0x40`.
- Both volatile color qualifiers removed: 162, 239, -16 bytes,
  `0xC0`, `+0x40`; four unused pointer locals still remained diagnostic.
- All four unused pointer locals removed: 164, 239, -16 bytes,
  `0xB0`, `+0x0`. This clean body remains structurally nonexact, with a
  16-byte non-save frame deficit; the save region is unchanged.

Actual stock compiler-input capture, section/symbol/relocation fidelity,
and unchanged self-context checks pass for the baseline and clean body.
The approved getter declaration changes are disclosed as changed context,
not presented as an unchanged-context candidate. Both objects retain the
same seven ordered relocation types and identities, at shifted offsets;
that is not exact target relocation geometry or linked promotion proof.

The getter behavior is authenticated, but this arity correction did not
explain the retail instruction extent or produce an admissible match.
Stop: no independently justified fresh mechanism remains after the three
controlled contrasts. Do not repeat the exhausted packing, frame, ordering,
or padding probes. All sources, objects, scores, and causal receipts remain
private. The original tracked body is restored solely as the unchanged
155-word diagnostic baseline: its volatile homes and unused pointers gain
no new admissibility or matching credit. No candidate source is adopted.

#### 2026-10-07, lane a-ovl2: GBI-macro rewrite fixes the loop preheader, not the colours

Not adopted; the 155 body stays. Read from the listing: the colour word's red term carries a deleted `& 0xFF` (the `or t9,s3,zero` copy before the shift), so the source used `_SHIFTL`-style masks on all four channels, i.e. `gDPSetPrimColor(commands++, 0, 0, red, green, blue, alpha / 3)`; the other packets are `gSPDisplayList`, `gDPPipeSync` and `gDPFillRectangle(commands++, x, y, x + 1, y + 1)`. The colour arguments to the angle-preparation call are the CSE'd `lbu` loads in a1/a2, so its parameters are `s32`.

A body written with those macros, non-volatile green and blue, `motion->remaining` read at each use, `while (row--)` and `while (count--)`, produces the target's preheader (0xFA000000 in s3, the divisor 3 in s1, 0xE7000000 in ra, 0xF6000000 in t1, 120 in t0, 1.0f and -10.0f in f24/f22, the packed RGB hoisted) and the target's loop body, but the frame is 0x90 to 0xD8 and red lands in memory instead of s3. Products over the packet pointer passed first to the preparation call (start local in five spellings, or `commands - 1`), three alpha-step forms, alpha strength-reduced or explicit, volatile or not and word order inside the macros: floor 173 masked at +8, and 204 at +8 for the non-volatile cells. In every cell with a `start` local that pointer takes s3 and colorA0 takes s1, where the target has start in a0, colorA0 in a1, colorA1 in a2, colorA2 in t0 and remaining in t1.

#### 2026-10-07, lane c-o066: void getters plus the listing's shape, 155 to 113

Adopted. The relocation finding that the two getters take no arguments was
the missing half of a-ovl2's GBI rewrite: with `func_8002468C(void)` and
`camGetPtr(void)` declared that way, the packet pointer, colorA0, colorA1 and
the command cursor sit in a0-a3 at those calls as the shipped leftovers, and
the rewrite (GBI macros, plain non-volatile green and blue, `while (row--)`
and `while (count--)`, four unused pointers first) lands frame 0xC0 with all
15 slots on the target ladder.

Measured, whole-TU products with fast_score (masked, size delta):

- natural body, alpha-step as default-then-override: 155 at +4; with
  volatile colours 169 at -4.
- alpha-step clamp as `if (remaining >= 64) step = 255; else step = remaining * 4;`
  (the target's branch over an empty arm): the decisive axis. With an alpha
  local assigned in the loop, 127 at 0 (frame 0xB0); the else-first spelling
  131; default-then-override 151 at -4.
- alpha inline in the colour macro: 100 at +8 then 83 at +8 with the frame
  fixed; the two extra words are a `mov.s` of the reciprocal into the
  inverseDepth register plus its nop, which the target does not have.
- count read before the phase wrap: 113 at 0 (adopted) against 123 after it
  and 114 before the frame pointer read.
- four unused pointers: frame 0xC0 and the full ladder; 1 to 3: 117 to 133.
- inert at 113: the depth and x expression operand orders (6 cells), alpha
  placed before or after the sync packet or as `(3 - row) * step`.
- the reciprocal written inline at both uses (no inverseDepth local): the
  `mov.s` goes and the reciprocal lands in f16 as shipped, but one more
  callee-saved FP register is spent (frame 0xB8) and the score is 184 to 239.
- point->y read into a local before or after the depth test (the target
  loads it early into f18): 195 at 0 and 226 at +4, FP colours reshuffled.

Aligned after adoption: byte-exact 170, naming 49, immediate 5, really
different 24 (from 146, 48, 0, 69), displacement tax 35. Candidate-only
words at +0x194 (3) and +0x2DC (2), target-only at +0x1AC, +0x1B4 and
+0x1EC (3). The residual is the preheader (the target emits the colour
word before the alpha product, ours the reverse) and the inner loop's FP
colours: the target holds z in f12, x in f2, depth in f14 and the
reciprocal in f16; ours z f14, x f16, depth f12, the reciprocal in f2 then
copied to f18.
<!-- plateau-handoff:overlay100DrawMotion:end -->
