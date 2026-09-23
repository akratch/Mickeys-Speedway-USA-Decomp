# Track B: the small-delta campaign (from 2026-09-23)

The last-mile harvest is over. Waves w32 through w36 (2026-09-19) ran
thirteen colour lanes for two matches; wave w36 went 0 for 4. The cheap
delta-0 pool is thirteen functions, five of them barred or proved. This file
is the campaign that replaces it. Every number below was recomputed from the
tree on 2026-09-23 after the tooling batch landed; recompute again before
quoting any of them.

Run `tools/triage.py` before every wave (warm runs take seconds now; a run
after a merge refills the classifier cache and takes about two minutes).
Read `docs/small-delta-census.md` and `docs/forced-floor-census.md` next to
it. Those three are the assignment arithmetic; this document is the
strategy that consumes them.

## The arithmetic

    resolved 579,724 / 944,344 = 61.39%
    65% = 613,823 bytes, gap 34,099
    queue 260 functions / 346,588 bytes; 28 of them not assignable

| group | functions | bytes | masked words |
|---|---:|---:|---:|
| delta-0 (colour work) | 110 | 133,340 | 16,316 |
| small-delta, 0 < \|Δ\| ≤ 12 | 52 | 68,232 | 13,740 |
| big-delta | 69 | 123,096 | 27,780 |

Not assignable: 14 functions (13,616 bytes) are **colour-exhausted**, with a
proved forced floor above zero at delta 0 (`docs/forced-floor-census.md`);
7 are already integrated and 7 carry a stale ledger. Triage names them and
leaves them out of every route.

Each group alone could cover the gap to 65%. The delta-0 route is the
cheapest by words (33 functions, 34,928 bytes, 1,326 masked words), but
those are the functions colour lanes have been plateauing on for two
weeks; the words are cheap to count and expensive to move.

## What the insertion-pair census changed

`tools/insertion_pairs.py` (landed 2026-09-23) reads each small-delta
function's one-sided words: which pair they form, how much positional
shadow (L155) they cast, what class of word they are, and which source line
and ugen construct emitted them. `gmake small-delta-census` runs it over the
class.

The previous plan assumed the class was "mostly shadow". It is not:

    positional masked words 14,421
    of which shadow          3,629   (25%)
    aligned after shadow    10,792
    in-pair register naming  4,359   (an upper bound on what one fix drags)

So subtracting the shadow does not make these functions cheap; it makes
them *addressable*. 55 of 58 are fully owned: every extra or missing word
has a line and a construct. Labels: missing-CSE 28, spill/reload 11,
split-not-copy 6, extra-ISTR 5, extra-ILOD 4, control-flow 2, callee-save 1.
The label names the word and the line, not the spelling that removes it.
That spelling is the lane's job, and it is a different job from a colour
sweep.

## Wave 1: seven Track B lanes

One owner per translation unit; `dispatch_check.py` refuses a split and
refuses a small-delta symbol to a lane not marked `--track LANE=B`.
Targets are the census order, smallest aligned residual first, grouped by
TU so each lane's second target is in a file it already understands.

| lane | targets (aligned residual after shadow, label) | bytes |
|---|---|---:|
| `B-obj` | `func_80006EE4` (17, control-flow), `func_80006B04` (45, extra-ISTR), `func_8000831C` (76, missing-CSE), `func_800084C4` (91, missing-CSE, 81 of it in-pair naming) | 2,300 |
| `B-small` | `func_8006E7E0` (1, one frame word), `func_80024978` camera.c (12, missing-CSE), `func_80030610` sched.c (24, missing-CSE), `func_8002B040` matrix.c (29, spill/reload) | 1,240 |
| `B-ovsmall` | `func_overlay_008_F0001000_185ED58` (29, control-flow), `func_overlay_014_F0001830_1871108` (46, split-not-copy, frame +8) | 1,464 |
| `B-track` | `func_8000D820` (57, split-not-copy), `func_8000DB34` (94, missing-CSE, Δ −12), `func_800133FC` (97, spill/reload, one unowned word) | 1,416 |
| `B-fx` | `wakeDraw` (89, spill/reload, frame −56), `func_800479D4` (90, extra-ISTR, frame −8), `func_80049B14` (117, split-not-copy) | 2,304 |
| `B-o020` | `overlay20UpdateGrid` (57, missing-CSE, frame +72), `func_overlay_020_F0001148_1877720` (137, spill/reload) | 1,688 |
| `B-resmix` | `func_80037414` frontend (44, missing-CSE, frame +8), `func_800180B4` shadows (82, missing-CSE), `func_8004C690` font (92, split-not-copy), `func_80019AB8` lights (109, missing-CSE) | 2,724 |

Scope 13,136 bytes. Planned at the measured rate of the last two weeks
(roughly one match per five lanes once a class is past its first few
closes) this wave is worth 3 to 6 matches, 3 to 8 KB. It is also the first
measurement of the method itself; the second wave is planned from what the
first one reports, not from this table.

### The Track B lane protocol

1. **Cycle 0, uncounted:** `tools/insertion_pairs.py <symbol>` and
   `tools/residual_map.py <symbol>`. Write down the pair(s), the owning
   line, the construct and the class before touching the source.
