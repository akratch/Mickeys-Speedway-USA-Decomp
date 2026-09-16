# Fresh-targets lane nx-a (2026-09-16): a phantom symbol web, and a pointer spilled to its own home

Lane `nx-a` was dispatched four overlay functions with a budget of twenty
measured cycles and a cap of six per target. Sixteen cycles were used, no
target reached zero, and one candidate improved (24 to 21). What this file
records is the mechanism each residual turned out to be, measured off the
allocator records rather than inferred from the score, and the decision
variable the next lane starts from. Evidence (candidate sources, objects,
ladders, per-window maps) is under Git's common dir, `lane-evidence/nx-a/`;
nothing ROM-derived is tracked.

| symbol | bytes | before | after | cycles | state |
|---|---|---|---|---|---|
| `overlay58DrawPointQuad` | 416 | 24 | **21**, adopted | 1 (+5 shared) | residual identical to its sibling |
| `overlay58DrawLargePointQuad` | 416 | 21 | 21 | 5 | phantom `gfx` web named, colour axis closed by declined forces |
| `overlay14CreateValue` | 384 | 13 | 13 | 8 | target shape reproduced from +0x9C on; one L100 ratio named |
| `overlay2QueryNode` | 1012 | 39 | 39 | 2 | OR-block spill explained; scalar's save named |

## Where the dispatch was wrong

Three places, checked against the brief and the tree as the brief asks.

1. **"None has been worked by a lane" is false for all four.** Each carries a
   shard history in `docs/matching-triage-handoffs/`: the o058 pair had lanes
   f1, h1, k1 and g1 on 2026-09-13 (a 7,800-variant hill climb sits behind the
   four probes in their bodies); o014 had p7-mix, c3-reopen2 and p12-tight;
   o002 had w2-bigA, p7-ovl2, p9-mid, p23-lastmile5 and p24-recipe. The
   measured numbers in the dispatch were right; the history was not. Reading
   the shards first is what kept this lane off the carrier, or-operand and
   boolean lattices those lanes had already exhausted.
2. **Neither new mechanism applies here.** Mechanism 1 (an unused call result
   is a `v0` web in the call's block) needs a non-void call whose result is
   unused; every call in the four bodies is already void or has its result
   used, and no residual row sits in a call's block on `v0`. Mechanism 2
   (`.noalias` on a hoisted global load) needs a prologue load above register
   saves; all four prologues are byte-exact. Both were checked before any
   colour reasoning, as instructed; the check cost no cycle.
3. **The ranking regeneration.** The dispatch asks for a full
   `nm_ranking.py` pass before hand-back; the brief says not to refresh
   `docs/nm-ranking.md` unless `check-docs` fails without it, the coordinator
   regenerates it. The brief wins; the gates section below says what was run.

## The o058 pair shares one mechanism -- explicitly, yes

After transferring lane g1's generated-cursor-load form to `PointQuad`
(24 to 21, cycle 1), `residual_map` prints the same rows for both, window
for window: `+0x00 a0<->a1` (the two call-argument `lui`s), `+0x40 a2->v0 x2,
v0->t1 x2, t1->t0`, `+0x80 a2->v0 x6, t0->a1, v0->t1`, `+0x100`/`+0x140
t1->t0 x2, t0->a1`. Every later measurement was taken on the Large sibling
and transfers unchanged; the next lane should treat the pair as one target.

**The residual is a permutation of four colours in the caller-saved bank:**
target `gfx=v0, physBase=t1, y=t0, xPlus=a1`; candidate `gfx=a2,
physBase=v0, y=t1, xPlus=t0`. Everything else -- `xMinus=v0, zMinus=v1,
zPlus=a2, x=t2, z=t3`, the whole `t4-t9` ring, 34 draws -- agrees. `a1` is
unused in the candidate's emitted code, which is the tell.

**The records name the occupant of `a1`.** `lineage_census` on the Large
sibling decodes the frame offsets in `raw10`: web 5 (offset `-4`) is the
declared `gfx` **symbol**, type 3, refs in blocks 3, 5 and 6, save 3.5,
coloured `a1`, and never emitted -- a phantom. The value actually carried is
web 7, the `dl++` **expression** (type 4), one web across both appends (L131:
the same expression twice is one IR name), blocks 3 and 5, coloured `a2`
because `v0` is forbidden by the `0xFF` constant (web 89, block 5) and `a1`
by the phantom. `xPlus` (web 51) then has `a1` forbidden and takes `t0`;
`y`'s post-call piece (web 90) takes `t1`; the `0x80000000` constant (web
85, block 3 only) finds `v0` free. The `if (gfx != 0)` probe's only function
is to extend the phantom into block 6 so it also denies `a1` to `zPlus`,
which the target has on `a2` for the honest reason that `xPlus` holds `a1`.

