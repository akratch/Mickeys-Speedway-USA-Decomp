# Overlay structure sprint: source ownership and final build inputs

Status: active, reviewed 2026-10-23. The US executable reconstruction is
complete under the project's matching standard. The remaining campaign makes
its storage, identities and build structure faithful enough to maintain and to
support future native work. Implementing a native port is outside this sprint.

## Current evidence

The measurements here are pinned to `962980e79`.
They describe the integrated source and a freshly rebuilt raw-object inventory,
not a promise that all overlay data or interfaces are finished.

| Area | Proven state | Remaining work |
|---|---|---|
| Executable reconstruction | 943,640 resolved bytes: 445,308 resident C, 29,740 verified original assembly, 468,592 overlay C | Preserve exact compiler output and the accepted assembly contracts during maintenance. |
| Translation units | 104 of 106 nonempty overlays have one text-C owner | Overlay 1 has 11; overlay 65 has two. Finish their source/type/flag work or establish independently justified original boundaries. |
| Initialized storage | 26,432 of 61,312 bytes have canonical projected C owners | 34,880 bytes remain. Another 840 diagnostic/trial bytes earn no physical ownership credit. |
| Initialized images | 34 of 87 data-bearing modules are fully projected to C; three are partial and 50 have no canonical C owner | Projected ownership and a complete source-module proof are distinct milestones. |
| Source-module linking | 27 modules use ordinary section-relative GNU linking | Retire the remaining per-object scaffolding as actual source owners become available. |
| Relocation comparison | Zero pool-layout or table-entry discrepancies; both packers emit the same region | 8,516 records still borrow authority: 8,116 placeholder references, 395 blob-owned data records and five main-table entries. Layout borrowing remains explicit too. |
| BSS headers | Complete faithful source storage drives the header; an actual compiled 32-to-48-byte growth follows source despite a stale atlas value | 41 header sizes remain borrowed. Unknown reserves and partial allocations are not complete C ownership. |
| Final build inputs | Guarded generation emits 198 metadata binaries, 162,784 bytes, with bound input/output hashes and explicit borrowing | The final build at this pin still consumes retained main/ORT/header/relocation binaries. Switching those inputs and proving full relayout remain open. |

All executable counts exclude new credit for padding or data. A compiler pool
that matches a retained slice does not acquire physical ownership while another
object still supplies that slice. In particular, overlay 8's restored pool and
its existing data-only owners need reconciliation before full data integration.

The full-function promotion census is the final release gate, in addition to
ROM, provenance, documentation, symbol, scoreboard and tooling checks. Focused
receipts are not a substitute for that completed release check. Integrate exact
units through `campaign/unchain` and publish with `tools/land.sh`.

## Completed structural and source repairs

The fold batches, including overlays 73–90 and normalization of 91–107, are
integrated. Overlays 47 and 82 no longer have the old per-function split. Module
end padding is derived, static procedure symbols are restored from compiler
metadata, and the obsolete `--force` steps are gone. Imported zero bindings have
private names, preventing them from impersonating real function definitions.
Compiler-recorded file-scope BSS anchors restore identities without inventing
allocation sizes. Generated aliases and the overlay graph have freshness gates.

The compiler's own pools now agree with retail: overlay 1's missing literals,
overlay 8's 21 read-only scalar loads, overlay 11's release cases and overlay
101's dispatch/sentinel cases were corrected in C. Their existing executable
bytes remain exact. Seven overlay 8 site specifications became one guarded
section anchor; no instruction words are edited.

Source-quality repairs include the two actual `Powerf` exponents, the shared
front-end command/cursor boundary, bounded overlay 47 scroll storage and address
encoding, and the complete six-byte shadow angle record. These close specific
contract errors; they do not imply whole-program type or bounds closure.

Overlay 36 now owns its callback/state tables, literal pool and actual BSS;
cross-overlay imports authenticate unique bounded source NOBITS exports.
Overlay 75 has ordinary compiler literals, actual five-slot state and compatible
positional-audio calls. Overlay 101's clock storage and all 19 event streams are
typed C owners: 686 event records plus compiler alignment. These changes add
storage and identity authority, not executable matching credit.

The shared PI declarations now distinguish RAM buffers from integer ROM offsets
and indices. The physical directory view includes all copied words, preserving
the retail ID-67 boundary discrepancy documented in
[asset loading](asset-loading.md).

## Remaining campaign, in dependency order

1. **Keep proof and ownership truthful.** Rebuild raw objects after integrations,
   review every ratchet delta and preserve explicit borrowing. Canonical C data
   totals exclude inactive trial and externalized declarations. Complete source
   and linked-input evidence must precede retiring a reserve or placement.
2. **Finish overlays 1 and 65.** Recover actual storage identities and C
   definitions, then revisit their remaining TU/flag constraints. A shared
   synthetic VMA or repeated placeholder spelling is not an object identity.
   Cross-overlay BSS imports need real bounded NOBITS definitions; a zero-valued
   alias is not a substitute. A header-sized unowned reserve earns no C credit.
