<!-- plateau-handoff:overlay15DrawRain:start -->
### `overlay15DrawRain` plateau handoff

- source: `src/overlays/o015/overlay_015.c`
- score: 13/54 words
- frame: 0x40
- relocations: 17
- first mismatch: +0x74
- summary: Force-split of the offsets ilda matches target schedule except one extra lui; as1 does not share lui at. Next: ugen hi-only ilda, not la or per-use macros.

#### 2026-09-12, lane `p9-tight`: as1 does not share a high half, so the stated decision variable is refuted

The previous note's residual measurement stands: three separate `extern f32`
scalars reproduce the target byte-for-byte except for **one extra `lui $at`**,
six words where the ROM spends five. The `ori at, zero, 0x8000` placement, the
pre-call `lwc1 $f8` / `lw $a3` scheduling and all four `swc1` orderings agree in
that form; the tail is shifted one word, which is the whole of its 20 against
the base's 13.

**What is refuted is the cause.** as1 does not merge `lui $at`. Measured on an
isolated probe: two references to the *same* extern struct, placed in two basic
blocks so uopt cannot pool them, emit two `lui $at` carrying identical
`R_MIPS_HI16` relocations against that one symbol, and as1 leaves both. So the
ROM's shared high half is not an assembler merge, and no C spelling can ask an
assembler for something it does not do. "Stop hunting declarations" was right;
the reason given for it was not.

**What the ROM actually spends** is a third lowering that neither reachable path
produces: one `lui` holding a `%hi`-only base with the `%lo` folded into *two*
displacements (128 and 132), then a second `lui` for the third (136). IDO takes
exactly two paths here, both confirmed on isolated probes:

- a symbol referenced **once** becomes an as1 absolute macro, `lui $at, %hi(S)`
  plus a load carrying `%lo(S)` in its displacement: two words per load;
- a symbol referenced **twice or more** makes uopt build a full base register,
  `lui` plus `addiu` plus one word per load.

The base-register decision is not splittable from source. An `if (1)` region, a
`do { } while (0)` region and a `volatile` cast all leave it in place on the
two-reference probe; only a real branch splits it, and that yields two separate
macros rather than one shared high half.

Eighteen further spellings measured flat beyond the previous note's twelve,
none below the base's 13 at delta 0 and none reaching five words for the three
loads: three externs (20, +4), a two-element extern array plus a scalar (24,
+4), a two-float extern struct plus a scalar (24, +4), a three-element array
(13, 0), the padded struct with z split out (24, +4), four L131 spelling splits
that give the two same-symbol references different written forms (24, +4 each),
a union with the pair reached through two different member paths (24, +4), the
pair reached as `*(&x + 1)` (24, +4), two padded-struct spelling splits (13, 0
each), and an `if (1)` region added to the three-extern form (20, +4).

**The absolute-literal forms are worth recording as a negative with a reason.**
`*(f32 *)0x80` and its array spelling collapse to `lwc1 $f18, 128($zero)` with
no `lui` at all, at delta **-8**. That is not a near miss, it is proof that the
ROM's three addresses are relocatable symbols rather than assemble-time
constants, which retires the whole absolute-literal family.

**Decision variable for the next lane:** uopt's choice between the absolute
macro and the base register, which is the same variable blocking
`overlay15MoveStars` two functions up this file (its note reaches it from the
other side, as a uopt address-constant decision, and names register pressure as
the untested hypothesis). Nothing in declarations, groupings, regions,
qualifiers or argument order moves it. The one axis neither lane has tried is
raising register pressure in the call's own block so that no allocatable base
register is free.

#### 2026-09-13, lane f1: call-bias identity differential

The full configured baseline reproduces 216 bytes, delta zero, 18 raw and
13 masked differences, first masked +0x74. The aligned map has 43 exact and
ten paired structural rows, with one candidate-only word at +0xBC and one
target-only word at +0xB4. Named Ucode authenticates procedure 11 of 12;
the census records 17 draws and 214 emission records. Stock/instrumented
full-TU text is byte-identical.

A single source probe changes only the camera-angle bias literal from signed
to unsigned, testing whether its IR identity alters the call-block draw/release
sequence around the shared address. The camera field is signed halfword and
adding the bias lies in the nonnegative halfword range in both forms, so the
value passed to the existing integer parameter is unchanged. Every line's
draw and emission counts, the draw sequence and the object text are unchanged.
The retained aligned comparison preserves every row and both gap offsets.
Thus this literal-identity lever does not move the affected call line or reach
the shared-base lowering.

