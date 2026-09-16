<!-- plateau-handoff:overlay2QueryNode:start -->
### `overlay2QueryNode` plateau handoff

- source: `src/overlays/o002/overlay2QueryNode.c`
- score: 39/253 words
- frame: 0x68
- relocations: 51
- first mismatch: +0x58
- summary: Fresh 48-draw census retains the schedule; parameter-reload probes restore reissue only with broad allocation regression.

#### 2026-09-09 lane `w2-bigA`: 40 to 39, and the residual is four named sites

One strictly better cell: writing the split test as `D_50 == D_54` rather than
`D_54 == D_50` fixes the operand order of the `bne` that implements it. The
baseline is now 214/253 relocation-masked words (51 raw), 51 of 51 relocations
emitted, frame 0x68, size delta 0.

The remaining 39 words are four sites, each localized to a specific choice.
Recording them so the next worker searches the right space:

- **Leaf-loop join, 7 words.** After the loop, the target rematerializes the
  leaf count as a zero into the argument register it is returned from; the
  candidate instead keeps a second live copy of the counter, made in the loop
  tail. Both sides carry the counter in the same register up to that point, and
  the two forms have the same instruction count. Eliminated: `register` on the
  counter (inert), both orders of the counter update against the cursor
  increment, `!= 0` on the loop condition, moving the pre-decrement into the
  initialiser, reordering the four head statements, and an explicit `count = 0`
  after the loop (+5). `return 0` in place of `return count` shortens the
  function by a word and collapses to 151 -- the leaf-zero hypothesis is dead.

- **Tail-call node selection, 4 words.** The target never writes the node
  parameter back; it computes the chosen child into a scratch register and
  passes it. The `node = node->sideN` form keeps it in the parameter's own
  saved register. A conditional expression removes the write-back and is
  structurally right, but colours the temp `v0` where the target has `a0`, so
  the site stays at 4. So this is now a colour question, not a structure one:
  the target spends one more colour before this temp than the candidate does.

- **Recursive OR block, 14 words (both mirror copies).** Two register renames,
  one adjacent load/store swap, and the bitwise-or's operand order. That order
  is **not** source-reachable: all four combinations of the two or-expressions
  are byte-identical, so uopt normalises the operand order and the difference
  has to come from the carrier the second operand is loaded into (the target
  uses `a0`, the candidate a temp). The single-element array holding the first
  result is load-bearing -- a plain scalar loses a word and collapses to 99.

- **Recursive short-circuit block, 14 words (both mirror copies).** The target
  computes `!= 0` into a temp with `sltu`, branches on the temp, and returns it
  from the branch delay slot. The candidate emits an inverted test plus a
  separate branch and a literal 1. Naming the boolean does produce the `sltu`
  -- verified in the object -- but every carrier tried inverts the branch
  polarity and costs a word: a fresh local (107, and the frame grows), and
  reusing `leafResult`, `remaining`, `count`, or the result array (78-79). A
  doubled `!= 0` is 80. The short-circuit-or spelling produces the `sltu` and is a word
  shorter overall but branches to a shared tail instead of returning from the
  delay slot (110-120).

A 48-cell cross of the comparison order, three node-selection forms, all four
or-operand orders, and both loop-tail orders floors at 39 with no interaction:
these knobs are independent and flat. The next lever should be a colour
argument, not another spelling sweep -- three of the four sites reduce to "the
target spends `a0` where the candidate spends a pool temp or a saved register".


#### 2026-09-11, lane p7-ovl2: the short-circuit block's target shape read off
#### the object, and 32 carrier forms measured against it

Baseline reproduces: 1,012 bytes, 253 of 253 words, delta 0, masked 39, raw 51,
frame 0x68. Aligner: sizes exact, with three surplus candidate words at +0x17C,
+0x314 and +0x3A8 paired against three missing ones at +0x194, +0x32C and
+0x3C0. Corrected census after today's float-bank fix: 15 substitution sites,
all integer, 88 percent coherence, three windows opening at +0x2FC and +0x390,
no closed cycle. Nothing was adopted, so the buckets are the same after.

