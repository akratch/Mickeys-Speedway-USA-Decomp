<!-- plateau-handoff:levelFreeAll:start -->
### `levelFreeAll` plateau handoff

- source: `src/main/level.c`
- score: 117/117 words
- frame: 0x28
- relocations: 36
- first mismatch: none
- summary: MATCHED. Mutate the s32 entry carrier with the 14-bit mask, then add the table pointer first.

#### 2026-09-17, lane w3-level: matched

The remaining three-word ring swap was a surviving mask evaluation in front of
a base-first address, which no fresh local and no spelling of the one
expression could produce. Assigning `temp_v0_2 &= 0x3FFF` on the existing
entry carrier (widened from s16 to s32 so the assignment is not a narrowing)
then writing `(u32) D_800C94E0 + (temp_v0_2 << 1)` emits mask, table, scale.
uopt does not forward-substitute a self-assignment of the loaded entry the way
it does a new mask local. A 16-bit carrier adds a sign-extend chain; a
subscript after the assignment uses a multiply for the scale. Configured
score 0 of 117, delta 0, frame 0x28, 36 relocations. The assembly fallback is
removed.

- 2026-09-09 pass, the residual reduced to one ring ordering. The three words
  are one three-temp allocation in the world-index arm of the model-release
  loop: a mask of the entry word, the table pointer's own load, and the scaled
  index. The target's ring order is mask, table, scale; the candidate's is
  mask, scale, table. Nothing else in the function differs and the frame,
  instruction count and relocations stay exact.
- The rule, measured on this site: the ring order is exactly the order of the
  surviving definitions inside the *expression*, and every spelling therefore
  lands in one of two classes. Writing the scaled index first gives mask,
  scale, table and 3 words; writing the table first gives table, mask, scale
  and 5. The target's order is neither, so no single-expression spelling can
  reach it.
- Eliminated this pass, each measured: reversed operand order on both the mask
  and the sum; multiply-by-two against a shift; byte-pointer, array-subscript,
  address-of-element and pointer-plus-index addressing; unsigned and 16-bit
  casts of the masked value; parenthesisation variants; splitting the mask into
  its own statement inside the arm; hoisting that statement to the top of the
  loop body, before the whole if-chain; a nested block scope for it; caching
  the table pointer in a local (8 words, the documented penalty for caching a
  twice-spelled global); and a comma expression that mentions the mask before
  the table, which does *not* move the order because the dead first mention is
  eliminated before web numbering.
- That last one refines the carrier-colour law: mentioning a value earlier only
  earns it an earlier colour when the earlier mention survives. A dead mention
  buys nothing.
- Next lever: the target's order implies the table pointer's address web is
  created between the mask and the scale, which no arrangement of this one
  expression produces. Look for a second surviving reference to the table
  elsewhere in the function, or for the arm's source being a call whose
  argument list, not an address expression, orders the three.

#### 2026-09-09, lane fin-near: subscript order is normalised by cfe

The reversed subscript `(index)[table]`, which in principle walks the index
before the table, and every other subscript or pointer-sum spelling of the
world-index arm (`table[index]`, `*(index + table)`, `*(table + index)`, with
and without the `s16 *` cast) all give table, mask, scale at 5 words. cfe
normalises the operand order of a subscript before uopt numbers the ring, so
the mask-table-scale order cannot come from a subscript. Baseline 3 retained.

#### 2026-09-09, lane fin-misc: the residual is a two-slot ring swap, and the
target carries the subscript signature

Read out exactly against the real object with a 64 ms direct-`tools/ido/cc`
loop (byte-identical in `.text` to the asm-processor `NON_MATCHING` object at
the flags recovered from `gmake -n build/src/main/level.c.o`), which made this
pass exhaustive rather than sampled -- about 800 scored candidates.

The three words are a swap of two adjacent ring slots and nothing else. All
three classes put the same instructions at the same positions; only the
register names move, so this is post-uopt emit order, not schedule:

