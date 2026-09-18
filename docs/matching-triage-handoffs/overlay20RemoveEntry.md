<!-- plateau-handoff:overlay20RemoveEntry:start -->
### `overlay20RemoveEntry` plateau handoff

- source: `src/overlays/o020/overlay20RemoveEntry.c`
- score: 2/53 words
- frame: frameless
- relocations: 10
- first mismatch: +0x6C
- summary: L145 delete-carrier stays the 6-word basin: store-forward keeps web 8. End-first inverts colours at 10-14 with a 4-word preheader shuffle. Baseline 2 of 53.

#### 2026-09-09, lane fin-misc: the tie is between two dead colours

Measured on a 64 ms direct-`tools/ido/cc` loop against the real object, scored
reloc-blind (the overlay post-process consumes most of the relocation entries,
so a name-comparing score is not the faithful one). About 250 candidates, every
one 2/53 at the same two words.

The colour order reads straight out of the object. The compaction bound is a
pool web, defined at +0x6C and dead at +0x7C. At that point both `entry`'s
colour (v0, last used at +0x2C) and the `&gOverlay20EntryCount` address web's
colour (a2, last used at +0x58) are free. The target reuses v0 -- the
earlier-freed of the two -- and IDO reuses a2, the more recently freed one.
Nothing in the emitted code keeps v0 live across the bound, which is why the
2026-09-08 proc-0 trace had to call the blocker an *invisible* web.

Newly eliminated this pass, each measured:

- All six declaration orders of the three locals, plus four extra-local shapes
  (a leading, middle and trailing dummy, and a `void **` cursor). Flat, which
  is the field guide's rule that declaration order does not drive colouring.
- A 63-cell colour-reserving dead-store sweep: `dead = 0`, `= i`, `= owner`,
  `= entry`, `= NULL`, `= gOverlay20EntryCount` and `= gOverlay20ShiftEntries`
  inserted at each of the nine statement positions. None of them reserves a
  colour here, so the `value = 0` reservation rule does not apply to this site.
- Eight compaction-loop spellings: re-reading the bound from the global (6
  words), `!=` and reversed conditions, unsigned bounds (14), a separately
  named end pointer (15), `++i` in the condition, and hoisting the bound above
  the guard (6).
- Five search-loop shapes and five marker-loop shapes, including the reversed
  entry comparison and a `for` marker loop (10 words, one instruction fewer).
- Use-side lock breaks on the bound, on `i`, on the store index and on the
  entry comparison -- the lever that closed `func_8003A2C8` the same day.
- Retyping the bound carrier as `void *` or `void **`, and folding it back onto
  `entry` so the two share one variable: 22 to 29 words, because the entry
  comparison's type changes with it.

The verdict is unchanged and now much better supported: no spelling of this
function reaches the target colour. The instrumented uopt capture -- a print in
the recompiled `globalcolor` showing the free list at the bound's web -- is the
only remaining lever.
- summary: p2 colours in ascending web number, so the decrement web (v0, web 8) is coloured before the limit (web 42) and forbids it v0; the cursor needs the same three webs the limit must not see, so the cursor-first ordering is self-contradictory and the open axis is the limit's web number

#### 2026-09-09 (second pass): the blocking web is named

The two words are still `addu v0,t0,t9` / `sltu at,a1,v0` against the
candidate's `a2`: the compaction loop's limit pointer. The instrumented uopt
identifies it as pool web 42. When web 42 is coloured its `forbidden0` is
`0x78000000` -- v0, v1, a0 and a1 are all taken by interfering, lower-numbered
webs -- and a2 is simply the lowest colour left. Its four interference partners
are web 37 (a1, the cursor), web 34 (a0), web 10 (v1, `i`) and web 8 (v0).

Web 8 is invisible. Forcing it to a3 changes no instruction anywhere outside
the compaction loop, so it holds v0 without emitting a word.
`CDX_FORCE=p2:w42=c1` is declined at both the decision and the colour site,
which proves the interference is real rather than a priority choice: no source
reordering that leaves web 8 where it is can reach the target. Freeing v0 by
forcing web 8 elsewhere is not the route either -- web 37 is coloured first,
takes v0, the limit takes a1, and the residual grows to six words. The target
needs web 8 not to *interfere* with web 42, not merely not to hold v0.

