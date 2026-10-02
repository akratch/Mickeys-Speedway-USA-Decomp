# Next wave after the overlay 1 promotion (2026-10-02)

This file is the single next-wave handoff. It replaces the previous
reconstruction brief. The byte totals below were recomputed from the
integrated tree (`gmake progress`, and the generated snapshot in
`docs/nm-ranking.md` / `config/nonmatching-ranking.us.json`) after the
overlay 1 promotion and the seven banked lanes were merged. Recompute
again before quoting them.

## This wave

One exact match. Seven other attempts, none of them matches.

**Match.** `func_overlay_001_F0002B4C_184EF2C`, 1,804 owned bytes.
`gmake promotion-proof` reported 451 words, frame `0xB0`, relocations
68/68, identity static. The lane's linked ROM hash is
`507341c0a40ca3e9a7cee969b396ee53facfb548`. The spelling that matched is
`for (sum = 0, k = 0; k < state->lap; k++)` after
`state = func_80005820(i)->state`: the zero stays with the induction, so
as1 fills that call's delay slot with the argument copy. The reversed
comma was measured and was not the match. Nesting the zero in the rank
test, wrapping the call or the zero in a region, and folding the zero
into the pointer did not beat the baseline. The TU's inherited
`-Wo,-loopunroll,0` was removed first and did not change the score; it
stayed off. `-Wab,-r4300_mul` stayed, because it is the proved hazard
schedule. The lever is checklist item 22 in `docs/LANE_BRIEF.md` and the
delay-slot entry in `docs/ido-learnings.md`.

**Banked improvement, not a match.** `rain_render_splashes` went from
105 to 100 masked words at size delta 0 (1,616 bytes, first mismatch
still +0x94). Each splash colour byte is `0xFF` combined with
`index - index`. The `0xFF` web itself did not move; the five words are
the index web. A second store of each colour byte and building alpha
from the other channels both grow the function. Stopped there.

**Plateaus (ADR 0018: three attempts, no better residual at size delta
0, nothing adopted).**

- `func_8001398C` (track.c) stays 8 masked of 330, 1,320 bytes, first
  +0x1A8. A 25-cell product of the surface-address and sort-preheader
  axes floored at 8. Putting the surfaces base in another local removes
  those words and costs a five-temp cycle. Do not repeat that product.
- `func_80051364` (anim.c) stays 12 masked, 1,148 bytes, first +0x88.
  No zero-new-block use raises only the command web at size delta 0.
  Or-zero and and-zero are byte-identical. Reloading `command` after
  the cursor increment raises the web and grows the function. That idea
  is eliminated.
- `overlay15InitStarsAndPalette` stays 17 masked of 247, 988 bytes,
  first +0x70. The matched siblings' static-field-through-pointer form
  scores 224 at size delta -4: IDO folds the field into the store
  displacement and deletes the address temporary. A late copy of the
  allocate result does the same. Palette index inits after the colour
  deltas are inert; after the palette load they are worse.
- `func_overlay_058_F00005FC_18AF7E4` stays 109 masked of 829, 3,316
  bytes, frame `0x88`, first +0x41C. Draw-loop addresses that do not
  CSE with the case-3 load leave case 3 byte-identical, and the
  spellings that fold to the target's offsets-first address do not
  split case 3. A named per-stage pointer grows the frame to `0x90`.
- `func_overlay_008_F00034A0_18611F8` stays 103 masked, 3,592 bytes,
  first +0x1C. Every denial of v0 that adds a use costs words. The
  two-argument event call is byte-identical to the three-argument call,
  because `selectedMode` is already in the third argument register. The
  previously forced 89-word shape was not reproduced from source.
- `func_800349A4` stays 172 masked of 272, 1,088 bytes, frame `0x40`,
  first +0x24. The only callee, `func_80034920`, is already one
  argument and void, which is what the relocation says. Moving
  `D_8007B680` to the cache compare puts the low half in the target
  delay slot and drops the high half. An extra local reaches the
  target spill slot only by growing the frame to `0x48`.

