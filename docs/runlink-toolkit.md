# The runlink kit

`tools/runlink/` is a Python package for the tables of Rare's runtime overlay
linker, as shipped in Mickey's Speedway USA (MSU) and Jet Force Gemini (JFG).
It uses only the standard library and needs no installation. It decodes and
re-encodes the tables, tests the generation rules against them, regenerates
them from a build, and classifies every difference. `docs/overlays.md` §5
describes the format. `docs/runlink-jfg.md` records the measurements on JFG.

## Running it

```sh
python3 tools/runlink [--config NAME|PATH] [--root DIR] [--rom PATH] <command>
```

- `--config`: a config name from `tools/runlink/configs/` (`msu-us`, the
  default, or `jfg-us`), or a path to a config file.
- `--root`: the directory that relative paths in the config resolve against.
  `msu-us` sets its own root, this repository. `jfg-us` needs `--root`
  pointing at a JFG checkout.
- `--rom`: overrides the config's ROM. `.z64`, `.v64` and `.n64` byte orders
  are all read.

TOML configs need Python 3.11 or later; a `.json` config with the same keys
works on older versions. Exit status is 0 on success, 1 when a check fails
and 2 when inputs are unusable.

| Command | What it does | What a pass proves |
|---|---|---|
| `check` | Decodes the main table, ORT, headers, module images and the optional name table, re-encodes them, and compares the result with the ROM. | The codec and the config describe the ROM exactly: every byte of the region round-trips, and the headers chain back to back. |
| `census [-v] [--json F]` | Counts records by table, operation and type, then tests each generation rule against the shipped tables alone. `-v` lists exceptions. | Which rules hold for every record of this game, with no build involved. |
| `decode [--out F]` | Dumps the decoded tables as JSON. | Nothing. The output is ROM-derived, so keep it out of git. |
| `names` | Prints the ORT as `<row> <name>`: from the ROM's name table where there is one (JFG), else from the configured list (MSU). | Nothing. JFG's names are ROM-derived, so keep the output local. |
| `generate [--mode M] [-v] [--json F] [--out F] [--unpatched OLD=NEW]` | Regenerates every table from the build and compares the result with the ROM. | See the modes below. With `--out`, it also writes the generated region. |
| `parity` | MSU only. Runs `tools/runlink_pack.py` and the kit on the same tree. | The kit reproduces MSU's packer: the same residuals (category, module, offset, expected, got), byte-identical generated regions and the same region proof. |

`generate` has three modes, which differ only in where each relocation's
**identity** comes from. Identity means the address the relocation refers to,
and for a SYMBOL record the ORT row that names that address.

- `objects`: from the build alone. A placeholder name (`func_0`, `D_28`)
  produces no record.
- `hybrid`: from the build, except that a placeholder takes the identity of
  the shipped record at the same (site, type). Each borrowed identity is
  counted.
- `oracle`: every identity comes from the shipped record at the same
  (site, type). This mode isolates the rules from the naming state of the
  source.

In every mode the rules alone decide the table, the operation, the index,
the order, the headers and the main table. Every `generate` run ends with a
region proof. It encodes the generated region, then encodes it again with
exactly the records, header fields and ORT rows named by a residual replaced
by the shipped ones. Equality of the second image with the ROM proves that
nothing outside the listed residuals differs, record order included.

These three modes apply to the generic `ld` adapter. The MSU adapter currently
supports only `hybrid`; requesting `objects` or `oracle` fails explicitly.
An MSU hybrid pass can still borrow named identities from the shipped tables
and is not evidence of source-only generation.

`--unpatched OLD=NEW` rewrites the map's object-path prefix `OLD` to `NEW`.
It is for builds that rename resident trap callees inside the objects after
compiling, as JFG's `tools/patch_symbols.py` does. The kit then reads each
callee's name from the objects as compiled, and that name gives the
main-table row.

## Configuration

Each config is one TOML file. `configs/msu-us.toml` and
`configs/jfg-us.toml` comment every value.

