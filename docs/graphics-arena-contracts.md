# Graphics arena and cursor contracts

This records the current target program's allocation, cursor and completion
contracts. The source and relocation joins establish the roles below; they do
not establish a native implementation, complete C storage ownership or safety
for every failure path. The SDK command view in overlay 18 is compatible with
main's `Gfx *` cursor. Other arena declarations and APIs still need repair.

## Frame arenas and menu cursor cells are different objects

[Main](../src/main/main.c) retains two frame allocations and four current
cursors: display-list commands, matrices, vertices and triangles. The frame
base arrays and cursor cells still come from retained BSS. Their named labels
are not genuine sized C definitions. Overlay 18's command cursor uses the SDK
`Gfx` type; its byte-pointer base-array views and other private cursor views
are not thereby compatible with all of main's declarations.

The four cells defined in [menu_cursors.c](../src/main/menu_cursors.c) are
separate C objects: `Gfx *`, `Mtx *`, `void *`, and `void *`.
[frontUpdate](../src/main/menu.c) copies incoming pointer values into them,
dispatches menu work, then copies the advanced values back. It neither allocates
an arena nor establishes a graphics fence. The cells retain their values after
return. Non-reentrancy and absence of stale use outside that interval are
preconditions, not consequences of their physical definitions.

Passing a cell address is different from passing its pointer value. A common
pointee representation does not permit a cast between `Mtx **` and `void **`.
The remaining erased vertex/triangle interfaces and private record declarations
need a coherent provider/consumer audit; equal target sizes alone do not make
C types compatible.

## Modes, allocation and partition order

[overlay18InitializeBuffers](../src/overlays/o018/overlay_018.c) initially
selects mode 3. The current runtime caller of `overlay18Reconfigure` is main's
transition path. [levelGetGfxIndex](../src/main/level.c) returns camera count
minus one, except that a valid level's nonzero graphics flag selects mode 4
when there is one camera. It does not return the raw level flag.

The normal one-to-four-camera UI invariant therefore selects modes 0 through
4. Overlay 47 initializes the player count to one, visits four controller slots,
and maintains the join/leave count. Actual camera-count callers use that count
or one. Nevertheless, `mainChangeCameras` accepts an unchecked integer, the
transition stores it in a byte, and overlay 18 performs no index validation.
Zero, arbitrary external calls, corrupted state and indirect writes are not
covered by the normal mode argument. Other stored table entries do not prove
reachable modes or original array capacities.

Let `C8`, `C64`, `C10`, and `C16` denote the four selected element counts.
Each of the two allocations requests
`8*C8 + 64*C64 + 10*C10 + 16*C16` bytes. The partition order really differs:

| Path | Order from the allocated base |
|---|---|
| Initial allocation and fallback | Commands, matrices, vertices, triangles |
| Reconfiguration | Commands, matrices, triangles, vertices |

The legacy `Region8`, `Region64`, and `Region16` names do not reliably describe
those roles. Current runtime identities and the consumer fields determine the
roles; the two paths must not be normalized into an assumed arena struct.
The retained count-table storage and separate trailing arithmetic-function
storage are not newly recovered C objects.

On a mode change, reconfiguration saves the allocator's delay policy, disables
it, frees both old bases and tries allocation at the old addresses. A partial
failure frees the successful half and invokes the default allocator. The local
requested index survives that fallback: later partition/count expressions use
it even though the fallback stored mode 3. The default allocation path has no
NULL check. This behavior remains part of the matched source; successful
allocation and a coherent mode are explicit preconditions.

[The allocator](../src/main/memory.c) aligns initial storage and rounds positive
allocation sizes to 16 bytes. Its split operations preserve that alignment;
successful fixed-address replacement uses an already aligned base. The observed
partition counts preserve sufficient alignment for the target's eight-byte
`Gfx` and `Mtx` alignment. This relies on a valid mode and intact allocator
state. Allocation rounding is not additional element capacity.

The delay token is an integer policy value, not an owned pointer. Overlay 18's
current `mmGetDelay` and delay-restoration declarations disagree with the actual
`s32 mmGetDelay(void)` and `void mmSetDelay(s32)` providers. That API debt remains
open and is separate from the command cursor view.

## Commands and the actual geometry records

