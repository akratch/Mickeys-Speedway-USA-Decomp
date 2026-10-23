# Jet Force Gemini's runlink tables, regenerated from its build

This document is for the maintainer of the Jet Force Gemini decomp. It reports
what the runlink kit (`tools/runlink/`, `docs/runlink-toolkit.md`) measures on
JFG US: how the shipped tables decode, which of the generation rules found on
Mickey's Speedway USA (MSU) hold for them, how much of the tables JFG's own
pinned build regenerates, and what blocks the rest. These are historical
snapshot measurements, not claims about current upstream JFG. Each number is marked
**verified** (computed by the kit, re-runnable with the commands in the last
section) or **inferred** (an interpretation of verified numbers).

## Inputs

- The JFG US ROM, SHA1 `493ced9008dbe932d6e91179b68e8630cf23a023`.
- JFG at commit `efd5abb`, extracted and built from clean in a copy outside
  the checkout (`make extract`, then `make`). The built ROM's SHA1 equals the
  baserom's (verified).
- That copy's resident objects that hold trap call sites, compiled a second
  time with `PATCH_SYMBOLS=:` into `build_unpatched/`. JFG's
  `tools/patch_symbols.py` renames each overlay callee to `TrapDanglingJump`
  inside the object, so only the unpatched objects still carry the names the
  C uses. 21 of the 22 source objects compiled. `src/hasm/refractOutputAssembler.s`
  did not build in that copy; its 4 sites are the HI16/LO16 entries below,
  which no callee name covers anyway.
- The ORT names come from the ROM's own name table (`symbolsTable_offsets`
  and `symbolsTable_symbol_names`, as described in `docs/overlays.txt` on the
  OverlayHandling branch). The kit decodes it at run time; no extracted
  name-table or relocation-table payload is stored here.

An older matching build (2026-08-30) gives different surface numbers, because
its extraction named placeholders `func_0` and `D_<value>`. Every number
below is from `efd5abb`.

## Same-commit reproduction, 2026-10-23 (verified)