## The arithmetic

```
resolved 741,548 / 943,640 = 78.58%
80% = 754,912 bytes, gap 13,364
queue 134 functions / 186,904 bytes
```

`gmake progress` printed `resolved: 741548 / 943640 whole-program text
(78.58%)`. The ranking snapshot printed 134 queued identities and
186,904 bytes. The gap is `754,912 - 741,548`.

What is left is still reconstruction. The ranking's closest rows under
20 masked words are the plateaus and parked functions named below, not
a fresh near-match.

## Next concrete action

Do not reopen this wave's seven plateaus, the parked allocator set, R8,
`func_80028FCC`, or either whale, unless the assignment states a lever
the shard does not already record. One owner per translation unit.
Before any other edit on a new target, remove that TU's inherited
per-file overrides (`-Wo,-loopunroll,0`, `-Olimit`, `-O2 -g3`) and
re-score. Leave `-Wab,-r4300_mul` where a matched sibling in the same
TU already needs it. If the strip unmatches a sibling, restore it.

Then assign, only after `tools/ready_queue.py` and a zero-exit
`tools/lane_status.py --symbol` `base-only` verdict:

1. `func_overlay_001_F0001D78_184E158` (2,508 bytes, 268 masked, size
   delta 0). Different TU from the function matched above, same
   overlay, so the new match is the sibling template. Checklist items
   6, 7 and 22, in that order.
2. Overlay 101, one file each: `func_overlay_101_F000C6E8_18E7F08`
   (1,268 bytes, 106 masked) and the four `BuildPresentation` functions
   (824, 832, 832 and 832 bytes; masked 130, 136, 136, 136). The
   previous handoff recorded that one shape fix may close the four.
   Test that; do not assume it.
3. `overlay1LoadBuildRecords` (`overlay_001_head.c`, 2,288 bytes, 470
   masked, size delta -92). Size mismatch: `insertion_pairs.py` before
   any last-mile edit.
4. `func_8004B1DC` (font.c, 2,224 bytes, 452 masked, size delta 0). The
   permitted DKR `render_text_string` shape was recorded untried.
   Disclose it with a `PROVENANCE` note if it is adopted.

`track.c`, `anim.c`, `weather.c`, `textures_354C8.c`, `overlay_008.c`,
`overlay_015.c`, and the overlay 58 TU just attempted are free for a
*different* function only. Do not put a second owner on the function
this wave plateaued.

## Still do not assign

- R8, `func_overlay_008_F0001294_185EFEC`: 5,036 bytes, 7 masked, size
  delta 0. Three passes are already in the shard. This wave did not
  take it. A new lever only.
- `func_80028FCC`: 108 bytes, 10 masked. Recorded flat. Low value.
- Parked allocator plateaus: `func_80019AB8`, the overlay 73 draw, `joyRead`,
  `overlay17AdvanceChain`, `effectboxControl`, overlay 25's update, and
  the overlay 27 sibling. They stay parked.
- Whales, one new idea each and nothing repeated: `func_800517E0`
  (7,232 bytes, 1,782 masked, size delta -332) and
  `func_overlay_047_F0000B30_1891948` (8,672 bytes, 1,377 masked, size
  delta +4). Together 15,904 bytes, more than the 13,364-byte gap.

## What worked

Each item is a numbered entry in `docs/LANE_BRIEF.md`. The brief carries
the measurements.

1. Rewrite the shape; do not polish the plateau.
2. Fake globals are literals.
3. Remove inherited per-file overrides first. This wave's overlay 1
   strip did not move the score; the match was item 22. Still strip
   first. The flag has hidden matches on other TUs.
