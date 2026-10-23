# Model animation and matrix-batch boundaries

Read-only contract audit at `2334e49040b9e9a1fb8d1e0675eacab38f88ed6a`,
2026-10-23. Six verified-assembly entries form two outer callable boundaries.
Their existing linked ranges (5,892 bytes) and complete ROM were byte-identical
at that pin. This audit used source, bindings and linked-code inspection; it
performed no new build, numerical execution or game execution. No assembly
classification, native implementation or new matching credit follows from it.
See the [boundary map](native-boundaries.md) for the wider scope.

## Outer animation call

The actual C caller is [func_8005AF14](../src/main/models_5B300.c#L609). It calls `func_800591B0(outputMatrices, rootMatrix, instance, hierarchyNodes, optionalPatchStream)`. Arguments one through four enter in `a0`–`a3`; the fifth is at entry `sp+0x10`, read at `sp+0x80` after the 112-byte frame. No numerical FP register is a hidden input. `v0` is scratch, not an authenticated result/status; the caller uses this as `void`.

The frame saves/restores `ra`, `s0`–`s7`, `fp`, and each single-precision value in `f20`–`f31`. The outer routine restores its saved integer registers and stack through the common epilogue at `8005A4F0`. It uses all FP registers, caller-saved integer state, and HI/LO; floating arithmetic can change FP exception/condition state. It does not write FP control through `ctc1`. The word-wise FP save proves preservation of the single-precision values used here, not arbitrary 64-bit contents of those registers. As in the earlier matrix audit, odd-register arithmetic belongs to the runtime FP-register mode, not a portable paired-register assumption.

The private readers preserve the outer numerical FP live values; the patcher writes no FP registers. The patcher destroys saved-register work values, and the outer routine deliberately establishes its next working bases/constants afterward. There are no external trig/matrix calls inside this cluster: lookup and composition are inline. Replacing a private helper with an ordinary C call would require an explicit boundary redesign.

The instance fields actually consumed by the entry are:

| Offset | Machine interpretation |
|---|---|
| `0x1C` | pointer to animation descriptor |
| `0x20` | pointer to packed frame data |
| `0x24` | pointer to persistent writable animation workspace |
| `0x2C` | `f32` frame position in normal mode |
| `0x30` | `f32` approach/blend factor in transition mode |
| `0x3C` | signed 16-bit target frame index in transition mode |
| `0x3E` | signed byte; zero selects normal mode |

These fields are read; the entry mutates the pointed-to workspace and output, not those instance fields. The caller subsequently stores `2` at instance byte `0x3F` and consumes the generated float matrices before queueing conversion. Current `ModelRenderInstance` and `ModelAnimationState` declarations expose different partial views of the same prefix: names such as `animated`, `scale`, `offset`, and padding are not an authenticated complete schema.

## Frame representation and memory effects

The descriptor has an unsigned node count at `+7`, frame count at `+8`, and byte stride between packed frames at `+9`. Translation bases are three signed halfwords at `+0xA/+0xC/+0xE`, with unsigned width bytes at `+0x10/+0x11/+0x12`. Node descriptors begin at `+0x14`, stride 20 bytes. Each contains three signed angle bases at `0/2/4`, angle widths at `6/7/8`, three unsigned scale bases at `0xA/0xC/0xE`, and scale widths at `0x10/0x11/0x12`. Unused bytes in this view remain unnamed.

Each frame starts with translation X/Y/Z fields. Each node then consumes angle X, scale X, angle Y, scale Y, angle Z, scale Z, in that order. Width zero consumes no bits and selects the base. Nonzero fields are unsigned bit values: translations add the value directly to the base; angles add `value << 5`; scales add `value << 8`. Halfword stores retain the low 16 bits. This packed animation stream is **MSB first**, unrelated to the inflater's LSB-first numeric fields.

Normal mode truncates frame position toward zero to obtain `k`, decodes frame `k` and frame `k+1` (wrapping the latter to zero when it reaches descriptor frame count), and uses the fractional remainder for interpolation. It does not validate `k`, finiteness, frame count, or stride. Transition mode decodes only the signed target index at instance `+0x3C`; it approaches the old persistent values using the factor at `+0x30`.

Each reader refills by one or two bytes, never a speculative machine-word read. Under the sufficient format precondition that every width is at most 16, the reservoir leaves at most seven bits after each field. For a frame whose widths sum to `B`, exactly `ceil(B/8)` bytes are touched. Thus a bounded resource audit can verify this against frame stride and the loaded record extent without decoding or exporting animation payloads. Actual resource width limits were not established in this packet.

Mutable global scratch is `D_8007FAB4..D_8007FF40`: two translation triples (6 bytes each), two angle arrays (288 bytes each), and two scale arrays (288 bytes each), total 1,164 bytes. Each angle/scale array accommodates 48 triples. It is initialized-data storage used as mutable scratch, not a per-instance allocation. The mask table at `D_8007FA30` has 33 words, independently confirmed as low-`n`-bit masks for `n=0..32`. These shared arrays make the operation non-reentrant unless calls are serialized. Transition mode fills only the B arrays; the optional patcher still writes A and B, but the transition computation consumes B.

For `n` nodes, the persistent workspace is:

| Interval relative to workspace | Contents |
|---|---|
| `[0,12)` | three `f32` translation values |
| `[12,12+16n)` | four `f32` orientation values per node |
| `[12+16n,12+28n)` | three `f32` scale values per node |

[func_8001FC50](../src/main/models.c#L429) independently allocates `12 + 28 * matrixCount` bytes for this mode data, places its pointer at instance `+0x24`, clears the containing allocation, and allocates two 64-byte-per-node matrix lists. This is real allocation support for the observed workspace layout. It does not prove descriptor node count equals the source's signed-byte matrix count for every resource.

The orientation formula is quaternion-like: half-angle table lookups, a four-component dot product, hemisphere sign selection, and linear blending. The blended four values are stored without a square-root normalization. Matrix construction then uses `2 / normSquared` when the norm is positive, otherwise zero. Calling this slerp or promising normalized stored quaternions would be inaccurate. Unsigned scale `0x8000` denotes 1.0; interpolation uses `1/32768`. Translation and scale have ordinary linear interpolation/approach behavior with the actual instruction association.

Hierarchy input has 16-byte records: signed parent at `0`, XYZ floats at `4/8/12`. Any negative parent selects the root; a nonnegative parent selects `output + parent*64`. Each output is a full 64-byte row-major affine float matrix with translation in row 3 and homogeneous column forced to `(0,0,0,1)`. Root and selected-parent reads use the twelve affine floats, not their fourth column. Parent records must already contain valid matrices; there is no parent bound/topology check. No arbitrary overlap contract with root, hierarchy, descriptor, workspace, or stream is established.

The node loops execute before testing their decremented count. Node count zero is unsafe, and no 48-node capacity check exists. Trusted resource agreement, positive count, valid indices, readable stream extents, and disjoint live backing remain preconditions, not observed validation.

## Private continuation convention

| Label | Actual live-ins and result/effects |
|---|---|
| `8005A550` | `t1` byte cursor A, `t2` bit count, `t3` reservoir, `t7` mask base, `t8` width, `ra` return. Returns field in `v0`; updates A cursor/count/reservoir, clobbers `at/v0/v1`; preserves B cursor state and FP values. |
| `8005A5C0` | Same operation with `t4/t5/t6` for stream B; same shared mask/width registers. Preserves A cursor state. |
| `8005A630` | `t1` is an optional aligned halfword patch stream. Establishes its own global bases; clobbers `s0`–`s6`, `t1`–`t9`, `at`; no FP stores. |
| `8005A6F0` | The patcher's return continuation: `jr ra`, no independent arguments/frame/API. |

The patcher reads a big-endian halfword tag: high nibble `0` means add the next signed halfword to an angle element in both A/B arrays; `0x1000` means add the next unsigned halfword to both scale arrays, retaining low 16 bits; `0x2000` terminates via `8005A6F0`. Low 12 bits select a halfword index. Other tags consume one word and continue. No index, stream length, or termination bound is checked. Safe array membership requires an appropriate index below 144, and ordinary node membership further depends on the actual node count. These are source-resource audit questions, not additional C function declarations.

## Matrix batch conversion and lifetime

`func_80058FF0(list, entryCount)` takes a 32-bit pointer and signed entry count in `a0/a1`. It is a leaf without a stack frame or semantic scalar result. Nonpositive entry count touches no list/matrix memory. Each 8-byte entry contains a pointer at `0` and signed 16-bit matrix count at `4`; the two remaining bytes are unused. For a positive entry count, the inner loop processes **at least one matrix even when that entry's matrix count is zero or negative**. Positive matrix counts are a trusted contract.

Each matrix occupies 64 bytes. The routine loads all twelve input affine floats before the first store, so its actual in-place conversion is supported. It multiplies by 65536, applies `cvt.w.s` using the current FP rounding mode, and stores the integer high halves in the first 32 bytes and fractional low halves in the second 32 bytes, with big-endian halfword layout. It writes all 64 bytes and forces the homogeneous column. Input column 3 is discarded. No finite/range validation or rounding-mode setup occurs. This is a representation mutation, not a matrix multiplication or a copy into separate output storage.

It clobbers `at`, `a0`–`a3`, `t0`, `t8`, `t9`, and single FP values `f0`–`f12` and `f19`; no saved integer registers or globals are touched by this leaf. It leaves the FP control setting in place but arithmetic/conversion can change exception state.

The [queue/flush wrappers](../src/main/models_5B300.c#L258) append pointer/count entries to `D_800D78F0`, then flush through this leaf and reset `D_800D7CF0`. The queue backing is 1,024 bytes, hence 128 entries. Append has no capacity check and narrows its `s32` count to `s16`. [main.c](../src/main/main.c#L614) flushes after display-list construction; a level-reset path simply drops the count. Matrix pointers must stay alive until flush; float consumers must finish before conversion. Duplicate/overlapping queued ranges would be converted twice. The audited model path double-buffers its matrix lists, but complete renderer/RSP ownership and duplicate-queue exclusion were not proved here.

## Bounded next packets

1. **Source/resource schema:** reconcile only the real model-instance animation prefix and descriptor/node fields across the constructor, frame selector, renderer, and decoder boundary. Preserve exact object/relocation output. Keep the private labels internal. Current partial C views are a source-contract debt; the verified assembly itself remains resolved.
2. **Resource metadata validation, read-only:** bound one loaded animation record via sections `0x28..0x2B` and the existing cache loader; prove `1 <= nodeCount <= 48`, node-count agreement, widths/refill domain, frame extent/stride, parent ordering/ranges, and patch termination/index limits. The loader allocates the exact adjacent-offset difference and retains/refcounts its blob; the selector points into that blob using its signed descriptor offset. Do not export payloads. Broaden only if the first bounded record produces useful new information.
3. **Render lifetime:** follow queue append/flush, matrix double-buffer selection, cache writeback/task submission, and instance free/reset to prove queue capacity, positive counts, no duplicate conversion, and the last float/fixed consumers. This informs native providers; it does not justify a host matrix/decoder replacement in the decomp campaign.

No matching target was reopened, no donor source was read, no portable implementation was designed, and no speculative safety behavior was added. The bounded ABI/memory questions have settled enough to stop; unresolved facts require resource/lifetime authority rather than more instruction inspection.
