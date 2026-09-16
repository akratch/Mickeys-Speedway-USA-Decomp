# W closes on a pointer walk; the residue is one load's schedule: lane wv-x

**Retained: 6 masked / 184 raw differing words, 14,456 bytes, delta zero,
frame 0x138, first mismatch +0x1314, 1,253 owned text relocations.**
Down from 9. Aligned buckets move from 3605 exact / 9 naming / 0 / 0 to
**3611 / 0 / 0 / 4**, displacement tax 2: one candidate-only word at
+0x1314 and one target-only word at +0x1320, the same instruction. Unforced.
The C remains guarded `NON_MATCHING`; the ROM is unaffected and
`gmake verify` proves the assembly fallback, not this C. Fifteen measured
cycles of the twenty; the lane stopped when the last fact had been reduced
to a three-way conflict between two folds and one liveness, each pair of
which is satisfied by a banked body.

## Cycle 0: the block graph, and where the difference had to be

The `-zdbug:2` flow graph of the 9 body (`uoptlist`, banked) gives case 12
as 180-182 (the three head calls, one per block), 183 (the head, ending in
the guard), 184 (the title preheader), 185 (the title call block), 186-188
(our `if (columnStep != 0);` probe blocks and the loop test), 189 (the loop
exit), 190 (the save block and the row guard), 191 (the row preheader),
192-201 (the row loop, 196-197 its calls), 202 (the restore), 203-227 (the
tail; 204 is our `if (columnCount != 0);` token block). Read from the
target's branch words by offset, the emitted structure is the same to the
block: `blez` at +0x1300 to +0x1368, the title loop's back edge from
+0x1344 to +0x1310, `blez` at +0x1370 to +0x1504, the row loop's from
+0x14F4 to +0x138C, the inner loop's from +0x14D4 to +0x148C, `bgez` at
+0x1530 out. Our only internal extras are the probe blocks behind the call
block 185, which W's growth never expands into, and the token block behind
203, which W rejects at margin -3 either way. So the difference wv-w
suspected was not in the block graph. It was in the allocator's neighbour
count, and cycle 1 found it.

## W: the pointer body, with the init from a lineage already in the window

wv-t's pointer cell (`cell18-ptr.c`: `title = D_o058_5C98` inside the
guard, `*title`, `title += 1`) had every property of the target's title
loop -- W accepting 185 and colouring `t0`, the reset in call 1's delay
slot, the constant cursor init -- and lost by one interferer: the
`&D_o058_5C98` lineage's remainder counted at 184 (that array is read in
cases 1/2/3/10, so its hull spans the loop), the 5E9C piece rejected 191 by
one and took 202 (+4). `D_o058_5C98` is `D_o058_5C80 + 24` bytes, and
`&D_o058_5C80` is read at 183 and 194 and in case 13, so its lineage
already spans the window. Initialising the pointer as
`(char **) (D_o058_5C80 + 12)` therefore adds no interferer, and the same
bytes come out after link (the relocation carries the addend). On that body
(`c1a`) the growth records read:

    W       184 +10, 190 +8, 185 +2 (accepted, left 15 -> 12), 191 -1, 202 +0 (accepted), colour t0
    5E9C    184 +13, 190 +11, 185 +3, 191 +0 (accepted, left 13 -> 12), 202 -1 (rejected)

which is the 48 body's growth on both pieces, unforced. W's seed numintf is
19 and its 202 test sits at exactly zero; the piece's 191 test sits at
exactly zero. The remaining rows on `c1a` were frame homes (a fresh
`char **title` took a slot and moved `slot` from 100(sp) to 96(sp)) and one
load's schedule. The pointer is now the unused `cursor` cell (retyped
`char **`), which restores the frame: **6 at delta zero** (`c2a`, adopted).
Physical-line splits and `cursor[0]` are byte-identical to it;
`*cursor++` is +4; three pointee types (array pointer, `const` pointee,
a one-member struct) are byte-identical.

