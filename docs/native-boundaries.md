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
| `void mtxf_transform_point(MtxF, f32, f32, f32, f32 *, f32 *, f32 *)`; `void mtxf_to_mtx(MtxF, Mtx *)` | [camera.c declarations](../src/main/camera.c#L271), [MtxF](../include/game/math.h), [Mtx](../include/n_audio/gbi.h#L999) | Call-site shapes and storage layouts. `MtxF` is 64 bytes of target `f32`; SDK `Mtx` is target s15.16 with separate integer/fraction halves. The [conversion contract](matrix-contracts.md#fixed-point-conversion) establishes current-rounding dependence, packed byte order, disjoint backing and target fault behavior. Point-transform and caller-wide domain/lifetime claims remain separate. |

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

**The audited matrix boundaries have specific alias and preservation rules.** The
[matrix audit](matrix-contracts.md) identifies `mtxf_mul`'s incoming f2 read as
save/restore state, not a hidden matrix operand. Destination-equals-left-input
calls work with disjoint rhs; general destination-equals-rhs calls do not.
`func_8002AA50` retains argument and FP registers across its trig calls and has
no ordinary argument-home area. A replacement's ordinary ABI compatibility alone
does not establish that narrower preservation/stack contract. `mtxf_to_mtx`
uses ambient FP rounding, cannot generally convert in place, and writes two
big-endian halfword planes. Its exceptional target path supplies no established
host-cast sentinel. Remaining erased caller views and graphics completion
lifetime are separate source/resource work.
The [sprite contract](sprite-render-contracts.md) completes the bounded
provider audit: O60 supplies 26 read bytes and 18 unknown bytes, with authenticated
float frame/flag domains and synchronous CPU inputs. Remaining work is the full
resource schema, compatible caller APIs and deferred graphics-storage lifetime;
the unread bytes do not justify repeating the same provider inspection.

## Data, numerical and lifetime assumptions by family

- **Numeric/angle utilities:** [math.h](../include/game/math.h) supplies the
  common full-word angle/trig declarations. `Arctanf` takes two single-precision
  inputs in f12/f14 and normally returns a zero-extended 16-bit turn in v0;
  signed-angle reinterpretation belongs to the caller. It is a leaf without
  stack or memory stores, changes the input FP registers, and does not call
  the neighboring trig pair. Overlay 29 retains its recorded four-call legacy
  `s16` disagreement: the genuine immediate-word/saved-halfword repair remains
  blocked by the proved frame/home allocation difference. No new type closure
  follows from naming its already-compatible aliases.

  The actual ratio lookup multiplies by 2048, converts with ambient FP rounding,
  masks the converted word to an even byte offset and reads a signed halfword.
  For an ordinary ordered finite path without denormal or conversion-fault
  behavior, sign/ratio
  selection keeps that offset at most `0x800`, ending the read at `0x802` within
  the measured `0x808` retained named span. That span is not an original C array
  capacity. The mask alone permits a read end of `0x1000`; NaN, infinity,
  denormal, conversion-fault and altered FP-state behavior are not established
  by the finite-path bound. There is no range/fault test after conversion.
  Thread creation initializes the named FS/EV/round-nearest settings, but the
  provider does not save or set FCSR, and that initialization is not a proved
  live-mode invariant for every caller. Preserve these exceptional-domain gaps;
  a mathematical name does not authorize host libm substitution. The separate
  [matrix contract](matrix-contracts.md) retains the trig pair's private
  preservation requirements. RNG state/order and other small math entries
  still need their own contracts.
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
- **Shared matrix cursor:** [frontUpdate](../src/main/menu.c) and overlay 11
  capture a caller's `Mtx *` in retained `D_800D3144` and copy it back. The
  resident menu and audited views in 15 overlays now use `Mtx *`; former byte
  array aliases pass the cell address to actual `Mtx **` parameters. Storage
  remains a retained BSS gap without a C definition or new ownership claim.
  Overlay 11's opaque export signature and overlay 101's pointer-to-`s32`
  transport remain unresolved. Full function-type compatibility, capacity,
  reset/free and asynchronous graphics lifetime, related overlay 18 views,
  other command/vertex contracts and native readiness remain open.
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
| Angle and FP ABI: `Arctanf`, then its trig pair | Normal return/clobber and conditional finite-table bounds are authenticated. Keep overlay29's genuine value/frame blocker separate from exceptional-input, FP-mode and fault behavior; no new cast/union sweep or host-libm replacement is justified. |
| Matrix follow-up | The [matrix contract](matrix-contracts.md) establishes multiplication aliasing, private trig preservation and fixed conversion's format/rounding/fault boundary. Stack-matrix, overlay 56/98 and shared `Mtx *` cursor-view repairs are integrated. Remaining work is caller-wide numerical ranges, the retained cursor's storage definition, overlay 11's opaque export and overlay 101's pointer/integer transport, related overlay 18 views, complete thread/FP-context coverage, and graphics completion lifetime. |
| Inflater follow-up | Core dialect, continuation and shared-state lifetime are audited in the [contract](inflater-contract.md). A conditional normal-completion lookahead bound is established; resolve fixed-handler incoming memory and measure per-record output/workspace/overlap limits privately, accounting for pending loads in the stored-block unroll. Establish byte-4 meaning from producer evidence; stop before decoder implementation. |
| Animation resources and render ordering | The [animation contract](animation-contracts.md) establishes outer/private live-ins, clobbers, writable intervals, patch tags and the matrix-batch conversion boundary. Next join these contracts to actual constructor/resource counts, selector/update/render ordering, optional patch ownership and conversion-queue lifetime. Do not repeat private-entry mapping. |
| Render resource anchor: `rainFastDraw` plus its sole inspected overlay15 call family | Recover exact pixel/position/color formats, count/bounds/clipping and stack FP words. Then schedule snow/lighting/output independently rather than porting twelve labels as one renderer. |
| Geometry anchor: `getXZCompareMask` | Resolve integer-coordinate conversion, box schema, mask bits, scratch writes and lifetimes against the existing float-expression callers; expand to other track/shadow helpers only with new evidence. |
| OS state/context boundary | The [exception/scheduler contract](exception-scheduler-contracts.md) establishes entry roles, queue ordering, lazy FP and cleanup. Next reconcile full FP context/backing, the audio FR mode and optional hook continuations; mask-token/count/compare semantics remain separate. |
| Rare interrupt wrapper prefixes | The [block-memory contract](block-memory-contracts.md) closes the trio's bounded provider questions, including signed counts, overlap, boolean comparison and cancelling incoming filler; its declaration correction is integrated. Audit the Rare interrupt wrappers' additional prefixes and tokens separately. Per-caller extents and ordinary-memory assumptions remain resource obligations. |
| Overlay continuation | Audit the saved argument/result/stack set against typed exports and code load/unload lifetimes. Reuse committed ORT/ownership evidence, not donor scans or an untyped dispatcher design. |

These are bounded evidence packets, not implementation assignments or portable
replacement recommendations. They can be scheduled by disjoint entry/resource
ownership after the coordinator reviews this map.
