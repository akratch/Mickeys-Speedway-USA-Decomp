# Overlay structure sprint: source ownership and final build inputs

Status: active, reviewed 2026-10-24. The US executable reconstruction is
complete under the project's matching standard. The remaining campaign makes
its storage, identities and build structure faithful enough to maintain and to
support future native work. Implementing a native port is outside this sprint.

## Current evidence

The source measurements here are pinned to `cfc7c75ca`; the combined borrowing
ledger was freshly recomputed from its bootstrap link and raw objects.
They describe the integrated source and a freshly rebuilt raw-object inventory,
not a promise that all overlay data or interfaces are finished.

Public release `076f239c2` passed a cold byte-identical rebuild, all six editable
layout variants and 2,185 uncached promotion proofs with zero failures or
uncovered targets before landing through `tools/land.sh`. The newer source pin
above passed its combined ROM, structural gates and all 150 tooling test files;
it has not yet completed another release census or landed.

| Area | Proven state | Remaining work |
|---|---|---|
| Executable reconstruction | 943,640 resolved bytes: 445,308 resident C, 29,740 verified original assembly, 468,592 overlay C | Preserve exact compiler output and the accepted assembly contracts during maintenance. |
| Translation units | 104 of 106 nonempty overlays have one text-C owner | Overlay 1 has 11; overlay 65 has two. Finish their source/type/flag work or establish independently justified original boundaries. |
| Initialized storage | 47,520 of 61,312 bytes have canonical projected C owners | 13,792 bytes remain. Another 660 diagnostic/trial bytes earn no physical ownership credit. |
| Initialized images | 37 of 87 data-bearing modules are fully projected to C; ten are partial and 40 have no canonical C owner | Projected ownership and a complete source-module proof are distinct milestones. |
| Source-module linking | 29 modules use ordinary section-relative GNU linking | Retire the remaining per-object scaffolding as actual source owners become available. |
| Relocation comparison | Zero pool-layout or table-entry discrepancies; both packers emit the same region | 7,943 records still borrow authority: 7,825 placeholder references, 114 blob-owned data records and four main-table entries. Layout borrowing remains explicit too. |
| BSS headers | Complete faithful source storage drives the header; an actual compiled 32-to-48-byte growth follows source despite a stale atlas value | 39 header sizes remain borrowed. Unknown reserves and partial allocations are not complete C ownership. |
| Final build inputs | All 198 generated metadata objects feed the final ordinary link; its ELF equals the retained-input bootstrap ELF, and the ROM is exact | The bootstrap still consumes retained metadata. Source-only identities and general relayout remain open; fixed-layout equality does not prove them. |

Source independence also requires reviewing encoded-coordinate names,
retained target storage and borrowed placements within object-produced records.
The direct borrowing counter alone does not measure those dependencies.

Actual held resident function definitions now supply identity for 894 module
call records and 282 ORT rows, including disclosed C and assembly providers.
Nine earlier function-proxy records likewise use actual input definitions;
three retain explicit compatibility authority. These routes preserve all
identities and bytes; they are distinct from fresh source-compilation acceptance.
The three compatibility records are the two overlay 20 guard calls and overlay
7's value-reset call. The guard views still interpret the same runtime owner's
resource word as an integer; their whole owner-family type contract remains open.
A fresh source scan at `cfc7c75ca` finds 25 per-input zero-addend call aliases
used across 81 sites with different runtime providers. Those aliases need actual
provider names and compatible contracts; matching output does not close that
source-quality debt. The 51-entry natural-C cleanup queue remains open too. A current definition
census resolves every queued name to one canonical C definition; it does not
remeasure or dismiss the recorded source forms.

All executable counts exclude new credit for padding or data. A compiler pool
that matches a retained slice does not acquire physical ownership while another
object still supplies that slice. Overlay 8 now has one C owner for its complete 512-byte main pool.
Its separate 64-byte surface-name table uses ordinary linking against actual
named bases in the mutable string input. The bounded cross-input LOCAL guard
preserves complete source relocations and rejects a missing pointer even when
its configured word stays zero. The remaining 720-byte prefix still lacks an
authenticated schema; repeated layouts do not establish its fields or capacity.
Overlay 3's thresholds, retained label and actual search scalar now own all
32 initialized bytes. Its genuine cache/pool now supplies the complete 2,608-byte
BSS extent, including ordinary compiler alignment; two fresh ordinary DATA
inputs carry distinct authenticated runtime roles, so the header no longer
borrows its BSS size. Before-array/cross-row expressions, call/formal agreement,
external group bounds and original declaration spelling still need work.

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
typed C owners: 686 event records plus compiler alignment. Its control state
and two label families now add 3,520 initialized bytes; eight genuine pooled
BSS owners supply 4,040 bytes plus eight compiler-alignment bytes. These retire
118 placeholder references. Presentation-specific root/node views still need
effective-type review. These changes add storage and identity authority, not
executable matching credit.
The remaining 304-byte tail is now source-produced too: three mutable strings
and two real f32 multipliers occupy 44 bytes, and all three compiler switch
tables plus actual literal reads occupy another 260. Source/object/access bounds
prove the removed compiler-end alignment; all 60 original pool relocations
remain available to the identity proof. This closes all 23,904 initialized bytes
without closing the fixed runtime RO origin or the remaining BSS/view contracts.

