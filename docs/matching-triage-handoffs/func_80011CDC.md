<!-- plateau-handoff:func_80011CDC:start -->
### `func_80011CDC` plateau handoff

- source: `src/main/track.c`
- score: 0/342 words, promoted
- frame: 0xc0
- relocations: 15
- first mismatch: none
- summary: Matched. Record y/z read in place (frame), struct-field material flag, z term first in two sums, one zero-cost loop block so the counter outranks the D_800792E8 address web.

Summary before this remeasure: Aligned 243 to 193. One p1 decision left: D_800792E8 address web outranks the record counter; forcing it split gives 342/342 words, 150 diff.

Summary before this remeasure: Mickey m2c reproduces existing edge/endpoint tests; no new structural identity. Next: source-proved texture-global and counter lifetimes.

#### Law (2026-10-07, lane c-track2): one zero-cost block crosses a save divisor

A web's save is totalsave / nocs with nocs = 1 + floor((blocks + 2) / 4)
over the blocks its range spans. When two webs tie on totalsave and
differ only because one spans a block or two more (here the record
counter, initialised in the entry block, against the byte offset and
three address webs that span only the loop), one `do { } while (0)`
around a statement inside the shared range adds a block to every
loop-spanning web at no instruction cost. If the shorter webs sit one
block below the next divisor, they cross it and the longer one does
not, so the tie is decided by web number instead of by span. Measured:
counter 31/8 against offset 31/7 and addresses 30/7 (328 at +20); with
the block, all at /8 and the counter (lower web number) wins: 0. Two
blocks overshoot (329). Read the ladder's nocs for the contested pair
before choosing where to put the block; a block outside the shared range
moves nothing. Same lever, 107 to 89, on func_80011980.

#### 2026-10-07, lane c-track2: matched (329 at +20 to 0)

- Counter initialised inside the `D_800C9D24 > 0` test: 329 at +20 to 124
  at 0. The counter's web then spans fewer blocks and outranks the
  D_800792E8 address, which is rematerialised at each use as shipped.
- Record direction y and z read from the record (`record->dy`,
  `record->dz`) instead of `normalY`/`normalZ` locals: frame 0xC8 to 0xC0.
  Each declared local takes a frame cell here; `node` removed instead is
  +132. 124 to 118.
- Material flag as a struct field (`((TrackTextureFlags *)
  D_800792E8->textures)[i].flag`, byte 7 of the 8-byte entry): base-first
  add. The four cast spellings of `((u8 *) &textures[i])[7]` all add index
  first.
- Sum of squares as `dz * dz + (dx * dx + dy * dy)` and each distance as
  `-(nz * pointZ + (pointX * nx + pointY * ny))`: the target adds the z
  term first in both. 120-cell product (material 5, sum 3, distance 4):
  floor 15 at 0, the counter-init prologue only.
- The 15: the target initialises the counter before the test (its zero
  fills the branch delay slot and its save is scheduled early). That shape
  measured 328 at +20 because the counter (31/8, 3.875) loses to the offset
  (31/7) and three address webs (30/7). Records (proc 43, .text identity
  gate passed): every loop-spanning web is one block short of the next
  save divisor. One `do { } while (0)` around the record computation (or
  around `edgeHit = 0`) puts them all at 8, the counter ties the offset at
  31/8 and wins on web number, and the D_800792E8 web gets no register:
  0 at 0. Two wrappers overshoot (329).

`gmake verify` passed with the GLOBAL_ASM branch removed.

#### Track B pass (lane B3-track), 2026-09-23

