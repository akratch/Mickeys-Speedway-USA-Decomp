<!-- plateau-handoff:overlay98RenderReflections:start -->
### `overlay98RenderReflections` plateau handoff

- source: `src/overlays/o098/overlay98RenderReflections.c`
- score: 267/389 words
- frame: 0x1C8
- relocations: 36
- first mismatch: +0x14
- summary: Natural rewrite plus two-statement packets in the matrix arms: 332 to 267 at delta 0, frame exact; open: second node copy (s5) missing.

Summary before this remeasure: Natural rewrite, fixed transform offsets, target home ladder: 332 to 278 at delta 0, frame exact; open: node in s3 and s5, hoisted data re-read.

Summary before this remeasure: Exact-size V0 has a 56-byte non-save frame deficit and 21/36 relocation tuple alignment; prior natural mechanisms are exhausted.

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

## 2026-10-07 (lane a-ovl4): natural rewrite, 332 to 278 at delta 0, frame exact

The inherited body stored the transform and inverse fields to the wrong
offsets: the target writes the reflected height to transform +0x10, the
object x to +0xC, and each rotation to its own slot (+0 gets rotation 0
plus 0x8000), so the old candidate was not semantically the target. Fixed
in the rewrite, together with:

  - one callee for the context set-up and the matrix load (both calls are
    SYMBOL records naming resident +0x29484), two-argument matrix builds;
  - the visible list as gOverlay98AcceptedCount/gOverlay98AcceptedEntries
    (records LOCAL +0x84/+0x88, the names overlay98CollectAccepted uses),
    indexed, so uopt makes the cursor a temporary at the target's +0x5C;
  - literal 0x80000000 at each use (the target hoists one constant web
    into s6), plain locals, no volatile on the float homes;
  - the home ladder: five register locals, then modelDisplayList (+0x1B0),
    savedDisplayList (+0x1AC), matrices C/B/A (+0x16C/+0x12C/+0xEC),
    referenceY/distance (+0xE8/+0xE4), one cell, inverse (+0xC8),
    transform (+0xB0), emitted (+0xAC), index (+0xA8), one cell, the
    state index (+0xA0), then fourteen unused cells. Frame 0x1C8 exact.

Aligned: exact 183, naming 143, immediate 11, really different 61.

Measured and open: the target loads the node into v0 and copies it to two
callee-saved registers (s3 for useAlternate and the part arrays, s5 for
vertexData), and re-reads node->data hoisted above the alpha test for the
display-list pick. `node2 = node` (before or after the state-index store),
a chained assignment, an (s32) round-trip, and re-reading the nodes
element are all copy-propagated or reload (300 to 357). A volatile
state index gives the re-read but not the hoist (300, exact 128). The
emitted flag set just before the inverse call scores 282.

#### 2026-10-07 (lane a-ovl4, second pass): packet split in the two matrix arms, 278 to 267

Decision records (instrumented uopt, .text identity gate passed): the two
highest-save webs are the `gfx` variable and the `*dl` expression that
`gfx = (*dl)++` leaves (saves 89.75 and 93, interfering), so they take s0
and s1 and every later callee-saved web sits one register up from the
target (object s2 for s1, dl s3 for s2, heap s5 for s4). Writing the
packet as `gfx = *dl; *dl = gfx + 1;` in the two matrix arms only scores
267 at delta 0 (aligned exact 196, naming 130, immediate 11, really
different 43). The same form in the display-list block puts gfx, object,
dl and node on the target's s0..s3 but shrinks the function by 12 bytes:
with one web fewer competing, the entry cursor takes s8 instead of its
+0x5C spill. So the target has one more callee-saved web than this body,
which is the second copy of the node (s5); none of the node2 spellings
measured (copy, chained assignment, reload, cast round-trip, 192 cells
with the packet forms) creates it at delta 0. Block-scoped GBI-style
packet macros give -12 as well.

#### 2026-10-07 (lane c-ovla): second node web, two products, no change

Read from the aligned listing: the target loads the node into v0, copies it to s3 at the model load and to s5 just before the alpha test, and the alpha test's display-list pick re-reads `->data` through v0 into a0 while the model stays in a1. That is either two source variables neither of which uopt copy-propagates, or a globalcolor split of one node web.

- A second node variable in the `padNode` cell (frame unchanged), typed s32, u32, O98Node pointer or void pointer, assigned at the node load, after the state-index store or before the alpha test, used for the display-list pick and/or the vertex-data word: 36 cells, every one 267 at delta 0 (uopt propagates the copy in every form).
- The same variable assigned from a re-read of `object->nodes[object->nodeIndex]` (plain or through an s32 view of the slot) at the three positions: 313 to 361, all at +4 to +32.

Next: dump the records for the node web and read whether it is offered a split (decision=split) in any form; a variable whose copy uopt keeps needs a redefinition of one of the two between the copy and its uses, which the listing does not show.
#### 2026-10-07 (lane d-mid3): records read, the node is one coloured web; 18-cell product flat

Records (instrumented uopt, CDX_PROC=0, .text byte-identical to the stock
compile): the node is web 63, save 13.33 (80/6), coloured s4 at the 12.5
toll; it is not split, and no decision=split record touches it. The
model pointer is web 26 (a1). So in this body there is no globalcolor
split that could produce the target's s3 and s5 pair: two webs have to
exist before colouring.

Read from the target listing: the second copy (s5) is taken after the
state-index store and serves only the vertex-data word; the display-list
pick re-reads node->data through v0 (the node temporary, not s3) in the
alpha test's delay slot, while the model stays in a1 for the special
byte and mode4E. So the target's re-read is based on the expression
temporary for the node, which both node variables copy.

Product, axes N (padNode as a second node variable: none, re-read of
object->nodes[object->nodeIndex] before the alpha test, or a copy of
node), V (stateIndex volatile) and D (display-list pick through model,
node->data, or an s32 view of the node's first word), 18 cells:

- every N2 (copy) cell equals its N0 cell: the copy is propagated;
- N1 (re-read) is 325 to 354 at +12 to +36: the re-read reloads the
  nodes array instead of reusing the node temporary;
- D1 (node->data at the pick) with V0: uopt copies model into a0 and
  uses v0, 313 at +4; with V1 the volatile store kills node->data and both
  arms reload it through s4 (333 at +8), not one load before the branch;
- D2 (s32 view): 333 at +8 throughout;
- base 267 at 0 unchanged (V alone is inert).

Then measured: a second node variable (in the padNode cell) assigned
from the same subscript directly after the first, used only for the
vertex word. Chained (node = node2 = subscript) it is propagated and the
object is the base (267 at 0). As a separate statement it survives as a
second name (328 at +8): node keeps s4 (web 63, save now 60/6), but the
second name gets no register at all; it is stored to its home at
entry (sw at +0x1BC) and reloaded for the vertex word.

Cycle-21 line: the decision variable is the second node name's
allocation: it exists (separate-statement form) but is homed instead of
coloured, where the target gives it s5 with the same single use. Read its
p1dec record (CDX_DETAIL_WEB on the home's web) to see whether it is a
register candidate at all; if it is excluded, find what makes it
memory-class (its first use is in the drewObject block, after both matrix
arms), and try the copy placed after the state-index store as shipped.

<!-- plateau-handoff:overlay98RenderReflections:end -->
