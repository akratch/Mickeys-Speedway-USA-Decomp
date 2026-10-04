# Grok handoff: the remaining route to complete executable accounting

Snapshot: 2026-10-04, integration source baseline `a1356f56e`.

## Current addendum

Recomputed from the built ELF after merging the published integration
branch: 834,612 / 943,640 resolved executable bytes (88.4460%). Remaining
109,028. `gmake verify` reproduced ROM
`507341c0a40ca3e9a7cee969b396ee53facfb548`. Functions stay 1,322 / 1,460.
`func_overlay_092_F0000308_18D6228` was already on `origin/master`: 1,832
overlay C bytes, 458 words, 33 relocation identities in that commit's
proof. This checkout reproduced the byte gain. The ranked queue is 61.
The local diagnostic commits below stay behind `NON_MATCHING` and are not
matches.

The follow-up requested on this branch is the cleanup queue: already-matched
source should read as ordinary C, and the stock object has to stay
byte-identical. `overlay7FillValues` now stores with an ordinary `*value = 0xF0`.
The comma operand was not required. The post-processed translation-unit object
matches the previous one, and `gmake verify` still rebuilds the US ROM. The
empty `!value` condition remains load-bearing. Removing only
`overlay1UpdateCountdown`'s impossible condition keeps every symbol size and
reschedules that function, so the condition stays.
`func_overlay_101_F0002510_18DDD30`'s bottom test and `sourceY` written from
`node->y + element->y` score 268 masked words at size delta +4, against a
proved baseline of 257 at -8. The body is not kept.
`refractOutputAssembler`'s per-slot end checks score the same 248 masked
words at size delta -52, against a proved baseline of 248 at -140. Aligned
really-different words fall from 175 to 168 and the total aligned residual
rises from 260 to 267. The frame stays 0x10 against 0x48. The body is not kept.
`func_8003C80C`'s colour destination, defined before the counter init, scores
115 masked words at size delta +4, against a proved baseline of 74 at delta 0.
The body is not kept.
`func_overlay_101_F0002510_18DDD30`'s y assignment, placed before the x
assignment, scores 262 masked words at size delta -8, against a proved
baseline of 257 at -8. The body is not kept.
`func_8003C80C` with `register` on the time parameter scores the same 74
masked words at 472 bytes. The body is not kept.

