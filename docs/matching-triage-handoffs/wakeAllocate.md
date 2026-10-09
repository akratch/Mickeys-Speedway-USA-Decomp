<!-- plateau-handoff:wakeAllocate:start -->
### `wakeAllocate` plateau handoff

- source: `src/main/fx.c`
- score: 16/351 words
- frame: 0x90
- relocations: 3
- first mismatch: +0x94
- summary: Unchanged at 16; uoptlist: no carrier or region boundary moves the conversion's creation out of its own block-0 statement. Left: the pre-call spill ladder.

Summary before this remeasure: Unchanged at 16; natural counts lose the dead copies (-8), declaration order does not move spill cells. Left: the pre-call spill ladder.

Summary before this remeasure: Unchanged at 16; slot trace, natural count product and lever sweep flat. Left: the pre-call spill ladder (web creation order).

Summary before this remeasure: NULL stores after the samples pointer, post-link stores reordered: 26 to 16. Left: only the pre-call spill ladder (16 immediate).

Summary before this remeasure: Alpha-start store moved after the texture index (lever_sweep reorder): 57 to 26. Left: pre-call spill ladder (14 immediate), tail rows.

Summary before this remeasure: XOR-kept copies, counts inline: 323 at -8 to 57. Left: cvt and segment must be kept symbols created late (target s0/t0); loop-bound copies alone go late

Summary before this remeasure: XOR-kept copies, counts inline: 323 at -8 to 57. Left: spill homes follow web number; pre-call webs must number after the loop temps

Summary before this remeasure: XOR-kept copies give the dead v0/v1 moves (size 0); counts inline so the segment copy takes v0: 323 at -8 to 57. Left: pre-call spill-cell order (web numbering)

Summary before this remeasure: XOR-kept copies of the two counts after the alpha branch give the dead v0/v1 moves: -8 to size 0, 323 to 60. Left: copies' v0/v1 order and spill-cell order

Summary before this remeasure: Forces w14/w324/w10/w5 leave only the two dead pre-call copies and the spill-cell order; eight source negatives recorded

Summary before this remeasure: sampleBytes after segmentCount (target spill-cell order): 325 to 323 at -8. Left: dead v0/v1 copies before the call, four cells out of order

Summary before this remeasure: Divisors read textureIndex back: tail ring aligned, naming 84 to 9 at -8. Left: dead v0/v1 copies before the call (split pieces).

Summary before this remeasure: Divisors read wake->textureIndex back (store forwarded): tail temp ring aligned, naming 84 to 9 at -8. Left: dead v0/v1 copies before the call.

Summary before this remeasure: Natural rewrite (own triangle count, if/else flags, word id, one cursor): -32 to -8, frame 0x90. Left: dead v0/v1 copies before the call.

Summary before this remeasure: JFG efd5abb remains assembly-only; zero source attempts. Need new initialization homes and buffer-loop topology evidence.


Reopening audit (2026-09-08), evidence D: PROVENANCE inspection of Jet Force
Gemini public decomp `src/fx.c` and `src/fx.h` at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no new target C body.
No donor source, names or values were adopted. Mickey remains authoritative.

Configured full-TU measurement: candidate 1372 bytes / 343 words,
target 1404 bytes / 351 words; size delta -32 bytes.
Raw and relocation-masked differences are 345 and 345, respectively,
first mismatch +0xC; candidate/target frames are
0x98/0x90. Candidate/target static relocation counts are
3/3; 0 tuples agree in function-relative offset, type and symbol.
These are fallback-object comparisons, not linked-C promotion evidence.

Workbench comparison: `structure-mismatch`. Diagnosis: `mixed(constant:25, structural:33, register:116)`;
playbook `constant-audit`, lever `stack-home`. The named guides were read.
This heuristic routing supplies no new donor evidence to reopen exhausted forms.
Next concrete lever requires new wakeAllocate donor C exposing initialization order, early stack homes and buffer-loop topology,
then a fresh gate and configured full-TU comparison.

Stopping evidence: zero source attempts; the authorized donor mechanism supplies
no new implementation. This is the assignment's early-exhaustion stop, not a
five-attempt stall. The prior TU lattice is not repeated: since its recorded
`func_8004ACC4` audit at `4be95a3d`, this TU differs only in EOF handoff comments.
See [the anchor donor audit](func_80049E4C.md) for the full delta and batch disposition.

Ignored evidence is retained in `build/tu-fx-audit/` and `build/wb/`: source and
full-TU object, gate receipts, summaries, diagnoses and relocation tuples.
Commands: `lane_status.py --symbol`, `wb_compare.sh --summary-json`, workbench
`diagnose` and `guide`, and `finalize_plateau.py`. The unchanged guarded C stays
NON_MATCHING and receives zero new exact bytes.

Width-only causal audit (2026-10-04), Mickey evidence B: the retail allocator
forwards its fifth incoming stack argument as a full word. The independently
matched sole evidenced caller sign-extends its signed-short source field before
storing the outgoing word, and the exact resource loader accepts a signed word
and masks it to its low sixteen bits before resource behavior. Direct resident
and overlay calls, runtime call records, source references and stored absolute
function pointers were checked; arbitrary computed indirect calls remain
outside this exhaustive claim. No shared declaration or header was changed.

One isolated contrast changed only the fifth formal and its same-TU forward
declaration from signed short to signed word. The configured output changed
only that argument import from a halfword load to a word load. The candidate
remained 1372 bytes / 343 words, delta -32 bytes, frame 0x98, 345 raw and masked
differences, first mismatch +0xC, with the same three relocation sites. All
thirty-eight other configured TU functions, including the exact caller,
retained identical owned bytes and function-relative relocation identities.
Actual compiler-input capture and stock preprocessing accepted each baseline
and contrast self-context; replay fidelity passed text, data, rodata, symbols
and relocations. Whole-file debug metadata differences were disclosed. The
approved formal/declaration change correctly reports changed cross-context.

The historical full-word fifth formal belonged to an abbreviated draft with a
different allocation topology and extra resource-loader arguments; it was not
a prior isolated control on the restored body. This fresh control eliminates
argument import width as the cause of the current structural deficit. The
unchanged baseline remains tracked; the contrast and raw receipts are private
ignored evidence in `build/wake-word/`. No candidate or exact bytes are adopted.
Next work requires independently authenticated topology or producer evidence;
no declaration, buffer, home or allocator grid follows this negative result.

