<!-- plateau-handoff:func_overlay_073_F0000190_18CAC50:start -->
### `func_overlay_073_F0000190_18CAC50` plateau handoff

- source: `src/overlays/o073/func_overlay_073_F0000190_18CAC50.c`
- score: 4/760 words
- frame: 0x98
- relocations: 46
- first mismatch: +0x7C8
- summary: 4 masked at size 0, case 4 query: one ring temp drawn and released between the two narrowings with nothing emitted; no test or argument spelling draws it.

Summary before this remeasure: 4 masked at size 0, all naming in the case 4 query: hitIndex - 1 is an a1 web in the target with one extra folded ring draw; ours is a ring temp.

Summary before this remeasure: 14 masked at size 0; the case 0 timer block reserves the extra temp that puts the spill at +0x30; case 4 query index web (8 words).

Summary before this remeasure: 153 masked at size 0; target is two ring draws ahead from the case 1 angle difference; float-rate spill at +0x30 not +0x34.

Summary before this remeasure: 163 masked at size 0 (was 740 at +8); ring two draws behind from the case 1 angle difference, one float spill home, one schedule swap.

Summary before this remeasure: Configured stock 760/762 result, 740 relocation-masked differences and exact frame; observed declared homes and the multiply-hazard schedule are reconstructed. Entry narrowing, one compiler scratch home and exact relocation/linked proof remain unresolved.

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0x0.

#### 2026-10-02, lane x-sib2: first notes after the sibling Draw matched

`func_overlay_073_F0000D70_18CB830` matched as a sibling copy of the overlay
71 renderer (packet macros, `vertices[vertexBank * 6]` over ten-byte
vertices); this updater writes the same vertex banks.

- `D_20` through `D_54` are LOCAL records against the module's rodata (data
  +0xD0); they are float literals (0.004, 0.1, 0.064, 22500.0, 1.2, 1.6).
  Writing them as literals is inert (755 at +24) but is the shape the
  promotion needs.
- Entry: the target re-sign-extends the 16-bit field at state+0x94 before
  the multiply by `updateRate` (a cast the candidate lacks), keeps the
  state pointer in a temporary spilled at +0x58, and in case 0 stores the
  zero timer and passes the zero float argument from two separate
  materialisations (the candidate shares one).  The hits buffer is at
  +0x38 in the target (+0x90 in the candidate), so the local set and order
  differ from the first block on.

#### 2026-10-03, owned output-storage and lifetime packet

Independent exact resident `func_8005776C` writes `HitCopyState *` elements and
returns an `s32` count. All four shipped calls bind to this callee. Replace the
false scalar output with a compatible opaque tagged pointer array of eight
entries and an exact pointer-to-pointer formal; preserve the indexed fourth
selection and the first three element-zero selections. The supported query
and RNG domain is count 0..8, index 1..8 and decremented index 0..7. Neither the
producer nor getter universally clamps the registry count to eight. Keep the
callee's `s32` ABI; narrow returned values only at independently observed
signed-halfword transports. These changes disclose an intentional local
prototype-context correction, not unchanged context against the old source.

The exact matched drawer independently proves twelve ten-byte, alignment-two
vertices, resource at state+0x78, bank byte at +0x7C, state size 0x9C and
alignment four. Reconstruct this actual producer storage and bank*6 indexing;
keep the eight x/y stores, all z/color bytes and opaque resource unchanged.
Supported banks are zero and one; the preserved-state path does not clamp the
bank. Share the mutually exclusive vector and angle/delta roles only after
proving their dominated assignments and nonoverlapping lifetimes.

The two case4 paths join the shipped existing timer tail. Both execute the
same load/add/wrap exactly once after their original side effects; the early
path sets mode two and skips countdown/query/target/RNG updates. This removes
the duplicate float-pool pair and closes text relocation cardinality from
48 to 46. It does not establish exact relocation offsets or linked identity.

In case3 and case4, refresh target data after the independently authenticated
angle/RNG calls instead of retaining a named data pointer across them. Retail
reloads and the eliminated compiler spills support the lifetime reconstruction.
Arctanf has no stores or calls; the RNG helper writes only its canonical seed.
Intervening angle stores cannot overlap state.target or target.data on valid
live allocated objects. The constructor places own state beyond the header
for nonnegative signed resource counts; allocation selector 84 means raw
header kind 85. No equivalence is claimed for malformed, interior, freed or
asynchronously mutated pointers, or negative resource counts.

