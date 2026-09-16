# Whale handoff — 2026-09-16 (wv-x), at 6 words, unforced

`func_overlay_058_F000138C_18B0574`, 14,456 bytes, the largest unmatched
function in the tree. Nine bounded lanes have taken it **187 → 6 masked
words at delta zero**, unforced. This file is the resumption point; the
standing plan is `docs/WHALE.md` and the lane reports it links.

## State

    masked                 6 / 3,614 words   (99.83% of words byte-exact)
    raw                  184
    delta                  0
    frame              0x138
    relocations        1,253
    buckets      3611 exact / 0 naming / 0 immediate / 4 structural
    displacement tax       2
    first mismatch    +0x1314, the title-loop load (candidate-only +0x1314, target-only +0x1320)

Verified with `tools/score_symbol.py`, `tools/align_symbol.py` and
`tools/residual_map.py` on the tree source. The C remains guarded
`NON_MATCHING`; nothing is promoted. Source on `lane/wv-x`; the report is
[whale-pointer-walk.md](whale-pointer-walk.md) and the cells are under
`cells-wv-x/`.

Lane `wv-x` (2026-09-16) closed W: case 12's title loop walks a pointer
(the unused `cursor` cell) initialised inside the guard from
`D_o058_5C80 + 12`, a lineage that already spans the window, so no index
web touches W's blocks and both growth tests are the 48 body's. wv-w's
suspected structural difference was not in the block graph (cycle 0: the
emitted structure is the target's to the block) but in the neighbour count,
by exactly the one web every dead carrier adds.

## What remains: one load's schedule, and the fact behind it

The title-loop load `lw a3,0(s1)` is emitted first in its block where the
target has it in the call's delay slot. as1 sinks it past `sw t8,16(sp)`
only under a `.noalias` fact, which ugen emits for a load-address base
(`islda`) and never for a pointer variable (`isvar`), L95; the alias
profile's traces are banked. An indexed named array has the fact, and
then its index must be known zero at the preheader with no web in 183,
184 or 190 -- W's 202 test and the 5E9C piece's 191 test are both at
margin zero. That index is `i`, and `i` is bounded by three measured
rules: a call between a reset and the loop kills the cursor-init fold
(every loop form, every reset position before 183, mini TUs, `register`,
`-Olimit`); a redundant in-block reset folds the init and is then removed
by redundant-store elimination, while the delay-slot reset survives only
if a loop phi still reads it (every other use is folded to 0 first); and
dead-store elimination runs before the redundancy check, so an in-block
reset that overwrites the delay-slot def's last phi use kills it. The
title side and the row side each need one such reset. `c12a` (title reset
inside the guard, row reset inside the guard after the loop) is the
target's head through +0x1370 and loses the row init (+8); `c13a` adds
the row reset inside the row guard and loses the delay-slot def (50).

## The next cycle, named by wv-x

**A use of the delay-slot `i = 0` that survives copy propagation and is
not a loop phi, or a redundant in-block reset that dead-store elimination
does not count as an overwrite.** Concretely, untested: (a) the row loop's
guard and preheader as the *same* source block as the title loop's exit
without an else-duplicated save block -- the saves are on both paths in
the target, so this needs uopt to merge them, which it did not for the
else form; (b) a row-loop form whose induction is not `i` at the preheader
edge but a variable the row phi reads through `i` (a copy `k = i` placed
in 191 is folded, so the copy must be one uopt keeps); (c) reading the
loop-exit reset back: the target's `move s7,zero` at +0x1364 is on the
loop path only, which the tree's `i = 0` at 190 already produces, so the
row side of the target may be exactly ours and the title side the open
one -- in which case what is needed is a title-init fold from a def at
184 that leaves the title phi reading 180, i.e. a def that is redundant
*before* copy propagation runs. Read `whale-pointer-walk.md` "What a
known-zero `i` can and cannot reach" before any cell; every cell is banked
with its records.

## The system, named by wv-w

The split instrument's rule (wv-t: a block is accepted iff
`2*left_after >= numintf + new`) holds on all 477 growth tests of the 9
body, and the neighbour lists of W (23 in {183,184,185,190}) and the
`&D_o058_5E9C` piece (the same 23 plus the 5EA0 remainder) are itemised and
all the target's: six spilled-or-late symbol candidates (`arg0`, `state`,
`rowHeight`, `savedOffset`, `savedPosition`, `rowBase`), eight lineage
hulls, nine coloured webs. So W's shape is a function of three numbers,
and the constraints do not meet:

