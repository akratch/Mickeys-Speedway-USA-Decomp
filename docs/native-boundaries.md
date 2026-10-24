# Verified-assembly native boundary map

This map covers the 107 routines in the verified-assembly ledger and the
contracts a future native implementation must preserve. Source references
were reviewed at `390eef6244b678d8ab94cbc8224fd440a51c9f47`; the map groups
existing evidence rather than establishing new matching results. Already-C
SDK/engine dependencies and RSP microcode also need platform contracts.

**Authorship/byte identity does not establish a native replacement contract.**
A current C declaration proves the caller's compiled argument shape, not every
assembly live-in, clobber, memory interval, numerical edge case or private exit.
Below, **shared** means a current header plus source usage; **call-site** means a
local declaration/use; **non-C** means the existing ledger identifies a machine
entry, private register convention or continuation. These are contract-confidence
labels, not new matching evidence tiers.

## Coverage and resource families

| Boundary family | Routines | Existing ownership | Principal resource/effect |
|---|---:|---|---|
| Numeric and matrix utilities | 44 | 37 non-interrupt entries in `main/math_util`; `math_matrix`, `matrix`, `matrix_2BC40`; SDK `sqrtf` | Float/fixed matrices, angle/RNG state, FP environment |
| Track and shadow geometry | 8 | `trackasm`; the three `shadows_fp*` segments | Mesh/box/query records, candidate and projection buffers |
| CPU lighting, weather and output | 12 | `lights2` (9), `weather_snow_asm` (2), `refractOutputAssembler` (1) | Mutable lighting/particle/vertex/framebuffer resources |
| Model animation cluster | 6 | `conv_mult_matrix` (1), `gen_anim_data` (5) | Matrix batches, animation/model records, private register state |
| Inflater core | 5 | `gzip_asm` | Shared stream cursors, Huffman workspace, decoded output |
| Overlay trap | 1 | `trapDanglingJump` | Call-site identity, loaded module/code lifetime, argument preservation |
| Machine startup | 1 | `entrypoint` | BSS, stack and boot-to-game transfer |
| Block memory | 3 | SDK `bzero`, `bcopy`, `bcmp` | Caller-owned byte ranges |
| Exception and scheduler context | 9 | SDK `exceptasm` | [Audited contract](exception-scheduler-contracts.md): target contexts, queues and private continuations; FP/backing gaps remain |
| CPU state, interrupt, cache and TLB | 18 | 16 other SDK entries plus the two `math_util` interrupt wrappers | Privileged state, cache visibility, counters and address translation |
| **Total** | **107** | Disjoint partition of the stored table | No per-function ledger is duplicated here |

The family counts partition the stored routine table in
[VERIFIED_ASSEMBLY.md](VERIFIED_ASSEMBLY.md). Absence of a C reference does not
establish unreachability: data, ORT or assembly references may still enter it.

## Established C-facing shapes and their limits