Newly eliminated, all byte-flat at two words: every dead-store colour
reservation on a spare `s32`/`u32`/pointer local at four placements and on
`entry` itself; a bare extra declaration of each type; `entry` typed as `s32`,
`u32`, `u8 *` and `void **`; a 120-row lattice of four declaration orders by
three search-loop spellings by four compaction spellings by two tail orders, in
which the limit register is only ever a1 or a2; the `owner`, `new_var` and
global-re-read bound carriers; and nesting the whole body instead of the early
returns. Deliberate extra pressure (keeping `entry` or `owner` live past the
loop) pushes the limit *up* to a3 and never down to v0, the same
one-directional signature.

Next: identify web 8's source construct (a `CDX_DETAIL_WEB=8` capture gives its
`bb`/`line` and neighbours) and remove its live range across the compaction
loop; that is the only remaining degree of freedom.

#### 2026-09-10, lane o7-tight: the ordering law, and the requirement stated exactly

The instrumented uopt was run and the reading in the two passes above is
wrong in one load-bearing detail, which changes what has to be searched.

**globalcolor's caller-saved sweep colours webs in ASCENDING WEB NUMBER, and
takes the lowest colour no already-coloured interferer holds.** Not in
descending `save`. All twelve of this function's `p2dec` records reproduce
their recorded `forbidden0` exactly under that order and under no other: web 0
is coloured first with only a0 forbidden (the incoming parameter register),
web 2 second with v0 forbidden (web 0), web 8 third with a0, web 10 fourth
with v0+a0, and so on to web 42 with v0+v1+a0+a1. Descending `save` predicts
web 0's forbidden set as v0+v1+a0 against the recorded a0 alone, so the two
orders are distinguishable and only one survives. The colour index is bit
`31 - c` of `forbidden0`/`available0`, and c1..c6 are v0, v1, a0, a1, a2, a3.

`save` is not the priority here; it is the colour/split gate. Web 63's
`save=0` is the only `decision=no-color` in the function.

**The lane also measured the other half of the law**, on
`overlay1ResolvePathPoint` in the same session: its nineteen **p1**
(callee-saved) decisions come in strictly descending `save` -- 30.5, 30.0,
20.0, 8.5, 5.0, 5.0, 3.0, 2.0, 1.5, 1.5, 1.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5 --
so L100's "descending save" describes p1 and does not describe p2. A lane
reasoning about a caller-saved residual from L100 will predict the wrong web
first every time.

**The web census, from `CDX_DETAIL_WEB` plus a `CDX_LINEAGE_TABLES=all`
capture that gives every web its occurrence blocks:**

- web 0, local at cfe -4, `entry`, blocks 0 and 4, coloured v0
- web 2, the parameter at cfe 0, `owner` (the count, then the marker pointer),
  blocks 0 through 12 and 16 through 19, coloured a0
- web 8, an expression web, the decremented count `owner - 1`, blocks 12 and
  13, coloured v0
- web 10, local at cfe -12, `i`, blocks 2 through 19, coloured v1
- web 16, an expression web, the search loop's cursor, blocks 3, 4 and 6,
  coloured a1
- web 34, local at cfe -8, `new_var`, blocks 13, 13 and 14, coloured a0
- web 37, an expression web, the compaction cursor, blocks 13 and 14, a1
- web 42, an expression web, **the compaction limit**, blocks 13 and 14, a2
- web 55, local at cfe -16, the `i--` post-decrement temp, block 19, v0

Web 8 is the "invisible v0 web" the earlier passes named. It is not the marker
loop's dead copy (that is web 55, and the earlier falsification of the dead-copy
reading was right); it is the decremented count. ugen re-materialises it into a
ring temp, so its colour never reaches an instruction, but it holds v0 and it
interferes with 42.

**The requirement, exactly.** The target wants the limit on v0 and the cursor
on a1. Under the law that means one of two orderings, and both are blocked:

- *cursor before limit* (the order every measured spelling produces). The
  cursor reaches a1 only if v0, v1 and a0 are all held by earlier interferers,
  which on this CFG means web 8 on v0, `i` on v1 and `new_var` on a0. Every one
  of those three also interferes with the limit, so the limit then sees v0
  taken and cannot have it. **The two conditions are the same webs, so this
  ordering is self-contradictory** -- which is the real reason the residual has
  survived three passes, and it is stronger than "web 8 must stop interfering".
- *limit before cursor*, which needs (a) no v0 interferer numbered below the
  limit and (b) exactly one a0 interferer numbered below the cursor. Reachable
  in principle: the numbering does invert (see below). What blocks it is that
  the decremented count is always numbered below the limit and always takes v0.

