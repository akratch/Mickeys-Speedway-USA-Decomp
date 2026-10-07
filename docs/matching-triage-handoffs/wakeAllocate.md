<!-- plateau-handoff:wakeAllocate:start -->
### `wakeAllocate` plateau handoff

- source: `src/main/fx.c`
- score: 325/351 words
- frame: 0x90
- relocations: 3
- first mismatch: +0x10
- summary: Divisors read wake->textureIndex back (store forwarded): tail temp ring aligned, naming 84 to 9 at -8. Left: dead v0/v1 copies before the call.

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

<!-- plateau-handoff:wakeAllocate:end -->
