# IDO buffer contracts and float diagnostics

Companion to the [generic IDO playbook](ido-learnings.md); the same evidence and
source/relocation/ROM acceptance rules apply.

### Pointer-valued buffer contracts preserve argument affinity

- **Symptom:** correcting a prototype or converting a pointer to an integer
  call argument can add an argument move or change nearby scheduling, even
  when both representations occupy one o32 word.
- **Mechanism:** measured stock IDO builds retain pointer/integer expression
  categories in argument affinity. A conversion can split the pointer
  producer from its argument web. This is an observed code-generation
  relationship, not identification of a particular compiler pass.
- **Lever:** authenticate the value domain first. Give RAM transfer buffers
  one compatible pointer-valued provider/caller contract and use byte
  pointers for staging within an allocation. Keep explicit conversions for
  actual encoded address holders; ROM offsets, sizes and indices retain
  their integer domains. Do not preserve incompatible prototypes or add
  aliases, guards or carrier storage to repair code generation.
- **Limits:** preserve ABI widths, signed alignment behavior, call order,
  queue effects and allocation lifetime. Prove byte arithmetic stays inside
  its real allocation under the trusted-data preconditions. Require exact
  raw output, ordered relocation identities and linked ROM after migration.
  This does not establish host-width compatibility or malformed-input safety.

### A finite maximum float can still produce a range diagnostic

- **Symptom:** the configured toolchain warns on a decimal spelling of the
  largest finite single-precision value even when its emitted pool value
  is that exact finite maximum.
- **Mechanism:** this diagnostic and the final target representation can
  disagree in the pinned IDO toolchain; the diagnostic alone does not prove
  that the object contains infinity.
- **Lever:** inspect the untouched compiler pool and prove the owned linked
  range. Use an ordinary finite decimal literal with the intended rounding;
  retain the diagnostic honestly rather than changing shared warning flags.
- **Limits:** this observation is specific to the configured compiler and
  assembler. It does not excuse an actual overflow, permit pool patches or
  establish identical behavior in another compiler.
