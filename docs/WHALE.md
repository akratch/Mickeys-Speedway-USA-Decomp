# Harpooning `func_overlay_058_F000138C_18B0574`

## Current checkpoint: 6, with W closed on a pointer walk

Lane wv-x retains **6 at delta zero, unforced** (3611 / 0 / 0 / 4, frame
0x138, 1,253 relocations, first mismatch +0x1314). Case 12's title loop
walks a pointer initialised inside the guard from the `D_o058_5C80`
lineage, whose hull already spans the window, so no index web touches
blocks 183/184/190 and both growth tests read as the 48 body's: W accepts
185, rejects 191, keeps 202 at margin zero and colours `t0`; the 5E9C
piece accepts 191 at margin zero and rejects 202. wv-t's pointer cell had
lost only to the `&D_o058_5C98` remainder counting at 184. The residue is
one instruction's schedule -- the title-loop load emitted first in its
block where the target has it in the call's delay slot -- because a load
through a pointer variable carries no `.noalias` fact (its base is
`isvar`/may-alias in the alias profile, the strength-reduced base is
`islda`/no-alias) and as1 cannot sink it past the argument store. An
indexed named array has the fact but needs its index known zero at the
preheader, and that is now a three-way conflict measured to the cell: the
delay-slot `i = 0` lives only through a loop phi, the title init folds
only from a redundant in-block def (which takes the title phi), and the
row init likewise (which takes the row phi); bodies holding any two of the
three are banked (`c12a`: the head exact through +0x1370 at +8; `c13a`:
both inits at 50). Fifteen cycles; the
[pointer-walk report](whale-pointer-walk.md) has the ledger.

## Previous checkpoint: 9, with the slot rows closed and W a closed system

Lane wv-w retains **9 at delta zero, unforced** (3605 / 9 / 0 / 0, frame
0x138, 1,253 relocations, first mismatch +0x12E4). The two slot-address
rows close on the spelling a matched sibling uses: `overlay60Prefix.c`
writes `i = f(); slot = (SavesSlot *)((u8 *)slots + i * 32)`, and the
operand order of the `addu` is the expression tree's evaluation order (the
byte form through a declared local evaluates the base first; `&saves[f()]`
and `&saves[i]` the index first), not a canonicalised sum -- wv-v had
measured the two halves of that spelling separately, each inert. What
remains is W alone, and it is now arithmetic: wv-t's growth rule
(`2*left_after >= numintf + new`) holds on all 477 tests, the neighbour
lists of W and the 5E9C piece are itemised and every member is the
target's, and the three decision variables that would make W `t0` while
the 5E9C piece keeps rejecting 202 -- w40 above 3.6, the `&D_o058_5EA0`
remainder above 3.0 at its 183 seed against a seed walk that pops switch
arms in descending case value, and the 5E9C lineage's 202 seed before W's
183 seed -- are each closed at zero width, with alias diagnostics showing
the seed moving exactly as the model says. Ten cycles; the
[growth-margins report](whale-growth-margins.md) has the ledger.

## Previous checkpoint: 11, with the reset web coloured by its symbol