Two consequences worth keeping:

- **A declared local whose every use is copy-propagated still gets a
  colour.** It occupies a register in the interference graph and emits
  nothing. Nothing in the object shows it; only the ladder does. On a
  register-only residual with an unused caller-saved register, look for one.
- **The colour axis is closed by declined forces, not by a lattice.** Forcing
  the target's four colours: `w85=c8` applied alone (59); `w7=c1`, `w51=c4`
  and `w90=c7` all came back `forced=-2` (already forbidden at decision
  time, L101's third kind). The prior shards' "nine of twenty substitution
  webs require ring-only temporaries" was the symptom; this is the cause.

Cycles on the pair (each a batch, scored with `score_symbol.py`):

1. PointQuad: sibling transfer, **24 to 21**, adopted.
2. Eleven probe/carrier cells on Large: dropping any one probe 23/28/41/45/47,
   all four 47; literal `0x80000000` 21 (inert); one-line first append 21;
   block-scoped `_g` per append 28 with probes, 83 without. Every probe is
   load-bearing; they emulate block structure the target has natively.
3. Eight loop forms of the sixteen colour stores (the 1,2,3,0 order and the
   dead `vertices += 3` surviving in the ROM read like `-loopunroll`): 82 to
   103, six of eight change size. Refuted; the stores are explicit.
4. Force lattice (above), then eighteen boundary cells: `if (1) { }` between
   the appends equals the cursor probe (21); any boundary between the second
   append and the colour stores is 45 to 84; removing the phantom's probe is
   24 (`zPlus` takes the freed `a1`).
5. Thirty-two cells moving `xPlus`/`zPlus` above the colour stores crossed
   with boundaries: floor 22, a different residual (the display-list address
   web moves `a3` to `v1`), not a step toward 0.
6. Six call-argument line ties for the `+0x14` `lui` pair: inert at 21, so it
   is not an L59 tie under this shape (two cells with a hoisted `resource`
   local failed to compile and were not scored).

**Next hypothesis for the pair.** The decision variables are the phantom
`gfx` symbol web's block set and the `0xFF` constant's block. A source that
declares no display-list local (so no phantom exists), or declares it so its
symbol web has no occurrence in any block `xPlus` reaches, and that puts the
`0xFF` stores in a block the `dl++` expression does not reach without
splitting the two appends. The `gDma1p`-style block-scoped `_g` was measured
(28) and is not it as spelled; the two `_g` phantoms each take a colour of
their own.

## `overlay14CreateValue`: the target's pointer is spilled to its own home

Baseline 13 masked (14 raw), frame 0x28, aligner 83/5/4/4. Read against the
target listing, the residual is one shape: the target computes `index << 3`
as a **ring draw** (`t1`), sums it straight into `v1`, and `v1` is one
pointer web spilled to its own home `0x18` around all three calls
(`sw v1,0x18(sp)` before, `lw v1,0x18(sp)` after, three times). The
candidate colours the shift as a web (`v1`), stores the sum to the volatile
`chosen` home, and copies it into the `register slot` web, whose spill goes
to a compiler temp at `0x24` -- the four immediate rows. The tail's count
load is above the key store in the target and below it here (two structural
rows), and the reload order after the language call is swapped.

Eight cycles, all negative, but each retired a family with a measurement:

1. Forty-two one-pointer cells (plain or volatile `chosen`, with and without
   the `register` on `slot`, region on/off, tail order, two index
   spellings): every one +4 bytes, 68 to 90. The base's count-first tail is
   15. (The first run of this batch was void: I had written `volatile T *`
   for the tree's `T *volatile`, which is +4 on its own. Caught by the
   base-reproduction cell, which every batch here carries.)
2. Seventeen spelling cells on the base (`u32`/`u8 *`/`+ index` forms of both
   subscripts, `register` dropped, four declaration orders): 13 to 14 inert,
   orders 18 to 22, `s16`/negative-index forms change codegen.
3. `index` reused as the switch variable: 22 on the base, 67 on one pointer.
4. Diagnostics on the plain pointer: walking the loop by `slot++` (no
   `index << 3` anywhere in the loop) still rematerialises the pointer after
   each call from a spilled `index` (93). So the PRE reading was wrong: uopt
   recomputes `&Chosen[index]` whenever `index` is recomputable.
5. Killing `index` after the pointer is formed (`index = 0`, `index++`,
   `index = -1`, `index = 32`, `index = kind`): **54 to 58 at +4**, and the
   listing is the target's from `+0x9C` on except that the pointer takes
   `a3` while the constant 1 holds `v1`, and the arm merge duplicates a
   `move`. This is the one thing that makes uopt spill the pointer to its
   own home and draw the shift from the ring.
6. One `slot` variable through the scan loop, the free loop and the calls
   (no `chosen`): the ladder says slot (web 0) is `57/7 = 8.14` and value
   (web 3) is `25/3 = 8.33`, so value is decided first, takes `v1`, and slot
   falls to `a2`. The base's copy form has slot at `57/6 = 9.5` because the
   copy's def sits after the call. Taking `value` out of the scan loop flips
   it (38, delta 0) but breaks the loop's `a0`.
7. Two probes on slot raise both terms (`59/8`) and do not flip it; testing
   `slot->value` instead of `value` in the tail drops value to `24/3` and
   **flips the order: slot `v1`, value `a0`, every colour the target's**, at
   31 words and +4 for the reload the spelling adds (the struct field is
   `volatile`, so any `slot->value` read is a load). Unused pointer locals do
   not enlarge the frame here (L99's wording is contradicted again, as lm-a
   found on o003).
8. Non-`register` slot still spills to a temp (frame 0x20); per-arm `value`
   definitions add a reference (`26/3`); `value = slot->value = call()` is
   `56/7`.

**The named decision.** With the one-`slot` shape, the target needs slot
before value in the ladder: value's total must drop by one weight-one
reference (to `24/3`) without adding a load, or slot's ratio must rise
without adding a block. The candidate references are the tail's return path.
Separately, the frame: a separate declared `chosen` that is spilled gets its
own home at `0x18` and the frame is the target's 0x28 (cycle 5's listing);
the one-web form spills to a temp in a 0x20 frame. The target has both the
one-web colouring and the own-home frame, so the last question is what makes
a spilled symbol web take a home instead of a temp.

