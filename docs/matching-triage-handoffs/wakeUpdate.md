<!-- plateau-handoff:wakeUpdate:start -->
### `wakeUpdate` plateau handoff

- source: `src/main/fx.c`
- score: 170/398 words
- frame: 0x90
- relocations: 2
- first mismatch: +0xC0
- summary: Aligned 125 to 104 (outputCount per vertex, height in value); 170 positional is one as1-hoisted zero init. Left: constant 20 in a0, stripIndex probes

Summary before this remeasure: Mark bit OR-assigned through the stored byte: ring lines up, 195 to 132 at 0. Left: constant 20 in a0 (target a2), loop t3/t5, stripIndex probes

Summary before this remeasure: Mark bit OR-assigned through the stored byte: ring lines up, 195 to 132 at 0. Left: loop t3/t5, outputCount/polygonOffset, stripIndex probes

Summary before this remeasure: Secondary cursor per vertex and polyCount per triangle replace two OR-zero probes (byte-identical, 195 at 0). Left: the three stripIndex probes.

Summary before this remeasure: OR-zero weight probes order secondaryVertices over wake and stripIndex over index (253 to 198 at 0). Left: polyCount/outputCount over polygonOffset.

Summary before this remeasure: Second-loop wrap test as ++index (257 to 253, delta 0). Left: p1 order wake/secondaryVertices, index/stripIndex, polygonOffset over polyCount.

Summary before this remeasure: Size delta 0, frame 0x90; residual is p1 colour order (wake/secondaryVertices, index/stripIndex swapped); four colour forces price it at 207.

Summary before this remeasure: JFG efd5abb remains assembly-only; zero source attempts. Need new counter lifetime and trig-call schedule evidence.

Track B lane B3-fx (2026-09-23), about twenty-five measured cycles. Start
367 masked, size delta -4, frame 0x98 against 0x90; insertion reader: five
pairs, aligned residual 310. The retained body was rewritten against the
target; each step with its measured effect:

- Two semantic errors in the old body, read off the target's stores: the
  value stored at vertices-record +0xE is a separate counter (polyCount,
  +2 inside the stripIndex branch, reset with outputCount), not the sample's
  shifted height; and stripIndex advances by 2 per sample (polygon[1] is
  stripIndex, [2] and [0x11] are stripIndex + 1) with polygon[0x12] also
  written in the branch. The shifted height is a plain temporary.
- index as `s32` (the target never masks it), `u16` reads of the linked
  counts, value4 stored without a cast. First version 385 at -12.
- First scan as `count = value3B; while (count--)` with a `start` value for
  the entry and post-scan reads: the target's copy-and-test shape and its
  PRE'd reload. 385 -> 369.
- `index * 5` through a temporary, then `* 4`; sine and cosine as a call
  followed by `*=`. Declarations merged to sixteen slots so the frame is
  0x90 with outputOffset on the target's +0x80 home. 369 -> 335.
- Sample stores in the order angle, +4, arg2, arg1, arg3: 335 -> 329 (best
  of all 120 orders).
- The +0x20 buffer read written as a subscript while +0x18 and +0x28 stay
  pointer arithmetic: the target computes wake + state * 4 twice, and a
  different spelling is what keeps uopt from sharing it. Delta -8 -> 0,
  329 -> 257. Spelling it `<< 2` or `* 4U` measures the same; moving the
  constant after the index does not, and writing all three as subscripts
  shares all three again (329, -8).

Residual at delta 0: byte-exact 146, register naming 232, immediate 1,
really different 23 (four one-sided words each way, all scheduling). The
naming is the p1 colour order, read from the globalcolor records (proc 13):
wake (save 21.86, nocs 14) outranks secondaryVertices (20.17) and takes s3
where the target has s4; the index web (15.38) outranks stripIndex (10.09)
and takes s5 where the target has s6; and polygonOffset (5.17) takes s7,
which the target gives to polyCount, spilling polygonOffset to +0x6C around
the calls instead of our +0x70. Forcing w162=c17 and w2=c18 gives 222;
adding w51=c19 and w12=c20 gives 207, still delta 0. Splitting the
polygonOffset web instead of colouring it moved the size to +8 (296), so
the target's polygonOffset is a caller-saved web that spills, not a split.

