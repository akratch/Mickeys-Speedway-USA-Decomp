<!-- plateau-handoff:fxScreenEffect:start -->
### `fxScreenEffect` plateau handoff

- source: `src/main/fx.c`
- score: 0/147 words, promoted
- frame: 0x30
- relocations: 10
- first mismatch: none
- summary: Matched. Rewritten in the shape of the matched sibling func_80036F08: packet macros on (*dList)++, gSPTextureRectangle, coordinates rescaled in place with arg5 as the row cursor, s taken from arg4 before the rescale, offset plus base, case 3 before case 2, DMA w1 before w0.


Reopening audit (2026-09-08), evidence D: PROVENANCE inspection of Jet Force
Gemini public decomp `src/fx.c` and `src/fx.h` at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no new target C body.
No donor source, names or values were adopted. Mickey remains authoritative.

Configured full-TU measurement: candidate 560 bytes / 140 words,
target 588 bytes / 147 words; size delta -28 bytes.
Raw and relocation-masked differences are 123 and 123, respectively,
first mismatch +0x8; candidate/target frames are
0x30/0x30. Candidate/target static relocation counts are
10/10; 7 tuples agree in function-relative offset, type and symbol.
These are fallback-object comparisons, not linked-C promotion evidence.

Workbench comparison: `structure-mismatch`. Diagnosis: `mixed(structural:131, register:25)`;
playbook `structure-buckets`, lever `none-known`. The named guides were read.
This heuristic routing supplies no new donor evidence to reopen exhausted forms.
Next concrete lever requires new screen-effect donor C showing a source-authentic stack-argument preload,
then a fresh gate and configured full-TU comparison.

Stopping evidence: zero source attempts; the authorized donor mechanism supplies
no new implementation. This is the assignment's early-exhaustion stop, not a
five-attempt stall. The prior TU lattice is not repeated: since its recorded
`func_8004ACC4` audit at `4be95a3d`, this TU differs only in EOF handoff comments.
See [the anchor donor audit](func_80049E4C.md) for the full delta and batch disposition.

Ignored evidence is retained in `build/tu-fx-audit/` and `build/wb/`: source and
full-TU object, gate receipts, summaries, diagnoses and relocation tuples.
Commands: `lane_status.py --symbol`, `wb_compare.sh --summary-json`, workbench
`diagnose` and `guide`, and `finalize_plateau.py`. The unchanged guarded C stays
NON_MATCHING and receives zero new exact bytes.

Prior plateau evidence retained (historical measurements and exhausted forms):

- relocation identity: Candidate and target each have 10 static relocations; 7 offsets, types, and identities align in fallback-static evidence.
- flag lattice: All 119 combinations were nonexact. The `-O2 -g3 -mips2` diagnostic was 141 words with 131 differing words and first mismatch `+0x4`; its remaining ABI/structure mismatch does not justify a translation-unit flag change.
- donor result: Mickey-only `m2c` reproduced the stale candidate; JFG provides assembly-only structural context, and the similarity scan found no credible exact C donor.

Matched 2026-10-02 (lane x-fx). The closures above ("no colour-only
route", "needs donor C showing a stack-argument preload") and the in-source
axis log (some 350 spellings measured flat) all varied spellings inside the
inherited m2c shape. Rewritten in the shape of the matched sibling
func_80036F08 (opcode similarity 0.56, coordinator lead) it was 116 at
delta -16 in one compile; then, as one product: the texture s held in a
local taken from arg4 before the in-place rescale (delta -16 -> -4, this is
the target's unfolded shift pair), arg5 itself as the row cursor (112 -> 7,
delta 0), offset plus base (7), case 3 written before case 2 and the DMA
command's w1 before its w0 (7 -> 0).

<!-- plateau-handoff:fxScreenEffect:end -->
