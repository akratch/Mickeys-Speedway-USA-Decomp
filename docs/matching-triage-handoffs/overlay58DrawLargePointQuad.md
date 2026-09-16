<!-- plateau-handoff:overlay58DrawLargePointQuad:start -->
### `overlay58DrawLargePointQuad` plateau handoff

- source: `src/overlays/o058/overlay58DrawLargePointQuad.c`
- score: 21/104 words
- frame: 0x18
- relocations: 11
- first mismatch: +0x14
- summary: 34-draw schedule is stable; register-only residual needs a new source-authentic web and temporary-ring mechanism
- base commit: `f8f3ec51a298dd0eddd0574a4313adbb1e39de9b`
- verdict: bounded plateau; zero exact bytes gained
- boundary: overlay 58 text `+0x50C8..+0x5268`, exactly 416 owned bytes with no target padding
- ABI: `void (s32, s32, s32)`; sole local caller at `+0x126C`
- configured recipe: `-O2 -mips2 -32` with `NON_MATCHING` selecting the guarded C body
- V0: exact 104-word extent and `0x18` frame on both sides; 34 positional words agree and 70 differ
- diagnosis: allocation-only residual with 70 register sites, no opcode or schedule drift, first pool divergence at slot 6, and first temporary-lane divergence at slot 0
- mechanism limit: nine of twenty substitution webs require ring-only temporary registers, so forced global coloring cannot close the full residual; web existence and temporary-queue phase both differ
- sibling: fresh `overlay58DrawPointQuad` preflight reproduces the same 104-word extent, frame, 34/104 score, first mismatch, pool signature, and temporary signature
- relocation proof: target and candidate each carry eleven records, but only ten offsets/types align; the payload high-half occurs four bytes late in C
- relocation identities: runtime authenticates the resident setup call plus the render-state, display-list, vertex-cursor, and payload roles; static fallback proof resolves only three candidate identities and zero complete aligned identities
- flags: the retained 119-row lattice is already nonexact and was not repeated
- donor check: current DKR/BK masked-skeleton scan is negative; the best size-compatible similarity is 0.062 and is not a credible body donor
- fresh hypothesis: reusing the dead physical-address scalar as the later color value is byte-identical to V0 and does not change pool position
- prior exhausted levers: qualifiers, declaration and statement order, pointer/array forms, signedness, literal forms, named color lifetimes, explicit cursor references, state wrappers, and split increments
- permutation: the retained 40-minute bounded run found only a synthetic do/while coalescing form; it is rejected and was not repeated
- validation: `tools/function_preflight.py overlay58DrawLargePointQuad --json`; `tools/wb_compare.sh --diagnose --no-build overlay58DrawLargePointQuad`; `tools/function_preflight.py overlay58DrawPointQuad --json`; `tools/skeleton_scan.py similar --target 58:+0x50C8`
- next lever: obtain new fidelity-clean allocator evidence that explains both the long-lived cursor-address pool position and the temporary-queue phase, then test only its source-authentic web-existence form; do not repeat flags, generic permutation, or forced colors

#### 2026-09-13, lane g1: a measured compensating draw pair improves 24 to 21

Fresh base-only assignment and stock/traced full-TU identity pass. The incoming
104-word candidate has 80 aligned exact and 24 naming rows, no structural or
immediate residual, and no surplus/missing offsets. Its 0x18 frame and four
used stack offsets agree with the target. It spends 34 draws and 188 emission
events; the older 70-word prose above is historical. No colour sweep is rerun.

The retained source generates the first vertex-cursor load inside the command
expression and removes that expression's redundant byte cast. Measured one
change at a time: removing only the physical-address declaration is byte-flat;
generating the vertex load then adds one draw at the command line (eight to
nine), giving 35 total draws and a 62-word positional residual. Removing the
byte cast on that shape removes exactly one draw at the same line (nine to
eight), restoring the original draw sequence. The result is 21 raw/masked
words, 83 aligned exact and 21 naming, with zero immediate/structural rows,
no unique offsets and unchanged 416-byte ownership. The +0x0 window loses
three naming rows; every later aligned window is unchanged.

