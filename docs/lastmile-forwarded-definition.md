# Last-mile lane s1-b (2026-09-16): a forwarded definition, and a counter's web number

Four targets in four translation units, 3,960 bytes, all at delta zero on
arrival: `func_80034448` (54), `overlay19BuildSpatialMasks` (63),
`overlay15InitStarsAndPalette` (70), `func_80040B88` (72). Seventeen of the
twenty budgeted cycles were spent (a cycle is one batched hypothesis
measured with a private direct-`cc` harness that reproduced
`score_symbol.py` on every TU before use, including the `-Wab,-r4300_mul`
particles carries; every adopted number was then re-taken with
`score_symbol.py` on the tree). No target reached zero. Two improved and are
adopted: o19 63 to 17 and o15 70 to 60, both at delta zero, both still
guarded. Two are unmoved with their mechanism read off the allocator
records rather than argued from the score. Evidence (every cell's source
and object, decoded ladders, side-by-side listings) is outside the tree
under Git's common dir, `lane-evidence/s1-b/`; nothing ROM-derived is
tracked.

Where the dispatch, a shard, or the brief was wrong, first.

- The dispatch listed three of the four targets with no shard. All three
  have one: `func_80034448` 9.3 KB (three lanes), o19 5.2 KB (three), o15
  6.8 KB (one, 54 attempts). Reading them first is what kept this lane off
  the o19 point-carrier and o15 adjacent-move families.
- The dispatch names `docs/lastmile-block-budget-globals.md`; the file is
  `docs/lastmile-block-budget.md`.
- The o19 shard's standing line "prior declaration, lifetime, load-order,
  alias, loop-variable ... families remain closed" was false for the
  loop-variable axis: the bin counter's identity was the largest single
  lever on the function (63 to 38), and no prior section had read the
  counter's `save` off the records.
- The dispatch's neighbour readings did not transfer. o15's pool literal
  (`255.0f / 292.0f` in `DrawScreenStars`) has no counterpart in `Init`,
  whose only pool constants are the two the target also has, and its
  residual is allocation and scheduling, not a literal. Particles' L151
  zeros (`func_8003F154`) do not reach `func_80040B88`, whose one
  structural row is a pointer reload's block, not a constant's web. Both
  were checked against the listing before any cell; the check cost no
  cycle. Within-file mechanisms carried across neither pair.
- L99's "an unused `s32` is eliminated before the frame is sized" did not
  hold on `func_80034448`: an unused `s32` reserved a home like the
  pointer and float did (six cells, all frame 0x58). L121's wording (every
  declared local reserves a home) is the one that held.
- The brief's statement that a leaf emits p2 records only is too strong as
  written: o19 is a leaf and emits both `p1dec` (for its long-range webs)
  and `p2dec` (for the span-head webs). The call test's *consequence* held
  exactly: the p2 webs were the ones ordered by web number.

## Mechanism 1: uopt forwards a single-definition local past a call, and the target's source did not let it

Two of the four targets share one residual shape. In each, a local is
defined from an expression, used as a call argument, and used again after
the call; in ours the definition is forwarded into a type-4 expression web
that survives the call in a temp, and in the target the value is a symbol
web that lives in the declared local's own home (or, on o15, shares a
register with the local's later definitions in the same block, which only
one web can do).

On `func_80034448` the frame arithmetic (L53, L121: `raw10` plus frame
size against the ladder) says the target has eleven declared homes and one
4-byte temp; ours has ten and, on every CSE form, an 8-byte temp region
with one idle slot. Nine spellings of a declared aligned-address local
never produced a symbol web: single definition, two definitions,
self-referencing second definition, DKR's `u32` with `(s32)` casts, a
separate size carrier with the local as a copy-then-self-reference. The
records show the same type-4 web each time (`t2`, two blocks) with the
local's home idle. The reason is visible in the one form that *did* keep a
symbol web, the retained candidate's `loadSize = f(loadSize)`: its
right-hand side reads an operand that the definition kills, and that
operand's own reaching definition (`+= 0x20`) has two definitions, so
nothing upstream is forwardable either. Once the first definition is a
plain expression, uopt forwards it into the second, the second's operands
are never killed, and the expression is carried past the call as a temp.

