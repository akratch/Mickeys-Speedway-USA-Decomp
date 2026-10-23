<!-- plateau-handoff:overlay17DrawStrip:start -->
### `overlay17DrawStrip` plateau handoff

- source: `src/overlays/o017/overlay_017.c`
- score: 119/119 words
- frame: 0x38
- relocations: 1
- first mismatch: none
- summary: ROM-exact: unsigned OR-zero cursor reassignment reaches save 25; 119 words, one exact relocation, 476 executable bytes.

#### 2026-09-10 lane `c2-reopen`: frame is not per-declaration here

Baseline reproduces: 476 bytes, 16 relocation-masked words, zero size delta,
frame 0x40 against the target's 0x38.

Two levers the new laws open were measured and both fail:

- The induction-exit-test law, applied to the scan loop's `while (remaining--)`
  as an explicit `!=` against the wrapped sentinel, leaves the frame at 0x40
  and adds four bytes.
- Inlining the `segment` constant, which removes a declaration outright, also
  leaves the frame at 0x40 and adds twelve bytes, because the constant is then
  materialized twice.

The second result is the useful one: on this function the frame is **not** a
per-declaration reservation, so the "recover the frame by consolidating packet
locals" route named in the summary is the only one that has ever moved it, and
its allocation cost is real rather than incidental.

Verdict: **the closure stands.**

#### 2026-09-10 lane `c6-close`: the frame is declaration ORDER, and it is closed

The 0x38/0x40 frame is recovered, and the route is neither packet-local
consolidation nor any lifetime change: it is the order of the eight
declarations. Moving `textured` ahead of `vertices` and `flush` gives frame
`0x38`, 119 instructions, the `t5` spill still at sp+0x28, every other home
unmoved -- **16 -> 14 masked words**.

Why the previous two probes read as refutations, both of which are real
measurements pointing the wrong way:

- Removing one declaration (inlining `segment`) left the frame at `0x40`
  because the non-save block is `align8(4N)`: N=8 and N=7 both give 32 bytes
  and only N=6 gives 24. One removal could never move it and two would have
  been needed, which is why that probe also cost twelve bytes.
- Adding declarations to probe the ladder is inert on this body -- an unused
  local is eliminated before the frame is sized, so a dummy `s32` changes
  nothing. Both a ninth and a tenth dummy leave frame `0x40` and 119
  instructions. The ladder here is over surviving locals in *order*, not count.

Exhaustively measured on this function: all 40,320 declaration orders (floor
14; every 14-scoring order carries frame `0x38`), and all 6,720 legal orders of
the eight prologue statements with the winning declaration order held (byte-flat
at 14, so statement order is spent).

**Remaining, named.** One three-row block and seven register webs, with the
temp ring and the shared lane already exact at 33/33 and 19/19:

- structural: the candidate emits `move t2,zero` / `move a1,v0` / the count
  test three rows before the target does, which emits `move a3,zero` first.
- register: t3->a3 (2 sites), a0->v1 (3), a1->a2 (3), v1->t3, t3->t1, a0->v0,
  t1->t3, over `start`, `previous` and the packet cursor. The pool lane first
  diverges at slot 7 and the divergence is an a0/a3 permutation, so the count
  of coloured webs is right and only their order is wrong.

#### 2026-09-11 lane `w3-low`: 14 to 3, structural residual closed

Baseline reproduced first: 476 bytes, 119 of 119 words, size delta 0,
positional masked 14, displacement tax 0, buckets 105 byte-exact / 12 register
naming / 0 immediate-only / 2 really different, first naming +0x68, first
structural +0x90, frame 0x38 on both sides with identical ladders.

The call test says p1 only, and the instrumented compiler's records confirm it:
zero p2 records, twenty p1 colour decisions. So declaration order and statement
order were the right things for the previous two lanes to exhaust, and they are
spent.

**The three-row block was a web-count fault, not a schedule fault.** Written as
a test followed by a separate decrement, the loop-entry guard and the
do-while's own post-decrement build two temp symbols; the target builds one.
Spelling the guard as a post-decrement makes the front end reuse a single temp
for both, the two webs merge, and the entire prologue colouring moves onto the
target's in one step. Adopted, with a comment at the point of use.

