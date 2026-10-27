# Runlink handover: useful tools now, source-driven generation next

This is the starting index for the Jet Force Gemini maintainer. The public kit can inspect and recover relocation information and can regenerate and relink one complete symbolic module after a live edit. The complete general reference builder remains work to implement and prove. This handover supplies the current commands, their actual prerequisites, the evidence to demand and an ordered implementation plan.

Use [the JFG evidence report](runlink-jfg.md) for measured results and [the kit reference](runlink-toolkit.md) for formats and APIs. The executable package is [tools/runlink](../tools/runlink/). The demonstrated JFG source snapshot is `efd5abb1c79636e297b831f7c2d5bf47eac39c0c`. Public MSU kit pin `458df7cabc8efdaf766273fdfe54db885165e1e8` contains the executable recipe. Record the exact kit pin used for a new run. A later CLI hash requires a fresh receipt even when its reviewed JFG behavior is unchanged.

## What “100% source-owned data” establishes

A physical initialized-data ownership total measures bytes emitted by justified source producers, including their genuine compiler alignment. Each producer still needs its actual declarations, original compiler output, relocations, map and linked-byte proof. The total does not establish that every caller has a compatible type, that an array index is valid, that a GPU resource has finished, or that metadata and layout are independent of an old ROM.

A source-driven build additionally needs current source/object/declaration authority for every export, relocation identity, runtime section role, BSS extent, entry point and placement. It must compute the lookup tables and final layout after source changes. A genuine opaque protocol payload needs an explicit resource representation and binding strategy; arbitrary extracted bytes in a C array are not semantic C reconstruction. Keep an unresolved asset visible rather than silently changing the ownership definition. A resource-node declaration can help a source-driven metadata builder while a separate requirement for genuine C ownership remains open; it is not an ownership exception.

Ordinary compiler processing, metadata preservation, section-relative linking and generated tables can be legitimate build machinery. Fabricated object authority, target-site proxies and borrowed identity fallback must disappear from a claimed source-only metadata path. Source-quality exceptions still need their separate owner-reviewed justification. C cleanup and native ABI/resource contracts have their own acceptance criteria; this roadmap does not declare them finished.

## Pick the deliverable