- W accepts 185 iff 12 colours survive the a1-a3 fold: 15 at its seed is
  one short. The margin is `v1` (w40, held since idx 136) or `s4` (`textY`'s
  dead def at 183). a0 never helps: free, it is folded too.
- W keeps 202 iff `2*left >= numintf + new` there. `new` at 202 is w157
  (`arg0 * 0xF`) plus the `&D_o058_5E9C` remainder whenever the 5E9C
  piece has rejected 202 -- which it must, or the tail's v1 piece is lost
  (+4). With 12 colours that is 24-23-2 = -1. wv-v's `c8c` (W `t0`, +4)
  passed 202 only because its 5E9C piece had taken 202 first.
- The 5E9C piece rejects 191 and 202 iff it has 12 after 185, i.e. `s4`
  held at 183 -- or `v1` held there before idx 127, the one colour W loses
  that the piece does not.

The consistent configuration is: **v1 held at 183 before the 5E9C split
(w40 above 3.6), no s4 holder at 183, and the 5E9C lineage's 202 seed
processed before W's 183 seed.** Each is closed at zero width (cycles
3-10): w40 is 3.0 in every partition (6/2 or 3/1); the seed walk is a CFG
DFS that follows case 1 and then pops the switch arms in **descending case
value**, so case 13's five seeds always precede case 12's and leave the
5EA0 remainder at 95/42 = 2.26; and the 5E9C remainder is 2.21 at 194 and
1.97 at 202. Alias-symbol diagnostics (`c8a`, `c9a`, `c10a`, `c10b`) move
the 183 seed to 5.9, 3.63, 3.34 and 3.08 and the growth records follow the
rule line by line (at 3.3: 185 accepted, 191 and 202 rejected with `new`
2), while the rest of the function moves from +0x104 on. They are not a
source.

## The cycle wv-w named (done)