**Force oracle.** `CDX_FORCE=p2:w8=c6,p2:w37=c4,p2:w42=c1` is declined at both
the decision and the colour site with `forbidden=0x6a000000`. Freeing web 8
from v0 does not free v0 for the limit: web 34 (`new_var`) simply takes it, its
own forbidden set having been only v1. That is the trap the earlier "force web
8 elsewhere" experiment fell into, and it generalises -- **there is always
exactly one v0 holder in front of the limit; moving one only promotes the
next.**

**The numbering IS controllable, and that half is now solved.** Hoisting the
array base into a named local (`list = gOverlay20ShiftEntries;` before the
store, loop written over `list`) inverts the pair: the limit is numbered below
the cursor, and the cursor lands on a1 exactly as the target has it. Combined
with merging the decrement into `new_var` (`new_var = gOverlay20EntryCount;`
written straight after the store, which makes web 8 disappear), the candidate
reaches 5 differing words with the limit on a0 and the cursor on a1 -- one
colour away, and the a0 is `new_var` sitting on v0 in front of it. The hoist
also swaps the two index shifts, which costs three of those five words, so it
is not a free lever; it is the proof that the ordering axis is open.

**The one construct that moves the decrement off v0, and why it does not
close.** Any use of `entry` placed after the search loop and before the store
extends web 0 (v0) across the block that defines the decrement, and the
decrement then takes v1. Measured: the limit goes to v0 with that in place.
But every such use emits an instruction, and web 8 also interferes with `i`,
so `i` is pushed off v1 to a1 in the same move. Twenty zero-footprint reads of
`entry` at that position -- self-assignment, `if (entry)`, `if (entry ==
entry)`, `(void)entry`, dead stores into each local, `& 0` and `* 0` masks,
an `if (1)` region -- are all folded before the web builder and leave web 8 on
v0. The single-mention fold is total here, as it is on
`overlay1ResolvePathPoint`.

**Newly measured and flat at two words this pass** (~1,050 candidates, ~300 a
second, each compiled with the real per-TU flags and compared against the
whole 53-instruction target text rather than against a word count):

- 702 cells of `new_var` position (five) x bound source (global / `owner - 1`)
  x base-hoist position (four) x loop form (six: bottom-tested, top-tested,
  `++i` in the condition, reversed comparison, and two pointer walks with the
  end pointer written first and second) x declaration order (three);
- 176 cells of guard spelling (eight, including `!=`, reversed operands, an
  unsigned compare and `i - bound < 0`) x bound carrier (four) x loop form
  (four) x store placement (before the guard / inside the `if` / after it);
- 168 cells of the earlier bound/loop/hoist product, re-run at this base.

Declaration order is byte-flat in every one of them, as the earlier passes
found; the reason is now visible in the trace, which is that cfe stack offsets
do not participate in the p2 ordering at all.

**Next lever, and it is now a narrow one.** The open question is no longer
"why does web 8 interfere" -- it interferes because block-level liveness puts
both it and the limit in the same block, and its last use *is* the limit's own
operand, so no C can separate them. The open question is whether the
decremented count can be given a web number **above** the limit's while staying
in the same block. That is a ucode-position question: web numbers here follow
the order uopt creates the symbols, and the compaction region's symbols are
created by strength reduction after the source-level ones. If a spelling exists
that makes the bound a strength-reduced symbol rather than a source symbol --
so that it is numbered with the limit and the cursor rather than in front of
them -- the limit takes v0 and the cursor takes a1 with no other change. A
`CDX_LINEAGE_TABLES=all` capture on any candidate prints the creation order
directly, so the next lane can screen spellings on the trace instead of the
score.

#### 2026-09-10, lane `w8-tu`: region boundaries do not reach the numbering, on either base

Baseline reproduces at two words, and so does the five-word hoisted candidate
the pass above describes, which makes both bases available to check the one
axis the record does not list.

**uopt region boundaries are inert here.** `if (1) { }` inserted at every
statement boundary of the function, and `if (1) { ... }` wrapped around every
simple statement -- 44 compiling cells -- leaves the baseline at exactly two
words in 36 of them and regresses in the rest; none goes below. The same sweep
over the hoisted five-word candidate, 28 compiling cells, is flat at five. So
the lever that redraws colouring inside a region on other functions does not
renumber this one's webs, which is consistent with the requirement being an
ordering between two webs in the *same* block rather than a colouring inside
one.

