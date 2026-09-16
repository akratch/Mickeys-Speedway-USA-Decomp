# The slot rows close on a sibling's spelling; W is a closed system of margins: lane wv-w

**Retained: 9 masked / 186 raw differing words, 14,456 bytes, delta zero,
frame 0x138, first mismatch +0x12E4, 1,253 owned text relocations.**
Down from 11. Aligned buckets move from 3603 exact / 11 naming / 0 / 0 to
**3605 / 9 / 0 / 0**, displacement tax 0. Unforced. The C remains guarded
`NON_MATCHING`; the ROM is unaffected and `gmake verify` proves the
assembly fallback, not this C.

The 9 is W alone: `&D_o058_5EA0`'s case-12 piece in `a2` where the target
has `t0`, at +0x12E4/+0x12E8/+0x12EC, +0x1354/+0x1358, +0x14FC/+0x1500,
+0x1520/+0x1528. The two slot-address rows are closed. Ten measured cycles
of the twenty; the lane stopped when the records had turned W into a
system of inequalities with no free variable it could reach from source.

## The two slot rows: the operand order is the tree's evaluation order

wv-r's fourteen and wv-v's seven spellings of `saves + index*32` compiled
to one object, and the handoff concluded the order was decided below the
expression. It is not; it was one spelling short. `overlay60Prefix.c`
(matched) writes the same lookup as

    i = levelGetBlurEffect(...);
    slot = (SavesSlot *)((u8 *)slots + i * 32);

On a mini TU (`cells-wv-w/mini/m1.c`) the forms separate: `&saves[f()]` and
`&saves[i]` evaluate the index first (`sll` draws the first ring register,
the base the second, `addu base-last`), while `(u8 *)saves + i * 32` with
`i` a declared local evaluates the base first. wv-v measured the dead-local
index (`c10g`) and the byte arithmetic (`c10c`) separately, each
byte-identical to `&saves[f()]`; together (cycle 1, `c1a`) they measure
**9 at delta zero**, `.text` identity held, and both slot rows are exact.
Adopted at both sites with `portraitIndex` (case 9) and `erase` (case 10)
as the dead index locals, as wv-v's cell had them. `saves + i` through the
same locals (`c1c`) stays 11: it is the byte form that orders the sum.

## W: the growth test is arithmetic, and every term is now named

The split instrument (wv-t) records each candidate block's `new`,
`left_before`, `left_after` and `numintf`. wv-t's rule -- a block is
accepted iff `2*left_after >= numintf + new` -- was checked here against
**all 477 growth tests** of the 9 body: 477 agree, 0 disagree. So a piece's
shape is a function of three inputs: the colours held by its neighbours at
the moment of its split, the number of its neighbours (coloured or not,
memory-class candidates and lineage remainders included), and the number of
neighbours each candidate block adds.

Read with neighbour captures on the 9 body (`d9W`, `d9P`) and on wv-v's
`c8c` body (`d8cW`, `d8cP`), the two pieces seeded at 183 are:

- **the `&D_o058_5E9C` piece** (split at idx 127, 3.60): 24 neighbours in
  {183,184,185,190}; 17 colours at its seed (v0, s0, s4, s5, s7 held).
  Growth: 184 (s1 folds: 16), 190 (16), 185 (a0-a3 fold: 12; margin
  24-24 = 0, accepted), 191 (12; 24-24-1 = -1, rejected), 202 (12;
  24-24-1 = -1, rejected). This piece is never coloured; its occurrences
  are materialised inline (`lui a0; lw a0,0(a0)`), which is exact.
- **W** (split at idx 178, 2.26): the same 23 neighbours less the
  `&D_o058_5EA0` remainder (its own lineage never counts), plus v1 (w40,
  idx 136) and a0 (w251, idx 171) held: 15 at the seed. Growth: 184 (14),
  190 (14), 185 (a1-a3 fold: 11; 22-23 = -1, **rejected**), 191 (14;
  28-23-2 = +3), 202 (14; 28-25-1 = +2). Coloured `a2` at idx 266 with v0,
  v1, a0, a1, s0, s1, s4, s5, s7 forbidden. The target's `t0` needs a2 and
  a3 forbidden too, and only block 185's call supplies them: **W must
  accept 185 and keep 202.**