A fresh isolated checkout of
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` reproduced the original build.
The prior private archive was reference-only. Extraction and a full build
produced ROM SHA1 `493ced9008dbe932d6e91179b68e8630cf23a023`; all 21 resident
C objects were also rebuilt with `PATCH_SYMBOLS=:`. This did not rebuild the
handwritten-assembly object mentioned above.

The reproduction copied the archived IDO 5.3 tools and Python environment
into the isolated checkout, restored the committed asm-processor submodule
at `f3b2f85a5bc7e8ae2030ea5a83ea8aa0bcbede03`, and supplied a private GNU
Binutils `2.46.0.20260210` copy through an explicit
`CROSS=<private-binutils>/mips64-elf-` make override. No shared toolchain or
donor pin changed. Python was 3.14.7, Splat 0.50.0 and spimdisasm 1.42.3.
The copied environment ran extraction/build, but its incomplete pip could
not run `pip freeze`; installed-package metadata was recorded through
`importlib.metadata` instead. This is a demonstrated setup, not a claim
that a new install from the partially pinned requirements is hermetic.

Using this kit at `236df8b8a1dbd6005264228b523aa09b95181bb6`:

- `check` reproduced the 1,162,776-byte region and name-table round trips.
- `objects` and `oracle`, with the unpatched-object path override, reproduced
  the archived JSON reports exactly, including all residual records:
  **32,334** and **11,295** respectively. Surface coverage remained
  **21,302/27,515**. This is residual-assisted reconstruction, not source-only
  table generation.
- Census counts reproduced. Two rule denominators now exclude the 23 empty
  tables: sorted tables are **174/287**, rather than the archive's 197/310;
  text-before-data is **287/287**, rather than 310/310. The same **113** tables
  are unsorted. The table below already uses the non-empty denominators.

Keep the unpatched-object option when comparing identities: without it the
fresh `objects` run has 32,808 residuals and no source names for the 520 main
entries. Restoring the 21 C objects recovers the historical 513 named entries;
46 main-row residuals include 39 wrong names and seven unnamed entries.

## 1. Format (verified)

`python3 tools/runlink --config jfg-us --root <jfg> check` decodes the main
table, the ORT, all 157 headers, the module images and the name table. It
re-encodes them and compares the result with the ROM: the region
`0x1ECF220`–`0x1FEB038` (1,162,776 bytes) and the name table round-trip
byte-identically.

- **Headers.** The 157 headers occupy `0x1ED2780`–`0x1ED3B20`. The module
  images they describe begin at `0x1ED3B20`. Modules 21 and 56 are empty.
- **Name table.** The table has 2,372 names and 2,371 distinct ones. Row 2371
  reuses offset 0, so a repeated name is stored once. A generator that writes
  every name out would differ from the ROM by that one name.
- **Record layout.** The record and main-table entry layout is the same in
  both games: `symbolIndex`, then `site << 8 | type << 4 | op`. `type` is the
  ELF MIPS relocation number. `op` is SYMBOL 0, LOCAL 1, JUMP 2 or DATA 3.
  `overlays.txt` calls the low nibble `relocType` and the high nibble
  `patchOperation`; these are the same bits.

## 2. The rules, tested on the ROM alone (verified)

`census` tests each rule against every shipped record, with no build
involved:

| Rule | JFG | MSU |
|---|---|---|
| Table 1 SYMBOL records target another module or the resident module | 11,076/11,076 | 6,943/6,943 |
| Table 2 SYMBOL records target their own module | 514/514 | 403/403 |
| Table 1 holds only SYMBOL records; no module record uses DATA | 27,515/27,515 | 18,542/18,542 |
| A JUMP is an R_MIPS_26 whose target has no ORT row | 1,090/1,090 | 618/618 |
| A JUMP's index is the most recent SYMBOL index, table 1 then table 2 (0 before the first) | 1,090/1,090 | 618/618 |
| Every HI16 is immediately followed by its LO16 (same op and index) | 8,476/8,476 | 6,480/6,480 |
| LOCAL bases are 0, text, text + data, and at most one more | 155/155 | 106/106 |
| Text-site records precede data-site records in each non-empty table | 287/287 | 195/195 |
| Text size is a multiple of 16 | 155/155 | 106/106 |
| Non-empty tables sorted by site | 174/287 | 110/195 |

The other shipped counts are these:

- **Records.** There are 27,515 records: 11,076 in table 1 and 16,439 in
  table 2.
- **Data sites.** 2,025 records patch a data word. All of them are R_MIPS_32
  records whose site is at or past the text size, measured from the text
  base: 1,981 LOCAL and 30 SYMBOL records in table 2, and 14 SYMBOL records
  in table 1.
- **Main table.** It has 520 entries. 513 are SYMBOL R_MIPS_26 entries whose
  site holds `jal TrapDanglingJump`. Four are SYMBOL HI16/LO16 entries, two
  pairs. Three are DATA R_MIPS_32 entries.
- **ORT.** It has 2,372 rows: 1,291 resident, 798 module, 110 `0xFFD`, 172
  `0xFFF` and 1 `0xFFC`. 803 rows are named by no record and no main entry.
- **Bss-region targets.** 154 LOCAL records use the data base for a target
  that lies past the end of data, in the bss range. MSU has 114 such records.

### Where `overlays.txt` and the ROM disagree (verified against the ROM)

- **§4.2 and §6.3, data relocations.** These sections say a data-section
  relocation "can only be expressed with relocType 3". No module record uses
  DATA. Every data-site record is SYMBOL or LOCAL, with its site measured
  from the text base. DATA appears only in the main table (3 entries).
- **§4.2 and §6.3, JUMP index.** These sections say to set a JUMP's
  `symbolIndex` "to the ORT index of the callee". No JUMP callee has an ORT
  row. The index is a stale value: the most recent SYMBOL index (1,090/1,090).
- **§6.3, table split.** This section puts LOCAL and JUMP records in the
  secondary table, and every relocType 0 and 3 record in the primary. In the
  ROM, table 1 holds exactly the references that leave the module. Table 2
  holds everything else, including 514 SYMBOL records that name the module's
  own exported symbols.
- **§6.3, HI16/LO16 references inside the module.** This section advises
  against LOCAL HI16/LO16 records. The ROM has 6,392 LOCAL HI16 and 6,462
  LOCAL LO16 records.
- **§6.4, record order.** This section says that sorting by `targetOffset`
  "matches the original tables". 113 of the 287 non-empty tables are not
  sorted. Their order is the compiler's relocation order (section 3.3).

## 3. Regeneration from JFG's build

`generate` reads `build/jfg.us.elf`, `build/jfg.us.map` and every object the
map places in `.overlay_N` and `.main`. Each module's text, data and bss
placement comes from the map. Two `--mode` runs isolate the two questions:

- **`objects`** asks what the build produces as it stands.
- **`oracle`** takes each relocation's identity from the shipped record at
  the same (site, type), and asks what the rules produce once identities are
  right. Identity means the referenced address, and the ORT row for a SYMBOL
  record.

Both runs end with a region proof. With exactly the listed residuals
replaced by the shipped records, the generated region is byte-identical to
the ROM: 32,334 residuals in `objects` mode and 11,295 in `oracle` mode
(verified). Everything outside the residuals, record order included, is
therefore already exact.

### 3.1 The relocation surface (verified)

This counts the shipped records that have an object relocation at their
(site, type):

| Shipped records | Have a relocation in the objects |
|---|---|
| text R_MIPS_26 | 8,467/8,467 |
| text R_MIPS_HI16 | 6,201/8,476 |
| text R_MIPS_LO16 | 6,267/8,547 |
| data R_MIPS_32 | 367/2,025 |
| total | 21,302/27,515 (77.4%) |

There are 3,832 object relocations with no shipped record: 3,819 data
R_MIPS_32 relocations and 13 text HI16/LO16 relocations.

The 6,213 missing records are references the disassembly left as plain
immediates or plain words. The 3,832 extra relocations are words the
disassembly symbolized but the game does not relocate. This is inferred: in
a module linked at VRAM 0, small integers look like in-module addresses.

### 3.2 Identities as the build has them (verified, `objects` mode)

Of the 21,302 covered records, 2,198 refer to the address the shipped record
does. The other 19,104 break down as follows:

| Count | What the object names |
|---|---|
| 7,344 | calls that name the module's own offset-0 function. An external call stores `jal 0`, and the disassembly resolved 0 inside the segment |
| 11,102 | HI16/LO16 and data references that name an in-module symbol at the stored immediate. A LOCAL immediate is an offset from a section base, and the disassembly read it as a module offset |
| 492 | section-relative references that resolve to a different offset |
| 134 | data-word pointer-table entries |
| 32 | module 14 LOCAL HI16/LO16 pairs that store `0x8000xxxx` and are spelled `D_8000xxxx`, as if resident. The base is the module's data. This is inferred to be a virtual-to-physical idiom (the addend `0x80000000` maps KSEG0 to physical) |

A further 1,008 records name an undefined `func_`/`D_` placeholder and get no
identity at all. Because the identities are wrong, 1,057 of the 1,090 JUMPs
take a wrong stale index.

### 3.3 The rules, given identities (verified, `oracle` mode)

On the 21,302 covered records:

- **Table split, operation, SYMBOL row.** All agree: 0 differences each.
- **Stale JUMP index.** 998 of 1,090 agree. Every one of the 92 misses
  follows a surface difference: 90 have a shipped predecessor SYMBOL that the
  objects lack, and 2 have an extra generated SYMBOL as predecessor.
- **LOCAL base.** 128 differ. All are references based on `.data` whose
  target lies past the end of data. A section-relative identity in the source
  would give the base directly; an address alone cannot.
- **Rodata base.** 80 modules need a fourth LOCAL base, a rodata boundary
  inside the data span. Splat emits each module's data and rodata as one
  `.data` section, so the build does not contain this boundary. `oracle` and
  `hybrid` take it from the shipped table and count it as borrowed.
- **Record order.** Every one of the 174 site-sorted tables agrees. 60 of the
  113 unsorted tables agree on the records both sides have. The 964 records
  out of order (473 HI16, 463 LO16 and 28 R_MIPS_26, in 48 modules) are
  shipped in compiler emission order, which places some HI16/LO16 pairs
  before pairs at lower sites. GNU as emits them in site order. MSU's
  measurement is that this order is IDO's own `.rel.text` order, which only
  the C reproduces (inferred to carry over to JFG).
- **Headers.** text, data, init and resume agree in all 155 non-empty
  modules, as do the text+data image bytes. init and resume come from
  `_AutoInit%05d` and `_AutoExit%05d`, resolved through the ROM's names: 4
  init entries and 12 resume entries. bss agrees in 93 of 155 modules, which
  are exactly the 93 whose bss is 0. No object owns overlay bss.
- **Main table.** All 520 entries are produced: the trap calls, the
  `gUnresolvedSymbolAddr` HI16/LO16 pairs outside `runLink.c`, and the
  `TrapDanglingJump` words in resident data. The order agrees when entries
  are taken by address. Taken in object relocation order they do not agree
  (verified), because `track.c.o`, an asm-processor object, lists its
  GLOBAL_ASM relocations apart from the C's.
- **Main-table rows from the objects.** In `objects` mode, 513 of 520 rows
  are named by the unpatched objects, and 474 of those 513 agree with the
  shipped row. The 39 that differ are trap calls whose C callee is a function
  at a different address from the shipped row's:

  | File | Calls |
  |---|---|
  | `anim.c` | 9 |
  | `hit.c` | 8 |
  | `objects.c` | 7 |
  | `main.c` | 4 |
  | `menu.c` | 3 |
  | `squads.c` | 3 |
  | `spranim.c` | 2 |
  | `memory.c` | 1 |
  | `prnBorder.c` | 1 |
  | `shadows.c` | 1 |

  The ROM bytes cannot catch these: every trap call is
  `jal TrapDanglingJump` whatever it names. The remaining 7 rows (the 4
  sentinel HI16/LO16 entries and the 3 DATA words) are named by nothing in
  the source.
- **ORT from names.** 2,368 of 2,372 rows are reproduced by resolving each
  ROM name through the linked ELF. Of the other 4, three are module bss
  symbols the build does not define: `Skips` (module 2), `pausePlayer`
  (module 19) and `pausetunevol` (module 12). The fourth is row 1747,
  `__osRdbSend`, which stores selector `0xD66` (neither a module nor a
  reserved selector) and is used by one main-table entry. The `0xFFC` row
  (`osTvType`, offset `0x300`) resolves as absolute `0x80000300`. That base
  is inferred from this one row; `ResolveRelocAddress` has no case for
  `0xFFC`.

Exact regeneration, with no residual of any kind in a module's tables, holds
for 11 of 153 modules in `oracle` mode and for none in `objects` mode. The
gap is the surface (section 3.1) and the source-level information below, not
the rules.

## 4. What JFG would need, by category

1. **A complete relocation surface: 6,213 records.** Every relocated site
   must be a relocation in the objects: 2,275 HI16, 2,280 LO16 and 1,658 data
   words. For assembly, this means symbolizing each relocated immediate and
   data word, and no others: the 3,832 extra relocations have to go. C
   supplies this surface by construction.
2. **Identities: 19,104 wrong and 1,008 absent.** A placeholder name has to
   stand for its real target. There are two routes. One is real names. The
   other, as MSU did while its overlays were partly assembly, is a
   per-module placeholder surface: a generated `name = value` list per
   module, so the linked bytes stay exact while each placeholder records
   which target it means. The shipped tables are the oracle for both (`hybrid`
   and `oracle` modes show the effect). The 7,344 offset-0 calls are the
   largest single class.
3. **Data and rodata ownership.** The 80 rodata bases require the data span
   to be split into data and rodata. The 128 bss-region LOCAL bases require
   section-relative references.
4. **BSS ownership: 62 modules.** No object defines overlay bss, so the header
   `bssSize` and the bss-based LOCAL offsets come from nowhere in the build.
5. **Order: 964 records in 48 modules.** These need the compiler's emission
   order. Matched C gives it; otherwise a per-module order override,
   generated from the ROM, is needed.
6. **Main-table names: 39 calls and 7 unnamed entries.** The 39 calls need
   correcting. The 7 entries need naming in the source if the main table is
   to be generated without the ROM.
7. **ORT: 4 rows.** Three need module bss symbols defined. One
   (`__osRdbSend`, selector `0xD66`) has to be kept as data.

Nothing measured here contradicts a rule. Every difference falls into one of
the categories above.

## 5. Running the kit on JFG

From this repository, with a JFG checkout or build at `<jfg>`:

```sh
python3 tools/runlink --config jfg-us --root <jfg> check
python3 tools/runlink --config jfg-us --root <jfg> census -v
python3 tools/runlink --config jfg-us --root <jfg> generate --mode objects
python3 tools/runlink --config jfg-us --root <jfg> generate --mode oracle
# with resident objects rebuilt without the trap rename:
#   make BUILD_DIR=build_unpatched PATCH_SYMBOLS=: <those objects>
python3 tools/runlink --config jfg-us --root <jfg> generate --mode objects \
    --unpatched build/=build_unpatched/ -v --limit 100000
