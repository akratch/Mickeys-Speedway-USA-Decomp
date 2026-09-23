<!-- plateau-handoff:func_overlay_054_F0000000_189ECA0:start -->
### `func_overlay_054_F0000000_189ECA0` plateau handoff

- source: `src/overlays/o054/overlay54Initialize.c`
- score: 17 differing words
- frame: 0x78
- relocations: 114
- first mismatch: +0x1AC
- summary: TU owns overlay 54 data; delta 0, frame exact; left: as1 hoists loop-A flag/value stores above their increments (no noalias for declared pointers).

Summary before this remeasure: Record copy is an unrolled 9-loop, body exact; remainder needs as1 to see static data (probe: 89 masked, -4); loop-A slti is L90.

Summary before this remeasure: One word long. Extra instruction is the unfilled delay of the context-equals-3 branch. L90 rewrites increment-then-i-lt-4 to not-equal-4; the target keeps slti 4 after increment. Delay fill is reachable only on inverted polarity.

#### Re-open under L90 and the one-instruction family (2026-09-19, lane/w22-o054)

Retained baseline remeasured on 5639d0a3: 972 bytes, size delta +4 (244 vs 243), masked 127, raw 144, frame 0x78 with an identical 12-slot ladder. Identity gate: instrumented `cc` `.text` is byte-identical to stock. `CDX_PROC=0` (one procedure, 50 p1 decisions, no p2). `--every-colour` was not run; it refuses until size 0.

The extra word is the nop delay of `bne ctx,3,else`. Target fills that slot with the lo-add of `gOverlay54Current` (D_668), having emitted the hi-half immediately before the branch, and rematerialises the same address in the then-arm after the nested allocate. Else uses the hoisted pointer. Candidate else and then both use at-style stores (`lui at` / `sw 0(at)`), so as1 has no addiu to put in the delay.

L90, measured not asserted:

- `i++ < 3` (retained) keeps slti, but against 3, with increment after the compare.
- `i++; while (i < 4)`, `++i < 4`, and `i++; while (i <= 3)` all rewrite to `li 4; bne s0,at`. Same length, wrong opcode. This is L90's basic-IV normalisation.
- `for (i = 0; i < 4; i++)` also rewrites (no slti), masked 127.
- A derived pointer exit (`src0 != base + 0x80`) scores 125 at the same +4, but replaces slti with a pointer bne. Trap 12: a better number on the wrong test. Not adopted.
- Pointer-difference `< 0x80` costs +4 words and still leaves the delay nop.
- Target has both the zero-move of i and slti-against-4 after increment. That pair is exactly the one L90 says a basic IV cannot emit.

One-instruction family, each eliminated:

- Missing copy of i, of the external word, and of `&gOverlay54Current`: flat at 127, or forwarded (L102). L104 with a redefined bss local still forwards into `sw 0x668(at)`.
- Fallthrough / goto with target polarity (`if != 3 goto else`) is byte-identical to the if/else. Invert (`if != 3` else-first) fills the delay with `lui at` and goes size -4, masked 140, with beq polarity. Wrong arm is the fallthrough; not adopted.
- Rematerialised address: BSS+0x668 stores and a pointer slot both forward (L102). An or-with-zero lock-break produces lui+addiu and a filled delay, but as branch-likely plus a duplicated else store, size +8 to +12.
- L144: dropping volatile and accessing locals through `&locals` is byte-identical to volatile on this function. The sentinel still reloads from `sp+0x44`; the delay stays nop.
- L97 `if (1) {}` before the if, `if (1) { slot = &current; }`, and `do { slot = &current; } while (0)`: 128 or flat, delay still nop.
- L131 / nested-call split into sequential temps: flat 127, confirming the prior closure.
- L111 third node via a hoisted word copy: no delay fill.

Decision variable that blocks: uopt forwards `&gOverlay54Current` into at-style stores, so the compare block never contains the lui+addiu pair whose last pick would fill the delay on bne polarity. Invert proves the delay is fillable, but only by flipping which arm falls through. L90's increment-then-slti-4 shape is still unreachable from a basic IV. Next lever is a source form that keeps a D_668 pointer temp in the compare block without forwarding, without bnel, and without inverting the branch, or a non-basic IV that keeps slti 4 after increment.

#### 2026-09-23, lane B2-ov2: the record copy is an unrolled loop; the rest of the gap is static data

Cycle 0 reader on 373ba8d1: +4, 7 pairs, 106 aligned after 21 shadow, 53
naming. The missing-CSE label was misleading. Most of the naming and the
post-loop pairs came from the hand-scheduled copy locals.

Adopted, masked 127 to 125, aligned after shadow 106 to 72, naming 53 to 20,
size still +4. The copy is `for (j = 0; j < 9; j++)` over a 16-byte record
{pad 8; s32 at 8; s16 at C; s16 at E}, copying fields C, E and 8 from the
source array to the destination array. The target's ring draws give that
order: loop A spends t8 t9 t0, the remainder t1 t2 t3, and the body starts
at t4. IDO unrolls the loop by four with a one-record remainder first, and
the unrolled body and the 64-byte pointers then match the target exactly.
The twelve copy locals are gone, and removing them is inert.

