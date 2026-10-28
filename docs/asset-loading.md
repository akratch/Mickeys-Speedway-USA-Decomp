# Asset-loading contracts

This reference describes the current US loading path for future native work. It
records existing behavior, including unchecked assumptions; it does not define a
new loader or promise that malformed input is supported. The authority is
Mickey's [PI source](../src/main/pi.c), [compression front end](../src/main/gzip.c),
the callers linked below, and a metadata-only reading of the locally supplied US
ROM directory. No asset payload is reproduced here.

## Address and index domains

Keep these domains separate even though the current ABI represents several with
32-bit integers or pointer types:

| Domain | Meaning and current representation |
|---|---|
| Logical file ID | `assetIndex` selects a pair in the top-level directory. It is not a ROM address, a resource type, or a subasset index. |
| File-relative offset | `assetOffset` selects bytes inside that logical file. Callers often obtain it from another file's offset table. |
| Physical ROM offset | `D_86760 + offsets[assetIndex] + assetOffset`; this is the PI device address supplied to `romCopy`. |
| RAM destination | `address`/`ramAddress` is a caller-owned RAM buffer, now expressed as `void *` by the shared section/DMA APIs. Legacy scalar RAM holders convert explicitly at the call boundary. |
| Decoded pointer/offset | A loaded asset may contain serialized offsets that its consumer later relocates relative to its RAM base. These are not automatically host pointers. |