**The target's short-circuit shape is not a shared tail, and that is new.** At
the first of the two mirror blocks the shipped code normalises the call result
to a boolean in a scratch temporary, branches on that temporary straight to the
function epilogue, and puts the copy of the temporary into the return register
in the branch's delay slot; the block is four instructions and the argument
reload that follows it is issued four words later. This candidate branches on
the call result itself, hoists that same argument reload into the delay slot,
and then jumps to the epilogue with a literal one. Both blocks are four
instructions, so the sizes agree and the difference is which four. The earlier
reading that the boolean shape necessarily costs a duplicated load is only true
of the short-circuit-or spelling, which folds the two reloads and lands eight
bytes short; the shipped shape keeps the size.

**32 boolean-carrier forms, measured on this baseline.** Four carriers, the
three dead-in-this-arm integer locals and the one-element result array, crossed
with eight return and branch shapes, each applied to one block alone and to both
mirror blocks. Results, all at the exact 0x68 frame: assigning the boolean and
then returning a literal one is byte-identical to the incumbent in every
carrier, because uopt folds the boolean away when it is not the returned value;
every form that returns the carrier costs four bytes per block and scores 80 or
81; the test-for-zero family that assigns the second call's boolean into the
same carrier and returns it once is eight bytes short and scores 78; and the
ternary and inline-assignment variants run eight to twelve bytes over.

**So the trade is confirmed and sharpened.** The boolean is reachable only by
returning it, and returning it costs exactly the word the incumbent spends
materialising the literal one, which is why every carrier lattice run so far
lands four bytes over or eight bytes under and never on. The next lever has to
be a form in which the boolean is already the value of the expression being
returned without a separate copy, not another carrier and not another branch
polarity. Do not re-run the carrier, polarity, or-operand, node-selection,
leaf-head or line-join lattices; they are recorded above and here.

#### 2026-09-12, lane p9-mid: the eight-byte shortfall is one named instruction

Baseline reproduces: 1,012 bytes, 253 of 253 words, delta 0, masked 39, raw 51,
frame 0x68. Aligner: 223 byte-exact, 15 register naming, 2 immediate-only, 16
really different, displacement tax 6, with the same three surplus/missing pairs
at +0x17C, +0x314, +0x3A8 against +0x194, +0x32C, +0x3C0. The frame census prints
two identical twenty-one-slot ladders, so the frame is not in play. Nothing was
adopted, so the buckets are unchanged.

**The previous closure said the short-circuit-or spelling "folds the two reloads"
and lands eight bytes short. That is right, and the instruction it folds is now
named.** At each mirror block the shipped code emits three words -- normalise the
call result to a boolean, branch on the boolean to the epilogue, copy the boolean
into the return register in the delay slot -- and then issues **both** of the
second call's stack-passed float arguments afterwards. The two-if form emits four
words there -- branch on the result, a hoisted load of the second call's third
argument in the delay slot, a jump to the epilogue, a literal one in its delay
slot -- and therefore does not need to reissue that argument later. The counts
balance exactly: the two-if form spends one extra word at the block and saves one
later, which is why it is the right size, and the or-spelling spends one fewer at
the block and still saves the later one, which is why it is four bytes short per
block. **The requirement is the shipped triple together with a separate reissue
of the second call's third argument, and that reissue is the whole of the eight
bytes.**

**Why the obvious range-splitting devices cannot supply it.** That argument is
not read from the global at all: it is loaded from a stack slot the frame already
holds, so L131's spelling-split family has nothing to act on. Measured here, each
applied to both mirror blocks: the plain or-spelling, the address form
`*(f32 *)&D_58` on the second call, on the first call, the volatile address form,
**`volatile` on the file-scope declarations of both globals** -- all four are
byte-identical to the plain or-spelling at 119 masked and minus eight bytes, so
volatile does not reach a stack reload of an already-copied global. Routing the
first call's arguments through the two dead float locals costs 24 bytes; routing
the second call's node selection through a dead pointer local is still eight
short; an L109 probe inside the second operand and assigning the first result to
the one-element array both run eight bytes over.

