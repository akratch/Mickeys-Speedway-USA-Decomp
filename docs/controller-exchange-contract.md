# Controller challenge exchange: current source contract

This bounded record covers the already-C exchange in resident `joy.c` and
overlay39, plus overlay18's startup caller. The compatible declaration repair
is `d980981439982276b4d56f4f3a6dc74f7889a32f`. It complements the
[native boundary map](native-boundaries.md); it supplies neither a native
implementation nor a cryptographic interpretation of the payload.

## Current API and byte intervals

[game/joy.h](../include/game/joy.h) shares the provider declarations with the
callers. `joyInit` returns `s32`; its startup result is stored in the existing
`s32 D_8007A1D4`. `arithmeticFunction` takes two byte pointers and returns void.
The overlay18 response declaration is an incomplete `u8[]`, so passing it uses
ordinary array decay. An incomplete declaration establishes no allocation or
capacity.

The following intervals come from the actual loop bounds in
[overlay39](../src/overlays/o039/overlay_039.c), not from an inferred buffer size.
They are required accessible spans, not backing-allocation proofs.

| Operation | Source-visible access or request |
|---|---|
| `overlay39Write(source, mq)` | Reads `source[0..16)`. Writes all 64 bytes through the shared pointer `D_8007A204`: 46 fill bytes, two command bytes and the copied 16-byte payload. Writes back that 64-byte cache range and requests SI DMA with direction argument 1. |
| `overlay39Reset(mq)` | Invalidates the shared 64-byte cache range and requests SI DMA with direction argument 0. The C body does not itself copy a response. |
| `overlay39Read(destination)` | Reads shared scratch bytes `[48..64)` and writes `destination[0..16)` in increasing index order. |

Both DMA-start wrappers acquire and release SI access around their requests.
Their current queue formal is unused by the bodies. Neither wrapper waits for
completion internally. A byte-copy loop does not establish general overlap
safety, and this record does not infer a scratch allocation from its pointer.

## Ordering and ownership

The [resident orchestrator](../src/main/joy.c) performs this exact sequence:

1. Receive a blocking message on `D_800CF340`.
2. Copy the challenge into shared scratch and request the outgoing transfer.
3. Receive another blocking message.
4. Invalidate shared scratch and request the incoming transfer.
5. Receive a third blocking message.
6. Start the next controller read, then copy the 16 response bytes to the caller.

The source ignores the called operations' return values. Message provenance,
error handling and exclusion of competing transfers remain provider/resource
obligations. Do not replace this sequence with a synchronous host copy merely
because the function returns void. The challenge is copied before DMA; the
shared scratch, rather than the original challenge pointer, is passed to DMA.
Scratch validity and reuse therefore need their own completion contract.

Overlay18 calls the exchange before unloading overlay39. Its separate 16-byte
protocol input remains retained. The response's existing address identity also
remains borrowed; the new incomplete declaration does not define its storage.
Original capacity, backing ownership, other readers, concurrent access and
unload/reload lifetime are separate questions. No response-buffer definition or
reference retirement is credited by the type repair.

## What the matching proof establishes

The declaration repair preserves the meaningful compiler/object surfaces of
all 16 affected translation units through four captured stages. The only
allowed original-object symbol change is overlay18's undefined response size
from 1 to 0. The final linked symbol multiset has no exception. All six strict
function proofs, the full ROM and the ordinary metadata checks pass.

That establishes target output and compatible current declarations. Native
work still needs actual buffer extents, permitted aliasing, SI/DMA/cache and
message semantics, scratch alignment and lifetime, and a principled
representation of the opaque protocol input. Pointer spelling and retail-byte
identity do not establish those resource contracts.
