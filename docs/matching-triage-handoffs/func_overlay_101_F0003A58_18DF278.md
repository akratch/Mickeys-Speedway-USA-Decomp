<!-- plateau-handoff:func_overlay_101_F0003A58_18DF278:start -->
### `func_overlay_101_F0003A58_18DF278` plateau handoff

- source: `src/overlays/o101/overlay_101.c`
- score: 1461/1461 words
- frame: 0x50
- relocations: 199
- first mismatch: none
- summary: ROM-exact with stock IDO -Wab,-r4300_mul; 5844 owned bytes and all 199 relocation identities proved.

#### What moved, 2026-09-12, lane p11-o101

The prior packet's five-trial stall window, its `structure-mismatch` verdict and
its "next: prove node-pool base liveness across loader" were all answering the
wrong question. The residual was the carrier, exactly as on
`func_overlay_101_F00069E8_18E2208`, which closed 613 -> 0 the same day on the
same three edits. One rewrite, measured against the configured TU:

- before: size delta +8, positional masked 1421, byte-exact 456, register
  naming 628, immediate only 53, really different 384, displacement tax 356.
- after: size delta 0, positional masked 2, byte-exact 1459, register naming 0,
  immediate only 0, really different 2, displacement tax 0.

The edits, all three transferred rather than rediscovered:

- L145, delete every local that held an element address or an element index.
  `index`, `orderIndex`, `node20`, `node24` and `node32` are gone; every store
  is spelled through the array subscript of the counter global itself.
- L146, the group header takes `mode` first. The retained shape had `value14`
  first and `mode` seventh, an order measured on the carrier shape and void on
  this one.
- L131, spell the root as `gOverlay101OrderEntries[1 + group]`, the expression
  the matched siblings and the ROM-exact `overlay101Cleanup` use, not a
  `D_0.groups[group]` member path.

Relocations now agree at 199 with the shipped runtime table's own count, so the
extra address pair the prior packet could not explain was the carrier too.

#### What is left, and the axes that are now closed

Two words at `+0x1194`, an adjacent transposition. The target emits
`lw <text pointer>` for group 9's first text node and then the following node's
index multiply; this candidate emits the multiply first. Nothing else in the
function differs: prologue, frame, every other word and every relocation agree.

Measured cause, from `cc -Wa,-R`'s node dump for that block:

- The multiply's node carries `aftercycles 13`, the text load's `aftercycles 6`,
  so the multiply wins on priority rather than on a tie. It is ready one slot
  early only because the count increment for the current node was lifted into
  the preceding block.
- Group 9 is the only one of the sixteen text nodes that both carries mixed
  colour constants and is followed by another text node. The mixed colours are
  four extra `li`s, which is what gives the preceding block room to absorb the
  increment. The other six text-to-text junctions keep the increment in the
  block, where it forces the multiply's position and both sides agree.
- The block's emitted order is not the order `-Wa,-R` records, so a pass after
  the scheduler reorders it; that pass is not visible in this trace.

Closed by measurement, all on the full configured TU:

- Every pairwise swap of the seventeen `ADD_TEXT` statements, 136 forms. The
  shipped order is a strict optimum: only `childType`/`child` is a tie at 2,
  and the next best form is 14.
- Every single-statement move of the text store across the body, 14 forms;
  position 8 is the unique optimum and its neighbours read 65 and 96.
- Every two-macro split of the body across two physical lines, 16 forms. The
  best three tie at 2 and none moves `+0x1194`, so the L59 line axis does not
  reach this residual.
- Folding the two group-9 invocations onto one line reads 12, folding the whole
  group reads 22, and inserting a line between them is inert at 2. Line numbers
  therefore neither cause nor fix it.
- Declaration order of the two remaining locals, `register` on the length local,
  and `D_1D0++` for the increment are all inert at 2.

- The physical-line axis is now exhaustively closed. Writing group 9's two
  text nodes longhand and sweeping the line boundary across all 33 positions of
  their 34 concatenated statements leaves `+0x1194` as the first mismatch in
  every one of them; only the natural boundary at 17 reads 2, its neighbours
  read 5, 10 and 11, and the rest read 12 or worse. Per-statement longhand is 8.
- The L149 redundant-mask generator does not apply. `length` is the `s32`
  return of the length call, so `length & 0xFF` is not already masked: adding a
  second mask makes as1 emit a second `and` rather than fold the pair, worth 955
  globally, 218 on group 9's first text alone and 186 on its second. Dropping
  the mask entirely is 1322 at delta -64.

Next lever, named: the pass that reorders as1's scheduled stream after
`-Wa,-R` prints it. Nothing in the source axes above reaches it, so the next
attempt should instrument that pass rather than sweep another spelling.

