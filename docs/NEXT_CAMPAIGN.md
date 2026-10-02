# The reconstruction campaign: 77% to 80% and beyond (from 2026-10-02)

This file replaces the Track B plan. Everything Track B set out to do is done
and then some: between 2026-10-01 and 2026-10-02 the tree went from 61.4% to
77.4% matched, about 110 functions, with the methods, tools and traps written
into `docs/LANE_BRIEF.md` as they were learned. This document is the outbrief
of that sprint and the scope of the next one, written so that a less capable
agent can run it: what is left, in what order, with which tool, and what each
open function's recorded decision variable is. Every number was recomputed
from the tree on 2026-10-02; recompute before quoting (`gmake progress`,
`config/nonmatching-ranking.us.json`).

## The arithmetic

    resolved 739,744 / 943,640 = 78.39%   (public master e804103d8)
    80% = 754,912 bytes, gap 15,168
    queue 135 functions / 188,708 bytes

What is left is **reconstruction, not polish**. Roughly 15 queue rows sit
under 60 masked words; the other 125 are inherited m2c-shaped candidates at
60-100% residual whose author's shape nobody has written yet. The sprint's
yield numbers say what to expect: an Opus lane on a batch of such functions
returned one to four matches per session; a Sonnet lane returned zero to
one, and zero on every sub-60-word allocator plateau it was given. Spend
Sonnet on batches whose shape is already known from a matched sibling, and
Opus on everything else.

## What worked, in the order it was learned

Each item below is now a numbered entry in `docs/LANE_BRIEF.md`'s shape
checklist; the brief carries the measurements, this is the summary.

1. **Rewrite the shape; do not polish the plateau.** Every closure in the
   shard archive describes an inherited candidate, not the function. Read the
   target listing, write the function as its author would have, list the
   candidate's artefacts (`tools/shape_lint.py`), measure a product of
   natural alternatives (`tools/shape_product.py`). Cells that regress alone
   are exact together.
2. **Fake globals are literals.** A float extern whose relocation record is
   local to the module's rodata is a literal in the TU's pool; write it at
   each use. Closed o007, o009, o027, o043, o084 and parts of overlay 8.
3. **Remove inherited per-file overrides first.** `-Wo,-loopunroll,0` hid
   five matches (o010, o050, o047, o008, anim.c); the target's "hand-unrolled"
   copies were IDO's default unroller. `-Olimit` hid two in main.c.
4. **`-Wab,-r4300_mul`** is what produces the rotated branch-likely float
   easing loop from a plain `for`; it closed both 6 KB initialisers and the
   overlay 47/52/53/55/84 family needed it before anything else moved.
5. **Data defined in the TU.** Overlay initialisers and HUD code only match
   when the overlay's `.data`/`.bss` are typed statics in ROM order in that
   TU, dropped at POSTPROCESS with a rebind spec (overlay 54 TailA is the
   template; o050, o052 x2, o047 followed).
6. **Sibling copy.** Once a module has one matched function, its siblings
   share the author's idiom (bitfield flag words, direct global reads,
   `while (n--)` fills, indexed loops, one-line packet macros, early returns
   kept at the end, `(u8)` casts on byte arguments). o053 was closed by
   rewriting it as the cut-down copy of the matched overlay52TailB; eight
   overlay 57 functions fell this way.
7. **Decode the relocation table before rewriting an m2c overlay candidate**
   (`tools/overlay_tables.py --json`): which named globals are one object,
   which callee names are one callee. Two overlay 57 functions at 585 and
   356 words matched in one pass once the table had been read.
8. **Frame homes are a count of declared locals** (every declared local takes
   a slot, in declaration order); spill cells below them move with loop
   indexing; a padded struct or union standing in for locals is always
   inherited. `tools/frame_census.py` and `cc -g3` read both sides.
9. **Hidden ring draws** come from natural spellings: a masked narrow store
   (`field = (field - n) & 0xFF`), a store-then-reload of a global, a
   bitfield extract, a cast on a counter increment, a `(u8)` call argument.
   One such draw was worth 149 words on R8 and closed o084 from 37.
10. **Basic-block count decides saved-register ties** (a web's save divisor
    is `1 + floor((blocks + 2) / 4)`); a one-line `do { } while (0)` macro
    around a statement group supplies blocks at no instruction cost. Closed
    o035 and o063.
11. **Expression-table order breaks priority ties:** a dead read of the
    second element, placed before the compare, enters it into uopt's table
    first (track.c, 45 to 11).
12. **Callee arity and return type are relocation facts, not candidate
    facts.** A target that passes two arguments where the candidate passes
    three (overlay 101 TailAB4C, 499 to 100 on that edit alone; overlay 8
    F00034A0) or whose callee returns a value the candidate declares void
    (o090, o087, overlay 8 F0002640) colours every following register
    wrong. Read the callee's own definition or its matched callers.