On o15 the same forwarding is what colours the size `s3` against the
target's `s0`: `starIndex = count * 12` becomes web 4 (3 over 2 blocks,
save 1.5, coloured last), and `starIndex`'s symbol web starts at
`starIndex = 1`. `register` is inert; splitting the multiply into
statements costs an instruction.

The decision variable, on both, is a definition whose right-hand side
reads a killed operand or a memory operand at zero instruction cost. Not
found in seventeen cells across the two functions; a lane that finds it
closes the frame on `func_80034448` outright (the eleventh scalar is then a
free declaration) and the head of o15.

Two smaller facts from the same cells: the `? :` max is expanded by cfe at
every occurrence (+152 bytes for six), so a "max as a temp" cannot be
spelled that way; and every CSE form here reserved a temp slot it never
used, which reassociation and binding the sub-expression to a dead local
did not remove.

## Mechanism 2: a tie among loop webs is broken by web number, and a reused counter is not a fresh one

`overlay19BuildSpatialMasks` reused the span head's `firstItem` as the bin
counter with a probe on it. The records price that: web 19 has six blocks
(it lives in the span head too) and a total of 10330 (the probe's 1000
included), save 1721.7, decided *after* `binEnd` and `binStart`, which tie
at 9300 over 5 blocks, 1860. A fresh `s16` counter is also 9300 over 5; the
three-way tie is broken by ascending web number, and its first reference
sits between `binEnd`'s and `binStart`'s in the loop setup: 63 to 38 in
one cell, with declaration position inert (three positions) and every
non-`do` form of loop 1 losing it (`for` 70, hoisted-init `for` 34,
`while` 42) because `binStart`'s first reference then precedes the
counter's. A probe on the counter puts it ahead of `binEnd` (66).

The same reading closed the xMax/xMin pair: xMax (3100 over 8) after xMin
(3100 over 7); an `if (xMax)` probe inside the vertex loop, weight 1000,
lifts xMax to 512.5, ahead of xMin and behind `mask`: 38 to 24. Swapping
the mask/selector inits (19) and folding the two loop-1 inits onto one
physical line (17, an L59 tie; reversed on one line 52) complete the
adoption.

What remains on o19 is read, not guessed: the span pointer (web 14) and
`firstItem` (web 19) are p2 webs coloured in ascending web number; `span`
is created first (lineage event 5 against 7, first reference in the IR,
unchanged by declaration order) and offered `v1` first at cost 0. The
target has span in `a0` and `firstItem` in `v1`. Reading the span through
an expression re-reads the `volatile` offset per occurrence (+24 bytes), a
`firstItem` probe ahead of the span load extends its range over the back
edge (frame 0xC0), and both init probes are load-bearing (dropping either
is 52). The next hypothesis is a `v1` phantom at the loop head or an
earlier reference to `firstItem` that costs nothing; the selector/mask
init order is neither statement order nor a physical-line tie.

## The two unmoved targets

`func_80034448`, 54, six cycles: mechanism 1 above; the frame is solved to
the slot, and the shard section names the decision variable.

`func_80040B88`, 72, one cycle: its one structural row is the `trigger`
reload, which the target emits in the block before the `pointCount != 9`
branch and ours in the block after, where L125 pins it under the
`entry->pointCount` store. Five statement cells on that block (scale first
124, after the point load 109, between store and point 73, read-modify-
write +4) do not move it into the earlier block. The prior shard's L144
forms are still the only measured way to place a reload there, and they
grow the frame. The procedure-29 ladder is saved for the next pass.

## Ledger

Seventeen measured cycles of twenty: `func_80034448` 6, o19 6, o15 4,
`func_80040B88` 1. Adopted: o19 63 to 17 (aligner 164/61/0/2 to
212/12/0/4, one candidate-only word at +0x204, one target-only at +0x1FC,
commit `fce16690`); o15 70 to 60 (198/13/1/38 to 205/15/1/27, one
candidate-only at +0x1CC, one target-only at +0x218). Both delta zero,
frames unchanged (0x80, 0x40), both still under `NON_MATCHING`; `gmake
verify` prints the expected SHA1 on the committed tree and the ranking rows
were re-measured with `nm_ranking.py --refresh-stale` before `check-docs`.
Nothing was forced; every claimed number is `score_symbol.py`'s on the
tree. Three cycles were left unspent rather than spent on cells the
records did not motivate.
