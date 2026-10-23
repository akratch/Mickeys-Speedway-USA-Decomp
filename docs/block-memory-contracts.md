# Block-memory boundaries for future native work

Contract audit at `5b92f8e468b07101e2785dd26507b08bf84bbe54`. The existing linked
`_bcopy`, `_bzero` and `_bcmp` ranges (1,212 bytes total) and complete ROM were
byte-identical. The audit used source, bindings and linked dataflow inspection;
it performed no new build, game execution or host implementation. Accepted
assembly classification and matching credit are unchanged. See the wider
[native boundary map](native-boundaries.md).

## Callable signatures and actual users

The shared [os_internal.h](../include/PR/os_internal.h#L121) declares:

```c
void _bcopy(const void *src, void *dst, int len);
void _bzero(void *dst, int len);
int _bcmp(const void *s1, const void *s2, int len);
```

The plain spellings are macros to these underscored entries, not separate providers. These are target o32 signatures: pointers and signed `int` counts are 32 bits. Counts are **bytes**, not words or an unqualified host `size_t`. The provider code and source callers agree on this width/domain. The five local `_bzero` declarations that used the project's `s32` (`long`) were unified with the canonical signed-int contract in `772a8aef1`, preserving affected object bytes, relocation identities and the ROM.

Fresh source inventory, excluding comments and declarations, finds:

| Provider | Calls | Direct source files | Actual use families |
|---|---:|---:|---|
| `_bcopy` | 20 | 11 | Controller Pak and Game Pak records; audio events/ADPCM state; task copies; debug thread/stack dump; overlay92 state snapshot |
| `_bzero` | 18 | 14 | OS/VI/Pak initialization; flash transaction record; Huffman count array; camera state; runtime linker table; save directory; overlay1/2/36 storage |
| `_bcmp` | 6 | 3 | Game Pak connector/read-ID and Controller Pak ID equality tests |

There are 25 unique caller files overall; the inventory records source sites rather than treating call counts as dynamic extent proofs. All six comparison uses test equality/nonzero, consistent with its actual boolean result. Copy results are ignored under the void declaration. Examples use fixed sizes or `sizeof`; some game clear sizes are computed from directory/count metadata. Per-call dynamic extent/lower-bound proofs were not performed, so this is not a blanket caller safety claim.

The scoped runtime table has nine overlay call imports: four zero calls in overlay1, three in overlay2, one in overlay36, and one copy in overlay92. These resolve to the same measured entries; no compare overlay import was found. Duplicate older automatic ORT labels name those same addresses and supply no additional API. No named assembly caller was found in the inspected generated assembly inventory; arbitrary indirect reachability is not inferred from that absence.

## Count, result and memory behavior

| Provider | Zero / negative count | Positive count and result |
|---|---|---|
| `_bzero` | Any nonpositive count returns without reading or writing pointed-to memory. | Stores zero to exactly the requested destination bytes. No C result; the routine does not assign a meaningful return value. |
| `_bcmp` | Any nonpositive count performs no pointed-to read and returns exactly0. | Returns exactly0 if all requested bytes agree, exactly1 upon a mismatch. It is not a lexicographic difference/sign result. Writes neither input. |
| `_bcopy` | Zero returns without data access. Equal source/destination also returns immediately for any count. A negative count with distinct pointers is **not safely rejected**. | Copies the requested bytes with overlap direction handling under the address preconditions below. On normal machine return, `v0` echoes the original destination, including zero/equal-pointer exits. The declared C API is void and no audited C caller consumes this echo. |

Negative distinct-pointer copy can enter a very large wrapping byte walk or trap during a checked signed address addition. It is not interchangeable with the other two routines' nonpositive no-access paths. No policy guard or new behavior is proposed here. Even with a nonnegative count, callers must supply live readable/writable extents, valid address arithmetic and the target ABI's canonical address/count representation.

For positive `_bcmp`, equal pointers are not a no-access special case: it still reads. Early mismatch can stop before the full range is consumed, but callers cannot infer a safe unread tail without establishing where the mismatch occurs.

## Copy overlap and alignment

The measured `_bcopy` copies forward when destination is below source, or destination is outside the source interval. It copies backward when destination lies strictly inside the upper part of the source interval. This provides the intended overlapping-copy behavior for normal linear RAM in the same virtual address view, with nonwrapping extents and valid checked additions. Address comparisons are signed target comparisons; some endpoint additions are trapping signed operations. Do not silently generalize them to arbitrary host-width integers, cross-region/wrapped intervals or distinct cached/uncached aliases of the same physical memory.

Both paths accept arbitrary byte alignment. Counts below16 and differing source/destination alignment use byte loops. Matching alignment permits a byte/halfword prefix or suffix to reach word alignment, then 32-, 16- and 4-byte groups, finishing with bytes. Loads for each bulk group precede its stores. With valid positive extents, the source data operands cover only requested bytes and destination stores modify exactly those bytes. There are no unaligned merge loads in this provider and no full extra data element is fetched after completion. This says nothing about hardware cache-line transaction sizes.

The overlap promise is about the routine's byte effects, not atomicity. There is no lock, fence or snapshot guarantee if another CPU/DMA agent changes a buffer during the call. The memory is borrowed until synchronous return; nothing is retained, allocated or freed.

## Zero and compare access granularity

`_bzero` accepts arbitrary byte alignment. For sufficiently large counts it uses a leading masked partial-word store, then aligned 32-byte/4-byte groups and a byte tail. The leading mask preserves bytes outside the requested destination interval. It does not use old byte values as computational input. Cache/write-allocate or device read-modify-write transactions are a hardware memory-provider concern, not an extra typed-data guarantee from this routine.

`_bcmp` uses bytes for short ranges. Larger equal-alignment inputs can use a leading partial-word comparison followed by aligned word comparisons; differing alignment aligns the second pointer and uses paired partial loads for the first. A finished loop's next-chunk preload is suppressed. It compares only requested bytes and can stop early at a byte/word/prefix mismatch. It may read later bytes within the same compared word before reporting an earlier mismatch.

A partial load's enclosing word can contain discarded lanes before or after the logical compared span. A word-oriented memory model therefore cannot assume every load is byte-granular at the object boundary; physical cache/bus activity can be wider still. Ordinary RAM stability/readability is the audited domain. No MMIO, byte-observation, inaccessible enclosing-word or generic device-buffer contract was established.

## The reported nonargument read in `_bcmp`

The ledger's read is of incoming **`v1`**, on the large, equally aligned but initially unaligned prefix path. The routine copies that value into the other comparison register before issuing the two same-alignment partial loads. Each partial load overwrites the corresponding prefix lanes and retains exactly the same filler lanes in the two registers.

Write each resulting value as “loaded prefix combined with common filler P.” Equality is independent of P: equal prefixes produce equal combined values, and differing loaded lanes cannot be changed by the disjoint filler lanes. Later comparisons or the fixed return assignments determine the actual result. Thus this is **shared filler for partial loads**, not an additional semantic argument, a saved-register restoration, or a C numerical live-in. `v1` is caller-saved and is not promised to survive; a scratch-register value may depend on its incoming filler without making the data/result contract depend on it. No hidden-register parameter or native API widening is warranted.

## Retained state and confidence

All three are leaves: no calls, stack frame, global workspace, allocation, FP operations, cache control or privileged-state changes. They retain `sp`, `ra`, `gp`, saved integer registers/`fp`, HI/LO, all FP registers and FP control. Worst-path scratch integer sets are:

- Copy: `at`, `v0/v1`, `a0..a3`, `t0..t5`.
- Zero: `at`, `v1`, `a0/a1/a3`; no meaningful C return.
- Compare: `at`, `v0/v1`, `a0..a3`, `t8`.

The provider's count/result/overlap behavior and cancelling filler are high-confidence instruction/dataflow findings against existing exact linked bytes. Stable ordinary RAM, valid extents, ownership for the full call, safe address arithmetic and synchronization remain caller preconditions. Target endian merge/store behavior is accounted for; no typed numeric interpretation of copied bytes is introduced.

The declaration repair covers `main/runlink.c`, overlay1 head/end/tail and `include/overlays/overlay_002.h`. It changes no provider or caller behavior and does not establish dynamic count or buffer bounds. The original provider audit and the later declaration maintenance are separate evidence packets.

This closes the bounded provider-family questions sufficiently for the native boundary map. It does not authorize a host implementation or establish all 44 callers' dynamic bounds, hardware memory behavior, or a full resource-safety claim.