| Table | Keys |
|---|---|
| top level | `name`, `root`, `rom`, `rom_sha1` (a mismatch warns) |
| `[tables]` | `main_table` (the count word's ROM offset), `main_table_block` (bytes up to the ORT), `ort`, `ort_count`, `headers`, `module_count`, `modules` (base of `romAddress`), `modules_end` |
| `[names]` | either `offsets`, `blob` and `blob_end` (a name table in ROM), or `list` (a tracked `<row> <name>` file) |
| `[resident]` | `text`, `data`, `bss` and `end` (resident section bases), `trap_symbol`, `trap_address`, `rom_delta` (resident VRAM minus ROM offset) |
| `[selectors]` | `data` (list), `bss`, `reserved` (list), and optionally `absolute` with `absolute_base` |
| `[build]` | `adapter` (`msu` or `ld`), plus the generic front end's inputs: `elf`, `map`, `module_section`, `module_bss_section`, `table_inputs`, `text_align`, `resident_sections`, `main_sentinel`, `runtime_objects`, `placeholder`, `main_order` (`link` or `address`), and `[build.entry_names]` (`init` and `resume` name formats) |

## Layout

| Module | Role |
|---|---|
| `codec.py` | Records, headers, ORT rows and modules: decode from ROM and encode back. Also handles relayout, where offsets and table sizes follow the content. |
| `config.py` | Loads a config. |
| `names.py` | The ROM name table (encode and decode; a repeated name shares one offset) and the tracked list format. |
| `rules.py` | The generation rules, from resolved references to tables. The module docstring states each rule. |
| `compare.py` | Classified comparison, shipped identities and the region proof. The docstring lists every category. |
| `census.py` | Counts, and rule checks against the ROM alone. |
| `elf.py`, `mapfile.py` | A big-endian ELF32 reader with MIPS REL addends, and a GNU ld map reader. |
| `generate.py` | The generic front end for a GNU ld build: linked ELF, map and objects. |
| `adapters/msu.py` | MSU's front end. It loads its inputs through `tools/runlink_pack.py` read-only, and is the only file that depends on this repository. |
| `synth.py` | Synthetic ELF objects and hand-written table bytes, for tests. |

To use the kit in another repository, copy `tools/runlink/` and delete
`adapters/msu.py`.

A minimal library use:

```python
from runlink import codec, config, census
cfg = config.load("jfg-us", root="path/to/jfg")
tables = codec.decode(codec.read_rom(cfg.rom), cfg)
print(census.rule_checks(tables, cfg)["JUMP index = most recent SYMBOL index (table 1 then 2)"])
```

## On MSU

### Generated metadata producer

After the ordinary build and `gmake runlink-raw`, run:

```sh
python3 tools/runlink_pack.py --elf build/runlink/bootstrap.elf --map build/runlink/bootstrap.map emit-metadata
# Optional separate output directory, with the same explicit bootstrap inputs:
python3 tools/runlink_pack.py --elf build/runlink/bootstrap.elf --map build/runlink/bootstrap.map --out build/metadata-review emit-metadata
```

The default output is `build/runlink/metadata/`. Canonical Make invokes this
producer with distinct bootstrap ELF/map inputs and consumes its exact metadata.
A standalone emission using the default final ELF is diagnostic; the canonical
consumer refuses that provenance and regenerates from the bootstrap.
It uses the existing packer and encoders, and refuses any input problem,
classified difference, stale borrowing ledger, or nonidentical **raw** region.
The existing `region` command's successful residual-patched proof is
insufficient for this operation.

Outputs have the retained inputs' granularity: `main_reloc_table.bin`,
`overlay_rom_table.bin`, `overlay_header_table.bin`, and nonempty
`overlays/oNNN/overlay_NNN_reloc1.bin` / `reloc2.bin`. The US layout has 198
such bins. It emits no duplicate module body or instruction patch.

`manifest.json` records input and output sizes/hashes, the loaded packer and
table parser hashes, checkout revision, raw-region verdict, and live output
counts. `borrowing.json` reuses the existing residual-manifest format and
preserves every actual named borrowing; its hash and category counts are in
the manifest. The producer binds actual object/ELF/map/ROM/atlas/ORT/ledger
inputs and resident trap sidecars. It does not independently authenticate their
compilation from current C: the caller still owns the normal build and source
proofs. This remains hybrid generation, with no source-only or relayout claim.

Inputs are snapshotted before a fresh pack and checked again before publication.
Files are staged first; publication invalidates the old completion marker,
replaces payloads, rechecks inputs, and writes `manifest.json` last. A later
consumer must require a successful invocation and verify the manifest's input,
implementation, payload and borrowing hashes. Failed validation can leave an
older output set available for inspection; it cannot make that set evidence
for changed inputs. Interrupted publication leaves no completion marker.
Unrelated files and symlink outputs are refused rather than overwritten.

### Fixed-layout final consumers

The canonical dependency graph is:

```text
ordinary objects + retained metadata -> bootstrap.elf / bootstrap.map
raw overlay objects + bootstrap -> guarded metadata bins + borrowing manifest
generated wrappers + metadata-only mapped script + ordinary code/data -> final ELF/map
```

`build/runlink/metadata-link.json` binds the producer manifest, source/mapped
scripts, actual implementation and linker/binutils hashes, and generated wrappers.
The script mapper requires exactly one ordinary `.data` selector for every
expected metadata object and verifies that reversing those replacements restores
all other script bytes. Binary wrappers retain the old `_binary_*` symbol names.
No whole-module blob replaces compiled code/data.

`build/runlink/final-proof.json` records both actual ordinary linker commands,
the full generated map inventory, zero retained metadata consumers, allocated
section/program-symbol equality, and the observed whole-ELF equality. The live
producer ledger remains explicit. Promotion preflight/raw receipts and the
existing census dependency set include the producer/consumer artifacts and
hashes; missing or tampered evidence cannot be treated as fresh.

Normal Make validates content on every canonical invocation, repairs missing or
tampered generated payloads/wrappers/scripts, and preserves timestamps on an
unchanged build. Question-mode Make traversals omit build-only validators;
proof freshness checks first validate the complete generated context and then
query real object/bootstrap dependencies. Non-matching/raw diagnostic rules
retain their existing paths. This does not relax the exact retail raw-region
requirement, remove the retained bootstrap, or permit general source growth.

`gmake check-runlink-kit` runs `check` and `parity`. Like
`check-runlink-tables`, it needs the baserom, a linked ELF and the raw
overlay objects. On the tree where the kit was written it reported 153
differences on both sides (pool-layout 124, table-entry 26, extra 1,
missing 1, unplaced 1), identical residual tuples, byte-identical generated
regions (693,360 bytes), and the same region proof: 8 bytes differ raw, and
none once the residuals are replaced. The gate itself stays
`check-runlink-tables`. The kit reimplements the rules, the encoding and the
comparison; it does not replace MSU's packer.

## Tests

`tools/test_runlink_codec.py`, `tools/test_runlink_rules.py` and
`tools/test_runlink_generate.py` run under `gmake check-tooling`. Every
fixture is synthetic, built with `runlink.synth`, and the table bytes are
written from the format with `struct` rather than through the codec. The
generate test builds a small GNU ld tree with objects, a linked ELF, a map
and a ROM. It requires an exact round trip, and requires that a reordered
relocation, a placeholder name and a wrong callee each come out as their own
residual category.