**A form that changes a lineage's weight or a seed's order without an
instruction.** Concretely, one of: (a) a third occurrence of `n*2` in
block 13 or 183 that replaces an existing instruction (there is none in
the target's stream); (b) a `&D_o058_5E9C` occurrence in a case walked
after 12 (11/7, 10, 9, 8, 6, 5, 3, 2) that is loop-weighted, so its
remainder stays above 2.26 through its 202 seed; (c) something that
lowers the 5EA0 remainder's own ratio at 183 below 1.97 -- 13 weight out
of case 12's row loop, or 7 `nocs` in. If none exists, the honest reading
is that the original's case-12 tail or head differs structurally from the
exact-row reconstruction in a way that does not show in the words -- for
example a dead def coloured v1 at 183, which the target's instruction
stream cannot carry. Read `whale-growth-margins.md` "The three decision
variables" before any cell; every alias cell is banked with its growth
records.

## The cycle wv-u named (done)

**Colour the seven-reset web harmlessly without a force.** Done by wv-v:
the web's colour follows its symbol, so the resets are written on `textY`
and `letter1`; see [whale-index-carriers.md](whale-index-carriers.md).

## The cycle wv-t named (done)

**A web living in blocks 183/184/185/190 that is coloured `v1` before the
`&D_o058_5E9C` remainder splits at save 3.60.** wv-u found the flip is
real (two head probes take w40 to 4.0 and the piece rejects 202 with
`left 12`) and that no zero-width occurrence exists to produce it: probes
insert blocks that the target's interleaved block 183 forbids, aliasing
through `&D_o058_5E98` splits the `D_o058_5E9C` value web or poisons alias
analysis, and bare expression statements are dropped by cfe.

## The cycle wv-s named (done)

The per-web block-set dump and the growth records, built by wv-t
(`webblocks`, `seed`, `grow`, `growv`, `livbb`; `lineage_census.py --web`);
see [whale-split-growth.md](whale-split-growth.md).

## The governing principle, and why this session moved

Six earlier passes held at 187 by asking which colour a value wanted. That
is the wrong question.

**uopt colours a symbol as ONE web across the whole function, so which cases
a variable spans fixes the decision order.** Decision order is strictly
descending `save` among coloured webs — all 46 inversions are split
fragments — so every remaining swap is a *ratio pair* you change by changing
the arithmetic, not by asking for a register.

Three of the four wins follow directly from it:

- `textY` spans cases 1/2/8/9/10 and is decided early into s4. The target's
  case-2 rank difference carries a *late* web's colours, so `columnX`
  (cases 9/12/13) is the carrier that lands in s1. **−10 words.**
- `opponent` and `columnX` exchange roles in cases 12/13, both keeping their
  colours: the target's column X *is* its s0 variable and its inner index
  *is* its s1 variable. The roles were assigned to the wrong variables.
  **−22 words.**
- A declared carrier hides the web the target needs (L160); delete the
  declaration and let strength reduction generate the value. **−3 words**,
  and six other functions matched on this mechanism this week.

## Axes that are closed, with receipts

Do not re-dispatch against any of these.

| axis | finding |
|---|---|
| `p2` forces | zero `phase=p2` records on this procedure |
| joint `wA+wB=cN` forces | `uopt` refuses the grammar, exit 55 |
| the colour axis, first order | landscape at 48: **zero winners at delta zero** |
| the colour axis, higher orders | third-order at 90 and second-order at 85 both found zero winners; 90 is the single-force same-kind floor on the 157 body |
| `portraitX` placement ratio | closed from every placement of either def; the pair stays 15.5 vs 11.67 |
| entry-loop index carrier | 7 carriers byte-identical — not an interference question |
| switch discriminant copy | the reload is aliasing-forced and the target has it too |
| compiler flags | settled at `-O2 -mips2 -32` on the nop/branch-likely census |
| case-12 capture constraint | 7 attempt families, 27 source cells |
| L160 at the case-3 cursor | already compiler-generated; forcing its register costs 4 bytes through lost transition-address sharing |
| the case-12 reset's placement | closed at 18: any dead carrier lands it in the delay slot; the residue is fragment splits, not the reset |
| interference tokens as a lever | zero-width in the zero-store block only; a load elsewhere, and a declared token costs nine rows unread |
| declared pointer cursors for case 12 | fresh, dead-role or shared: exact loop head, paid through `&D_o058_5E9C` or `rowBase` |
| case-13 coordinate spellings with `opponent` | 17 head/row-preheader spellings, guarded reset, nine tokens: the head piece stays shed |
| `saves` carrier deletion (cases 9/10) | 28-36; the operand order is below the records |

| a dead carrier web at 183 | the itemised identity: the two pieces share every window interferer but W's own remainder, colours differ by `v1`/`a0`/`a1` timing only; no such body passes both tests |
| zero-width occurrences | `if` probes insert two blocks each and every web live across them gains `nocs`; probes after a reset are not folded; bare expression statements are dropped by cfe |
| aliasing `D_o058_5E9C` through `&D_o058_5E98` | case 2/3 loops: 3.20 and 39 rows or a -16 fold; the head read: splits the `a0` value web (+4); the row-loop negate: alias analysis poisoned |
| pointer walks in cases 1/2 | +24..+56 and the frame moves; the frame census is identical on both sides |
| the case-13 coordinate as any web but `opponent` | a real def counts an occurrence, so no coordinate beats the cursor's 31/2; portraitX can never be `s0` (`w26` holds it at blocks 9 and 85) |
| the case-13 row loop on `i` | exact locally (36 at delta 0) but splits the `D_o058_5EE0[portraitX]` SR temp shared with case 3: 20 rows there |
| a reset outside its preheader | before/inside a head call or above the if/else join: the index stays live into the loop, +12..+36 (wv-v cycle 1) |
| a dead-def web pushed later | `nocs = f(reference blocks)`, so n dead defs give n/(((n-2)>>2)+2) >= 2.33 and the web is always decided before `w222`, `w9` and the `a2` piece; only its symbol decides its colour |
| one carrier for all seven resets | the `D_o058_5EF8` address web spans the entry loop and both draw loops (splitting it moves the entry cursor `s3` -> `s2`), and `textY` is live in case 1's draw loop: two carriers are required |
| `s0` locals as the cases-1/2 carrier | `letter0`, `portraitIndex`, `countdownX` find `s0` held at 9/85, take `s1`, and flip their own case: 15 |
| freeing `v1` for W by moving w40 | w251 (2.33) takes `v1` instead; W's growth byte-identical (wv-v cycle 7) |
| a late case-12 carrier, also live at 191 | W t0 but the 5E9C piece keeps 13 colours, rejects 191 by one and takes 202: +4 (wv-v cycle 8, wv-t's k) |
| deleting the unused `s32 portraitX;` | 284 at delta 0: an unused declaration's symbol number orders later web numbers and their tie-breaks |
| the slot-address sum's spelling | wv-r's 14 and wv-v's 7 (pointer arithmetic, `(u32)`, byte arithmetic, reversed order, dead-local index) are one object; the base-first order is the byte form THROUGH a declared local, which is exact (wv-w) |
| case-12 head statement order | four permutations byte-identical; `textY = 0` above the head's calls +12 (wv-w cycle 2) |
| w40's save | 3.0 in every partition: 6/2 across blocks 13/183, 3/1 when split by literal type (width moves); `(s32)` casts do not split (wv-w cycle 3) |
| forcing W to t0 on the 9 body | +64 from +0x600, the caller-saved table reshapes (wv-w cycle 4) |
| memory class for the five spilled candidates | `*&x` is folded by cfe; `volatile` moves the frame and width on all five, so they are candidates in the target too (wv-w cycles 5-6) |
| an alias symbol for the 5EA0 lineage | the 183 seed moves as the rule predicts (5.9, 3.63, 3.34, 3.08) and the function moves from +0x104 on (-12 to -104); diagnostic only (wv-w cycles 7-10) |
| a pointer-variable base for the title-loop load, any pointee type | no `.noalias` fact (isvar/may-alias); the load stays above the argument store: 6 (wv-x cycles 2, 11) |
| pointer-difference subscripts | computed, +16..+28 (wv-x cycle 10) |
| `D_o058_5C98[i]` with the reset at 180, any loop form | the cursor init is `base + i*4`, +8; a call between the reset and the loop kills the fold (wv-x cycles 3-5, 7) |
| the reset at the top of 183 | the 48 form: 39, seven aligned rows, all the `move s7,zero` position (wv-x cycle 4) |
| an uninitialised index | loaded from its home, +12 (wv-x cycle 8) |
| a redundant reset inside both guards | the delay-slot def dies to dead-store elimination: 50 (wv-x cycles 13-14); inside the title guard only, with the row reset in the loop exit: the head exact through +0x1370, the row init +8 (cycle 12) |
| the row loop nested in the title guard, saves on an else branch | +8, structure moves (wv-x cycle 15) |

`p1:wN=s` split forces **are** honoured and `force_lattice.py` parses the
receipt, but every split measured so far costs width.

## Tooling built this session

- **`web_footprint.py --hold`** — the second-order landscape. Holds a force
  set applied while probing every other web, reading the probe plan from the
  *held* baseline's own trace, because held colours change which colours the
  other webs are offered. `--trace` is refused alongside it for that reason.
  Every landscape before this was first-order. It found 12 winners invisible
  first-order on its first run.
- **`lineage_census.py`** — the split axis. Use `bb`, never `line`: the
  member `line` is a constant 8 on this procedure and `webdetail.line` is −1
  for 330 of 395.
- **`draw_census.py`** — the ugen freelist/emission trace as a per-line
  census. A draw is `ALLOC_GP_RESULT`/`ALLOC_FP_RESULT` only.
- **A landscape freshness guard** — every report stamps the ranking's
  `source_context_sha256`, and `--report` exits nonzero with `STALE:` when
  the tree no longer matches. Five consecutive lanes once measured colours
  against a source that had moved underneath them; that is now an error
  message rather than a judgement call.

## Banked resources

`.git/whale-resources/`, outside every worktree, never tracked. Complete
instrument sets at **18, 48, 147, 157, 160, 169, 187** (footprints, landscape,
lattice, lineage, draws, residual, allocator), plus the held landscapes
`157hold5`, `157hold7`, `147hold7`, wv-r's cell tables under `cells-wv-r/`,
and wv-t's growth-profile traces: `allocator18-growth.log`,
`allocator48-growth.log`, the `allocator18-cell-*.log`/`cell18-*.c` pairs,
three `allocator18-detail-w*.log` neighbour captures, `uoptlist18.txt`, and
`cells-wv-t/` (every wv-t cell with its source and the scratch harness).
Lane wv-x adds `cells-wv-x/`: every cell, result and trace, the objects `c2a-6.o`, `c1a-14.o`, `c4b-39.o`, `c13a-50.o`, `c12a-2350-delta8.o`, the neighbour captures on the 14 body (`d1aW`, `d1aP`), the mini TUs under `mini/`, and under `alias/` the `cc -S` listings and the alias-provenance traces of the 9 and pointer bodies. Lane wv-w adds `cells-wv-w/`: every cell, result and trace, the 9 object (`unforced9.o`), neighbour captures on the 9 body (`d9W`, `d9P`) and on wv-v's `c8c` (`d8cW`, `d8cP`), the alias-lineage cells with their growth records, and `mini/m1.c` (the slot-order mini TU). Lane wv-v adds `cells-wv-v/`: every cell, result and trace, the 16 / 15 / 11
objects, and the readers `readsym.py`, `early.py`, `mkcarrier.py`. Lane wv-u adds `cells-wv-u/`: every cell's source, result and trace, the
reader scripts (`readW.py`, `intfdiff.py`, `peels.py`, `colourdiff.py`,
`case13.py`), the neighbour captures on the pointer and opponent bodies, the
case-13-exact 36 object, and the forced 11 objects `forced11-w22a3.o` /
`forced11-w22s4.o` with their unforced twin `c15-unforced120.o`. **Re-running a banked measurement is wasted budget**, and the
freshness guard will say so if a landscape has gone stale.

## Two repository faults fixed, worth remembering

- **`tools/merge_lane.sh` piped its own clean-room gate into `tail -1`** —
  the exact footgun `CLAUDE.md` documents, living in the integration tool.
  A failing scan reported `tail`'s status, always 0, so it read as a pass
  and the findings scrolled away, leaving one line of remediation advice
  with no finding above it. Now captures output, branches on the real
  status, and prints the whole scan on failure.
- **`nm_ranking.py --write-doc` is documentation mode and does not compile.**
  It validates the existing JSON and rewrites the Markdown from it, so
  running it against a stale row is a no-op that looks like a success. The
  measuring form is the default mode, then `--write-doc` for the document.

## Open housekeeping

- The per-pass shard `docs/matching-triage-handoffs/func_overlay_058_F000138C_18B0574.md`
  crossed the 262,144-byte clean-room `oversize` limit and now carries a
  `CONTENT_EXEMPTIONS` entry. The exemption is narrow — that path, that rule
  — and the word-table, hex-run and mnemonic-density detectors still apply to
  the file, but it grows every pass and should be split, with superseded
  prose archived to a sibling, before it doubles.
- `origin` reports that it has **moved to
  `git@github.com:akratch/Mickeys-Speedway-USA-Decomp.git`**. Pushes still
  succeed by redirect, but `CLAUDE.md` names the old URL and the remote
  should be updated before the redirect stops being honoured.

## The standing arithmetic

The whale is worth **14,456 bytes**, and credits **zero** until it fully
matches — a partial match banks nothing. At 6 words the function is 99.8%
matched by word; at 6 every remaining row is one instruction's schedule,
and that instruction's fact is the only thing between here and the
largest single byte gain available in the tree.
