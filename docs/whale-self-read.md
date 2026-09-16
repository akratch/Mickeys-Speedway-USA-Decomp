# The whale matches on a self-reading reset: lane wv-y

**Retained: 0 masked / 0 raw differing words, 14,456 bytes, delta zero,
frame 0x138, 1,253 of 1,253 relocations, no first mismatch.** Down from 6.
Aligned buckets move from 3611 exact / 0 naming / 0 immediate / 4 structural
(tax 2) to **3614 / 0 / 0 / 0**. Unforced. The `NON_MATCHING` guard and the
assembly fallback are gone; `gmake verify` prints the expected SHA1 from the
C, `gmake promotion-proof` passes (`3614 words, frame=0x138,
relocations=1253/1253, identity=static-plus-runtime-table-and-linked-rom`),
and `gmake check-overlay-syms` is up to date. One measured cycle of the
twenty; the rest of the lane was spent on a two-loop mini TU that reproduces
wv-x's rules, and on two promotion faults that the score cannot see.

## The residue, and the fact it needed

wv-x left one instruction's schedule: the title-loop load, emitted first in
its block where the target has it in the call's delay slot, because a load
through a pointer variable carries no `.noalias` fact. An indexed named
array carries the fact, but its strength-reduced cursor init folds to the
bare address only from a reset of `i` in the guard's own block (183/184),
and wv-x had measured the three-way conflict: that reset survives only as a
*redundant* store, redundancy needs the delay-slot reset to still be live,
and every form that kept the delay-slot reset alive lost one of the other
two (`c12a` +8, `c13a` 50, `c9a` 46).

Read against the target's own stream by offset (cycle 0), the title loop
keeps the counter in `s7` and tests it against the reloaded count, block
184 is two words of cursor init and nothing else, and the row reset sits in
the loop-exit block 189 -- so the target's title phi reads the delay-slot
def directly, the guard-block reset was deleted, and the row reset was sunk
from 190 by the redundancy pass exactly as base6's row side already does.
What was missing was one thing: **a read of the delay-slot reset that
dead-store elimination counts and nothing later pays for.**

## The pass model, measured on a mini TU

The mini (`mini/m*.c`, banked) is the case-12 shape in forty lines: a reset
in the first call's argument, two more calls, a guarded `do`-loop over
`arr[i]`, two saves, a row reset, a second guarded loop. It reproduces
wv-x's rules (`m0`: `base + i*4`; `m1`: the guard reset kills the
delay-slot def; `m16e`: c12a's unfolded row init) and each of the following
was read from its `cc -S` listing:

1. **Dead-store elimination runs first and keeps only a def that some path
   reads before overwriting.** A read folded away before it runs does not
   count. `x += i & 0` at 183 (`m12b`), in a call argument (`m14d`), in the
   `else` arm (`m12g`) and after the row reset (`m17a-d`) are all folded --
   uopt knows `i == 0` across the three calls (`if (i != 0) h(3)` at 183
   loses the call, `m11a`) -- and the def dies.
2. **A read cannot be folded when a def of the same variable follows it in
   its block.** `x += i & 0; i = 0;` at 184 keeps the read and the
   delay-slot def (`m11f`, `m12e`); the same read one block earlier does
   not (`m12b`). A def and a read in one statement fold locally (`m14a-f`).
3. **The redundant-store pass deletes a store only when it is its block's
   first reference to the variable.** A plain `i = 0` behind any other
   statement is deleted (`m22a`, `m22b`, `m22d`); behind a read of `i` it
   is kept (`m22c`, `m8i`, `m11f`). The same rule blocks the sink of the
   row reset from 190 to 189 whenever a read of `i` precedes it in 190
   (`m2`, `m12a`, `m13e`): the sink is a delete-and-insert, and the delete
   is refused.
4. **The strength-reduction fold uses the block's own def, not propagated
   knowledge.** With the def in 183/184 the init is the bare address; from
   180 through the calls it is `base + i*4` even though the same pass has
   already proved `i == 0` there (`m11b`).
5. **A conditional reset of a known value is deleted before strength
   reduction** by the if-body no-op rule, whatever the condition
   (`m20a-e`), so it folds nothing.
6. **A self-reading def is not a dead-store candidate.** `i &= 0` at 184
   keeps the delay-slot def alive, folds the init and sinks the row reset,
   but is itself emitted (`m8i`, `m11d`): as a def it is kept by rule 3
   (its own read precedes it), and it only becomes a plain zero store after
   that pass has run.

