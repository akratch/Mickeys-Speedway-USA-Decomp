<!-- plateau-handoff:overlay3RunCachedModeAction:start -->
### `overlay3RunCachedModeAction` plateau handoff

- source: `src/overlays/o003/overlay3RunCachedModeAction.c`
- score: 34/113 words
- frame: 0x58
- relocations: 15
- first mismatch: +0x84
- summary: Packed w29=c2 plus w39=c3 is 25 additive, not 0. valid is one web spanning encode so v1 is forbidden. L97 and lever 51 do not apply.
#### 2026-09-12, lane p23-lastmile5: complete colour landscape and boolean shapes

Fresh baseline: 113 words, delta zero, 34 raw and masked differences, frame
0x58, 15 relocations, first masked mismatch +0x84. The older 79-word header
counts agreeing words. Aligned buckets are 91 exact, 11 naming, zero immediate,
ten paired structural rows, plus a candidate-only word at +0x98 and a
target-only word at +0xE0. No source was adopted.

The single-procedure TU's full same-save-kind scan covers nine coloured webs
and 82 accepted alternatives: 74 retain geometry and eight grow. The complete
winners list is `p1:w29=c2` at 29, `p1:w29=c3` at 31,
`p1:w36=c4`, `p1:w36=c5`, `p1:w36=c6` at 31 each,
`p1:w39=c3` at 30, and `p1:w79=c3` at 31. The coarse-window packing selects
only web 29 at colour 2. A 15-cell lattice takes that force, web 36 at colour
4, and the web 39/79 winners, one colour per web. All cells apply; its measured
floor is 25 with webs 29 and 39, which are additive despite sharing coarse
windows. This is the selected lattice's floor, not an exhaustive combination
bound over the other colours. Stock/instrumented section, symbol and relocation
fidelity passes before that lattice.

Eleven source forms are retained under `build/p23/overlay3RunCachedModeAction/`.
Keeping the angle in its shifted representation, using either signed or unsigned
shifts, scores 43 at delta zero. Its aligned structural bucket falls to five
paired rows, but naming grows to 30 and the surplus moves to +0xA0; the missing
+0xE0 word remains. A short angle local scores 83 with eight extra bytes.
Copying valid through each of the existing angle, encoded and random locals
before the mode switch is flat on both shapes: 34 on the retained shape and
43 on the shifted one. The combined short-circuit expression scores 79 on the
raw shape and 84 on the shifted shape, each twelve bytes too large.

The source question is the transition from the comparison/call-result boolean
to the mode-switch boolean carrier. Moving the angle shift earlier repairs
some structure but does not supply that final carrier transfer. Existing-local
copies normalize away; a complete logical chain changes the frame and code
geometry. Stop after those carrier copies stall and the final expression-shape
check excludes the direct chain. Retain the guarded 34-word candidate, with
both source shapes and their distinct aligned residuals preserved privately.

#### 2026-09-12, lane p24-recipe: carrier deletion does not supply the boolean transfer

The configured baseline reproduces 34 raw/masked differences at 113 words,
delta zero, frame 0x58 and fifteen relocations. Aligned buckets are 91 exact,
eleven naming, zero immediate and ten paired structural rows, plus the
+0x98 candidate-only and +0xE0 target-only pair. First mismatch remains +0x84.
The prior complete nine-web landscape and selected lattice floor 25 were
reused on this unchanged baseline.

Deleting the encoded-result declaration and inlining its call scores 43 at
delta zero. Deleting the random-result declaration and inlining its call also
scores 43. Both lose a frame quantum; their unchanged naming buckets show that
removing these names does not change the desired colour. Retaining the unused
encoded declaration as a frame diagnostic reproduces 34 and the original
aligned map. That diagnostic is not adopted. The inline-call variants would
also require argument/read-order review before any promotion.