Preserved stock controls: typed array alone 757/+24/frame B8; common carriers
744/+28/frame98; precise return-width transports 762/+80/frame98; typed vertex
storage 765/+92/frame98. A used short-copy/compound multiply is output-inert.
Shared timer tail yields 750/+32/frame98/46 sites. Widening only the angle
snapshot reproduces the missing premultiply conversion but regresses to
758/+40/frameA0, so remains a private diagnostic. Case3 pointer refresh keeps
750/+32/frame98 while reducing opcode distance 103 to 96; case4 refresh yields
749/+32/frame98 and reduces it to 90. The final aligned residual has 110
structural sites; alignment changes make these counts incomparable with a
positional score.

Actual configured compiler input was captured synchronously under untouched
stock IDO arguments. Self-context is unchanged; all allocated TU bytes,
section geometry and effective relocation rows match raw stock output.
Only this guarded body and its local declaration/type context changed;
canonical fallback ROM remains the acceptance baseline. The narrower fidelity
reader does not account for the compiler-owned switch section; full-TU byte
and relocation equality supplies the stronger capture proof, not runtime
binding proof. Exact linked-byte and runtime relocation geometry remain
unproved. No match credit is claimed.

Remaining concrete blockers are eight excess words, stack-home relationships
(buffer/state versus the retail homes), and the entry signed-width conversion.
No independently supported next source lever was available after the two
informative pointer-lifetime controls; no declaration-order or padding grid
was run. Preserve all meaningful source/object/capture/score artifacts privately.
#### 2026-10-07: committed handoff recovery

Recovered the source and handoff from committed lane result
`e90f31aff2eca30cf491406feec42285f09fcf2f`, without accessing that lane's
working state or making a new matching attempt. Fresh configured stock IDO
reproduces 768 versus 760 words, 749 raw and relocation-masked differences,
first difference +0x1C and the exact 0x98 frame. Static and target runtime
relocation cardinality is 46 on both sides, but only two offsets/types align;
16 candidate identities resolve and 30 literal/switch-table address records remain
unresolved. This remains NON_MATCHING, with zero new matched bytes.

The actual compiler input was captured and stock-preprocessed synchronously;
self-context comparison is unchanged. Every allocated section's geometry and
bytes and every normalized relocation identity agree between raw compiler
output and the configured object. The only symbol-table addition is the
assembler prelude guard. The query producer's pointer-array ABI and matched
drawer's vertex layout remain the independently authenticated storage basis
from the earlier handoff; no declarations or semantics were changed here.

`gmake verify` reproduces the expected US ROM SHA1 with the assembly fallback
selected. `tools/wb_compare.sh --summary-json`, targeted ranking refresh and
`function_preflight.py --analysis-only --json` provide the fresh measurements.
Actual compiler capture, self-context, fidelity, preflight and verify evidence
remain private under `build/recovery-o073/`. This recovery does not authorize
new attempts: integrate it and establish current reopen pins before a separate
complete stack-home and signed-angle reconstruction packet.

#### 2026-10-07: joint automatic-home and angle packet

The authorized packet followed recovery integration and a fresh zero-exit
`base-only` assignment gate. The configured candidate is now 762 versus 760
words, 741 raw and 740 relocation-masked differences, first difference +0x1C,
with the target 0x98 frame. It remains NON_MATCHING: zero matched bytes.
The shape-tolerant opcode distance fell from 90 to 6; this diagnostic is not a
percentage or a substitute for exact bytes and relocation identities.

Real function-scope roles reconstruct every observed declared home: coordinate
components, the signed step, vertex and state pointers, and the eight-entry hit
buffer. Shared roles have dominated assignments and disjoint lifetimes. The
remaining frame difference is one compiler-created float-rate spill home,
+0x30 instead of +0x34, with the same three saves and three reloads. Target
storage widths, vertex-bank bounds and the earlier query/RNG domain remain
unchanged. The signed angle snapshot is word-sized; the persistent step stays
a signed halfword. Both steering comparisons now negate the promoted signed
step without an incorrect narrowing cast, preserving the target's behavior
when that step is the signed-halfword minimum.

