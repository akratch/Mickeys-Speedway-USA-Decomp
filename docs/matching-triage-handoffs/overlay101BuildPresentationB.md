<!-- plateau-handoff:overlay101BuildPresentationB:start -->
### `overlay101BuildPresentationB` plateau handoff

- source: `src/overlays/o101/overlay101BuildPresentationB.c`
- score: 136 differing words
- frame: 0x20
- relocations: 52
- first mismatch: +0x10
- summary: Delta 0: dim colour as counter*0+0xC0 is an expression web that takes a2 (type-2 const never does). 136 at 0x20; colour ceiling to re-derive.

Summary before this remeasure: Identity-gate proc 0. Extra 0xC0 into a2; w198=c5 declined. Copy/L97/L144/L145/fallthrough/remat leave +4. Force floor 134 at +4.

Measured 2026-09-11, lane `lane/s1-trio`, on the four-function overlay-101
presentation-builder cluster. Every number is `tools/align_symbol.py`, whose
positional figure agrees with `tools/score_symbol.py` by construction, and each
of the four was measured separately at every step.

Before and after, in the aligner's four buckets:

- A: 161 masked, 65 byte-exact, 87 naming, 4 immediate, 57 structural, delta +4.
  After 145 masked, 77 byte-exact, 106 naming, 1 immediate, 32 structural, +4.
- B: 157 masked, 69 byte-exact, 98 naming, 4 immediate, 42 structural, delta +4.
  After 145 masked, 77 byte-exact, 106 naming, 1 immediate, 32 structural, +4.
- C: 163 masked, 61 byte-exact, 87 naming, 5 immediate, 61 structural, delta +4.
  After 145 masked, 77 byte-exact, 106 naming, 1 immediate, 32 structural, +4.
- D: 151 masked, 68 byte-exact, 85 naming, 4 immediate, 53 structural, delta 0.
  After 131 masked, 82 byte-exact, 103 naming, 1 immediate, 27 structural, 0.

A, B and C are now identical in every bucket and at every insertion and
deletion offset. The frame is 0x20 on all four and its three-slot ladder
matches the target at every offset, before and after.

Three levers, all source shape, none costing an instruction.

- L59, the group fold. Every assignment group down to the node-24 header is ONE
  physical line. as1 minimises `(start_time, -aftercycles, -latency, addr,
  lineno, ready-list position)`; with a group's stores on separate lines
  `lineno` is the deciding key and emits them in source order, and the ROM
  emits several of these groups' constant materialisations reversed. Folding
  retires the key. All 1,024 fold subsets were measured on each of the four.
  The minimal paying set is the root header, the node-32 pre-call group and the
  root's second header: on B that reads 145 against 157, and folding any of the
  other four groups on top is flat on the positional count while moving three
  words out of the structural bucket into the naming one. The adopted form
  folds the first seven groups, which is the lowest structural residual
  available at 145.
- The node-24 text store goes BEFORE the counter bump, never last in the tail
  group. Worth four words on A, B and C and six on D. All 1,440 orders of the
  eight tail statements were measured on B: 145 is the floor, 80 orders reach
  it, and the worst order reads 149.
- C only, and the interesting result of the lane. C's order counter was written
  as a post-increment in one statement where A, B and D read it and write back
  `index + 1` in two. That spelling alone was worth ten words: with the fold but
  the post-increment retained C reads 159, and with the two-statement form it
  reads 149 and then 145 with the text-store placement. C's former four-word gap
  to A in the ranking was entirely this, and nothing to do with the family's
  levers.

Refuted here, each with its measurement.

- The quadruplet's call-spanning-web lever, which the campaign brief named as
  the first thing to try, does not apply. Storing handle, previousType and
  previous through the PRE-call node-32 pointer reads 184 at a size delta of
  +12 against 145. This relative's target re-reads the node-32 counter after
  the call and stores through the recomputed pointer, which is what this shape
  already does, so there is no missing callee-saved web here.
- The `volatile` casts A, C and D carried in the tail are exactly inert:
  removing all three reads the same number to the word, before and after the
  other levers. They are gone from the adopted sources.
- Counter partitioning is already separated, as the brief's caution said to
  check. Merging the two node-24 index locals reads 206 at a size delta of +8;
  inlining the node-24 index local reads 148 and the node-32 one 155.