The SDK `Gfx` command occupies eight target bytes. Overlay 18 preserves its
existing cursor increments, casts, store order and values, accessing the words
through `words.w0` and `words.w1`. Its former members used `u32` (unsigned long);
SDK `Gwords` uses unsigned int. Both are 32-bit under the actual IDO o32 build,
but they are not identical typedefs or a promise about host `long`.

[Camera](../src/main/camera.c) writes a ten-byte vertex containing three signed
halfword coordinates followed by four color bytes. [Track](../src/main/track.c)
uses the same field layout and a separate sixteen-byte triangle containing
flags, three byte-sized vertex indices and six signed-halfword texture
coordinates. Its producers write and advance those records, and emit their
addresses into the command stream. The vertex is not the SDK sixteen-byte
`Vtx`. Main's opaque byte records have the same strides, not the same C type
identity. FP-to-halfword conversions retain the exceptional-domain and rounding
limits in the [sprite](sprite-render-contracts.md) and
[matrix](matrix-contracts.md) contracts.

Main computes consumed counts and updates high-water/overflow state **after**
rendering has written. Replacing an oversized outgoing command list with a
sync and end command does not undo earlier CPU writes. These checks detect
excess; they do not prove a pre-write capacity bound for every producer.

[Model command generation](../src/main/models.c) separately encodes ten-byte
vertex and sixteen-byte triangle transfer roles and allocates cached command
lists. Persistent model geometry, textures and nested command lists have their
own ownership and unload obligations. Frame arena reuse does not settle those
resource lifetimes or authorize a common backing-object declaration.

## Normal completion and the failure boundary

Overlay 18 seeds the initial command slot. Main submits the completed slot,
toggles the index, resets all four cursors to the other slot and builds the next
frame. Under normal completion, the other slot's previous task was waited for
in the preceding iteration. This argument requires no external retained writer
and the current non-reentrant frame sequence.

[rcpFast3d](../src/main/rcpFast3d.c) prepares a task, binds the submitted command
range, writes back the data cache and enqueues work using the main completion
queue. Enqueue success is not graphics completion. The shared
[task interface](../include/game/rcp_task.h) returns `s32` and accepts mutable
`void *` command boundaries in main and overlays 33, 42 and 43. The exclusive
end must belong to the same live backing representation range as the start;
both are eight-byte aligned and their nonnegative byte distance must be
representable and divisible by eight. The provider converts that distance to
a count of complete eight-byte words. This preserves the graphics task's
existing convention; it does not reinterpret every SDK task length as words.

The task descriptor retains its SDK `u64 *` address field. Passing the address
does not establish a common `u64` array or make the distinct command unions
compatible. These conditional interface repairs add no capacity check, backing
allocation proof, authenticated completion, or resource lifetime guarantee.

[The scheduler](../src/main/sched.c) tracks SP and DP completion separately.
The normal completion routine notifies only after both state bits clear; the
retrace flag can defer notification until two retraces. `rcpWaitDP` receives
from the main queue, also waits the flagged blur/refraction work and the overlay
43 hook, then clears its pending flag. Main performs framebuffer work,
`runlinkTick`, `mmFreeTick`, capacity accounting and transition processing after
that wait. The sole overlay 18 reconfiguration occurs in this transition path,
after the wait and explicit free-stack flushes. This supports a conditional
normal-path reuse/free sequence, not a general allocator fence.

The timeout path sends to the same queue before the hardware status resets and
optional diagnostic rendering. `rcpWaitDP` does not authenticate a successful
completion identity. Diagnostic work can itself reuse task command storage and
halt; `mainPreNMI` separately stops hardware after a wait and does not return.
DMA quiescence, cancellation ordering, wakeup races and safe reuse on those
paths remain unproved. Queue-full/submit failure, interrupt ordering, arbitrary
reentry and malformed task types also remain outside the normal-path argument.
Allocator delayed frees count ticks; they are not per-object completion tokens.

## Remaining native-start obligations

- Resolve timeout/reset notification versus actual SP/DP quiescence, and the
  lifetime of every persistent resource reachable from submitted commands.
- Establish a common ten-byte vertex/sixteen-byte triangle provider/consumer
  contract across main, menu, camera, track and overlays, including current
  pointer-object transport and genuine frame storage definitions.
- Repair allocator and task API disagreements with full caller proof, and
  establish valid mode, successful allocation and pre-write capacity bounds.

None of these obligations is discharged by the compatible command cursor view.