Lane wv-v retains **11 at delta zero, unforced** (3603 / 11 / 0 / 0, frame
0x138, 1,253 relocations, first mismatch +0x12E4): wv-u's forced witness
reached from source. The seven folded loop-index resets are dead defs that
uopt colours as one web per symbol, and on a symbol of their own they take
`a0` -- nothing decided before them holds `a0`-`a2` at any preheader, no
preheader has a call, and a dead-def web's save (n / (((n-2)>>2)+2), from
L29) never falls below the webs it would have to follow. So the resets are
written on locals that are dead where the loops run and already coloured
harmlessly elsewhere, and they inherit those colours: `textY` (`s4`) for
cases 3/13, `letter1` (`s1`) for cases 1/2 (two carriers, because the
`D_o058_5EF8` address web ties the entry loop to both draw loops and
`textY` is case 1's y). That closes case 13's seven rows and the `+0x0`
cascade together. What remains is W (nine rows) and the two slot rows; on
this body W's identity is unchanged and itemised from neighbour captures,
and its lever is `w40`'s `v1` decided after W's own split (not before the
5E9C piece): freeing `v1` by force, a late carrier live at 191, and the
unused-declaration and slot-spelling axes are measured closed. Eleven
cycles; the [index-carriers report](whale-index-carriers.md) has the ledger.

## Previous checkpoint: 18, with case 13 closed under one colour (forced)

Lane wv-u retains **18 at delta zero**; no unforced cell measured below it
and the source is unchanged. It itemised wv-t's growth identity from the
banked neighbour captures -- the two case-12 pieces share every window
interferer but W's own lineage remainder, and their held colours differ by
`v1`/`a0`/`a1` timing only, so no dead-carrier body passes both tests and
the target is the 48 configuration on the allocator side -- and measured
every zero-width handle for the one lever inside it (w40's order against
the `&D_o058_5E9C` remainder) closed: head probes insert blocks, aliasing
splits a value web or poisons alias analysis, bare expressions vanish. On
case 13 it derived the target's roles from the ladder (only `opponent` can
beat the cursor's 15.5) and found the zero-width shape that keeps the `a2`
piece at 28: loop 1 indexed by portraitX, reset folded at 235. On that body
portraitX's web is seven folded resets and takes `a0`; **forced to `a3`,
`t0` or `s4` the body measures 11 at delta zero** (3603 / 11 / 0 / 0: W's
nine rows and the two slot rows). The [split-order report](whale-split-order.md)
carries the itemisation, the seventeen cycles, and the constraint set the
harmless colouring must satisfy.

## Previous checkpoint: 18, with the gap closed on a split rather than a colour

Lane wv-s banks **18 at delta zero**, down from 48, and the last gap pair
(candidate +0x12F8 against target +0x1260) is gone with its 30-word
displacement tax. Three edits in case 12: the visible index is reset through
the first title-colour call's last argument (wv-i's value-producing
spelling, which lands it in the delay slot), the title loop subscripts by a
*dead* carrier (`textY`) so the reset can leave the loop's block while the
generated cursor still folds a constant init, and a discarded read of a
local that is dead on case 12's path sits in the transition's zero-store
block so the `&D_o058_5EA0` fragment does not adjoin it. Aligned buckets
**3596 / 18 / 0 / 0**. Every cost wv-i paid was an address-constant lineage
re-split, and so is everything that remains: that fragment coloured `a2`
where the target has `t0` (nine rows; `a2` is offered once its `numintf`
reaches 25, base is 22), case 13's coordinate/cursor pair (seven rows;
`opponent` as the coordinate lands both colours and sheds one piece of the
`&D_o058_5E9C` fragment, one word), and the two slot-address operand orders.
The records give interference *counts* and not members, so the next
instrument is a per-web block-set dump. The [split-tokens
report](whale-split-tokens.md) records the mechanism, the fourteen cycles,
and the corrections: `forced=<colour>` is acceptance, and "twice the
available registers" is not a colouring bound.

## Previous checkpoint: 48, with roles given to the carriers that hold their colours

