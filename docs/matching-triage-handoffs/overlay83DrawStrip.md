<!-- plateau-handoff:overlay83DrawStrip:start -->
### `overlay83DrawStrip` plateau handoff

- source: `src/overlays/o083/overlay83DrawStrip.c`
- score: 69/77 words
- frame: frameless
- relocations: 2
- first mismatch: +0x4
- summary: L100 empty-if pair 73 to 69 at delta 0. Leaf, zero p1 probes. Cursor force t0 is 68. Missing a0 copy at +0x4. D_80000000 identity fail-closed.

Authorized donor-pass evidence on layered base
`99496346799c96eea701dece64f1c0015b3e76aa`:

- the pinned authorization and current structured evidence are
  `c98707b3d50df6ea8d4953cc720aaadb153b9b99` and
  `e70d0e4907bee5b88c82506dc776f9c35bbac480`;
- overlay 83 uniquely owns `.text` offset `0x850..0x984`, ROM
  `0x18D0010..0x18D0144`: exactly 308 executable bytes / 77 words, frameless,
  with no padding before `overlay83Dispatch`;
- the permitted JFG donor is an assembly-only fallback at
  `src/overlays/o64/overlay_64.c`, not a C implementation. Its 304-byte
  function has the same four packet semantics and `0.538` masked four-gram
  similarity, but it cannot supply source spelling or lifetime authority;
- the one natural donor-guided experiment retained the existing saved-list
  alias and expressed the four packet writes directly in the observed store
  order. It remained exact-size and frameless but regressed from 73 to 74 raw
  and masked differences;
- that experiment also moved the candidate HI16/LO16 sites from the baseline's
  target-aligned `+0x114/+0x120` to `+0xE4/+0xEC`. It was rejected and its
  untracked object was preserved under `build/attempts/overlay83DrawStrip/`;
- the original C body was restored and freshly recompiled. It reproduces the
  77-word baseline with four matching positional words, 73 differences from
  `+0x4`, and the two target-aligned relocation sites;
- preflight still fails closed because `D_80000000` has conflicting overlay
  runtime identities. Offset, type, and symbol spelling agreement alone does
  not satisfy exact relocation identity proof;
- prior alias, type/width, first-use, expression, hoist, generic flag,
  synthetic-temporary, and ungated-permutation families were not reopened.

Next lever: obtain source-authentic packet macro/lifetime evidence from a
permitted C donor or a procedure-scoped compiler trace, and separately bind
the two candidate relocations to a unique overlay-local identity. The current
assembly-only donor falsifies direct packet-store transcription as a lever.

Lane `w30-o083` on `8eb96979` (identity gate pass, proc 0, p2 only):

- configured re-measure of the inherited body: 308 bytes, 73 masked of 77,
  size delta 0, frameless, first mismatch +0x4, two HI16/LO16 records at
  `+0x114/+0x120`;
- instrumented `.text` is byte-identical to stock; the procedure emits 18
  p2dec / 11 p2color and zero p1 records, so a p1 `--every-colour` landscape
  has no probes;
- overlay22 L100: trailing empty `if (vertexCount)` then `if (count)` is 69
  masked at delta 0 (43 draws, down from 45). Reverse order is 73. Count-only
  is 71. VertexCount-only is 72. Semicolon form equals the brace form;
- leftover count OR-zero, AND-minus-one, and XOR-zero are inert on the 73
  body and regress the 69 pair. Extra empty-ifs on doubledCount, strip,
  displayList, saved, and `if (1)` are byte-identical with 69;
- delayed `vertexCount = doubledCount + 2` after the VERTEX increment (so a0
  can die and keep the saved-list copy) costs 8 bytes and scores 78. Overlay40
  comma-assign of vertexCount without an extra cmd local is 71. Overlay17
  cursor OR-zero costs 8 bytes. L160 deleting vertexCount returns 73;
- p2 force `w16=c7` (cursor onto t0) is accepted at 68, delta 0, first still
  +0x4. A 13-web sweep over colours a0..t0 found no other delta-0 improvement.
  That one-word colour is not reached unforced by early empty-ifs, rgb locals,
  index locals, or extra probe symbols;
- aligned 69-form residual: 19 exact, 41 naming, 1 immediate, 10 structural,
  plus six candidate-only and six target-only words. Dominant substitution
  remains a2 to t0 (16 sites). First mismatch is still the missing a0 copy;
- `D_80000000` still has conflicting overlay identities. Packet-store
  transcription was not reopened.

Next lever: a source form that clobbers a0 after the VERTEX increment without
the extra word the delayed-vertexCount spelling adds, so the saved-list copy
survives at +0x4; then a unique overlay-local triangle symbol for the two
relocations. Colour of the cursor web is priced at one word and is not the
remainder.
<!-- plateau-handoff:overlay83DrawStrip:end -->