| Deliverable | Available evidence / remaining bar |
|---|---|
| Relocation-recovery toolkit and actionable implementation roadmap | Available now, with the explicit prerequisites and limits below. No need to wait for all Mickey source cleanup or native contracts. |
| Bounded current-module generation/final-link starter | Implemented and historically reproduced for the documented module and live edit. Manual symbolic-producer preparation is required. |
| General reference builder | Not yet proved. Requires the source-owned graph, bindings, registry, layout and independence checkpoints below. |
| Overall Mickey completion | The [structural exit criteria](OVERLAY_STRUCTURE_SPRINT.md#structural-exit-criteria), [source cleanup](cleanup-queue.md), [native contracts](native-boundaries.md) and [release checks](CONTRIBUTING.md#checks) remain required. This handover does not waive any of them. |

## Inputs and setup

Bring your own legally available JFG US ROM; expected SHA1 is `493ced9008dbe932d6e91179b68e8630cf23a023`. Use an independent private checkout at the exact JFG snapshot and its ordinary matching build. Keep generated source/assembly, extracted payloads, maps, objects, reports and ROMs private. Do not add the demonstration's extracted function bodies or a ROM-derived name/table dump to this kit.

The verified environment used IDO 5.3, asm-processor `f3b2f85a5bc7e8ae2030ea5a83ea8aa0bcbede03`, GNU Binutils `2.46.0.20260210`, Python 3.14.7, Splat 0.50.0 and spimdisasm 1.42.3. The toolchain and Python environment were copied from an available installation. That setup built successfully but did not yield a hermetic fresh-install lockfile; incomplete pip prevented `pip freeze`. The portable kit itself uses the Python standard library and Python 3.11+ for TOML configuration. Its target build still requires the real compiler, assembler, extraction dependencies and scripts.

All commands below are templates: replace `<jfg>`, `<kit>`, `<pilot>` and `<binutils>` before running. Use private output paths without whitespace, double quotes or backslashes for the bounded final-link recipe. Keep global kit options before the subcommand.

From the target checkout, use its ordinary procedures, with the same explicit binutils override used in its matching build:

```sh
make extract CROSS=<binutils>/mips64-elf-
make CROSS=<binutils>/mips64-elf-
```

Require the resulting ordinary ROM to have the expected SHA1. Preserve the actual compiler and submodule pins, environment package metadata and build commands. This is a required matching baseline, not an installation script or a promise that untested tool versions work.

## Start with recovery and measurement

Run from the kit checkout:

```sh
python3 tools/runlink --config jfg-us --root <jfg> check
python3 tools/runlink --config jfg-us --root <jfg> census -v
python3 tools/runlink --config jfg-us --root <jfg> generate --mode objects
python3 tools/runlink --config jfg-us --root <jfg> generate --mode oracle
```

`check` tests byte round trips. `census` tests the rules against the shipped format. `objects` measures current object identities; `oracle` is a diagnostic that borrows shipped identities. A residual-assisted pass is not source-only generation.

To inspect resident trap identities, rebuild the affected resident C object targets with `BUILD_DIR=build_unpatched PATCH_SYMBOLS=:`. This is an explicit manual object-inventory step: the demonstrated snapshot rebuilt 21 C objects; its handwritten refract-output object was not rebuilt. Do not describe this as an automatically supplied complete unpatched build. Then use:

```sh
python3 tools/runlink --config jfg-us --root <jfg> generate --mode objects \
  --unpatched build/=build_unpatched/ -v --limit 100000
```

The historical snapshot covered 21,302 of 27,515 runtime records. It had 6,213 missing source relocations and 3,832 extra object relocations. Those are historical diagnostics, not a current upstream progress report. Use [JFG sections 3–4](runlink-jfg.md#3-regeneration-from-jfgs-build) to distinguish missing surface, incorrect identities, order and section boundaries before changing code.

## The executable starter: manual producer, then generated tables and final link

The `module-recipe` command consumes a **locally authored, symbolically corrected module ELF**. It does not reconstruct the C, recover every identity automatically, or accept an uncorrected extraction as equivalent input.

### Manual prerequisite 1 — preserve complete symbolic producers

In a private pilot directory, preserve both complete functions of the documented module in their original order. Authenticate the resident import through JFG's own relocation/export/provider evidence and correct its symbolic identity. Preserve genuine function type/extent metadata and complete text coverage. For a separately preserved changed producer, introduce the documented genuine live four-byte float datum and its symbolic load; the datum must affect the computation. Do not add unused data or patch compiled instructions. The extracted function bodies and their corrected producer are not shipped by this kit.

### Manual prerequisite 2 — assemble and link the producer normally

Use the actual matching assembler and pinned asm-processor prelude. The observed intermediate script is ordinary zero-based section linking:

```ld
SECTIONS {
  .text 0 : { *(.text) . = ALIGN(8); }
  .data 0 : { *(.data) . = ALIGN(8); }
  .bss 0 (NOLOAD) : { *(.bss) . = ALIGN(8); }
}
```

Its BSS clause does not mean this recipe admits nonempty BSS. Save the script as `<pilot>/module.ld`. Set `<variant-dir>` to `<pilot>/baseline` for the baseline producer and `<pilot>/changed` for the edited producer, preserving both source/output sets. From the target checkout, run these templates once per variant:

```sh
<binutils>/mips64-elf-as -march=vr4300 -32 -G0 \
  -I include tools/asm-processor/prelude.inc \
  <variant-dir>/module.s -o <variant-dir>/producer.o
<binutils>/mips64-elf-ld -EB -m elf32ebmip -r \
  -T <pilot>/module.ld -o <variant-dir>/module.elf <variant-dir>/producer.o
```

### Supplied consumer — generation and ordinary final linking

Create the existing bounded declaration locally:

```json
{
  "schema": "jfg-current-module-example-v1",
  "module": 79,
  "imports": { "hitInitObjectHit": 168 },
  "exports": { "buoyInit": 1359, "buoyControl": 1472 }
}
```

These declarations select explicitly declared EXISTING ABI identities, checked against the pinned compatibility registry; they cannot freely assign new rows or exports. They contain no object addresses, capacities or selected layout. Account for every existing selected export. From the kit checkout:

```sh
python3 tools/runlink --config jfg-us --root <jfg> module-recipe \
  --elf <pilot>/baseline/module.elf --declarations <pilot>/bindings.json \
  --out <pilot>/recipe-baseline --link --expect-retail \
  --cross <binutils>/mips64-elf-
python3 tools/runlink --config jfg-us --root <jfg> module-recipe \
  --elf <pilot>/changed/module.elf --declarations <pilot>/bindings.json \
  --out <pilot>/recipe-changed --link --cross <binutils>/mips64-elf-
```

Each output directory must be fresh. `--expect-retail` requires `--link`; omitting `--link` only generates metadata inputs. The command writes current section bytes, both runtime tables, headers and ORT, then uses wrappers, the ordinary final linker, objcopy and the existing checksum tool. It does not execute the game.

Preserve `receipt.json`, `link-receipt.json`, source, declarations, commands, original objects, linked producer, generated inputs and final map/image privately. Demand:

- full baseline ROM equality;
- exact current selected section bytes and all ordered relocation identities;
- real sized exported definitions and current export offsets;
- unchanged 156 unselected images/records/header fields except computed ROM positions;
- unchanged resident records/name payloads and independently explained resident boundary relocations;
- valid final decoding, actual terminal fill and overflow refusal.

The documented live edit grew the module payload by24 bytes. Its historical edited ROM SHA1 is `ca357d4e64635c025dcefdf546ec1f9d8777b9c2`; that hash belongs to that exact producer edit, not arbitrary edited input. New runs need their own current hashes and receipts.

## Authority matrix

| Component | Present authority | What remains before a general source-driven builder |
|---|---|---|
| Selected module bytes/records/exports | Current corrected producer ELF plus authored import/export rows | General compiler/input-graph frontend; no selected old image/table/header as fallback |
| Initial producer | Manual symbolic assembly and ordinary linker script | Reusable current C/ASM producer recipes with real source/dependency/flags holds |
| Unselected 156 modules | External matching build/compatibility data | Current declared source/object graph for all modules claimed source-driven |
| Export identities and names | Existing registry/name authority plus selected declarations | One authored ABI registry resolved through current real definitions; generate JFG name offsets/blob from it |
| Resident code and homes | Matching resident objects, ordinary script and compatibility authority | Authenticated source/object providers; explicit fixed-address hardware declarations where proved |
| BSS/RO/runtime roles | Only the demonstrated empty cases | Genuine allocations and source-derived runtime bases; compiler section spelling alone is insufficient |
| Placement/layout/fill | Computed selected layout inside a held outside build, with retained boundary/fill authority | Current declared input order/alignment/loader limits and derived cartridge packing, with no old-layout authority |
| Fixed-layout MSU final route |198 generated/0 retained metadata inputs in the final link | Its retained bootstrap and borrowed identities still prevent a general source-only claim |
| Source quality/native behavior | Separate C/ABI/bounds/lifetime/hardware contracts | Continued review; static relinking does not prove gameplay, GPU safety or native equivalence |

Current JFG admission requires correctly flagged zero-based aligned text/data, complete sized functions, resident calls, and at most one genuine four-byte local datum plus ordinary alignment. It refuses nonempty BSS/RO, data-site references, internal calls, nonzero addends and other owner shapes. It has no admitted module/export addition/removal or entry-point interface. The script's ability to spell a section is not evidence that its contents are supported.

## Next implementation work, in dependency order

1. **Declare the graph and ABI.** Extend current declaration concepts to ordered real C/ASM/resource inputs, entry names, export owners/kinds and logical runtime section roles. Source registry rows can be explicit stable ABI policy; there is no need to guess the original global sorting algorithm. Derive values and extents from actual current objects/maps. Preserve per-input original relocation order before GNUld regrouping. Reject missing/duplicate/foreign owners and undeclared imports. Keep opaque resources explicit; they are not fake C arrays.
2. **Resolve real providers and stored operands.** Prove FUNCTION/DATA/BSS targets through genuine definitions and actual source/object contracts. Separate a definition's physical allocation from the loader's stored section-relative operand. Support assets through declared real resource inputs and typed consumer contracts, not old-site backfill or zero proxies. Audit reserved selectors independently; the historical `__osRdbSend`/`osTvType` cases are not permission for arbitrary ABS targets. Unproved capacity, original type and native lifetime remain explicit.
3. **Build and prove one reusable whole path.** Reuse `rules.build_module`, `codec.layout_modules`, the name encoder and ordinary final linker. Prove baseline equality, real code, DATA, BSS and export growth, nonadjacent selected modules and removal refusal for both code and initializer references. Remove selected old inputs before setup, test warm request/input changes and prove recovery after failure. Then remove retained-bootstrap/outside-layout authority across the scope actually claimed. A per-module allowlist or successful arithmetic-only example is not generality.

MSU already provides useful mechanisms in `tools/runlink_metadata_link.py`, `tools/link_overlay_module.py`, `tools/proof_provenance.py` and `tools/runlink_pack.py`. They are project-specific reference code, not a ready JFG adapter. Keep its retail strict route separate from edited outputs; do not add an allow-difference switch to matching acceptance. Choose real source/provider capabilities by proof class, not repeated program-specific admission exceptions.

The MSU edited frontend now accepts an [ordered C-input vector](runlink-toolkit.md#ordered-c-inputs-in-the-msu-edited-builder) in its existing ABI registry. The reviewed overlay97 split and merge-back preserve original per-input relocation identities through an ordinary aggregate link, with a live cross-input data consumer and a missing-provider refusal. This supplies a bounded graph mechanism for step 1. The subsequent overlay75 split also resolves a real private BSS provider internally, preserving all 31 records and every ABI row. It admits contiguous sized GLOBAL NOBITS allocations with explicit compiler-tail and loader-word checks. COMMON, GP use, static-symbol index transformations and other unsupported roles remain excluded. These bounded mechanisms do not extend the portable JFG `module-recipe` interface or complete steps 2–3.

## Handover timing and completion claims

This roadmap and bounded starter can be handed over now, with the manual prerequisites above. A polished public package is one focused documentation/review batch. Its publication depends on that review and the ordinary gates; this is not an ETA for the general builder, the remaining linked Mickey requirements.

There is no defensible full-builder calendar estimate from a percentage of initialized bytes. The next forecast checkpoint is an accepted graph/provider contract and a real end-to-end capability demonstrating the missing authority classes with selected old metadata unavailable. Record each actual first blocker, then estimate remaining mechanism work from those results. Keep the final independent cold/release audit as a separate stage after implementation closes.