- `register` on any of the six index locals is inert, and so is every
  statement order inside a folded group. CORRECTED 2026-09-12: the measurement
  stands, the reason given for it does not. Order inside a folded group is inert
  because a folded group is one physical line and therefore one `lineno` key
  (L132), not because of the call test. The call test retires L106 and nothing
  else; statement order across separate lines still reaches as1's scheduling and
  ugen's emission order, and was decisive on seven functions closed in one lane.

Next lever, with the decision variable named.

The A, B and C surplus is exactly one instruction, and it is a second
materialisation of the 0xC0 colour constant for the final call's third
argument. The target materialises it once into the argument register and reads
that register for both colour stores. Read on the instrumented uopt
(`~/Desktop/dev/ido-instrumented`, `.text` confirmed byte-identical to the
stock object with traces off, procedure ordinal 0): the constant is web 198,
`type=2 dtype=8 table=192`, `save=1.000000 nocs=1`, and its `p1dec` carries
`forbidden0=0x7fc30000`, which forbids colours c1 through c9 -- the argument
register among them -- leaving c10 through c13 available, and it takes c10.
`CDX_FORCE=p1:w198=c5` is DECLINED: no `p1color` record with `forced=5`, and
the object returns byte-identical, so by L101's third kind that experiment
proves nothing rather than proving the colour unreachable. Forcing the web that
holds that colour (web 108) out of it, to c10, c11 or c12, does not hand it to
web 198 either, and reads 147, 145 and 150.

Measured inert against the surplus word, all at 145: the position of the
`dimColor` assignment anywhere in its block (seven placements), its spelling
(`s32`, `u8`, `s8`, `u32`, `short`, `char`, the bare literal at every site,
`register`, decimal, octal, an explicit cast), an old-style declaration for the
callee, and L109 discarded-expression probes (OR-with-zero, AND-with-minus-one,
XOR-with-zero, in two different basic blocks, singly and doubled). Measured
regressive: `if (1) { }` and `do { } while (0)` region boundaries around the
assignment, the colour stores or the call read 171, 169 and 176. The
copy-propagation narrow on L109 is the reason the probes are inert: the value
is a constant, so uopt has already folded it and there is no symbol reference
left to count.

How much of the rest is the allocator. A greedy force ceiling on B -- every
p1 candidate web against every colour and the split path, best kept as a
prefix, repeated -- ran three productive rounds and a flat fourth: 145 to 136
to 130 to 127 masked, byte-exact 77 to 97. So about 18 words, 12 percent of the
residual, is allocator-reachable from this source shape, which is in line with
the project's measured one-sixth budget. The remaining 127 is not: 106 of the
145 are register naming and the aligner's structural rows are all schedule
order over the same instructions.

A correction to L114 as the standing brief states it. `globalcolor` DOES assign
`t3` in this procedure: web 182 and web 198 both carry `p1color` records naming
`color=10 reg=t3`, and the `p1cost` lists price `color=11 reg=t4` and
`color=12 reg=t5` as ordinary caller-saved candidates. The colour table here
reads c1 v0, c2 v1, c3 a0, c4 a1, c5 a2, c6 a3, c7 t0, c8 t1, c9 t2, c10 t3,
c11 t4, c12 t5, c13 unnamed, c14 s0, c15 s1, c16 s2 and up. So the rule that
`t3`..`t9` are never colours is too strong: in this procedure only `t7`..`t9`
and the float ring are outside the table, and a naming row spelled `t3`..`t6`
here is a colour decision a force can reach, not ring phase.

This relative already placed the node-24 text store before the counter bump and
carried no `volatile` casts, so the fold is the whole of its gain. Adopting the
other relatives' tail shape on it regresses 145 to 149, which is the same
four-word step measured in the other direction.

Lane `lane/p2-pres`, 2026-09-11, re-measured this cluster and pushed the two
axes the campaign brief named. The source is unchanged: everything below is a
negative, and each one names what it varied.

The forced-colour census reproduces exactly. 145 to 136 to 130 to 127, flat on
the fourth round, with the same prefix: `p1:w186=c11`, `p1:w31=c2`, `p1:w78=c8`,
every force checked for a `p1color` record carrying `forced` before its object
was scored. Of 587 single forces tried, 324 were accepted, 18 improved, 58 were
exactly flat and 248 regressed; the improving forces touch twelve distinct webs.