The target retains a named scaled-radius value across each distance expression.
Reusing the existing, nonoverlapping float `limit` role reconstructs that
transport. Integer zero in the case1 velocity/query lifetime and the case3
sign comparisons reproduces the target's shared positive floating zero. These
integer-to-float conversions preserve the value and floating comparison
semantics; the distinct float zeros after the reset calls remain distinct.
First-hit reads precede independent state-field writes. The fourth query uses
a signed-halfword count and a word-sized selected index, with its actual
nonzero arm first. The case3 nonnull arm and final velocity-limit arms follow
the target's control flow. Case4 uses a complete natural if/else with one
shared timer tail, preventing an incorrect early conversion merge while
preserving the skipped side effects and exactly one timer update.

The object-local `-Wab,-r4300_mul` correction is supported by the target's
independent floating-multiply hazard sequence, not by a positional score.
On identical captured compiler input it restores four missing scheduler nops
and the radius-test branch-delay form. The full TU owns only this guarded
function; no other object receives the flag. Without the flag the same source
is 758 words with opcode distance 12. The configured flagged source has two
excess entry normalization words, followed by a query-result scheduling
residual and allocation differences. The actual stock compiler input and raw
output are preserved; no emitted instruction is edited.

Closed entry controls include direct/result casts, ternary and assignment
predicates, a register hint, widened carriers, phase/short-phi splits, unsigned
views, low-product masks, unsigned negation, multiply-by-negative-one, external `abs`,
and scalar/one-field-struct/one-element-array storage. None closes the signed
step's entry normalization and persistent halfword home together. The array
introduces real entry memory traffic; the widened forms spill a word; standard
`abs` emits an unwanted call. Named float-rate controls either hoist the wrong
store lifetime or grow the frame. Chained query assignment, assignment inside
the predicate and count-versus-index predicates are output-inert; stop that
schedule axis after these three controls. Destructive coordinate squaring and
nonzero integer-literal spelling also failed to explain the residual.

Fresh configured preflight still finds 46 static and 46 runtime relocation
sites, but only one offset/type pair aligns. Sixteen candidate identities are
resolved and thirty literal/switch-table identities remain unresolved. The
candidate exceeds the owned executable range by eight bytes. Actual compiler
capture agrees with the configured object's allocated sections, geometry and
all normalized relocation identities. The dedicated TU flag-impact report
confirms one consumer and 46 static sites under either flag choice; its raw
positional preference for the unflagged build does not override the observed
hazard mechanism. Fresh extraction, overlay-symbol regeneration and
`gmake verify` reproduce the expected US ROM SHA1 with the assembly fallback. No
candidate linked-byte or runtime-identity proof is claimed.

All meaningful sources, actual preprocessed inputs, raw/configured objects,
contexts, frame censuses, scores and stock phase streams remain private under
`build/o073-home/`. A reproducible next packet needs new evidence for the CFE
short-assignment/predicate web split or the compiler spill-pool ownership;
repeating the closed width and spelling controls is not a new hypothesis.
The float-pool/switch address bindings and final linked owned-byte proof also
remain necessary before any promotion.

#### 2026-10-07: two-short entry carrier diagnostic

A fresh authorized lane and zero-exit `base-only` gate tested a distinct
width-preserving split: compute the signed product in existing short `delta`,
copy it to short `absStep`, and use `delta` for the sign predicate and negative
arm. The later angle-difference assignment dominates all subsequent `delta`
uses, so the two roles have disjoint lifetimes. No automatic homes, flags or
headers were added or changed.

The configured baseline reproduced 762 versus 760 words, 741 raw and 740
relocation-masked differences, first +0x1C, frame 0x98 and opcode distance 6.
The diagnostic's complete executable section and relocation inventory are
identical to that baseline. It therefore retains the duplicate normalization
web and supplies no new allocation or control-flow lever. Actual configured
preprocessed inputs preserve all context outside the function; untouched stock
compiler output and configured objects agree in allocated section geometry,
bytes and normalized relocation identities for both builds.