- Start: size delta +8, frame 0xD0 against 0xC0, 327 masked, aligned after shadow 243. Reader: a prologue hoist pair and one 318-word open pair labelled missing-CSE, owned by the texture-global lines, the three call sites (two stack stores each) and the plane/edge reload lines.
- Typed rewrite (records as TrackClipOutput, whose segment is the s16 the target reads, hit as TrackRayHit, node indices with named material and data fields at unchanged offsets; prototype and both in-TU call sites retyped, linked ROM unchanged): the target's two reloads of hit normals and its re-read segment index now agree, 349 words, 329.
- Foot point carried in the difference locals before the three normal divisions: register shape only, 331.
- Point sums written origin-first: puts the product first as in the target, 330.
- Dropping the three unused locals: frame 0xD8 to 0xC8 (t and tEnd land at the target's offsets from the frame top), 330.
- All three points computed before the hit stores in the two endpoint branches: t is address-taken, so an interleaved store forced a reload of t: 349 to 347 words, 329, aligned after shadow 193.
- Flat: subscripted record pointer, for loop, pre-increment compare, explicit byte-offset carrier, result/count initialisation order.
- Decision variable, from the instrumented records: the D_800792E8 address web (save 30/7) outranks the record counter (31/8) and the result flag (4.0), takes s8, and leaves the counter in a caller-saved register that is spilled and reloaded around each of the three calls, plus two hoist words. Forcing only that web to split (accepted, forced=-1) gives 342/342 words and 150 differing; adding the result flag in v1 gives 124. The counter still cannot reach s5 because the SR offset web (save 4.43) takes it first. In the target the counter outranks the offset and the three address webs, so its source gives the counter more references or fewer blocks than any form tried here. That is the next lever.

#### Mickey m2c structural audit, 2026-09-09

- The assignment gate returned `base-only` for the authorized Mickey-only reconstruction mechanism. A fresh configured full-TU baseline and workbench diagnosis retain 344 candidate versus 342 target words, 327 differing words, first +0x0, candidate frame 0xd0 versus target 0xc0, and 11 candidate versus 15 target relocation records with 2 exact offset/type/identity records. Verdict `structure-mismatch`, playbook `constant-audit`.
- The m2c draft reproduces the existing edge test followed by the two conditional endpoint tests, including the minimum-distance checks, normal/position output order, metadata lookup and texture-byte access. The record stride is 0x2c and the direction begins at record offset 0x18. Calls resolve to the existing `func_80012234` and `func_80012574` implementations. No missing branch, constant, call identity or field interpretation was exposed.
- The draft's extra copies of the return flag around calls describe spills in the generated program; they are not evidence for new source locals. The named plane differences have already been tested in the committed prior plateau. No allocation rewrite, repeated flag lattice, or permuter trial was substituted for the authorized mechanism.
- Stop evidence: zero new source attempts. The recovered data flow already exists in the guarded candidate; the remaining texture-global hoist, loop-counter spill and frame excess are the previously documented source-lifetime problem. Source and configured object are preserved with the fresh baseline under ignored `build/wb/tu-track/func_80011CDC/`.
- Next concrete lever: source-attributed lifetimes explaining the unhoisted texture global and loop counter, authenticated against Mickey's ABI, under a new allocation/source-lifetime authorization. Preserve the existing plane-difference evidence.
- Validation: configured full-TU `wb_compare.sh`, workbench `diagnose` and `guide constant-audit`, `tools/finalize_plateau.py`, and `gmake verify cleanroom check-docs`. The original `NON_MATCHING`/`GLOBAL_ASM` guard remains; zero matching bytes are claimed.

#### Pinned donor audit, 2026-09-08

- Assignment gate: `base-only` at `32a75d648e8954f7455897fb8f16a8ef5f05df11` for this exact symbol and source path.
- Jet Force Gemini `efd5abb1c79636e297b831f7c2d5bf47eac39c0c` has 12 C implementations and 53 assembly placeholders in `src/track.c`. Its implemented routines do not supply this target's body. The closest masked track-object hit, `func_800175A0` (Jaccard 0.0800), corresponds by verified TU order to the still-assembly `func_80017794_18394` in that pinned source. This is diagnostic structural context, not an adopted name, ABI identity, or exact donor match.
- Fresh configured full-TU measurement: 342 target words and 344 candidate words; 327 raw and 327 relocation-masked differences; first +0x0; target frame 0xc0, candidate frame 0xd0. There are 11 candidate versus 15 target relocation records, with 2 stable identities at matching offsets/types.
- Workbench verdict `structure-mismatch`, playbook `constant-audit`. The existing diagnosis still requires source-shape/lifetime evidence; the donor-only reopen does not authorize substituting a flag, allocator, or permuter mechanism.
- ADR 0018 stop: zero new source attempts for this target. The pinned source disproves the new matched-donor-C hypothesis. No unchanged flag lattice or previously exhausted source family was repeated. The compiled candidate is unchanged and remains behind `NON_MATCHING` and its original `GLOBAL_ASM` fallback; zero new matching bytes are credited.
- Next concrete lever: a published matched counterpart with an authenticated ABI and useful source lifetimes, followed by Mickey-specific field, branch and call proof. The remaining general levers below require separately authorized changed evidence.
- Commands: `tools/wb_compare.sh --summary-json func_80011CDC`, workbench `diagnose` on the configured full-TU object, the track-object masked skeleton audit, `tools/finalize_plateau.py`, `gmake cleanroom`, and `gmake check-docs`. Source, object, scores, first mismatch and previous handoff are retained under ignored `build/wb/`; no instruction rows are included here.

#### Prior committed plateau evidence (historical)


- source: `src/main/track.c`
- score: 15/342 words
- frame: 0xD0
- relocations: 11
- first mismatch: +0x0
- summary: Plane-difference locals improve 339 to 327 diffs and recover target saves; texture-global hoisting still spills the counter (344 vs 342 words, 11 vs 15 relocs).
#### 2026-10-03: isolated named input-vector contrast

The authorized packet on `a9ff430a183f5c61e6e838b8ae2d3bda32f86f15`
tested the existing `TrackRayPoint` input-member view suggested by the exact
collision-response sibling. Stock IDO layout assertions and the caller audit
establish twelve-byte vectors, four-byte alignment and member offsets zero,
four and eight. The input views preserve float width, qualifiers, call order
and arithmetic association; the existing helper prototypes remain unchanged
through explicit boundary casts.

The fresh configured full-TU baseline reproduces 347 candidate versus 342
target words, 329 differences, first `+0x0`, and frame `0xC8` versus `0xC0`.
Naming both inputs produces 330 differences at unchanged size and frame.
Direction-only is byte-identical to that both-input candidate; origin-only
is byte-identical to the baseline. Direction spelling changes 109 candidate
words starting at `+0x10C`, but no compiler-internal cause is attributed to
that observation. This eliminates the direct input-member transfer hypothesis
under the current body; it does not justify another lifetime or colour sweep.

All three contrasts preserve the other 65 full-TU functions' sizes, bytes
and relocation tuples, and preserve data and rodata. Every canonical object
retains all 66 functions unchanged because the target still uses its fallback.
Actual compiler-input self-context and section/symbol/relocation fidelity
pass for the baseline and every contrast. The intentional prototype and
caller-cast context changes are disclosed rather than labelled body-only.
Sources, objects, measurements and receipts remain private under ignored
`build/wb/track-input-members/`.

The original source and 329-word baseline are retained; no matching credit
is claimed. Stop after the three isolated contrasts because neither input
view improves the residual or frame. Further work requires newly authenticated
source or compiler evidence for the remaining lifetime/allocation deficit,
not the sibling analogy or these exhausted representations.

<!-- plateau-handoff:func_80011CDC:end -->
