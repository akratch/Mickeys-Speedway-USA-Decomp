<!-- plateau-handoff:overlay19BuildAdjacency:start -->
### `overlay19BuildAdjacency` plateau handoff

- source: `src/overlays/o019/overlay_019.c`
- score: 0/123 words
- frame: 0x80
- relocations: 1
- first mismatch: none
- summary: Resolved ROM-exact by placing the existing fifth-argument assignment inside the call; 492 executable bytes, one exact relocation.

### 2026-09-13, lane d1: move the assignment into the call

**Resolved, Tier A:** the canonical linked range at overlay 19 text offset
0xA30 through 0xC1C is byte-identical to the ROM. Promotion proof passes for
123 words, frame 0x80, one relocation and static identity. Full `gmake verify`
prints SHA-1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. The complete TU is now
ordinary C; its four trailing compiler-alignment bytes are trimmed and receive
no credit. Exactly 492 previously unmatched executable bytes are recovered.

The authenticated stock baseline is 492 bytes, 123 words, frame 0x80, one
relocation, and six raw/masked naming differences in the call-setup window.
The aligned buckets are 117 exact, six naming, zero immediate and zero
structural, without gaps. The existing exhaustive colour landscape was read;
no colour sweep or force was repeated.

Move the existing fifth-argument assignment from its own preceding statement
into the fifth argument expression. The carrier remains declared and assigned;
only its evaluation context changes. Stock output becomes 123 aligned exact
words, zero raw/masked differences, no gaps, and the same frame and extent.
The pre-promotion preflight authenticates the single relocation's offset,
type and effective overlay identity. The fallback comparator's generated
callee name differs from the friendly C name, which is not an identity failure.

The draw-census differential is localized: four draws leave the old setup
line and join the call line, which goes from two to six. All 39 draw-register
positions remain unchanged. The trace has 200 versus 199 emission records;
these counts include source-location directives and do not mean a machine
instruction was deleted. The aligned call window alone improves by six rows.
Full-TU stock/instrumented text, data, rodata, symbols and relocation fidelity
passes for baseline and both controls.

A cleanup control removes only the embedded assignment while retaining its
local declaration. It adds a fortieth draw, changes the ring sequence, and
reintroduces a displaced load plus naming differences across six windows.
The exact assignment spelling is restored and retained for cleanup review.
This refutes a source-level closure of the temporary pair: deleting the carrier
and moving its definition into an argument are different compiler inputs.

Semantic review of the source change: the assignment's value has the same
s32 type and value as the previously passed local. No other argument reads or
writes that local, and its address never escapes. The moved expression only
reads the point selector and the existing vertex base; there are no intervening
calls or stores. A u8 selector plus an s16 base fits s32 throughout its domain.
Call identity, argument values, loop bounds and output stores remain unchanged.

Evidence is retained privately under build/d1/overlay19BuildAdjacency, with
source snapshots, stock and traced objects, scores, draw profiles, per-window
maps, fidelity receipts, and preflight reports. Commands include draw_census.py,
residual_map.py --object/--against, align_symbol.py, wb_compare.sh,
function_preflight.py, and the ordinary overlay promotion sequence.

### Exhaustive landscape and retained improvement (2026-09-12, p23-lastmile5)

The fresh configured baseline is 123 words (492 bytes), frame `0x80`,
one candidate and one target ELF relocation, and 41 raw/masked positional
differences, first `+0x7C`. Its aligned split is 82 exact, 41 naming,
zero immediate and zero structural rows, without gaps.

`web_footprint.py --every-colour` on procedure zero sampled 17 coloured
webs: 110 rows, comprising 109 accepted same-save-kind alternative forces
at size delta zero and one singleton without another colour. The complete
winners list is `p1:w81=c7` at 39 words. The selected single-force lattice
reproduces 39; stock/instrumented section, symbol and relocation fidelity
passes. This is a diagnostic floor for the measured force set, not a lower
bound over all possible combinations or source shapes.

Five stock source probes crossed signed narrowing with naming the fifth call
argument. Naming alone scores 51; removing the narrowing alone scores 48;
combining both scores **36**. Removing the redundant mask scores 57, or 51
with the named argument. All retain size delta zero. The retained guarded
candidate uses the existing `edgeOffset` local for the fifth argument and
removes the signed narrowing before masking `frame.suppressed`. The preceding
`0x1080` mask already confines that value to the positive signed-halfword
range; this removal preserves its value. The argument expression has no call
or store, and naming it preserves the values passed to the sole call.

Fresh proof of the retained source gives 36 raw/masked differences, first
`+0x7C`, with the same 123 words, one relocation and `0x80` frame. Frame
census reports identical sixteen-slot home sets and no differing shared
slots. Aligned buckets are 96 exact, 25 naming, one immediate and zero paired
structural rows, plus one candidate-only word at `+0xD4` and one target-only
word at `+0x90`. This is a positional improvement with a moved-load gap, not
an entirely register-only residual or a match.

