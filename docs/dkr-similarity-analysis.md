# Diddy Kong Racing similarity closeout

Status: measured on 2026-08-27 at `3cbc6ab9`, against the pinned Diddy Kong
Racing US v77 reference at `38d7f9ba39642e2b5311a76e0b83fb3fe2733262`.

## Result

Mickey contains **28,668 executable bytes that are provably shared with the
DKR build**. That is 3.04% of Mickey's current 944,368-byte whole-program text
census and 8.13% of the 352,476 bytes currently resolved as C or verified
original assembly.

The total is deliberately narrower than “looks related”:

| Exact surface | Executable bytes | Share of Mickey text |
|---|---:|---:|
| Rare game/engine code | 5,596 | 0.59% |
| Nintendo/libultra code | 23,072 | 2.44% |
| **Total** | **28,668** | **3.04%** |

This is not 28,668 bytes uniquely attributable to DKR. Most of the libultra
component is SDK code and some is independently present in JFG or another
reference build. It is code that the pinned DKR object surface proves Mickey
shares instruction-for-instruction, with reference relocations masked exactly
as described in `tools/find_known_objects.py`.

The exact surface is highly concentrated in resident code: 28,604 bytes, or
6.02% of the 475,104-byte resident text. The overlays contribute only the
64-byte `alSeqFileNew`, 0.014% of their 469,264-byte text. This agrees with the
broader structural measurement: DKR covers 10.4% of resident masked 8-word
ngrams but only 3.1% of overlay ngrams. Mickey's corresponding JFG values are
34.2% and 7.9%, so DKR is a valuable ancestor and source crosswalk, not the
closest whole-engine build.

All of the trustworthy exact DKR surface known at current function and TU
boundaries is now resolved. The remaining DKR opportunity is revised source,
not an undiscovered block of immediately transplantable object code.

## What was counted

Three evidence classes must remain separate:

1. **Exact shared code** is byte-identical after masking only relocation fields
   named by the DKR object's own relocation table. A promotion still has to
   compile with untouched IDO output and link byte-identically in Mickey.
2. **Source lineage** means DKR source explains the same subsystem, algorithm,
   or control flow, but Mickey compiles different instructions. It is useful
   decompilation evidence, not matching credit.
3. **Skeleton similarity** compares opcode shapes. It ranks leads and cannot
   establish identity by itself.

The accepted function-bounded exact census contributes 25,324 bytes: 5,596
bytes in 34 game/engine ranges and 19,728 bytes in 86 libultra ranges. A
whole-section audit found another 3,344 executable bytes in static helpers
that DKR's object symbol table does not expose as function symbols:

| Whole exact TU | Hidden executable prefix | State |
|---|---:|---|
| `libultra/src/io/sptask.c.o` | 284 | already matched C |
| `libultra/src/libc/rmonPrintf.c.o` | 20 | matched in this lane |
| `libultra/src/libc/xprintf.c.o` | 1,648 | matched in this lane |
| `libultra/src/libc/xldtob.c.o` | 1,392 | matched in this lane |
| **Total** | **3,344** | |

Compiler section alignment is excluded. In particular, the exact section
sizes are larger than executable code by 12 bytes for `rmonPrintf`, 12 bytes
for `xprintf`, and 4 bytes for `xldtob`. Data and rodata identity is supporting
whole-object evidence but does not enter the code-byte total.

PROVENANCE: DKR's permitted published source was read for the bodies promoted
in this lane. Point-of-use disclosures are in the new C files. The aggregate
audit above reads the pinned build's object sections, symbols, and relocations;
no ROM-derived bytes, disassembly, or workbench ledger are recorded here.

## Exact work closed in this lane

The initial function-symbol scan found 3,996 exact bytes that were still
unresolved. Every one now has a final verdict. The whole-object pass then
exposed 3,060 additional static-helper bytes in three of those promoted TUs.
Together this lane resolved 7,056 executable bytes of DKR-shared code:

| Unit | Executable bytes closed | Verdict | Commit |
|---|---:|---|---|
| entrypoint | 80 | verified original DKR-identical `hasm` | `02701860` |
| `rsp_segment` | 56 | IDO C exact | `f63cd9e0` |
| `byteswap32` | 48 | IDO C exact | `715f3ab7` |
| `mainproc` + `thread1_main` | 240 | IDO C exact | `7c22cad8` |
| `rmonPrintf` TU | 84 | whole-text IDO C exact | `bcf74a48` |
| `__osEepStatus` | 548 | exact function-tail carve | `3e95b911` |
| `xprintf` TU | 3,252 | whole text/data/rodata exact | `20183c19` |
| `xldtob` TU | 2,748 | whole text/rodata exact | `b567468f` |
| **Total** | **7,056** | | |

The xprintf-family objects require DKR's direct IDO flag family rather than
the normal asm-processor wrapper. The explicit Makefile recipes preserve that
compiler evidence. No instruction word is edited after compilation.

The lane also fixed an unrelated scoreboard attribution defect where nonzero
`*ABS*` overlay aliases could overwrite or double-count resident functions
(`d0ee7b1e`), then refreshed generated progress after a clean split
(`3cbc6ab9`). Current progress is 1,008 of 1,460 resident functions matched to
C and 352,476 of 944,368 whole-program text bytes resolved.

## Is exact DKR code left?

No trustworthy exact candidate remains at a currently known boundary.

- A fresh DKR v77 function scan checked all 368 open resident functions
  (232,204 bytes) after excluding matched C and verified original assembly.
  Requiring at least six unmasked words and ROM-wide uniqueness produced zero
  exact candidates.
