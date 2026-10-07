<!-- plateau-handoff:func_overlay_056_F00001A0_18A2F18:start -->
### `func_overlay_056_F00001A0_18A2F18` plateau handoff

- source: `src/overlays/o056/overlay_056.c`
- score: 538 differing words
- frame: 0x1F8
- relocations: 75
- first mismatch: +0x50
- summary: Colour table reconstructed as a field at overlay-data +0x50 (gOverlay56Data.colors[index]) so both marker sites emit lui plus scaled addu plus lw 0x50 off the data-section base, the live pointer the target already holds. Indexing gOverlay56Colors by name still costs an extra addiu per site. Minimap sprite identity is gOverlay56Resource, not a NULL-page load. Aligned split 132 naming 244 immediate 28 structural 216 versus the prior 130/237/28/223; size is 591 versus 581 (delta +40) because the colour loads add the four missing base words plus leftover surplus elsewhere. The eight-word hole at target +0x6D8 remains unpack schedule: the target shifts RGB immediately after the load (green, blue, then red in the mode-branch delay slot) while this candidate still unpacks in the call. Identity-gate passed with CDX_PROC=6. Do not colour-landscape until size delta is 0. Ghost slots at D_800D1494 (alpha 255) and D_800D1498 (alpha 85) are identified but naming them did not move the aligned residual. Next: force the AI-site unpack before the mode branch without extra copies, then the unsigned-float surplus around +0x498.

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

<!-- plateau-handoff:func_overlay_056_F00001A0_18A2F18:end -->