The prior declaration, grouping, region and pressure receipts were read.
Their negatives and this measured no-movement result leave no new identified
source route in this packet. Stop early under ADR 0018 rather than repeat those
families. The guarded baseline is restored and no matching bytes are credited.
The unresolved question remains how to obtain the shared high-half load form
without a full materialized base or an additional absolute load expansion.
Sources, both compilers' objects, mapping capture, draw profiles and aligned
differences remain ignored under build/f1/overlay15DrawRain and build/f1/o015-map.

Commands: configured stock/capture compile, allocator_trace_receipt.py
--map-only, draw_census.py profiles/comparison, residual_map.py --object
--against, finalize_plateau.py and tools/gates.sh. The procindex listing was
captured for mapping only; no colour landscape was repeated.

#### 2026-09-13, lane l1: current-schedule revalidation

The stock baseline and traced full-TU text agree: 54 words, delta zero,
frame 0x40, 18 raw and thirteen masked differences, first +0x74. Procedure 11
reproduces seventeen draws and 214 emissions. Alignment is 43 exact, no naming
or immediate rows, ten paired structural rows, candidate-only +0xBC and
target-only +0xB4. The guarded source is unchanged.

Zero new source attempts: the shard already contains the new instrument's
call-bias differential as well as the address-lowering, grouping, region and
qualifier controls. No untested source mechanism was identified that supplies
the shared high-half form. Repeating these recorded controls or the colour
landscape would not add information; stop early under ADR 0018. This is a
baseline revalidation, not a newly closed axis or source-impossibility proof.
Next action remains evidence for shared-high-half lowering without the extra
full base or separate absolute expansion. Fresh source/object and census
receipts are ignored under build/l1/overlay15DrawRain. Commands: configured
compilation, draw_census, residual_map --object, finalize_plateau and
tools/gates.sh. ROM verification covers the fallback; no new byte credit.

#### 2026-09-17, lane w7-o015: pressure forces three absolute macros; as1 does not share lui at

Identity gate: instrumented IDO .text is byte-identical to stock. Procedure 11
of 12, seven p1 decisions. Web 41 is the type-1 ilda of the padded rain-offset
struct, save 2.0, nocs 1, block 4 only, coloured v1. ugen emits la v1 plus
three field loads at 128, 132 and 136; as1 expands la to lui plus addiu. Align
43 exact, 0 naming, 0 immediate, 12 structural, candidate-only +0xBC,
target-only +0xB4, first +0x74. Frame 0x40, 14 slots identical.

The named pressure lever was measured, not inferred.

- Force-split web 41 (accepted forced=-1): 20 masked, delta +4. ugen emits
  three l.s symbol-plus-offset; as1 emits three lui at. The rest of the call
  setup matches the target: 0x8000 before the offset loads, projectionScale
  and visibleCount reloaded early, stores projectionScale then x then y, z in
  the jal delay slot. The whole leftover is one extra lui between x and y.
- Two-field xy plus scalar z, then force-split the remaining 2-use ilda: still
  20 / +4. Two consecutive lui at against the same symbol, then a third for z.
  as1 does not share even two consecutive identical HI16 lui at in one block.
- Consecutive absolute l.s with no .loc between them (named f32 carriers plus
  split, and a .loc-stripped as1 diagnostic) still expand to three macros.
  as1 merge of lui at is closed, including the same-block consecutive case
  the 2026-09-12 two-block probe could not see.
- L109 OR-zero and eight dummy OR-zeros in the call block are byte-identical
  to the base. Without a loop they do not occupy colours (unlike
  overlay31CreatePool, whose probe sits in a loop). OR-zero on width, height
  and visibleCount is 37. Live s32 copies of positions and colors: 18, still
  la. Deleting the camera carrier: 14, same lowering. Deleting visibleCount
  duplicates the conversion, +36. Generated [32],[33],[34] subscripts: 13,
  identical to struct fields. Comma-assigned f32 locals: 13 masked, 41 exact
  plus 2 naming, worse aligned than the base.

So the two IDO paths remain the only ones: la (lui plus addiu, one base, three
loads) or per-use absolute macros (one lui at each). Pressure selects the
second and overshoots by one lui. The target is ugen emitting one hi-only
address covering the 128 and 132 loads, then a second lui at for 136: not la
(which always addius) and not three l.s symbol-plus-offset (which always
become three macros).

Do not retry: L109 without a loop, dummy OR-zeros, deleting camera or
visibleCount, generated subscripts, xy-plus-scalar-z, force-split of the ilda,
or as1 lui-at merging. overlay15MoveStars has the same ilda-versus-absolute
choice; the same ugen addressing-mode question applies.

<!-- plateau-handoff:overlay15DrawRain:end -->