After: 476 bytes, size delta 0, positional masked 3, displacement tax 0,
buckets 116 byte-exact / 3 register naming / 0 immediate-only / **0 really
different**, first naming +0x110, no first structural difference, frame 0x38
with the ladder unchanged.

**Decision variable for the last three words, with a receipt.** The residual is
one web: the vertex packet's segment-relocated address, coloured one register
later than the target. Four webs tie at save 30.0 with one live-range chunk
each -- the two packet-cursor block locals, the vertex-count shift temp, and
the address -- and the tie breaks on ascending web number. The address is a
uopt cross-statement common subexpression, so it is numbered after every
front-end symbol and is therefore decided last of the four. Forcing the strip
cursor's web to the fifth colour is ACCEPTED and reproduces the target exactly,
119 of 119 words; forcing the address web to the fourth is DECLINED with the
first four colours already in its forbidden mask, so this is interference, not
a ratio.

Axes swept at this state, cell counts in parentheses, every one byte-flat at 3
unless noted:

- loop-entry idiom and prologue line layout (9): post-decrement wins at 3;
  `remaining-- != 0` and `remaining-- > 0` are worse; splitting the three
  initialisers onto separate physical lines is flat, reversing their text
  order costs one word.
- the count-to-counter assignment respelled or dropped (5): all -8 bytes, so
  the copy is load-bearing and the swap is not the lever.
- packet-macro temp identity (9): binding the address to a named local before
  or after the cursor, re-reading the stored word, and hoisting the cursor are
  all flat; binding the strip macro's address to a local costs +8 bytes.
- statement fusion by comma operator, both orders, and store-order swaps (6):
  flat. The front end lowers the comma before the local common-subexpression
  pass, so this does not turn the address into an early-numbered temp.
- cursor carrier identity (5): carrying either cursor in `previous` or `start`
  regresses to 17 or 19, or loses 8 bytes.
- cursor-web splitting into two locals, byte-pointer and word-pointer cursors,
  reordering the strip write against the start reset (8): flat, because copy
  propagation merges the split straight back. Two orderings cost 8 words.
- L109 discarded-expression probes on the address, or-with-zero,
  and-with-minus-one and xor-with-zero (3): every save in the records is
  unchanged. **L109 does not reach a subexpression uopt has already folded** --
  the probe is eliminated before the reference count, exactly the narrowing the
  brief warns about, and this is the measurement for it.

The arithmetic that bounds the cell: the strip cursor's save must land strictly
between the counter web's 20.6 and the group's 30.0, or the address web must be
numbered below the strip cursor's. Every reference in that block is at loop
depth one and weighs ten, and all four webs have a single chunk, so a save
there can only be a multiple of ten: 20.0 hands the fifth colour to the counter
web instead, and 30.0 keeps the tie. A fractional save needs the chunk count to
move, and no source form measured here moves it. The web number cannot be
reached either, because a uopt common-subexpression temp is always numbered
after the front-end symbols it is built from.

Not run in this lane, per the lane rules: no build, no `gmake verify`, no
commit.

##### Bounded second pass: the tie set and the numbering, both measured

The first pass swept the save. This pass swept the other two routes, and both
are now refuted from the records rather than from arithmetic.

**Which web actually stands in the way.** With the strip cursor forced to the
fifth colour, the address web's own decision record reads forbidden bits for
colours one, three and five, available bits for colour four upward, and it
takes colour four. Its interference list names fifteen neighbours, of which
exactly four are coloured at that moment: the command-value web at colour one,
the vertex cursor at two, the vertex-count shift temp at three, and the strip
cursor at five. Unforced, that fourth entry reads colour four instead. So one
web stands between the address and the register it wants, it is the strip
packet's cursor, and it is invisible in the emitted code because its copy is
peephole-deleted.