`func_80010654`'s matched-family lifetime packet is closed: an outer array
index scored 163, output-pointer copies left 162 words, and a difference
aggregate was scalar-replaced. `overlay83DrawStrip` is a diagnostic
improvement only, 69 to 68 masked words at the same 308 bytes, first
mismatch still +0x4. Dead zeros, pointer-copy placement, and an early
address local did not make vertexCount take the parameter register.
A folded or-zero identity on the saved display-list pointer was then
measured and left the 68-word comparison unchanged. It is eliminated.
`func_8003C80C`'s destination-pointer numbering packet is closed on the
unchanged 74-word body: storing alpha first scored 79, and defining the
destination before the source pointers grew the function to 119 words. The
register pool, including v1, already matched, so that lever cannot create
the missing colour. Defining that counter immediately before the colour
loop is hoisted back across the time loop and leaves the positional
residual at 74. `func_80028FCC` does not take second arguments 0, 1 and 2:
that pattern scores 12 positional words against the 10-word baseline and
was reverted. Overlay 66's filter was then reopened for one spelling:
binding the trailing 1 to the blue field before the channel OR. The
configured baseline stayed 249 masked words at 1184 bytes, and the owned
text was byte-identical, so that association is folded.
`MatrixMultiplyVec4` was then diagnosed on its configured full-TU build:
53 of 53 words, frameless, and all 47 differences are floating-point
register mismatches. The first divergence is the odd-single class, and no
source lever is supported. No source edit was made.
`func_8001FC50` was reopened for one size-sum order: add dataBytes48
immediately after the doubled matrix term. The configured ranking scorer
measures 271 differing words at 1332 bytes, size delta 0, first mismatch
still +0x18, down from 277. The predicted colour transfer did not happen,
and the early temporary is still a3 here. The body is retained and the
reopen is closed. `func_80009414` then moved its static-loop `unk8` load
to after the `unk4` test. The same scorer measures 234 differing words at
1684 bytes, size delta 0, frame 0x198, first mismatch still +0xC0, down
from 243. Reassigning `object` from `arg3` in the dynamic-loop test then
scored the same 234 words and was not kept. The sort remainder that
starts `j` at 1 then scored 256 words at the same 1684 bytes and frame,
and forming the entries address before the type test grew the frame to
0x1A0 and scored 286. Neither was kept. That body is retained and its
reopen is closed. None of those diagnostic edits is a match. `func_80016890` was
reopened for named product scalars and then closed with no source edit:
the proved full-TU baseline is 315 masked words at 2224 bytes, frame
0x1A0 against 0x190, with zero opcode and zero structural mismatches,
and the retained note already gives those products to `temp[]` at 291
words and size +16. `func_overlay_038_F0000154_1885E64` was reopened
for the overlay 92 union self-copy and closed without that edit: the
proved baseline is 159 masked words at 808 bytes and frame 0x38, and
the six pool relocations are one local base at offsets 0, 4 and 8. An
array and an extern struct of that pool each address-take it, score 180
words, and shorten the function to 804 bytes. Neither was kept.
`func_overlay_079_F0000FA0_18CDF40` was reopened for the same union on
projectedX and closed without keeping it: the proved baseline is 143
masked words at 736 bytes and frame 0x98, and the union grew the
function by one instruction, escaped translation-unit ownership, and
scored 159 raw words at the same frame. The four-constant array was
already measured at size +12 and was not retried.
`func_8005AF14` was reopened for float arguments to `func_8002B040` and
closed without keeping them: the proved baseline is 377 differing words
at 1852 candidate bytes against 1840, and the float calls scored 391
words at 1860 bytes. `func_overlay_066_F00004E0_18C6948` was
reopened for the screen-size if/else and closed without keeping it: the
proved baseline is 202 differing words at 764 candidate bytes against
816, and the if/else scored 225 words at 900 bytes.
`func_overlay_101_F0002510_18DDD30` was reopened for declaring y before x
on the repeated y sum and closed without keeping it: the proved baseline
is 257 differing words at 1164 candidate bytes against 1172, and that
form scored 259 words at 1168 bytes.
`func_overlay_045_F0001158_188D5B0` was reopened for float screen coordinates after the 0xA0 bias and the body was kept: the proved baseline is 566 differing words at 2692 candidate bytes against 2696, and the float locals score 561 words at 2696 bytes, size delta 0, frame 0x190, first mismatch still +0x4. That is a diagnostic improvement, not a match. The call-spanning left/right spill remains open. `func_overlay_001_F0001D78_184E158` was reopened for a per-region state local and closed without keeping it: the proved baseline is 268 masked words at 2508 bytes and frame 0x70, and assigning `D_1DA0` again after `overlay1FindClosestSample`, `amSndPlay`, and each `overlay7UpdateOwnerMode` scored 609 masked words at 2364 bytes. The frame stayed 0x70. That single local did not become the target a1 web. `func_800349A4` was reopened for the matched sibling's in-place `frameIndex` scale and closed without keeping it: the proved baseline is 172 differing words at 1088 bytes and frame 0x40, and scaling `frameIndex` after `nextFrame` is captured scored 263 words at 1120 bytes. Raw resident assembly (9,908 bytes) and the shared
overlay 88 body remain in the gap; their latest committed notes do not
name an untried source form that is currently assignable.
The owner requested this handoff because weekly model usage is nearly spent.
Continue matching; do not restart a tooling project or repeat exhausted sweeps.

## Start here

Read `AGENTS.md`, `docs/CONTRIBUTING.md`, `docs/CLEANROOM.md`, and ADRs
0001, 0002, 0011, 0017 and 0018. The public repository is
`akratch/Mickeys-Speedway-USA`; `campaign/unchain` integrates and `master`
ships. Every push publishes immediately. Use `tools/land.sh`; never force-push,
never push `upstream`, never disable hooks, never commit ROM-derived artifacts.