Measured flat: a separate symbol for the first scan's cursor (value, mark
or outputOffset: 266, 259, 308), all three buffer reads as subscripts.

Decision variable: the save ratio of the wake parameter against
secondaryVertices, and of the two index webs against stripIndex. The next
lever is whatever lowers wake's totalsave by about two per reference block
(fewer wake reads inside the sample loop) without moving the code.


Reopening audit (2026-09-08), evidence D: PROVENANCE inspection of Jet Force
Gemini public decomp `src/fx.c` and `src/fx.h` at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no new target C body.
No donor source, names or values were adopted. Mickey remains authoritative.

Configured full-TU measurement: candidate 1584 bytes / 396 words,
target 1592 bytes / 398 words; size delta -8 bytes.
Raw and relocation-masked differences are 334 and 334, respectively,
first mismatch +0x0; candidate/target frames are
0x98/0x90. Candidate/target static relocation counts are
2/2; 1 tuples agree in function-relative offset, type and symbol.
These are fallback-object comparisons, not linked-C promotion evidence.

Workbench comparison: `structure-mismatch`. Diagnosis: `mixed(constant:3, structural:165, register:200)`;
playbook `constant-audit`, lever `stack-home`. The named guides were read.
This heuristic routing supplies no new donor evidence to reopen exhausted forms.
Next concrete lever requires new wakeUpdate donor C exposing counter lifetime and the first trigonometric call schedule,
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

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read score 334 differing words.

#### 2026-10-02, lane g-objB: 257 to 253 at delta 0

The second loop's wrap test written `if (++index >= wake->segmentCount)`
with the sample address computed first: 257 to 253 (a 16-cell product
over that, the first scan's increment and a while form of the second loop;
only this axis moved).

Measured worse: a separate variable for the first scan's index (273 in
every declaration position). It lowers the second-loop index web to save
10.33 (62/6), still above stripIndex's 10.09 (111/11), and moves the first
scan onto v0 where the target keeps s6, so the target's index is one web
across both loops with a lower ratio than ours (15.38). polygonOffset
written at each use instead of a local is not hoisted (338, +20).

Decision records (proc 13): the three open orders are wake (306/14)
over secondaryVertices (121/6), index (123/8) over stripIndex (111/11), and
polygonOffset (31/6) over polyCount and outputCount (42/12 each). Every
emitted reference count matches the target, so the extra weight is in IR
occurrences the listing does not show; the per-web detail records give no
occurrence list.
Lane x-fx, 2026-10-02 (not banked: 302 > 253 on the masked count). A typed
rewrite from the listing reaches size delta 0 with only 24 structural rows
left (244 register-naming rows), so the inherited offset-cast shape is not
needed for structure. Three structural facts, each measured:
- the four vertex buffers are ONE array at +0x18 (vertex = buffers[state],
  secondary = buffers[state + 2]; triangles at +0x28). That is the target's
  doubled state*4 address (two sll/addu pairs); separate arrays CSE it.
  wakeAllocate's four-element fill at +0x18 agrees.
- the vertex cursor advances before the first vertex's stores (accessed as
  vertex[-1]) and again between the second vertex's alpha and y stores; one
  `vertex += 2` at the end costs four bytes and the schedule.
- the `started` flag declared fourth puts its call-spill home at sp+0x80.
Typed layout used: sample 0x14 (u8 time, u8 alpha, s16 angle, s16 v,
s16 y, f32 x, f32 z, f32 width), vertex 10 bytes (s16 x,y,z; u8 r,g,b,a),
triangle 0x10 (u8 flags, vi0, vi1, vi2; s16 u/v pairs), batch 0x10
(vertices, secondary, tris, s16 vertexCount, s16 triCount). Decision variable
reached: p1 colour order across the whole loop (wake s4/index s6/stripIndex
s5/triCount s7/vertexCount s8 in the target; the rewrite shifts each by one
slot), plus the polygonOffset spill home (0x6C target, 0x60 here). Not
reached: the records for that order.

#### 2026-10-05: hoisting the sample base grows the function