A second exhaustive scan of this actual retained shape sampled 18 coloured
webs: 115 rows, comprising 114 accepted alternative forces at size delta zero
and one singleton. Its complete winners list is empty; the measured
single-force floor is therefore 36. There are no improving forces to combine
in a winner lattice. Stock/instrumented fidelity passes again. Force requests
remain diagnostics and none enter the source.

Three closing stock probes split the two frame stores onto separate lines,
reversed their order, and made the output-offset home unsigned. All score 36
at size delta zero without a better residual or new identity. This is the
ADR 0018 stall evidence. The next source question is which temporary producer
and loop-head load lifetime yield the target ring phase without moving the
output-offset load; another isolated forced colour cannot answer it.

Evidence is preserved privately under `build/p23/overlay19BuildAdjacency/`,
including both landscapes, all source/object/result cells, the aligned map,
frame census and fidelity receipts. Commands: `residual_map.py`,
`web_footprint.py --every-colour`, `force_lattice.py`, configured stock source
probes, `frame_census.py`, `finalize_plateau.py`, and
`tools/gates.sh verify cleanroom check-docs`. The fallback remains canonical;
no executable bytes receive new matching credit.

### 2026-09-12, lane p24-recipe: two deletions and tail order, 36 to 6

The stock baseline reproduces 36 raw/masked differences at 123 words, delta
zero, frame 0x80 and one relocation. Its aligned buckets are 96 exact,
25 naming, one immediate, zero paired structural rows, with one candidate-only
word at +0xD4 and one target-only word at +0x90.

Three source changes are retained. The invalid-record pointer alias is removed
and the stores index output->records directly. The redundant 0x7FFF mask on
frame.suppressed is removed: the preceding 0x1080 mask already clears every
bit it would remove. Finally, the outer span index is advanced before its byte
offset, with both updates on one physical source line. These frame fields are
independent and neither update calls out or changes their later values.

The recipe's interaction is decisive. Deleting the pointer alone gives 53
pure naming differences with the original frame, geometry and relocation;
it removes the moved-load gap. Removing the redundant mask on that shape
gives 13 pure naming differences. Swapping the tail updates gives eight:
six naming and two store-order rows. Joining those two updates on one line
removes the store-order pair, reaching six. The original first two windows
and the tail are now exact; only the +0x100 window contains differences.

Fresh retained-object proof: 117 exact, six naming, zero immediate and zero
structural rows, no gaps, 492 bytes, delta zero, frame 0x80 on both sides,
one candidate and one target ELF relocation, and first mismatch +0x10C.
The six rows exchange two temporary-register pairs in the sole call's
argument preparation. This remains NON_MATCHING, not a promotion or new
executable-byte credit. The assembly fallback supplies canonical ROM bytes.

Each changed landscape was authenticated against stock. The 53-word shape
has 17 coloured webs, 102 accepted alternatives and one singleton with no
second colour; five single winners score
49, 49, 51, 52 and 52. No combined-force claim is made for that superseded
shape. The 13-word shape and final six-word shape each have 17 coloured webs
and 102 accepted alternatives plus one singleton, with no improving single force. The final
Ucode receipt maps procedure zero and passes text, data, rodata, symbols and
relocations fidelity. These are single-force negatives, not unrestricted
source or multi-force lower bounds.

Additional source evidence is retained, rather than silently discarded.
Deleting the span pointer gives 42 with a larger 0xA8 frame; deleting the
next-edge local costs four bytes and gives 88. On the deleted-record shape,
restoring signed narrowing is flat at 53 and generating the fifth argument
scores 56. On the improved shape, generating that argument instead of naming
it regresses to 61 and restores the load gap. Naming the current argument
instead also regresses; it changes earlier allocation instead of only swapping
the two requested temporary pairs.

The closing source probes establish no better residual or target identity.
Grouping call setup is flat at six. Byte-view selector access, with either
signed or unsigned displacement, costs eight bytes and scores 87. Naming the
current selector through the existing call-result local scores 86 at delta
zero. Reusing the dead next-edge index for that value costs four bytes and
scores 91. The final three measured selector forms neither improve the best
six-word candidate nor explain the required temporary assignment, meeting
ADR 0018. Prepared but uncompiled alternatives are not evidence.

The named source question is the evaluation identity/order of the current and
next selector address/load pairs: obtain the target temporary pair for each
argument while retaining the named fifth-argument web and the now-exact
loop-head and tail shapes. Another isolated globalcolor force does not do it.
Sources, objects, scores, mismatch locations and aligned per-window comparisons
are private under build/p24/overlay19BuildAdjacency. The final authenticated
landscape and receipt are under build/p24/adjacency-six-trace; the earlier
53- and 13-word receipts remain alongside them. No forced compiler output is
used by the canonical build.

<!-- plateau-handoff:overlay19BuildAdjacency:end -->