The 127 is NOT a sweep artefact. The brief's caution is that a sweep never
offers a colour another web has forbidden, so this lane swept the displacement
move directly: for every coloured web, move it off its colour (to the split
path or to any of colours 1 through 22) AND force another web onto the colour
it vacated, both forces verified accepted. 12,320 such pairs at the 127
plateau, 2,474 of them accepted, and not one beat 127. From the unforced 145
base, 15,004 pairs, 3,291 accepted, best 135 -- worse than the single-force
greedy reaches. So the interfering-web hypothesis is refuted on this function:
freeing an incumbent colour does not hand it to any other web profitably, and
127 is the allocator's floor for this source shape rather than a lower bound
on one.

Three new source axes, all flat.

- Line breaks INSIDE a folded group. The previous lane measured all 1,024
  whole-group fold subsets; it did not measure where a group is broken. All 81
  intra-group break positions across the seven folded groups were measured
  here, then greedily accumulated. Nothing raises aligned byte-exact above 77.
  One break, the first group at its fourth statement, reads 144 masked but 75
  byte-exact and 34 structural, so it trades two agreeing words for one
  positional word and is not adopted. A, C and D give the identical picture
  (144 at 75 exact, 144 at 75, and 130 at 80), which is a fourth independent
  confirmation that the trio moves together.
- Tail statement order, widened. The previous lane measured 1,440 orders of
  eight tail statements. This lane searched the whole twenty-statement tail
  under its real dependences: a constrained single-move greedy is flat at the
  first round over 209 neighbours, and 88 random restarts with hill climbing
  spent 19,630 evaluations without ever beating 145. The sibling function
  `func_overlay_101_F000C144_18E7964` moves 24 words on exactly this axis, so
  the axis is live in the family and exhausted here.
- Compiler flags. `tools/flag_sweep.py`, 119 combinations: the configured
  `-O2 -mips2 -32` is first at 145 and no override, including every loop-unroll
  level, `-Wab,-r4300_mul`, `-g3`, `-mips1`, `-mips3` and an all-phase `-O3`,
  improves on it.

One mechanism worth carrying forward, read from this procedure's own records.
A caller-saved colour costs 2 for each call the web spans beyond the first, and
once a web's `nocs` reaches 3 the `available0` mask offers NO caller-saved
colour at all -- every such web takes `s0` upward in descending `save`. That is
why the seven declined webs here all sit at `save` 0.5 with `bestcost` 2.0: the
decision variable is `totalsave` against `nocs`, and on this function no source
form reached above it. `p1:w186=c11`, which pays 9 words, wants the node-20
array base's `totalsave` above 4 at `nocs` 2, and the web has two references.
Holding that base in a local pointer instead reads 170 at a size delta of 8.

Lane `lane/p4-pres`, 2026-09-11. One adoption and four measured negatives, two
of them against axes the dispatch named as this cluster's live edges.

Adopted: the byte-length local is `u8`, not `s8`, the store takes an `(s8)`
cast and the redundant mask in the opacity expression goes. 145 to 143 masked
on each of the three, aligned byte-exact 77 to 79, register naming 106 to 104,
immediate 1, structural 32, size delta still +4, and all three remain identical
in every bucket. A 540-cell type lattice was measured -- the callee's declared
return type across six spellings, the local's across six, the store cast across
three and the opacity mask across five -- and every cell that reaches 143 has
the local `u8` and the store cast `(s8)`, with the return type indifferent. The
change is semantically exact: the value is only ever truncated to eight bits, so
`& 0xFF` becomes redundant once the local says so, and the `(s8)` cast is what
draws the extra ugen temp. The same axis is worth 67 and 81 words on the
1520-byte relatives in this family, where nobody had swept it either; two words
is all it is worth here.

