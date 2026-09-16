# Last-mile lane lm-a (2026-09-16): a call's unused result owns its block

Lane `lm-a` was dispatched six near-miss functions, each a compiled
`NON_MATCHING` body at size delta zero, with a budget of 24 measured cycles.
This file records what closed each target and, for each that did not close,
the residual's shape and the next hypothesis. Evidence (candidate sources,
objects, allocator traces, per-window maps) is kept outside the tree under
Git's common dir, `lane-evidence/lm-a/`, and nothing ROM-derived is tracked.

The one mechanism this lane adds to the law index is stated first because it
transferred, or was priced, on more than one target.

## The mechanism: L101 is block-granular, and an unused call result is a web

`func_overlay_086_F0000474_18D22AC` had sat at 7 words for seven lanes: two
generated pointer loads, the case-0 `+0x48` access and the `+0x3E0` access,
both on `a0` where the ROM reads `v0`. Every closure said the same thing --
"the web spans a call result, so `v0` is absent from its cost list, and the
only split is a twelfth declaration" -- and every lattice measured colours,
carriers and spellings around that statement.

The records say something narrower. With the `+0x3E0` access written as the
expression itself (no `temp_a0`), it is web 213: `save` 20, `nocs` 2, block
set `{68, 69}`, `forbidden0` carrying bit 30 (`v0`) and `v0` absent from its
`p1cost` list. No `v0`-coloured web shares a block with it -- the `v0` webs
decided before it are at blocks 70, 75-87. The only `v0` value in block 68
is the **unused result of the `ext_o0_5a914` call** that precedes the load.

So the rule, measured on this function in four cycles:

- **A call's result is a `v0` web that occupies the call's whole basic
  block, whether or not the source uses it.** Any web defined in that block
  after the call is denied `v0` by interference, not by the L101 span test
  -- the same symptom, a different cause, and one that source can reach.
- **Two source forms remove the interferer, and they are byte-identical.**
  Declaring the call `void` (its result is never used, so the reconstruction
  is at least as honest as `M2C_UNK`) deletes the result web. An L97 region
  boundary -- `if (1) { }` or `do { } while (0)` -- placed between the call
  and the load starts a new block, so the load is no longer in the call's
  block. The `void` form is adopted; the boundary is the alternative when a
  call's result *is* used elsewhere.
- **It is per call, not per block.** Voiding only `ext_o0_5a914` (the last
  call before each freed load) matches; voiding the four calls before the
  loads matches; voiding every unused-result call in the TU moves the head
  to 13 words at `+0xA8`, because other result webs are load-bearing there.
  Price the call whose block holds the residual, not the family.
- **The carrier had been hiding it.** Routing the load through the declared
  `temp_a0` (a head web on `a0` by argument affinity) gave the load `a0` and
  made the residual read as a carrier question. L145's "write the access as
  the expression itself" is the first half; without the second half the
  expression web takes `v1` (the `+0x3E0` case) or merges with an identical
  expression elsewhere into one call-spanning IR name (the case-0 access and
  line 434, L131) and evicts the state-byte family from the head.

## Target 1: `func_overlay_086_F0000474_18D22AC` -- matched and promoted

2,648 bytes, 662 of 662 words, frame `0xA8`, 38 of 38 relocation identities.
`gmake verify` prints the expected SHA1 from the C with the `GLOBAL_ASM`
pragma gone; `gmake promotion-proof` passes (`identity=static`);
`gmake check-overlay-syms` is up to date.

Aligner before: 655 byte-exact / 7 naming / 0 immediate / 0 structural,
displacement tax 0, positional masked 7, first `+0x330`. After: 662 / 0 / 0 /
0, masked 0.

Cycles (each a batch measured with `tools/score_symbol.py`, the score of
record):

1. Nine carrier/spelling cells around the two sites. Inlining `+0x3E0`
   alone: 7 (the generated web takes `v1`). Any form that takes the case-0
   site out of `temp_a0`: 42, first `+0x70`, whatever line 434's spelling
   -- the merged IR name takes `v1` and the head follows. Int-typed loads
   with integer `+6`: +4 bytes. Negative, but it located the cascade.
2. Records read for base and the inlined form (web 213 above), then nine
   cells: a region boundary between the call and the inlined `+0x3E0` load
   closes the `+0x600` window, **7 to 3**, both boundary spellings; the
   same boundary on the `temp_a0` form is inert (7); int-typed case-0 loads
   with pointer arithmetic after the cast still cascade (38, 36).
3. Five cells on the case-0 site over the 3-word base: boundary after the
   block's last call plus the inlined access, **3 to 0** in four spellings
   (pointer-typed, u32, s32, `do`-form); boundary with `temp_a0` kept: 3.