## 2026-09-12 (lane `p12-close`): the two words are priced, and no source form reaches them

Still 2, and nothing was adopted. Baseline re-measured on arrival: 2 masked of
1461 words, size delta 0, byte-exact 1459, register naming 0, immediate only 0,
really different 2, displacement tax 0, first structural +0x1194.

The previous packet's reading was right about where the residual is and wrong
about which pass owns it. It said the emitted order is not the order `-Wa,-R`
records and named "the pass after the list scheduler" as the lever. There is
such a pass, it is a block-filling pass, and it is measured below -- but the
relative order of the two words in question is decided by the list scheduler
after all, on a priority that is not a tie and that no source form can move.

### What the two words are, read off the objects

The two differing words are an adjacent transposition, confirmed mechanically:
the candidate's word at +0x1194 equals the target's at +0x1198 and the
candidate's at +0x1198 equals the target's at +0x1194. Nothing else in the
function differs -- the other seventeen raw differences are all
relocation-masked words, which is why the masked count is exactly 2.

The candidate emits, at +0x1194, the index multiply for group 9's second text
node, and at +0x1198 the load of the first node's text pointer. The target has
them the other way round.

### The block, and why both are ready at once

`cc -Wa,-R`, identity-gated against the configured object (its .text is byte-identical to the configured object).

The junction sits in the block entered at the branch target of group 9's first
text node's unsigned-to-float correction. That block holds 29 instructions and
begins at +0x1190. Its first three picks are the float multiply at +0x1190, the
index multiply at +0x1194 and the text load at +0x1198.

Three numbers out of the node dump decide it:

  - the float multiply carries `aftercycles` 17,
  - the index multiply carries `aftercycles` 13 and, at block entry, `before` 0
    and `time` 0,
  - the text load carries `aftercycles` 6, `before` 0 and `time` 0.

Both are ready at the same cycle, so the pick is priority alone, and 13 beats 6.

### The pick rule, validated over the whole function

as1 picks, among the nodes whose `time` is at or below the current issue cycle,
the one with the largest `aftercycles`; the block terminator is scheduled last
whatever its priority. Checked against every pick in this function that had a
real choice: **5,334 picks, zero violations.** 3,094 of those were ties on
`aftercycles`, and the tie is not settled by ugen emission order alone -- the
lowest `INST` wins 1,581 of them and the highest wins 605 -- so a tie-break
lever does exist on this compiler. This site is not a tie.

### Why the index multiply is ready at entry, and why that cannot be changed

It is ready because the count increment that feeds it is not in this block. as1
runs a block-filling pass after the list scheduler that moves ready instructions
out of this block into its predecessor and re-schedules both, alternately, until
the predecessor's integer-multiply stall is covered. The trace shows this block
scheduled six times, 34 nodes down to 29, with the predecessor growing 11 own
instructions to 16 over the same five steps. The five it absorbed are the four
constant loads group 9's mixed colour bytes and its mode and kind constants
need, and the count increment. The 29-node schedule is byte-for-byte the emitted
block.

That is the whole of the previous packet's "lifted into the preceding block",
and it is correct. What does not follow is that the lever is there:

  - **The predecessor block is byte-identical to the target's.** It runs
    +0x1140 to +0x117C and not one of its words is in this function's raw
    difference list. The increment sits at +0x1168 on both sides. So the target
    absorbs the same five instructions into the same slots, and its version of
    this block therefore also has the multiply at `before` 0.
  - **The other state of that variable is observable and is also wrong.** At the
    six text-to-text junctions where the predecessor has no room, the increment
    stays in the block and both sides then emit float multiply, increment, index
    multiply, text load -- group 1's junction at +0x244 is the instance, and
    candidate and target agree there. So if the increment were not hoisted at
    group 9 the word at +0x1194 would be the increment, and the target's is the
    text load.

Both reachable states of the only variable are measured, and neither produces
the target's order.

### Both priorities are pinned by the emitted stream itself

`aftercycles` of the index multiply is 10 plus the 3 its `mflo` carries, where
10 is the integer multiply latency and the 3 is the `mflo`'s distance to the
block terminator through the following node's base address `addu`. `aftercycles`
of the text load is 3 plus the 3 its store carries, where 3 is the load latency
and the store's 3 is the same distance through the same `addu`, which is
anti-dependent on all twelve uses of the old base register.

Both successor chains are the target's own instructions -- 1,459 of 1,461 words
agree -- so neither height is a quantity the source chooses. 13 over 6 is not
reachable from any spelling that keeps this emitted stream.

### What was swept, and what it cost

