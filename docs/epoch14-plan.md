# Epoch 14 plan of attack: search, don't stare

Date: 2026-08-28. Baseline `campaign/unchain` at the commit that adds this
file. Numbers below are the `gmake progress` / ranking snapshot at campaign
start; recompute before quoting them anywhere else (`CLAUDE.md`: derived
numbers are recomputed, never remembered).

## Why the previous mode stopped working

Matches per day (commit subjects starting `Match`) against plateau-record
commits: 08-24 291/37, 08-25 314/494, 08-26 17/125, 08-27 36/5, 08-28 0/~60.
The per-function "intelligent agent reads a diff" mode -- near-miss lanes,
crew pipelines, Opus/Fable trace-lever passes -- is exhausted on this tail:
its last 72 hours produced ~4 matches for ~3 M tokens, and its final night
produced sixty plateau notes and no match. Every productive event in the same
72 hours was a *tool* fix, not agent-hours: the permuter `-mips1` flag bug,
the objcopy scratch-fidelity fix, the stack-diff scoring fix, and the JFG
donor wring-out. After those fixes the permuter cracked four functions that
hand analysis had "proven" walled (`docs/matching-triage.md`).

## What is left, and which tool owns it

| Pool | Size at start | Owner tool | Why |
|---|---|---|---|
| NON_MATCHING, overlay | 285 fns / 299 KB (33% of the program) | **permuter sweep** (`tools/permute_sweep.sh`) | never searched by a sound scratch: the 2026-08-25 farm ran before all three fidelity fixes. These bodies' original "matches" were post-compile register-field swaps (`docs/acceleration-survey.md`), i.e. `perm_*` territory. |
| NON_MATCHING, resident | 86 fns / 53 KB | permuter sweep, then trace levers | same; the resident subset is where the headline function count moves. |
| bare GLOBAL_ASM, overlay | 164 fns / 110 KB | Codex "shape" lanes → permuter | luna proved it reaches opcode/frame-exact and then stalls on allocation; the stall is permuter input, not a plateau. |
| bare GLOBAL_ASM, resident | 95 fns / 102 KB (track 24 K, anim 18 K, fx 14 K, charControl 10 K, shadows 9 K) | Codex shape lanes → permuter; donors first | same pipeline; JFG/SDK donors are the cheap subset and are largely wrung out (`docs/jfg-final-wringout-2026-08-27.md`). |
| permuter-stuck (`P!`), measured by the sweep | unknown until the sweep reports | Opus/Fable trace levers (`docs/breakthrough-campaign.md`), ≤1 attempt each | only with a downward-trending score or a trace-named lever. |
| flat-from-the-start (`W` candidates) | unknown until the sweep reports | **deferred**; decides whether the g0-scheduler provenance build is funded | the instrumentation decision is made on this list's size, not on the current handful. |

## Phases

**Phase 0 (done 2026-08-28).** `tools/permute_batch.py` ported to the
`permute.sh` fidelity fixes and proven (`lightAdjustGlowingLight` re-found in 75 s from
its pre-match source; see `docs/permute-batch.md` "Scratch fidelity"). Lane
worktrees pruned 81 → 1, fully-merged branches deleted, plateau-note branches
kept. Workbench instrumentation branch merged and pushed; backlog items
#10–#12 filed. `shadowInitBuffers` evidence correction integrated.

**Phase 1 (running).** `tools/permute_sweep.sh permute-sweep`: full
NON_MATCHING queue, closest-first, 2 concurrent searches × 4 threads, 20-min
cap with one 20-min extension when the score is still descending, verify-once
promotion, one commit per match on `lane/permute-sweep`. Integrate with
`tools/merge_lane.sh permute-sweep` between passes (then `gmake extract` before
`gmake scoreboard`). Output: promotions, plus `build/permuter/summary.json` as
the measured `P!`/`W` list.

**Phase 2.** Shape lanes for the bare pool on free Codex: one mandate --
produce a compiling, frame-exact `NON_MATCHING` candidate and hand it to the
sweep; never spend a second pass on allocation. Order: the 39 overlays with
≤2 KB remaining (module closures), then bare overlay functions smallest-first,
then resident smallest-first. Each lane must prove it can build before it gets
a task (the last two Codex lanes died on macOS TCC `EPERM`).

**Phase 3.** Expensive models only on the sweep's `P!` list with a trace-named
lever, one attempt each, deep-debug non-transfers only once cheap candidates
are gone (`docs/matching-triage.md` transition rule). Fund the g0 provenance
build only if the sweep's flat list is large enough to pay for it.

## Standing rules for the epoch

- The permuter is the arbiter for allocation ties; no "no source lever"
  verdict is recorded before a sound sweep has run on that function.
- A sweep lane is resynced to `campaign/unchain` before every pass; a result
  is only valid against the source it searched.
- Verify-once: a score-0 that fails `gmake verify` is reverted and marked
  still-open, and the cause goes to the workbench backlog, not into an hour
  of debugging -- until the easy queue is empty.
- Ugly-but-verifying forms are kept and logged in `docs/cleanup-queue.md`.
- Every lesson that would help another project becomes a workbench backlog
  item (`~/Desktop/dev/n64-decomp-workbench/docs/improvement-backlog.md`),
  and each phase boundary includes a spike to land the top item.

## Exit criteria

Resident ≥ 1,100 / 1,460; resolved ≥ 45%; ≥ 20 overlays closed; ranking and
`docs/matching-triage.md` regenerated from sweep data; public mirror synced
after the gates.

## Status at the close of the first phase (2026-08-28 evening)

`campaign/unchain` a6968fff: 1,038/1,460 (71.10%), resolved 38.61%. Phase 0
and Phase 1 are done and the Phase 2 pipeline (shape lane → sweep) has
produced matches on both sides. Two additions changed the plan's overlay
half: `tools/promotion_trial.py` is the only sound oracle for overlay
candidates, and `tools/reloc_surface.py` generates the relocation surface
(the hand-written POSTPROCESS ritual is gone), so overlay work is routed by
trial class — ≤8 words to the annotated-target permuter or hand levers,
`rom-size` to the rodata-ownership carve, `schedule-divergence-at-site` to
reshape lanes. Sixteen bare resident functions remain (34 KB). The lessons
are in `docs/lessons-ledger.md`; the running fleet and the integration
recipe are in the private hand-off note.
