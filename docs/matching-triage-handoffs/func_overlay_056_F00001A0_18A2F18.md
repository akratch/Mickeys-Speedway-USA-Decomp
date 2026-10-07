<!-- plateau-handoff:func_overlay_056_F00001A0_18A2F18:start -->
### `func_overlay_056_F00001A0_18A2F18` plateau handoff

- source: `src/overlays/o056/overlay_056.c`
- score: 577 differing words
- frame: 0x1F0
- relocations: 75
- first mismatch: +0x0
- summary: Natural rewrite: aligned 316/121/61/90, delta -8; open: mapY web 197 takes caller c29 (20.0) under the 20.75 callee toll, no f24.

Summary before this remeasure: Colour table reconstructed as a field at overlay-data +0x50 (gOverlay56Data.colors[index]) so both marker sites emit lui plus scaled addu plus lw 0x50 off the data-section base, the live pointer the target already holds. Indexing gOverlay56Colors by name still costs an extra addiu per site. Minimap sprite identity is gOverlay56Resource, not a NULL-page load. Aligned split 132 naming 244 immediate 28 structural 216 versus the prior 130/237/28/223; size is 591 versus 581 (delta +40) because the colour loads add the four missing base words plus leftover surplus elsewhere. The eight-word hole at target +0x6D8 remains unpack schedule: the target shifts RGB immediately after the load (green, blue, then red in the mode-branch delay slot) while this candidate still unpacks in the call. Identity-gate passed with CDX_PROC=6. Do not colour-landscape until size delta is 0. Ghost slots at D_800D1494 (alpha 255) and D_800D1498 (alpha 85) are identified but naming them did not move the aligned residual. Next: force the AI-site unpack before the mode branch without extra copies, then the unsigned-float surplus around +0x498.

- ownership: Overlay 56 text `+0x1A0..+0xAB4`, ROM `0x18A2F18..0x18A382C`, exactly 2,324 bytes. `overlay56UnpackColor` starts at `+0xAB4`; the separate `+0xAF4..+0xB00` alignment tail is not owned.
- ABI: the resident inbound at ROM `0x27F18` passes the display-list pointer address, vertex-cursor address, and the current update-rate word. It is the sole direct resident relocation to export table index 1361 at `+0x1A0`.
- configured baseline: the prior retained Mickey-only form emitted 585 instructions, exact `0x1F8` frame, 577 differing words, and first mismatch `+0x4`. The current base is `229a63af7761df4c9b0a620150c4a9c6fd9b8946`.
- early CFG: runtime identities prove three distinct gates. Fade state advances only while `D_800C3A3C == 0`; rendering returns when `gOverlay56Resource == NULL`; and a nonzero `D_800D3450` rejects game states `0/2/1/3/4` from `func_80028F54`. The retained nested spelling preserves the target branch topology and exact frame.
- call ABI: all 21 calls now have authenticated identities. The sequence is `func_80028F54`, `viGetCurrentSize`, `camStandardOrtho`, `func_80005750`, `levelGetLevel`, `frontGet2PlayerSplit`, `func_8002FB34`, `func_8002F618`, a second `viGetCurrentSize`, sine/cosine, `func_800349A4`, `func_8002A82C`, `matrixTranslate`, two `func_8002A604`, `func_80024978`, `mtxf_mul`, `mtxf_to_mtx`, and two final `func_8002F618` calls. Correcting the FP and shifted argument slots is the main strict gain.
- retained result: configured IDO emits 591 instructions versus 581 with the exact `0x1F8` frame, 538 differing words, and first mismatch `+0x50`. The colour-field spelling plus `gOverlay56Resource` as the minimap sprite improve the aligned structural bucket 223 to 216 and byte-exact 130 to 132. Size grew from 583 to 591; positional 538 is insertion shadow of that +40 delta (L155).
- relocation surface: the shipped owner has 75 runtime records. All target runtime identities resolve; the extracted target object exposes 31 text records and the retained C function emits 61 static records. Relocation count, offset, and identity are not exact, so no linked-match claim is made.
- bounded attempts: the prior 119-row flag lattice and five identity forms remain exhausted. This reopen compiled explicit-return and nested gate spellings, early-only identities, typed call ABIs, combined downstream identities, and the `+0x58` base identity. Explicit and nested returns were byte-identical; the typed/combined forms supplied strict gains. No generic permutation was run.
- donor evidence: pinned DKR v77 `src/game_ui.c::hud_render_general` supplies the permitted semantic minimap phase order—fade/gates, orthographic setup, viewport offsets, map transform, reverse racer iteration, marker rendering, and state restore. Exact DKR v77/v80 and JFG object/source scans remain negative; the donor is structural evidence, not a promotable body.
- integration blocker: a guarded definition in this consolidated TU routes 5,024 previously credited C bytes through the nonmatching object, so the global atlas correctly fails stale. Session B did not rewrite it. Agent A can preserve exact C accounting with reviewed mixed ranges `+0..+0x1A0` and `+0xAB4..+0xAF4`, retain fallback `+0x1A0..+0xAB4`, and exclude padding `+0xAF4..+0xB00`; do not physically split at unaligned `+0xAB4`.
- next mechanism: hoist the AI-site RGB unpack (green and blue, red in the mode-branch delay slot) immediately after the colour load without introducing extra copies. Then the unsigned-to-float surplus around candidate +0x498. Ghost identities `D_800D1494` and `D_800D1498` are known but inert as named loads. Identity-gate is `CDX_PROC=6`. Do not colour-landscape until size delta is 0. Do not rerun flags or generic source permutation.
#### 2026-10-04: used RGB lifetime and sibling-backed signedness controls