The canonical source is restored unchanged. This closes the two-short
product/predicate split with zero matched bytes; it does not reopen other
closed spellings. Private source, input, object, context, fidelity and score
artifacts are preserved under `build/o073-home/` in the diagnostic lane. No
linked promotion is claimed and no ROM rebuild is needed for this report-only
closure. The next packet still needs independently new evidence for the
normalization web or spill-pool ownership, plus eventual literal/switch binding
and linked owned-byte proof.

#### 2026-10-07, lane b-o073: entry region, direct hit copy, field-read ramp

Three source-shape edits, each measured as a product with
`tools/shape_product.py`, took the function from 740 masked words at
size +8 to 163 at size 0 (aligned: 597 exact, 155 naming, 6 immediate,
2 structural; first mismatch +0x180).

- Entry, 740 at +8 to 212 at 0. The duplicate narrowing was the
  multiply sharing a uopt region with the sign test. Putting the
  multiply in its own `do { } while (0)` region removes it (237, with
  the step coloured a1 because its web no longer spans the entry
  block). Reading the field directly (`absStep = state->angle;`, not
  through the s32 snapshot) restores the step's entry-block definition,
  so the parameter register is forbidden and the step takes t1; the
  entry words then agree with the target. Nine carrier/cast forms and
  four sign-test forms without a region were all +8 or -8 (two
  products, 45 cells); region placement was a 32-cell product.
- Case 0 hit copy, 212 to 164. `state->target = hits[0];` written
  first, with no `target` local. The same edit at the case 1 or case 3
  copy regresses (27-cell product; best combination is case 0 only).
- Angle ramps, 164 to 163 and first mismatch +0x124 to +0x180.
  Test and update the field itself (`if (state->angle < 0x480)
  state->angle += updateRate * 0x10;`, and the case 4 mirror); the
  s32 snapshot local is then dead and its store is removed. Deleting
  its declaration as well regresses to 229 (frame side effect), so
  the declaration stays.

Measured flat (do not repeat on this shape): Arctanf return type
s16/s32/u16, targetAngle s16/s32, and four casts on the angle
difference (24 cells, all 163); the ramp add's operand order.

Remaining: the ring is two draws behind the target from the case 1
angle difference (+0x18C) onward, with every instruction between
the ramp and that point register-identical; the float spill home
(+0x30 against +0x34) and one schedule swap near +0x7A4 are the two
structural rows.

#### 2026-10-07, lane b-o073 (continued): case 4 data read, 163 to 153

- Case 4 reads `data = ((Func073Target *)state->target)->data;` with no
  `target` local: 163 to 153 at size 0 (aligned 607 exact, 145 naming,
  6 immediate, 2 structural).
- The target also reads `state->target` and `hits[0]` into ring
  temporaries in case 1 (hit copy) and case 3 (first data read, inner
  hit copy), where this source keeps the colored `target` local (v1).
  Rewriting those three sites directly is the target's shape but
  measures worse on this source (36-cell product, best 168 to 190
  aligned naming rows) because the ring is already out of phase there;
  re-measure them after the phase is fixed.
- Ring phase, read from the listing in address order: the source
  agrees with the target through the case 1 ramp (+0x120), then the
  target is two integer ring draws ahead at the angle difference
  (+0x18C), with every instruction in between identical. Flat on this
  shape: eight statement forms for the Arctanf call and difference in
  each of cases 1 and 3 (64 cells), Arctanf return type, targetAngle
  type and four casts (24 cells), the ramp's operand order and
  compound forms. The two draws are folded (L149): nothing emitted
  between the ramp and the difference consumes them.
- Case 4 selected index: the target computes `hitIndex - 1` into a1,
  the count's register, as a colored web; this source uses a ring
  temp. `hitIndex--` makes it a web but colours it a0 (174);
  `phase = hitIndex - 1` and an s16 `hitCount = hitIndex - 1` are
  propagated away (153, unchanged); an s32 `hitCount` is -8.
- Float-rate spill: the CSE of `(f32)updateRate` spans the case 1
  calls and is homed at hits-8 (+0x30) where the target uses hits-4
  (+0x34). Moving the dead `s32` pad through all 18 declaration
  positions never moves it (position 3, the current one, is best);
  deleting the pad drops the frame to 0x90.

