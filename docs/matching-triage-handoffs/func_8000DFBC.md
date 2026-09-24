<!-- plateau-handoff:func_8000DFBC:start -->
### `func_8000DFBC` plateau handoff

- source: `src/main/track.c`
- score: 303 differing words
- frame: 0x70
- relocations: 51
- first mismatch: +0x48
- summary: Declaring batchIndex and groupIndex first improves 304 to 303. Size stays +8, frame 0x70, first +0x48. Stall: the spill/reload pair on the tail-call lines remains after that declaration reorder.

Summary before this remeasure: Correct next-batch vertex boundary and unsigned command types; five m2c structural follow-ups fail to improve. Next: batch/display-list lifetimes.

#### Mickey m2c reconstruction, 2026-09-09

- A fresh `base-only` gate and configured full-TU measurement established 398 candidate versus 396 target words, 304 differing words, first +0x48, exact 0x70 frame, and 51 candidate/target relocations with 35 exact offset/type/identity records. Workbench verdict `structure-mismatch`, playbook `constant-audit`.
- Mickey's m2c draft exposes a concrete field error: the vertex count ends at the next batch's signed `u0`, not the current batch's unsigned animation `frame`. The target reads the two adjacent vertex boundaries; sibling `func_8000F198` independently corroborates this layout. Attempt 84 corrects that identity. Attempt 85 retains it and uses the existing texture-header type plus defined unsigned packed-colour, address and batch-mask arithmetic. Both retain the measured baseline residual; the incorrect prior field is not a valid alternative.
- Attempts 86–90 tested the recovered per-command display-list publication, positive entry guard, common batch increment, split entry/loop latch and the SDK command macros used by the sibling renderer. Cursor/macro forms produce 380 words and 371 differences; the common increment produces 395/371; the positive guard stays 398/304; split entry/latch gives 403/379 and two excess relocation records. Each trial is preserved with full source, object, first mismatch and rationale under ignored `build/wb/tu-track/func_8000DFBC/`.
- Stop evidence: five structural follow-ups do not improve the corrected candidate. They exhaust the newly recovered m2c control/write-order alternatives; no missing call or field identity remains to justify another structural rewrite. The retained attempt 85 is guarded and nonexact: 398/396 words, 304 differences, first +0x48, frame 0x70, relocations 51/51 with 35 identities aligned. Zero matching bytes are claimed.
- Next concrete lever: source-attributed batch/display-list lifetimes explaining the two-word excess, with separately authorized allocation work. Preserve the next-batch boundary correction and unsigned command arithmetic. The existing overlay call aliases are not rebound merely to improve a relocation score.
- Validation: `wb_compare.sh`, workbench `diagnose` and `guide constant-audit`, own-assembly field audit, retained-object comparison, `gmake check-nonmatching-builds`, `tools/finalize_plateau.py`, and `gmake verify cleanroom check-docs`.

#### Pinned donor audit, 2026-09-08

- Assignment gate: `base-only` at `32a75d648e8954f7455897fb8f16a8ef5f05df11` for this exact symbol and source path.
- Jet Force Gemini `efd5abb1c79636e297b831f7c2d5bf47eac39c0c` has 12 C implementations and 53 assembly placeholders in `src/track.c`. Its implemented routines do not supply this target's body. The closest masked track-object hit, `func_80014B6C` (Jaccard 0.1044), corresponds by verified TU order to the still-assembly `func_80014EBC_15ABC` in that pinned source. This is diagnostic structural context, not an adopted name, ABI identity, or exact donor match.
- Fresh configured full-TU measurement: 396 target words and 398 candidate words; 304 raw and 304 relocation-masked differences; first +0x48; target frame 0x70, candidate frame 0x70. There are 51 candidate versus 51 target relocation records, with 35 stable identities at matching offsets/types.
- Workbench verdict `structure-mismatch`, playbook `constant-audit`. The existing diagnosis still requires source-shape/lifetime evidence; the donor-only reopen does not authorize substituting a flag, allocator, or permuter mechanism.
- ADR 0018 stop: zero new source attempts for this target. The pinned source disproves the new matched-donor-C hypothesis. No unchanged flag lattice or previously exhausted source family was repeated. The compiled candidate is unchanged and remains behind `NON_MATCHING` and its original `GLOBAL_ASM` fallback; zero new matching bytes are credited.
- Next concrete lever: a published matched counterpart with an authenticated ABI and useful source lifetimes, followed by Mickey-specific field, branch and call proof. The remaining general levers below require separately authorized changed evidence.
- Commands: `tools/wb_compare.sh --summary-json func_8000DFBC`, workbench `diagnose` on the configured full-TU object, the track-object masked skeleton audit, `tools/finalize_plateau.py`, `gmake cleanroom`, and `gmake check-docs`. Source, object, scores, first mismatch and previous handoff are retained under ignored `build/wb/`; no instruction rows are included here.

#### Prior committed plateau evidence (historical)


- source: `src/main/track.c`
- score: 304 differing words
- frame: 0x70
- relocations: 51
- first mismatch: +0x48
- summary: Exact frame and relocation count; 2-word structural drift remains, and flag sweep lacks unique resident ownership metadata.
<!-- plateau-handoff:func_8000DFBC:end -->