The committed assignment base `eb90496fa674057eeccf3ba67aea8cbb68f8aedf`
passed the local zero-exit `base-only` gate with source and ledger both pinned
at `4d685542da42fa2b9be7883d065ba43fa0bbf245`. The fresh configured full-TU
baseline reproduces 538 raw/relocation-masked words, 591 versus 581
instructions, size delta +40 bytes, frame `0x1F8`, and first mismatch `+0x50`.
This packet tests the previously recorded RGB-before-mode-branch question;
it does not restart flags, identities, declaration ordering or permutation.

The first natural control names three genuinely used `u32` components
immediately after the existing packed-colour read and before the existing
mode-dependent x branch. Original shifts, masks, load identity, call order,
widths and stores remain unchanged. All values are bounded to 0..255; the
pure extractions do not add observable memory accesses. It produces 541
raw/masked words at the same +40-byte extent, frame `0x200`, first mismatch
`+0x0`. Every one of its 84 changed owned instructions retains its opcode
and registers and changes only a stack-relative displacement or the frame
adjustment. The unpack/branch instruction sequence does not change.

The exact same-TU `overlay56UnpackColor` independently supplies a second
source-type witness: `u32` red and `s32` green/blue. Its existing linked
64-byte range at module `+0xAB4..+0xAF4` compares byte-identically with the
same physical ROM range. One separately authorized contrast changes only
the two named green/blue declarations to `s32`. Both masked values fit that
signed domain. It is text, symbol and ordered-relocation identical to the
all-unsigned control, with the same score, frame and first mismatch. No
casts, extra references, padding, reordered expressions or helper call
were added.

Authentic retained whole-TU streams distinguish the source effects. The
unsigned control adds three load, three store and three local-location
records at CFE output; only the three local-location records remain in
optimized output. Signed green/blue additionally adds four CFE conversion
records and two optimized conversion records. Neither changes the final
non-stack instruction output. These are measured stage counts and output
comparisons; no source-variable generation, PRE event, allocator-home or
optimizer replacement lineage is inferred from them. The prediction that
these named component lifetimes recover the earlier target unpack emission
is falsified for both supported forms.

