<!-- plateau-handoff:overlay2ChooseBoundary:start -->
### `overlay2ChooseBoundary` plateau handoff

- source: `src/overlays/o002/overlay_002.c`
- score: 0 differing words
- frame: 0x90
- relocations: 64
- first mismatch: none
- summary: Resolved Tier A: 1168 ROM-exact C bytes, 292 exact words and 64 exact relocation identities from countdown and source-line scheduling.
- first mismatch: +0x5C
- summary: draw census did not expose a direct-count schedule lever; extra sltu and naming residual remain
- baseline: 45/292 words, 0x98 frame, 62 relocations, 247 masked differences
- attempts: the complete 119-combination flag lattice did not beat canonical O2/mips2; coherent direct-branch and carrier-width forms regressed
- bounded permutation: a five-minute, gain-gated batch improved its internal score only by inserting an inert empty guard, so the result was rejected and not applied
- blocker: the candidate emits an extra `sltu` at +0x5c; removing it with natural direct-branch spellings disrupts later allocation and function geometry
- next lever: reproduce the target's direct count branch while retaining the one-carrier 0x90 frame and current stack-slot order; then resolve the remaining relocation identities
- validation: `tools/wb_compare.sh --summary-json overlay2ChooseBoundary`; `gmake verify`; `gmake cleanroom`; `gmake check-docs`; `gmake check-scoreboard`
#### 2026-09-11, lane f10-mid: the closure held the declaration list fixed, and the list was wrong

Masked 106 to 97 at delta 0, aligner byte-exact 209 to 225, immediate-only 33
to 17, naming 44 unchanged, structural 7 unchanged. `gmake verify` passed with
the guarded assembly still linked; no bytes are credited.

The retained closure named the extra `sltu` at +0x5C as the blocker and kept
"the current stack-slot order". The aligner said otherwise: 33 of the 106
words were immediate-only, and a stack-home census of both objects showed the
candidate resident at 84, 108 and 120 where the target is resident at 92, 96
and 104. `side1` is address-taken, so its home is pinned at 112 in the
candidate and observed at 116 in the target; reading the roles of the other
slots off the objects gives the target ladder as side1 116, side2 112,
bestScore 104, axis 96, endpoint 92 against the candidate's endpoint 120,
axis 116, side1 112, side2 108, bestScore 84. That is a pure [L99] readout:
one fewer local declared above side1, and axis and endpoint declared below
bestScore. Four declaration orders satisfying it all reach every home and all
score 97; the slots between (120, 108, 100) are register-class and inert.

The function contains calls, so it is p1-only and declaration order decides
no colour here; it decides homes, and that is what it was never used for.
What remains is the +0x5C `sltu` (structural, 7 rows) and 44 naming rows,
whose decision variable is the p1 save ratio ([L100]); the p1 records have
not been read for this function.

#### 2026-09-11, lane p5-ovl: p1 trace and ring probe

The fresh baseline is 97/292 relocation-masked words at unchanged 0x90 frame
and 64 relocations: 225 exact, 44 register naming, 17 immediate-only, and 7
structural. Instrumented text is stock-identical; the trace has 51 p1
decisions and no p2. Register substitutions are 95% coherent and form one
closed t-register ring, so the cycle is not an incoherent mapping. The target's
first structural row uses a direct decrement/branch shape; an explicit
pre-body decrement changed geometry and regressed to 260/292. A self-assigned
inner-loop pointer, tested as a zero-byte ring-phase probe, was text-identical
and remained 97/292. Both probes were reverted; no source improvement was
proved.


#### g1 countdown and draw-schedule receipt (2026-09-13)

The configured baseline has 97 masked differences at 292 words: 225 aligned
exact, 44 naming, 17 immediate and five structural paired rows, plus one word
unique to each side. Procedure 0 has 84 draws and 437 emission events; stock
and instrumented complete-TU text agree. The old structural count of seven
included the two unpaired words. The near-match oracle found only weak
candidates, strongest at 0.059; no reference source was incorporated. No flag,
permuter or colour sweep was repeated.