Rules 1-3 leave no block between 181 and 190 where a lone read survives
without either an emitted def after it or a blocked sink. Rule 6 is the way
out once the def is moved off the guard block: `i &= 0` **in block 181**,
after the first call, reads the delay-slot def (rule 1 satisfied), cannot be
folded (its own def follows it, rule 2), and is then deleted -- with the
guard-block `i = 0` now the first reference of 184 and the delay-slot value
available, the redundancy pass removes both, and the row reset at 190 sinks
as before. `m24f` is byte-identical to `m0` except for the folded init and
the ring renumbering that follows it. Placed as a statement *before* the
first call, or on its own before call 1 (`m24a-c`), the read has an
undefined value, folds, and the def dies; inside the first call's argument
(`m23a-c`, `m24e`) it survives but the argument no longer folds.

## Cycle 1: the whale

Six cells, one batch, from base6 with the title loop as `D_o058_5C98[i]`,
`i = 0` in the guard, and the row side unchanged:

    y1a   i &= 0 on the fontColour line                     0 masked / 177 raw / delta 0
    y1b   i &= 0 on its own line after call 1               0 / 177 / 0   (adopted)
    y1c   i &= 0 on call 2's line                            0 / 177 / 0
    y1d   i &= 0 on call 3's line                            0 / 177 / 0
    y1e   spelled i = i & 0                                  0 / 177 / 0
    y1f   control: no self-read                              46 / 218 / 0  (wv-x's c9a)

The tree re-scored **0 masked, 3614 / 0 / 0 / 0, tax 0** with
`tools/score_symbol.py` and `tools/align_symbol.py` before the guard was
removed. Zero instructions, no web in 183/184/190, W and the 5E9C piece as
wv-x left them.

## Two promotion faults the score cannot see

Both broke `gmake verify` with the function's own bytes already exact, and
neither is named by the error that reports it.

- **The mode switch's jump table.** The C object carries its own thirteen-
  entry table in `.rodata`; the shipped module already holds those bytes at
  data_rodata `+0x3F4` (rodata-relative `+0x124`, which the text's `%hi/%lo`
  pair encodes). Linked naively the module grows by the pool and every
  overlay after 58 shifts (`overlay-syms` reports the `.rodata` section
  symbol as `UNRESOLVED ... 0x0 vs 0x124`). The metadata-only route is
  overlay 14's: `--add-symbol gOverlay58ModeJumpTableReloc=0x124`, rebind
  the two text sites through
  `config/normalizations/func_overlay_058_F000138C_18B0574.rebind.spec`,
  externalize `.rodata` against its digest, drop `.rel.rodata`. An
  *anchored* externalization (overlay 59's form) is classed `altered` by
  `postprocess_audit.py` and would forfeit the credit. The atlas gets a
  `FIXED_DATA_RODATA_OWNERSHIP` row `(0x3F4, 0x428, ..., ".rodata", ...,
  True)`; the table's words were checked against the raw slice before the
  digest was pinned.
- **Three resident data symbols named bare.** The TU declared
  `D_8007BEF8`, `D_8007C0B8` and `D_800D3140` without the `_o058Reloc`
  suffix it already used for a dozen others. As an assembly fallback that
  was invisible; as a linked C object the surface generator writes a value
  line for each (`= 0x0`), the linker script assignment overrides the
  resident definition, and **95 bytes move in the resident segment** while
  overlay 58 stays exact. `CLAUDE.md` documents this for calls; it holds
  for data. The three were renamed; the object is unchanged by a rename.

## Cycle ledger

     0   (uncounted) harness at 6 with `.text` identity; the target's case-12 stream by offset; ~60 mini TUs for the pass model
     1   `i &= 0` in blocks 181/182/183, two line placements, `i = i & 0`; control      6 cells: **0 at delta 0** (five), 46

Promotion: atlas exact range and rodata row, `overlay-atlas-write`, digest,
`extract`, `overlay-syms`, build, `overlay-syms`, build, `verify`,
`check-overlay-syms`, `promotion-proof`, `scoreboard`, `nm_ranking.py
--refresh-stale` (the retired row), `check-docs`.

## Reproduction

Cells compiled from copies of the TU in private scratch with the tree's
`tools/ido/cc` and the configured flags (`cellcc.py`, wv-x's harness
re-pointed), scored with `nm_ranking.process_item` on the stock object; the
adopted body re-scored with the tools of record on the tree, then linked
and verified. Minis compiled with the same flags and `-S`
(`mini/run.sh`). Banked under `whale-resources/cells-wv-y/` in Git's common
directory: every cell's source and `result-cell-*.json`, the six stock
objects and the linked matched object, the externalized pool payload, the
matched tree source, and every mini with its listing.