All seven configured C siblings retain their bytes and symbol geometry.
All 88 ordered full-TU static relocation rows retain offsets, types and
symbols, and the REL operands at their sites remain unchanged. Fresh target
preflight still refuses promotion: 75 shipped runtime records versus 69
candidate owned static records, 37 candidate identities resolved and 32
unresolved, with ten effective identities aligned. This supersedes the
older retained 61-record candidate count; it does not relax identity proof.
The null-address tail proxies and other incomplete physical bindings remain
unproved and cannot supply linked-match credit.

Untouched stock compiler-input capture/replay passes every allocated-content
section, symbols and relocations for the baseline and both controls.
Whole-file debug metadata differences remain disclosed. Raw prepared-input
self-comparison refuses multiline comments containing macro definitions;
untouched stock preprocessing of the captured actual input passes expanded
self-context, and both outside-body comparisons are unchanged. Ordinary
fresh-lane overlay-alias generation repairs the initial cold-link failure;
the failure and retry receipts are retained rather than treated as a source
change.

The baseline source is restored and recompiled, with full-content/symbol/REL
restoration fidelity passing. All meaningful inputs, objects, scores,
first mismatches, retained phase streams and receipts remain ignored under
`build/o056-rgb/`. Stop on elimination of the two assigned mechanisms,
without a count-based stall claim. Future work needs an independently
supported executable, ABI or physical-storage fact; another component
spelling, type or ordering grid is not a fresh mechanism. No C source or
matching credit is adopted.

#### 2026-10-07: duplicate half-dimension conversion corrections are real but nonexact

The assigned source/handoff pair passed the zero-exit base-only gate at
`a6c6a897e`. The configured full-TU baseline reproduces 591 versus 581
instructions, 538 raw/masked differing words, size delta +40, and frame
504 bytes. Actual configured compiler output contains four unsigned-float
correction branches around the two half-dimensions; the target has two.
Each baseline dimension first converts a u32 to float, then explicitly tests
its signed interpretation and adds the unsigned correction again.

Both inputs are logical right shifts by one, so their values lie in
0..2147483647 for every 32-bit input. The explicit negative predicates are
therefore unreachable. The packet removes only those two explicit fixup
blocks, retaining the native unsigned conversions, the earlier signed-height
conversion/correction, RGB expressions, declarations and other control flow.
This is a defined semantic correction, not an invented operation or padding.

Measured controls, as candidate words / byte delta / raw and masked
mismatches / frame bytes / aligned structural rows:

- Baseline: 591 / +40 / 538 / 504 / 195.
- Width fixup removed: 587 / +24 / 580 / 496 / 321.
- Height fixup removed: 585 / +16 / 578 / 496 / 184.
- Both fixups removed (native-only): 575 / -24 / 553 / 496 / 172.

The combined control retains exactly the target's two native conversion
corrections. Its normalized distance falls from 593 to 573, but its frame is
eight bytes short and its instruction count is six short. The single-site
controls isolate the two removals; neither reaches exact code. The first
positional mismatch is entry offset zero for all controls, versus baseline
+0x50. These are diagnostic full-TU comparisons, not relocation or linked
identity proofs. No candidate is adopted and no matching bytes are credited.

Captured actual compiler-input replay agrees with an independent configured
stock invocation in allocated content, symbols and ordered relocations for
baseline and all controls. Expanded compiler-input self/context comparisons
pass; all seven C siblings retain their bytes, extents and relative relocation
surfaces. Source and configured baseline allocated content/symbols/relocations
are restored exactly. Inputs, objects, dumps and comparison/fidelity receipts
remain ignored under `build/o056-half/`.

A separately authorized contrast at `a36b99d4c` retains both manual fixups
and changes only their initial conversions to `(f32)(s32)` of each shifted
value. The same range proof makes both signed casts exact and defined. This
removes the native correction paths while preserving the manual lowering.
This manual-only form emits 579 words, size delta -8, the exact 504-byte
frame, 521 raw/masked
differences and first mismatch +0x40. Aligned constant/register/structural
rows are 30/316/181, versus baseline 30/315/195; normalized distance worsens
from 593 to 635. Actual context, stock executable/symbol/relocation fidelity
and all seven sibling checks pass. This alternate form is also nonexact and
remains private; it does not establish a promotable reconstruction. The two
combined artifacts are retained in `build/o056-half/both/` (native-only) and
`build/o056-half/signed-initial/` (manual-only), alongside the baseline and
single-site controls.