The 106 naming rows are NOT a closed cycle, which answers the question the
dispatch asked and closes that direction. `tools/register_census.py` prints a
four-cycle over t1, t0, t4 and t3, but that is its dominant mapping and the
dominant mapping discards fan-out. Every heavy register fans out: ours `t7`
goes to their `t6` eleven times, `t5` eight and `t9` six; `t4` to `t3` eleven
and `t8` eight; `t8` to four different registers; `t1` to three. Resolved by
offset at the adopted plateau, 80 of 133 pair-instances -- 60 percent -- are
consistent with one global permutation; the best single prefix/suffix split, at
+0x1EC, reaches 75 percent; and six windows are needed before every window is
internally a permutation, at boundaries +0x11C, +0x194, +0x1D4, +0x250 and
+0x2A0. The picture is the same before the type adoption, 78 of 135 and the
same six boundaries, so it is a property of the function rather than of one
plateau. So these rows are
per-region colour, not one ring-phase fact, L127 has nothing here to put back in
phase, and what remains is a colouring question per web. For contrast, the case
that instrument was built for had 195 of 279 words in a single four-cycle, and
the 1520-byte relatives two functions away resolve to one clean one-position
shift at 83 percent -- so the instrument does separate the two, and this
function is the scattered kind.

The L126 copy carrier is flat here. 245 carriers were measured over all thirteen
locals: each value copied into a fresh local of its own type, the copy placed at
every statement boundary after the value's definition, and read by each single
downstream use and by all of them at once. Nothing beats 145, and every one of
the 245 leaves the size delta at +4 -- so the copy really is byte-free exactly
as L126 says, and byte-free is all it is on this function. One corner is open
and worth naming: four pointer locals could not be reached because their
definition and all their uses sit on the same folded physical line, so no
"downstream" boundary exists for them. They are all pre-call and the surplus is
post-call.

The nocs lever reaches 138 at size delta 0 and is SEMANTICALLY INVALID. This is
recorded because it looks like a breakthrough and the next lane will find it
again. A second definition of `previousType` placed after the root's childType
store, or of `previous` after the root's child store, reads 138 at delta 0
against 145 at +4, and a greedy second round is flat there. But by that point
the root fields hold this node's own new values, so the node would link to
itself instead of to the previous chain entry: it is trap 8 in the standing
brief, a sweep offering a candidate that scores better and means something
different. The same trap swallowed a second lattice: respelling the chain-head
reads produced 136 at delta 0 across 304 cells, and every cell at 136 or 138
turned out to read a root field after that field had been overwritten.

What those invalid cells do establish is a fact about the target, and it is the
useful part. Deleting the previous-link LOAD closes the size delta exactly. So
the trio's surplus instruction is not only the second materialisation of the
colour constant that the earlier shard named -- it is reachable from the load
side as well, and a source form that legitimately removes that load would close
the +4. The chain head is a known value at the node-24 push, since the root's
childType and child were set to 1 and the node-20 pointer earlier in this same
function and nothing between writes either -- but spelling them as those
constants is strongly regressive, and that is measured rather than assumed:
193 at a size delta of 8 with both spelled as literals, 189 at +4 with only the
previous pointer spelled that way, and 166 at +8 with the pointer carried
through a local of its own.

The tail statement order is now exhausted exhaustively rather than by search.
The previous lane covered 19,630 orders by constrained greedy plus random
restarts and never beat 145; the type adoption moved the plateau, and a plateau
that moves invalidates every order result taken above it. So this re-enumerated
the tail COMPLETELY: all 4,480 permutations of the eight tail statements that
respect the four real dependences -- each chain-head read happens before the
root field it reads is overwritten, and before it is stored into the node. 143
is the floor, the adopted order sits on it, and the worst valid order reads 149.
Those four dependences are exactly what every 136 and 138 cell above violates,
which is why this sweep cannot emit one.

Next lever, with the decision variable named. The +4 is one surplus instruction
and there are now TWO independent routes to it rather than the one the earlier
shard named. The first is that shard's: a second materialisation of the 0xC0
colour constant, web 198 at `save=1.0 nocs=1` whose `p1dec` carries
`forbidden0=0x7fc30000`, forbidding c1 through c9 -- so a force on it is
silently declined and, by L101's third kind, proves nothing. The second is the
previous-link LOAD, which every invalid variant above deletes to reach size
delta 0, and which no lattice has yet attacked from a semantically valid form.
The load side is the newer and the cheaper-looking of the two.

Order the work that way rather than by bucket size. The 104 naming rows are a
separate question and the census above says they are per-web colour, so they
will not fall to one edit; and the previous lane's forced-colour ceiling of 127
was measured on a source shape that no longer exists, so it should be
re-derived, not inherited, once the +4 is closed.

#### 2026-09-12, lane w1-1: carrier deletion is byte-inert across the cluster