4. `-Wab,-r4300_mul` for the rotated branch-likely float loop.
5. Data defined in the TU, in ROM order, dropped at POSTPROCESS.
6. Sibling copy, once one function in the module matches.
7. Decode `tools/overlay_tables.py --json` before rewriting an m2c body.
8. Frame homes are declared locals, in declaration order.
9. Hidden ring draws from a natural narrowing, a store-then-reload, or
   a cast.
10. Basic-block count decides saved-register ties (`do { } while (0)`).
11. Expression-table order: the entry made first wins a tie.
12. Callee arity and return type are relocation facts.
13. Constant spelling is pool identity (`/ 2.0f` against `* 0.5f`).
14. A free assignment after a call fills that call's delay slot. Bind it
    into the for-init ahead of the induction. Brief item 22.

## Tools, in the order a lane uses them

| tool | what it answers |
|---|---|
| `tools/ready_queue.py` | which rows are `base-only` now |
| `tools/lane_status.py --symbol S` | whether a stale lane holds S |
| `tools/shape_lint.py S` | inherited artefacts, each with a product axis |
| `tools/donor_match.py S` | permitted-decomp counterparts; weak on unnamed callees |
| `tools/overlay_tables.py --json` | which relocation names are one object |
| `tools/fast_score.py S cand.c --diff` | one candidate, the TU's real flags |
| `tools/shape_product.py S cand.c --jobs 3` | a `#if SHAPE_axis == n` product |
| `tools/insertion_pairs.py S` | on a size mismatch, which construct emitted the extra word |
| `tools/align_symbol.py`, `residual_map.py`, `frame_census.py`, `register_census.py`, `draw_census.py` | residual bucket, window, frame, registers, ring draws |
| instrumented `uopt` (`CDX_PROC`, `CDX_FORCE`, `CDX_OUT`) | the price of one allocator decision |
| `tools/finalize_plateau.py` | a checked shard; if it refuses, write the shard by hand and re-measure |
| `tools/gates.sh` | verify, cleanroom, check-docs, check-tooling; its own exit status |
| `tools/merge_lane.sh` / `tools/merge_lanes.sh` / `tools/land.sh` | one match lane, then a bank-only batch, then land only a batch that contains a match |
| `tools/lane_fleet.py` | commits ahead, dirty files |

`gmake -j4` inside a lane. Never `tools/gates.sh --promotion` inside one.
Never edit the primary checkout while a merge runs. Promotion traps in
the brief still apply: `--redefine-sym` for every resident callee, no
shared `D_` name across overlays, `gmake overlay-syms` then a rebuild
after `extract` and after any `symbol_addrs.us.txt` edit.

## How to run the wave

1. `tools/ready_queue.py --scan 60 --top 20`, then `lane_status` on each
   chosen symbol. Prefer the four targets under "Next concrete action".
2. `tools/new_lane.sh <name> campaign/unchain`. One TU per lane.
   Reconstruction and near-matches go to the stronger model. Sibling-shaped
   batches are the only work the weaker model has returned matches on;
   it returned none on allocator plateaus.
3. Strip inherited overrides, re-score, then the tool order above.
   Adopt a cell only when `size_delta` shrinks, or masked words shrink
   at delta 0. Stop on ADR 0018.
4. `tools/merge_lane.sh` for a lane that matched. `tools/merge_lanes.sh`
   for a bank-only batch. `tools/land.sh` only when the batch contains
   an exact match.
5. Record a new reusable lever in `docs/LANE_BRIEF.md` the day it is
   measured, without per-function scores, and a generic entry in
   `docs/ido-learnings.md` when an exact result proved it.

## Known tool gaps, still open

This wave did not need them. They are still worth an hour each.

- `finalize_plateau.py` refuses a plateau block that is not an EOF
  suffix, and any tree with a dirty ranking.
- `donor_match.py` is blind when callees are unnamed `func_` symbols.
- `shape_product.py` needs an explicit `== 0` before a bare `#else` arm
  becomes a cell.
- The scoreboard's function count does not move on an overlay
  promotion. The byte total is the number to quote.
