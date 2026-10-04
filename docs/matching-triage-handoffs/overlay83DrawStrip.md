<!-- plateau-handoff:overlay83DrawStrip:start -->
### `overlay83DrawStrip` plateau handoff

- source: `src/overlays/o083/overlay83DrawStrip.c`
- score: 68/77 words
- frame: frameless
- relocations: 2
- first mismatch: +0x4
- summary: vertexCount defined before the packet stores scores 68/77 at delta 0, first +0x4. Dead zeros, pointer-copy placement and an early address local do not give vertexCount a0. The opening display-list copy is still absent.

Summary before this remeasure: hypothesis=clobber a0 after the VERTEX increment without an extra word; spellings=macro assign inert at 69, L104 redefine 70 at +4, in-place cast 82 at +20; stall=a0 stays live and all three were reverted

Summary before this remeasure: L100 empty-if pair 73 to 69 at delta 0. Leaf, zero p1 probes. Cursor force t0 is 68. Missing a0 copy at +0x4. D_80000000 identity fail-closed.

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

## 2026-10-02 lane w7-o83

Flags. mk/overlays.mk gives this object POSTPROCESS redefine plus trim only.
No loop-unroll cap, no Olimit, and no -O2 -g3 line exists, so nothing was
removed. -Wab,-r4300_mul is not set and was not added. The overlay-wide ISA
stays -mips2 -32 and OPT_FLAGS stays -O2.

shape_lint reported zero artefacts. Its copy pattern does not see a typed
declaration, so the saved display-list local was still measured. overlay_tables
--json for overlay 83: text 2912 bytes, data 272, two reloc tables of 88 bytes
each. The function owns two LOCAL records, HI16 then LO16, at text offsets
0x964 and 0x970, symbol index 0xB60, the data-section base. They are not
SYMBOL records. D_80000000 is only a placeholder for the linked immediate.

Attempt 1. Product on the saved-pointer axis and the vertex-length spelling.
shape_product measured 6 cells. Floor 69 masked at delta 0, the tracked body.
Dropping the saved local is byte-inert at 69: copy propagation deletes it, so
it is not the missing opening move. Removing the empty if pair returns 73 at
delta 0, the regression already recorded. The trailing else, an inlined
count*2+2 with no doubled or vertex local, is not an axis value to
shape_product; scored by hand it is 74 masked at delta -4. Writing the vertex
length as shifts (the permitted Jet Force Gemini gSPVertexJFG spelling) does
the same. Both stop reassociation of (doubled+2)*10+8 into doubled*10+28, and
the length then uses vertexCount the way the target does. The function is 304
bytes. insertion_pairs names the target-only word at +0x4 as a move. The other
one-sided words are the env colour schedule and the triangle-count shift
hoisted into the first multiply delay. Not adopted: masked words rose, and the
size delta left zero.

Attempt 2. On that short shift-length shape, a pointer identity meant to keep
a real copy web. or-zero and and-0xFFFFFFFF, with the empty ifs, score 76 at
delta +4. Without the ifs, 75 at delta +4. The identity does not fold to a
free move. It adds two words. Not adopted.

Attempt 3. Same short shape. vertexCount or-zero, a definition already wrapped
in or-zero, and an env colour built red then green then blue after the opcode
store. All six cells are 74 masked at delta -4, identical to the bare shift
shape. Inert.

Stall, ADR 0018. Three attempts, no better residual. Best tracked score
remains 69 of 77 words, size delta 0, first mismatch +0x4, frameless. The
opening move still needs the display-list parameter to leave a0. That web is
coloured first and a0 is its cheapest colour, so a later vertexCount web
cannot take a0 while their ranges overlap. The cursor colour sweep was not
repeated.

Next lever: number vertexCount ahead of the display-list parameter so a0 is
taken first, without a word the 304-byte shape has no room for. Bind the two
LOCAL records to the overlay data base before any promotion.

#### 2026-10-04, vertexCount numbering

Configured full-TU baseline reproduced 77/77 words, 69 raw and masked
differences, first mismatch +0x4, frameless, two relocations, and zero
exact relocation identities. The opening gap is the absent copy of the
display-list parameter; the target then reuses that register for
vertexCount. Six controls:

- A dead zero before the saved pointer was deleted and left the 69-word
  residual. Eliminated.
- Defining doubledCount and vertexCount before the packet stores, on the
  size-exact body: 68 differences, aligned register class 45 to 33, first
  still +0x4. Retained. vertexCount still does not take the parameter
  register, so the opening copy is not emitted.
- Moving the saved-pointer assignment to the statement between the count
  load and the branch left that 68-word result unchanged. Eliminated.
- A dead zero inside the block, with the packet stores between it and the
  real definition, returned to 69. Eliminated.
- The same saved-pointer assignment inside the branch condition left the
  68-word result unchanged. Eliminated.
- Naming the vertex address before the colour stores shortened the function
  by two words and scored 76. Reverted.

The retained body is the early vertexCount definition. No match and no
byte credit. Do not repeat these six controls, the 304-byte shift-length
shape, the empty-if grid, or the cursor colour sweep. A later packet needs
a different way to make the parameter copy survive once vertexCount is
live, without a new stack home or an added word.

### 2026-10-04, lane grok-o083-copy: folded or-zero does not keep the copy

Configured full-TU baseline reproduced 77/77 words, 68 raw differences,
aligned differences 55, structural 22, register 33, first +0x4, frameless,
two relocations and zero exact identities.

Casting the saved display-list initializer through a folded or-zero left
that scored comparison unchanged at the same size and frame. The whole
object digest moved and the function comparison did not, which is the
line-metadata effect of a folded identity. Eliminated. The tracked body is
unchanged. No bytes are credited. Do not repeat this identity family.

#### 2026-10-04: parameter reassigned to the vertex count

Configured full-TU baseline: 68 raw and masked words, target 308 bytes,
size delta 0, first mismatch +0x4. Reassigning the display-list parameter
to `doubledCount + 2` after its cursor increment, and using that value in
the expanded vertex command, recompiled to 82 masked words at size delta
+20. The first mismatch moved to +0x0. The restored body re-scores 68 at
delta 0. The body is not kept. Do not expand the vertex command into a
parameter reassignment.

#### 2026-10-04: vertexCount stored into the display-list parameter

Configured full-TU baseline: 68 raw and masked words, target 308 bytes, size delta 0, first mismatch +0x4. Assigning that vertexCount into the display-list parameter after the colour stores, and passing the parameter to the unexpanded vertex macro, recompiled to 80 masked words at size delta +20. The first mismatch moved to +0x0. The restored body re-scores 68 at delta 0. The body is not kept. Do not repeat this parameter store.

<!-- plateau-handoff:overlay83DrawStrip:end -->
