# 0021. Link complete source modules with runtime section-relative addresses

Status: Accepted
Date: 2026-10-23

## Context

The runtime loads a module's initialized text, data and read-only data as one
body. Its stored operands use offsets relative to their own input section;
runtime relocation records supply the final module or exported-symbol base.
A conventional host link that assigns every input section one combined VMA
changes those operands. Retaining a binary pool and externalizing the compiler
pool hid this distinction while text was being reconstructed.

The overlay 24 pilot has exact compiler-emitted text and a literal pool that
matches the retained initialized data. Source ownership should remove that
retained pool without changing function bodies or introducing offset patches.

## Decision

Allow an explicitly selected complete translation-unit module to pass through
`tools/link_overlay_module.py`. Preserve the original compiler object. Prepare
only symbol and relocation metadata: undefined references must name declared
ORT exports, and references to an exported definition in the same object use
a separate zero binding while retaining the actual definition. Private targets
retain ordinary local linking. Unknown references fail closed.

Use ordinary GNU ld to link `.text`, `.data`, `.rodata` and `.bss` independently
at zero, with initialized load addresses concatenated in runtime body order.
These zero bases represent the actual stored runtime operands, rather than
substitute addresses used only to obtain a diagnostic score. The serialized
relocation tables are subsequent standalone ROM segments, outside the body's
loadable sections and BSS extent.

Retain the ordinary linker output, explicit bindings, script and command.
After localizing external zero definitions, convert its ELF container from
ET_EXEC to ET_REL by changing only ELF header type, entry and program-header
metadata. All section payloads and symbol/section tables remain unchanged.
The final ROM link consumes this container. This conversion models the
original module format; it is distinct from per-function instruction editing,
blanket unresolved-symbol suppression or hand-authored relocation-site lists.

## Acceptance and limits

The source-module promotion route freshly compiles the configured ordinary C,
replays the actual module linker command with pinned tools and export names,
and authenticates compiler, prepared, linked and converted objects. It checks
the configured allocated sections and symbols against that reproduction.
Source section ownership and canonical exported definitions independently
resolve every original module relocation; the shipped site never supplies its
identity. Require exact count, type, offset and identity against all runtime
records, exact initialized module bytes in both linked ELF and ROM, and exact
BSS extent. Keep configured relocation count zero explicit; do not relabel a
stripped object as retained compiler relocation evidence.

A cross-overlay BSS export requires a unique global definition in actual
nonempty allocated writable NOBITS storage, with the linked module bytes and
section capacity authenticated against its runtime header. Translate the
symbol's offset within that section onto the runtime text-plus-data prefix;
an older host layout may place serialized tables before the BSS section.
Neither an encoded symbol name nor an ABS assignment supplies that identity.
An explicit physical loader reserve can support such an unsized export while
its unknown contents remain borrowed; it earns no reconstructed C ownership.

Existing function geometry, provenance and full-ROM gates still apply. No
instruction bytes may be edited to obtain a match. The linker may perform its
ordinary relocations on untouched compiler instructions. The initial selection
is overlay 24; incomplete, multi-object or otherwise unsupported modules remain
on the existing flow until their own evidence proves this route. The raw
compiler objects remain the packer's relocation authority during this staged
transition, and remaining borrowed layout or table residuals stay explicit.

A separately selected complete data-only input can use the same linker for
its declared ORT imports. The overlay 8 tuning-pointer tables are this narrow
case: no instruction or BSS payload, one complete genuine initialized input,
and a separate artifact directory for that source. Require fresh untouched
compiler output, the ordinary helper receipt, a unique final-map contribution,
and runtime identities independently anchored to current source-owned export
definitions. Keep the twenty original compiler R32 relocations explicit; the
configured object has zero relocations after linking. Hash the actual recipe,
helper, export names and tools when recording this proof. This does not relax
the complete main-module promotion guard or authorize incomplete-body padding,
section trimming or handwritten relocation-site bindings. Data-only inputs
remain outside the packer's current producer discovery, so source-only metadata
borrowing remains explicit until that separate producer capability is proved.
The packer's supported-case guard nevertheless freshly compiles and replays
selected pure ORT-pointer inputs, authenticates their C providers and unique
map contributions, and refuses any source/runtime identity disagreement.
Warm metadata dependencies include those raw/source/provider/header/tool inputs,
so unchanged zero pointer operands cannot hide a changed source target. LOCAL,
grouped and implicit-literal inputs remain unsupported by this guard.

Producer capture pins its actual bootstrap/final ELF-map pair and ordinary
link command, source, headers, recipe and tools before and after reproduction.
It cannot require the generated final-metadata output it is producing. Its
explicit producer receipt is distinct from promotion proof; ordinary promotion
capture still requires the current final-metadata proof. This lifecycle boundary
does not relax initialized-byte, allocation, relocation or identity checks.

## Consequences

Compiler-emitted initialized data can become canonical without a retained
binary pool, new per-function shaping, or executable credit for data bytes.
Synthetic regression coverage distinguishes external and same-module exported
calls from private calls and exercises separate data/read-only-data bases.
This establishes the source-to-module boundary needed for subsequent table
generation; it does not claim that all modules or generated tables are closed.