All on the configured full TU, all flat at 2 unless stated.

  - Group 9's two text nodes written longhand as two physical lines, one line
    per node with all seventeen statements concatenated, is byte-inert at 2.
    That is the permutable form, and it changes no line number.
  - A complete move-one over each node's seventeen statements, 544 forms: **2 is
    a move-one fixed point.** 25 of the 544 are inert and every other form is
    worse. This sweep does move statements across the length call, so it covers
    block membership as well as within-block order.
  - A greedy two-move climb from each of those 25 inert forms, 12,800 further
    forms, all at 2 or worse.
  - Every uopt region boundary over group 9: `if (1) { ... }` and
    `do { ... } while (0)` around each of the 153 sub-spans of each node's
    statement list, 612 forms. Each is either inert at 2 or 1,385 and worse.
  - Four increment spellings (`D_1D0++`, `++D_1D0`, `D_1D0 += 1`,
    `D_1D0 = 1 + D_1D0`) applied to either node or both: all inert at 2.
  - Three `child` pointer spellings: `&(D_540[D_1D0])` and the `(void *)` cast
    are inert at 2; `D_540 + D_1D0` is 308 and 356.
  - Two `.text` spellings: inert at 2.
  - All 23 non-identity permutations of group 9's four statements -- the group
    header, the two text nodes and the sprite node: best 7, so the shipped order
    is a strict optimum there too.

### Next lever

There is no source lever here, and the next lane should not spend a sweep
looking for one. What would be needed is a change to the emitted stream at group
9 -- which is the target's own stream -- or a second, currently unmeasured input
to as1's dependence graph, since the graph is otherwise a function of the
instruction sequence and the pick rule over it is now validated at 5,334 picks.
The one such input this trace does not exhaust is ugen's `.alias`/`.noalias`
memory-disambiguation directives, which are not bytes and do reach the `before`
counts; whether any source form changes them at this junction without changing
the stream is the only question left open.

Validation: `gmake verify` printed 507341c0a40ca3e9a7cee969b396ee53facfb548.
The candidate is unchanged and remains `NON_MATCHING`, so no bytes are credited.

#### 2026-09-12, lane p23-lastmile: the scheduler flag closes the residual

The configured baseline reproduced the recorded 2 masked differences at zero
size delta. The exhaustive same-kind landscape planned 307 cells across all
42 coloured webs: 305 forces compiled, two webs had no alternative colour,
257 forces were accepted at delta zero, 43 changed size, and five failed the
strict acceptance receipt. No accepted force beat the baseline; the best
single force was w438 to c6 at five masked words. The improving-force lattice
is therefore empty, rather than evidence for an unmeasured combination floor.

The stock and instrumented function text agreed. A separate listing replay
through stock as0 and as1 also reproduced the complete text and all 199
relocations, after removing the assembly-driver-only pic0 and noglobal flags.
Ninety alias/noalias controls covered all pairs of the six relevant address
registers at the first text node, the conversion join, and the next text node.
Seventy-two were inert and eighteen regressed; none beat two. These controls
are ignored diagnostic artifacts, not edits to compiled matching output.

The decisive control was the configured stock compiler with only
-Wab,-r4300_mul added: zero masked differences, unchanged 5844-byte size and
199 relocations. The prior scheduler proof held this flag fixed, so its
claim that the instruction stream pins the only available schedule was too
broad. A g3 control regressed; an O1 wrapper invocation was rejected before
compiling and supplies no evidence. No C-body permutation was needed.

Impact scope is one single-function TU. The canonical rule adds the flag
there and rebinds its two resident callees through overlay 101's existing
relocation surface. No shared flag group changes. The guarded result was
recorded with finalize_plateau before promotion; the source reconstruction
is preserved and only its guard and stale source commentary are removed.

Private reproduction material is under build/p23/o101/: the complete colour
landscape, accepted-force records, stock and instrumented objects, faithful
listing replay, alias controls, and the flag-control objects. These paths
are ignored. Promotion and full-ROM verification results follow below.

Promotion proof passed: 1461 words, frame 0x50, 199/199 relocation identities,
using static metadata plus the runtime table and linked ROM. The owned range
is overlay 101 text +0x3A58 through +0x512C, all executable bytes, with only
compiler trailing alignment trimmed. Stock as1 replay with the scheduler flag
also equals the flagged C object in text and relocations, isolating the change
to that phase. No forced object or replay object enters the canonical build.

The atlas was regenerated before extraction; aliases were regenerated before
the first build and again afterward. The first build exposed expected stale
resident alias bindings; the second pass restored them and the build passed.
`tools/gates.sh verify` printed the expected and built SHA-1
507341c0a40ca3e9a7cee969b396ee53facfb548. This is new exact C credit for
5844 bytes. The remaining 17 raw pre-link differences were relocation
artifacts, resolved by the linked proof. README and the queue were regenerated.

<!-- plateau-handoff:func_overlay_101_F0003A58_18DF278:end -->
