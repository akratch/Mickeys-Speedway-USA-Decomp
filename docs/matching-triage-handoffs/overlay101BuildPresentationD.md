<!-- plateau-handoff:overlay101BuildPresentationD:start -->
### `overlay101BuildPresentationD` plateau handoff

- source: `src/overlays/o101/overlay101BuildPresentationD.c`
- score: 130 differing words
- frame: 0x20
- relocations: 52
- first mismatch: +0x10
- summary: u8 length 131 to 130. Packed forces 118, split-base 113, none score 0. L109/L131 do not raise the node-20 base save.

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

This relative is at delta 0 and has no colour constant, so the surplus word the
A, B and C relatives carry is absent here and the levers that would close it are
inert by construction. Its residual is 103 naming and 27 schedule order.

Lane `lane/p2-pres`, 2026-09-11, re-measured this function alongside its
relatives. The source is unchanged; everything here is a negative, and the
full working is in
`docs/matching-triage-handoffs/overlay101BuildPresentationB.md`.

- The forced-colour census reproduces: 131 to 123 to 117 to 114 on `p1:w186=c11`, `p1:w31=c2`, `p1:w81=c8`, the same three roles the A, B and C relatives name. Every force was checked for a
  `p1color` record carrying `forced` before its object was scored.
- The intra-group line-break axis, which the previous lane did not measure (it
  measured whole-group fold subsets), is flat. All 81 break positions
  across the folded groups: nothing raises aligned byte-exact above 82.
  One break, the first group at its fourth statement, reads 130 masked but 80 byte-exact and 29 structural, the same trade the other three make, and is not adopted.
- The tail statement order, widened from the previous lane's eight statements
  to the whole tail under its real dependences: 14,138 constrained orders
  searched by single-move greedy plus random restarts, never beating
  131.
- The displacement-pair sweep also ran here: at the 114 plateau, 11,286 pairs
  that move a coloured web off its colour AND force another web onto the colour
  it vacated, 2,358 of them accepted in the records, and not one beat 114. The
  interfering-web hypothesis is refuted on this relative as it is on B.

#### 2026-09-12, lane w1-1: zero-size sibling confirms the L160 limit

The exact assignment gate returned base-only. Baseline and retained result:
206 candidate and target words, delta 0; 132 raw and 131 masked differences,
first +0x10. Aligned buckets are 82 exact, 103 naming, 1 immediate and 27
structural. Structural includes 13 paired rows, seven candidate-only words
and seven target-only words. The paired displacement sites agree with A/B/C,
but D has no surplus colour materialisation at the trio's +0x31C. Its final
call has no colour argument. This is why the trio's size question must not
be transferred as a claimed D deficit.

There are 52 candidate object relocations and 34 in the assembled comparison
target, with unequal literal symbol identities. These are comparison-object
measurements, not an authenticated overlay relocation or linked promotion
proof. No new matching bytes are claimed.

This function's web_footprint --every-colour run covered 21 coloured webs and
133 accepted same-save-class forces. Of these, 106 stay at delta 0 and 27
increase size. Fourteen zero-delta cells beat 131; the best single is 123.
Re-rendering the saved JSON with --report nominates p1:w31=c2 and p1:w81=c8
as a disjoint packing, predicting 119. The measured pair is exactly 119.
The report also groups eight identical-radius alternatives as rivals, so
those alternatives are not additive extra credit.

The split node-20 array-base web is outside the coloured-web census. The
mapped B roles, with the existing D ordinal for the counter web, give
p1:w186=c11, p1:w31=c2 and p1:w81=c8. The independent stock/instrumented
fidelity gate passed. All seven nonempty subsets were accepted; singles
are 123, 123 and 127, and the three-force diagnostic floor is 114 at delta 0.
This confirms the existing forced floor, not a source match.

The identical L160 pointer-removal transfer maps B's declaration lines
179 through 184 to D's 161 through 166, and the node-construction/tail groups
to lines 170 through 203. All six pointer declarations and assignments are
removed and their uses become indexed accesses. D has no dimColor local to
remove. This independent compile returns the identical baseline object, with
no per-function tuning and no change in any aligned window or displacement
site. The lead's traced limitation is the implicit node-20 base's unchanged
save 0.5 against cost 2 after carrier deletion, rather than a hidden declared
cursor whose deletion supplies the target's colour.

The named next source question is the node-20 base's benefit and the remaining
header/naming allocation. No zero-scoring force-to-source route was established,
and copying the trio's colour-argument experiments here would test a feature
D does not have. The incoming guarded body is retained and the flat transfer
is banked; the prior order and force sweeps are not repeated.

#### 2026-09-19, lane w25-o101d: u8 length, independent packing, ADR 0018

Fresh configured baseline reproduced 206 candidate and target words, delta 0,
132 raw and 131 masked differences, first +0x10. Frame 0x20 matches the target
ladder at +0x1C, +0x18 and +0x14. Aligned buckets were 82 exact, 103 naming,
1 immediate and 27 structural, including seven candidate-only and seven
target-only words. Register census: 103 pure substitutions, 57 percent
coherence, six windows, no closed cycle.

Identity-gate: stock and instrumented `.text` agree (decomp-workbench fidelity
PASS on every gate). `web_footprint --list-procs` prints one procindex row,
proc 0 with 32 p1 decisions and 21 p1color records. The web set is unchanged
from the 21-web landscape, so `--every-colour` was not re-run. No saved
footprints.json accompanied this worktree; packing was taken from the named
L159 set and scored with `--object`.

Named packing and split-base lattice, all accepted, all delta 0, scored on
the forced objects:

- p1:w31=c2 123, p1:w81=c8 127, p1:w186=c11 123
- p1:w31=c2 + p1:w81=c8 119 (L159 packing, additive, disjoint radii)
- three-force floor 114 (w186 contends with both, +2 and +1)

No cell scores 0, so L160 is not opened. w186 remains split, save 0.5, nocs 2,
totalsave 1, bestcost 2, bestcolor 11.

Adopted: the A/B/C u8 length local, (s8) store cast, and no redundant mask.
131 to 130 masked, 82 to 83 exact, 103 to 102 naming, still delta 0, same
seven-and-seven insertion offsets. The 21 coloured webs and the three packed
web identities are unchanged. Re-measured lattice on the adopted body, again
identity-gated: singles 122, 126, 122; packing 118; three-force floor 113.
Every cell moved by exactly one word. Still no zero-scoring force.

Three consecutive non-improving attempts after that adoption, then stop
(ADR 0018):

- L109 discarded OR-with-zero on gOverlay101BuilderNodes20A after the node-20
  stores: 130, delta 0. w186 totalsave stays 1. The copy-prop narrow that made
  colour-constant probes inert on A/B/C also applies to this type-1 address
  constant.
- Inlining compactSelector to a 0x18 literal at the three use sites: 130,
  same aligned buckets and frame. The declared 0x18 carrier is not hiding a
  colour.
- L131 spelling split, node20B as array + index against node20A as
  subscript: 132, 81 exact, 104 naming. Reverted.

Retained body is the u8 length form. Decision variable is still the node-20
base's save 0.5 against cost 2, with no source-reachable extra occurrence
that copy-prop will count. Colour floor 113 is diagnostic, not a match.

<!-- plateau-handoff:overlay101BuildPresentationD:end -->
