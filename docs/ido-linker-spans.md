# IDO linker spans and address reuse

Companion to the [IDO playbook](ido-learnings.md). This is a measured source
maintenance result with exact untouched compiler output and a real linked-image
proof; it earns no new executable credit.

- **Symptom:** replacing a linker-boundary pointer difference with explicit
  integer-address subtraction changes address reuse across allocation/copy
  calls, grows the frame and changes instruction/relocation shape although all
  retail addresses agree.
- **Mechanism:** integer conversions can expose shared numeric boundary
  expressions to common-subexpression elimination across calls. Retained
  addresses compete with actual byte counts for registers and stack homes.
  A signed difference can preserve the intended representable arithmetic
  domain without preserving expression identity or allocation.
- **Lever:** model each independently meaningful linker span with its actual
  start/end symbols. The next span's start is not the current span's end merely
  because the two coincide in one layout. Use representable target-width
  integer-address subtraction; prove untouched compiler bytes, ordered resolved
  identities, real linker geometry and the final image. Independent real spans
  handle future gaps and can remove accidental cross-span address reuse.
- **Limits:** the ends must already be real linker boundaries with semantic
  roles. Do not invent aliases, volatile carriers, flags or site patches to
  influence optimization. An exact rename-only control does not justify
  retaining invalid subtraction between unrelated C arrays. Neither symbol
  spelling nor integer conversion is assumed inert. This is a target 32-bit
  linker-address convention, not a claim about arbitrary host pointers.