- target: mask t3, table t4, scale t5, sum t6
- manual shift, `(m << 1) + (u32) table`: mask t3, table t5, scale t4, sum t6
- subscript, `table[m]`: mask t4, table t3, scale t5, sum t6

The scale is created *third* in both the target and the subscript class and
*second* in the manual class, so the target carries the subscript lowering's
signature with the index temp created before the base load. cfe evaluates a
subscript's base before its index, and canonicalises `int + ptr` to `ptr + int`
before web numbering -- which is why integer-left pointer arithmetic
(`*(m + table)`) and the reversed subscript `(m)[table]` land in the base-first
class too, and why no spelling of this one expression reaches the target.

Newly eliminated this pass, each measured:

- The local prototype of `func_80004B04` (`s16`, `s32`, `u16`, no prototype).
  Flat; `u16` costs one extra word at the call. The TU-local prototype is
  `s16` while `objects.c` defines it `s32`, so this was a real axis, not a
  formality.
- **192 physical line splits** of the arm across every token boundary in four
  address spellings -- completely flat. The line-key lever (L87) that has
  closed other functions does not reach this site.
- A **surviving** comma first operand -- `(masked = temp_v0_2 & 0x3FFF, ...)`
  inside and outside the subscript, and an embedded assignment in the index.
  All fold back. This is the lever that closed `func_8003A2C8` the same day,
  so its failure here is informative: the earlier note that only a *dead*
  first mention fails is too weak -- a surviving one fails too.
- Use-side lock breaks `^ 0`, `- 0`, `+ 0`, `* 1` and a zero bitwise-or on the index.
- Two-statement pointer-local forms (`entry = &D_800C94E0[m]`, `entry =
  D_800C94E0` then `entry[m]`): 8 words.
- `sizeof(s16)` scaling, `(u8 *)`/`(s32)`/`(u32)` base casts,
  `*(s16 **) &D_800C94E0`, `((s16 (*)[1]) D_800C94E0)[m][0]`, a bitwise-or in place of the sum
  (4 words, and the `or` is wrong), and `(u16)` on the subject.
- Restructuring the third `else if` into a nested `else` with the mask hoisted
  above the inner `if`, and hoisting it to the top of the loop body, crossed
  with all four address spellings.

Tooling: **the permuter is not usable on this function.** Its scratch reports
`base score = 20` against the real object's 3 -- a 6.7x disagreement -- and it
exits without producing a candidate. Recorded under the "permuter scores do not
transfer" blind spot.

Donor: JFG's `levelFreeAll` has no model-release loop, and none of the five
permitted decomps contains the `0x3FFF` resource-tag idiom. The matched sibling
`levelInit` in this same TU spells it `D_800C94E0[resourceId & 0x3FFF]`, which
compiles into the base-first class here -- so the natural spelling is not the
answer at this site.

Next lever: the only remaining route to {mask, table, scale} is a subscript
whose index temp is created before the base load, and cfe canonicalises every
source form that would order it that way. So the reason must come from outside
the expression -- a second, partially dead reference to `D_800C94E0` that uopt
sinks into this arm (which would create its web late), or evidence that the
base is not this global at all.

#### 2026-09-10, lane p1-perm: the free-list hypothesis is closed

The only two ways a ring residual can arise are a different within-expression
emission order and a different free list reaching the site. The second is now
ruled out. Reading the register sequence of the loop body on both sides: the
head and the if-chain draw t7, t8, t9, t0, t1 and t2 in that order before the
arm, and the arm then draws four **fresh** registers t3, t4, t5, t6. Nothing is
recycled, so the list reaching the arm is plain cycle order on both sides and
the three words are entirely ugen's order inside the one expression. No
perturbation of earlier frees can reach the target, which removes the whole
class of "spend a temp earlier" experiments.

That leaves the recorded impossibility: the target's creation order is mask,
table, scale; cfe evaluates a `+` strictly left to right and canonicalises
`int + ptr` to `ptr + int`, so a two-operand address expression gives either
mask, scale, table (index-first) or table, mask, scale (base-first) and never
mask, table, scale.