**Reducing the tie set works on the address and breaks the counter.** Serving
both packet macros from one shared cursor local collapses the tie from four
webs to two: the merged cursor's save doubles to 60.0 and it is decided first,
the command-value web slides to the second colour, and **the address does take
the fourth colour, the one it wants**. But the counter web then takes the fifth
colour where the target has it in the sixth, and the merge deletes a copy, so
the candidate is 8 bytes short at 117 masked words. Measured identically with
the shared cursor declared in the inner block, in the enclosing flush block,
and at function scope, and again with the segment constant inlined to hold the
local count at eight, which keeps the frame but scores 99 at plus 4 bytes.
Carrying the vertex-count shift temp in `previous` instead grows the tie to
five webs and costs 20 bytes.

That is the exact shape of the wall, now confirmed by measurement: the four-way
tie is load-bearing. Five webs must be decided before the counter web so that
it lands in the sixth colour, and the address must be third of them. Removing a
tie member gives the address its colour and takes the counter's away.

**The numbering cannot be reordered by declaration.** Declaring the strip
cursor before the vertex cursor, hoisting either or both cursor declarations
into the enclosing flush block, and every combination of the two all produce a
byte-identical record set: same four web numbers, same saves, same chunk
counts, same colours, same score. Web numbers here follow the order of the
defining assignments in the intermediate form, not the order of declaration, so
no declaration placement reaches them. Reordering the assignments would mean
emitting the two packets in the wrong order, which the format forbids.

**Making the contested neighbour a uopt temp works and costs the command-value
web.** Writing the strip packet through the command pointer and incrementing
afterwards does stop the cursor being a front-end symbol -- the tie set falls
to three and the address moves up -- but the command-value web stops being one
shared four-site range, its save falls from 33.0 to 18.0, it loses the first
colour, and the candidate scores 116 at zero size delta.

**The cell, stated exactly.** A fourth web is needed whose save is strictly
between the counter web's 20.6 and the group's 30.0. Every reference in that
block sits at loop depth one and weighs ten, and all four tied webs have a
single live-range chunk, so a save there is a multiple of ten: 20.0 hands the
fifth colour to the counter web and 30.0 keeps the tie the address loses. A
fractional save needs the chunk count to move, and nothing measured here moves
it. Second pass cell counts: 6 merge and carrier cells, 6 numbering and
inlining cells, 2 forced-colour receipts, 1 interference dump.

##### Third pass: the divisor fitted, and a positive control for the interval

The previous pass closed with "a save in that block can only be a multiple of
ten". That was wrong, and the counter web in the same block reading 20.6
disproves it. The divisor is real and it is movable. Corrected here, by
perturbing the strip cursor web directly and reading its own record.

**The fit.** For a web, save is totalsave over the divisor; totalsave is the sum
over references of ten to the loop depth, counting the defining reference; and
on this web the divisor is the number of definitions. Base: three references,
one definition, totalsave 30, divisor 1, save 30.0. Giving the cursor a second
definition leaves totalsave at 30 and moves the divisor to 2, save 15.0 -- at
zero size delta. Adding one read-modify-write through the cursor makes it five
references and two definitions, totalsave 50, divisor 2, save 25.0. Adding two
makes it seven and two, totalsave 70, divisor 2, save 35.0.

**Positive control: the interval is real and hitting it gives the target.** At
save 25.0 the ladder comes out as the target has it -- the address web takes
the fourth colour, the strip cursor the fifth, and the counter web the sixth.
That is the first source form in three passes to reproduce the target's colour
assignment without a forced colour. It scores 24 at plus 4 bytes, because the
read-modify-write that bought the divisor emits an instruction. So the
requirement named in the second pass is confirmed exactly, and the obstacle is
not the colouring but the instruction that pays for it.

**Both zero-cost routes to the interval are refuted by measurement.** A
discarded or-with-zero probe on the cursor itself is folded away entirely --
totalsave, divisor, colour and score all unchanged -- so a reference cannot be
added for free. Hoisting the cursor to function scope and giving it a dead
initialisation outside the loop, which would have put one of its three
references at loop depth zero and landed the save at 21.0, is likewise
eliminated before the count: totalsave stays 30 and the divisor stays 1. A
reference that costs no instruction is also a reference uopt does not count.