- A fresh whole-`.text` scan of all 243 DKR build objects found the already
  documented resident TUs and the four hidden-prefix cases above. It found no
  new reliable section in remaining code.
- The committed overlay scan checks both DKR v77 and v80. Each reports one
  strong hit, one ambiguous hit, 104 negative non-empty overlays, and one
  empty overlay. The strong hit is the already-resolved `alSeqFileNew`; the
  short overlay 46 collision remains rejected.

“Zero at current boundaries” is not a proof that no additional exact
instruction run exists anywhere. Short generic leaves, an uncarved function
inside a large asm subsegment, or a DKR revision absent from the reference
farm can evade the acceptance rule. It does mean another blind exact scan of
the same v77/v80 objects is very unlikely to pay for itself unless Mickey's
boundaries change.

## Shared source likely left in the queue

There are two disjoint, non-exact queues totaling 18,564 candidate bytes.
They are leads, not progress credit.

First, 11 functions totaling **12,320 bytes** already carry point-of-use DKR
source provenance but remain assembly or `NON_MATCHING`:

| Area | Functions | Bytes |
|---|---:|---:|
| resident font, main loop, particles, RCP, saves, track, vehicle audio | 7 | 7,228 |
| overlays 31, 35, and 97 | 4 | 5,092 |
| **Total** | **11** | **12,320** |

Those targets are `func_8004BA8C`, `func_80026FB4`, `func_80040B88`,
`rcpClearZBuffer`, `packInit`, `func_8000FAE0`, `func_8005830C`,
`overlay31InitializeBuffers`, both DKR-adapted overlay 35 functions, and
`overlay97InitScale`. Their prior non-exact outcomes are evidence that DKR
supplies semantics and structure, not drop-in matching C.

Second, an exhaustive resident nearest-neighbour pass found **16 open
functions totaling 6,244 bytes** whose best DKR masked 4-gram Jaccard score is
at least 0.30:

| Mickey target(s) | Bytes | Best DKR evidence | Score |
|---|---:|---|---:|
| `resetMixCycle` | 76 | `init_pulsating_light_data`, identical skeleton and size | 1.000 |
| `getXZCompareMask` | 284 | `compute_grid_overlap_mask` | 0.760 |
| `func_8004D5E0` | 368 | `osCreatePiManager`, same size | 0.667 |
| `func_800056F0`, `func_80005820`, `func_800056A4` | 208 | object/asset accessors | 0.667 / 0.524 / 0.320 |
| `func_8004D7E0`, `func_8004D840` | 1,664 | `gzip_inflate`, `gzip_huft_build` | 0.519 / 0.449 |
| `func_8004E9F8` | 616 | `render_subtitles`, nearly the same size | 0.509 |
| `func_8002B040` | 144 | `mtxf_transform_point` | 0.471 |
| `func_8004D750` | 40 | `menu_dialogue_end` | 0.455 |
| `func_80006868` | 344 | `obj_init_emitter` | 0.398 |
| `updateMixCycle` | 292 | `update_pulsating_light_data` | 0.362 |
| `texInitTextures` | 400 | `tex_init_textures` | 0.310 |
| `func_80073490` | 604 | `osEepromRead` | 0.309 |
| `func_8000AA38` | 1,204 | `run_object_init_func` | 0.300 |

The cutoff is intentionally permissive. The one perfect skeleton is much
stronger than a 0.30 tail result, and generic SDK/accessor shapes can produce
false semantic names. None of these 16 is byte-exact.

## Recommended order

1. Try the adjacent pulse-light pair first. `resetMixCycle` has a perfect
   19-word skeleton and exact size against DKR, `updateMixCycle` selects DKR's
   adjacent update routine, and Mickey's level loader has the same
   asset-then-init call flow. This is the strongest evidence of revised Rare
   source still sitting in raw assembly.
2. Work the C half of gzip as one source-family investigation:
   `func_8004D5E0`, `func_8004D7E0`, and `func_8004D840`. The exact
   `byteswap32` and the following whole exact hand-written inflate object pin
   both sides of the TU. Use the DKR source as a hypothesis and Mickey's ABI,
   relocations, and workbench diagnosis as authority.
3. Try `func_8004E9F8` against `render_subtitles`, then the three object/asset
   accessors. Their scores and sizes are useful, but the intended data types
   need to come from Mickey's callers.
4. Treat the 11 explicit-provenance plateaus as bounded matching work, not
   donor-copy tasks. Preserve the current best object and stop after the ADR
   0009 attempt cap if the mechanism does not move.
5. Do not repeat the exact overlay sweep without a new DKR revision, new
   boundaries, or new object evidence. Use DKR there as a semantic crosswalk;
   exact overlay identity has already been exhausted.

## Reproduction and validation

The pinned reference identity and build digest are recorded in
`docs/references.md` and `tools/reference-builds.lock`. The principal scans
used here were:

```sh
tools/find_known_objects.py \
  ~/Desktop/dev/decomp-refs/diddy-kong-racing/build \
  --sections --start 0x1000 --end 0x76D10 --min-size 8 --rom-occ

tools/find_known_objects.py \
  ~/Desktop/dev/decomp-refs/diddy-kong-racing/build \
  --sections --all-overlays --min-size 8 --rom-occ

python3 tools/skeleton_scan.py kinship \
  --refs ~/Desktop/dev/decomp-refs/diddy-kong-racing

gmake overlay-donors-scan-check
```

The open-function exact and nearest-neighbour censuses used the same matcher
and skeleton routines against function address/size data from the freshly
linked Mickey ELF, excluding names still classified as verified original
assembly. No generated game binary or ROM was executed. A clean low-priority,
single-job build ended with Mickey's expected full-ROM SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`.