13. **Constant spelling is pool identity.** `/ 2.0f` and `* 0.5f` are the
    same arithmetic and different pool entries; spelling one `* 0.5f` made
    it share a register with another `0.5f` and cost a function 400 words
    (vehicle_sounds). `0.100000001f` keeps a second `0.1` entry (objects.c).

## Tools, in the order a lane uses them

| tool | what it answers | cost |
|---|---|---|
| `tools/ready_queue.py` | which rows are assignable now (`base-only`), ranked | minutes |
| `tools/lane_status.py --symbol S` | whether a stale lane holds S | seconds |
| `tools/shape_lint.py S` | the candidate's inherited artefacts, each with a product axis | seconds |
| `tools/donor_match.py S` | permitted-decomp counterparts by shared named callees; weak when callees are unnamed | 1 s |
| `tools/overlay_tables.py --json` | the overlay's runtime relocation records (which names are one object) | seconds |
| `tools/fast_score.py S cand.c --diff` | compile one candidate with the configured flags and score it | 1 s |
| `tools/shape_product.py S cand.c --jobs 3` | every cell of a `#if SHAPE_axis == n` product, ranked | 6 cells/s |
| `tools/insertion_pairs.py S` | on a size mismatch: which word is extra or missing and which construct emitted it | seconds |
| `tools/align_symbol.py`, `residual_map.py`, `frame_census.py`, `register_census.py`, `draw_census.py` | the four residual buckets, per-window splits, frame homes, register cycles, ring draws per source line | seconds to a minute |
| instrumented `uopt` (`CDX_PROC`, `CDX_FORCE`, `CDX_OUT`) | price one allocator decision in words; confirm web numbers after an edit | minutes |
| `tools/finalize_plateau.py` | bank an improvement with a checked shard header (refuses dirty rankings and inline plateau blocks; then do it by hand: block, shard, `nm_ranking.py --refresh-stale`, `--write-doc`, gates, commit) | minutes |
| `tools/gates.sh --staged` | the five gates before every commit; read its exit status, never pipe it | 2-6 min |
| `tools/merge_lane.sh`, `tools/merge_lanes.sh`, `tools/land.sh` | integrate one lane / a batch under one gate run / land on master | 10-20 min each |
| `tools/lane_fleet.py` | per lane: commits ahead, dirty files, matches | seconds |

Promotion traps (brief, "Promotion traps") are the other half of the tool
knowledge: resident callees renamed to `_oNNNReloc` in the object's
POSTPROCESS rule, cross-overlay callees as `*Reloc` placeholders, rodata
externalised by digest, jump tables bound to the retained table, TU hash pins
repinned in the same commit, and `gmake overlay-syms && gmake` after any
header edit.

## The next phase, scoped

### Tier 1: priced near-matches (about 10 KB, one Opus lane each or a pair)

These carry a decision variable and a priced force in their shard; the work
is finding the source form that makes uopt take the decision itself.

| function | bytes | words | what is open (shard has the numbers) |
|---|---:|---:|---|
| `func_overlay_008_F0001294_185EFEC` (R8) | 5,036 | 7 | three allocator rankings (compare operand at +0xD8; `(s32) update` conversion takes v0; the unkFE load loses to a shift temporary); three Opus passes already; take it only with a new lever |
| `func_8001398C` (track.c) | 1,320 | 8 | surface-base load order at +0x1A8 and the sort preheader order at +0x400 |
| `func_overlay_001_F0002B4C_184EF2C` | 1,804 | 6 | uopt emits the bottom loop's `sum = 0` before the first call so as1 fills that delay slot with it; init placement and loop forms flat |
| `overlay15InitStarsAndPalette` | 988 | 17 | stars-store base a1 vs a2 and the block-1 tail; the static-field-through-pointer form of its three matched siblings |
| `func_80051364` (anim.c) | 1,148 | 12 | command web 3/3 against the clock value web 3/2; forcing both takes it to 5; every added block costs timeScale its f20 |
| `func_80028FCC` (main.c) | 108 | 10 | three return stores each reading its own ring temporary; 140 spellings flat; low value |

### Tier 2: banked reconstructions, exact size (about 25 KB)

Shape is right, allocation is not. Each has a window map in its shard.
`residual_map.py` first, then one product per window.

`func_overlay_058_F00005FC` (3,316 B, 109: the case-3 table-address web joins
the drawing loop's into one 26-block web offered only s8; the target keeps a
case-3-only web in s0), `func_overlay_008_F00034A0` (3,592 B, 103),
`func_80010B4C` (2,712 B, 167), `func_80053868` (anim.c, 4,820 B, 522: the
constant 2 hoisted into a2 in the target; axis loops counting in s2),
`func_overlay_001_F0001D78` (2,508 B, 268), `func_overlay_090_F00000FC`
(2,592 B, 327 at -8), `func_8001DD70` (2,132 B, 224: the records-address web
is only splittable by a wrong offset; needs a different idea),
`func_overlay_092_F0000308` (1,832 B, 119), `func_overlay_008_F00042A8`
(1,788 B, 240), `rain_render_splashes` (1,616 B, 105),
`overlay68UpdateAnimation` (1,424 B, 180), `func_overlay_012_F00003A8`
(1,384 B, 159), `func_800349A4` (1,088 B, 172), `func_80054B3C` (anim.c,
1,480 B, 349).