Configured full-TU baseline: 253 masked and 253 raw words, target 1592 bytes, size delta 0. Loading wake's sample pointer once before the sample loop, and advancing that local inside the loop, scores 333 masked words at size delta +4. Not kept. The 253-word body stays. Do not repeat this hoist.

## 2026-10-06: the first scan walks vertexCount

The unmodified body scores 1592 bytes, 253 raw and 253 masked words, size delta 0. The first sample scan advances vertexCount instead of index, so index exists only in the second loop. That scores 381 masked and 381 raw words at size delta -4. Not kept. The 253-word body stays. Do not repeat this scan.

#### 2026-10-07, lane a-front: reference-count probes, 253 to 198 at delta 0

Aligner after: byte-exact 205, register naming 173, immediate 2, really
different 22. Five measured cycles (products of 6, 18, 10 and 10 cells,
records on proc 13, instrumented .text identical to the stock object).

- One OR-with-zero probe of secondaryVertices inside the sample loop (L109;
  uopt deletes it, globalcolor counts it) raises its web to 141/6 (23.5)
  over wake's 306/14 (21.86): secondaryVertices s3, wake s4, as shipped.
  253 to 216. Position in the loop and a second probe are inert.
- Three probes of stripIndex at the loop end raise it to 171/11 (15.55)
  over index's 123/8 (15.38): stripIndex s5, index s6. 216 to 201; one
  polyCount probe with them 198.
- polyCount probes alone move only polyCount (to 122/12, s7). The target
  also needs outputCount (42/12, on a1 here) above polygonOffset (31/6) so
  outputCount takes fp and polygonOffset falls to a1 and spills: one to six
  outputCount probes do exactly that (fp, as shipped, aligned exact rows 199
  to 203) but measure 248 positionally, so they are not banked.