**Merging the decrement into the parameter is not the route either.** Writing
the count-down as a pre-decrement of the reused parameter, so that no separate
web is created for the decremented value at all, is 19 words; the same with the
bound still carried in a local is 18, with a post-decrement 37 at +4 bytes, with
a separate subtract statement 19, re-reading the global for the guard 10, and a
top-tested loop 19. Every one of them is worse than the baseline, and the two
that keep the size move the first mismatch earlier rather than later.

**Loop-body spelling under the hoist is flat.** Six spellings of the compaction
copy -- subscript both sides, pointer arithmetic both sides, the two mixed
forms, and two address-of-element forms -- crossed with the hoist placed before
and after the count store, are all seven words, so the three words the hoist
costs are not a property of how the copy is written.


#### 2026-09-11, lane p6-tight: confirmed unchanged, no new search

Re-measured against the current tree: 212 bytes, 53 of 53 words, size delta 0,
positional masked 2, aligner buckets 51 byte-exact, 2 register naming, 0
immediate only, 0 really different, first naming-only difference +0x6C. One
source register maps to one target register at both sites, 100 percent
coherent, one window, no closed cycle -- the compaction limit, as recorded.

Nothing was searched this pass. The 2026-09-10 closure is a contradiction
argument rather than an exhaustion claim: under the measured ascending-web-
number order, the cursor reaches its target colour only when the same three
webs that must not forbid the limit are coloured in front of it, and there is
always exactly one holder of the wanted colour numbered below the limit, so
moving one only promotes the next. The numbering axis is open and priced at
three words; nothing in this lane's levers reaches it.


#### 2026-09-12, lane p10-tight: the force closure re-established with CDX_PROC set

`CDX_FORCE` is silently ignored unless `CDX_PROC` is set, and a dropped force is
indistinguishable from a legitimate decline, so the 2026-09-09 and 2026-09-10
force results were re-run here with the procedure selected and the accepted flag
read rather than the object compared. The instrumented compiler was gated first:
its text for this symbol is byte-identical to the configured build's.

**The closure survives, and the tool was demonstrably driving.** Forcing the
compaction limit onto the first colour records the never-applied value and
leaves every colour in the procedure unchanged, and the same run's decision
record carries that colour in the limit's forbidden mask, so this is a genuine
decline. In the same harness, forcing the decremented count onto the sixth
colour IS accepted and records it, which is the positive control the earlier
entries lacked: the mechanism works, the procedure selection is right, and the
decline is real.

**The promotion chain is confirmed and now enumerated.** With the decremented
count forced away, the bound carrier takes the first colour and the limit lands
one colour better at six words. With both the entry web and the decremented
count forced away, the index takes the first colour and the limit lands in the
same place at 33 words. So "there is always exactly one holder of the wanted
colour in front of the limit, and moving one only promotes the next" is now
measured over three configurations rather than one.

**The full p2 ladder, for whoever picks this up.** Twelve decisions in strictly
ascending web number, taking the lowest colour no already-coloured interferer
holds, with the entry web and the decremented count both on the first colour,
the index on the second, the search cursor and the compaction cursor on the
fourth, the bound carrier on the third, and the limit on the fifth. The limit's
forbidden mask names the first four colours.

**The requirement, stated exactly and three-sided.** The target wants the limit
on the first colour and the compaction cursor on the fourth. That needs, all at
once: the decremented count gone so it cannot hold the first colour; the limit
numbered BELOW both the bound carrier and the compaction cursor, because both of
those live in the compaction loop's two blocks and therefore interfere with the
limit, so nothing outside those blocks can forbid them a colour without also
forbidding it to the limit; and exactly one interferer numbered below the cursor
holding the third colour. The recorded array-base hoist buys the second of those
three and costs three words in the two index shifts, which is why that candidate
sits at five. Nothing measured here buys the first and third together.

#### 2026-09-12, lane p21-two: fresh L154 probes and force receipt

Fresh alignment at the assigned base reproduces 212 bytes, 53 of 53 words,
zero size delta, with buckets 51 byte-exact, 2 register naming, 0 immediate
only and 0 really different. The two sites remain one coherent `a2` to `v0`
mapping at `+0x6C`; the candidate is frameless with ten relocations.

Four source probes were run against the configured full-TU command. A global
pre-decrement regresses to 52 of 53 words with a four-byte size excess and
register/structure churn. Computing `owner - 1` into `new_var` before the
global store preserves size but regresses to 34 exact words and 19 masked
differences. The unsigned-literal and addition-negative-one decrement
spellings preserve the baseline at 2 masked words. No candidate was adopted.

