<!-- plateau-handoff:overlay1UpdateRangeFlags:start -->
### `overlay1UpdateRangeFlags` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 2/120 words
- frame: 0x70
- relocations: 4
- first mismatch: +0x190
- summary: L145-L154 reopen retained 2. 1-draw-right s16 is 14; post-sum u8 probes DCE; s32 corners 20-21; deleting rangeSquared or otherState regresses.

#### tu2-o1tail: the residual is one FP pool web, same law as overlay1AppendPathPoint

Re-measured at the assigned base: 31 masked words, 120 of 120 instructions,
0x70 frame exact, prefix exact to row 13, integer temp ring identical 8 of 8.
The fp-pool lane diverges at slot 2: the target computes the second squared
term into the FP temp ring (f4) while the candidate spends an fp-pool colour
(f18) on it, so the candidate carries one FP pool web the target does not.
That is the same named-local-versus-CSE-temp law that took
overlay1AppendPathPoint from 21 words to 6, applied to the squared-distance
comparison rather than to an integer carrier. The lower block at +0xD4 shows
the mirror image: the target holds the other-kind byte in a pool colour (v1)
where the candidate uses a temp (t1).

#### 2026-09-09, lane fin-near: the u16 truncation temp is the second ring draw

In the ugen listing case 1's `u16 masked` costs two ring temps (the mask and
its 16-bit truncation), drawn as t4 then t3, and the clear draws t5; the
final code folds the pair into one `andi t3` and leaves `and t5`. Removing
the truncation (`if (flags & 8)`, or `u32`/`s32 masked`) draws t4 for the
test and t3 for the clear -- the inversion the earlier record describes --
at 4 words. The target's t3-then-t4 needs the free list ordered t3 above t4
at the arm's entry, which the angle block's frees decide; that sweep is
recorded above as exhausted. Baseline 2 retained.

#### 2026-09-09, lane win-b: the ring is a FIFO queue and the queue is the whole residual

Re-measured with a direct `tools/ido/cc` full-TU compile (byte-identical in
`.text` to the asm-processor NON_MATCHING object at this TU's real flags --
`overlay_001_tail.c` **does** carry `-Wab,-r4300_mul`; omitting it silently
costs one instruction and 80 words). 118 of 120, frame 0x70 exact, and the two
words are +0x190 `and $t?, $v0, $s7` and +0x198 `sh $t?, 0x1A8($s1)`: the
target draws `$t4`, we draw `$t5`. Every other row, both switch arms' tests,
and the whole angle block are exact.

**The mechanism, read straight out of `cc -S`.** ugen's integer temporaries are
a FIFO queue over `$8..$15,$24,$25`, ordered by *free* time, and both switch
arms draw from that one queue *in sequence*: `case 0` takes the first two, then
`case 1` continues from the third. The queue reaching the switch is set by the
angle block, whose ugen form is fixed:

    sll  $9, $3, 8     sll $10, $9, 16    sra $11, $10, 16
    sll  $12, $2, 16   sra $13, $12, 16   addu $4, $11, $13
    sll  $14, $4, 16   ... sra $15, $4, 16

