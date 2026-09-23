<!-- plateau-handoff:func_8004B1DC:start -->
### `func_8004B1DC` plateau handoff

- source: `src/main/font.c`
- score: 451 differing words
- frame: 0x80
- relocations: 48
- first mismatch: +0x30
- summary: Unchanged body; the u8 parameter on func_8004C690 moved this caller from 465 to 451 at the same size delta -32

Summary before this remeasure: Half the 465 is a t6-t9 ring phase downstream of the eight-word size deficit; the +0x54 head is caller-saved naming and schedule, not structure.
- current assignment base: `8aafabd4945580f6ba72e3c7bb7744a6a063198e`
- current configured baseline and retained candidate: 2,192 candidate bytes / 548 words against 2,224 target bytes / 556 words; size delta is -32 bytes, with 466 raw and 465 relocation-masked differing words; both frames are `0x80`
- first mismatch distinction: direct object words first differ at `+0x30` (the early branch displacement); the workbench's aligned structural report begins at `+0x54`. The header now records the raw mismatch and the candidate's 48 relocations; the target has 42.
- authorized donor audit: Jet Force Gemini `src/font.c::func_8006FD98_70998` at `efd5abb1c79636e297b831f7c2d5bf47eac39c0c` is the same 153-line NON_EQUIVALENT function as `func_80070518` at reference `c82affffe8f11cb5b440cfa918f4582ad8573279`, exactly after five symbol renames. The renamed identifiers are the function, initial graphics table, and three noise-colour globals. Both TU revisions have eight GLOBAL_ASM fallbacks; there is no new donor body in this range.
- donor applicability: the shared caller ABI and four-step tail confirm the family, while Mickey retains its converted-buffer traversal, glyph-cache lookup, window-intersection scissor, and background extents. The donor's direct text traversal, atlas texture indexing, alternate scissor calculation, pre-loop text offsets, and minus-one fill extents cannot be imported over Mickey's contrary evidence. Its texture/clipping and GBI spellings supply no changed inputs to the prior exhausted forms.
- workbench routing: `wb_compare.sh` reports `structure-mismatch`; `structure-buckets` levers 1 (constant audit), 4 (full-TU context parity), and 24 (conditional audit) were reviewed. `diagnose` reports a mixed residual and directs constant audit before structure and register classes; context lint with NON_MATCHING defined reports zero findings. The configured full TU remains `-O2 -mips2 -32`; no flag, header, declaration, or allocator search was reopened.
- stopping evidence: zero material C attempts. Baseline compilation and donor-history inspection are not attempts. The assignment's early-exhaustion rule applies: the sole authorized mechanism requires new donor evidence, and this body contains none. The prior ten-attempt plateau remains binding; its source and objects are preserved instead of repeating known-flat forms.
- next concrete lever: obtain an actually changed donor setup/glyph-command body or an explicitly authorized Mickey-only setup address-carrier investigation. Start at the structural `+0x54` setup residual with structure-buckets context parity, then remeasure the custom glyph-command sequence; do not reopen this target merely because a donor symbol was renamed.
- local evidence: baseline source/object/summary, donor-history equality receipt, direct geometry and relocation census, and validation logs are retained under ignored `build/wb/d-font-audit/`; workbench target and diagnosis remain under ignored `build/wb/`.
- current authenticated relocation evidence: after building this lane's canonical ELF, the workbench summary resolves all 48 candidate identities, with 13 offset/type alignments and 12 exact relocation identities against 42 target records. The prior runtime-identity blocker is not reproduced by the current tooling; this remains a nonexact static surface.
- validation: `gmake verify` reproduces the expected US ROM SHA1 with the guarded assembly fallback; the rebuilt NON_MATCHING full-TU candidate retains the baseline score. `gmake check-nonmatching-builds` passes all 243 candidate-bearing TUs. `finalize_plateau.py` preserves the guard and records the measured plateau.
- gate repair: the first finalization exposed pre-existing punctuation drift between the func_8000FAE0 source marker and its documentation summary. Only the extra terminal period in that separate documentation shard was removed; its measurements and source were untouched.
- commands: `tools/wb_compare.sh func_8004B1DC`; `tools/wb_compare.sh --summary-json func_8004B1DC`; workbench `diagnose` on `build/wb/func_8004B1DC.target.o` and `build_non_matching/src/main/font.c.o`; `guide structure-buckets`; `context lint src/main/font.c --define NON_MATCHING=1`; donor `git diff c82affff..efd5abb -- src/font.c` and exact body comparison after renames; `tools/reloc_surface.py compare func_8004B1DC --candidate-object build_non_matching/src/main/font.c.o --json`; `gmake verify`; `gmake check-nonmatching-builds`; `gmake cleanroom check-docs`; `tools/finalize_plateau.py`; and worker checkpoints with best-artifact archival.
- previous campaign evidence (retained verbatim below; historical measurements and first-mismatch wording are distinguished from the current header):
- assignment base: `8b29adc5`
- ownership: resident ROM `0x4BDDC..0x4C68C`, VRAM `0x8004B1DC..0x8004BA8C`; 2,224 bytes / 556 words, with no target padding before `func_8004BA8C`
- ABI/callers: `void (Gfx **, DialogueBoxBackground *, char *, s32)`; direct C callers are `func_8004B13C` and `func_8004C200`
- configured build: full `src/main/font.c` TU under `-O2 -mips2 -32`; the earlier 119-row flag lattice already retained this row as best
- baseline: 528/556 words, 509 positional/raw differences, exact `0x80` frame, first structural divergence `+0x54`, and 48 candidate versus 42 target relocations
- retained result: 548/556 words, 465 positional differences (466 raw), exact `0x80` frame, 348 opcode mismatches, 190 alignment gaps, and 67 relocation-symbol mismatches
- relocation evidence: the target has 42 text relocations and the candidate has 48; configured preflight and checkpoint-summary generation fail closed because the target's `D_800D64F2` HI16/LO16 pair lacks a stable runtime identity, although the linked map fixes that symbol at `0x800D64F2`
- donor evidence: the bounded five-reference skeleton oracle ranks JFG assembly-backed `func_80070518` first at 0.3231 (2,364 bytes); DKR `render_text_string` is second at 0.0642 (2,044 bytes) and remains the disclosed source-organization starting point, not byte authority
- attempts: ten coherent forms covered the missing glyph bound, standalone window-zero y2 identity, current-text load timing, active-font address lifetime, GBI colour/sync macros, scissor/fill isolation, the texture macro, the glyph-bound combination, and block-scoped custom-command pointers
- rejected plateaus: the most structurally compact alternate reached 554/556 words and 60 relocation mismatches but regressed to 474 positional differences; the target-supported glyph guard plus that texture form overshot by one word and regressed to 478
- tooling: no permuter batch was run because the retained residual remains broadly structural rather than a close allocator/scheduler finish; every attempt and object is retained under ignored `build/wb/` state
- next action: preserve the macro-authentic colour/sync/fill spelling and manual scissor/texture spelling, then explain the `+0x54` setup address-carrier web before revisiting the custom glyph-command pointer chronology

