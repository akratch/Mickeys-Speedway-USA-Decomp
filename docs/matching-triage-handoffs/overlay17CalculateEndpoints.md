<!-- plateau-handoff:overlay17CalculateEndpoints:start -->
### `overlay17CalculateEndpoints` plateau handoff

- source: `src/overlays/o017/overlay17CalculateEndpoints.c`
- score: 0/198 words, promoted
- frame: 0x58
- relocations: 3
- first mismatch: none
- summary: Matched. Per-file -Wab,-r4300_mul; one scale variable that is stored once and never reloaded; the tail in source order; one shared index whose transform loop compares element addresses.

Summary before this remeasure: Fresh V0 retains 133 differences; call proxies have no exact sibling witness, zero identities resolve, and only one relocation site aligns.

- geometry: Target and configured candidate remain exactly `0x318`/792 bytes/198 words. The owned Overlay 17 range is `+0x0..+0x318`, ROM `0x18739B8..0x1873CD0`, followed immediately by `overlay17CreateChain` with no padding.
- ABI/flags: `void overlay17CalculateEndpoints(Overlay17ChainHead *, f32 *, f32 *, f32 *, f32 *, f32 *, f32 *)` uses Overlay 17 game-code `-O2 -mips2 -32`. Target and candidate frames are both `0x58`.
- relocation proof: Target and candidate each have three `R_MIPS_26` records. Target sites are `+0x68`, `+0x108`, and `+0x1D4`; candidate sites are `+0x68`, `+0x10C`, and `+0x1D8`. One offset/type aligns and zero candidate identities resolve.
- diagnosis: Workbench reports 65/198 matching words, 133 raw/masked differences from `+0x24`, 88 opcode mismatches, and no instruction-count delta. Its acceptance basis is relocation-symbol mismatch.
- callers: Overlay 17 calls at `+0x51C` and `+0x75C` authenticate the entry and seven-argument ABI.
- donors: Fresh masked-skeleton search remains weak: Conker `func_1512DEA4` leads at 0.086 Jaccard, followed by Conker `func_151B118C` at 0.084 and Perfect Dark `model_update_chr_info` at 0.080; none supplies a source or relocation witness.
- proxy recheck: `overlay17TransformReloc` and `overlay17SqrtReloc` occur only in this unmatched function. No exact same-overlay sibling authenticates either name, and current tooling correctly refuses target-assisted identity inference or offset normalization.
- next action: Reopen only if independent exact ownership evidence authenticates the call proxies or a genuinely new source mechanism explains the two four-byte call-site displacements. Prior source, flag, and permutation families remain closed.

### 2026-10-01, lane d-ovl2: declaration order, 133 to 121

Measured with 400 random permutations of the eight local declarations
(tools/fast_score.py, size delta 0 throughout): 121 is the floor, reached by
seven orders; the one adopted declares index, deltaZ, scale, lengthSquared,
points, transform, point, deltaX. It moves `points` to the target's 0x30 home.
Non-volatile lengthSquared (plain, or with a volatile write-only sink) was
measured and is 12 to 16 bytes short, with a different float ring from the
first block; volatile stays. Remaining blocker: the target stores the length
squared once to 0x24 and keeps f18 live, where volatile emits two reloads.
#### 2026-10-02, lane g-ovl5: no change, 121

- Product over volatile on/off, three spellings of the zero fill and two of
  the transform loop (12 cells): floor 121 (volatile on, current loops).
  Volatile off is 168 to 177 at -4 to -16 with frame 0x60 against 0x58, first
  mismatch moving to +0x18 (a float ring one position off in the scale loads).
- Declaration-order hill climb over the 8 locals: floor 121.
- Target reading: the length-squared store at 0x24 is never reloaded (the
  value stays in f18), and the two-point transform loop is unrolled by two with
  a `sltu` guard; neither is reproduced yet.

#### 2026-10-02, lane x-ovlb: matched and promoted

121 -> 0 at size delta 0, frame 0x58, three call relocations, verified.
The steps, each measured in turn:

- Per-file `-Wab,-r4300_mul`. The target has a nop between two `mul.s`
  and a rotated loop that ends in a branch-likely, and IDO emits both only
  with that flag. Under the flag the old volatile spelling grew by 8 bytes
  (two extra reloads). The flag did not change AdvanceChain or CreateChain.
- No separate squared-length variable: `scale` gets the squared length and
  is overwritten by the sqrtf quotient. Of eight spellings measured this is
  the only one at delta 0, and it scored 83. The target's one store with no
  reload is `scale` being live into the call, not a volatile.
- The tail order was searched over 1,440 dependency-valid statement orders.
  The natural order was best, at 3: scale both deltas, store oldX/oldY/oldZ,
  clear dirty, write points 3/4/5, then adjust points 0 and 2. Putting
  `index = 5` before the cursor in the zero loop gave 1.
- The last word was where `scale` lives on the stack: sp+0x20 against the
  target's sp+0x24. Declaration order does not reach it; 210 placements of
  index/scale/point all scored 1. What reaches it is how many integer loop
  variables the function has. An index plus a cursor gets two register cells,
  and the target has one. Writing the transform loop with the shared index
  costs a different instruction, because IDO turns `index < 6` into an
  equality branch. Comparing the element addresses
  (`&points[index] < &points[6]`) keeps the target's unsigned pointer test,
  and that closed the function.
<!-- plateau-handoff:overlay17CalculateEndpoints:end -->