**This snapshot supersedes the numerical/target advice in `NEXT_CAMPAIGN.md`.**
That file contains owner edits and was deliberately preserved unchanged.
Its suggestions to inspect other workers' uncommitted work conflict with current
lane isolation: inspect committed lane refs through Git, request a handoff,
and never enter another live worker's worktree/index/process state.
Many functions in its old next-wave list are now matched. The current ranking,
current source guard, assignment gate and actual proof outrank old prose.

## Precisely what remains

These values were recomputed from the built ELF, current source, overlay atlas
and reviewed executable accounting, using `tools/progress.py --csv`.

| Account | Resolved executable bytes | Total executable bytes | Remaining |
|---|---:|---:|---:|
| Resident C plus verified handwritten assembly | 410,712 | 475,048 | 64,336 |
| Overlay C | 422,068 | 468,592 | 46,524 |
| Whole program | 832,780 | 943,640 | 110,860 |

Whole-program resolution is **88.25%**. Actual C is 815,596 bytes; the existing
verified handwritten-assembly allowance is 17,184 bytes. The function badge is
1,322 / 1,460 (90.55%): it is derived from resident ELF function ownership,
not a whole-program overlay-function census. Do not use its denominator to
count remaining overlay tasks. The next byte milestone, 90%, needs 16,496
additional resolved bytes. Recompute all numbers after each integration.

The whole remaining gap reconciles exactly:

- Ranked NON_MATCHING identities: **62 functions / 99,516 bytes**.
- Shared-body companion not separately ranked: **1,436 bytes**, overlay 88's
  `overlay88DrawSortedGeometry`, paired with overlay 69's ranked body.
- Raw resident assembly outside the candidate queue: **9,908 executable bytes**.
- Therefore 99,516 + 1,436 + 9,908 = 110,860. Finishing the ranked queue alone
  cannot finish the project. The progress tool reports NON_MATCHING 100,952
  and GLOBAL_ASM remaining 9,908.

The fresh ranking has complete context evidence for all 62 identities, no
unresolved or stale rows. See [the complete ranked inventory](nm-ranking.md)
and `config/nonmatching-ranking.us.json`, rather than a second stale copy.
A score is a diagnostic, never match credit; positional counts exaggerate a
one-instruction length shift. Atlas `matched: true` means C ownership and must
be read together with `nonmatching`, not treated as proof by itself.

## What Astra actually closed, and what to reuse

- `7b83943aa`: overlay 44 `func_overlay_044_F0000580_188BDE0`, 1,396 bytes,
  all 349 words and seven relocation records exact. Moving the real stride
  conversion earlier reproduced the desired allocation; a reviewed TU-local
  multiply-hazard flag supplied the required scheduling. See that function's
  handoff and `fbc1de13e` in `docs/ido-learnings.md`. Do not apply its flag
  globally or assume a flag change is needed on every floating-point target.
- **`b2cfa80bc`: `func_80010B4C`, 2,712 bytes**, all 678 words and nine
  relocation records exact, frame `0x148`, linked range and full ROM exact.
  It started at 167 differences. `a1356f56e` records the reusable learning.
  [The closure shard](matching-triage-handoffs/func_80010B4C.md) has the history.

The track closure is useful precisely because this was not a seven-word
permutation exercise. The decisive mechanisms were:

1. Existing declaration roles explained the stack homes. An unused padding
   local became a genuinely used third cursor; no storage extent was inflated.
2. Three actual pointer roles were necessary: first-copy source/failure-copy
   destination; failure-copy source/final-update destination; and the relative
   coordinates. Two generic pointers had conflated these allocation histories.
3. Three `*destination++ = ...` component stores, instead of fixed subscripts
   plus one final increment, naturally produced the allocation isolated by an
   instrumented diagnostic. Stock output improved beyond that forced control.
4. Initializing the loop counter before the other independent initializers
   recovered scheduling. A real selected-record pointer recovered the index
   expression's temporary allocation; repeated addition then matched.
5. Loading `radius[index]` between the two point-address definitions changed
   the compiler-generated cursor home. The last four differing words vanished
   without changing frame, size or program effects.