Also newly measured and flat at three words: **256 line-grouping cells** --
every subset of the four arms with its body joined onto its own `if`/`else`
line, crossed with joining `temp_v0_2 = D_8007A0F4[i];` onto the first `if`
line, joining `D_800CF490[i] = NULL;` onto the closing brace line, and four
address spellings. This is a different space from the recorded 192 line
*splits* of the arm, and it is equally inert; the L87 line-key lever does not
reach this site from either direction.

#### 2026-09-10, lane c2-resident: the expression space is closed, exhaustively

Still 3, and the residual is confirmed unchanged: the mask, the table load and
the scale occupy the same three positions on both sides and only their creation
order differs, so this is uopt/ugen emission order and nothing else.

The candidate creates the mask first, then the scale, then the table; the
base-first class creates the table first, then the mask, then the scale; the
target creates the mask first, then the *table*, then the scale. Reading the
compiler's own listing (`cc -S` at the TU's flags) makes the three classes
visible directly, and it is a faster instrument than the score for this
residual: the class is legible in one line, before the assembler's scheduling
hides it.

Newly measured and eliminated this pass, about **1,030 further candidates**, all
of them landing in the same two classes (3 or 5) and none below:

- A **960-cell cross product**: six base spellings (`(u32)`, `(s32)`, `(u8 *)`,
  a double-dereference through the symbol's own address, an address-of-element,
  and the raw pointer) x fifteen index spellings (plain, four integer casts,
  five arithmetic-identity locks, an unsigned constant, a complement of the
  inverse mask, and a shift-pair sign extension) x six scale spellings (shift,
  multiply, `sizeof`, two casts of the shifted value, and self-addition) x both
  written operand orders, plus every subscript, reversed-subscript and
  pointer-sum form of each index spelling.
- **Compound and nested address shapes**: `*&a[i]`, `(&a[i])[0]`, `(&a[0])[i]`,
  a `char *` base, a re-cast base, three-term sums with an added zero in every
  position, a double negation of the sum, an array-typed pointer cast, and a
  `volatile`-qualified read of the base pointer (the last is +1 instruction).
- **Two-statement forms**, re-measured against the compiler listing rather than
  the score: a hoisted `s32`/`u32` index folds back into the base-first
  expression with an *identical compiler listing* over the whole arm, so
  hoisting is not a near miss but an exact no-op, and `register` on the index
  does not change that. `s16` and `u16` index locals cost 8 and 0 bytes respectively and both
  regress.

What this pass adds beyond the elimination: the target's order requires the
mask to be a ring temporary that is *emitted before the address expression
begins*, while remaining single-use. Every construct that emits it early
(a second surviving use, a comma, an assignment inside the index) makes it a
uopt symbol instead, which takes a pool colour rather than a ring number, and
every construct that keeps it a ring temporary emits it inside the expression.
Confirmed on the self-addition spelling, whose listing shows exactly the
target's *order* with the mask on a pool colour and an add where the target has
a shift. So the remaining question is sharper than "which spelling": what makes
IDO emit a single-use masked value as its own ring temporary ahead of the
expression that consumes it. A scan of every matched `main` TU's compiler
listing for the target's signature found five instances of "index materialised
before the base load", and in all five the index was an *already-live variable*
(a loop counter or a parameter), never a mask computed at the site.

#### 2026-09-11, lane s1-one: carrier identity and discarded probes both retired here

Re-measured: 3 relocation-masked words, 468 bytes, delta 0, first mismatch
+0x13C. The twin `levelInit` re-measured at 6 the same hour, and its arm
differs from this one only by the phantom pop.

