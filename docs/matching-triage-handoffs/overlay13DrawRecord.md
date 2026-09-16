<!-- plateau-handoff:overlay13DrawRecord:start -->
### `overlay13DrawRecord` plateau handoff

- source: `src/overlays/o013/overlay13DrawRecord.c`
- score: 0/189 words, promoted
- frame: 0x38
- relocations: 18
- first mismatch: none
- summary: Matched 2026-09-16 (lane s1-a); each command is a gbi-style block with its own cursor, the vertex word is built in gDma1p's OR order, and the colour bytes carry _SHIFTL's masks.
- assignment base: `6ce89d6b`
- owned range: overlay 13 `+0x580..+0x874`, 756 bytes / 189 words
- baseline: 108 differing positional words, `0x40` frame, first mismatch `+0x0`
- retained score: 100 differing positional words / 89 of 189 raw words exact; the masked 97-word residual is only a floor because 19 relocation sites name different symbols
- frame proof: exact `0x38`; declaring `savedScale` before the two pointer locals removes the candidate-only eight-byte non-save area
- relocation proof: target object has 10 text relocations and the candidate has 18; workbench reports 13 metadata mismatches, and preflight fails closed on eight unresolved candidate static identities
- mismatch proof: first mismatch is `+0x30` in the unlinked object comparison
- attempts: all 119 flag combinations and seven coherent declaration, pointer-type, command-store, constant-materialization, and expression-tree forms; no generic permuter was run
- residual: 28 opcode, 69 register, and six relocation-layout diff sites; workbench still classifies the function as `structure-mismatch`, so it is not an allocator-only near miss
- next action: reopen only with new evidence for the overlay-relative relocation identities or the render-state lifetime/register web; do not repeat the completed flag lattice or these source forms
- JFG: `func_overlay_1_00100678_1ED4198` in `src/overlays/o1/overlay_1.c` is the 0.245 masked 4-gram lead and has the same state split, color setup, vertex/triangle emission, optional phase-gated draw, and cleanup sequence. It remains assembly-only and is 768 bytes, with different render-state/data offsets. If Mickey reaches exact C, this body would be useful as the starting source at JFG's existing pragma for that symbol; at this plateau it is not release-grade and should not enter the public JFG ledger.
#### 2026-09-16, lane s1-a: matched and promoted, 97 to 0 -- the shared cursor was a phantom web

Baseline reproduced at 97 masked (100 raw), delta zero, first +0x54,
aligned 99 exact, 71 naming, 0 immediate, 22 structural with three
candidate-only words (+0x150, +0x238, +0x278) and three target-only
(+0x178, +0x210, +0x22C). Two cycles, twenty-one cells. Verified:
`gmake verify` prints the expected SHA1 from the C with the pragma gone,
`gmake promotion-proof` passes, `gmake check-overlay-syms` up to date.

The word count above was 192; the function is 189 words (756 bytes) and
the header is corrected. The "structural and relocation-bound residual"
this page closed on was three source-shape facts read off the listing:

- The ROM builds the vertex word as the OR of 0x04000000 with the shifted
  packet byte first, then ORs in 0x30: `or` with `at` holding 0x04000000,
  then `ori 0x30`. Ours folded `0x04000030` into one constant. That is
  `gDma1p`'s order (the command byte, then the packet byte, then the
  length, each through `_SHIFTL`), and uopt does not reassociate the
  constants.
  Alone it is 104 (the schedule shifts against the phantom below).
- Wherever both words of a command are ready in the same cycle the ROM
  stores w1 before w0 (the 05/&D pair, FB/-256, and the three tail
  commands): as1's LIFO line tie (L59), so each command's statements share
  a physical line, as a gbi block macro (a block-scoped `Gfx *_g` cursor
  followed by its two word stores) would (L132). Folding the two stores onto one line is 92; the
  block-scoped `_g` macro per append is 71, and with the OR order 39,
  first +0x208. With the block-scoped cursor the render pointer takes t1,
  the D_24 address t2 and the ring phase agrees: the shared `cmd` local
  was a phantom web holding v1 across the else arm (the mechanism nx-a
  named on the o058 quads), and the ROM's unused v1, a0 and t0 were its
  shadow. Every one-line and cursor-fold spelling that keeps one declared
  cursor stays at 92 to 104.
- The last 39 were the prim-colour word: the ROM evaluates r before g and
  draws the env command's `lui 0xfb00` immediately after the prim command's
  `lui 0xfa00`, so as1 hoists it to the block top. The OR of the three
  colour bytes each masked with 0xFF before its shift (24, 16, 8) and the
  alpha 0xA0 -- `_SHIFTL`'s masks on the u8 fields -- is 0; with `u32`
  casts inside the masks also 0. Without masks
  (casts or not, grouped, reversed, added, `s32`) every spelling is 38 or
  39, and the env word's spelling is inert.

The retained C is the DKR macro shape written out with a file-local
`O13_GFX(w0, w1)` block macro (PROVENANCE at the point of use: the shape of
`fast3d_cmd`/`gDma1p` in DKR's f3ddkr.h and gbi.h; the words are this ROM's
own). The remaining raw 4 are the overlay-relative `%lo` immediates of
D_20, D_28, D_24 and D_4, which the linker supplies; the promotion needed no
rebind, no pool and no new resident alias.
<!-- plateau-handoff:overlay13DrawRecord:end -->