The 23 neighbours are all real. Six are register candidates that end up
wholly spilled (`arg0` w0, `state` w5, `rowHeight` w38, `savedOffset` w230,
`savedPosition` w291) or coloured late (`rowBase` w44, s2 at idx 323);
eight are address-lineage remainders whose hulls span the window
(`&D_800D3140` 919, `&D_o058_5C80` 957, 914, 915, 916, 924, 952, 953);
nine are coloured (v0 n, v1 w40, a0 w251, a1 w222 at colouring, s0, s1,
s4 textY, s5, s7). Only `textY`'s dead def at 183 is this source's own
artefact, and it is load-bearing for the piece (below).

### Why no configuration of the known variables reaches the target

With the rule and the lists, every candidate configuration was worked
through; the growth records of the cells confirm each arithmetic line.

1. **W accepts 185** iff 12 colours survive the fold: 15 at the seed is one
   short, and a0 does not help (with a0 free the fold takes four instead
   of three). The margin is v1 (wv-v) *or* s4: W passes 185 with v1 free at
   its split, or with no s4 holder at 183.
2. **W keeps 202** iff, after rejecting 191, `2*left >= numintf + new` at
   202. `new` at 202 is w157 (`arg0 * 0xF`, coloured s4 at idx 281) plus
   the `&D_o058_5E9C` remainder whenever the 5E9C piece has rejected 202
   (it covers 202 through its member there). With 12 colours that is
   24-23-2 = -1: **rejected**. wv-v's cycle 8 (`c8c`, W `t0`) accepted 202
   only because its 5E9C piece had taken 202 first (the +4), which removed
   the remainder from 202 and made `new` 1.
3. **The 5E9C piece must reject 191 and 202** or the tail's v1 piece
   (`lui v1/addiu v1` at +0x1514) is lost (+4, measured by wv-v and read
   here in `c8c`'s records: 185 accepted at 13 left, 191 rejected on the
   s4 fold from the row cursor w386, 202 accepted at 26-24-1 = +1). With
   s4 held at 183 it has 12 after 185 and rejects both; without, 13, and
   202 is taken.
4. Putting these together: W wants s4 free (or v1 free) and 13 colours at
   202; the piece wants s4 held, or a colour it alone loses. The only
   colour W loses that the piece does not is **v1**, so the piece needs v1
   held at 183 before idx 127 (w40 above 3.6) *and* W needs the 5E9C
   remainder off 202 before its own split (the 5E9C lineage's 202 seed,
   now at idx 195 and save 1.97, processed before W's 183 seed at 2.26)
   *and* no s4 holder at 183. Each of the three is closed below.

### The three decision variables, each closed at zero width

- **`w40`'s save is 3.0 in every partition** (cycle 3). It is `n*2` with
  three occurrences per block in blocks 13 and 183 (6/2); a literal-type
  split (`- 1U`, L151) leaves a single-block half at 3/1 = 3.0 and costs
  width (+4/-12: the unsigned index changes codegen); `(s32)` casts do not
  split (byte-identical). Nothing lowers it below 2.26 or raises it above
  3.6 without an instruction, and a use of `n*2` that folds into its
  consumer (block 234's single subscript) is not a member at all.
- **The `&D_o058_5EA0` remainder reaches its 183 seed at 95/42 = 2.26
  because of the seed walk, which is fixed.** The lineage members are
  created by a DFS over the CFG in which a switch's first arm (case 1) is
  followed immediately and the rest are popped in **descending case
  value**: 13, 12, 11, 10, 9, 8, 6, 5, 3, 2 (read from the jump table at
  .rodata 0x0-0x30 against the walk order). Case 13's seeds (229, 234, 243,
  245, 275) are therefore always processed before case 12's (181, 183),
  and they carry 89 of the lineage's 185 with only 4 of its 46 `nocs`.
  Aliasing case 12's occurrences onto a second symbol (cycles 7-10:
  `c7a`/`c8a` case 12 only, 47/8 = 5.9, split at idx 87; `c9a` all but
  cases 1/13, 138/38 = 3.63; `c10a`/`c10b` with one or two loop members
  left behind, 3.34 and 3.08) moves the seed exactly as the model says --
  at 3.3 the piece accepts 185 with margin +1 and rejects 191 and 202
  (`new` 2 at 202: w157 and the 5E9C remainder, whose 183 piece is carved
  later than at 178 in that ordering) -- and every alias changes the
  function from +0x104 on (-12 to -104), because the other lineage's
  timing moves too. The alias is a diagnostic, not a source.