Lane wv-r banks **48 at delta zero**, down from 147. Five source changes,
each measured alone and then composed: the entry store, case 2's decrement
and case 2's draw loop read `D_o058_5E50` through one name (`portraitX`,
with `i` counted beside it in the decrement loop so the target's post-loop
reset survives), which spans the draw loop's calls and takes s0 on its own
(147 to 85); the probe between `savedX` and `savedOffset` goes (83); case 8's
`textY` sum is reordered (82); `opponent` and `columnX` exchange roles in
cases 12/13 -- the target's column X is its s0 variable and its inner index
its s1 variable, and both keep their colours (63); case 9's `columnX = 0;
do {` shares a physical line (60); and case 13 keeps the stride in
`columnStep` and the count in `columnCount` with one depth-1 probe on each
so the decision order is stride, count, `i` (48). Aligned buckets 3597 /
11 / 1 / 8; 720 draws in the identical order; the gap pair is unchanged and
is now the first mismatch. The first-order landscape at 48 (140 probes)
finds **no single same-kind winner**. The [role-carriers
report](whale-role-carriers.md) records the cursor-init mechanism that made
every earlier `[i]` spelling cost width, the two role swaps, the ratio
arithmetic behind the count/stride probes, and the eighteen cycles.

## Previous checkpoint: 147, with case 2's rank difference in a shared carrier

Lane wv-q banks **147 at delta zero**, down from 157. Case 2's rank
difference is carried by `columnX` rather than `textY`: uopt colours a symbol
as one web across every case, `textY` spans five cases and takes s4 early,
and the target's value there has the colours of a late-decided web. Ten
naming rows heal in case 2; draws, emissions, relocations and the gap pair
are unchanged. The [second-order report](whale-second-order.md) records the
new instrument (`web_footprint.py --hold`), the second-order landscape at the
packed five (116 to a measured **90** with `w101=c17` and `w26=c4`, both
invisible at first order), the third-order landscape at 90 (no single
same-kind winner), the per-case cursor webs the forces hide, and the
entry/decrement cursor mechanism (one shared `D_o058_5E50[i]` web, 86 under
force, callee-save on its own only when it also shares the draw loop's name,
at +4). On the 147 body the seven forces measure 85. Fourteen cycles.

## Previous checkpoint: 157, with the case-6 walker generated

Lane wv-p banks **157 at delta zero**, down from 160. Case 6's node X is
written as an indexed expression and strength reduction generates the
walker, as the target's temporary shows; nine rows heal there and
`countdownX` takes s0. The cost is arithmetic: `portraitX`'s scalar web
loses its case-6 member, falls from 16.5 to 11.67, and case 13's first loop
swaps s0/s1 with its cursor (15.5). The [save-order report](whale-save-order.md)
records the decision-order proof (descending `save` among coloured webs),
the web identifications behind every remaining swap, the case-12/13
inner-index arithmetic (164 against 79.5), and the fourteen cycles. Two
dispatch levers are shown not to exist here: `p2` forces (zero p2 records)
and the joint-force grammar (refused by `uopt`); the compiler's own
`p1:wN=s` split force does work and needs a receipt parser. The 160 and 157
instrument sets are banked at `whale-resources/`.

## Previous checkpoint: 160, with natural menu colours

Lane wv-n banks **160 at delta zero**, down from 169. Indexed menu Y and
text accesses let strength reduction generate both carriers. All nine menu
naming rows disappear, the complete 720-draw sequence survives, and every
other aligned window is unchanged. No structural rows or gaps are added.
The first 165 checkpoint in `df18e628` retained an explicit text cursor;
removing it then restored the target load order. The
[source-colour report](whale-source-colours.md#lane-wv-n-2026-09-14) records
the source proof, all twelve earlier controls remeasured at 169, and the
freshly reproduced 1,898-probe baseline landscape. At 160, a fresh scan of
1,915 legal forces and the complete four-force lattice measure 125 as a
diagnostic reference; the retained C remains NON_MATCHING at 160.

Lane wv-o retains **160 at delta zero**. Its
[neighbour comparison](whale-rowbase-colour.md)
identifies the added entry edge that closes rowBase's sole saved colour.
An array-base capture removes that edge and lets both values take s2 naturally,
but costs coordinate draws, width and relocations. Nine source controls supply
no adoption; the final three repairs stall. The complete 1,915-force landscape
reproduces wv-n's results exactly. The next constraint is to retain the absent
edge without the memory-coordinate cost, not merely to lower interference.

## Previous checkpoint: 169, with another call slot repaired

Lane wv-m retains **169 at delta zero**. Fifteen source controls and a fresh
cross-kind landscape do not recover the target's early offset rematerialization.
A new composition shares both addresses and restores every per-line draw count
as well as the complete 720-draw sequence, but is sixteen bytes short. Forcing
its offset pointer into the target temporary register adds stack traffic rather
than constant rematerialization. The [rematerialization report](whale-offset-rematerialization.md)
distinguishes this result from a general incompatibility claim.

Lane wv-l retains **169 at delta zero**. Its [attempt-21 accounting](whale-attempt21.md)
attributes the twelve-byte deficit to three missing early offset-address
instructions. Exchanging the restoration stores recovers the complete original
draw order and retains position-address reuse at delta -4, but the early sharing
is still missing. Thirteen source controls give no adoption; the full per-line
accounting and the new order witness are preserved for the next packet.

Lane wv-k retains the same **169 at delta zero**. Its
[load-sharing controls](whale-load-sharing.md) recover both redundant tail
reloads while preserving the case-12 reset and position-address reuse, but
leave a twelve-byte deficit and changed draw order. Thirty source cells,
including the remaining independent two-statement orders, yield no adoption.

Lane wv-j retains **169 masked / 340 raw differences at delta zero**.
Moving the empty-record portrait assignment after its formatting call repairs
three words and moves the corresponding format-string relocation. Draw order
and owned extent are unchanged. The [address-reuse report](whale-address-reuse.md)
accounts for the case-12 witness's four bytes and preserves a separate control
with the solved reset, later address reuse and target width, but substantial
replacement gaps and draw-order costs. That control is not adopted.

Lane wv-i previously retained **172 masked / 343 raw differences at delta zero**, down
from 187. Five already-early index resets move into their target call slots;
all 15 changed non-relocation words become target-exact, with no other code,
relocation or draw-order change. See the
[delay-slot report](whale-gap-emission.md#the-reset-reaches-the-slot-lane-wv-i).

Case 12's reset also reaches its exact target slot in a separate source
witness, but its independent array induction still costs four bytes and
later gaps. The retained body keeps that case unchanged and has one gap pair:
candidate +0x12F8 against target +0x1260. The reset remains at +0x12F8 there.
No function is promoted.

The sections below describe the **historical 187 baseline**. Its supplied
landscape and forced 149 are not measurements of the current 169 source.
The wv-j same-kind landscape measured 172 before its source improvement;
it is historical too. Re-measure before interpreting colours on a new body.

14,456 bytes, the largest unmatched function in the tree and **63% of the
remaining distance to 60%**. Nine passes have taken it 733 → 227 → 217 → 187,
and five consecutive passes have now held at 187. This document is the plan for
the next phase: what it actually consists of, every lever still available, and
the tooling that makes each lever readable.

`docs/matching-triage-handoffs/func_overlay_058_F000138C_18B0574.md` remains the
per-pass log and the source of truth for what has been tried. This is the map.

## Wave G: the banked witnesses have now been composed

[The composition matrix](whale-compositions.md) measures 59 new source
compositions and ten nested or repeated controls. None beats **187 at delta
zero**; the best new target-width composition is C19 plus the menu cursor,
at 615 masked and 349 aligned paired rows. The guarded 187 body is unchanged.

The witnesses are not independent source edits: C19 is contained in C23,
and C23 is contained in the width-pressure witness. C's address conversions
are absorbed by B's cached-address spelling. Retaining literal conversions
instead recovers only 12–20 of B's missing 280 bytes. There is partial
compensation: C19, C23 and the width-pressure witness absorb the menu cursor's
earlier naming collateral, leaving four fewer paired residual rows, but the
menu gap survives. The decrement cursor does not repair the remaining costs.

The whole-shape option discussed later in this document now has this bounded
composition experiment behind it. Do not repeat the banked unions or infer
disjoint costs from their lane labels. The matrix does not establish an
unrestricted impossibility result.

## Wave D: target width and reduced pressure coexist

[The wv-d source witness](whale-width-pressure.md) takes the previous attempt
23 to target width with unchanged stack traffic and address interference
98–119. It scores 1,446 masked words and changes the draw sequence; the guarded
187 body and its resource profiles below remain the best retained residual.
The complete alternative source and proof packet are preserved privately.
No new executable bytes are matched, and the original address family still
has no naturally coloured pair.

## What 187 is made of

Measured 2026-09-13 on the retained source, delta zero, frame `0x138`, first
mismatch `+0x50`, 1,253 relocations.

    positional masked        187
      aligned rows           155   = 134 naming + 1 immediate + 20 structural
      insertion shadow        32   from the ONE remaining gap pair

**Three of the four original insertion pairs are closed.** What remains is a
single pair: target-only `+0x1260` against candidate-only `+0x12F8`. It accounts
for about 32 positional words. Source repairs can affect later address reuse.
The [wv-h emission report](whale-gap-emission.md) identifies the displaced
operation as the visible index reset at source line 651: the target puts it
in the first colour call's delay slot at `+0x1268`. The capture at line 660
is outside the gap. The report also demonstrates that changed per-line counts
can leave the entire emission sequence and executable output unchanged.

Aligned rows by region (0x400 windows), naming first:

    +0x0000  36 rows  34 naming  2 structural   <- the largest single block
    +0x0800  21 rows  21 naming
    +0x1400  20 rows  18 naming  2 structural
    +0x1C00  13 rows  13 naming
    +0x0400  12 rows  12 naming
    +0x1000  14 rows   9 naming  1 imm  4 structural
    +0x2400   9 rows   7 naming  2 structural
    +0x3000   6 rows   2 naming  4 structural

The `+0x0000` window has 34 naming rows and a dominant saved-register
substitution cycle. It spans several basic blocks and is **not a temporary
FIFO phase**: the supplied draw sequence never draws those saved registers.
The [wv-e entry experiment](whale-entry-phase.md) changes FIFO phase twice at
zero byte cost and repairs none of the original entry naming rows. The
retained score remains 187; the first masked mismatch remains +0x50.

The ugen census is **720 draws and 5,072 emissions**, with two draws at the
coordinate line and zero at the capture line.

## The thing to undo first

**Every colour landscape on record was measured against a source that no longer
exists, and the last five whale briefs told the lane not to re-run it. That was
my error.**

- The complete 1,875-probe landscape was measured at **227**.
- A second complete landscape, 1,914 probes across 142 coloured webs, was
  measured at **207**, and its best disjoint packing measured **162** with six
  forces (w75→c16, w379→c20, w225→c14, w320→c18, w27→c17, w506→c15).
- The source is now **187**, and its colour axis has never been measured.

The standing rule is already correct — a landscape is void once the source
changes — and I contradicted it in five consecutive dispatch briefs on the
grounds that the axis was "closed and proved". It was closed and proved *for a
different function body*. A fresh landscape at 187 is the first action of the
next phase, not a formality: the 207 landscape found a 45-word forced gain, and
the source has moved twice since.

## The lever nobody has pulled

**The allocator makes 395 recorded decisions on this procedure. 141 are
`decision=color`. 254 are `decision=split`.** Every force this campaign has
ever applied — 1,875 probes, then 1,914, then every lattice — has addressed only
the 141. `CDX_FORCE=p1:wN=cM` sets a colour; it has nothing to say to a web the
allocator decided to split.

And the blocker is stated in split terms. The last four passes converged on:

> removing a capture range fixes the **first split**, but a **later split** then
> takes the restore; reuse has to be preserved across **both successive splits**
> while holding draws and target size.

The split decisions carry the pressure, too: their median `numintf` is 92
against 24 for coloured webs, and the most congested sit at `numintf=140` with
`regsleft=6`. The admission bound the passes keep hitting — "admits the row
entry at 24 interference entries and rejects restoration at 25", "admits
restoration at 26 against its 26-entry bound" — is this number, and it is
recorded per web in the `p1dec` rows that nothing currently reads.

**This is 64% of the allocator's decisions on the tree's biggest function, it is
where the blocker lives, and it has never been measured.**

## The instrument that already exists and is unused

The instrumented `uopt` exposes six knobs: `CDX_LOG`, `CDX_OUT`, `CDX_PROC`,
`CDX_DETAIL_WEB`, `CDX_FORCE`, and **`CDX_LINEAGE_TABLES`**. The campaign uses
five. On this function `CDX_LINEAGE_TABLES=all` emits:

    lineage_range    204 rows   proc event table chain type dtype sym exprtable exprchain
    lineage_member  1514 rows   ... plus bb, line, flags

`event` is a monotonic sequence number, so these rows are the **web creation
order**. L154 says a web's number follows its type first and its first use
second, and that numbering decides colouring order — so creation order is the upstream cause of the colour
questions the landscapes keep measuring downstream. Nothing in `tools/` parses
these records. One earlier handoff spotted the possibility and said a lineage
capture would let a lane "screen spellings on the trace instead of the score";
no lane has.

204 lineage ranges against 254 split decisions is not a coincidence worth
ignoring.

## First readings from the new instruments (2026-09-13)

`tools/lineage_census.py` now exists and its first run on this function says:

    395 webs, 395 decisions: 141 coloured, 254 SPLIT
    204 lineages, 35 of which became more than one web
    numintf median: coloured 24, split 92

    the five largest lineages, by webs they became:
      event  members  webs  split/coloured  first basic blocks
         37       70    29     29 / 0       15,19,21,27,43,44 (+64 more)
         45       53    28     26 / 2       17,19,21,27,43,44 (+47 more)
         39       47    21     14 / 7       15,17,57,58,91,93 (+31 more)
        212       43    20     13 / 7       78,108,110,111,114,129 (+29 more)
        223       40    20     14 / 6       79,91,95,97,99,105 (+26 more)

**One lineage became twenty-nine webs and every one of them was split.** The
top five account for 118 of the 395 webs. The most contended webs sit at
`numintf=140` with `regsleft=6`, all split, and they cluster: webs 1098, 1104,
1172, 1173, 1181, 1183, 1185, 1186 all carry identical pressure, which is the
signature of one live range fighting one region rather than eight separate
problems.

That is the shape of the thing. It was invisible before today.

## Levers, ranked by expected value

1. **Measure the split axis.** What the 254 splits are, where they fall, which
   two are the "successive splits" in the blocker, and what the interference
   bound is at each. Needs the lineage reader below. No source change required
   to get the first real picture.
2. **Fresh colour landscape at 187**, then the packing, then measure the packed
   set. The 207 run bought 45 words; assume nothing, measure it.
3. **Higher-order forces.** Every landscape to date is *single*-force. Two webs
   forced jointly, and any `p2` force at all, are explicitly outside all of
   them. The blocker being a *pair* of successive splits is exactly the shape a
   single-force landscape cannot see.
4. **The last gap pair**, `+0x1260` / `+0x12F8`, worth about 32 positional
   words. Read with `draw_census`: an extra instruction is an extra emission at
   a line.
5. **The `+0x0000` pool-allocation residual**, 34 naming rows across several
   basic blocks. The wv-e experiment rules out treating its aggregate cycle
   as a FIFO-phase diagnosis. Read its entry-web partition and the supplied
   w75/w27/w962 force footprints before proposing another source lever.
6. **The interference bound itself** — whether anything in source can lower
   `numintf` at the restoration site from 25 to 24.

## Tooling: all three are built (2026-09-13)

### 1. `tools/lineage_census.py` — built, 22 tests

Parses `CDX_LINEAGE_TABLES=all` and reports, per procedure:

- **creation order**: every lineage in `event` order with the basic blocks it
  touches, so "which of these two values is numbered first" is a lookup rather
  than an inference. This is the direct readout L154 has been asking for.
  **Positionally, use `bb` and not `line`**: the member `line` field reads 8 for
  all 1,514 rows on this procedure -- a constant -- and `webdetail`'s is -1 for
  330 of its 395. Anything attributing a lineage to a source line off these
  records is reading nothing; `draw_census` is where lines come from.
- **the split picture**: every `decision=split` web from `p1dec` joined to its
  lineage range and members — where it splits, into how many members, and at
  which lines.
- **the interference bound**: `numintf`, `regsleft`, and the decoded
  `forbidden`/`available` masks per web, so "admits at 24, rejects at 25" is a
  number you can look at rather than a symptom you infer.
- `--compare before.json after.json`, like `draw_census`, so a source edit is
  judged by whether it changed the creation order or the split structure.

This is the analogue of `web_footprint` for the phase upstream of colouring, and
it is the single highest-value tool left to build. It needs no new compiler
work — the records are already emitted and thrown away.

### 2. Higher-order forces in `tools/force_lattice.py` — built, 58 tests

The grammar now accepts a **joint force**, `p1:w27+w75=c17`: two or more webs
driven onto one colour in the same compile. Validation compares *webs* rather
than whole spec strings, so a web appearing in both a joint and a single force
is refused, and acceptance requires every web the force names -- a joint force
that lands on only one of its two webs is reported as not applied, because
reading its score as the pair's would attribute a single force's result to a
coupling that never happened. `p2` was already in the grammar and has still
never been exercised on this function.

### 3. A landscape freshness guard — built, in `web_footprint`

A landscape is void when the source changes, and nothing enforces that. `web_footprint` now stamps the ranking's `source_context_sha256` into every
report it writes, and `--report` prints a `STALE:` line and exits nonzero when
that hash no longer matches the tree. A report written before the stamp existed
is flagged too, because it cannot be vouched for. The mistake this document
opens with is now a line of output rather than a judgement call.

## Wave A is done: the blocker has a number (2026-09-13)

`lineage_census` read the split axis and it answers the question the prose
constraint could not.

**Colouring collapses as interference rises**, measured across all 395 webs:

    numintf   0- 40   149 webs   127 coloured   85%
    numintf  40- 70    66 webs    11 coloured   16%
    numintf  70-100    78 webs     2 coloured    2%
    numintf 100-130    72 webs     1 coloured    1%
    numintf 130+       30 webs     0 coloured    0%

**The two successive splits are identified**, and both span exactly the
coordinate and capture sites at source lines 632-665:

- **lineage event 37** -- 70 members became **29 webs, all 29 split**, at
  `numintf` 131-140, `regsleft` 6-7. Nothing in the 130+ band is ever coloured,
  so as the source stands those 29 webs have a structurally zero chance.
- **lineage event 45** -- 53 members became 28 webs, 26 split and **two
  coloured**: web 1063 at `numintf=7` and web 1065 at `numintf=35`, both in the
  band where 85% get a register.

That contrast is the finding. Lineage 45 keeps registers because two of its webs
are cheap; lineage 37 keeps none because all of its are expensive. The bound the
last six passes kept hitting -- "admits at 24, rejects at 25", "26 against 26" --
is this pressure, and it was recorded per web the whole time.

**So the objective is now a measurement rather than a prose constraint: reduce
`numintf` at lines 632-665 toward 40.** `lineage_census --compare` says whether
an edit moved it, which the score does not -- six passes moved the score by zero
while the axis went unread.

The lines themselves come from joining each lineage's basic blocks to the
emission records' `block`/`line` fields; `bb` is positional and the lineage
`line` field is a constant, so that join is the only way to put a lineage on a
source line.

## The campaign: resources are gathered, do not re-derive them

Everything below was measured on 2026-09-13 against the current 187 source and
is retained under Git's common directory at **`whale-resources/`** — outside the
worktree, never tracked, and small enough (7.8 MB) to keep indefinitely:

| file | what it is |
|---|---|
| `footprints187.json` | the complete 1,898-probe landscape at 187, with the packing |
| `landscape187.txt` | its rendered report: winners, rivals, the packed set |
| `lattice187.txt` | the measured six-force lattice, radii and every interaction |
| `lineage187.json` | the split census: 395 webs, 254 splits, 204 lineages |
| `draws187.json` | the ugen census: 720 draws, 5,072 emissions, per line |
| `residual187.txt` | the aligned residual map by address, with the register census |
| `allocator187.log` | the raw unforced trace, `CDX_DETAIL_WEB` + `CDX_LINEAGE_TABLES` |

A lane reads these. **Re-running any of them is wasted budget**, and re-running
the landscape is wasted twice over because it now carries a source fingerprint
and `web_footprint --report` will say so if it ever goes stale.

### The fresh landscape changed the picture

The re-run that this document opened by demanding has been done. At 187:

    1,898 probes across 141 webs, both save kinds
    10 forces beat the unforced 187 at delta 0
    best disjoint packing predicts 149 -- and the lattice MEASURES 149

    p1:w75=c16   172  (+15)      p1:w962=c17  183  (+4)   <- new web
    p1:w225=c20  177  (+10)      p1:w27=c17   184  (+3)
    p1:w379=c20  177  (+10)      p1:w498=c15  184  (+3)   <- new web
    p1:w225=c14  180  (+7)

**The forced diagnostic is 149, not 187**, and two of the productive webs — 962
and 498 — appear in no earlier landscape, because no earlier landscape was
measured against this body. The measured best set is five forces:

    w75=c16 + w379=c20 + w225=c14 + w27=c17 + w498=c15  ->  149

Radii, for nominating anything further:

    w75=c16    0x0180 0x0280 0x0300 0x0380 0x0580 0x0880 0x0980 0x0a00 0x0a80 0x0b80
    w379=c20   0x1280 0x1700 0x1780 0x1900
    w225=c14   0x0780
    w27=c17    0x0080
    w498=c15   0x1c00 0x1c80 0x1d00
    w962=c17   0x0180 0x0300 0x0480 0x0580 0x0600 0x0680 0x0c00 0x0c80 0x1e00 ...

`w225=c20` and `w379=c20` remain rivals on one radius. `w962` is antagonistic
with `w75` (+43, contending in `0x0180, 0x0300, 0x0580`), which is why the
packing takes `w498` instead.

### Wave structure, in order, one lane each

**Wave A — the split axis, no source change.** Read `lineage187.json` and the
`allocator187.log` it came from. Answer: which two of the 254 splits are the
"successive splits" in the blocker; what the interference bound is at each; and
whether the 29-web lineage at event 37 is the live range the whole thing turns
on. The deliverable is a map, not a match. Nobody has ever looked at this.

**Wave B — the coupled repair, using joint forces.** `force_lattice` now accepts
`p1:wA+wB=cN`. Take the two webs Wave A names and drive them onto one colour
together. 3,789 single-force probes could not see a coupled repair; this is the
first instrument that can. If a joint force reaches a materially better score
than 149, that names the source question exactly.

**Wave C — the last gap pair.** Target `+0x1260` against candidate `+0x12F8`,
about 32 of the 187 positional words, and the only one of the original four
still open. An extra instruction is an extra emission at a line, and
`draws187.json` is the before-profile: take an after-profile and `--compare`.

**Entry wave — measured by wv-e.** The `+0x0000` window contains 34 naming
rows, but its aggregate cycle is not one temporary-ring rotation. The first
mismatch is a pool-coloured switch load before the first temporary draw.
Two zero-width FIFO interventions and eight carrier/type/pointer controls
supply no admissible entry repair. The unchanged-source landscape also has
w75 and w962 winners touching this window; w27 is not its only colour handle.
See the [entry-phase report](whale-entry-phase.md) for the measured boundaries,
draw orders, category-migration trap and source-attempt stop evidence.

Waves A and B are sequential; C and D are independent of both and of each other,
so they can run concurrently with A.

### What a whale brief must carry, and what it must not

Carry: the paths above, the measured 149 and its five forces, the radii table,
and the wave this lane is running. Nothing else — the shard has the rest.

Must not: re-run a landscape, re-derive the residual map, re-try any of the
seven exhausted case-12 attempt families, or re-try L160 at the case-3 cursor.
Each is recorded with evidence, and a pass that repeats one has spent its budget
learning what the shard already says.

## Campaign outcome, 2026-09-13: the axis opened, the score did not

Five lanes ran the plan above. **The source is still 187 masked at delta zero.**
What changed is everything around it, and the honest summary is that the whale
is now well understood and still uncaught.

**What the campaign established**

- **The split axis is readable.** `lineage_census` exists; 254 of 395 decisions
  are splits; the two successive splits in the blocker are lineages 37 and 45,
  both spanning lines 632-665; and colouring collapses above `numintf` 40 (85%
  coloured below it, 0% above 130).
- **The split axis is movable.** A source change reached `numintf=39` with a
  naturally coloured web — the first measurable movement on an axis six earlier
  passes could not see at all.
- **Target width and reduced pressure are achievable together.** Lane wv-d's
  witness holds delta zero, frame 0x138, the same 35 stack homes, and pulls its
  members to 101-119.
- **The colour axis is worth 38 words, not zero.** The landscape at the current
  source measures a five-force packing at **149**, and twelve source controls
  did get the compiler to take the wanted colours *naturally*.
- **A standing claim was wrong.** A closed cycle in one window nominates a
  ring-phase hypothesis; it does not establish one, because a window spans
  several blocks and unrelated pool-coloured values. `residual_map.py`'s
  documentation said otherwise and now does not.

**Why none of it landed**

Every improvement was offset, and **the offsets were different every time**:
address lifetime cost 280 bytes; the interpolation cost stack traffic and
relocation changes; the width-pressure witness cost gap rows and draw order; the
natural-colour controls cost naming rows. This is not one wall with a seam in
it — it is a tightly coupled function in which each axis is paid for out of
another. That is a finding, and it is the reason to stop treating the next pass
as "one more attempt".

**What that implies for the next phase**

Two honest options, and the evidence does not yet choose between them:

1. **A whole-shape rewrite rather than local edits.** Every attempt to date
   holds most of the function fixed and perturbs one site. The couplings are
   between sites, so the thing that is never varied is the one that would have
   to change. This is expensive and has never been tried.
2. **Argue the bar.** The standard is `overlay57UpdateModeState`'s: enumerate
   the space and show why nothing outside it reaches the target. With ~100
   measured attempts across five lanes plus two complete colour landscapes,
   this function is closer to that standard than anything else in the tree --
   but it is not there, because 155 aligned rows would all have to be accounted
   for and only the transition reuse and the entry block have been.

**Do not simply dispatch another local-edit lane.** Five have now returned the
same shape of result, each with a different collateral, and the marginal value
of a sixth is low. Either option above is a decision worth taking deliberately.

## What not to spend the next pass on

- **Another variation on the case-12 capture constraint.** Seven attempt
  families and 27 source cells are recorded against it: capture-carrier,
  first-loop join, saved-local access, capture-type split, late coordinate
  roles, restore-exit distribution, nested-sum identity. Each was measured and
  each added draws or opened a replacement gap. Read them before proposing
  anything adjacent.
- **L160 at the case-3 cursor.** Ruled out with evidence: that cursor is already
  compiler-generated, and forcing its target register costs four bytes through
  lost transition-address sharing.
- **Re-deriving the colour landscape at 227 or 207.** Both are complete and on
  record. The one to run is 187.

## The honest alternative

If the split axis is measured and the coupled repair is still not reachable,
the right outcome is an argued bar, not another pass. The standard is the one
`overlay57UpdateModeState` meets: enumerate the space, and show why nothing
outside the enumeration reaches the target. A plateau, however stubborn and
however large the function, is not that — and this function has 155 aligned
rows, so any such argument has to account for all of them, not just the
transition reuse. Nobody has been close to that argument yet, which is itself a
reason to think the axis above is worth measuring first.

## Checkpoint 2026-09-15 (lane `wv-t`): the split axis is now readable, and it is a growth test

Still 18 masked at delta zero. What changed is that the 254 `decision=split`
records this file said were unmeasured are now measured to the block: the
instrumented uopt emits every decided web's block sets and the whole of
`split()` -- the seed, each candidate block with `new`, `left_before`,
`left_after`, `numintf`, the verdict, and every liveblock that moves.
`tools/lineage_census.py --web N` prints a web's growth with the margin of
each step. Full account in [whale-split-growth.md](whale-split-growth.md).

- A piece grows breadth-first from its seed and accepts a block only while
  `new < left_before` and `2*left_after >= numintf + new`; a call block is
  accepted but never expanded. All three remaining fragments are one such
  verdict, off by one.
- W (`&D_o058_5EA0`) is `a2` because its piece rejects the title loop's call
  block (22 < 23); with it, the call's `a2`/`a3` are forbidden and it takes
  `t0`, as it does on the 48 body. The `&D_o058_5E9C` 202-piece exists only
  because a dropped 183-seeded piece rejects 202 (24 < 25). Both margins are
  paid by the dead carrier `textY`: one interferer and one colour (`s4`).
- **The two are coupled by an identity**: with any dead-carrier web at 183,
  W needs `left >= 12` at its 185 step while the 202-piece needs `left <= 12`
  at its 202 step, and the two lefts differ by exactly one (`w40` takes `v1`
  between them). No such body can satisfy both; `textY` fails W, a fresh
  carrier fails 5E9C (+4), both measured. The target has no carrier web at
  183, and the 48 shape cannot reach the delay-slot reset under the shard's
  laws (re-measured).
- The explicit-pointer exit costs one interferer at 184 (its init is a
  reference of the `&D_o058_5C98` lineage, where the strength-reduced cursor
  init is not), which is the same one off.
- Case 13: `opponent` as coordinate fails the `a2` piece's 254 step by one
  (30 < 31) because `portraitX`'s web still sits at 241 for its reset; the
  probe route to a higher `portraitX` save converges on 10, under the cursor
  temp's 15.5.
- Twelve cycles, listed in the report; cycle 13 named there: a web in W's
  blocks coloured `v1` before the 5E9C remainder splits at save 3.60.
