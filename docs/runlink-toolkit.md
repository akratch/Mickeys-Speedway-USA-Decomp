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
| `module-recipe` | JFG only: consume a locally authored corrected module ELF and declarations, generate metadata and optionally run the ordinary final link. | The bounded selected-module result described in the [handover](runlink-handover.md); manual producer prerequisites and outside compatibility remain. |
| `parity` | MSU only. Runs `tools/runlink_pack.py` and the kit on the same tree. | The kit reproduces MSU's packer: the same residuals (category, module, offset, expected, got), byte-identical generated regions and the same region proof. |
| `check-build` | MSU only. Runs core compare, region, kit table check and independent kit parity in one held request. | One fresh source inventory; all four checks and their current-input holds still run. |

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

The MSU build can explicitly select `link_overlay_module.py
--own-ro-prefix-source` for one complete C text input with no DATA or BSS and
an entirely source-owned read-only prefix. This route accepts complete const
signed o32 integer arrays only after fresh source, actual CPP, compiler layout,
object definitions and current source-graph checks. It preserves allocated
GLOBAL definitions and instruction bytes; private reference bindings encode the
section-relative operands consumed by the existing runtime linker. Original
relocation sites, normal metadata replay, linked definitions and all loaded
inputs remain held through publication. A retained suffix or unresolved imports
remain compatibility inputs. Zero-valued bindings can become symbol-zero
references during ordinary metadata normalization, so a configured static
comparison alone is not a complete identity proof: the unchanged promotion
checks must also prove the linked bytes and runtime table. This route does not
establish general edited-source support or source-only generation.

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
compilation from current C in general: the caller still owns the normal build
and source proofs. The explicitly selected complete data-only inputs additionally
pass fresh compiler/helper/provider/map authentication before producer discovery
(ADR 0021). Their original relocations produce their records directly; all
other named borrowing remains explicit. This remains hybrid generation, with
no source-only or relayout claim.

Explicitly selected resident assembly data-address pairs use a separate
`.addresssites` receipt and preserved `.address-original.o`. Splat's declared
relocations reconstruct the original symbolic HI/LO operands; the names-only
hook changes their undefined symbol name for the ordinary retail link. It
preserves all text bytes and the complete ordered relocation inventory. The
packer and MSU kit independently construct main references from the validated
original names, then select ORT rows by a unique actual canonical/source NOBITS
export and map geometry. They do not obtain those identities from retained
main-table sites. Initially supported nonadjacent pairs permit only intervening
LUI/ORI writes to other registers, and require zero-addend LUI/ADDIU pairs.
Missing halves, source/receipt/provider changes, ambiguous definitions, ABS
exports and inconsistent maps refuse the selected route. Allocated unknown
workspace storage remains borrowed; an unsized export does not prove capacity,
C ownership, lifetime safety or native portability.

An authority audit pinned to commit `0bdc9452af3dda5a91703dd94e56f6dc38e2e830`
found 3,636 of 10,209 module records labelled object-produced still resolved
through encoded-name coordinates. Separately, 739 section-target records
landed in retained blobs, and 879 raw-target incidences used already-listed
borrowed placement; 735 incidences overlap those last two groups. Four of the
blob references instead used a noncanonical trial-pool Atlas row, outside the
named placement borrowing. These are pinned audit counts, not live progress
metrics or additional residuals to add to the then-current 8,338 borrowed
records. An object-produced label or a zero borrowing total alone therefore
does not establish independent source authority: declaration/provider-backed
identities and actual source-linked storage placement must also be closed.
Existing exact matching proofs are unchanged, and no per-site audit ledger is
published here.

Generated function proxies with an exact, unique sized global raw function
definition now derive their target from that definition and its verified linked
input, rather than numeric name fields. An absent definition remains explicitly
on the `generated-proxy` compatibility route. Present but invalid or ambiguous
definitions refuse. This narrow authority closure does not turn the remaining
compatibility records or retained storage into source-independent generation.

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

`gmake check-runlink-tables check-runlink-kit` requests both legacy gates in
one Make invocation. Their shared prerequisite runs `check-build` once. Either
target alone also runs all four checks; separate Make invocations each create
a fresh inventory. Standalone Python commands keep their independent producer
behavior.

The combined request shares held compiler/provider observations, not generated
tables: the kit still constructs fresh resolver, relocation and rule results
independently from the core packer. Source, tools, recipes, loaded and parsed
objects, namespace and placement state, current files and generated products
remain checked between logical operations and before reporting success. A
change invalidates the request; no persisted success receipt replaces current
source admission. This changes no retained-bootstrap, raw-region, source-only
or general-layout requirement.


### Ordered C inputs in the MSU edited builder

The MSU-specific `tools/runlink_metadata_link.py edited` frontend accepts an
ordered source vector in a module declaration. In a private copy of
`config/runlink-edited.us.json`, replace that module's `source` and `build`
fields with `inputs`; keep its existing exports and entries. For example,
after creating the two actual source files:

```json
"inputs": [
  {"source": "src/overlays/o097/direction.c", "build": "complete-overlay"},
  {"source": "src/overlays/o097/remainder.c", "build": "complete-overlay"}
]
```

This is a fragment of the existing module declaration, not a complete registry.
The legacy single-source form remains supported; the two forms cannot be mixed.
`complete-overlay` uses the existing complete-source compiler recipe;
`registered` requires an existing registered recipe. Run the edited builder with
the private registry and a separate output directory:

```sh
gmake edited-runlink EDITED_MODULES=97 \
  EDITED_DECLARATIONS=build/ordered-example/registry.json \
  EDITED_OUT=build/ordered-example/result
```