The instrumented compiler was identity-gated by comparing its `.text` section
byte-for-byte with stock. With procedure ordinal 0 selected, forcing web 42
to the target's first colour is recorded as declined (`forced=-2`) and the
forced object scores 2 directly. Forcing web 8 to the sixth colour is accepted
and the direct object score is 6. This confirms the force path and leaves the
three-sided web-creation-order requirement as the precise remaining decision;
no further source spelling was found that changes it without paying size or
altering the loop's other colours.

#### 2026-09-12, lane `p23-lastmile4`: current zero-web landscape

The configured overlay baseline remains 212 bytes, 53 words, delta 0,
frameless, ten relocations, and 2 masked words. The aligned bucket split is
two naming rows only, both the same a2-to-v0 substitution in the opening
window; there are no immediate or structural rows.

The fresh exhaustive footprint reports zero legal coloured-web probes for this
procedure, so its winners list is empty by construction. This eliminates a
new single-force colour pass as an action rather than merely repeating a flat
score. The named source question remains whether a zero-width source form can
alter the pre-colour web creation order described in the retained L154 work.
#### 2026-09-13, lane g1: draw-difference controls

The base-only assignment and configured baseline reproduce 212 bytes at zero
size delta, 51 aligned exact words and two naming rows at +0x6C. Stock and
instrumented full-TU text compare identical. The baseline spends 11 scratch
draws and 130 emission events. No colour sweep was repeated.

Three source controls were retained independently under ignored
build/g1/overlay20RemoveEntry with source, object, trace and census evidence.
Computing the bound as owner minus an unsigned one adds a draw at the guard,
with no net emission-event increase, but adds an executable word: the aligned
residual rises to 13 naming, five immediate and two structural rows, plus one
candidate-only offset. Removing the bound carrier preserves all 11 draws and
their order while removing two emission events; it regresses from two to six
naming rows at equal size. Folding the copy and increment onto one source line
preserves the complete draw order and the two-row residual; six emission events
change line attribution without changing the object.

The typed expression reaches the draw schedule at an instruction cost; carrier
removal changes colouring without moving the scratch draws; the line fold does
not move the allocator. None removes the recorded pre-colour web-order conflict.
The earlier exhaustive source and lineage evidence still blocks that mechanism,
so the baseline is retained and the packet stops without another spelling or
colour sweep. This is a guarded plateau with no new matching credit. Setup
failures before editing are excluded from the three source controls; the shared
checkpoint counter remains monotonic. Commands: draw_census.py --save/--compare,
residual_map.py --object/--against, align_symbol.py and finalize_plateau.py.

#### 2026-09-17, lane w3-o020: the named L154 lever is ruled out at zero width

Configured baseline reproduces 212 bytes, 53 words, delta 0, frameless, ten
relocations, two masked words, 51 aligned exact and two naming rows at +0x6C.
Stock and instrumented full-TU text compare identical. Procedure ordinal 0,
twelve p2 decisions. Baseline lineage creates the decremented-count expression
(type 4) before strength reduction mints the compaction cursor and limit; that
count is live in the store block and the loop, so it is coloured first and
holds the first colour.

The named question was whether an index-shape spelling can make that count a
strength-reduced symbol numbered with, or above, the limit at zero width.
Screened on creation order, not on score. No colour landscape and no exhausted
declaration, region, or top-tested product was repeated.

- For-loop, delete the bound carrier, bound is the stored global. The
  decremented-count expression is still created at the store, before the cursor
  and the limit. Size grows by two words. Does not delay the count.
- For-loop, first use of owner minus one is the guard, store after the loop.
  The count is still created before strength reduction, because the guard is
  the first use. Size grows by two words.
- For-loop, bound written as i plus one against owner. The limit web
  disappears; a different early expression takes the first colour. Size grows
  by two words.
- Bottom-tested loop, guard spelled without owner minus one, bound is owner
  minus one in the tail, store after. This is the hypothesized move: the count
  joins the strength-reduction cluster. It is created between the cursor and
  the limit, never above the limit. The limit is minted from that bound, so
  the bound ICHAIN has to exist first. Size grows by two words. The cursor
  takes the first colour; the limit still cannot.
- Bottom-tested, delete carrier, store before, bound is the global. Zero
  width, six words. Confirms the recorded carrier-deletion basin: the count is
  still early, the cursor takes the third colour, the limit the fourth.