**The or-block's own five words are scheduling, not spelling.** At the
`D_60 != 0` arm the shipped code issues the next node pointer load before storing
the first call's result and reloads the saved result into an argument register
where this candidate uses a pool temp. Joining the two statements onto one
physical line (L59), both operand orders of the bitwise or, an L109 probe between
them, and a compound-assignment two-statement form are all byte-identical at 39
or worse; hoisting the node selection into a dead local costs 36.

Do not re-run the carrier, branch-polarity, or-operand, node-selection, leaf-head
or line-join lattices, and do not re-run the four reload-split devices above. The
next lever has to produce the extra argument reissue -- most plausibly a source
form in which the second call's arguments are not the same values the first call
was given.
#### 2026-09-12, lane p23-lastmile5: the missing argument reissue is reachable

The baseline remains 253 words, delta zero, 39 masked and 51 raw differences,
frame 0x68. First masked mismatch is +0x58; the older +0x40 figure is raw.
Aligned buckets: 223 exact, 15 naming, two immediate, ten paired structural
rows, plus three candidate-only words at +0x17C, +0x314 and +0x3A8 against
three target-only words at +0x194, +0x32C and +0x3C0. The candidate object has
51 ELF relocation entries; the assembled target has 25. Literal-address
relocation artifacts require runtime-record and linked proof before any future
promotion; this pass makes no exact-relocation or match claim.

The complete same-save-kind landscape covers all 20 coloured webs and 135
accepted alternative forces: 86 preserve geometry and 49 grow. The exhaustive
delta-zero winners list is empty. There is no improving packing to combine;
the measured single-force floor including baseline is 39, not a bound over all
multi-force combinations or source shapes. Stock/instrumented fidelity passes
text, data, rodata, symbols and relocations.

The inherited global-reload experiments missed the actual parameter. On the
short-circuit-or shape, a volatile address read of the second call's `x1`
parameter restores all eight missing bytes: 253 words at delta zero, scoring
93. Its aligned buckets are 163 exact, 75 naming, two immediate and twelve
paired structural rows, with only the +0x17C/+0x194 unmatched pair remaining.
The two short-circuit surplus/missing pairs disappear. This overturns a broad
claim that the required argument reissue is not source-reachable; it does not
make this candidate exact, and its substantial allocation regression prevents
adoption.

Seven stock source probes are preserved under `build/p23/overlay2QueryNode/`.
The or-control and nonvolatile address read of `x1` both score 119, eight bytes
short. The volatile `x1` read scores 93 at exact size; making the whole `x1`
parameter volatile scores 136, four bytes short. A volatile address read of
`y1` instead scores 95 at exact size; making both reads volatile scores 132,
eight bytes long; subscript spelling of the `x1` volatile read is byte-flat
at 93. No body-shape edit was adopted. The last spelling is flat and the available
parameter-reissue variants are exhausted; retain the lower-residual guarded
body and the structurally informative diagnostics separately.

The next question is a narrowly scoped parameter-reload identity that retains
the reissue without changing allocation throughout the function. Another
volatile file-scope global or the already-closed boolean-carrier lattice does
not address that question. The overlay-2 SYMBOL-versus-JUMP trap remains a
mandatory per-call runtime-record check if a future source reaches zero.

Runtime relocation audit with `tools/overlay_tables.py`: the owned range has
51 records, comprising twenty LOCAL high halves, twenty LOCAL low halves,
ten JUMP calls and one SYMBOL call. The SYMBOL call is at function +0xC4,
resolving to overlay 2 +0x1364 (`overlay2IntersectSegments`). All ten other
owned calls are JUMP records. This is the same-module trap inside this
function itself: `overlay2IntersectSegments` already has an in-module C
definition. The guarded caller now names `overlay2IntersectSegmentsReloc`.
Fresh stock reproof retains all 253 owned words byte-identically, with 39
masked and 51 raw differences, and the relocation-table delta is solely the
callee identity at +0xC4. The colour landscape above precedes this metadata
binding correction; its source spelling and objects are preserved. The raw
runtime records remain private. A future promotion still requires the full
linked proof, even if its masked score reaches zero.

