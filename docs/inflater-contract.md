# Inflater contract and remaining native work

Mickey's decoder accepts a five-byte application prefix followed by a raw
DEFLATE-family block stream. It uses shared state and trusts its input. Its
completion flag is not an error result. These properties come from Mickey's
[front end and Huffman builder](../src/main/gzip.c), the five `gzip_asm` entries
in the [verified-assembly ledger](VERIFIED_ASSEMBLY.md), and a read-only audit
of their ROM-exact linked output at
`390eef6244b678d8ab94cbc8224fd440a51c9f47`.

This is a consumer contract, not a claim of standards compliance or a native
decoder implementation. [Asset loading](asset-loading.md) describes the PI and
allocation callers. No compressed payload or instruction listing is included.

## Framing, blocks and completion

| Component | Established behavior |
|---|---|
| Prefix bytes 0–3 | Little-endian allocation/output-size word, `U`. The decoder does not enforce it. |
| Prefix byte 4 | Skipped by the consumer. Producer meaning remains unknown. |
| Bitstream | Begins at prefix+5. Byte refill appends at the high end of the valid low-bit reservoir; numeric fields consume low bits first. Huffman lookup uses reversed canonical indices. |
| Block header | One final bit, then two type bits. Type 1 selects fixed Huffman, type 2 dynamic Huffman, and both 0 and 3 select stored. Reserved type 3 is not rejected. |
| Huffman termination | Table operation 15 ends the block; operation 16 emits a literal. Output-size `U` does not terminate decoding. |
| Stored blocks | Align to a byte, read 16-bit LEN, discard the next 16 bits without checking its complement, then copy. LEN zero enters the four-byte copy loop with a wrapping counter; it is not an empty-copy path. |
| Core result | `gzip_inflate_block` returns `1 - final`, overwriting inner handler results. The wrapper repeats while nonzero. This is continuation, not success/error. |
| Wrapper result | Returns the original output pointer, with no decoded length. No checksum, trailer, output-size equality or exact input-consumption check follows the final block. |

Dynamic repeat counts, tree validity, reserved symbols, available history and
input/output bounds are trusted. Reserved length operation 99 is not rejected:
operations at least 17 enter subtable traversal. A malformed-input rejection
policy cannot be inferred from the normal format path.

## Entries and memory ownership

| Entry | Actual boundary |
|---|---|
| `gzip_inflate_block()` | Reads/writes shared stream state, resets table allocation per block, returns the continuation flag. |
| `gzip_inflate_dynamic()` | Builds code-length, literal/length and distance tables, then invokes codes. No independent error contract. |
| `gzip_inflate_fixed()` | Builds fixed literal/length and distance tables, then invokes codes. No independent error contract. |
| `gzip_inflate_stored()` | Copies a stored block and publishes cursor/bit state. No independent error contract. |
| `gzip_inflate_codes(literalTree, distanceTree, literalRootBits, distanceRootBits)` | Tree pointers in a0/a1, root widths in a2/a3; publishes state at end-of-block. Its return register is not a defined error result. |

The table entry is eight target bytes: operation at +0, consumed-bit count at
+1, and a union at +4 holding a 16-bit value or 32-bit pointer. Target pointer
width, structure layout and big-endian table fields are separate from the
stream's byte/bit order. The C builder's extras argument is declared `u16 *`
but accessed as a byte vector through `(u8 *)e`; that declaration remains debt.

| State | Lifetime and constraint |
|---|---|
| `D_8007D630` | Persistent `0x2800`-byte Huffman workspace allocated after `mmInit` and before `piInit`. No allocation-failure check or free is shown. |
| `D_800D6AA8` | Count of eight-byte workspace entries. Builder reserves `z+1` per table. Capacity is 1280 entries, unchecked; reset per block, invalidating previous tables. |
| `D_8007D634`, `D_8007D638` | Next physically fetched input byte and next output byte. Handler-local cursors can advance ahead of the published globals. Neither has an end pointer. |
| `D_800D6AA0`, `D_800D6AA4` | Bit reservoir and valid-bit count; reset at stream entry, retained across blocks. They may hold input beyond the last consumed code. |
| `D_800D6A90` | Separate header scratch. The helper fetches eight bytes and interprets the first four as `U`; this is not decoder history. |

