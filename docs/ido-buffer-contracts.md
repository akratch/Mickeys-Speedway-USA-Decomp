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
- **Lever:** check that the decimal literal is mathematically in range and
  rounds to the intended finite value. Inspect the untouched compiler pool
  and prove the owned linked range. Retain both front-end and assembler
  diagnostics rather than changing shared warning flags.
- **Limits:** the internal diagnostic cause is not established by the observed
  output. This observation is specific to the configured compiler and
  assembler. It does not excuse actual overflow, permit pool patches or word
  arrays, establish another compiler's behavior, or prove general floating-point
  input and exception contracts.

### An unused variadic tail can change the provider prologue

- **Symptom:** adding an unused trailing ellipsis makes a provider larger even
  though its body never calls `va_start` or `va_arg`.
- **Mechanism:** a controlled stock IDO compile at `-O2 -mips2 -32` kept a
  fixed pointer, integer and float prefix and the forwarding body unchanged.
  The ellipsis introduced argument-home stores and a larger frame. Compiler
  end alignment absorbed the growth, so the whole TU's text size stayed equal
  while the true function size and relocation positions changed.
- **Lever:** establish which arguments the actual provider consumes before
  choosing a callable contract. An ignored extra argument is not an output,
  and a variadic declaration is not automatically a free arity repair.
  Compile the provider definition as well as its callers; compare function
  geometry, ordered relocation identities and linked bytes. If the prologue
  differs, retain the measured ABI debt rather than compensate with inert
  locals or invented parameter roles.
- **Limits:** this is a measured signature/body in the pinned toolchain, not a
  rule for every variadic function or compiler mode. It does not recover the
  original declaration, make incompatible pointer views safe or justify
  moving fixed arguments into a variadic tail. Full-TU size alone is not a
  matching proof. A different compiler mode needs independent target evidence
  and the ordinary flag-impact review.

### Pointer representation can preserve a physical-address operand shape

- **Symptom:** replacing a cached-segment pointer adjustment with unsigned
  integer arithmetic changes address materialization and register allocation,
  even though the intended N64 address bits agree. Pointer arithmetic outside
  the referenced object is not a sound way to preserve that compiler shape.
- **Mechanism:** IDO distinguishes an address expression from an unsigned
  integer expression. A union containing a pointer and a signed word can
  expose the target representation without subtracting from the pointer.
  Writing the pointer member, then reading the signed member and XORing its
  sign bit preserved an exact static-geometry operand in a whole-module proof.
  The unsigned-word union alternative did not preserve the same output.
- **Lever:** use the pointer/signed-word union only for a real N64 address
  conversion, with explicit sequencing and an assertion that both pointer and
  signed word are 32 bits. Convert the resulting word to unsigned. Ordinary
  unsigned pointer-to-integer conversion remains the simpler first choice.
- **Limits:** this relies on the configured compiler's union representation
  behavior and the N64 two's-complement ABI. It is not a host-portability
  abstraction or permission to reinterpret unrelated storage. Keep command
  cursor lvalue types unchanged unless their actual allocation and consumers
  establish a shared type. Require whole-function and relocation proof; a
  matching local expression does not establish a matching function.


### Preserve the captured pointer when recovering its real pointee type

- **Symptom:** replacing a command-word view with a genuine typed member
  access can add a copy and shift later code, even when both forms read the
  same value.
- **Mechanism:** re-reading the containing object's pointer field changes the
  expression relative to reading through the already captured cursor. In the
  observed stock compile, one form materialized a temporary and copied it to
  the cursor surviving a call. Keeping the original capture removed that copy.
- **Lever:** authenticate the actual pointee and alignment, convert the
  captured pointer value back to the real object type, and access its member.
  Preserve the original capture and evaluation flow.
- **Limits:** this does not justify casting a pointer object, inventing a
  carrier or treating an arbitrary byte pointer as an authenticated object.
  Object provenance and alignment need independent evidence. This is one
  measured recovery, not a guarantee for other type changes. Reprove complete
  allocated surfaces, geometry, ordered identities and linked bytes. No saved
  compiler IR was available; the mechanism is a compiled-dataflow explanation.


See also [storage width and callable return width](ido-learnings.md#storage-width-does-not-establish-callable-return-width).

## Inspect BSS definition placement before trusting aggregate size

- **Symptom:** a stock o32 build emits the expected total BSS size, but the
  genuine objects occupy the wrong individual offsets.
- **Mechanism:** in one measured IDO case, a contiguous group of uninitialized
  definitions placed after function uses was emitted in use order. Moving the
  same group before those uses restored its definition order without changing
  executable bytes.
- **Lever:** put real file-scope definitions before their consumers, preserving
  their types, order and actual extents; inspect every symbol offset and the
  complete raw/configured allocation and relocation surfaces.
- **Limits:** this is empirical o32 behavior, not an ordering guarantee or proof
  of an original declaration. Aggregate size alone is insufficient. Do not add
  fake storage, padding, attributes or unrelated source changes to force layout;
  retain independent linked-address and byte-identity proof.

### Complete-word counts from a bounded byte span

- **Symptom:** an explicit signed byte-distance division by eight adds code
  compared with scaled pointer subtraction.
- **Mechanism:** the compiler corrects negative dividends to implement signed
  division's truncation toward zero. A scaled pointer difference already has
  the element-size divisibility property.
- **Lever:** when an independently justified interface requires a nonnegative,
  representable distance in complete eight-byte words, a direct right shift
  expresses that unit conversion without the negative-dividend correction.
  Prove the provider and every caller with their real types and linked output.
- **Limits:** this is no negative-shift equivalence or general pointer rewrite.
  Character-pointer conversion does not make unrelated backing objects
  subtractable or prove alignment, bounds, lifetime or host representation.
  Add no guard, carrier, formal, flag or new precondition merely for a match.