The previous entry's closing observation -- that in all five matched instances
of the target's signature the index was an *already-live variable*, never a
mask computed at the site -- points straight at L115, the one-web-per-symbol
mechanism that closed `levelInit`'s 16-word colour term and overlay 9's two FP
colours the same week. It does not reach this residual, and that is now
measured rather than assumed. The masked index was named into every local this
function already has: the `void *` local, which carries its own earlier live
range and is therefore exactly the shape the colour term needed; the `s8`
local; and the `s16` loop value itself. Each was crossed with five address
spellings -- base-first, shift-first, the plain subscript, a pre-scaled
byte-offset carrier, and a byte-pointer base. Every cell is either
byte-identical to the inline form (3 or 5 words, so the carrier is
forward-substituted away exactly as recorded) or costs 4 to 8 bytes of
narrowing. Naming it in the `s16` loop value is 42 at delta 8, because the
assignment needs its own narrowing.

L109's discarded-expression probes are inert too: OR-with-zero,
AND-with-minus-one and XOR-with-zero on the named carrier, placed between the
definition and the use, are each byte-identical to the un-probed form. So the
one documented way to raise an occurrence count at zero instruction cost does
not survive to be counted here, and the "give the index a second use" family is
closed at zero cost as well as at instruction cost.

A doubled mask at the index -- the phantom-pop carrier `levelInit` uses -- is
9 words here in both address orders, so this function does not want the extra
pop.

The decision variable is unchanged: what makes IDO emit a single-use masked
value as its own ring temporary ahead of the expression that consumes it.
Three mechanisms are now measured not to be the answer.

#### 2026-09-11, lane w3-low: the ring is read directly, and the residual is closed mechanically rather than by exhaustion

Re-measured first: 468 bytes, 117 of 117 words, size delta 0, 3 relocation
masked words, first naming difference at plus 0x13C, aligner buckets 114
byte-exact, 3 register naming, 0 immediate-only, 0 really different.

Every earlier pass inferred the ring order from the emitted registers. This
pass read the instrumented ugen free-list trace instead, so the order is now
evidence rather than inference. Three facts come straight out of it.

First, the ring free list for this procedure is built once with ten members in
a fixed cycle and only the two value registers are struck from it, so all ten
temporary registers are available. Second, a ring register is drawn
immediately before each instruction is emitted, never earlier, so the draw
order is literally the emission order and the register naming is a pure
consequence of it. Third, the whole procedure makes twelve draws in one
unbroken cycle, and the four draws in the world-index arm are the eighth
through eleventh, in the order mask, scale, table, sum; the target needs mask,
table, scale, sum.

The closure argument is now structural rather than a count of forms. Colouring
any web into the temporary-register bank strikes that register from the ring
for the whole procedure, and because the cycle is shorter than the number of
draws, the shift reaches every draw in the loop and not only the ones after
the removal point. Measured three ways on the unmodified candidate by forcing
three different existing webs into the colour that names the arm's first
temporary: in all three the loop's guard register, the arm's four registers
and the loop tail's register all move. The target's guard register and loop
tail register are byte-identical to the candidate's, so the target's ring is
the full ten-member cycle, no temporary register in this procedure is a
coloured web, and all four of the arm's registers are consecutive ring draws.
That kills the reading in which the target's masked value is a coloured symbol
that happens to land on the arm's first temporary, which was the only
remaining alternative to the recorded emission-order reading.

That also settles the previous entry's closing observation, that in all five
matched instances of this signature the index was an already-live variable.
An already-live variable is a symbol, a symbol takes a pool colour, and a pool
colour in the temporary bank perturbs the entire loop. So the five matched
instances are a different configuration, not a template for this one, and the
original's masked value is computed at the site as a ring temporary. Measured
directly as well: giving the masked value a second surviving use makes it a
web whose save is the highest in the procedure, and globalcolor hands it the
lowest available colour, the first value register, displacing the entry word
onto the second value register. It is offered every colour from the first
value register down through the temporary bank and takes the lowest, so no
save ratio reaches the colour the target would need.