### Tier 3: reconstructions with a template (about 40 KB)

Inherited m2c shape in a module that has matched siblings. Method: items 3,
5, 6, 7 above, in that order, then the checklist. Expect one to three
matches per Opus lane of five.

Overlay 1: `overlay1LoadBuildRecords` (2,288 B). Overlay 57: nothing left.
Overlay 101: `TailC6E8` (1,268 B), the four `BuildPresentation` functions
(3,320 B, one fix closes four, 1,300 Sonnet cells found nothing). Overlays
19, 22 (402), 43, 45 (573), 56, 61, 11, 12, 64, 29 (one function each,
2,000-3,300 B; o061 needs a 9-state switch rebuilt, o056 is raw-offset
m2c). track.c: `func_8000E920` (2,168 B, 314 at +8), `func_8001291C`
(2,192 B, needs a rewrite from the listing). textures_354C8.c:
`func_800355A0` (105). font.c: `func_8004B1DC` (2,224 B; DKR
`render_text_string` shape untried).

### Tier 4: the two whales and the parked

`func_800517E0` (anim.c, 7,232 B, 1,782 at -332): a wholesale allocation
difference (one 293-word insertion pair spanning the body; the constant 6000
held in a3 across the loop; 20 more declared locals than ours, but per-case
locals were refuted). `func_overlay_047_F0000B30` (8,672 B, 1,377 at +4):
register colouring that every structural edit disturbs. Together they are
15,904 bytes, more than the whole remaining gap to 80%, which is why they
are worth one more idea each from a strong model and nothing from a weak one. Both need a new
idea, not another pass. Parked allocator plateaus under 60 words (o025, o027
sibling, o073, joyRead, effectboxControl, func_80019AB8, overlay17AdvanceChain)
returned nothing to three Sonnet passes each; do not spend Sonnet on them.

## How to run a wave (the coordinator's loop)

1. `tools/ready_queue.py --scan 60 --top 20`; pick targets per lane by TU,
   one owner per TU; prefer the tiers above.
2. `tools/new_lane.sh <name> campaign/unchain` per lane; dispatch with the
   brief plus the target's shard; Opus for reconstruction and near-matches,
   Sonnet for sibling-shaped batches only. Ten lanes run comfortably; keep
   `gmake -j4` in lanes and never run `tools/gates.sh --promotion` inside one.
3. On each report: `tools/merge_lane.sh <name>` (or `merge_lanes.sh` for a
   batch of bank-only lanes). If it stops, read the last lines of its log:
   the two recurring causes are a stale ranking row (`nm_ranking.py
   --refresh-stale`, `--write-doc`, gate, commit) and a build-state rename
   loss (`gmake overlay-syms && gmake -j4`). Never edit the primary checkout
   while a merge runs.
4. `tools/land.sh` after every batch that carries a match. Reclaim finished
   worktrees with `tools/reclaim_worktrees.py --apply --exclude <running>`.
5. Record new levers in `docs/LANE_BRIEF.md` the day they are measured; a
   lever that lives only in a report is lost by the next wave.

## The sprint in numbers

Two days, 2026-10-01 and 2026-10-02: 61.4% to 78.4%, about 120 functions
matched, roughly 45 lanes (Opus and Sonnet) coordinated from one session,
every batch landed on public master with the ROM byte-identical. Per-lane
yield: Opus lanes returned one to four matches per session on reconstruction
targets and closed every sub-20-word residue they were given except R8;
Sonnet lanes returned up to seven matches on checklist-shaped batches
(wave A) and zero on every allocator plateau and on every reconstruction
batch. Six matches were hidden behind inherited per-file overrides.

## Known tool gaps worth an hour each

- `finalize_plateau.py` refuses TUs whose plateau blocks are not an EOF
  suffix and any tree with a dirty ranking; lanes work around it by hand
  every time. Teach it to refresh the ranking itself and to accept inline
  blocks.
- `donor_match.py` is blind when a target's callees are unnamed `func_`
  symbols (most of main/). Naming callees in `symbol_addrs.us.txt` from
  their matched callers would make it useful on the resident residue.
- `shape_product.py` needs an explicit `== 0` to make a bare `#else` arm a
  cell; document or infer it.
- The scoreboard's "functions matched" count does not move on overlay
  promotions; the byte totals are authoritative.