## The residue: a load without its `.noalias` fact

The title-loop load is emitted in the same place on the 9 body and on the
pointer body (`cc -S`, banked): after `li $6,55` and before `li $24,4;
sw $24,16($sp)`. The strength-reduced body carries `.noalias $17,$sp` on
the load and as1 sinks it into the call's delay slot, past the argument
store, as the target has it; the pointer body carries no directive and as1
keeps the load above the store (L95). The workbench's alias profile
(`DKWB_UOPT_ALIAS_TRACE=1`, banked traces) names the descriptor: the
strength-reduced base is `kind=1 type=islda` and the query returns
`no-alias`; the pointer base is `kind=3 type=isvar` (the variable's own
frame cell) and the query returns `may-alias`. The fact rides on the base
being a load-address expression, which a pointer variable never is,
whatever it points at. Pointer-difference subscripts (`arr[p - arr]`, three
spellings) are computed rather than simplified, +16 to +28.

So the target's title loop indexes a named array, and its index is known
zero at the preheader without a web in blocks 183, 184 or 190 (W's 202 test
and the piece's 191 test are both at margin zero on the adopted body, and
every dead carrier at 183 or 184 is one web too many for both). That is a
statement about `i`, and the rest of the lane measured what `i` can do.

## What a known-zero `i` can and cannot reach

- **Calls kill the cursor-init fold, block boundaries do not.** With the
  reset in call 1's argument (block 180) the title init is `base + i*4`,
  +8, on every loop form (`do`/`while`, `while`, `for`, `if (i < n)`); with
  the reset in call 3's argument (182) or as a statement before call 3,
  the same; with the reset at the top of 183 the init is constant and the
  body is the 48 form, **39 at delta zero and seven aligned rows, all of
  them the position of `move s7,zero`** (`c4b`). A reset after call 3
  followed by a non-call probe boundary propagates the zero but leaves
  `sll t8,zero,2` unfolded. Four mini TUs (`mini/m_*.c`) reproduce the
  rule with no frame exposure at all; `register` and `-Olimit 5000` are
  inert (the latter reshapes the whole function, -20 bytes).
- **Availability survives the calls even though propagation does not.**
  The target's row-loop reset `move s7,zero` sits at +0x1364, in the
  loop-exit block 189, not in 190: uopt's redundant-store removal knows
  `i = 0` is still available on the skip path from block 180, through the
  three calls, and places the row reset on the loop path only. Our source
  writes it at 190 and gets the same placement, so this is a shared fact.
- **A redundant in-block reset folds the init and then disappears.** With
  the reset in the delay slot, `i = 0` inside the title guard (184) and
  the row reset moved inside the guard after the loop (189), the title
  init is constant, the 184 store is removed as redundant, the delay-slot
  `move s7,zero` survives (its use is the row loop's entry on the skip
  path), and the loop-exit reset lands at +0x1364: **the whole case-12
  head through +0x1370 is the target's** (`c12a`). Its +8 is the row
  cursor's init, `&D_o058_5EE0 + i*4`, because `i`'s reaching defs at 191
  are now 180 and 189 and neither is in-block.
- **The three requirements cannot be met at once.** Adding `i = 0` inside
  the row guard (191) folds the row init but overwrites the delay-slot
  def's only surviving use, so dead-store elimination removes it first:
  50 at delta zero (`c13a`), the same with the row guard written
  `if (i < n)` (the guard's use folds under propagation before liveness is
  judged), with both guards on `i`, and with the row loop nested inside
  the title guard with the saves duplicated on an else branch (+8,
  structure moves). `i = k = 0` in the delay-slot argument with `k` as the
  title index is +20. An uninitialised index is loaded from its home
  (+12). So: the delay-slot def lives only through a loop phi (every other
  use is folded to 0 by copy propagation), the title init needs a
  redundant in-block def that takes the title phi, and the row init needs
  one that takes the row phi; `c12a` holds the first two, `c13a` the last
  two, and no measured form holds all three.