6. A local union with the actual point member and `f32[3]` array replaced the
   inherited `(&direction.x)[2]` scalar-member overrun and stayed exact.

One inherited unsigned OR-zero operation remains, with independent semantic
review, source disclosure and a cleanup-queue entry. It is initialized and
nonvolatile. Removing it is nonexact. Do not generalize that exception into
permission for arbitrary dead probes, fake guards, padding or undefined reads.

The full-TU baseline was captured after asm-processor preparation, replayed
through stock IDO, and self-compared with `candidate_context`. The diagnostic
instrumented build was proved faithful before its records were interpreted.
Forced output was never promoted. This feedback-loop discipline matters more
than generating hundreds of syntactic variants.

## Assignment reality: fix ownership before scheduling

A fresh `triage.py --target-pct 100 --top 62 --json` at this baseline found:
25 identities / 37,720 bytes `active`; 30 / 57,972 `already-integrated/exhausted`;
one / 2,168 `stale-ledger`; only the six matrix identities / 1,656 bytes were
`base-only`. These are committed-ref/evidence verdicts, **not a claim that 25
processes are currently running**. Do not accept the six-item route as a
recommendation to grind matrix code first.

Use the batch interface to avoid rebuilding the Git ownership scan per target:

```sh
python3 tools/lane_status.py --pending-only --json
python3 tools/lane_status.py --base campaign/unchain --symbols <comma-separated-symbols>
python3 tools/lane_status.py --base campaign/unchain --symbol <chosen-symbol> --json
```

Reproduce claimed matches from committed refs in your own lane; match subjects
are scheduling hints, not proof. Ask an actual active owner for handoff. Resolve
superseded claims through the existing claim-disposition mechanism with exact
claim/decision commits; do not delete refs or fabricate release evidence.
An exhausted target needs a **new, specific causal mechanism**, current source
and handoff pins in `config/lane-reopen-authorizations.us.json`, then a zero-exit
`base-only` verdict. A stale structured ledger needs maintenance reproof first; the snapshot's
stale row is `func_8000E920`.
Do not copy old authorization reasons onto new pins just to unlock a queue.

The following are research priorities **after those gates pass**, not currently
unconditional assignments or promises of easy matches.

## Recommended packets

| Priority | Target and bytes | Current masked differences / size delta | Concrete next mechanism; already exhausted |
|---|---|---|---|
| A | o008 `func_overlay_008_F00034A0_18611F8`, 3,592 | 17 / 0 | First address pair contributes 14 differences, loop-save order two, mode comparison one. Reconstruct the authentic call-return block and pointer lifetime. The trace links the required return-register exclusion to that block; adding a call or prolonging its result introduces unwanted words. Do not repeat those diagnostics as proposed fixes. |
| A | o027 `func_overlay_027_F0000624_187BFFC`, 1,016 | 12 / 0 | Two accepted diagnostic color decisions reach zero. Identify real call-argument webs live at the vertex-address definitions. Direct symbols, duplicate locals and cast round-trips collapse to the same web. A forced zero is not a source solution. |
| A | track `func_80010654`, 684 | 162 / 0 | Re-audit real aggregate views, pointer roles and definition lifetimes against the newly matched track family. Current shard has old frame/size prose; measure first. A fresh sibling scan ranks matched `overlay21ApplyPriorities` at 0.43, a shape hint, not donor identity. Do not copy the new track function's declarations blindly. |
| B | o092 `func_overlay_092_F0000308_18D6228`, 1,832 | 119 / 0 | Earliest FP load order/ring-phase divergence starts at +0x220. Trace that expression's evaluation and carrier identity; the prior natural smoothing loop/add-last rewrite improved it. Do not price it as a general register-color problem. |
| B | `overlay68UpdateAnimation`, 1,424 | 108 / 0 | Index-carrier reconstruction already solved the state/index allocation. Trace the first remaining temporary draw at +0xD4, then neighbor-pointer allocation. Existing unused cells are not evidence of original storage; independently audit them before any promotion. |
| B | `overlay17CreateChain`, 784 | 45 / 0 | Pre-call allocation-size home, template address definition regions and relocation count must close together. Target has seven records versus five candidate records in the recorded pass; all three call identities align. The template is independently bounded to 16 records. Do not normalize shifted sites away or repeat declaration hill climbs. |
| B | `func_8001E5C4`, 1,664 | 157 / 0 | Absolute-dot lifetime across the second square-root call: target spills the earlier value and reuses the register; candidate preserves it as one web. BSS ownership and matched sibling `func_8001EC44` are available. Bounce statement orders and the first three addition operand orders were already swept flat by Grok. |
| C | o069/o088 sorted geometry, 2,872 together | ranked owner 57 / 0 | One missing temporary draw propagates through later blocks. Ordinary casts, zero identities, regions, volatile forms and a bounded permuter were exhausted. Needs a causal ugen/as1 investigation that yields a defined ordinary source form, not another blind spelling sweep. Prove both overlay identities separately. |