Each input is compiled separately. The builder retains the untouched compiler
object, effective preprocessor output, dependency closure, flags and ordered
relocations. It requires static-symbol preparation to preserve the admitted
original symbol-index and relocation surface. An ordinary relocatable link then
combines the inputs in declared order before the existing module-link step.
The aggregate is a linker product, not an untouched compiler object. Actual map
contributions translate original sites and targets; numeric section origins are
not authored in the registry. Each declared export must have exactly one real
provider. A same-graph undefined name can resolve through its unique actual
strong GLOBAL definition and mapped input origin. A private provider requires
no new ABI export; an existing declared export retains its ABI row. Names
outside the graph still require declared ABI ownership and provider proof.

The admitted vector class is MIPS II/o32 with an authentic `-G0` recipe and no
GP use, COMMON, integer-word witness or resident-cell witness. The admitted BSS
shape contains contiguous sized GLOBAL objects followed only by genuine
compiler section alignment. Actual NOBITS type, flags, zero base, alignment,
ordered map contributions and extent must agree; padding is not a target object.
The runtime zero-init base and extent must contain whole loader words. This
allocation proof does not establish dynamic indices or storage lifetime.
Unsupported sections, ABI combinations and relocation roles refuse. Original per-input ABI
records remain authoritative: GNU ld can retain only one input's REGINFO rather
than union all register masks, and its aggregate GP value is not the original
compiler value. The audit checks the actual retained contributors and the
restricted no-GP contract; it does not manufacture an all-input REGINFO union.

The reviewed overlay97 exercise split its first function and existing context
datum from the remainder, including a live consumer in the second input. The
one-to-two-to-one sequence preserved the retail image and all 21 runtime records;
removing only the context provider refused. Overlay75 additionally moved its
existing five-element slot array into a separate source input: 20 object bytes
within the same 32-byte NOBITS allocation. All 31 runtime records remained
exact, including six LOCAL references to that private provider, with no ABI row
added. Provider omission refused with the consumer functions retained; recovery,
warm reuse and merge-back passed. These exercises prove the stated bounded
classes. They add no source-ownership credit and do not establish arbitrary
module/data/BSS growth, a source-only build or a general JFG builder. This vector
interface belongs to the MSU edited frontend; the portable kit's JFG
`module-recipe` still takes one already corrected producer ELF as described in
the [handover](runlink-handover.md).

### Bounded editable-layout diagnostic

`gmake check-editable-layout` compiles copies of the complete overlay75 source
and a deliberate tail module108, links their sections with the ordinary module
linker, regenerates the individual metadata inputs, then runs the ordinary final
linker. It checks an unchanged retail control, actual function/data/BSS growth,
changed exports and relocation counts, an odd-export alignment gap, addition of
module108, and removal of an unreferenced tail module. Removing either a target
still referenced by overlay75 or overlay75 itself is refused with actual callers.
Overlay75 actually calls the new module's offset-zero function and references
its BSS export; both must have authentic raw and final SYMBOL identities.
The original five variants retain 375 resident overlay-call records. A sixth
`resident-call` variant compiles a diagnostic copy of `src/main/spranim.c`,
replacing the existing `animseqPlay()` call in the admitted parameter-one branch
of `func_8001B798` with typed `overlay75MarkSlot(1)`. This yields 376 overlay-call
records, not an increased instruction-call count. The entire resident TU retains
its section geometry and every existing symbol home; all other functions retain
their bytes. Fresh raw names pass through the existing metadata-only trap rename
and authenticated sidecar. The new record is independently joined to the actual
final call site, real trap and qualified source export. All prior 375 records
retain their exact identities, sites and order. Ordinary resident replay explains
the changed TU bytes; the main table grows by eight bytes and the existing
boundary/loader checks prove downstream movement. Geometry changes refuse with a
broader-layout prerequisite. This does not establish arbitrary resident growth.
Before emitting wrappers, the driver rejects signed loader allocation overflow
including RT1 and any relocation whose complete four-byte word escapes its body.

Each run preserves a fresh `build/editable-layout/check-*` directory containing
commands, compiler/module receipts, generated inputs, final maps/images and a
separate nonmatching diagnostic receipt. Check its freshness with
`.venv/bin/python tools/check_editable_layout.py --check-receipt <directory>/receipt.json`.
Missing, modified or stale artifacts/inputs are refused. Receipts carry the
semantically validated artifact snapshots through publication; later observations
must match those hashes and cannot replace them. Toolchain snapshots are checked
again when freshness validation ends. The final image is
independently decoded and joined to raw source relocation identities, ordinary
module payload/NOBITS layout and actual final symbols. Unselected payloads,
VMAs and program symbols remain exact; selected resident code and the loader are independently relinked at
its real VMA against the new boundary values. Header DMA excludes alignment
padding. The diagnostic removes only the declared fill input and symbols, checks
the script mapping by exact reversal, and emits a 32 MiB image with an FF tail
and a stable recalculated checksum.

Canonical overlay75 source, IDs 1–107 and existing export-row identities stay
unchanged. The diagnostic receipts retain the canonical catalog and its named
borrowing, including the retained bootstrap. Data-only discovery is inventoried
but not activated. This bounded source-complete pilot does not establish a
source-only build, arbitrary module renumbering, arbitrary resident growth or general
relayout. The canonical retail producer still requires raw equality with zero
differences, and its final link still requires bootstrap equality.

## Tests

`tools/test_runlink_codec.py`, `tools/test_runlink_rules.py` and
`tools/test_runlink_generate.py` run under `gmake check-tooling`. Every
fixture is synthetic, built with `runlink.synth`, and the table bytes are
written from the format with `struct` rather than through the codec. The
generate test builds a small GNU ld tree with objects, a linked ELF, a map
and a ROM. It requires an exact round trip, and requires that a reordered
relocation, a placeholder name and a wrong callee each come out as their own
residual category.