B was the lead. The assignment gate returned base-only for each of A, B, C
and D before source experiments. All four configured stock baselines were
retained, and each stock/instrumented fidelity gate passed.

The retained B baseline is 209 candidate words against 208 target words,
delta +4, 144 raw and 143 masked differences, first masked mismatch +0x10.
The aligned buckets remain 79 exact, 104 naming, 1 immediate and 32 structural.
The structural total includes 17 paired rows, eight candidate-only words and
seven target-only words. The candidate-only offsets are +0x24, +0x110,
+0x118, +0x120, +0x154, +0x170, +0x174 and +0x31C; the target-only offsets
are +0x54, +0x12C, +0x130, +0x138, +0x148, +0x188 and +0x18C. A and C
reproduce every bucket and offset. D has the same seven paired displacement
sites but lacks the surplus +0x31C materialisation of the dim colour. Its
final call has no colour argument. This confirms the instruction question
before considering any colouring score.

The candidate has 52 object relocations. The assembled comparison target has
34, and their literal object symbol identities are not equal. These are
comparison-object counts, not an authenticated overlay relocation proof.
No linked promotion or matching credit is claimed.

No saved baseline footprint file accompanied the prior shard. Fresh
web_footprint --every-colour measurements cover 22 coloured webs and 132
accepted same-save-class forces on B. Of these, 107 retain delta +4 and 25
increase it; none reaches delta 0. The best single is 135 at +4 on
p1:w31=c2. --report was run on the saved JSON, but its zero-delta filter
excludes every cell here, so it cannot supply a disjoint packing for this
nonzero-size baseline. That absence is not evidence that the forces are inert.

The already-recorded node-20 array-base web is split, rather than coloured,
and therefore lies outside that coloured-web census. Its current record is
web 186, save 0.5, nocs 2, totalsave 1, bestcost 2, decision split. A bounded
four-force lattice combined p1:w186=c11, p1:w31=c2, p1:w78=c8 and the new
single winner p1:w115=c7. All 15 nonempty subsets were accepted. The tested
floor is 125 masked at +4 on the first three forces; adding the fourth gives
128. This is a diagnostic subset floor, not an exact source result or proof
about every interacting allocator decision. A and C reproduce the mapped
three-force lattice independently, including the 125 floor.

Source experiments on B, each retained with its configured object and aligned
per-window comparison:

- Unsigned final-call colour parameter alone: 143, delta +4, byte-inert.
- Delete the node20A pointer declaration and use indexed accesses: 143, +4,
  byte-inert.
- Delete opacityScale and generate its 1.0f literal: 150, delta -8. This
  changes the tail instruction inventory instead of fixing the surplus.
- Delete node24B and use indexed accesses throughout the tail: 143, +4,
  byte-inert.
- Delete all six node pointer declarations and the dimColor declaration,
  substituting indexed accesses and the literal at their uses: 143, +4,
  byte-inert. Applied without tuning at the corresponding declaration and
  node-construction groups in A, C and D; each retains its baseline object.
- Delete previousType and previous, writing each node's previous fields at
  the original chain-head reads, before the root is overwritten: 152, +4.
  This is a valid early-store form, not the previously rejected self-linking
  reconstruction. It is regressive and is not adopted.
- Make the colour literal, local and final-call parameter unsigned together:
  143, +4, byte-inert. The type identity hypothesis does not close +0x31C.

The allocator trace makes the L160 limit precise. After all pointer carriers
are deleted, the node-20 base becomes web 165 rather than web 186, with the
same type-1 identity table, save 0.5, nocs 2, totalsave 1, bestcost 2 and split
decision. Removing these straight-line copies does not expose a hidden base
web or improve its benefit. The compiler already generates the same effective
address computation. No zero-scoring force-to-source route was found.

The retained source is unchanged apart from this handoff summary. The final
carrier-deletion transfer, valid early-store form and joint unsigned form all
fail to improve the residual or establish a shippable route. Stop this packet
rather than repeating the prior order/type lattices. The next named source
question is how the node-20 base can legitimately earn more benefit without
extra instructions; separately, the trio still needs one shared dim-colour
materialisation across the stores and final call. A colour score alone does
not answer that extra-instruction question.

#### 2026-09-13, lane c2: per-line colour-store result differential