Attempts 66–67 separately rewrote the outer and inner loops as while loops;
both preserved the draw sequence and regressed geometry. The positive signed
predicate in attempt 68 was byte-flat. Attempt 69 added a census receipt for
the already-known separated-decrement route: one additional total draw, two
extra words, and much worse alignment. Attempt 70 shifted the initial countdown
while retaining the original count snapshot; no benefit. Attempt 71 delayed
range-result initialization and gained one aligned row while worsening the
positional result and structural residual; it remains an ignored diagnostic.
Attempt 72's unsigned truth projection added one draw on the guard line and
worsened alignment by 70 paired rows.

Attempt 73 removed the explicit zero comparison from the outer postdecrement
condition. Its per-line counts and draw sequence were unchanged, but the
surplus outer word disappeared. One inner word was still missing and the result
was four bytes short. Attempt 74's direct inner memory truth was byte-flat.
Attempt 75 instead put the existing inner countdown snapshot and decrement
in the inner guard. It restored the missing word and exact geometry, reaching
45 differences: 247 aligned exact, 41 naming and four structural, with no
immediate or unpaired rows. This preserves the loaded counts and countdown
iteration counts without introducing a new guard.

Attempt 76 grouped zero initializers and was byte-flat. Attempt 77 split the
line-array base from its index before the coordinate branches; it added five
draws and regressed. Attempt 78 moved the four independent zero initializers
to the coordinate-branch join immediately before classification. This moved
the base load ahead of those initializers, closing the first window and
reaching 40 differences: 252 aligned exact, 38 naming and two structural.
Attempt 79 expressed the final selection loop's existing snapshot/decrement
as a direct guard too. The final count load gained one draw, changing the
sequence and removing the tail's closed register rotation: three rows remain,
289 aligned exact, one naming and two structural. The first three windows
and full frame remain exact.

Attempts 80–81's pointer-addition spellings were flat. Attempt 82 separated
base and offset through the existing candidate pointer; it removed one draw
and brought back a 36-row residual. Attempt 83 generated an explicit byte
stride instead: the base and scale received the target temporaries, leaving
two emission-order rows. Attempt 84 joined that address definition and its
following do-loop opener on one physical source line. It reaches all 292
aligned words with zero masked differences at exactly 1168 bytes and frame
0x90. The final census has 85 draws; no compiler force or instruction editing
is involved. The initial fallback-static workbench receipt resolves 51 of 64
relocation identities, so masked zero alone is not a matching claim.

All intermediate sources, objects, score maps, census JSON and logs remain
ignored under `build/g1/overlay2ChooseBoundary/`. The source-level result is
preserved with `tools/finalize_plateau.py` before canonical promotion and linked
proof. Final acceptance is recorded below.

#### Canonical closure

**Resolved, Tier A:** overlay 2 text `[+0x6E0, +0xB70)`, ROM
`[0x18574D8, 0x1857968)`, contributes 1168 executable C bytes and 292 words.
The configured frame is 0x90. `promotion_proof.py` reports 64 configured and
64 exact relocation identities, with `static-plus-runtime-table-and-linked-rom`
evidence and verdict `exact`. `tools/gates.sh verify` rebuilds the full ROM to
SHA-1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. No padding or already-matched
island receives new credit, and no post-compile instruction editing is used.

Attempt 85 binds the two existing validation calls to the canonical
`overlay2ValidateRegion` symbol and reproduces zero masked differences.
The source is now unconditional C. The atlas, donor digest, generated aliases,
ranking retirement and scoreboard are regenerated from the tree. Removing the
fallback preserves the function's physical line count. The initial promotion
receipt ran while the verification rebuild was finishing and refused stale
ELF state; the sequential rerun on the completed build passes. Attempt 86
expands the grouped counter resets into ordinary separate statements while
removing unused blank lines earlier in the function; configured text remains
identical. The final cursor definition and do opener stay on one physical
line because their separation is the measured two-row emission blocker.

Final commands: `gmake overlay-atlas-write`, `refresh_atlas_digest.py`,
`gmake extract`, `gmake overlay-syms`, configured build, second alias generation,
`tools/gates.sh verify`, `promotion_proof.py overlay2ChooseBoundary --json`,
`nm_ranking.py --prune-stale`, `nm_ranking.py --write-doc`, `gmake scoreboard`,
and `tools/gates.sh --promotion`. The guarded masked-zero handoff was first
preserved through `finalize_plateau.py`; it was then superseded by this proof.
The next action is coordinator integration and reproduction of these gates.

<!-- plateau-handoff:overlay2ChooseBoundary:end -->