`piRomGetSectionPtr` returns a `u8 *`-typed **ROM offset value**, without copying or
mapping the file into CPU-addressable RAM. The
[audio sequence loader](../src/main/audio_manager_1050.c#L622) uses this value in
ROM-address arithmetic before calling `piRomLoadSection`; its return type is not
proof that callers may dereference it.

The target's scalar and pointer widths come from the o32 ABI, including 32-bit
`long`, as [ultratypes.h](../include/PR/ultratypes.h) explains. Native pointers,
serialized words, ROM offsets and RAM addresses must not be treated as one domain
merely because they occupy the same width in this build.

## Front-end cache extent and valid resource IDs

`initFront` clears 180 pointer entries in the resident front-end cache. Its C
producer in [menu_resources.c](../src/main/menu_resources.c) supplies that
720-byte NOBITS span; the separate loaded-flag storage follows it. Entries
hold textures, sprites, models or objects according to the directory flags.
The shared [declaration](../include/game/menu_resources.h) describes pointer
storage, rather than integer or byte-array aliases.

The US file `0x1A` loaded by `piRomLoad` occupies 256 bytes. The first signed
halfword `-1` follows 126 resource entries. Thus valid directory IDs are
0 through 125, even though the constructor clears 180 cache slots. The
load/free helpers do not independently validate arbitrary IDs. Original
source capacity, dynamic caller bounds and resource lifetime remain separate
obligations; the cache extent alone does not establish them.

## Top-level directory: layout and the boundary discrepancy

[piInit](../src/main/pi.c#L42) copies `[D_86640, D_86760)` into allocator-owned RAM
and stores it in `D_800D2470`. The source view is:

| Field | Location | Interpretation supported by the code |
|---|---|---|
| `fileCount` | First native `u32` | Compared against the supplied file ID; the name alone does not establish the valid domain. |
| `offsets[i]` | Following native `u32` words | Start of file `i`, relative to `D_86760`. |
| `offsets[i + 1]` | Next word | End of file `i`; size is the adjacent difference. |

The source view covers all 71 copied words after the header: 68 offset/end
words and three zero alignment words. This physical backing capacity does not
extend the valid logical file-ID domain.

On the N64 these native directory reads are big-endian. No directory-byte swap is
performed. The increment-and-pointer expression in the source still selects
`offsets[assetIndex]` and `offsets[assetIndex + 1]`; it does not make file IDs
one-based.

A fresh reading of the US directory gives the following **metadata observation**:
its copied extent is `0x120` bytes, the header is 67, offsets 0 through 67 are
nondecreasing, and they describe 67 adjacent intervals, three empty. The terminal
offset resolves to physical ROM `0x1848B70`. Three zero alignment words follow
that terminal offset before `D_86760`. These results follow from big-endian word
decoding and the source's directory/base boundaries; they are not a payload or
compression classification.

This layout supports IDs **0 through 66** as file intervals. However, the PI
routines reject only `assetIndex > fileCount`, so they also admit 67. For that
input the size-based routines subtract the terminal offset from the first zero
padding word, producing a negative signed span. The section/address accessors
can likewise compute an address from the terminal offset without a valid file
interval. Thus the source comparison must not be promoted into a claim that the
header is an inclusive maximum. The US metadata is consistent with a file count,
while the implemented guard has this edge discrepancy. This document does not
repair it or establish whether any reachable caller supplies the boundary value.

Directory monotonicity, a usable end offset, nonnegative lengths, and containment
within the ROM are trusted-data preconditions rather than runtime validations in
these functions. The [ROM layout](../mickey.us.yaml) labels broad storage bands;
those bands are not a replacement for the directory's file boundaries.

## Provider APIs and observed checks

These are the definitions in [pi.c](../src/main/pi.c), not a proposed host ABI.
[pi.h](../include/game/pi.h) now shares all seven PI provider declarations with
the canonical callers. The private overlay14 binding has the same load signature.
RAM destinations are pointer-valued, while file IDs and ROM offsets remain
integer-valued. Provider return types and the retail boundary behavior remain
unchanged; this does not convert serialized fields or legacy address holders into
a host representation.

| API | Result and ownership | Checks actually present |
|---|---|---|
| `piInit(void)` | Creates PI queues/manager, allocates and loads the directory. | No directory allocation-failure check or directory-content validation. |
| `piRomLoad(u32 id)` | New `mmAlloc(size, 0x84)` buffer, returned as `u32 *`; caller owns it. | Rejects `id > header`, zero span and allocation failure. Does not validate negative spans or serialized contents. |
| `piRomLoadCompressed(u32 id, s32 extra)` | New output allocation with tail staging; caller owns its base. | Rejects `id > header` and final allocation failure. No empty-file, scratch-allocation, length, overflow, framing or decompression-result check. |
| `piRomLoadSection(u32 id, void *address, s32 offset, s32 size)` | Copies into caller-owned RAM; allocates nothing. Returns the requested `size`. | Returns zero for `size == 0` or `id > header`. Does not check `offset`, negative size, file containment or destination capacity. |
| `piRomGetSectionPtr(u32 id, u32 offset)` | Returns a ROM address value; no new RAM ownership. | Rejects `id > header`; no offset/end/wrap validation. |
| `piRomGetFileSize(u32 id)` | Returns adjacent-offset difference as `s32`. | Returns zero for `id > header`; no monotonicity check. Zero is also a valid empty span. |
| `romCopy(u32 rom, void *ram, s32 bytes)` | Synchronous-at-return transfer on the normal completion path; no allocation and no status result. | Loops only while `bytes > 0`; does not surface DMA submission or completion errors. |

A `piRomLoadSection` return equal to the request is an echo of the requested
length, not an independently checked byte count. In particular, a negative
request can be returned even though `romCopy`'s transfer loop does not execute.
Its cache-invalidation call occurs before that loop. Normal callers therefore
need valid positive lengths, valid destination storage, and valid initialized PI
state; these are not guarantees established by the section API itself.

## Endianness and per-file tables

`piRomLoad` copies opaque bytes. The returned `u32 *` does not mean every asset is
a word array. Callers choose their own layouts, often reading big-endian native
halfwords/words directly and then converting stored offsets into RAM references.
The compression length reader is a distinct little-endian operation.

Representative caller contracts demonstrate why there is no universal inner
header or sentinel:

| Caller | File/subasset contract visible in source |
|---|---|
| [Model initialization/loading](../src/main/models.c#L90) | File `0x26` is an offset table scanned to signed `-1`, then the count is decremented to exclude its terminal offset. Adjacent offsets select compressed records in file `0x27`. Loading rebases many model fields onto the decoded buffer; it also maintains cache/refcount state. |
| [Screen loading](../src/main/frontend_37680.c#L195) | File `0x14` is another `-1`-terminated offset table with a terminal end offset. Adjacent entries select records in file `0x13`; invalid screen indices fall back to zero. The temporary table is freed after the operation. |
| [Overlay30 initialization](../src/overlays/o030/overlay_030.c) | Files `0x2C` and `0x19` are scanned for signed `-1`; file `0x2E` is viewed as halfwords and scanned backward past zeros. These are consumer-specific conventions. |
| [Language resources](../src/main/menu.c#L275) | File 7 supplies an inner header and language offsets; a selected interval is copied from file 6 into preallocated RAM. Entries equal to `-1` become null, and other entries become base-relative RAM addresses. Its header is separate from the top-level directory header. |
| [Object loading](../src/main/objects.c#L994) | Adjacent offsets in a loaded table select sections of file `0x2D`. A pool allocation, with optional fallback, receives the bytes and then has internal fields rebased. |
| [Texture loading](../src/main/textures_35024.c#L80) | A header flag chooses direct loading or decompression. Compressed texture records have an outer texture prefix: the inflater receives the staged address plus `0x20`, not the record's beginning. Cache/refcount and display-list setup follow decoding. |
| [Sequence loading](../src/main/audio_manager_1050.c#L622) | File `0x32` supplies a section copied into an existing sequence buffer, then passed to the audio player. Loading does not transfer ownership of that caller-supplied buffer. |

A sentinel search in these callers usually has no independent byte bound. A
trusted terminator, a following end offset, suitable alignment, and compatible
field widths are part of that caller's input contract. They do not prove that
arbitrary file contents are safe, or that every offset table has the same count
convention. A future format description must preserve each family's distinction
between file ID, subasset ID, sentinel, terminal offset and relocated pointer.

## Compression framing and allocation

Compression selection belongs to the chosen API and consumer. `piRomLoad`
copies bytes unchanged; model, screen and texture loaders select compressed
records within files rather than imposing one container on every file ID.

[byteswap32](../src/main/gzip.c#L94) assembles four bytes in little-endian order
and returns `s32`. Despite its name, it reads a serialized integer rather than
conditionally swapping a native word.
[func_8004D7A8](../src/main/gzip.c#L104) fetches eight bytes from a file section into
shared scratch and applies that reader. It does not validate the section's span
or report a failed fetch.

The contract visible at the compression front end is:

| Framing component | Established behavior |
|---|---|
| Prefix bytes 0–3 | Read as a little-endian decoded-size value by allocation callers. The decompression wrapper itself does not enforce that size. |
| Prefix byte 4 | Skipped, without interpretation or validation by the C wrapper. Its meaning is not established here. |
| Bytes beginning at `compressed + 5` | Initial input cursor supplied to `gzip_inflate_block`. |
| Output | Starts at the caller's supplied buffer; no capacity/end pointer is passed. |
| Return/termination | The core returns `1 - finalBlock`; the wrapper repeats while nonzero, then returns the original output pointer. This is continuation, not an error or decoded-length result. |

[func_8004D7E0](../src/main/gzip.c#L109) resets shared bit-state globals and sets
shared input/output cursors. The source has Huffman-table construction and named
stored/fixed/dynamic inflate routines, but the core remains the assembly segment
identified in [mickey.us.yaml](../mickey.us.yaml) and
[VERIFIED_ASSEMBLY.md](VERIFIED_ASSEMBLY.md). Those names and the five-byte skip
are insufficient evidence for a general gzip-file API. The C wrapper does not
parse a variable header, validate a checksum, or enforce input/output bounds.
The [core audit](inflater-contract.md) establishes a raw DEFLATE-family block
stream, unchecked stored lengths and reserved block/symbol handling, global
workspace lifetime, and lookup-driven read-ahead. Prefix byte 4 is unused by the
consumer and has no established producer meaning. The fixed-table path also
depends on incoming stack contents. No universal lookahead or safe tail-overlap
margin has been proved. Choose any replacement against these actual behaviors,
not solely against the function names.

For `piRomLoadCompressed`, let `U` be the little-endian prefix size, `E` the
`extraMemory` argument and `C` the directory span. The source computes:

- `totalSpace = U + E`;
- requested allocation size `U + 2 * E`;
- staged input start `out + U + E - C`, with `C` bytes copied there;
- decompression output start `out`.

The double use of `E` is actual behavior, not a simplification to change during
format documentation. An eight-byte temporary allocation is fetched and freed
before the output allocation; its allocation is not checked. Integer overflow,
negative `E`, a span shorter than the initial eight-byte fetch, a staging start
before the allocation, and overwrite of unread compressed input are not checked.
The size word is used to plan memory, not to verify the final output length.

Other consumers choose different staging margins: models include an
`ObjectModel` margin, screens add `0x80` and align the tail input downward, and
textures use their own outer prefix and size/alignment rules. Consequently,
this code does not establish a universally safe in-place decompression contract
for arbitrary streams. Valid overlap and capacity depend on both the decoder and
the asset family's trusted layout.

## DMA, queues and lifetime

[romCopy](../src/main/pi.c#L154) invalidates destination data cache, submits
`OS_READ` transfers of at most `0x400` bytes, and blocks for a completion message
after each chunk. Only after completion does it advance addresses and optionally
call `mainPreNMI`. No decompression or endian conversion occurs in this provider.

[piInit](../src/main/pi.c#L42) prepares a command queue with capacity `0x20` and a
one-message completion queue. The manager is established by
[func_8004D5E0](../src/main/gzip.c#L43); it binds PI events, access serialization,
raw DMA entry points and its worker thread. The
[submission routine](../src/libultra/pidma.c) can fail when the manager is inactive
or its nonblocking command enqueue fails. The
[device manager](../src/libultra/devmgr.c) reports normal completion after the
hardware event, and the [raw provider](../src/libultra/pirawdma.c) programs PI
registers using ROM/device and physical RAM addresses. `romCopy` ignores the
submission return and can consequently block waiting for a message that never
arrives. It also does not validate hardware alignment requirements.

The shared request object and completion queue in `romCopy` provide no visible
per-caller isolation. Treat normal loading as serialized; neither concurrent
transfers nor reentrant decompression are established contracts. These N64 cache,
queue, physical-address and interrupt details belong to the hardware provider,
not to the directory or compression format.

On the successful completion path, a section's RAM bytes are available before
the function returns. Their subsequent lifetime belongs to the consumer: model
and texture caches, relocated interior pointers, audio playback and rendering can
outlive the load call. The loading API does not prove when those users are done.

Full loads return allocator-owned storage; section loads use caller-owned
storage; ROM address accessors return no allocation. Only the original allocation
base is owned: an interior compressed staging pointer is not a second allocation
to free. [mmFree](../src/main/memory.c#L285) may release immediately or enqueue a
delayed free, with reclamation advanced by `mmFreeTick`; a free call does not
promise immediate reuse. The directory is retained by PI state, with no release
path in `pi.c`.

[Main initialization](../src/main/main.c#L484) initializes memory, allocates the
shared `0x2800` Huffman workspace through
[func_8004D750](../src/main/gzip.c#L86), then initializes PI before ordinary asset
loads. That workspace allocation is unchecked and its release is not shown in the
front end. Shared decoder cursors, bit state and workspace make independent
per-stream lifetime or concurrency an unresolved requirement, not a feature of
the existing API.

## Open questions and bounded next checks

| Question | Next source/metadata check |
|---|---|
| Can a reachable caller supply top-level ID 67 or another invalid ID? | Trace each PI entry's call/ORT argument domains, including variable IDs. Keep the observed metadata discrepancy separate from a future input-rejection policy. |
| What does prefix byte 4 mean to the producer? | The consumer skips it; sampled compressed prefixes all contain nine. Trace producer evidence rather than infer a meaning from the value. Framing, continuation and unchecked core behavior are recorded in the [inflater contract](inflater-contract.md). |
| Which tail-overlap and output-size constraints are sufficient? | For one owned caller family, prove its staging interval against allocation bounds and the core's maximum read-ahead/write behavior. This reference establishes no universal margin. |
| Which serialized fields are offsets, indices or native runtime pointers? | Recover one family's bounded header/record schema and every rebase step before defining a native representation. Models, textures, screens and language resources have distinct evidence. |
| What errors and concurrent use must a future provider expose? | Follow manager submission/completion failures and all calls sharing the request, completion queue and decoder globals. Current APIs provide neither a complete error channel nor per-stream state. |
| Which declarations and address views remain outside the PI repair? | PI declarations are shared and its RAM-buffer contract is pointer-valued. Audit compression declarations with differing return/pointer types, scalar address holders and the ROM-address-valued pointer return before defining host interfaces. |

No host implementation, adapter, payload conversion, emulator run or new test
framework accompanies this reference.