**The reachable set, with the fit behind it.** With every reference at loop
depth one, save is ten times the reference count over the definition count, and
landing strictly between 20.6 and 30.0 needs the definition count to sit
strictly between a third and 0.4854 of the reference count. At three
references that interval holds no integer, which is why the cursor as written
can only read 30.0 or 15.0. The first reachable points are five references over
two definitions at 25.0, seven over three at 23.3, and eight over three at
26.7 -- each of which costs at least two more instructions than the target
has. So the interval is non-empty but **empty at zero size delta**, and that,
not the earlier arithmetic, is the closure.

**One prediction left untested**, because this pass was stopped for an
unrelated promotion: the o009 idiom, an invisible copy web that forbids the
contested colour to the neighbour rather than moving anyone's save, would work
here in principle -- if the strip cursor were forbidden the fourth colour at
its own decision it would take the fifth and the address would take the fourth.
But interference here is a block-set intersection, and the strip cursor and the
address are live in the same single basic block, so any web that interferes
with one interferes with the other and forbids the colour to both. That is
reasoning, not a measurement, and it is the first thing the next lane should
falsify. Third pass cell counts: 5 perturbation cells with records, plus the
base control.


#### 2026-09-11, lane p6-tight: the second-definition lever is not byte-free at this site

Re-measured unchanged: 476 bytes, 119 of 119 words, size delta 0, positional
masked 3, aligner buckets 116 byte-exact, 3 register naming, 0 immediate only,
0 really different, first naming-only difference +0x110. One source register
maps to one target register at all three sites, 100 percent coherent, one
window, no cycle -- one web, as recorded.

The recorded arithmetic says the strip cursor's save has to land strictly
between the counter web's 20.6 and the tied group's 30.0, and that the divisor
is the number of definitions. A second definition alone halves 30.0 to 15.0,
which is below the counter, so the only reachable points are a second
definition PLUS two more references: five references over two definitions is
25.0, seven over three is 23.3. Both need a copy of the cursor that survives
into the records.

Twelve such copies were measured, and the lever is empty at zero size delta:

- a self-assignment of the cursor placed after both packet stores is
  dead-code-eliminated and is byte-identical to the base at 3, twice over;
- the same self-assignment placed between the two stores, or before them, is
  11 words at zero size delta -- it survives, it does move the colouring, and
  it moves it the wrong way;
- a cast copy, an add-of-zero copy, a copy through an unsigned round trip, a
  declaration split from the assignment, two self-assignments in a row, and
  binding the second store to a separate named pointer are all 11 as well.

So on this web the two outcomes are "the copy is deleted and the save does not
move" or "the copy survives and costs eight words". That is the measured form
of L126's bound for this site: a copy carrier is byte-free only where it is
also record-free. The interval the recorded arithmetic identifies is still
non-empty in principle and still unreached.


#### 2026-09-12, lane p10-tight: the whole colour ladder read from the records, and L147 does not open it

Re-measured unchanged: 476 bytes, 119 of 119 words, size delta 0, positional
masked 3, first naming-only difference +0x110.

The instrumented compiler was gated first: its object's text for this symbol is
byte-identical to the configured build's, compared byte for byte rather than by
score.

**The ladder, complete, from this procedure's own records.** Twenty phase-one
decisions, no phase-two records, and the selection order is strictly descending
save, which is L100 agreeing with itself on a p1-only procedure. In decision
order, with save, the divisor the records call nocs, and totalsave:

- the command value web, 33.0, divisor 2, total 66, takes the first colour
- four webs tied at 30.0, divisor 1, total 30 each, take the second through
  fifth colours in ascending web number
- the counter web, 20.6, divisor 5, total 103, takes the sixth colour

That reproduces the recorded arithmetic exactly, including the 20.6 that the
second pass called impossible, and it confirms the third pass's fit of save as
totalsave over the divisor.

