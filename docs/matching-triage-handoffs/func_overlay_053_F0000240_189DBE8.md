<!-- plateau-handoff:func_overlay_053_F0000240_189DBE8:start -->
### `func_overlay_053_F0000240_189DBE8` plateau handoff

- source: `src/overlays/o053/func_overlay_053_F0000240_189DBE8.c`
- score: 403 differing words
- frame: 0xF8
- relocations: 117
- first mismatch: +0x0
- summary: 636 to 403 at delta 0: -Wab,-r4300_mul, single loop var, paired div/mod; frame 0xF8 vs 0xD8 open

Summary before this remeasure: 608/636 instructions, -112 bytes (was +308). Residual: loop store rotation, frame 0x178 vs 0xD8, unrecovered indexed tables.
### Structural pass, 2026-09-11 (lane/p9-struct)

Measured with `tools/align_symbol.py`, which aligns the two streams on a
register-erased shape and splits the residual by cause. As inherited the
function was **+308 bytes**, 713 instructions against 636, with a displacement
tax of 3 -- the two streams were not shifted relative to each other, they were
carrying different content. Three constructs account for most of that, and all
three are scaffold artefacts rather than compiler behaviour:

**1. The scaffold had transcribed IDO's own loop unroll as source.** The
largest insertion run was 36 words at candidate +0x1AC with an 11-word run
immediately before it. Reading them shows a second-level unroll: the target's
counted loop is 4x-unrolled with the usual remainder prologue, the scaffold
wrote those four copies out as four source statements inside a step-4 loop,
and IDO then unrolled *that* by four again -- a 16x body, 195 candidate words
where the target has 79. Replacing the whole construct with the single plain
loop it was compiled from (one accumulate statement, counter to the argument)
took the candidate from 713 instructions to 568 and the byte-exact bucket from
69 to 112. The constant materialisations on both sides of the remainder test
are byte-exact after the change, which is the confirmation.

**2. The draw packet was six dead scalars.** The scaffold named six stack
slots (0x84, 0x88, 0x8C, the halfword pair 0x90/0x92, and 0x94) and passed
only the first one's address to the eight-argument draw call. Five of the six
stores are then dead and uopt removes them -- that is what the missing-word
runs in the two arms of the `if` at target +0x7C0/+0x828 and the three-word
run at +0x898 were. Declaring the six as one 20-byte aggregate and passing its
address restores 29 instructions (568 -> 597) and lifts byte-exact to 117.
This is L107 read from the paying side: the aggregate is free to declare, and
its address-taken-ness is what keeps the stores alive.

**3. Unrecovered data symbols cost a word each.** The overlay reaches these
through runtime LOCAL relocation records, so the extracted object carries no
static symbol and the scaffold rendered each as an absolute address. An
absolute address compiles to one $zero-based access; the target materialises
a %hi/%lo pair, so every such site is one missing word. Giving the six scalar
sites real externs recovers 11 more instructions (597 -> 608) and byte-exact
124. The *indexed* sites were also tried as arrays: they add 7 instructions
but cost 11 byte-exact rows, so they are left as they were and remain open.

Progression, as instructions, size delta, positional masked words, and then
the three aligned buckets exact / naming / different:

- as inherited: 713, +308, 709, buckets 69 / 194 / 512
- plain loop: 568, -272, 627, buckets 112 / 161 / 406
- plus packet aggregate: 597, -156, 626, buckets 117 / 164 / 403
- plus scalar externs (retained): 608, -112, 636, buckets 124 / 164 / 403

**What is left, and what it is not.** The remaining deficit is no longer a
missing construct. Both of the function's loops are now the right shape and
the right length per iteration; they differ in *rotation phase*. The target
sinks the accumulator's store across the back edge -- a peeled preheader
computes the first iteration's value, the loop stores the previous one at its
top, and an epilogue drains the last -- which costs it a 6-word preheader and
a 17-word drain per loop that the candidate does not emit. Writing that
rotation into the source directly was measured and is worse (571 instructions,
104 byte-exact, and the remainder count becomes `(n-1) & 3` instead of the
target's `n & 3`), so it is a uopt transform to provoke, not a spelling to
transcribe. Beyond that the frame is still 0x178 against 0xD8 and the
candidate carries 91 relocations against the target's 59; both follow from the
scaffold's ~40 undifferentiated temporaries and its single stand-in callee
name, and neither is reachable without recovering the call and data identities
the extracted object does not carry.
### Rewrite from the listing, 2026-10-02 (lane/f-mixed)

636 masked at delta -112 became 403 masked at delta 0. Priced edits, each
measured with `tools/fast_score.py` on a copy of the TU:

- `-Wab,-r4300_mul` on the object: the easing loop becomes the shipped
  rotated branch-likely loop from a plain `for`, delta -112 to +20 and 636 to
  630 masked.
- The scaffold's goto and temp web rewritten as four plain loops over
  `D_118`/`D_288`/the spC8 pair, with typed placeholder structs for the object,
  its state and the entry rows: 630 to 586, delta +128.
- One loop variable `i` for the fade loop, the first loop, the third loop and
  the draw loop (a product over the variable choice, 32 cells, floor 403;
  separate variables regressed to 449 to 533): 586 to 403 at delta 0.
- The cleanup loop over `D_288` as an index loop (the pointer loop adds a
  guard the shipped code lacks).
- Each digit pair as quotient and remainder temporaries so one `div` yields
  both halves: 6 divisions with checks became 3 (measured 515 to 455 before
  the single-variable change).

Open: frame 0xF8 against the shipped 0xD8. The shipped frame has about five
declared scalar homes (two above `spC8`, three between `spA4` and the packet);
the candidate declares 12 plus the two quotient temps, and removing
`obj`/`q`/`inner`/`ent` regressed 403 to 430 to 515. The first mismatch is the
prologue's frame size, so every stack offset still differs.
<!-- plateau-handoff:func_overlay_053_F0000240_189DBE8:end -->
