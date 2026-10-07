<!-- plateau-handoff:overlay98RenderReflections:start -->
### `overlay98RenderReflections` plateau handoff

- source: `src/overlays/o098/overlay98RenderReflections.c`
- score: 57/389 words
- frame: 0x190
- relocations: 36
- first mismatch: +0x0
- summary: Exact-size V0 has a 56-byte non-save frame deficit and 21/36 relocation tuple alignment; prior natural mechanisms are exhausted.

## 2026-09-11 the frame runs the other way, and the 56 bytes are solved (lane `lane/o11-frames`)

No source change adopted.  The frame arithmetic is now closed as arithmetic, and
the placement is measured rather than argued.

**The identity.**  Both objects put the argument build, the ten saved-register
words and the single saved double at identical absolute offsets, and both leave
the same 80 bytes below the declaration block.  So the target's larger frame is
entirely block: 376 bytes against this candidate's 320.  **This candidate is 56
bytes of declaration short**, which is the opposite of every other frame residual
on the queue and is why `L112` is the law that reaches it -- the free parameter
here is how many cells the target declares, not how many this one can shed.

**Where the 56 bytes go.**  The three 64-byte matrices are visible on both sides
as the only address-taken slots, and their spacing is 64 on both, so they can be
used as rulers.  Measured from each frame top:

- this candidate: matrixC at -64, so the matrices are the first declarations
- the target: matrixC at -92, so **28 bytes -- seven cells -- are declared before
  the matrices**
- between matrixA and `transform`: two cells here, three in the target
- after `inverse`: eighteen cells here, twenty-four in the target

7 + 1 + 6 = fourteen cells = 56 bytes.

**Two of the seven pre-matrix cells are identified.**  The target homes -24 with
one load and two stores and -28 with two loads and one store.  Those are exactly
the traffic signatures this candidate's `gfx` and `cursor` carry at the bottom of
its own block.  Declaring `gfx` sixth and `cursor` seventh, ahead of the
matrices, is therefore the reconstruction, not a padding choice.

**Verified by construction.**  Filling the three regions with five, one and eight
unused `s32` cells and moving `gfx` and `cursor` to positions six and seven:

- frame 0x190 to **0x1C8**, the target's, at delta 0
- first mismatch +0x0 to **+0x68**
- immediate-only 17 to **9**; masked 332 to 327; byte-exact 96 to 102
- **24 of the 37 stack slots land on the target's exact offsets**, up from the
  14 saved-register and argument slots that agreed before: all three matrices,
  every field of `transform` and `inverse`, `cursor`, and `savedStateIndex`

The padding is a measurement, not a candidate, and is not committed.  What it
establishes is that the target's block is solvable and that the placement above
is right within two cells.

**The two cells still wrong.**  After that fill, the candidate homes -368 and
-372 (`savedReferenceY`, `savedDistance`) where the target homes neither, and
misses the target's home at -296.  Both are `L118` shaped: the target carries
those two volatiles in registers and homes something this candidate does not
declare at all.  Deleting the two volatile declarations frees two of the eight
trailing cells for whatever the target's -296 local is.

**Not the lever:** declaration order alone.  Moving `gfx` and `cursor` to the
front without the fourteen cells leaves the frame at 0x190 and the score at 332.

## 2026-10-03, authenticated two-argument matrix-build packet

The untouched configured baseline again measures 332 masked / 333 raw
**differing** words at 1,556 bytes / 389 words, frame `0x190` against retail
`0x1C8`, first mismatch `+0x0`, and 36 static relocations versus 36 runtime
records. The older header's `57/389` is matching-word coverage, not a
57-word residual. The translation unit contains only the owned function.

Mickey's runtime table independently binds both matrix-build calls to
`func_8002AA50` and the inverse call to `func_8002AC84`. Already-matched
camera/model callers use two arguments. Direct retail helper inspection,
rather than their nonmatching C reconstructions alone, establishes that both
helpers overwrite the incoming third argument register before reading it.
The retail caller also performs no third-register preparation between the
inverse return and the following build call. This supports a focused
argument-constraint contrast; callee behavior alone does not establish the
original caller's source prototype.

Removing only the two third formals and three pure zero actuals removes
exactly three argument preparations. Stock IDO measures 316 masked / 317 raw
differences at 1,544 bytes / 386 words, delta `-12`, with the unchanged frame
and 36 relocations. The masked positional reduction is nonexact evidence:
three executable instructions and the 56-byte frame deficit remain missing,
and relocation identity/schedule proof is incomplete. This artifact remains
private; no candidate source or byte credit is adopted.

A separately authorized storage correction replaces the three 64-byte byte
facades with `f32[4][4]`, preserving their extent and supplying truthful
array-compatible matrix declarations and projection-pointer access. Actual
retail readers/writers establish complete 64-byte float inputs/outputs; the
multiply supports the existing destination-equals-left-input use. Native
compile-only assertions confirm 64-byte extent, 16-byte row stride, and
four-byte alignment. This correction is raw-code, relocation, and
readonly-data identical to the arity artifact. It repairs the local matrix
storage view without explaining the residual or frame deficit. Fixed-output context
facades and the integer pointer carrier remain outside this correction and
require independent semantic review before any promotion.

The actual asm-processor compiler inputs, stock objects, comparisons, and
meaningful source alternatives remain private. Untouched IDO replay passes
text, data, readonly-data, symbol, and relocation fidelity; whole-file debug
metadata differences are disclosed. Baseline and candidate expanded-input
self-context pass. Cross-context comparison correctly reports the authorized
external prototype/type changes; it was not forced to claim unchanged input.

The tracked diagnostic body is restored exactly. This packet closes early on
an authenticated arity effect and an eliminated matrix-storage mechanism;
no padding, declaration/home grid, flags, or forced allocator work followed.
A future packet needs independently supported missing executable behavior or
compiler producer evidence. Lower positional differences at a short extent
and truthful type spelling alone do not reopen the closed layout families.

<!-- plateau-handoff:overlay98RenderReflections:end -->