4. Control, scored object-against-object because the TU had left the queue:
   boundaries removed and `ext_o0_5a914` declared `void` is byte-identical
   to the matched object (adopted); boundaries removed alone is 44; all
   unused-result calls voided is 13.

Promotion needed two things the score does not show, both predicted by the
dispatch:

- The compiler's 32-byte pool -- two float constants at `+0x0`/`+0x4` and
  the five-entry state-switch table at `+0x8` -- duplicates the shipped pool
  at data_rodata `+0x80`, rodata-relative `+0x0` (the shipped `%hi/%lo`
  immediates are `0x0000` / `0x0000, 0x0004, 0x0008`; the loader's six
  LOCAL HILO roles supply the base). Linked as-is it appended 32 bytes and
  shifted every module after 86 (87,422 differing bytes). Fix: overlay
  58's metadata-only form -- `--add-symbol gOverlay86StatePoolReloc=0x0`,
  rebind the six references, externalize `.rodata` by digest, drop
  `.rel.rodata`, and the atlas row marked externalized.
- The five same-module `jal`s (`overlay86ScaledVectorPosition` x2,
  `overlay86SelectPosition`, `overlay86BuildTransform` x2) are `SYMBOL`
  records with a zero addend. Naming the in-module definitions linked them
  as `JUMP`s (5 words). Fix: `*Reloc` placeholder declarations, as
  CLAUDE.md's overlay 2 note says.

The three 2026-08-28 ownership-trial spec files under
`config/normalizations/` were consumed by no rule; the rebind spec is
replaced with the pool rebind and the two filters are deleted.

Source facts carried over from earlier lanes and still load-bearing: the
one-argument `ext_o0_2d98` calls (arity frees `a1` for the command pointer),
the split advance, the payload stores in memory order, and the cursor
definition folded onto the `0x2C` store line.

## Target 2: `overlay3SelectScoredObject` -- matched and promoted

472 bytes, 118 of 118 words, frame `0x80`, 5 of 5 relocation identities.
`gmake verify` prints the expected SHA1 from the C; `gmake promotion-proof`
passes (`identity=static`). Aligner before: 110 / 5 / 1 / 1 with one
candidate-only (`+0x88`) and one target-only (`+0x48`) word, positional
masked 21; after: 118 / 0 / 0 / 0.

Two cycles:

1. Six cells. L160 -- the loop written `object = objects[index]` with no
   cursor arithmetic -- is **21 to 0** when the `cursor` declaration is
   kept unused, and 7 (first `+0x40`, the frame) when it is deleted. The
   decision the shard named ("what makes uopt copy the helper's return
   register into `a1` while the cached path keeps the raw result") is the
   declared cursor: it made the helper result one web, `save` 20 in block
   14 only, coloured `v0` unopposed; without it uopt keeps the raw result
   for the cached path, copies at the split point before the guard, and
   both `count` reloads take `v0`. Region boundaries after the call or
   after `result = 0` are inert at 21.
2. Five cells on the frame: an unused `f32 dy` beside `dx`/`dz` (either
   order), a used `f32 distance`, and dropping `u16 timer` all stay at 7;
   an unused `Overlay3Object **base` is 0. So the ROM's body declared one
   more *pointer* than it used, and an unused `f32` is eliminated on this
   TU -- L99's wording ("an unused f32 or pointer is not") is half wrong
   here. `cursor` is kept unused and listed in `docs/cleanup-queue.md`.

## Target 3: `func_overlay_014_F0001540_1870E18` -- matched and promoted

752 bytes, 188 of 188 words, frame `0x80`, 26 of 26 relocation identities.
`gmake verify` prints the expected SHA1 from the C; `gmake promotion-proof`
passes (`identity=static-plus-runtime-table-and-linked-rom`). Aligner
before: 176 / 12 / 0 / 0, masked 12; after: 188 / 0 / 0 / 0.

One cycle, eight cells, and the mechanism above applied by reading the
records first: the entries-base web (30: `save` 10.67, `nocs` 3, blocks 4-7
and 26-27) had `v0` in its `forbidden` mask with no `v0`-coloured web in any
of its blocks -- the generated stride web (50, `v0`) is block 11 only. The
`v0` values in its head blocks are the unused results of the two
`overlay14Dispatch` calls between `cellWidth = overlay14Dispatch(2)` and the
while test. Declaring those two void is **12 to 0**; declaring every unread
call void is also 0; the draw call alone or the colour calls alone leave 12.
`overlay14Dispatch` is an m2c catch-all for several resident functions
behind one runtime-table call, so the unread ones now go through the void
`overlay14DrawPrimitive` surface the sibling TU already uses for the same
argument shapes; the two value-returning calls keep `overlay14Dispatch`.
Promotion needed nothing beyond the sequence: no pool, no bare resident
names, 26/26 relocation identities.

## Target 4: `func_8003F154` -- 13 to 11, not matched

1,188 bytes, 297 words, frame `0x58`, 16 relocations; all integer lanes and
the schedule are exact and the residual is FP allocation. Six cycles.

1. Direct `offset[2] = -config->value3C` and comparison spellings: the
   direct store is the shard's 31 (read off the objects, all FP ring rows
   -- `residual_map`'s GPR names are misattribution on this function); the
   comparison spellings are inert at 13.
2. `scale` hosting the zero from the head through the comparison: 78-168
   at +4 to +12 bytes. A symbol carrying the zero across six calls is a
   real web with spills; the ROM's zero is not a named local.
3. Natural heads crossed with comparison-zero spellings, 27 cells. **The
   head closes on the literal's type (L151):** `offset[0] = 0; offset[1] =
   0; offset[2] = -config->value3C;` with the comparison kept at `0.0f` is
   **13 to 11, first mismatch `+0x21C` to `+0x3E4`** -- the head no longer
   spends a ring draw on a zero that lands in the pool colour, and the
   value3C load is a ring temp negated straight into `offset[2]`. Spelling
   the comparison `0` or `(f32)0` as well merges it into the head's
   constant web across the calls (168); `0.0` is 53.
4. The same literal-type lever at the other zero sites (the
   disableTransform arm, the velocity head) merges with the head's web
   (168, 18); the direct Z accumulate is 12.
5. The inlined x-first sum is exact through `+0x3F0` and moves the first
   mismatch to the comparison zero, but the tail then reads one ring step
   off (17); `scale = 0.0f` after the sqrt call is propagated into the
   comparison (11, inert).
6. `scale = 0.0f` *before* the sqrt call, which would have been a
   rematerialisation across the call, is propagated the same way (17,
   identical to the inlined sum alone). Refuted.

Adopted: the cycle-3 head. What remains is one decision: the ROM's
comparison zero is a globalcolor web with a single use in the block after
the sqrt call (`mtc1 zero, f2`, the colour `scale` takes on the next
instruction), and ours is a ugen ring draw. Every symbol that could carry
it is constant-propagated away before colouring and every other spelling
of the literal joins the wrong constant. The next lane should look for a
source in which that block holds a second surviving use of the same
`0.0f`, or in which `sqrtf` is not a call.

## Target 5: `overlay34CreateRecord` -- 6, not matched

500 bytes, 125 words, frame `0x30`. Two cycles, seventeen cells, none below
6. The four naming rows are a ranking the records already state -- the
resource copy (save 3, `nocs` 1) decided before height (save 2, `nocs` 2)
-- and height's second block is what no lane has named: width has all its
references in block 10, the constant `1` reaches block 13 through
`active = 1`, so the twenty-store sequence spans several uopt blocks and
the boundary is invisible in the source. Routing the resource loads
through the loop aliases (119-124, +4 to +24 bytes), splitting or
multiplying the intermediate (16, 12), `s16`/`u16` dimensions (50, 102),
the mode multiply moved last (39), height first (6, 92). Next: dump the
instrumented uopt's block table for the then-block and place the boundary
before any further order sweep.

## Target 6: `func_overlay_071_F0000870_18CA390` -- 9, not matched

728 bytes, 182 words, frame `0x30`. Two cycles, eleven cells, none below
9. The mechanism above does not reach it: the second flag pair's web
already has `v0` and `v1` forbidden by real webs, and the ROM's `a2`
needs `a0` and `a1` held by values the emitted code never shows. Naming
the dereferenced resource index in the call's block is absorbed (9), the
pointer is a real web (65), hoisting either above the `flags & 6` test
makes two-block webs with the wrong codegen (17, 71), keeping the second
call's result or declaring both calls `s32` changes nothing (9, `v0` was
already forbidden), and a local copy of `commands` is absorbed (9). Next:
the invisible interferers are not carriers of anything this body keeps;
look for a value the ROM's block 11 computes and discards.

## Ledger

Seventeen measured cycles of 24: o086 4, o003 2, o014 1, f154 6, o034 2,
o071 2. Matched and promoted: o086 (2,648 bytes), o003 (472), o014 (752)
-- 3,872 of the lane's 6,288. Improved: f154 13 to 11. Unmoved: o034 6,
o071 9. One law-grade finding (the block-granular call-result web), one
correction to L99 (an unused `f32` was eliminated on o003), and one
transfer of L151 to an FP head.

Every claimed match was verified with `tools/score_symbol.py` at 0 masked
and delta 0 before promotion, and with `gmake verify` printing the
expected SHA1 from the C after it; `gmake promotion-proof` passes for all
three. Nothing was forced.