3. **Recover initialized and BSS families.** Prioritize bounded tables, records,
   strings, geometry and callback families, not opaque copies of whole modules.
   The largest canonical initialized deficits are overlay 8 (9,040 bytes),
   overlay 101 (4,416), overlay 7 (2,384), overlay 59 (1,936), overlay 48 (1,792)
   and overlay 57 (1,632). Overlay 101's remaining pointer table, mutable labels,
   control state and compiler pool, and overlay 8's mutable tables/pool overlap,
   need separate owned packets. Asset payloads require an explicit generated binding/format strategy,
   not public extracted-byte arrays.
4. **Remove borrowed identities.** Replace each placeholder and blob-owned
   pointer field with its actual source definition, compatible callback type or
   authenticated export. Resolve the five resident main-table targets as well.
   Delete filters, rebinds, aliases and retained pools only after their genuine
   producer supplies the exact identities and bytes.
5. **Make generation authoritative.** Feed generated headers, tables, ORT and
   main table into the final build. Exercise a real function-size change and a
   BSS-size change through compilation, table generation, header chaining,
   exports and final layout. The existing BSS growth experiment and codec
   relayout test are groundwork, not this complete end-to-end result.
6. **Close source contracts.** Reconcile shared declarations against providers;
   recover actual array/object bounds, signedness and effective types; and work
   the current [cleanup queue](cleanup-queue.md). Reconcile the historical
   [naturalness ledger](NATURALNESS_LEDGER.md) against current folded paths
   before reusing its counts or reopening an old experiment. Preserve the
   authenticated retail defect in [ADR 0019](adr/0019-preserve-retail-loop-defect.md).
7. **Reproduce the JFG demonstration.** Keep the useful
   [runlink kit](runlink-toolkit.md) and its MSU parity. Reproduce the documented
   [JFG snapshot](runlink-jfg.md) in isolation and report its actual unresolved
   categories. A decoder round trip is not a self-hosting source linker. Donor
   reference pins and their checkout drift are separate from that demonstration;
   a digest refresh is not a new donor result.

Each packet has disjoint ownership, a proved baseline, a bounded hypothesis and
an exact commit or explicit plateau. Apply ADRs 0017, 0018, 0020 and 0021; do not
replace an eliminated mechanism with an unbounded permutation sweep.

## Native-start boundary

A matching target is not automatically a portable C program. The remaining work
must leave a concrete interface map for the verified assembly, scheduler/cache/
PI and RSP/RDP providers, serialized assets, word widths, byte order, pointer
rebasing and lifetime. The [verified-assembly ledger](VERIFIED_ASSEMBLY.md)
remains a completed matching result: its 107 routines need future replacement
contracts, not a new assembly-matching campaign or premature host implementations.

The [native boundary map](native-boundaries.md) partitions the 107 assembly
routines into ten resource/ABI families and identifies specific bounded audits.
The [asset-loading reference](asset-loading.md) records actual directory, PI,
DMA, ownership and caller contracts. The [inflater audit](inflater-contract.md)
establishes the stream dialect, continuation flag, shared workspace and unchecked
edge paths; producer byte-4 meaning, fixed-handler incoming memory, lookahead
bounds and dynamic output/overlap limits remain open. Broader cursor extern
views, compression declarations, serialized resource schemas and float/word
views still need source-contract packets.

The native-start exit is an explicit, source-backed ABI/resource/hardware map
and a decomp whose C identities and storage are trustworthy. Implementing host
rendering, audio, input, timing, DMA or portable assembly replacements belongs to
the later port project.

## Structural exit criteria

- One justified translation-unit layout per module, with compatible shared
  headers and evidence for every retained flag exception.
- Initialized data and BSS supplied by actual source producers; no remaining
  overlay `data_rodata` binary owner, placeholder identity or hybrid data record.
- Generated relocation tables, headers, ORT and main table used by the final
  build with zero borrowing, exact retail output and a real relayout proof.
- Obsolete overlay post-processing and alias machinery retired. Ordinary module
  linking and compiler-metadata restoration remain documented format/toolchain
  operations, not instruction editing.
- Current source-quality findings resolved or supported by explicit evidence,
  with preserved retail behavior and no hidden source-shaping diagnostics.
- Reproducible tooling and a bounded, accurately reported JFG demonstration.

The following probe is historical evidence from before these integrations. Its
counts, file layout and descriptions of missing gates refer to 2026-10-21;
use the pinned current measurements above for scheduling and reporting.

## Historical probe: 2026-10-21

- **The format is shared with JFG** and fully decoded by
  `tools/overlay_tables.py`. The loader, `src/main/runlink.c`, is matched C.