History is the actual output buffer. Backreferences copy forward byte by byte,
so overlapping runs, including distance one, intentionally reuse newly written
output. No output-base/history check enforces a valid distance. Workspace and
stream globals make the decoder non-reentrant. Returned asset storage belongs
to the caller; the persistent table workspace has an independent lifetime.

The fixed handler has a hidden incoming-memory dependency: it stores requested
widths seven/five in different stack cells from those passed to the builder.
The passed cells are not initialized in that routine. Builder clamping yields
literal width 7–9 and distance width five, so literal lookup geometry and
read-ahead can depend on previous stack contents. A translation must resolve
this dependency explicitly; neither an arbitrary seed nor uninitialized C
reads express an established replacement contract.

## Read-ahead and staged overlap

A table lookup refills to its root/subtable width before discovering how many
bits the selected symbol consumes. It can therefore fetch beyond the logical
end of a short final code. No universal tail allowance has been proved. The
decoder does not rewind or return semantic input consumption.

Tail staging is safe only if output stores preserve bytes still needed by
future input fetches. For monotonically advancing output-before-input staging,
each store being below the next unread input byte is sufficient. Use the
handler-local physical cursor for that reasoning: the global cursor can be
stale, while already fetched bits survive in the reservoir. Initial separation
or a record fitting its allocation does not prove the property throughout a
decode. Output history must remain intact too.

Let `B` be the output/allocation base, `C` the record span and `A` its staged
base. These are actual caller geometries, not universal safe margins:

| Caller | Allocation and staging |
|---|---|
| Generic compressed PI load | Allocate `U + 2E`; `A = B + U + E - C`; start stream at `A + 5`. |
| Model | Allocate `U + 0x80`; `A = B + U + 0x80 - C`; start at `A + 5`. |
| Screen | Allocate `U + 0x80`; round `B + U + 0x80 - C` down to 16 for `A`; start at `A + 5`. |
| Texture | `L = max(U,C) + 0x20`; allocate `L + 0x90`; stage from `B + L - C` using target signed-remainder alignment. Prefix at `A + 0x20`, stream at `A + 0x25`. |
| Overlay 35 track | Fixed `0x9F000` allocation; stage from its end minus `C` with signed-remainder alignment; stream at `A + 5`. This caller does not consult `U`. |

PI finishes the transfer before decoding. The inflater itself has no DMA,
callback refill or queue. Overflow, destination capacity, actual output length,
lookahead and each intermediate overlap frontier remain caller preconditions.

## Corroboration and bounded next work

A metadata-only US audit examined 5221 model, screen, track and texture records;
5171 selected compression and all had prefix byte 4 equal to nine. All sampled
spans were positive and inside their data asset, and compressed `U` values were
positive below `2^31`. All 221 model spans fit `U + 0x80`. No payload was decoded.
The consistent nine is a producer convention; it does not identify compression
level, method, version or a validation byte.

Before choosing a native decoder, settle these bounded questions:

- Derive lookup read-ahead bounds, including the fixed-handler memory dependency.
- Privately measure true output length, consumed input, workspace peak and the
  minimum overlap frontier per record. Metadata checks alone establish none of
  these dynamic properties.
- Trace producer evidence before assigning byte 4 a meaning.
- Reconcile wrapper declarations: its provider returns `u8 *` and takes byte
  pointers, while some callers declare `void(void *, void *)`. Observed callers
  ignore the result; that does not make the declarations compatible.

These are format, ABI and lifetime tasks. They do not require a new decoder,
game execution, another matching census or a general test framework.
