<!-- plateau-handoff:overlay17CreateChain:start -->
### `overlay17CreateChain` plateau handoff

- source: `src/overlays/o017/overlay17CreateChain.c`
- score: 65 differing words
- frame: 0x80
- relocations: 7
- first mismatch: +0x34
- summary: One counter shared by both loops: 83 to 65. Open: half-buffer size coloured a3 not ring t7; else-arm template address coloured not a pre-branch ring temp.

Summary before this remeasure: Declaration order and chain->red masked with 0xFF (ring phase): 115 to 83. Template-loop webs rotate one position; halfBufferBytes in a3 not t7.

Summary before this remeasure: Template pointer is source on the no-material arm; source-before-destination; while(x--) loops: 130 to 115, frame 0x80 closed. Ring colour rotation remains.

Summary before this remeasure: Fresh V0 retains 130 differences; proxy evidence cannot authenticate the shifted template HI16/LO16 pair, leaving no new source lever.

- geometry: Target and configured C remain exactly `0x310`/784 bytes/196 words; the owned Overlay 17 range is `+0x318..+0x628`, ROM `0x1873CD0..0x1873FE0`, followed immediately by `overlay17ReleaseChain`. Target frame is `0x80`; candidate frame is `0x88`.
- ABI/flags: The configured constructor takes owner/count/material/scale, four `f32` geometry values, and four color bytes, returns `Overlay17Chain *`, and uses overlay game-code `-O2 -mips2 -32`.
- relocation proof: Target has seven runtime records and candidate five static records. Three offsets/types and one stable/effective identity align. The target template pair is at `+0xA4/+0xB0` with identity `overlay:17:+0xA90`; candidate `gOverlay17TemplateReloc` remains unresolved at `+0xA8/+0xB4`.
- diagnosis: Workbench reports 66/196 matching words, 130 raw/masked differences from `+0x0`, 41 opcode mismatches, and no instruction-count delta. Its acceptance basis is relocation-symbol mismatch.
- caller/donor evidence: One resident `R_MIPS_26` caller at VRAM `0x8001C70C` and the Overlay 17 export at `+0x318` authenticate the entry. The retained donor scan remains weak (best Conker Jaccard 0.0574), with no source or relocation witness.
- proxy recheck: Canonical proxy tooling does not infer an identity from the target runtime identity alone and does not normalize the four-byte relocation offset displacement. No exact same-overlay matched sibling supplies a unique witness for this template proxy.
- history: Authorization commit `d5ccd88015299d96c20e3c793d6dfc52bf593eb9` permits only fresh V0 and structured finalization. The guarded C body, prior source forms, flags, and permutation remain closed and unchanged.
- next action: Reopen only if a source-authentic mechanism explains both the frame/schedule difference and the shifted template relocation sites, or if independent exact same-overlay ownership evidence authenticates the proxy without target-assisted circularity.

### 2026-10-01, lane d-ovl2: frame closed, 130 to 115

Priced edits, each measured with tools/shape_product.py at size delta 0:

- `chain->template = source` on the no-material arm (it was 0): the target
  stores the template address there. Frame 0x88 to 0x80 with `vertexCount`
  inlined as `count * 2` and `while (index--)`/`while (buffer--)` loops
  (`!= 0` post-decrement spelling emitted `sltu` and a branch-likely): 130 to 123.
- `source = ...` assigned before `destination = ...`: 123 to 115.
- Flat: `while (vertex--)` against `vertex-- != 0` (same count, matches the
  target's copy shape), halfBufferBytes type, header-size local (worse),
  inline `count * 20` (+8 bytes), loop spellings (index = 16 while, for-down).
- Remaining residual is colour only: template-loop webs rotate one position
  (cand v1,v0,a1,a2,a3,a0 against target v0,a1,a2,a3,a0,v1) and halfBufferBytes
  takes a3 where the target holds it in a ring temp (t7).

### 2026-10-01, lane d-ovl2 (second pass): declaration order and a ring no-op, 115 to 83

- Declaration order, best of 150 random permutations of the eight locals
  (floor 109 reached by several orders): source, vertex, buffer, destination,
  halfBufferBytes, index, the six floats, chain. 115 to 109.
- `chain->red = (red & 0xFF)`: the u8 mask the peephole deletes still spends a
  ring temp (L127); 109 to 83 at size delta 0. It is the best of 37 single
  `& 0xFF` wraps of the u8 reads and stores (next best 85, chain->blue), and
  no pair of them beats it. Wraps on the s32 locals (count, index, buffer,
  vertex, halfBufferBytes) with `& -1`, or-zero, `* 1` and `(s32)` all fold early
  and are flat.
- Left: 58 naming rows, 10 structural; the template-loop webs still rotate one
  position and halfBufferBytes takes a3 where the target holds a ring temp.
#### 2026-10-02, lane g-ovl5: no change, 83

- Template loop as `do/while (index--)` or `for (index = 15; index >= 0; ...)`:
  both 83 (canonicalised); count-up loops 190 at +128.
- Declaration-order hill climb over the 6 locals: floor 83.

#### 2026-10-02, lane x-ovlb: one shared loop counter, 83 -> 65

The decision records ranked the two loop counters and the template loop's
post-decrement copy as separate webs. In the target the template-copy loop
and the alpha-clearing loop use the same registers for counter and copy
(a0 and v1), and uopt colours a symbol as one web. Writing both loops with
`index`, so the separate `vertex` counter goes away, gives the template loop
the target's v0/a1/a0/a2/a3 assignment. 65 at delta 0. Aligned buckets:
141 exact, 31 naming, 7 immediate, 21 structural. The one-sided words are
at +0xC0 and +0x190.

Measured and rejected:

- Five spellings of the half-buffer size and the alloc argument. Computing
  it after the call, inline, or through sizeof was 84 to 184.
- An 8-cell product over where `source`/`destination` are initialised and
  whether the else arm stores the template constant directly. The cells that
  stay well-defined are 65, 90 and 149. The 64 cell reads `source` before it
  is set in the else arm and is rejected.
- A fully natural rewrite, with indexed template copy, resident alloc and
  texture calls on their real arity, and a separate `size` local, is 185 at
  +12. The target keeps the header size in s0, which is the chain variable,
  so the size and the chain must be one symbol.

Still open, each one a named web:

- `halfBufferBytes` before the alloc call. The target computes it in ring
  temp t7 and stores it straight to its home at sp+0x38. Ours colours that
  segment a3.
- The else arm's template address. The target forms it in ring temp t2 in
  the block before the branch and rematerialises it in the textured arm
  (v0). Ours has one coloured web plus a copy.
<!-- plateau-handoff:overlay17CreateChain:end -->