- **The encoding rules a packer must follow** were counted over all 18,542
  module records:
  - SYMBOL vs JUMP is decided by the callee's export status: all 618 JUMP
    targets are unexported.
  - LOCAL bases are section starts, with at most four per module.
  - Every HI16 is immediately followed by a LO16; 46 LOCAL LO16 records
    stand alone.
  - Table 1 holds the targets outside the module (6,943 records) and table
    2 holds the self-references (11,599).
  - 47 HI16 sites store a non-zero high half.
- **Record order is the compilers' own relocation order.** A probe compiled
  all 649 overlay sources raw (no POSTPROCESS) and rebuilt every module's
  records from their ELF relocations.
  - The emission order is each object's `.rel.text` in IDO's order, with
    objects in atlas order, then `.rel.rodata`, then `.rel.data`, each
    ascending.
  - Table 1 is the subsequence whose target lies outside the module; table 2
    is the rest. Data-site records appear only in table 2, after all text
    records.
  - That rule reproduces the 85 unsorted shipped tables, so a packer that
    sorted by site would get all 85 wrong.
  - Concatenating per-function objects reproduces one-TU order, with no
    cross-function exceptions.
  - The only exceptions are 9 single HI/LO pair transpositions (18 records;
    o001, o014, o015, o058 twice, o059, o060, o065, o101). Our source
    evaluates those addresses in a different order from the original, so
    the fix belongs in the source.
- **Verdicts by module:** 89 of 106 regenerate exactly, 3 match as a
  multiset, and 14 differ.
  - Of the shipped 18,542 records, the ELF produces 18,090, plus 1 extra:
    a C function pointer in o031 where the shipped word has no relocation.
  - The 452 missing records, in 13 modules, are all R_MIPS_32 pointer words
    inside data blobs that no C object owns yet.
- **Converter quirks the packer must reproduce:**
  - A JUMP record's `symbolIndex` is a stale value: it always equals the
    index of the most recent SYMBOL record in emission order, table 1 then
    table 2 (618/618).
  - LOCAL bases follow the layout [text][data][rodata][bss]: `.data` is
    based at the text size and `.bss` at text + data, both from the header.
    48 modules also need a separate rodata base, which the header does not
    carry.
- **JUMP means a static callee.** In the one-TU trial, o030 became
  byte-identical once its one callee was made `static`. Our per-function
  files currently force every callee extern, so all 618 JUMP sites are named
  globals and JUMP versus SYMBOL cannot be derived from today's objects.
- **72.7% of record identities are placeholders today.** 13,152 records name
  `*Reloc` or `D_xxxx` symbols whose target exists only in the shipped
  tables.
- **182 resolvable references name the wrong target while the bytes still
  match**, because a SYMBOL call stores 0 whatever it names:
  - 96 calls name the module's own offset-0 function;
  - 31 name the wrong resident callee, 28 of them by table offset, 0x450
    short of the real address;
  - 44 resident data references use the wrong base or offset;
  - 10 const objects are placed in `.data` instead of `.rodata` (o077);
  - 1 exported function is reached by a JUMP.

  The packer comparison now checks these identities; this was a gap at the probe date.
- **Main table.** Its 370 R26 entries are exactly the `TrapDanglingJump`
  call sites in the resident objects' `.rel.text`, in link order (370/370).
  The two HI/LO pairs, from an asm object, follow by ascending site, and the
  one R32 DATA record comes last. Twelve address-of-`TrapDanglingJump` HI/LO
  relocations, in `runlink.c` and `fx.c`, must be excluded.
- **Module structure.**
  - All 106 module text sizes are multiples of 16.
  - Padding appears only at module ends (26×12, 30×8 and 30×4 bytes).
  - No module has an interior alignment gap.
  - Each module is therefore one IDO translation unit, with its trailing
    pad counted in the header's text size.
  - JFG's 153 overlay text segments are also 16-byte multiples.
  - The only source path in overlay data is `clone/clone.c`, in module 43.
- **Flags are almost uniform.** One set,
  `-Wab,-r4300_mul -O2 -mips2 -32`, reproduces 636 of the 641 linked
  overlay objects. Rare most likely compiled every overlay with
  `-Wab,-r4300_mul`.
  - Only five objects differ, all on loop unrolling: o039, o046, o065 (two)
    and o082. Overlays o065 and o082 carry a real conflict between two
    unroll settings.
  - In the one-TU trial, 9 of the 70 multi-file overlays compiled once their
    files were simply `#include`d together; the rest failed on duplicate
    type definitions. All 9 produced `.text` of exactly the module's size,
    trailing pad included. o069, o091 and o100 were byte-identical as they
    stood.
- **Data.** 56,412 of 61,312 initialised overlay bytes are blobs, and BSS
  (77,680 bytes) has no C owner.
- **The ORT** has 2,004 rows and no names in MSU's ROM; 844 rows are
  referenced by no relocation.