| Current entry/exit shape | Evidence | Boundary qualification |
|---|---|---|
| `u32 __osDisableInt(void)`; `void __osRestoreInt(u32)` | [os_internal.h](../include/PR/os_internal.h#L58), repeated save/restore callers | Shared previous-mask token round trip. Preserve nesting/order; no mutex or thread-local semantics are established by these names. |
| `OSIntMask osSetIntMask(OSIntMask)` | [n_event.c](../src/libultra/n_event.c#L14), `OSIntMask` in [os_internal.h](../include/PR/os_internal.h) | Call-site mask-token contract with paired restoration. Precise CPU/RCP bit effects need audit. |
| `u32 osGetCount(void)`; `void __osSetCompare(u32)` | [OS declarations](../include/PR/os_internal.h#L114), [timer declarations](../include/PRinternal/osint.h#L45), [osGetTime](../src/libultra/gettime.c) | Target-width wrapping counter/compare domain. Source computes unsigned deltas and extends time separately. Frequency, epoch, interrupt delivery and wrap behavior belong to the provider audit. |
| `u32 __osGetSR(void)`, `u32 __osGetCause(void)`, `void __osSetSR(u32)`, `u32 __osSetFpcCsr(u32)` | [os_internal.h](../include/PR/os_internal.h#L91) | Shared widths/returns; exact affected bits and old/new-value meaning must follow the assembly, not a portable state variable assumption. |
| `u32 __osProbeTLB(void *)`; `void osUnmapTLBAll(void)`; `void osMapTLBRdb(void)` | [osint.h](../include/PRinternal/osint.h), [virtualtophysical.c](../src/libultra/virtualtophysical.c) | Address-translation state, not RAM ownership. Failure values, page/ASID policy and privileged side effects need explicit audit. |
| Cache range calls `void(void *, s32)`; cache-all `void(void)` | [os.h](../include/PR/os.h), cache-all declaration in [sched.c](../src/main/sched.c#L84), [PI provider](../src/main/pi.c), [runlink](../src/main/runlink.c) | CPU/RCP/PI visibility and executable-code publication. Ordering is observable; do not assume a no-op contract from coherent host memory. |
| `void _bcopy(const void *, void *, int)`, `void _bzero(void *, int)`, `int _bcmp(const void *, const void *, int)` | [os_internal.h](../include/PR/os_internal.h#L116), ledger SDK identity | Shared source/destination/count shapes, no allocation transfer. The [block-memory audit](block-memory-contracts.md) establishes signed-count/zero/negative cases, overlap and access granularity. `_bcmp` returns boolean 0/1; its incoming scratch value cancels as common partial-load filler. Ordinary-RAM extent, address-view and synchronization preconditions remain explicit. |
| `f32 Powerf(f32, s32)` | [charControl.c](../src/main/charControl.c#L259), [shadows.c](../src/main/shadows.c), [resolved arity note](NATURALNESS_LEDGER.md) | Integer exponent is authenticated by the existing correction; it must not become a floating exponent. Negative/overflow/FP-status semantics are not supplied by a mathematical name. |
| `void mtxf_transform_point(MtxF, f32, f32, f32, f32 *, f32 *, f32 *)`; `void mtxf_to_mtx(MtxF, Mtx *)` | [camera.c declarations](../src/main/camera.c#L271), [MtxF](../include/game/math.h), [Mtx](../include/n_audio/gbi.h#L999) | Call-site shapes and storage layouts. `MtxF` is 64 bytes of target `f32`; SDK `Mtx` is target s15.16 with separate integer/fraction halves. Mathematical convention, rounding, aliasing and serialized byte order still need per-entry proof. |

The scalar `sqrtf(f32)->f32` call shape is used throughout the source and backed
by the stored SDK identity, but its result/exception behavior depends on the FP
environment. Odd single FP registers or a hand-written frame are authorship
evidence, not by themselves proof of a violated C ABI.

## Nonstandard entries and partial contracts

**Startup and exceptions are machine boundaries.** The
[entry map](../mickey.us.yaml#L119) establishes BSS zeroing, stack
installation and transfer to `mainproc`; [boot.c](../src/main/boot.c)
then initializes the OS and threads. There is no demonstrated ordinary caller
return contract for `entrypoint`.

The nine `exceptasm` labels include exception/vector entries, private message and
coprocessor handlers, queue primitives, dispatch/yield, and thread cleanup. The
ledger records `k0/k1`, COP0, interior entries, stack replacement and unsaved
callee-saved writes in this context. Do not give every label an independent C
function boundary. [osint.h](../include/PRinternal/osint.h#L39) exposes
only the C-facing queue shapes: enqueue/yield takes `OSThread **`, enqueue takes
that plus `OSThread *`, pop returns `OSThread *`, and dispatch has no arguments.
[createthread.c](../src/libultra/createthread.c) explicitly builds
64-bit GPR/FP context storage, sign-extends entry argument/stack values, installs
cleanup as the saved return address and initializes FP status. Context, thread,
queue and stack lifetimes are coupled; a host pointer-sized `OSThread` overlay is
not an established representation. The [exception/scheduler audit](exception-scheduler-contracts.md)
proves the conditional `0x230` context extent and cleanup nonreturn path. It
also measures VI live stack words inside the conditional FP image. Actual
FP-used save reachability, the real negative-ID audio mode and optional hook
continuations still require reconciliation. Adjacency is not a storage or
safety proof.

**The two Rare interrupt wrappers are not SDK aliases.** The
[symbol evidence](../symbol_addrs.us.txt#L397) says the SDK disable/
restore bodies match only their tails; the wrappers have additional prefixes.
No current C prototype for those wrappers was found. Prefix effects and their
entry/return tokens need audit before merging them with the shared SDK API.

**The runtime trap is a typed-call-preserving continuation.**
[overlays.md §5.1](overlays.md#L9) records that
`TrapDanglingJump` saves `a0`–`a3`, `f12`–`f15`, `v0/v1`, recovers the call site
from the return address, resolves its ORT identity, loads/relocates code, restores
arguments and jumps to the real callee. The `void TrapDanglingJump(void)` in
[runlink.c](../src/main/runlink.c) is an address declaration, not the
call signature of every overlay export. Module identity, stack arguments,
returned values, code-cache publication and unload lifetime must remain tied to
the actual typed export. No generic varargs dispatcher contract is proved here.

**Animation helpers form a private register cluster.**
[models_5B300.c](../src/main/models_5B300.c#L219) declares the outer
`void func_800591B0(Matrix *, Matrix, ModelRenderInstance *, ModelMatrixNode *,
void *)` call and the separate `void func_80058FF0(ConvListEntry *, s32)` batch.
The latter's list contains a matrix pointer and signed-halfword count; its caller
flushes the batch before resetting its global count. The outer animation call
borrows per-model matrices, hierarchy nodes and an optional patch stream.
Its owner follows the actual object path: the type-1 player path supplies
dynamic `ControlPlayer` state, despite the partial renderer view's `asset` name.
It is not an animation-blob or ROM-descriptor extent.
The [animation audit](animation-contracts.md) establishes its private readers'
t-register inputs, the patcher's s0–s6 clobbers and the private return exit.
`func_8005A6F0` is that exit, not an independently established public function.
The outer entry restores its saved registers; descriptor bounds, shared scratch
serialization and matrix queue lifetime still need resource-level authority.

**The two matrix boundaries have specific alias and preservation rules.** The
[matrix audit](matrix-contracts.md) identifies `mtxf_mul`'s incoming f2 read as
save/restore state, not a hidden matrix operand. Destination-equals-left-input
calls work with disjoint rhs; general destination-equals-rhs calls do not.
`func_8002AA50` retains argument and FP registers across its trig calls and has
no ordinary argument-home area. A replacement's ordinary ABI compatibility alone
does not establish that narrower preservation/stack contract. Float/fixed caller
views, actual stack-matrix ownership and graphics completion lifetime remain
separate source/resource work.

## Data, numerical and lifetime assumptions by family

- **Numeric/angle utilities:** [math.h](../include/game/math.h) now supplies
  shared full-word angle/trig declarations to 58 of 59 canonical consumers.
  `Arctanf` returns a zero-extended 16-bit turn in an `s32`; signed-angle
  reinterpretation belongs to the caller. Overlay 29 retains its explicitly
  recorded legacy `s16` disagreement. This does not complete the shared API or
  all private assembly contracts. Angle wrapping, RNG state/order, float
  rounding/denormals, trapping arithmetic and fixed conversion cannot be inferred
  from the common names. Some small math-util entries have no typed C declaration
  in this inventory; their function role remains open.
- **Track/shadows:** [track.c](../src/main/track.c#L439) and
  [shadows.c](../src/main/shadows.c#L130) provide partial local shapes
  for masks, query/output records and projection helpers. `trackMakePolylist`
  takes count, start/end vectors, radius storage and two words in current callers.
  `shadowBoundingBox` takes count/point storage and four scalar output pointers.
  The mask APIs currently declare integer coordinates even where callers supply
  float expressions: those conversions are part of the compiled source, not
  evidence for silently changing the ABI to float. Counts, scratch/global writes,
  signedness, capacities, mesh strides and mutable aliasing need entry-specific
  audit. No assembly allocation/free ownership is inferred from `void *`.
- **Lighting/weather/output:** [lights.c](../src/main/lights.c#L359)
  has local lighting-table/joint/vertex signatures with scalar-held addresses.
  For example, the current table builder is declared
  `void lightCreateLightTable(s32, s32, s32, void *)`; that is a call-site
  shape, not a proved table capacity or completed schema. [weather.c](../src/main/weather.c#L240)
  declares `snow_update(weather,gfx,count,particles,step)` and an `s32`
  `snow_vertices(camera,gfx,count,particles,matrix,vertices)` result. The caller
  retains particle state and alternates vertex buffers. The two
  [overlay15 declarations](../include/overlays/overlay_015.h#L90)
  expose a star mover with twelve trailing `f32` values and a rain drawer with
  framebuffer/dimensions/record/color pointers plus mixed scalar arguments.
  Stack-passed FP words, record stride, clipping, pixel format and output capacity
  are not a renderer contract yet. `cpuLine` and `refractOutputAssembler` have no
  typed C declaration in the inspected text; that does not prove dead code.
- **Inflater:** only `s32 gzip_inflate_block(void)` is C-declared by the
  [front end](../src/main/gzip.c). The five core entries share input/
  output cursors, bit state and the allocated Huffman workspace. The wrapper
  starts input at prefix+5 and repeats until the core returns zero, without end
  pointers or decoded-length/error output. The existing
  [inflater contract](inflater-contract.md) now establishes the raw
  DEFLATE-family block dialect, continuation flag, workspace lifetime and
  unchecked edge paths. These are not five independent APIs. Producer byte-4
  meaning, a fixed-handler incoming-memory dependency, read-ahead bounds and
  per-record output/overlap limits remain open.
- **Width/endian resources:** o32 pointers/longs are 32 bits; saved OS context
  words can be 64 bits; target float matrices are not host SIMD structs; graphics
  packed words/halfwords and serialized model/mesh records retain target byte
  order. Current callers rebasing integer fields or passing addresses in `s32`
  do not establish host pointer layouts. Overlay 27's local
  [physical-address macro](../src/overlays/o027/overlay_027.c#L4) still subtracts
  the KSEG0 base through a character pointer. Its four command-address uses need
  a defined target-address contract and an exact source correction before host
  compilation. Buffer ownership remains with the
  allocator/cache/frame/OS object that supplied it unless an actual transfer is
  proved. Framebuffer and RCP-visible data may outlive the CPU call.

## Concrete next packets, before replacement decisions

| Packet / initial owned anchor | Deliverable and stopping boundary |
|---|---|
| Angle and FP ABI: `Arctanf`, then its trig pair | Authenticate input/result widths, low-word/wrap rules and clobbers from Mickey; reconcile the conflicting caller declarations. Record FP-state dependencies. Stop before shared-header edits until that evidence supports one contract. |
| Matrix follow-up | The [matrix contract](matrix-contracts.md) settles the nonargument read, lhs-only alias allowance and private trig preservation. Repair `func_8005B644`'s actual stack-matrix view, then overlay 56's float/fixed views. Bound conversion edge cases and graphics completion lifetime before replacement decisions. |
| Inflater follow-up | Core dialect, continuation and shared-state lifetime are audited in the [contract](inflater-contract.md). Derive lookahead bounds, resolve fixed-handler incoming memory and measure per-record output/workspace/overlap limits privately. Establish byte-4 meaning from producer evidence; stop before decoder implementation. |
| Animation outer/private cluster | Map live-in/out registers and writable record intervals at `func_800591B0`, including its private t/s-register helpers and exit. Keep the converter batch separate until its matrix mutation/format is proved. |
| Render resource anchor: `rainFastDraw` plus its sole inspected overlay15 call family | Recover exact pixel/position/color formats, count/bounds/clipping and stack FP words. Then schedule snow/lighting/output independently rather than porting twelve labels as one renderer. |
| Geometry anchor: `getXZCompareMask` | Resolve integer-coordinate conversion, box schema, mask bits, scratch writes and lifetimes against the existing float-expression callers; expand to other track/shadow helpers only with new evidence. |
| OS state/context boundary | The [exception/scheduler contract](exception-scheduler-contracts.md) establishes entry roles, queue ordering, lazy FP and cleanup. Next reconcile full FP context/backing, the audio FR mode and optional hook continuations; mask-token/count/compare semantics remain separate. |
| Small ABI closure: block-memory trio and Rare interrupt prefixes | Audit overlap/return/clobber effects and wrapper-only state. A familiar SDK name or matching tail is insufficient evidence for an interchangeable primitive. |
| Overlay continuation | Audit the saved argument/result/stack set against typed exports and code load/unload lifetimes. Reuse committed ORT/ownership evidence, not donor scans or an untyped dispatcher design. |

These are bounded evidence packets, not implementation assignments or portable
replacement recommendations. They can be scheduled by disjoint entry/resource
ownership after the coordinator reviews this map.
