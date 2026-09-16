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
