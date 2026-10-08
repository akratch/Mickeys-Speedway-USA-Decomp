<!-- plateau-handoff:wakeAllocate:start -->
### `wakeAllocate` plateau handoff

- source: `src/main/fx.c`
- score: 57/351 words
- frame: 0x90
- relocations: 3
- first mismatch: +0x94
- summary: XOR-kept copies, counts inline: 323 at -8 to 57. Left: spill homes follow web number; pre-call webs must number after the loop temps

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
<!-- plateau-handoff:wakeAllocate:end -->