**L147 does not reach this residual, and the records say why.** The lineage
capture puts all four tied webs in ONE basic block, each with divisor 1. Reading
the divisor as a count of basic blocks rather than of definitions therefore
changes nothing here: it is already at its floor, and the only direction a block
count can move is up, which divides the save further and lands it below the
counter web. The third pass's reachable-point arithmetic stands under either
reading.

**What the records add to the cell.** With every reference at loop depth one
weighing ten and one reference at depth zero weighing one, a divisor of 1 admits
only totals of 30 for three references, 21 for two plus one at depth zero, and
12 for one plus two. Exactly one of those, 21, lands strictly inside the open
interval, so the requirement is now a single sentence: ONE of the strip cursor's
three references must sit at loop depth zero and survive. The third pass already
measured that a dead initialisation outside the loop is eliminated before the
count, and the two packet stores and the cursor's own definition cannot leave
the loop without changing what is emitted. The alternative, five references over
a divisor of two at 25.0, is the recorded positive control and costs two
instructions.

**The prediction the third pass left untested is refuted.** An invisible web
that forbids the contested colour to the strip cursor would have to be live in
the block the strip cursor occupies, and the address web occupies that same
single block, so interference by block-set intersection reaches both and forbids
the colour to both. The reasoning the third pass gave is correct and can be
retired as a lever.

#### 2026-09-12, lane p23-lastmile2: source and ROM exact

Baseline: 476 bytes, size delta zero, aligned buckets 116 byte-exact, 3 naming,
0 immediate, 0 structural, first +0x110, no unmatched offsets. Named Ucode
maps procedure 0 of 1, and stock/instrumented full-TU text is byte-identical.
The exhaustive same-kind landscape samples all 20 coloured webs with 131
accepted forces. The COMPLETE improving winners list is one entry:
`p1:w99=c5`, score 0 at size delta zero, radius confined to the +0x100 window
with three differences removed. The singleton lattice floor is 0; no packing
or combination is needed because this one handle answers the entire residual.

Twenty-four literal-type cells retain 3 or grow by eight bytes. Nine sequential
cursor forms confirm that a real pointer increment reaches the desired colours
but adds an instruction: the best such form has no aligned naming residual,
one surplus instruction and changed branch displacements. Sixteen algebraic
reassignments then distinguish the missing mechanism. Eight OR-zero/XOR-zero
forms produce all 119 words exactly with stock IDO. Add-zero and subtract-zero
copies stay at 11; AND-with-all-ones forms grow by 12 or 16 bytes.

Adopted: unsigned OR-zero reassignment of the initialized strip-packet cursor
between its two stores. The fresh unforced trace gives that cursor total 50,
divisor 2, save 25, and colour 5. Its former record was total 30, divisor 1,
save 30, colour 4. Thus the prior zero-size closure is disproved: an algebraic
read-modify-write can retain two references that a plain self-copy lacks, and
IDO can remove the operation after allocation. The emitted source object has
119 exact words, frame 0x38, and zero naming/immediate/structural rows.

Semantic review, independently of the matching score: the cursor is initialized
from the same single post-increment expression as before. This target ABI uses
32-bit pointers and unsigned 32-bit u32. Conversion to u32, OR with zero, and
conversion back preserve every pointer bit. Only the local cursor is read and
assigned; no memory read, store, call, branch or increment is added or reordered.
Both packet stores retain their original destinations and values. This is an
inert allocation spelling, not evidence of an original debug macro. The source
discloses it and docs/cleanup-queue.md records naturalization.

Promotion follows the complete atlas-write, digest refresh, extract,
overlay-syms, build, SECOND overlay-syms and rebuild sequence. The function
completes its TU, so there is no mixed-TU range to retain. promotion-proof
passes: 119 words, frame 0x38, 1/1 exact static relocation identities, and the
owned linked ROM range exact. gmake verify prints the expected SHA-1
507341c0a40ca3e9a7cee969b396ee53facfb548. Newly matched executable credit is
476 bytes, with no padding credit. finalize_plateau.py recorded the exact
candidate before the guard was removed. Ignored evidence is retained under
build/p23/overlay17DrawStrip.

<!-- plateau-handoff:overlay17DrawStrip:end -->
