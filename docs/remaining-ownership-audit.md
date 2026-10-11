# Remaining padding and original-assembly evidence

Audit baseline: d4f75a605837509edd4c7232e288db815d45e66e.
This audit separates executable work from accounting. It changes neither the
scoreboard denominator nor matching credit. Evidence is recomputed from the
configured ELF, overlay atlas, YAML boundaries and the authenticated US ROM.
Detailed range receipts remain in ignored build/phase100-accounting.

## Historical accounting candidates: 700 bytes

| Category | Bytes | Evidence and next action |
| --- | ---: | --- |
| Named overlay padding | 656 | 82 atlas owners, all zero-filled in the ROM; review exclusion from the executable denominator. |
| Embedded overlay alignment | 16 | Four trailing zero bytes each in overlay 9's main TU and the ranked ranges in overlays 12, 46 and 99; separate function endpoints from section extents. |
| Resident function extents | 24 | ELF exceeds the measured target by 4 bytes for func_800180B4, 12 for func_8003C80C and 8 for func_8002B040; each excess is zero-filled. |
| Single nop identity | 4 | func_8005800C has a four-byte ELF identity and no source declaration or use; incoming references and predecessor control flow still require review. |

The raw-assembly census reports only 644 of the 656 named overlay padding
bytes. The other twelve are the four-byte compiler-alignment owners in
overlays 15, 22 and 45, deliberately retained in the atlas although their
configured YAML has no separate assembly subsegment. This is a representation
difference, not missing executable code.

Zero contents alone do not prove unreachability: delay-slot nops and explicit
entry points are executable. The proposed accounting correction therefore
requires an independent boundary/reference review and regression coverage
before changing the metric. Any denominator reduction must be reported
separately from newly ROM-exact executable bytes.

## Unresolved original-assembly evidence: 9,380 executable bytes

The 2026-10-11 raw resident census still reports 9,396 bytes outside the accepted original-asm
ledger. Twelve bytes are trailing section padding, and four are the nop
identity above. The remaining executable inventory is:

| Owner | Executable bytes | Present evidence and acceptance gap |
| --- | ---: | --- |
| main/trackasm | 2384 | Tier B call graph and tier D four-function ordering against JFG's extracted assembly; shorter revisions prevent whole-object identity. |
| main/shadows_fp_19144 | 272 | func_80018544, JFG's shadowYHeight in a shorter revision (68 against 69 words, first divergence +0x0); odd-FP evidence only. Its neighbours main/shadows_fp (routines one and two) and main/shadows_fp_19254 (routine four) are verified since 2026-10-09 by masked identity with JFG's built `src/shadows_214A0.c.o`. |
| main/weather_snow_asm | 832 | Snow update/vertex pair, tier B/D lineage and odd-FP vertex operations; no permitted reference build holds either routine (DKR/JFG have compiled C with a different calling convention). |
| main/conv_mult_matrix (ROM 0x59BF0) | 444 | Handwritten instruction shapes and odd-FP operations; tier B as the consumer of camConvertMatrixList's list, JFG's conv_mult_matrix role, but a shorter revision (count argument, no multiply) with no byte-identical reference object. |
| main/gen_anim_data (ROM 0x59DB0) | 5448 | Large unrolled routine and four subsidiary entries, with odd-FP and handwritten instruction shapes; internal branch labels are not separate callable functions. Tier B: called where JFG's modGenAnimMatrices calls gen_anim_data, but a byte-wise bitstream revision with no byte-identical reference object. |

The final two owners have four and eight trailing padding bytes respectively.
Those twelve bytes are outside their ELF function extents and do not add to
the 700-byte displayed-deficit accounting correction.

The existing compiler-mechanism evidence in docs/modules.md section 6.2 closes
repeated IDO odd-FP flag/version sweeps. It does not itself establish reference
identity. The current `verified_asm.us.txt` class 2 accepts a proved handwritten
routine whose owned range is byte-identical to the same routine in a permitted
reference build, even when that build carries extracted `GLOBAL_ASM`. A tracked
original assembly source is not an additional prerequisite for that class.
Exact boundaries, relocation identities and all ordinary acceptance proofs
still apply; resemblance or handwritten instruction shapes alone earn no credit.

The earlier source-tree check at JFG revisions
`c82affffe8f11cb5b440cfa918f4582ad8573279` and
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no tracked trackasm assembly
source. That finding does not disqualify a future exact class-2 reference match.
The accepted shadows neighbours already use that route. The remaining shorter
revisions have no qualifying exact reference match; see the bounded retail and
compressed-code searches in [raw-asm-census.md](raw-asm-census.md).

The two matrix candidates are outside this raw-owner census: `func_8002AA50`
owns 296 bytes and `func_8002B040` owns 136, with its eight alignment bytes
excluded. Together with the 9,380 raw bytes, they account for 9,812 executable
bytes awaiting assembly authentication. Their separate reference-search
closures are recorded in their matching handoffs. After the 2026-10-11
wakeAllocate promotion, the seven remaining C candidates own 19,468 bytes,
recomputed from the current nonmatching ranking after excluding those two
matrix owners. The generated scoreboard leaves 29,280 executable bytes:
19,468 awaiting C matches and 9,812 awaiting assembly authentication. The
wake promotion adds 1,404 genuinely matched C bytes; the accounting
reconciliation itself adds no credit.

PROVENANCE: the existing JFG lineage descriptions are drawn from
mickey.us.yaml, docs/resident.md and docs/modules.md, which disclose the public
Jet Force Gemini decompilation's track assembly and shadows/weather sources.
No external source body was imported by this audit.

Validation: raw-assembly census; ELF function-size census; atlas ownership
reconciliation; ROM zero-range checks; documentation and clean-room gates.