The two-word deficit is not a proved two-nop correction. The target's early
rotated-coordinate multiplies have two hazard separators, while this candidate
defers part of that arithmetic and has one adjacent multiply pair elsewhere.
No further flag sweep, RGB spelling, home or allocation control is justified
by that net size. Both ways of eliminating duplicate conversion lowering are
now measured. Further work needs independent evidence for the remaining
expression-availability differences and incomplete physical bindings.

The restored canonical fallback passes full US ROM verification. A fresh-lane
alias bootstrap initially preceded the guarded canonical-object rebuild and
caused resident relocation overflows; regeneration after that rebuild restored
the committed alias file exactly and verification passed. Documentation,
clean-room and all 99 tooling test files pass. No game source or generated
alias change is committed.

#### 2026-10-07 (lane b-o056): natural-source rewrite from the listing

The inherited m2c body was discarded and the function rewritten from the
target listing and the relocation records (brief items 1, 3, 6, 20). Each
cell below is a whole-TU compile scored by aligned edit distance, measured
as exact / naming / immediate / structural rows and size delta.

- Inherited body: 132 / 244 / 28 / 216, delta +40.
- First natural draft (typed structs, while (i--) loops, three distinct
  resident bytes for the mode tests): 144 / 225 / 59 / 185, delta +24.
- GBI packet macros taking dl++ (pipe sync, scissor, matrix, prim colour,
  DKR vertex and polygon): 156 / 231 / 46 / 171, delta -8.
- Ghost selector as an else-if chain with a final else obj = NULL (the
  shipped redundant branch to the join): structural 91 to 90.
- x/z products written inline, not as locals: mapY is then computed at the
  loop head as shipped instead of being forwarded past the calls.
- mapY rebased in place (mapY += offset) before matrixTranslate: 440 to 421
  aligned disagreement.
- Colour word read from the table at each unpack with no colour local:
  421 to 283. This alone produces the shipped srl/srl/andi/andi/move/move
  unpack before the mirror branch; a colour carrier lets uopt forward all
  three components into the call block. Typing red/green/blue u8, splitting
  the masks into a second statement, and four statement orders were
  byte-identical or worse (measured, 16 cells).
- Retained: 316 / 121 / 61 / 90, 579 against 581 words, frame 0x1F0
  against 0x1F8. Positional 577 is insertion shadow.

Resident mode bytes, from the relocation records: 0x800D3198 is the
fade-rate test (== 3), 0x800D3194 the player count (== 2 split test,
slot = count - 1, loop bound i < count), 0x800D31A8 the mirror flag (every
negation and +520 offset). The header's gOverlay56Mode is a fourth object
(SetMode stores 0x800CD60C). Promotion needs a placeholder name per byte.

Decision variable. The two missing words are the f24 save and restore.
Instrumented uopt (CDX_PROC=6, .text identity-gated against the stock
object) records web 197 (mapY) with caller cost 20.0 at c29 against the
callee toll 20.75 (L56: nBB 83), so it is never offered a callee register.
Forcing p1:w197=c30 is accepted (forced=30) and puts mapY, cos and sin in
f20, f22, f24 exactly as shipped; the forced object scores 386 positional
at delta -8 with only scheduling and ghost-loop rows left structural.
Web 339 (ghostAlpha, v1, caller cost 20.0) is the same decision; the target
holds it in s1. Web 203 (mapX, save 20, caller cost 20.0) is decided first
and must stay caller (the target spills it to its declared home at 0x1B4),
so lowering the toll below 20 alone would hand mapX the bank. The source
form has to raise mapY's caller cost above the toll (another call crossing)
or order mapY ahead of mapX. Not yet found.

<!-- plateau-handoff:func_overlay_056_F00001A0_18A2F18:end -->
