<!-- plateau-handoff:overlay1ActivateObject:start -->
### `overlay1ActivateObject` plateau handoff

- source: `src/overlays/o001/overlay_001.c`
- score: 0/88 words, promoted
- frame: 0x18
- relocations: 13
- first mismatch: none
- summary: Matched. Natural shape: store the record to D_1D68 and read it back, one-argument chain helpers, D_1DA0 read at each use, subscript of the start pointer.

#### p8-o001: the records priced both edits, and a lattice would have missed the second

The function issues three calls, so by the call test it emits p1 records only.
The instrumented toolchain's object is byte-identical to the tree's, checked
with `cmp` on both the whole object and the extracted `.text`.

The configured base aligned as 41 byte-exact, 47 register-naming, 0
immediate-only and 0 really-different at size delta 0 -- a pure allocation
residual. `register_census.py` read it as 86 percent of substitutions following
one global mapping, four windows, and a coherent `t8 -> t9 -> t0 -> v0 -> t8`
cycle, with the tail from +0xD0 a clean plus-one shift through the whole scratch
ring.

The first edit came from the decision records rather than the census. One web
carried `save` 4.0 at `nocs` 2 and took the first colour offered, `v1`; the web
holding the address of `D_1DA0` then read `forbidden0=0x78000000`, so `v1` was
genuine interference and `p1:w58=c2` was declined at every combination tried.
The high web was the single `state` local, which in the original source is
assigned once before the calls and again after them, making one live range that
spans all three -- which is also why it is denied `v0` (L101). Splitting it into
a pre-call `state` and a post-call `current` retired the interference: `state`
then took `v0`, the address of `D_1DA0` took `v1`, and the score went 47 to 37
with the whole head prefix becoming exact.

The second edit is L127 and then its own inverse. With the split in place, a u8
mask on the first of the four tail stores -- `(current->selector & 0xFF)` --
took 37 to 19 by consuming one scratch temp that the peephole then deleted.
Independently, dropping the `index` local so the record index stays an
expression temp took 37 to 27, a regression. Together, with the mask removed
again, they take it to 3. That is L100's warning about half-directions in its
strongest form measured here: the winning pair contains one edit that regresses
alone and one that helps alone but must then be withdrawn.

Measured and eliminated from the 3-word base, do not repeat: all sixteen subsets
of the u8 mask over the four tail stores (best is the empty set at 3; masking
store 4 alone is 8, store 3 alone is 14); 170 placements of the ADR 0017 inert
read family over seventeen slots and ten expressions on the pre-split base, every
one byte-flat, which is what classified the residual as not a population
question; both commutations of the record address sum; four spellings of the
record expression; three spellings of the store; six declaration orders of the
three locals; blank-line and statement-folding shifts around the store and the
record definition; a chained `D_1D68 = record = ...`; a shadow local; a slot
pointer local; reading the chain-40 argument and the tail through
`*(Overlay1Sample **)&D_1D68` so the address is referenced twice; a duplicate
non-volatile store through that address; a discarded `(void)&D_1D68`; and all
twenty-two positions of the `D_1D68` and `D_1D68Read` extern declarations in
their block.

#### p8-o001: what blocks the last three words

The three are `+0x6C`, `+0x70` and `+0x90`: the address of `D_1D68` is
materialised into a register and stored through at offset zero, and the target
puts that address in `v0` where the candidate uses a scratch temp. Everything
else in the function is byte-exact.

A p1 force sweep over every coloured web and every colour from c1 to c14, plus
the split path, never reaches below 3. By L140 that is an existence proof: the
colour belongs to a web that does not exist in this candidate, so the lever has
to create one. The colour table here decodes from the records as c1 `v0`, c2
`v1`, c3 `a0`, c4 `a1`, c5 `a2`, c6 `a3`, c7 `t0`.

Forcing the `state` web off `v0` is worse in every direction (best 11, most
above 37), so `state` keeping `v0` is part of the target's own shape and the
address web must share the colour rather than take it -- the two ranges do not
overlap, so that is consistent.

The addresses that do become coloured webs in this function are `D_1D9C`,
`D_1DA0`, `D_1D60` and `D_1D6C`; `D_1D58`, `D_1D64` and `D_1D68` do not. A plain
reference count does not separate them, since `D_1D64` has the same two
references as `D_1D60`. Decision variable: what promotes a load-address node to
a p1 live range in this procedure. The record that blocks it is the absence of
any `p1cand` line naming the `D_1D68` address at all -- it is not outbid, it is
not present. Axes this lane covered: colour forcing over every existing web, the
ring-draw family, source-level reference counting on that address, statement
order, declaration order, extern order, and physical line folding.

#### 2026-10-01, lane b-misc: matched by discarding the inherited shape

The closure above named the right fact (the `D_1D68` address is not a web in
the candidate) and looked for it on the allocator axis. It is a source-shape
fact. The inherited body carried a `record` local, a volatile `D_1D68` plus a
second alias extern for reading it, a volatile cast on `D_1DA0`, a post-call
`current` local and a two-argument first helper call. Writing the block as its
author plausibly did -- assign `D_1D68`, pass `D_1D68` to the helper, read
`D_1DA0` at every use, no locals but `state` -- makes the store and the
forwarded read two references to one global, which is what gives its address a
coloured web. That shape scored 19 with the whole residual one ring draw late
from the record sum; spelling the record address as a subscript of the start
pointer, or with the scaled index on the left of the sum, puts the index
product before the base load and scored 0. Five spellings of the sum were all
exact. The first helper takes one argument; the second-argument register only
held the leftover `D_1D9C` address.

`gmake verify`, `check-overlay-syms` and `promotion-proof` pass.
<!-- plateau-handoff:overlay1ActivateObject:end -->
