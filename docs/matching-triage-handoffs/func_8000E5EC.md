<!-- plateau-handoff:func_8000E5EC:start -->
### `func_8000E5EC` plateau handoff

- source: `src/main/track.c`
- score: 12/205 words
- frame: 0xD8
- relocations: 56
- first mismatch: +0x104
- summary: Listing rewrite 185 -> 12; left two v0/v1 ties: forces p1:w61=c2,w52=c1,w4=c2,w17=c1 (proc 23) score 0.

#### 2026-10-02, lane x-track, second pass: one force left (still 12)

Single forces on the banked source (proc 23): `p1:w61=c2` alone scores 3
and `p1:w4=c2` alone 9; together they score 0, so webs 52 and 17 follow.
Pair B (the camera block's segment-count load taking v0) closes from
source: keeping the `camGetPtr()` result live through the camera test
(`camera = camGetPtr(); i = camera->segmentIndex;` and a trailing empty
`if (camera) {}` after `i *= ...`) scores 9 with only pair A left, and
`p1:w66=c2` on that source scores 0. The empty conditional is
diagnostic, as in func_8003A754, so it is not banked; a natural statement
that holds the call result over those three blocks would be. Other camera
spellings (field re-read at each use, `camera` held but not re-used, the
assignment inside the condition) score 12 to 14.

Pair A is a near tie: the list cursor (62/2) beats the shared
post-decrement temporary (60/2), and the target needs the temporary first
while the cursor still beats the then-loop's byte load (30/1) and `j`
(64/3). That ordering needs the cursor at exactly 30 (a tie at 30 goes to
the temporary, the lower web number) or the temporary at 31. Wrapping
either flag loop's body in `do { } while (0)` or `if (1) { }` raises the
cursor's block count to nocs 3 (20.7), which puts it behind the byte load
and `j` as well (16 to 32 words).

#### 2026-10-02, lane x-track: 185 to 12 at delta 0

The m2c body was replaced by a rewrite from the listing. Direct-compile
measurements (masked words, size delta):

- Natural rewrite (indexed segmentList, `while (j--)` flag loops, one
  counter): 166 at -4. Separate counter `j` for the two flag loops (the
  final loop's counter spans calls, theirs do not): 160 at -12.
- Camera block: the camera segment held in the final loop's counter `i`
  (the target keeps it, then the product, in s1) and scaled in place,
  `i *= D_800792E8->segmentCount`, with the count read in the condition
  (a 10-cell product over carrier and spelling): 43 at 0.
- Declarations for the 0xD8 homes (visibleCount third, resultCount fourth,
  one more slot before `segmentList[128]`, `records` after it): 12 at 0.
- Flat at 12: three `while` spellings per loop (`--j >= 0`, `j-- != 0`,
  `j-- > 0` all regress), index-operand order, `u32` counter, explicit `!= 0`.

The 12 words are v0/v1 only. Instrumented records on this source
(proc 23): forcing `p1:w61=c2,p1:w52=c1,p1:w4=c2,p1:w17=c1` scores 0 masked
at delta 0 (all four accepted). Web 52 is the post-decrement temporary that
all three `while (x--)` loops share (bbs 23, 28, 46; totalsave 60, nocs 2);
web 61 is the segment-list cursor of both flag loops (62/2) and is
coloured first, so the cursor takes v0. Web 4 is the camera block's
segment-count load (3/2) and takes v0 ahead of web 17 (3/3). The next lever
is a source form that moves two units of `totalsave` between the cursor
and the shared temp (a tie at 60 goes to web 52, the lower number).


Summary before this remeasure: Scoped-carrier forms are inert; direct visibility access changes the 31-draw schedule but regresses to 187 words. The 0x10 frame deficit remains.

#### Mickey m2c structural audit, 2026-09-08

- Fresh assignment gate returned `base-only`. Baseline: 209 candidate / 205 target words, 185 raw/masked differences, first +0x0, frame 0xe8 / 0xd8, 56/56 relocations with 9 exact offset/type/identity sites. Workbench verdict `structure-mismatch`, playbook `constant-audit`; the first differing immediate is frame extent. Visibility flags, buffer stride, module constant and dispatch modes agree with Mickey.
- The fresh Mickey m2c draft recovers the existing list-generation, visibility and rendering call identities. All reverse loops were checked against branch-delay semantics: m2c's printed post-decrement normalization must not replace the current count-correct C literally. The typed track pointer also has a target reload after the list-generation calls, which the raw draft elides.
- Five new structural forms test that explicit track reload/reuse, short-circuited segment-count load, typed record/pointer-list buffers, captured segment byte before nested dispatch, and separate draw countdown. Measurements: 40: 208 words, 187 differences, frame 0xe8; 41: 210 words, 192 differences, frame 0xe8; 42: 210 words, 192 differences, frame 0xe8; 43: 213 words, 210 differences, frame 0xf0; 44: 213 words, 210 differences, frame 0xf8. None improves the retained 185-word baseline or provides an additional unresolved identity. The pointer/call and CFG reconstruction space has been exhausted for this draft; the prior flag and lexical-policy plateau remains closed.
- Retained best is the unchanged guarded baseline with its original assembly fallback. All five candidates and complete full-TU object/score artifacts are retained under ignored `build/wb/tu-track/func_8000E5EC/`. Zero new matching bytes.
- Next concrete lever: source-attributed compiler home/lifetime evidence for the 128-byte list and call-crossing locals. The historical compiler trace lacks stack homes and attributable webs; neither that trace nor this m2c draft justifies an allocator experiment.
- Validation: full-TU comparisons after each attempt, finalizer, `gmake verify`, `gmake cleanroom`, and `gmake check-docs` before the plateau commit.

#### Pinned donor audit, 2026-09-08

- Assignment gate: `base-only` at `32a75d648e8954f7455897fb8f16a8ef5f05df11` for this exact symbol and source path.
- Jet Force Gemini `efd5abb1c79636e297b831f7c2d5bf47eac39c0c` has 12 C implementations and 53 assembly placeholders in `src/track.c`. Its implemented routines do not supply this target's body. The closest masked track-object hit, `trackUpdateLighting` (Jaccard 0.0479), corresponds by verified TU order to the still-assembly `trackUpdateLighting` in that pinned source. This is diagnostic structural context, not an adopted name, ABI identity, or exact donor match.
- Fresh configured full-TU measurement: 205 target words and 209 candidate words; 185 raw and 185 relocation-masked differences; first +0x0; target frame 0xd8, candidate frame 0xe8. There are 56 candidate versus 56 target relocation records, with 9 stable identities at matching offsets/types.
- Workbench verdict `structure-mismatch`, playbook `constant-audit`. The existing diagnosis still requires source-shape/lifetime evidence; the donor-only reopen does not authorize substituting a flag, allocator, or permuter mechanism.
- ADR 0018 stop: zero new source attempts for this target. The pinned source disproves the new matched-donor-C hypothesis. No unchanged flag lattice or previously exhausted source family was repeated. The compiled candidate is unchanged and remains behind `NON_MATCHING` and its original `GLOBAL_ASM` fallback; zero new matching bytes are credited.
- Next concrete lever: a published matched counterpart with an authenticated ABI and useful source lifetimes, followed by Mickey-specific field, branch and call proof. The remaining general levers below require separately authorized changed evidence.
- Commands: `tools/wb_compare.sh --summary-json func_8000E5EC`, workbench `diagnose` on the configured full-TU object, the track-object masked skeleton audit, `tools/finalize_plateau.py`, `gmake cleanroom`, and `gmake check-docs`. Source, object, scores, first mismatch and previous handoff are retained under ignored `build/wb/`; no instruction rows are included here.

#### Prior committed plateau evidence (historical)


- source: `src/main/track.c`
- score: 185 differing words
- frame: 0xE8
- relocations: 56
- first mismatch: +0x0
- summary: 119 flags flat; fidelity-clean proc 23 has 34 integer decisions but no stack homes or source-attributed webs, so no lexical experiment is justified
<!-- plateau-handoff:func_8000E5EC:end -->