The probes are a measured stand-in, not the source: the target's author
reads these variables that often somewhere. Next: with the outputCount
probe applied, the residual is the pre-loop region (the second sample
pointer in v0 where ours keeps one s0 web for both regions, the mark flag in
v1, the first scan's count in a1) plus a one-register ring shift in the
counter block; give the pre-loop sample its own local and re-read the
records. Then replace each probe with a real reference.

#### 2026-10-07, lane a-front (resumed): 198 to 195 at delta 0

Aligner after: byte-exact 208, register naming 170, immediate 2, really
different 22. The strip-start mark is written through the expression,
((u8 *) wake->samples + (vertexCount * 0x14))[1] OR-assigned 0x80, so the pre-loop
sample local no longer spans it (198 to 195). With the outputCount probe
added this measures 248 positionally but 206 aligned exact rows (best
aligned); not banked.

Measured flat or worse (each a product): a separate cursor local for the
first scan (worse); moving the stripIndex, polyCount and outputCount zero
inits to just before the loop (flat or worse); mark as u8, as an assignment
from the compare, or with an else arm, and the first scan as count-- != 0
(u8 assign shifts the temp ring the other way: 262 aligned exact but +4).

Left: the pre-loop zero init our build places at +0xDC (a2) where the
target has none until +0x284, the mark/first-scan swap between v1 and a1,
and a one-register temp-ring shift through the counter block. Next: the
freelist trace (draw_census --proc 13) on the pre-loop lines, then replace
the probes with real references.

#### 2026-10-07, lane g-7: the secondary buffer through the +0x18 base, flat

One 6-cell product on the retained 195 body: the secondary pointer read
as `((u8 **)(wake + 0x18))[state + 2]` (x-fx's one-array reading) or as
the byte-offset form `+ 0x18 + (state + 2) * 4`, times the
secondaryVertices probe on or off. The retained spelling (the separate
+0x20 subscript) is the only delta-0 cell: 195 with the probe, 234
without. Both +0x18 spellings let uopt share the state*4 address with the
vertex read, -4 (288, 319) and -8 (289, 320). In this shape the doubled
address needs the two spellings to differ (B3-fx's finding stands); the
one-array reading is only consistent with x-fx's typed shape.

Cycle-21 line: unchanged from a-front (replace the four probes with real
references; decision variable: secondaryVertices' totalsave against
wake's, 141/6 needed against 306/14, and stripIndex's 171/11 against
index's 123/8, read on proc 13).


#### 2026-10-07, lane h-7: two of the four probes are real source now (195 kept)

Byte-identical object, 195 masked at delta 0, aligned byte-exact 208,
naming 170, immediate 2, really different 22. Records on proc 13 for the
probe-free form give the same ranking the probes gave:

- secondaryVertices written one vertex at a time, the cursor advanced by
  0xA after each vertex (stores at +0..+9, then the second vertex): uopt
  folds the two steps into the shipped single add, and the web reads
  141/6, over wake's 306/14, as shipped (s3 against s4). The three other
  spellings of the same two steps (advance first, both advances first)
  measure the same.
- polyCount advanced once per triangle (two increments in the strip
  branch): 62/12, ties polygonOffset's 31/6 and keeps s7 by web number.
- With neither probe and no stripIndex probes: 210 (the un-probed body
  was 250). With the three stripIndex probes kept: 195, the object
  identical to the four-probe body.

Measured, not adopted: the loop's vertexCount through its own local
(count 260, mark 251, value 245; worse than the shared web); stripIndex
advanced by two single increments after the strip stores (both forms
raise its web to 141/11 but rotate the temp ring, 217 alone and 204 with
two probes); the reset written as > 0x10 or as a conditional
expression (flat).

Cycle-21 line: the stripIndex probes. Decision variable: stripIndex's
totalsave on proc 13 (111/11 with no probe, 141/11 with the single-step
increments) against index's 123/8; it needs about 170. The single-step
increments carry half of it at the cost of a ring rotation; look for the
form that carries the rest without moving the draws (draw_census
--compare against this body).

#### 2026-10-07, lane i-6: the mark bit OR-assigned through the byte, 195 to 132 at delta 0

Measured by tools/bank.py: masked 132 (raw 132), size delta +0, candidate 398 words vs target 398. Aligned: byte-exact 277, register naming 101, immediate only 2, really different 22.

Aligner after: byte-exact 277, naming 101, immediate 2, really different
22 (before 208, 170, 2, 22). The new-sample block's mark store is
sample[1] = value, then sample[1] OR-assigned 0x80 inside the mark test. uopt forwards
the byte just stored, so `wake->value8 >> 1` stays a ring temporary (t8 in
the target, where ours had the `value` web in v0) and the forwarded byte
spends the one extra draw the target spends before the `ori` (t9 skipped,
the `ori` lands in t0): the whole temp ring from +0x1C0 to the end of the
pre-loop region lines up. This was the one-register ring shift earlier
lanes read as a counter-block phase.

Seven other spellings of the same statement group, one product: re-reading
(wake->value8 >> 1) OR 0x80 in the arm reloads the halfword (the byte
store may alias wake), +8; the `u8` cast and `& 0xFF` forms are the same
+8; `sample[1] = value = ...` is byte-identical to the old body. The (u8) value
OR 0x80 and (value & 0xFF) OR 0x80 arms compile to the same object
as the OR-assign (132).

Measured since, on the 132 body: the loop's vertexCount through its own
carrier (value: 182 positional but aligned residual 117 against 125, the
pre-loop vertexCount then lands in a2 against the target's a1 and the
count copy and mark take v1 as shipped; mark 188; count 197; a new local
281); the second loop's sample address as `index * 0x14` (+4, multu),
`(index * 5) << 2` or `* 4` (byte-identical).

Left: a t3/t5 swap in the loop (the target computes index * 5 into t3 and
shifts it in place after loading the sample base into t5), the
outputCount/polygonOffset pair (a1 and fp swapped), the first window's
vertexCount/count-copy pair, and the outputOffset zero init (+0x284 in the
target, ours in the branch delay slot at +0x298).

Cycle-21 line: draw_census --proc 13 on the second loop's head (lines of
the sample address) to find the draw that puts index * 5 in t4 here; then
the stripIndex probes (decision variable unchanged: stripIndex 171/11 with
probes against index 123/8).

#### 2026-10-07, lane i-6 (continued): outputCount per vertex, and the constant 20's colour

Measured by tools/bank.py: masked 132 (raw 132), size delta +0, candidate 398 words vs target 398. Aligned: byte-exact 277, register naming 101, immediate only 2, really different 22.

Not banked (positional rises; aligned falls). On the 132 body, measured:

  - outputCount advanced once per vertex (two single increments where the
    body has the += 2; uopt folds them into the shipped single add): the
    web reads 62/12 and ties polyCount and polygonOffset, so it takes fp
    and polygonOffset falls to a1 and spills around the calls, as shipped.
    The a1/fp cycle leaves the residual map. Alone 183 positional, aligned
    residual 119 (against 125).
  - with that and the loop's vertexCount carried by value (so the
    pre-loop vertexCount web no longer spans the loop): 170 positional,
    aligned byte-exact 298, naming 81, immediate 2, really different 21,
    residual 104. The pre-loop region then reads count copy v1, mark v1,
    vertexCount a1, outputCount fp, polygonOffset a1, all as shipped.
  - why positional rises: with mark off a2, a2 is free from the pre-loop
    zero group to the loop, and as1 hoists outputOffset's zero init into
    that group (+0xE0; the target keeps it at +0x284), a one-word shadow
    over 0xD0..0x284. In the target a2 is held there by the constant 20
    (the multiply at +0xC0 and +0x1B0), while ours colours that constant
    a0 for its whole range (web 326, decision 27, a0 the lowest free).
    Forcing it (p1:w326=c5, accepted) on that cell prices it: 170 to 108
    at delta 0, the best number this function has measured.

Flat: the constant 20 at the two pre-loop sites spelt (s16), (u8),
(u16), (s8) or L (cfe folds the cast, 6 cells, all 170); the 0x14U
literal at the first-scan or mark site (byte-identical); the second
loop's sample address as (value * 4) plus the base (135: index * 5 lands
in t3 as shipped but the shift draws t4 where the target shifts in
place), as an index into the base, or split over two statements (132 to
199).

Cycle-21 line: the constant 20. In the target the first scan's 20 is in
a0 and the mark and new-sample sites' 20 in a2, so it is two webs (or a
split) where ours is one web over bb2..bb25. Decision variable: web 326's
colour on proc 13 (a0 taken at decision 27 with a2 also free). Find the
spelling that makes the first-scan multiply a different constant web
(L131: the spellings must differ; casts on the literal are folded by
cfe), then adopt the outputCount and vertexCount edits above with it.
Then the stripIndex probes.