2. **Change the owning line, not the residual.** A `missing-CSE` word is an
   expression the target computed once and the candidate twice; a
   `spill/reload` word is a home the target never allocated; a
   `split-not-copy` word is a web uopt split where the target's stayed one
   web (LANE_BRIEF laws L145–L154 on carriers apply). Re-spell that line
   and re-score. A size delta that reaches 0 is the milestone, even if the
   masked count rises: the function moves to the delta-0 pool and the
   colour instruments apply from there.
3. **Do not run a colour landscape on a nonzero delta.** The census shows
   why: at most 4,359 of 10,792 aligned words could be naming, and none of
   them can move until the inserted word is gone.
4. **A frame delta is a declared home**, and `tools/frame_census.py` names
   it. Close the frame before the words when both are off.
5. Stop on ADR 0018: three consecutive attempts with no better residual, no
   new identity, and no eliminated hypothesis. Then write the handoff with
   the pair, the line and the spellings tried, so the next reader does not
   repeat them.

## Track A, kept warm

Two delta-0 lanes at most, and only where triage reports a tight cluster
(identical word counts across siblings): the overlay 101 triple at 143
words each and the two `main` pairs at 154 words. One lead per cluster. No
other delta-0 dispatch until a lane reports a new mechanism.

## Do not spend a lane on this

- `overlay57UpdateModeState`: floor of 2, barred (`config/unassignable-symbols.us.json`).
- The 14 colour-exhausted functions in `docs/forced-floor-census.md`, in a colour lane. A structural lane may take one only with a named reason the handoff does not already refute.
- `func_8005ABA8`: the residual is an as1 branch-likely conversion with no C spelling.
- `overlay1UpdateRangeFlags` and the rest of the overlay-1 tail: one file, one owner, and that owner is not a Track B lane.
- Any function whose census row says `owned: no` as a first target. The three (`func_800133FC`, `func_8001EC44`, `func_overlay_001_F000438C_185076C`) each carry one unowned word; take them second, after a sibling in the same TU has taught the lane the file.

## Coordinator work, not lanes

- Done 2026-09-23 (lane tb-follow): the classifier cache is keyed on each
  symbol's own evidence (source, shard and ledger blobs and last-change
  commits, its authorization row) instead of the base commit, so a merge
  batch no longer refills it cold, and `lane_status.py --symbols` -- hence
  `tools/authorize_reopen.py` -- uses it.
- Six `objects.c` handoffs need a remeasure before their pins can arm;
  `overlay7UpdateOwnerMode` needs a new authorization with a reason.
- `gmake check-promotion-proofs` (lane tb-proofs) must reach zero failures
  and then join the `--promotion` gate set.
- The lane brief's colour-first ordering needs a Track B paragraph that
  points at this protocol; the instrument section already describes the
  reader.

## What the 2026-09-19 waves produced (kept for rate planning)

- 60.72% → 61.39% (+6,316 bytes) over waves w32–w36: two matches
  (`overlay1BendPathPoint`, `overlay34SortAndDraw`) and eleven plateaus.
- Every plateau handoff records a colour landscape that came back flat, and
  every one of those functions now sits in the colour-exhausted or unproved
  rows of the forced-floor census. That is the measurement behind moving the
  campaign to Track B.

## Footguns that still apply

Never pipe a gate. Renew reopen pins after the last handoff edit.
Regenerate generated files after a merge rather than trusting the merge.
One owner per TU. A force is a diagnostic. Score a forced object directly,
never through `score_symbol.py`'s recompile. And, new this week:
`tools/wb_compare.sh` declares a stock build; under any exported `CDX_*` or
`DKWB_*` variable it refuses until the caller says `--build-env forced`, and
an exact result without a declared stock build is `claim: unverified`, not a
match.

## Waves 1 and 2, measured (2026-09-23)

Thirteen Track B lanes on 34 symbols. Recomputed from the tree at landing:

    61.39% -> 62.34%   (+8,968 bytes, 16 matches, 13 lanes)
    a further 6 functions brought to size delta 0 and left in the delta-0 pool

Per-lane yield was 1.2 matches, six times the last-mile rate of the
previous week. Every lane reported the same two things: the reader named
the owning line every time, and the mechanism about half the time. The
recurring mechanisms it did not name are workbench backlog items 33 to 41
(ISA-hazard nops, self-reassignment copies, narrow-parameter stores,
memory-across-call, unprototyped callees, constant call arguments, L56
block pricing) and the corrected L99 in the brief. Frame census plus a
declaration lattice closed the frame on every match; a match's last step is
now order and layout, never colour.

Three closes used inert spellings (`| 0`, empty `do {} while (0)`) and are
in `docs/cleanup-queue.md`; two overlay 101 candidates at delta 0 carry a
`* 0 +` diagnostic that needs a natural equivalent before promotion.

Wave 3 should be planned from `docs/small-delta-census.md` in census order
again (34 functions, 56,540 bytes remain in the class), with the delta-0
leftovers from these waves as second targets in the same TUs.