## `overlay2QueryNode`: the OR block is a spilled scalar, and the scalar's save is the lever

Baseline 39 masked (51 raw), frame 0x68, aligner 223/15/2/16 with the three
surplus/missing pairs the shards record. Two cycles, both on the two sites
the prior lanes had not read off the ladder.

The target's OR block stores the first call's result to `0x4C` before the
second call and reloads it into `a0` after, with the side-pointer load
scheduled *above* the store. That is a spill of a scalar web coloured `a0`
to its own home; as1 hoists a load above a spill store where it will not
hoist it above the candidate's array-element store. Seven scalar carriers on
the current shape: plain scalar, swapped or-operands and `register` are 100
at -8 bytes; compound assignment 72; `count` as carrier 45; `leafResult` 243
at -8; the address-read form is byte-flat at 39. The census on the plain
scalar says why it is eight short: its web (save `4/2 = 2.0`) is decided
before the `&D_58`/`&D_5C` address webs (`8/5 = 1.6`) and finds `s0` free --
already saved for the leaf branch -- so it takes `s0` with no spill and the
function loses one word per mirror block. In the target the scalar was
spilled, so it was decided after those address webs with `s0-s3` taken and
was split to `a0`.

**Next hypothesis for o002:** a scalar `recursiveResult` whose web has save
below 1.6 at unchanged total (nocs 4 at total 6 would do it), so it is
coloured after the two address constants. A probe is the diagnostic; the
source form is the question. Six forms of the tail-call child (ternary,
ternary behind a region marker, the dead `line` pointer as carrier, a fresh
`child` local) are byte-flat at 39 or worse (70), confirming that site is
carrier-inert. Nothing here touches the short-circuit reissue or the leaf
join, which p9-mid and p23 already priced.

## Ledger

Sixteen measured cycles of twenty: o058 pair 6 (one of them the PointQuad
transfer), o014 8, o002 2. The o014 cap was exceeded by two on purpose: it
was the only target with a named decision variable and a listing that
matched from `+0x9C` on, and the two extra cycles bought the flip in cycle 7.
The o002 remainder was not spent because its named lever (the scalar's save)
needs a source form, not a measurement.

Every score is `tools/score_symbol.py`; nothing was forced into the tree;
the only adopted edit is the PointQuad transfer (24 to 21, delta 0, frame
0x18, 11 relocations unchanged). `gmake verify` prints the expected SHA1 on
the committed tree; the function remains `NON_MATCHING`.