```

`-v` prints every residual as `category module +offset: expected ...; got
...`. `grep main-row` lists the 39 calls. `--json FILE` writes the measures
and every residual.

The kit needs only Python 3.11 or later. To use it without this repository,
copy `tools/runlink/`, delete `adapters/msu.py`, and run it the same way.
The JFG-specific values are all in `configs/jfg-us.toml`: the table
locations, resident bases, selectors, section names and entry-point name
formats.

## 6. Actual `reloc_addrs` extraction pilot (verified)

The same snapshot's installed Splat 0.50.0/spimdisasm 1.42.3 performed a
separate module-1 extraction with six ROM-site overrides. This was an actual
split and ELF assembly, beyond an in-memory parser check. The first 288-byte
initializer was assembled with the snapshot's unmodified asm-processor
prelude. The control's extracted file equals the full-build extraction.

| Module text site | ELF relocation | Supplied target role |
|---|---|---|
| `+0x0C`, `+0x20` | `R_MIPS_26` | One resident callee |
| `+0x14`, `+0x18` | `R_MIPS_HI16`, `R_MIPS_LO16` | Module-1 data target A |
| `+0x28`, `+0x2C` | `R_MIPS_HI16`, `R_MIPS_LO16` | Module-1 data target B |

All 13 initializer relocation sites/types/order and all 288 raw text bytes
remain unchanged; precisely these six references take the supplied target
names. The shipped records independently confirm two SYMBOL imports and
two LOCAL address pairs based at the module's data boundary, with relative
offsets `0x20` and `0x24`. The annotations prevent Splat's old guesses from
binding the calls to the module's offset-zero function and the data operands
to labels in its text.

All three supplied targets are still undefined ELF symbols. A qualified name
is an identity assertion, not an allocated object, section definition or
export. The string's spelling supplies no addend or data-base semantics;
these overrides use addend zero. The raw object is not a newly linked-ROM
proof. The baseline full build remains the ROM proof; the pilot demonstrates
recovered symbolic operands only. Neither override-file order nor GNU-as site
order establishes the original compiler's relocation emission order.

### Concrete continuation

1. **Finish a module-1 identity/definition pilot.** Authenticate real objects
   and types behind the two data-relative word accesses, and define them in
   their actual section. Module 1 has `0xC30` text, `0x40` initialized data and
   `0x1010` BSS bytes; the six overrides do not establish ownership of those
   spans. Use `(module, section, offset)` identities for shared-address
   overlays and resolve the resident import through its real export.
2. **Recover the remaining surface with explicit scope.** Cover the missing
   HI16/LO16 and data references in section 3.1 and remove the extra guessed
   relocations. ROM-site annotations bootstrap legacy extraction; current
   symbolic source and object relocations must carry the facts after code
   moves. Do not retain original ROM sites as the editable build's layout.
3. **Own sections and exports.** Establish data/rodata boundaries and BSS
   definitions, section-relative references, actual export descriptors and
   init/resume entries. The 80 rodata bases and 62 missing BSS owners above
   remain separate work; unresolved/reserved ORT cases need explicit audit.
4. **Generate from the current build.** Consume ordinary linked module
   sections, current ELF relocations and owned exports to choose SYMBOL,
   LOCAL or JUMP, table split, HI/LO pairing and ordering. Then generate the
   ORT, main table, headers, cumulative ROM offsets and deduplicated name
   table. Correct the 39 resident call identities separately: identical trap
   call bytes cannot prove them. Historical exact order requires its own
   evidence; the six annotations do not supply it.
5. **Demonstrate independence and editability separately.** Reprove original
   bytes and every relocation identity with explicit remaining borrowing,
   then prove layout/table regeneration after a deliberate bounded source
   edit. No result in this report establishes that final source-only or
   generically editable pipeline.
