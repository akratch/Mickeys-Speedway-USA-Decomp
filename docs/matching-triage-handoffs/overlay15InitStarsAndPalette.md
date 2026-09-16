<!-- plateau-handoff:overlay15InitStarsAndPalette:start -->
### `overlay15InitStarsAndPalette` plateau handoff

- source: `src/overlays/o015/overlay_015.c`
- score: 60/247 words
- frame: 0x40
- relocations: 14
- first mismatch: +0x4
- summary: Index inits ahead of the colour block and the zero/zMax stores ahead of colorDivisor reach 60; the forwarded count*12 definition (s3 against s0) and the FP ring from yRange remain.

#### 2026-09-13, lane l1: counter reuse and measured bounds scheduling

Retained guarded source: 247 words, delta zero, seventy raw and masked
differences from plus 0x4. Alignment is 198 exact, thirteen naming, one
immediate and 32 paired structural rows, plus three candidate-only and three
target-only words. The frame and all thirteen stack-slot traffic records match
at 0x40. Candidate static relocations number fourteen against six in the
extracted target object; those different representations are not an exact
runtime relocation proof. Ownership remains overlay 15 text offsets 0x4C
through 0x428, ROM 0x18723E4 through 0x18727C0: 988 executable bytes. The
GLOBAL_ASM fallback remains active, and no new matched bytes are claimed.

The fresh procedure-2 baseline was 120 masked differences, 133 aligned exact
words, 73 naming, one immediate and 33 paired structural rows with seven gaps
on each side. Geometry and stack traffic already agreed. Its census was 114
draws and 310 emissions. Named Ucode mapping authenticates procedure 2 for
every retained source; complete stock/capture text fidelity passes throughout.
No instrumented or forced object is used as canonical compiler output.

Counter identity supplied the first gain. Reusing the completed star-loop
counter for the first palette index reached 94 masked differences. Reusing it
for the second index instead reached 65, with 185 aligned exact words and no
palette-tail naming residual. Total draws and their sequence stayed unchanged;
the definition-site emissions moved. This is allocator source identity, not a
claim that a hidden temporary draw disappeared. The 65-difference source and
object remain preserved as the best positional alternative.

Only after that source change, a fresh exhaustive landscape measured 171
single-force probes across 32 webs: 156 retained geometry and fifteen changed
size. Two address-carrier forces improved 65 to 63 individually, p1:w332=c5
and p1:w334=c3; neither reached zero. The disjoint positional footprints suggest
61 for the pair, but that combination was not compiled and is not a measured
result. The initial byte-count web cannot offer the target's occupied colour
in this shape. Existing landscapes were not repeated on unchanged source.

Reusing the other completed counter for the first palette index alone was
byte-inert. Splitting its zero initialization across the existing nonempty-loop
paths then reached 188 aligned exact words at delta plus four. The same-width
signed integer round trip of the count-global address restored delta zero and
reached 190 exact words, with six naming rows. It preserves pointer identity on
the configured 32-bit ABI; the cast is disclosed source shaping, not a new
storage identity. Generating the initial byte-count expression directly was
byte-inert. Moving the star-counter initialization into the positive-count
guard, alone or with that generated expression, worsened the result. Splitting
the initial multiply and retaining the byte count in the input parameter each
added a draw and disturbed the otherwise exact palette tail.

The draw census then located useful bounds-store moves. Advancing the y-range
store changed draw order and reached 194 aligned exact words. Advancing the
divisor store alone also reached 194, but their combination reached only 192.
Folding the x-max and y-range statements onto one source line was byte-inert.
Swapping the two existing entry counter definitions did not move any census
line and gave no gain. Separating the x-bound calculations from their stores
removed two draws but regressed to 171 exact words; reject it.

A bounded differential batch tested twelve dependency-safe adjacent statement
moves from the 194-word source. Each probe made one move, retained its census
comparison and aligned per-window delta, and preserved widths, accesses and
call order. Placing y-range after x-min reached 196 exact words; placing the
divisor store after z-max independently reached 196. Their measured combination
reached 198. The second move adds one paired structural row in one middle
window but removes two rows in the next window and removes a gap pair. This
local trade is retained explicitly; positional arithmetic alone would miss it.
The final source has 114 draws and 312 emissions. The palette tail remains
aligned exact, while the allocation-size carrier and bounds/entry scheduling
remain nonexact.