L127 was swept and it is real here, but it cannot produce the move this
function needs. Fifteen cells on the mask, the table base, the scaled index,
the sum, and both address orders. Identity operations, or with zero, and with
minus one, exclusive-or with zero and shift by zero, are all folded before
ugen and draw nothing. A redundant second mask of an already masked value is
not folded: it survives to ugen, draws a ring temporary, and the assembler's
peephole deletes the instruction, leaving the byte count and the frame
unchanged. That is a clean confirmed instance of the law on this body. Its
effect is a uniform phase shift of every later draw by one, measured at nine
words in both address orders, and the direction this function needs is a
transposition of two adjacent draws. A no-op can only insert a draw, never
reorder two, so L127 is a precise negative here. The peephole keeps the last
member of a redundant mask chain, so the surviving instruction takes the
second draw rather than the first, which rules out the one arrangement that
might have looked like a transposition.

L125 does not apply. The assembler builds its dependence graph per basic
block, and the arm is its own block containing one symbol load, one indexed
load, an arithmetic chain and the call; there is no store in it, so there is
no store-before-load edge to constrain anything. The node table also explains
a previously unexplained negative: the six nodes in this block have six
distinct critical-path lengths, so the scheduler never reaches a tie, and the
line-number tie-break can therefore never fire here. That is the mechanism
behind the 192 physical line splits and the 256 line-grouping cells both
coming out flat, and it means the line-key lever is not merely untried but
structurally unreachable in this block. The scheduler's chosen order is
identical on both sides, which independently confirms that the residual is
emission order and not schedule.

The new result, and the sharpest statement of the remaining gap. One
construct does make the mask a ring temporary emitted as its own step ahead of
a base-first address, which the previous entries recorded as impossible: a
comma inside the left operand of the sum, whose first element assigns the mask
and whose value is the table base. The mask is emitted first and stays a ring
temporary; the order is exactly the target's. It costs eight words for one
reason only, and the reason is general. A comma's value is materialised into a
uopt symbol, that symbol is coloured, and the table base therefore leaves the
ring, so the arm draws three registers instead of four. Ten further cells
confirm the pattern is the comma and not the spelling: the same construct with
a byte-pointer base, a pointer base with a subscript, a nested comma, an
existing local as the comma's value, a doubled comma and a preceding mask
statement all put the base on the same pool colour, and moving the comma out
to the call argument, to statement level, or to a value of zero lets copy
propagation fold the mask back into the expression at five words. Forcing the
comma's symbol into the temporary bank does not rescue it, for the ring reason
above.

So the decision variable is no longer "what makes IDO emit a single-use masked
value as its own ring temporary". That is now answered: a comma in the left
operand of the sum does. The decision variable is one step further in: a
construct that orders the mask before the address expression without
materialising any part of that address expression into a uopt symbol. Comma is
the only side-effect ordering operator in C without control flow, and it
necessarily materialises its own value, so no such construct exists in the
language this compiler accepts. Unless that is wrong, the function is closed
on the source axis and the remaining three words are not reachable by
spelling.

Also newly measured and flat or worse this pass, beyond the above: hoisted
mask locals in five widths, where the 32-bit signed and unsigned forms fold
back to five words, the 16-bit signed form costs eight bytes, and the 16-bit
and 8-bit unsigned forms leave a surviving width conversion that draws a ring
temporary and is then peepholed away, landing at nine; a two-statement form
that masks and then re-masks the same local; a form with a dead first
definition to give the local two definitions; and an assignment used as a
multiplied-by-zero or anded-with-zero operand of the sum, which either folds
or turns the mask into a web. Sixty scored cells this pass, plus nine
forced-colour compilations, one free-list trace and one scheduler node table.


#### 2026-09-11, lane p6-tight: the order term re-derived from ugen, and three new negatives

Re-measured unchanged: 468 bytes, 117 of 117 words, size delta 0, positional
masked 3, aligner buckets 114 byte-exact, 3 register naming, 0 immediate only,
0 really different, first naming-only difference +0x13C.
`tools/register_census.py` reads it as a closed two-cycle over one adjacent
ring pair at 100 percent coherence in a single window, which is the same fact
the earlier passes call a ring-index swap.