## 2026-09-11 (lane `lane/w3-mid`): half the residual is downstream of the size deficit

Unchanged at 465 words. Aligned: 182 byte-exact, 232 register naming, 18
immediate only, 136 really different; size delta -32 bytes; displacement tax 79.
First naming difference +0x54, first immediate-only +0x30, first structural
+0x60.

`tools/frame_census.py` reports frame 0x80 and fifteen stack slots on both
sides. The ladders agree on the top two entries and from +0x3C downward; three
homes sit at +0x50, +0x64 and +0x68 here and at +0x60, +0x6C and +0x70 in the
target. Same count, same frame: the difference is how much unhomed space sits
between +0x3C and the first home above it, 0x20 bytes in the target against
0x10 here, which by L121 is a count of register-class scalars rather than a gap.

The 232-word naming bucket is not an independent problem and should not be
worked as one. The register census shows 234 substitution instances confined to
t6, t7, t8 and t9, with the phase differing between regions because the stream
is eight words short and displaced at seventeen separate sites. That ring will
follow the structure.

The size deficit is eight words net: twelve surplus candidate words in eight
groups, and twenty missing target words in seventeen groups. The largest single
sites are three words missing at target +0x710 and two each at +0x6F4 and
+0x1DC-adjacent positions; the rest are one word apiece.

The prior handoff routes the head at +0x54 through structure-buckets. Under
alignment that is the wrong bucket: +0x54 to +0x98 is register naming and
schedule with the same instruction multiset on both sides, the candidate holding
a setup address and the reloaded output pointer in two temporaries where the
target holds them in two argument registers. The first genuinely structural site
is +0x60, and the first missing target word is at +0x74. A lane picking this up
should start from the size deficit and the caller-saved allocation of the setup
block, and should not open a register-class investigation until the eight words
close.
<!-- plateau-handoff:func_8004B1DC:end -->