## What this means for the target

The target has the delay-slot reset, both constant inits, the loop-exit
row reset and W at `t0`, on a title loop whose load base is a load-address
expression. Under the rules measured here that needs a use of the
delay-slot `i = 0` that survives copy propagation and is not a loop phi,
or a redundant in-block def that dead-store elimination does not count as
an overwrite. Neither has a spelling this lane found in fifteen cycles;
the cheapest untested shapes are named in the handoff.

## Cycle ledger

     1   pointer body on the 9 body: init from D_o058_5C98 / D_o058_5C80 + 12 / byte arithmetic   3 cells: +4 (wv-t's result) / **14 at delta 0**, W t0 / 14
     2   the pointer on the `cursor` cell; `cursor[0]`; `*cursor++`; two physical-line splits      5 cells: **6 at delta 0** (adopted) / 6 / +4 / 6 / 6
     3   `D_o058_5C98[i]`; `cursor[i]` with cursor fixed, two inits                                 3 cells: +8, all three: init base + i*4, load in the delay slot
     4   the reset in call 3's argument / at the top of 183 / before call 3                          3 cells: +8 / 39 (the 48 form, seven rows) / +8
     5   `while (i < n)` / `for` / `if (i < n)` with the reset at 180                                3 cells: +8, one object
     6   neighbour captures for W and the 5E9C piece on the 14 body; the alias-provenance traces    2 captures, 2 traces: the 22 named, isvar/may-alias vs islda/no-alias
     7   `register s32 i`; a non-call boundary after the reset; `-Olimit 5000`                        3 cells: +8 / +8 with `sll t8,zero,2` / 3510 at -20
     8   uninitialised index (the unused `portraitX`), mini TU and whale                              2 cells: loaded from its home, +12
     9   `i = 0` inside the title guard, with and without the delay-slot def; at the top of 183       3 cells: 46 / 46 (one object) / 39
    10   pointer-difference subscripts, three spellings                                               3 cells: +28 / +16 / +28
    11   pointee type: `char *(*)[6]`, `char *const *`, a one-member struct                            3 cells: 6, byte-identical
    12   row reset inside the title guard after the loop, with and without the preheader def          2 cells: **2350 at +8, the head exact through +0x1370** / +16
    13   `i = 0` inside the row guard too, and its control without the title preheader def            2 cells: 50 at delta 0 / +8
    14   the row guard on `i`; both guards on `i`                                                     2 cells: 50 / 50
    15   row loop nested in the title guard with else-duplicated saves; `i = k = 0`                   2 cells: +8 / +20

Cycle 0 (uncounted): the harness re-pointed at this worktree reproduces 9
on the tree source with `.text` identity; the flow graph from `-zdbug:2`
against the target's branch layout; the `cc -S` listings of the 9 and the
pointer bodies; eight mini TUs on the propagation rule.

## Reproduction

Cells compiled from copies of the TU in private scratch with the tree's
`tools/ido/cc` and the configured flags, then with the instrumented `cc`
under `CDX_LOG=1 CDX_PROC=0 CDX_OUT=... CDX_DETAIL_WEB=all
CDX_LINEAGE_TABLES=all`; every unforced cell passed the `.text` identity
gate. Scores from `nm_ranking.process_item` on the stock object; the
adopted body re-scored with `tools/score_symbol.py`, `tools/align_symbol.py`
and `tools/residual_map.py` on the tree. Banked under
`whale-resources/cells-wv-x/` in Git's common directory: every cell's
source, `result-cell-*.json` and `allocator-cell-*.log`, the objects
`c2a-6.o`, `c1a-14.o`, `c4b-39.o`, `c13a-50.o` and `c12a-2350-delta8.o`,
the neighbour captures `d1aW`/`d1aP`, the mini TUs, and under `alias/` the
`cc -S` listings and the two alias-provenance traces.