Stall evidence: the last improved shape is the paired store move, global
attempt 45. A second bounded batch checks nine previously unmeasured adjacent
neighbours of that shape and finds no improvement. The last three controls
(attempts 52 through 54) retain 198 aligned exact words and seventy masked
differences with unchanged geometry and stack traffic. They supply no new
carrier identity or residual reduction. Stop under ADR 0018 with this local
neighbourhood closed, not with a claim that all source schedules are exhausted.
A next pass needs a new trace-backed mechanism for the initial byte-count
interference or the remaining nonlocal store/entry scheduling; repeating these
adjacent moves or the unchanged colour landscape is not a new hypothesis.

Semantic review: the star loop retains its guard, calls, widths, increments
and global-count retest. Both paths enter the palette loop with the first
index zero; the reused second index is explicitly reset to one. The same four
palette formulas, index increments and 256-entry extent remain. The moved
bounds stores access distinct nonvolatile fields without intervening calls;
all field-read dependencies remain ordered. Source spelling and the concise
reuse comment are recompiled after selection. Meaningful alternatives are
preserved, not overwritten.

Evidence lives under ignored build/l1/overlay15InitStarsAndPalette, including
baseline, reuse-second-index, signed-count-address, paired-store-moves,
landscape65, adjacent-results.json, schedule-results.json and each saved
source/object/census/delta. Commands include configured full-TU compilation,
allocator_trace_receipt mapping/fidelity, draw_census profiling/comparison,
residual_map object comparison, web_footprint --every-colour on the changed
source, finalize_plateau, and tools/gates.sh. Stock output remains nonexact;
the normal fallback ROM is separately verified before committing.

#### 2026-09-16, lane s1-b: the size definition is forwarded, and two store moves

Baseline reproduced at 70 masked, delta zero, frame 0x40, aligner 198
exact, 13 naming, 1 immediate, 38 structural. Retained at 60 (61 raw, one
relocation artefact): 205 exact, 15 naming, 1 immediate, 27 structural,
one candidate-only word at +0x1CC and one target-only at +0x218. Four
cycles, 23 cells; the instrumented uopt (procedure 2, identity gate passed)
was read before any cell.

The s3-against-s0 head. Ours forwards starIndex = count * 12 into a
type-4 expression web (web 4: 3 over 2 blocks, save 1.5, decided last
among the callee-saved colours, s3), and starIndex's own symbol web (web 0,
s0) begins at starIndex = 1. The target keeps the size in s0 through the
allocate call and in the same block as starIndex = 1, which under
block-set interference (L115) is only possible as one symbol web: the
original's definition was not forwarded. This is the mechanism found on
func_80034448 in the same lane. register on starIndex is inert (70); a
two-statement size ((count << 2) - count, then <<= 2, or three statements)
emits an instruction (+4, 235); the L144 address form on the post-call
use costs +24.

Two store moves. Placing the three palette index inits ahead of the colour
block is 69 (one word; every placement of the three among the colour
statements is 69). Moving bounds->zero and bounds->zMax ahead of the
colorDivisor conversion, with the count store after it, is 60. The other
five placements of the colorDivisor store are 69 to 81.

Refuted at 60: a region opener before the index inits in three positions
(61: the three li's are still hoisted to the block head by as1, so this is
not a block question), palette read before the colour block (61),
operand order on the xMin/xMax products (60, inert), yRange after xMax or
before xMin (79, 61), a probe on bounds->xRange (+60, a load).

Next, in order of what the records support: the size definition needs to
stay a symbol web (the decision variable is uopt's expression forwarding
across the allocate call, exactly as on func_80034448, where the same
negative was measured over nine spellings); the FP ring from yRange's
mtc1 on is one draw behind the target, which under L149 is a folded FP
draw between the xMin product and yRange's conversion that no operand
order or statement order here produced; and the tail's three index inits
are scheduled at the block head by as1 in ours and at its end in the
target, with no dependence visible in ours that the target could have.

Evidence under Git's common dir, lane-evidence/s1-b/t3: sources, objects,
the procedure-2 ladder, side-by-side listings. Commands: private direct-cc
harness reproducing score_symbol on the base, score_symbol, align_symbol,
the instrumented cc with CDX_DETAIL_WEB=all.

<!-- plateau-handoff:overlay15InitStarsAndPalette:end -->
