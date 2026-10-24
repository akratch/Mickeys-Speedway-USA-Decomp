# Sprite rendering contracts

This reference describes overlay60's static sprite argument and the actual
camera/texture providers at source pin
`0d44a82d016a4d8cb624ae0f3fcc4dac4cf239c6`. The audit used Mickey's own source,
qualified relocation metadata and previously ROM-exact linked output. Complete
camera, sprite and immediate transform provider ranges were independently
compared with the local ROM. No source schema, storage ownership, assembly
replacement or new executable credit is claimed.

## Argument fields and limits

The character-preview branch in
[overlay60](../src/overlays/o060/overlay_060.c#L850) passes initialized module
storage at `0x178` to
[camDo2DSprite](../src/main/camera.c#L1485). Its final read reaches byte `0x2B`;
the separately referenced four-pointer table begins at module offset `0x1A4`.
The actual retained input covers **60 bytes**, comprising this 44-byte region
and the following 16-byte table. There is no independently sized 44-byte ELF
object witness. These facts establish readable backing and a neighboring
consumer boundary, not the original C allocation.

| Relative offset | Authenticated view | Limit |
|---|---|---|
| `0x00..0x05` | Three signed 16-bit rotations; initialized zero. | Camera uses y/x/z naming, negating the first two and preserving the third. This does not settle every caller's axis names. |
| `0x06..0x07` | Two zero bytes. | Unread here; no O60 flag semantics or padding claim. |
| `0x08` | `f32` planar scale, initialized `0.5f`. | Camera constructs z scale as one. |
| `0x0C/0x10/0x14` | `f32` x/y/z, initialized −15/−39/0. | Converted into a signed-halfword render vertex; arbitrary conversion extremes are not validated. |
| `0x18..0x27` | Sixteen zero bytes. | Unread here; no velocity, scale-velocity, reserve or padding interpretation. |
| `0x28` | `f32` sprite frame, initialized zero. | A floating frame index, not a fixed-point word. |

Exactly **26 bytes are read and 18 remain unknown**: the read intervals are
`[0x00,0x06)`, `[0x08,0x18)` and `[0x28,0x2C)`. The camera does not write,
forward or retain this argument pointer. Its local `CameraObjectSegment`
contains incomplete byte views; names such as `pad18` are not proof of padding.

Mickey's
[CircularParticle](../include/game/particles.h#L432) has a matching prefix with
real transform flags, scale velocity, velocity components and texture frame.
Its [update paths](../src/main/particles.c#L1758) consume those fields. However,
standalone [menu](../src/main/menu.c#L1073),
[object](../src/main/objects.c#L3751),
[overlay62](../src/overlays/o062/overlay_062.c#L114) and
[overlay101](../src/overlays/o101/overlay_101.c#L1351) render packets initialize
only the consumed fields, leaving the same interior uninitialized. Shared
offsets establish an access view, not permission to give O60 particle semantics.

## Frame, resource and flag domains

The actual chain is `camDo2DSprite` →
[sprDPset](../src/main/textures_354C8.c#L384). O60 supplies flags zero and alpha
255 with a CPU sprite returned by
[texLoadSprite](../src/main/textures_354C8.c#L547). Its six sprite IDs are
selected through the existing bounded preview-mode path; this audit does not
validate those resources' contents.

`sprDPset` reads an unsigned-byte frame count. It wraps sufficiently large
positive frames by subtracting an integer quotient times that count, clamps
negative frames to zero, then truncates the resulting float to an integer frame
index. Effective render flag `0x40` also turns the fractional part into blend
opacity. The separate resource draw flag `0x40` selects the two-texture command
path, whose next frame wraps or clamps according to the resource. Otherwise the
integer index selects a frame display list. The two `0x40` conditions can differ
when only the explicit flags argument carries that bit.

Keep three flag domains distinct:

- The unused two bytes at **argument `+0x06`**.
- The separate **`s32` render-flags parameter**.
- The loaded **Sprite.drawFlags halfword at resource `+0x06`**, which is ORed
  into the parameter before masking by shared state.

Flags zero at the call does not establish resource flags zero. Positive frame
count, finite representable frame input, coherent texture/display-list tables
and sufficient output capacity remain trusted preconditions. The providers do
not establish malformed-resource handling. The separate
[texAnimateSprite](../src/main/textures_354C8.c#L1030) updates this float domain
by speed × ticks / 60; O60 does not invoke it on the static argument.

## Matrix dependency and actual ABI

The camera provider's outer declaration uses `Gfx **`, `Mtx **`,
`CameraVertex **`, `CameraObjectSegment *`, a byte-pointer sprite view, `s32`
flags and `u8` alpha, returning void. O60's
[local declaration](../include/overlays/overlay_060.h#L192) erases several pointer
types and declares alpha as `s32`. Its observed value agrees, but the declarations
are not a completed compatible C API. A future header cleanup must authenticate
and migrate the actual consumer closure rather than weakening the provider.

For a nonnull sprite, the camera writes one 10-byte vertex and advances that
cursor by 10, writes one 64-byte packed SDK `Mtx` and advances that cursor by 64,
and emits five fixed `Gfx` commands plus the variable sprite sequence. Null
sprite returns without those writes. Capacity and pointer-cell backing are
caller obligations. Float scratch matrices and the packed fixed matrix are
different representations; SDK `Mtx` requires eight-byte alignment. The
[matrix contract](matrix-contracts.md) covers multiplication, fixed conversion
and their remaining FP/capacity limits.

The camera copies the consumed rotations into shared `D_800CEC68`, reverses the
first two signs and sets its translation to zero. Planar scaling may also
include the current camera aspect. `func_8002AE10` reads signed rotations and
translation from this camera scratch, writes a float matrix, and never receives
O60's original argument. Its six trig calls retain destination/source pointers
in a1/a2 and accumulated FP temporaries; each cosine-phase call also needs the
preceding sine call to preserve its angle. The routine has an eight-byte frame
without an ordinary argument-home area. Existing leaf trig bodies meet this
private preservation contract; ordinary C ABI compatibility alone would not
make a replacement interchangeable.

Shared float scratch and render-state globals make reentrancy an unproved
contract. Argument fields are read throughout the call, after some output
writes; arbitrary input/output overlap is not supported by this audit.

## Addresses and lifetime

The static argument and loaded Sprite header are CPU-readable RAM objects.
Their scalar values are consumed synchronously; the argument address is not
emitted into graphics commands. The provider instead emits references to
vertex, fixed-matrix, texture-command and nested display-list storage.

Camera encodes target 32-bit vertex/matrix addresses with explicit KSEG0 bias;
the sprite blend path separately biases texture-command addresses. The
nonblend path uses SDK
[gSPDisplayList/gDma1p](../include/n_audio/gbi.h#L1548), which casts a selected
pointer into a command word without itself proving a generic CPU-to-physical
conversion. CPU pointers, encoded command addresses and microcode/segment
interpretation must remain separate domains in a future provider.

The input need only remain stable through its synchronous reads. Referenced
graphics storage must remain backed through its consumer's completion. The
renderer receives no completion fence or allocation capacity. O60 loads the
sprite lazily, releases it on selection changes or submenu closure, and nulls
its cell. Its separate resource-release function still has a multi-provider
alias/declaration debt. Cache refcounts, immediate versus delayed
[freeing](asset-loading.md#dma-queues-and-lifetime), overlay lifetime and
frame completion must be joined before claiming safety; this audit establishes
neither a corruption claim nor complete lifetime closure.

The next storage question is a genuine complete segment-format/type-flow
witness for the 18 unread bytes. A same-size particle declaration or zero-filled
comparison is insufficient. The unknown region remains borrowed; the separate
Gfx block, label-pointer table, terminal tail and full ordinary-module/native
closure remain outside this audit.
