# Overlay structure sprint: from matched code to Rare's structure

Status: planned 2026-10-21, revised the same day with the regeneration probe's results. Every overlay function is matched C and the ROM
rebuilds byte-identically, but the overlays are not yet in Rare's structure.
The relocation tables, headers, ORT and main table ship as ROM blobs. Most
initialised data is a blob. The code is split into 649 files where Rare had
one per module. A layer of per-object post-processing reconciles the
difference. This sprint removes that gap, so the overlay build derives
everything from source and the result can be handed to the JFG decomp as a
reference implementation of Rare's runlink format.

## Definition of done

1. **One translation unit per module.** Each of the 106 non-empty modules
   is built from `src/overlays/oNNN/overlay_NNN.c` with a single flag set,
   over shared headers (ADR 0006). The evidence for that target is below.
2. **Data and BSS owned by C.** No overlay `data_rodata` `bin` subsegment
   remains. Literal pools and jump tables are emitted by the compiler, and
   BSS offsets come from C definitions.
3. **Real names in C.** No `*Reloc` placeholder identifiers in overlay C,
   and no `TrapDanglingJump(...)` spelled in resident C (lane hk-trapnames
   covers the latter).
4. **Tables are build output.** A packer emits every module's header,
   table 1 and table 2, plus the ORT and the main table, from the linked
   objects. A gate requires byte equality with the shipped tables, and no
   overlay table `bin` subsegment remains.
5. **No overlay post-processing.** `config/postprocess-audit.us.json` lists
   no overlay object with a POSTPROCESS recipe. The overlay half of
   `overlay_undefined_syms.us.txt` and the per-module alias machinery are
   retired.
6. **Shiftable.** A test changes an overlay function's size in a scratch
   tree and confirms that the packer's tables, the header chaining and the
   loader's invariants all follow.
7. **Upstreamable.** The decoder, packer and comparison tools are packaged
   with documentation and synthetic tests, for contribution to JFG, and are
   demonstrated against JFG's own build.

`gmake verify` holds after every commit, as always.

## Evidence base (measured 2026-10-21)

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

  A byte-equal packer gate catches all of these. Today nothing does.
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

## Phases

### Phase 0: housekeeping and probe (probe done; lanes in flight)

- **hk-altered.** The 9 "altered" objects are re-expressed as metadata-only
  steps, and the audit is wired into credit.
- **hk-surface.** Surface conflicts fail closed. Generated rename rules
  replace in-place object mutation, so a fresh clone builds on its own.
  Vestigial atlas rows and inert renames are deleted, and stale docs fixed.
- **hk-trapnames.** Resident C calls overlay functions by their real names,
  which are renamed to the trap at object level and checked against the main
  table. This gives the packer the resident call graph it needs for the main
  table.
- **Probe (done).** Its results are recorded under Evidence above. The
  scripts live in session scratch. Phase 1 turns them into tracked tools.

Exit: all three lanes integrated.

### Phase 1: identities, then the packer (about 5 days)

The probe shows that sites, types, order and the table split already come
out of the objects. What the objects lack is identity, so this phase restores
identity first and then emits the tables.

1. **Real names.** Replace the placeholder identities: 1,573 `*Reloc`
   identifiers across 371 files, plus the `D_xxxx` absolute placeholders.
   Use the symbol each one stands for, from the surface's
   generated-identity → name mapping.
2. **Fix the wrong targets.** Correct the 182 references whose names resolve
   to a different target than the shipped record. Each fix is a source edit
   that leaves the ROM identical. The packer gate is what proves them
   correct.
3. **The export set.** Exported means having an ORT row. Until phase 2 makes
   callees `static`, the packer takes the static/export distinction from the
   ORT name list: a same-module call to an unexported target becomes JUMP.
   After the fold, `static` in the C carries it directly.