#### 2026-10-07, lane i-6 (resumed): stride spellings flat, aligned edits adopted

Measured by tools/bank.py: masked 170 (raw 170), size delta +0, candidate 398 words vs target 398. Aligned: byte-exact 298, register naming 81, immediate only 2, really different 21.

Adopted on the coordinator's instruction, ranked aligned: outputCount
advanced once per vertex and the loop's strip height carried by value.
132 to 170 positional at delta 0, aligned byte-exact 277 to 298, naming
101 to 81, really different 22 to 21 (residual 125 to 104). The whole
positional rise is one shadow: a2 is free through the pre-loop region, so
as1 hoists outputOffset's zero init to +0xE0 (target +0x284).

Measured flat on this shape (one 20-cell product, ranked aligned): the
first scan's stride as 0x14U, (5 * 4), sizeof of a 20-byte struct, or a
typed struct subscript is byte-identical every time (uopt makes one
constant 20 web); at the mark and new-sample sites the struct subscript,
sizeof or 0x14U forms add 4 bytes (the unsigned multiply at the
new-sample site). A byte cursor stepping 20 is ruled out by the target,
which multiplies inside the first scan (multu with the constant in a0).
Forcing web 326 to split (p1:w326=s, accepted) sends it to memory, 349 at
-4; forcing it whole to a2 (c5) is 108. Probes: removing the three
stripIndex probes costs 19 (189 against 170); stripIndex advanced as two
single increments reads 121/11 against index 123/8 on proc 13, so it
still needs about five more loop references; the strip test as > 0 is
flat.

Cycle-21 line: the constant 20's colour (web 326, a0 at decision 27; the
target has a0 in the first scan and a2 at the mark and new-sample sites,
which needs two webs, and no literal spelling makes two). Next look: what
holds a0 across bb9..bb25 in the target (an argument-register web or a
call), then the stripIndex references.
<!-- plateau-handoff:wakeUpdate:end -->