The fresh configured baseline reproduces 209 candidate words against 208
target words, 144 raw and 143 masked differences, first +0x10. Alignment is
79 exact, 104 naming, one immediate and 32 structural rows, including eight
candidate-only and seven target-only words. Procedure zero has 55 draws and
374 emission records. Stock and instrumented full-TU text agree. A and C
independently reproduce every aligned bucket and gap offset. No colour sweep
was repeated; the existing exhaustive receipts were read first.

Three source probes preserve physical line positions. Splitting the two tail
chain-capture identities leaves every draw and emission count and draw order
unchanged, but regresses to 145 masked. Defining dimColor inside the first
colour store is full-text inert: only its old source-marker emission disappears.
Capturing the byte-store RESULT into dimColor instead changes the intended
store line from zero draws to one, and its emissions from two to four. Total
draws become 56 and emissions 373. The call consumes that result, replacing
the duplicate constant materialisation with a byte mask; the geometry remains
209 against 208 words. Alignment becomes 79 exact, 102 naming, one immediate
and 38 structural, with 158 masked differences. This is a measured emission
lever, but it does not remove an instruction or improve the retained residual.

The result-capture form was transferred without tuning to the first dim-colour
store in A and C. Both reproduce B's 158 masked differences and all aligned
buckets and gap offsets. A uses color1 at that point, while B and C use color0.
An initial A replacement missed this distinction and left dimColor undefined;
that preparation is marked INVALID in private evidence and excluded entirely.
The corrected transfer is separately compiled and retained.

Restore the guarded 143-word body. Stop this packet on the measured result
capture regression and the prior closed carrier, type, order and colour axes;
no general source-impossibility claim follows. The remaining question is a
shared final-call colour value without either a duplicate materialisation or
the new mask, plus the earlier schedule/allocation residual. The surplus lies
near the tail and cannot by itself explain all preceding differing rows.

Candidate/assembled-target static relocation counts remain 52/34, with no
literal tuple identities at matching positions. This is not an authenticated
overlay relocation proof. Private build/c2 evidence retains source/object
pairs, complete traces, census profiles and aligned maps. Commands include
lane_status.py, configured stock compilation, draw_census.py, residual_map.py
with retained objects and paired deltas, finalize_plateau.py and tools/gates.sh.
ROM verification covers the assembly fallback and adds no matching credit.

#### 2026-09-19, lane w22-o101b: size levers on the extra dim-colour word

Fresh configured baseline reproduces 209 candidate words against 208 target
words, 144 raw and 143 masked differences, first +0x10. Frame 0x20 matches
the three-slot ladder. Alignment is 79 exact, 104 naming, one immediate and
32 structural, with the same eight candidate-only and seven target-only
offsets as the inherited shard. Candidate object relocations remain 52.

Identity gate: stock `tools/ido/cc` and instrumented `ido-instrumented/cc`
`.text` are byte-identical (fidelity pass on `.text`, `.data`, `.rodata`,
relocations and symbols). Nonnumeric `CDX_PROC` prints one procindex row:
proc 0, 33 decisions. All forces below used `CDX_PROC=0` and were scored with
`--object` on the forced object. Acceptance was read from `forced`, not from
whether the object changed.

The extra word is still the second materialisation of the dim colour 0xC0
into the call's third argument register. The target materialises that
constant once into that register before the unsigned length conversion and
reuses it for both colour stores and the call. This candidate materialises
it into t3 (web 198: type 2, dtype 8, table 192, save 1.0, nocs 1,
forbidden0 0x7fc30000, cost table c10-c13 plus callee-saved) and
rematerialises it for the call. `CDX_FORCE=p1:w198=c5` is declined
(`forced=-2`, no cost-table entry for a2). No accepted force reached size
delta 0. Splits of webs 198, 108, 182 and 3 grow to +8. Colouring still
moves only the naming residual at +4: w186=c11 is 134, w31=c2 is 135,
w78=c8 is 139, matching the inherited three-force prefix.

Named size levers, each compiled with the configured command and scored
directly. Source restored after every cell. Guarded body unchanged.

- Sibling opacity spelling `(f32)(s32)1` in place of the `opacityScale`
  local: 143 at +4, byte-inert. The inherited 150 at -8 was the `1.0f`
  literal specifically; the cast form keeps the conversion and does not
  close the extra word.
- Copy 0xC0 into existing dead s32 carriers (`orderIndex`, `node24IndexA`,
  `node32IndexB`) or via `handle`: 143 at +4, copy-prop folds them back.