What the remainder still needs, measured: the target loads all three remainder
fields before it stores any, and shares one `lui at` across the three stores.
Ours interleaves them and takes three. That is an as1 alias decision. As a
scratch probe only, declaring the two record arrays `static` in the TU gives
the target's remainder schedule exactly, 89 masked, 57 aligned, size -4
(one word short, all of it in the tail). With extern symbols as1 cannot prove
that source and destination do not alias. Putting both on one extern symbol
lets as1 reorder, but uopt then fuses the two loop pointers into one: -8,
135 masked, and the same with a struct that has both members. The literal
offsets splat leaves in this function's relocations fit the same reading:
the original TU defined overlay 54's data itself, so relocations were
section-relative. Closing this means moving overlay 54's data into C, which
is structural work and not a spelling.

Other spellings measured this cycle:

- Loop A exit. `++i < 4`, `i++; if (i < 4) goto`, and `i++` placed before
  the stores all give `li 4` plus `bne`, the L90 normalisation. An `s16` or
  `s8` counter keeps the slti but pays a sll/sra pair (+12). A `u8` or `u16`
  counter emits no slti at all. L90 stands.
- Tail stores 0x66E and 0x66C. With separate scalar externs, each takes its
  own `lui at` as in the target (aligned 72 to 70). But positional masked
  rises to 134 because the size gap moves, so this is not adopted while the
  delta is +4. With a two-member struct, uopt CSEs the base.
- gOverlay54Current declared `static` (probe): inert. Its address still
  forwards into at-form stores, and the target's hoisted pointer is still
  missing.
- Loads-first block locals in the copy body: they take global colours (L145),
  126 masked.

Next lever: after overlay 54's data is moved into the TU, re-measure. The
static probe says the function is then one word short, and that word is
the `&gOverlay54Current` pointer the target hoists into the branch delay.

#### 2026-09-23, lane B3-o054: the TU owns overlay 54's data; 125 (+4) to 17 (delta 0)

The data move is its own commit. overlay54Initialize.c defines all of
data_rodata +0x0..+0x2D0 as static typed objects in ROM order, declared in
the atlas `DATA_RODATA_OWNERSHIP` (the +0x2D0..+0x2E0 tail is the original
TU's 0x10-byte .rodata, a separate relocation base, and stays raw), and the
whole 0x670-byte .bss as statics. Its compiled .data is byte-identical to
the ROM slice and `gmake verify` passes. With both record arrays local the
copy remainder is the target's (89, -4, as B2-ov2's probe said).

Then, each measured:

- Loop-A pointer setup folded onto one physical line: 89 to 81. The target
  sets the eight pointers in reverse, an L59 tie.
- Tail statement order, an exhaustive 5,040-order sweep of the seven tail
  statements: 72 to 56 at the order state, obj+0x26, obj+0x28, field08,
  field00, field04, field10, bounds.
- `o54Configure(sOverlay54Current, 0)` in place of `o54Configure(object, 0)`:
  56 to 18 and delta 0. Reading the global back keeps its address a register
  web, which uopt hoists into the compare block (the branch-delay addiu) and
  rematerialises in the then-arm. The prior closure named this word; the
  value read back is forwarded, the address web stays.
- Exit test `(++i ^ 0) < 4`: 18 to 17. It keeps slti 4 after the increment,
  which L90 otherwise rewrites to li 4 plus bne (`++i < 4`, `i < 4` after an
  early `i++`, a flag variable, `(s32)(u32)`, or-with-zero, `& -1`, `+ 0` all
  normalise; `(u16)` adds an andi).

What is left, all in loop A's tail (+0x1AC..+0x1EC, 10 aligned rows, 0
naming, frame exact): as1 schedules the flag and value stores right after
the volatile sentinel store, above the pointer increments (offset 0), where
the target keeps them after the increments (offsets -1 and -2) with the
compare and the three constants first. The as1 trace (`-Wa,-R`) shows why:
each loop-A store carries a dependence chain to the next store and to the
sentinel store, so its aftercycles outrank the increments. The target's
stores behave as if they have no successors, i.e. as1 had noalias
information for them. uopt emits `.noalias` only for pointers it created
(indexed arrays); the declared walking pointers get none.

Measured and flat or worse on this axis: 2,520 orders of the tail
statements with the increment first (floor 17); every line fold of the
tail; volatile flag and value pointers; indexed value (27), indexed flag
(67, uopt uses base+i for a byte stride); indexed sentinel without the
volatile struct (+4 size, frame 200); a plain sentinel pointer (uopt
spills the value pointer instead, 26); a plain context local (frame 0x80,
33; unreferenced s32 pads inert).

Next lever: a loop-A spelling whose flag, value and sentinel stores are all
uopt-created pointers (so `.noalias` is emitted) while the sentinel still
takes the 0x44 spill home and the frame stays 0x78. Also: overlays 50 and
52's Initialize functions carry the same record-copy family and still
reference their data as externs; the same data move should apply there.

<!-- plateau-handoff:func_overlay_054_F0000000_189ECA0:end -->