Deleting the two intermediate bound booleans, initializing valid to zero and
assigning the path-check result only inside the range, scores 52 at delta zero.
It removes the four structural rows in the mode-action windows but adds naming
rows throughout and retains the missing boolean-transfer word. Computing one
combined bounds boolean first and then checking the path costs eight bytes
and scores 80. The generated call-result and bounds shapes establish no exact
transfer identity, and the last three forms do not improve the baseline. Stop
under ADR 0018 with the original guarded body.

The named source question remains how a generated comparison or call-result
boolean reaches the mode-switch carrier without losing a frame quantum or
adding the target's missing transfer as an extra instruction. Sources, objects,
scores and aligned per-window comparisons are preserved privately under
build/p24/overlay3RunCachedModeAction. No new bytes are counted as matched.

#### 2026-09-13, lane f1: boolean width and conversion placement

The baseline reproduces 452 bytes, delta zero, 34 raw/masked differences,
first +0x84. The aligned map is 91 exact, eleven naming and ten structural
paired rows, with the candidate-only +0x98 and target-only +0xE0 words.
The single-procedure census has 17 draws and 171 emission records; each probe
passes full-TU stock/instrumented text identity. The prior nine-web landscape,
selected lattice and carrier-deletion receipts were read; no sweep was repeated.

Making valid an unsigned byte is semantically defined because every assignment
is a comparison yielding zero or one. It adds one draw at each of the three
boolean definitions, 20 total, and four emission records. The missing target
transfer disappears from alignment, but the function grows 24 bytes. Its
paired map is 23 naming, four immediate and eighteen structural rows, with
five candidate-only words. This is a measured width cost, not a near match.

Separating the path-call result from its equality-to-zero assignment preserves
all 17 draws and their sequence. One emission record is added overall, with
attribution moving from the call line onto the comparison line. Object text
and the entire aligned residual, including both gap offsets, are unchanged.

Keeping valid as a word and copying it into a separate byte only for the
switch produces two extra draws at the mode-three and mode-four tests, not a
single shared transfer at the copy. It grows eight bytes and yields seventeen
naming, three immediate and seventeen structural paired rows, with one
candidate-only word. Thus narrowing sinks to the consumers and does not
provide the wanted zero-cost mode-switch carrier.

The baseline is restored. Stop early under ADR 0018: both narrowing routes
have measured width costs, the split conversion is byte-inert, and the prior
boolean and existing-carrier controls cover the remaining identified forms.
The open requirement is still the call/comparison-to-switch transfer at the
original frame and geometry. No general source-impossibility claim is made.

Sources, objects, profiles and aligned deltas are retained in ignored
build/f1/overlay3RunCachedModeAction. Commands: configured stock compile,
draw_census.py profiles/comparison, residual_map.py --object --against,
finalize_plateau.py and tools/gates.sh. No new executable-byte credit.


#### 2026-09-13, lane `j1`: addressed boolean consumers

Baseline reproduces 113 words, delta zero, frame 0x58, masked 34 and first
+0x84. Aligned buckets are 91 exact, eleven naming, zero immediate and
twelve structural rows including the candidate-only/target-only pair.
Procedure 0 has seventeen draws and 171 emissions, with stock/traced
full-TU text identity. Static tables contain fifteen candidate versus eleven
extracted target relocations; runtime identities remain a separate proof.

The existing width, copy and conversion-placement receipts were read. One
new test reads valid through its s32 address in the two mode-switch consumers.
This tests the L144 reload mechanism without adding volatile ordering edges.
Draw count remains seventeen, but the integer draw sequence changes after
the path call. Emissions rise to 175, including increases at the switch and
both consumers. The function grows by 24 bytes and scores 98. Alignment is
55 exact, 33 naming, six immediate and 25 structural rows, including six
candidate-only words. The frame remains 0x58. Taking this local's address
therefore changes its emitted traffic and range, but does not supply the
wanted single transfer at unchanged geometry.