Flags-CFG causal audit (2026-10-04), Mickey evidence B: the retail guarded
flags initialization branches to separate byte stores of one or zero, whereas
the configured baseline computes a Boolean and emits one byte store. One
isolated explicit if/else contrast at that same nonnull-loader point, on the
proved full-word fifth-formal reconstruction, retained this fork in stock
output. Both paths still execute exactly one store, with the same full signed
word condition, guard, byte value, call order and other expressions.

The contrast measured 1384 bytes / 346 words versus 1404 bytes / 351 words,
delta -20 bytes, 342 raw and masked differences, first mismatch +0xC and
frame 0x98 versus 0x90. It has the same three call identities; relocation
positions remain nonexact. All thirty-eight other configured TU functions,
including the exact caller, retained identical bytes and function-relative
relocation identities. Actual compiler-input self-context and stock replay
fidelity passed text, data, rodata, symbols and relocations; cross-context outside the owned body
remained unchanged. The fixed full-word baseline reproduced the
previous width-only result rather than constituting another width hypothesis.

This proves a source CFG discrepancy and closes three words of the size
shortfall, but leaves five words and the frame deficit. No supported additional
source lever was established within this packet. The canonical diagnostic
baseline is restored unchanged; the defined branch contrast, configured
objects, capture receipts and scores remain ignored in `build/wake-flags/`.
Neither a private candidate improvement nor a mask score receives match credit.
Further work needs fresh producer or structural evidence, not old allocator,
loop, home or declaration grids.

#### 2026-10-07: divisor conversion audit closes without a source change

The configured full-TU baseline reproduces 343/351 words, 345 raw and masked
differences, size delta -32 bytes, first mismatch +0xC, and frame 0x98 against
0x90. Captured compiler-input self-context passes; replay agrees with the
configured object in every allocated section, symbols, and relocation tuples.

The proposed missing unsigned-to-float correction was a mistaken initial
reading: the baseline already emits that correction for its byte divisor.
Explicitly widening the byte cast to u32 is executable-byte-inert. Replacing
the divisor with an unsigned low-byte mask adds one instruction and measures
344 differing words at -28 bytes, while opcode distance worsens from 30 to 36
and aligned structural differences from 33 to 39. This is not a structural
improvement and is not adopted. Both forms preserve the byte-valued divisor.

Source is restored unchanged. The conversion hypothesis is closed; no new
matching bytes. Ignored source, actual inputs, objects, context and fidelity
receipts remain under build/wake-conversion. Existing word-argument and flags
CFG findings remain separate; this packet does not repeat or promote them.

#### 2026-10-07: induction-bound identity controls

Read-only target comparison identifies three count webs: one original count
for guards/remainders and separate copies for the ten-byte and sixteen-byte
unrolled termination extents. The configured baseline instead shares one
copy between those extents. Neither missing allocation space nor reuse of a
saved stride explains this discrepancy.

Three configured full-TU controls, each with accepted compiler-input context:

- Unsigned conversion of only the second loop's address index leaves the
  entire executable text byte-identical to the 343-word baseline.
- A separate signed second-loop index retains the same single bound copy,
  343 words and opcode distance 30; its frame grows from 0x98 to 0xA0.
- An unsigned second-loop index with a signed loop test retains one shared
  bound copy but prevents that loop's remainder/four-way unrolling. It emits
  324/351 words, opcode distance 53 and 97 aligned structural differences,
  versus 30 and 33 on the baseline. The original signed trip domain is retained;
  the smaller output is not progress toward the target.

Independent inspection of frozen configured objects confirms that none creates
the target's third count web. These typed-address and induction-index identity
controls are closed; no further causal lever emerged. Source is restored,
with unchanged allocated sections, symbols and relocation tuples. No matching
credit. Private source/object/context captures remain in build/wake-conversion.

#### 2026-10-07, lane a-front: natural rewrite, -32 to -8 bytes

345 masked at -32, frame 0x98, to 325 masked at -8, frame 0x90 (exact).
Aligner after: byte-exact 234, register naming 84, immediate only 8, really
different 27; one-sided words 2 candidate, 4 target (before: naming 109,
immediate 19, structural 33, 16 one-sided). Seven measured cycles, products
of 16, 12, 4, 18, 5, 18 and 32 cells. What moved it, each measured:

- The second fill loop bounds on its own variable, triCount = segmentCount
  * 2 written again after groupCount. uopt keeps the two variables apart, so
  the unroller takes one bound copy per loop: three count webs, as shipped.
  This is the "third count web" the induction-bound controls could not
  create (they varied the index, never the bound). Worth 12 bytes.
- The flags store as if/else (the 2026-10-04 contrast, kept), the alpha
  default as default-then-override (an if/else there adds a branch), and the
  resource id as a full word.
- The textureBytes product reads triCount, not groupCount.
- No size or vertexBytes locals, and one pointer local walking both areas
  (the vertex-buffer base, then the sample area): each declared local costs a
  frame cell here, and 0x90 needs exactly these. With i declared first and
  groupCount, alpha, j, triCount, bufferCount last, the three declared homes
  the target spills around both calls (groupCount, alpha, triCount) land on
  the target's offsets. Declaring order among the first eight is inert.
