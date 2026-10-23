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

## Consequences

Compiler-emitted initialized data can become canonical without a retained
binary pool, new per-function shaping, or executable credit for data bytes.
Synthetic regression coverage distinguishes external and same-module exported
calls from private calls and exercises separate data/read-only-data bases.
This establishes the source-to-module boundary needed for subsequent table
generation; it does not claim that all modules or generated tables are closed.
