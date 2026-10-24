# Matrix boundaries for future native work

This reference describes `mtxf_mul`, `func_8002AA50` and their immediate format
and trig dependencies. The source references below were checked against
`0d2c6336dea35316ec792e9c781991c1adab6195`; the audit used Mickey's own ROM-exact
linked providers. No assembly was replaced and no new matching credit is claimed.
The separate fixed-conversion audit below is pinned to
`b740117da9e07f2ac8ea8cd6084924b9611b4ec7`; its retained provider is 156 bytes
and all 39 instruction words agree with Mickey's ROM. No game code was executed.
See the [boundary map](native-boundaries.md) for the wider native-start scope.

## Float multiplication and aliasing

`mtxf_mul(lhs, rhs, dest)` takes three pointers to contiguous 64-byte float
matrices: sixteen four-byte cells, row stride 16, target big-endian storage.
It computes the row-major product `lhs * rhs`. The neighboring point transform
establishes row-vector use, with translation in the final row.

Each product is a single-precision multiply. For one output cell, let `pk` be
`lhs[row][k] * rhs[k][column]`; the accumulation is
`(p1 + p2) + (p0 + p3)`. Fused operations or reassociation do not preserve this
contract automatically. FP rounding, exception and denormal state remain
ambient inputs; this function does not configure them.

The reported nonargument read is preservation: entry saves f2 with an eight-byte
store, arithmetic overwrites f2 from lhs before using it, and exit restores it.
There is no hidden incoming matrix operand. In the observed main-thread FR1
mode this preserves the full f2 register; f3 is untouched by this body.

| Effect | Established contract |
|---|---|
| Reads | `lhs[0,64)`, `rhs[0,64)`; rhs is reread for each output row. |
| Writes | All sixteen destination cells, `dest[0,64)`; no global scratch. |
| Alignment/stack | Matrix words require four-byte alignment; its private eight-byte save requires an aligned stack. No capacity/null checks. |
| Registers | a0/a2 advance by 64; t0 reaches zero; a1 is unchanged; no scalar result in v0 is established. FP scratch is f4/f6/f8/f10/f12/f14/f16/f18; f2 is restored. |
| Lifetime | Sources remain stable until their last reads; destination is writable for the call. No pointer is retained. |

All four coefficients of an lhs row are loaded before its first output store.
Consequently:

- Disjoint matrices work.
- `dest == lhs` works when rhs is disjoint. Later lhs rows are still intact.
- `lhs == rhs` works when dest is disjoint.
- `dest == rhs`, including all three equal, is not generally supported: writes
  overwrite rhs values needed by later rows. Arbitrary partial overlap is not
  an established API contract.