Read off the object rather than off the count, the three words are the table
load and the index shift trading ring slots while the mask and the sum keep
theirs, and the add's two operand registers are identical on both sides. The
candidate's ring order is mask, scale, table, sum; the target's is mask, table,
scale, sum. Both are four consecutive ring draws, so the phase is already
right and only the ORDER of two adjacent draws is wrong. That rules out every
phase lever: a wasted draw shifts all four and a cycle needs a transposition.

Since a postorder walk of one expression can produce only mask, scale, table
(index-first) or table, mask, scale (base-first), and both were re-measured
here at 3 and 5, the target's order requires the mask to be a SEPARATE
surviving evaluation in front of a base-first address. That is the 2026-09-10
closure and it survives this pass.

Newly measured and flat, all zero size delta:

- the carrier declared `register`, in `s32`, `u32` and `int` spellings, with the
  address written base-first and index-first: 5 and 3, byte-identical to the
  plain local, so the storage-class keyword does not defeat uopt's forward
  substitution;
- the carrier hoisted into the ENCLOSING block, in front of the arm's own
  guard rather than inside the arm: 5, so a block boundary above the guard is
  no better than one below it;
- identity-op phantoms on either operand, or on the whole sum, in all three
  forms L109 names: or-with-zero, and-with-minus-one and xor-with-zero are
  byte-identical to the base in every position tried, index-first and
  base-first alike. uopt folds them before the web builder, so they never reach
  ugen and cannot be used to buy a ring draw here. Only a redundant AND with
  the SAME constant survives, as a copy, and it draws before its own operand
  subtree, which is the wrong end;
- two identical masks combined with or-of-itself and and-of-itself common
  completely and waste no draw: 5, unchanged from the plain base-first form;
- an explicit base local, with and without the mask named first: 8;
- a mask carrier forced to two uses inside the address: 5.

The one remaining direction is unchanged and is not a spelling: a construct
that makes a separately assigned mask survive uopt's forward substitution
without becoming a pool colour. Every construct measured across four lanes
either is folded back into the address or turns the mask into a coloured
symbol.


#### 2026-09-12, lane p10-tight: the two-class closure re-derived from the compiler listing, not from scores

Re-measured unchanged: 468 bytes, 117 of 117 words, size delta 0, positional
masked 3, first naming-only difference +0x13C.

Every previous pass inferred the emission order from the emitted registers,
which is indirect because the assembler schedules the block and moves the table
pointer's load to the front on BOTH sides. Read off the compiler's own listing
instead, the picture is simpler and the closure is mechanical.

- The ring free list for this procedure is one ascending cycle, and every
  release in the loop head and the guard chain happens in draw order, so the
  four members reaching the arm are consecutive and in order. The transposition
  therefore cannot come from the free list, which confirms the p1-perm entry
  from the other side.
- In the arm the compiler emits the mask, the scale, the table pointer and the
  sum, in that order. The target's registers require the mask, the table
  pointer, the scale and the sum.
- A postorder walk of one two-operand address expression can only produce
  mask-scale-table (index first) or table-mask-scale (base first). Twenty-three
  further spellings were read off the listing this pass rather than off the
  score -- self-combined masks, doubled masks, a conditional index, a comma in
  the base operand, a multiply for the doubling, byte-pointer bases, casts of
  the base, a hoisted index in two widths, a hoisted pre-scaled offset, negation
  pairs and a subtraction -- and every one lands in one of those two classes.
  The two that do not, the doubling written as a self-sum and a re-read of the
  table entry, produce the target's ORDER and are the recorded regressions.

So the requirement is unchanged and is now stated as a property of the listing
rather than of the count: the mask must be a separately emitted evaluation in
front of a base-first address while remaining a ring temporary. The self-sum
form shows what happens when it is separately emitted -- two uses make it a uopt
symbol, a symbol takes a pool colour, and the doubling becomes an addition.

#### 2026-09-12, lane p10-tight, addendum: L145 does not reach this site