4. **`tools/runlink_pack.py`:**
   - **Inputs.** Each module's raw objects in atlas order, the resident
     objects, and a tracked ORT name list (`config/runlink-ort.us.txt`: row
     index → symbol name, names only).
   - **Records.** Each ELF relocation becomes a Rare record using the
     measured emission order. Table 1 is outbound targets, and table 2 is
     the rest, with data records after text records. The stale `symbolIndex`
     rule applies to JUMP.
   - **LOCAL bases.** `.data` is based at the text size and `.bss` at
     text + data. A per-module rodata base comes from the C object's
     `.rodata` start, for the 48 modules that need one.
   - **Headers and main table.** Headers come from section sizes, text
     including its pad, and from the init/resume symbols. The main table
     takes the resident trap call sites in link order, then the asm HI/LO
     pairs by site, then the DATA record. Address-of-`TrapDanglingJump`
     relocations are excluded.
   - **Hybrid mode.** For the 13 modules with blob-owned pointer data, the
     packer takes those 452 data records from the shipped table, and each
     such module stays flagged hybrid until phase 3.
5. **Gate: `gmake check-runlink-tables`.** Packer output must be
   byte-identical to the shipped tables for every module, the ORT and the
   main table. It runs in `tools/gates.sh --promotion` and `land.sh`. Then
   switch the build: the atlas stops emitting table `bin` subsegments, and
   the link takes the packer's output.
6. **Source fixes for the 9 HI/LO transpositions**: o001, o014, o015, o058
   twice, o059, o060, o065 and o101. Reorder the address evaluation to match
   the original.

Exit: tables are build output for all 106 modules (13 hybrid), the gate is
green, and no placeholder names remain in overlay C.

### Phase 2: one translation unit per module (about 1 week)

The probe removed most of this phase's risk. One flag set
(`-Wab,-r4300_mul -O2 -mips2 -32`) reproduces 636 of the 641 objects, so
folding is mostly type de-duplication. The work is per-overlay lanes in the
TU-batch shape, integrated in batches of about ten with `tools/integrate.sh`.
Each lane:

1. Folds the module's files, in atlas order, into `overlay_NNN.c` plus one
   header. Private typedefs and duplicate externs collapse into shared
   definitions (ADR 0006; `docs/overlay-consolidation.md`). Duplicate type
   definitions are what stopped 61 of 70 overlays compiling as a plain
   concatenation.
2. Makes every unexported function `static`. JUMP then follows from the C,
   as the o030 trial showed.
3. Builds with the single overlay flag set and deletes the module's trims,
   renames and rebinds. Done means no POSTPROCESS recipe for that module.
4. Proves the result: `gmake verify`, `check-runlink-tables`, and
   promotion-proof for every function in the module.

Order of work:

- **o069, o091 and o100:** already byte-identical as one file.
- **The other single-file modules and the uniform-flag modules.**
- **The five loop-unroll objects:** o039, o046, o065 (two) and o082. The
  o065 and o082 conflicts are matching targets, to be re-spelled until they
  match under one setting.

Exit: 106 single-file modules, one overlay flag set, and zero overlay
POSTPROCESS recipes outside the data-ownership remainder.

### Phase 3: data and BSS into C (about 1–2 weeks, overlapping phase 2)

The work is per-module, after that module's fold:

1. **Rodata.** Delete the externalized copy. With the TU whole and in order,
   IDO emits the jump tables, float literals and string pools itself.
   - Three C switch tables currently disagree with the shipped entries, and
     post-processing hides it today: o011 ReleaseCurrentGroup (4 of 6
     entries), o101 DispatchActive (14 of 16, the same targets permuted) and
     o101 DispatchEvents (1 of 19).
   - Each is a case-order or case-value correction in the source.
   - o001_tail's table sits 4 bytes away from the rest of its rodata;
     resolve it in the fold.
2. **Initialised data.** Define tables, strings and pointer tables in C,
   with types. The 452 missing R32 records are the pointer fields:
   - 267 point to rodata;
   - 91 to data;
   - 38 to static functions;
   - 56 to exported functions.

   Also remove the o031 initializer that is a function pointer where the
   shipped word has no relocation.
3. **BSS.** Defining BSS in C makes the LOCAL BSS-base offsets fall out of
   the definitions. Watch for COMMON ordering: IDO emits uninitialised
   globals as COMMON, so control the order with `static` or explicit
   definitions where the layout requires it.
4. Clear the module's hybrid flag in the packer.

Exit: no overlay `data_rodata` `bin`, and no hybrid modules.