The snapshot gate marks every recommended row above `active` except
`overlay17CreateChain`, which is `already-integrated/exhausted`. Reconciliation
is therefore the first action, not parallel dispatch.

For each row, open `docs/matching-triage-handoffs/<symbol>.md`, read both its
current header and the newest dated experiment, then compile V0. Some shards
put their latest pass near the top; their final paragraph is not necessarily
newest. No candidate should be selected solely by this table's low score.

Very small residuals can be expensive: `func_80051364` is seven words but
hundreds of conversion/cursor/state-order forms and Grok's canceling-cursor
edge were flat. Its live-range priority must change through an authentic use.
`func_80028FCC` is ten words in a 108-byte function; return types, Boolean
chains and conditional forms keep producing one colored result instead of the
required per-arm temporaries. Park both until a new mechanism is stated.

## Larger families: the work beyond the first wave

Queue totals by owned TU identify where a successful source model can amortize:

| TU | Queued identities | Target bytes |
|---|---:|---:|
| `src/main/anim.c` | 5 | 15,596 |
| `src/overlays/o047/func_overlay_047_F0000B30_1891948.c` | 1 | 8,672 |
| `src/main/track.c` | 6 | 7,912 |
| `src/main/shadows.c` | 3 | 5,576 |
| `src/overlays/o008/overlay_008.c` | 2 | 5,380 |
| `src/main/fx.c` | 3 | 3,820 |
| `src/main/charControl.c` | 2 | 3,796 |
| `src/overlays/o073/func_overlay_073_F0000190_18CAC50.c` | 1 | 3,040 |
| `src/main/frontend_37D50.c` | 2 | 2,780 |
| `src/overlays/o045/func_overlay_045_F0001158_188D5B0.c` | 1 | 2,696 |

The largest individual ranked target is overlay 47's renderer, 8,672 bytes,
1,377 differences at +4. Its proposed duplicate-symbol lead has already been
refuted: relocation records identify one object with the same addend. A
preserved walk-on-the-main-index alternative removes the extra word and fixes
several loop registers, but worsens other allocation. The next investigation
is the real index/speed/actor use distribution and region splitting, not fake
extern identities. `func_800517E0` is 7,232 bytes and still 332 bytes short;
its next pass needs structural/call/spill reconstruction, not a small-color
search. Read each shard before reauthorizing it.

Near size misses in overlays 12, 20, 45 and 101 deserve aligned structural
comparison before dismissal: their positional mismatch counts largely reflect
length changes. That is a research lead, not proof that one edit solves them.
Pair family anchors with matched siblings, and commit each exact function
individually before expanding a batch.

## Raw assembly: do not let the final bytes disappear from the plan

`raw_asm_census.py` reports six unclassified physical ranges totaling 9,924
bytes. The scoreboard's reviewed executable accounting leaves 9,908; do not
count the difference as a new match. Overlay raw carving is already complete.
The executable resident function inventory outside NON_MATCHING is:

| Owner / group | Function | Executable bytes |
|---|---|---:|
| `main/trackasm` | `trackMakePolylist` | 1,180 |
| `main/trackasm` | `getXZCompareMask` | 284 |
| `main/trackasm` | `getYCompareMask` | 152 |
| `main/trackasm` | `trackLightAsm` | 768 |
| `main/shadows_fp` | `shadowBoxPolyOverlap` | 196 |
| `main/shadows_fp` | `shadowBoundingBox` | 144 |
| `main/shadows_fp` | `func_80018544` | 272 |
| `main/shadows_fp` | `func_80018654` | 188 |
| `main/weather_snow_asm` | `snow_update` | 324 |
| `main/weather_snow_asm` | `snow_vertices` | 508 |
| `not yet split` | `func_80058FF0` | 444 |
| `not yet split` | `func_800591B0` | 5,024 |
| `not yet split` | `func_8005A550` | 112 |
| `not yet split` | `func_8005A5C0` | 112 |
| `not yet split` | `func_8005A630` | 192 |
| `not yet split` | `func_8005A6F0` | 8 |

Carve a TU and prove the ROM before attempting its contained function.
`docs/resident.md` already describes trackasm and the snow island as likely
handwritten; the current accounting deliberately still leaves these ranges
unclassified. The unnamed 5,024-byte function is a substantial ownership and
reconstruction task. A tiny listed size is not proof of a safe easy C carve.
Authenticate entrypoints, neighboring bounds, relocation and data ownership.

Likewise, the six matrix candidates total 1,656 bytes but have documented
odd-single-FP-register/toolchain concerns. Inspect the evidence in `matrix.c`
before accepting the automatic ready route. Establish original compiler versus
handwritten provenance; do not invent a compiler-output patch or reclassify
bytes merely to improve the badge. The goal's accepted metric includes
**proved** original handwritten assembly, not arbitrary fallback assembly.
`overlay57UpdateModeState` is already unguarded C and absent from the live
queue: its old unassignable entry is not a remaining-gap target.

## Reproducible matching workflow

Create a disjoint lane from the newest integration commit, then establish
ownership and a budgeted hypothesis packet. Keep one anchor and explicit stop
conditions; parallel workers need disjoint source/TU ownership, including
macro-included shared bodies. There is no universal worker/job ceiling.

```sh
tools/new_lane.sh grok-<target>-<date> campaign/unchain
# In the resulting lane, after the assignment gate passes:
gmake -j$(sysctl -n hw.ncpu) verify
tools/wb_compare.sh --summary-json <symbol>
.venv/bin/decomp-workbench diagnose build/wb/<symbol>.target.o     build_non_matching/src/<tu>.c.o --function <symbol>     --objdump tools/binutils/mips64-elf-objdump
python3 tools/sibling_scan.py --symbol <symbol> --top 5 --jobs $(sysctl -n hw.ncpu)
```

Use the actual object path printed by the build; overlay paths differ. Use the
machine's real core count. The workbench is required evidence, not an automatic
solver. Its aligned diagnosis distinguishes wrong width/association, CFG,
allocation, temporary-ring phase, homes and relocation identity. Consult
`decomp-workbench guide`; `campaign --stop-on-exact` can rank a bounded set.
Check installed CLI help: this environment did not expose `frame-ladder`.
The stock `-Wo,-zdbug:2` route crashed in float formatting on the track target;
do not spend a matching packet fixing that unrelated diagnostic path.

Capture/replay the **actual prepared full-TU compiler input**, not a snippet
with a guessed declaration context. Require self-context acceptance and stock
baseline text/relocation fidelity before trusting instrumentation. Debug `-g3`
may reveal homes but changes codegen; it is not the baseline. Web identifiers
change with source shape, so never transplant force IDs from another shape.
Keep source, object, score, first mismatch and hypothesis for every meaningful
attempt in ignored storage. If three consecutive hypotheses give no better
residual, new identity or eliminated explanation, preserve a plateau and stop
that mechanism. Continue a genuinely improving series within its packet.

## Promotion, contribution integration and economical validation

1. Require untouched configured IDO bytes, exact boundary/frame and exact
   relocation count/type/offset/identity. Synthetic/zero-proxy links are only
   diagnostics. Review widths, bounds, aliases and evaluation order separately.
2. Promote the ordinary source guard and ownership metadata. No executable
   post-compile edits, invented guards or dummy padding. An exact inert form
   needs independent semantic review, disclosure and the cleanup queue.