The deletion half of L145 was exercised, which the earlier passes did not
separate from the addition half. Deleting the entry carrier and writing the
table read as the expression itself at all four test sites is the recorded
56-word regression and costs three instructions, so the one-IR-name mechanism
cannot be applied to the value that feeds this arm. The mask itself has one use
and cannot be given a second without becoming a symbol, which is the same wall
from the other direction.

#### 2026-09-12, lane p23-lastmile2: exhaustive colour landscape

Fresh configured baseline: 468 bytes, zero size delta, aligned buckets 114
byte-exact, 3 naming, 0 immediate, 0 structural, first +0x13C; no candidate-only
or target-only offsets. The substitution census retains the adjacent temporary
pair exchange. Named Ucode maps procedure 17 of 21; stock and instrumented
full-TU text compare byte-identically.

The exhaustive same-kind pass samples all 10 coloured webs with 90 accepted
forces. Sixty-three retain size and 27 change it. The complete improving
winners list is empty. The empty winner-set packing retains the unforced floor
of 3; arbitrary combinations of individually regressing forces were not tested.
The window nomination table confirms that moving a pool web also changes the
already-correct earlier and later regions rather than isolating the required
mask/table/scale order.

ADR 0018 stop with zero new source attempts. The authenticated exhaustive
landscape supplies no colour handle, and the committed expression, carrier,
comma, redundant-mask and phase receipts already exclude the available source
families. The successful strip-packet OR-zero reassignment operates on a priced
cursor web; this function needs a single-use expression to remain in the
uncoloured temporary ring, so that mechanism does not answer its constraint.
The named source question remains a separate mask evaluation before the table
load and scaling, with neither the mask nor the table becoming a pool symbol.
The source body and original assembly fallback are unchanged; no byte credit.

Evidence is retained under ignored build/p23/levelFreeAll. Commands:
residual_map.py, allocator_trace_receipt.py --map-only, web_footprint.py
--every-colour, and finalize_plateau.py.

#### 2026-09-13, lane d1: per-line draw differential

The configured baseline remains 468 bytes, 117 words, frame 0x28, three
raw/masked differences, first +0x13C. Aligned buckets are 114 exact, three
naming, zero immediate and zero structural, with no gaps. The existing
exhaustive colour landscape and expression/free-list closures were read;
no colour sweep was repeated.

The procedure-17 census records 12 draws and 183 emission records. The
world-index arm contributes four draws and ten emission records, with the
same whole-procedure ring sequence documented by the previous raw trace.
Full-TU stock/instrumented text, data, rodata, symbols and relocations pass
the fidelity gate.

One source probe distributes the doubling through the mask: double the entry
value before applying the doubled mask, retaining the index-first address
form. The arm's guard ensures a positive signed-halfword entry on this path,
so this probe does not shift a negative value. The census differential reports
no moved source line: all draw and emission counts and the draw-register
sequence stay unchanged. It therefore does not supply the required draw-order
lever. The aligned result regresses to 113 exact, two naming and two structural
rows at the same extent, four raw/masked differences. The original guarded
candidate is restored.

Equal census counts do not establish instruction identity: this probe changes
the mask/scale operations while preserving their counts. The retained aligned
object delta, rather than the census's unchanged-count footer, establishes
that regression. Likewise, emission records include source-location directives;
their total is not a count of machine instructions.

Stop early under ADR 0018. This measured expression-axis probe supplies no
better residual, and the committed comma, carrier, free-list, phase and line-key
receipts already exclude the remaining available variants of this mechanism.
Repeating those lattices would not test a new explanation. The open requirement
remains a separately emitted mask before the base-first address without moving
the mask or base into the coloured pool. No source-axis impossibility beyond
the recorded compiler observations is claimed.

Sources, stock and traced objects, scores, census profiles and aligned maps are
retained privately under build/d1/levelFreeAll. Commands: align_symbol.py,
draw_census.py --save/--compare, residual_map.py --object/--against, the
workbench fidelity gate and finalize_plateau.py. No executable bytes are newly
credited; the assembly fallback remains canonical.

#### 2026-09-13, lane f1: independent draw-baseline confirmation