The shared integer-angle declarations cover 58 of 59 canonical consumers;
overlay 29 retains its explicitly recorded legacy return-width disagreement.
The overlay 8/17 chain pointer and integer flags, the model stack-matrix backing,
and the shared byte-buffer decompression API are repaired with exact output.
Their wider graphics and resource-lifetime contracts remain separate.
Overlay 56 now distinguishes float matrix scratch from its SDK packed-matrix
cursor. Overlay 8 has another 208 initialized bytes of typed selector factors,
record indices and flag tables, plus 192 bytes of constant scales with exact
interior read-only input placement. Its six tuning-getter call sites now share
one bounded 344-byte record type; all 19 affected objects remain exact. Twenty
actual mutable C records now supply all 6,880 bytes of that family, with genuine
sized definitions and exact raw, configured, linked and retail storage. Their
two pointer tables now supply another 80 bytes through genuine C references.
A durable guard authenticates their raw import identities and fresh provider
objects, including a target-swap refusal when configured bytes would be equal.
The packer now consumes those twenty raw pointer relocations with identities
anchored to fresh actual providers. Geometric/runtime index safety and the
remaining pool overlap stay open.

Overlay 7 now owns 2,304 of its 2,384 initialized bytes: the dispatch grid,
both mode grids, sound families, event indices and actual compiler pool. The
consolidated input preserves all previous owners exactly once. Nine genuine
objects supply all 688 BSS bytes, including the ten-halfword override array;
80 initialized bytes, direct backing-object views and traversal contracts remain
open. Its header BSS authority therefore stays borrowed. Separating the mode
word from two handle pointers also removes four placeholder records through
qualified naming; the mode word still belongs to overlay 1's retained BSS
reserve, so this is not four new source-independent definitions.

Overlay 11 now supplies 464 initialized bytes of background geometry, menu
controls and handles, plus its genuine 464-byte BSS allocation. Another 224
initialized bytes and the equal-offset DATA/BSS name ambiguity remain open.
Its eighteen ambiguous setter calls now name fourteen `overlay45SetMode` calls
and four `overlay45SetField22` calls through the provider's compatible opaque
descriptor type; existing qualified setter calls preserve their identities.

Overlay 64 is the twenty-ninth ordinary source-module link. Its two initialized
statics and procedural texture code supply all 1,696 initialized bytes without
the former data-section externalization. The resident four-slot texture callback
array is now typed C too. Metadata-only symbol renaming preserves the original
callee name in a hashed sidecar; ordinary linking supplies the trap pointer.
The main DATA/R32 entry uses the actual sized source function and ORT identity.
Overlay 33 also has an explicit physical loader reserve and one genuine unsized
NOBITS workspace export. Its contents are unreconstructed, the workspace
extent is unknown and BSS-header authority remains borrowed. The four resident
HI/LO records still await named paired-address recovery at
this source pin. This reserve earns no reconstructed C ownership.
A callback-to-NULL experiment rejects stale identity even though raw zero data
bytes are unchanged, because the actual R32 relocation disappears. No instruction
or initializer word is patched. Image allocation bounds, runtime address APIs
and remaining naturalness work are separate obligations.

The shared physics/channel prefix, true integer Powerf update-rate parameter,
six physics getter/setter/point/buffer calls and pointer-volatile command entry
now have compatible actual-provider declarations with exact output. Original
volatile qualification and other opaque owner views remain uncertain. Objdiff
now includes all 42 declared data-only C inputs while retaining separate text
ownership checks; these inputs receive no executable-complete flag or new
executable matching credit.
Overlay 48 is now a complete ordinary source-module link: its 93-entry caption
directory, 92 labels, command stream, five initial seeds and compiler literal
produce all 1,792 initialized bytes. Five real descriptor entries and their
controls produce the 96-byte BSS extent with ordinary alignment. All 147 runtime
identities and four strict function proofs pass. Distinct setup calls use their
actual resident providers. Its resident byte export has an authenticated label
within retained resident data; that label does not establish resident C ownership.
This retires 122 borrowed records and its BSS-size/read-only-base fallbacks.

