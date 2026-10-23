# IDO symbol extents and real storage

Companion to the [IDO playbook](ido-learnings.md). These findings come from
controlled synthetic compiles, with production-object corroboration where
stated. They concern object metadata and storage, not new executable credit.

## Partial aggregate initialization can underreport symbol size

- **Symptom:** a partially specified zero initializer emits the complete correct
  storage but reports an ELF `st_size` smaller than the C object.
- **Measured distinction:** a synthetic 92-byte aggregate allocates 92 bytes
  with both `{0}` and explicit real-field initializers. Their contents agree,
  but the former reports five bytes in `st_size` and the latter reports 92.
  This distinguishes allocation/type geometry from initializer-associated
  symbol metadata; it does not establish the compiler's internal cause.
- **Lever:** initialize the genuine fields explicitly when that restores correct
  metadata. Prove compiler-evaluated `sizeof`, symbol extent, section extent and
  emitted bytes separately. Do not invent fields, enlarge storage or patch a
  size just to satisfy a lookup tool.
- **Limits:** this is a measured aggregate case in the pinned toolchain, not a
  universal rule for IDO versions or initializers. Character-array initializers
  can also report zero `st_size`; that does not establish zero source storage.
  Their actual compiler-evaluated array bounds and physical placement still need
  proof. Correct contents alone do not establish a usable symbol extent, source
  ownership, runtime identity or a safe array access.

## Visible data directives do not describe every initialized byte

- **Symptom:** counting storage directives in the compiler's text listing
  understates the final read-only section and can mistake real storage for
  terminal alignment.
- **Mechanism:** the final assembler creates floating literals while expanding
  floating-point pseudo-instructions. A controlled switch-and-double example
  emits 24 bytes of explicit table directives but allocates 32 bytes of real
  read-only storage. The extra eight bytes are a literal, not padding. A
  production object likewise interleaves assembler-created literals with its
  explicit tables, so directive counting also misplaces later contributors.
- **Lever:** distinguish source object extents, explicit emitted reservations,
  assembler-created storage and section alignment. Authenticate each against
  the untouched ordinary object. Unknown implicit reservations must fail
  closed; an ELF symbol size or apparent zero tail is not a complete witness.
- **Limits:** explicit-data-only controls reproduce ordinary section rounding,
  but do not establish a general trimming rule. Neither a desired module span
  nor a matching retained slice supplies missing producer evidence.

## A text-assembly replay can preserve bytes while losing symbol types

- **Symptom:** replaying an unmodified compiler text listing reproduces every
  allocated byte and function boundary, yet ordered relocation identities
  differ because external function symbols become object symbols.
- **Measured distinction:** the assembly-file driver initially also changed
  code. Restoring the ordinary C pipeline's actual assembler arguments removed
  those byte differences, but not the symbol-type differences. Replaying the
  original retained binary compiler stream and symbol table through the final
  assembler reproduced the full ordinary object's allocated bytes, identities
  and function geometry.
- **Lever and limits:** preserve the actual command and original intermediates
  when diagnosing a pipeline. Byte equality alone does not authenticate symbol
  metadata. Do not normalize away the type difference, infer individual flag
  causality from a combined argument change, or substitute this diagnostic
  replay for the ordinary compiler acceptance path.
