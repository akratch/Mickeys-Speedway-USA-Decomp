<!-- plateau-handoff:overlay1BendPathPoint:start -->
### `overlay1BendPathPoint` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 19/107 words
- frame: 0x30
- relocations: 6
- first mismatch: +0x10
- summary: Empty-if L100 spills index to parameter home; jal delay still holds the selector mask, and currentIndex still refuses v1.

#### tu2-o1tail: the u8 parameter's spill slot is the residual, and it is not source-reachable so far

Re-measured at the assigned base: 25 masked words, 107 of 107 instructions,
0x30 frame exact, first mismatch +0xC. Two mechanisms were isolated and both
resisted every form tried.

First, the spill slot. The target spills the `index` parameter to the byte of
its own incoming argument home (sp+59, the low byte of a2's slot at sp+56) and
reloads it there after the `overlay1GetPathReloc` call. The candidate's
`volatile u8 localIndex` hack produces the same store/reload pair, but into a
separate local at sp+25. Removing the hack so IDO spills the parameter itself
does not work: the instruction count drops to 106 and the residual explodes to
104 words. Also eliminated: `index = *(volatile u8 *)&index` after the call
(108 instructions, 105 words), a `volatile u8 index` parameter (108, 103), and
dereferencing the address of the volatile local (108, 106).

Second, the index-block colours. The target colours currentIndex v1,
previousIndex a0 and the path-count carrier a1; the candidate has previousIndex
v1, currentIndex a1, count a2. Eliminated: writing `currentIndex = index` in
the else branch where index is known zero, which does reproduce the target's
`move v1,a2` but costs 97 words overall; ordering the else branch with
currentIndex first (flat at 25); folding the decrement into
`previousIndex = path->count - 1` (26); and spelling the decrement as
`previousIndex = previousIndex - 1` (flat).

Three consecutive attempts on each mechanism produced no better residual and no
new identity, which is the stall. Next lever: the parameter-home spill is an
allocator decision, so it needs ugen ring or emit-order instrumentation rather
than another source form.

#### lm-o1tail: the parameter-home spill is source-reachable after all

The previous record called the spill slot an allocator decision needing ugen
instrumentation. It is not: `*(u8 *)&index = index;` written before the
`overlay1GetPathReloc` call reproduces it. Taking the parameter's address makes
IDO keep `index` in memory, and the redundant store lands on the byte of the
parameter's own incoming argument home -- the exact slot the target uses -- with
107 instructions and the 0x30 frame both held. It also puts the third argument's
save into the target's out-of-order position, second rather than third, so the
prologue's first four words become exact.

Two things it does not yet buy. The store is scheduled before the call rather
than into its delay slot, where the target puts it and where this form puts the
selector mask instead; and the reload lands in a ugen ring temp rather than the
argument register the target keeps it in, which shifts the index-block colours
downstream. Positionally that form scores 48 words against the retained hack's
25, so the `volatile u8` local stays as the candidate -- but the hack cannot
reach the target's slot at all, because its local is allocated in the local area
(sp+25) and only the parameter itself lives in the argument area. The address
form is the structurally correct route and the hack is not.

Measured and eliminated from this base, do not repeat: `volatile u8 index` as
the parameter, with the value read directly, read once into an `s32`, read once
into a `u8`, and read before the call (108 to 110 instructions); a plain
self-assignment and `*&index = index`, both eliminated before codegen (106);
`*(volatile u8 *)&index = index` before, after, and on both sides of the call
(109 to 111); a `u8 *` pointer local carrying the address (107, same 48-word
basin as the direct address form); reading back through `*(u8 *)&index` or a
named `u8`/`s32` local after the store; the volatile local declared first and
last in the list (frame 0x38 and 0x40 -- its position moves the frame); and the
plain no-hack form, which is 106 instructions because it reloads from the
argument home without ever storing to it.

Next lever: the delay-slot choice between the parameter store and the selector
mask, and then the reload's carrier. Both sit on the same three instructions.


#### p8-o001: the address lever gives the reload but not the store

`overlay1MeasureCurves` matched in this lane on the general form of what the
previous record here was reaching for: taking a parameter's address makes it
memory class, so every read is a load from its argument home and -- unlike
`volatile` -- no scheduling edge is emitted. Applied here it does not close the
gap, and the measurement says why.

Reading `index` through `*(u8 *)&index` at all four of its uses, with the
`volatile u8 localIndex` hack removed, produces 106 instructions against the
target's 107 and scores 104. Adding `*(u8 *)&index = index;` before the call
restores the count and scores 55. The store is therefore load-bearing and the
reads are not: the target genuinely stores the parameter to the byte of its own
home at `sp+59` and reloads from there, and address-taken reads alone give the
reload without the store. Also measured: the same address reads alongside the
retained volatile local (105, delta +8), and the selector read through its
address as well (104, delta -4).

So the previous record's ranking stands -- the retained hack at 21 words, the
structurally correct address form at 55 -- and the open question is unchanged
and now sharper: it is the *store*, not the reload, and specifically whether
as1 can be made to put that store in the call's delay slot where the target has
it, instead of the selector mask. Nine physical-line arrangements of the store
and the call were measured from the address-form base and every one is byte-flat
at 55: the two statements folded onto one line in both orders, one and two blank
lines between them, the store after the call, the store written volatile, and the
selector argument cast at the call. L59's line-number tie-break is live in this
unit but does not reach this pair.
#### 2026-09-12, lane `p9-tight`: the index block is not a colour question

The records were read on this function for the first time (instrumented object
`cmp`-identical to the configured one; this TU's ordinal is **proc 36**, the
37th `.ent` in the `cc -S` listing). The four index webs are identified by
forcing each to c9 and reading which sites move:

- **web 22** is `currentIndex` -- ours `a1`, the ROM's `v1`
- **web 25** is `previousIndex` -- ours `v1`, the ROM's `a0`
- **web 99** is the path-count carrier -- ours `a2`, the ROM's `a1`
- **web 44** is `nextIndex` -- ours `v0`, the ROM's `a0`
- **web 2** is the reloaded `index` parameter, and its `a2` already agrees

**`currentIndex` is denied `v1` outright.** Its `forbidden0` is `0x70000000` --
c1, c2, c3 -- and a force onto c2 reads `forced=-2`, never applied. The
decisive experiment is the one L114 asks for before calling a lever
unreachable: free the interferer first. Forcing webs 79, 49 and 25, every other
web holding `v1`, onto `t0`, `t1` and `a0` in the same run leaves
`currentIndex`'s mask **bit-for-bit unchanged** and the force still declined.
So the denial is not interference with a coloured rival, and no ratio,
spelling, statement order or force reaches the ROM's assignment. The live range
itself has to change.

That overturns this shard's framing. "The index block's pool colours" is not a
pool-priority question and the three previous records' plan -- reproduce the
ROM's `v1`/`a0`/`a1` triple by moving the assignments around -- cannot work on
this shape.

**A 36-point single-force sweep confirms it from the other side.** Twelve
integer webs against c2, c3 and c4: fifteen forces accepted, twenty-one
declined, and **not one scores below 21**. The best accepted alternatives are
24 and 26; `p1:w2=c2` costs 56 and `p1:w76=c4` costs 55 at delta -4. The
nearest joint force -- `previousIndex` to `a0`, count to `a1`, `nextIndex` to
`a0` -- reaches only **23**, worse than the retained body, because
`currentIndex` still refuses `v1`.

**L146 applied to the address-form route.** This shard records the four-pointer
declaration order as an identity, "the only order that lands both spilled stack
homes", measured over all 24. That climb was done on the `volatile u8` local
shape. Under L146 it is void on the address-form shape, which is the
structurally correct route for the byte spill, so it was re-climbed there: all
24 orders score **55 to 59**, the best two equal to the address form's own 55.
The address route does not become competitive under a fresh order climb, and
the retained hack at 21 keeps the body.

**Next lever:** a structural change to `currentIndex`'s live range, not a
colour. The mask to watch is web 22's `forbidden0`; anything that does not move
it cannot reach the ROM.

#### 2026-09-12, lane p23-lastmile5: authenticated full colour landscape

Fresh baseline is 21 differing words, not 86 differing words: 86 is the
agreeing-word count. Geometry is 107 words, delta zero, frame 0x30, six
relocations, first mismatch +0xC. Aligned buckets: 86 exact, 15 naming, one
immediate, five structural, with no surplus/missing words. The byte home is
still +0x19 in the candidate and +0x3B in the target; the other nine homes
agree. No source was adopted.

Captured Ucode authenticates procedure 35 of 38 in this checkout. The inherited
ordinal 36 now names another procedure; its initial scan is quarantined and
not used as evidence. The corrected scan covers all 15 coloured webs and all
129 legal alternative same-save-kind requests. Of these, 115 were accepted at
delta zero, one accepted force adds four bytes, and 13 were refused by the
acceptance checker for lacking the required final record. Stock/instrumented
fidelity passes all five section, symbol and relocation gates.

The complete accepted delta-zero winners list is empty. The single-force floor
with the baseline is 21; there is no improving packing to combine. This is not
a proof of an unrestricted multi-force floor. Six stock source probes address
the still-open current-index identity: unsigned next-index literal, unsigned
previous-index decrements, and an explicit unsigned comparison cast each stay
21; a byte or halfword current-index local scores 101 with four extra bytes;
an unsigned word local scores 52 at delta zero. Thus changing only this
carrier's width or arithmetic-literal identity does not supply the required
range. The retained source, every probe and complete force receipt remain
private under `build/p23/overlay1BendPathPoint/`.

Stop on the exhausted literal/type mechanism and the earlier range-mask and
parameter-home evidence. The next source question is a current-index range
that permits the wanted caller-saved register while also moving the byte spill
into the parameter's own home. The volatile local cannot satisfy that second
requirement, and these type changes cannot satisfy both together.

#### 2026-09-12, lane p24-recipe: deleted index and point carriers

The configured baseline reproduces 21 raw/masked differences in 107 words,
delta zero, frame 0x30 and six relocations. Aligned buckets are 86 exact,
15 naming, one immediate and five structural rows; first mismatch +0xC.
The authenticated procedure-35 landscape in the preceding shard was reused.

Three deletion probes do not improve it. Using the byte parameter directly
in place of currentIndex removes one required word, scoring 97 at delta minus
four. Inlining current-point accesses costs 20 bytes and scores 106. Inlining
previous-point accesses retains size but scores 66, introduces two unmatched
pairs and increases the aligned residual by 41 rows. The intermediate objects
and their per-window deltas are retained under build/p24/overlay1BendPathPoint.
The current-point form is a source-shape diagnostic only: promotion would also
need the angle helper's lack of mutation authenticated before substituting
repeated path reads for the saved pointer.

The recipe fails to preserve both geometry and the byte-home surface here.
Deleting currentIndex removes a required transfer, while deleting a point
pointer lengthens the index/address lifetimes instead of recovering the
wanted short current-index range. None repairs the parameter-home displacement.
After three non-improving forms without a new target identity, stop under
ADR 0018 and retain the original guarded candidate. The source question is
still the parameter-owned byte store together with a current-index lifetime
that permits the target register; neither an isolated type change nor deleting
these carriers supplies both.

#### 2026-09-19, lane w34-o1tail: empty-if L100 closes the byte home, 21 to 19

Identity-gate: stock and instrumented function text and relocations are
byte-identical. This TU indexes overlay1BendPathPoint at proc 35 of 0-37
(15 coloured webs, 23 p1 decisions, no p2). Frame 0x30, 107 words, six
relocations, size delta 0 throughout the adopted form.

L160 indexed `path->points[i]` (AdvanceGauge transfer) regresses: dropping
the three point pointers is plus 8 to plus 12 bytes and 107-108 masked.
Inlining next alone is plus 8. `path->points + i` is byte-flat with the
subscript. There is no walking cursor; L160 is not the lever.

Leftover OR-zero on index (and xor-0, plus-0, comma-assign of the same store)
reproduces the parameter-home byte store and the out-of-order third-argument
save, but spends a deleted identity draw (L149) that rotates the temp ring
for 52 masked words. Address-form store is the same basin at 55. Dual
`currentIndex = index` in the else still hoists and drops a word (96-101)
on leftover, volatile, goto, L97, and previousIndex-tested shapes.

Overlay22 empty `if (index) {}` on the plain no-hack body (no volatile
local) is the closer for the home: L100 weight makes uopt spill the u8
parameter to sp+59 without an identity op, so t8 on the first shift agrees
and every frame slot matches, including the target-only +0x3B byte. Adopted
score 19/107 words, first mismatch +0x10. Aligned 90 exact, 15 naming, 0
immediate, 3 structural, displacement tax 1. Candidate-only word at +0x18
against target-only at +0x10: the selector mask still sits in the jal delay
and the byte store still sits before it. The 15 naming rows are the same
coherent four-cycle (currentIndex, previousIndex, count, nextIndex) plus a
one-window t-ring pair. `if (index != 0) {}` is plus 12 bytes; do not use
the compare form.

Volatile u8 local is retired: its store can never reach the argument area
and it pinned the store before the jal (L144 edges). Plain u8 local puts
the store in the delay slot but at sp+25 and reloads a ring temp (58).
L99 pad pointer on the empty-if shape is 35. Selector leftover / volatile
sel on this shape returns the 52-basin extra draw or adds a word.

Next lever: schedule the selector mask before the remaining argument saves
so the parameter-home store can take the jal delay, then a currentIndex
pair of assignments that IDO will not hoist. Empty-if line placement, plus-0
and cast of the selector, empty-if on selector, do-while-0 around the call,
and comma of index into the call are byte-flat at 19 on this shape.

#### B3-o001 (2026-09-23): typed-view copies and spill placement, all measured

Re-measured 19 at delta 0. The ROM copies the reloaded index into
currentIndex (`move v1,a2`) in both arms; every same-IR spelling is hoisted
into one move (103 at delta -8 with the copy last or first in the else arm,
99 at -4 with the count decrement folded). The typed-view else-arm copies the
last handoff proposed were measured from this base: `(s8)`, `(s16)` are 101
at +12; `(u16)`, `(u32)`, plain parentheses, minus zero and or-with-zero are
103 at -8 (folded back to the same IR); `& 0xFF`, `& 0xFFU` and xor-with-zero
are 99 at +4 (a real extra word). None keeps two copies at delta 0. The
empty-if spill moved after the call, folded onto the call's line, behind an
empty `do {} while (0)`, or joined by an empty `if (selector)` is byte-flat
at 19: the store-versus-mask delay-slot order is not reached by the placement
of the L100 probe. No source edit adopted.

<!-- plateau-handoff:overlay1BendPathPoint:end -->