- **The 5E9C lineage's 202 seed cannot precede W's 183 seed.** Its
  remainder is 73/33 = 2.21 at 194 and 63/32 = 1.97 at 202; W's is 2.26.
  Raising the 5E9C side needs weight in a case walked after 12; lowering
  W's needs 13 fewer from case 12's own row loop. Both are instructions.

### Closed here, with receipts

- Case-12 head statement order (cycle 2): four permutations of `opponent
  =`, `textY = 0`, `columnStep =` are byte-identical; `textY = 0` above the
  head's calls is +12 (the carrier goes live across them). Within-block
  order does not reach the allocator.
- Forcing W to `t0` (cycle 4, `f4a`, `forced=7` accepted): +64 with the
  residual starting at +0x600, the same as wv-r's reading; the force
  reshapes the caller-saved table, it does not model the target.
- `*&local` on the five spilled candidates (cycle 5): folded by cfe, the
  allocator log is byte-identical -- not a memory-class lever. `volatile`
  on the same five (cycle 6): every one moves the frame and the width
  (`state` +56, `arg0` +156, `rowHeight` -12, `savedPosition` -8,
  `savedOffset` -12), so all five are register candidates in the target
  too, and their six-way presence in W's `numintf` is the target's as well.
- wv-v's "a0 does not matter" and "v1 is the whole margin" both stand; the
  identity is now exact rather than itemised.

## Cycle ledger

     1   slot address: overlay60Prefix's form (dead local index + byte arithmetic) at both sites; `saves + i` through the same locals   2 cells: **9 at delta 0**, adopted / 11
     2   case-12 head statement order, five forms                                              5 cells: 9 (x4, byte-identical) / +12 (textY above the calls)
     3   w40 split by literal type: `- 1U` in case 12, in block 13, in one subscript, `(s32)` casts   4 cells: +4 (w40 3/1 at 13) / +4 / -12 / 9 (no split)
     4   force W = t0 on the 9 body                                                           1 cell: forced=7, +64, residual from +0x600
     5   `*&x` on state, arg0, rowHeight, savedPosition, savedOffset                          5 cells: allocator log identical to base (cfe folds it)
     6   `volatile` on the same five                                                          5 cells: +56 / +156 / -12 / -8 / -12, frame moves in each
     7   case-12-only alias symbol (file scope); inner-loop weight probe                      2 cells: -12 (alias split at idx 87, 185 accepted, 202 rejected) / -16 (blocks inserted)
     8   the same alias at block scope (numbering-neutral)                                     1 cell: -12, same object as 7
     9   alias of all cases but 1 and 13; alias of cases 2 and 12                             2 cells: -104 (3.63 at 183, before the 5E9C piece) / -76 (7.0)
    10   alias 9a with one, then two, row-loop members left on the main symbol                2 cells: -96 (3.34: 185 +1, 191 -1, 202 -1 with new 2) / -4 (3.08: same growth)

Cycle 0 (uncounted): the harness re-pointed at this worktree reproduces 11
on the tree source with `.text` identity, then 9 after cycle 1; neighbour
captures on the 9 body (`d9W`, `d9P`) and on wv-v's `c8c` (`d8cW`,
`d8cP`); the growth rule checked on all 477 tests; the jump table read from
the object against the lineage walk.

## Reproduction

Cells compiled from copies of the TU in private scratch with the tree's
`tools/ido/cc` and the configured flags, then with the instrumented `cc`
under `CDX_LOG=1 CDX_PROC=0 CDX_OUT=... CDX_DETAIL_WEB=all
CDX_LINEAGE_TABLES=all` (a web number in `CDX_DETAIL_WEB` for the
neighbour captures); every unforced cell passed the `.text` identity gate.
Scores from `nm_ranking.process_item` on the stock object; the adopted
body re-scored with `tools/score_symbol.py` and `tools/residual_map.py` on
the tree. Banked under `whale-resources/cells-wv-w/` in Git's common
directory: every cell's source, `result-cell-*.json` and
`allocator-cell-*.log`, the 9 object (`unforced9.o`), the readers copied
from wv-u/wv-v, and `mini/m1.c` with its object.