`$9` frees at the second `sll`, `$10` at the `sra`, `$12` at the second `sra`,
and `$11`/`$13` together at the `addu`. So the queue is `$9,$10,$12,$11,$13` --
`case 0` gets `$9,$10` (exact, both arms' `andi`/`ori` match) and `case 1` gets
`$12,$11,$13`. That predicts, and measurement confirms:

- a two-temp `case 1` (`if (flags & 8)`) draws `$12` then `$11`, i.e. the test
  and store inverted: **4 words**;
- a three-temp `case 1` (the `u16 masked` carrier, whose truncation `and
  $x,$y,65535` as1 folds into the `andi`) draws `$12` (mask, folded away),
  `$11` (visible `andi`, correct) and `$13` (store, one too high): **2 words**.

The target needs the queue `$9,$10,$11,$12`, i.e. `$11` freed before `$12`.
`$11` is only freed by the `addu`, and `$12` is only freed by the second
operand's sign-extension `sra`, which must precede the `addu`. Making the
second operand cost no temp does free `$11` first -- and then the outer
truncation draws `$12,$13` instead of the target's `$14,$15`, moving the same
two words to +0xE0/+0xE4. The target's `sll $t6`/`sra $t7` pin five ring draws
ahead of them, the `sll $t3` at +0xD8 pins the left chain's terminal at `$11`,
and the `addu $a0` pins the sum on the pool colour; those three constraints
plus the free order are jointly unsatisfiable. **This is a structural
conflict, not a search gap.**

Exhausted here, all against the full-TU object: 960 points crossing 24 angle-sum
spellings (six `angleHigh` shift forms x plain/outer-cast/cast-read/`+=`) with
four physical line groupings of the three angle statements, five `case 1`
carrier forms and two `angleHigh` types -- floor exactly 2, reached at 576 of
them; plus 20 further `case 1` shapes (`!=0`, `==8`, `!!`, `>0`, negated with
`break`, negated with an empty `then`, compound assignment, field re-read,
nested load, a pre-computed `cleared` carrier in five types, and `u8`/`u16`/
`u32`/`s32`/`s16` masks). A pre-computed store carrier does not survive: uopt
sinks the partially dead expression back into the `if`, so the store is always
emitted after the test whatever the source order.

Next lever is a ugen free-list trace (`DKWB_UGEN_TRACE`), not another source
form. Do not re-search `case 1`, the angle spellings, or line grouping.

#### 2026-09-10, lane p1-perm: a third 2-word corner, and the count argument closed

A third arrangement exists and is strictly more informative than either
recorded one. With the two-temp `case 1` (drop the `masked` carrier) and

    angle = ((s16)((u32)angleHigh << 8) & 0xFFFF) + (u16)angle;

every word is exact except +0xD8/+0xDC: both switch arms are right, the `addu`
writes the pool colour `a0`, the sign extension is on t6/t7, and the only
residual is that the folded left chain survives on t4 where the target has t3.

The object pins four things at once -- the folded left chain must survive on
$11, the `addu` must write the pool, the sign-extension pair must be $14/$15,
and the free list reaching the switch must be ascending. as1 folds a chain onto
its LAST destination, so a survivor of $11 forces a three-temp left chain;
$14/$15 forces exactly five temps drawn before the truncation; and an `addu`
that writes the pool forces the sum to be the statement's top-level operation,
so no temp can be drawn after it. That leaves the second operand owing two
temps, and any two-instruction conversion frees its first at the second's
definition -- before the `addu` frees $11 -- which is the original inversion.

The three reachable corners are therefore exactly 3+2 (inversion,
+0x190/+0x198), 4+1 (survivor t4, +0xD8/+0xDC) and 3+1+outer (`addu` on a ring
temp, +0xDC/+0xE0). All three are two words and no fourth corner exists in C.

Newly measured and flat, ~200 candidates/second against the full-TU object:
360 cells of case-1 form x carrier type x nine left-operand spellings x five
right-operand conversions x two assignment casts; 260 cells of thirteen outer
operations x five right conversions x two types x two assignments; 24 double-
and triple-conversion spellings crossed with an `s32` carrier and an
`(s16)`-cast comparison. Every cell is 2, 4, 18, 19, 21 or worse.

The retained body is the original 3+2 corner: it is the most plausible C of the
three and no corner is numerically better.

Next lever: not C. Either uopt/ugen instrumentation that shows why the target's
free list is ascending with a three-temp chain, or a construct that draws a ring
temp after a pool-writing `addu`, which this grammar does not produce.


#### c2-o001: the ring-queue conflict survives the inert-probe family

The win-b record closes this at two words on a jointly unsatisfiable set of
free-order constraints, and calls it a structural conflict rather than a
search gap. That ruling was tested from a direction the recorded sweeps did
not cover and it holds.

242 zero-footprint reads were measured under ADR 0017, spanning eleven
placement slots from the top of the loop body through both switch arms, and 22
expressions including the two phantom-pop forms the field guide names for
buying a ring pop. None of them reaches below two words. Six placements are
byte-flat -- an already-pooled pointer read costs nothing anywhere it is put --
and every other combination regresses, most of them to seven or more.

So the ring queue reaching the switch cannot be re-ordered by adding a draw
either, which is the last cheap thing left to try on it. Two words stand.
#### 2026-09-10, lane o7-tight: the uopt-region lever does not open the ring either

The win-b record's free-order argument was re-derived from the object and it
holds. Restated so the next lane does not have to rebuild it: the angle sum's
left operand owns three ring temps and its last one is freed only by the
`addu`, while the right operand's sign extension frees its first temp at its
own second instruction, which is necessarily before the `addu`. So the right
operand's temp is always freed ahead of the left operand's survivor, and the
free list reaching the switch is always descending across that pair. The
target needs it ascending. Making the right operand cost no temp does free the
survivor first and then moves the outer truncation's pair down by two, which is
the same two words at a different offset.

What this pass adds is the one axis the recorded sweeps did not vary: **block
membership**. ADR 0017's inert reads, the 960-point angle lattice, the 360-cell
operand lattice and the 96 line groupings all vary the *spelling* of statements
inside one region. L97 says `if (1) { }` and `do { } while (0)` open a uopt
region where a bare brace does not, so they are the one construct that can move
a statement into a different region without changing a token of it. That is the
lever that closed `overlay1ResolvePathPoint` in this lane (there in its
basic-block form rather than its region form), so it was worth testing here.

119 cells, each compiled with this TU's real flags -- `-Wab,-r4300_mul`
included, without which the object silently loses an instruction -- and
compared against the whole 120-instruction target text:

- nine angle-block forms: the recorded spelling, the summands exchanged,
  `+=`, `(s16)(angleHigh << 8)` without the `u32` cast, a `* 0x100` multiply
  in place of the shift, and each of an `if (1)` region and a `do/while (0)`
  region wrapped around the sum, around the exchanged sum, and around the
  `angleHigh` carrier assignment;
- crossed with two placements of the `angleHigh` read (plain, and inside its
  own region);
- crossed with seven `case 1` shapes: the recorded `u16 masked`, `u32` and
  `s32` carriers, no carrier at all, `!= 0` on the carrier, an explicit `(u16)`
  cast on the mask, and a region opened at the head of the `if` body.

Floor is 2, reached by nine cells; the region wrappers are all at 2 or worse,
and the carrier-type changes reproduce the recorded 4-word inversion. No cell
moves the pair. **A uopt region boundary does not re-order the ugen ring**,
which is a useful negative in its own right: the ring free list is a
per-procedure structure and survives the region openings that move uopt's
colouring webs around.

The verdict stands: three reachable corners, all two words, on a jointly
unsatisfiable set of free-order constraints. The next lever is still the ugen
free-list trace (`DKWB_UGEN_TRACE` on the instrumented ugen at
`~/Desktop/dev/ido-instrumented`, which this lane confirmed is built and
fidelity-clean), used to ask whether any construct can free a temp between the
left chain's survivor and the right chain's first -- not another C lattice.

#### 2026-09-10, lane `w8-tu`: the free-order conflict survives the neighbouring expressions

The recorded conflict is stated over the angle block alone, and the ring queue
is a function-wide FIFO, so the two integer expressions that sit *between* the
angle block and the switch are a separate axis: the vertical-scale conversion
in the height test and the squared-range product. Fifty-four cells crossing six
spellings of the vertical scale, three of the range product and three case-1
carrier shapes were compiled and scored against the whole 120-instruction
target. Every cell that keeps the instruction count is exactly 2; the two
spellings that change it are 54 and 90 words at +16 bytes. So the queue
reaching the switch is not reachable from those expressions either, and the
structural conflict the win-b pass recorded now covers the whole loop body
rather than the angle block alone.

#### 2026-09-11, lane w3-low: the ugen free-list trace was run, and it closes this

Five records named a ugen free-list trace as the next lever and none of them had
run it. It was run here. The instrumented ugen was confirmed fidelity-clean on
this translation unit first, at this TU's real flags: the object it produces with
the trace off and the object it produces with the trace on are both byte-identical
to the tree's own. This procedure is ordinal 24 in the unit and emits 20 ring
allocations and 89 releases.

The trace turns the recorded free-order argument from an inference into a
mechanism, and it confirms it exactly as written. The ring is a ten-register
queue. Every allocation takes the head, every release appends to the tail, so the
draw order is a pure rotation of the queue unless some temporary outlives one
allocated after it. In this procedure exactly one does: the angle sum's left
operand owns a temporary released only when the sum is formed, while the right
operand's conversion releases its own first temporary one emission step earlier.
The trace records those two releases in that order, at consecutive emission
indices, in the two rows the earlier passes predicted.

What the trace adds is that the resulting swap is permanent rather than local.
Because a release appends to the tail, a pair released out of order stays out of
order in the queue for the whole rest of the procedure. The queue reaching the
switch therefore always carries the two contested slots transposed, at the same
two queue positions, however many further allocations happen in between. The
first arm reads the two positions ahead of them and is byte-exact, which pins the
phase, and the second arm is then forced onto the transposed pair. This is why
every carrier spelling ever tried on the second arm only moves the same two words
around.

That converts the plateau into a counting statement that can be checked rather
than argued. Let k be the number of extra ring allocations added before the
surviving temporary and m the number added after the sum. The first arm keeps the
target's two slots only when k plus m is a multiple of ten, and the swap misses
the second arm only when m is neither zero nor nine modulo ten. Together those
force k to be at least two, and any k at all shifts every ring slot in the angle
block and in the height test, all of which are byte-exact today. Measured rather
than predicted: k of two with m of eight does give both arms the target's exact
slots at an unchanged instruction count, and scores 16.

L127 is live on this translation unit, and that was established before it was
relied on. A redundant byte mask on a value already known to be a byte draws a
ring temporary and is then removed at the peephole, leaving the instruction count
at 120 and the size delta at zero. Chained masks stack, one draw per layer, so an
arbitrary number of zero-footprint draws is available at the switch selector, at
the vertical-scale read and at the angle-base read. This is a different family
from ADR 0017's inert reads, which never reach ugen at all; several of those
forms were re-measured here and confirmed to draw nothing, which is why their
failure never bore on this.

Swept and flat, every cell compiled at this TU's real flags and scored against the
whole 120-instruction target text: 264 cells of a single zero-footprint operation,
in twelve spellings, at eleven placements spanning the loop head, the squared
range, the angle base, three positions inside the angle sum, both halves of the
angle comparison, the vertical scale and the switch selector, crossed with both
second-arm shapes; and 220 cells of the phase lattice, k from zero to nine crossed
with m from zero to ten crossed with both second-arm shapes. Floor is exactly 2
and nothing reaches below the retained body. The best cell in the phase lattice is
4, which is the recorded inversion.

The two contested values are ugen ring temporaries, not coloured webs, and this
procedure's own records say so rather than a general law. Every colouring decision
here is a phase-one decision, which is the call test agreeing with itself; there
are 22 of them out of 169 candidate webs, none declined, and every one lands on a
pool register or the float pool. Neither contested value appears among them. So
save ratios, candidate ordering and colour cost have no purchase on this residual
at all, and L127 reaches the right family but cannot pay the arithmetic above.

What is left is not a C lever on the switch. The transposition is forced by the
dataflow of an angle block that is byte-exact with the target, so the target's own
source must carry something that changes liveness without changing a byte. The one
shape that would do it, keeping the right operand's first temporary alive past the
sum, was traced: it produces a different transposition rather than none, and lands
the second arm's pair one slot further on. The next lever is therefore the angle
block itself, not the switch: either a different type for the angle carrier that
produces the same words from a different expression tree, or evidence that the
target's height test allocates a different number of ring temporaries than ours,
which would move the phase without touching the angle block. Do not re-run the
zero-footprint family or the phase lattice.


#### 2026-09-11, lane p6-tight: the allocation ORDER rule, read from ugen, and block shape retired

Re-measured unchanged: 480 bytes, 120 of 120 words, size delta 0, positional
masked 2, aligner buckets 118 byte-exact, 2 register naming, 0 immediate only,
0 really different, first naming-only difference +0x190. The census reads one
source register mapping to one target register at both sites, 100 percent
coherent, one window, no cycle.

The ugen listing settles what the earlier passes inferred. The two arms of the
mode switch are structurally identical, and their temps come out of the same
free list, but they take them in OPPOSITE orders: the first arm allocates its
test before its store, the second arm allocates its store before its test. The
free list itself is ascending here, so the earlier reading that the angle
block hands the switch an out-of-order queue no longer applies to this base --
what is left is purely an allocation-order fact inside the second arm.

The recorded carrier fixes the order by spending a temp rather than by
reordering: a sixteen-bit carrier gives the test's truncation the first number
and its inner AND the second, as1 folds the pair into one instruction keeping
the first number, and the store therefore slides to the third. The target
spends two: test first, store second. So the requirement is exact -- the test
must be allocated first at a cost of ONE temp, and every construct that
allocates it first costs two.

Newly measured and flat this pass, all at zero size delta:

- eight bit-width spellings of the carrier. Unsigned sixteen-bit and unsigned
  eight-bit both give 2; `s8`, `s16` and `short` cost eight bytes and 31 words
  because the truncation sign-extends; every thirty-two-bit spelling
  (`u32`, `s32`, `int`, `unsigned int`, `long`) gives 4 and is byte-identical
  to having no carrier at all, which is the measured proof that the lever is
  the truncation node and not the declaration;
- thirteen block shapes of the arm: an early `break` on the negated test, with
  and without braces and with the comparison spelled three ways; an empty then
  with the body in the `else`; a `goto` to the arm's end; a `do`-`while(0)`
  wrapper; a `while` that breaks after one pass; an `if (1)` region around the
  store; a bare brace pair around the store; and a nested region around the
  whole body. The four early-exit shapes are all 4, the same as no carrier, so
  moving the store into a later basic block does NOT make the test allocate
  first. The region and goto shapes are 9 to 11;
- a carrier read from the pointer rather than from the already-loaded value: 6;
- the store re-reading the field: 6;
- a carrier on the store instead of the test, and carriers on both: 4 and 2,
  neither better than the recorded one;
- identity-op phantoms on the angle sum, both operands and the whole sum, in
  all three forms L109 names, crossed with the carrier and no-carrier arms:
  byte-identical in every cell, so those phantoms are folded before ugen and
  cannot move this free list either;
- declaring the angle as a thirty-two-bit local and truncating the sum
  explicitly: 18, because the truncation pair then lands two ring slots early
  and shifts the rest of the arm. The one variant of that family that keeps the
  layout, a doubled mask on the right operand, is byte-identical to the base at
  2.

Next lever, unchanged in kind but now stated against the right mechanism: a
construct that makes the second arm allocate its test temp before its store
temp while emitting one node, not two. Do not re-search carrier types or block
shapes.


#### 2026-09-12, lane p10-tight: the p6-tight allocation-order reading is refuted, and a new two-word corner

Re-measured first: 480 bytes, 120 of 120 words, size delta 0, positional masked
2, aligner buckets 118 byte-exact, 2 register naming, 0 immediate only, 0 really
different, first naming-only difference +0x190.

**The p6-tight entry above is wrong on its load-bearing detail, and the
correction reopens the function.** It says the free list reaching the switch is
ascending on this base and that the residual is an allocation-ORDER fact inside
the second arm. Read off the compiler's own listing rather than off the emitted
registers, ugen draws a ring register immediately before each instruction it
emits, so the draw order IS the listing order, and the free list reaching the
switch carries the fourth and third ring members transposed exactly as the
win-b entry first described. The second arm with the recorded carrier makes
three draws (the inner mask, its truncation, the store) and the assembler folds
the first two, which is why the test lands correctly and the store lands one
slot late. The no-carrier arm makes two draws and lands both wrong. Neither
reading of the arm is the residual: the transposition is upstream.

**The requirement, restated against the angle block.** The angle sum makes five
ring draws, its left operand owning three and the right operand's widening two.
Releases go to the queue tail as each value dies, so the left operand's survivor
is released at the sum while the right operand's first temporary is released one
emission step earlier, and the pair reaches the switch out of order. The target
needs the five releases in draw order.

**A new two-word corner, reached and receipted.** Declaring the angle as a
thirty-two-bit local, spelling the right summand as an explicit sixteen-bit mask
of it so the widening costs one draw instead of two, and writing the second
switch arm with NO carrier at all makes the ENTIRE switch byte-exact, both arms,
and moves the residual to +0xdc and +0xe0: the sum's destination and the source
of the first truncation instruction. The target puts the sum in the angle's own
pool colour; this candidate puts it in a ring temporary. Every cell is zero size
delta. Reached three ways, all scoring 2 with the switch exact: the masked right
summand carried in the same statement, the same split across two statements, and
the truncation moved into the comparison.

**Why the fifth draw cannot be paid.** With the right summand costing one draw
the block makes four, and the outer truncation then lands two ring slots early,
which is the recorded 18-word regression. A fifth draw is therefore needed
strictly between the sum and the truncation. The assembler deletes a no-op
instruction by RENAMING ITS PRODUCER'S DESTINATION to the no-op's own
destination -- measured on three different phantom families -- so any
zero-footprint operation placed on the sum renames the sum off the pool colour
and re-opens those same two words. A phantom appended to the left operand's
chain is folded into it and moves the survivor one slot on, for the same reason.
The only payment left is a zero-footprint ring draw on some OTHER live value
between the sum and the comparison, and every value live at that point is a
pointer or a thirty-two-bit count whose bits are all needed, so the one
documented zero-footprint family (a redundant mask on a value already known to
be narrow) has nothing to attach to.

Newly measured and flat at two or worse this pass, all zero size delta unless
noted: 29 second-arm respellings including the carrier reused as the store
operand in four widths (twelve bytes short), the store carrier reused as the
test, inline truncations in two widths, three flag widths crossed with three
carrier shapes, an assignment inside the condition, a dead first definition, and
both carriers together; ten arm block shapes including three early-exit forms,
a goto, a do-while wrapper and a comma condition; 26 angle-block node-count
variants crossing the angle's declared width with hoisted and cast left
operands; 34 phantom-draw placements across the sum, the left operand, the right
operand, a split assignment and the comparison.

**A second corner confirms the release-order reading independently.** Adding a
redundant byte mask to the angle-high read inside the left operand -- the value
is loaded as a byte, so the mask is a no-op the assembler deletes -- gives the
block its fifth ring draw and the switch comes out byte-exact, both arms, with
everything from the truncation onwards exact as well. It scores 3, not 0, and
the three words are exactly the ones the mechanism predicts: the assembler
deletes the no-op by renaming the byte load's destination, so the load loses the
pool colour it should keep, and because the draw was spent at the HEAD of the
left operand's chain the chain's survivor moves one ring slot on, taking the
shift and the sum's first operand with it.

That pins the remaining constraint to one sentence. The fifth draw has to be the
fifth, not the first: the left operand must own draws one to three so its
survivor is the third, the right summand must own the fourth so it dies at the
sum after the survivor, and the fifth must fall between the sum and the
truncation. Every no-op placed there is on the sum, and deleting it renames the
sum. Everything else in the function is byte-exact in that configuration.

Next lever: not another spelling of the arm, and not the carrier. Either a live
narrow value that can carry a zero-footprint draw between the sum and the
comparison, or a source form in which the sum's own destination survives a
no-op placed after it.

#### 2026-09-12, lane p23-lastmile2: exhaustive colour landscape and literal identity

Fresh configured baseline: 480 bytes, zero size delta, aligned buckets 118
byte-exact, 2 naming, 0 immediate, 0 structural; no unmatched offsets on either
side. First difference remains +0x190. The named Ucode map authenticates
procedure 23 of 38; stock and instrumented full-TU text compare byte-identically.

The exhaustive same-kind landscape sampled all 22 coloured webs with 155
single forces: 147 retained the size and 8 grew by eight bytes. Every force
was accepted. The complete improving winners list is empty. Consequently the
winner-set packing is empty and its measured floor is the unforced 2; this is
not an exhaustive claim about combinations of individually regressing forces.
The nomination table shows that some forces remove the late pair while adding
more differences in earlier windows. No source colour question was solved.

A 12-cell source check crossed signed/unsigned shift literals, signed/unsigned
complement masks and an explicit signed-sixteen-bit angle read. Eight cells
retain 480 bytes and the same 2 differences. Four cells using the equivalent
narrow clear mask lose twelve bytes and score 111; that changes instruction
selection rather than the required reuse order. The retained source is unchanged.

ADR 0018 stop: the exhaustive colour pass and literal-identity check supply no
route below the retained residual; the prior angle-chain and carrier receipts
already exclude the available source family. The named source question remains
how to preserve the angle sum's pool destination while placing one additional
temporary draw after that sum without changing emitted instructions. No match
or new byte credit is claimed.

Evidence is retained under ignored build/p23/range and
build/p23/overlay1UpdateRangeFlags. Commands: residual_map.py,
allocator_trace_receipt.py --map-only, web_footprint.py --every-colour,
the configured full-TU source lattice, and finalize_plateau.py.

#### 2026-09-17, lane w2-o001: L145-L154 reopen does not pay the fifth draw

Re-measured on the assigned base: 480 bytes, 120 of 120 words, size delta 0,
positional masked 2, aligner buckets 118 byte-exact, 2 register naming,
0 immediate, 0 structural. Census: t5 to t4 at both sites, 100 percent
coherent, one window, no cycle. First naming-only difference +0x190.
Proc 23 draw census: 33 draws, 174 emissions. Case 1 still draws 12, 11, 13
(t4, t3, t5) with the u16 carrier; the target needs t3 then t4.

The reopen asked for L145 (delete the carrier for a ring temp), L149 (count
draws), L146 (re-climb after a shape change) and L154 (type then first use).
The retained body is unchanged. Attempts, all at this TU's real flags
including -Wab,-r4300_mul:

- s16 angle with the right summand spelled (u16)angle: 14 naming, first
  difference +0xE0. Line 2300 drops 7 draws to 6. Switch draw order becomes
  t0, t1, t2, t3, t4 -- ascending, one slot early. This is the missing-fifth-
  draw shape, now measured on s16 rather than only on the recorded s32 corner.
- Post-sum `angleHigh = (u8)(angleHigh & 0xFF)` and an OR-with-zero
  assignment to angleHigh on that shape: draw order identical to the 14-word
  cell. uopt deletes the assignment or emits a pool-register no-op as1 then
  folds. L109/L127 on the u8 local after the sum do not reach ugen as a ring
  draw.
- `(s16)(angle & 0xFFFF)` on both comparisons, and a named `u16 bits = angle`
  carrier for the same truncation: 19 naming, 34-35 draws. The extra mask
  costs two draws, not one, so the tail overshoots.
- Reuse angleHigh for `config->mode & 0xFF` and switch on that local: 41,
  size delta 0 but structural at +0xEC / missing +0x14C. Hoisting the mode
  load is not a zero-footprint draw.
- s32 angle, left as `(s16)((u32)angleHigh << 8)` or with an extra
  `& 0xFFFF`, right as `(u16)angle`, with and without the case-1 u16
  carrier: 20 or 21, 4 structural rows, first structural +0xD8. The recorded
  two-word switch-exact s32 corner did not reproduce from those spellings.
- Delete the named `rangeSquared` f32 (L145): 14 naming from +0xA0. The
  named product remains load-bearing for the FP pool.
- Delete `otherState` and spell `other->state` at the flag sites (L145):
  109 masked, size delta -4. The pointer carrier is load-bearing.

ADR 0018: more than three consecutive attempts with no better residual.
New identities: the s16 one-draw-right shape is 14 with an ascending switch
queue one slot early; dead assignments on angleHigh after the sum do not
draw; L127 attached to the comparison or a named u16 truncation overshoots
by one draw; the named rangeSquared and otherState carriers are not the
fifth-draw budget.

The decision variable is unchanged: one folded ring draw strictly after the
pooled addu and before the s16 truncation, on the one-draw-right shape, that
does not rename the sum and does not move an existing load. This grammar
still does not produce it. Do not re-search case-1 carriers, angleHigh
identity ops, rangeSquared deletion, otherState deletion, or the s32
spellings above.

<!-- plateau-handoff:overlay1UpdateRangeFlags:end -->