### Phase 4: retire the scaffolding (about 2 days)

- Remove the overlay half of the surface: value lines, alias lines and the
  `_oNNNReloc` renames. Remove the atlas `TEXT_SUBSEGMENTS` rows beyond one
  per module, and the overlay rebind, filter, externalize and trim spec files.
- Add the shiftability test.
- Update `docs/overlays.md`, `docs/reloc-surface.md`, ADR 0006 (to
  "Implemented"), CLAUDE.md's overlay promotion recipe, `docs/LANE_BRIEF.md`
  and the README scoreboard text.

### Phase 5: package for JFG (about 3 days)

Status 2026-10-22: the kit is `tools/runlink/` (`docs/runlink-toolkit.md`);
`gmake check-runlink-kit` holds it to `tools/runlink_pack.py`'s exact result.
The JFG demonstration is `docs/runlink-jfg.md`.

- Package a self-contained `runlink` toolset: decode, pack, compare and
  census. It needs no MSU-specific paths and takes configuration for ROM
  offsets, module count and ORT names. Include docs and synthetic tests.
  MSU is CC0.
- **Demonstrate it on JFG.** Run the packer over JFG's current overlay
  objects (assembly, in hybrid mode) and compare with JFG's shipped tables.
  Measure how much of JFG's tables regenerate today, and what JFG needs for
  each remaining category. JFG's ROM ships ORT names, so its name list comes
  from the ROM.
- Write the format and rules document, extending the notes already shared,
  with the measured rules, the order rule and the single-TU evidence.

## Gates added by this sprint

| Gate | Checks | Runs in |
|---|---|---|
| `check-runlink-tables` | Packer output equals the shipped tables, byte for byte | `gates.sh --promotion`, `land.sh` |
| altered-object credit | No credited object is classed `altered` | `check-docs` or `check-scoreboard` (hk-altered) |
| surface conflicts | `reloc_surface.py generate` fails on unresolved values | `overlay-syms` (hk-surface) |
| trap-name check | Each resident trap site's main-table target equals the name the C uses | hk-trapnames |
| shiftability | Tables follow a size change | `check-tooling` (scratch build) |

## Risks and fallbacks

- **The order rule meets a new exception.** The probe confirmed it across
  every module, apart from 9 single-pair HI/LO transpositions with a source
  cause.
  - If a fold or a data conversion surfaces a different exception, treat it
    as a source finding first.
  - Only if no source fix exists, record a per-module order override
    (offsets only, generated by the decoder) and document it in the format
    write-up.
- **A wrong-target fix changes behaviour.** The 182 fixes rename symbols
  without changing any byte, but the C then names a different function or
  object. Give each one a quick semantic check that the new callee's
  signature fits the call, and run the behaviour drivers where one exists
  for that code.
- **A function cannot be re-matched under the overlay flag set.** Only five
  objects are at risk (o039, o046, o065 ×2, o082).
  - First, recheck the single-TU evidence for that module.
  - If the evidence stands, the source spelling is wrong: work it as a
    matching target.
  - Splitting a module into two translation units is allowed only when an
    alignment-consistent boundary and independent evidence both support it.
- **Data typing.** The ROM catches wrong bytes. Semantic mistakes (for
  example, a pointer typed as an integer) are caught by the packer, because
  a pointer field must produce the shipped R32 record.
- **ORT names.** Of the 844 unreferenced rows, overlay rows name the
  function at that offset; resident rows use the resident symbol at that
  address. Rows with no symbol get a generated name in the project's
  convention.
- **IDO literal and COMMON ordering** after a fold. The overlay 77 pilot hit
  two consolidation hazards. Verify each fold before folding the next.

## Schedule

| Week | Work |
|---|---|
| 1 | Phase 0 integrates; phase 1: real names, the 182 target fixes, the packer and gate |
| 2 | Phase 2 folds (all batches; flag work is limited to five objects); phase 3 starts on folded modules |
| 3 | Phase 3: data, BSS and the three switch tables |
| 4 | Phase 4 retirement; phase 5 package and JFG demonstration |

Phase 1 is the critical path. Phases 2 and 3 parallelise per module.