#### 2026-10-07, lane b-o073 (resumed): Arctanf returns int, 153 to 131

- The two missing ring draws before each angle difference were the
  narrowing of an int-returning `Arctanf` into the s16 `targetAngle`;
  as1 folds the pair, so no word changes. Declaring
  `extern s32 Arctanf(f32, f32)` (u16 and u32 read the same) with
  `targetAngle` kept s16: the ring then agrees from the case 1
  difference through case 3. With s32 `targetAngle` uopt drops the
  narrowing and nothing moves (24-cell product).
- With the ring in phase, the target's direct reads are now
  improvements: case 1 hit copy `state->target = hits[0];` written
  first, case 3 `data = ((Func073Target *)state->target)->data;`,
  case 3 inner hit copy written first. Together 153 to 131 masked at
  size 0 (aligned 629 exact, 123 naming, 6 immediate, 2 structural).
- Flat or worse on the ramp and difference (draw-placement products,
  90 + 18 + 15 + 36 cells): `<< 4`, `0x10 * x`, compare spellings,
  argument assignment order and inline assignment, `delta -=`, an
  `(s16)` on the clamp or the call, an s16/s32 carrier for
  `object->angle`. An `(s16)` on the ramp product does add two folded
  draws, but before the ramp's add, so it misnames the ramp.

- With the ring in phase, the velocity blocks of cases 3 and 4 read
  `((Func073Target *)state->target)->data->y` at the use, with no
  `data` local: 131 to 74 (case 3) to 33 (case 4) at size 0.

- The first height difference in cases 1 and 3 (velocity steering) is
  its own float local, `height`, declared in the dead s32 pad's slot
  (pad replaced, frame unchanged); reusing `limit` or `dy` keeps one
  web whose colour is f2 where the target has f0. 33 to 20 at size 0.
  A new f32 declared elsewhere shifts every home (43 immediate rows).
- Remaining 20: the case 3 velocity zero is f14 where the target has
  f2 (int-zero against `0.0f` on the four compares: 16 cells, only
  the all-int cell holds size 0); case 1 sqrtf reload of dz into f14
  where the target uses f18; the float-rate spill +0x30 against +0x34;
  and the case 4 query (narrowing schedule order and `hitIndex - 1`
  coloured a1 in the target, a ring temp here).

- Zero spellings (512-cell product over the nine float-zero sites that
  are not sign tests): the case 1 reset writes `object->velocityY = 0;`
  (int zero). 20 to 14 at size 0; it fixes the case 3 velocity zero
  (f2) and the case 1 dz reload (f18) together.
- Exhaustive single-force landscape at 20 (278 probes): no force beats
  the unforced build, so the remaining rows are not colour decisions.
- Extra pad of s32, s16 or u8 at each of 19 declaration positions (57
  cells): the float-rate spill stays 8 below `hits` in every cell; it
  only moves when `hits` moves.
- Case 4 query, 14 remaining rows minus the spill: narrowing order and
  `hitIndex - 1` into a1. Chained, comma and in-predicate assignments,
  int-typed count, regions around the index store, and s32/s16
  carriers for the index (41 cells) are flat. `hitIndex--` makes the
  index a web (coloured a0, not a1) but no longer draws the ring
  register the target spends there (75).

- The fourteen `D_20`..`D_54` externs are now float literals at the
  point of use (0.004, 0.1, 0.064, 22500, 1.2, 1.6 in use order). Score
  unchanged at 14; the compiled literal pool is byte-identical to the
  target's (compared locally), which the promotion needs.
- Spill home, located by bisection: removing the case 0 timer block
  (both the add and the wrap) puts the float-rate spill at +0x34, the
  target's home; removing either half alone, or cases 1, 3 or 4, does
  not. Flat on the case 0 block: operand order, int or float
  zero/one spellings, `+=` against an explicit add, the wrap's compare
  and subtract forms, `break` for `goto common`, and a declared `f32
  rate` after `hits` (uopt still spills to its own temp, 8 below the
  lowest home). A double `1.0` moves it to +0x38, so the case 0 timer
  code reserves one extra 4-byte frame temp that the target's
  source does not.