- value36 is stored before the four byte clears (the target's schedule).

Measured flat or worse: computing the vertex base inline (wake + 0x40
inside the loop is re-added per iteration, +12 bytes), (u8 *)(wake + 1)
(+12), re-reading wake->vertices and wake->samples instead of the cursor
(+28), bufferCount as a literal 2 (the loop unrolls away, -144), s32/u32 on
segmentCount, groupCount and frameCount and a two-statement segmentCount
(all byte-identical), u32 triCount (-72).

Left, from the decision records (proc 9, instrumented .text identical to
the stock object): the two missing words are dead copies the target makes
just before the first call, the segment count into v0 and the group count
into v1. In our records those values exist only as expression webs (types
4): the variables were propagated away, and the split webs whose pre-call
pieces take v0/v1/a2/a3 are sampleBytes, vertexBytes, segmentCount and
frameCount, in that order. The target colours the variables' pieces v0 and
v1 first, then sampleBytes/vertexBytes a2/a3, and frameCount's piece lands
on s0. Decision variable: what keeps segmentCount and groupCount as symbol
webs live across the first call (the target re-reads segmentCount from its
spill cell after the call, as we do).

#### 2026-10-07, lane c-fx: what the two dead copies are (no source change)

Score unchanged at 325 masked, delta -8, frame 0x90. Instrumented .text on
proc 9 is identical to the stock object.

The copies are not variables. `cc -S` on our body shows as1 forwarding
copies into later uses: ugen's `move` of the shifted count into the count
web, then a multiply reading the web, comes out of as1 as a shift reading
the original temporary. Read the target the same way: ugen emitted the
two moves into v0 and v1 at a split-piece boundary, every later use in that
block, the two spill stores included, was forwarded back to t0 and t5, and
the moves were left dead. So in the target the segment-count web (ours
w10) and the doubled-count web (ours w14) are each split, with their pieces
in the block before the first call coloured v0 and v1 ahead of sampleBytes
and vertexBytes (w327 and w332, save 0.75 each, which take v0/v1 here).
Here w10 (save 0.556, nocs 9) is split after those two, seeded at block 0
and grown through block 2 in one colour (a2), and w14 (save 4.33, nocs 15)
is not split at all: globalcolor gives it t5 with a caller save around the
call. frameCount's piece lands on s0 in the target because v0 through t5
are all taken over blocks 0-2 once those two extra pieces exist.

Measured on the way, each worse or flat: groupCount as a shift (164
positional at +4, but a third saved register and frame 0xA0), groupCount
and triCount after the alpha branch (351 at -4), the inner bounds written
as the product (342 at -20), a copy of the count before the call (327 at
-8; propagated away), one to four OR-with-zero assignments of the count
before the call (the whole web moves to v0; 325 to 328), the alpha branch
first (350 at -16), segmentBytes from the count times 0x14 (346 at -16).

Decision variable: w14 must fail globalcolor at its turn and both split
webs' block-2 pieces must outrank 0.75. Next: read w14's cost list and
forbidden mask in the records, and find the source change that takes away
its last colour (an extra web live across all blocks, the way case 2's
limit local did for func_80049B14 in this TU) before any spelling product.

#### 2026-10-07, lane c-fx (resumed): the tail's temp ring, naming 84 to 9

Positional 325 at -8 both sides; aligned before byte-exact 234, naming
84, immediate 8, really different 27 (plus 6 one-sided); after 308, 9, 8,
31. Ten measured cycles.

- residual_map split the 84 naming rows: about 75 of them are one closed
  four-cycle over t6..t9 from +0x280 to the end, a single ring-draw phase.
  The draw is frameCount after the call: the target reloads it from its
  spill cell into a ring temporary at its use, where ours coloured the
  post-call piece (records: proc 9, w374, v1). Forcing that piece to
  split (p1:w374=s, accepted) prices it at naming 84 to 9.
- Source that does it: both divisors read wake->textureIndex instead of
  (u8) frameCount. uopt forwards the byte just stored, so the value is the
  reload masked once, as shipped. Mixed forms (one divisor each way) lose
  the ring again and land at -4 (311 and 312 positional, naming 84).
- Forcing w327, w332, w10 and w5 to the target colours (c5, c6, c7, c14)
  on top leaves naming 1 and 15 structural rows: what remains is the two
  missing pre-call words and their schedule.

Flat or worse, measured: an extra web for the byte-size product, the
vertex byte count or the loop bound as named locals (twelve cells, 325
to 345, none produces the copies); forcing w14 to split (p1:w14=s,
accepted: the block 0-2 piece takes v0 and the rest goes to memory, 337
at -16), so the doubled count is not split in the target by that route.

Next: the two dead copies. They are block-2 pieces coloured v0/v1 ahead
of w327/w332; the forced split above shows ours seeds at block 0 and grows
through block 2. Read the growv records for w10 under a candidate where
block 2 holds one more live value, and look for the source that makes the
growth into block 2 refuse.

#### 2026-10-07, lane g-7: count spellings, flat (325 at -8 kept)

One 12-cell product on the retained body: groupCount/triCount written
as two products (kept), `groupCount = triCount = segmentCount * 2`, the
reverse chain, and `triCount = groupCount`, times three orders of the
three byte-size statements. 325 to 327 at -8 everywhere; the chain forms
are 326 and 327. No cell creates the block-2 pieces.

Reading of the target's frame (tools/frame_census.py): its seven spill
cells are one contiguous run, +0x24 to +0x3C (textureBytes, segment
bytes, the doubled count, segmentCount * 0x14, sampleBytes, segmentCount,
frameCount); ours splits them into +0x20 to +0x2C and +0x40 to +0x48
(the doubled count, segmentCount, frameCount). The three that land apart
are exactly the three webs c-fx names (w14, w10 and frameCount's piece),
so the frame is a second readout of the same decision.

Cycle-21 line: unchanged from c-fx (w14 must fail globalcolor and the
block-2 pieces of w10/w14 must outrank 0.75); confirm any candidate with
frame_census first, since the spill run moving to +0x24..+0x3C is the
cheapest sign the pieces exist.

#### 2026-10-07, lane h-7: four products, all flat (325 at -8 kept)

Instrumented .text on proc 9 identical to the stock object. Each product
measured with shape_product; no cell moved the size.

- A size local for the allocation, sampleBytes inline or as a local, and
  wake->segmentCount from segmentCount or from (frameCount + 5) >> 1:
  8 cells, 325 or 327 at -8.
- Unsigned literals on the four size products (the sizeof shape, which
  converts the count operand): 16 cells, all 325, so uopt drops the
  conversion and makes no second name for the count.
- Position of bufferCount = 2 (7 places) times alpha = 2 (6 places):
  42 cells, 325 or 327.
- Dead reads of the counts into the existing i, j and cursor before the
  first call (7 cells), and the count carried in j across both calls into
  wake->segmentCount (2 cells): all 325; uopt copy-propagates every one.
- Forces on proc 9 that take t5 from w14 (w16, w12, w295, w60, w70 to c12;
  w14 to c13), accepted: 331 to 335, still -8. No colour force produces
  the two pieces; they need a split, not a colour.

Reading (proc 9 records): growth refusals elsewhere in the TU happen with
colours left (left_after 7 to 11), so a piece is not refused for want of
colours; the refusal rule is still unread. The two dead moves are the
block-2 pieces of w10 and w14 whose uses as1 forwards back to t0 and t5.

Cycle-21 line: unchanged (w14 must fail globalcolor and the block-2
pieces of w10/w14 must be coloured before w320/w325's). First read the
growv refusal rule from the records of a refused case (proc 47 has
several), then look for the source change that makes w10's growth into
block 2 refuse.

#### 2026-10-07, lane i-6: spill-cell creation order, 325 to 323 at -8

Measured by tools/bank.py: masked 323 (raw 323), size delta -8, candidate 349 words vs target 351. Aligned: byte-exact 311, register naming 10, immediate only 6, really different 29.

Aligner after: byte-exact 311, naming 10, immediate 6, really different
29 (before 308, 9, 8, 31). The target's seven pre-call spill cells read,
from the top down, frameCount, segmentCount, sampleBytes, segmentCount *
0x14, the doubled count, segmentBytes, textureBytes; expression-temporary
homes are handed out in creation order, so the target creates sampleBytes
straight after segmentCount. Moving that one statement there puts our
sampleBytes cell between segmentCount and the doubled count (325 to 323,
immediate 8 to 6). Adding segmentCount * 0x14 through a local before
groupCount completes the target's cell order exactly but scores 325.
What is left of the frame: four cells sit between the doubled count and
segmentBytes in ours, while the target has those four above frameCount;
moving alpha, bufferCount and the alpha branch above frameCount does not
move them (pre-placement product, 30 cells, flat at 323).

Measured flat, each a product on the 323 or 325 body: groupCount and/or
triCount after the alpha branch (8 cells, 337/338 at -12 or flat); copies
of segmentCount, groupCount or triCount into i or j in the call block,
used by the byte products (16 cells, all 325: uopt propagates every copy).

Records (web_report, proc 9, identity-gated): split() seeds a piece at the
first liveblock that passes, in list order; w10's piece seeds at bb0 (its
def) and grows into bb2 at new 4, left 16 to 11, numintf 7, far from the
L161 refusal (needs left_after 5 or fewer). So the target's v0 copy is not
a refused growth of w10 from bb0; it needs either a seed at bb2 or a
separate web.

Cycle-21 line: find the four temporaries in our cell gap (between the
doubled count and segmentBytes) and what makes the target create them
before frameCount; the same creation order decides which webs exist in
bb2 when w10 and w14 are coloured.

#### 2026-10-08, lane j-6: what the forces leave, and eight negatives (323 at -8 kept)

Measured by tools/bank.py: masked 323 (raw 323), size delta -8, candidate 349 words vs target 351. Aligned: byte-exact 311, register naming 10, immediate only 6, really different 29.

Score unchanged: 323 masked at -8, aligned byte-exact 311, naming 10,
immediate 6, really different 29.

Forces on proc 9 (instrumented compile of the tree body, each accepted):
p1:w14=c5 (sampleBytes piece a2), p1:w324=c6 (segmentCount * 0x14 piece a3),
p1:w10=c7 (segmentCount piece t0) and p1:w5=c14 (frameCount piece s0).
The object then differs from the target only in: the two dead copies
before the first call (segment count into v0, doubled count into v1), the
five pre-call spill cells (ours sampleBytes +0x40, s20 +0x20, segmentCount
+0x44, doubled +0x3C, frameCount +0x48; target +0x34, +0x30, +0x38, +0x2C,
+0x3C), and the scheduling that follows from them (andi a1,ra,3 and the
zero of i after the first call, li a0,255 in the tail). Everything after
the first call is otherwise exact. The rest of the function is done; the
whole residual is the pre-call block's split pieces and cell order.

The same two dead copies, the same seven-cell spill run and the same
pre-call schedule are in Jet Force Gemini's wakeAllocate (its retail
listing in the reference tree, read only; JFG calls mmAlloc2 and rounds
with cvt.w.s), so the shape is the engine's common source, not a Mickey
edit.

Measured flat or worse, each a product ranked aligned:
- or-with-zero kills (the overlay17CreateChain lever) on segmentCount,
  groupCount, triCount and frameCount after the allocation (16 cells) and
  after the alpha branch (27 cells): 323 or worse; segmentCount killed
  after the call is delta 0 but sends it to a memory home (246, aligned
  192, frame 0x88).
- a block boundary before the call (do-while, if (1), a goto label, the
  size through i, the byte products in a do-while): 7 cells, all
  byte-identical.
- copies of segmentCount/groupCount into i and j before or after the alpha
  branch, used by the byte products, with kills before or after the call
  (one product): 323 at best, delta-0 cells at 228 with i in v1.
- sampleBytes, segmentBytes and textureBytes inline or as locals, with
  segmentCount * 0x14 through a local (16 cells): flat (segmentBytes
  inline +8, textureBytes inline 211 aligned).
- post-call copies n = segmentCount and m = groupCount defined before the
  call, killed after it, read by wake->segmentCount and the fill-loop
  bounds (two new locals): 325 or worse (frame cells, immediate 21).

Reading. The target's cell run follows the allocation argument's term
order (frameCount, segmentCount, sampleBytes, segmentCount * 0x14, the
doubled count, segmentBytes, textureBytes), which is the order uopt would
number those expressions if it first met them in the argument. Ours meets
four of them at their statements in block 0.

Cycle-21 line: the wakeUpdate match came from the split rule (a web splits
iff totalsave <= bestcost), so read the split decisions here the same way:
web_report --proc 9 for w10 (segmentCount, totalsave 5 against bestcost
16.25, split, piece seeded at bb0 grows into bb2 at left 16 to 11) and w18
(the doubled count, totalsave 65 against bestcost 3, coloured t5). The
target needs both to have a bb2-only piece coloured before the
sampleBytes/s20 pieces; find the source that makes w10's bb0 piece stop at
bb1 (its colour unavailable in bb2) and gives w18 a split. Decision
variable: w18's totalsave against its bestcost on proc 9.

#### 2026-10-08, lane k-6: the two dead copies are XOR-kept copies, -8 to size 0

Measured by tools/bank.py: masked 60 (raw 60), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 325, register naming 3, immediate only 14, really different 13.

Score 323 masked at -8 to 60 masked at size delta 0. Aligned before:
byte-exact 311, naming 10, immediate 6, really different 29 (12 one-sided
words); after: byte-exact 325, naming 3, immediate 14, really different 13
(4 candidate-only, 4 target-only, all in the tail schedule at +0x2D4..+0x36C).

What it is. The target's two dead moves before the first call are copy
webs, not split pieces of the segment-count or doubled-count webs: a symbol
assigned from each count in the block after the alpha branch, read by that
block's byte products, and dead at the call. as1 forwards the products back
to t0 and t5 and leaves the moves. The source that makes them: after the
alpha branch, `j = segmentCount; j ^= 0; i = groupCount; i ^= 0;`, with
sampleBytes and the segment-times-0x14 term computed from j, segmentBytes
and textureBytes from i, and the post-call cursor add reading `j * 0x14`
(j is not redefined until the fill loops). Both copies then take v0/v1 in
that block before sampleBytes and the 0x14 term are coloured, so those two
take a2/a3, segmentCount's piece takes t0 and frameCount's piece s0, exactly
the colours j-6 had to force (w14=c5, w324=c6, w10=c7, w5=c14).

Measured on the way (each a shape_product, ranked aligned):
- plain copies into i/j (no kill), before or after the alpha branch: 325,
  propagated (5 cells, as j-6 recorded).
- the kill form matters: OR-zero, `&= ~0` and `+= 0` are folded and the copy
  is propagated (325 at -8); only `^= 0` keeps the copy web (32 cells).
- one copy alone (the group count in i or j): 292 at -4, one dead move.
- the segment copy in i instead of j: i is redefined by the vertex loop
  before the cursor adds, so the 0x14 term splits (328 at -12 and worse).
- copies placed before the alpha branch: uopt hoists them into block 0.
- kill literal type (0, 0U, (s16) 0, (u8) 0, `x = x ^ 0`), 25 cells: flat 60.
- statement order of the two copy groups (4), the three byte products (3)
  and the call argument's first two terms (3), 36 cells: flat 60.

Left (60 masked, aligned residual 30): the two copies take v1 (segment) and
v0 (doubled) where the target has v0 and v1. Records (proc 9): both copy
webs have save 3.0, nocs 1, and tie; the doubled copy is web 300 and the
segment copy web 326, so the doubled one is decided first. The pre-call
spill cells are still in the old order (ours frameCount 0x48, segment 0x44,
doubled 0x40, segment bytes 0x2C, texture bytes 0x28, sampleBytes 0x24,
the 0x14 term 0x20; target 0x3C, 0x38, 0x2C, 0x28, 0x24, 0x34, 0x30), and
the tail schedule at +0x2D4 follows the cells. One sampleBytes + 0x14-term
addu has its operands reversed.

#### 2026-10-08, lane k-6 (continued): v0/v1 order, 60 to 57

Measured by tools/bank.py: masked 57 (raw 57), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 328, register naming 0, immediate only 14, really different 13.

Measured by tools/bank.py at the end of this section. Aligned before:
byte-exact 325, naming 3, immediate 14, really different 13; after: 328,
0, 14, 13 (the same 4 candidate-only and 4 target-only tail words).

- The sample-byte term written inline in the allocation argument and the
  cursor add (`j * 0x10`, sampleBytes still declared): the
  sampleBytes-plus-0x14-term addu takes the target's operand order (60 to
  59). Deleting the now-unused sampleBytes declaration moves the cells (71);
  keep it.
