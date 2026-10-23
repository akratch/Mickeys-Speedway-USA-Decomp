# IDO symbol extents and real storage

Companion to the [IDO playbook](ido-learnings.md). This finding comes from a
controlled synthetic compile without ROM inputs; it concerns object metadata,
not new executable matching credit.

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
