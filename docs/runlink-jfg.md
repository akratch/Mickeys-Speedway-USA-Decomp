# Jet Force Gemini's runlink tables, regenerated from its build

This document is for the maintainer of the Jet Force Gemini decomp. It reports
what the runlink kit (`tools/runlink/`, `docs/runlink-toolkit.md`) measures on
JFG US: how the shipped tables decode, which of the generation rules found on
Mickey's Speedway USA (MSU) hold for them, how much of the tables JFG's own
build regenerates today, and what blocks the rest. Each number is marked
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
  C uses. 21 of the 22 C objects compiled. `src/hasm/refractOutputAssembler.s`
  did not build in that copy; its 4 sites are the HI16/LO16 entries below,
  which no callee name covers anyway.
- The ORT names come from the ROM's own name table (`symbolsTable_offsets`
  and `symbolsTable_symbol_names`, as described in `docs/overlays.txt` on the
  OverlayHandling branch). The kit decodes it at run time. Nothing in this
  repository contains JFG's names or table bytes.

An older matching build (2026-08-30) gives different surface numbers, because
its extraction named placeholders `func_0` and `D_<value>`. Every number
below is from `efd5abb`.

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