- The frame and segment counts written as the expression at each use
  instead of through frameCount and segmentCount (both still declared): the
  segment copy's web is then numbered 26 against the doubled copy's 288, so
  it is coloured first and takes v0 as shipped (59 to 57, naming 2 to 0).
  Either one inline alone stays at 59 (4-cell product).

Measured flat: kill literal types and forms on the copies (25 cells);
statement order of the copies, byte products and the argument's first two
terms (36 cells); a separate 0x14-term local (fixes the operand order but
adds a frame cell, 79); the alpha branch moved above the counts (347 at -20
and worse); the segment count carried to the post-call store in j (65, the
segment copy takes t0 and segmentCount's piece v1); segmentCount = i copied
from a count computed in i (285 at -4, segmentCount becomes memory class at
its declared home); count spellings `frameCount ^= 0`, two-statement shift,
`/ 2`, `<< 1`, `seg + seg` (18 cells, flat 59).

Reading for the cells (web_report on the 57 shape, proc 9): spill homes are
handed out in web-number order. Ours: cvt 5, segment 7, doubled 8, sample
term 40, 0x14 term 41, the four unroller autos 174-255, segment-bytes 289,
texture-bytes 308, which is exactly our cell ladder from 0x48 down. The
target ladder from 0x3C down is cvt, segment, sample term, 0x14 term,
doubled, segment-bytes, texture-bytes, with the free cells above it: every
pre-call spilled web numbered after the unroller autos, in the order a walk
of the allocation argument meets them. In ours only the copies of the
group count (substituted late, after unrolling, because groupCount is a
loop bound) are numbered that late.

#### 2026-10-08, lane k-6 (resumed): the spill-home law, read from uopt's slot trace

Measured by tools/bank.py: masked 57 (raw 57), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 328, register naming 0, immediate only 14, really different 13.

Score unchanged at 57 masked, size delta 0 (byte-exact 328, naming 0,
immediate 14, really different 13). The 57 body is kept.

Law (spill homes). uopt's spilltemps hands out pre-call spill homes in
ascending web number. Each web either reuses an earlier slot whose
occupants it does not interfere with, or takes a fresh 4-byte slot below
the previous one. Read with the slot-tracing uopt (DKWB_UOPT_SLOT_TRACE,
its .text identical to stock on this TU), proc 9 on the 57 body:
- cvt (web 5) at -72, segment (7) at -76, doubled (8) at -80, sample term
  (40) at -84, 0x14 term (41) at -88: each one is fresh.
- the vertex-buffer loop's index temporaries (i + 1) (67), (i + 2) (191)
  and (i + 3) (195): fresh at -92, -96 and -100. They interfere with all
  five webs above, so they cannot reuse any of those slots.
- $v0[i * 4] (287): fresh at -104. segment-bytes (289) at -108,
  texture-bytes (308) at -112. Everything else reuses a slot.
This is exactly the cell ladder of the object (-72 is 0x48). The target
ladder is three free cells (0x48 to 0x40), then cvt, segment, sample term,
0x14 term, doubled, segment-bytes and texture-bytes. In web-number terms,
the three loop temporaries are numbered before every pre-call spilled
web, and the pre-call webs are numbered in the order a walk of the
allocation argument meets them. Read it with web_report --proc 9 and check
it with frame_census.

Measured flat or worse this pass:
- The count definitions moved below the alpha branch, below the copies,
  or post-call before the fill loops, with the doubled copy from
  groupCount, from the expression, or from triCount (12 cells). Below the
  branch: 347 at -20. Post-call: 349 at -28. bb0 has to compute them.
- The cvt rooted through a reused parameter (`wakeValue88 *= 60.0f`, or
  assigned): 310 at +20. Through `frameCount = wakeValue88 * 60.0f`: 59.
- The post-call segment store reading j (65). The segment web then no
  longer spans the call; it keeps a dead slot at -76.

Flag: the two `^= 0` keep-alives are stand-ins. They reproduce the target's
dead v0/v1 copies but are not the author's source.

Cycle-21 line: the decision variable is web-creation order. The pre-call
expression webs need numbers above the vertex-buffer loop's (i + 2)/(i + 3)
temporaries (191/195 here); they are 5 to 41 now. Only values reached
through a late substitution of a loop-bound symbol (the group-count copy,
288) are numbered that late. Find the source that delays creating the cvt
chain the same way while bb0 still computes it.

#### 2026-10-08, lane m-1: which symbols uopt keeps, read from the target (57 kept)

Measured by tools/bank.py: masked 57 (raw 57), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 328, register naming 0, immediate only 14, really different 13.

Score unchanged at 57 masked, size delta 0 (aligned byte-exact 328, naming
0, immediate 14, really different 13). No source change adopted.

Reading of the target's block 0. The trunc result goes straight into s0
(frameCount), and the segment count is copied from its expression into t0
(`segmentCount`, spilled at 0x38), beside the two fill-loop bounds copied
into t2 and t4 (groupCount and triCount, homes 0x6C and 0x60), alpha in ra
and bufferCount = 2 held in s1 across both calls. uopt keeps a symbol web
apart from its defining expression like this for loop bounds (alpha,
bufferCount, groupCount, triCount all are), so the target also keeps
frameCount and segmentCount as symbols.

Diagnostic (not adoptable, +120 bytes): segmentCount declared, assigned in
bb0, the copy `j = segmentCount; j ^= 0`, and a dummy loop bounded by
segmentCount. web_report --proc 9: the copy and both terms read from it
move from webs 26, 40, 41 to 357, 358, 363 (numbered with the late
substitutions, after the unroller's autos), while cvt, the segment
expression and the doubled expression keep 5, 7 and 11 because bb0's
definitions still create them first. So the dispatch's mechanism is
confirmed on the terms, and refuted as a full answer for cvt and the
segment value: those also need their defining expressions to be first
created late, which a loop bound alone does not do.

Measured worse: the 0x14 term (and the sample term) written inside the
sample-buffer loop's subscript so the hoisted add sits in the guard's
delay slot as shipped: 283 at +4 and 263 at +20.

Cycle-21 line: find the source that keeps frameCount and segmentCount as
symbols (the target's s0 and t0) without a loop; decision variable is the
first-creation web number of the cvt and segment expressions (5 and 7
now, must exceed the unroller autos 174-255); record web_report --proc 9
numbers and frame_census's ladder (target cvt at 0x3C, three free cells
above it).

#### 2026-10-08, lane p-3: the alpha-start store after the texture index, 57 to 26

Measured by tools/bank.py: masked 26 (raw 26), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 327, register naming 0, immediate only 14, really different 11.

On the 57 body. The residual has no naming rows (aligned 328/0/14/13), so no colour force prices it; the run used a satisfied oracle (p1:w48=c14, wake in s0, accepted, forced object identical to the base) and ranked by aligned residual only.

Measured by tools/lever_sweep.py (proc 9, every statement position, identity gate passed): 2,724 cells generated and measured: scored 2,469 (1,991 inert), size-skipped 87, compile errors 168; exact 0. One cell moved the tail: `wake->value4 = wakeValue80;` stored after `wake->textureIndex = ...` instead of before it (a reorder of two stores to different fields, no read between them), 26 at +0 (aligned 327/0/14/11, residual 25). The four candidate-only and four target-only tail words at +0x2D4..+0x36C fall to one candidate-only word at +0x2DC. Next best: empty tests and do-while wrappers on lines 734-746 (`if (segmentBytes) {}` after the allocation, 55 at +0, residual 25; `if (wake) {}` or a do-while at 743, 54). Kept the reorder.

Left: the 14 immediate rows are the pre-call spill ladder (target cvt 0x3C, segment 0x38, sample term 0x34, 0x14 term 0x30, doubled 0x2C, segment bytes 0x28, texture bytes 0x24; ours 0x48 down), unchanged by any cell, and 11 structural rows in the tail.

#### 2026-10-08, lane p-3 (second pass): statement order of the stores, 26 to 16, only the spill ladder left

Measured by tools/bank.py: masked 16 (raw 16), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 335, register naming 0, immediate only 16, really different 0.

On the 26 body. Two store groups set the two remaining schedule windows (as1 reads statement order there):

- The two NULL sample-buffer stores at eight positions in the cursor sequence (fast_score, aligned residual): after `cursor += j * 0x10` or after `wake->samples = cursor`, 22 at +0 (residual 21); where they were, after the vertex pointer, or after the 0x14 add, 26; at the cursor start or after the alpha loop, 72 to 74. Kept: after `wake->samples = cursor`.
- Every order of the five post-link stores (state, segment count, value8, value3C, alpha start) with the texture-index store at each of six positions, 720 cells on that body (fast_score): segment count, state, texture index, value8, value3C, alpha start is one of three cells at 16 at +0 (aligned 335/0/16/0, residual 16); the next are 18. Kept.

A second lever_sweep on the 26 body (proc 9, the same satisfied oracle p1:w48=c14; 2,743 cells generated and measured: scored 2,481 (2,000 inert), size-skipped 87, compile errors 175; exact 0): best `if (segmentBytes) {}` after the allocation at line 734, 24 at +0 (immediate 12, structural 11), then the reorder at 745, 25. The empty test is the only cell in either sweep that moves the ladder (immediate 14 to 12); not adopted, it was measured on the old store order.

Left: 16 immediate rows and nothing else (no naming, no structural). The whole residual is the pre-call spill ladder: target cvt 0x3C, segment 0x38, sample term 0x34, 0x14 term 0x30, doubled 0x2C, segment bytes 0x28, texture bytes 0x24 under three free cells; ours cvt 0x48, segment 0x44, doubled 0x40, sample term 0x3C, 0x14 term 0x38, segment bytes 0x24, texture bytes 0x20.

#### 2026-10-08, lane p-3 (third pass): the ladder under the lever catalogue, 16 kept

Measured by tools/bank.py: masked 16 (raw 16), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 335, register naming 0, immediate only 16, really different 0.

On the 16 body (aligned 335/0/16/0; the residual is only the pre-call spill ladder). No source change adopted.

- `if (segmentBytes) {}` after the alpha (sample-buffer) loop, carried over from the second sweep: 14 at +0 (immediate 14). It moves only segment bytes from 0x24 to the target's 0x28 by lengthening its range; every other cell keeps its slot. A stand-in, not adopted.
- tools/lever_sweep.py stacked on that cell (--candidate, proc 9, satisfied oracle p1:w48=c14; 2,809 cells generated and measured: scored 2,536 (2,049 inert), size-skipped 87, compile errors 186; exact 0): floor 15 (assigned dead reads of a fill-loop element into i, or `if (i) {}`, at line 759); nothing goes below the base 14. No catalogue lever at any position changes the creation order of the cvt, segment, doubled, sample and 0x14 webs.

Cycle-21 line: the decision variable is unchanged from k-6 and m-1: the first-creation web numbers of the cvt, segment, sample-term, 0x14-term and doubled webs (5, 7, 8, 40, 41 on proc 9) must exceed the vertex-buffer loop's unroller temporaries (67, 191, 195), and the doubled web must follow the 0x14 term. Record: web_report --proc 9 plus frame_census. No CDX force prices a spill slot, so the next instrument is a CDX knob that renumbers webs (or a slot-order override in spilltemps) to confirm the ladder alone closes the function, then a source search for a late creation of those expressions (k-6: only loop-bound symbols are substituted after unrolling).

#### 2026-10-08, lane q-1: five measurements on the spill ladder, 16 kept

Measured by tools/bank.py: masked 16 (raw 16), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 335, register naming 0, immediate only 16, really different 0.

On the 16 body (aligned 335/0/16/0). No source change adopted.

The slot trace (DKWB_UOPT_SLOT_TRACE, proc 9) on the 16 body gives the
spill requests in ascending web number: cvt 5, segment 7, doubled 8,
sample term 40, 0x14 term 41 take fresh slots -72 to -88, (i + 1) 67,
(i + 2) 191 and (i + 3) 195 take -92 to -100, $v0[(i * 4)] 287 -104,
segment bytes 289 -108, texture bytes 308 -112. The target's ladder is
three unused cells (the loop temporaries), then cvt, segment, sample term,
0x14 term, doubled, segment bytes and texture bytes, with 0x20 unused
($v0[(i * 4)] after them). So the order to reach is: the three loop
temporaries, cvt, segment, sample term, 0x14 term, doubled, segment bytes,
texture bytes, then $v0[(i * 4)].

- lever_sweep with `--levers split_local,merge_locals,reorder,loop_move,
  zero_def,const_iv` (oracle `p1:w48=c14`, satisfied): 121 cells, all
  measured, exact 0, best 18 (a reorder at line 705).
- Natural count product (288 cells, shape_product): frameCount and
  segmentCount as declared symbols or inline expressions, groupCount and
  triCount from `segmentCount * 2`, from each other or in full, the i copy
  from groupCount, `segmentCount * 2` or triCount, and both post-call
  stores through the symbols or inline: flat at 16. With frameCount and
  segmentCount as symbols (the most natural cell, 18) the sample and 0x14
  terms are numbered late (318, 325) because `j = segmentCount` is
  substituted after unrolling, but cvt, segment and doubled stay 5, 10
  and 14.
- Diagnostic, not adoptable (+240 bytes): frameCount and segmentCount as
  the bounds of two dummy loops. cvt (5) and segment (10) are still the
  spilled webs, so a loop bound alone does not delay their creation (as
  m-1 found for segmentCount).
- Assigned dead reads of the loop temporaries before the counts: `j = i +
  1;` alone renumbers (i + 1) to web 4, which then takes the first slot
  (-72) and pushes cvt to -76, but the masked count stays 16; reads of
  (i + 2) and (i + 3) change the unrolled loops' colours (30 masked).
- The counts and the allocation wrapped in `do { } while (0)` or
  `for (;;) { break; }`: byte-identical when the wrapper closes after the
  call, 361 at +60 when it closes before it.

Cycle-21 line: the decision variable is still the itable first-occurrence
order (web numbers 5/7/8/40/41 against the loop temporaries 67/191/195).
Every source expression in block 0 is numbered before loop A's (i + 1),
so the target's spilled webs must be values uopt creates after unrolling,
not the source's own block-0 expressions. Next: dump `uoptlist`
(`-Wo,-zdbug:2`) on the 16 body and list which passes create expressions
after the unroller (late substitution, hoisting, rematerialisation), then
look for the one that can produce the whole allocation argument in walk
order (cvt, segment, sample, 0x14, doubled, segment bytes, texture bytes).

#### 2026-10-09, lane s-2: uoptlist reading of the web numbers, and a float local (no change, 16)

Measured by tools/bank.py: masked 16 (raw 16), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 335, register naming 0, immediate only 16, really different 0.

Base re-measured: 16 masked at size 0, aligned 335/0/16/0.

uoptlist (`-Wo,-zdbug:2`, direct cc with the TU's flags in scratch; the
file is never committed). The expression table printed after REMOVAL OF
REDUNDANT STORES is numbered exactly as web_report numbers the webs: cvt
bit 5, segment 7, doubled 8, sample term 40, 0x14 term 41, (i + 1) 67,
(i + 2) 191, (i + 3) 195, $v0[i * 4] 287, segment bytes 289, texture
bytes 308. So a web number is the expression's index in that table. The
table is built in IR order after the unroller (the unrolled loop bodies
sit inline at 176-202), with forward substitution inside each block done
while it is built. Entries after the last original statement (287 on)
are created by global copy propagation across blocks: 287 is the
call-result substitution of `wake`, 288 is `i ^ 0` with groupCount's
block-0 expression substituted, and 289/308 are built on it. That is why
segment bytes and texture bytes are late here, and why q-1's
frameCount/segmentCount-symbol cells made the sample and 0x14 terms late.

Consequence: the target's order (cvt, segment, sample, 0x14, doubled,
segment bytes, texture bytes, all after 195) is exactly the order a
recursive substitution into the allocation argument would create them
in, which says all seven are created by cross-block substitution. cvt,
segment and doubled are computed before the alpha branch (block 0) in the
target, and nothing is substituted into block 0, so the open question is
which source puts their first IR occurrence outside block 0 while ugen
still emits them before the branch.

Measured flat: a float local for the frame count (`frames = wakeValue88
* 60.0f;`, in the unused sampleBytes cell), read as `(s32) frames` or
through frameCount, with segmentCount as a symbol or inline (6 cells):
16 for the tree cell, 18 for the rest. The table shows why: the
substitution happens inside block 0 while the table is built, so cvt
stays bit 8.

Cycle-21 line: find a source whose first IR occurrence of the frame-count
conversion is outside block 0 (the decision variable is the creation
index of cvt, segment and doubled against 195); read uoptlist's table
after each candidate rather than the score.

#### 2026-10-09, lane t-2: natural counts and declaration order (no change, 16)

Measured by tools/bank.py: masked 16 (raw 16), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 335, register naming 0, immediate only 16, really different 0.

Base re-measured: 16 masked at size 0, aligned 335/0/16/0.

- Natural count product (tools/shape_product.py, 24 cells): frameCount
  and segmentCount as symbols, groupCount and triCount from
  `segmentCount * 2` or from each other, the alpha default before or
  after the counts, sizes through segmentCount or through plain j/i
  copies without the XOR. Without the XOR the two dead pre-call copies
  go: 325 at -8 (aligned residual 37); sizes through the symbols 331 at
  -4; alpha first 350 at -16 or worse.
- frameCount and segmentCount symbols with the XOR copies kept: 18 at 0
  (aligned 333/2/16/0). Declaring frameCount, segmentCount, sampleBytes,
  a 0x14-term local, segmentBytes and textureBytes after twelve pads so
  their declared cells would sit at the target's 0x3C to 0x24: 40 at 0,
  and the spilled segment and doubled values still go to spill cells
  (0x44, 0x40), not to their declared homes. A symbol spilled across the
  call is homed in the spill area, so declaration order is not the lever
  for this ladder.
- The stock compiler's uopt aborts on `-Wo,-zdbug:2` here (float printer
  assertion); the instrumented cc writes uoptlist.