The actual literal left-input aliases occur in
[overlay 43](../src/overlays/o043/overlay_043.c#L327) and
[overlay 98](../src/overlays/o098/overlay_098.c#L229). Current camera calls use
distinct named matrix buffers; the earlier camera citation was incorrect.

## Transform construction and the private trig contract

`func_8002AA50` takes a transform pointer in a0 and a writable float-matrix
pointer in a1, with no defined scalar result. It reads signed angle halfwords
at +0/+2/+4, float scale at +8 and translation words at +12/+16/+20. It does
not read +6/+7. The readable input intervals are `[0,6)` and `[8,24)`;
the writable output interval is `[0,64)`, with four-byte alignment.
[CameraScaledTransform](../src/main/camera.c) and
[ModelRenderTransform](../src/main/models_5B300.c) expose this layout.

The output has a uniformly scaled 3×3 rotation block, zero in the first three
cells of its last column, translation in the last row and one at [3][3].
Translation is copied as words, preserving its bits. All used input fields are
captured before the first output store. Hardware-level overlap can therefore
work with sufficient backing; a 24-byte C transform alone is insufficient
backing for a 64-byte destination. No literal overlapping caller was found.

With other rotations zero, a positive quarter-turn in the first field sends
+X toward −Z, the second sends +Y toward +Z, and the third sends +X toward +Y
under this row-vector convention. Camera calls these y/x/z rotation. This does
not establish every caller's axis naming or global scene handedness.

For each angle the routine calls `func_8002A8C0` then `func_8002A8BC`.
C0 is sine-phase lookup; BC adds a quarter-turn and falls into it. These are
table-based approximations, not permission to substitute host libm. Their
internal requirements go beyond their C parameter/result declarations:

| Requirement | Reason |
|---|---|
| C0 preserves a0 | The following BC call needs the same angle. |
| Both preserve a1/a2 | Destination/source pointers remain live across all six calls. |
| Both preserve accumulated FP temporaries | The six call sites retain respectively none, f2, f2–f3, f2–f4, f2–f5 and f2–f6. Current trig bodies change only f0 among those FP registers. |
| Both avoid stack writes | The caller allocates eight bytes, with saved ra at sp+4; it has no ordinary sixteen-byte argument-home area. |

The existing leaf trig bodies satisfy those requirements. An otherwise ordinary
ABI-compatible replacement need not. AA50 itself uses a0/a2, at/t0–t2 and
f0–f19 as temporaries, preserves ra/sp and leaves saved GPRs and f20–f31 alone.
Every numerical temporary it uses is defined internally.

FP mode is a separate resource contract: [boot.c](../src/main/boot.c) creates
mainThread with positive ID 3, and
[osCreateThread](../src/libultra/createthread.c) sets Status.FR, clearing it for
negative IDs such as audio's −4. Exception/context code restores that status.
This establishes the observed main-thread FR1 setting, not a complete OS audit
or proof that every possible caller runs on that thread. Compiler pairing
conventions must not be substituted for actual hardware register mode.

## Fixed-point conversion

`mtxf_to_mtx(src, dest)` reads sixteen target `f32` cells from `src[0,64)`
and writes the entire packed matrix at `dest[0,64)`. Its retained
[math-util provider](VERIFIED_ASSEMBLY.md) multiplies each cell by the exact
single-precision value 65536, then uses `cvt.w.s`, which obeys ambient FCSR
rounding. It neither saves nor changes FCSR. For a successfully converted signed
word `q[i]`, the high 16 bits are stored at destination byte `2*i` and the low
16 bits at `32 + 2*i`, in target big-endian halfword order. These are two planes,
not sixteen consecutive host integers. The [SDK Mtx type](../include/n_audio/gbi.h)
is a 64-byte s15.16 record with eight-byte alignment on the target; its `long`
layout must not be assumed on LP64 hosts.

| Effect | Established contract |
|---|---|
| Memory | Source words require four-byte alignment; output stores require two-byte hardware alignment, with the SDK type requiring eight. Both complete spans need valid backing. No null/capacity checks or pointer retention. |
| Capture and aliasing | Four source cells are captured per iteration before that row's stores. Disjoint input/output works; `dest == src` overwrites unread later rows and is unsafe in general. Partial overlap is not a promised C API. |
| Registers | a0 advances by 64, a1 by 32, t0 reaches zero; at/t1–t4 and f4/f6/f8/f10/f12 are scratch. No stack, calls, global scratch or scalar result is established. |
| Failure | Output is not atomic: stores start before all sixteen conversions complete. A fault can leave an earlier portion written. |

For finite normal inputs or signed zero in `[-32768, 32768)`, scaling by this
power of two is exact and the scaled values fit signed word conversion. Under
the initialized nearest mode, half-LSB inputs `+/- 2^-17` produce zero and
`+/- 3*2^-17` produce `+/-2`; this is not truncation. Signed zero loses its sign.
The largest target float below 32768 is 32767.998046875, yielding 2147483520;
-32768 yields the minimum signed word. No range check or clamp is present.

NEC's VR4300 manual, seventh edition (August 2000), defines nearest mode as
ties-to-even. FS controls denormal results, not denormal-input acceptance.
Denormal inputs and QNaN arithmetic raise non-disableable Unimplemented;
SNaN raises Invalid, enabled by Mickey's initialization. The specific
`cvt.w` restrictions also assign Unimplemented to integer overflow, infinity
and NaN. Its general Invalid-result description therefore does not establish
a conversion sentinel for this path
([FCSR, pp. 211–215](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=211),
[exceptions, pp. 239–244](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=239),
[CVT.W, pp. 579–580](https://datasheets.chipdb.org/NEC/Vr-Series/Vr43xx/U10504EJ7V0UMJ1.pdf#page=579)).
Tiny normal inputs scale upward and may round to integer zero; they are distinct
from subnormal inputs. Host casts, flush modes or saturation are not established
replacements.

[osInitialize](../src/libultra/initialize.c#L56) and
[osCreateThread](../src/libultra/createthread.c#L75) initialize FCSR to
flush-results, Invalid-enabled, nearest, with Inexact/Underflow/Overflow disabled.
The converter can update arithmetic exception state. The retail-text audit also
found compiler-generated temporary FCSR changes: the inspected bounded
conversion regions restore their saved FCSR on ordinary exits before calling
out. None of the converter's immediate callers writes FCSR itself. This is
stronger evidence than searching only named setter calls, but does not prove
arbitrary context memory, optional hooks or every caller's thread reachability.

The normal [exception path](exception-scheduler-contracts.md) routes FP faults
to the stopped/faulted-thread event path and redispatches; it does not emulate
this conversion or return a substitute word. The debug monitor stops ordinary
application threads and enters its diagnostic loop. These are static control-flow
facts, not a claim that every exceptional runtime state was exercised.

At the audit pin, 15 resident direct calls and six overlay SYMBOL/R26 calls
join the current source: camera, overlays 43/47/56, and overlay 98's reflection
renderer. Main-thread creation and `mainThread -> mainChangeLevel -> camInit`
establish one actual initialized-thread call path. Complete callback/thread
reachability, caller-wide numeric ranges and scheduler FP backing remain open.
Overlay 98 still exposes erased pointer/byte-matrix views; this audit does not
repair its API or prove destination allocation, alignment and graphics lifetime.

## Formats, lifetime and remaining source work

The first two routines produce float matrices; conversion produces the distinct
packed fixed record. Equal 64-byte extents do not make those domains
interchangeable.

Render callers convert CPU float scratch into separate fixed matrix storage,
emit its address and advance the fixed cursor by one 64-byte record. The fixed
storage must outlive its graphics consumer; float locals can end with the CPU
call chain. Allocator capacity, frame completion/free and concurrent camera
scratch use are not settled by this audit.

The stack-matrix source repair is integrated in `2334e4904`:
[func_8005B644](../src/main/models_5B300.c) now declares the genuine 64-byte
matrix at stack `[0x40,0x80)` in its `0x88`-byte frame and passes its actual
base. The earlier pointer eight bytes before its declared object is gone;
whole-object bytes/identities and ROM remain exact. This closes that local
backing error, not the model renderer's wider contracts.

Overlay 56's value-domain repair is integrated in `5b92f8e46`: three genuine
`MtxF` scratch matrices feed a separate SDK `Mtx *` packed output, and the cursor
cell is accessed as `Mtx **`. Raw/configured allocated bytes, all 94 ordered
relocations, function geometry and ROM remain exact. The separate private
graphics-pointer view, erased export declaration, related overlay 18 globals,
frame capacity and alternate reset/free lifetime still need review. This repair
does not establish the complete renderer API.

Overlay 98's reflection renderer now uses genuine `MtxF` locals and camera
projection input, separate SDK `Mtx` context views and a typed fixed-matrix
cursor. The [typed export](../include/game/reflection.h) agrees with its actual
track caller; one cursor step remains one 64-byte record. The two context slots
are views of retained module BSS, not newly C-owned allocation. This source
repair preserves target bytes and identities, but does not close the resident
object renderer's incompatible legacy object/opaque SDK views, overlay 18's
related cursor views, numeric preconditions or graphics completion lifetime.

These source debts and remaining graphics lifetime questions do not reopen the
verified assembly's matching status. Future translation must preserve arithmetic
grouping, alias limits, input capture, FP environment and the private trig calls.
