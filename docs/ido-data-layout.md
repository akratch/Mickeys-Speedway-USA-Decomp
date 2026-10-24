# IDO initialized storage and compiler pools

These entries extend the [IDO playbook](ido-learnings.md). Each separates a
measured compiler mechanism from its source lever and limits. Original object
semantics, untouched compiler output, relocation identities and real linked
ownership remain independent requirements.

### Const qualification does not select the initialized section

- **Symptom:** a named floating constant has the right value but occupies
  `.data`, while the runtime LOCAL records require a separate `.rodata` base.
- **Mechanism:** in a measured IDO 5.3 TU at `-O2 -mips2 -32 -G0`, with
  `-Xcpluscomm -non_shared -Wab,-r4300_mul`, both global `const f32` and
  `static const f32` definitions were emitted into `.data`. Float literal
  expressions emitted the separate compiler-owned `.rodata` pool. A `const`
  qualifier constrains C accesses; it is not a section-placement directive.
- **Lever:** distinguish a stored variable from a private literal value.
  Where the references prove a private constant with no writes, exports or
  address uses, a named float literal macro can recover the compiler pool
  without inventing a storage object. Compare section bytes and relocation
  bases as well as instruction bytes.
- **Limits:** this was measured for scalar floats in a TU with an initialized
  integer global, not for arrays, strings, other optimization modes or other
  IDO versions. Substituting a literal for mutable or addressable storage is
  not equivalent. An exact raw pool also does not prove that the configured
  host link consumes it; its physical ownership and runtime base still need
  a linked proof.

### Static const aggregate arrays can split the initialized sections

- **Symptom:** typed tables reproduce their values but the combined initialized
  extent grows and the runtime section bases disagree.
- **Mechanism:** with IDO 5.3 at `-O2 -mips2 -32 -G0 -Xcpluscomm
  -non_shared`, static const arrays of triangle and RGB structs were placed in
  `.rodata`; unqualified static or global arrays occupied `.data`. Splitting
  them from mutable vectors and scalars added independent section alignment.
- **Lever:** recover each table's actual fields and access behavior, then
  compare `.data`, `.rodata`, their alignment and relocation bases separately.
  Do not infer a target section solely from read-only uses or scalar-constant
  behavior, and do not add padding objects to repair a section split.
- **Limits:** this measured aggregate case does not establish global const
  array placement, other optimization modes or other IDO versions. A typed
  initialized/BSS layout proof alone does not establish exact function code
  or configured ownership.

### Real read-only string context can recover a compiler-pool base

- **Symptom:** a compiler table has the right entries but needs an unexplained
  base addend or appears to leave an unmatched alignment suffix.
- **Mechanism:** measured IDO 5.3 at `-O2 -mips2 -32 -G0 -Xcpluscomm
  -non_shared -Wab,-r4300_mul` places genuine named global `const char[]`
  objects before the compiler tables in the same `.rodata` input. Distinct
  named objects can retain distinct bases despite equal text. Recovering that
  context changes the compiler's own table addends and final alignment while
  preserving function geometry and every table target.
- **Lever:** recover actual preceding strings and audit their read/write and
  retention evidence. Compile the complete natural input, then compare every
  section byte, original relocation and linked target. Keep the real zero-size
  character symbols; source literals establish their bounds independently.
- **Limits:** this does not generalize scalar-float placement, establish
  original qualifiers from section names, or justify invented objects, padding
  or uses. An authentic unreferenced diagnostic still needs explicit disclosure
  that its original declaration and retention intent are inferred. Whole-input
  byte identity does not establish direct host binding: metadata-only
  externalization can leave neutral zero-symbol references that require separate
  raw-identity and real linked/runtime proofs.

### First declarations can change BSS order without changing its size

- **Symptom:** text, initialized bytes, function geometry and total BSS size
  agree, but references to genuine BSS objects resolve to different offsets.
- **Mechanism:** a controlled IDO 5.3 experiment at the production `-O2 -mips2
  -32 -G0 -Xcpluscomm -non_shared -Wab,-r4300_mul` settings introduced direct
  references to arrays declared before their trailing definitions. Those arrays
  moved ahead of the other BSS objects. Placing all genuine BSS definitions in
  authenticated first-declaration order before their consumers, and removing
  redundant private externs, restored every object offset and reference target
  without changing text, initialized storage or the total BSS allocation.
- **Lever:** inspect the preprocessed declaration order, including headers,
  rather than only the order of definitions at the end of the source. Recover
  the real objects, types and extents first. Compare each BSS symbol's offset
  and size, then resolve every affected reference against its actual definition.
- **Limits:** this is a measured array/declaration case, not a universal BSS
  sorting rule. Exact size or executable bytes alone cannot prove object
  identity. Do not invent declarations, reserves or padding to force an order.
  A raw-object storage experiment does not establish a complete module link,
  runtime binding, header authority or native ABI compatibility.