- L97 `if (1)` / `do { } while (0)` around the prefix, the opacity
  conversion, or the tail: 154-160 masked, size +4 to +12. Bare `{ }` is
  143 at +4, as L97 says. Empty `if (1) { }` between opacity and dimColor
  is 157 at +8.
- Delay `dimColor = 0xC0` until after opacity, nested block, or late
  assignment with literal stores: 143 at +4. uopt still hoists the
  constant.
- Moving the dim colour stores next to the call, folding them onto the
  call line, or a comma-expression argument: 148 at +4. The inherited
  eight-statement tail lattice did not include those stores; moving them
  is now measured and is worse.
- L145 node-20 `pool[count]` on both sides of the create call: 149 at +4.
- Rematerialised node24 address (`pool + index`, two names for the text
  store, indexed colour0): 143 at +4.
- L144 address-form of `dimColor` uses or assignment: 142 masked at +4,
  but first mismatch +0x0 and frame 0x50 with a spill replacing the
  second materialisation. Not adopted. Call-only L144 is 158 at +8.
  `volatile s32 dimColor` is 162 at +16.
- Reusing the existing `previous` / `previousType` carriers for the
  node-20 chain head (valid, not the self-link trap): 192-197, size +8
  or +16, extra callee-saved. Folding that capture onto the root writes
  reaches size delta 0 at 192 masked, first +0x0, frame 0x28 with an
  extra s2, and the second 0xC0 materialisation still present. Coincidental
  size close; not adopted.

Three consecutive productive rounds on the extra word produced no better
retained residual, no new valid identity, and no force-to-source route
that colours web 198 as a2. Stop under ADR 0018. Next named question:
give the dim colour an s32 argument identity (the target stores the
byte from the argument register) without homing it. dtype 8 on web 198
is the copy-propagated byte-store form; L144 proves the s32 home but
pays a frame. Do not re-run the copy, L97, tail-store-move, or
node-20 L145 cells above.
#### 2026-09-23, lane B2-o101: size delta +4 closed to 0 on A and B

Transferred from A with no separate tuning. The mechanism, the factor sweep and the negatives below were measured on A; on B itself the counter factor reads 137 and the index factor 144 at delta 0, identical to A, the tail climb found the same single move to 136, and the final buckets and frame match A. Cycle 0. insertion_pairs reads five pairs; four are balanced schedule
swaps inside the folded header groups and the fifth, open to the end, is the
one candidate-only word: the final call's own load of the 0xC0 dim colour
(line of the call, class const). Aligned residual after shadow 137.

Mechanism, measured on the instrumented uopt and on small test units in a
private scratch directory. The call's argument registers are precoloured
against every web in the call's uopt block, and the web that IS a given
argument is exempt for its own register: a type-3 variable (the node-24
index takes a1), a type-1 address (the counter address takes a3), a type-4
expression (the node-20 pointer takes a1 in block 1). A type-2 integer
constant is never exempt, even when the argument names it: with the callee's
third parameter declared u32 the argument does reference the constant web
(ugen emits a copy instead of a second load) and the record still reads
forbidden0=0x7fc30000, a2 excluded. In a unit with no unsigned-to-float
conversion the same shape matches anyway, because as1 renames the constant
load into a2 to delete the copy; here ugen's branch for the u32-to-f32
conversion puts the load and the copy in different as1 blocks, so the rename
cannot happen. That is why every constant spelling was +4.

The edit. Spell the value as a product with zero plus the colour, which uopt
does not fold before colouring. The value becomes an expression web, it is
the call's argument, it takes a2, and one load serves both colour stores and
the call. Size delta 0, frame 0x20 with the target's three-slot ladder,
52 relocations as before. Measured factors: the node-24 counter global 137,
the local index 144, the length 147, the order index 185 at +8, the node-24
A index 158 at +4; operand order inside the expression is inert. Measured
+4 and inert: (x - x), (x & 0), (x ^ x), float and pointer cast forms, a u8,
s8, u16, s16 or u32 parameter type with every local type and call cast (180
cells), reading a colour field back as the argument (forwarded with a mask),
a copy made before the earlier call, a reassigned parameter, and a for-loop
initialiser.