The baseline is retained. Stop early under ADR 0018: the new addressed-read
route has a measured width cost, and the prior narrowing/copy controls cover
the other identified mechanisms. This is no general source-impossibility
claim. Next action still needs a source-authentic call/comparison-to-switch
transfer at the original geometry. Private source/object pairs, profiles and
aligned deltas are retained. Commands: assignment gate, configured stock and
traced compilation, draw census before/after, residual_map.py comparison,
finalize_plateau.py and tools/gates.sh. ROM proof covers the fallback only.
#### 2026-09-13, lane l1: boolean-polarity and boundary controls

Baseline: 113 words, delta zero, frame 0x58, 34 masked differences, first
+0x84. Alignment has 91 exact, eleven naming and ten paired structural rows,
with one gap pair. Procedure 0 records seventeen draws and 171 emissions.
The earlier width, addressed-read and carrier-deletion controls were read.
Every new source/object pair passes named Ucode mapping and full stock/capture
section, symbol and relocation fidelity.

Inverting all three boolean definitions and their consumers preserves branch
truth conditions and call order. It leaves all per-line counts and the draw
sequence unchanged, and still scores 34 at exact geometry/frame. However,
alignment has 93 exact, five naming and thirteen paired structural rows, with
two gap pairs. The missing switch-transfer word remains. The extra structural
differences reject this trade of naming rows for a different boolean shape.

Moving the inversion to the switch boundary instead leaves the original
comparison/call definitions intact. It adds one draw and three emissions at
that exact line: eighteen draws and 174 emissions, with a changed sequence.
It costs eight bytes and scores 81; alignment is 72 exact, twenty naming,
three immediate and eighteen paired structural rows, plus two candidate-only
words. The target-only transfer disappears, but the new draw does not provide
it at the target geometry. The frame remains 0x58.

Retain the original guarded baseline. Stop early under ADR 0018: distributed
polarity retains the missing transfer, while a dedicated boundary inversion
has a measured instruction cost; the prior width/copy/address controls cover
the other identified routes. No general source-impossibility claim. Next action
still needs a source-authentic comparison/call-to-switch transfer at exact
geometry. All private sources, objects, profiles and aligned deltas remain
under build/l1/overlay3RunCachedModeAction. Commands: configured compilation,
allocator_trace_receipt, draw_census differences, residual_map comparison,
finalize_plateau and tools/gates.sh. No new matching-byte credit.

#### 2026-09-17, lane w10-o003: colour floor 25; valid cannot take v1

Identity-gated stock versus instrumented text, symbols and relocations pass
on the retained 34-word body. Procedure 0. Reproducing the priced pair
`p1:w29=c2` (29) plus `p1:w39=c3` (30) scores 25 at delta zero, additive.
That is the selected-lattice floor, not zero, so L160 has no zero-scoring
force to source-route.

On this shape valid is web 36: save 1.67, nocs 3, colour a0. Its cost list
starts at a0; v0 and v1 are absent because the one `valid` name spans the
encode call. Forcing it onto v1 is not a legal colour. Lever 51 does not
apply: the body loads `objects[cachedIndex]` once and does not walk a
pointer.

L97 `if (1)` and `do { } while (0)` around the valid block are
byte-identical to the baseline. Repeating `angle << 16` at the bounds is
the same object as `(s16)angle`. Copying valid through pointer-typed
`objects` copy-propagates away. Rehoming the shift into `encoded` is the
known 43-word shifted form: the extra word is a move of that symbol into
a1, and forcing that web onto a1 regresses because valid already occupies
a1. An else-zero birth of valid after encode costs 12 bytes.

The named requirement is still two ranges: a bounds boolean that dies
before encode (so v1 is offered) and a switch carrier born after the path
check, without else-zero stores or a new frame slot. The guarded 34-word
body is retained. Stop under ADR 0018.

<!-- plateau-handoff:overlay3RunCachedModeAction:end -->