Overlay 57 now owns 528 initialized bytes of typed option masks, shifts, spawn
positions, mutable rectangles, bounded sprite lists, scalar controls and labels.
Overlay 9 has 192 bytes of parameter and camera storage; overlay 58 has 208 bytes
of result assets, badge nodes, layout and labels. Their wider caller and
source-type contracts remain open. Overlay 50 owns 736 initialized bytes of
cache-index/texture nodes, nine actual time templates, HUD controls, sound-handle
state and pixel advances, including ten ordinary alignment bytes. Its two cleanup
calls now name the separate sound-stop and descriptor-release providers; 160
initialized bytes and its BSS contract remain open.

Overlay 46 owns 656 initialized bytes of genuine asset/state, entry/transform
and trail geometry. Fifteen actual C objects now also supply its entire
10,016-byte BSS allocation, retained in the configured object and the unique
final writable NOBITS input. Unknown initialized regions still total 256 bytes,
so its header allocation remains a listed fallback. The new storage names do
not unify its legacy root, particle, spark/group or placeholder views.

Overlay 59 owns its forty 36-byte frame records;
their forty genuine LOCAL references now produce runtime records directly.
The data-only producer publishes inputs atomically after fresh source/helper/
provider/map checks. Together the overlay 8 and 59 data-only producers retire
sixty hybrid records; remaining inputs and their identity/layout borrowing stay explicit.

Overlay 101 now owns all 113 label targets; its pointer-table prefix and
768 bytes of initialized storage remain unowned. Local block-clear declarations
now use the canonical signed-int count type. Two rumble calls now use the
provider's float third-argument type and actual 0.15f value; the separate
four-argument caller's ABI debt remains explicit. The runtime loader uses seven real linker boundaries
and representable address differences, including distinct header-end and
module-base symbols. None of these repairs proves general source relayout.

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
   The largest canonical initialized deficits are overlay 8 (1,136 bytes),
   overlay 57 (1,104), overlay 58 (864), overlay 9 (848), overlay 101 (768)
   and overlay 1 (704). Overlay 101's 768-byte remainder includes its pointer table, remaining formats
   and small control/compiler-pool tail, and overlay 8's mutable tables/pool
   overlap, need separate owned packets. Unreferenced format strings are actual
   objects, not alignment to be counted away. Asset payloads require an explicit generated binding/format strategy,
   not public extracted-byte arrays.
4. **Remove borrowed identities.** Replace each placeholder and blob-owned
   pointer field with its actual source definition, compatible callback type or
   authenticated export. Resolve the four remaining resident sentinel HI16/LO16
   records in verified assembly as well.
   Delete filters, rebinds, aliases and retained pools only after their genuine
   producer supplies the exact identities and bytes.
5. **Make generation independent of the retained layout.** The final build now
   consumes generated headers, tables, ORT and main table. Remove bootstrap and
   identity borrowing, then exercise real function/data/BSS growth, export and
   module changes through ordinary linking, header chaining and final layout.
   The bounded editable-layout diagnostic now proves actual function/data/BSS
   growth, moved exports, a genuine call into an added tail module, alignment
   gaps and safe unreferenced removal through ordinary final linking. A sixth
   variant compiles a real resident call-target substitution, producing a
   376th overlay-call record and an eight-byte main-table increase while all
   prior 375 records and resident symbol homes remain exact. This is table
   growth with fixed resident code geometry, not arbitrary resident code growth.
   The retained catalog, borrowing, general relayout and source-only generation
   remain open.
6. **Close source contracts.** Reconcile shared declarations against providers;
   recover actual array/object bounds, signedness and effective types; and work
   the current [cleanup queue](cleanup-queue.md). Reconcile the historical
   [naturalness ledger](NATURALNESS_LEDGER.md) against current folded paths
   before reusing its counts or reopening an old experiment. Preserve the
   authenticated retail defect in [ADR 0019](adr/0019-preserve-retail-loop-defect.md).
7. **Extend the JFG demonstration.** Keep the useful
   [runlink kit](runlink-toolkit.md) and its MSU parity. The documented
   [JFG snapshot and six-site extraction pilot](runlink-jfg.md) have now been
   reproduced in isolation, including exact ROM and historical residual reports.
   A complete symbolic-source module now regenerates its baseline and a live
   data edit relays through ordinary final linking, growing its ROM payload by
   24 bytes while all unselected module images remain exact. This bounded
   demonstration retains the other modules and global registry/name authority.
   Next own real initialized data/BSS and its full relocation surface in a
   complete module. Whole-game source independence and general editability
   remain open. Donor
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
views, serialized resource schemas and float/word views still need source-contract
packets. The [animation boundary audit](animation-contracts.md) establishes its
two outer calls, private continuations, mutable scratch and in-place matrix
conversion; resource bounds and render lifetime remain explicit follow-ups.

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