#### 2026-09-12, lane p24-recipe: leaf-cursor and returned-count deletion

The fresh stock baseline remains 253 words, delta zero, frame 0x68,
39 masked and 51 raw differences, first masked mismatch +0x58. Aligned
buckets are 223 exact, fifteen naming, two immediate and ten paired structural
rows, plus the three surplus/missing pairs already recorded above. The existing
20-web colour landscape was reused; its lack of a single-force winner is not
an unrestricted lower bound.

Three deletion probes fail to improve this shape. Replacing the walking line
pointer with a global-array index gives 257 masked at delta plus 24. That
global-read form is diagnostic only until the helper's global effects are
authenticated; the original captures the base before its calls. Using a
captured array-end pointer and indexing it by the negative remaining count
preserves the original captured base but still gives 251 at delta plus 24.
Deleting the returned count alias and deriving the zero-trip/return values
from remaining gives 234 masked at delta minus eight. These forms alter frame
or induction geometry throughout the leaf branch; none produces the wanted
zero-cost loop-exit value or changes the short-circuit argument-reissue
identity. Stop after these three non-improving probes under ADR 0018.

The original guard and the corrected overlay2IntersectSegmentsReloc name are
retained. Its owned +0xC4 site remains the SYMBOL relocation documented in the
preceding runtime audit; the other ten owned calls remain JUMP sites. No
placeholder is reverted, and no zero-masked or linked-match claim is made.
The candidate still has 51 ELF records versus the assembled target's 25;
runtime-record identity and linked bytes remain mandatory before promotion.

The named source question remains a scoped parameter reload that preserves
both the target short-circuit return and the second call's argument reissue.
The cursor recipe does not supply it: it changes the leaf loop instead. Sources,
objects, scores and aligned per-window deltas are preserved privately under
build/p24/overlay2QueryNode. No new matching bytes are credited.

#### 2026-09-16, lane nx-a: the OR block's target is a spilled scalar, and why the scalar form is eight short

Baseline reproduces 1,012 bytes, delta zero, 39 masked and 51 raw, frame
0x68; aligner 223 exact, 15 naming, 2 immediate, 16 really different, with
the three surplus and three missing words recorded above. Nothing adopted;
two cycles.

Reading: at the OR block the target stores the first call's result to 0x4C
before the second call and reloads it into a0 after, with the side pointer
load scheduled above the store. That is a spill of a scalar web coloured a0
to its own home, and as1 hoists a load above a spill store where it will not
hoist it above the candidate's array-element store. Seven scalar carriers on
this shape: a plain scalar, its swapped or-operands and its register form are
100 at minus eight; the compound-assignment form 72; the count carrier 45;
the leafResult carrier 243 at minus eight; the address-read form is byte
flat at 39. The census on the plain scalar says why: its web (save 2.0, total
4, nocs 2) is decided before the `&D_58` and `&D_5C` address webs (1.6) and
finds s0 free, already saved for the leaf branch, so it takes s0 with no
spill and the function loses one word per mirror block. In the target the
scalar was spilled, so it was decided after those address webs with s0 to s3
taken, and split to a0. The named decision is the scalar's save against 1.6.

Six forms of the tail-call child: the ternary, the ternary behind a region
marker, and the dead `line` pointer as carrier (assigned per arm or by
ternary) are all byte-flat at 39; a fresh `child` local is 70 with or without
the region marker.

Next hypothesis: a scalar `recursiveResult` whose web has save below 1.6 at
unchanged total, so it is coloured after the two address constants; nocs 4
at total 6 would do it. A probe is the diagnostic; the source form is the
question. Nothing here touches the short-circuit reissue or the leaf join.

<!-- plateau-handoff:overlay2QueryNode:end -->