3. Run `python3 tools/promotion_proof.py --canonical --json <symbol>` and
   preserve the receipt privately. Regenerate overlay aliases with
   `gmake overlay-syms` after symbol edits/splat if resident references become
   unresolved. A fallback newer than its split receipt needs a real re-split,
   never a forged timestamp.
4. Refresh ranking/context evidence, generated ranking documentation and
   scoreboard; retire stale plateau markers, update the exact closure shard,
   resident/overlay ownership and symbol evidence. Keep authored handoff notes
   separate from generated documents.
5. Run `tools/gates.sh --promotion`, preserve true exit statuses, then integrate
   function-sized commits through `campaign/unchain` and use `tools/land.sh`.
   A shared-worktree full verify uses `tools/with_verify_lock.sh`; isolated
   lanes have their own build. The full promotion census can take about half
   an hour when the ELF changes. Budget for it, exploit its valid cache and
   do not restart it just to get progress output. A docs-only handoff needs
   focused documentation/clean-room checks, not a second unchanged census.
6. Treat Grok PRs and other lane commits as input: reproduce, inspect provenance,
   then integrate with the same proof. Do not merge a match claim on trust.

The public repository had no open PRs at the final read-only check on
2026-10-04. Recheck periodically with
`gh pr list --repo akratch/Mickeys-Speedway-USA --state open`; future Grok
contributions still require local reproduction before integration.

No broad tool rewrite is required to begin. Prefer a small source hypothesis
with a predicted observable and a proved baseline. Recent wins came from
source lifetimes and correct storage views; color forcing priced a problem
but did not itself solve it. The queue needs ownership reconciliation and
new causal packets, not another fleet repeatedly taking the smallest score.

## Final validation / local artifact handoff

The individual track promotion receipt says exact 678 words, nine of nine
relocations, static identities, linked ROM comparison, full-ROM identity and
overlay-relocation proof. All eight promotion gates passed, including all 98 tooling test files.
The final full census proved **2,129 functions, zero failures, zero uncovered**
(1,322 resident and 807 overlay entries), in 1,639.3 seconds with no cached
results. The handoff itself passes documentation and staged clean-room checks.
There is no unfinished matching search or failed validation to inherit.

This session's lane is `lane/astra-track10b4c-next-20261004`, hosted by the
reused worktree `../mickey-lane-codex-o036-expiry-review-20261003`.
After the closing handoff it is released for artifact recovery; no compiler
search or source work remains in flight. Private receipts live under
`build/astra-track10b4c/`: `actual-input.c`, `actual-args.json`, `driver.py`,
`probe*.py`, candidate sources/objects/scores, `fidelity.json`,
`self-context.json`, `promotion.json`, `semantic-review.txt`, `gates.log`,
`triage-final.json`, `raw-gap-functions.txt` and `assignment-inventory.txt`.
They are workstation-local and ROM-derived; never commit or publish them.
The portable source and causal conclusions are in the commits and shards.
Remote Grok workers must regenerate evidence from their own authorized ROM.

## Pasteable Grok kickoff

> Continue Mickey's Speedway USA toward complete executable resolution from
> the latest `campaign/unchain`. Read `docs/GROK_HANDOFF_2026-10-04.md`, then
> AGENTS, CONTRIBUTING, CLEANROOM and the applicable ADRs. First confirm the
> baseline/proofs and reconcile committed lane claims: the snapshot's ready
> route contains only matrix code and is not a good priority recommendation.
> Audit the entire gap, including the shared o069/o088 body and raw resident
> assembly. Pick one family anchor with a new, falsifiable source mechanism,
> obtain current pins and a zero-exit assignment gate, and work in your own
> lane. Use the existing workbench and matched siblings, prove actual full-TU
> baseline fidelity, preserve attempts, and do not repeat the shard's exhausted
> axes. Finish exact stock C with relocation, linked-byte and full-ROM proof;
> announce verified wins loudly, commit each function, integrate and land using
> the repository gates. Do not substitute tooling activity, forced output,
> arbitrary assembly reclassification or a good scalar score for a match.