A fresh configured compile reproduces 468 bytes at delta zero, three raw and
masked differences, first +0x13C, and aligned buckets 114 exact, three naming,
zero immediate and zero structural with no gaps. The procedure-17 draw census
independently reproduces d1's 12 draws and 183 emission records. Stock and
instrumented full-TU text compare byte-identically. Retained source, both
objects, the census and aligned map are under ignored build/f1/levelFreeAll.

The full shard, including d1's scale-mask distribution differential, was read
before choosing a source probe. That differential already establishes no draw
or order movement on the newly proposed distribution axis. The earlier records
also cover every identified expression, carrier, comma, line and phase route.
No new hypothesis was found that changes a held-fixed decision variable, so
ADR 0018's early evidence stop applies with zero new source attempts. No colour
landscape or source lattice was repeated, and the guarded body is unchanged.
The unresolved requirement remains a separate mask evaluation before the table
load and scale while retaining all four temporary-ring draws.

Commands: lane_status.py, configured stock compile, draw_census.py profile,
residual_map.py --object, finalize_plateau.py and tools/gates.sh. This is a
baseline-confirmation plateau, not a new source-axis exhaustion or byte credit.

#### 2026-09-13, lane h1: authenticated schedule baseline

Fresh configured stock compilation reproduces 117 words, 468 bytes, frame
0x28, three raw and masked differences, and first mismatch +0x13C. All 36
static relocation tuples agree. Aligned buckets remain 114 exact, three
naming, zero immediate and zero structural, with no gaps. The procedure-17
census reproduces 12 draws and 183 emission records, four draws at the arm.
Stock and instrumented full-TU text compare identically.

The complete shard was read, including d1's scale-mask differential and f1's
confirmation. No new source hypothesis survives the recorded expression,
carrier, comma, free-list and line-key observations. ADR 0018 early evidence
stop applies with zero new attempts; no colour sweep or known-flat source
probe was repeated. This confirms the baseline, not a new axis exhaustion.
The separate-mask-before-base requirement remains unresolved and no bytes
are credited. Sources, objects, score, frame, static relocation measurements
and draw profile remain ignored under build/h1/levelFreeAll.

Commands: lane_status.py, align_symbol.py, configured stock compilation,
draw_census.py, residual_map.py --object, finalize_plateau.py and tools/gates.sh.

#### 2026-09-13, lane k1: baseline receipt and early evidence stop

Fresh configured stock C reproduces 468 bytes, 117 words, frame 0x28,
three raw/masked differences, first +0x13C, and all 36 static relocation
identities. Aligned buckets are 114 exact, three naming, zero immediate and
zero structural, without gaps. The procedure-17 census has 12 draws and
183 emission records; stock and instrumented full-TU text agree.

The complete shard and source annotations were read before proposing a probe.
The existing distribution differential already leaves this line's draw count
and sequence unchanged, while the expression, carrier, comma, redundant-mask,
free-list and line-key receipts cover the identified remaining mechanisms.
No new held-fixed decision variable was identified. ADR 0018 early evidence
stop therefore applies with zero source attempts. This is baseline confirmation,
not another source-axis exhaustion or a general impossibility claim.

The guarded body is unchanged. Source, stock/traced objects, scalar and frame
measurements, draw profile and aligned map are retained under ignored
build/k1/levelFreeAll. Commands: lane_status.py, align_symbol.py, configured
stock compilation, draw_census.py, residual_map.py --object,
finalize_plateau.py and tools/gates.sh. No matching bytes are credited.

#### 2026-09-16, lane lm-c: read only, one untested axis named

Baseline reproduced at 3, delta zero, first +0x13C. No cycle spent; the
ring-order closure is mechanically argued and the carrier-type lens this
lane used elsewhere does not reach a draw order. One axis nobody has
written down: the index as a cfe bit-field extraction of the entry word
rather than an and-mask, which is a different lowering and may
materialise the field before the base load. Cheap to refute because the
load would probably become an unsigned halfword.
<!-- plateau-handoff:levelFreeAll:end -->