- Spill home, continued (cycles 45-46): `(f32)1.0`, the case 0 timer
  through the float `limit` local (two forms), the add and the
  wrap as one expression, and `!(timer < 1.0f)` all leave the spill
  at +0x30. The uopt `-zdbug:2` list carries the same memory variables
  and offsets with and without the case 0 timer block, so the
  extra 4-byte cell is allocated after uopt (in the emitter's
  temporary area), not as a uopt variable.
- Case 4 query (cycle 46, 7 cells): if/else for the selected index
  (+8), the ternary inline in the subscript (+8), the ternary into
  the index, the copy inside the count test, testing the index
  against 2, and pointer arithmetic for the subscript are flat or
  worse; none puts `hitIndex - 1` in a1.

#### 2026-10-07, lane f-o073: the spill home and the count/index line, 14 to 4

- Float-rate spill (+0x30 against +0x34): the extra 4-byte compiler cell
  is reserved whenever `object->timer` is read in case 0 or before the
  switch, by any read (the add, a compare, `(s32)object->timer`, a copy
  into a local). A store alone (`object->timer = 0`), reads of other
  object fields (x, scale, velocityY), `state->timer`, an int field, and
  the same read placed in `default:` reserve nothing (18 cells). Writing
  the case 0 add as `limit = (f32)updateRate * 0.004f; object->timer +=
  limit;` (any existing float local: height and dx measure the same)
  numbers the product before the timer read; the case 0 code is
  byte-identical and the spill lands on +0x34: 14 to 8. The direct
  `object->timer = (f32)updateRate * 0.004f + object->timer` also moves
  the spill but swaps the add's operands (136). The cell follows which of
  the two expressions uopt numbers first, not the spelling of the wrap.
- Case 4 query, first block: the count assignment, the index copy and
  the test (`hitCount = (s16)func(...); hitIndex = hitCount; if
  (hitIndex != 0) {`) on one physical line: 8 to 4. as1's line tie-break
  then schedules the index narrowing before the count narrowing and puts
  the state reload in the beqz delay slot, as shipped. The same statements
  on two or three lines, chained, in the predicate, or with blank lines:
  8 (7 cells).
- Remaining 4, all naming at +0x7C8 to +0x7D8: the target draws one more
  ring temp between the beqz and the mathRnd narrowing (its sra takes t9,
  ours t8) and computes `hitIndex - 1` into a1 as a web where ours is a
  ring temp. Flat or worse on this shape (products of 432 and 128 cells
  plus about 30 single cells): count/index assignment forms, test
  spellings, the mathRnd result cast, s16 hitIndex (frame 0x90),
  `hitIndex--`, `-= 1` and `= hitIndex - 1` (the web exists but takes a0
  and no extra draw, so the ring runs one behind to the end: 65 to 69),
  `hitCount = hitIndex - 1` with `hits[hitCount]` (substituted away, 4,
  also with a store between, a region around the def or the use,
  `register`, or-zero on the def, the use or as a statement), a second
  real use (web in v0, +4 bytes), product-by-zero second uses, or-zero on
  the mathRnd arguments, test or result, mathRnd returning s16/u16,
  taking s16 parameters, or unprototyped.
- Records (instrumented uopt, identity-gated): with `hitIndex = hitIndex
  - 1` the a0 web does not appear among the p1 decisions; forcing the
  three a0-coloured p1 webs near it (w301, w309, w316) to other colours
  leaves the addiu in a0, so its colour is not a p1 choice. In the target
  the count web (a1) and the subtraction (a1) do not interfere, so the
  target's count web ends before the join block; ours spans it. The
  decision variable is what colours that non-p1 web, and what draws the
  extra ring temp in the if arm; both are open.

- Resumed (lane f-o073): a 108-cell product over the if arm is flat at 4:
  the count test as `>= 2`, `> 1`, `hitCount - 1 > 0`, `hitIndex - 1 > 0`,
  `(hitCount - 1) != 0`, `hitIndex > 1`; the mathRnd arguments as
  `hitCount`, `(s32)hitCount`, `hitCount + 0`, `(u16)hitCount`,
  `(hitIndex, hitCount)`, `(1, hitIndex)`; the index as the ring temp,
  `hitIndex = hitIndex - 1`, or masked with `& 7`. No cell draws the
  extra ring temp or puts the subtraction in a1. Cycle-21 line: the
  a-register of the join-block subtraction web is not a p1 decision, so
  trace which pass assigns it (CDX_DETAIL_WEB on the `hitIndex = hitIndex
  - 1` cell, then the ugen trace for any a-register ALLOC in that block);
  the extra draw is between the beqz and the mathRnd narrowing.