Cycle-21 line unchanged from lane s-2.

#### 2026-10-09, lane v-2: where the conversion is first created, read from uoptlist (no change, 16)

Measured by tools/bank.py: masked 16 (raw 16), size delta +0, candidate 351 words vs target 351. Aligned: byte-exact 335, register naming 0, immediate only 16, really different 0.

No source change; the 16 body is kept (16 masked at size 0, aligned 335/0/16/0).

Method: whole fx.c compiled with the instrumented cc and `-Wo,-zdbug:2`
(the TU's own flags, `-Wab,-r4300_mul` included), the expression table
printed after REMOVAL OF REDUNDANT STORES read for the bit of
`ucvt(umpy(wakeValue88, 60.0))`; every cell also scored with fast_score.
uoptlist and the objects stay in private scratch.

- Base: the conversion is bit 5 (block 0), as s-2 recorded.
- A float local `frames = wakeValue88 * 60.0f;` in block 0 with every
  integer count after the alpha branch (frames read as `(s32) frames`), or
  with frames itself assigned after the branch: the conversion is created
  in the call block at bit 19, still ahead of the vertex loop. Size moves
  (target-only words at +0x50).
- frameCount, segmentCount, groupCount and triCount as symbols defined in
  block 0 from each other, with nothing, `if (1) {}`, `do {} while (0);` or
  a label between the frameCount definition and the rest (4 cells): the
  conversion stays bit 5 (it is in frameCount's own statement); 18 masked at
  0 for all four (aligned 333/2/16/0).
- `frames` (f32) assigned in block 0, then `frameCount = frames;` behind the
  same four separators (4 cells): `if (1) {}` and `do {} while (0);` do stop
  block-local forward substitution (the table then holds `ucvt(frames)` at
  bit 8 and the full conversion is first created at bit 112, at the later
  explicit `(s32) (wakeValue88 * 60.0f)`), but the float symbol is not copy
  propagated afterwards; 40 masked at 0 (immediate 38). A label and nothing
  leave it at bit 8 as a substitution. So a region boundary is a working
  way to keep an expression out of block 0's table, but not with a float
  carrier.
- The doubled count spelled `* 2`, `<< 1` or `x + x` in block 0, crossed
  with the doubled copy taken from groupCount or rebuilt from j as
  `j * 2`, `j << 1`, `j + j` (12 cells, shape_product): `* 2` and `<< 1`
  identical (16); `x + x` 17; every rebuilt copy 55 to 56 at 0.
- An integer carrier behind the same separators (`frameCount = wakeValue88
  * 60.0f;`, then `do {} while (0);` or `if (1) {}`, then the counts from
  `(frameCount + 5) >> 1`; 3 cells with the no-separator control): the
  separator keeps block 0b's table entries over the frameCount symbol
  (`frameCount + 5` at bit 9) and the full segment expression is first
  created at bit 106 by the later explicit spelling, but frameCount is not
  copy propagated into them either, and the conversion stays bit 5 in its
  own statement. 18 masked at 0 for all three (aligned 333/2/16/0); the
  ladder (immediate 16) does not move.

Cycle-21 line: decision variable unchanged (first-creation index of the
conversion, segment and doubled expressions against the vertex loop's
unrolled (i + 2)/(i + 3) at 191/195). What is now ruled out: any carrier
(float or integer symbol, with or without a region boundary) leaves the
conversion in its own block-0 statement, and copy propagation substitutes
neither carrier. So the conversion's bit is fixed by the first statement
that converts, and the target's late bit needs that statement to come after
the vertex loop in IR order. Next: read which pass reorders or re-enters
expressions between the unroller and REMOVAL OF REDUNDANT STORES (the
table is printed after it) on the target-shaped question "can a block-0
statement be entered after the loops", e.g. with the slot-trace uopt on a
mini TU that defines a value before a call and a counted loop after it.
<!-- plateau-handoff:wakeAllocate:end -->