- Global pre-decrement: the count expression is created even earlier and its
  live range reaches the search loop. Size grows.
- Assign owner minus one to the bound local and store after: the type-4 count
  dies in the store block and no longer shares the loop with the limit, but
  the type-3 bound local takes the first colour in its place. Size grows by
  one word. Same substitution: exactly one holder of the first colour in front
  of the limit.
- Merge the decrement into the parameter at zero width: the type-4 count is
  gone, 21 words. Cursor takes the first colour, limit the fourth. The owner
  parameter stays live through the loop.

Zero-width conclusion: the bottom-tested shape always uses the bound before
the loop (the store, the guard, or both), so a source-level decremented count
is always created before strength reduction. A bound cannot number above the
limit that strength reduction creates from it. Same-block and above-limit
together are contradictory for that pair. Baseline retained at two of 53
words. The next concrete action is not another index-shape first-use spelling;
the numbering axis that remains is whatever can mint the limit before the
cursor without paying the recorded array-base hoist, which this packet does
not have.

#### 2026-09-18, lane w16-o020: L145 delete-the-carrier does not free v0 at zero width

Configured baseline reproduces 212 bytes, 53 words, delta 0, frameless, ten
relocations, two masked words, 51 aligned exact and two naming rows at +0x6C,
one window, a2 to v0, no cycle. Stock and instrumented full-TU text compare
identical. Procedure ordinal 0. Forcing the limit onto the first colour is
declined (forced=-2) and scores 2; forcing web 8 onto the sixth colour is
accepted (forced=6) and scores 6. Draw census: 11 draws, 130 emissions.

Authorized lever was L145-L154, especially delete-the-carrier for the owner-1
ring temp, without minting a new strength-reduced limit from that bound.
Creation order still makes web 8 a type-4 expression in the store block before
the cursor (type 4) and the limit (type 4); new_var is the type-3 bound
carrier. No declaration lattice, no loop-form product, and no p2:w8=c6 search
on the incumbent shape was repeated.

- Delete new_var, bound is the stored global: zero width, six naming rows.
  Web 8 survives by store-forward, still on the first colour; the cursor takes
  the third, the limit the fourth. Address-form store is byte-identical to
  this cell, so taking the global's address does not break forwarding here.
  Volatile store breaks forwarding and grows four bytes / 36 words.
- L151 unsigned bound without a carrier: four extra bytes, 36 words.
- i plus one against owner (no owner-1 web, no limit web): zero width, 20
  words. Cursor lands on the fourth colour; the pointer-limit compare is gone.
- Merge decrement into the parameter: 19 words, as recorded.
- Global pre-decrement / DKR gCount--: 52 words and plus four bytes, or six
  words if the search still copies the count into owner then deletes new_var.
- Reuse entry as the array base after the store: 17 words.
- Declared end pointer from owner, not owner-1: plus four to plus 28 bytes.
- On the delete-new_var shape, forcing web 8 to the sixth colour is accepted
  and still scores 6: the cursor takes the first colour, the limit the third.
  The promotion chain survives the carrier-shape change (L146).
- End-first pointer compare while (&arr[gCount] greater than &arr[i]) kills
  web 8 and numbers the limit below the cursor. Limit takes the first colour,
  cursor the fourth (the target pair) at 14 words, delta 0, but with four
  candidate-only and four target-only preheader words. Cursor-first pointer
  compare is the same 14. Indexed body plus end-first tail returns to the
  six-word basin; adding an end-first guard on top of the indexed loop grows
  sixteen bytes.
- Declared limit pointer plus indexed copy: 10 words, delta 0, same four-plus-
  four preheader shuffle, limit on the first colour and cursor on the fourth,
  leftover naming is a ring cycle not the limit colour.

Zero-width conclusion: deleting the s32 bound carrier leaves owner-1 live as
the forwarded store value, so it is still a type-4 web numbered below the
limit. Breaking that forward, or writing the exclusive end from owner rather
than owner-1, either grows size, destroys the limit web, or inverts colouring
only by shuffling the preheader four words against the target. Target colours
and the target preheader were not observed together. Baseline kept at 2 of 53.
Next is not another delete-carrier of owner-1 or new_var; it is a spelling that
emits the indexed-SR preheader while creating the limit ICHAIN first, which
this packet's hybrids did not.

<!-- plateau-handoff:overlay20RemoveEntry:end -->