#### 2026-10-07, lane g-near: the a1 web is the unsubstituted count definition; one ghost draw remains

Fresh eyes on the 4-word residual. The recorded next step (trace which pass
colours the join-block subtraction) was not the right question: the target's
`addiu a1, v1, -1` is not a fresh web coloured a1, it is hitCount's own a1
web redefined in the join block. uopt forward-substitutes `hitCount =
hitIndex - 1` into `hits[hitCount]` (every earlier lane measured that cell
as 4, "substituted away"); a no-op redefinition of hitIndex between the two
statements, the same lever that matched overlay68UpdateAnimation today,
kills the substitution: `hitCount = hitIndex - 1;`, then hitIndex or-assigned
with zero, then `state->target = hits[hitCount];` compiles to the target's join block
exactly (addiu a1, then the s16 narrowing of a1 and the subscript, with the
same ring temps in the same order relative to the block's first draw). The
cast and the and-with-minus-one and xor-with-zero spellings measure the
same; the or on hitCount instead of hitIndex, and a trailing use of either
variable, do not (30-cell product). Source and tree are unchanged: that
shape measures 64 masked at size 0 because what remains is a single ring
draw, not colour, and the positional count counts its whole shadow.

What remains, located exactly: the target spends one ring draw with no
surviving instruction between the hitIndex narrowing's `or v1, t7` and the
mathRnd arm's `sra` (ugen emits 680 to 692 in our trace: the two tests, the
constant a0 argument, the state spill and reload). The freelist is a pure
rotation here (simulated from the trace against all 121 draws), so the draw
cannot be anywhere else: earlier would shift the call block's t4..t7, later
would shift nothing. A ghost draw of this kind exists in this very
function, at the switch dispatch (ALLOC t4 freed before any emission).

Measured flat on the kill shape, all 64 at size 0 unless noted:

- The mathRnd arm: cast or no cast, s16-returning callee, result through an
  s32 local, count carried through hitCount, argument as hitIndex, (s16)
  casts and or-zero on either argument, 1U and (s16)1, five test spellings
  (90-cell product); or-zero, cast and self-copy probes on either variable
  before the test (35 cells).