The source equivalence is narrow: the deleted cast follows a mask and OR whose
result is at most 0x26, and the explicit byte mask remains. There is no write
between the original cursor load and its new use; discarded pointer tests have
no observable effect. The removed declared intermediate is unused elsewhere.
The retained function remains NON_MATCHING. This is an improved compiled
candidate, not object-exact or ROM-exact C, and adds no matching credit.

Other individually preserved controls: generated modular bias spellings retain
all draws but add a word; reusing the spent command carrier for the color stores
keeps 34 draws but regresses to 24 naming rows. Generating positive X reaches
its requested register but merely transfers three naming rows to positive Z,
so it ties 21 with a worse later window. Generating positive Z on that shape,
or moving its declared definition after the existing boundaries, adds a
register to the scratch ring without changing the total draw count; they give
64 and 65 positional differences with structural rows and paired offsets.
These last coordinate controls supply no better residual or new route to the
remaining address/coordinate allocation, ending the packet under ADR 0018.
The first 21-word source is retained rather than the equal-score local tradeoff.

Stock align_symbol.py, score_symbol.py and frame_census.py reprove the retained
candidate: 104/104 words, frame 0x18, first naming difference +0x14. The relocation
count remains eleven; exact runtime relocation identities have not been newly
proved. Evidence under ignored build/g1/overlay58DrawLargePointQuad includes
source snapshots, stock/traced objects, census pairs, raw logs and aligned
per-window maps. Commands include draw_census.py --save/--compare,
residual_map.py --object/--against, allocator_trace_receipt.py, the configured
compiler and finalize_plateau.py. Full ROM gates validate the fallback tree.

#### 2026-09-16, lane nx-a: the residual is a phantom symbol web, not a colour choice

Baseline reproduces 416 bytes, delta zero, 21 masked and raw, frame 0x18,
first mismatch +0x14; aligner 83 exact, 21 naming, no other buckets. The
sibling overlay58DrawPointQuad now carries this exact residual after adopting
this file's generated-cursor-load form (24 to 21 there), so the two are one
question.

Records (lineage census, 17 webs, 14 coloured, 3 split): the declared `gfx`
symbol is web 5, type symbol, frame offset minus 4, refs in blocks 3, 5 and 6,
save 3.5, coloured a1, and it is never emitted, because the two `dl++`
expression values are one web (web 7, type expression, blocks 3 and 5) that
supplies every use. Web 7 has v0 forbidden by the 0xFF constant (web 89,
block 5) and a1 by the phantom, so it takes a2; xPlus (web 51) then has a1
forbidden by the phantom and takes t0; y's post-call piece (web 90) takes t1;
the 0x80000000 constant (web 85, block 3 only) finds v0 free. The target
colours are gfx v0, xPlus a1, y t0, constant t1.

Force lattice on those four: w85 to c8 applied and scores 59 alone; w7 to c1,
w51 to c4 and w90 to c7 were not applied (forced minus 2, already forbidden),
so the colour axis is closed for the three that matter. Deleting the gfx probe
leaves the phantom at blocks 3 and 5, still a1, and zPlus takes a1 instead
(24). Eleven probe and carrier cells: dropping any probe regresses (23, 28, 41,
45, 47), literal 0x80000000 and one-line first append are inert (21), block
scoped `_g` per append is 28 with probes and 83 without. Eight loop forms of
the colour stores: 82 to 103, most changing size. Eighteen boundary cells:
`if (1) { }` between the appends equals the cursor probe (21); any boundary
after the second append is 45 to 84. Thirty-two cells moving xPlus and zPlus
above the colour stores: floor 22 with a different residual (the display-list
address moves to v1). Six call-argument line ties: inert at 21; two failed to
compile. The +0x14 lui pair is therefore not an L59 tie under this shape.

Next hypothesis: as recorded on the sibling shard. The decision variable is
the phantom `gfx` symbol web and the block set of the 0xFF constant; no
spelling of the current locals moves either.

<!-- plateau-handoff:overlay58DrawLargePointQuad:end -->
