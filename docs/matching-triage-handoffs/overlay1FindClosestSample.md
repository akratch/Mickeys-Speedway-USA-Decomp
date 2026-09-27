<!-- plateau-handoff:overlay1FindClosestSample:start -->
### `overlay1FindClosestSample` plateau handoff

- source: `src/overlays/o001/overlay_001.c`
- score: 0 differing words
- frame: 0x60
- relocations: 7
- first mismatch: none
- summary: MATCHED; exact configured C, five independent call identities and two source-literal relocation witnesses.

## 2026-09-27: promote a previously hidden shared-guard candidate

The repaired queue enumerator exposed this 324-byte body separately from
its shared guard's first function. The explicit committed-base assignment
returned base-only with no prior target plateau or active target owner.
Initial configured measurement was 81 words at the exact size and frame,
zero masked differences and one raw relocation-field difference. That score
alone was not accepted as a match.

Mickey's runtime records establish seven relocation sites, although the
extracted fallback assembly carries only five static call records. The calls
resolve independently to the already matched previous/next ring-pointer
helpers and resident splinePos. The latter uses the authenticated float ABI;
integer pointer-punning arguments are unnecessary. The next-pointer reference
retains the same runtime-symbol alias used by a separately matched sibling,
because its shipped call addend is zero despite the helper's nonzero offset.

The initial distance is the maximum finite single-precision value in the
owned readonly constant input. C now emits that literal directly. A fresh raw
configured compile produces its exact four bytes, and the declared metadata
binding places them at the runtime record's readonly section and addend.
The new literal has its own alias: equal numeric placeholder values do not
make it the existing phase-scale literal's storage. The assembler emits a
range warning for this endpoint literal; its emitted finite value is checked
explicitly against the owned constant bytes.

The raw object's 324 instruction bytes are unchanged by postprocessing.
All five calls have static identities. The remaining HI16/LO16 pair has a
separate source-literal value, raw-section offset, declared binding and
runtime-section witness; linked proof reports seven effective identities.
No instruction bytes are patched. The existing digest-guarded readonly
externalization is updated for the new compiler-emitted literal.

The guard is split manually so overlay1ActivateObject retains its original
fallback. Its fresh full-TU residual remains three masked words; the other
unmatched sibling remains twelve. No sibling is promoted. Whole-ROM proof
protects the neighbouring ranges and the original readonly data.

Changed files: this C function and its declarations/guard boundary, its
metadata-only recipe, atlas ownership, generated aliases and derived ranking,
donor digest and scoreboard. Source, raw objects, baseline, literal witness,
preflight and gate receipts remain in ignored build/phase100-closest.

Validation: assignment gate; configured baseline/preflight; raw stock compile;
raw-versus-configured instruction equality; independent call and literal
witnesses; promotion-proof; full-TU sibling refresh; full ROM verify;
cleanroom; check-docs; check-scoreboard; check-overlay-syms; check-tooling.
No unresolved matching blocker remains for this owned range.
<!-- plateau-handoff:overlay1FindClosestSample:end -->
