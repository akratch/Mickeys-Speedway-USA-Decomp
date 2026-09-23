# After 60%: last-mile harvest, then the size-mismatch method

The 60% goal is met. `gmake scoreboard` on the landed tree reads
573,408 / 944,344 = **60.72%**. This file is the next campaign, not a
restatement of the sprint that got here. Numbers below are recomputed from
`README.md`'s Progress block and `config/nonmatching-ranking.us.json` at
the same tip; do not carry them forward from an earlier message.

Run `tools/triage.py` before every wave. It is still the assignment
arithmetic; this document is the strategy that consumes it.

## The arithmetic

    resolved 573,408 / 944,344 = 60.72%
    remaining                370,936 bytes
    ranking queue            279 functions / 352,904 bytes
    unranked remainder        18,032 bytes (no NON_MATCHING candidate)

The ranking splits the reachable unmatched pool two ways:

| class | functions | bytes | share of remaining |
|---|---:|---:|---:|
| size-mismatch (`size_delta != 0`) | 156 | 238,000 | 64.2% |
| delta-0 | 123 | 114,904 | 31.0% |
| unranked | — | 18,032 | 4.9% |

Delta-0 by mechanism:

| category | functions | bytes | masked words |
|---|---:|---:|---:|
| other | 107 | 106,528 | 13,503 |
| register-only | 15 | 7,468 | 274 |
| schedule-only | 1 | 908 | 9 |

That is the ceiling of last week's methods: colour landscapes, L160
carrier deletion, draw census, and the spilltemps / web-number laws.
They operate at delta 0. They do not emit or delete an instruction.

The previous sprint treated every nonzero size delta as "structurally
wrong C". That is too coarse. Of the 156 size-mismatch functions, **80
are only 4, 8 or 12 bytes off** (one to three instructions) and those 80
cover **104,860 bytes**. Their masked-word counts are mostly L155
positional shadow of the insertion, not 200 independent mistakes. That
class is the bridge to 65% and beyond; last-mile harvest alone is not.

    65% = 613,824 bytes, gap 43,292
    last-mile cheap pool (delta-0, masked ≤ 10, excluding the barred
    overlay57UpdateModeState) = 17 functions / 8,488 bytes

Closing every cheap last-mile function reaches ~61.6%. Reaching 65%
requires either large delta-0 `other` functions (the 106,528-byte band)
or the small-delta size-mismatch class. Reaching 70% (gap 90,509) is
the small-delta class plus a slice of the large `other` band. The
remaining 133,140 bytes of |delta| > 12 are a later problem.

## Track A — last-mile harvest (this wave)

One owner per translation unit. `dispatch_check.py` refuses a split.
`overlay57UpdateModeState` stays in `config/unassignable-symbols.us.json`
(floor of 2, proved); it is the second-best ratio in the tree and must
not be dispatched.

### Batch 1, named next steps

| symbol | bytes | masked | TU | next step |
|---|---:|---:|---|---|
| `func_8000590C` | 2,876 | 0 | `src/main/objects.c` | **Matched and landed** (lane lm-0590c). Nested-add commute: ILOD off a forwarded temp is temp-first; bind to an existing isvar for object-first. |
| `overlay14CreateValue` | 384 | 2 | o014 | Tail count-load hoist over the key store. **Live: lane/lm-o014.** |
| `func_8005ABA8` | 444 | 2 | `src/main/models_5B300.c` | as1 delay-slot; suppressor (`.align` after else-arm label) is not reachable from C. Do not dispatch. |
| `overlay20RemoveEntry` | 212 | 2 | o020 | register-only. Shard: no spelling reaches the target colour; remaining lever is the instrumented free list. |
| `overlay34CreateRecord` | 500 | 0 | o034 | **Matched and promoted** (lane w3-o034). Post-call `record = candidate` rebind. |
| `overlay1UpdateRangeFlags` | 480 | 2 | o001 tail | register-only. **Same file as other overlay-1 tail rows; do not split.** |
| `overlay19BuildSpatialMasks` | 908 | 9 | o019 | as1 order on the mask/selector zero-init pair and loop-1 tail. **Live: lane/lm-o019.** |

`objects.c` is one owner. `func_8000590C` is matched, so the live
objects.c lane (`lane/lm-obj`) takes the five unroller-grown siblings
below. `func_8005ABA8` is not dispatched: the remaining two words are an
as1 branch-likely conversion whose suppressor is not a C spelling.

### Same-TU follow-on on `objects.c`

`-Wo,-loopunroll,0` was a NON_MATCHING claim, removed. Five other
candidates still carry the hand-unrolled loops that flag was papering
over; under the unroller they grow 12–700 bytes and their ranking rows
are current as size-mismatch:

| symbol | bytes | delta | masked |
|---|---:|---:|---:|
| `func_80004FE0` | 1,384 | +12 | 328 |
| `func_80009414` | 1,684 | +232 | 468 |
| `func_8000A39C` | 656 | +300 | 231 |
| `func_80005548` | 348 | +632 | 245 |
| `func_80004590` | 396 | +700 | 272 |

Rewrite each as the plain loop the unroller expects, then re-score.
This is restoration, not a new match attempt: until the size delta is
back to zero they are not last-mile targets. Three further `objects.c`
siblings sit at ±4 (`func_80006EE4`, `func_80006B04`, `func_8000831C`)
and one at +4 (`func_800084C4`); they are Track B once the lane has
the insertion-pair reader.

### Method, unchanged

Aligned bucket split first (`residual_map.py`), then records, then a
force scored against the *forced object*, then L160. Colour landscapes
already exist on these shards; do not re-run one unless the source
moved. Stop on ADR 0018: three consecutive attempts with no better
residual, no new identity, and no eliminated hypothesis.

Expected yield this wave: 8–15 KB if two or three of the 2-word
functions close and `func_8000590C` lands. That is ~61.3–62.0%, not 65%.

## Track B — the size-mismatch method (the rest of the 40%)

Last week's instruments answer "which colour" and "which web". They do
not answer "which IR node emitted the extra word". That is the method
gap `docs/NEXT_CAMPAIGN.md` of 2026-09-12 already named as item 3, and
it is now the campaign's load-bearing problem.

Build, in this order:

1. **An insertion-pair reader.** Input: `residual_map.py`'s
   candidate-only / target-only offsets plus the instrumented `uopt`
   graph. Output: the IR construct (an extra ILOD, a missing CSE, a
   split that should have been a copy, a loop that should have
   unrolled) that owns the ±4/±8/±12. Same shape as
   `web_footprint.py`: a number, not a guess.
2. **A small-delta census.** For each of the 80 functions, the pair
   offsets, the frame delta, and whether the extra word is a move, a
   reload, or a real op. Tracked summary only; objects stay ignored.
3. **Apply it to `objects.c`'s ±4 siblings first**, because that TU is
   already owned and the unroller lesson is in hand.

Do not dispatch a size-mismatch function to a colour-landscape lane.
Do not rank its windows by the positional count (L155). The 106,528
bytes of delta-0 `other` are still last-mile work; they are just
expensive last-mile work, and they wait until the 2-word pool is dry
or a proved-zero appears.

## Track C — housekeeping, not matching

- **`overlay96DrawObject` promotion-proof: done (2026-09-23).** The
  premise was inverted: `(0, 214356)` was right (the shipped record is a
  resident call to `func_800349A4`), and `(96, 0)` came from the
  generated name's shape. The resident identity reached the proof only
  through the function witnessing itself. See
  `docs/reloc-surface.md`, "The function under proof is never its own
  witness".
- **Stale whale reopen pin.** `config/lane-reopen-authorizations.us.json`
  still pins `func_overlay_058_F000138C_18B0574` (`ledger_commit`
  `9d581a54`). The function is matched. Retire the pin.
- **Forced-floor census and assignment-classifier cache** remain the
  two highest-payback tooling items from the 60% sprint. They are
  coordinator work; they do not consume a matching lane.

## What this sprint's waves actually produced (kept)

The 2026-09-12 plan's four waves ran, then the whale closed. Recording
the outcome so this scoping is against measured rates.

- 57.56% → 60.42% (+27,004 bytes). 19 functions, headed by
  `func_overlay_058_F000138C_18B0574` (14,456 bytes, 187 → 0 over ten
  bounded lanes).
- The two routes that produced matches were a proved-zero force plus
  L160, and the draw census. Colour landscapes without a source
  spelling were uniformly flat.
- Last-mile batches returned two to three matches early and about one
  per five-lane wave once the sub-20-word functions drained. Plan this
  wave at the later rate.

## Do not spend a lane on this

- `overlay57UpdateModeState` — floor of 2, barred.
- `func_overlay_054_F00005AC_189F24C` and
  `func_overlay_052_F000063C_189ACAC` — colour axis priced; residual is
  structural.
- The whale. It is matched.
- Any size-mismatch function whose shard does not yet have an
  insertion-pair reading. That is Track B's job, not a colour sweep.

## Footguns that still apply

The 2026-09-12 list is still the list. In particular: never pipe a
gate; renew reopen pins after the last handoff edit; regenerate
generated files after a merge; one owner per TU; a force is a
diagnostic; score a forced object directly, never through
`score_symbol.py`'s recompile.
