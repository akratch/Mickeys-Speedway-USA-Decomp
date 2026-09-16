# Last-mile lane nx-c (2026-09-16): the global that was a pool literal

Five named-hypothesis targets, 3,204 bytes, all at delta zero on arrival.
One matched and promoted (`overlay15DrawScreenStars`, 420 bytes); four left
with a measured negative each and the decision variable sharpened. 17 of
the 20 budgeted cycles were spent (a cycle is one batched hypothesis
measured against `tools/score_symbol.py`'s comparator; a private direct-`cc`
harness reproduced the score of record on every TU before use). Evidence
lives outside the tree in the session scratchpad; nothing ROM-derived is
tracked.

Where this lane and its dispatch disagree with the brief: the dispatch asked
for a full `nm_ranking.py` regeneration before hand-back; the brief says to
refresh only when `check-docs` fails without it. It did fail (the promoted
symbol had left the queue), so the ranking was regenerated in the measuring
form and `--write-doc` run after it. Two claims in the o15 shard were wrong
and are corrected there.

The finding that paid: **read the shipped words for what the original
declared, not just what it computes.** `gOverlay15StarFadeScale` was never a
global. Its two words are a LOCAL `%hi/%lo` pair with 0x0000 immediates and
the loader supplying the base, the shape of overlay 86's literal pool, and
the value at data_rodata +0x40 is 0x3f5f8fc8 = 255/292 = 255 over the visible
depth range 300 - 8. A float literal folds to that exact word, and as a pool
constant it does what no global read could: uopt hoists it into the loop
preheader below the synthesised guard and, single-use and numbered after
8.0f and 300.0f, colours it last, on the ROM's register.

## `overlay15DrawScreenStars` -- matched and promoted

9 to 2 to 0 at delta zero, three cycles. Named step (a pointer view with a
discarded late second dereference) refuted: cfe drops a value-less
dereference before uopt sees it, so every such spelling is byte-identical to
the plain pointer view (48). Corrections: the loop-top pointer form (16) does
not hoist -- its back-edge targets the block's first instruction and the
fade load runs every iteration, which is why its web weighs 20 over 3
components and ties 8.0f; and the global was the pool literal. Adopted:
`fadeScale = 255.0f / 292.0f;` before the loop (9 to 2) and the two
setup-command stores on one physical line, an as1 line tie the earlier
85-cell order sweep had measured on the old block shape (2 to 0; L146).
Promotion is overlay 86's metadata-only pool form: the two `.rodata`
references rebound to `gOverlay15FadePoolReloc`, the 16-byte pool
externalized by digest, `.rel.rodata` dropped. `gmake verify` prints the
expected SHA1 from the C; `promotion-proof` passes with 10 of 10 relocation
identities; `check-overlay-syms` up to date; `tools/gates.sh --promotion`
green. Commit `904759fa`.

## `func_8003F154` -- 11, unchanged, mechanism priced

Five cycles, 49 cells. Named step (a second surviving `0.0f` use in the
block after the sqrt call, or sqrtf not a call): sharpened to arithmetic
and not closed. The records say the `0.0f` web is split by seeding the FIRST
reference block in `seedcand` order, a piece's save is references minus one,
and the compare's block (42) has one reference so its piece is never
coloured. Two positive controls prove the diagnosis: a `0.0f` store added
in block 42 puts the compare's zero on f2 as the ROM; the same store in the
then-arm or the multiply block lands on f12 because the piece then shares a
block with `scale`. So the ROM's second reference is consumed inside block
42 and emits nothing. Every arithmetic zero folds at cfe; a dead
`speed = 0.0f` is eliminated; a pre-set `scale = 0.0f` either collapses the
`if` or, through a symbol, costs an `add.s` that uopt does not fold. A
3-word basin exists (then-arm `scale = 0.0f`, direct Z accumulate) and is
recorded as a diagnostic: the ROM's then-arm is `scale = speed`.

## `func_8005A7A0` -- 10, unchanged, colour half closed

Five cycles, 34 cells. Named step (compare split-record spill placement
between the forced object and the target): done, and it names the
difference. The forced split stores the piece at its definition and takes
v0 because the piece stops at the call block; the ROM's piece spans call 1
(hence s0) and stores at the remainder's entry, before call 2. The colour
half is one edit: with the doubling moved between the calls the value takes
s0 with no spill (the ROM's colour; the ROM merely reads it again after call
2 from memory). The frame's 24-byte home block is a binary switch, ON when
the spilled web is an expression temp (type 4, cost 2.0) and OFF when a
symbol web is spilled (type 3, cost 3.0) or nothing is; the ROM is OFF with
one spill temp. Next: what makes split() expand a call block or place the
store at the boundary, and whether that is the same switch.

## `overlay15DrawScreenStars`'s sibling lesson applied to `func_overlay_071_F0000870_18CA390` -- 9, unchanged

One cycle, eight cells. Named step (a value block 11 computes and discards)
found nothing: the block is instruction-identical but for the two `lhu`
registers, and every second-pair carrier is 82 at -4. The ROM's head pointer
is `command`'s web (v0), so the a0 interferer at block 5 and the a0/a1 pair
at blocks 11-13 are all invisible webs the source does not yet name.

## `func_8005ABA8` -- 2, read only

Re-measured at 2, delta zero, first +0x3C. No cycle spent; the as1
zero-node-block closure stands.

## Ledger

Seventeen measured cycles of 20: o015 3, f154 5, a7a0 5, o071 1, ABA8 0,
plus instrument reads on every target (records, forced pieces, ugen trace)
that were not counted as cycles. Matched and promoted: o015 (420 bytes).
Unmoved: f154 11, a7a0 10, o071 9, ABA8 2, each with a sharper decision
variable than it arrived with. Two shard claims corrected on o015; three
tool notes: `residual_map`'s GPR names remain misattribution on f154, the
diff harness must not count relocation lines as instructions (it shifted
every offset by the reloc count until fixed), and `register` on a local is
rejected by this compiler when it follows other declarators on the same line.