- The countdown block: an and-mask or a (u16) cast on the countdown
  subtraction spends a draw but two lines too early (12 at size 0, first
  mismatch +0x754, the call block's t4..t7 shift); (s16)1 and &hits[0] as
  arguments, a double cast on the count, (s16) on the index copy (45 cells).
- The query line: five assignment forms, three index types, one or two
  physical lines, three spellings of each test (180 cells). One form is
  informative: `phase = func(...); hitCount = phase; hitIndex = phase;` with
  s32 hitIndex and the tests on hitCount and hitIndex reads 6 at size 0,
  first mismatch +0x794: the ring is then in phase through the mathRnd arm
  and the join block, because the test narrows phase into two ring temps
  (sll t7, sra t8) while hitIndex is a plain copy. That proves the target's
  call block spends five draws where ours spends four, and that the fifth
  is a narrowing-sized draw. With hitIndex narrowed (s16, or an explicit
  cast) the test shares hitIndex's narrowing again and the draw is gone
  (108 + 270 cells over phase-carrier definitions, test operands, A-block
  redefinitions). Explicit double narrowings, shift pairs, multiply-by-one
  and add-zero on the index definition (24 cells) are folded by uopt before
  ugen draws.
- Forces (instrumented uopt, identity-gated): the constant-1 argument web
  to a2, a3, t2, t5 or split, and the ghost narrowing web (type 4, a0, the
  test blocks only) to a2, a3, t2..t5 or split: none puts t9 under the
  mathRnd narrowing; t2/t3 forces rotate the ring the other way. t8 and t9
  are not colours in this procedure (c13 is unnamed, c14 is s0), so the
  skip cannot be a coloured web removed from the ring.
- Countdown store before the target store, and the countdown clear moved
  after the query call: worse (11 and 314 at -8).

Cycle-21 line: the decision variable is the construct in the target's
source that makes ugen draw and release one ring temp between the index
narrowing and the mathRnd call with no surviving instruction. The two
candidates the data leaves are an as1-forwarded copy of the narrowed index
(ugen `move tN, v1` for a test operand that is a temp rather than the
symbol, deleted after forwarding, which is how `beqz t7` already reads
through `or v1, t7`) and a second narrowing of the same value that as1
folds like the double andi of L149. Next: on the kill shape, compile the
`phase` carrier cell (6 words) with the ugen trace and diff its ALLOC rows
against the kill shape's to name the handler that drew the fifth temp;
then look for a source form whose test reads an expression temporary of
the narrowed index rather than hitIndex itself. Do not re-run the
spellings above.

#### 2026-10-07, lane i-near: the fifth draw is not a spelling of the test or argument sites

Measured by tools/bank.py: masked 4 (raw 19), size delta +0, candidate 760 words vs target 760. Aligned: byte-exact 756, register naming 4, immediate only 0, really different 0.

Records (web_report, identity-gated): hitCount is web 212, a1, two
occurrences (def at the query line, use at the count test); hitIndex is not a
symbol web at all but the expression web 26 `cvtl($v0, @16)` coloured v1 over
the query, test, mathRnd and join blocks. The target's `addiu a1, v1, -1` is
hitCount's web redefined in the join, as g-near read it. Nothing in blocks
87 to 92 is coloured a0, and no web for the mathRnd constant exists.

The ugen trace on the tree body (draw_census, kept) shows no draw between
the hitIndex narrowing (sra t7, emit 681) and the mathRnd narrowing (ALLOC
t8, emit 693): the two tests read the symbol register through f_ureg and
f_get_reg1, the constant goes straight into a0 through f_get_dest, the
state spill and reload use t0. The function's only ring rotations with no
emission are the switch dispatch (f_is_available: a draw of t4 freed before
any emission) and a MOVE_END at line 289 (f_eval2 re-reading updateRate's
load still held in t2, which moves t2 to the free list's tail). t8's last
value before the query is the 0x180 clamp constant from line 277, in another
block, so the target cannot be re-reading it; the touch is a draw whose
instruction as1 folded, or a value freed late. The only foldable word in the
span is `addiu a0, zero, 1` (a temp plus a deleted move when a0 is not free
for f_get_dest), and nothing occupies a0 there in our records.

Measured flat (do not repeat):

- a frame-compensated product, 48 cells: hitIndex declared s16 with an s32
  pad after it (positions 1 and 2) restores the 0x98 frame and reads exactly
  the tree's 4 rows; pad elsewhere shifts the homes (10 to 44); the second
  test's operand (hitCount against hitIndex) is inert because uopt makes
  them one value; the kill-shape join is 64 in every cell as recorded.
- a mini-TU scan of the query/test/mathRnd shape with the configured flags
  (24 single-axis spellings, read from the ugen listing): the mathRnd
  constant as (s32)1, 1U, (s16)1, 1L, 0x1; the second argument as hitIndex,
  (s32)hitCount, (s16)hitIndex, hitCount + 0, hitIndex or zero; the first
  test as hitCount, bare hitIndex, > 0, (s16)hitIndex != 0; the second test
  as hitIndex >= 2, > 1, (s16)hitIndex >= 2, hitCount - 1 > 0, (s32)hitCount,
  (s16)2, 2U. None draws exactly one extra ring temp with the same
  instruction count; the explicit (s16) casts draw two and emit two.

Cycle-21 line: the decision variable is still the construct that draws and
releases one ring register between emit 681 and 693. Next: a scan of the
mini TU over constructs that make f_get_dest refuse a0 for the constant
(something pinned in a0 across the two tests: a web coloured a0 live into
the mathRnd block, or a call argument moved through a0), read from the ugen
trace, not the score; the records say no such web exists in the tree body,
so it must be created by the source.
<!-- plateau-handoff:func_overlay_073_F0000190_18CAC50:end -->
