<!-- plateau-handoff:func_overlay_009_F0000744_1866DBC:start -->
### `func_overlay_009_F0000744_1866DBC` plateau handoff

- source: `src/overlays/o009/overlay_009.c`
- score: 0 differing words
- frame: 0x38
- relocations: 9
- first mismatch: none
- summary: MATCHED; independently proved ordinary-C ownership recovery.

## 2026-09-27: ordinary-C proof recovery

Evidence tier A: the existing ordinary C owns 632 executable bytes in overlay 9
text +0x744..+0x9BC. A fresh configured stock full-TU compile, before
postprocessing, reproduces the owned bytes and independently resolves all nine
runtime relocation identities. The linked promotion proof reports 158 exact
words, frame 0x38, and nine of nine identities. First mismatch: none.

The resident symbol boundaries and Mickey's audio implementation authenticate
four previously unresolved calls. Correct float coordinate fields, pointer
handle storage and audio prototypes express that ABI directly. The header has
one TU consumer. All 5,408 configured text bytes remain identical to the saved
baseline; the two guarded siblings retain their previous residuals after a
fresh full-TU ranking measurement.

Eight convergent relocation aliases now have distinct destinations. This
preserves the existing linked values while making the metadata transformation
invertible. The four declared constant-relocation filters are unchanged; their
raw identities and linked bytes are checked by the existing promotion proof.
No instruction editing, identity-rule relaxation or new padding credit is used.

Changed files: the local overlay header and source, its alias recipe, generated
alias list, atlas ownership and derived atlas/ranking/scoreboard records.
Raw objects, relocation reports and the before/after full-TU comparison remain
in ignored build/phase100-proof. This recovers existing C ownership credit.

Validation: fresh raw compile; independent raw relocation comparison;
whole-TU text comparison; promotion-proof; configured sibling ranking refresh;
full ROM verify; cleanroom; check-docs; check-scoreboard; check-overlay-syms;
check-tooling. No unresolved matching blocker remains for this owned range.

<!-- plateau-handoff:func_overlay_009_F0000744_1866DBC:end -->