Then, on the new shape (L146 voids earlier order results): a single-move
tail climb over the 19 tail statements under their real dependences found
one word, the text store after the root header, 137 to 136, and is a local
optimum. The length local becomes s32 with a raw store and a (u8) conversion,
which makes the length store take the raw return register as the target's
does; 72 local, store and conversion type cells are flat at 136.

Aligned buckets. Before 79 exact, 104 naming, 1 immediate, 32 structural at
+4. After 79 exact, 101 naming, 1 immediate, 34 structural at 0; the
positional count falls 143 to 136, most of it the removed insertion shadow.

Next lever. The function is now in the delta-0 pool and the colour
instruments apply. The forced-colour ceiling in this shard (127, then 125)
was measured on the +4 shape and must be re-derived, not inherited. The
remaining structural rows are schedule: the root second header's constant
materialisations, and the tail, where the target issues the text global's
load and the chain-head reads before the counter bump.
#### 2026-09-23, lane B3-o101: natural dim-colour spellings and tail rows

No change to the source; 136 at delta 0, frame 0x20, 52 relocations. Everything below was measured on A, whose body B shares row for row; the B shard's own numbers are unchanged.

Natural spellings for the dim colour, all measured on A against the 136
diagnostic (masked, size delta): plain 0xC0 148 +4; file-scope static const
158 +4 and function-local static const 158 +4 (both a load); an enum
constant 148 +4; (u8)~0x3F 148 +4; the callee unprototyped with an s32, u8
or s8 dim local 148 +4 each (unprototyped with the diagnostic kept stays
136); float-derived forms through the live 1.0f opacity scale (192.0f times
the scale, the scale times 0xC0, 0xC0 times the scale converted, 0xFF less
63.0f times the scale) 161 at +16 or 164 at +28, because uopt folds no float
operation; a chained colour1 = colour2 store 148 +4; a masked or shifted
read of the root's already-stored colour byte 159 +8. None replaces the
diagnostic. What does score 136 at delta 0 are other annihilating
operations on the loaded counter: 0xC0 OR (count AND 0), count % 1 + 0xC0,
0xC0 >> (count * 0). So the fold that matters is ugen's, not uopt's: uopt
keeps any binary operation with a loaded operand as an expression web, and
ugen then folds a literal-zero or literal-one operand to $zero, emitting
the single addiu from $zero the target has. B2's note that x & 0 folds
early holds only for a local operand. A natural source therefore needs a
non-constant operand that a literal annihilates -- plausibly a macro
parameter passed as 0 by this builder -- and no such parameter is visible.

Tail and second-header rows, measured on A at 136: moving the two
chain-head loads before the opacity, the length, the kind, the colours or
the index read 147 to 150; mode before kind, folding kind/mode, the
colours, the chain-head pair or the node stores onto one line, all flat at
136; folding the counter bump and root stores 138; the chain-head values
written straight into the node (no carrier, L145) 139 to 140 on four line
layouts, because as1 then keeps the node stores ahead of the root stores
(it treats the two bases as possibly aliasing). Second header: folding the
node-32 group into the header line 138; moving the order-count read to
the end of the header, or the handle store to the front of its group,
flat. Register reading: the target holds the node-24 chain-head values in
t7/t8 and the node-20 ones in t4/t5, while ours colours both pairs
t1/t2 (webs 159/174 and 93/97, save 1.5/1.0 and 2.0). t7/t8 are outside
this procedure's colour table (c1 v0 to c12 t5, c14 s0, c15 s1), so in the
target at least the node-24 pair are ugen ring temps, not coloured locals,
yet their loads precede the root stores and their stores follow them.
That is the named question for the tail: a carrier that is not a
colourable symbol but survives across the root stores. ADR 0018 reached:
three consecutive layout and carrier attempts with no better residual.
#### 2026-09-24, lane w4-o101: natural spellings of the * 0 + diagnostic

The dim-colour line is `gOverlay101BuilderNode24CountB * 0 + 0xC0`. Three natural spellings were compiled against that object: the multiply commuted (`0 * counter + 0xC0`), the add commuted (`0xC0 + counter * 0`), and a parenthesized product (`(counter * 0) + 0xC0`). Each object differed, so the diagnostic stays. The earlier note that `(x - x)`, `(x & 0)`, `(x ^ x)` and casts fold early still stands; those were not re-run.

<!-- plateau-handoff:overlay101BuildPresentationB:end -->
